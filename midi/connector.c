#include <stdlib.h>
#include "connector.h"
#include "midi/midi.h"

typedef int (*RelayFn)( void *, struct MIDIMessage *  );
typedef int (*ConnectFn)( void *, struct MIDIConnector * );
typedef int (*DisconnectFn)( void *, struct MIDIConnector * );

extern struct MIDIConnectorTargetDelegate MIDIDeviceInConnectorDelegate;
extern struct MIDIConnectorSourceDelegate MIDIDeviceOutConnectorDelegate;
extern struct MIDIConnectorSourceDelegate MIDIDeviceThruConnectorDelegate;

extern struct MIDIConnectorTargetDelegate MIDIDriverSendConnectorDelegate;
extern struct MIDIConnectorSourceDelegate MIDIDriverReceiveConnectorDelegate;

/**
 * @ingroup MIDI
 * @brief A connection between two MIDI objects.
 * The MIDIConnector acts much like a MIDI cable. It passes MIDIMessages
 * between the collaborators of a MIDI session.
 */
struct MIDIConnector {
  size_t refs;
  void * target;
  void * source;
  struct MIDIConnectorTargetDelegate * target_delegate;
  struct MIDIConnectorSourceDelegate * source_delegate;
};

static int _target_connect( struct MIDIConnector * connector ) {
  if( connector->target != NULL &&
      connector->target_delegate != NULL &&
      connector->target_delegate->connect != NULL )
    return (connector->target_delegate->connect)( connector->target, connector );
  return 0;
}

static int _source_connect( struct MIDIConnector * connector ) {
  if( connector->source != NULL &&
      connector->source_delegate != NULL &&
      connector->source_delegate->connect != NULL )
    return (connector->source_delegate->connect)( connector->source, connector );
  return 0;
}

static int _target_disconnect( struct MIDIConnector * connector ) {
  void * target = connector->target;
  connector->target = NULL;
  if( target != NULL &&
      connector->target_delegate != NULL &&
      connector->target_delegate->disconnect != NULL )
    return (connector->target_delegate->disconnect)( target, connector );
  return 0;
}

static int _source_disconnect( struct MIDIConnector * connector ) {
  void * source = connector->source;
  connector->source = NULL;
  if( source != NULL &&
      connector->source_delegate != NULL &&
      connector->source_delegate->disconnect != NULL )
    return (connector->source_delegate->disconnect)( source, connector );
  return 0;
}

#pragma mark Creation and destruction
/**
 * @name Creation and destruction
 * Creating, destroying and reference counting of MIDIConnector objects.
 * @{
 */

/**
 * @brief Create a MIDIConnector instance.
 * Allocate space and initialize a MIDIConnector instance.
 * @public @memberof MIDIConnector
 * @return a pointer to the created connector structure on success.
 * @return a @c NULL pointer if the connector could not created.
 */
struct MIDIConnector * MIDIConnectorCreate() {
  struct MIDIConnector * connector = malloc( sizeof( struct MIDIConnector ) );
  MIDIPrecondReturn( connector != NULL, ENOMEM, NULL );

  connector->refs = 1;
  connector->target = NULL;
  connector->source = NULL;
  connector->target_delegate = NULL;
  connector->source_delegate = NULL;
  return connector;
}

/**
 * @brief Destroy a MIDIConnector instance.
 * Free all resources occupied by the connector and detach all
 * connected objects.
 * @public @memberof MIDIConnector
 * @param connector The connector.
 */
void MIDIConnectorDestroy( struct MIDIConnector * connector ) {
  MIDIPrecondReturn( connector != NULL, EFAULT, (void)0 );
  _source_disconnect( connector );
  _target_disconnect( connector );
  free( connector );
}

/**
 * @brief Retain a MIDIConnector instance.
 * Increment the reference counter of a connector so that it won't be destroyed.
 * @public @memberof MIDIConnector
 * @param connector The connector.
 */
void MIDIConnectorRetain( struct MIDIConnector * connector ) {
  MIDIPrecondReturn( connector != NULL, EFAULT, (void)0 );
  connector->refs++;
}

/**
 * @brief Release a MIDIConnector instance.
 * Decrement the reference counter of a connector. If the reference count
 * reached zero, destroy the connector.
 * @public @memberof MIDIConnector
 * @param connector The connector.
 */
void MIDIConnectorRelease( struct MIDIConnector * connector ) {
  MIDIPrecondReturn( connector != NULL, EFAULT, (void)0 );
  if( ! --connector->refs ) {
    MIDIConnectorDestroy( connector );
  }
}

/** @} */

#pragma mark Connector attachment
/**
 * @name Connector attachment
 * Methods to obtain connectors that are attached to the driver.
 * @{
 */

/**
 * @brief Detach the target from a connector.
 * @public @memberof MIDIConnector
 * @param connector The connector.
 * @retval 0  on success.
 * @retval >0 if the target could not be detached.
 */
int MIDIConnectorDetachTarget( struct MIDIConnector * connector ) {
  MIDIPrecond( connector != NULL, EFAULT );
  return _target_disconnect( connector );
}

/**
 * @brief Attach a target with a given delegate.
 * @public @memberof MIDIConnector
 * @param connector The connector.
 * @param target    The target to attach.
 * @param delegate  The delegate to use for connecting and disconnecting.
 * @retval 0  on success.
 * @retval >0 if the target could not be attached.
 */
