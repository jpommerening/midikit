#include <string.h>
#include <stdio.h>
#include <sys/select.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <errno.h>

#include "applemidi.h"
#include "driver/common/rtp.h"
#include "driver/common/rtpmidi.h"
#include "midi/driver.h"
#include "midi/message.h"
#include "midi/message_queue.h"
#include "midi/runloop.h"

#define APPLEMIDI_PROTOCOL_SIGNATURE          0xffff

#define APPLEMIDI_COMMAND_INVITATION          0x494e /** "IN" on control & rtp port */
#define APPLEMIDI_COMMAND_INVITATION_REJECTED 0x4e4f /** "NO" on control & rtp port */
#define APPLEMIDI_COMMAND_INVITATION_ACCEPTED 0x4f4b /** "OK" on control & rtp port */
#define APPLEMIDI_COMMAND_ENDSESSION          0x4259 /** "BY" on control port */
#define APPLEMIDI_COMMAND_SYNCHRONIZATION     0x434b /** "CK" on rtp port */
#define APPLEMIDI_COMMAND_RECEIVER_FEEDBACK   0x5253 /** "RS" on control port */

#define APPLEMIDI_CONTROL_SOCKET 0
#define APPLEMIDI_RTP_SOCKET     1

#define APPLEMIDI_MAX_MESSAGES_PER_PACKET 16
#define APPLEMIDI_MSG_BUFFER_SIZE 16

struct AppleMIDICommand {
  struct RTPPeer * peer; /* use peers sockaddr instead .. we get initialization problems otherwise */
  struct sockaddr_storage addr;
  socklen_t size;
/*unsigned short channel;*/
  unsigned short type;
  union {
    struct {
      unsigned long version;
      unsigned long token;
      unsigned long ssrc;
      char name[64];
    } session;
    struct {
      unsigned long ssrc;
      unsigned long count;
      unsigned long long timestamp1;
      unsigned long long timestamp2;
      unsigned long long timestamp3;
    } sync;
    struct {
      unsigned long ssrc;
      unsigned long seqnum;
    } feedback;
  } data;
};

struct AppleMIDIPeer {
  char * name;
  unsigned short port;
  unsigned long token;
  unsigned long ssrc;
  unsigned long long timestamp_delay;
  unsigned long long timestamp_diff;
  struct RTPMIDIPeer * rtp_peer;
};

/**
 * @brief MIDIDriver implementation using Apple's network MIDI protocol.
 */
struct MIDIDriverAppleMIDI {
  size_t refs;
  int control_socket;
  int rtp_socket;
  unsigned short port;
  unsigned char  accept;
  unsigned char  sync;
  unsigned long  token;
  char name[32];
  
  struct MIDIRunloopSource runloop_source;
  struct AppleMIDICommand  command;

  struct RTPPeer * peer;
  struct RTPSession * rtp_session;
  struct RTPMIDISession * rtpmidi_session;

  struct MIDIDriverDelegate * delegate;
  struct MIDIMessageQueue * in_queue;
  struct MIDIMessageQueue * out_queue;
};

static int _applemidi_read_fds( void * drv, int nfds, fd_set * fds );
static int _applemidi_write_fds( void * drv, int nfsd, fd_set * fds );
static int _applemidi_idle_timeout( void * drv, struct timespec * ts );

static int _applemidi_init_runloop_source( struct MIDIDriverAppleMIDI * driver ) {
  struct MIDIRunloopSource * source = &(driver->runloop_source);

  FD_ZERO( &(source->readfds) );
  FD_ZERO( &(source->writefds) );
  FD_SET( driver->control_socket, &(source->readfds) );
  FD_SET( driver->rtp_socket,     &(source->readfds) );
  FD_SET( driver->control_socket, &(source->writefds) );
  FD_SET( driver->rtp_socket,     &(source->writefds) );
  if( driver->control_socket > driver->rtp_socket ) {
    source->nfds = driver->control_socket + 1;
  } else {
    source->nfds = driver->rtp_socket + 1;
  }
  source->timeout.tv_sec  = 1;
  source->timeout.tv_nsec = 0;
  source->remain.tv_sec  = 1;
  source->remain.tv_nsec = 0;
  source->info = driver;

  source->read  = NULL;
  source->write = NULL;
  source->idle  = NULL;
  return 0;
}

/**
 * @brief Update the runloop source's handles.
 * - If the driver can accept more data, enable the runloop's @c read callback.
 * - If the driver needs to write data, enable the @c write callback.
 * @param driver The driver.
 * @retval 0 on success.
 * @retval >0 if something bad happened.
 */
static int _applemidi_update_runloop_source( struct MIDIDriverAppleMIDI * driver ) {
  size_t in = 0, out = 0;
  MIDIMessageQueueGetLength( driver->in_queue,  &in );
  MIDIMessageQueueGetLength( driver->out_queue, &out );
  if( (in==0) || driver->accept || (driver->sync>0) ) {
    driver->runloop_source.read = &_applemidi_read_fds;
  } else {
    driver->runloop_source.read = NULL;
  }
  if( (out==0) ) {
    driver->runloop_source.write = NULL;
  } else {
    driver->runloop_source.write = &_applemidi_write_fds;
  }
  if( (in==0) && (out==0) ) {
    driver->runloop_source.idle = &_applemidi_idle_timeout;
  } else {
    driver->runloop_source.idle = NULL;
  }
  return 0;
}


