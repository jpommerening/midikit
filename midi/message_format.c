#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include "message_format.h"

#define VOID_BYTE( buffer, n ) ((unsigned char*)buffer)[n]

/**
 * Structure of MIDI message object.
 */
struct MIDIMessage {
  size_t refs;
  struct MIDIMessageFormat * format;
  struct MIDIMessageData data;
//MIDITimestamp timestamp;
};

#pragma mark Encoding & decoding
/**
 * Encoding & decoding functions.
 * These functions encode a MIDI message to or from a stream.
 */
//@{

int MIDIMessageDataInit( struct MIDIMessageData * data ) {
  if( data == NULL ) return 1;
  data->bytes[0] = 0;
  data->bytes[1] = 0;
  data->bytes[2] = 0;
  data->bytes[3] = 0;
  data->size = 0;
  data->data = NULL;
  return 0;
}

int MIDIMessageDataEncode( struct MIDIMessageData * data, size_t size, void * buffer ) {
  if( data == NULL || buffer == NULL ) return 1;
  if( size > data->size ) {
    size = data->size;
  }
  if( data->size > sizeof( data->bytes ) ) {
    if( data->data != NULL ) {
      memcpy( buffer, data->data, size );
    }
  } else {
    memcpy( buffer, &(data->bytes[0]), size );
  }
  return 0;
}

int MIDIMessageDataDecode( struct MIDIMessageData * data, size_t size, void * buffer ) {
  if( data == NULL || buffer == NULL ) return 1;
  if( size > sizeof( data->bytes ) ) {
    if( data->size <= sizeof( data->bytes ) ) {
      data->data = malloc( size ); // previously 
    } else if( data->data == NULL ) {
      data->data = malloc( size );
    } else {
      data->data = realloc( data->data, size );
    }
  }
  return 0;
}

static int _encode_one_byte( struct MIDIMessageData * data, size_t size, void * buffer ) {
  if( data == NULL || buffer == NULL ) return 1;
  if( size < 1 ) return 1;
  VOID_BYTE(buffer,0) = data->bytes[0];
  return 0;
}

static int _decode_one_byte( struct MIDIMessageData * data, size_t size, void * buffer ) {
  if( data == NULL || buffer == NULL ) return 1;
  if( size < 1 ) return 1;
  data->bytes[0] = VOID_BYTE(buffer,0);
  return 0;
}

static int _encode_two_bytes( struct MIDIMessageData * data, size_t size, void * buffer ) {
  if( data == NULL || buffer == NULL ) return 1;
  if( size < 2 ) return 1;
  VOID_BYTE(buffer,0) = data->bytes[0];
  VOID_BYTE(buffer,1) = data->bytes[1];
  return 0;
}

static int _decode_two_bytes( struct MIDIMessageData * data, size_t size, void * buffer ) {
  if( data == NULL || buffer == NULL ) return 1;
  if( size < 2 ) return 1;
  data->bytes[0] = VOID_BYTE(buffer,0);
  data->bytes[1] = VOID_BYTE(buffer,1);
  return 0;
}

static int _encode_three_bytes( struct MIDIMessageData * data, size_t size, void * buffer ) {
  if( data == NULL || buffer == NULL ) return 1;
  if( size < 3 ) return 1;
  VOID_BYTE(buffer,0) = data->bytes[0];
  VOID_BYTE(buffer,1) = data->bytes[1];
  VOID_BYTE(buffer,2) = data->bytes[2];
  return 0;
}

static int _decode_three_bytes( struct MIDIMessageData * data, size_t size, void * buffer ) {
  if( data == NULL || buffer == NULL ) return 1;
  if( size < 3 ) return 1;
  data->bytes[0] = VOID_BYTE(buffer,0);
  data->bytes[1] = VOID_BYTE(buffer,1);
  data->bytes[2] = VOID_BYTE(buffer,2);
  return 0;
}

