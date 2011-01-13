#include "rtp.h"
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/time.h>
#include <arpa/inet.h>

#define SOCKADDR_BUFLEN 32
#define USEC_PER_SEC 1000000

struct RTPHeader {
  unsigned version         : 2;  /* automatic */
  unsigned padding         : 1;  /* packet spec. */
  unsigned extension       : 1;  /* packet spec. */
  unsigned csrc_count      : 4;  /* packet spec. */
  unsigned marker          : 1;  /* packet spec. */
  unsigned payload_type    : 7;  /* packet spec. */
  unsigned sequence_number : 16; /* automatic */
  unsigned long timestamp;       /* automatic */
  unsigned long ssrc;            /* automatic */
  unsigned long * csrc_list;     /* packet spec. */
};

/**
 * @struct RTPHeader rtp.h
 * @brief RTP header.
 * The RTP header has the following format:
 *
 * <pre>
 *     0                   1                   2                   3
 *     0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1
 *    +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 *    | V |P|X|  CC   |M|     PT      |        Sequence number        |
 *    +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 *    |                           Timestamp                           |
 *    +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 *    |                             SSRC                              |
 *    +=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+
 *    |            contributing source (CSRC) identifiers             |
 *    |                             ....                              |
 *    +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 * </pre>
 *
 * The first twelve octets are present in every RTP packet, while the
 * list of CSRC identifiers is present only when inserted by a mixer.
 */
/**
 * @public @property RTPHeader::version
 * @brief Version (V): 2 bits.
 * The version field identifies the version of RTP. The version we use
 * and the one specified by RFC3550 is two (2). (The value 1 is used by
 * the first draft version of RTP and the value 0 is used by the
 * protocol initially implemented in the "vat" audio tool.)
 */
/**
 * @public @property RTPHeader::padding
 * @brief Padding (P): 1 bit.
 * If the padding bit is set, the packet contains one or more additional
 * padding octets at the end which are not part of the payload. The last
 * octet of the padding contains a count of how many padding octets
 * should be ignored, including itself.  Padding may be needed by some
 * encryption algorithms with fixed block sizes or for carrying several
 * RTP packets in a lower-layer protocol data unit.
 */
/**
 * @public @property RTPHeader::extension
 * @brief Extension (X): 1 bit.
 * If the extension bit is set, the fixed header MUST be followed by
 * exactly one header extension, with a format defined by
 * RTPHeaderExtension.
 */
/**
 * @public @property RTPHeader::csrc_count
 * @brief CSRC count (CC): 4 bits.
 * The CSRC count contains the number of CSRC identifiers that follow
 * the fixed header.
 */
/**
 * @public @property RTPHeader::marker
 * @brief Marker (M): 1 bit.
 * The interpretation of the marker is defined by a profile.  It is
 * intended to allow significant events such as frame boundaries to be
 * marked in the packet stream. A profile MAY define additional marker
 * bits or specify that there is no marker bit by changing the number
 * of bits in the payload type field.
 */
/**
 * @public @property RTPHeader::payload_type
 * @brief Payload type (PT): 7 bits.
 * This field identifies the format of the RTP payload and determines
 * its interpretation by the application.  A profile MAY specify a
 * default static mapping of payload type codes to payload formats.
 */
/**
 * @public @property RTPHeader::sequence_number;
 * @brief Sequence number: 16 bits.
 * The sequence number increments by one for each RTP data packet sent,
 * and may be used by the receiver to detect packet loss and to restore
 * packet sequence. The initial value of the sequence number SHOULD be
 * random (unpredictable) to make known-plaintext attacks on encryption
 * more difficult, even if the source itself does not encrypt, because
 * the packets may flow through a translator that does.
 */
/**
 * @public @property RTPHeader::timestamp
 * @brief Timestamp: 32 bits.
 * The timestamp reflects the sampling instant of the first octet in
 * the RTP data packet. The sampling instant MUST be derived from a
 * clock that increments monotonically and linearly in time to allow
 * synchronization and jitter calculations.
 */
/**
 * @public @property RTPHeader::ssrc
 * @brief SSRC: 32 bits.
 * The SSRC field identifies the synchronization source.  This
 * identifier SHOULD be chosen randomly, with the intent that no two
 * synchronization sources within the same RTP session will have the
 * same SSRC identifier.
 */
