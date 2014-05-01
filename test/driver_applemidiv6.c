#include <stdlib.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/select.h>
#include <sys/time.h>
#include "test.h"
#include "midi/type.h"
#include "midi/port.h"
#include "midi/event.h"
#include "midi/driver.h"
#include "midi/message.h"
#include "midi/runloop.h"
#include "driver/applemidi/applemidi.h"

#define CLIENT_SSRC 0x5d72fb43

#define CLIENT_ADDRESS "::1"

#define CLIENT_CONTROL_PORT 5104
#define CLIENT_RTP_PORT CLIENT_CONTROL_PORT + 1

#define SERVER_ADDRESS CLIENT_ADDRESS
#define SERVER_CONTROL_PORT 5204
#define SERVER_RTP_PORT SERVER_CONTROL_PORT + 1

static struct MIDIDriverAppleMIDI * driver = NULL;

static int client_control_socket = 0;
static int client_rtp_socket = 0;

static struct sockaddr_in6 client_addr;
static struct sockaddr_in6 server_addr;

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

static int _n_msg = 0;
static int _receive( void * target, void * source, struct MIDITypeSpec * type, void * data ) {
  struct MIDIMessage * message;
  char * buffer;
  int i;
  if( target != &_n_msg ) {
    printf( "Incorrect port target.\n" );
  }

  if( type == MIDIMessageType ) {
    message = data;
    printf( "Received message!\n" );
    MIDIMessageRelease( message );
    _n_msg++;
  } else if( type == MIDIEventType ) {
    printf( "Received event!\n" );
  } else {
    buffer = data;
    printf( "Received unknown type '%p':\n", type );
    if( type == NULL ) return 0;
    for( i=0; i<type->size; i++ ) {
      if( (i+1)%8 == 0 || (i+1) == type->size ) {
        if( buffer[i-(i%8)] < 128 ) {
          printf( "0x%02x | %s\n", buffer[i], buffer+i-(i%8) );
        } else {
          printf( "0x%02x\n", buffer[i] );
        }
      } else {
        printf( "0x%02x ", buffer[i] );
      }
    }
    return 1;
  }
  return 0;
}

static struct MIDIPort * _port;

/**
 * Test that AppleMIDI sessions can be created.
 */
int test001_applemidiv6( void ) {
  unsigned char buf[32];
  struct MIDIPort * port;
  char straddr[INET6_ADDRSTRLEN];

  driver = MIDIDriverAppleMIDICreate( "My MIDI Session", SERVER_CONTROL_PORT );
  ASSERT_NOT_EQUAL( driver, NULL, "Could not create AppleMIDI driver." );

  ASSERT_NO_ERROR( MIDIDriverGetPort( driver, &port ), "Could not get driver port." );

  _port = MIDIPortCreate( "AppleMIDI test port", MIDI_PORT_IN, &_n_msg, &_receive );
  ASSERT_NOT_EQUAL( _port, NULL, "Could not create test port." );

  ASSERT_NO_ERROR( MIDIPortConnect( port, _port ), "Could not connect ports." );

  client_control_socket = socket( PF_INET6, SOCK_DGRAM, 0 );
  ASSERT_NOT_EQUAL( client_control_socket, 0, "Could not create control socket." );
  client_rtp_socket = socket( PF_INET6, SOCK_DGRAM, 0 );
  ASSERT_NOT_EQUAL( client_rtp_socket, 0, "Could not create RTP socket." );

  client_addr.sin6_family = AF_INET6;
  client_addr.sin6_addr = in6addr_any;

  client_addr.sin6_port = htons( CLIENT_CONTROL_PORT );
  ASSERT_NO_ERROR( bind( client_control_socket, (struct sockaddr *) &client_addr, sizeof(client_addr) ),
                   "Could not bind control socket." );

  client_addr.sin6_port = htons( CLIENT_RTP_PORT );
  ASSERT_NO_ERROR( bind( client_rtp_socket, (struct sockaddr *) &client_addr, sizeof(client_addr) ),
                   "Could not bind rtp socket." );

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

  server_addr.sin6_family = AF_INET6;
  inet_pton(AF_INET6, "::1",
                    &(server_addr.sin6_addr));
  server_addr.sin6_port = htons( SERVER_CONTROL_PORT );

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

  server_addr.sin6_port = htons( SERVER_RTP_PORT );
  printf( "send ok to %s:%i\n", inet_ntop( AF_INET6, &server_addr.sin6_addr, straddr, sizeof(straddr)), htons( server_addr.sin6_port ) );

  sendto( client_rtp_socket, &(buf[0]), 21, 0,
          (struct sockaddr *) &server_addr, sizeof(server_addr) );

  ASSERT_NO_ERROR( MIDIDriverAppleMIDIReceive( driver ), "Could not receive accepted invitation on RTP port." );

  return 0;
}

