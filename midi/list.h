#ifndef MIDIKIT_MIDI_LIST_H
#define MIDIKIT_MIDI_LIST_H

struct MIDIList;

struct MIDIList * MIDIListCreate( void (*retain)( void * ), void (*release)( void * ) );
void MIDIListDestroy( struct MIDIList * list );
void MIDIListRetain( struct MIDIList * list );
void MIDIListRelease( struct MIDIList * list );
int MIDIListAdd( struct MIDIList * list, void * item );
int MIDIListRemove( struct MIDIList * list, void * item );

#endif