/**
 * @public @property RTPHeader::csrc_list
 * @brief CSRC list: 0 to 15 items, 32 bits each.
 * The CSRC list identifies the contributing sources for the payload
 * contained in this packet. The number of identifiers is given by the
 * CC field. If there are more than 15 contributing sources, only 15 can
 * can be identified. CSRC identifiers are inserted by mixers, using the
 * SSRC identifiers of contributing sources. For example, for audio
 * packets the SSRC identifiers of all sources that were mixed together
 * to create a packet are listed, allowing correct talker indication at
 * the receiver.
 */

/**
 * @struct RTPHeaderExtension
 * @brief RTP header extension.
 * The RTP header extension has the following format:
 *
 * <pre>
 *     0                   1                   2                   3
 *     0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1
 *    +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 *    |      defined by profile       |           length              |
 *    +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 *    |                        header extension                       |
 *    |                             ....                              |
 *    +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 * </pre>
 *
 * An extension mechanism is provided to allow individual implementations
 * to experiment with new payload-format-independent functions that
 * require additional information to be carried in the RTP data packet
 * header. This mechanism is designed so that the header extension may
 * be ignored by other interoperating implementations that have not been
 * extended.
 */
/**
 * @public @property RTPHeaderExtension::profile_data
 * @brief Defined by profile: 16 bits.
 * To allow multiple interoperating implementations to each experiment
 * independently with different header extensions, or to allow a
 * particular implementation to experiment with more than one type of
 * header extension, the first 16 bits of the header extension are left
 * open for distinguishing identifiers or parameters.  The format of
 * these 16 bits is to be defined by the profile specification under
 * which the implementations are operating.  This RTP specification does
 * not define any header extensions itself.
 */
/**
 * @public @property RTPHeaderExtension::length
 * @brief Length: 16 bits.
 * The length field counts the number of 32-bit words in the extension,
 * excluding the four-octet extension header (therefore zero is a valid
 * length).
 */
/**
 * @public @property RTPHeaderExtension::header_extension
 * @brief Header extension.
 * The header extension may contain arbitary data defined by one or more
 * profiles.
 */

struct RTPAddress {
  unsigned long ssrc;
  socklen_t size;
  struct sockaddr_storage addr;
};

struct RTPPeer {
  size_t refs;
  struct RTPAddress address;
  unsigned long in_timestamp;
  unsigned long out_timestamp;
  unsigned long in_seqnum;
  unsigned long out_seqnum;
  void * info;
};

struct RTPSession {
  size_t refs;
  
  int socket;
  
  struct RTPAddress self;
  struct RTPPeer *  peers[RTP_MAX_PEERS];
  struct RTPPacketInfo info;
  
  size_t buflen;
  void * buffer;

  unsigned long timestamp_offset;
  double        timestamp_divisor;
};

/**
 * @brief Create an RTPPeer instance.
 * Allocate space and initialize an RTPPeer instance.
 * @public @memberof RTPPeer
 * @param ssrc The synchronization source identifier that uniquely identifies the peer.
 * @param size The size of the address pointed to by @c addr.
 * @param addr A pointer to an address that can be used to send packets to the client.
 * @return a pointer to the created peer structure on success.
 * @return a @c NULL pointer if the peer could not created.
 */
struct RTPPeer * RTPPeerCreate( unsigned long ssrc, socklen_t size, struct sockaddr * addr ) {
  struct RTPPeer * peer = malloc( sizeof( struct RTPPeer ) );
  peer->refs = 1;
  peer->address.ssrc = ssrc;
  peer->address.size = size;
  memcpy( &(peer->address.addr), addr, size );
  peer->in_seqnum      = 0;
  peer->in_timestamp   = 0;
  peer->out_seqnum     = 0;
  peer->out_timestamp  = 0;
  peer->info = NULL;
  return peer;
}

/**
 * @brief Destroy an RTPPeer instance.
 * Free all resources occupied by the peer.
 * @public @memberof RTPPeer
 * @param peer The peer.
 */
