#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>

#include "clock.h"

#ifdef __MACH__
#include <mach/mach.h>
#include <mach/mach_time.h>
#define _MIDI_CLOCK_MACH { &_init_clock_mach, &_timestamp_mach }
#endif

#if defined(_POSIX_TIMERS) && _POSIX_TIMERS > 0
#include <time.h>
#ifdef _CLOCK_MONOTONIC_FAST
#define POSIX_CLOCK_TYPE CLOCK_MONOTONIC_FAST
#else
#ifdef _CLOCK_MONOTONIC
#define POSIX_CLOCK_TYPE CLOCK_MONOTONIC
#endif
#endif
#endif
#ifdef POSIX_CLOCK_TYPE
#define _MIDI_CLOCK_POSIX { &_init_clock_posix, &_timestamp_posix }
#endif

#if !defined(_MIDI_CLOCK_MACH) && !defined(_MIDI_CLOCK_POSIX)
#include <sys/time.h>
#define _MIDI_CLOCK_SYS { &_init_clock_sys, &_timestamp_sys }
#endif

#define MSEC_PER_SEC 1000
#define USEC_PER_SEC 1000000
#define NSEC_PER_SEC 1000000000

/**
 * @ingroup MIDI
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

static void _normalize_frac( unsigned long long * numer, unsigned long long * denom ) {
  static int prime[] = { 7, 5, 3, 2, 1 };
  int i;
  MIDILog( DEBUG, "Normalize: (%llu / %llu)\n", *numer, *denom );
  for( i=0; prime[i] > 1; i++ ) {
    while( (*numer % prime[i] == 0) && (*denom % prime[i] == 0) ) {
      *numer /= prime[i];
      *denom /= prime[i];
    }
  }
  MIDILog( DEBUG, "Result: (%llu / %llu)\n", *numer, *denom );
}

static void _divide_frac( unsigned long long * numer, unsigned long long * denom, unsigned long long fac ) {
  static int prime[] = { 7, 5, 3, 2, 1 };
  int i;
  MIDILog( DEBUG, "Divide: (%llu / %llu) by %llu\n", *numer, *denom, fac );
  for( i=0; prime[i] > 1; i++ ) {
    while( (fac % prime[i]) == 0 ) {
      if( (*numer % prime[i]) == 0 ) {
        *numer /= prime[i];
      } else {
        *denom *= prime[i];
      }
      fac /= prime[i];
    }
  }
  *denom *= fac;
  MIDILog( DEBUG, "Result: (%llu / %llu) with last factor: %llu\n", *numer, *denom, fac );
}

static void _multiply_frac( unsigned long long * numer, unsigned long long * denom, unsigned long long fac ) {
  _divide_frac( denom, numer, fac );
}

/* use mach_absolute_time */
#ifdef _MIDI_CLOCK_MACH
static void _init_clock_mach( struct MIDIClock * clock ) {
  static mach_timebase_info_data_t info = { 0, 0 };
  if( info.denom == 0 ) {
    mach_timebase_info( &info );
    MIDILog( DEBUG, "MACH Timebase info: %i / %i\n", info.numer, info.denom );
  }
  clock->numer = info.numer;
  clock->denom = info.denom;
  _divide_frac( &(clock->numer), &(clock->denom), NSEC_PER_SEC );
}

static unsigned long long _timestamp_mach( void ) {
  return mach_absolute_time();
}
#endif

/* use posix clock_gettime */
#ifdef _MIDI_CLOCK_POSIX
static void _init_clock_posix( struct MIDIClock * clock ) {
  clock->numer = 1;
  clock->denom = NSEC_PER_SEC;
}

static unsigned long long _timestamp_posix( void ) {
  static struct timespec ts;
  clock_gettime( CLOCK_TYPE, &ts );
  return (ts.tv_sec * NSEC_PER_SEC + ts.tv_nsec);
}
#endif

/* use good old gettimeofday */
#ifdef _MIDI_CLOCK_SYS
static void _init_clock_sys( struct MIDIClock * clock ) {
  clock->numer = 1;
  clock->denom = USEC_PER_SEC;
}

static unsigned long long _timestamp_sys( void ) {
  static struct timeval tv;
  gettimeofday( &tv, NULL );
  return (tv.tv_sec * USEC_PER_SEC + tv.tv_usec);
}
#endif

static struct {
  void (*init)( struct MIDIClock * );
  unsigned long long (*timestamp)( void );
} _midi_clock[] = {
#ifdef _MIDI_CLOCK_MACH
  _MIDI_CLOCK_MACH,
#endif
#ifdef _MIDI_CLOCK_POSIX
  _MIDI_CLOCK_POSIX,
#endif
#ifdef _MIDI_CLOCK_SYS
  _MIDI_CLOCK_SYS,
#endif
};

static struct MIDIClock * _midi_global_clock = NULL;

