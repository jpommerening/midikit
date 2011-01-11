#include "rtpmidi.h"
#include "midi/util.h"

struct RTPMIDIHeaderInfo {
  unsigned char journal;
  unsigned char zero;
  unsigned char phantom;
  int messages;
};

struct RTPMIDIPeerInfo {
  struct RTPMIDIJournal * receive_journal;
  struct RTPMIDIJournal * send_journal;
  void * info;
};

/**
 * @brief Send and receive @c MIDIMessage objects via an @c RTPSession.
 * This class describes a payload format to code MIDI messages as
 * RTP payload. It extends the @c RTPSession by methods to send and
 * receive @c MIDIMessage objects.
 */
struct RTPMIDISession {
  size_t refs;
  size_t size;
  struct MIDIMessage ** message_buffer;
  struct RTPSession   * rtp_session;
  struct RTPPacketInfo  rtp_info;
};

#pragma mark Creation and destruction
/**
 * @name Creation and destruction
 * Creating, destroying and reference counting of RTPMIDISession objects.
 * @{
 */

/**
 * @brief Create an RTPMIDISession instance.
 * Allocate space and initialize an RTPMIDISession instance.
 * @public @memberof RTPMIDISession
 * @param rtp_session The RTP session to use for transmission.
 * @return a pointer to the created structure on success.
 * @return a @c NULL pointer if the session could not created.
 */
struct RTPMIDISession * RTPMIDISessionCreate( struct RTPSession * rtp_session ) {
  struct RTPMIDISession * session = malloc( sizeof( struct RTPMIDISession ) );
  if( session == NULL ) return NULL;

  session->refs = 1;
  session->size = 16;
  session->message_buffer = malloc( sizeof( struct MIDIMessage * ) * session->size );
  if( session->message_buffer == NULL ) session->size = 0;
  session->rtp_session    = rtp_session;
  RTPSessionRetain( rtp_session );
  return session;
}

/**
 * @brief Destroy an RTPMIDISession instance.
 * Free all resources occupied by the session and release the
 * RTP session.
 * @public @memberof RTPMIDISession
 * @param session The session.
 */
void RTPMIDISessionDestroy( struct RTPMIDISession * session ) {
  if( session->message_buffer == NULL ) {
    free( session->message_buffer );
  }
  RTPSessionRelease( session->rtp_session );
  free( session );
}

/**
 * @brief Retain an RTPMIDISession instance.
 * Increment the reference counter of a session so that it won't be destroyed.
 * @public @memberof RTPMIDISession 
 * @param session The session.
 */
void RTPMIDISessionRetain( struct RTPMIDISession * session ) {
  session->refs++;
}

/**
 * @brief Release an RTPMIDISession instance.
 * Decrement the reference counter of a session. If the reference count
 * reached zero, destroy the session.
 * @public @memberof RTPMIDISession 
 * @param session The session.
 */
void RTPMIDISessionRelease( struct RTPMIDISession * session ) {
  if( ! --session->refs ) {
    RTPMIDISessionDestroy( session );
  }
}

/** @} */

#pragma mark RTP transmission extension
/**
 * @name RTP transmission extension
 * Sending and receiving @c MIDIMessage objects using the RTP-protocol.
 * @{
 */

int RTPMIDISessionTrunkateSendJournal( struct RTPMIDISession * session, struct RTPPeer * peer, unsigned long seqnum ) {
  return 0;
}

int RTPMIDISessionTrunkateReceiveJournal( struct RTPMIDISession * session, struct RTPPeer * peer, unsigned long seqnum ) {
  return 0;
}

static void * _rtpmidi_peer_info_create() {
  struct RTPMIDIPeerInfo * info = malloc( sizeof( struct RTPMIDIPeerInfo ) );
  info->send_journal = NULL;
  info->receive_journal = NULL;
  info->info = NULL;
  return info;
}

/**
 * @brief Set the pointer of the internal info-structure.
 * @relates RTPMIDISession
 * @param peer The peer.
 * @param info A pointer to the info-structure.
 * @retval 0 on success.
 * @retval >0 if the info-structure could not be set.
 */
