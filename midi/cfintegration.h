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

struct CFMIDIRunloopSource * CFMIDIRunloopSourceCreate( struct MIDIRunloopSource * source );
void CFMIDIRunloopSourceDestroy( struct CFMIDIRunloopSource * source );
/*void CFMIDIRunloopSourceRetain( struct CFMIDIRunloopSource * source );*/
/*void CFMIDIRunloopSourceRelease( struct CFMIDIRunloopSource * source );*/

void CFRunLoopAddMIDIRunloopSource( CFRunLoopRef rl, struct CFMIDIRunloopSource * source, CFStringRef mode );
void CFRunLoopRemoveMIDIRunloopSource( CFRunLoopRef rl, struct CFMIDIRunloopSource * source, CFStringRef mode );

#endif
