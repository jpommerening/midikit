#include <unistd.h>
#include "test.h"
#include "midi/clock.h"

/**
 * Test that the clock ticks.
 */
int test001_clock( void ) {
  struct MIDIClock * clock = MIDIClockCreate( MIDI_SAMPLING_RATE_DEFAULT );
  MIDITimestamp start, ticks;

  ASSERT_NOT_EQUAL( clock, NULL, "Could not create MIDI clock." );
  ASSERT_NO_ERROR( MIDIClockGetNow( clock, &start ), "Could not get current clock time." );
  usleep( 1000000 / MIDI_SAMPLING_RATE_DEFAULT + 100 );
  ASSERT_NO_ERROR( MIDIClockGetNow( clock, &ticks ), "Could not get current clock time." );
  ASSERT_GREATER_OR_EQUAL( ticks, start, "MIDIClock ticks backwards!" );
  if( ticks > start ) {
    MIDIClockRelease( clock );
    return 0;
  }
  ASSERT( 0, "MIDIClock does not tick." );
  return 1;
}

/**
 * Test that the clock ticks approximately right.
 */
int test002_clock( void ) {
  struct MIDIClock * clock = MIDIClockCreate( MIDI_SAMPLING_RATE_44K1HZ );
  MIDITimestamp start, ticks;

  ASSERT_NOT_EQUAL( clock, NULL, "Could not create MIDI clock." );
  ASSERT_NO_ERROR( MIDIClockGetNow( clock, &start ), "Could not get current clock time." );
  sleep( 1 );
  ASSERT_NO_ERROR( MIDIClockGetNow( clock, &ticks ), "Could not get current clock time." );
  ASSERT_NEAR( ticks, (start + 44100), "MIDIClock does not tick close enough to 44.1KHz!" );
  MIDIClockRelease( clock );
  return 0;
}

/**
 * Test that clock-second conversion works properly.
 */
int test003_clock( void ) {
  struct MIDIClock * clock = MIDIClockCreate( MIDI_SAMPLING_RATE_44K1HZ );
  MIDITimestamp timestamp = 96000;
  double seconds;

  ASSERT_NOT_EQUAL( clock, NULL, "Could not create MIDI clock." );
  ASSERT_NO_ERROR( MIDIClockSetSamplingRate( clock, MIDI_SAMPLING_RATE_96KHZ ), "Failed to set 96KHz sampling rate!" );
  ASSERT_NO_ERROR( MIDIClockTimestampToSeconds( clock, timestamp, &seconds ), "Failed to convert to seconds!" );
  ASSERT_EQUAL( seconds, 1.0, "MIDIClockToSeconds conversion incorrect." );
  ASSERT_NO_ERROR( MIDIClockTimestampFromSeconds( clock, &timestamp, 2.5 ), "Failed to convert from seconds!" );
  ASSERT_EQUAL( timestamp, 240000, "MIDIClockFromSeconds conversion incorrect." );
  MIDIClockRelease( clock );
  return 0;
}

/**
 * Test that set method works properly.
 */
int test004_clock( void ) {
  struct MIDIClock * clock = MIDIClockCreate( MIDI_SAMPLING_RATE_96KHZ );
  MIDITimestamp timestamp;

  ASSERT_NOT_EQUAL( clock, NULL, "Could not create MIDI clock." );
  ASSERT_NO_ERROR( MIDIClockGetNow( clock, &timestamp ), "Could not get current clock time." );
  ASSERT_NEAR_GREATER( timestamp, 0, "Clock is not initialized with zero." );
  timestamp = -1000;
  ASSERT_NO_ERROR( MIDIClockSetNow( clock, timestamp ), "Could not set current clock time." );
  MIDIClockGetNow( clock, &timestamp );
  ASSERT_NEAR_GREATER( timestamp, -1000, "Setting clock did not work." );
  usleep( 1000 );
  MIDIClockGetNow( clock, &timestamp );
  ASSERT_GREATER( timestamp, -1000, "Clock stopped ticking after resetting." );
  MIDIClockRelease( clock );
  return 0;
}

/**
 * Test that the clocks precision is sufficient, even on 192KHz.
 */
int test005_clock( void ) {
  struct MIDIClock * clock = MIDIClockCreate( MIDI_SAMPLING_RATE_192KHZ );
  MIDITimestamp start, ticks;
  int i;

  ASSERT_NOT_EQUAL( clock, NULL, "Could not create MIDI clock." );
  ASSERT_NO_ERROR( MIDIClockGetNow( clock, &start ), "Could not get current clock time." );
  for( i=0; i<10000; i++ ) {
    MIDIClockGetNow( clock, &ticks );
    if( ticks != start ) break;
  }
  ASSERT( ticks > start, "Clock did not tick in 1000 loop cycles." );
  ASSERT_EQUAL( ticks, start+1, "Clock ticked more than 1 step. Insufficient precision." );
  MIDIClockRelease( clock );
  return 0;
}