/**
 * Test that the AppleMIDI sockets can be accessed.
 */
int test002_applemidiv6( void ) {
  int server_control_socket, server_rtp_socket;

  ASSERT_NO_ERROR( MIDIDriverAppleMIDIGetControlSocket( driver, &server_control_socket ), "Could not get control socket." );
  ASSERT_NO_ERROR( MIDIDriverAppleMIDIGetRTPSocket( driver, &server_rtp_socket ), "Could not get rtp socket." );

  return 0;
}

/**
 * Test that the AppleMIDI driver can be used as a runloop source.
 */
int test003_applemidiv6( void ) {
  struct MIDIRunloopSource * source;
  struct MIDIRunloop * runloop;
  unsigned char buf[36] = { 0 };

  runloop = MIDIRunloopCreate( NULL );
  ASSERT_NOT_EQUAL( runloop, NULL, "Could not create runloop." );
  ASSERT_NO_ERROR( MIDIDriverAppleMIDIGetRunloopSource( driver, &source ), "Could not create runloop source." );
  ASSERT_NO_ERROR( MIDIRunloopAddSource( runloop, source ), "Could not add source to runloop." );
  ASSERT_NO_ERROR( MIDIRunloopStep( runloop ), "Could not step through runloop." );
  ASSERT_NO_ERROR( MIDIRunloopRemoveSource( runloop, source ), "Could not remove source from runloop." );

  /* answer sync request */
  _fillin_sync( &(buf[0]), 1 );
  ASSERT_EQUAL( 36, sendto( client_rtp_socket, &(buf[0]), sizeof(buf), 0,
                (struct sockaddr *) &server_addr, sizeof(server_addr) ), "Could not send sync." );

  ASSERT_NO_ERROR( MIDIDriverAppleMIDIGetRunloopSource( driver, &source ), "Could not recreate runloop source." );
  ASSERT_NO_ERROR( MIDIRunloopAddSource( runloop, source ), "Could not add source to runloop." );

  ASSERT_NO_ERROR( MIDIRunloopStep( runloop ), "Could not step through runloop." );
  ASSERT_NO_ERROR( MIDIRunloopStep( runloop ), "Could not step through runloop." );
  ASSERT( _check_socket_in( client_rtp_socket ), "Expected message on client control socket." );
  ASSERT_EQUAL( 36, recv( client_rtp_socket, &(buf[0]), sizeof(buf), 0), "Did not receive synchronization answer." );
  ASSERT_NO_ERROR( MIDIRunloopStep( runloop ), "Could not step through runloop." );
  ASSERT_NO_ERROR( MIDIRunloopStep( runloop ), "Could not step through runloop." );
  ASSERT_NO_ERROR( MIDIRunloopStep( runloop ), "Could not step through runloop." );

  MIDIRunloopRelease( runloop );
  return 0;
}

/**
 * Test that RTP MIDI messages can be sent.
 */
