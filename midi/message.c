#include <stdlib.h>
#include "message.h"
#include "message_format.h"

/**
 * @struct MIDIMessage message.h
 * @brief Structure of MIDI message object.
 */
struct MIDIMessage {
  size_t refs;
  struct MIDIMessageFormat * format;
  struct MIDIMessageData data;
  MIDITimestamp timestamp;
};

#pragma mark Creation and destruction
/**
 * @name Creation and destruction
 * Creating, destroying and reference counting of MIDIMessage objects.
 * @{
 */

/**
 * @brief Create a MIDIMessage instance.
 * Allocate space and initialize a MIDIMessage instance.
 * @public @memberof MIDIMessage
 * @param status The message status to be used for initialization.
 * @return a pointer to the created message structure on success.
 * @return a @c NULL pointer if the message could not created.
 */
struct MIDIMessage * MIDIMessageCreate( MIDIStatus status ) {
  struct MIDIMessage * message;
  struct MIDIMessageFormat * format = NULL;
  MIDITimestamp timestamp = 0;
  int i;

  if( status != 0 ) {
    format = MIDIMessageFormatForStatus( status );
    if( format == NULL ) {
      return NULL;
    }
  }
  message = malloc( sizeof( struct MIDIMessage ) );
  if( message == NULL ) {
    return NULL;
  }
  message->refs   = 1;
  message->format = format;
  for( i=1; i<MIDI_MESSAGE_DATA_BYTES; i++ ) {
    message->data.bytes[i] = 0;
  }
  message->data.size = 0;
  message->data.data = NULL;
  if( status != 0 ) {
    MIDIMessageSetStatus( message, status );
  }
  MIDIMessageSetTimestamp( message, timestamp );
  return message;
}

/**
 * @brief Destroy a MIDIMessage instance.
 * Free all resources occupied by the message.
 * @public @memberof MIDIMessage
 * @param message The message.
 */
void MIDIMessageDestroy( struct MIDIMessage * message ) {
  if( message->data.data != NULL && ( message->data.bytes[3] & 1 ) ) free( message->data.data );
  free( message );
}

/**
 * @brief Retain a MIDIMessage instance.
 * Increment the reference counter of a message so that it won't be destroyed.
 * @public @memberof MIDIMessage
 * @param message The message.
 */
void MIDIMessageRetain( struct MIDIMessage * message ) {
  message->refs++;
}

/**
 * @brief Release a MIDIMessage instance.
 * Decrement the reference counter of a message. If the reference count
 * reached zero, destroy the message.
 * @public @memberof MIDIMessage
 * @param message The message.
 */
void MIDIMessageRelease( struct MIDIMessage * message ) {
  if( ! --message->refs ) {
    MIDIMessageDestroy( message );
  }
}

/** @} */

#pragma mark Property access
/**
 * @name Property access
 * Get and set message properties.
 * @{
 */

/**
 * @public @memberof MIDIMessage
 * @param message The message.
 * @param status The MIDIStatus.
 */
int MIDIMessageSetStatus( struct MIDIMessage * message, MIDIStatus status ) {
  struct MIDIMessageFormat * format = MIDIMessageFormatForStatus( status );
  if( format == NULL ) return 1;
  message->format = format;
  return MIDIMessageSet( message, MIDI_STATUS, sizeof( MIDIStatus ), &status );
}

/**
 * @public @memberof MIDIMessage
 * @param message The message.
 * @param status The MIDIStatus.
 */
int MIDIMessageGetStatus( struct MIDIMessage * message, MIDIStatus * status ) {
  return MIDIMessageGet( message, MIDI_STATUS, sizeof( MIDIStatus ), status );
}

/**
 * @public @memberof MIDIMessage
 * @param message The message.
 * @param timestamp The MIDITimestamp.
 */
int MIDIMessageSetTimestamp( struct MIDIMessage * message, MIDITimestamp timestamp ) {
  if( message == NULL ) return 1;
  message->timestamp = timestamp;
  return 0;
}

/**
 * @public @memberof MIDIMessage
 * @param message The message.
 * @param timestamp The MIDITimestamp.
 */
int MIDIMessageGetTimestamp( struct MIDIMessage * message, MIDITimestamp * timestamp ) {
  if( message == NULL || timestamp == NULL ) return 1;
  *timestamp = message->timestamp;
  return 0;
}

/**
 * @public @memberof MIDIMessage
 * @param message The message.
 * @param size The size.
 */
int MIDIMessageGetSize( struct MIDIMessage * message, size_t * size ) {
  if( message == NULL || size == NULL ) return 1;
  return MIDIMessageFormatGetSize( message->format, &(message->data), size );
}

/**
 * @brief Set properties.
 * Set properties of the message.
 * @public @memberof MIDIMessage
 * @param message  The message.
 * @param property The property to set.
 * @param size     The size of the memory object pointed to by @c value.
 * @param value    A pointer to the memory object who's contents shall be
 *                 copied to the message property.
 * @retval 0 on success.
 * @retval 1 if the property was not set.
 */
int MIDIMessageSet( struct MIDIMessage * message, MIDIProperty property, size_t size, void * value ) {
  return MIDIMessageFormatSet( message->format, &(message->data), property, size, value );
}

/**
 * @brief Get properties.
 * Get properties of the message.
 * @public @memberof MIDIMessage
 * @param message  The message.
 * @param property The property to get.
 * @param size     The size of the memory object pointed to by @c value.
 * @param value    A pointer to the memory object who's contents shall be
 *                 copied from the message property.
 * @retval 0 on success.
 * @retval 1 if the value was not set.
 */
int MIDIMessageGet( struct MIDIMessage * message, MIDIProperty property, size_t size, void * value ) {
  return MIDIMessageFormatGet( message->format, &(message->data), property, size, value );
}

