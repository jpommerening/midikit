#include <time.h>
#include "test.h"
#include "midi/clock.h"

/**
 * Test that the clock ticks.
 */
int test001_clock( void ) {
  int i;
  MIDIClock start;
  MIDIClock clock;
  ASSERT( MIDIClockGetNow( &start ) == 0, "Could not get current clock time." );
  clock = start;
  for( i=0; i<CLOCKS_PER_SEC; i++ ) {
    ASSERT( MIDIClockGetNow( &clock ) == 0, "Could not get current clock time." );
    ASSERT( clock >= start, "MIDIClock ticks backwards!" );
    if( clock > start ) {
      return 0;
    }
  }
  ASSERT( 0, "MIDIClock does not tick." );
  return 1;
}

/**
 * Test that the clock ticks approximately right.
 */
int test002_clock( void ) {
  MIDIClock start_mclock;
  MIDIClock mclock;
  clock_t start_cclock;
  clock_t cclock;

  ASSERT( MIDIClockSetSamplingRate( MIDI_SAMPLING_RATE_44K1HZ ) == 0, "Failed to set 44.1KHz sampling rate!" );
  ASSERT( MIDIClockGetNow( &start_mclock ) == 0, "Could not get current clock time." );
  start_cclock = clock();
  do {
    cclock = clock();
  } while( cclock < start_cclock + CLOCKS_PER_SEC );
  ASSERT( MIDIClockGetNow( &mclock ) == 0, "Could not get current clock time." );
  ASSERT( mclock >= ( start_mclock + 44099 ), "MIDIClock ticks too slow!" );
  ASSERT( mclock <= ( start_mclock + 44101 ), "MIDIClock ticks too fast!" );
  return 0;
}

/**
 * Test that clock-second conversion works properly.
 */
int test003_clock( void ) {
  MIDIClock clock = 96000;
  double seconds;
  ASSERT( MIDIClockSetSamplingRate( MIDI_SAMPLING_RATE_96KHZ ) == 0, "Failed to set 96KHz sampling rate!" );
  ASSERT( MIDIClockToSeconds( clock, &seconds ) == 0, "Failed to convert to seconds!" );
  ASSERT_EQUAL( seconds, 1.0, "MIDIClockToSeconds conversion incorrect." );
  ASSERT( MIDIClockFromSeconds( &clock, 2.5 ) == 0, "Failed to convert from seconds!" );
  ASSERT_EQUAL( clock, 240000, "MIDIClockFromSeconds conversion incorrect." );
  return 0;
}
