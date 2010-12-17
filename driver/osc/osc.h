#ifndef MIDI_DRIVER_OSC_H
#define MIDI_DRIVER_OSC_H
#include <stdlib.h>
#include "midi/message.h"
#include "midi/driver.h"

/*
 * Default OSC namespace
 * See also: http://www.illposed.com/software/occam.html
/osc/midi/out/noteOn  	channel (int)  	key (int)  	velocity (int)
/osc/midi/out/noteOff 	channel (int) 	key (int) 	velocity (int)
/osc/midi/out/polyTouch 	channel (int) 	key (int) 	pressure (int)
/osc/midi/out/control 	channel (int) 	index (int) 	value (int)
/osc/midi/out/program 	channel (int) 	index (int)
/osc/midi/out/touch 	channel (int) 	pressure (int)
/osc/midi/out/bend 	channel (int) 	value (int)
/osc/midi/out/allNotesOff 	channel (int)
 */

#define MIDI_DRIVER_OSC_DEFAULT_ADDRESS_SPACE_OUT     "/osc/midi/out"
#define MIDI_DRIVER_OSC_DEFAULT_ADDRESS_SPACE_IN      "/osc/midi/in"
#define MIDI_DRIVER_OSC_DEFAULT_ADDRESS_RAW           "raw"
#define MIDI_DRIVER_OSC_DEFAULT_ADDRESS_NOTE_ON       "noteOn"
#define MIDI_DRIVER_OSC_DEFAULT_ADDRESS_NOTE_OFF      "noteOff"
#define MIDI_DRIVER_OSC_DEFAULT_ADDRESS_POLY_TOUCH    "polyTouch"
#define MIDI_DRIVER_OSC_DEFAULT_ADDRESS_CONTROL       "control"
#define MIDI_DRIVER_OSC_DEFAULT_ADDRESS_PROGRAM       "program"
#define MIDI_DRIVER_OSC_DEFAULT_ADDRESS_TOUCH         "touch"
#define MIDI_DRIVER_OSC_DEFAULT_ADDRESS_BEND          "bend"
#define MIDI_DRIVER_OSC_DEFAULT_ADDRESS_ALL_NOTES_OFF "allNotesOff"

struct MIDIDriverOSC;

struct MIDIDriverOSCDelegate {
  int (*encode)( struct MIDIDriverOSC * driver, struct MIDIMessage * message, size_t size, void * buffer );
  int (*decode)( struct MIDIDriverOSC * driver, struct MIDIMessage * message, size_t size, void * buffer );
};

struct MIDIDriverOSC * MIDIDriverOSCCreate( struct MIDIDriverOSCDelegate * delegate );
void MIDIDriverOSCDestroy( struct MIDIDriverOSC * driver );
void MIDIDriverOSCRetain( struct MIDIDriverOSC * driver );
void MIDIDriverOSCRelease( struct MIDIDriverOSC * driver );

int MIDIDriverOSCEncodeMessageDefault( struct MIDIDriverOSC * driver, struct MIDIMessage * message,
                                       size_t size, void * buffer, size_t * bytes_written );
int MIDIDriverOSCDecodeMessageDefault( struct MIDIDriverOSC * driver, struct MIDIMessage * message,
                                       size_t size, void * buffer, size_t * bytes_read );

int MIDIDriverOSCEncodeMessageRaw( struct MIDIDriverOSC * driver, struct MIDIMessage * message,
                                   size_t size, void * buffer, size_t * bytes_written );
int MIDIDriverOSCDecodeMessageRaw( struct MIDIDriverOSC * driver, struct MIDIMessage * message,
                                   size_t size, void * buffer, size_t * bytes_read );

#endif
