#ifndef MIDIKIT_DRIVER_RTP_H
#define MIDIKIT_DRIVER_RTP_H
#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>

#define RTP_MAX_PEERS 16

struct RTPPacketInfo {
  struct RTPPeer * peer;
  unsigned char  padding;         // number of padding bytes
  unsigned char  extension;       // extension present?
  unsigned char  csrc_count;      // number of valid entries in csrc[]
  unsigned char  marker;          // marker set?
  unsigned char  payload_type;
  unsigned short sequence_number;
  unsigned long  timestamp;
  unsigned long  ssrc;            // sender ssrc identifier
  unsigned long  csrc[16];
  size_t total_size;   // total packet size in bytes
  size_t payload_size; // size of payload
  void * payload;      // payload
};


struct RTPPeer * RTPPeerCreate( unsigned long ssrc, socklen_t size, struct sockaddr * addr );
void RTPPeerDestroy( struct RTPPeer * peer );
void RTPPeerRetain( struct RTPPeer * peer );
void RTPPeerRelease( struct RTPPeer * peer );

int RTPPeerGetSSRC( struct RTPPeer * peer, unsigned long * ssrc );
int RTPPeerGetAddress( struct RTPPeer * peer, socklen_t * size, struct sockaddr ** addr );
int RTPPeerGetInfo( struct RTPPeer * peer, void * info );

struct RTPSession * RTPSessionCreate( int socket );
void RTPSessionDestroy( struct RTPSession * session );
void RTPSessionRetain( struct RTPSession * session );
void RTPSessionRelease( struct RTPSession * session );

int RTPSessionSetMarker( struct RTPSession * session, unsigned char marker );
int RTPSessionGetMarker( struct RTPSession * session, unsigned char * marker );
int RTPSessionSetPayloadType( struct RTPSession * session, unsigned char payload_type );
int RTPSessionGetPayloadType( struct RTPSession * session, unsigned char * payload_type );
int RTPSessionSetPaddingBlockSize( struct RTPSession * session, size_t size );
int RTPSessionGetPaddingBlockSize( struct RTPSession * session, size_t * size );

int RTPSessionSetSamplingRate( struct RTPSession * session, double rate );
int RTPSessionGetSamplingRate( struct RTPSession * session, double * rate );

int RTPSessionAddPeer( struct RTPSession * session, struct RTPPeer * peer );
int RTPSessionRemovePeer( struct RTPSession * session, struct RTPPeer * peer );
int RTPSessionFindPeerBySSRC( struct RTPSession * session, struct RTPPeer ** peer,
                              unsigned long ssrc );
int RTPSessionFindPeerByAddress( struct RTPSession * session, struct RTPPeer ** peer,
                                 socklen_t size, struct sockaddr * addr );

int RTPSessionSendToPeer( struct RTPSession * session, struct RTPPeer * peer, size_t size, void * payload, struct RTPPacketInfo * info );
int RTPSessionReceiveFromPeer( struct RTPSession * session, struct RTPPeer * peer, size_t size, void * payload, struct RTPPacketInfo * info );

#endif
