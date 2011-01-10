#ifndef MIDIKIT_DRIVER_RTPMIDI_H
#define MIDIKIT_DRIVER_RTPMIDI_H
#include <stdlib.h>
#include "rtp.h"
#include "midi/message.h"

struct RTPMIDISession;

struct RTPMIDISession * RTPMIDISessionCreate( struct RTPSession * session );
void RTPMIDISessionDestroy( struct RTPMIDISession * session );
void RTPMIDISessionRetain( struct RTPMIDISession * session );
void RTPMIDISessionRelease( struct RTPMIDISession * session );

int RTPMIDISessionTrunkateSendJournal( struct RTPMIDISession * session, struct RTPPeer * peer, unsigned long seqnum );
int RTPMIDISessionTrunkateReceiveJournal( struct RTPMIDISession * session, struct RTPPeer * peer, unsigned long seqnum );

int RTPMIDIPeerSetInfo( struct RTPPeer * peer, void * info );
int RTPMIDIPeerGetInfo( struct RTPPeer * peer, void ** info );

int RTPMIDISessionSend( struct RTPMIDISession * session, struct MIDIMessageList * messages,
                        struct RTPPacketInfo * info );
int RTPMIDISessionReceive( struct RTPMIDISession * session, struct MIDIMessageList * messages,
                           struct RTPPacketInfo * info );

#endif
