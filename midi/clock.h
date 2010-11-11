#ifndef MIDIKIT_MIDI_CLOCK_H
#define MIDIKIT_MIDI_CLOCK_H

#define MIDI_SAMPLING_RATE_8KHZ      8000.0
#define MIDI_SAMPLING_RATE_11KHZ    11025.0
#define MIDI_SAMPLING_RATE_44K1HZ   44100.0
#define MIDI_SAMPLING_RATE_48KHZ    48000.0
#define MIDI_SAMPLING_RATE_88K2HZ   88200.0
#define MIDI_SAMPLING_RATE_96KHZ    96000.0
#define MIDI_SAMPLING_RATE_176K4HZ 176400.0
#define MIDI_SAMPLING_RATE_192KHZ  192000.0
#define MIDI_SAMPLING_RATE_DEFAULT MIDI_SAMPLING_RATE_44K1HZ

typedef double MIDISamplingRate;

typedef unsigned int MIDIClock;

int MIDIClockSetNow( MIDIClock now );
int MIDIClockGetNow( MIDIClock * now );

int MIDIClockSetSamplingRate( MIDISamplingRate rate );
int MIDIClockGetSamplingRate( MIDISamplingRate * rate );

int MIDIClockToSeconds( MIDIClock clock, double * seconds );
int MIDIClockFromSeconds( MIDIClock * clock, double seconds );

#endif
