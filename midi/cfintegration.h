#ifndef MIDIKIT_MIDI_CFINTEGRATION_H
#define MIDIKIT_MIDI_CFINTEGRATION_H
#ifdef __APPLE__
#include <CoreFoundation/CFSocket.h>
#include <CoreFoundation/CFRunLoop.h>
#include <CFNetwork/CFNetServices.h>

#include "driver/applemidi/applemidi.h"

struct MIDIDriver;

/*
CFRunLoopSourceRef MIDIDriverCreateRunloopSource( struct MIDIDriver * driver );
*/

int MIDIDriverAppleMIDIAddPeerWithCFNetService( struct MIDIDriverAppleMIDI * driver,
                                                CFNetServiceRef netService );
int MIDIDriverAppleMIDIRemovePeerWithCFNetService( struct MIDIDriverAppleMIDI * driver,
                                                   CFNetServiceRef netService );

#endif
#endif
