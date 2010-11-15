#include <stdlib.h>
#include "device.h"
#include "message.h"
#include "connector.h"

struct MIDIDevice {
  size_t refs;
  struct MIDIConnector  * in;
  struct MIDIConnector * out;
  struct MIDIConnector * thru;
  struct MIDIDeviceDelegate * delegate;
};

struct MIDIDevice * MIDIDeviceCreate( struct MIDIDeviceDelegate * delegate ) {
  struct MIDIDevice * device = malloc( sizeof( struct MIDIDevice ) );
  device->in   = NULL;
  device->out  = NULL;
  device->thru = NULL;
  device->delegate = delegate;
  return device;
}

void MIDIDeviceDestroy( struct MIDIDevice * device ) {
  if( device->in != NULL ) {
    MIDIConnectorRelease( device->in );
  }
  if( device->out != NULL ) {
    MIDIConnectorRelease( device->out );
  }
  if( device->thru != NULL ) {
    MIDIConnectorRelease( device->thru );
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
  int result = 0;
  if( device->in != NULL ) {
    result = MIDIConnectorDetach( device->in );
    MIDIConnectorRelease( device->in );
    device->in = NULL;
  }
  return result;
}

int MIDIDeviceAttachIn( struct MIDIDevice * device, struct MIDIConnector * in ) {
  MIDIDeviceDetachIn( device );
  device->in = in;
  MIDIConnectorRetain( in );
  return MIDIConnectorAttachDevice( device->in, device );
}

int MIDIDeviceDetachOut( struct MIDIDevice * device ) {
  if( device->out != NULL ) {
    MIDIConnectorRelease( device->out );
    device->out = NULL;
  }
  return 0;
}

int MIDIDeviceAttachOut( struct MIDIDevice * device, struct MIDIConnector * out ) {
  MIDIDeviceDetachOut( device );
  device->out = out;
  MIDIConnectorRetain( out );
  return 0;
}

int MIDIDeviceDetachThru( struct MIDIDevice * device ) {
  if( device->thru != NULL ) {
    MIDIConnectorRelease( device->thru );
    device->thru = NULL;
  }
  return 0;
}

int MIDIDeviceAttachThru( struct MIDIDevice * device, struct MIDIConnector * thru ) {
  MIDIDeviceDetachThru( device );
  device->thru = thru;
  MIDIConnectorRetain( thru );
  return 0;
}

int MIDIDeviceReceive( struct MIDIDevice * device, struct MIDIMessage * message ) {
  MIDIStatus     status;
  MIDIValue      v[3];
  MIDILongValue  lv;
  if( device->thru != NULL ) {
    MIDIConnectorRelay( device->thru, message );
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
      MIDIMessageGet( message, MIDI_CHANNEL,   sizeof(MIDIValue), &v[0] );
      MIDIMessageGet( message, MIDI_VALUE_LSB, sizeof(MIDIValue), &v[1] );
      MIDIMessageGet( message, MIDI_VALUE_MSB, sizeof(MIDIValue), &v[2] );
      lv = MIDI_LONG_VALUE( v[2], v[1] );
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
  return MIDIConnectorRelay( device->out, message );
}

/**
 * Note off
 */
//@{

int MIDIDeviceReceiveNoteOff( struct MIDIDevice * device, MIDIChannel channel, MIDIKey key, MIDIVelocity velocity ) {
  if( device->delegate == NULL || device->delegate->recv_nof == NULL ) {
    return 0;
  }
  return (*device->delegate->recv_nof)( device, channel, key, velocity );
}

int MIDIDeviceSendNoteOff( struct MIDIDevice * device, MIDIChannel channel, MIDIKey key, MIDIVelocity velocity ) {
  struct MIDIMessage * message = MIDIMessageCreate( MIDI_STATUS_NOTE_OFF );
  MIDIMessageSet( message, MIDI_CHANNEL,  sizeof(MIDIChannel),  &channel );
  MIDIMessageSet( message, MIDI_KEY,      sizeof(MIDIKey),      &key );
  MIDIMessageSet( message, MIDI_VELOCITY, sizeof(MIDIVelocity), &velocity );
  return MIDIDeviceSend( device, message );
}

//@}

/**
 * Note on
 */
//@{

int MIDIDeviceReceiveNoteOn( struct MIDIDevice * device, MIDIChannel channel, MIDIKey key, MIDIVelocity velocity ) {
  if( device->delegate == NULL || device->delegate->recv_non == NULL ) {
    return 0;
  }
  return (*device->delegate->recv_non)( device, channel, key, velocity );
}

int MIDIDeviceSendNoteOn( struct MIDIDevice * device, MIDIChannel channel, MIDIKey key, MIDIVelocity velocity ) {
  struct MIDIMessage * message = MIDIMessageCreate( MIDI_STATUS_NOTE_ON );
  MIDIMessageSet( message, MIDI_CHANNEL,  sizeof(MIDIChannel),  &channel );
  MIDIMessageSet( message, MIDI_KEY,      sizeof(MIDIKey),      &key );
  MIDIMessageSet( message, MIDI_VELOCITY, sizeof(MIDIVelocity), &velocity );
  return MIDIDeviceSend( device, message );
}

//@}

/**
 * Polyphonic key pressure
 */
//@{

int MIDIDeviceReceivePolyphonicKeyPressure( struct MIDIDevice * device, MIDIChannel channel, MIDIKey key, MIDIPressure pressure ) {
  if( device->delegate == NULL || device->delegate->recv_pkp == NULL ) {
    return 0;
  }
  return (*device->delegate->recv_pkp)( device, channel, key, pressure );
}

int MIDIDeviceSendPolyphonicKeyPressure( struct MIDIDevice * device, MIDIChannel channel, MIDIKey key, MIDIPressure pressure ) {
  struct MIDIMessage * message = MIDIMessageCreate( MIDI_STATUS_POLYPHONIC_KEY_PRESSURE );
  MIDIMessageSet( message, MIDI_CHANNEL,  sizeof(MIDIChannel),  &channel );
  MIDIMessageSet( message, MIDI_KEY,      sizeof(MIDIKey),      &key );
  MIDIMessageSet( message, MIDI_PRESSURE, sizeof(MIDIPressure), &pressure );
  return MIDIDeviceSend( device, message );
}

//@}

/**
 * Control change
 */
//@{

int MIDIDeviceReceiveControlChange( struct MIDIDevice * device, MIDIChannel channel, MIDIControl control, MIDIValue value ) {
  if( device->delegate == NULL || device->delegate->recv_cc == NULL ) {
    return 0;
  }
  return (*device->delegate->recv_cc)( device, channel, control, value );
}

int MIDIDeviceSendControlChange( struct MIDIDevice * device, MIDIChannel channel, MIDIControl control, MIDIValue value ) {
  struct MIDIMessage * message = MIDIMessageCreate( MIDI_STATUS_CONTROL_CHANGE );
  MIDIMessageSet( message, MIDI_CHANNEL, sizeof(MIDIChannel), &channel );
  MIDIMessageSet( message, MIDI_CONTROL, sizeof(MIDIControl), &control );
  MIDIMessageSet( message, MIDI_VALUE,   sizeof(MIDIValue),   &value );
  return MIDIDeviceSend( device, message );
}

//@}

/**
 * Program change
 */
//@{

int MIDIDeviceReceiveProgramChange( struct MIDIDevice * device, MIDIChannel channel, MIDIProgram program ) {
  if( device->delegate == NULL || device->delegate->recv_pc == NULL ) {
    return 0;
  }
  return (*device->delegate->recv_pc)( device, channel, program );
}

int MIDIDeviceSendProgramChange( struct MIDIDevice * device, MIDIChannel channel, MIDIProgram program ) {
  struct MIDIMessage * message = MIDIMessageCreate( MIDI_STATUS_PROGRAM_CHANGE );
  MIDIMessageSet( message, MIDI_CHANNEL, sizeof(MIDIChannel), &channel );
  MIDIMessageSet( message, MIDI_PROGRAM, sizeof(MIDIProgram), &program );
  return MIDIDeviceSend( device, message );
}

//@}

/**
 * Channel pressure
 */
//@{

int MIDIDeviceReceiveChannelPressure( struct MIDIDevice * device, MIDIChannel channel, MIDIPressure pressure ) {
  if( device->delegate == NULL || device->delegate->recv_cp == NULL ) {
    return 0;
  }
  return (*device->delegate->recv_cp)( device, channel, pressure );
}

int MIDIDeviceSendChannelPressure( struct MIDIDevice * device, MIDIChannel channel, MIDIPressure pressure ) {
  struct MIDIMessage * message = MIDIMessageCreate( MIDI_STATUS_CHANNEL_PRESSURE );
  MIDIMessageSet( message, MIDI_CHANNEL,  sizeof(MIDIChannel),  &channel );
  MIDIMessageSet( message, MIDI_PRESSURE, sizeof(MIDIPressure), &pressure );
  return MIDIDeviceSend( device, message );
}

//@}

/**
 * Pitch wheel change
 */
//@{

int MIDIDeviceReceivePitchWheelChange( struct MIDIDevice * device, MIDIChannel channel, MIDILongValue value ) {
  if( device->delegate == NULL || device->delegate->recv_pwc == NULL ) {
    return 0;
  }
  return (*device->delegate->recv_pwc)( device, channel, value );
}

int MIDIDeviceSendPitchWheelChange( struct MIDIDevice * device, MIDIChannel channel, MIDILongValue value ) {
  struct MIDIMessage * message = MIDIMessageCreate( MIDI_STATUS_PITCH_WHEEL_CHANGE );
  MIDIMessageSet( message, MIDI_CHANNEL, sizeof(MIDIChannel),   &channel );
  MIDIMessageSet( message, MIDI_VALUE,   sizeof(MIDILongValue), &value );
  return MIDIDeviceSend( device, message );
}

//@}
