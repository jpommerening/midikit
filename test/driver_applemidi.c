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
#define SERVER_CONTROL_PORT 5204
#define SERVER_RTP_PORT SERVER_CONTROL_PORT + 1

static struct MIDIDriverAppleMIDI * driver = NULL;

static int client_control_socket = 0;
static int client_rtp_socket = 0;

static struct sockaddr_in client_addr;
static struct sockaddr_in server_addr;

static int _fillin_invitation_accepted( unsigned char * buf ) {
  buf[0]  = 0xff;
  buf[1]  = 0xff;
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

static int _fillin_sync( unsigned char * buf, int count ) {
  int offset = 12 + (8*count);
  struct timeval tv = { 0, 0 };
  unsigned long long ts;
  gettimeofday( &tv, NULL );
  ts = tv.tv_sec * 1000 + tv.tv_usec;

  buf[0]  = 0xff;
  buf[1]  = 0xff;
  buf[2]  = 'C';
  buf[3]  = 'K';
  buf[4]  = 0xff & (CLIENT_SSRC >> 24);
  buf[5]  = 0xff & (CLIENT_SSRC >> 16);
  buf[6]  = 0xff & (CLIENT_SSRC >> 8);
  buf[7]  = 0xff &  CLIENT_SSRC;
  buf[8]  = count;
  buf[9]  = 0;
  buf[10] = 0;
  buf[11] = 0;

  buf[offset+0] = (ts >> 56) & 0xff;
  buf[offset+1] = (ts >> 48) & 0xff;
  buf[offset+2] = (ts >> 40) & 0xff;
  buf[offset+3] = (ts >> 32) & 0xff;
  buf[offset+4] = (ts >> 24) & 0xff;
  buf[offset+5] = (ts >> 16) & 0xff;
  buf[offset+6] = (ts >>  8) & 0xff;
  buf[offset+7] =  ts        & 0xff;
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
 * Test that the AppleMIDI sockets can be accessed.
 */
int test002_applemidi( void ) {
  int server_control_socket, server_rtp_socket;

  ASSERT_NO_ERROR( MIDIDriverAppleMIDIGetControlSocket( driver, &server_control_socket ), "Could not get control socket." );
  ASSERT_NO_ERROR( MIDIDriverAppleMIDIGetRTPSocket( driver, &server_rtp_socket ), "Could not get rtp socket." );

  return 0;
}

/**
 * Test that the AppleMIDI driver can be used as a runloop source.
 */
int test003_applemidi( void ) {
  struct MIDIRunloopSource * source;
  struct MIDIRunloop * runloop;
  unsigned char buf[36];

  runloop = MIDIRunloopCreate();
  ASSERT_NOT_EQUAL( runloop, NULL, "Could not create runloop." );
  ASSERT_NO_ERROR( MIDIDriverAppleMIDIGetRunloopSource( driver, &source ), "Could not create runloop source." );
  ASSERT_NO_ERROR( MIDIRunloopAddSource( runloop, source ), "Could not add source to runloop." );
  ASSERT_NO_ERROR( MIDIRunloopStep( runloop ), "Could not step through runloop." );
  ASSERT_NO_ERROR( MIDIRunloopRemoveSource( runloop, source ), "Could not remove source from runloop." );

  /* answer sync request */
  _fillin_sync( &(buf[0]), 1 );
  ASSERT_EQUAL( 36, sendto( client_rtp_socket, &(buf[0]), 36, 0,
                (struct sockaddr *) &server_addr, sizeof(server_addr) ), "Could not send sync." );
 
  ASSERT_NO_ERROR( MIDIDriverAppleMIDIGetRunloopSource( driver, &source ), "Could not recreate runloop source." );
  ASSERT_NO_ERROR( MIDIRunloopAddSource( runloop, source ), "Could not add source to runloop." );

  ASSERT_NO_ERROR( MIDIRunloopStep( runloop ), "Could not step through runloop." );
  ASSERT_NO_ERROR( MIDIRunloopStep( runloop ), "Could not step through runloop." );
  source->write = NULL;
  ASSERT_NO_ERROR( MIDIRunloopStep( runloop ), "Could not step through runloop." );
  source->read = NULL;
  source->nfds = 0;
  ASSERT_NO_ERROR( MIDIRunloopStep( runloop ), "Could not step through runloop." );
  source->idle = NULL;
  ASSERT_NO_ERROR( MIDIRunloopStep( runloop ), "Could not step through runloop." );

  MIDIRunloopRelease( runloop );
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
