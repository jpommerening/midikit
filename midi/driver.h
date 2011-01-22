#ifndef MIDIKIT_MIDI_DRIVER_H
#define MIDIKIT_MIDI_DRIVER_H

struct MIDIMessage;
struct MIDIConnector;

struct MIDIDriver;

struct MIDIDriverDelegate {
  int (*send)( void * implementation, struct MIDIMessage * message );
  int (*receive)( void * interface, struct MIDIMessage * message );
  int (*event)( void * observer, void * interface, void * implementation, int event, void * info );
  void * implementation;
  void * interface;
  void * observer;
};

/**
 * @def MIDI_DRIVER_DELEGATE_INITIALIZER
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
