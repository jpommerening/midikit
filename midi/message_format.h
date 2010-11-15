#ifndef MIDIKIT_MIDI_MESSAGE_FORMAT_H
#define MIDIKIT_MIDI_MESSAGE_FORMAT_H
#include <stdlib.h>
#include <stdint.h>
#include "midi.h"

struct MIDIMessageData {
  uint8_t bytes[4];
  size_t  size;
  void *  data;
};

/**
 * Functions that access messages of a certain type.
 */
struct MIDIMessageFormat {
  int (*test)( void * buffer );
  int (*set)( struct MIDIMessageData * data, MIDIProperty property, size_t size, void * value );
  int (*get)( struct MIDIMessageData * data, MIDIProperty property, size_t size, void * value );
  int (*encode)( struct MIDIMessageData * data, size_t size, void * buffer );
  int (*decode)( struct MIDIMessageData * data, size_t size, void * buffer );
};

struct MIDIMessageFormat * MIDIMessageFormatDetect( void * buffer );
struct MIDIMessageFormat * MIDIMessageFormatForStatus( MIDIStatus status );

#endif