#ifndef MIDIKIT_MIDI_DRIVER_H
#define MIDIKIT_MIDI_DRIVER_H

struct MIDIMessage;
struct MIDIConnector;

struct MIDIDriver;

#define MIDI_DRIVER_WILL_SEND_MESSAGE 0
#define MIDI_DRIVER_WILL_RECEIVE_MESSAGE 1
#define MIDI_DRIVER_NUM_EVENT_TYPES 2

struct MIDIDriverDelegate {
  int (*send)( void * implementation, struct MIDIMessage * message );
  int (*receive)( void * interface, struct MIDIMessage * message );
  int (*event)( void * observer, void * interface, void * implementation, int event, void * info );
  void * implementation;
  void * interface;
  void * observer;
};

int MIDIDriverDelegateSendMessage( struct MIDIDriverDelegate * delegate, struct MIDIMessage * message );
int MIDIDriverDelegateReceiveMessage( struct MIDIDriverDelegate * delegate, struct MIDIMessage * message );
int MIDIDriverDelegateTriggerEvent( struct MIDIDriverDelegate * delegate, int event, void * info );

#define MIDI_DRIVER_DELEGATE_INITIALIZER { NULL, NULL, NULL, NULL, NULL, NULL }

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
