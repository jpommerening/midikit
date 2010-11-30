#ifndef MIDIKIT_MIDI_UTIL_H
#define MIDIKIT_MIDI_UTIL_H

typedef unsigned int MIDIVarLen;
int MIDIUtilReadVarLen( unsigned char * buffer, size_t bytes, MIDIVarLen * value, size_t * read );
int MIDIUtilWriteVarLen( unsigned char * buffer, size_t bytes, MIDIVarLen * value, size_t * written );

#endif
