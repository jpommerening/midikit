#include "rtp.h"
#include <stdlib.h>
#include <string.h>
#include <sys/time.h>

#define SOCKADDR_BUFLEN 32
#define USEC_PER_SEC 1000000.0

struct RTPHeader {
  unsigned version         : 2; // automatic
  unsigned padding         : 1; // packet spec.
  unsigned extension       : 1; // packet spec.
  unsigned csrc_count      : 4; // packet spec.
  unsigned marker          : 1; // packet spec.
  unsigned payload_type    : 7; // packet spec.
  unsigned sequence_number : 16; // automatic
  unsigned long timestamp; // automatic
  unsigned long ssrc; // automatic
  unsigned long * csrc_list; // packet spec.
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

struct RTPPeer * RTPPeerCreate( unsigned long ssrc, socklen_t size, struct sockaddr * addr ) {
  struct RTPPeer * peer = malloc( sizeof( struct RTPPeer ) );
  peer->refs = 1;
  peer->address.ssrc = ssrc;
  peer->address.size = size;
  memcpy( &(peer->address.addr), addr, size );
  peer->timestamp_diff = 0;
  peer->in_seqnum      = 0;
  peer->in_timestamp   = 0;
  peer->out_seqnum     = 0;
  peer->out_timestamp  = 0;
  peer->info = NULL;
  return peer;
}
  
void RTPPeerDestroy( struct RTPPeer * peer ) {
  free( peer );
}

void RTPPeerRetain( struct RTPPeer * peer ) {
  peer->refs++;
}

void RTPPeerRelease( struct RTPPeer * peer ) {
  if( ! --peer->refs ) {
    RTPPeerDestroy( peer );
  }
}

int RTPPeerGetSSRC( struct RTPPeer * peer, unsigned long * ssrc ) {
  if( ssrc == NULL ) return 1;
  *ssrc = peer->address.ssrc;
  return 0;
}

int RTPPeerGetAddress( struct RTPPeer * peer, socklen_t * size, struct sockaddr ** addr ) {
  if( size == NULL || addr == NULL ) return 1;
  *size =   peer->address.size;
  *addr = &(peer->address.addr);
  return 0;
}

struct RTPSession {
  size_t refs;
  
  int socket;
  struct RTPAddress self;
  struct RTPPeer *  peers[RTP_MAX_PEERS];
  
  size_t buflen;
  void * buffer;

