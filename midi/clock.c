#include <stdlib.h>
#include <stdio.h>
#include <sys/time.h>
#include "clock.h"

#define USEC_PER_SEC 1000000.0

struct MIDIClock {
  size_t           refs;
  MIDITimestamp    offset;
  MIDISamplingRate rate;
  double           divisor;
};

static MIDITimestamp _get_real_time( struct MIDIClock * c ) {
  struct timeval tv;
  gettimeofday( &tv, NULL );
  return (tv.tv_sec * USEC_PER_SEC + tv.tv_usec) / c->divisor;
}

struct MIDIClock * MIDIClockCreate( MIDISamplingRate rate ) {
  struct MIDIClock * clock = malloc( sizeof( struct MIDIClock ) );
  if( clock == NULL ) {
    return NULL;
  }
  clock->refs    = 1;
  clock->rate    = rate;
  clock->divisor = USEC_PER_SEC / rate;
  clock->offset  = 0 - _get_real_time( clock );
  if( rate < 1.0 || rate > USEC_PER_SEC ) {
    free( clock );
    return NULL;
  }
  return clock;
}

void MIDIClockDestroy( struct MIDIClock * clock ) {
  free( clock );
}

void MIDIClockRetain( struct MIDIClock * clock ) {
  clock->refs++;
}

void MIDIClockRelease( struct MIDIClock * clock ) {
  if( ! --clock->refs ) {
    MIDIClockDestroy( clock );
  }
}

int MIDIClockSetNow( struct MIDIClock * clock, MIDITimestamp now ) {
  clock->offset = now - _get_real_time( clock );
  return 0;
}

int MIDIClockGetNow( struct MIDIClock * clock, MIDITimestamp * now ) {
  *now = _get_real_time( clock ) + clock->offset;
  return 0;
}

int MIDIClockSetSamplingRate( struct MIDIClock * clock, MIDISamplingRate rate ) {
  clock->rate = rate;
  clock->divisor = USEC_PER_SEC / rate;
  return 0;
}

int MIDIClockGetSamplingRate( struct MIDIClock * clock, MIDISamplingRate * rate ) {
  *rate = clock->rate;
  return 0;
}

int MIDIClockTimestampToSeconds( struct MIDIClock * clock, MIDITimestamp timestamp, double * seconds ) {
  *seconds = timestamp / clock->rate;
  return 0;
}

int MIDIClockTimestampFromSeconds( struct MIDIClock * clock, MIDITimestamp * timestamp, double seconds ) {
  *timestamp = seconds * clock->rate;
  return 0;
}
