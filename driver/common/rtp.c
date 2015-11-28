#include "rtp.h"
#include <string.h>
#include <unistd.h>
#include <sys/time.h>
#include <arpa/inet.h>

#ifndef NO_LOG
#include "midi/midi.h"
#endif

#define RTP_MAX_PEERS 16
#define RTP_BUF_LEN   512
#define RTP_IOV_LEN   16

#define USEC_PER_SEC 1000000

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
  
  struct iovec iov[RTP_IOV_LEN];
  size_t buflen;
  void * buffer;
};

/**
 * @defgroup RTP RTP
 * @{
 */

/**
 * @struct RTPPacketInfo rtp.h
 * @ingroup RTP
 * @brief RTP packet information.
 * Hold information, data for one RTP packet. The RTP header has the
 * following format:
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
 *
 * @par Version (V): 2 bits.
 * The version field identifies the version of RTP. The version we use
 * and the one specified by RFC3550 is two (2). (The value 1 is used by
 * the first draft version of RTP and the value 0 is used by the
 * protocol initially implemented in the "vat" audio tool.)
 *
 * @par Padding (P): 1 bit.
 * @copydetails RTPPacketInfo::padding
 * (See @ref RTPPacketInfo::padding)
 *
 * @par Extension (X): 1 bit.
 * @copydetails RTPPacketInfo::extension
 * (See @ref RTPPacketInfo::extension)
 *
 * @par CSRC count (CC): 4 bits.
 * @copydetails RTPPacketInfo::csrc_count
 * (See @ref RTPPacketInfo::csrc_count)
 *
 * @par Marker (M): 1 bit.
 * @copydetails RTPPacketInfo::marker
 * (See @ref RTPPacketInfo::marker)
 *
 * @par Payload type (PT): 7 bits.
 * @copydetails RTPPacketInfo::payload_type
 * (See @ref RTPPacketInfo::payload_type)
 *
 * @par Sequence number: 16 bits.
 * @copydetails RTPPacketInfo::sequence_number
 * (See @ref RTPPacketInfo::sequence_number)
 *
 * @par Timestamp: 32 bits.
 * @copydetails RTPPacketInfo::timestamp
 * (See @ref RTPPacketInfo::timestamp)
 *
 * @par SSRC: 32 bits.
 * @copydetails RTPPacketInfo::ssrc
 * (See @ref RTPPacketInfo::ssrc)
 *
 * @par CSRC list: 0 to 15 items, 32 bits each.
 * @copydetails RTPPacketInfo::csrc
 * (See @ref RTPPacketInfo::csrc)
 *
 * RTP header extension.
 *
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
 *
 * Defined by profile: 16 bits.
 *
 * To allow multiple interoperating implementations to each experiment
 * independently with different header extensions, or to allow a
 * particular implementation to experiment with more than one type of
 * header extension, the first 16 bits of the header extension are left
 * open for distinguishing identifiers or parameters.  The format of
 * these 16 bits is to be defined by the profile specification under
 * which the implementations are operating.  This RTP specification does
 * not define any header extensions itself.
 *
 * @par Length: 16 bits.
 * The length field counts the number of 32-bit words in the extension,
 * excluding the four-octet extension header (therefore zero is a valid
 * length).
 *
 * @par Header extension.
 * The header extension may contain arbitary data defined by one or more
 * profiles.
 */

/**
 * @struct RTPAddress
 * An RTP address consisting of an internet-address (socket with length)
 * and a synchronization source identifier.
 */

/**
 * @struct RTPPeer
 * @brief A control structure to manage the connection to an RTP peer.
 */

/**
 * @struct RTPSession
 * @brief An RTP session that may be connected to multiple peers.
 */

/** @} */

/**
 * @property RTPPacketInfo::peer
 * @brief The peer associated with the packet.
 * This can either be the peer that originally sent the packet we did
 * receive or it can be the peer that the packet should be sent to.
 */
/**
 * @property RTPPacketInfo::padding
 * @brief The number of padding bytes following the payload.
 * If the padding bit is set, the packet contains one or more additional
 * padding octets at the end which are not part of the payload. The last
 * octet of the padding contains a count of how many padding octets
 * should be ignored, including itself.  Padding may be needed by some
 * encryption algorithms with fixed block sizes or for carrying several
 * RTP packets in a lower-layer protocol data unit.
 */
