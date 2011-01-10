#ifndef MIDIKIT_MIDI_MESSAGE_QUEUE_H
#define MIDIKIT_MIDI_MESSAGE_QUEUE_H
#include "midi.h"
#include "message.h"

struct MIDIMessage;
struct MIDIMessageQueue;

struct MIDIMessageQueue * MIDIMessageQueueCreate();
void MIDIMessageQueueDestroy( struct MIDIMessageQueue * queue );
void MIDIMessageQueueRetain( struct MIDIMessageQueue * queue );
void MIDIMessageQueueRelease( struct MIDIMessageQueue * queue );

int MIDIMessageQueueGetLength( struct MIDIMessageQueue * queue, size_t * length );

int MIDIMessageQueuePush( struct MIDIMessageQueue * queue, struct MIDIMessage * message );
int MIDIMessageQueuePeek( struct MIDIMessageQueue * queue, struct MIDIMessage ** message );
int MIDIMessageQueuePop( struct MIDIMessageQueue * queue, struct MIDIMessage ** message );

#endif
