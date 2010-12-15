#include <stdlib.h>
#include "test.h"
#include "driver/common/rtp.h"

#define RTP_ADDRESS "127.0.0.1"
#define RTP_SERVER_PORT 5004
#define RTP_CLIENT_PORT 5005
#define RTP_CLIENT_SSRC 1234567890

static struct RTPSession * session = NULL;

static struct sockaddr_in server_address;
static struct sockaddr_in client_address;

static int _rtp_address( struct sockaddr_in * address, short port ) {
  address->sin_family = AF_INET;
  address->sin_port   = port;
  ASSERT_NOT_EQUAL( inet_aton( RTP_ADDRESS, &(address->sin_addr) ), 0,
                    "Could not create internet address." );
  return 0;
}

static int _rtp_socket( int * s, struct sockaddr_in * address ) {
  int socket_id = socket( AF_INET, SOCK_DGRAM, 0 );

  ASSERT_GREATER_OR_EQUAL( socket_id, 0, "Could not create socket." );

  ASSERT_NO_ERROR( bind( socket_id, (void*)address, sizeof(struct sockaddr_in) ),
                   "Could not bind socket." );

  *s = socket_id;
  return 0;
}

/**
 * Test that RTP sessions can be created and set up.
 */
int test001_rtp( void ) {
  int s;
  ASSERT_NO_ERROR( _rtp_address( &server_address, RTP_SERVER_PORT ),
                   "Could not fill out server address." );
  ASSERT_NO_ERROR( _rtp_socket( &s, &server_address ),
                   "Could not create server socket." );

  session = RTPSessionCreate( s );
  ASSERT_NOT_EQUAL( session, NULL, "Could not create RTP session." );
  return 0;
}

/**
 * Test that peers can be added, removed and looked up.
 */
int test002_rtp( void ) {
  struct RTPPeer * peer;
  struct RTPPeer * p;
  ASSERT_NO_ERROR( _rtp_address( &client_address, RTP_CLIENT_PORT ),
                   "Could not fill out client address." );

  peer = RTPPeerCreate( RTP_CLIENT_SSRC, sizeof(struct sockaddr_in), (void*) &client_address );

  ASSERT_NOT_EQUAL( peer, NULL, "Could not create RTP peer." );

  ASSERT_NO_ERROR( RTPSessionAddPeer( session, peer ), "Could not add peer." );
  ASSERT_NO_ERROR( RTPSessionFindPeerBySSRC( session, &p, RTP_CLIENT_SSRC ), "Could find peer by SSRC." );
  ASSERT_EQUAL( peer, p, "Lookup by SSRC returned wrong peer." );
  ASSERT_NO_ERROR( RTPSessionFindPeerByAddress( session, &p, sizeof(struct sockaddr_in), (void*) &client_address ),
                   "Could find peer by address." );
  ASSERT_EQUAL( peer, p, "Lookup by address returned wrong peer." );
  ASSERT_NO_ERROR( RTPSessionRemovePeer( session, peer ), "Could not remove peer." );
  ASSERT_ERROR( RTPSessionFindPeerBySSRC( session, &p, RTP_CLIENT_SSRC ), "Peer was not removed." );
  ASSERT_ERROR( RTPSessionFindPeerByAddress( session, &p, sizeof(struct sockaddr_in), (void*) &client_address ),
               "Could find peer by address." );

  ASSERT_NO_ERROR( RTPSessionAddPeer( session, peer ), "Could not add peer." );
  RTPPeerRelease( peer );
  return 0;
}

/**
 * Test that messages can be sent via an RTP session and are
 * syntactically correct.
 */
int test003_rtp( void ) {
  struct RTPPeer * peer;
  struct RTPPacketInfo info;
  unsigned char send_buffer[8] = { 0, 1, 2, 3, 4, 5, 6, 7 };
  unsigned char recv_buffer[32];
  int s, bytes;
  ASSERT_NO_ERROR( _rtp_socket( &s, &client_address ),
                   "Could not create client socket." );

  info.padding = 0;
  info.extension = 0;
  info.csrc_count = 2;
  info.marker = 0;
  info.payload_type = 96;
  info.sequence_number = 0x1234;
  info.timestamp = 0;
  info.csrc[0] = 0x80706050;
  info.csrc[1] = 0x04030201;
  info.size = 0;
  info.data = NULL;

  ASSERT_NO_ERROR( RTPSessionFindPeerBySSRC( session, &peer, RTP_CLIENT_SSRC ),
                   "Could not find peer." );
  ASSERT_NO_ERROR( RTPSessionSendToPeer( session, peer, sizeof(send_buffer), &send_buffer[0], &info ),
                   "Could not send payload to peer." );

  bytes = recv( s, &recv_buffer[0], sizeof(recv_buffer), 0 );
  ASSERT_EQUAL( bytes, 28, "Received message of unexpected size." );
  ASSERT_EQUAL( recv_buffer[0], 0x82, "First byte (V, P, X, CC) of RTP message has incorrect value." );
  ASSERT_EQUAL( recv_buffer[1],   96, "Second byte (M, PT) of RTP message has incorrect value." );
  // ASSERT_EQUAL( recv_buffer[2], 0x34, "Third byte (Seqnum LSB) of RTP message has incorrect value." );
  // ASSERT_EQUAL( recv_buffer[3], 0x12, "Forth byte (Seqnum MSB) of RTP message has incorrect value." );
  return 0;
}

/**
 * Test that messages can be received via an RTP session and
 * are correctly interpreted.
 */
int test004_rtp( void ) {
  return 0;
}

/**
 * Test that malicious packets don't mess up the RTP session.
 */
int test005_rtp( void ) {
  return 0;
}


/**
 * Test that an RTP session can be properly teared down.
 */
int test006_rtp( void ) {
  RTPSessionRelease( session );

  return 0;
}