/**
 * @brief Release auxiliary data.
 * Check if the message has auxiliary data (variable length sysex data) and release it if
 * necessary.
 * @private @memberof MIDIMessage
 * @param message The message.
 */
static void _check_release_data( struct MIDIMessage * message ) {
  if( message->data.data != NULL && ( message->data.bytes[3] & 1 ) ) {
    free( message->data.data );
    message->data.data = NULL;
  }
}


/**
 * @brief Encode messages.
 * Encode message objects into a buffer.
 * @public @memberof MIDIMessage
 * @param message  The message.
 * @param size     The size of the memory pointed to by @c buffer.
 * @param buffer   The buffer to encode the message into.
 * @retval 0 on success.
 * @retval 1 if the message could not be encoded.
 */
int MIDIMessageEncode( struct MIDIMessage * message, size_t size, unsigned char * buffer, size_t * written ) {
  if( size == 0 || buffer == NULL ) return 1;
  return MIDIMessageFormatEncode( message->format, &(message->data), size, buffer, written );
}

/**
 * @brief Decode messages.
 * Decode message objects from a buffer.
 * @public @memberof MIDIMessageFormat
 * @param message  The message.
 * @param size     The size of the memory pointed to by @c buffer.
 * @param buffer   The buffer to decode the message from.
 * @retval 0 on success.
 * @retval 1 if the message could not be encoded.
 */
int MIDIMessageDecode( struct MIDIMessage * message, size_t size, unsigned char * buffer, size_t * read ) {
  if( size == 0 || buffer == NULL ) return 1;
  _check_release_data( message );
  message->format = MIDIMessageFormatDetect( buffer );
  if( message->format == NULL ) return 1;
  return MIDIMessageFormatDecode( message->format, &(message->data), size, buffer, read );
}

/**
 * @brief Encode messages with running status.
 * Encode message objects into a buffer. Use the running status and update it if necessary.
 * @public @memberof MIDIMessageFormat
 * @param message  The message.
 * @param status   A pointer to the running status.
 * @param size     The size of the memory pointed to by @c buffer.
 * @param buffer   The buffer to decode the message from.
 * @retval 0 on success.
 * @retval 1 if the message could not be encoded.
 */
int MIDIMessageEncodeRunningStatus( struct MIDIMessage * message, MIDIRunningStatus * status,
                                    size_t size, unsigned char * buffer, size_t * written ) {
  if( size == 0 || buffer == NULL ) return 1;
  return MIDIMessageFormatEncodeRunningStatus( message->format, &(message->data), status, size, buffer, written );
}
                                    
/**
 * @brief Decode messages with running status.
 * Decode message objects from a buffer. Use the running status and update it if necessary.
 * @public @memberof MIDIMessageFormat
 * @param message  The message.
 * @param status   A pointer to the running status.
 * @param size     The size of the memory pointed to by @c buffer.
 * @param buffer   The buffer to decode the message from.
 * @retval 0 on success.
 * @retval 1 if the message could not be encoded.
 */
int MIDIMessageDecodeRunningStatus( struct MIDIMessage * message, MIDIRunningStatus * status,
                                    size_t size, unsigned char * buffer, size_t * read ) {
  if( size == 0 || buffer == NULL ) return 1;
  _check_release_data( message );
  message->format = MIDIMessageFormatDetectRunningStatus( buffer, status );
  if( message->format == NULL ) return 1;
  return MIDIMessageFormatDecodeRunningStatus( message->format, &(message->data), status, size, buffer, read );
}


/**
 * @brief Encode multiple messages at one.
 * Encode multiple message objects into a buffer and use running status coding.
 * @public @memberof MIDIMessage
 * @param messages The list of messages.
 * @param size     The size of the memory pointed to by @c buffer.
 * @param buffer   The buffer to encode the message into.
 * @retval 0 on success.
 * @retval 1 if the message could not be encoded.
 */
int MIDIMessageListEncode( struct MIDIMessageList * messages, size_t size, unsigned char * buffer, size_t * written ) {
  struct MIDIMessage * message;
  MIDIRunningStatus status = 0;
  size_t p = 0, w = 0, s = size;
  int result = 0;

  while( messages != NULL && result == 0) {
    message = messages->message;
    if( message != NULL ) {
      result = MIDIMessageFormatEncodeRunningStatus( message->format, &(message->data), &status, s, buffer+p, &w );
      p += w;
      s -= w;
    }
    messages = messages->next;
  }
  if( written != NULL ) *written = p;
  return result;
}

/**
 * @brief Decode multiple messages at once.
 * Decode multiple message objects from a buffer and use running status coding.
 * @public @memberof MIDIMessageFormat
 * @param messages The list of messages.
 * @param size     The size of the memory pointed to by @c buffer.
 * @param buffer   The buffer to decode the message from.
 * @retval 0 on success.
 * @retval 1 if the message could not be encoded.
 */
int MIDIMessageListDecode( struct MIDIMessageList * messages, size_t size, unsigned char * buffer, size_t * read ) {
  struct MIDIMessage * message;
  MIDIRunningStatus status = 0;
  size_t p = 0, r = 0, s = size;
  int result = 0;

  while( messages != NULL && result == 0) {
    message = messages->message;
    if( message != NULL ) {
      _check_release_data( message );
      message->format = MIDIMessageFormatDetectRunningStatus( buffer+p, &status );
      if( message->format == NULL ) {
        result = 1;
      } else {
        result = MIDIMessageFormatDecodeRunningStatus( message->format, &(message->data), &status, s, buffer+p, &r );
        p += r;
        s -= r;
      }
    }
    messages = messages->next;
  }
  if( read != NULL ) *read = p;
  return result;
}

/** @} */
