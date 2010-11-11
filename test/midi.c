#include "test.h"
#include "midi/midi.h"

int test001_midi( void ) {
   ASSERT_EQUAL( MIDI_LSB( 127 ), 127,    "MIDI_LSB macro computes incorrect value." );
   ASSERT_EQUAL( MIDI_MSB( 127 ),   0,    "MIDI_MSB macro computes incorrect value." );
   ASSERT_EQUAL( MIDI_LSB( 128 ),   0,    "MIDI_LSB macro computes incorrect value." );
   ASSERT_EQUAL( MIDI_MSB( 128 ),   1,    "MIDI_MSB macro computes incorrect value." );
   ASSERT_EQUAL( MIDI_MSB( 1<<13 ), 1<<6, "MIDI_MSB macro computes incorrect value." );
   ASSERT_EQUAL( MIDI_MSB( 1<<14 ), 0,    "MIDI_MSB macro computes incorrect value." );
   return 0;
}
