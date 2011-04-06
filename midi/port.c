#include <stdlib.h>
#include <string.h>
#include "midi.h"
#include "list.h"
#include "port.h"

/**
 * @ingroup MIDI
 * @brief Endpoint for message based communication.
 */
struct MIDIPort {
  int refs;
  int mode;
  int valid;
  char * name;
  void * target;
  void * observer;
  MIDIPortReceiveFn * receive;
  MIDIPortInterceptFn * intercept;
  struct MIDIList * ports;
};

/**
 * @internal Parameter struct for apply function used for sending.
 */
struct MIDIPortApplyParams {
  struct MIDIPort * port; /**< source port */
  int    type; /**< message type */
  size_t size; /**< number of bytes pointed to by data */
  void * data; /**< message data */
};

/**
 * @brief Applier function to send to a port.
 * This is used when a port sends data to all connected ports.
 * If the port to send to was invalidated before, remove it from the
 * list of connected ports. Send the message to the port otherwise.
 * @private @memberof MIDIPort
 * @param item A pointer to the @c MIDIPort to send to.
 * @param info A pointer to the @c MIDIPortApplyParams structure
 *             that was passed to @c MIDIListApply.
 * @retval 0 on success.
 */
static int _port_apply_send( void * item, void * info ) {
  struct MIDIPort            * port   = item;
  struct MIDIPortApplyParams * params = info;
  if( port->mode & MIDI_PORT_INVALID ) {
    MIDIPortRetain( port );
    MIDIListRemove( params->port->ports, port );
    MIDIPortRelease( port );
    return 0;
  } else {
    return MIDIPortReceiveFrom( port, params->port, params->type, params->size, params->data );
  }
}

/**
 * @brief Applier function to release invalidated ports.
 * This is used to break retainment cycles.
 * @private @memberof MIDIPort
 * @param item A pointer to the @c MIDIPort to check.
 * @param info A pointer to the @c MIDIPort that is connected
 *             to the port to check.
 * @retval 0 on success.
 */
static int _port_apply_check( void * item, void * info ) {
  struct MIDIPort * port   = item;
  struct MIDIPort * source = info;
  if( port->mode & MIDI_PORT_INVALID ) {
    /* retain the port, before removing to avoid recursion
     * release the port *after* it was removed from the list */
    MIDIPortRetain( port );
    MIDIListRemove( source->ports, port );
    MIDIPortRelease( port );
  }
  return 0;
}

#pragma mark Creation and destruction
/**
 * @name Creation and destruction
 * Creating, destroying and reference counting of MIDIDriver objects.
 * @{
 */

/**
 * @brief Create a MIDIPort instance.
 * Allocate space and initialize a MIDIPort instance.
 * @public @memberof MIDIPort
 * @param name    The name of the MIDIPort.
 * @param target  The target for receiving messages.
 * @param receive The callback for incoming messages.
 * @return a pointer to the created port structure on success.
 * @return a @c NULL pointer if the port could not created.
 */
struct MIDIPort * MIDIPortCreate( char * name, int mode, void * target,
                                  int (*receive)( void *, void *, int, size_t, void * ) ) {
  MIDIPrecondReturn( name != NULL, EINVAL, NULL );
  MIDIPrecondReturn( target != NULL, EINVAL, NULL );
  MIDIPrecondReturn( receive != NULL, EINVAL, NULL );
  struct MIDIPort * port = malloc( sizeof( struct MIDIPort ) );
  size_t namelen;
  MIDIPrecondReturn( port != NULL, ENOMEM, NULL );

  namelen = strlen( name );
  if( namelen > 128 ) namelen = 128;

  port->refs    = 1;
  port->mode    = mode;
  port->name    = malloc( namelen );
  port->target  = target;
  port->receive = receive;
  port->ports   = MIDIListCreate( (MIDIRefFn*) &MIDIPortRetain, (MIDIRefFn*) &MIDIPortRelease );

  if( port->name == NULL ) {
    free( port );
    return NULL;
  }
  if( port->ports == NULL ) {
    /* probably ENOMEM, in that case, error code is already set by MIDIList */
    free( port );
    return NULL;
  }

  strncpy( port->name, name, namelen );

  return port;
}

