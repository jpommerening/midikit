#ifndef MIDIKIT_MIDI_UTIL_H
#define MIDIKIT_MIDI_UTIL_H

struct MIDIPort;
struct MIDIDevice;
struct MIDIDriver;

typedef unsigned int MIDIVarLen;
int MIDIUtilReadVarLen( MIDIVarLen * value, size_t size, unsigned char * buffer, size_t * read );
int MIDIUtilWriteVarLen( MIDIVarLen * value, size_t size, unsigned char * buffer, size_t * written );

int MIDIDriverConnectDevice( struct MIDIDriver * driver, struct MIDIDevice * device );

#endif
