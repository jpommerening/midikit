#ifndef MIDIKIT_MIDI_CFINTEGRATION_H
#define MIDIKIT_MIDI_CFINTEGRATION_H
#ifdef __APPLE__
#include <CoreFoundation/CoreFoundation.h>
#if TARGET_OS_MAC
#include <CoreServices/CoreServices.h>
#endif
#if TARGET_OS_IPHONE
#include <CFNetwork/CFNetServices.h>
#endif

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
