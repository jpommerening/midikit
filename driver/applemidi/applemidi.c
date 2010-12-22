#include "applemidi.h"
#include "driver/common/rtp.h"

#define APPLEMIDI_PROTOCOL_SIGNATURE          0xffff

#define APPLEMIDI_COMMAND_INVITATION          0x494e /** "IN" */
#define APPLEMIDI_COMMAND_INVITATION_REJECTED 0x4e4f /** "NO" */
#define APPLEMIDI_COMMAND_INVITATION_ACCEPTED 0x4f4b /** "OK" */
#define APPLEMIDI_COMMAND_ENDSESSION          0x4259 /** "BY" */
#define APPLEMIDI_COMMAND_SYNCHRONIZATION     0x434b /** "CK" */
#define APPLEMIDI_COMMAND_RECEIVER_FEEDBACK   0x5253 /** "RS" */

struct MIDIDriverAppleMIDI {
  size_t refs;
  int domain;
  int control_socket;
  int rtp_socket;
  unsigned short control_port;
  unsigned short rtp_port;

  struct RTPSession * rtp_session;
};

struct MIDIDriverDelegate MIDIDriverDelegateAppleMIDI = {
  NULL
};

static int _applemidi_connect( struct MIDIDriverAppleMIDI * driver ) {
  if( driver->control_socket <= 0 ) {
    /* connect the control socket */
  }
  return 0;
}

static int _applemidi_disconnect( struct MIDIDriverAppleMIDI * driver ) {
  if( driver->control_socket > 0 ) {
    /* disconnect the control socket */
    if( close( driver->control_socket ) ) {
      return 1;
    }
    driver->control_socket = 0;
  }
  return 0;
}

/**
 * @brief Create a MIDIDriverAppleMIDI instance.
 * Allocate space and initialize an MIDIDriverAppleMIDI instance.
 * @public @memberof MIDIDriverAppleMIDI
 * @return a pointer to the created driver structure on success.
 * @return a @c NULL pointer if the driver could not created.
 */
struct MIDIDriverAppleMIDI * MIDIDriverAppleMIDICreate() {
  struct RTPSession * session;
  struct MIDIDriverAppleMIDI * driver;
  struct sockaddr_in addr;

  driver = malloc( sizeof( struct MIDIDriverAppleMIDI ) );
  if( driver == NULL ) return NULL;
  
  driver->refs = 1;
  driver->domain         = AF_INET;
  driver->control_socket = 0;
  driver->rtp_socket     = 0;
  driver->control_port   = 5004;
  driver->rtp_port       = 5005;
  
  addr.sin_family = AF_INET;
  addr.sin_port   = htons(driver->rtp_port);
  addr.sin_addr.s_addr = INADDR_ANY;
  
  session = RTPSessionCreate( sizeof(addr), (struct sockaddr *) &addr, SOCK_DGRAM );
  if( session == NULL ) {
    free( driver );
    return NULL;
  }
  RTPSessionSetTimestampRate( session, 44100.0 );
  driver->rtp_session    = session;
  return driver;
}

/**
 * @brief Destroy a MIDIDriverAppleMIDI instance.
 * Free all resources occupied by the driver.
 * @public @memberof MIDIDriverAppleMIDI
 * @param driver The driver.
 */
void MIDIDriverAppleMIDIDestroy( struct MIDIDriverAppleMIDI * driver ) {
  RTPSessionRelease( driver->rtp_session );
}

/**
 * @brief Retain a MIDIDriverAppleMIDI instance.
 * Increment the reference counter of a driver so that it won't be destroyed.
 * @public @memberof MIDIDriverAppleMIDI
 * @param driver The driver.
 */
void MIDIDriverAppleMIDIRetain( struct MIDIDriverAppleMIDI * driver ) {
  driver->refs++;
}

/**
 * @brief Release a MIDIDriverAppleMIDI instance.
 * Decrement the reference counter of a driver. If the reference count
 * reached zero, destroy the driver.
 * @public @memberof MIDIDriverAppleMIDI
 * @param driver The driver.
 */
void MIDIDriverAppleMIDIRelease( struct MIDIDriverAppleMIDI * driver ) {
  if( ! --driver->refs ) {
    MIDIDriverAppleMIDIDestroy( driver );
  }
}

