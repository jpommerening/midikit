#ifndef MIDIKIT_MIDI_DRIVER_H
#define MIDIKIT_MIDI_DRIVER_H

struct MIDIMessage;
struct MIDIConnector;

struct MIDIDriver;
struct MIDIDriverDelegate {
  int (*send)( struct MIDIDriver * driver, struct MIDIMessage * event );
};

extern struct MIDIDriverDelegate * midiDriverLoopback;

struct MIDIDriver * MIDIDriverCreate();
void MIDIDriverDestroy( struct MIDIDriver * driver );
void MIDIDriverRetain( struct MIDIDriver * driver );
void MIDIDriverRelease( struct MIDIDriver * driver );

int MIDIDriverProvideInput( struct MIDIDriver * driver, struct MIDIConnector ** input );
int MIDIDriverProvideOutput( struct MIDIDriver * driver, struct MIDIConnector ** output );

int MIDIDriverSend( struct MIDIDriver * driver, struct MIDIMessage * message );
int MIDIDriverReceive( struct MIDIDriver * driver, struct MIDIMessage * message );
#endif
