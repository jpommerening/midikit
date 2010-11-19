#include <stdlib.h>
#include "midi.h"
#include "timer.h"

struct MIDITimer {
  size_t refs;
  struct MIDITimerDelegate * delegate;
};

struct MIDITimer * MIDITimerCreate( struct MIDITimerDelegate * delegate ) {
  struct MIDITimer * timer = malloc( sizeof( struct MIDITimer ) );
  timer->refs = 1;
  timer->delegate = delegate;
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