/**
 * @property RTPPacketInfo::extension
 * @brief Set if the first @c iovec element is used as a header extension.
 * If the extension bit is set, the fixed header MUST be followed by
 * exactly one header extension.
 */
/**
 * @property RTPPacketInfo::csrc_count
 * The CSRC count contains the number of CSRC identifiers that follow
 * the fixed header.
 */
/**
 * @property RTPPacketInfo::marker
 * The interpretation of the marker is defined by a profile.  It is
 * intended to allow significant events such as frame boundaries to be
 * marked in the packet stream. A profile MAY define additional marker
 * bits or specify that there is no marker bit by changing the number
 * of bits in the payload type field.
 */
/**
 * @property RTPPacketInfo::payload_type
 * This field identifies the format of the RTP payload and determines
 * its interpretation by the application.  A profile MAY specify a
 * default static mapping of payload type codes to payload formats.
 */
/**
 * @property RTPPacketInfo::sequence_number
 * The sequence number increments by one for each RTP data packet sent,
 * and may be used by the receiver to detect packet loss and to restore
 * packet sequence. The initial value of the sequence number SHOULD be
 * random (unpredictable) to make known-plaintext attacks on encryption
 * more difficult, even if the source itself does not encrypt, because
 * the packets may flow through a translator that does.
 */
/**
 * @property RTPPacketInfo::timestamp
 * The timestamp reflects the sampling instant of the first octet in
 * the RTP data packet. The sampling instant MUST be derived from a
 * clock that increments monotonically and linearly in time to allow
 * synchronization and jitter calculations.
 */
/**
 * @property RTPPacketInfo::ssrc
 * The SSRC field identifies the synchronization source.  This
 * identifier SHOULD be chosen randomly, with the intent that no two
 * synchronization sources within the same RTP session will have the
 * same SSRC identifier.
 */
/**
 * @property RTPPacketInfo::csrc
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
 * @property RTPPacketInfo::total_size
 * @brief The total size of the sent or received RTP packet.
 * The total size includes all RTP-Header informatio and the padding.
 * It only acts as an information for the payload implementation. It
 * is never read or interpreted by the RTP session.
 */
/**
 * @property RTPPacketInfo::payload_size
 * @brief The total payload size including all @c iovec elements.
 */
/**
 * @property RTPPacketInfo::iovlen
 * @brief The number of elements in the @c iovec structure.
 */
/**
 * @property RTPPacketInfo::iov
 * @brief A number of iovec elements that belong to the packet.
 */

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
  if( peer == NULL ) return NULL;
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

/* MARK: Creation and destruction *//**
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

static void _session_randomize_ssrc( struct RTPSession * session ) {
  static struct timeval tv = { 0, 0 };
  unsigned long seed;
  if( tv.tv_sec == 0 ) {
    gettimeofday( &tv, NULL );
    seed = tv.tv_sec * 1000000 + tv.tv_usec;
    srandom( seed );
  }
  session->self.ssrc = random();
}

/**
 * @brief Create an RTPSession instance.
 * Allocate space and initialize an RTPSession instance.
 * @public @memberof RTPSession
 * @param socket The socket to use for communication.
 * @return a pointer to the created controller structure on success.
 * @return a @c NULL pointer if the controller could not created.
 */
struct RTPSession * RTPSessionCreate( int socket ) {
  int i;
  struct RTPSession * session = malloc( sizeof( struct RTPSession ) );
  if( session == NULL ) return NULL;

  session->refs   = 1;
  session->socket = socket;

  _init_addr_with_socket( &(session->self), socket );
  for( i=0; i<RTP_MAX_PEERS; i++ ) {
    session->peers[i] = NULL;
  }
  
  
  session->buflen = RTP_BUF_LEN;
  session->buffer = malloc( session->buflen );
  if( session->buffer == NULL ) {
    session->buflen = 0;
  }
  session->iov[0].iov_base = session->buffer;
  session->iov[9].iov_len  = session->buflen;
  for( i=1; i<RTP_IOV_LEN; i++ ) {
    session->iov[i].iov_base = NULL;
    session->iov[i].iov_len  = 0;
  }