void RTPPeerDestroy( struct RTPPeer * peer ) {
  free( peer );
}

/**
 * @brief Retain an RTPPeer instance.
 * Increment the reference counter of a peer so that it won't be destroyed.
 * @public @memberof RTPPeer
 * @param peer The peer.
 */
void RTPPeerRetain( struct RTPPeer * peer ) {
  peer->refs++;
}

/**
 * @brief Release an RTPPeer instance.
 * Decrement the reference counter of a peer. If the reference count
 * reached zero, destroy the peer.
 * @public @memberof RTPPeer
 * @param peer The peer.
 */
void RTPPeerRelease( struct RTPPeer * peer ) {
  if( ! --peer->refs ) {
    RTPPeerDestroy( peer );
  }
}

/**
 * @brief Get the synchronization source identifier of a peer.
 * @public @memberof RTPPeer
 * @param peer The peer.
 * @param ssrc The synchronization source.
 * @retval 0 on success.
 * @retval >0 if the SSRC could not be obtained.
 */
int RTPPeerGetSSRC( struct RTPPeer * peer, unsigned long * ssrc ) {
  if( ssrc == NULL ) return 1;
  *ssrc = peer->address.ssrc;
  return 0;
}

/**
 * @brief Obtain the size of the address and a pointer to it's content.
 * @public @memberof RTPPeer
 * @param peer The peer.
 * @param size The size.
 * @param addr The address.
 * @retval 0 on success.
 * @retval >0 if the address could not be obtained.
 */
int RTPPeerGetAddress( struct RTPPeer * peer, socklen_t * size, struct sockaddr ** addr ) {
  if( size == NULL || addr == NULL ) return 1;
  *size =   peer->address.size;
  *addr = (struct sockaddr *) &(peer->address.addr);
  return 0;
}

/**
 * @brief Set the internal info pointer.
 * @public @memberof RTPPeer
 * @param peer The peer.
 * @param info The info.
 * @retval 0 on success.
 * @retval >0 if the info could not be changed.
 */
int RTPPeerSetInfo( struct RTPPeer * peer, void * info ) {
  peer->info = info;
  return 0;
}

/**
 * @brief Obtain a pointer to the info structure of a peer.
 * @public @memberof RTPPeer
 * @param peer The peer.
 * @param info The info.
 * @retval 0 on success.
 * @retval >0 if the info could not be obtained.
 */
int RTPPeerGetInfo( struct RTPPeer * peer, void ** info ) {
  if( info == NULL ) return 1;
  *info = peer->info;
  return 0;
}

#pragma mark Creation and destruction
/**
 * @name Creation and destruction
 * Creating, destroying and reference counting of RTPSession objects.
 * @{
 */

static void _init_addr_with_socket( struct RTPAddress * address, int socket ) {
  address->ssrc = 0;
  address->size = sizeof(address->addr);
  getsockname( socket, (struct sockaddr *) &(address->addr), &(address->size) );
}

static void _init_addr_empty( struct RTPAddress * address ) {
  address->ssrc = 0;
  address->size = 0;
  memset( &(address->addr), 0, sizeof(address->addr) );
}

static void _init_addr( struct RTPAddress * address, socklen_t size, struct sockaddr * addr ) {
  address->ssrc = 0;
  address->size = size;
  memcpy( &(address->addr), addr, size );
}

static unsigned long long _session_get_timestamp( struct RTPSession * session ) {
  struct timeval tv = { 0, 0 };
  unsigned long long ts;
  gettimeofday( &tv, NULL );

  ts = (1000000.0 * tv.tv_sec + tv.tv_usec) / session->timestamp_divisor
     + session->timestamp_offset;
  return ts;
}

static void _session_randomize_ssrc( struct RTPSession * session ) {
  unsigned char * ss_addr = (unsigned char *) &(session->self.addr);
  unsigned long i, seed = _session_get_timestamp( session );
  for( i=0; i<session->self.size; i++ ) {
    seed = (seed * 7) + ss_addr[i];
  }
  srandom( seed );
  session->self.ssrc = random();
}

