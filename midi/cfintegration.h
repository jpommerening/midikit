#ifndef MIDIKIT_MIDI_CFINTEGRATION_H
#define MIDIKIT_MIDI_CFINTEGRATION_H
#include <CoreFoundation/CoreFoundation.h>
#if TARGET_OS_IPHONE
#include <CFNetwork/CFNetServices.h>
#else
#if TARGET_OS_MAC
#include <CoreServices/CoreServices.h>
#endif
#endif


struct MIDIDriver;

CFRunLoopSourceRef MIDIDriverCreateRunloopSource( struct MIDIDriver * driver );


#include "driver/applemidi/applemidi.h"

int MIDIDriverAppleMIDIAddPeerWithCFNetService( struct MIDIDriverAppleMIDI * driver,
                                                CFNetServiceRef netService );
int MIDIDriverAppleMIDIRemovePeerWithCFNetService( struct MIDIDriverAppleMIDI * driver,
                                                   CFNetServiceRef netService );

#include "midi/runloop.h"

void CFRunLoopAddMIDIRunloopSource( CFRunLoopRef rl, struct MIDIRunloopSource * source, CFStringRef mode );

#endif
