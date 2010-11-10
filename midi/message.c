#include <stdlib.h>
#include "message.h"

#define MIDI_MESSAGE_MAX_BYTES 8

struct MIDIMessageFormat {
  MIDIKey key;
  unsigned char byte;
  unsigned char mask;
};

static struct MIDIMessageFormat _key_velocity[5] = {
  { MIDI_STATUS,   0, 0xf0 },
  { MIDI_CHANNEL,  0, 0x0f },
  { MIDI_KEY,      1, 0x7f },
  { MIDI_VELOCITY, 2, 0x7f },
  { MIDI_NOTHING,  0, 0x00 }
};

static struct MIDIMessageFormat _control[5] = {
  { MIDI_STATUS,  0, 0xf0 },
  { MIDI_CHANNEL, 0, 0x0f },
  { MIDI_CONTROL, 1, 0x7f },
  { MIDI_VALUE,   2, 0x7f },
  { MIDI_NOTHING, 0, 0x00 }
};

static struct MIDIMessageFormat _program[4] = {
  { MIDI_STATUS,  0, 0xf0 },
  { MIDI_CHANNEL, 0, 0x0f },
  { MIDI_PROGRAM, 1, 0x7f },
  { MIDI_NOTHING, 0, 0x00 }
};

static struct MIDIMessageFormat _value_1b[4] = {
  { MIDI_STATUS,  0, 0xf0 },
  { MIDI_CHANNEL, 0, 0x0f },
  { MIDI_VALUE,   1, 0x7f },
  { MIDI_NOTHING, 0, 0x00 }
};

static struct MIDIMessageFormat _value_2b[5] = {
  { MIDI_STATUS,    0, 0xf0 },
  { MIDI_CHANNEL,   0, 0x0f },
  { MIDI_VALUE_MSB, 1, 0x7f },
  { MIDI_VALUE_LSB, 2, 0x7f },
  { MIDI_NOTHING,   0, 0x00 }
};

static struct MIDIMessageFormat _sys_excl[3] = {
  { MIDI_STATUS,          0, 0xff },
  { MIDI_MANUFACTURER_ID, 0, 0x7f },
  { MIDI_NOTHING,         0, 0x00 }
};

static struct MIDIMessageFormat _sys_flag[2] = {
  { MIDI_STATUS,  0, 0xff },
  { MIDI_NOTHING, 0, 0x00 }
};

static struct MIDIMessageFormat _sys_value_1b[3] = {
  { MIDI_STATUS,  0, 0xff },
  { MIDI_VALUE,   1, 0x7f },
  { MIDI_NOTHING, 0, 0x00 }
};

static struct MIDIMessageFormat _sys_value_2b[4] = {
  { MIDI_STATUS,    0, 0xff },
  { MIDI_VALUE_LSB, 1, 0x7f },
  { MIDI_VALUE_MSB, 2, 0x7f },
  { MIDI_NOTHING,   0, 0x00 }
};

static struct MIDIMessageFormat * _format_for_status( MIDIMessageStatus status ) {
  struct MIDIMessageFormat * format;
  switch( status ) {
    case MIDI_STATUS_NOTE_OFF:
    case MIDI_STATUS_NOTE_ON:
    case MIDI_STATUS_POLYPHONIC_KEY_PRESSURE:
      format = &_key_velocity[0];
      break;
    case MIDI_STATUS_CONTROL_CHANGE:
      format = &_control[0];
      break;
    case MIDI_STATUS_PROGRAM_CHANGE:
      format = &_program[0];
      break;
    case MIDI_STATUS_CHANNEL_PRESSURE:
      format = &_value_1b[0];
      break;
    case MIDI_STATUS_PITCH_WHEEL_CHANGE:
      format = &_value_2b[0];
      break;
    case MIDI_STATUS_SYSTEM_EXCLUSIVE:
      format = &_sys_excl[0];
      break;
    case MIDI_STATUS_TIME_CODE_QUARTER_FRAME:
      format = &_sys_value_1b[0];
      break;
    case MIDI_STATUS_SONG_POSITION_POINTER:
      format = &_sys_value_2b[0];
      break;
    case MIDI_STATUS_SONG_SELECT:
      format = &_sys_value_1b[0];
      break;
    case MIDI_STATUS_TUNE_REQUEST:
    case MIDI_STATUS_TIMING_CLOCK:
    case MIDI_STATUS_START:
    case MIDI_STATUS_CONTINUE:
    case MIDI_STATUS_STOP:
    case MIDI_STATUS_ACTIVE_SENSING:
    case MIDI_STATUS_RESET:
      format = &_sys_flag[0];
      break;
    case MIDI_STATUS_UNDEFINED0:
    case MIDI_STATUS_UNDEFINED1:
    case MIDI_STATUS_END_OF_EXCLUSIVE:
    case MIDI_STATUS_UNDEFINED2:
    case MIDI_STATUS_UNDEFINED3:
    default:
      return NULL;
      break;
  }
  return format;
}

