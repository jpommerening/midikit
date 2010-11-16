#include <stdlib.h>
#include "message.h"
#include "message_format.h"

/**
 * Structure of MIDI message object.
 */
struct MIDIMessage {
  size_t refs;
  struct MIDIMessageFormat * format;
  struct MIDIMessageData data;
  MIDITimestamp timestamp;
};


struct MIDIMessage * MIDIMessageCreate( MIDIStatus status ) {
  struct MIDIMessage * message;
  struct MIDIMessageFormat * format;
  MIDITimestamp timestamp = 0;
  int i;

  format = MIDIMessageFormatForStatus( status );
  if( format == NULL ) {
    return NULL;
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
  MIDIMessageSetStatus( message, status );
  MIDIMessageSetTimestamp( message, timestamp );
  return message;
}

void MIDIMessageDestroy( struct MIDIMessage * message ) {
  if( message->data.data != NULL && message->data.bytes[3] == 1 ) free( message->data.data );
  free( message );
}

void MIDIMessageRetain( struct MIDIMessage * message ) {
  message->refs++;
}

void MIDIMessageRelease( struct MIDIMessage * message ) {
  if( ! --message->refs ) {
    MIDIMessageDestroy( message );
  }
}

int MIDIMessageSetStatus( struct MIDIMessage * message, MIDIStatus status ) {
  struct MIDIMessageFormat * format = MIDIMessageFormatForStatus( status );
  if( format == NULL ) return 1;
  message->format = format;
  return MIDIMessageSet( message, MIDI_STATUS, sizeof( MIDIStatus ), &status );
}

int MIDIMessageGetStatus( struct MIDIMessage * message, MIDIStatus * status ) {
  return MIDIMessageGet( message, MIDI_STATUS, sizeof( MIDIStatus ), status );
}

int MIDIMessageSetTimestamp( struct MIDIMessage * message, MIDITimestamp timestamp ) {
  if( message == NULL ) return 1;
  message->timestamp = timestamp;
  return 0;
}

int MIDIMessageGetTimestamp( struct MIDIMessage * message, MIDITimestamp * timestamp ) {
  if( message == NULL || timestamp == NULL ) return 1;
  *timestamp = message->timestamp;
  return 0;
}

int MIDIMessageGetSize( struct MIDIMessage * message, size_t * size ) {
  if( message == NULL || size == NULL ) return 1;
  return MIDIMessageFormatGetSize( message->format, &(message->data), size );
}

int MIDIMessageSet( struct MIDIMessage * message, MIDIProperty property, size_t size, void * value ) {
  return MIDIMessageFormatSet( message->format, &(message->data), property, size, value );
}

int MIDIMessageGet( struct MIDIMessage * message, MIDIProperty property, size_t size, void * value ) {
  return MIDIMessageFormatGet( message->format, &(message->data), property, size, value );
}

int MIDIMessageDecode( struct MIDIMessage * message, size_t size, unsigned char * buffer ) {
  return MIDIMessageFormatDecode( message->format, &(message->data), size, buffer );
}

int MIDIMessageEncode( struct MIDIMessage * message, size_t size, unsigned char * buffer ) {
  return MIDIMessageFormatEncode( message->format, &(message->data), size, buffer );
}