static int _encode_system_exclusive( struct MIDIMessageData * data, size_t size, void * buffer ) {
  if( data == NULL || buffer == NULL ) return 1;
  if( size < data->size+3 ) return 1;
  VOID_BYTE(buffer,0) = data->bytes[0];
  VOID_BYTE(buffer,1) = data->bytes[1];
  if( data->size > 0 && data->data != NULL )
    memcpy( buffer+2, data->data, data->size );
  VOID_BYTE(buffer,data->size+2) = data->bytes[2];
  return 0;  
}

// duh … rewrite the sysex:
// - incorporate end of exclusive as a message
// - send system exclusive messages (up to a predetermined size)
//   as long as no other status (1st bit) is received (except real time)
// - the function below can rely on the size it was given to parse the data
// - it will not receive an "end of exclusive" status byte

static int _decode_system_exclusive( struct MIDIMessageData * data, size_t size, void * buffer ) {
  if( data == NULL || buffer == NULL ) return 1;
  data->bytes[0] = VOID_BYTE(buffer,0);
  data->bytes[1] = VOID_BYTE(buffer,1);
  data->data = malloc( size-2 );
  memcpy( data->data, (buffer+2), size );
  data->size = size;
  return 0;
}

//@}

#pragma mark Message format detectors
/**
 * Message format detectors.
 */
//@{

static int _test_note_off_on( void * buffer ) {
  return (VOID_BYTE(buffer,0) & 0xf0) == (MIDI_STATUS_NOTE_OFF<<4)
      || (VOID_BYTE(buffer,0) & 0xf0) == (MIDI_STATUS_NOTE_ON<<4);
};

static int _test_polyphonic_key_pressure( void * buffer ) {
  return (VOID_BYTE(buffer,0) & 0xf0) == (MIDI_STATUS_POLYPHONIC_KEY_PRESSURE<<4);
};

static int _test_control_change( void * buffer ) {
  return (VOID_BYTE(buffer,0) & 0xf0) == (MIDI_STATUS_CONTROL_CHANGE<<4);
};

static int _test_program_change( void * buffer ) {
  return (VOID_BYTE(buffer,0) & 0xf0) == (MIDI_STATUS_PROGRAM_CHANGE<<4);
};

static int _test_channel_pressure( void * buffer ) {
  return (VOID_BYTE(buffer,0) & 0xf0) == (MIDI_STATUS_CHANNEL_PRESSURE<<4);
};

static int _test_pitch_wheel_change( void * buffer ) {
  return (VOID_BYTE(buffer,0) & 0xf0) == (MIDI_STATUS_PITCH_WHEEL_CHANGE<<4);
};

static int _test_system_exclusive( void * buffer ) {
  return VOID_BYTE(buffer,0) == MIDI_STATUS_SYSTEM_EXCLUSIVE;
};

static int _test_time_code_quarter_frame( void * buffer ) {
  return VOID_BYTE(buffer,0) == MIDI_STATUS_TIME_CODE_QUARTER_FRAME;
};

static int _test_song_position_pointer( void * buffer ) {
  return VOID_BYTE(buffer,0) == MIDI_STATUS_SONG_POSITION_POINTER;
};

static int _test_song_select( void * buffer ) {
  return VOID_BYTE(buffer,0) == MIDI_STATUS_SONG_SELECT;
};

static int _test_tune_request( void * buffer ) {
  return VOID_BYTE(buffer,0) == MIDI_STATUS_TUNE_REQUEST;
};

static int _test_real_time( void * buffer ) {
  return (VOID_BYTE(buffer,0) >= MIDI_STATUS_TIMING_CLOCK)
      && (VOID_BYTE(buffer,0) <= MIDI_STATUS_RESET);
};

//@}

#pragma mark Getters and setters
/**
 * Getters and setters.
 * Functions to get ans set properties of the different messages.
 */
//@{

#include <stdio.h>

#define PROPERTY_CASE_BASE(flag,type) \
    case flag: \
      if( size != sizeof(type) ) return 1

#define PROPERTY_CASE_SET(flag,type,field) \
    PROPERTY_CASE_BASE(flag,type); \
      field = *((type*)value); \
      return 0

#define PROPERTY_CASE_SET_H(flag,type,field) \
    PROPERTY_CASE_BASE(flag,type); \
      field = MIDI_NIBBLE_VALUE( *((type*)value), MIDI_LOW_NIBBLE(field) ); \
      return 0

