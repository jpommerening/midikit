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
  int socket;
  unsigned short control_port;
  unsigned short rtp_port;
  struct RTPSession * rtp_session;
};

struct MIDIDriverDelegate MIDIDriverDelegateAppleMIDI = {
  NULL
};

static int _applemidi_connect( struct MIDIDriverAppleMIDI * driver ) {
  if( driver->socket > 0 ) return 0;
  // connect the socket
  return 0;
}

static int _applemidi_disconnnect( struct MIDIDriverAppleMIDI * driver ) {
  if( driver->socket <= 0 ) return 0;
  // disconnect the socket
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
  addr.sin_family = AF_INET;
  addr.sin_port   = htons(5004);
  addr.sin_addr.s_addr = INADDR_ANY;
  
  session = RTPSessionCreate( sizeof(addr), (struct sockaddr *) &addr, SOCK_DGRAM );
  if( session == NULL ) return NULL;
  driver  = malloc( sizeof( struct MIDIDriverAppleMIDI ) );
  if( driver == NULL ) return NULL;
  
  RTPSessionSetTimestampRate( session, 44100.0 );
  
  driver->refs = 1;
  driver->rtp_session = session;
  driver->socket = 0;
  driver->control_port = 5004;
  driver->rtp_port     = 5005;
  
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

int MIDIDriverAppleMIDISetRTPPort( struct MIDIDriverAppleMIDI * driver, unsigned short port ) {
}

int MIDIDriverAppleMIDIGetRTPPort( struct MIDIDriverAppleMIDI * driver, unsigned short * port ); 
int MIDIDriverAppleMIDISetControlPort( struct MIDIDriverAppleMIDI * driver, unsigned short port ); 
int MIDIDriverAppleMIDIGetControlPort( struct MIDIDriverAppleMIDI * driver, unsigned short * port ); 

int MIDIDriverAppleMIDIAddPeer( struct MIDIDriverAppleMIDI * driver, socklen_t size, struct sockaddr * addr ) {
  struct RTPPeer * peer;
  int result;
  unsigned long ssrc = 0;
  
  // use apple midi protocol to determine ssrc
  
  peer = RTPPeerCreate( ssrc, size, addr );
  if( peer == NULL ) return 1;
  result = RTPSessionAddPeer( driver->rtp_session, peer );
  RTPPeerRelease( peer );
  return result;
}

int MIDIDriverAppleMIDIRemovePeer( struct MIDIDriverAppleMIDI * driver, socklen_t size, struct sockaddr * addr ) {
  struct RTPPeer * peer;
  int result;
  
  result = RTPSessionFindPeerByAddress( driver->rtp_session, &peer, size, addr );
  if( result ) return result;
  return RTPSessionRemovePeer( driver->rtp_session, peer );
}


int MIDIDriverAppleMIDIGetLatency( struct MIDIDriverAppleMIDI * driver, double seconds );

int MIDIDriverAppleMIDISetRTPSocket( struct MIDIDriverAppleMIDI * driver, int socket ) {
  return RTPSessionSetSocket( driver->rtp_session, socket );
}

int MIDIDriverAppleMIDIGetRTPSocket( struct MIDIDriverAppleMIDI * driver, int * socket ) {
  return RTPSessionGetSocket( driver->rtp_session, socket );
}

int MIDIDriverAppleMIDISetControlSocket( struct MIDIDriverAppleMIDI * driver, int socket ) {
  int result = _applemidi_disconnect( driver );
  if( result == 0 ) {
    driver->socket = socket;
  }
  return result;
}

int MIDIDriverAppleMIDIGetControlSocket( struct MIDIDriverAppleMIDI * driver, int * socket ) {
  int result = _applemidi_connect( driver );
  if( result == 0 ) {
    *socket = driver->socket;
  }
  return result;
}

int MIDIDriverAppleMIDIConnect( struct MIDIDriverAppleMIDI * driver );
int MIDIDriverAppleMIDIDisconnect( struct MIDIDriverAppleMIDI * driver );