static int _applemidi_connect( struct MIDIDriverAppleMIDI * driver ) {
  struct sockaddr_in addr;
  int result;
  
  if( driver->control_socket <= 0 ) {
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = INADDR_ANY;
    addr.sin_port = htons( driver->port );

    driver->control_socket = socket( PF_INET, SOCK_DGRAM, 0 );
    result = bind( driver->control_socket, (struct sockaddr *) &addr, sizeof(addr) );
  }

  if( driver->rtp_socket <= 0 ) {
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = INADDR_ANY;
    addr.sin_port = htons( driver->port + 1 );

    driver->rtp_socket = socket( PF_INET, SOCK_DGRAM, 0 );
    result = bind( driver->rtp_socket, (struct sockaddr *) &addr, sizeof(addr) );
  }
  return result;
}

static int _applemidi_endsession( struct MIDIDriverAppleMIDI *, int, socklen_t, struct sockaddr * );
static int _applemidi_control_addr( socklen_t, struct sockaddr *, struct sockaddr * );

static int _applemidi_disconnect_peer( struct MIDIDriverAppleMIDI * driver, struct RTPPeer * peer ) {
  int result = 0;
  struct sockaddr * rtp_addr = NULL;
  struct sockaddr_storage addr;
  socklen_t size;
  if( RTPPeerGetAddress( peer, &size, &rtp_addr ) || rtp_addr == NULL ) {
    return 1;
  }
  _applemidi_control_addr( size, rtp_addr, (struct sockaddr *) &addr );
  result = _applemidi_endsession( driver, driver->control_socket, size, (struct sockaddr *) &addr );
  RTPSessionRemovePeer( driver->rtp_session, peer );
  return result;
}

static int _applemidi_disconnect( struct MIDIDriverAppleMIDI * driver, int fd ) {
  struct RTPPeer * peer = NULL;
  RTPSessionNextPeer( driver->rtp_session, &peer );
  while( peer != NULL ) {
    _applemidi_disconnect_peer( driver, peer );
    peer = NULL; /* peer was removed, find the new first */
    RTPSessionNextPeer( driver->rtp_session, &peer );
  }
  if( fd == driver->control_socket || fd == 0 ) {
    if( driver->control_socket > 0 ) {
      if( close( driver->control_socket ) ) {
        return 1;
      }
      driver->control_socket = 0;
    }
  }

  if( fd == driver->control_socket || fd == 0 ) {
    if( driver->rtp_socket > 0 ) {
      if( close( driver->rtp_socket ) ) {
        return 1;
      }
      driver->rtp_socket = 0;
    }
  }

  return 0;
}

static int _driver_send( void * driverp, struct MIDIMessage * message ) {
  return MIDIDriverAppleMIDISendMessage( driverp, message );
}

#pragma mark Creation and destruction
/**
 * @name Creation and destruction
 * Creating, destroying and reference counting of MIDIClock objects.
 * @{
 */

/**
 * @brief Create a MIDIDriverAppleMIDI instance.
 * Allocate space and initialize an MIDIDriverAppleMIDI instance.
 * @public @memberof MIDIDriverAppleMIDI
 * @return a pointer to the created driver structure on success.
 * @return a @c NULL pointer if the driver could not created.
 */
struct MIDIDriverAppleMIDI * MIDIDriverAppleMIDICreate( struct MIDIDriverDelegate * delegate, char * name, unsigned short port ) {
  struct MIDIDriverAppleMIDI * driver;
  unsigned long long ts;

  driver = malloc( sizeof( struct MIDIDriverAppleMIDI ) );
  if( driver == NULL ) return NULL;
  
  driver->refs = 1;
  driver->control_socket = 0;
  driver->rtp_socket     = 0;
  driver->port           = port;
  driver->accept         = 0;
  driver->sync           = 0;
  strncpy( &(driver->name[0]), name, sizeof(driver->name) );
  
  _applemidi_connect( driver );

  driver->peer = NULL;
  driver->rtp_session     = RTPSessionCreate( driver->rtp_socket );  
  driver->rtpmidi_session = RTPMIDISessionCreate( driver->rtp_session );
  driver->in_queue  = MIDIMessageQueueCreate();
  driver->out_queue = MIDIMessageQueueCreate();
  

  RTPSessionSetTimestampRate( driver->rtp_session, 44100.0 );
  RTPSessionGetTimestamp( driver->rtp_session, &ts );
  driver->token = ts;

  memset( &(driver->command), 0, sizeof(driver->command) );
  driver->command.peer = NULL;

  _applemidi_init_runloop_source( driver );

  driver->delegate = delegate;

  if( delegate != NULL ) {
    delegate->send = &_driver_send;
    delegate->implementation = driver;
  }
  return driver;
}

/**
 * @brief Destroy a MIDIDriverAppleMIDI instance.
 * Free all resources occupied by the driver.
 * @public @memberof MIDIDriverAppleMIDI
 * @param driver The driver.
 */
