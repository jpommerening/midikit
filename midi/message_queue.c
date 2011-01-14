#include <stdlib.h>
#include "message_queue.h"

/**
 * Queue for MIDI message objects.
 */
struct MIDIMessageQueue {
  size_t refs;
  size_t length;
  struct MIDIMessageList * first;
  struct MIDIMessageList * last;
};

#pragma mark Creation and destruction
/**
 * @name Creation and destruction
 * Creating, destroying and reference counting of MIDIMessageQueue objects.
 * @{
 */

/**
 * @brief Create a MIDIMessageQueue instance.
 * Allocate space and initialize a MIDIMessageQueue instance.
 * @public @memberof MIDIMessageQueue
 * @return a pointer to the created device structure on success.
 * @return a @c NULL pointer if the device could not created.
 */
struct MIDIMessageQueue * MIDIMessageQueueCreate() {
  struct MIDIMessageQueue * queue = malloc( sizeof( struct MIDIMessageQueue ) );
  if( queue == NULL ) {
    return NULL;
  }
  queue->refs   = 1;
  queue->length = 0;
  queue->first  = NULL;
  queue->last   = NULL;
  return queue;
};

/**
 * @brief Destroy a MIDIMessageQueue instance.
 * Free all resources occupied by the queue and release all referenced messages.
 * @public @memberof MIDIMessageQueue
 * @param queue The message queue.
 */
void MIDIMessageQueueDestroy( struct MIDIMessageQueue * queue ) {
  struct MIDIMessageList * item = queue->first;
  struct MIDIMessageList * next;
  queue->first = NULL;
  while( item != NULL ) {
    MIDIMessageRelease( item->message );
    next = item->next;
    free( item );
    item = next;
  }
  free( queue );
}

/**
 * @brief Retain a MIDIMessageQueue instance.
 * Increment the reference counter of a message queue so that
 * it won't be destroyed.
 * @public @memberof MIDIMessageQueue
 * @param queue The message queue.
 */
void MIDIMessageQueueRetain( struct MIDIMessageQueue * queue ) {
  queue->refs++;
}

/**
 * @brief Release a MIDIMessageQueue instance.
 * Decrement the reference counter of a message queue. If the
 * reference count reached zero, destroy the message queue.
 * @public @memberof MIDIMessageQueue
 * @param queue The message queue.
 */
void MIDIMessageQueueRelease( struct MIDIMessageQueue * queue ) {
  if( ! --queue->refs ) {
    MIDIMessageQueueDestroy( queue );
  }
}

/** @} */

#pragma mark Queueing operations
/**
 * @name Queueing operations
 * Basic queue operations to store and retrieve messages.
 * @{
 */

/**
 * Get the length of a message queue.
 * @public @memberof MIDIMessageQueue
 * @param queue  The message queue.
 * @param length The length.
 * @retval 0 on success.
 * @retval >0 if the length could not be determined.
 */
int MIDIMessageQueueGetLength( struct MIDIMessageQueue * queue, size_t * length ) {
  if( length == NULL ) return 1;
  *length = queue->length;
  return 0;
}

/**
 * Add a message to the end queue.
 * @public @memberof MIDIMessageQueue
 * @param queue The message queue.
 * @param message The message.
 * @retval 0 on success.
 * @retval >0 if the item could not be added.
 */
int MIDIMessageQueuePush( struct MIDIMessageQueue * queue, struct MIDIMessage * message ) {
  struct MIDIMessageList * item;
  if( message == NULL ) return 1;
  item = malloc( sizeof( struct MIDIMessageList ) );
  if( item == NULL ) return 2;
  
  MIDIMessageRetain( message );
  item->message = message;
  item->next = NULL;
  if( queue->last == NULL ) {
    queue->first = item;
    queue->last  = item;
  } else {
    queue->last->next = item;
    queue->last = item;
  }
  queue->length++;
  return 0;
}

/**
 * Get the message at the beginning of the queue but do not remove it.
 * @public @memberof MIDIMessageQueue
 * @param queue The message queue.
 * @param message The message.
 * @retval 0 on success.
 * @retval >0 if the item could not be fetched.
 */
int MIDIMessageQueuePeek( struct MIDIMessageQueue * queue, struct MIDIMessage ** message ) {
  if( message == NULL ) return 1;
  if( queue->first != NULL ) {
    *message = queue->first->message;
  } else {
    *message = NULL;
  }
  return 0;
}

/**
 * Remove the first message in the queue and store it.
 * @public @memberof MIDIMessageQueue
 * @param queue The queue.
 * @param message The message.
 * @retval 0 on success.
 * @retval >0 if the item could not be fetched or removed.
 */
int MIDIMessageQueuePop( struct MIDIMessageQueue * queue, struct MIDIMessage ** message ) {
  struct MIDIMessageList * item;
  if( message == NULL ) return 1;
  item = queue->first;
  if( item != NULL ) {
    *message     = item->message;
    queue->first = item->next;
    if( queue->last == item ) {
      queue->last = NULL;
    }
    queue->length--;
    free( item );
  } else {
    *message = NULL;
  }
  return 0;
}

/** @} */
