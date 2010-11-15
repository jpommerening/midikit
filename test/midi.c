#include "test.h"
#include "midi/midi.h"

/**
 * Test that the MIDI_LSB and MIDI_MSB macros work.
 */
int test001_midi( void ) {
  ASSERT_EQUAL( MIDI_LSB( 127 ), 127,    "MIDI_LSB macro computes incorrect value." );
  ASSERT_EQUAL( MIDI_MSB( 127 ),   0,    "MIDI_MSB macro computes incorrect value." );
  ASSERT_EQUAL( MIDI_LSB( 128 ),   0,    "MIDI_LSB macro computes incorrect value." );
  ASSERT_EQUAL( MIDI_MSB( 128 ),   1,    "MIDI_MSB macro computes incorrect value." );
  ASSERT_EQUAL( MIDI_MSB( 1<<13 ), 1<<6, "MIDI_MSB macro computes incorrect value." );
  ASSERT_EQUAL( MIDI_MSB( 1<<14 ), 0,    "MIDI_MSB macro computes incorrect value." );
  return 0;
}

/**
 * Test that the MIDI_LONG_VALUE macro works.
 */
int test002_midi( void ) {
  ASSERT_EQUAL( MIDI_LONG_VALUE( 0x01, 0x01 ), (1<<7)+1,  "MIDI_LONG_VALUE macro computes incorrect value." );
  ASSERT_EQUAL( MIDI_LONG_VALUE( 0x40, 0x00 ),  1<<13,    "MIDI_LONG_VALUE macro computes incorrect value." );
  ASSERT_EQUAL( MIDI_LONG_VALUE( 0x7f, 0x7f ), (1<<14)-1, "MIDI_LONG_VALUE macro computes incorrect value." );
  return 0;
}

/**
 * Test that the MIDI_LONG_VALUE, MIDI_LSB and MIDI_MSB are orthogonal.
 */
int test003_midi( void ) {
  MIDILongValue v = 12345;
  ASSERT_EQUAL( MIDI_LONG_VALUE( MIDI_MSB( v ), MIDI_LSB( v ) ), v,
                "MIDI_LONG_VALUE and MIDI_MSB / MIDI_LSB are not orthogonal." );
  return 0;
}