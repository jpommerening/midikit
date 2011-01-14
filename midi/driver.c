#include <stdlib.h>
#include "driver.h"
#include "message.h"
#include "connector.h"

struct MIDIConnectorList;
  
struct MIDIDriver {
  size_t refs;
  struct MIDIDriverDelegate * delegate;
  struct MIDIConnectorList  * receivers;
  struct MIDIConnectorList  * senders;
  struct MIDIClock * clock;
};

#pragma mark Connector list management
/**
 * @internal
 * Connector list management.
 * @{
 */

struct MIDIConnectorList {
  struct MIDIConnector     * connector;
  struct MIDIConnectorList * next;
};

static void _list_destroy( struct MIDIConnectorList ** list, int (*fn)( struct MIDIConnector * ) ) {
  struct MIDIConnectorList * item;
  struct MIDIConnectorList * next;
  item = *list;
  *list = NULL;
  while( item != NULL ) {
    if( fn != NULL ) (*fn)( item->connector );
    MIDIConnectorRelease( item->connector );
    next = item->next;
    free( item );
    item = next;
  }
}

static void _list_push( struct MIDIConnectorList ** list, struct MIDIConnector * connector ) {
  struct MIDIConnectorList * item = malloc( sizeof( struct MIDIConnectorList ) );
  if( list == NULL ) return;
  if( item == NULL ) return;
  item->connector = connector;
  item->next = *list;
  MIDIConnectorRetain( connector );
  *list = item;
}

static void _list_remove( struct MIDIConnectorList ** list, struct MIDIConnector * connector, int (*fn)( struct MIDIConnector * ) ) {
  struct MIDIConnectorList * item;
  struct MIDIConnectorList * release;
  if( list == NULL || *list==NULL ) return;
  while( *list != NULL ) {
    item = *list;
    if( item->connector == connector ) {
      *list = item->next;
      free( item );
      _list_push( &release, connector );
    } else {
      list = &(item->next);
    }
  }
  _list_destroy( &release, fn );
}

static int _receiver_connect( void * driverp, struct MIDIConnector * receiver ) {
  struct MIDIDriver * driver = driverp;
  _list_push( &(driver->receivers), receiver );
  return 0;
}

static int _receiver_disconnect( void * driverp, struct MIDIConnector * receiver ) {
  struct MIDIDriver * driver = driverp;
  _list_remove( &(driver->receivers), receiver, &MIDIConnectorDetachSource );
  return 0;
}

static int _sender_relay( void * driverp, struct MIDIMessage * message ) {
  return MIDIDriverSend( driverp, message );
}

static int _sender_connect( void * driverp, struct MIDIConnector * sender ) {
  struct MIDIDriver * driver = driverp;
  _list_push( &(driver->senders), sender );
  return 0;
}

static int _sender_disconnect( void * driverp, struct MIDIConnector * sender ) {
  struct MIDIDriver * driver = driverp;
  _list_remove( &(driver->senders), sender, &MIDIConnectorDetachTarget );
  return 0;
}

static int _driver_receive( void * driverp, struct MIDIMessage * message ) {
  return MIDIDriverReceive( driverp, message );
}

/** @} */

#pragma mark Creation and destruction
/**
 * @name Creation and destruction
 * Creating, destroying and reference counting of MIDIDriver objects.
 * @{
 */

/**
 * @brief Create a MIDIDriver instance.
 * Allocate space and initialize a MIDIDriver instance.
 * @public @memberof MIDIDriver
 * @param delegate The delegate to use for the driver. May be @c NULL.
 * @return a pointer to the created driver structure on success.
 * @return a @c NULL pointer if the driver could not created.
 */
struct MIDIDriver * MIDIDriverCreate( struct MIDIDriverDelegate * delegate ) {
  struct MIDIDriver * driver = malloc( sizeof( struct MIDIDriver ) );
  driver->refs = 1;
  driver->delegate  = delegate;
  driver->receivers = NULL;
  driver->senders   = NULL;
  driver->clock     = NULL;

  if( delegate != NULL ) {
    delegate->receive   = &_driver_receive;
    delegate->interface = driver;
  }
  return driver;
}

/**
 * @brief Destroy a MIDIDriver instance.
 * Free all resources occupied by the driver and release all referenced objects.
 * @public @memberof MIDIDriver
 * @param driver The driver.
 */
void MIDIDriverDestroy( struct MIDIDriver * driver ) {
  if( driver->clock != NULL ) {
    MIDIClockRelease( driver->clock );
  }
  _list_destroy( &(driver->receivers), &MIDIConnectorDetachSource );
  _list_destroy( &(driver->senders), &MIDIConnectorDetachTarget );
  free( driver );
}

/**
 * @brief Retain a MIDIDriver instance.
 * Increment the reference counter of a driver so that it won't be destroyed.
 * @public @memberof MIDIDriver
 * @param driver The driver.
 */
