#include <stdlib.h>
#include <stdint.h>
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
  device->refs = 1;
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
  uint8_t b;
  size_t  s;
  void *  vp;
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
      MIDIMessageGet( message, MIDI_CHANNEL, sizeof(MIDIValue), &v[0] );
      MIDIMessageGet( message, MIDI_VALUE,   sizeof(MIDILongValue), &lv );
      return MIDIDeviceReceivePitchWheelChange( device, v[0], lv );
      break;
    case MIDI_STATUS_SYSTEM_EXCLUSIVE:
      MIDIMessageGet( message, MIDI_MANUFACTURER_ID, sizeof(MIDIValue), &v[0] );
      MIDIMessageGet( message, MIDI_SYSEX_SIZE,      sizeof(size_t), &s );
      MIDIMessageGet( message, MIDI_SYSEX_DATA,      sizeof(void*), &vp );
      MIDIMessageGet( message, MIDI_SYSEX_FRAGMENT,  sizeof(uint8_t), &b );
      return MIDIDeviceReceiveSystemExclusive( device, v[0], s, vp, b );
      break;
    case MIDI_STATUS_TIME_CODE_QUARTER_FRAME:
      MIDIMessageGet( message, MIDI_TIME_CODE_TYPE, sizeof(MIDIValue), &v[0] );
      MIDIMessageGet( message, MIDI_VALUE,          sizeof(MIDIValue), &v[1] );
      return MIDIDeviceReceiveTimeCodeQuarterFrame( device, v[0], v[1] );
      break;
    case MIDI_STATUS_SONG_POSITION_POINTER:
      MIDIMessageGet( message, MIDI_VALUE, sizeof(MIDILongValue), &lv );
      return MIDIDeviceReceiveSongPositionPointer( device, lv );
    case MIDI_STATUS_SONG_SELECT:
      MIDIMessageGet( message, MIDI_VALUE, sizeof(MIDIValue), &v[0] );
      return MIDIDeviceReceiveSongSelect( device, v[0] );
      break;
    case MIDI_STATUS_TUNE_REQUEST:
      return MIDIDeviceReceiveTuneRequest( device );
      break;
    case MIDI_STATUS_END_OF_EXCLUSIVE:
      return MIDIDeviceReceiveEndOfExclusive( device );
      break;
    case MIDI_STATUS_TIMING_CLOCK:
    case MIDI_STATUS_START:
    case MIDI_STATUS_CONTINUE:
    case MIDI_STATUS_STOP:
    case MIDI_STATUS_ACTIVE_SENSING:
    case MIDI_STATUS_RESET:
      return MIDIDeviceReceiveRealTime( device, status );
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

/**
 * System exclusive
 */
//@{

int MIDIDeviceReceiveSystemExclusive( struct MIDIDevice * device, MIDIManufacturerId manufacturer_id,
                                      size_t size, void * data, uint8_t fragment ) {
  if( device->delegate == NULL || device->delegate->recv_sx == NULL ) {
    return 0;
  }
  return (*device->delegate->recv_sx)( device, manufacturer_id, size, data, fragment );
}

int MIDIDeviceSendSystemExclusive( struct MIDIDevice * device, MIDIManufacturerId manufacturer_id,
                                   size_t size, void * data, uint8_t fragment ) {
  struct MIDIMessage * message = MIDIMessageCreate( MIDI_STATUS_SYSTEM_EXCLUSIVE );
  MIDIMessageSet( message, MIDI_MANUFACTURER_ID, sizeof(MIDIManufacturerId), &manufacturer_id );
  MIDIMessageSet( message, MIDI_SYSEX_SIZE,      sizeof(size_t), &size );
  MIDIMessageSet( message, MIDI_SYSEX_DATA,      sizeof(void *), &data );
  MIDIMessageSet( message, MIDI_SYSEX_FRAGMENT,  sizeof(uint8_t), &fragment );
  return MIDIDeviceSend( device, message );
}

//@}

/**
 * Time code quarter frame
 */
//@{

