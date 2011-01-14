#ifndef MIDIKIT_DRIVER_RTPMIDI_H
#define MIDIKIT_DRIVER_RTPMIDI_H
#include "midi/driver.h"

struct MIDIDriverRTPMIDI;

struct MIDIDriverRTPMIDI * MIDIDriverRTPMIDICreate( struct MIDIDriverDelegate * delegate );
void MIDIDriverRTPMIDIDestroy( struct MIDIDriverRTPMIDI * driver );
void MIDIDriverRTPMIDIRetain( struct MIDIDriverRTPMIDI * driver );
void MIDIDriverRTPMIDIRelease( struct MIDIDriverRTPMIDI * driver );

int MIDIDriverRTPMIDISetPort( struct MIDIDriverRTPMIDI * driver, unsigned short port ); 
int MIDIDriverRTPMIDIGetPort( struct MIDIDriverRTPMIDI * driver, unsigned short * port ); 

int MIDIDriverRTPMIDISetPeerAddress( struct MIDIDriverRTPMIDI * driver, size_t length, char * address, unsigned short port );
int MIDIDriverRTPMIDIGetPeerAddress( struct MIDIDriverRTPMIDI * driver, size_t length, char * address, unsigned short * port );

int MIDIDriverRTPMIDIGetLatency( struct MIDIDriverRTPMIDI * driver, double seconds );

int MIDIDriverRTPMIDIConnect( struct MIDIDriverRTPMIDI * driver );
int MIDIDriverRTPMIDIDisconnect( struct MIDIDriverRTPMIDI * driver );

#endif