  size_t         padding_block_size;  
  unsigned char  marker_payload_type;
};

#pragma mark Creation and destruction
/**
 * @name Creation and destruction
 * Creating, destroying and reference counting of RTPSession objects.
 * @{
 */

static void _init_addr_with_socket( struct RTPAddress * address, int socket ) {
  address->ssrc = random();
  getsockname( socket, &(address->addr), &(address->size) );
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

static unsigned long _get_timestamp( struct RTPSession * session ) {
  struct timeval tv;
  gettimeofday( &tv, NULL );
  return (tv.tv_sec * USEC_PER_SEC + tv.tv_usec);
}

/**
 * @brief Create an RTPSession instance.
 * Allocate space and initialize an RTPSession instance.
 * @public @memberof RTPSession
 * @param socket A socket to use for communication. Should already be bound.
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
  
  session->marker_payload_type  = 0;
  return session;
}

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

void RTPSessionRetain( struct RTPSession * session ) {
  session->refs++;
}

void RTPSessionRelease( struct RTPSession * session ) {
  if( ! --session->refs ) {
    RTPSessionDestroy( session );
  }
}

int RTPSessionSetMarker( struct RTPSession * session, unsigned char marker ) {
  if( marker ) {
    session->marker_payload_type |= 0x80;
  } else {
    session->marker_payload_type &= 0x7f;
  }
  return 0;
}

int RTPSessionGetMarker( struct RTPSession * session, unsigned char * marker ) {
  if( marker == NULL ) return 1;
  *marker = session->marker_payload_type & 0x80;
  return 0;
}

int RTPSessionSetPayloadType( struct RTPSession * session, unsigned char payload_type ) {
  session->marker_payload_type &= 0x80;
  session->marker_payload_type |= payload_type & 0x7f;
  return 0;
}

int RTPSessionGetPayloadType( struct RTPSession * session, unsigned char * payload_type ) {
  if( payload_type == NULL ) return 1;
  *payload_type = session->marker_payload_type & 0x7f;
  return 0;
}

int RTPSessionAddPeer( struct RTPSession * session, struct RTPPeer * peer ) {
  int i;
  for( i=0; i < RTP_MAX_PEERS; i++ ) {
    if( session->peers[i] == NULL ) {
      session->peers[i] = peer;
      RTPPeerRetain( peer );
      return 0;
    }
  }
  return 1;
}

int RTPSessionRemovePeer( struct RTPSession * session, struct RTPPeer * peer ) {
  int i;
  for( i=0; i < RTP_MAX_PEERS; i++ ) {
    if( session->peers[i] == peer ) {
      session->peers[i] = NULL;
      RTPPeerRelease( peer );
      return 0;
    }
  }
  return 1;
}

int RTPSessionFindPeerBySSRC( struct RTPSession * session, struct RTPPeer ** peer,
                              unsigned long ssrc ) {
  int i;
  unsigned long s = 0;
  for( i=0; i < RTP_MAX_PEERS; i++ ) {
    if( session->peers[i] != NULL ) {
      RTPPeerGetSSRC( session->peers[i], &s );
      if( s == ssrc ) {
        *peer = session->peers[i];
        return 0;
      }
    }
  }
  return 1;
}

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

/**
 * Encode an RTP packet for transmission over network.
 * Read all RTP information (timestamp, ssrc, payload-size, etc.) from an info structure
 * and encode it to an RTP packet.
 * @private @memberof RTPSession
 * @param info The packet information structure to fill.
 * @param size The size of the received packet in bytes.
 */
static int _encode_packet( struct RTPSession * session, struct RTPPacketInfo * info, size_t size, void * data ) {
  int i, j;
  unsigned char * buffer = data;
  size_t header_size     = 12 + ( info->csrc_count * 4 );
  size_t ext_header_size = header_size + ( info->extension ? 4 : 0 );
  size_t data_size       = ext_header_size + info->size;
  size_t total_size      = data_size + info->padding;

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
  
  if( info->size > 0 && info->data != NULL ) {
    memcpy( data+ext_header_size, info->data, info->size );
  //memset( data+data_size, 0, info->padding-1 ); // should be ignored
    buffer[total_size-1] = info->padding;
  }
  return 0;
}

/**
 * Decode an RTP packet received on a socket.
 * Store all RTP information (timestamp, ssrc, payload-size, etc.) inside the info structure
 * and set the info-structures data pointer to the location of the payload.
 * @private @memberof RTPSession
 * @param info The packet information structure to fill.
 * @param size The size of the received packet in bytes.
 */
static int _decode_packet( struct RTPSession * session, struct RTPPacketInfo * info, size_t size, void * data ) {
  int i, j;
  unsigned char * buffer = data;
  size_t header_size;
  size_t ext_header_size;
  size_t data_size;
  size_t total_size = size;

  if( ( buffer[0] & 0x60 ) != 0x80 ) {
    return 1; // wrong rtp version
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
    //buffer[header_size];
    //buffer[header_size+1];
    j =   buffer[header_size+2]
      | ( buffer[header_size+3] << 8 );
    ext_header_size = header_size + 4 + (j*4);
  } else {
    ext_header_size = header_size;
  }

  info->size = total_size - ext_header_size - info->padding;

  if( info->size == 0 ) {
    info->data = NULL;
  } else {
    if( info->data != NULL ) {
      memcpy( info->data, data+ext_header_size, info->size );
    } else {
      info->data = data+ext_header_size;
    }
  }
  return 0;
}

int RTPSessionSendToPeer( struct RTPSession * session, struct RTPPeer * peer, size_t size, void * payload,
                          struct RTPPacketInfo * info ) {
  size_t header_size     = 12 + ((info->csrc_count & 0x0f) * 4 );
  size_t ext_header_size = header_size;
  size_t data_size       = ext_header_size + size;
  size_t total_size      = data_size;
  
  if( session->padding_block_size ) {
    info->padding = session->padding_block_size - ( data_size % session->padding_block_size );
    total_size   += info->padding;
  } else {
    info->padding = 0;
  }
  info->csrc_count      = info->csrc_count & 0x0f;
  info->sequence_number = peer->out_seqnum + 1;
  info->timestamp       = 0;
  info->ssrc            = session->self.ssrc;
  info->size            = size;
  info->data            = payload;

  if( total_size > session->buflen ) {
    session->buflen = total_size * 1.2;
    session->buffer = realloc( session->buffer, session->buflen );
    if( session->buffer == NULL ) {
      session->buflen = 0;
      return 1;
    }
  }
  
  _encode_packet( session, info, total_size, session->buffer );
  total_size = sendto( session->socket, session->buffer, total_size, 0,
                       &(peer->address.addr), peer->address.size );

  if( total_size == -1 ) {
    return 1;
  }
  
  peer->out_seqnum    = info->sequence_number;
  peer->out_timestamp = info->timestamp;
  return 0;
}

int RTPSessionReceiveFromPeer( struct RTPSession * session, struct RTPPeer * peer, size_t size, void * payload,
                               struct RTPPacketInfo * info ) {
  peer->in_seqnum    = info->sequence_number;
  peer->in_timestamp = info->timestamp;
  return 0;
}

int RTPSessionReceive( struct RTPSession * session, size_t size, void * payload,
                       struct RTPPacketInfo * info ) {
  struct RTPPeer * peer;
  struct RTPPacketInfo _info;
                        
  char buffer[SOCKADDR_BUFLEN];
  struct RTPAddress address;
  size_t total_size;
  
  if( info == NULL ) {
    info = &_info;
  }
  
  total_size = recvfrom( session->socket, session->buffer, session->buflen,
                         0, &address.addr, &address.size );

  if( total_size >= session->buflen ) {
    // message probably too large for buffer, grow buffer & cancel
    session->buflen = total_size * 1.2;
    session->buffer = realloc( session->buffer, session->buflen );
    if( session->buffer == NULL ) {
      session->buflen = 0;
    }
    return 1;
  }
  
  RTPSessionFindPeerByAddress( session, &peer, address.size, &address.addr );
  
  _decode_packet( session, info, total_size, session->buffer );
  
  if( payload != NULL ) {
    memcpy( payload, info->data, (size < total_size ? size : total_size) );
  }
  
  return RTPSessionReceiveFromPeer( session, peer, info->size, info->data, info );
}

/*
 * New calling strategy:
 *
 * SEND:
 *
 * RTPSessionSend( session, size, payload, info=NULL )
 *  |
 * < is info NULL?> -yes-+
 *  |                    |
 *  no         ( use session->info )
 *  |                    |
 *  +--------------------+
 *  |
 * ( send to next peer in list ) -+-> RTPSessionSendToPeer( session, peer, size, payload, info=NULL )
 *                                |           |
 *                                |   ( call delegate )
 *                                |   ( encode packet )
 *                                |    ( send packet )
 *                                |           |
 *  +-----------------------------------------+
 *  |                             |
 * < more peers connected? > -yes-+
 *  |
 * ( done )
 *
 * RECEIVE:
 *
 * RTPSessionReceive( session, size, payload, info=NULL )
 *  |
 * < is info NULL?> -yes-+
 *  |                    |
 *  no         ( use session->info )
 *  |                    |
 *  +--------------------+
 *  |
 * ( receive packet )
 * ( decode packet )
 *  |
 * < is peer in list? > -yes-> RTPSessionReceiveFromPeer( session, peer, size, payload, info=NULL )
 *                                     |
 *                             ( call delegate )
 *                                     |
 *  +----------------------------------+
 *  |
 * < message fits into payload buffer? > -no-> return
 *  |
 *  yes
 *  |
 * ( store message )
 * ( done )
 */