int test004_applemidiv6( void ) {
  struct MIDIMessage * messages[3];
  unsigned char buffer[128];
  unsigned char expect[128] = {
    /* RTP header / seqnum, random */
    0x80, 0x60, 0x00, 0x00,
    /* timestamp, random */
    0x00, 0x00, 0x00, 0x00,
    /* SSRC */
    0x00, 0x00, 0x00, 0x00,
    /* MIDI header, Z bit (2) is set */
    0x0b,
    /* MIDI payload, first timestamp is zero */
          0x90, 0x42, 0x68,
    0x00, 0xa0, 0x42, 0x78,
    0x00, 0x80, 0x42, 0x68
  };
  int i;
  unsigned long long ssrc;
  size_t bytes;
  MIDIChannel  channel = MIDI_CHANNEL_1;
  MIDIKey      key = 66;
  MIDIVelocity velocity = 104;
  MIDIPressure pressure = 120;

  messages[0] = MIDIMessageCreate( MIDI_STATUS_NOTE_ON );
  messages[1] = MIDIMessageCreate( MIDI_STATUS_POLYPHONIC_KEY_PRESSURE );
  messages[2] = MIDIMessageCreate( MIDI_STATUS_NOTE_OFF );
  MIDIMessageSet( messages[0], MIDI_CHANNEL, sizeof(MIDIChannel), &channel );
  MIDIMessageSet( messages[1], MIDI_CHANNEL, sizeof(MIDIChannel), &channel );
  MIDIMessageSet( messages[2], MIDI_CHANNEL, sizeof(MIDIChannel), &channel );
  MIDIMessageSet( messages[0], MIDI_KEY, sizeof(MIDIKey), &key );
  MIDIMessageSet( messages[1], MIDI_KEY, sizeof(MIDIKey), &key );
  MIDIMessageSet( messages[2], MIDI_KEY, sizeof(MIDIKey), &key );
  MIDIMessageSet( messages[0], MIDI_VELOCITY, sizeof(MIDIVelocity), &velocity );
  MIDIMessageSet( messages[1], MIDI_PRESSURE, sizeof(MIDIPressure), &pressure );
  MIDIMessageSet( messages[2], MIDI_VELOCITY, sizeof(MIDIVelocity), &velocity );

  ASSERT_NO_ERROR( MIDIDriverAppleMIDISendMessage( driver, messages[0] ), "Could not queue midi message 0." );
  ASSERT_NO_ERROR( MIDIDriverAppleMIDISendMessage( driver, messages[1] ), "Could not queue midi message 1." );
  ASSERT_NO_ERROR( MIDIDriverAppleMIDISendMessage( driver, messages[2] ), "Could not queue midi message 2." );
  MIDIMessageRelease( messages[0] ); messages[0] = NULL;
  MIDIMessageRelease( messages[1] ); messages[1] = NULL;
  MIDIMessageRelease( messages[2] ); messages[2] = NULL;

  ASSERT_NO_ERROR( MIDIDriverAppleMIDISend( driver ), "Could not send queued messages." );

  ASSERT( _check_socket_in( client_rtp_socket ), "Expected message on client control socket." );
  bytes = recv( client_rtp_socket, &(buffer[0]), sizeof(buffer), 0 );
  ASSERT_GREATER_OR_EQUAL( bytes, 23, "Could not received RTP MIDI packet from AppleMIDI driver." );

  for( i=0; i<bytes; i++ ) {
    if( (i+1)%8 == 0 || (i+1) == bytes ) {
      printf( "0x%02x\n", buffer[i] );
    } else {
      printf( "0x%02x ", buffer[i] );
    }
  }

  ssrc = ( buffer[8] << 24)
       | ( buffer[9] << 16 )
       | ( buffer[10] << 8 )
       | ( buffer[11] );

  ASSERT_EQUAL( 24, sendto( client_rtp_socket, &(expect[0]), 24, 0,
                (struct sockaddr *) &server_addr, sizeof(server_addr) ), "Could not send RTP-MIDI." );
  usleep( 1000 );
  ASSERT_NO_ERROR( MIDIDriverAppleMIDIReceive( driver ), "Could not receive sent messages." );
  ASSERT_EQUAL( _n_msg, 3, "Received wrong number of messages." );
  return 0;
}

/**
 * Test that AppleMIDI sessions can be torn down and
 * clients receive the proper ENDSESSION commands.
 */
int test005_applemidiv6( void ) {
  MIDIDriverRelease( driver );

  close( client_control_socket );
  close( client_rtp_socket );
  return 0;
}
