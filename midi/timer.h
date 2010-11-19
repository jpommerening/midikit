#ifndef MIDIKIT_MIDI_TIMER_H
#define MIDIKIT_MIDI_TIMER_H
#include "midi.h"
#include "device.h"

struct MIDITimer;
struct MIDITimerDelegate {
};

struct MIDITimer * MIDITimerCreate( struct MIDITimerDelegate * delegate );
void MIDITimerDestroy( struct MIDITimer * timer );
void MIDITimerRetain( struct MIDITimer * timer );
void MIDITimerRelease( struct MIDITimer * timer );

#endif