/**
 * @brief Create an RTPSession instance.
 * Allocate space and initialize an RTPSession instance.
 * @public @memberof RTPSession
 * @param size The size of the socket address pointed to by @c addr.
 * @param addr A socket address.
 * @param type The communication type to use. (SOCK_DGRAM, SOCK_STREAM, etc.)
 * @return a pointer to the created controller structure on success.
 * @return a @c NULL pointer if the controller could not created.
 */
struct RTPSession * RTPSessionCreate( int socket ) {
  struct RTPSession * session = malloc( sizeof( struct RTPSession ) );
  int i;
  session->refs   = 1;
  session->socket = socket;

  _init_addr_with_socket( &(session->self), socket );
  for( i=0; i<RTP_MAX_PEERS; i++ ) {
    session->peers[i] = NULL;
  }
  
  session->buflen = 1024;
  session->buffer = malloc( session->buflen );
  if( session->buffer == NULL ) {
    session->buflen = 0;
  }
  
  session->timestamp_offset  = 0;
  session->timestamp_divisor = 1000000.0 / 44100.0;

  _session_randomize_ssrc( session );
  
  session->info.peer         = NULL;
  session->info.padding      = 0;
  session->info.extension    = 0;
  session->info.csrc_count   = 0;
  session->info.marker       = 0;
  session->info.payload_type = 0;
  session->info.ssrc         = session->self.ssrc;
  session->info.payload_size = 0;
  session->info.payload      = NULL;

  return session;
}

/**
 * @brief Destroy an RTPSession instance.
 * Free all resources occupied by the session.
 * @public @memberof RTPSession
 * @param session The session.
 */
void RTPSessionDestroy( struct RTPSession * session ) {
  int i;
  for( i=0; i<RTP_MAX_PEERS; i++ ) {
    if( session->peers[i] != NULL ) {
      RTPPeerRelease( session->peers[i] );
    }
  }
  free( session->buffer );
  free( session );
}

/**
 * @brief Retain an RTPSession instance.
 * Increment the reference counter of a session so that it won't be destroyed.
 * @public @memberof RTPSession
 * @param session The session.
 */
void RTPSessionRetain( struct RTPSession * session ) {
  session->refs++;
}

/**
 * @brief Release an RTPSession instance.
 * Decrement the reference counter of a session. If the reference count
 * reached zero, destroy the session.
 * @public @memberof RTPSession
 * @param session The session.
 */
void RTPSessionRelease( struct RTPSession * session ) {
  if( ! --session->refs ) {
    RTPSessionDestroy( session );
  }
}

/**
 * @brief Set the SSRC used by this session.
 * @public @memberof RTPSession
 * @param session The session.
 * @param ssrc The synchronization source.
 * @retval 0 on success.
 */
int RTPSessionSetSSRC( struct RTPSession * session, unsigned long ssrc ) {
  session->self.ssrc = ssrc;
  return 0;
}

/**
 * @brief Get the SSRC used by this session.
 * @public @memberof RTPSession
 * @param session The session.
 * @param ssrc The synchronization source.
 * @retval 0 on success.
 */
int RTPSessionGetSSRC( struct RTPSession * session, unsigned long * ssrc ) {
  if( ssrc == NULL ) return 1;
  *ssrc = session->self.ssrc;
  return 0;
}

/**
 * @brief Set the timestamp offset used by this session.
 * @public @memberof RTPSession
 * @param session The session.
 * @param offset The timestamp offset measured in RTP ticks.
 * @retval 0 on success.
 */
int RTPSessionSetTimestampOffset( struct RTPSession * session, unsigned long offset ) {
  session->timestamp_offset = offset;
  return 0;
}

/**
 * @brief Get the timestamp offset used by this session.
 * @public @memberof RTPSession
 * @param session The session.
 * @param offset The timestamp offset measured in RTP ticks.
 * @retval 0 on success.
 */
int RTPSessionGetTimestampOffset( struct RTPSession * session, unsigned long * offset ) {
  if( offset == NULL ) return 1;
  *offset = session->timestamp_offset;
  return 0;
}

/**
 * @brief Set the timestamp rate used by this session.
 * @public @memberof RTPSession
 * @param session The session.
 * @param rate The timestamp rate (timestamp ticks per second)
 * @retval 0 on success.
 */