int RTPMIDIPeerSetInfo( struct RTPPeer * peer, void * info ) {
  struct RTPMIDIPeerInfo * info_ = NULL;
  RTPPeerGetInfo( peer, (void **) &info_ );
  if( info_ == NULL ) {
    info_ = _rtpmidi_peer_info_create();
  }
  info_->info = info;
  return 0;
}

/**
 * @brief Get a pointer to the info sub-structure.
 * @relates RTPMIDISession
 * @param peer The peer.
 * @param info A pointer to the info-structure.
 * @retval 0 on success.
 * @retval >0 if the info-structure could not be obtained.
 */
int RTPMIDIPeerGetInfo( struct RTPPeer * peer, void ** info ) {
  struct RTPMIDIPeerInfo * info_ = NULL;
  RTPPeerGetInfo( peer, (void **) &info_ );
  if( info_ == NULL ) {
    *info = NULL;
  } else {
    *info = info_->info;
  }
  return 0;
}

static void _advance_buffer( size_t * size, void ** buffer, size_t bytes ) {
  *size   -= bytes;
  *buffer += bytes;
}

static int _rtpmidi_encode_header( struct RTPMIDIHeaderInfo * info, size_t size, void * data, size_t * written ) {
  unsigned char * buffer = data;
  if( info->messages > 0x0fff ) return 1;
  if( info->messages > 0x0f ) {
    if( size<2 ) return 1;
    buffer[0] = 0x80
              | (info->journal ? 0x40 : 0 )
              | (info->zero    ? 0x20 : 0 )
              | (info->phantom ? 0x10 : 0 )
              | ( (info->messages >> 8) & 0x0f );
    buffer[1] = info->messages & 0xff;
    *written = 2;
  } else {
    if( size<1 ) return 1;
    buffer[0] = (info->journal  ? 0x40 : 0 )
              | (info->zero     ? 0x20 : 0 )
              | (info->phantom  ? 0x10 : 0 )
              | (info->messages & 0x0f );
    *written = 1;
  }
  return 0;
}

static int _rtpmidi_decode_header( struct RTPMIDIHeaderInfo * info, size_t size, void * data, size_t * read ) {
  unsigned char * buffer = data;
  if( buffer[0] & 0x80 ) {
    if( size<2 ) return 1;
    info->journal  = (buffer[0] & 0x40) ? 1 : 0;
    info->zero     = (buffer[0] & 0x20) ? 1 : 0;
    info->phantom  = (buffer[0] & 0x10) ? 1 : 0;
    info->messages = ((buffer[0] & 0x0f) << 8) | buffer[1];
    *read = 2;
  } else {
    if( size<1 ) return 1;
    info->journal  = (buffer[0] & 0x40) ? 1 : 0;
    info->zero     = (buffer[0] & 0x20) ? 1 : 0;
    info->phantom  = (buffer[0] & 0x10) ? 1 : 0;
    info->messages = (buffer[0] & 0x0f);
    *read = 1;
  }
  return 0;
}

static int _list_length( struct MIDIMessageList * messages ) {
  int i;
  for( i=0; messages != NULL; i++ ) { messages = messages->next; }
  return i;
}

/**
 * @brief Send MIDI messages over an RTPSession.
 * Broadcast the messages to all connected peers. Store the number of sent messages
 * in @c count, if the @c info argument was specified it will be populated with the
 * packet info of the last sent packet.
 * The peer's control structures will be updated with the required journalling
 * information.
 * @public @memberof RTPMIDISession
 * @param session The session.
 * @param size The number of valid message pointers pointed to by @c messages.
 * @param messages A pointer to a list of @c size message pointers.
 * @param count The number of sent messages.
 * @param info The info for the last sent packet.
 * @retval 0 On success.
 * @retval >0 If the message could not be sent.
 */