  _session_randomize_ssrc( session );
  
  session->info.peer         = NULL;
  session->info.padding      = 0;
  session->info.extension    = 0;
  session->info.csrc_count   = 0;
  session->info.marker       = 0;
  session->info.payload_type = 0;
  session->info.ssrc         = session->self.ssrc;
  session->info.iovlen       = RTP_IOV_LEN;
  session->info.iov          = &(session->iov[0]);

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
  close( session->socket );
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

/** @} */

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

static int _rtp_encode_header( struct RTPPacketInfo * info, size_t size, void * data, size_t * written ) {
  int i, j;
  unsigned char * buffer = data;
  size_t header_size = 12 + ( info->csrc_count * 4 );

  if( size < header_size ) return 1;
    
  buffer[0] = 0x80
            | ( info->padding ? 0x20 : 0 )
            | ( info->extension ? 0x10 : 0 )
            | ( info->csrc_count & 0x0f );
  
  buffer[1] = ( info->payload_type & 0x7f )
            | ( info->marker ? 0x80 : 0x00 );
  
  buffer[2] = ( info->sequence_number >> 8 ) & 0xff;
  buffer[3] =   info->sequence_number        & 0xff;

  buffer[4] = ( info->timestamp >> 24 ) & 0xff;
  buffer[5] = ( info->timestamp >> 16 ) & 0xff;
  buffer[6] = ( info->timestamp >> 8 )  & 0xff;
  buffer[7] =   info->timestamp         & 0xff;

  buffer[8]  = ( info->ssrc >> 24 ) & 0xff;
  buffer[9]  = ( info->ssrc >> 16 ) & 0xff;
  buffer[10] = ( info->ssrc >> 8 )  & 0xff;
  buffer[11] =   info->ssrc         & 0xff;

  for( i=0, j=0; i<info->csrc_count; i++, j+=4 ) {
    buffer[12+j] = ( info->csrc[i] >> 24 ) & 0xff;
    buffer[13+j] = ( info->csrc[i] >> 16 ) & 0xff;
    buffer[14+j] = ( info->csrc[i] >> 8 )  & 0xff;
    buffer[15+j] =   info->csrc[i]         & 0xff;
  }
  
  *written = header_size;
  return 0;
}

static int _rtp_decode_header( struct RTPPacketInfo * info, size_t size, void * data, size_t * read ) {
  int i, j;
  unsigned char * buffer = data;
  size_t header_size;

  if( ( buffer[0] & 0xc0 ) != 0x80 ) {
    return 1; /* wrong rtp version */
  }
  info->padding         = ( buffer[0] & 0x20 ) ? buffer[size-1] : 0;
  info->extension       = ( buffer[0] & 0x10 ) ? 1 : 0;
  info->csrc_count      =   buffer[0] & 0x0f;

  info->marker          = ( buffer[1] & 0x80 ) ? 1 : 0;
  info->payload_type    =   buffer[1] & 0x7f;

  info->sequence_number = ( buffer[2] << 8 )
                        |   buffer[3];

  info->timestamp       = ( buffer[4] << 24 )
                        | ( buffer[5] << 16 )
                        | ( buffer[6] << 8 )
                        |   buffer[7];

  info->ssrc            = ( buffer[8] << 24 )
                        | ( buffer[9] << 16 )
                        | ( buffer[10] << 8 )
                        |   buffer[11];

  header_size = 12 + ( info->csrc_count * 4 );
  
  if( size < header_size ) return 1;

  for( i=0, j=0; i<info->csrc_count; i++, j+=4 ) {
    info->csrc[i] = ( buffer[12+j] << 24 )
                  | ( buffer[13+j] << 16 )
                  | ( buffer[14+j] << 8 )
                  |   buffer[15+j];
  }

  *read = header_size;
  return 0;
}

static int _rtp_encode_extension( struct RTPPacketInfo * info, size_t size, void * data, size_t * written ) {
  int i;
  unsigned char * buffer = data;
  size_t ext_header_size;
  
  if( info->extension ) {
    if( info->iovlen < 1 ) return 1;
    ext_header_size = info->iov[0].iov_len;
    if( ext_header_size % 4 || ext_header_size == 0 ) {
      /* fill up to whole 4 bytes words */
      ext_header_size += 4 - (info->iov[0].iov_len % 4);
    }

    memcpy( buffer, info->iov[0].iov_base, info->iov[0].iov_len );
    i = ext_header_size / 4;
    buffer[2] = ( i >> 8 ) & 0xff;
    buffer[3] =   i        & 0xff;
  } else {
    ext_header_size = 0;
  }
  
  *written = ext_header_size;
  return 0;
}

static int _rtp_decode_extension( struct RTPPacketInfo * info, size_t size, void * data, size_t * read ) {
  int i;
  unsigned char * buffer = data;
  size_t ext_header_size;

  if( info->extension ) {
    if( info->iovlen < 1 ) return 1;
    i = ( buffer[2] << 8 )
      |   buffer[3];
    ext_header_size = 4 + (i*4);
    info->iov[0].iov_base = buffer;
    info->iov[0].iov_len  = ext_header_size;
  } else {
    ext_header_size = 0;
  }

  *read = ext_header_size;
  return 0;
}

static int _rtp_encode_padding( struct RTPPacketInfo * info, size_t size, void * data, size_t * written ) {
  unsigned char * buffer = data;
  if( info->padding ) {
    if( size < info->padding ) return 1;
    buffer[info->padding-1] = info->padding;
  }
  *written = info->padding;
  return 0;
}

static void _advance_buffer( size_t * size, void ** buffer, size_t bytes ) {
  *size   -= bytes;
  *buffer += bytes;
}

static void _append_iov( size_t * iovlen, struct iovec * iov, size_t size, void * buffer ) {
  iov[*iovlen].iov_len  = size;
  iov[*iovlen].iov_base = buffer;
  *iovlen += 1;
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
  size_t size, written = 0, iovlen = 0;
  void * buffer;
  struct msghdr msg;
  struct iovec  iov[RTP_IOV_LEN+3];
  ssize_t bytes_sent;
  
  if( info == NULL || info->peer == NULL ) return 1;
  if( info->iovlen > RTP_IOV_LEN ) return 1;
  
  size   = session->buflen;
  buffer = session->buffer;

  info->ssrc            = session->self.ssrc;
  info->sequence_number = info->peer->out_seqnum + 1;

  info->total_size = 0;
  _rtp_encode_header( info, size, buffer, &written );
  _append_iov( &iovlen, &(iov[0]), written, buffer );
  _advance_buffer( &size, &buffer, written );
  info->total_size += written;
  if( info->extension ) {
    _rtp_encode_extension( info, size, &buffer, &written );
    _append_iov( &iovlen, &(iov[0]), written, buffer );
    _advance_buffer( &size, &buffer, written );
    info->total_size += written;
  }
  info->payload_size = 0;
  while( (iovlen-1)<info->iovlen ) {
    info->payload_size += info->iov[iovlen-1].iov_len;
    _append_iov( &iovlen, &(iov[0]), info->iov[iovlen-1].iov_len, info->iov[iovlen-1].iov_base );
  }
  info->total_size += info->payload_size;
  if( info->padding ) {
    _rtp_encode_padding( info, size, &buffer, &written );
    _append_iov( &iovlen, &(iov[0]), written, buffer );
    _advance_buffer( &size, &buffer, written );
    info->total_size += written;
  }

#ifndef NO_LOG
  MIDILogLocation( DEBUG, "Sending RTP message consisting of %i iovecs.\n", (int) iovlen );
  int i, j;
  for( i=0; i<iovlen; i++ ) {
    MIDILog( DEBUG, "[%i] iov_len: %i, iov_base: %p\n", i, (int) iov[i].iov_len, iov[i].iov_base );
    for( j=0; j<iov[i].iov_len; j++ ) {
      unsigned char c = *((unsigned char*)iov[i].iov_base+j);
      if( (j+1) % 8 == 0 || j+1 == iov[i].iov_len ) {
        MIDILog( DEBUG, "0x%02x\n", c );
      } else {
        MIDILog( DEBUG, "0x%02x ", c );
      }
    }
  }
#endif

  msg.msg_name       = &(info->peer->address.addr);
  msg.msg_namelen    = info->peer->address.size;
  msg.msg_iov        = &(iov[0]);
  msg.msg_iovlen     = iovlen;
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
  size_t size, read = 0;
  void * buffer;
  struct sockaddr_storage name;
  struct msghdr msg;
  struct iovec  iov;
  ssize_t bytes_received;

  iov.iov_base = session->buffer;
  iov.iov_len  = session->buflen;

  msg.msg_name       = &name;
  msg.msg_namelen    = sizeof(name);
  msg.msg_iov        = &iov;
  msg.msg_iovlen     = 1;
  msg.msg_control    = NULL;
  msg.msg_controllen = 0;
  msg.msg_flags      = 0;

  bytes_received = recvmsg( session->socket, &msg, 0 );

  if( bytes_received == -1 ) return -1;
  if( msg.msg_flags != 0  )  return 1;
  if( bytes_received < 12 )  return 1;

  size   = bytes_received;
  buffer = session->buffer;
  info->total_size = bytes_received;
  _rtp_decode_header( info, size, buffer, &read );
  _advance_buffer( &size, &buffer, read );
  if( info->extension ) {
    _rtp_decode_extension( info, size, buffer, &read );
    _advance_buffer( &size, &buffer, read );
  }
  info->payload_size = size - info->padding;
  if( info->extension ) {
    info->iovlen = 2;
    info->iov[1].iov_base = buffer;
    info->iov[1].iov_len  = info->payload_size;
  } else {
    info->iovlen = 1;
    info->iov[0].iov_base = buffer;
    info->iov[0].iov_len  = info->payload_size;
  }
  
#ifndef NO_LOG
  int i, j;
  MIDILogLocation( DEBUG, "Received RTP message consisting of %i iovecs.\n", (int) info->iovlen );
  for( i=0; i<info->iovlen; i++ ) {
    MIDILog( DEBUG, "[%i] iov_len: %i, iov_base: %p\n", i, (int) info->iov[i].iov_len, info->iov[i].iov_base );
    for( j=0; j<info->iov[i].iov_len; j++ ) {
      unsigned char c = *((unsigned char*)info->iov[i].iov_base+j);
      if( (j+1) % 8 == 0 || j+1 == info->iov[i].iov_len ) {
        MIDILog( DEBUG, "0x%02x\n", c );
      } else {
        MIDILog( DEBUG, "0x%02x ", c );
      }
    }
  }
#endif

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
 * @param size    The number of bytes in the @c payload buffer.
 * @param payload The buffer to read the payload from.
 * @param info    The packet info.
 * @retval 0 On success.
 * @retval >0 If the message could not be sent.
 */
int RTPSessionSend( struct RTPSession * session, size_t size, void * payload,
                    struct RTPPacketInfo * info ) {
  int i, result = 0;
  struct iovec iov;
  
  if( info == NULL ) {
    info = &(session->info);
  }

  iov.iov_base    = payload;
  iov.iov_len     = size;
  info->extension = 0;
  info->iovlen    = 1;
  info->iov       = &iov;
  
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
 * @param size    The number of available bytes in the @c payload buffer.
 * @param payload The buffer to store the payload in.
 * @param info    The packet info.
 * @retval 0 On success.
 * @retval >0 If the message could not be received.
 */
int RTPSessionReceive( struct RTPSession * session, size_t size, void * payload,
                       struct RTPPacketInfo * info ) {
  int result;
  struct iovec iov;
  
  if( info == NULL ) {
    info = &(session->info);
  }
  
  info->iovlen = 1;
  info->iov    = &iov;

  result = RTPSessionReceivePacket( session, info );
  if( size >= info->iov[0].iov_len && payload != NULL ) {
    memcpy( payload, info->iov[0].iov_base, info->iov[0].iov_len );
    info->iov[0].iov_base = payload;
  }
  return result;
}
