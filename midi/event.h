#ifndef MIDIKIT_EVENT_H
#define MIDIKIT_EVENT_H
#include "type.h"

struct MIDIEvent;
extern struct MIDITypeSpec * MIDIEventType;

struct MIDIEvent * MIDIEventCreate( int id, void * info, char * message, ... );
void MIDIEventDestroy( struct MIDIEvent * event );
void MIDIEventRetain( struct MIDIEvent * event );
void MIDIEventRelease( struct MIDIEvent * event );

int MIDIEventEncode( struct MIDIEvent * event, size_t size, void * buffer, size_t * written );
int MIDIEventDecode( struct MIDIEvent * event, size_t size, void * buffer, size_t * read );

#endif
