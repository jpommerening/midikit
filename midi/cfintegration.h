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

#include "driver/applemidi/applemidi.h"

int MIDIDriverAppleMIDIAddPeerWithCFNetService( struct MIDIDriverAppleMIDI * driver,
                                                CFNetServiceRef netService );
int MIDIDriverAppleMIDIRemovePeerWithCFNetService( struct MIDIDriverAppleMIDI * driver,
                                                   CFNetServiceRef netService );

#include "midi/runloop.h"
struct CFMIDIRunLoopSource;

struct CFMIDIRunLoopSource * CFMIDIRunLoopSourceCreate( struct MIDIRunloopSource * source );
void CFMIDIRunLoopSourceDestroy( struct CFMIDIRunLoopSource * source );
void CFMIDIRunLoopSourceRetain( struct CFMIDIRunLoopSource * source );
void CFMIDIRunLoopSourceRelease( struct CFMIDIRunLoopSource * source );

void CFRunLoopAddMIDIRunLoopSource( CFRunLoopRef rl, struct CFMIDIRunLoopSource * source, CFStringRef mode );
void CFRunLoopRemoveMIDIRunLoopSource( CFRunLoopRef rl, struct CFMIDIRunLoopSource * source, CFStringRef mode );

#endif
