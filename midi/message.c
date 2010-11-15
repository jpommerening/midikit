#include <stdlib.h>
#include "message.h"
#include "message_format.h"
#include "clock.h"

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
  struct MIDIMessage * message = malloc( sizeof( struct MIDIMessage ) );
  int i;
  if( message == NULL ) {
    return NULL;
  }
  message->refs   = 1;
  for( i=1; i<MIDI_MESSAGE_DATA_BYTES; i++ ) {
    message->data.bytes[i] = 0;
  }
  message->data.size = 0;
  message->data.data = NULL;
  message->format    = MIDIMessageFormatForStatus( status );
  if( message->format == NULL ) {
    free( message );
    return NULL;
  }
  MIDIMessageSetStatus( message, status );
  return message;
}

void MIDIMessageDestroy( struct MIDIMessage * message ) {
  if( message->data.data != NULL ) free( message->data.data );
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
  return MIDIMessageSet( message, MIDI_STATUS, sizeof( MIDIStatus ), &status );
}

int MIDIMessageGetStatus( struct MIDIMessage * message, MIDIStatus * status ) {
  return MIDIMessageGet( message, MIDI_STATUS, sizeof( MIDIStatus ), status );
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
