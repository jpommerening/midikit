#ifndef MIDIKIT_MIDI_MESSAGE_H
#define MIDIKIT_MIDI_MESSAGE_H
#include <stdlib.h>
#include "midi.h"
#include "clock.h"
#include "type.h"

struct MIDIMessage;
extern struct MIDITypeSpec * MIDIMessageType;

struct MIDIMessageList {
/*size_t refs;
  size_t length;*/
  struct MIDIMessage * message;
  struct MIDIMessageList * next;
};

struct MIDIMessage * MIDIMessageCreate( MIDIStatus status );
void MIDIMessageDestroy( struct MIDIMessage * message );
void MIDIMessageRetain( struct MIDIMessage * message );
void MIDIMessageRelease( struct MIDIMessage * message );

int MIDIMessageSetStatus( struct MIDIMessage * message, MIDIStatus status );
int MIDIMessageGetStatus( struct MIDIMessage * message, MIDIStatus * status );
int MIDIMessageSetTimestamp( struct MIDIMessage * message, MIDITimestamp timestamp );
int MIDIMessageGetTimestamp( struct MIDIMessage * message, MIDITimestamp * timestamp );
int MIDIMessageGetSize( struct MIDIMessage * message, size_t * size );
int MIDIMessageSet( struct MIDIMessage * message, MIDIProperty property, size_t size, void * value );
int MIDIMessageGet( struct MIDIMessage * message, MIDIProperty property, size_t size, void * value );

int MIDIMessageEncode( struct MIDIMessage * message, size_t size, unsigned char * buffer, size_t * written );
int MIDIMessageDecode( struct MIDIMessage * message, size_t size, unsigned char * buffer, size_t * read );

int MIDIMessageEncodeRunningStatus( struct MIDIMessage * message, MIDIRunningStatus * status,
                                    size_t size, unsigned char * buffer, size_t * written );
int MIDIMessageDecodeRunningStatus( struct MIDIMessage * message, MIDIRunningStatus * status,
                                    size_t size, unsigned char * buffer, size_t * read );

/*
struct MIDIMessageList * MIDIMessageListCreate( size_t length );
void MIDIMessageListDestroy( struct MIDIMessageList * messages );
void MIDIMessageListRetain( struct MIDIMessageList * messages );
void MIDIMessageListRelease( struct MIDIMessageList * messages );

int MIDIMessageListAppend( struct MIDIMessageList * messages, struct MIDIMessage * message );
int MIDIMessageListConcat( struct MIDIMessageList * messages, struct MIDIMessageList * other );

int MIDIMessageListGetLength( struct MIDIMessageList * messages, size_t * length );
*/
int MIDIMessageListEncode( struct MIDIMessageList * messages, size_t size, unsigned char * buffer, size_t * written );
int MIDIMessageListDecode( struct MIDIMessageList * messages, size_t size, unsigned char * buffer, size_t * read );

#endif