void MIDIDriverAppleMIDIDestroy( struct MIDIDriverAppleMIDI * driver ) {
  _applemidi_disconnect( driver, 0 );
  RTPMIDISessionRelease( driver->rtpmidi_session );
  RTPSessionRelease( driver->rtp_session );
  MIDIMessageQueueRelease( driver->in_queue );
  MIDIMessageQueueRelease( driver->out_queue );
  free( driver );
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

/** @} */

/**
 * @brief Set the base port to be used for session management.
 * The RTP port will be the control port plus one.
 * @public @memberof MIDIDriverAppleMIDI
 * @param driver The driver.
 * @param port The port.
 * @retval 0 On success.
 * @retval >0 If the port could not be set.
 */
int MIDIDriverAppleMIDISetPort( struct MIDIDriverAppleMIDI * driver, unsigned short port ) {
  if( port == driver->port ) return 0;
  /* reconnect if connected? */
  driver->port = port;
  return 0;
}

/**
 * @brief Get the port used for session management.
 * The RTP port can be computed by adding one.
 * @public @memberof MIDIDriverAppleMIDI
 * @param driver The driver.
 * @param port The port.
 * @retval 0 On success.
 * @retval >0 If the port could not be set.
 */
int MIDIDriverAppleMIDIGetPort( struct MIDIDriverAppleMIDI * driver, unsigned short * port ) {
  if( port == NULL ) return 1;
  *port = driver->port;
  return 0;
}

int MIDIDriverAppleMIDIAcceptFromNone( struct MIDIDriverAppleMIDI * driver ) {
  driver->accept = 0;
  return 0;
}

int MIDIDriverAppleMIDIAcceptFromAny( struct MIDIDriverAppleMIDI * driver ) {
  driver->accept = 0xff;
  return 0;
}

int MIDIDriverAppleMIDIAcceptFromPeer( struct MIDIDriverAppleMIDI * driver, char * address, unsigned short port ) {
  driver->accept = 1;
  return 1;
}

int MIDIDriverAppleMIDISetRTPSocket( struct MIDIDriverAppleMIDI * driver, int socket ) {
  if( socket == driver->rtp_socket ) return 0;
  int result = _applemidi_disconnect( driver, driver->rtp_socket );
  if( result == 0 ) {
    driver->rtp_socket = socket;
  }
  return result;
}

int MIDIDriverAppleMIDIGetRTPSocket( struct MIDIDriverAppleMIDI * driver, int * socket ) {
  *socket = driver->rtp_socket;
  return 0;
}

int MIDIDriverAppleMIDISetControlSocket( struct MIDIDriverAppleMIDI * driver, int socket ) {
  if( socket == driver->control_socket ) return 0;
  int result = _applemidi_disconnect( driver, driver->control_socket );
  if( result == 0 ) {
    driver->control_socket = socket;
  }
  return result;
}

int MIDIDriverAppleMIDIGetControlSocket( struct MIDIDriverAppleMIDI * driver, int * socket ) {
  *socket = driver->control_socket;
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
  return MIDIDriverDelegateReceiveMessage( driver->delegate, message );
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
  unsigned long long ts;
  RTPSessionGetTimestamp( driver->rtp_session, &ts );
  MIDIMessageSetTimestamp( message, ts );
  return MIDIMessageQueuePush( driver->out_queue, message );
}


static int _applemidi_init_addr_with_peer( struct AppleMIDICommand * command, struct RTPPeer * peer ) {
  struct sockaddr * addr;
  socklen_t size;

  RTPPeerGetAddress( peer, &size, &addr );
  memcpy( &(command->addr), addr, size );
  command->size = size;
  return 0;
}

/**
 * @brief Test incoming packets for the AppleMIDI signature.
 * Check if the data that is waiting on a socket begins with the special AppleMIDI signature (0xffff).
 * @private @memberof MIDIDriverAppleMIDI
 * @param fd The file descriptor to use for communication.
 * @retval 0 if the packet is AppleMIDI
 * @retval 1 if the packet is not AppleMIDI
 * @retval -1 if no signature data could be received
 */
static int _test_applemidi( int fd ) {
  ssize_t bytes;
  unsigned short buf[2];
  bytes = recv( fd, &buf, 4, MSG_PEEK );
  if( bytes != 4 ) return -1;
  if( ntohs(buf[0]) == APPLEMIDI_PROTOCOL_SIGNATURE ) {
    switch( ntohs(buf[1]) ) {
      case APPLEMIDI_COMMAND_INVITATION:
      case APPLEMIDI_COMMAND_INVITATION_ACCEPTED:
      case APPLEMIDI_COMMAND_INVITATION_REJECTED:
      case APPLEMIDI_COMMAND_RECEIVER_FEEDBACK:
      case APPLEMIDI_COMMAND_SYNCHRONIZATION:
      case APPLEMIDI_COMMAND_ENDSESSION:
        return 0;
    }
  }
  return 1;
}

/**
 * @brief Send the given AppleMIDI command.
 * Compose a message buffer and send the datagram to the given peer.
 * @private @memberof MIDIDriverAppleMIDI
 * @param driver The driver.
 * @param fd The file descriptor to use for communication.
 * @param command The command.
 * @retval 0 On success.
 * @retval >0 If the packet could not be sent.
 */
static int _applemidi_send_command( struct MIDIDriverAppleMIDI * driver, int fd, struct AppleMIDICommand * command ) {
  unsigned int ssrc;
  unsigned int msg[16];
  int len;

  msg[0] = htonl( ( APPLEMIDI_PROTOCOL_SIGNATURE << 16 ) | command->type );
  switch( command->type ) {
      case APPLEMIDI_COMMAND_INVITATION:
      case APPLEMIDI_COMMAND_INVITATION_ACCEPTED:
      case APPLEMIDI_COMMAND_INVITATION_REJECTED:
      case APPLEMIDI_COMMAND_ENDSESSION:
        ssrc   = command->data.session.ssrc;
        msg[1] = htonl( command->data.session.version );
        msg[2] = htonl( command->data.session.token );
        msg[3] = htonl( command->data.session.ssrc );
        if( command->data.session.name[0] != '\0' ) {
          len = strlen( command->data.session.name );
          memcpy( &(msg[4]), command->data.session.name, len );
          len += 16;
        } else {
          len  = 16;
        }
        break;
      case APPLEMIDI_COMMAND_SYNCHRONIZATION:
        ssrc   = command->data.sync.ssrc;
        msg[1] = htonl( command->data.sync.ssrc );
        msg[2] = htonl( command->data.sync.count << 24 );
        msg[3] = htonl( command->data.sync.timestamp1 >> 32 );
        msg[4] = htonl( command->data.sync.timestamp1 & 0xffffffff );
        msg[5] = htonl( command->data.sync.timestamp2 >> 32 );
        msg[6] = htonl( command->data.sync.timestamp2 & 0xffffffff );
        msg[7] = htonl( command->data.sync.timestamp3 >> 32 );
        msg[8] = htonl( command->data.sync.timestamp3 & 0xffffffff );
        len    = 36;
        break;
      case APPLEMIDI_COMMAND_RECEIVER_FEEDBACK:
        ssrc   = command->data.feedback.ssrc;
        msg[1] = htonl( command->data.feedback.ssrc );
        msg[2] = htonl( command->data.feedback.seqnum );
        len    = 12;
        break;
      default:
        return 1;
  }

  if( command->addr.ss_family == AF_INET ) {
    struct sockaddr_in * a = (struct sockaddr_in *) &(command->addr);
    printf( "send %i bytes to %s:%i on s(%i)\n", len, inet_ntoa( a->sin_addr ), ntohs( a->sin_port ), fd );
  } else {
    printf( "send %i bytes to <unknown addr family> on s(%i)\n", len, fd );
  }
  if( sendto( fd, &msg[0], len, 0,
              (struct sockaddr *) &(command->addr), command->size ) != len ) {
    return 1;
  } else {
    return 0;
  }
}

/**
 * @brief Receive an AppleMIDI command.
 * Receive a datagram and decompose the message into the message structure.
 * @private @memberof MIDIDriverAppleMIDI
 * @param driver The driver.
 * @param fd The file descriptor to use for communication.
 * @param command The command.
 * @retval 0 On success.
 * @retval >0 If the packet could not be sent.
 */
static int _applemidi_recv_command( struct MIDIDriverAppleMIDI * driver, int fd, struct AppleMIDICommand * command ) {
  unsigned int ssrc;
  unsigned int msg[16];
  int len;
  
  command->size = sizeof(command->addr);
  len = recvfrom( fd, &msg[0], sizeof(msg), 0,
                  (struct sockaddr *) &(command->addr), &(command->size) );
  if( command->addr.ss_family == AF_INET ) {
    struct sockaddr_in * a = (struct sockaddr_in *) &(command->addr);
    printf( "recv %i bytes from %s:%i on s(%i)\n", len, inet_ntoa( a->sin_addr ), ntohs( a->sin_port ), fd );
  } else {
    printf( "recv %i bytes from <unknown addr family> on s(%i)\n", len, fd );
  }

  command->type = ntohl( msg[0] ) & 0xffff;
  
  switch( command->type ) {
      case APPLEMIDI_COMMAND_INVITATION:
      case APPLEMIDI_COMMAND_INVITATION_ACCEPTED:
      case APPLEMIDI_COMMAND_INVITATION_REJECTED:
      case APPLEMIDI_COMMAND_ENDSESSION:
        if( len < 16 ) return 1;
        command->data.session.version = ntohl( msg[1] );
        command->data.session.token   = ntohl( msg[2] );
        command->data.session.ssrc    = ntohl( msg[3] );
        len -= 16;
        if( len > 0 ) {
          if( len > sizeof( command->data.session.name ) - 1 ) {
            len = sizeof( command->data.session.name ) - 1;
          }
          memcpy( &(command->data.session.name[0]), &msg[4], len ); 
          command->data.session.name[len] = '\0';
        }
        ssrc = command->data.session.ssrc;
        break;
      case APPLEMIDI_COMMAND_SYNCHRONIZATION:
        if( len != 36 ) return 1;
        command->data.sync.ssrc        = ntohl( msg[1] );
        command->data.sync.count       = ntohl( msg[2] ) >> 24;
        command->data.sync.timestamp1  = (unsigned long long) ntohl( msg[3] ) << 32;
        command->data.sync.timestamp1 += ntohl( msg[4] );
        command->data.sync.timestamp2  = (unsigned long long) ntohl( msg[5] ) << 32;
        command->data.sync.timestamp2 += ntohl( msg[6] );
        command->data.sync.timestamp3  = (unsigned long long) ntohl( msg[7] ) << 32;
        command->data.sync.timestamp3 += ntohl( msg[8] );
        ssrc = command->data.sync.ssrc;
        break;
      case APPLEMIDI_COMMAND_RECEIVER_FEEDBACK:
        if( len != 12 ) return 1;
        command->data.feedback.ssrc   = ntohl( msg[1] );
        command->data.feedback.seqnum = ntohl( msg[2] );
        ssrc = command->data.feedback.ssrc;
        break;
      default:
        return 1;
  }
  return 0;
}

/**
 * @brief Start or continue a synchronization session.
 * Continue a synchronization session identified by a given command.
 * The command must contain a pointer to a valid peer.
 * @param driver The driver.
 * @param fd The file descriptor to be used for communication.
 * @param command The previous sync command.
 * @retval 0 On success.
 * @retval >0 If the synchronization failed.
 */
static int _applemidi_sync( struct MIDIDriverAppleMIDI * driver, int fd, struct AppleMIDICommand * command ) {
  unsigned long ssrc;
  unsigned long long timestamp, diff;
  RTPSessionGetSSRC( driver->rtp_session, &ssrc );
  RTPSessionGetTimestamp( driver->rtp_session, &timestamp );

  if( command->type != APPLEMIDI_COMMAND_SYNCHRONIZATION || 
      command->data.sync.ssrc == ssrc ||
      command->data.sync.count > 2 ) {
    command->type = APPLEMIDI_COMMAND_SYNCHRONIZATION;
    command->data.sync.ssrc       = ssrc;
    command->data.sync.count      = 0;
    command->data.sync.timestamp1 = timestamp;
    
    driver->sync = 1;
    return _applemidi_send_command( driver, fd, command );
  } else {
    RTPSessionFindPeerBySSRC( driver->rtp_session, &(driver->peer), command->data.sync.ssrc );

    /* received packet from other peer */
    if( command->data.sync.count == 2 ) {
      /* compute media delay */
      diff = ( command->data.sync.timestamp3 - command->data.sync.timestamp1 ) / 2;
      /* approximate time difference between peer and self */
      diff = command->data.sync.timestamp3 + diff - timestamp;

      /* RTPPeerSetTimestampDiff( command->peer, diff ) */
      /* finished sync */
      command->data.sync.ssrc  = ssrc;
      command->data.sync.count = 3;

      driver->sync = 0;
      return 0;
    }
    if( command->data.sync.count == 1 ) {
      /* compute media delay */
      diff = ( command->data.sync.timestamp3 - command->data.sync.timestamp1 ) / 2;
      /* approximate time difference between peer and self */
      diff = command->data.sync.timestamp2 + diff - timestamp;

      /* RTPPeerSetTimestampDiff( command->peer, diff ) */

      command->data.sync.ssrc       = ssrc;
      command->data.sync.count      = 2;
      command->data.sync.timestamp3 = timestamp;
      
      driver->sync = 0;
      return _applemidi_send_command( driver, fd, command );
    }
    if( command->data.sync.count == 0 ) {
      command->data.sync.ssrc       = ssrc;
      command->data.sync.count      = 1;
      command->data.sync.timestamp2 = timestamp;
      
      driver->sync = 2;
      return _applemidi_send_command( driver, fd, command );
    }
  }
  return 1;
}

static int _applemidi_start_sync( struct MIDIDriverAppleMIDI * driver, int fd, socklen_t size, struct sockaddr * addr ) {
  if( addr != (struct sockaddr *) &(driver->command.addr) ) memcpy( &(driver->command.addr), addr, size );
  driver->command.size = size;
  driver->command.type = APPLEMIDI_COMMAND_SYNCHRONIZATION;
  driver->command.data.sync.count = 3;
  RTPSessionGetSSRC( driver->rtp_session, &(driver->command.data.sync.ssrc) );

  return _applemidi_sync( driver, fd, &(driver->command) );
}

static int _applemidi_invite( struct MIDIDriverAppleMIDI * driver, int fd, socklen_t size, struct sockaddr * addr ) {
  if( addr != (struct sockaddr *) &(driver->command.addr) ) memcpy( &(driver->command.addr), addr, size );
  driver->command.size = size;
  driver->command.type = APPLEMIDI_COMMAND_INVITATION;
  driver->command.data.session.version = 2;
  driver->command.data.session.token   = driver->token;
  RTPSessionGetSSRC( driver->rtp_session, &(driver->command.data.session.ssrc) );
  strncpy( &(driver->command.data.session.name[0]), driver->name,
           sizeof(driver->command.data.session.name) );

  return _applemidi_send_command( driver, fd, &(driver->command) );
}

static int _applemidi_endsession( struct MIDIDriverAppleMIDI * driver, int fd, socklen_t size, struct sockaddr * addr ) {
  if( addr != (struct sockaddr *) &(driver->command.addr) ) memcpy( &(driver->command.addr), addr, size );
  driver->command.size = size;
  driver->command.type = APPLEMIDI_COMMAND_ENDSESSION;
  driver->command.data.session.version = 2;
  driver->command.data.session.token   = driver->token;
  RTPSessionGetSSRC( driver->rtp_session, &(driver->command.data.session.ssrc) );
  strncpy( &(driver->command.data.session.name[0]), driver->name,
           sizeof(driver->command.data.session.name) );
  
  return _applemidi_send_command( driver, fd, &(driver->command) );
}

/**
 * @brief Determine the RTP address belonging to a control address.
 * @param size The size of the address structures (should be sizeof(struct sockaddr_in))
 * @param control_addr The control address.
 * @param rtp_addr The resulting RTP address. (Control port + 1)
 * @return 0 on success.
 * @return >0 on error.
 */
static int _applemidi_rtp_addr( socklen_t size, struct sockaddr * control_addr, struct sockaddr * rtp_addr ) {
  struct sockaddr_in * in_addr;
  if( control_addr != rtp_addr ) {
    memcpy( rtp_addr, control_addr, size );
  }
  if( rtp_addr->sa_family == AF_INET ) {
    in_addr = (struct sockaddr_in *) rtp_addr;
    in_addr->sin_port = htons( ntohs( in_addr->sin_port ) + 1 );
    return 0;
  } else {
    return 1;
  }
}

/**
 * @brief Determine the control address belonging to an RTP address.
 * @param size The size of the address structures (should be sizeof(struct sockaddr_in))
 * @param rtp_addr The RTP address.
 * @param control_addr The resulting control address. (RTP port - 1)
 * @return 0 on success.
 * @return >0 on error.
 */
static int _applemidi_control_addr( socklen_t size, struct sockaddr * rtp_addr, struct sockaddr * control_addr ) {
  struct sockaddr_in * in_addr;
  if( rtp_addr != control_addr ) {
    memcpy( control_addr, rtp_addr, size );
  }
  if( control_addr->sa_family == AF_INET ) {
    in_addr = (struct sockaddr_in *) control_addr;
    in_addr->sin_port = htons( ntohs( in_addr->sin_port ) - 1 );
    return 0;
  } else {
    return 1;
  }
}

/**
 * @brief Respond to a given AppleMIDI command.
 * Use the command as response and - if neccessary - send it back to the peer.
 * @private @memberof MIDIDriverAppleMIDI
 * @param driver The driver.
 * @param fd The file descriptor to be used for communication.
 * @param command The command.
 * @retval 0 On success.
 * @retval >0 If the packet could not be sent.
 */
static int _applemidi_respond( struct MIDIDriverAppleMIDI * driver, int fd, struct AppleMIDICommand * command ) {
  struct RTPPeer * peer = NULL;

  switch( command->type ) {
    case APPLEMIDI_COMMAND_INVITATION:
      MIDIDriverDelegateTriggerEvent( driver->delegate, MIDI_APPLEMIDI_PEER_DID_SEND_INVITATION, peer );
      if( driver->accept ) {
        command->type = APPLEMIDI_COMMAND_INVITATION_ACCEPTED;
        if( fd == driver->rtp_socket ) {
          peer = RTPPeerCreate( command->data.session.ssrc, command->size, (struct sockaddr *) &(command->addr) );
          RTPSessionAddPeer( driver->rtp_session, peer );
          RTPPeerRelease( peer );
        }
      } else {
        command->type = APPLEMIDI_COMMAND_INVITATION_REJECTED;
      }
      RTPSessionGetSSRC( driver->rtp_session, &(command->data.session.ssrc) );
      return _applemidi_send_command( driver, fd, command );
    case APPLEMIDI_COMMAND_INVITATION_ACCEPTED:
      if( command->data.session.token == driver->token ) {
        if( fd == driver->control_socket ) {
          _applemidi_rtp_addr( command->size, (struct sockaddr *) &command->addr, (struct sockaddr *) &command->addr );
          return _applemidi_invite( driver, driver->rtp_socket, command->size, (struct sockaddr *) &(command->addr) );
        } else {
          peer = RTPPeerCreate( command->data.session.ssrc, command->size, (struct sockaddr *) &(command->addr) );
          RTPSessionAddPeer( driver->rtp_session, peer );
          MIDIDriverDelegateTriggerEvent( driver->delegate, MIDI_APPLEMIDI_PEER_DID_ACCEPT_INVITATION, peer );
          RTPPeerRelease( peer );
          return _applemidi_start_sync( driver, driver->rtp_socket, command->size, (struct sockaddr *) &(command->addr) );
        }
      }
      break;
    case APPLEMIDI_COMMAND_INVITATION_REJECTED:
      MIDIDriverDelegateTriggerEvent( driver->delegate, MIDI_APPLEMIDI_PEER_DID_REJECT_INVITATION, NULL );
      break;
    case APPLEMIDI_COMMAND_ENDSESSION:
      RTPSessionFindPeerBySSRC( driver->rtp_session, &peer, command->data.session.ssrc );
      MIDIDriverDelegateTriggerEvent( driver->delegate, MIDI_APPLEMIDI_PEER_DID_END_SESSION, peer );
      if( peer != NULL ) {
        RTPSessionRemovePeer( driver->rtp_session, peer );
      }
      break;
    case APPLEMIDI_COMMAND_SYNCHRONIZATION:
      return _applemidi_sync( driver, fd, command );
    case APPLEMIDI_COMMAND_RECEIVER_FEEDBACK:
      RTPSessionFindPeerBySSRC( driver->rtp_session, &peer, command->data.feedback.ssrc );
      RTPMIDISessionJournalTrunkate( driver->rtpmidi_session, peer, command->data.feedback.seqnum );
      break;
    default:
      return 1;
  }
  return 0;
}

/**
 * @brief Connect to a peer with a socket address.
 * Use the AppleMIDI protocol to establish an RTP-session, including a SSRC that was received
 * from the peer. Send the session packets to the given socket address.
 * @public @memberof MIDIDriverAppleMIDI
 * @param driver The driver.
 * @param size The size of the address pointed to by @c addr.
 * @param addr A pointer to an address that can be used to send packets to the client.
 * @retval 0 on success.
 * @retval >0 if the connection could not be established.
 */
int MIDIDriverAppleMIDIAddPeerWithSockaddr( struct MIDIDriverAppleMIDI * driver, socklen_t size, struct sockaddr * addr ) {
  return _applemidi_invite( driver, driver->control_socket, size, addr );
}

/**
 * @brief Connect to a peer.
 * Use the AppleMIDI protocol to establish an RTP-session, including a SSRC that was received
 * from the peer.
 * @public @memberof MIDIDriverAppleMIDI
 * @param driver The driver.
 * @param address The internet address of the peer.
 * @param port The AppleMIDI control port (usually 5004), the RTP-port is the next port.
 * @retval 0 on success.
 * @retval >0 if the connection could not be established.
 */
int MIDIDriverAppleMIDIAddPeer( struct MIDIDriverAppleMIDI * driver, char * address, unsigned short port ) {
  struct addrinfo * res;
  int result;
  char portname[8];

  sprintf( &(portname[0]), "%hu", port );
  result = getaddrinfo( address, portname, NULL, &res );
  if( result ) {
    return 1;
  }  

  result = MIDIDriverAppleMIDIAddPeerWithSockaddr( driver, res->ai_addrlen, res->ai_addr );
  freeaddrinfo( res );
  return result;
}

/**
 * @brief Disconnect from a peer.
 * Use the AppleMIDI protocol to tell the peer that the session ended.
 * Remove the peer from the @c RTPSession.
 * @public @memberof MIDIDriverAppleMIDI
 * @param driver The driver.
 * @param size The size of the address pointed to by @c addr.
 * @param addr A pointer to an address that can be used to send packets to the client.
 * @retval 0 on success.
 * @retval >0 if the session could not be ended.
 */
int MIDIDriverAppleMIDIRemovePeerWithSockaddr( struct MIDIDriverAppleMIDI * driver, socklen_t size, struct sockaddr * addr ) {
  int result;
  struct RTPPeer * peer;
  result = RTPSessionFindPeerByAddress( driver->rtp_session, &peer, size, addr );
  if( result ) {
    return result;
  }

  result  = RTPSessionRemovePeer( driver->rtp_session, peer );
  result += _applemidi_endsession( driver, driver->control_socket, size, addr );
  return result;
}

/**
 * @brief Disconnect from a peer.
 * Use the AppleMIDI protocol to tell the peer that the session ended.
 * Remove the peer from the @c RTPSession.
 * @public @memberof MIDIDriverAppleMIDI
 * @param driver The driver.
 * @param address The internet address of the peer.
 * @param port The AppleMIDI control port (usually 5004), the RTP-port is the next port.
 * @retval 0 on success.
 * @retval >0 if the session could not be ended.
 */
int MIDIDriverAppleMIDIRemovePeer( struct MIDIDriverAppleMIDI * driver, char * address, unsigned short port ) {
  struct addrinfo * res = NULL;
  int result;
  char portname[8];

  sprintf( &(portname[0]), "%hu", port );
  result = getaddrinfo( address, portname, NULL, &res );
  if( result ) {
    return 1;
  }
  
  result = MIDIDriverAppleMIDIRemovePeerWithSockaddr( driver, res->ai_addrlen, res->ai_addr );
  freeaddrinfo( res );
  return result;
}

static int _applemidi_receive_rtpmidi( struct MIDIDriverAppleMIDI * driver ) {
  struct MIDIMessageList messages[APPLEMIDI_MAX_MESSAGES_PER_PACKET];
  int i, result;

  for( i=0; i<APPLEMIDI_MAX_MESSAGES_PER_PACKET; i++ ) {
    messages[i].message = NULL;
    messages[i].next = &(messages[i+1]);
  }
  messages[i-1].next = NULL;

  result = RTPMIDISessionReceive( driver->rtpmidi_session, &(messages[0]) );
  if( result != 0 ) return result;

  for( i=0; i<APPLEMIDI_MAX_MESSAGES_PER_PACKET && messages[i].message != NULL; i++ ) {
  /*MIDIMessageQueuePush( driver->in_queue, messages[i].message );
    MIDIMessageRelease( messages[i].message );*/
    MIDIDriverAppleMIDIReceiveMessage( driver, messages[i].message ); /* fixme: add scheduling! */
  }
  
  return 0;
}

static int _applemidi_send_rtpmidi( struct MIDIDriverAppleMIDI * driver ) {
  struct MIDIMessageList messages[APPLEMIDI_MAX_MESSAGES_PER_PACKET];
  int i, result;
  size_t length;
  MIDIMessageQueueGetLength( driver->out_queue, &length );

  if( length > 0 ) {
    for( i=0; i<length && i<APPLEMIDI_MAX_MESSAGES_PER_PACKET; i++ ) {
      MIDIMessageQueuePop( driver->out_queue, &(messages[i].message) );
      messages[i].next = &(messages[i+1]);
    }
    messages[i-1].next = NULL;

    result = RTPMIDISessionSend( driver->rtpmidi_session, &(messages[0]) );

    for( i=0; i<length && i<APPLEMIDI_MAX_MESSAGES_PER_PACKET; i++ ) {
      if( messages[i].message != NULL ) MIDIMessageRelease( messages[i].message );
    }
    return result;
  } else {
    return 0;
  }
}

static int _applemidi_read_fds( void * drv, int nfds, fd_set * readfds ) {
  struct MIDIDriverAppleMIDI * driver = drv;
  int fd, result = 0;

  if( nfds <= 0 ) return 0;

  if( FD_ISSET( driver->control_socket, readfds ) ) {
    fd = driver->control_socket;
    if( _test_applemidi( fd ) == 0 ) {
      if( _applemidi_recv_command( driver, fd, &(driver->command) ) == 0 ) {
        result += _applemidi_respond( driver, fd, &(driver->command) );
      }
    }
  }
  
  if( FD_ISSET( driver->rtp_socket, readfds ) ) {
    fd = driver->rtp_socket;
    if( _test_applemidi( fd ) == 0 ) {
      if( _applemidi_recv_command( driver, fd, &(driver->command) ) == 0 ) {
        result += _applemidi_respond( driver, fd, &(driver->command) );
      }
    } else {
      result += _applemidi_receive_rtpmidi( driver );
    }
  }
  
  _applemidi_update_runloop_source( driver );

  return result;
}

static int _applemidi_write_fds( void * drv, int nfds, fd_set * writefds ) {
  struct MIDIDriverAppleMIDI * driver = drv;
  int fd, result = 0;

  if( nfds <= 0 ) return 0;

  if( FD_ISSET( driver->rtp_socket, writefds ) ) {
    fd = driver->rtp_socket;
    result += _applemidi_send_rtpmidi( driver );
  }

  if( FD_ISSET( driver->control_socket, writefds ) ) {
    fd = driver->control_socket;
  }

  _applemidi_update_runloop_source( driver );

  return result;
}

static int _applemidi_idle_timeout( void * drv, struct timespec * ts ) {
  struct MIDIDriverAppleMIDI * driver = drv;
  struct sockaddr * addr;
  socklen_t size;

  _applemidi_update_runloop_source( driver );

  RTPSessionNextPeer( driver->rtp_session, &(driver->peer) );
  if( driver->peer != NULL ) {
    /* check if receiver feedback needs to be sent */
    if( driver->sync == 0 ) {
      /* no sync packets active. start new sync */
      RTPPeerGetAddress( driver->peer, &size, &addr );
      return _applemidi_start_sync( driver, driver->rtp_socket, size, addr );
    }
  }

  /* check for messages in dispatch (incoming) queue:
   *   if message needs to be dispatched (timestamp >= now+latency)
   *   call MIDIDriverAppleMIDIReceiveMessage
   * send receiver feedback
   * if the last synchronization happened a certain time ago, synchronize again */

  return 0;
}

/**
 * @brief Initialize an @c fd_set with the driver's sockets and return the number
 * of descriptors in the set.
 * @param driver The driver.
 * @param fds    The filedescriptor set.
 * @return The number of descriptors (max ID + 1) in @ fds.
 */
static int _applemidi_init_fds( struct MIDIDriverAppleMIDI * driver, fd_set * fds ) {
  FD_ZERO( fds );
  FD_SET( driver->control_socket, fds );
  FD_SET( driver->rtp_socket, fds );
  
  if( driver->control_socket > driver->rtp_socket ) {
    return driver->control_socket + 1;
  } else {
    return driver->rtp_socket + 1;
  }
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
  int nfds;
  fd_set fds;
  static struct timeval tv = { 0, 0 };
  nfds = _applemidi_init_fds( driver, &fds );
  select( nfds, &fds, NULL, NULL, &tv );
  return _applemidi_read_fds( driver, nfds, &fds );
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
  int nfds;
  fd_set fds;
  static struct timeval tv = { 0, 0 };
  nfds = _applemidi_init_fds( driver, &fds );
  select( nfds, NULL, &fds, NULL, &tv );
  return _applemidi_write_fds( driver, nfds, &fds );
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
  static struct timespec ts = { 0, 0 };
  return _applemidi_idle_timeout( driver, &ts );
}

/**
 * @brief Get a pointer to the driver's runloop source.
 * @public @memberof MIDIDriverAppleMIDI
 * @param driver The driver.
 * @param source The runloop source.
 * @retval 0 on success.
 * @retval >0 if the runloop source could not be created.
 */
int MIDIDriverAppleMIDIGetRunloopSource( struct MIDIDriverAppleMIDI * driver, struct MIDIRunloopSource ** source ) {
  if( source == NULL ) return 1;
  _applemidi_update_runloop_source( driver );
  *source = &(driver->runloop_source);
  return 0;
}
