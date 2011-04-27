#ifndef MIDIKIT_DRIVER_COREMIDI_H
#define MIDIKIT_DRIVER_COREMIDI_H
#ifdef __APPLE__
#include <CoreMIDI/MIDIServices.h>


#ifndef MIDI_DRIVER_INTERNALS
/**
 * When used as an opaque pointer type, an instance of
 * @c MIDIDriverCoreMIDI can be used as a @c MIDIDriver.
 */
#define MIDIDriverCoreMIDI MIDIDriver
#endif

struct MIDIMessage;
struct MIDIDriverCoreMIDI;

struct MIDIDriverCoreMIDI * MIDIDriverCoreMIDICreate( char * name, MIDIClientRef client );

int MIDIDriverCoreMIDIReceiveMessage( struct MIDIDriverCoreMIDI * driver, struct MIDIMessage * message );
int MIDIDriverCoreMIDISendMessage( struct MIDIDriverCoreMIDI * driver, struct MIDIMessage * message );

int MIDIDriverCoreMIDIReceive( struct MIDIDriverCoreMIDI * driver );
int MIDIDriverCoreMIDISend( struct MIDIDriverCoreMIDI * driver );
int MIDIDriverCoreMIDIIdle( struct MIDIDriverCoreMIDI * driver );

#endif
#endif
