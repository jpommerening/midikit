#include "rtpmidi.h"
#include "rtp.h"
#include "midi/util.h"

/**
 * @defgroup RTP-MIDI RTP-MIDI
 * @ingroup RTP
 * @{
 */

/**
 * @brief RTP-MIDI payload header.
 */
struct RTPMIDIInfo {
  unsigned char journal;
  unsigned char zero;
  unsigned char phantom;
  int len;
};

/**
 * Structure for easily removing journal parts by sequence number.
 */
struct RTPMIDIJournalChapterPart {
  unsigned short checkpoint_pkt_seqnum; /**< The RTP sequence number encoded in this journal part */
  unsigned short length;                /**< Number of bytes[] allocated */
  unsigned char  bytes[1];              /**< Variable length array of chapter bytes */
};

/**
 * Hold information for the system history.
 */
struct RTPMIDISystemJournal {
  unsigned short checkpoint_pkt_seqnum;         /**< The latest RTP sequence number encoded in the journal */
  struct RTPMIDIJournalChapterPart * chapter_d; /**< Chapter D: Song Select (0xf3), Tune Request (0xf6), Reset (0xff),
                                                                undefined System commands (0xf4, 0xf5, 0xf9, 0xfd) */
  struct RTPMIDIJournalChapterPart * chapter_v; /**< Chapter V: Active Sense (0xfe) */
  struct RTPMIDIJournalChapterPart * chapter_q; /**< Chapter Q: Sequencer State (0xf2, 0xf8, 0xf9, 0xfa, 0xfb, 0xfc) */
  struct RTPMIDIJournalChapterPart * chapter_f; /**< Chapter F: MTC Tape Position (0xf1, 0xf0, 0x7f, 0xcc 0x01 0x01) */
  struct RTPMIDIJournalChapterPart * chapter_x; /**< Chapter X: System Exclusive (all other 0xf0) */
};

/**
 * Hold information for the channel history.
 */
struct RTPMIDIChannelJournal {
  unsigned short checkpoint_pkt_seqnum;         /**< The latest RTP sequence number encoded in the journal */
  unsigned char  enhanced_chapter_c;            /**< Use enhanced chapter C coding */
  unsigned char  channel;                       /**< The channel number */
  struct RTPMIDIJournalChapterPart * chapter_p; /**< Chapter P: MIDI Program Change (0xc) */
  struct RTPMIDIJournalChapterPart * chapter_c; /**< Chapter C: MIDI Control Change (0xb) */
  struct RTPMIDIJournalChapterPart * chapter_m; /**< Chapter M: MIDI Parameter System (part of 0xb) */
  struct RTPMIDIJournalChapterPart * chapter_w; /**< Chapter W: MIDI Pitch Wheel (0xe) */
  struct RTPMIDIJournalChapterPart * chapter_n; /**< Chapter N: MIDI NoteOff (0x8), NoteOn (0x9) */
  struct RTPMIDIJournalChapterPart * chapter_e; /**< Chapter E: MIDI Note Command Extras (0x8, 0x9) */
};

/**
 * Hold information for the RTP-MIDI session history.
 */
struct RTPMIDIJournal {
/*unsigned char  single_pkt_loss;*/
  unsigned char  enhanced_chapter_c;    /**< Use enhanced chapter C coding in at least one channel journal */
  unsigned char  total_channels;        /**< The number of valid entries in the channel journals list */
  unsigned short checkpoint_pkt_seqnum; /**< The latest RTP sequence number encoded in the journal */
  struct RTPMIDISystemJournal  * system_journal;
  struct RTPMIDIChannelJournal * channel_journals[16];

  size_t size;
  void * buffer;
};

struct RTPMIDIPeerInfo {
  struct RTPMIDIJournal * receive_journal;
  struct RTPMIDIJournal * send_journal;
  void * info;
};

/**
 * @brief Send and receive MIDIMessage objects via an @c RTPSession.
 * This class describes a payload format to code MIDI messages as
 * RTP payload. It extends the RTPSession by methods to send and
 * receive MIDIMessage objects.
 * For implementation details refer to RFC 4695 & 4696.
 */
struct RTPMIDISession {
/**
 * @privatesection
 * @cond INTERNALS
 */
  size_t refs;
  struct RTPMIDIInfo   midi_info;
  struct RTPPacketInfo rtp_info;
  struct RTPSession  * rtp_session;

  size_t size;
  void * buffer;
/** @endcond */
};

/** @} */