int RTPSessionSetTimestampRate( struct RTPSession * session, double rate ) {
  session->timestamp_divisor = 1000000.0 / rate;
  return 0;
}

/**
 * @brief Get the timestamp rate used by this session.
 * @public @memberof RTPSession
 * @param session The session.
 * @param rate The timestamp rate (timestamp ticks per second)
 * @retval 0 on success.
 */
int RTPSessionGetTimestampRate( struct RTPSession * session, double * rate ) {
  if( rate == NULL ) return 1;
  *rate = 1000000.0 / session->timestamp_divisor;
  return 0;
}

/**
 * @brief Set the socket used to communicate with other clients.
 * @public @memberof RTPSession
 * @param session The session.
 * @param socket The socket.
 * @retval 0 on success.
 */
int RTPSessionSetSocket( struct RTPSession * session, int socket ) {
  if( socket == session->socket ) return 0;
  session->socket = socket;
  return 0;
}

/**
 * @brief Get the socket used to communicate with other clients.
 * @public @memberof RTPSession
 * @param session The session.
 * @param socket The socket.
 * @retval 0 on success.
 */
int RTPSessionGetSocket( struct RTPSession * session, int * socket ) {
  if( socket == NULL ) return 1;
  *socket = session->socket;
  return 0;
}

/**
 * @brief Get the current RTP timestamp.
 * @public @memberof RTPSession
 * @param session The session.
 * @param timestamp The timestamp in RTP ticks.
 * @retval 0 on success.
 */
int RTPSessionGetTimestamp( struct RTPSession * session, unsigned long long * timestamp ) {
  *timestamp = _session_get_timestamp( session );
  return 0;
}

/**
 * @brief Add an RTPPeer to the session.
 * Lookup the peer using the (pseudo) hash-table, add it to the list and retain it.
 * The peer will be included when data is sent via RTPSessionSendPayload.
 * @public @memberof RTPSession
 * @param session The session.
 * @param peer The peer to add.
 * @retval 0 on success.
 * @retval >0 if the peer could not be added.
 */
int RTPSessionAddPeer( struct RTPSession * session, struct RTPPeer * peer ) {
  int i, off, p;
  off = peer->address.ssrc % RTP_MAX_PEERS;
  for( i=0; i < RTP_MAX_PEERS; i++ ) {
    p = (i+off)%RTP_MAX_PEERS;
    if( session->peers[p] == NULL ) {
      session->peers[p] = peer;
      RTPPeerRetain( peer );
      return 0;
    }
  }
  return 1;
}

/**
 * @brief Remove an RTPPeer from the session.
 * Lookup the peer using the (pseudo) hash-table, remove it from the list and release it.
 * @public @memberof RTPSession
 * @param session The session.
 * @param peer The peer to remove.
 * @retval 0 on success.
 * @retval >0 if the peer could not be removed.
 */
int RTPSessionRemovePeer( struct RTPSession * session, struct RTPPeer * peer ) {
  int i, off, p;
  off = peer->address.ssrc % RTP_MAX_PEERS;
  for( i=0; i < RTP_MAX_PEERS; i++ ) {
    p = (i+off)%RTP_MAX_PEERS;
    if( session->peers[p] == peer ) {
      session->peers[p] = NULL;
      RTPPeerRelease( peer );
      return 0;
    }
  }
  return 1;
}

/**
 * @brief Advance the pointer to the next peer.
 * Given a @c NULL pointer the first peer will be returned. When the
 * last peer was reached a @c NULL pointer will be returned.
 * @public @memberof RTPSession
 * @param session The session.
 * @param peer The peer.
 * @retval 0 on success.
 * @retval >0 if the given peer does not exist.
 */
int RTPSessionNextPeer( struct RTPSession * session, struct RTPPeer ** peer ) {
  int i;
  if( peer == NULL ) return 1;
  if( *peer == NULL ) {
    i=-1;
  } else {
    for( i=0; i < RTP_MAX_PEERS; i++ ) {
      if( session->peers[i] == *peer ) {
        break;
      }
    }
    if( i == RTP_MAX_PEERS ) return 1;
  }

  do {
     i++;
  } while( i < RTP_MAX_PEERS && session->peers[i] == NULL );
  if( i == RTP_MAX_PEERS ) {
    *peer = NULL;
  } else {
    *peer = session->peers[i];
  }
  return 0;
}

