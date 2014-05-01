#include <stdlib.h>
#include <unistd.h>
#include <arpa/inet.h>
#include "test.h"
#include "driver/common/rtp.h"
#include <netdb.h>
#include <string.h>

#define RTP_ADDRESS "::1"
static struct sockaddr_in6 server_address;
static struct sockaddr_in6 client_address;

#define RTP_CLIENT_PORT 5204
#define RTP_CLIENT_SSRC 123456789
#define RTP_SERVER_PORT 5104

static struct RTPSession * _session = NULL;

static int _rtp_addressv6( struct sockaddr_in6 * address, short port ) 
{
  struct addrinfo hints;
  struct addrinfo * result;
  char portname[8];

  memset(&hints, 0, sizeof(hints));
  sprintf( &(portname[0]), "%hu", port );

  if (getaddrinfo( RTP_ADDRESS , portname, &hints, &result) == 0) {
    memcpy ( address, (struct sockaddr_in6 *)result->ai_addr, sizeof(struct sockaddr_in6));
    freeaddrinfo(result);
  }

  return 0;
}

static int _rtp_socketv6( int * s, struct sockaddr_in6 * address ) 
{
  int socket_id = socket( AF_INET6, SOCK_DGRAM, 0 );

  ASSERT_GREATER_OR_EQUAL( socket_id, 0, "Could not create socket." );

  ASSERT_NO_ERROR( bind( socket_id, (void*)address, sizeof(struct sockaddr_in6) ),
      "Could not bind socket." );
  *s = socket_id;
  return 0;
}

/**
 * Test that RTP sessions can be created and set up.
 */
int test001_rtpv6( void ) {
  int s;
  unsigned long ssrc;
  ASSERT_NO_ERROR( _rtp_addressv6( &server_address, RTP_SERVER_PORT ),
                   "Could not fill out server address." );
  ASSERT_NO_ERROR( _rtp_socketv6( &s, &server_address ),
                   "Could not create server socket." );

  _session = RTPSessionCreate( s );
  ASSERT_NOT_EQUAL( _session, NULL, "Could not create RTP session." );

  RTPSessionGetSSRC( _session, &ssrc );
  printf( "SSRC: 0x%lx\n", ssrc );
  return 0;
}

/**
 * Test that peers can be added, removed and looked up.
 */
int test002_rtpv6( void ) {
  struct RTPPeer * peer;
  struct RTPPeer * p;
  ASSERT_NO_ERROR( _rtp_addressv6( &client_address, RTP_CLIENT_PORT ),
                   "Could not fill out client address." );

  peer = RTPPeerCreate( RTP_CLIENT_SSRC, sizeof(struct sockaddr_in6), (void*) &client_address );

  ASSERT_NOT_EQUAL( peer, NULL, "Could not create RTP peer." );

  ASSERT_NO_ERROR( RTPSessionAddPeer( _session, peer ), "Could not add peer." );
  ASSERT_NO_ERROR( RTPSessionFindPeerBySSRC( _session, &p, RTP_CLIENT_SSRC ), "Could find peer by SSRC." );
  ASSERT_EQUAL( peer, p, "Lookup by SSRC returned wrong peer." );
  ASSERT_NO_ERROR( RTPSessionFindPeerByAddress( _session, &p, sizeof(struct sockaddr_in6), (void*) &client_address ),
                   "Could find peer by address." );
  ASSERT_EQUAL( peer, p, "Lookup by address returned wrong peer." );
  p=NULL;
  ASSERT_NO_ERROR( RTPSessionNextPeer( _session, &p ), "Could not get first peer." );
  ASSERT_EQUAL( peer, p, "First peer returned wrong peer." );
  ASSERT_NO_ERROR( RTPSessionNextPeer( _session, &p ), "Could not get next peer." );
  ASSERT_EQUAL( NULL, p, "First peer returned wrong peer." );
  
  ASSERT_NO_ERROR( RTPSessionRemovePeer( _session, peer ), "Could not remove peer." );
  ASSERT_ERROR( RTPSessionFindPeerBySSRC( _session, &p, RTP_CLIENT_SSRC ), "Peer was not removed." );
  ASSERT_ERROR( RTPSessionFindPeerByAddress( _session, &p, sizeof(struct sockaddr_in6), (void*) &client_address ),
               "Could find peer by address." );

  ASSERT_NO_ERROR( RTPSessionAddPeer( _session, peer ), "Could not add peer." );
  RTPPeerRelease( peer );
  return 0;
}

