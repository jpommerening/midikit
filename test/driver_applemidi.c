#include <stdlib.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/select.h>
#include <sys/time.h>
#include "test.h"
#include "driver/applemidi/applemidi.h"

#define CLIENT_SSRC 0x5d72fb43
#define CLIENT_ADDRESS "127.0.0.1"
#define CLIENT_CONTROL_PORT 5104
#define CLIENT_RTP_PORT CLIENT_CONTROL_PORT + 1

#define SERVER_ADDRESS CLIENT_ADDRESS
#define SERVER_CONTROL_PORT 5004
#define SERVER_RTP_PORT SERVER_CONTROL_PORT + 1

static struct MIDIDriverAppleMIDI * driver = NULL;

static int client_control_socket = 0;
static int client_rtp_socket = 0;

static int _fillin_invitation_accepted( unsigned char * buf ) {
  buf[2] = 'O';
  buf[3] = 'K';
  /* leave version & token as is */
  buf[12] = 0xff & (CLIENT_SSRC >> 24);
  buf[13] = 0xff & (CLIENT_SSRC >> 16);
  buf[14] = 0xff & (CLIENT_SSRC >> 8);
  buf[15] = 0xff &  CLIENT_SSRC;
  buf[16] = 'T';
  buf[17] = 'e';
  buf[18] = 's';
  buf[19] = 't';
  buf[20] = '\0';
  return 0;
}

static int _check_socket_in( int fd ) {
  static struct timeval tv = { 1, 0 };
  static fd_set fds;
  FD_ZERO( &fds );
  FD_SET( fd, &fds );
  select( fd+1, &fds, NULL, NULL, &tv );
  return FD_ISSET( fd, &fds );
}

/**
 * Test that AppleMIDI sessions can be created.
 */
int test001_applemidi( void ) {
  struct sockaddr_in client_addr, server_addr;
  unsigned char buf[32];

  driver = MIDIDriverAppleMIDICreate( "My MIDI Session", SERVER_CONTROL_PORT );

  client_control_socket = socket( PF_INET, SOCK_DGRAM, 0 );
  ASSERT_NOT_EQUAL( client_control_socket, 0, "Could not create control socket." );
  client_rtp_socket = socket( PF_INET, SOCK_DGRAM, 0 );
  ASSERT_NOT_EQUAL( client_rtp_socket, 0, "Could not create RTP socket." );

  client_addr.sin_family = AF_INET;
  client_addr.sin_addr.s_addr = INADDR_ANY;

  client_addr.sin_port = htons( CLIENT_CONTROL_PORT );
  ASSERT_NO_ERROR( bind( client_control_socket, (struct sockaddr *) &client_addr, sizeof(client_addr) ),
                   "Could not bind control socket." );

  client_addr.sin_port = htons( CLIENT_RTP_PORT );
  ASSERT_NO_ERROR( bind( client_rtp_socket, (struct sockaddr *) &client_addr, sizeof(client_addr) ),
                   "Could not bind rtp socket." );

  ASSERT_NOT_EQUAL( driver, NULL, "Could not create AppleMIDI driver." );

  ASSERT_NO_ERROR( MIDIDriverAppleMIDIAddPeer( driver, CLIENT_ADDRESS, CLIENT_CONTROL_PORT ),
                   "Could not add client." );

  /* hope for invitation on control socket */
  ASSERT( _check_socket_in( client_control_socket ), "Expected message on client control socket." );
  recv( client_control_socket, &(buf[0]), sizeof(buf), 0 );

  /* check packet header */
  ASSERT_EQUAL( buf[0], 0xff, "Received wrong AppleMIDI signature." );
  ASSERT_EQUAL( buf[1], 0xff, "Received wrong AppleMIDI signature." );
  ASSERT_EQUAL( buf[2], 'I', "Received wrong AppleMIDI command." );
  ASSERT_EQUAL( buf[3], 'N', "Received wrong AppleMIDI command." );

  _fillin_invitation_accepted( &(buf[0]) );
  server_addr.sin_family = AF_INET;
  inet_aton( "127.0.0.1", &(server_addr.sin_addr) );
  server_addr.sin_port = htons( SERVER_CONTROL_PORT );

  sendto( client_control_socket, &(buf[0]), 21, 0,
          (struct sockaddr *) &server_addr, sizeof(server_addr) );

  ASSERT_NO_ERROR( MIDIDriverAppleMIDIReceive( driver ), "Could not receive accepted invitation on control port." );

  /* the driver should now have sent an invitation on the rtp port */
  ASSERT( _check_socket_in( client_rtp_socket ), "Expected message on client control socket." );
  recv( client_rtp_socket, &(buf[0]), sizeof(buf), 0 );

  ASSERT_EQUAL( buf[0], 0xff, "Received wrong AppleMIDI signature." );
  ASSERT_EQUAL( buf[1], 0xff, "Received wrong AppleMIDI signature." );
  ASSERT_EQUAL( buf[2], 'I', "Received wrong AppleMIDI command." );
  ASSERT_EQUAL( buf[3], 'N', "Received wrong AppleMIDI command." );

  _fillin_invitation_accepted( &(buf[0]) );
  server_addr.sin_port = htons( SERVER_RTP_PORT );

  printf( "send ok to %s:%i\n", inet_ntoa( server_addr.sin_addr ), htons( server_addr.sin_port ) );
  sendto( client_rtp_socket, &(buf[0]), 21, 0,
          (struct sockaddr *) &server_addr, sizeof(server_addr) );

  ASSERT_NO_ERROR( MIDIDriverAppleMIDIReceive( driver ), "Could not receive accepted invitation on RTP port." );

  return 0;
}

/**
 * Test that AppleMIDI sessions can be torn down and
 * clients receive the proper ENDSESSION commands.
 */
int test005_applemidi( void ) {

  MIDIDriverAppleMIDIRelease( driver );

  close( client_control_socket );
  close( client_rtp_socket );
  return 0;
}