/* MARK: Creation and destruction *//**
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
  session->rtp_session = rtp_session;
  RTPSessionRetain( rtp_session );

  session->midi_info.journal = 0;
  session->midi_info.zero    = 0;
  session->midi_info.phantom = 0;
  session->midi_info.len     = 0;

  session->size   = 512;
  session->buffer = malloc( session->size );
  if( session->buffer == NULL ) {
    session->size = 0;
  }
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
  RTPSessionRelease( session->rtp_session );
  if( session->size > 0 && session->buffer != NULL ) {
    free( session->buffer );
  }
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

/* MARK: RTP-MIDI journal coding *//**
 * @name RTP-MIDI journal coding
 * Functions for encoding the various journals and their chapters to a
 * continuous stream of bytes.
 * @{
 */

/**
 * @brief Encode the RTP-MIDI history to a stream.
 * @memberof RTPMIDIJournal
 * @param session The session.
 * @param journal The journal.
 * @param size    The number of available bytes in the buffer.
 * @param buffer  The buffer to write the journal to.
 * @param written The number of bytes written to the stream.
 */
static int _rtpmidi_journal_encode( struct RTPMIDISession * session, struct RTPMIDIJournal * journal,
                                    size_t size, void * buffer, size_t * written ) {
  *written = 0;
  return 0;
}

/**
 * @brief Decode the RTP-MIDI history from a stream.
 * @memberof RTPMIDIJournal
 * @param session The session.
 * @param journal The journal.
 * @param size    The number of available bytes in the buffer.
 * @param buffer  The buffer to read the journal from.
 * @param read    The number of bytes read from the stream.
 */
static int _rtpmidi_journal_decode( struct RTPMIDISession * session, struct RTPMIDIJournal * journal,
                                    size_t size, void * buffer, size_t * read ) {
  *read = 0;
  return 0;
}

/**
 * @brief Store a list of messages in the journal.
 * @memberof RTPMIDIJournal
 * @param journal    The journal.
 * @param checkpoint The sequence number of the packet that contains the messages.
 * @param messages   The list of messages to store.
 */
static int _rtpmidi_journal_encode_messages( struct RTPMIDIJournal * journal, unsigned short checkpoint,
                                             struct MIDIMessageList * messages ) {
  struct MIDIMessage * message;
  return 0;
}

/**
 * @brief Restore a list of messages from the journal.
 * @memberof RTPMIDIJournal
 * @param journal    The journal.
 * @param checkpoint The sequence number of the packet that contains the messages.
 * @param messages   The list to store the recovered messages in.
 */
static int _rtpmidi_journal_decode_messages( struct RTPMIDIJournal * journal, unsigned short checkpoint,
                                             struct MIDIMessageList * messages ) {
  struct MIDIMessage * message;
  return 0;
}

/** @} */

/* MARK: RTP transmission extension *//**
 * @name RTP transmission extension
 * Sending and receiving MIDIMessage objects using the RTP-protocol.
 * @{
 */

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
    RTPPeerSetInfo( peer, info_ );
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
    info_ = _rtpmidi_peer_info_create();
    RTPPeerSetInfo( peer, info_ );
    *info = NULL;
  } else {
    *info = info_->info;
  }
  return 0;
}

/**
 * @brief Trunkate a peers send journal.
 * Remove all message entries that have a sequence number less or equal to @c seqnum.
 * @public @memberof RTPMIDISession
 * @param session The session.
 * @param peer    The peer.
 * @param seqnum  The sequence number.
 * @retval 0 on success.
 */
int RTPMIDISessionJournalTrunkate( struct RTPMIDISession * session, struct RTPPeer * peer, unsigned long seqnum ) {
  return 0;
}

/**
 * @brief Store a list of messages in a peer's send journal and
 * associate them with the given sequence number.
 * @public @memberof RTPMIDISession
 * @param session  The session.
 * @param peer     The peer.
 * @param seqnum   The sequence number.
 * @param messages The messages list to encode.
 * @retval 0 on success.
 */
int RTPMIDISessionJournalStoreMessages( struct RTPMIDISession * session, struct RTPPeer * peer,
                                        unsigned long seqnum, struct MIDIMessageList * messages ) {
  struct RTPMIDIPeerInfo * info = NULL;

  RTPPeerGetInfo( peer, (void**) &info );
  if( info == NULL ) {
    info = _rtpmidi_peer_info_create();
    RTPPeerSetInfo( peer, info );
  }

  if( info->send_journal == NULL ) {
    /* create journal */
  }

  return _rtpmidi_journal_encode_messages( info->send_journal, seqnum, messages );
}

/**
 * @brief Restore a list of messages from a peer's receive
 * journal associated with the given sequence number.
 * @public @memberof RTPMIDISession
 * @param session  The session.
 * @param peer     The peer.
 * @param seqnum   The sequence number.
 * @param messages The messages list to decode messages to.
 * @retval 0 on success.
 */
