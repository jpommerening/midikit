#include <stdlib.h>
#include "message.h"

#define MIDI_MESSAGE_MAX_BYTES 8

struct MIDIMessageFormat {
  MIDIProperty property;
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

static struct MIDIMessageFormat _key_pressure[5] = {
  { MIDI_STATUS,   0, 0xf0 },
  { MIDI_CHANNEL,  0, 0x0f },
  { MIDI_KEY,      1, 0x7f },
  { MIDI_PRESSURE, 2, 0x7f },
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

static struct MIDIMessageFormat _channel_pressure[4] = {
  { MIDI_STATUS,   0, 0xf0 },
  { MIDI_CHANNEL,  0, 0x0f },
  { MIDI_PRESSURE, 1, 0x7f },
  { MIDI_NOTHING,  0, 0x00 }
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

static struct MIDIMessageFormat _sys_excl[4] = {
  { MIDI_STATUS,          0, 0xff },
  { MIDI_MANUFACTURER_ID, 1, 0x7f },
  { MIDI_SYSEX_DATA,      2, 0x7f },
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

static struct MIDIMessageFormat * _format_for_status( MIDIStatus status ) {
  struct MIDIMessageFormat * format;
  switch( status ) {
    case MIDI_STATUS_NOTE_OFF:
    case MIDI_STATUS_NOTE_ON:
      format = &_key_velocity[0];
      break;
    case MIDI_STATUS_POLYPHONIC_KEY_PRESSURE:
      format = &_key_pressure[0];
      break;
    case MIDI_STATUS_CONTROL_CHANGE:
      format = &_control[0];
      break;
    case MIDI_STATUS_PROGRAM_CHANGE:
      format = &_program[0];
      break;
    case MIDI_STATUS_CHANNEL_PRESSURE:
      format = &_channel_pressure[0];
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
  size_t sysex_size;
  unsigned char * sysex_data;
};

struct MIDIMessage * MIDIMessageCreate( MIDIStatus status ) {
  struct MIDIMessage * message = malloc( sizeof( struct MIDIMessage ) );
  int i;
  if( message == NULL ) {
    return NULL;
  }
  message->refs    = 1;
  message->byte[0] = status;
  message->format  = _format_for_status( status );
  message->sysex_size = 0;
  message->sysex_data = NULL;
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

int MIDIMessageGetStatus( struct MIDIMessage * message, MIDIStatus * status ) {
  return MIDIMessageGet( message, MIDI_STATUS, sizeof( MIDIStatus ), status );
}

int MIDIMessageSetStatus( struct MIDIMessage * message, MIDIStatus status ) {
  return MIDIMessageSet( message, MIDI_STATUS, sizeof( MIDIStatus ), &status );
}

static int _get_message_byte( struct MIDIMessage * message, struct MIDIMessageFormat * format, MIDIValue * value ) {
  *value = message->byte[ format->byte ] & format->mask;
  return 0;
}

static int _get_message_sysex_data( struct MIDIMessage * message, size_t size, unsigned char * data ) {
  size_t i;
  if( message->sysex_data == NULL ) {
    return 1;
  }
  for( i=0; i<size && i<message->sysex_size; i++ ) {
    data[i] = message->sysex_data[i];
  }
  return 0;
}

int MIDIMessageGet( struct MIDIMessage * message, MIDIProperty property, size_t size, void * value ) {
  struct MIDIMessageFormat * format;
  if( value == NULL ) {
    return 1;
  }
  format = message->format;
  while( format->mask != 0 ) {
    if( format->property == property ) {
      if( property == MIDI_SYSEX_DATA ) {
        return _get_message_sysex_data( message, size, value );
      } else {
        return _get_message_byte( message, format, (MIDIValue *) value );
      }
    }
    format++;
  }
  return 1;
}

static int _set_message_byte( struct MIDIMessage * message, struct MIDIMessageFormat * format, MIDIValue value ) {
  if( (value & format->mask) != value ) {
    // value is too big!
    return 1;
  }
  message->byte[ format->byte ] &= ~format->mask;
  message->byte[ format->byte ] |= value;
  return 0;
}

static int _set_message_sysex_data( struct MIDIMessage * message, size_t size, unsigned char * data ) {
  size_t i;
  if( message->sysex_data == NULL ) {
    message->sysex_data = malloc( size );
  } else if( message->sysex_size < size ) {
    message->sysex_data = realloc( message->sysex_data, size );
  }
  if( message->sysex_data == NULL ) {
    return 1;
  }
  for( i=0; i<size; i++ ) {
    if( data[i] & 0x80 ) {
      return 1;
    }
    message->sysex_data[i] = data[i] & 0x7f;
  }
  return 0;
}

int MIDIMessageSet( struct MIDIMessage * message, MIDIProperty property, size_t size, void * value ) {
  struct MIDIMessageFormat * format;
  if( property == MIDI_STATUS ) {
    if( size != sizeof( MIDIStatus ) ) {
      return 1;
    }
    message->format = _format_for_status( *((MIDIStatus*) value) );
  }
  format = message->format;
  while( format->mask != 0 ) {
    if( format->property == property ) {
      if( property == MIDI_SYSEX_DATA ) {
        return _set_message_sysex_data( message, size, value );
      } else {
        return _set_message_byte( message, format, *((MIDIValue *) value) );
      }
    }
    format++;
  }
  return 1;
}

int MIDIMessageRead( struct MIDIMessage * message, size_t size, unsigned char * buffer ) {
  size_t i;
  if( message->byte[0] != MIDI_STATUS_SYSTEM_EXCLUSIVE ) {
    for( i=0; i<size && i<MIDI_MESSAGE_MAX_BYTES; i++ ) {
      buffer[i] = message->byte[i];
    }
  } else {
    if( size > 0 ) {
      buffer[0] = message->byte[0];
      if( size > 1 ) {
        buffer[1] = message->byte[1];
      }
    }
  }
  return 0;
}


int MIDIMessageWrite( struct MIDIMessage * message, size_t size, unsigned char * buffer ) {
  size_t i;
  if( message->byte[0] != MIDI_STATUS_SYSTEM_EXCLUSIVE ) {
    for( i=0; i<size && i<MIDI_MESSAGE_MAX_BYTES; i++ ) {
      message->byte[i] = buffer[i];
    }
  } else {
    for( i=0; i<size && i<MIDI_MESSAGE_MAX_BYTES; i++ ) {
      message->byte[i] = buffer[i];
    }
  }
  return 0;
}
