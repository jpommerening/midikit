#ifndef MIDIKIT_DRIVER_APPLEMIDI_H
#define MIDIKIT_DRIVER_APPLEMIDI_H
#include "midi/driver.h"

#define MIDI_APPLEMIDI_PROTOCOL_SIGNATURE          0xffff

#define MIDI_APPLEMIDI_COMMAND_INVITATION          0x494e /** "IN" */
#define MIDI_APPLEMIDI_COMMAND_INVITATION_REJECTED 0x4e4f /** "NO" */
#define MIDI_APPLEMIDI_COMMAND_INVITATION_ACCEPTED 0x4f4b /** "OK" */
#define MIDI_APPLEMIDI_COMMAND_ENDSESSION          0x4259 /** "BY" */
#define MIDI_APPLEMIDI_COMMAND_SYNCHRONIZATION     0x434b /** "CK" */
#define MIDI_APPLEMIDI_COMMAND_RECEIVER_FEEDBACK   0x5253 /** "RS" */

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

int MIDIDriverAppleMIDISetPeerAddress( struct MIDIDriverAppleMIDI * driver, size_t length, char * address, unsigned short port );
int MIDIDriverAppleMIDIGetPeerAddress( struct MIDIDriverAppleMIDI * driver, size_t length, char * address, unsigned short * port );

int MIDIDriverAppleMIDIGetLatency( struct MIDIDriverAppleMIDI * driver, double seconds );

//int MIDIDriverAppleMIDIGetRTPSocket( struct MIDIDriverAppleMIDI * driver, int * sockid );
//int MIDIDriverAppleMIDIGetControlSocket( struct MIDIDriverAppleMIDI * driver, int * sockid );

int MIDIDriverAppleMIDIConnect( struct MIDIDriverAppleMIDI * driver );
int MIDIDriverAppleMIDIDisconnect( struct MIDIDriverAppleMIDI * driver );

#endif
