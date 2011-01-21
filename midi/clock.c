#include <stdlib.h>
#include <stdio.h>
#include "clock.h"

/**
 * @brief Provider for accurate timestamps.
 * The MIDIClock provides accurate timestamps at any desired
 * rate with a selectable offset.
 */
struct MIDIClock {
  size_t           refs;
  MIDITimestamp    offset;
  MIDISamplingRate rate;
  unsigned long long numer;
  unsigned long long denom;
};

/* use clock_gettime */
#ifdef _POSIX_SOURCE
#include <time.h>
#ifdef _CLOCK_MONOTONIC_FAST
#define CLOCK_TYPE CLOCK_MONOTONIC_FAST
#else
#ifdef _CLOCK_MONOTONIC
#define CLOCK_TYPE CLOCK_MONOTONIC
#endif
#endif

/* use posix clock_gettime */
#ifdef CLOCK_TYPE
#define NSEC_PER_SEC 1000000000

static unsigned long long _timestamp_clock( void ) {
  static struct timespec ts;
  clock_gettime( CLOCK_TYPE, &ts );
  return (ts.tv_sec * NSEC_PER_SEC + ts.tv_nsec);
}

static void _init_clock_clock( struct MIDIClock * clock ) {
  clock->numer = 1;
  clock->denom = NSEC_PER_SEC;
}
#endif
#endif


/* use mach_absolute_time */
#ifdef _MACH
#include <mach/mach.h>
#include <mach/mach_time.h>

static unsigned long long _timestamp_mach( void ) {
  return mach_absolute_time();
}

static void _init_clock_mach( struct MIDIClock * clock ) {
  static mach_timebase_info_data_t info;
  if( info.denom == 0 ) {
    mach_timebase_info( &info );
  }
  clock->numer = info.numer;
  clock->denom = info.denom;
}
#endif

/* use good old gettimeofday */
#ifdef _POSIX_SOURCE
#include <sys/time.h>
#define USEC_PER_SEC 1000000

static unsigned long long _timestamp_posix( void ) {
  static struct timeval tv;
  gettimeofday( &tv, NULL );
  return (tv.tv_sec * USEC_PER_SEC + tv.tv_usec);
}

static void _init_clock_posix( struct MIDIClock * clock ) {
  clock->numer = 1;
  clock->denom = USEC_PER_SEC;
}
#endif


static MIDITimestamp _get_real_time( struct MIDIClock * clock ) {
  return (_timestamp_posix() * clock->numer) / clock->denom;
}

struct MIDIClock * MIDIClockCreate( MIDISamplingRate rate ) {
  struct MIDIClock * clock = malloc( sizeof( struct MIDIClock ) );
  if( clock == NULL ) {
    return NULL;
  }
  clock->refs = 1;
  _init_clock_posix( clock );
  if( rate == 0 ) rate = ( clock->denom / clock->numer );
  clock->rate   = rate;
  clock->numer *= rate;
  clock->offset = -1 * _get_real_time( clock );
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
  clock->numer = ( clock->numer / clock->rate ) * rate;
  clock->rate  = rate;
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
