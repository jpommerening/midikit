#ifndef MIDIKIT_MIDI_CLOCK_H
#define MIDIKIT_MIDI_CLOCK_H
#include "midi.h"

#define MIDI_SAMPLING_RATE_8KHZ      8000
#define MIDI_SAMPLING_RATE_11KHZ    11025
#define MIDI_SAMPLING_RATE_44K1HZ   44100
#define MIDI_SAMPLING_RATE_48KHZ    48000
#define MIDI_SAMPLING_RATE_88K2HZ   88200
#define MIDI_SAMPLING_RATE_96KHZ    96000
#define MIDI_SAMPLING_RATE_176K4HZ 176400
#define MIDI_SAMPLING_RATE_192KHZ  192000
#define MIDI_SAMPLING_RATE_DEFAULT      0

struct MIDIClock;

int MIDIClockSetGlobalClock( struct MIDIClock * clock );
int MIDIClockGetGlobalClock( struct MIDIClock ** clock );

struct MIDIClock * MIDIClockCreate( MIDISamplingRate rate );
void MIDIClockDestroy( struct MIDIClock * clock );
void MIDIClockRetain( struct MIDIClock * clock );
void MIDIClockRelease( struct MIDIClock * clock );

int MIDIClockSetNow( struct MIDIClock * clock, MIDITimestamp now );
int MIDIClockGetNow( struct MIDIClock * clock, MIDITimestamp * now );

int MIDIClockSetSamplingRate( struct MIDIClock * clock, MIDISamplingRate rate );
int MIDIClockGetSamplingRate( struct MIDIClock * clock, MIDISamplingRate * rate );

int MIDIClockTimestampToSeconds( struct MIDIClock * clock, MIDITimestamp timestamp, double * seconds );
int MIDIClockTimestampFromSeconds( struct MIDIClock * clock, MIDITimestamp * timestamp, double seconds );

int MIDIClockConvertTimestamp( struct MIDIClock * clock, struct MIDIClock * source, MIDITimestamp * timestamp );

#endif
