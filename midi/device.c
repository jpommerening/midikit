#include <stdlib.h>
#include "device.h"
#include "message.h"
#include "input.h"
#include "output.h"

struct MIDIDevice {
  size_t refs;
  struct MIDIInput  * in;
  struct MIDIOutput * out;
  struct MIDIOutput * thru;
  struct MIDIDeviceContext * context;
};

struct MIDIDevice * MIDIDeviceCreate( struct MIDIDeviceContext * context ) {
  struct MIDIDevice * device = malloc( sizeof( struct MIDIDevice ) );
  device->in   = NULL;
  device->out  = NULL;
  device->thru = NULL;
  device->context = context;
  return device;
}

void MIDIDeviceDestroy( struct MIDIDevice * device ) {
  if( device->in != NULL ) {
    MIDIInputRelease( device->in );
  }
  if( device->out != NULL ) {
    MIDIOutputRelease( device->out );
  }
  if( device->thru != NULL ) {
    MIDIOutputRelease( device->thru );
  }
  free( device );
}

void MIDIDeviceRetain( struct MIDIDevice * device ) {
  device->refs++;
}

void MIDIDeviceRelease( struct MIDIDevice * device ) {
  if( ! --device->refs ) {
    MIDIDeviceDestroy( device );
  }
}

int MIDIDeviceDetachIn( struct MIDIDevice * device ) {
  int result;
  if( device->in == NULL ) {
    return 0;
  }
  result = MIDIInputDisconnect( device->in );
  MIDIInputRelease( device->in );
  return result;
}

int MIDIDeviceAttachIn( struct MIDIDevice * device, struct MIDIInput * in ) {
  MIDIDeviceDetachIn( device );
  device->in = in;
  MIDIInputRetain( in );
  return MIDIInputConnect( device->in );
}

int MIDIDeviceDetachOut( struct MIDIDevice * device ) {
  int result;
  if( device->out == NULL ) {
    return 0;
  }
  result = MIDIOutputDisconnect( device->out );
  MIDIOutputRelease( device->out );
  return result;
}

int MIDIDeviceAttachOut( struct MIDIDevice * device, struct MIDIOutput * out ) {
  MIDIDeviceDetachOut( device );
  device->out = out;
  MIDIOutputRetain( out );
  return MIDIOutputConnect( device->out );
}

int MIDIDeviceDetachThru( struct MIDIDevice * device ) {
  int result;
  if( device->thru == NULL ) {
    return 0;
  }
  result = MIDIOutputDisconnect( device->thru );
  MIDIOutputRelease( device->thru );
  return result;
}

int MIDIDeviceAttachThru( struct MIDIDevice * device, struct MIDIOutput * thru ) {
  MIDIDeviceDetachThru( device );
  device->thru = thru;
  MIDIOutputRetain( thru );
  return MIDIOutputConnect( device->thru );
}