int RTPMIDISessionSend( struct RTPMIDISession * session, struct MIDIMessageList * messages,
                        struct RTPPacketInfo * info ) {
  int m, result;
  size_t size = 200, written = 0;
  void * buffer = malloc( 200 ); /* fix me, use global buffer */
  struct RTPMIDIHeaderInfo minfo;
  MIDIRunningStatus status;
  MIDITimestamp     timestamp;
  MIDIVarLen        time_diff;

  if( info == NULL ) {
    info = &(session->rtp_info);
  }

  MIDIMessageGetTimestamp( messages->message, &timestamp );

  info->padding         = 0;
  info->extension       = 0;
  info->csrc_count      = 0;
  info->marker          = 0;
  info->payload_type    = 96;
  info->sequence_number = 0; /* filled out by rtp */
  info->timestamp       = timestamp;
  info->payload_size    = 0;
  info->payload         = buffer;

  minfo.journal = 0;
  minfo.phantom = 0;
  minfo.zero = 1;
  minfo.messages = _list_length( messages );
  
  _rtpmidi_encode_header( &minfo, size, buffer, &written );
  _advance_buffer( &size, &buffer, written );
  for( m=0; (m<minfo.messages) && (size>0) && (messages!=NULL) && (messages->message!=NULL); m++ ) {
    MIDIMessageGetTimestamp( messages->message, &timestamp );
    time_diff = timestamp - info->timestamp;
    if( m > 0 || minfo.zero != 0 ) {
      MIDIUtilWriteVarLen( &time_diff, size, buffer, &written );
      _advance_buffer( &size, &buffer, written );
    }
    MIDIMessageEncodeRunningStatus( messages->message, &status, size, buffer, &written );
    _advance_buffer( &size, &buffer, written );
    messages = messages->next;
  }
  info->payload_size = buffer - info->payload;

  result = RTPSessionSend( session->rtp_session, info->payload_size, info->payload, info );

  free( info->payload ); /* fix me, use global buffer */
  return result;
}


/**
 * @brief Receive MIDI messages over an RTPSession.
 * Receive messages from any connected peer. Store the number of received messages
 * in @c count, if the @c info argument was specified it will be populated with the
 * packet info of the last received packet.
 * If lost packets are detected the required information is recovered from the
 * journal.
 * @public @memberof RTPMIDISession
 * @param session The session.
 * @param messages A pointer to a list of midi messages.
 * @param count The number of received messages.
 * @param info The info for the last received packet.
 * @retval 0 on success.
 * @retval >0 If the message was corrupted or could not be received.
 */
int RTPMIDISessionReceive( struct RTPMIDISession * session, struct MIDIMessageList * messages,
                           struct RTPPacketInfo * info ) {
  int m, result;
  size_t size, read;
  void * buffer;
  struct RTPMIDIHeaderInfo minfo;
  MIDIRunningStatus status;
  MIDITimestamp     timestamp;
  MIDIVarLen        time_diff;
  
  result = RTPSessionReceivePacket( session->rtp_session, info );
  if( result != 0 ) return result;
  
  buffer = info->payload;
  size   = info->payload_size;
  read   = 0;
  
  timestamp = info->timestamp;
  time_diff = 0;
  
  _rtpmidi_decode_header( &minfo, size, buffer, &read );
  if( minfo.phantom ) {
    /* whatever .. */
  }
  for( m=0; (m<minfo.messages) && (size>0) && (messages!=NULL); m++ ) {
    if( messages->message == NULL ) {
      messages->message = MIDIMessageCreate( 0 );
    }
    if( m>0 || minfo.zero != 0 ) {
      MIDIUtilReadVarLen( &time_diff, size, buffer, &read );
      _advance_buffer( &size, &buffer, read );
    }
    MIDIMessageDecodeRunningStatus( messages->message, &status, size, buffer, &read );
    _advance_buffer( &size, &buffer, read );
    MIDIMessageSetTimestamp( messages->message, timestamp + time_diff );
    messages = messages->next;
  }
  
  /* - clear the internal packet buffer
   * - repeat as long as the socket holds packets:
   *   - if the packet is not currupted sort it into the internal packet buffer
   *     using it's sequence number (first sent, first dispatched)
   *   - if the packet is corrupted, omit it
   * - write messages from the internal packet buffer to the message
   *   list and recover lost packets on the way
   * - store messages that don't fit into the supplied buffer in the
   *   internal message buffer
   */
  return 0;
}

/** @} */