#define PROPERTY_CASE_SET_L(flag,type,field) \
    PROPERTY_CASE_BASE(flag,type); \
      field = MIDI_NIBBLE_VALUE( MIDI_HIGH_NIBBLE(field), *((type*)value) ); \
      return 0

#define PROPERTY_CASE_GET(flag,type,field) \
    PROPERTY_CASE_BASE(flag,type); \
      *((type*)value) = (type) field; \
      return 0

#define PROPERTY_CASE_GET_H(flag,type,field) PROPERTY_CASE_GET(flag,type,MIDI_HIGH_NIBBLE(field))
#define PROPERTY_CASE_GET_L(flag,type,field) PROPERTY_CASE_GET(flag,type,MIDI_LOW_NIBBLE(field))
      
#define PROPERTY_DEFAULT \
    default: \
      return 1

/**
 * Set properties of note on/off messages.
 */
static int _set_note_off_on( struct MIDIMessageData * data, MIDIProperty property, size_t size, void * value ) {
  uint8_t * m = &(data->bytes[0]);
  if( size == 0 || value == NULL ) return 1;
  switch( property ) {
  //PROPERTY_CASE_SET_H(MIDI_STATUS,MIDIStatus,m[0]);
    PROPERTY_CASE_SET_L(MIDI_CHANNEL,MIDIChannel,m[0]);
    PROPERTY_CASE_SET(MIDI_KEY,MIDIKey,m[1]);
    PROPERTY_CASE_SET(MIDI_VELOCITY,MIDIVelocity,m[2]);
    PROPERTY_DEFAULT;
  }
  return 1;
}

/**
 * Get properties of note on/off messages.
 */
static int _get_note_off_on( struct MIDIMessageData * data, MIDIProperty property, size_t size, void * value ) {
  uint8_t * m = &(data->bytes[0]);
  if( size == 0 || value == NULL ) return 1;
  switch( property ) {
    PROPERTY_CASE_GET_H(MIDI_STATUS,MIDIStatus,m[0]);
    PROPERTY_CASE_GET_L(MIDI_CHANNEL,MIDIChannel,m[0]);
    PROPERTY_CASE_GET(MIDI_KEY,MIDIKey,m[1]);
    PROPERTY_CASE_GET(MIDI_VELOCITY,MIDIVelocity,m[2]);
    PROPERTY_DEFAULT;
  }
  return 1;
}

/**
 * Set properties of polyphonic key pressure messages.
 */
static int _set_polyphonic_key_pressure( struct MIDIMessageData * data, MIDIProperty property, size_t size, void * value ) {
  uint8_t * m = &(data->bytes[0]);
  if( size == 0 || value == NULL ) return 1;
  switch( property ) {
  //PROPERTY_CASE_SET_H(MIDI_STATUS,MIDIStatus,m[0]);
    PROPERTY_CASE_SET_L(MIDI_CHANNEL,MIDIChannel,m[0]);
    PROPERTY_CASE_SET(MIDI_KEY,MIDIKey,m[1]);
    PROPERTY_CASE_SET(MIDI_PRESSURE,MIDIPressure,m[2]);
    PROPERTY_DEFAULT;
  }
  return 1;
}

/**
 * Get properties of polyphonic key pressure messages.
 */
static int _get_polyphonic_key_pressure( struct MIDIMessageData * data, MIDIProperty property, size_t size, void * value ) {
  uint8_t * m = &(data->bytes[0]);
  if( size == 0 || value == NULL ) return 1;
  switch( property ) {
    PROPERTY_CASE_GET_H(MIDI_STATUS,MIDIStatus,m[0]);
    PROPERTY_CASE_GET_L(MIDI_CHANNEL,MIDIChannel,m[0]);
    PROPERTY_CASE_GET(MIDI_KEY,MIDIKey,m[1]);
    PROPERTY_CASE_GET(MIDI_PRESSURE,MIDIPressure,m[2]);
    PROPERTY_DEFAULT;
  }
  return 1;
}