/**
 * Retrieve peer information by looking up the given SSRC identifier.
 * @public @memberof RTPSession
 * @param session The session.
 * @param peer The peer.
 * @param ssrc The SSRC.
 * @retval 0 on success.
 * @retval >0 if no peer with the given ssrc was found.
 */
int RTPSessionFindPeerBySSRC( struct RTPSession * session, struct RTPPeer ** peer,
                              unsigned long ssrc ) {
  int i, off, p;
  off = ssrc % RTP_MAX_PEERS;
  for( i=0; i < RTP_MAX_PEERS; i++ ) {
    p = (i+off)%RTP_MAX_PEERS;
    if( session->peers[p] != NULL ) {
      if( session->peers[p]->address.ssrc == ssrc ) {
        *peer = session->peers[p];
        return 0;
      }
    }
  }
  return 1;
}

/**
 * Retrieve peer information by looking up the given host address.
 * @public @memberof RTPSession
 * @param session The session.
 * @param peer The peer.
 * @param size The length of the structure pointed to by @c addr.
 * @param addr Any sockaddr structure.
 * @retval 0 on success.
 * @retval >0 if no peer with the given address was found.
 */
int RTPSessionFindPeerByAddress( struct RTPSession * session, struct RTPPeer ** peer,
                                 socklen_t size, struct sockaddr * addr ) {
  int i;
  socklen_t s;
  struct sockaddr * a;
  for( i=0; i < RTP_MAX_PEERS; i++ ) {
    if( session->peers[i] != NULL ) {
      RTPPeerGetAddress( session->peers[i], &s, &a );
      if( s == size && memcmp( a, addr, s ) == 0 ) {
        *peer = session->peers[i];
        return 0;
      }
    }
  }
  return 1;
}

static size_t _info_payload_size( struct RTPPacketInfo * info ) {
  size_t i, size = 0;
  for( i=0; i<info->iov_len; i++ ) {
    size += info->iov[i].iov_len;
  }
  return size;
}

/**
 * Encode an RTP packet for transmission over network.
 * Read all RTP information (timestamp, ssrc, payload-size, etc.) from an info structure
 * and encode it to an RTP packet.
 * @relates RTPSession
 * @param info The packet information structure to fill.
 * @param size The size of buffer that should hold the packet.
 * @param data The buffer to hold the encoded packet.
 */
static int RTPEncodePacket( struct RTPPacketInfo * info, size_t size, void * data ) {
  int i, j;
  unsigned char * buffer = data;
  size_t header_size     = 12 + ( info->csrc_count * 4 );
  size_t ext_header_size = header_size + ( info->extension ? 4 : 0 );
  size_t payload_size    = _info_payload_size( info );
  size_t total_size      = ext_header_size + info->payload_size + info->padding;
  info->total_size = total_size;

  if( total_size > size ) return 1;
  
  buffer[0] = 0x80
            | ( info->padding ? 0x20 : 0 )
            | ( info->extension ? 0x10 : 0 )
            | ( info->csrc_count & 0x0f );
  
  buffer[1] = ( info->payload_type & 0x7f )
            | ( info->marker ? 0x80 : 0x00 );
  
  buffer[2] =   info->sequence_number        & 0xff;
  buffer[3] = ( info->sequence_number >> 8 ) & 0xff;

  buffer[4] =   info->timestamp         & 0xff;
  buffer[5] = ( info->timestamp >> 8 )  & 0xff;
  buffer[6] = ( info->timestamp >> 16 ) & 0xff;
  buffer[7] = ( info->timestamp >> 24 ) & 0xff;

  buffer[8]  =   info->ssrc         & 0xff;
  buffer[9]  = ( info->ssrc >> 8 )  & 0xff;
  buffer[10] = ( info->ssrc >> 16 ) & 0xff;
  buffer[11] = ( info->ssrc >> 24 ) & 0xff;

  for( i=0, j=0; i<info->csrc_count; i++, j+=4 ) {
    buffer[12+j] =   info->csrc[i]         & 0xff;
    buffer[13+j] = ( info->csrc[i] >> 8 )  & 0xff;
    buffer[14+j] = ( info->csrc[i] >> 16 ) & 0xff;
    buffer[15+j] = ( info->csrc[i] >> 24 ) & 0xff;
  }
  
  if( info->extension ) {
    buffer[header_size]   = 0;
    buffer[header_size+1] = 0;
    buffer[header_size+2] = 0;
    buffer[header_size+3] = 0;
  }
  
  if( info->payload_size > 0 && info->payload != NULL ) {
    memcpy( data+ext_header_size, info->payload, info->payload_size );
  /*padding should be ignored*/
  /*memset( data+data_size, 0, info->padding-1 );*/
    if( info->padding ) {
      buffer[total_size-1] = info->padding;
    }
  }
  return 0;
}

