#ifdef __APPLE__

#define MIDI_DRIVER_INTERNALS
#include "coremidi.h"
#include "midi/driver.h"
#include "midi/message.h"
#include "midi/message_queue.h"

#define COREMIDI_CLOCK_RATE 10000

struct MIDIDriverCoreMIDI {
  struct MIDIDriver base;
  MIDIClientRef client;
  MIDIPortRef   cm_in_port;
  MIDIPortRef   cm_out_port;
  struct MIDIDriverDelegate * delegate;
};

static void _coremidi_readproc( const MIDIPacketList *pktlist, void * readProcRefCon, void * srcRefCon ) {
  struct MIDIDriverCoreMIDI * driver = readProcRefCon;
  struct MIDIMessage * message       = NULL;
  int i;
  size_t size, read = 0;
  const void * buffer;
  
  for( i=0; i<pktlist->numPackets; i++ ) {
    size   =   pktlist->packet[i].length;
    buffer = &(pktlist->packet[i].data[0]);
    do {
      message = MIDIMessageCreate( MIDI_STATUS_RESET );
      MIDIMessageDecode( message, size, buffer, &read );
      MIDIDriverCoreMIDIReceiveMessage( driver, message );
      MIDIMessageRelease( message );
      size   -= read;
      buffer += read;
    } while( size > 0 );
  }
}

int MIDIDriverCoreMIDISendMessage( struct MIDIDriverCoreMIDI * driver, struct MIDIMessage * message );
static int _driver_send( void * driverp, struct MIDIMessage * message ) {
  return MIDIDriverCoreMIDISendMessage( driverp, message );
}

void MIDIDriverCoreMIDIDestroy( struct MIDIDriverCoreMIDI * driver );
static void _driver_destroy( void * driverp ) {
  MIDIDriverCoreMIDIDestroy( driverp );
}

/**
 * @brief Create a MIDIDriverCoreMIDI instance.
 * Allocate space and initialize an MIDIDriverCoreMIDI instance.
 * @public @memberof MIDIDriverCoreMIDI
 * @return a pointer to the created driver structure on success.
 * @return a @c NULL pointer if the driver could not created.
 */
struct MIDIDriverCoreMIDI * MIDIDriverCoreMIDICreate( char * name, MIDIClientRef client ) {
  struct MIDIDriverCoreMIDI * driver;
  CFStringRef in_name, out_name;
  
  driver = malloc( sizeof( struct MIDIDriverCoreMIDI ) );
  MIDIPrecondReturn( driver != NULL, ENOMEM, NULL );
  MIDIDriverInit( &(driver->base), name, COREMIDI_CLOCK_RATE );

  driver->client = client; 
  in_name  = CFStringCreateWithFormat( NULL, NULL, CFSTR("MIDIKit %s input"), name );
  out_name = CFStringCreateWithFormat( NULL, NULL, CFSTR("MIDIKit %s input"), name );
  
  MIDIInputPortCreate( client, in_name, &_coremidi_readproc, driver, &(driver->cm_in_port) );
  MIDIOutputPortCreate( client, out_name, &(driver->cm_out_port) );

  return driver;
}

/**
 * @brief Destroy a MIDIDriverCoreMIDI instance.
 * Free all resources occupied by the driver.
 * @public @memberof MIDIDriverCoreMIDI
 * @param driver The driver.
 */
void MIDIDriverCoreMIDIDestroy( struct MIDIDriverCoreMIDI * driver ) {
  MIDIPortDispose( driver->cm_in_port );
  MIDIPortDispose( driver->cm_out_port );
}

/**
 * @brief Handle incoming MIDI messages.
 * This is called by the RTP-MIDI payload parser whenever it encounters a new MIDI message.
 * There may be multiple messages in a single packet so a single call of MIDIDriverCoreMIDI
 * may trigger multiple calls of this function.
 * @public @memberof MIDIDriverCoreMIDI
 * @param driver The driver.
 * @param message The message that was just received.
 * @retval 0 on success.
 * @retval >0 if the message could not be processed.
 */
int MIDIDriverCoreMIDIReceiveMessage( struct MIDIDriverCoreMIDI * driver, struct MIDIMessage * message ) {
  return 0;
}

/**
 * @brief Process outgoing MIDI messages.
 * This is called by the generic driver interface to pass messages to this driver implementation.
 * The driver may queue outgoing messages to reduce package overhead, trading of latency for throughput.
 * @public @memberof MIDIDriverCoreMIDI
 * @param driver The driver.
 * @param message The message that should be sent.
 * @retval 0 on success.
 * @retval >0 if the message could not be processed.
 */
int MIDIDriverCoreMIDISendMessage( struct MIDIDriverCoreMIDI * driver, struct MIDIMessage * message ) {
  return 0;
}

/**
 * @brief Receive from any peer.
 * This should be called whenever there is new data to be received on a socket.
 * @public @memberof MIDIDriverCoreMIDI
 * @param driver The driver.
 * @retval 0 on success.
 * @retval >0 if the packet could not be processed.
 */
int MIDIDriverCoreMIDIReceive( struct MIDIDriverCoreMIDI * driver ) {
  return 0;
}

/**
 * @brief Send queued messages to all connected peers.
 * This should be called whenever new messages are added to the queue and whenever the
 * socket can accept new data.
 * @public @memberof MIDIDriverCoreMIDI
 * @param driver The driver.
 * @retval 0 on success.
 * @retval >0 if packets could not be sent, or any other operation failed.
 */
int MIDIDriverCoreMIDISend( struct MIDIDriverCoreMIDI * driver ) {
  return 0;
}

/**
 * @brief Do idling operations.
 * When there is nothing else to do, keep in sync with connected peers,
 * dispatch incoming messages, send receiver feedback.
 * @public @memberof MIDIDriverCoreMIDI
 * @param driver The driver.
 * @retval 0 on success.
 * @retval >0 if packets could not be sent, or any other operation failed.
 */
int MIDIDriverCoreMIDIIdle( struct MIDIDriverCoreMIDI * driver ) {
  return 0;
}

#endif