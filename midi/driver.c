#include <stdlib.h>
#define MIDI_DRIVER_INTERNALS
#include "driver.h"

#include "list.h"
#include "port.h"
#include "event.h"
#include "message.h"

#include "runloop.h"
#include "clock.h"

/**
 * @defgroup MIDI-driver MIDI driver implementations
 * @ingroup MIDI
 * Implementations of the MIDIDriver interface.
 */

/**
 * @ingroup MIDI
 * @def MIDI_DRIVER_WILL_SEND_MESSAGE
 * @brief MIDI driver will send a message.
 * This is called by the driver interface before it will send a message to
 * the implementation. Return a value other than 0 to cancel the sending.
 * The info pointer points the the message that will be sent.
 */
/**
 * @ingroup MIDI
 * @def MIDI_DRIVER_WILL_RECEIVE_MESSAGE
 * @brief MIDI driver will receive a message.
 * This is called by the driver interface after it has been notified
 * (by the implementation) that a new message was received. The interface
 * will then deliver it to all connected devices unless the callback returned
 * a value other than 0. The info pointer points to the message that will
 * be received.
 */

/**
 * @ingroup MIDI
 * @struct MIDIDriver driver.h
 * @brief Abstract class to send MIDI messages with various drivers.
 * The MIDIDriver is an abstract class / interface that can be used
 * to pass messages to an underlying implementation.
 * Extend this class by using it as the first member in your
 * driver implementation structure.
 * The C-standard asserts that there is no unnamed padding at
 * the beginning of a struct. Extended structs can be used whereever
 * a MIDIDriver struct is expected.
 */

/**
 * @brief Port callback.
 * This may be confusing at first but when the driver <b>receives</b>
 * a message on it's port it has to <b>send</b> the message using it's
 * implementation.
 * This is because the port <b>receives</b> messages from (virtual)
 * MIDIDevices when these devices <b>send</b> a message through
 * their own ports.
 * @private @memberof MIDIDriver
 * @param target  The target of the port callback, the driver itself.
 * @param source  The source that sended the message.
 * @param type    The type of the message that was received.
 * @param object  The actual message object that was received.
 * @retval 0 on success.
 */
static int _port_receive( void * target, void * source, struct MIDITypeSpec * type, void * object ) {
  struct MIDIDriver * driver = target;

  if( type == MIDIMessageType && driver->send != NULL ) {
    return (*driver->send)( driver, object );
  } else {
    return 0;
  }
}

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
 * @param name The name to identify the driver.
 * @param rate The sampling rate to use.
 * @return a pointer to the created driver structure on success.
 * @return a @c NULL pointer if the driver could not created.
 */
struct MIDIDriver * MIDIDriverCreate( char * name, MIDISamplingRate rate ) {
  struct MIDIDriver * driver = malloc( sizeof( struct MIDIDriver ) );
  MIDIPrecondReturn( driver != NULL, ENOMEM, NULL );
  MIDIDriverInit( driver, name, rate );
  return driver;
}

/**
 * @brief Initialize a MIDIDriver instance.
 * @public @memberof MIDIDriver
 * @param name The name to identify the driver.
 * @param rate The sampling rate to use.
 */
void MIDIDriverInit( struct MIDIDriver * driver, char * name, MIDISamplingRate rate ) {
  MIDIPrecondReturn( driver != NULL, EFAULT, (void)0 );
  MIDISamplingRate global_rate;

  driver->refs  = 1;
  driver->rls   = NULL;
  driver->port  = MIDIPortCreate( name, MIDI_PORT_IN | MIDI_PORT_OUT, driver, &_port_receive );
  driver->clock = NULL;

  MIDIClockGetGlobalClock( &(driver->clock) );
  MIDIClockGetSamplingRate( driver->clock, &global_rate );

  if( global_rate == rate ) {
    MIDIClockRetain( driver->clock );
  } else {
    driver->clock = MIDIClockCreate( rate );
  }

  driver->send    = NULL;
  driver->destroy = NULL;
}

/**
 * @brief Destroy a MIDIDriver instance.
 * Free all resources occupied by the driver and release all referenced objects.
 * @public @memberof MIDIDriver
 * @param driver The driver.
 */