/**
 * Decode an RTP packet received on a socket.
 * Store all RTP information (timestamp, ssrc, payload-size, etc.) inside the info structure
 * and set the info-structures data pointer to the location of the payload.
 * @relates RTPSession
 * @param info The packet information structure to fill.
 * @param size The size of the received packet in bytes.
 * @param data The buffer holding the encoded packet.
 */
static int RTPDecodePacket( struct RTPPacketInfo * info, size_t size, void * data ) {
  int i, j;
  unsigned char * buffer = data;
  size_t header_size;
  size_t ext_header_size;
  size_t total_size = size;

  if( ( buffer[0] & 0xc0 ) != 0x80 ) {
    return 1; /* wrong rtp version */
  }
  info->padding         = ( buffer[0] & 0x20 ) ? buffer[total_size-1] : 0;
  info->extension       = ( buffer[0] & 0x10 ) ? 1 : 0;
  info->csrc_count      =   buffer[0] & 0x0f;

  info->marker          = ( buffer[1] & 0x80 ) ? 1 : 0;
  info->payload_type    =   buffer[1] & 0x7f;

  info->sequence_number =   buffer[2]
                        | ( buffer[3] << 8 );

  info->timestamp       =   buffer[4]
                        | ( buffer[5] << 8 )
                        | ( buffer[6] << 16 )
                        | ( buffer[7] << 24 );

  info->ssrc            =   buffer[8]
                        | ( buffer[9] << 8 )
                        | ( buffer[10] << 16 )
                        | ( buffer[11] << 24 );

  header_size = 12 + ( info->csrc_count * 4 );

  for( i=0, j=0; i<info->csrc_count; i++, j+=4 ) {
    info->csrc[i] =   buffer[12+j]
                  | ( buffer[13+j] << 8 )
                  | ( buffer[14+j] << 16 )
                  | ( buffer[15+j] << 24 );
  }

  if( info->extension ) {
       /* buffer[header_size];   */
       /* buffer[header_size+1]; */
    j =   buffer[header_size+2]
      | ( buffer[header_size+3] << 8 );
    ext_header_size = header_size + 4 + (j*4);
  } else {
    ext_header_size = header_size;
  }

  info->total_size   = total_size;
  info->payload_size = total_size - ext_header_size - info->padding;

  if( info->payload_size == 0 ) {
    info->payload = NULL;
  } else {
    if( info->payload != NULL ) {
      memcpy( info->payload, data+ext_header_size, info->payload_size );
    } else {
      info->payload = data+ext_header_size;
    }
  }
  return 0;
}

/**
 * @brief Send an RTP packet.
 * @public @memberof RTPSession
 * @param session The session.
 * @param info The packet info.
 * @retval 0 On success.
 * @retval >0 If the message could not be sent.
 */
