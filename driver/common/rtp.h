#ifndef MIDIKIT_DRIVER_RTP_H
#define MIDIKIT_DRIVER_RTP_H
#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>

#define RTP_MAX_PEERS 16
#define RTP_IOV_LEN 4

struct RTPPacketInfo {
  struct RTPPeer * peer;
  unsigned char  padding;         /**< number of padding bytes */
  unsigned char  extension;       /**< if set, use first iov element as header extension */
  unsigned char  csrc_count;      /**< number of valid entries in csrc[] */
  unsigned char  marker;          /**< set marker bit? */
  unsigned char  payload_type;    /**< type of payload */
  unsigned short sequence_number; /**< sequence number of the packet */
  unsigned long  timestamp;       /**< timestamp (the time the packet was sent */
  unsigned long  ssrc;            /**< sender ssrc identifier */
  unsigned long  csrc[16];        /**< contributing sources */
  size_t total_size;   /**< total packet size in bytes */
  size_t payload_size; /**< size of payload */
  void * payload;      /**< payload data */
/*replace payload with iovec structure?*/
  size_t iov_len;      /**< numver of elements in iov[] */
  struct iovec * iov;  /**< list of io vectors contianing pkg data */
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

int RTPSessionSetSocket( struct RTPSession * session, int socket );
int RTPSessionGetSocket( struct RTPSession * session, int * socket );
int RTPSessionSetSSRC( struct RTPSession * session, unsigned long ssrc );
int RTPSessionGetSSRC( struct RTPSession * session, unsigned long * ssrc );
int RTPSessionSetTimestampOffset( struct RTPSession * session, unsigned long offset );
int RTPSessionGetTimestampOffset( struct RTPSession * session, unsigned long * offset );
int RTPSessionSetTimestampRate( struct RTPSession * session, double rate );
int RTPSessionGetTimestampRate( struct RTPSession * session, double * rate );

int RTPSessionGetTimestamp( struct RTPSession * session, unsigned long long * timestamp );

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