static MIDITimestamp _get_real_time( struct MIDIClock * clock ) {
  return ((*_midi_clock[0].timestamp)() * clock->numer) / clock->denom;
}

static struct MIDIClock * _get_global_clock( void ) {
  if( _midi_global_clock == NULL ) {
    _midi_global_clock = MIDIClockCreate( MIDI_SAMPLING_RATE_DEFAULT );
  }
  return _midi_global_clock;
}

int MIDIClockSetGlobalClock( struct MIDIClock * clock ) {
  if( clock == _midi_global_clock ) return 0;
  if( _midi_global_clock != NULL ) MIDIClockRelease( _midi_global_clock );
  _midi_global_clock = clock;
  MIDIClockRetain( clock );
  return 0;
}

int MIDIClockGetGlobalClock( struct MIDIClock ** clock ) {
  *clock = _get_global_clock();
  return 0;
}

#pragma mark Creation and destruction
/**
 * @name Creation and destruction
 * Creating, destroying and reference counting of MIDIClock objects.
 * @{
 */

/**
 * @brief Create a MIDIClock instance.
 * Allocate space and initialize a MIDIClock instance.
 * @public @memberof MIDIClock
 * @param rate The number of times the clock should tick per second.
 * @return a pointer to the created clock structure on success.
 * @return a @c NULL pointer if the clock could not created.
 */
struct MIDIClock * MIDIClockCreate( MIDISamplingRate rate ) {
  struct MIDIClock * clock = malloc( sizeof( struct MIDIClock ) );
  MIDIPrecondReturn( clock != NULL, ENOMEM, NULL );

  clock->refs = 1;
  (*_midi_clock[0].init)( clock );
  if( rate == 0 ) rate = ( clock->denom / clock->numer );
  _multiply_frac( &(clock->numer), &(clock->denom), rate );
  _normalize_frac( &(clock->numer), &(clock->denom) );
  clock->rate   = rate;
  clock->offset = -1 * _get_real_time( clock );
  MIDILogLocation( INFO, "Initialized clock:\n  rate: %u, offset: %lli\n  numer: %llu / denom: %llu\n",
    clock->rate, clock->offset, clock->numer, clock->denom );
  return clock;
}

/**
 * @brief Destroy a MIDIClock instance.
 * Free all resources occupied by the clock and release all referenced objects.
 * @public @memberof MIDIClock
 * @param clock The clock.
 */
void MIDIClockDestroy( struct MIDIClock * clock ) {
  MIDIPrecondReturn( clock != NULL, EFAULT, (void)0 );
  free( clock );
}

/**
 * @brief Retain a MIDIClock instance.
 * Increment the reference counter of a clock so that it won't be destroyed.
 * @public @memberof MIDIClock
 * @param clock The clock.
 */
void MIDIClockRetain( struct MIDIClock * clock ) {
  MIDIPrecondReturn( clock != NULL, EFAULT, (void)0 );
  clock->refs++;
}

/**
 * @brief Release a MIDIClock instance.
 * Decrement the reference counter of a clock. If the reference count
 * reached zero, destroy the clock.
 * @public @memberof MIDIClock
 * @param clock The clock.
 */
void MIDIClockRelease( struct MIDIClock * clock ) {
  MIDIPrecondReturn( clock != NULL, EFAULT, (void)0 );
  if( ! --clock->refs ) {
    MIDIClockDestroy( clock );
  }
}

/** @} */

int MIDIClockSetNow( struct MIDIClock * clock, MIDITimestamp now ) {
  if( clock == NULL ) clock = _get_global_clock();
  clock->offset = now - _get_real_time( clock );
  return 0;
}

int MIDIClockGetNow( struct MIDIClock * clock, MIDITimestamp * now ) {
  if( clock == NULL ) clock = _get_global_clock();
  *now = _get_real_time( clock ) + clock->offset;
  return 0;
}

int MIDIClockSetSamplingRate( struct MIDIClock * clock, MIDISamplingRate rate ) {
  if( clock == NULL ) clock = _get_global_clock();
  clock->numer = ( clock->numer / clock->rate ) * rate;
  clock->rate  = rate;
  return 0;
}

int MIDIClockGetSamplingRate( struct MIDIClock * clock, MIDISamplingRate * rate ) {
  if( clock == NULL ) clock = _get_global_clock();
  *rate = clock->rate;
  return 0;
}

int MIDIClockTimestampToSeconds( struct MIDIClock * clock, MIDITimestamp timestamp, double * seconds ) {
  if( clock == NULL ) clock = _get_global_clock();
  *seconds = timestamp / clock->rate;
  return 0;
}

int MIDIClockTimestampFromSeconds( struct MIDIClock * clock, MIDITimestamp * timestamp, double seconds ) {
  if( clock == NULL ) clock = _get_global_clock();
  *timestamp = seconds * clock->rate;
  return 0;
}