struct MIDIMessage {
  size_t refs;
  struct MIDIMessageFormat * format;
  unsigned char byte[MIDI_MESSAGE_MAX_BYTES];
  unsigned char * sysex_data;
};

struct MIDIMessage * MIDIMessageCreate( MIDIMessageStatus status ) {
  struct MIDIMessage * message = malloc( sizeof( struct MIDIMessage ) );
  int i;
  if( message == NULL ) {
    return NULL;
  }
  message->refs    = 1;
  message->byte[0] = status;
  message->format  = _format_for_status( status );
  if( message->format == NULL ) {
    free( message );
    return NULL;
  }
  for( i=1; i<MIDI_MESSAGE_MAX_BYTES; i++ ) {
    message->byte[i] = 0;
  }
  return message;
}

void MIDIMessageDestroy( struct MIDIMessage * message ) {
  free( message );
}

void MIDIMessageRetain( struct MIDIMessage * message ) {
  message->refs++;
}

void MIDIMessageRelease( struct MIDIMessage * message ) {
  if( ! --message->refs ) {
    MIDIMessageDestroy( message );
  }
}

int MIDIMessageGetStatus( struct MIDIMessage * message, MIDIMessageStatus * status ) {
  return MIDIMessageGet( message, MIDI_STATUS, status );
}

int MIDIMessageSetStatus( struct MIDIMessage * message, MIDIMessageStatus status ) {
  return MIDIMessageSet( message, MIDI_STATUS, status );
}

int MIDIMessageGet( struct MIDIMessage * message, MIDIKey key, MIDIValue * value ) {
  struct MIDIMessageFormat * format;
  int byte, mask;
  if( value == NULL ) {
    return 1;
  }
  format = message->format;
  while( format->mask != 0 ) {
    if( format->key == key ) {
      byte = format->byte;
      mask = format->mask;
      *value = message->byte[ byte ] & mask;
      return 0;
    }
    format++;
  }
  return 1;
}

int MIDIMessageSet( struct MIDIMessage * message, MIDIKey key, MIDIValue value ) {
  struct MIDIMessageFormat * format;
  int byte, mask;
  if( key == MIDI_STATUS ) {
    message->format = _format_for_status( value );
  }
  format = message->format;
  while( format->mask != 0 ) {
    if( format->key == key ) {
      if( (value & format->mask) != value ) {
        return 1;
      } else {
        byte = format->byte;
        mask = format->mask;
        message->byte[ byte ] &= ( 0xff ^ mask );
        message->byte[ byte ] |= value;
        return 0;
      }
    }
    format++;
  }
  return 1;
}

int MIDIMessageRead( struct MIDIMessage * message, size_t bytes, unsigned char * buffer ) {
  size_t i;
  for( i=0; i<bytes && i<MIDI_MESSAGE_MAX_BYTES; i++ ) {
    buffer[i] = message->byte[i];
  }
  return 0;
}


int MIDIMessageWrite( struct MIDIMessage * message, size_t bytes, unsigned char * buffer ) {
  size_t i;
  for( i=0; i<bytes && i<MIDI_MESSAGE_MAX_BYTES; i++ ) {
    message->byte[i] = buffer[i];
  }
  return 0;
}
