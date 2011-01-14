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
   */
  int (*send)( void * implementation, struct MIDIMessage * message );

  /**
   * @brief Callback for receiving.
   * This callback can be called by the implementation when it wants to notify the
   * driver interface of incoming messages. The @interface element has to be passed as
   * first parameter.
   */
  int (*receive)( void * interface, struct MIDIMessage * message );

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
};

struct MIDIDriver * MIDIDriverCreate( struct MIDIDriverDelegate * delegate );
void MIDIDriverDestroy( struct MIDIDriver * driver );
void MIDIDriverRetain( struct MIDIDriver * driver );
void MIDIDriverRelease( struct MIDIDriver * driver );

int MIDIDriverMakeLoopback( struct MIDIDriver * driver );

int MIDIDriverProvideReceiveConnector( struct MIDIDriver * driver, struct MIDIConnector ** receive );
int MIDIDriverProvideSendConnector( struct MIDIDriver * driver, struct MIDIConnector ** send );

int MIDIDriverSend( struct MIDIDriver * driver, struct MIDIMessage * message );
int MIDIDriverReceive( struct MIDIDriver * driver, struct MIDIMessage * message );

#endif
