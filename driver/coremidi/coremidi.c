#ifdef __APPLE__
#include "coremidi.h"
#include "midi/message_queue.h"

struct MIDIDriverCoreMIDI {
  size_t refs;
  MIDIClientRef client;
  MIDIPortRef   in_port;
  MIDIPortRef   out_port;
  struct MIDIDriverDelegate * delegate;
};

static void _coremidi_readproc( const MIDIPacketList *pktlist, void * readProcRefCon, void * srcRefCon ) {
  struct MIDIDriverCoreMIDI * driver = readProcRefCon;
  struct MIDIMessage * message       = NULL;
  int i;
  size_t size, read = 0;
  void * buffer;
  
  for( i=0; i<pktlist->numPackets; i++ ) {
    size   = pktlist->packet[i].length;
    buffer = pktlist->packet[i].data;
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

static int _driver_send( void * driverp, struct MIDIMessage * message ) {
  return MIDIDriverCoreMIDISendMessage( driverp, message );
}

/**
 * @brief Create a MIDIDriverCoreMIDI instance.
 * Allocate space and initialize an MIDIDriverCoreMIDI instance.
 * @public @memberof MIDIDriverCoreMIDI
 * @return a pointer to the created driver structure on success.
 * @return a @c NULL pointer if the driver could not created.
 */
struct MIDIDriverCoreMIDI * MIDIDriverCoreMIDICreate( struct MIDIDriverDelegate * delegate, MIDIClientRef client ) {
  struct MIDIDriverCoreMIDI * driver;

  driver->client    = client;
  MIDIInputPortCreate( client, CFSTR("MIDIKit input"), &_coremidi_readproc, driver, &(driver->in_port) );
  MIDIOutputPortCreate( client, CFSTR("MIDIKit output"), &(driver->out_port) );
  driver->delegate  = delegate;

  if( delegate != NULL ) {
    delegate->send = &_driver_send;
    delegate->implementation = driver;
  }
  return driver;
}

/**
 * @brief Destroy a MIDIDriverCoreMIDI instance.
 * Free all resources occupied by the driver.
 * @public @memberof MIDIDriverCoreMIDI
 * @param driver The driver.
 */
void MIDIDriverCoreMIDIDestroy( struct MIDIDriverCoreMIDI * driver ) {
  MIDIPortDispose( driver->in_port );
  MIDIPortDispose( driver->out_port );
}

/**
 * @brief Retain a MIDIDriverCoreMIDI instance.
 * Increment the reference counter of a driver so that it won't be destroyed.
 * @public @memberof MIDIDriverCoreMIDI
 * @param driver The driver.
 */
void MIDIDriverCoreMIDIRetain( struct MIDIDriverCoreMIDI * driver ) {
  driver->refs++;
}

/**
 * @brief Release a MIDIDriverCoreMIDI instance.
 * Decrement the reference counter of a driver. If the reference count
 * reached zero, destroy the driver.
 * @public @memberof MIDIDriverCoreMIDI
 * @param driver The driver.
 */
void MIDIDriverCoreMIDIRelease( struct MIDIDriverCoreMIDI * driver ) {
  if( ! --driver->refs ) {
    MIDIDriverCoreMIDIDestroy( driver );
  }
}

/**
 * @brief Handle incoming MIDI messages.
 * This is called by the RTP-MIDI payload parser whenever it encounters a new MIDI message.
 * There may be multiple messages in a single packet so a single call of @c MIDIDriverCoreMIDI
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