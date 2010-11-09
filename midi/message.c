#include <stdlib.h>
#include "message.h"

struct MIDIMessageFormat {
  MIDIKey key;
  char    bits;
};

static struct MIDIMessageFormat _key_velocity[5] = {
  { MIDI_STATUS, 4 },
  { MIDI_CHANNEL, 4 },
  { MIDI_KEY, 8 },
  { MIDI_VELOCITY, 8 },
  { MIDI_NOTHING, 0 }
};

static struct MIDIMessageFormat _control[5] = {
  { MIDI_STATUS, 4 },
  { MIDI_CHANNEL, 4 },
  { MIDI_CONTROL, 8 },
  { MIDI_VALUE, 8 },
  { MIDI_NOTHING, 0 }
};

static struct MIDIMessageFormat _program[4] = {
  { MIDI_STATUS, 4 },
  { MIDI_CHANNEL, 4 },
  { MIDI_PROGRAM, 8 },
  { MIDI_NOTHING, 0 }
};

static struct MIDIMessageFormat _value_1b[4] = {
  { MIDI_STATUS, 4 },
  { MIDI_CHANNEL, 4 },
  { MIDI_VALUE, 8 },
  { MIDI_NOTHING, 0 }
};

static struct MIDIMessageFormat _value_2b[5] = {
  { MIDI_STATUS, 4 },
  { MIDI_CHANNEL, 4 },
  { MIDI_VALUE_MSB, 8 },
  { MIDI_VALUE_LSB, 8 },
  { MIDI_NOTHING, 0 }
};

static struct MIDIMessageFormat _sys_excl[3] = {
  { MIDI_STATUS, 8 },
  { MIDI_MANUFACTURER_ID, 8 },
  { MIDI_NOTHING, 0 }
};

static struct MIDIMessageFormat _sys_flag[2] = {
  { MIDI_STATUS, 8 },
  { MIDI_NOTHING, 0 }
};

static struct MIDIMessageFormat _sys_value_1b[3] = {
  { MIDI_STATUS, 8 },
  { MIDI_VALUE, 8 },
  { MIDI_NOTHING, 0 }
};

static struct MIDIMessageFormat _sys_value_2b[4] = {
  { MIDI_STATUS, 8 },
  { MIDI_VALUE_LSB, 8 },
  { MIDI_VALUE_MSB, 8 },
  { MIDI_NOTHING, 0 }
};

struct MIDIMessage {
  size_t refs;
  MIDIMessageStatus status;
  struct MIDIMessageFormat * format;
  unsigned char byte[8];
};

struct MIDIMessage * MIDIMessageCreate( MIDIMessageStatus status ) {
  struct MIDIMessage * message = malloc( sizeof( struct MIDIMessage ) );
  MIDIMessageSetStatus( message, status );
  message->refs    = 1;
  message->byte[0] = 0;
  message->byte[1] = 0;
  message->byte[2] = 0;
  message->byte[3] = 0;
  message->byte[4] = 0;
  message->byte[5] = 0;
  message->byte[6] = 0;
  message->byte[7] = 0;
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
  if( status == NULL ) {
    return 1;
  }
  *status = message->status;
  return 0;
}

int MIDIMessageSetStatus( struct MIDIMessage * message, MIDIMessageStatus status ) {
  switch( status ) {
    case MIDI_STATUS_NOTE_OFF:
    case MIDI_STATUS_NOTE_ON:
    case MIDI_STATUS_POLYPHONIC_KEY_PRESSURE:
      message->format = &_key_velocity[0];
      break;
    case MIDI_STATUS_CONTROL_CHANGE:
      message->format = &_control[0];
      break;
    case MIDI_STATUS_PROGRAM_CHANGE:
      message->format = &_program[0];
      break;
    case MIDI_STATUS_CHANNEL_PRESSURE:
      message->format = &_value_1b[0];
      break;
    case MIDI_STATUS_PITCH_WHEEL_CHANGE:
      message->format = &_value_2b[0];
      break;
    case MIDI_STATUS_SYSTEM_EXCLUSIVE:
      message->format = &_sys_excl[0];
      break;
    case MIDI_STATUS_TIME_CODE_QUARTER_FRAME:
      message->format = &_sys_value_1b[0];
      break;
    case MIDI_STATUS_SONG_POSITION_POINTER:
      message->format = &_sys_value_2b[0];
      break;
    case MIDI_STATUS_SONG_SELECT:
      message->format = &_sys_value_1b[0];
      break;
    case MIDI_STATUS_TUNE_REQUEST:
    case MIDI_STATUS_TIMING_CLOCK:
    case MIDI_STATUS_START:
    case MIDI_STATUS_CONTINUE:
    case MIDI_STATUS_STOP:
    case MIDI_STATUS_ACTIVE_SENSING:
    case MIDI_STATUS_RESET:
      message->format = &_sys_flag[0];
      break;
    case MIDI_STATUS_UNDEFINED0:
    case MIDI_STATUS_UNDEFINED1:
    case MIDI_STATUS_END_OF_EXCLUSIVE:
    case MIDI_STATUS_UNDEFINED2:
    case MIDI_STATUS_UNDEFINED3:
    default:
      return 1;
      break;
  }
  message->status = status;
  return 0;
}

int MIDIMessageGet( struct MIDIMessage * message, MIDIKey key, MIDIValue * value ) {
  if( value == NULL ) {
    return 1;
  }
  return 0;
}

int MIDIMessageSet( struct MIDIMessage * message, MIDIKey key, MIDIValue value ) {
  return 0;
}