int MIDIConnectorAttachTargetWithDelegate( struct MIDIConnector * connector, void * target,
                                           struct MIDIConnectorTargetDelegate * delegate ) {
  MIDIPrecond( connector != NULL, EFAULT );
  _target_disconnect( connector );
  connector->target = target;
  connector->target_delegate = delegate;
  return _target_connect( connector );
}

/**
 * @brief Detach the source from a connector.
 * @public @memberof MIDIConnector
 * @param connector The connector.
 * @retval 0  on success.
 * @retval >0 if the source could not be detached.
 */
int MIDIConnectorDetachSource( struct MIDIConnector * connector ) {
  MIDIPrecond( connector != NULL, EFAULT );
  return _source_disconnect( connector );
}

/**
 * @brief Attach a target with a given delegate.
 * @public @memberof MIDIConnector
 * @param connector The connector.
 * @param source    The source to attach.
 * @param delegate  The delegate to use for connecting and disconnecting.
 * @retval 0  on success.
 * @retval >0 if the source could not be attached.
 */
int MIDIConnectorAttachSourceWithDelegate( struct MIDIConnector * connector, void * source,
                                           struct MIDIConnectorSourceDelegate * delegate ) {
  MIDIPrecond( connector != NULL, EFAULT );
  _source_disconnect( connector );
  connector->source = source;
  connector->source_delegate = delegate;
  return _source_connect( connector );
}

/**
 * @brief Attach a device @c IN port as target.
 * Use the MIDIDeviceInConnectorDelegate to attach a device's @c IN port
 * as the connector's target.
 * @public @memberof MIDIConnector
 * @param connector The connector.
 * @param device    The device.
 * @see MIDIDevice
 * @retval 0  on success.
 * @retval >0 if the device could not be attached.
 */
int MIDIConnectorAttachToDeviceIn( struct MIDIConnector * connector, struct MIDIDevice * device ) {
  MIDIPrecond( connector != NULL, EFAULT );
  return MIDIConnectorAttachTargetWithDelegate( connector, device,
                                                &MIDIDeviceInConnectorDelegate );
}

/**
 * @brief Attach a driver as target.
 * Use the MIDIDriverSendConnectorDelegate to attach a driver
 * as the connector's target.
 * @public @memberof MIDIConnector
 * @param connector The connector.
 * @param driver    The driver.
 * @see MIDIDriver
 * @retval 0  on success.
 * @retval >0 if the driver could not be attached.
 */
int MIDIConnectorAttachToDriver( struct MIDIConnector * connector, struct MIDIDriver * driver ) {
  MIDIPrecond( connector != NULL, EFAULT );
  return MIDIConnectorAttachTargetWithDelegate( connector, driver,
                                                &MIDIDriverSendConnectorDelegate );
}

/**
 * @brief Attach a device @c OUT port as source.
 * Use the MIDIDeviceOutConnectorDelegate to attach a device's @c OUT port
 * as the connector's source.
 * @public @memberof MIDIConnector
 * @param connector The connector.
 * @param device    The device.
 * @see MIDIDevice
 * @retval 0  on success.
 * @retval >0 if the device could not be attached.
 */
int MIDIConnectorAttachFromDeviceOut( struct MIDIConnector * connector, struct MIDIDevice * device ) {
  MIDIPrecond( connector != NULL, EFAULT );
  return MIDIConnectorAttachSourceWithDelegate( connector, device,
                                                &MIDIDeviceOutConnectorDelegate );
}

/**
 * @brief Attach a device @c THRU port as source.
 * Use the MIDIDeviceThruConnectorDelegate to attach a device's @c THRU port
 * as the connector's source.
 * @public @memberof MIDIConnector
 * @param connector The connector.
 * @param device    The device.
 * @see MIDIDevice
 * @retval 0  on success.
 * @retval >0 if the device could not be attached.
 */
int MIDIConnectorAttachFromDeviceThru( struct MIDIConnector * connector, struct MIDIDevice * device ) {
  MIDIPrecond( connector != NULL, EFAULT );
  return MIDIConnectorAttachSourceWithDelegate( connector, device,
                                                &MIDIDeviceThruConnectorDelegate );
}

/**
 * @brief Attach a driver as source.
 * Use the MIDIDriverReceiveConnectorDelegate to attach a driver
 * as the connector's source.
 * @public @memberof MIDIConnector
 * @param connector The connector.
 * @param driver    The driver.
 * @see MIDIDriver
 * @retval 0  on success.
 * @retval >0 if the driver could not be attached.
 */
int MIDIConnectorAttachFromDriver( struct MIDIConnector * connector, struct MIDIDriver * driver ) {
  MIDIPrecond( connector != NULL, EFAULT );
  return MIDIConnectorAttachSourceWithDelegate( connector, driver,
                                                &MIDIDriverReceiveConnectorDelegate );
}

/** @} */

#pragma mark Message passing
/**
 * @name Message passing
 * Relaying MIDIMessage objects.
 * @{
 */

/**
 * @brief Relay a message to the attached target.
 * Pass the given message to the attached target using the target delegate's relay method.
 * This is should be called when an attached source wants to pass a message to the other end.
 * @public @memberof MIDIConnector
 * @param connector The connector.
 * @param message   The message.
 * @retval 0  on success.
 * @retval >0 if the message could not be relayed.
 */
int MIDIConnectorRelay( struct MIDIConnector * connector, struct MIDIMessage * message ) {
  MIDIPrecond( connector != NULL, EFAULT );
  if( connector->target == NULL ||
      connector->target_delegate == NULL ||
      connector->target_delegate->relay == NULL )
    return 1;
  return (connector->target_delegate->relay)( connector->target, message );
}

/** @} */