/**
 * @brief Destroy a MIDIPort instance.
 * Free all resources occupied by the port and release connected ports.
 * @public @memberof MIDIPort
 * @param port The port.
 */
void MIDIPortDestroy( struct MIDIPort * port ) {
  MIDIPrecondReturn( port != NULL, EFAULT, (void)0 );
  MIDIListRelease( port->ports );
  /* If we get problems with with access to freed ports we could
   * enable this temporarily ..
   * MIDIPrecondReturn( port->valid == 0, ECANCELED, (void)0 ); */
  free( port );
}

/**
 * @brief Retain a MIDIPort instance.
 * Increment the reference counter of a port so that it won't be destroyed.
 * @public @memberof MIDIPort
 * @param port The port.
 */
void MIDIPortRetain( struct MIDIPort * port ) {
  MIDIPrecondReturn( port != NULL, EFAULT, (void)0 );
  port->refs++;
}

/**
 * @brief Release a MIDIPort instance.
 * Decrement the reference counter of a port. If the reference count
 * reached zero, destroy the port. Before decrementing the reference
 * count check for invalidated connected ports and remove them to
 * break retain cycles.
 * @public @memberof MIDIPort
 * @param port The port.
 */
void MIDIPortRelease( struct MIDIPort * port ) {
  MIDIPrecondReturn( port != NULL, EFAULT, (void)0 );
  if( ! --port->refs ) {
    MIDIPortDestroy( port );
  } else {
    MIDIListApply( port->ports, port, &_port_apply_check );
  }
}

/** @} */

#pragma mark Connection management and message passing
/**
 * @name Connection management and message passing
 * Methods to connect ports and pass messages between them.
 * @{
 */

static int _observer_intercept( struct MIDIPort * port, int mode, int type, size_t size, void * data ) {
  MIDIPrecond( port != NULL, EFAULT );
  if( port->observer != NULL && port->intercept != NULL ) {
    return (*port->intercept)( port->observer, port, mode, type, size, data );
  } else {
    return 0;
  }
}

/**
 * @brief Connect a port to another port.
 * Connect a target port to a source port so that the target port will
 * receive messages sent from the source port.
 * @public @memberof MIDIPort
 * @param port   The source port.
 * @param target The target port.
 */
int MIDIPortConnect( struct MIDIPort * port, struct MIDIPort * target ) {
  MIDIPrecond( port != NULL, EFAULT );
  MIDIPrecond( target != NULL , EINVAL );
  return MIDIListAdd( port->ports, target );
}

/**
 * @brief Disconnect a port from another port.
 * Disconnect a connected port from another port.
 * @public @memberof MIDIPort
 * @param port   The source port.
 * @param target The target port.
 */
int MIDIPortDisconnect( struct MIDIPort * port, struct MIDIPort * target ) {
  MIDIPrecond( port != NULL, EFAULT );
  MIDIPrecond( target != NULL , EINVAL );
  return MIDIListRemove( port->ports, target );
}

/**
 * @brief Invalidate the port.
 * This has to be called by the instance that created the port,
 * when it is being destroyed or no longer available. A port that
 * has been invalidated will never again dereference the @c target
 * pointer that was passed during creation or call the given @c
 * receive function.
 * @public @memberof MIDIPort
 * @param port The port to invalidate.
 * @retval 0 on success.
 */
int MIDIPortInvalidate( struct MIDIPort * port ) {
  MIDIPrecond( port != NULL, EFAULT );
  _observer_intercept( port, MIDI_PORT_INVALID, 0, 0, NULL );
  port->mode   |= MIDI_PORT_INVALID;
  port->target  = NULL;
  port->receive = NULL;
  return 0;
}

