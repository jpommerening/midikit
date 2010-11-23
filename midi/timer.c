#include <stdlib.h>
#include "midi.h"
#include "timer.h"
#include "clock.h"
#include "message.h"

struct MIDITimer {
  size_t refs;
  struct MIDITimerDelegate * delegate;
  MIDILongValue song_position;
  MIDILongValue beats_per_minute;
  MIDITimestamp last_timestamp;
  MIDITimestamp clocks[4];
};

struct MIDITimer * MIDITimerCreate( struct MIDITimerDelegate * delegate ) {
  struct MIDITimer * timer = malloc( sizeof( struct MIDITimer ) );
  timer->refs = 1;
  timer->delegate         = delegate;
  timer->song_position    = 0;
  timer->beats_per_minute = 120;
  timer->last_timestamp   = -1;
  timer->clocks[0]        = -1;
  timer->clocks[1]        = -1;
  timer->clocks[2]        = -1;
  timer->clocks[3]        = -1;
  return timer;
}

void MIDITimerDestroy( struct MIDITimer * timer ) {
  free( timer );
}

void MIDITimerRetain( struct MIDITimer * timer ) {
  timer->refs++;
}

void MIDITimerRelease( struct MIDITimer * timer ) {
  if( ! --timer->refs ) {
    MIDITimerDestroy( timer );
  }
}

int MIDITimerReceive( struct MIDITimer * timer, struct MIDIDevice * device,
                      struct MIDIMessage * message ) {
  MIDIStatus    status;
  MIDITimestamp timestamp;
  MIDILongValue value;
  if( message == NULL ) return 1;
  MIDIMessageGetStatus( message, &status );
  MIDIMessageGetTimestamp( message, &timestamp );
  switch( status ) {
    case MIDI_STATUS_SONG_POSITION_POINTER:
      MIDIMessageGet( message, MIDI_VALUE, sizeof(MIDILongValue), &value );
      timer->song_position = value;
      break;
    case MIDI_STATUS_SONG_SELECT:
      timer->song_position = 0;
      break;
    case MIDI_STATUS_TIMING_CLOCK:
      break;
    case MIDI_STATUS_START:
      break;
    case MIDI_STATUS_CONTINUE:
      break;
    case MIDI_STATUS_STOP:
      break;
    default:
      return 1;
  }
  return 0;
}

int MIDITimerSend( struct MIDITimer * timer, struct MIDIDevice * device,
                   struct MIDIMessage * message ) {
  MIDIStatus status;
  if( message == NULL ) return 1;
  MIDIMessageGetStatus( message, &status );
  switch( status ) {
    case MIDI_STATUS_SONG_POSITION_POINTER:
      break;
    case MIDI_STATUS_SONG_SELECT:
      break;
    case MIDI_STATUS_TIMING_CLOCK:
      break;
    case MIDI_STATUS_START:
      break;
    case MIDI_STATUS_CONTINUE:
      break;
    case MIDI_STATUS_STOP:
      break;
    default:
      return 1;
  }
  return 0;
}
