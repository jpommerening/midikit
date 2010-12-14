#ifndef MIDIKIT_DRIVER_RTP_H
#define MIDIKIT_DRIVER_RTP_H
#include <stdlib.h>
#include <sys/socket.h>

#define RTP_MAX_PEERS 16

struct RTPAddress {
  unsigned long ssrc;
  socklen_t size;
  struct sockaddr * addr;
};

struct RTPPeer {
  size_t refs;
  struct RTPAddress address;
  unsigned long timestamp_diff;
  unsigned long in_timestamp;
  unsigned long out_timestamp;
  unsigned long in_seqnum;
  unsigned long out_seqnum;
  void * info;
}; 

struct RTPPacketInfo {
  unsigned char  padding;         // number of padding bytes
  unsigned char  extension;       // extension present?
  unsigned char  csrc_count;      // number of valid entries in csrc[]
  unsigned char  marker;          // marker set?
  unsigned char  payload_type;
  unsigned short sequence_number;
  unsigned long  timestamp;
  unsigned long  ssrc;     // sender ssrc identifier
  unsigned long  csrc[16];
  size_t size; // size of payload
  void * data; // payload
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

int RTPSendToPeer( struct RTPSession * session, struct RTPPeer * peer, size_t size, void * payload, struct RTPPacketInfo * info );
int RTPReceiveFromPeer( struct RTPSession * session, struct RTPPeer * peer, size_t size, void * payload, struct RTPPacketInfo * info );

#endif
