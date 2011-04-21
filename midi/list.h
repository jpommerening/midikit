#ifndef MIDIKIT_MIDI_LIST_H
#define MIDIKIT_MIDI_LIST_H
#include "type.h"

struct MIDIList;

struct MIDIList * MIDIListCreate( struct MIDITypeSpec * type );
void MIDIListDestroy( struct MIDIList * list );
void MIDIListRetain( struct MIDIList * list );
void MIDIListRelease( struct MIDIList * list );
int MIDIListAdd( struct MIDIList * list, void * item );
int MIDIListRemove( struct MIDIList * list, void * item );
int MIDIListApply( struct MIDIList * list, void * info, int (*func)( void * item, void * info ) );

#endif
