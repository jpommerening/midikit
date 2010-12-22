#include "rtpmidi.h"

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
  struct RTPSession * rtp_session;
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
  session->size = 0;
  session->message_buffer = NULL;
  session->rtp_session = rtp_session;
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

/**
 * @brief Send MIDI messages over an RTPSession.
 * Broadcast the messages to all connected peers. Store the number of sent messages
 * in @c count, if the @info argument was specified it will be populated with the
 * packet info of the last sent packet.
 * The peer's control structures will be updated with the required journalling
 * information.
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
  /* use RTPSessionSendPacket() */
  return 0;
}

/**
 * @brief Receive MIDI messages over an RTPSession.
 * Receive messages from any connected peer. Store the number of received messages
 * in @c count, if the @info argument was specified it will be populated with the
 * packet info of the last received packet.
 * If lost packets are detected the required information is recovered from the
 * journal.
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
  /* use RTPSessionReceivePacket()
   * - clear the internal packet buffer
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