/**
 * Set properties of control change messages.
 */
static int _set_control_change( struct MIDIMessageData * data, MIDIProperty property, size_t size, void * value ) {
  uint8_t * m = &(data->bytes[0]);
  if( size == 0 || value == NULL ) return 1;
  switch( property ) {
  //PROPERTY_CASE_SET_H(MIDI_STATUS,MIDIStatus,m[0]);
    PROPERTY_CASE_SET_L(MIDI_CHANNEL,MIDIChannel,m[0]);
    PROPERTY_CASE_SET(MIDI_CONTROL,MIDIControl,m[1]);
    PROPERTY_CASE_SET(MIDI_VALUE,MIDIValue,m[2]);
    PROPERTY_DEFAULT;
  }
  return 1;
}

/**
 * Get properties of control change messages.
 */
static int _get_control_change( struct MIDIMessageData * data, MIDIProperty property, size_t size, void * value ) {
  uint8_t * m = &(data->bytes[0]);
  if( size == 0 || value == NULL ) return 1;
  switch( property ) {
    PROPERTY_CASE_GET_H(MIDI_STATUS,MIDIStatus,m[0]);
    PROPERTY_CASE_GET_L(MIDI_CHANNEL,MIDIChannel,m[0]);
    PROPERTY_CASE_GET(MIDI_CONTROL,MIDIControl,m[1]);
    PROPERTY_CASE_GET(MIDI_VALUE,MIDIValue,m[2]);
    PROPERTY_DEFAULT;
  }
  return 1;
}

/**
 * Set properties of program change messages.
 */
static int _set_program_change( struct MIDIMessageData * data, MIDIProperty property, size_t size, void * value ) {
  uint8_t * m = &(data->bytes[0]);
  if( size == 0 || value == NULL ) return 1;
  switch( property ) {
  //PROPERTY_CASE_SET_H(MIDI_STATUS,MIDIStatus,m[0]);
    PROPERTY_CASE_SET_L(MIDI_CHANNEL,MIDIChannel,m[0]);
    PROPERTY_CASE_SET(MIDI_PROGRAM,MIDIProgram,m[1]);
    PROPERTY_DEFAULT;
  }
  return 1;
}

/**
 * Get properties of program change messages.
 */
static int _get_program_change( struct MIDIMessageData * data, MIDIProperty property, size_t size, void * value ) {
  uint8_t * m = &(data->bytes[0]);
  if( size == 0 || value == NULL ) return 1;
  switch( property ) {
    PROPERTY_CASE_GET_H(MIDI_STATUS,MIDIStatus,m[0]);
    PROPERTY_CASE_GET_L(MIDI_CHANNEL,MIDIChannel,m[0]);
    PROPERTY_CASE_GET(MIDI_PROGRAM,MIDIProgram,m[1]);
    PROPERTY_DEFAULT;
  }
  return 1;
}

/**
 * Set properties of channel pressure messages.
 */
static int _set_channel_pressure( struct MIDIMessageData * data, MIDIProperty property, size_t size, void * value ) {
  uint8_t * m = &(data->bytes[0]);
  if( size == 0 || value == NULL ) return 1;
  switch( property ) {
  //PROPERTY_CASE_SET_H(MIDI_STATUS,MIDIStatus,m[0]);
    PROPERTY_CASE_SET_L(MIDI_CHANNEL,MIDIChannel,m[0]);
    PROPERTY_CASE_SET(MIDI_PRESSURE,MIDIPressure,m[1]);
    PROPERTY_DEFAULT;
  }
  return 1;
}

/**
 * Get properties of channel pressure messages.
 */
static int _get_channel_pressure( struct MIDIMessageData * data, MIDIProperty property, size_t size, void * value ) {
  uint8_t * m = &(data->bytes[0]);
  if( size == 0 || value == NULL ) return 1;
  switch( property ) {
    PROPERTY_CASE_GET_H(MIDI_STATUS,MIDIStatus,m[0]);
    PROPERTY_CASE_GET_L(MIDI_CHANNEL,MIDIChannel,m[0]);
    PROPERTY_CASE_GET(MIDI_PRESSURE,MIDIPressure,m[1]);
    PROPERTY_DEFAULT;
  }
  return 1;
}

