#include <string.h>
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
  ASSERT( MIDIMessageEncode( message, 8, &buffer[0], NULL ) == 0, "Could not read MIDI data from message." );
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
  MIDIMessageRelease( message );
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
  unsigned char * result = &buffer[0];
  MIDIValue values[2] = { 123, 234 };

  ASSERT( message != NULL, "Could not create system exclusive message." );
  ASSERT( MIDIMessageSet( message, MIDI_MANUFACTURER_ID, sizeof(MIDIManufacturerId), &values[0] ) == 0, "Could not set manufacturer id." );
  ASSERT( MIDIMessageSet( message, MIDI_MANUFACTURER_ID, sizeof(MIDIManufacturerId), &values[1] ) != 0, "Can set invalid manufacturer id." );
  ASSERT( MIDIMessageSet( message, MIDI_SYSEX_DATA, sizeof(void*), &result ) == 0, "Could not set system exclusive data." );
  ASSERT( MIDIMessageGet( message, MIDI_SYSEX_DATA, sizeof(void*), &result ) == 0, "Could not get system exclusive data." );
  ASSERT_EQUAL( buffer[0], result[0], "Sysex-data was not stored properly." );
  ASSERT_EQUAL( buffer[1], result[1], "Sysex-data was not stored properly." );
  ASSERT_EQUAL( buffer[2], result[2], "Sysex-data was not stored properly." );
  ASSERT_EQUAL( buffer[3], result[3], "Sysex-data was not stored properly." );
  ASSERT_EQUAL( buffer[4], result[4], "Sysex-data was not stored properly." );
  ASSERT_EQUAL( buffer[5], result[5], "Sysex-data was not stored properly." );
  ASSERT_EQUAL( buffer[6], result[6], "Sysex-data was not stored properly." );
  ASSERT_EQUAL( buffer[7], result[7], "Sysex-data was not stored properly." );
  MIDIMessageRelease( message );
  return 0;
}

/**
 * Test that running status coding works properly.
 */
