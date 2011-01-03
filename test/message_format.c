#include <stdlib.h>
#include "test.h"
#include "midi/message_format.h"

/**
 * Test that note off works.
 */
int test001_message_format( void ) {
  unsigned char buffer[3] = { MIDI_NIBBLE_VALUE(MIDI_STATUS_NOTE_OFF,MIDI_CHANNEL_1), 123, 123 };
  struct MIDIMessageData * message = malloc( sizeof( struct MIDIMessageData ) );
  struct MIDIMessageFormat * format;

  MIDIStatus    status;
  MIDIChannel   channel;
  MIDIKey       key;
  MIDIVelocity  velocity;
  
  format = MIDIMessageFormatDetect( &buffer[0] );
  ASSERT_NOT_EQUAL( format, NULL, "Could not detect message format of note off buffer." );
  ASSERT_NO_ERROR( MIDIMessageFormatDecode( format, message, sizeof(buffer), &buffer[0], NULL ), "Could not decode note off buffer." );
  ASSERT_NO_ERROR( MIDIMessageFormatGet( format, message, MIDI_STATUS, sizeof(MIDIStatus), &status ), "Could not get message status." );
  ASSERT_EQUAL( status, MIDI_STATUS_NOTE_OFF, "Stored wrong status." );
  ASSERT_NO_ERROR( MIDIMessageFormatGet( format, message, MIDI_CHANNEL, sizeof(MIDIChannel), &channel ) , "Could not get message channel." );
  ASSERT_EQUAL( channel, MIDI_CHANNEL_1, "Stored wrong channel." );
  ASSERT_NO_ERROR( MIDIMessageFormatGet( format, message, MIDI_KEY, sizeof(MIDIKey), &key ), "Could not get message key." );
  ASSERT_EQUAL( key, 123, "Stored wrong key." );
  ASSERT_NO_ERROR( MIDIMessageFormatGet( format, message, MIDI_VELOCITY, sizeof(MIDIVelocity), &velocity ), "Could not get message velocity." );  
  ASSERT_EQUAL( velocity, 123, "Stored wrong velocity." );

  free( message );
  return 0;
}


/**
 * Test that pitch wheel change works.
 */
int test002_message_format( void ) {
  unsigned char buffer[3] = { MIDI_NIBBLE_VALUE(MIDI_STATUS_PITCH_WHEEL_CHANGE,MIDI_CHANNEL_1), MIDI_LSB(12345), MIDI_MSB(12345) };
  struct MIDIMessageData * message = malloc( sizeof( struct MIDIMessageData ) );
  struct MIDIMessageFormat * format;

  MIDIStatus    status;
  MIDIChannel   channel;
  MIDIValue     lsb, msb;
  MIDILongValue value;  
  
  format = MIDIMessageFormatDetect( &buffer[0] );
  ASSERT_NOT_EQUAL( format, NULL, "Could not detect message format of pitch wheel change buffer." );
  ASSERT_NO_ERROR( MIDIMessageFormatDecode( format, message, sizeof(buffer), &buffer[0], NULL ), "Could not decode pitch wheel change buffer." );
  ASSERT_NO_ERROR( MIDIMessageFormatGet( format, message, MIDI_STATUS, sizeof(MIDIStatus), &status ), "Could not get message status." );
  ASSERT_EQUAL( status, MIDI_STATUS_PITCH_WHEEL_CHANGE, "Stored wrong status." );
  ASSERT_NO_ERROR( MIDIMessageFormatGet( format, message, MIDI_CHANNEL, sizeof(MIDIChannel), &channel ) , "Could not get message channel." );
  ASSERT_EQUAL( channel, MIDI_CHANNEL_1, "Stored wrong channel." );
  ASSERT_NO_ERROR( MIDIMessageFormatGet( format, message, MIDI_VALUE_LSB, sizeof(MIDIValue), &lsb ), "Could not get message value lsb." );
  ASSERT_EQUAL( lsb, MIDI_LSB(12345), "Stored wrong value lsb." );
  ASSERT_NO_ERROR( MIDIMessageFormatGet( format, message, MIDI_VALUE_MSB, sizeof(MIDIValue), &msb ), "Could not get message value msb." );
  ASSERT_EQUAL( msb, MIDI_MSB(12345), "Stored wrong value msb." );
  ASSERT_NO_ERROR( MIDIMessageFormatGet( format, message, MIDI_VALUE, sizeof(MIDILongValue), &value ), "Could not get message value." );  
  ASSERT_EQUAL( value, 12345, "Stored wrong value." );

  free( message );
  return 0;
}

/**
 * Test that system exclusive works.
 */
int test003_message_format( void ) {
  unsigned char buffer[16] = { MIDI_STATUS_SYSTEM_EXCLUSIVE, 123, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14 };
  struct MIDIMessageData * message = malloc( sizeof( struct MIDIMessageData ) );
  int i;
  struct MIDIMessageFormat * format;

  MIDIStatus         status;
  MIDIManufacturerId manufacturer_id;
  size_t sysex_size;
  unsigned char * sysex_data;
  
  format = MIDIMessageFormatDetect( &buffer[0] );
  ASSERT_NOT_EQUAL( format, NULL, "Could not detect message format of system exclusive buffer." );
  ASSERT_NO_ERROR( MIDIMessageFormatDecode( format, message, sizeof(buffer), &buffer[0], NULL ), "Could not decode system exclusive buffer." );
  ASSERT_NO_ERROR( MIDIMessageFormatGet( format, message, MIDI_STATUS, sizeof(MIDIStatus), &status ), "Could not get message status." );
  ASSERT_EQUAL( status, MIDI_STATUS_SYSTEM_EXCLUSIVE, "Stored wrong status." );
  ASSERT_NO_ERROR( MIDIMessageFormatGet( format, message, MIDI_MANUFACTURER_ID, sizeof(MIDIManufacturerId), &manufacturer_id ) , "Could not get message manufacturer id." );
  ASSERT_EQUAL( manufacturer_id, 123, "Stored wrong manufacturer id." );
  ASSERT_NO_ERROR( MIDIMessageFormatGet( format, message, MIDI_SYSEX_DATA, sizeof(void*), &sysex_data ), "Could not get system exclusive data." );
  ASSERT_NO_ERROR( MIDIMessageFormatGet( format, message, MIDI_SYSEX_SIZE, sizeof(size_t), &sysex_size ), "Could not get system exclusive size." );
  ASSERT_EQUAL( sysex_size, 14, "System exclusive message was truncated." );
  for( i=0; i<sysex_size; i++ ) {
    ASSERT_EQUAL( sysex_data[i], buffer[i+2], "Stored wrong system exclusive data." );
  }

  if( message->data != NULL && message->bytes[3] == 1 )
    free( message->data );
  free( message );
  return 0;
}
