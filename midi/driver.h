#ifndef MIDIKIT_MIDI_DRIVER_H
#define MIDIKIT_MIDI_DRIVER_H

struct MIDIMessage;
struct MIDIConnector;

struct MIDIDriver;

struct MIDIDriverDelegate {
  int (*send)( void * implementation, struct MIDIMessage * message );
  int (*receive)( void * interface, struct MIDIMessage * message );
  void * implementation;
  void * interface;
};

extern struct MIDIDriverDelegate * midiDriverLoopback;

struct MIDIDriver * MIDIDriverCreate( struct MIDIDriverDelegate * delegate );
void MIDIDriverDestroy( struct MIDIDriver * driver );
void MIDIDriverRetain( struct MIDIDriver * driver );
void MIDIDriverRelease( struct MIDIDriver * driver );

int MIDIDriverProvideReceiveConnector( struct MIDIDriver * driver, struct MIDIConnector ** receive );
int MIDIDriverProvideSendConnector( struct MIDIDriver * driver, struct MIDIConnector ** send );

int MIDIDriverSend( struct MIDIDriver * driver, struct MIDIMessage * message );
int MIDIDriverReceive( struct MIDIDriver * driver, struct MIDIMessage * message );
#endif
