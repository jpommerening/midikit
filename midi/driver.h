#ifndef MIDIKIT_MIDI_DRIVER_H
#define MIDIKIT_MIDI_DRIVER_H

struct MIDIMessage;
struct MIDIInput;
struct MIDIOutput;
struct MIDIDevice;

struct MIDIDriver;
struct MIDIDriverContext {
  int (*send)( struct MIDIDriver * driver, struct MIDIMessage * event );
};

extern struct MIDIDriverContext * midiDriverLoopback;

struct MIDIDriver * MIDIDriverCreate();
void MIDIDriverDestroy( struct MIDIDriver * driver );
void MIDIDriverRetain( struct MIDIDriver * driver );
void MIDIDriverRelease( struct MIDIDriver * driver );

int MIDIDriverAddSender( struct MIDIDriver * driver, struct MIDIOutput * sender );
int MIDIDriverRemoveSender( struct MIDIDriver * driver, struct MIDIOutput * sender );

int MIDIDriverAddReceiver( struct MIDIDriver * driver, struct MIDIInput * receiver );
int MIDIDriverRemoveReceiver( struct MIDIDriver * driver, struct MIDIInput * receiver );

int MIDIDriverSend( struct MIDIDriver * driver, struct MIDIMessage * message );
int MIDIDriverReceive( struct MIDIDriver * driver, struct MIDIMessage * message );
#endif