int MIDIDeviceReceive( struct MIDIDevice * device, struct MIDIMessage * message ) {
  MIDIMessageStatus status;
  MIDIValue         v[3];
  MIDILongValue     lv;
  if( device->thru != NULL ) {
    MIDIOutputSend( device->thru, message );
  }
  MIDIMessageGetStatus( message, &status );
  switch( status ) {
    case MIDI_STATUS_NOTE_OFF:
      MIDIMessageGet( message, MIDI_CHANNEL,  sizeof(MIDIValue), &v[0] );
      MIDIMessageGet( message, MIDI_KEY,      sizeof(MIDIValue), &v[1] );
      MIDIMessageGet( message, MIDI_VELOCITY, sizeof(MIDIValue), &v[2] );
      return MIDIDeviceReceiveNoteOff( device, v[0], v[1], v[2] );
      break;
    case MIDI_STATUS_NOTE_ON:
      MIDIMessageGet( message, MIDI_CHANNEL,  sizeof(MIDIValue), &v[0] );
      MIDIMessageGet( message, MIDI_KEY,      sizeof(MIDIValue), &v[1] );
      MIDIMessageGet( message, MIDI_VELOCITY, sizeof(MIDIValue), &v[2] );
      return MIDIDeviceReceiveNoteOn( device, v[0], v[1], v[2] );
      break;
    case MIDI_STATUS_POLYPHONIC_KEY_PRESSURE:
      MIDIMessageGet( message, MIDI_CHANNEL,  sizeof(MIDIValue), &v[0] );
      MIDIMessageGet( message, MIDI_KEY,      sizeof(MIDIValue), &v[1] );
      MIDIMessageGet( message, MIDI_VELOCITY, sizeof(MIDIValue), &v[2] );
      return MIDIDeviceReceivePolyphonicKeyPressure( device, v[0], v[1], v[2] );
      break;
    case MIDI_STATUS_CONTROL_CHANGE:
      MIDIMessageGet( message, MIDI_CHANNEL, sizeof(MIDIValue), &v[0] );
      MIDIMessageGet( message, MIDI_CONTROL, sizeof(MIDIValue), &v[1] );
      MIDIMessageGet( message, MIDI_VALUE,   sizeof(MIDIValue), &v[2] );
      return MIDIDeviceReceiveControlChange( device, v[0], v[1], v[2] );
      break;
    case MIDI_STATUS_PROGRAM_CHANGE:
      MIDIMessageGet( message, MIDI_CHANNEL, sizeof(MIDIValue), &v[0] );
      MIDIMessageGet( message, MIDI_PROGRAM, sizeof(MIDIValue), &v[1] );
      return MIDIDeviceReceiveProgramChange( device, v[0], v[1] );
      break;
    case MIDI_STATUS_CHANNEL_PRESSURE:
      MIDIMessageGet( message, MIDI_CHANNEL, sizeof(MIDIValue), &v[0] );
      MIDIMessageGet( message, MIDI_VALUE,   sizeof(MIDIValue), &v[1] );
      return MIDIDeviceReceiveChannelPressure( device, v[0], v[1] );
      break;
    case MIDI_STATUS_PITCH_WHEEL_CHANGE:
      MIDIMessageGet( message, MIDI_CHANNEL, sizeof(MIDIValue), &v[0] );
      MIDIMessageGet( message, MIDI_VALUE,   sizeof(MIDILongValue), &lv );
      return MIDIDeviceReceivePitchWheelChange( device, v[0], lv );
      break;
    default:
      break;
  }
  return 0;
}

int MIDIDeviceSend( struct MIDIDevice * device, struct MIDIMessage * message ) {
  if( device->out == NULL ) {
    return 0;
  }
  return MIDIOutputSend( device->out, message );
}

/**
 * Note off
 */
//@{

int MIDIDeviceReceiveNoteOff( struct MIDIDevice * device, MIDIValue channel, MIDIValue key, MIDIValue velocity ) {
  if( device->context == NULL || device->context->recv_nof == NULL ) {
    return 0;
  }
  return (*device->context->recv_nof)( device, channel, key, velocity );
}

int MIDIDeviceSendNoteOff( struct MIDIDevice * device, MIDIValue channel, MIDIValue key, MIDIValue velocity ) {
  struct MIDIMessage * message = MIDIMessageCreate( MIDI_STATUS_NOTE_OFF );
  MIDIMessageSet( message, MIDI_CHANNEL,  sizeof(MIDIValue), &channel );
  MIDIMessageSet( message, MIDI_KEY,      sizeof(MIDIValue), &key );
  MIDIMessageSet( message, MIDI_VELOCITY, sizeof(MIDIValue), &velocity );
  return MIDIDeviceSend( device, message );
}

//@}

/**
 * Note on
 */
//@{

int MIDIDeviceReceiveNoteOn( struct MIDIDevice * device, MIDIValue channel, MIDIValue key, MIDIValue velocity ) {
  if( device->context == NULL || device->context->recv_non == NULL ) {
    return 0;
  }
  return (*device->context->recv_non)( device, channel, key, velocity );
}

int MIDIDeviceSendNoteOn( struct MIDIDevice * device, MIDIValue channel, MIDIValue key, MIDIValue velocity ) {
  struct MIDIMessage * message = MIDIMessageCreate( MIDI_STATUS_NOTE_ON );
  MIDIMessageSet( message, MIDI_CHANNEL,  sizeof(MIDIValue), &channel );
  MIDIMessageSet( message, MIDI_KEY,      sizeof(MIDIValue), &key );
  MIDIMessageSet( message, MIDI_VELOCITY, sizeof(MIDIValue), &velocity );
  return MIDIDeviceSend( device, message );
}

//@}

/**
 * Polyphonic key pressure
 */
//@{

