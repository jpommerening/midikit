#ifndef MIDIKIT_MIDI_DRIVER_H
#define MIDIKIT_MIDI_DRIVER_H

struct MIDIMessage;
struct MIDIConnector;

struct MIDIDriver;

/**
 * Delegate for bi-directional communication between a MIDIDriver and
 * it's implementation using MIDIMessages.
 */
struct MIDIDriverDelegate {
  /**
   * @brief Callback for sending.
   * This callback is called by the driver when it wants to send a @c MIDIMessage
   * with the implementation. The @c implementation element is passed as first parameter.
   * @param implementation The implementation pointer given to the delegate.
   * @param message        The message that will be sent.
   */
  int (*send)( void * implementation, struct MIDIMessage * message );

  /**
   * @brief Callback for receiving.
   * This callback can be called by the implementation when it wants to notify the
   * driver interface of incoming messages. The @interface element has to be passed as
   * first parameter.
   * @param interface The interface pointer given to the delegate.
   * @param message   The message that was received.
   */
  int (*receive)( void * interface, struct MIDIMessage * message );

  /**
   * @brief Callback for various state changes or events.
   * This is called in various places and it's semantics depend on the
   * event that happened. In general, you should only respond to events
   * you know.
   * @param observer       The observer that handles the events.
   * @param interface      The interface pointer given to the delegate.
   * @param implementation The implementation pointer given to the delegate.
   * @param event          An event number.
   * @param info           Ancillary information specified by the event type.
   */
  int (*event)( void * observer, void * interface, void * implementation, int event, void * info );

  /**
   * @brief The driver implementation.
   * This should point to a valid driver implementation object, for example a
   * MIDIDriverAppleMIDI, MIDIDriverCoreMIDI or MIDIDriverOSC object.
   */
  void * implementation;

  /**
   * @brief The driver interface.
   * This will be set by the MIDIDriver interface to point to the MIDIDriver.
   */
  void * interface;

  /**
   * @brief The observer that manages event and status changes.
   */
  void * observer;
};

/**
 * @brief Initializer for MIDI driver delegates.
 * Assign to a delegate to initialize as empty.
 */
#define MIDI_DRIVER_DELEGATE_INITIALIZER { NULL, NULL, NULL, NULL, NULL, NULL }

/**
 * @brief MIDI driver will send a message.
 * This is called by the driver interface before it will send a message to
 * the implementation. Return a value other than 0 to cancel the sending.
 * The info pointer points the the message that will be sent.
 */
#define MIDI_DRIVER_WILL_SEND_MESSAGE 0

/**
 * @brief MIDI driver will receive a message.
 * This is called by the driver interface after it has been notified
 * (by the implementation) that a new message was received. The interface
 * will then deliver it to all connected devices unless the callback returned
 * a value other than 0. The info pointer points to the message that will
 * be received.
 */
#define MIDI_DRIVER_WILL_RECEIVE_MESSAGE 1

struct MIDIDriver * MIDIDriverCreate( struct MIDIDriverDelegate * delegate );
void MIDIDriverDestroy( struct MIDIDriver * driver );
void MIDIDriverRetain( struct MIDIDriver * driver );
void MIDIDriverRelease( struct MIDIDriver * driver );

int MIDIDriverMakeLoopback( struct MIDIDriver * driver );

int MIDIDriverProvideReceiveConnector( struct MIDIDriver * driver, struct MIDIConnector ** receive );
int MIDIDriverProvideSendConnector( struct MIDIDriver * driver, struct MIDIConnector ** send );

int MIDIDriverTriggerEvent( struct MIDIDriver * driver, int event, void * info );

int MIDIDriverSend( struct MIDIDriver * driver, struct MIDIMessage * message );
int MIDIDriverReceive( struct MIDIDriver * driver, struct MIDIMessage * message );

#endif