/**
 * @brief Set the port to be used for RTP communication.
 * @public @memberof MIDIDriverAppleMIDI
 * @param driver The driver.
 * @param port The port.
 * @retval 0 On success.
 * @retval >0 If the port could not be set.
 */
int MIDIDriverAppleMIDISetRTPPort( struct MIDIDriverAppleMIDI * driver, unsigned short port ) {
  if( port == driver->rtp_port ) return 0;
  /* reconnect if connected? */
  driver->rtp_port = port;
  return 0;
}

/**
 * @brief Get the port used for RTP communication.
 * @public @memberof MIDIDriverAppleMIDI
 * @param driver The driver.
 * @param port The port.
 * @retval 0 On success.
 * @retval >0 If the port could not be set.
 */
int MIDIDriverAppleMIDIGetRTPPort( struct MIDIDriverAppleMIDI * driver, unsigned short * port ) {
  if( port == NULL ) return 1;
  *port = driver->rtp_port;
  return 0;
}

/**
 * @brief Set the port to be used for session management.
 * @public @memberof MIDIDriverAppleMIDI
 * @param driver The driver.
 * @param port The port.
 * @retval 0 On success.
 * @retval >0 If the port could not be set.
 */
int MIDIDriverAppleMIDISetControlPort( struct MIDIDriverAppleMIDI * driver, unsigned short port ) {
  if( port == driver->control_port ) return 0;
  /* reconnect if connected? */
  driver->control_port = port;
  return 0;
}

/**
 * @brief Get the port used for session management.
 * @public @memberof MIDIDriverAppleMIDI
 * @param driver The driver.
 * @param port The port.
 * @retval 0 On success.
 * @retval >0 If the port could not be set.
 */
int MIDIDriverAppleMIDIGetControlPort( struct MIDIDriverAppleMIDI * driver, unsigned short * port ) {
  if( port == NULL ) return 1;
  *port = driver->control_port;
  return 0;
}

/**
 * @brief Connect to a peer.
 * Use the AppleMIDI protocol to establish an RTP-session, including a SSRC that was received
 * from the peer.
 * @public @memberof MIDIDriverAppleMIDI
 * @param driver The driver.
 * @param size The size of the address pointed to by @c addr.
 * @param addr The internet address of the peer.
 * @retval 0 on success.
 * @retval >0 if the connection could not be established.
 */
int MIDIDriverAppleMIDIAddPeer( struct MIDIDriverAppleMIDI * driver, socklen_t size, struct sockaddr * addr ) {
  struct RTPPeer * peer;
  int result;
  unsigned long ssrc = 0;
  
  /* use apple midi protocol to determine ssrc,
   * send invitation "IN" command and check next message:
   * "NO" - invitation rejected, return 1
   * "OK" - invitation accepted, open RTP session, add peer with received ssrc,
   *        start initial synchronization, return 0
   */

  peer = RTPPeerCreate( ssrc, size, addr );
  if( peer == NULL ) return 1;
  result = RTPSessionAddPeer( driver->rtp_session, peer );
  RTPPeerRelease( peer );
  return result;
}

/**
 * @brief Disconnect from a peer.
 * Use the AppleMIDI protocol to tell the peer that the session ended.
 * Remove the peer from the @c RTPSession.
 * @public @memberof MIDIDriverAppleMIDI
 * @param driver The driver.
 * @param size The size of the address pointed to by @c addr.
 * @param addr The internet address of the peer.
 * @retval 0 on success.
 * @retval >0 if the session could not be ended.
 */
int MIDIDriverAppleMIDIRemovePeer( struct MIDIDriverAppleMIDI * driver, socklen_t size, struct sockaddr * addr ) {
  struct RTPPeer * peer;
  int result;

  result = RTPSessionFindPeerByAddress( driver->rtp_session, &peer, size, addr );
  if( result ) return result;

  /* send endsession "BY" command */
  
  return RTPSessionRemovePeer( driver->rtp_session, peer );
}

int MIDIDriverAppleMIDISetRTPSocket( struct MIDIDriverAppleMIDI * driver, int socket ) {
  /* get port from socket info */
  return RTPSessionSetSocket( driver->rtp_session, socket );
}

int MIDIDriverAppleMIDIGetRTPSocket( struct MIDIDriverAppleMIDI * driver, int * socket ) {
  return RTPSessionGetSocket( driver->rtp_session, socket );
}

