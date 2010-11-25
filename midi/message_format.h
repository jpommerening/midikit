#ifndef MIDIKIT_MIDI_MESSAGE_FORMAT_H
#define MIDIKIT_MIDI_MESSAGE_FORMAT_H
#include <stdint.h>
#include "midi.h"

#define MIDI_MESSAGE_DATA_BYTES 4

/**
 * @brief Store any kind of MIDI message.
 * Usually the message data only makes sense in combination with a message format.
 * However, there is one important thing to remember. You may only free the data
 * field if bytes[3] is one! (And you need to set it to one if you allocate some
 * buffer for it.
 *
 * The size and data fields are only used for system exclusive messages. Those
 * messages store the system exclusive data inside the data field. Status,
 * manufacturer ID and fragment number are stored in the bytes array.
 * @see MIDIMessageFormat
 */
struct MIDIMessageData {
  uint8_t bytes[MIDI_MESSAGE_DATA_BYTES];
  size_t  size;
  void *  data;
};

struct MIDIMessageFormat * MIDIMessageFormatDetect( void * buffer );
struct MIDIMessageFormat * MIDIMessageFormatForStatus( MIDIStatus status );
int MIDIMessageFormatTest( struct MIDIMessageFormat * format, void * buffer );
int MIDIMessageFormatGetSize( struct MIDIMessageFormat * format, struct MIDIMessageData * data, size_t * size );
int MIDIMessageFormatSet( struct MIDIMessageFormat * format, struct MIDIMessageData * data, MIDIProperty property, size_t size, void * value );
int MIDIMessageFormatGet( struct MIDIMessageFormat * format, struct MIDIMessageData * data, MIDIProperty property, size_t size, void * value );
int MIDIMessageFormatEncode( struct MIDIMessageFormat * format, struct MIDIMessageData * data, size_t size, void * buffer );
int MIDIMessageFormatDecode( struct MIDIMessageFormat * format, struct MIDIMessageData * data, size_t size, void * buffer );

#endif