void MIDIDriverRetain( struct MIDIDriver * driver ) {
  driver->refs++;
}

/**
 * @brief Release a MIDIDriver instance.
 * Decrement the reference counter of a driver. If the reference count
 * reached zero, destroy the driver.
 * @public @memberof MIDIDriver
 * @param driver The driver.
 */
void MIDIDriverRelease( struct MIDIDriver * driver ) {
  if( ! --driver->refs ) {
    MIDIDriverDestroy( driver );
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
 * @brief Delegate for receiving from a driver.
 * @relatesalso MIDIDriver
 * @see         MIDIConnector
 */
struct MIDIConnectorSourceDelegate MIDIDriverReceiveConnectorDelegate = {
  &_receiver_connect,
  &_receiver_disconnect
};

/**
 * @brief Delegate for sending through driver.
 * @relatesalso MIDIDriver
 * @see         MIDIConnector
 */
struct MIDIConnectorTargetDelegate MIDIDriverSendConnectorDelegate = {
  &_sender_relay,
  &_sender_connect,
  &_sender_disconnect
};

/**
 * @brief Provice a connector for sending MIDI data.
 * Provide a connector that can be used to send MIDI messages
 * using the driver.
 * The connector that is stored in @c send will have a retain count
 * of one and should only be released by the user if it was retained
 * before.
 * @public @memberof MIDIDriver
 * @param driver The driver.
 * @param send   The location to store the pointer to the connector in.
 * @retval 0  on success.
 * @retval >0 if the connector could not be provided.
 */
int MIDIDriverProvideSendConnector( struct MIDIDriver * driver, struct MIDIConnector ** send ) {
  struct MIDIConnector * connector;
  if( send == NULL ) return 1;
  connector = MIDIConnectorCreate();
  if( connector == NULL ) return 1;
  MIDIConnectorAttachToDriver( connector, driver );
  _list_push( &(driver->senders), connector );
  *send = connector;
  MIDIConnectorRelease( connector ); /* retained by list */
  return 0;
}

/**
 * @brief Provice a connector for receiving MIDI data.
 * Provide a connector that can be used to receive MIDI messages
 * using the driver.
 * The connector that is stored in @c receive will have a retain count
 * of one and should only be released by the user if it was retained
 * before.
 * @public @memberof MIDIDriver
 * @param driver  The driver.
 * @param receive The location to store the pointer to the connector in.
 * @retval 0  on success.
 * @retval >0 if the connector could not be provided.
 */
int MIDIDriverProvideReceiveConnector( struct MIDIDriver * driver, struct MIDIConnector ** receive ) {
  struct MIDIConnector * connector;
  if( receive == NULL ) return 1;
  connector = MIDIConnectorCreate();
  if( connector == NULL ) return 1;
  MIDIConnectorAttachFromDriver( connector, driver );
  _list_push( &(driver->receivers), connector );
  *receive = connector;
  MIDIConnectorRelease( connector ); /* retained by list */
  return 0;
}

/** @} */

#pragma mark Message passing
/**
 * @name Message passing
 * Receiving and sending MIDIMessage objects.
 * @{
 */

/**
 * @brief Make the MIDIDriver implement itself as loopback.
 * The driver's delegate will be modified so that it passes
 * outgoing messages to it's own receive method.
 * @public @memberof MIDIDriver
 * @param driver The driver
 * @retval 0  on success.
 * @retval >0 if the operation could not be completed.
 */
int MIDIDriverMakeLoopback( struct MIDIDriver * driver ) {
  if( driver->delegate == NULL ) return 1;
  driver->delegate->send = &_driver_receive;
  driver->delegate->implementation = driver;
  return 0;
}

/**
 * @brief Receive a generic MIDIMessage.
 * Relay an incoming message via all attached receiving connectors.
 * This should be called by the driver delegate whenever
 * a new message was received.
 * @public @memberof MIDIDriver
 * @param driver  The driver.
 * @param message The message.
 * @retval 0  on success.
 * @retval >0 if the message could not be relayed.
 */
int MIDIDriverReceive( struct MIDIDriver * driver, struct MIDIMessage * message ) {
  struct MIDIConnectorList * item = driver->receivers;
  int result = 0;
  while( item != NULL ) {
    result += MIDIConnectorRelay( item->connector, message );
    item = item->next;
  }
  return result;
}

/**
 * @brief Send a generic MIDIMessage.
 * Pass an outgoing message to the driver delegate.
 * The delegate should take care of sending.
 * @public @memberof MIDIDriver
 * @param driver  The driver.
 * @param message The message.
 * @retval 0  on success.
 * @retval >0 if the message could not be sent.
 */
int MIDIDriverSend( struct MIDIDriver * driver, struct MIDIMessage * message ) {
  if( driver->delegate == NULL || driver->delegate->send == NULL ) {
    return 0;
  }
  return (*driver->delegate->send)( driver->delegate->implementation, message );
}

/** @} */
