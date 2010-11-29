#ifndef MIDIKIT_MIDI_UTIL_H
#define MIDIKIT_MIDI_UTIL_H

typedef unsigned int MIDIVarLen;
int MIDIUtilReadVarLen( void * buffer, size_t bytes, MIDIVarLen * value, size_t * read );
int MIDIUtilWriteVarLen( void * buffer, size_t bytes, MIDIVarLen * value, size_t * written );

#endif