int MIDIDeviceReceivePolyphonicKeyPressure( struct MIDIDevice * device, MIDIValue channel, MIDIValue key, MIDIValue velocity ) {
  if( device->context == NULL || device->context->recv_pkp == NULL ) {
    return 0;
  }
  return (*device->context->recv_pkp)( device, channel, key, velocity );
}

int MIDIDeviceSendPolyphonicKeyPressure( struct MIDIDevice * device, MIDIValue channel, MIDIValue key, MIDIValue velocity ) {
  struct MIDIMessage * message = MIDIMessageCreate( MIDI_STATUS_POLYPHONIC_KEY_PRESSURE );
  MIDIMessageSet( message, MIDI_CHANNEL,  sizeof(MIDIValue), &channel );
  MIDIMessageSet( message, MIDI_KEY,      sizeof(MIDIValue), &key );
  MIDIMessageSet( message, MIDI_VELOCITY, sizeof(MIDIValue), &velocity );
  return MIDIDeviceSend( device, message );
}

//@}

/**
 * Control change
 */
//@{

int MIDIDeviceReceiveControlChange( struct MIDIDevice * device, MIDIValue channel, MIDIValue control, MIDIValue value ) {
  if( device->context == NULL || device->context->recv_cc == NULL ) {
    return 0;
  }
  return (*device->context->recv_cc)( device, channel, control, value );
}

int MIDIDeviceSendControlChange( struct MIDIDevice * device, MIDIValue channel, MIDIValue control, MIDIValue value ) {
  struct MIDIMessage * message = MIDIMessageCreate( MIDI_STATUS_CONTROL_CHANGE );
  MIDIMessageSet( message, MIDI_CHANNEL, sizeof(MIDIValue), &channel );
  MIDIMessageSet( message, MIDI_CONTROL, sizeof(MIDIValue), &control );
  MIDIMessageSet( message, MIDI_VALUE,   sizeof(MIDIValue), &value );
  return MIDIDeviceSend( device, message );
}

//@}

/**
 * Program change
 */
//@{

int MIDIDeviceReceiveProgramChange( struct MIDIDevice * device, MIDIValue channel, MIDIValue program ) {
  if( device->context == NULL || device->context->recv_pc == NULL ) {
    return 0;
  }
  return (*device->context->recv_pc)( device, channel, program );
}

int MIDIDeviceSendProgramChange( struct MIDIDevice * device, MIDIValue channel, MIDIValue program ) {
  struct MIDIMessage * message = MIDIMessageCreate( MIDI_STATUS_PROGRAM_CHANGE );
  MIDIMessageSet( message, MIDI_CHANNEL, sizeof(MIDIValue), &channel );
  MIDIMessageSet( message, MIDI_PROGRAM, sizeof(MIDIValue), &program );
  return MIDIDeviceSend( device, message );
}

//@}

/**
 * Channel pressure
 */
//@{

int MIDIDeviceReceiveChannelPressure( struct MIDIDevice * device, MIDIValue channel, MIDIValue value ) {
  if( device->context == NULL || device->context->recv_cp == NULL ) {
    return 0;
  }
  return (*device->context->recv_cp)( device, channel, value );
}

int MIDIDeviceSendChannelPressure( struct MIDIDevice * device, MIDIValue channel, MIDIValue value ) {
  struct MIDIMessage * message = MIDIMessageCreate( MIDI_STATUS_CHANNEL_PRESSURE );
  MIDIMessageSet( message, MIDI_CHANNEL, sizeof(MIDIValue), &channel );
  MIDIMessageSet( message, MIDI_VALUE,   sizeof(MIDIValue), &value );
  return MIDIDeviceSend( device, message );
}

//@}

/**
 * Pitch wheel change
 */
//@{

int MIDIDeviceReceivePitchWheelChange( struct MIDIDevice * device, MIDIValue channel, MIDILongValue value ) {
  if( device->context == NULL || device->context->recv_pwc == NULL ) {
    return 0;
  }
  return (*device->context->recv_pwc)( device, channel, value );
}

int MIDIDeviceSendPitchWheelChange( struct MIDIDevice * device, MIDIValue channel, MIDILongValue value ) {
  struct MIDIMessage * message = MIDIMessageCreate( MIDI_STATUS_PITCH_WHEEL_CHANGE );
  MIDIMessageSet( message, MIDI_CHANNEL, sizeof(MIDIValue),     &channel );
  MIDIMessageSet( message, MIDI_VALUE,   sizeof(MIDILongValue), &value );
  return MIDIDeviceSend( device, message );
}

//@}
