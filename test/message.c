#include "test.h"
#include "midi/message.h"

/**
 * Test that MIDI messages can be created properly and
 * produce the expected bitstream.
 */
int test001_message( void ) {
   struct MIDIMessage * message = MIDIMessageCreate( MIDI_STATUS_NOTE_OFF );
   unsigned char buffer[8] = { 0 };
   MIDIValue values[3] = { MIDI_CHANNEL_1, 60, 127 };
   ASSERT( message != NULL, "Could not create note off message." );
   ASSERT( MIDIMessageSet( message, MIDI_CHANNEL,  sizeof(MIDIChannel),  &values[0] ) == 0, "Could not set channel." );
   ASSERT( MIDIMessageSet( message, MIDI_KEY,      sizeof(MIDIKey),      &values[1] ) == 0, "Could not set key." );
   ASSERT( MIDIMessageSet( message, MIDI_VELOCITY, sizeof(MIDIVelocity), &values[2] ) == 0, "Could not set velocity." );
   ASSERT( MIDIMessageEncode( message, 8, &buffer[0] ) == 0, "Could not read MIDI data from message." );
   ASSERT_EQUAL( buffer[0], 0x80, "Read wrong status / channel byte!" );
   ASSERT_EQUAL( buffer[1], 60,   "Read wrong key byte!" );
   ASSERT_EQUAL( buffer[2], 127,  "Read wrong velocity byte!" );
   ASSERT_EQUAL( buffer[3], 0,    "Trailing data in message!" );
   MIDIMessageRelease( message );
   return 0;
}

/**
 * Test that MIDI messages can not be created with an invalid status.
 */
int test002_message( void ) {
   struct MIDIMessage * message = MIDIMessageCreate( 0x81 );
   ASSERT( message == NULL, "Can create MIDI message with invalid status." );
   return 0;
}

/**
 * Test that the MIDI channel number can not exceed 15 (zero based.)
 */
int test003_message( void ) {
   struct MIDIMessage * message = MIDIMessageCreate( MIDI_STATUS_NOTE_ON );
   MIDIValue values[3] = { MIDI_CHANNEL_16, 16 };
   ASSERT_NOT_EQUAL( message, NULL, "Could not create note on message." );
   ASSERT( MIDIMessageSet( message, MIDI_CHANNEL, sizeof(MIDIChannel), &values[0] ) == 0, "Could not set channel number 16." );
   ASSERT( MIDIMessageSet( message, MIDI_CHANNEL, sizeof(MIDIChannel), &values[1] ) != 0, "Can set invalid channel number." );
   return 0;
}

/**
 * Test that messages can not be created with a status that is reserved for
 * future use or intended to end the system exclusive message.
 */
int test004_message( void ) {
   struct MIDIMessage * message;

   message = MIDIMessageCreate( MIDI_STATUS_UNDEFINED0 );
   ASSERT_EQUAL( message, NULL, "Can create MIDI message with undefined (reserved 0) status." );
   message = MIDIMessageCreate( MIDI_STATUS_UNDEFINED1 );
   ASSERT_EQUAL( message, NULL, "Can create MIDI message with undefined (reserved 1) status." );
   message = MIDIMessageCreate( MIDI_STATUS_UNDEFINED2 );
   ASSERT_EQUAL( message, NULL, "Can create MIDI message with undefined (reserved 2) status." );
   message = MIDIMessageCreate( MIDI_STATUS_UNDEFINED3 );
   ASSERT_EQUAL( message, NULL, "Can create MIDI message with undefined (reserved 3) status." );
   message = MIDIMessageCreate( MIDI_STATUS_END_OF_EXCLUSIVE );
   ASSERT_EQUAL( message, NULL, "Can create MIDI message with end of exclusive status." );
   return 0;
}

/**
 * Test that system exclusive messages can be created properly and
 * produce the expected bitstream.
 */
int test005_message( void ) {
   struct MIDIMessage * message = MIDIMessageCreate( MIDI_STATUS_SYSTEM_EXCLUSIVE );
   unsigned char buffer[8] = { 1, 2, 3, 4, 5, 6, 7, 8 };
   unsigned char result[14];
   MIDIValue values[2] = { 123, 234 };

   ASSERT( message != NULL, "Could not create system exclusive message." );
   ASSERT( MIDIMessageSet( message, MIDI_MANUFACTURER_ID, sizeof(MIDIManufacturerId), &values[0] ) == 0, "Could not set manufacturer id." );
   ASSERT( MIDIMessageSet( message, MIDI_MANUFACTURER_ID, sizeof(MIDIManufacturerId), &values[1] ) != 0, "Can set invalid manufacturer id." );
   ASSERT( MIDIMessageSet( message, MIDI_SYSEX_DATA, sizeof(buffer), &buffer[0] ) == 0, "Could not set system exclusive data." );
   ASSERT( MIDIMessageGet( message, MIDI_SYSEX_DATA, sizeof(result), &result[0] ) == 0, "Could not get system exclusive data." );
   ASSERT_EQUAL( buffer[0], result[0], "Sysex-data was not stored properly." );
   ASSERT_EQUAL( buffer[1], result[1], "Sysex-data was not stored properly." );
   ASSERT_EQUAL( buffer[2], result[2], "Sysex-data was not stored properly." );
   ASSERT_EQUAL( buffer[3], result[3], "Sysex-data was not stored properly." );
   ASSERT_EQUAL( buffer[4], result[4], "Sysex-data was not stored properly." );
   ASSERT_EQUAL( buffer[5], result[5], "Sysex-data was not stored properly." );
   ASSERT_EQUAL( buffer[6], result[6], "Sysex-data was not stored properly." );
   ASSERT_EQUAL( buffer[7], result[7], "Sysex-data was not stored properly." );
   return 0;
}
