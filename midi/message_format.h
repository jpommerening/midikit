#ifndef MIDIKIT_MIDI_MESSAGE_FORMAT_H
#define MIDIKIT_MIDI_MESSAGE_FORMAT_H
#include "midi.h"

#define MIDI_MESSAGE_DATA_BYTES 4

struct MIDIMessageData {
  unsigned char bytes[MIDI_MESSAGE_DATA_BYTES];
  size_t size;
  void * data;
};

struct MIDIMessageFormat * MIDIMessageFormatDetect( void * buffer );
struct MIDIMessageFormat * MIDIMessageFormatDetectRunningStatus( void * buffer, MIDIRunningStatus * status );
struct MIDIMessageFormat * MIDIMessageFormatForStatus( MIDIStatus status );
int MIDIMessageFormatTest( struct MIDIMessageFormat * format, void * buffer );
int MIDIMessageFormatGetSize( struct MIDIMessageFormat * format, struct MIDIMessageData * data,
                              size_t * size );
int MIDIMessageFormatSet( struct MIDIMessageFormat * format, struct MIDIMessageData * data,
                          MIDIProperty property, size_t size, void * value );
int MIDIMessageFormatGet( struct MIDIMessageFormat * format, struct MIDIMessageData * data,
                          MIDIProperty property, size_t size, void * value );
int MIDIMessageFormatEncodeRunningStatus( struct MIDIMessageFormat * format, struct MIDIMessageData * data,
                                          MIDIStatus * status, size_t size, void * buffer, size_t * written );
int MIDIMessageFormatDecodeRunningStatus( struct MIDIMessageFormat * format, struct MIDIMessageData * data,
                                          MIDIStatus * status, size_t size, void * buffer, size_t * read );
int MIDIMessageFormatEncode( struct MIDIMessageFormat * format, struct MIDIMessageData * data,
                             size_t size, void * buffer, size_t * written );
int MIDIMessageFormatDecode( struct MIDIMessageFormat * format, struct MIDIMessageData * data,
                             size_t size, void * buffer, size_t * read );

#endif
