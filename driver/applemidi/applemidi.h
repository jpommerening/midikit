#ifndef MIDIKIT_DRIVER_APPLEMIDI_H
#define MIDIKIT_DRIVER_APPLEMIDI_H
#include <stdlib.h>
#include <sys/socket.h>
#include "midi/driver.h"

struct MIDIDriverAppleMIDI;
extern struct MIDIDriverDelegate MIDIDriverDelegateAppleMIDI;

struct MIDIDriverAppleMIDI * MIDIDriverAppleMIDICreate();
void MIDIDriverAppleMIDIDestroy( struct MIDIDriverAppleMIDI * driver );
void MIDIDriverAppleMIDIRetain( struct MIDIDriverAppleMIDI * driver );
void MIDIDriverAppleMIDIRelease( struct MIDIDriverAppleMIDI * driver );

int MIDIDriverAppleMIDISetRTPPort( struct MIDIDriverAppleMIDI * driver, unsigned short port ); 
int MIDIDriverAppleMIDIGetRTPPort( struct MIDIDriverAppleMIDI * driver, unsigned short * port ); 
int MIDIDriverAppleMIDISetControlPort( struct MIDIDriverAppleMIDI * driver, unsigned short port ); 
int MIDIDriverAppleMIDIGetControlPort( struct MIDIDriverAppleMIDI * driver, unsigned short * port ); 

int MIDIDriverAppleMIDIAddPeer( struct MIDIDriverAppleMIDI * driver, socklen_t size_t, struct sockaddr * address );
int MIDIDriverAppleMIDIRemovePeer( struct MIDIDriverAppleMIDI * driver, socklen_t size_t, struct sockaddr * address );

int MIDIDriverAppleMIDIGetLatency( struct MIDIDriverAppleMIDI * driver, double seconds );

int MIDIDriverAppleMIDIGetRTPSocket( struct MIDIDriverAppleMIDI * driver, int * socket );
int MIDIDriverAppleMIDIGetControlSocket( struct MIDIDriverAppleMIDI * driver, int * socket );

int MIDIDriverAppleMIDIConnect( struct MIDIDriverAppleMIDI * driver );
int MIDIDriverAppleMIDIDisconnect( struct MIDIDriverAppleMIDI * driver );

#endif