int test006_message( void ) {
  struct MIDIMessageList messages[12];
  MIDIChannel  chan[2] = { MIDI_CHANNEL_8, MIDI_CHANNEL_7 };
  MIDIKey      keys[2] = { 63, 54 };
  MIDIVelocity vels[5] = { 127, 76, 64, 30, 70 };

  MIDIStatus   status;
  MIDIChannel  channel;
  MIDIKey      key;
  MIDIVelocity velocity;
  
  unsigned char buffer[32] = { 0 };
  unsigned char expect[32] = {
    /* note on with status byte */
    0x90 + chan[0],
    keys[0], vels[0],
    /* next note on, running status byte */
    keys[1], vels[1],
    /* note off with status byte */
    0x80 + chan[0],
    keys[0], vels[2],
    /* reset, real time message, does not affect running status */
    0xff,
    /* note off, running status byte */
    keys[1], vels[3],
    /* note off with status byte, because of different channel */
    0x80 + chan[1],
    keys[0], vels[4],
    0
  };
  size_t bytes = 0;

  messages[0].next  = &(messages[1]);
  messages[1].next  = &(messages[2]);
  messages[2].next  = &(messages[3]);
  messages[3].next  = &(messages[4]);
  messages[4].next  = &(messages[5]);
  messages[5].next  = NULL;
  messages[6].next  = &(messages[7]);
  messages[7].next  = &(messages[8]);
  messages[8].next  = &(messages[9]);
  messages[9].next  = &(messages[10]);
  messages[10].next = &(messages[11]);
  messages[11].next = NULL;
  
  messages[0].message = MIDIMessageCreate( MIDI_STATUS_NOTE_ON );
  messages[1].message = MIDIMessageCreate( MIDI_STATUS_NOTE_ON );
  messages[2].message = MIDIMessageCreate( MIDI_STATUS_NOTE_OFF );
  messages[3].message = MIDIMessageCreate( MIDI_STATUS_RESET );
  messages[4].message = MIDIMessageCreate( MIDI_STATUS_NOTE_OFF );
  messages[5].message = MIDIMessageCreate( MIDI_STATUS_NOTE_OFF );
  
  messages[6].message  = MIDIMessageCreate( 0 );
  messages[7].message  = MIDIMessageCreate( 0 );
  messages[8].message  = MIDIMessageCreate( 0 );
  messages[9].message  = MIDIMessageCreate( 0 );
  messages[10].message = MIDIMessageCreate( 0 );
  messages[11].message = MIDIMessageCreate( 0 );
  
  ASSERT_NO_ERROR( MIDIMessageSet( messages[0].message, MIDI_CHANNEL, sizeof(MIDIChannel), &(chan[0]) ),
                   "Could not set channel of message 0." );
  ASSERT_NO_ERROR( MIDIMessageSet( messages[0].message, MIDI_KEY, sizeof(MIDIKey), &(keys[0]) ),
                   "Could not set key of message 0." );
  ASSERT_NO_ERROR( MIDIMessageSet( messages[0].message, MIDI_VELOCITY, sizeof(MIDIVelocity), &(vels[0]) ),
                   "Could not set velocity of message 0." );

  ASSERT_NO_ERROR( MIDIMessageSet( messages[1].message, MIDI_CHANNEL, sizeof(MIDIChannel), &(chan[0]) ),
                   "Could not set channel of message 1." );
  ASSERT_NO_ERROR( MIDIMessageSet( messages[1].message, MIDI_KEY, sizeof(MIDIKey), &(keys[1]) ),
                   "Could not set key of message 1." );
  ASSERT_NO_ERROR( MIDIMessageSet( messages[1].message, MIDI_VELOCITY, sizeof(MIDIVelocity), &(vels[1]) ),
                   "Could not set velocity of message 1." );

  ASSERT_NO_ERROR( MIDIMessageSet( messages[2].message, MIDI_CHANNEL, sizeof(MIDIChannel), &(chan[0]) ),
                   "Could not set channel of message 2." );
  ASSERT_NO_ERROR( MIDIMessageSet( messages[2].message, MIDI_KEY, sizeof(MIDIKey), &(keys[0]) ),
                   "Could not set key of message 2." );
  ASSERT_NO_ERROR( MIDIMessageSet( messages[2].message, MIDI_VELOCITY, sizeof(MIDIVelocity), &(vels[2]) ),
                   "Could not set velocity of message 2." );

  /* Reset message is real-time, has no properties. */

  ASSERT_NO_ERROR( MIDIMessageSet( messages[4].message, MIDI_CHANNEL, sizeof(MIDIChannel), &(chan[0]) ),
                   "Could not set channel of message 4." );
  ASSERT_NO_ERROR( MIDIMessageSet( messages[4].message, MIDI_KEY, sizeof(MIDIKey), &(keys[1]) ),
                   "Could not set key of message 4." );
  ASSERT_NO_ERROR( MIDIMessageSet( messages[4].message, MIDI_VELOCITY, sizeof(MIDIVelocity), &(vels[3]) ),
                   "Could not set velocity of message 4." );

  ASSERT_NO_ERROR( MIDIMessageSet( messages[5].message, MIDI_CHANNEL, sizeof(MIDIChannel), &(chan[1]) ),
                   "Could not set channel of message 5." );
  ASSERT_NO_ERROR( MIDIMessageSet( messages[5].message, MIDI_KEY, sizeof(MIDIKey), &(keys[0]) ),
                   "Could not set key of message 5." );
  ASSERT_NO_ERROR( MIDIMessageSet( messages[5].message, MIDI_VELOCITY, sizeof(MIDIVelocity), &(vels[4]) ),
                   "Could not set velocity of message 5." );
                   
  ASSERT_NO_ERROR( MIDIMessageEncodeList( &(messages[0]), sizeof(buffer), &(buffer[0]), &bytes ),
                   "Could not encode message list." );

  ASSERT_EQUAL( bytes, 14, "Wrote unexpected number of bytes." );
  ASSERT_EQUAL( memcmp( buffer, expect, 32 ), 0, "Encoded message has unexpected format." );
  
  ASSERT_NO_ERROR( MIDIMessageDecodeList( &(messages[6]), sizeof(expect), &(expect[0]), &bytes ),
                  "Could not decode message list." );

  ASSERT_NO_ERROR( MIDIMessageGet( messages[6].message, MIDI_STATUS, sizeof(MIDIStatus), &status ),
                   "Could not get status of out message 0." );
  ASSERT_NO_ERROR( MIDIMessageGet( messages[6].message, MIDI_CHANNEL, sizeof(MIDIChannel), &channel ),
                   "Could not get channel of out message 0." );
  ASSERT_NO_ERROR( MIDIMessageGet( messages[6].message, MIDI_KEY, sizeof(MIDIKey), &key ),
                   "Could not get key of out message 0." );
  ASSERT_NO_ERROR( MIDIMessageGet( messages[6].message, MIDI_VELOCITY, sizeof(MIDIVelocity), &velocity ),
                   "Could not get velocity of out message 0." );

  ASSERT_EQUAL( status,   MIDI_STATUS_NOTE_ON, "Message 0 has wrong status." );
  ASSERT_EQUAL( channel,  MIDI_CHANNEL_8, "Message 0 has wrong channel." );
  ASSERT_EQUAL( key,      keys[0], "Message 0 has wrong key." );
  ASSERT_EQUAL( velocity, vels[0], "Message 0 has wrong velocity." );
  
  ASSERT_NO_ERROR( MIDIMessageGet( messages[7].message, MIDI_STATUS, sizeof(MIDIStatus), &status ),
                   "Could not get status of out message 1." );
  ASSERT_NO_ERROR( MIDIMessageGet( messages[7].message, MIDI_CHANNEL, sizeof(MIDIChannel), &channel ),
                   "Could not get channel of out message 1." );
  ASSERT_NO_ERROR( MIDIMessageGet( messages[7].message, MIDI_KEY, sizeof(MIDIKey), &key ),
                   "Could not get key of out message 1." );
  ASSERT_NO_ERROR( MIDIMessageGet( messages[7].message, MIDI_VELOCITY, sizeof(MIDIVelocity), &velocity ),
                   "Could not get velocity of out message 1." );

  ASSERT_EQUAL( status,   MIDI_STATUS_NOTE_ON, "Message 1 has wrong status." );
  ASSERT_EQUAL( channel,  MIDI_CHANNEL_8, "Message 1 has wrong channel." );
  ASSERT_EQUAL( key,      keys[1], "Message 1 has wrong key." );
  ASSERT_EQUAL( velocity, vels[1], "Message 1 has wrong velocity." );

  MIDIMessageRelease( messages[0].message );
  MIDIMessageRelease( messages[1].message );
  MIDIMessageRelease( messages[2].message );
  MIDIMessageRelease( messages[3].message );
  MIDIMessageRelease( messages[4].message );
  MIDIMessageRelease( messages[5].message );
  
  MIDIMessageRelease( messages[6].message );
  MIDIMessageRelease( messages[7].message );
  MIDIMessageRelease( messages[8].message );
  MIDIMessageRelease( messages[9].message );
  MIDIMessageRelease( messages[10].message );
  MIDIMessageRelease( messages[11].message );
  return 0;
}