int MIDIDriverAppleMIDISetControlSocket( struct MIDIDriverAppleMIDI * driver, int socket ) {
  if( socket == driver->control_socket ) return 0;
  int result = _applemidi_disconnect( driver );
  if( result == 0 ) {
    driver->control_socket = socket;
  }
  return result;
}

int MIDIDriverAppleMIDIGetControlSocket( struct MIDIDriverAppleMIDI * driver, int * socket ) {
  int result = _applemidi_connect( driver );
  if( result == 0 ) {
    *socket = driver->control_socket;
  }
  return result;
}

int MIDIDriverAppleMIDIConnect( struct MIDIDriverAppleMIDI * driver ) {
  return 0;
}

int MIDIDriverAppleMIDIDisconnect( struct MIDIDriverAppleMIDI * driver ) {
  return 0;
}

/**
 * @brief Handle incoming MIDI messages.
 * This is called by the RTP-MIDI payload parser whenever it encounters a new MIDI message.
 * There may be multiple messages in a single packet so a single call of @c MIDIDriverAppleMIDI
 * may trigger multiple calls of this function.
 * @public @memberof MIDIDriverAppleMIDI
 * @param driver The driver.
 * @param message The message that was just received.
 * @retval 0 on success.
 * @retval >0 if the message could not be processed.
 */
int MIDIDriverAppleMIDIReceiveMessage( struct MIDIDriverAppleMIDI * driver, struct MIDIMessage * message ) {
  /* put message in dispatch queue */
  return 1;
}

/**
 * @brief Process outgoing MIDI messages.
 * This is called by the generic driver interface to pass messages to this driver implementation.
 * The driver may queue outgoing messages to reduce package overhead, trading of latency for throughput.
 * @public @memberof MIDIDriverAppleMIDI
 * @param driver The driver.
 * @param message The message that should be sent.
 * @retval 0 on success.
 * @retval >0 if the message could not be processed.
 */
int MIDIDriverAppleMIDISendMessage( struct MIDIDriverAppleMIDI * driver, struct MIDIMessage * message ) {
  /* put message in outgoing queue */
  return 1;
}

/**
 * @brief Receive from any peer.
 * This should be called whenever there is new data to be received on a socket.
 * @public @memberof MIDIDriverAppleMIDI
 * @param driver The driver.
 * @retval 0 on success.
 * @retval >0 if the packet could not be processed.
 */
int MIDIDriverAppleMIDIReceive( struct MIDIDriverAppleMIDI * driver ) {
  /* check both sockets for applemidi signature 0xffff (peek, do not remove)
   * if signature matches, check for message type and respond immediately
   * if signature on RTP port is not 0xffff check for rtp version 0x80
   * and delegate to RTPMIDISessionReceive which parses the received packet */
  return 1;
}

/**
 * @brief Send queued messages to all connected peers.
 * This should be called whenever new messages are added to the queue and whenever the
 * socket can accept new data.
 * @public @memberof MIDIDriverAppleMIDI
 * @param driver The driver.
 * @retval 0 on success.
 * @retval >0 if packets could not be sent, or any other operation failed.
 */
int MIDIDriverAppleMIDISend( struct MIDIDriverAppleMIDI * driver ) {
  /* if queue is empty return 0
   * if socket would block return 0
   * check timestamp of messages in queue: if the timestamp of the oldest
   * message exceeds a threshold send the whole queue in a single packet
   * by using RTPMIDISessionSend
   * threshold for realtime messages and note on/off messages is 0 */
  return 1;
}

/**
 * @brief Do idling operations.
 * When there is nothing else to do, keep in sync with connected peers,
 * dispatch incoming messages, send receiver feedback.
 * @public @memberof MIDIDriverAppleMIDI
 * @param driver The driver.
 * @retval 0 on success.
 * @retval >0 if packets could not be sent, or any other operation failed.
 */
int MIDIDriverAppleMIDIIdle( struct MIDIDriverAppleMIDI * driver ) {
  /* check for messages in dispatch (incoming) queue:
   *   if message needs to be dispatched (timestamp >= now+latency)
   *   call MIDIDriverAppleMIDIReceiveMessage
   * send receiver feedback
   * if the last synchronization happened a certain time ago, synchronize again */
  return 1;
}
