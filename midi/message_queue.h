#ifndef MIDIKIT_MIDI_MESSAGE_QUEUE_H
#define MIDIKIT_MIDI_MESSAGE_QUEUE_H
#include "midi.h"

struct MIDIMessage;
struct MIDIMessageQueue;

struct MIDIMessageQueue * MIDIMessageQueueCreate( size_t size );
void MIDIMessageQueueDestroy( struct MIDIMessageQueue * queue );
void MIDIMessageQueueRetain( struct MIDIMessageQueue * queue );
void MIDIMessageQueueRelease( struct MIDIMessageQueue * queue );

int MIDIMessageQueueGetSize( struct MIDIMessageQueue * queue, size_t * size );
int MIDIMessageQueueGetAvail( struct MIDIMessageQueue * queue, size_t * size );

int MIDIMessageQueuePush( struct MIDIMessageQueue * queue, struct MIDIMessage * message );
int MIDIMessageQueuePop( struct MIDIMessageQueue * queue, struct MIDIMessage * message );

#endif