int RTPMIDISessionJournalRecoverMessages( struct RTPMIDISession * session, struct RTPPeer * peer,
                                          unsigned long seqnum, struct MIDIMessageList * messages ) {
  struct RTPMIDIPeerInfo * info = NULL;

  RTPPeerGetInfo( peer, (void**) &info );
  if( info == NULL ) return 0;
  if( info->receive_journal == NULL ) return 0;

  return _rtpmidi_journal_decode_messages( info->receive_journal, seqnum, messages );
}

static void _advance_buffer( size_t * size, void ** buffer, size_t bytes ) {
  *size   -= bytes;
  *buffer += bytes;
}

static int _rtpmidi_encode_header( struct RTPMIDIInfo * info, size_t size, void * data, size_t * written ) {
  unsigned char * buffer = data;
  if( info->len > 0x0fff ) return 1;
  if( info->len > 0x0f ) {
    if( size<2 ) return 1;
    buffer[0] = 0x80
              | (info->journal ? 0x40 : 0 )
              | (info->zero    ? 0x20 : 0 )
              | (info->phantom ? 0x10 : 0 )
              | ( (info->len >> 8) & 0x0f );
    buffer[1] = info->len & 0xff;
    *written = 2;
  } else {
    if( size<1 ) return 1;
    buffer[0] = (info->journal ? 0x40 : 0 )
              | (info->zero    ? 0x20 : 0 )
              | (info->phantom ? 0x10 : 0 )
              | (info->len     & 0x0f );
    *written = 1;
  }
  return 0;
}

static int _rtpmidi_decode_header( struct RTPMIDIInfo * info, size_t size, void * data, size_t * read ) {
  unsigned char * buffer = data;
  if( buffer[0] & 0x80 ) {
    if( size<2 ) return 1;
    info->journal = (buffer[0] & 0x40) ? 1 : 0;
    info->zero    = (buffer[0] & 0x20) ? 1 : 0;
    info->phantom = (buffer[0] & 0x10) ? 1 : 0;
    info->len     = ((buffer[0] & 0x0f) << 8) | buffer[1];
    *read = 2;
  } else {
    if( size<1 ) return 1;
    info->journal = (buffer[0] & 0x40) ? 1 : 0;
    info->zero    = (buffer[0] & 0x20) ? 1 : 0;
    info->phantom = (buffer[0] & 0x10) ? 1 : 0;
    info->len     = (buffer[0] & 0x0f);
    *read = 1;
  }
  return 0;
}

static int _rtpmidi_encode_messages( struct RTPMIDIInfo * info, MIDITimestamp timestamp, struct MIDIMessageList * messages, size_t size, void * data, size_t * written ) {
  int m, result = 0;
  void * buffer = data;
  size_t w;
  MIDIRunningStatus status = 0;
  MIDITimestamp     timestamp2;
  MIDIVarLen        time_diff;

  for( m=0; (size>0) && (messages!=NULL) && (messages->message!=NULL); m++ ) {
    MIDIMessageGetTimestamp( messages->message, &timestamp2 );
    time_diff = ( timestamp2 > timestamp ) ? ( timestamp2 - timestamp ) : 0;
    timestamp = timestamp2;
    if( m == 0 ) {
      info->zero = time_diff ? 1 : 0;
    }
    if( m > 0 || info->zero == 1 ) {
      MIDIUtilWriteVarLen( &time_diff, size, buffer, &w );
      _advance_buffer( &size, &buffer, w );
    }
    MIDIMessageEncodeRunningStatus( messages->message, &status, size, buffer, &w );
    _advance_buffer( &size, &buffer, w );
    messages = messages->next;
  }

  info->len = buffer - data;
  
  *written = buffer - data;
  return result;
}

static int _rtpmidi_decode_messages( struct RTPMIDIInfo * info, MIDITimestamp timestamp, struct MIDIMessageList * messages, size_t size, void * data, size_t * read ) {
  int m, result = 0;
  void * buffer = data;
  size_t r;
  MIDIRunningStatus status = 0;
  MIDIVarLen        time_diff;

  if( info->phantom ) {
    /* we don't really care about the source coding for now .. */
  }
  for( m=0; (size>0) && (messages!=NULL) && (buffer-data) < info->len; m++ ) {
    if( messages->message == NULL ) {
      messages->message = MIDIMessageCreate( 0 );
    }
    if( m > 0 || info->zero == 1 ) {
      MIDIUtilReadVarLen( &time_diff, size, buffer, &r );
      _advance_buffer( &size, &buffer, r );
    } else {
      time_diff = 0;
    }

    MIDIMessageDecodeRunningStatus( messages->message, &status, size, buffer, &r );
    _advance_buffer( &size, &buffer, r );

    timestamp += time_diff;
    MIDIMessageSetTimestamp( messages->message, timestamp );
    messages = messages->next;
  }

  *read = buffer - data;
  return result;
}

