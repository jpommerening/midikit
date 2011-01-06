#ifndef MIDIKIT_MIDI_CFINTEGRATION_H
#define MIDIKIT_MIDI_CFINTEGRATION_H
#ifdef __APPLE__
#include <CoreFoundation/CFSocket.h>
#include <CoreFoundation/CFRunLoop.h>

struct MIDIDriver;

/*
CFRunLoopSourceRef MIDIDriverCreateRunloopSource( struct MIDIDriver * driver );
*/

#endif
#endif
