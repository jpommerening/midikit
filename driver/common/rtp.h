#ifndef MIDIKIT_DRIVER_RTP_H
#define MIDIKIT_DRIVER_RTP_H
#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>

struct RTPPeer;
struct RTPSession;

struct RTPPacketInfo {
  struct RTPPeer * peer;
  unsigned char  padding;
  unsigned char  extension;
  unsigned char  csrc_count;
  unsigned char  marker;
  unsigned char  payload_type;
  unsigned short sequence_number;
  unsigned long  timestamp;
  unsigned long  ssrc;
  unsigned long  csrc[16];
  size_t total_size;
  size_t payload_size;
  size_t iovlen;
  struct iovec * iov;
};

struct RTPPeer * RTPPeerCreate( unsigned long ssrc, socklen_t size, struct sockaddr * addr );
void RTPPeerDestroy( struct RTPPeer * peer );
void RTPPeerRetain( struct RTPPeer * peer );
void RTPPeerRelease( struct RTPPeer * peer );

int RTPPeerGetSSRC( struct RTPPeer * peer, unsigned long * ssrc );
int RTPPeerGetAddress( struct RTPPeer * peer, socklen_t * size, struct sockaddr ** addr );
int RTPPeerSetInfo( struct RTPPeer * peer, void * info );
int RTPPeerGetInfo( struct RTPPeer * peer, void ** info );

struct RTPSession * RTPSessionCreate( int socket );
void RTPSessionDestroy( struct RTPSession * session );
void RTPSessionRetain( struct RTPSession * session );
void RTPSessionRelease( struct RTPSession * session );

int RTPSessionSetSSRC( struct RTPSession * session, unsigned long ssrc );
int RTPSessionGetSSRC( struct RTPSession * session, unsigned long * ssrc );
int RTPSessionSetSocket( struct RTPSession * session, int socket );
int RTPSessionGetSocket( struct RTPSession * session, int * socket );

int RTPSessionAddPeer( struct RTPSession * session, struct RTPPeer * peer );
int RTPSessionRemovePeer( struct RTPSession * session, struct RTPPeer * peer );
int RTPSessionNextPeer( struct RTPSession * session, struct RTPPeer ** peer );
int RTPSessionFindPeerBySSRC( struct RTPSession * session, struct RTPPeer ** peer,
                              unsigned long ssrc );
int RTPSessionFindPeerByAddress( struct RTPSession * session, struct RTPPeer ** peer,
                                 socklen_t size, struct sockaddr * addr );

int RTPSessionSendPacket( struct RTPSession * session, struct RTPPacketInfo * info );
int RTPSessionReceivePacket( struct RTPSession * session, struct RTPPacketInfo * info );
int RTPSessionSend( struct RTPSession * session, size_t size, void * payload, struct RTPPacketInfo * info );
int RTPSessionReceive( struct RTPSession * session, size_t size, void * payload, struct RTPPacketInfo * info );

#endif
