#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include "event.h"
#include "type.h"
#include "midi.h"

struct MIDIEvent {
  int refs;
  int id;
  char * message;
  void * info;
};

MIDI_TYPE_SPEC_CODING( MIDIEvent, 0x3010 );

struct MIDIEvent * MIDIEventCreate( int id, void * info, char * message, ... ) {
  size_t length;
  struct MIDIEvent * event = malloc( sizeof( struct MIDIEvent ) );
  MIDIPrecondReturn( event != NULL, ENOMEM, NULL );

  event->refs = 1;
  event->id   = id;
  event->info = info;

  if( message != NULL ) {
    length = strlen( message );
    event->message = malloc( length );
    if( event->message == NULL ) {
      free( event );
      return NULL;
    }
    strncpy( event->message, message, length );
  } else {
    event->message = NULL;
  }

  return event;
}

void MIDIEventDestroy( struct MIDIEvent * event ) {
  MIDIPrecondReturn( event != NULL, EFAULT, (void)0 );
  if( event->message != NULL ) {
    free( event->message );
  }
  free( event );
}

void MIDIEventRetain( struct MIDIEvent * event ) {
  MIDIPrecondReturn( event != NULL, EFAULT, (void)0 );
  event->refs++;
}

void MIDIEventRelease( struct MIDIEvent * event ) {
  MIDIPrecondReturn( event != NULL, EFAULT, (void)0 );
  if( ! --event->refs ) {
    MIDIEventDestroy( event );
  }
}

int MIDIEventEncode( struct MIDIEvent * event, size_t size, void * buffer, size_t * written ) {
  MIDIPrecond( event != NULL, EFAULT );
  MIDIPrecond( buffer != NULL, EINVAL );
  size_t required = 4 + strlen( event->message );
  MIDIPrecond( size >= required, ENOMEM );

  return 0;
}

int MIDIEventDecode( struct MIDIEvent * event, size_t size, void * buffer, size_t * read ) {
  MIDIPrecond( event != NULL, EFAULT );
  MIDIPrecond( buffer != NULL, EINVAL );
  size_t required = 5;
  MIDIPrecond( size >= required, EINVAL );
  
  return 1;
}