int MIDIDeviceReceiveTimeCodeQuarterFrame( struct MIDIDevice * device, MIDIValue time_code_type, MIDIValue value ) {
  if( device->delegate == NULL || device->delegate->recv_tcqf == NULL ) {
    return 0;
  }
  return (*device->delegate->recv_tcqf)( device, time_code_type, value );
}

int MIDIDeviceSendTimeCodeQuarterFrame( struct MIDIDevice * device, MIDIValue time_code_type, MIDIValue value ) {
  struct MIDIMessage * message = MIDIMessageCreate( MIDI_STATUS_TIME_CODE_QUARTER_FRAME );
  MIDIMessageSet( message, MIDI_TIME_CODE_TYPE, sizeof(MIDIValue), &time_code_type );
  MIDIMessageSet( message, MIDI_VALUE,          sizeof(MIDIValue), &value );
  return MIDIDeviceSend( device, message );
}

//@}

/**
 * Song position pointer
 */
//@{

int MIDIDeviceReceiveSongPositionPointer( struct MIDIDevice * device, MIDILongValue value ) {
  if( device->delegate == NULL || device->delegate->recv_spp == NULL ) {
    return 0;
  }
  return (*device->delegate->recv_spp)( device, value );
}

int MIDIDeviceSendSongPositionPointer( struct MIDIDevice * device, MIDILongValue value ) {
  struct MIDIMessage * message = MIDIMessageCreate( MIDI_STATUS_SONG_POSITION_POINTER );
  MIDIMessageSet( message, MIDI_VALUE, sizeof(MIDILongValue), &value );
  return MIDIDeviceSend( device, message );
}

//@}

/**
 * Song select
 */
//@{

int MIDIDeviceReceiveSongSelect( struct MIDIDevice * device, MIDIValue value ) {
  if( device->delegate == NULL || device->delegate->recv_ss == NULL ) {
    return 0;
  }
  return (*device->delegate->recv_ss)( device, value );
}

int MIDIDeviceSendSongSelect( struct MIDIDevice * device, MIDIValue value ) {
  struct MIDIMessage * message = MIDIMessageCreate( MIDI_STATUS_SONG_SELECT );
  MIDIMessageSet( message, MIDI_VALUE, sizeof(MIDIValue), &value );
  return MIDIDeviceSend( device, message );
}

//@}

/**
 * Tune request
 */
//@{

int MIDIDeviceReceiveTuneRequest( struct MIDIDevice * device ) {
  if( device->delegate == NULL || device->delegate->recv_tr == NULL ) {
    return 0;
  }
  return (*device->delegate->recv_tr)( device );
}

int MIDIDeviceSendTuneRequest( struct MIDIDevice * device ) {
  struct MIDIMessage * message = MIDIMessageCreate( MIDI_STATUS_TUNE_REQUEST );
  return MIDIDeviceSend( device, message );
}

//@}

/**
 * End of exclusive
 */
//@{

int MIDIDeviceReceiveEndOfExclusive( struct MIDIDevice * device ) {
  if( device->delegate == NULL || device->delegate->recv_eox == NULL ) {
    return 0;
  }
  return (*device->delegate->recv_eox)( device );
}

int MIDIDeviceSendEndOfExclusive( struct MIDIDevice * device ) {
  struct MIDIMessage * message = MIDIMessageCreate( MIDI_STATUS_END_OF_EXCLUSIVE );
  return MIDIDeviceSend( device, message );
}

//@}

/**
 * Real time
 */
//@{

int MIDIDeviceReceiveRealTime( struct MIDIDevice * device, MIDIStatus status ) {
  if( device->delegate == NULL || device->delegate->recv_rt == NULL ) {
    return 0;
  }
  if( status < MIDI_STATUS_TIMING_CLOCK || status > MIDI_STATUS_RESET ) {
    return 1;
  }
  return (*device->delegate->recv_rt)( device, status );
}

int MIDIDeviceSendRealTime( struct MIDIDevice * device, MIDIStatus status ) {
  if( status < MIDI_STATUS_TIMING_CLOCK || status > MIDI_STATUS_RESET ) {
    return 1;
  }
  struct MIDIMessage * message = MIDIMessageCreate( status );
  return MIDIDeviceSend( device, message );
}

//@}
