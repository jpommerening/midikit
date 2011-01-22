#ifndef MIDIKIT_DRIVER_RTPMIDI_H
#define MIDIKIT_DRIVER_RTPMIDI_H
#include <stdlib.h>
#include "midi/message.h"

struct RTPPeer;
struct RTPSession;

struct RTPMIDISession;

struct RTPMIDISession * RTPMIDISessionCreate( struct RTPSession * session );
void RTPMIDISessionDestroy( struct RTPMIDISession * session );
void RTPMIDISessionRetain( struct RTPMIDISession * session );
void RTPMIDISessionRelease( struct RTPMIDISession * session );

int RTPMIDISessionJournalTrunkate( struct RTPMIDISession * session, struct RTPPeer * peer, unsigned long seqnum );
int RTPMIDISessionJournalStoreMessages( struct RTPMIDISession * session, struct RTPPeer * peer,
                                        unsigned long seqnum, struct MIDIMessageList * messages );
int RTPMIDISessionJournalRecoverMessages( struct RTPMIDISession * session, struct RTPPeer * peer,
                                          unsigned long seqnum, struct MIDIMessageList * messages );

int RTPMIDIPeerSetInfo( struct RTPPeer * peer, void * info );
int RTPMIDIPeerGetInfo( struct RTPPeer * peer, void ** info );

int RTPMIDISessionSend( struct RTPMIDISession * session, struct MIDIMessageList * messages );
int RTPMIDISessionReceive( struct RTPMIDISession * session, struct MIDIMessageList * messages );

#endif