int MIDIPortSetObserver( struct MIDIPort * port, void * target, MIDIPortInterceptFn * intercept ) {
  MIDIPrecond( port != NULL, EFAULT );
  port->observer  = target;
  port->intercept = intercept;
  return 0;
}

int MIDIPortGetObserver( struct MIDIPort * port, void ** target, MIDIPortInterceptFn ** intercept ) {
  MIDIPrecond( port != NULL, EFAULT );
  *target    = port->observer;
  *intercept = port->intercept;
  return 0;
}

/**
 * @brief Simulate an incoming message that was sent by another port.
 * @public @memberof MIDIPort
 * @param port   The target port.
 * @param source The source port.
 * @param type   The message type that was received.
 * @param data   The message data that was received.
 * @retval 0 on success.
 */
int MIDIPortReceiveFrom( struct MIDIPort * port, struct MIDIPort * source, int type, size_t size, void * data ) {
  int result;
  MIDIPrecond( port != NULL, EFAULT );
  MIDIPrecond( port->mode & MIDI_PORT_IN, EPERM );
  
  if( port->mode & MIDI_PORT_INVALID ) {
    /* invalidated ports don't receive messages. */
    return 0;
  } else {
    MIDIAssert( port->target  != NULL );
    MIDIAssert( port->receive != NULL );

    _observer_intercept( port, MIDI_PORT_IN, type, size, data );
    if( source != NULL ) {
      result = (*port->receive)( port->target, source->target, type, size, data );
    } else {
      result = (*port->receive)( port->target, NULL, type, size, data );
    }
    if( port->mode & MIDI_PORT_THRU ) {
      return result + MIDIPortSend( port, type, size, data );
    } else {
      return result;
    }
  }
}

/**
 * @brief Simulate an incoming message.
 * This is called whenever the port receives a new message and can be
 * used to simulate an incoming message.
 * @public @memberof MIDIPort
 * @param port The target port.
 * @param type The message type that was received.
 * @param size   The size of the message data.
 * @param data The message data that was received.
 * @retval 0 on success.
 */
int MIDIPortReceive( struct MIDIPort * port, int type, size_t size, void * data ) {
  return MIDIPortReceiveFrom( port, NULL, type, size, data );
}

/**
 * @brief Send a message to another port.
 * Send the given message to any other port. The target port does not have to be
 * connected to the source port.
 * @public @memberof MIDIPort
 * @param port   The source port.
 * @param target The target port.
 * @param type   The message type to send.
 * @param size   The size of the message data.
 * @param data   The message data to send.
 * @retval 0 on success.
 */
int MIDIPortSendTo( struct MIDIPort * port, struct MIDIPort * target, int type, size_t size, void * data ) {
  MIDIPrecond( port != NULL, EFAULT );
  MIDIPrecond( port->mode & MIDI_PORT_OUT, EPERM );
  
  if( port->mode & MIDI_PORT_INVALID ) {
    return 0;
  } else {
    _observer_intercept( port, MIDI_PORT_OUT, type, size, data );
    return MIDIPortReceiveFrom( target, port, type, size, data );
  }
}

/**
 * @brief Send the given message to all connected ports.
 * Use the apply mechanism of the list to send the given message
 * to all connected ports.
 * @public @memberof MIDIPort
 * @param port The source port.
 * @param type The message type to send.
 * @param size   The size of the message data.
 * @param data The message data to send.
 * @retval 0 on success.
 */
int MIDIPortSend( struct MIDIPort * port, int type, size_t size, void * data ) {
  struct MIDIPortApplyParams params;
  MIDIPrecond( port != NULL, EFAULT );
  MIDIPrecond( port->mode & MIDI_PORT_OUT, EPERM );
  
  if( port->mode & MIDI_PORT_INVALID ) {
    return 0;
  } else {
    _observer_intercept( port, MIDI_PORT_OUT, type, size, data );
    params.port = port;
    params.type = type;
    params.size = size;
    params.data = data;
    return MIDIListApply( port->ports, &params, &_port_apply_send );
  }
}

/** @} */
