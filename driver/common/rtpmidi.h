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


int RTPMIDISessionSend( struct RTPMIDISession * session, size_t size, struct MIDIMessage ** messages,
                        size_t * count, struct RTPPacketInfo * info );
int RTPMIDISessionReceive( struct RTPMIDISession * session, size_t size, struct MIDIMessage ** messages,
                           size_t * count, struct RTPPacketInfo * info );

#endif
