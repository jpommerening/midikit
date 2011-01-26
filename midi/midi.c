#include <stdio.h>
#include "midi.h"

/**
 * @defgroup MIDI MIDI
 */

int MIDIErrorNumber = MIDI_ERR_NO_ERROR;

#ifdef DEBUG
MIDILogFunction MIDILogger = &printf;
#else
MIDILogFunction MIDILogger = NULL;
#endif