/**
 * Set properties of pitch wheel change messages.
 */
static int _set_pitch_wheel_change( struct MIDIMessageData * data, MIDIProperty property, size_t size, void * value ) {
  uint8_t * m = &(data->bytes[0]);
  if( size == 0 || value == NULL ) return 1;
  switch( property ) {
  //PROPERTY_CASE_SET_H(MIDI_STATUS,MIDIStatus,m[0]);
    PROPERTY_CASE_SET_L(MIDI_CHANNEL,MIDIChannel,m[0]);
    PROPERTY_CASE_SET(MIDI_VALUE_LSB,MIDIValue,m[1]);
    PROPERTY_CASE_SET(MIDI_VALUE_MSB,MIDIValue,m[2]);
    PROPERTY_CASE_BASE(MIDI_VALUE,MIDILongValue);
      m[1] = MIDI_LSB(*(MIDILongValue*)value);
      m[2] = MIDI_MSB(*(MIDILongValue*)value);
      return 0;
    PROPERTY_DEFAULT;
  }
  return 1;
}

/**
 * Get properties of pitch wheel change messages.
 */
static int _get_pitch_wheel_change( struct MIDIMessageData * data, MIDIProperty property, size_t size, void * value ) {
  uint8_t * m = &(data->bytes[0]);
  if( size == 0 || value == NULL ) return 1;
  switch( property ) {
    PROPERTY_CASE_GET_H(MIDI_STATUS,MIDIStatus,m[0]);
    PROPERTY_CASE_GET_L(MIDI_CHANNEL,MIDIChannel,m[0]);
    PROPERTY_CASE_GET(MIDI_VALUE_LSB,MIDIValue,m[1]);
    PROPERTY_CASE_GET(MIDI_VALUE_MSB,MIDIValue,m[2]);
    PROPERTY_CASE_BASE(MIDI_VALUE,MIDILongValue);
      *(MIDILongValue*)value = MIDI_LONG_VALUE( m[2], m[1] );
      return 0;
    PROPERTY_DEFAULT;
  }
  return 1;
}

/**
 * Set properties of system exclusive messages.
 */
static int _set_system_exclusive( struct MIDIMessageData * data, MIDIProperty property, size_t size, void * value ) {
  if( size == 0 || value == NULL ) return 1;
  switch( property ) {
  //PROPERTY_CASE_SET(MIDI_STATUS,MIDIStatus,data->bytes[0]);
    PROPERTY_CASE_SET(MIDI_MANUFACTURER_ID,MIDIManufacturerId,data->bytes[1]);
  //PROPERTY_CASE_SET(MIDI_SYSEX_SIZE,size_t,data->size);
    case MIDI_SYSEX_DATA:
      if( data->size == 0 || data->data == NULL ) {
        data->data = malloc( size );
      } else {
        data->data = realloc( data->data, size );
      }
      data->size = size;
      memcpy( data->data, value, size );
      return 0;
    PROPERTY_DEFAULT;
  }
  return 1;
}

/**
 * Get properties of system exclusive messages.
 */
static int _get_system_exclusive( struct MIDIMessageData * data, MIDIProperty property, size_t size, void * value ) {
  if( size == 0 || value == NULL ) return 1;
  switch( property ) {
    PROPERTY_CASE_GET(MIDI_STATUS,MIDIStatus,data->bytes[0]);
    PROPERTY_CASE_GET(MIDI_MANUFACTURER_ID,MIDIManufacturerId,data->bytes[1]);
    PROPERTY_CASE_GET(MIDI_SYSEX_SIZE,size_t,data->size);
    PROPERTY_CASE_GET(MIDI_SYSEX_FRAGMENT,uint8_t,data->bytes[2]);
    case MIDI_SYSEX_DATA:
      if( data->size == 0 || data->data == NULL ) return 0;
      memcpy( value, data->data, (size < data->size) ? size : data->size );
      return 0;
    PROPERTY_DEFAULT;
  }
  return 1;
}

