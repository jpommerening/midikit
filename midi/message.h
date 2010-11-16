#ifndef MIDIKIT_MIDI_MESSAGE_H
#define MIDIKIT_MIDI_MESSAGE_H
#include "midi.h"
#include "clock.h"

struct MIDIMessage;

struct MIDIMessage * MIDIMessageCreate( MIDIStatus status );
void MIDIMessageDestroy( struct MIDIMessage * message );
void MIDIMessageRetain( struct MIDIMessage * message );
void MIDIMessageRelease( struct MIDIMessage * message );

int MIDIMessageSetStatus( struct MIDIMessage * message, MIDIStatus status );
int MIDIMessageGetStatus( struct MIDIMessage * message, MIDIStatus * status );
int MIDIMessageSetTimestamp( struct MIDIMessage * message, MIDITimestamp timestamp );
int MIDIMessageGetTimestamp( struct MIDIMessage * message, MIDITimestamp * timestamp );
int MIDIMessageGetSize( struct MIDIMessage * message, size_t * size );
int MIDIMessageSet( struct MIDIMessage * message, MIDIProperty property, size_t size, void * value );
int MIDIMessageGet( struct MIDIMessage * message, MIDIProperty property, size_t size, void * value );

int MIDIMessageDecode( struct MIDIMessage * message, size_t bytes, unsigned char * buffer );
int MIDIMessageEncode( struct MIDIMessage * message, size_t bytes, unsigned char * buffer );

#endif
