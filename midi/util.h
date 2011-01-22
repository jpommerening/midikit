#ifndef MIDIKIT_MIDI_UTIL_H
#define MIDIKIT_MIDI_UTIL_H

typedef unsigned int MIDIVarLen;
int MIDIUtilReadVarLen( MIDIVarLen * value, size_t size, unsigned char * buffer, size_t * read );
int MIDIUtilWriteVarLen( MIDIVarLen * value, size_t size, unsigned char * buffer, size_t * written );

#endif
