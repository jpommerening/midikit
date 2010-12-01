#include <stdlib.h>
#include "message_queue.h"
#include "message.h"

#define DEFAULT_QUEUE_SIZE 16

/**
 * Queue for MIDI message objects.
 */
struct MIDIMessageQueue {
  size_t refs;
  size_t size;
  size_t first;
  size_t avail;
  struct MIDIMessage ** messages;
};

#pragma mark Creation and destruction
/**
 * @name Creation and destruction
 * Creating, destroying and reference counting of MIDIMessageQueue objects.
 * @{
 */

struct MIDIMessageQueue * MIDIMessageQueueCreate( size_t size ) {
  struct MIDIMessageQueue * queue = malloc( sizeof( struct MIDIMessageQueue ) );
  if( queue == NULL ) {
    return NULL;
  }
  if( size == 0 ) {
    size = DEFAULT_QUEUE_SIZE;
  }
  struct MIDIMessage ** messages = malloc( sizeof( struct MIDIMessage * ) );
  if( messages == NULL ) {
    free( queue );
    return NULL;
  }
  queue->refs  = 1;
  queue->size  = size;
  queue->first = 0;
  queue->avail = 0;
  queue->messages = messages;
  return queue;
};

void MIDIMessageQueueDestroy( struct MIDIMessageQueue * queue ) {
  size_t p;
  for( p=0; p<queue->avail; p++ ) {
    MIDIMessageRelease( queue->messages[ (p+queue->first)%queue->size ] );
  }
  free( queue );
}

void MIDIMessageQueueRetain( struct MIDIMessageQueue * queue ) {
  queue->refs++;
}

void MIDIMessageQueueRelease( struct MIDIMessageQueue * queue ) {
  if( ! --queue->refs ) {
    MIDIMessageQueueDestroy( queue );
  }
}

/** @} */

