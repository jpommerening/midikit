#include "osc.h"
#include "midi/message_queue.h"
#include "midi/controller.h"
#include <unistd.h>
#include <string.h>
#include <sys/socket.h>
#include <arpa/inet.h>

/**
 * @ingroup MIDI-driver
 * @brief MIDIDriver implementation using the opensoundcontrol protocol.
 */
struct MIDIDriverOSC {
  size_t refs;
  int    socket;
  struct MIDIDriverDelegate * delegate;
  struct MIDIMessageQueue * in_queue;
  struct MIDIMessageQueue * out_queue;
};

struct MIDIDriverOSC * MIDIDriverOSCCreate( struct MIDIDriverDelegate * delegate ) {
  struct MIDIDriverOSC * driver = malloc( sizeof( struct MIDIDriverOSC ) );
  struct sockaddr_in addr;
  
  driver->refs   = 1;
  driver->socket = socket( PF_INET, SOCK_DGRAM, 0 );
  
  addr.sin_family = AF_INET;
  addr.sin_port = 5006;
  addr.sin_addr.s_addr = INADDR_ANY;
  
  bind( driver->socket, (struct sockaddr *) &addr, sizeof(addr) );
  
  driver->delegate  = delegate;
  driver->in_queue  = MIDIMessageQueueCreate();
  driver->out_queue = MIDIMessageQueueCreate();
  return driver;
}

void MIDIDriverOSCDestroy( struct MIDIDriverOSC * driver ) {
  MIDIMessageQueueRelease( driver->in_queue );
  MIDIMessageQueueRelease( driver->out_queue );
  if( driver->socket > 0 ) {
    close( driver->socket );
  }
  free( driver );
}

void MIDIDriverOSCRetain( struct MIDIDriverOSC * driver ) {
  driver->refs++;
}

void MIDIDriverOSCRelease( struct MIDIDriverOSC * driver ) {
  if( ! --driver->refs ) {
    MIDIDriverOSCDestroy( driver );
  }
}

int MIDIDriverOSCEncodeMessageDefault( struct MIDIDriverOSC * driver, struct MIDIMessage * message,
                                       size_t size, void * buffer, size_t * bytes_written ) {
  MIDIStatus  status;
  MIDIControl control;
  MIDIMessageGetStatus( message, &status );
  switch( status ) {
    case MIDI_STATUS_NOTE_OFF:
      break;
    case MIDI_STATUS_NOTE_ON:
      break;
    case MIDI_STATUS_POLYPHONIC_KEY_PRESSURE:
      break;
    case MIDI_STATUS_CONTROL_CHANGE:
      MIDIMessageGet( message, MIDI_CONTROL, sizeof(MIDIControl), &control );
      if( control == MIDI_CONTROL_ALL_NOTES_OFF ) {
      } else {
      }
      break;
    case MIDI_STATUS_PROGRAM_CHANGE:
      break;
    case MIDI_STATUS_CHANNEL_PRESSURE:
      break;
    case MIDI_STATUS_PITCH_WHEEL_CHANGE:
      break;
  }
  return 0;
}

int MIDIDriverOSCDecodeMessageDefault( struct MIDIDriverOSC * driver, struct MIDIMessage * message,
                                       size_t size, void * buffer, size_t * bytes_read ) {
  return 0;
}

int MIDIDriverOSCEncodeMessageRaw( struct MIDIDriverOSC * driver, struct MIDIMessage * message,
                                   size_t size, void * buffer, size_t * bytes_written ) {
  return 0;
}

int MIDIDriverOSCDecodeMessageRaw( struct MIDIDriverOSC * driver, struct MIDIMessage * message,
                                   size_t size, void * buffer, size_t * bytes_read ) {
  return 0;
}

int MIDIDriverOSCSendMessage( struct MIDIDriverOSC * driver, struct MIDIMessage * message );
int MIDIDriverOSCReceiveMessage( struct MIDIDriverOSC * driver, struct MIDIMessage * message );

int MIDIDriverOSCSend( struct MIDIDriverOSC * driver );
int MIDIDriverOSCReceive( struct MIDIDriverOSC * driver );
int MIDIDriverOSCIdle( struct MIDIDriverOSC * driver );