int RTPSessionSendPacket( struct RTPSession * session, struct RTPPacketInfo * info ) {
  ssize_t bytes_sent;
  struct iovec  msg_iov[RTP_IOV_LEN+1];
  struct msghdr msg;

  if( info->peer == NULL ) return 1;

  info->ssrc            = session->self.ssrc;
  info->sequence_number = info->peer->out_seqnum + 1;
  info->timestamp       = _session_get_timestamp( session ) & 0xffffffff;

  RTPEncodePacket( info, session->buflen, session->buffer );

  msg_iov[0].iov_base = session->buffer;
  msg_iov[0].iov_len  = info->total_size;

  msg.msg_name       = &(info->peer->address.addr);
  msg.msg_namelen    = info->peer->address.size;
  msg.msg_iov        = &(msg_iov[0]);
  msg.msg_iovlen     = 1;
  msg.msg_control    = NULL;
  msg.msg_controllen = 0;
  msg.msg_flags      = 0;

  bytes_sent = sendmsg( session->socket, &msg, 0 );
  
  if( bytes_sent != info->total_size ) {
    return bytes_sent;
  } else if( msg.msg_flags != 0 ) {
    return 1;
  } else {
    info->peer->out_seqnum    = info->sequence_number;
    info->peer->out_timestamp = info->timestamp;
    return 0;
  }
}

/**
 * @brief Receive an RTP packet.
 * @public @memberof RTPSession
 * @param session The session.
 * @param info The packet info.
 * @retval 0 On success.
 * @retval >0 If the message could not be received.
 */
int RTPSessionReceivePacket( struct RTPSession * session, struct RTPPacketInfo * info ) {
  ssize_t bytes_received;
  struct sockaddr_storage msg_name;
  struct iovec  msg_iov[RTP_IOV_LEN+1];
  struct msghdr msg;

  msg_iov[0].iov_base = session->buffer;
  msg_iov[0].iov_len  = session->buflen;

  msg.msg_name       = &msg_name;
  msg.msg_namelen    = sizeof(msg_name);
  msg.msg_iov        = &(msg_iov[0]);
  msg.msg_iovlen     = 1;
  msg.msg_control    = NULL;
  msg.msg_controllen = 0;
  msg.msg_flags      = 0;

  bytes_received = recvmsg( session->socket, &msg, 0 );

  if( bytes_received == -1 ) return -1;
  if( msg.msg_flags != 0  )  return 1;
  if( bytes_received < 12 )  return 1;

  RTPDecodePacket( info, bytes_received, session->buffer );

  info->peer = NULL;
  RTPSessionFindPeerBySSRC( session, &(info->peer), info->ssrc );
  if( info->peer == NULL ) {
    info->peer = RTPPeerCreate( info->ssrc, msg.msg_namelen, msg.msg_name );
    RTPSessionAddPeer( session, info->peer );
    RTPPeerRelease( info->peer );
  }
  if( info->sequence_number == info->peer->in_seqnum + 1 ) {
    info->peer->in_seqnum    = info->sequence_number;
    info->peer->in_timestamp = info->timestamp;
  }
  return 0;
}

/**
 * @brief Send an RTP packet.
 * @public @memberof RTPSession
 * @param session The session.
 * @param info The packet info.
 * @retval 0 On success.
 * @retval >0 If the message could not be sent.
 */
int RTPSessionSend( struct RTPSession * session, size_t size, void * payload,
                    struct RTPPacketInfo * info ) {
  int i, result = 0;
  if( info == NULL ) {
    info = &(session->info);
  }
  info->payload_size  = size;
  info->payload       = payload;
  info->iov_len = 1;
  info->iov[0].iov_base = payload;
  info->iov[0].iov_len  = size;

  if( info->peer == NULL ) {
    for( i=0; i<RTP_MAX_PEERS; i++ ) {
      if( session->peers[i] != NULL ) {
        info->peer = session->peers[i];
        result += RTPSessionSendPacket( session, info );
      }
    }
  } else {
    result = RTPSessionSendPacket( session, info );
  }
  return result;
}

/**
 * @brief Receive an RTP packet.
 * @public @memberof RTPSession
 * @param session The session.
 * @param info The packet info.
 * @retval 0 On success.
 * @retval >0 If the message could not be received.
 */
int RTPSessionReceive( struct RTPSession * session, size_t size, void * payload,
                       struct RTPPacketInfo * info ) {
  int result;
  if( info == NULL ) {
    info = &(session->info);
  }
  info->payload_size = 0;
  info->payload      = NULL;

  result = RTPSessionReceivePacket( session, info );
  if( size >= info->payload_size && payload != NULL ) {
    memcpy( payload, info->payload, info->payload_size );
    info->payload = payload;
  }
  return result;
}
