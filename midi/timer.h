#ifndef MIDIKIT_MIDI_TIMER_H
#define MIDIKIT_MIDI_TIMER_H
#include "midi.h"
#include "device.h"

#define MIDI_CLOCKS_PER_BEAT          6
#define MIDI_CLOCKS_PER_QUARTER_NOTE 24
#define MIDI_BEATS_PER_QUARTER_NOTE   4

struct MIDITimer;
struct MIDITimerDelegate {
};

struct MIDITimer * MIDITimerCreate( struct MIDITimerDelegate * delegate );
void MIDITimerDestroy( struct MIDITimer * timer );
void MIDITimerRetain( struct MIDITimer * timer );
void MIDITimerRelease( struct MIDITimer * timer );

int MIDITimerReceiveRealTime( struct MIDITimer * timer, struct MIDIDevice * device,
                              MIDIStatus status, MIDITimestamp timestamp );
int MIDITimerSendRealTime( struct MIDITimer * timer, struct MIDIDevice * device,
                           MIDIStatus status, MIDITimestamp timestamp );

#endif