/**
 * @brief Send MIDI messages over an RTPSession.
 * Broadcast the messages to all connected peers. Store the number of sent messages
 * in @c count, if the @c info argument was specified it will be populated with the
 * packet info of the last sent packet.
 * The peer's control structures will be updated with the required journalling
 * information.
 * @public @memberof RTPMIDISession
 * @param session  The session.
 * @param messages A pointer to a list of @c size message pointers.
 * @retval 0 On success.
 * @retval >0 If the message could not be sent.
 */
int RTPMIDISessionSend( struct RTPMIDISession * session, struct MIDIMessageList * messages ) {
  int result = 0;
  struct iovec iov[3];
  size_t written = 0;
  size_t size    = session->size;
  void * buffer  = session->buffer;

  struct RTPPeer        * peer    = NULL;
  struct RTPMIDIJournal * journal = NULL;
  struct RTPMIDIInfo    * minfo   = &(session->midi_info);
  struct RTPPacketInfo  * info    = &(session->rtp_info);

  MIDITimestamp timestamp;

  MIDIMessageGetTimestamp( messages->message, &timestamp );

  info->peer            = 0;
  info->padding         = 0;
  info->extension       = 0;
  info->csrc_count      = 0;
  info->marker          = 0;
  info->payload_type    = 97;
  info->sequence_number = 0; /* filled out by rtp */
  info->timestamp       = timestamp; /* filled out by rtp but shouldn't */

  minfo->journal = 0;
  minfo->phantom = 0;
  minfo->zero    = 0;

  _rtpmidi_encode_messages( minfo, timestamp, messages, size, buffer, &written );
  iov[1].iov_base = buffer;
  iov[1].iov_len  = written;
  _advance_buffer( &size, &buffer, written );

  _rtpmidi_encode_header( minfo, size, buffer, &written );
  iov[0].iov_base = buffer;
  iov[0].iov_len  = written;
  _advance_buffer( &size, &buffer, written );

  /* send encoded messages to each peer
   * each peer has its own journal */
  result = RTPSessionNextPeer( session->rtp_session, &peer );
  while( peer != NULL ) {
    if( minfo->journal ) {
      journal = NULL; /* peer out journal */
      _rtpmidi_journal_encode( session, journal, size, buffer, &written );
      iov[2].iov_base = buffer;
      iov[2].iov_len  = written;
      _advance_buffer( &size, &buffer, written );
    } else {
      iov[2].iov_base = NULL;
      iov[2].iov_len  = 0;
    }

    info->peer   = peer;
    info->iovlen = ( minfo->journal ) ? 3 : 2;
    info->iov    = &(iov[0]);
    info->payload_size = iov[0].iov_len + iov[1].iov_len + iov[2].iov_len;

    result = RTPSessionSendPacket( session->rtp_session, info );

    if( result == 0 && minfo->journal ) {
      _rtpmidi_journal_encode_messages( journal, info->sequence_number, messages );
    }

    RTPSessionNextPeer( session->rtp_session, &peer );
  }

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
 * @param session  The session.
 * @param messages A pointer to a list of midi messages.
 * @retval 0 on success.
 * @retval >0 If the message was corrupted or could not be received.
 */
int RTPMIDISessionReceive( struct RTPMIDISession * session, struct MIDIMessageList * messages ) {
  int result = 0;
  struct iovec iov[3];
  size_t read = 0;
  size_t size;
  void * buffer;
  MIDITimestamp timestamp;

  struct RTPPeer        * peer    = NULL;
  struct RTPMIDIJournal * journal = NULL;
  struct RTPMIDIInfo    * minfo   = &(session->midi_info);
  struct RTPPacketInfo  * info    = &(session->rtp_info);

  info->iovlen = 3;
  info->iov    = &(iov[0]);
  
  if( messages == NULL ) return 1;
  result = RTPSessionReceivePacket( session->rtp_session, info );
  if( result != 0 ) return result;
  
  timestamp = info->timestamp;
  size      = info->iov[0].iov_len;
  buffer    = info->iov[0].iov_base;

  _rtpmidi_decode_header( minfo, size, buffer, &read );
  _advance_buffer( &size, &buffer, read );

  _rtpmidi_decode_messages( minfo, timestamp, messages, size, buffer, &read );
  _advance_buffer( &size, &buffer, read );
  
  if( minfo->journal ) {
    if( 0 /* check for out of order package */ ) {
      if( 0 /* any more packets pending? */ ) {
        /* move messages to buffer */
        /* receive pending packets, try again */
      } else {
        journal = NULL; /* peer in journal */
        _rtpmidi_journal_decode( session, journal, size, buffer, &read );
        _advance_buffer( &size, &buffer, read );
        /* recover from journal */
      }
    }
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
  return result;
}

/** @} */
