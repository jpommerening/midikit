#ifndef MIDIKIT_MIDI_MESSAGE_FORMAT_H
#define MIDIKIT_MIDI_MESSAGE_FORMAT_H
#include <stdint.h>
#include "midi.h"

#define MIDI_MESSAGE_DATA_BYTES 4

struct MIDIMessageData {
  uint8_t bytes[MIDI_MESSAGE_DATA_BYTES];
  size_t  size;
  void *  data;
};

struct MIDIMessageFormat * MIDIMessageFormatDetect( void * buffer );
struct MIDIMessageFormat * MIDIMessageFormatForStatus( MIDIStatus status );
int MIDIMessageFormatTest( struct MIDIMessageFormat * format, void * buffer );
int MIDIMessageFormatGetSize( struct MIDIMessageFormat * format, struct MIDIMessageData * data,
                              size_t * size );
int MIDIMessageFormatSet( struct MIDIMessageFormat * format, struct MIDIMessageData * data,
                          MIDIProperty property, size_t size, void * value );
int MIDIMessageFormatGet( struct MIDIMessageFormat * format, struct MIDIMessageData * data,
                          MIDIProperty property, size_t size, void * value );
int MIDIMessageFormatEncode( struct MIDIMessageFormat * format, struct MIDIMessageData * data,
                             size_t size, void * buffer );
int MIDIMessageFormatDecode( struct MIDIMessageFormat * format, struct MIDIMessageData * data,
                             size_t size, void * buffer );

#endif