#undef PROPERTY_CASE_BASE
#undef PROPERTY_CASE_SET
#undef PROPERTY_CASE_SET_H
#undef PROPERTY_CASE_SET_L
#undef PROPERTY_CASE_GET
#undef PROPERTY_CASE_GET_H
#undef PROPERTY_CASE_GET_L
#undef PROPERTY_DEFAULT

//@}

/**
 * Message format definitions.
 * These definitions hold some message formats.
 */
//@{

static struct MIDIMessageFormat _note_off_on = {
  &_test_note_off_on,
  &_set_note_off_on,
  &_get_note_off_on,
  &_encode_three_bytes,
  &_decode_three_bytes
};

static struct MIDIMessageFormat _polyphonic_key_pressure = {
  &_test_polyphonic_key_pressure,
  &_set_polyphonic_key_pressure,
  &_get_polyphonic_key_pressure,
  &_encode_three_bytes,
  &_decode_three_bytes
};

static struct MIDIMessageFormat _control_change = {
  &_test_control_change,
  &_set_control_change,
  &_get_control_change,
  &_encode_three_bytes,
  &_decode_three_bytes
};

static struct MIDIMessageFormat _program_change = {
  &_test_program_change,
  &_set_program_change,
  &_get_program_change,
  &_encode_two_bytes,
  &_decode_two_bytes
};

static struct MIDIMessageFormat _channel_pressure = {
  &_test_channel_pressure,
  &_set_channel_pressure,
  &_get_channel_pressure,
  &_encode_two_bytes,
  &_decode_two_bytes
};

static struct MIDIMessageFormat _pitch_wheel_change = {
  &_test_pitch_wheel_change,
  &_set_pitch_wheel_change,
  &_get_pitch_wheel_change,
  &_encode_three_bytes,
  &_decode_three_bytes
};

static struct MIDIMessageFormat _system_exclusive = {
  &_test_system_exclusive,
  &_set_system_exclusive,
  &_get_system_exclusive,
  &_encode_system_exclusive,
  &_decode_system_exclusive
};

static struct MIDIMessageFormat _time_code_quarter_frame = {
  &_test_time_code_quarter_frame,
  NULL,
  NULL,
  &_encode_two_bytes,
  &_decode_two_bytes
};

static struct MIDIMessageFormat _song_position_pointer = {
  &_test_song_position_pointer,
  NULL,
  NULL,
  &_encode_three_bytes,
  &_decode_three_bytes
};

static struct MIDIMessageFormat _song_select = {
  &_test_song_select,
  NULL,
  NULL,
  &_encode_two_bytes,
  &_decode_two_bytes
};

static struct MIDIMessageFormat _tune_request = {
  &_test_tune_request,
  NULL,
  NULL,
  &_encode_one_byte,
  &_decode_one_byte
};

static struct MIDIMessageFormat _real_time = {
  &_test_real_time,
  NULL,
  NULL,
  &_encode_one_byte,
  &_decode_one_byte
};

#define N_ELEM(a) (sizeof(a) / sizeof(a[0]))

struct MIDIMessageFormat * MIDIMessageFormatDetect( void * buffer ) {
  static struct MIDIMessageFormat * formats[] = {
    &_note_off_on,
    &_polyphonic_key_pressure,
    &_control_change,
    &_program_change,
    &_channel_pressure,
    &_pitch_wheel_change,
    &_system_exclusive,
    &_time_code_quarter_frame,
    &_song_position_pointer,
    &_song_select,
    &_tune_request,
    &_real_time
  };
  int i;
  for( i=0; i<N_ELEM(formats); i++ ) {
    if( (formats[i]->test)( buffer ) ) {
      return formats[i];
    }
  }
  return NULL;
}

struct MIDIMessageFormat * MIDIMessageFormatForStatus( MIDIStatus status ) {
  unsigned char byte;
  if( status >= 0x80 ) {
    byte = status;
  } else {
    byte = status << 4;
    if( byte < 0x80 ) return NULL; // no status bit?
  }
  return MIDIMessageFormatDetect( &byte );
}

//@}