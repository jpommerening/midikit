#include <stdlib.h>
#include <stdarg.h>
#include <stdio.h>
#include "event.h"
#include "type.h"
#include "midi.h"

/**
 * @ingroup MIDI
 * A basic event consisting of an id, a message and arbitary optional
 * data.
 */
struct MIDIEvent {
/**
 * @privatesection
 * @cond INTERNALS
 */
  int refs;
  size_t id;
  size_t length;
  char * message;
  void * info;
/** @endcond */
};

/**
 * @brief Declare the MIDIEventType type specification.
 */
MIDI_TYPE_SPEC_CODING( MIDIEvent, 0x3010 );

#pragma mark Creation and destruction
/**
 * @name Creation and destruction
 * Creating, destroying and reference counting of MIDIEvent objects.
 * @{
 */

/**
 * @brief Create a MIDIEvent instance.
 * Allocate space and initialize a MIDIEvent instance.
 * @public @memberof MIDIEvent
 * @param id      A (hopefully) unique ID.
 * @param info    Any optional data to be associated with the event.
 * @param message A message format that may contain placeholders.
 * @param ...     The data to be filled into the message format placeholders.
 *                (Refer to the sprintf specification for details.)
 * @return a pointer to the created event structure on success.
 * @return a @c NULL pointer if the event could not created.
 */
struct MIDIEvent * MIDIEventCreate( size_t id, void * info, char * message, ... ) {
  void * buffer;
  size_t length, required;
  va_list vargs;
  struct MIDIEvent * event = malloc( sizeof( struct MIDIEvent ) );
  MIDIPrecondReturn( event != NULL, ENOMEM, NULL );

  event->refs = 1;
  event->id   = id;
  event->info = info;

  if( message != NULL ) {
    length = 64;
    buffer = malloc( length );

    va_start( vargs, message );
    do {
      if( buffer == NULL ) {
        MIDIError( ENOMEM, "Could not allocate space for static buffer." );
        return NULL;
      }
      required = vsnprintf( buffer, length-1, message, vargs );
      if( required > length ) {
        buffer = realloc( buffer, required );
        length = required;
      }
    } while( required > length );
    va_end( vargs );

    event->message = buffer;
    event->length  = length;
  } else {
    event->message = NULL;
    event->length  = 0;
  }

  return event;
}

/**
 * @brief Destroy a MIDIEvent instance.
 * Free all resources occupied by the event and release all referenced objects.
 * @public @memberof MIDIEvent
 * @param event The event.
 */
void MIDIEventDestroy( struct MIDIEvent * event ) {
  MIDIPrecondReturn( event != NULL, EFAULT, (void)0 );
  if( event->message != NULL ) {
    free( event->message );
  }
  free( event );
}

/**
 * @brief Retain a MIDIEvent instance.
 * Increment the reference counter of an event so that it won't be destroyed.
 * @public @memberof MIDIEvent
 * @param event The event.
 */
void MIDIEventRetain( struct MIDIEvent * event ) {
  MIDIPrecondReturn( event != NULL, EFAULT, (void)0 );
  event->refs++;
}

/**
 * @brief Release a MIDIEvent instance.
 * Decrement the reference counter of an event. If the reference count
 * reached zero, destroy the event.
 * @public @memberof MIDIEvent
 * @param event The event.
 */
void MIDIEventRelease( struct MIDIEvent * event ) {
  MIDIPrecondReturn( event != NULL, EFAULT, (void)0 );
  if( ! --event->refs ) {
    MIDIEventDestroy( event );
  }
}

/** @} */

#pragma mark Property access
/**
 * @name Property access
 * Acces properties of MIDIEvents
 * @{
 */

/**
 * @brief Get the event identifier.
 * @public @memberof MIDIEvent
 * @param event The event.
 * @param id    The ID.
 * @retval 0 on success.
 */
int MIDIEventGetId( struct MIDIEvent * event, size_t * id ) {
  MIDIPrecond( event != NULL, EFAULT );
  MIDIPrecond( id != NULL, EINVAL );
  *id = event->id;
  return 0;
}

/**
 * @brief Get the event info-object.
 * @public @memberof MIDIEvent
 * @param event The event.
 * @param info  The info.
 * @retval 0 on success.
 */
int MIDIEventGetInfo( struct MIDIEvent * event, void ** info ) {
  MIDIPrecond( event != NULL, EFAULT );
  MIDIPrecond( info != NULL, EINVAL );
  *info = event->info;
  return 0;
}

/** @} */

#pragma mark Coding
/**
 * @name Coding
 * Encoding and decoding of MIDIEvent objects.
 * @{
 */

/**
 * @brief Encode events.
 * Encode an event to a buffer.
 * @public @memberof MIDIEvent
 * @param event   The event.
 * @param size    The size of the memory pointed to by @c buffer.
 * @param buffer  The buffer to encode the event into.
 * @param written The number of bytes that have been written.
 * @retval 0 on success.
 * @retval 1 if the event could not be encoded.
 */
int MIDIEventEncode( struct MIDIEvent * event, size_t size, void * buffer, size_t * written ) {
  MIDIPrecond( event != NULL, EFAULT );
  MIDIPrecond( buffer != NULL, EINVAL );
  size_t required = 8 + event->length;
  MIDIPrecond( size >= required, ENOMEM );
  /** @todo implement me */
  return 1;
}

/**
 * @brief Decode events.
 * Decode event objects from a buffer.
 * @public @memberof MIDIEvent
 * @param event  The event.
 * @param size   The size of the memory pointed to by @c buffer.
 * @param buffer The buffer to decode the event from.
 * @param read   The number of bytes that were actually read.
 * @retval 0 on success.
 * @retval 1 if the event could not be encoded.
 */
int MIDIEventDecode( struct MIDIEvent * event, size_t size, void * buffer, size_t * read ) {
  MIDIPrecond( event != NULL, EFAULT );
  MIDIPrecond( buffer != NULL, EINVAL );
  size_t required = 5;
  MIDIPrecond( size >= required, EINVAL );
  /** @todo implement me */
  return 1;
}

/** @} */
