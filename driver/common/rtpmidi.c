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
int RTPMIDISessionSend( struct RTPMIDISession * session, size_t size, struct MIDIMessage ** messages,
                        size_t * count, struct RTPPacketInfo * info ) {
  int result;
  size_t m;
  struct RTPMIDIHeaderInfo minfo;
  MIDIStatus status;

  /* use RTPSessionSendPacket() */
  
  /* _rtpmidi_encode_header( info->payload_size, info->payload, &minfo ); */
  for( m=0; m<size; m++ ) {
    MIDIMessageGetStatus( messages[m], &status );
    if( status == MIDI_STATUS_SYSTEM_EXCLUSIVE ) {
      /* MIDIMessageEncode( messages[m], <#size_t bytes#>, <#unsigned char *buffer#>); */
    } else {
      /* MIDIMessageEncode( messages[m], <#size_t bytes#>, <#unsigned char *buffer#>); */
    }
  }
  result = RTPSessionSendPacket( session->rtp_session, info );

  return 0;
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
 * @param size The number of allocated entries pointed to by @c messages.
 * @param messages A pointer to a list of @c size message pointers.
 * @param count The number of received messages.
 * @param info The info for the last received packet.
 * @retval 0 on success.
 * @retval >0 If the message was corrupted or could not be received.
 */
int RTPMIDISessionReceive( struct RTPMIDISession * session, size_t size, struct MIDIMessage ** messages,
                           size_t * count, struct RTPPacketInfo * info ) {
  int result;
  size_t m;
  struct RTPMIDIHeaderInfo minfo;
  
  /* use RTPSessionReceivePacket() */
  result = RTPSessionReceivePacket( session->rtp_session, info );
  /* _rtpmidi_decode_header( info->payload_size, info->payload, &minfo ); */
  if( minfo.phantom ) {
  }
  if( ! minfo.zero ) {
    /* MIDIUtilReadVarLen( info->payload, <#size_t bytes#>, <#MIDIVarLen *value#>, <#size_t *read#>); */
  }
  /* MIDIMessageEncode( messages[0], <#size_t bytes#>, info->payload ); */
  for( m=0; m<minfo.messages; m++ ) {
    /* MIDIUtilReadVarLen( info->payload, <#size_t bytes#>, <#MIDIVarLen *value#>, <#size_t *read#>);
    MIDIMessageEncode( messages[0], <#size_t bytes#>, info->payload ); */
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