/**
 * Test that messages can be sent via an RTP session and are
 * syntactically correct.
 */
int test003_rtpv6( void ) {
  struct RTPPeer * peer;
  struct RTPPacketInfo info;
  struct iovec iov;
  unsigned char send_buffer[8] = { 0, 1, 2, 3, 4, 5, 6, 7 };
  unsigned char recv_buffer[32];
  int s, bytes;
  ASSERT_NO_ERROR( _rtp_socketv6( &s, &client_address ),
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
  info.iovlen = 1;
  info.iov    = &iov;

  ASSERT_NO_ERROR( RTPSessionFindPeerBySSRC( _session, &peer, RTP_CLIENT_SSRC ),
                   "Could not find peer." );

  info.peer = peer;
  iov.iov_len  = sizeof(send_buffer);
  iov.iov_base = &(send_buffer[0]);

  ASSERT_NO_ERROR( RTPSessionSendPacket( _session, &info ),
                   "Could not send payload to peer." );

  bytes = recv( s, &recv_buffer[0], sizeof(recv_buffer), 0 );
  ASSERT_EQUAL( bytes, 28, "Received message of unexpected size." );
  ASSERT_EQUAL( recv_buffer[0], 0x82, "First byte (V, P, X, CC) of RTP message has incorrect value." );
  ASSERT_EQUAL( recv_buffer[1],   96, "Second byte (M, PT) of RTP message has incorrect value." );
/*ASSERT_EQUAL( recv_buffer[2], 0x34, "Third byte (Seqnum LSB) of RTP message has incorrect value." );
  ASSERT_EQUAL( recv_buffer[3], 0x12, "Forth byte (Seqnum MSB) of RTP message has incorrect value." );*/
  close( s );
  return 0;
}

/**
 * Test that messages can be received via an RTP session and
 * are correctly interpreted.
 */
int test004_rtpv6( void ) {
  struct RTPPeer * peer;
  struct RTPPacketInfo info;
  unsigned char send_buffer[20] = { 0xa0, 96,   /* V=2, P=0, X=0, CC=0, PT=96 */
                                    0x34, 0x12, /* Seqnum = 0x1234 */
                                    5, 6, 7, 8, /* timestamp */
                                  ( RTP_CLIENT_SSRC >> 24 ) & 0xff,
                                  ( RTP_CLIENT_SSRC >> 16 ) & 0xff,
                                  ( RTP_CLIENT_SSRC >> 8 ) & 0xff,
                                  ( RTP_CLIENT_SSRC ) & 0xff,
                                  1, 2, 3, 4,
                                  0xca, 0xfe, 0x00, 4 };
  unsigned char recv_buffer[8] = { 0 };
  int s;
  ASSERT_NO_ERROR( _rtp_socketv6( &s, &client_address ),
                   "Could not create client socket." );

  ASSERT_NO_ERROR( RTPSessionFindPeerBySSRC( _session, &peer, RTP_CLIENT_SSRC ),
                   "Could not find peer." );

  sendto( s, &send_buffer[0], sizeof(send_buffer), 0,
          (struct sockaddr *) &server_address, sizeof(server_address) );
  ASSERT_NO_ERROR( RTPSessionReceive( _session, sizeof(recv_buffer), &recv_buffer[0], &info ),
                   "Could not receive payload from peer." );

  ASSERT_EQUAL( info.payload_size, 4, "Received message of unexpected size." );
  ASSERT_EQUAL( info.padding, 4, "Message has unexpected padding." );
  ASSERT_EQUAL( info.ssrc, RTP_CLIENT_SSRC, "Message has unexpected SSRC." );
  ASSERT_EQUAL( recv_buffer[0], 1, "First byte of RTP payload has incorrect value." );
  ASSERT_EQUAL( recv_buffer[1], 2, "Second byte of RTP payload has incorrect value." );
  ASSERT_EQUAL( recv_buffer[2], 3, "Third byte of RTP payload has incorrect value." );
  ASSERT_EQUAL( recv_buffer[3], 4, "Fourth byte of RTP payload has incorrect value." );
  close( s );
  return 0;
}

/**
 * Test that malicious packets don't mess up the RTP session.
 */
int test005_rtpv6( void ) {
  return 0;
}


/**
 * Test that an RTP session can be properly teared down.
 */
int test006_rtpv6( void ) {
  if( _session != NULL ) {
    RTPSessionRelease( _session );
  }

  return 0;
}