void MIDIDriverDestroy( struct MIDIDriver * driver ) {
  MIDIPrecondReturn( driver != NULL, EFAULT, (void)0 );
  if( driver->destroy != NULL ) {
    (*driver->destroy)( driver );
  }
  if( driver->clock != NULL ) {
    MIDIClockRelease( driver->clock );
  }
  if( driver->rls != NULL ) {
    MIDIRunloopSourceInvalidate( driver->rls );
    MIDIRunloopSourceRelease( driver->rls );
  }
  MIDIPortInvalidate( driver->port );
  MIDIPortRelease( driver->port );
  free( driver );
}

/**
 * @brief Retain a MIDIDriver instance.
 * Increment the reference counter of a driver so that it won't be destroyed.
 * @public @memberof MIDIDriver
 * @param driver The driver.
 */
void MIDIDriverRetain( struct MIDIDriver * driver ) {
  MIDIPrecondReturn( driver != NULL, EFAULT, (void)0 );
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
  MIDIPrecondReturn( driver != NULL, EFAULT, (void)0 );
  if( ! --driver->refs ) {
    MIDIDriverDestroy( driver );
  }
}

/** @} */

#pragma mark Port access
/**
 * @name Port access
 * @{
 */

/**
 * @brief Get the driver port.
 * Provide a port that can be used to send and receive MIDI messages
 * using the driver.
 * The port that is stored in @c port will have a retain count
 * of one and should only be released by the user if it was retained
 * before.
 * @public @memberof MIDIDriver
 * @param driver The driver.
 * @param port   The port.
 * @retval 0 on success.
 */
int MIDIDriverGetPort( struct MIDIDriver * driver, struct MIDIPort ** port ) {
  MIDIPrecond( driver != NULL, EFAULT );
  MIDIPrecond( port != NULL, EINVAL );
  *port = driver->port;
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
 * The driver's callback will be modified so that it passes
 * outgoing messages to it's own receive method.
 * @public @memberof MIDIDriver
 * @param driver The driver
 * @retval 0  on success.
 * @retval >0 if the operation could not be completed.
 */
int MIDIDriverMakeLoopback( struct MIDIDriver * driver ) {
  MIDIPrecond( driver != NULL, EFAULT );
  driver->send = &MIDIDriverReceive;
  return 0;
}

/**
 * @brief Receive a generic MIDIMessage.
 * Relay an incoming message via all attached receiving ports.
 * This should be called by the driver implementation whenever
 * a new message was received.
 * @public @memberof MIDIDriver
 * @param driver  The driver.
 * @param message The message.
 * @retval 0  on success.
 * @retval >0 if the message could not be relayed.
 */
int MIDIDriverReceive( struct MIDIDriver * driver, struct MIDIMessage * message ) {
  MIDIPrecond( driver != NULL, EFAULT );
  MIDIPrecond( message != NULL, EINVAL );
  return MIDIPortSend( driver->port, MIDIMessageType, message );
}

/**
 * @brief Send a generic MIDIMessage.
 * Pass an outgoing message (through the port) to the implementation.
 * The implementation's @c send callback is responsible for sending the
 * message.
 * @public @memberof MIDIDriver
 * @param driver  The driver.
 * @param message The message.
 * @retval 0  on success.
 * @retval >0 if the message could not be sent.
 */
int MIDIDriverSend( struct MIDIDriver * driver, struct MIDIMessage * message ) {
  MIDIPrecond( driver != NULL, EFAULT );
  MIDIPrecond( message != NULL, EINVAL );
  return MIDIPortReceive( driver->port, MIDIMessageType, message );
}

/**
 * @brief Trigger an event that occured in the driver implementation.
 * @public @memberof MIDIDriver
 * @param driver The driver.
 * @param event  The event.
 * @retval 0  on success.
 */
int MIDIDriverTriggerEvent( struct MIDIDriver * driver, struct MIDIEvent * event ) {
  MIDIPrecond( driver != NULL, EFAULT );
  MIDIPrecond( event != NULL, EINVAL );
  return MIDIPortSend( driver->port, MIDIEventType, event );
}

/** @} */
