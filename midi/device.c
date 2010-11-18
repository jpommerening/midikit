#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include "device.h"
#include "message.h"
#include "connector.h"

struct MIDIDevice {
  size_t refs;
  struct MIDIConnector * in;
  struct MIDIConnector * out;
  struct MIDIConnector * thru;
  struct MIDIDeviceDelegate * delegate;
};

static int _connect( struct MIDIConnector ** device_connector, struct MIDIConnector * connector ) {
  MIDIConnectorRetain( connector );
  *device_connector = connector;
  return 0;
}

static int _disconnect( struct MIDIConnector ** device_connector ) {
  struct MIDIConnector * connector = *device_connector;
  *device_connector = NULL;
  MIDIConnectorRelease( connector );
  return 0;
}

static int _in_relay( void * devicep, struct MIDIMessage * message ) {
  return MIDIDeviceReceive( devicep, message );
}

static int _in_connect( void * devicep, struct MIDIConnector * in ) {
  struct MIDIDevice * device = devicep;
  if( device->in == in ) return 0;
  if( device->in != NULL ) MIDIConnectorDetachTarget( device->in );
  return _connect( &(device->in), in );
}

static int _in_disconnect( void * devicep, struct MIDIConnector * in ) {
  struct MIDIDevice * device = devicep;
  if( device->in != in ) return 1;
  return _disconnect( &(device->in) );
}

static int _out_connect( void * devicep, struct MIDIConnector * out ) {
  struct MIDIDevice * device = devicep;
  if( device->out == out ) return 0;
  if( device->out != NULL ) MIDIConnectorDetachSource( device->out );
  return _connect( &(device->out), out );
}

static int _out_disconnect( void * devicep, struct MIDIConnector * out ) {
  struct MIDIDevice * device = devicep;
  if( device->out != out ) return 1;
  return _disconnect( &(device->out) );
}

static int _thru_connect( void * devicep, struct MIDIConnector * thru ) {
  struct MIDIDevice * device = devicep;
  if( device->thru == thru ) return 0;
  if( device->thru != NULL ) MIDIConnectorDetachSource( device->thru );
  return _connect( &(device->thru), thru );
}

static int _thru_disconnect( void * devicep, struct MIDIConnector * thru ) {
  struct MIDIDevice * device = devicep;
  if( device->thru != thru ) return 1;
  return _disconnect( &(device->thru) );
}

struct MIDIConnectorTargetDelegate MIDIDeviceInConnectorDelegate = {
  &_in_relay,
  &_in_connect,
  &_in_disconnect
};

struct MIDIConnectorSourceDelegate MIDIDeviceOutConnectorDelegate = {
  &_out_connect,
  &_out_disconnect
};

struct MIDIConnectorSourceDelegate MIDIDeviceThruConnectorDelegate = {
  &_thru_connect,
  &_thru_disconnect
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
  MIDIDeviceDetachIn( device );
  MIDIDeviceDetachOut( device );
  MIDIDeviceDetachThru( device );
  free( device );
}

void MIDIDeviceRetain( struct MIDIDevice * device ) {
  device->refs++;
}

void MIDIDeviceRelease( struct MIDIDevice * device ) {
  struct MIDIConnector * in = device->in;
  if( ! --device->refs ) {
    MIDIDeviceDestroy( device );
  } else {
    // device and connector have circular dependencies
    // we try to break them like this ..
    if( device->refs == 1 && in != NULL ) {
      device->in = NULL;
      MIDIConnectorRelease( in );
    }
  }
}

int MIDIDeviceDetachIn( struct MIDIDevice * device ) {
  if( device->in == NULL ) return 0;
  return MIDIConnectorDetachTarget( device->in );
}

int MIDIDeviceAttachIn( struct MIDIDevice * device, struct MIDIConnector * in ) {
  if( device->in != NULL ) MIDIDeviceDetachIn( device );
  return MIDIConnectorAttachToDeviceIn( in, device );
}

int MIDIDeviceDetachOut( struct MIDIDevice * device ) {
  if( device->out == NULL ) return 0;
  return MIDIConnectorDetachSource( device->out );
}

int MIDIDeviceAttachOut( struct MIDIDevice * device, struct MIDIConnector * out ) {
  if( device->out != NULL ) MIDIDeviceDetachOut( device );
  return MIDIConnectorAttachFromDeviceOut( out, device );
}

int MIDIDeviceDetachThru( struct MIDIDevice * device ) {
  if( device->thru == NULL ) return 0;
  return MIDIConnectorDetachSource( device->thru );
}

int MIDIDeviceAttachThru( struct MIDIDevice * device, struct MIDIConnector * thru ) {
  if( device->thru != NULL ) MIDIDeviceDetachThru( device );
  return MIDIConnectorAttachFromDeviceThru( thru, device );
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
      MIDIMessageGet( message, MIDI_CHANNEL,  sizeof(MIDIChannel),  &v[0] );
      MIDIMessageGet( message, MIDI_KEY,      sizeof(MIDIKey),      &v[1] );
      MIDIMessageGet( message, MIDI_VELOCITY, sizeof(MIDIVelocity), &v[2] );
      return MIDIDeviceReceiveNoteOff( device, v[0], v[1], v[2] );
      break;
    case MIDI_STATUS_NOTE_ON:
      MIDIMessageGet( message, MIDI_CHANNEL,  sizeof(MIDIChannel),  &v[0] );
      MIDIMessageGet( message, MIDI_KEY,      sizeof(MIDIKey),      &v[1] );
      MIDIMessageGet( message, MIDI_VELOCITY, sizeof(MIDIVelocity), &v[2] );
      return MIDIDeviceReceiveNoteOn( device, v[0], v[1], v[2] );
      break;
    case MIDI_STATUS_POLYPHONIC_KEY_PRESSURE:
      MIDIMessageGet( message, MIDI_CHANNEL,  sizeof(MIDIChannel),  &v[0] );
      MIDIMessageGet( message, MIDI_KEY,      sizeof(MIDIKey),      &v[1] );
      MIDIMessageGet( message, MIDI_PRESSURE, sizeof(MIDIPressure), &v[2] );
      return MIDIDeviceReceivePolyphonicKeyPressure( device, v[0], v[1], v[2] );
      break;
    case MIDI_STATUS_CONTROL_CHANGE:
      MIDIMessageGet( message, MIDI_CHANNEL, sizeof(MIDIChannel), &v[0] );
      MIDIMessageGet( message, MIDI_CONTROL, sizeof(MIDIControl), &v[1] );
      MIDIMessageGet( message, MIDI_VALUE,   sizeof(MIDIValue),   &v[2] );
      return MIDIDeviceReceiveControlChange( device, v[0], v[1], v[2] );
      break;
    case MIDI_STATUS_PROGRAM_CHANGE:
      MIDIMessageGet( message, MIDI_CHANNEL, sizeof(MIDIChannel), &v[0] );
      MIDIMessageGet( message, MIDI_PROGRAM, sizeof(MIDIProgram), &v[1] );
      return MIDIDeviceReceiveProgramChange( device, v[0], v[1] );
      break;
    case MIDI_STATUS_CHANNEL_PRESSURE:
      MIDIMessageGet( message, MIDI_CHANNEL,  sizeof(MIDIChannel),  &v[0] );
      MIDIMessageGet( message, MIDI_PRESSURE, sizeof(MIDIPressure), &v[1] );
      return MIDIDeviceReceiveChannelPressure( device, v[0], v[1] );
      break;
    case MIDI_STATUS_PITCH_WHEEL_CHANGE:
      MIDIMessageGet( message, MIDI_CHANNEL, sizeof(MIDIChannel),   &v[0] );
      MIDIMessageGet( message, MIDI_VALUE,   sizeof(MIDILongValue), &lv );
      return MIDIDeviceReceivePitchWheelChange( device, v[0], lv );
      break;
    case MIDI_STATUS_SYSTEM_EXCLUSIVE:
      MIDIMessageGet( message, MIDI_MANUFACTURER_ID, sizeof(MIDIManufacturerId), &v[0] );
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
  int result;
  result  = MIDIMessageSet( message, MIDI_CHANNEL,  sizeof(MIDIChannel),  &channel );
  result += MIDIMessageSet( message, MIDI_KEY,      sizeof(MIDIKey),      &key );
  result += MIDIMessageSet( message, MIDI_VELOCITY, sizeof(MIDIVelocity), &velocity );
  if( result != 0 ) return result;
  result  = MIDIDeviceSend( device, message );
  MIDIMessageRelease( message );
  return result;
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
  int result;
  result  = MIDIMessageSet( message, MIDI_CHANNEL,  sizeof(MIDIChannel),  &channel );
  result += MIDIMessageSet( message, MIDI_KEY,      sizeof(MIDIKey),      &key );
  result += MIDIMessageSet( message, MIDI_VELOCITY, sizeof(MIDIVelocity), &velocity );
  if( result != 0 ) return result;
  result  = MIDIDeviceSend( device, message );
  MIDIMessageRelease( message );
  return result;
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
  int result;
  result  = MIDIMessageSet( message, MIDI_CHANNEL,  sizeof(MIDIChannel),  &channel );
  result += MIDIMessageSet( message, MIDI_KEY,      sizeof(MIDIKey),      &key );
  result += MIDIMessageSet( message, MIDI_PRESSURE, sizeof(MIDIPressure), &pressure );
  if( result != 0 ) return result;
  result  = MIDIDeviceSend( device, message );
  MIDIMessageRelease( message );
  return result;
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
  int result;
  result  = MIDIMessageSet( message, MIDI_CHANNEL, sizeof(MIDIChannel), &channel );
  result += MIDIMessageSet( message, MIDI_CONTROL, sizeof(MIDIControl), &control );
  result += MIDIMessageSet( message, MIDI_VALUE,   sizeof(MIDIValue),   &value );
  if( result != 0 ) return result;
  result  = MIDIDeviceSend( device, message );
  MIDIMessageRelease( message );
  return result;
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
  int result;
  result  = MIDIMessageSet( message, MIDI_CHANNEL, sizeof(MIDIChannel), &channel );
  result += MIDIMessageSet( message, MIDI_PROGRAM, sizeof(MIDIProgram), &program );
  if( result != 0 ) return result;
  result  = MIDIDeviceSend( device, message );
  MIDIMessageRelease( message );
  return result;
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
  int result;
  result  = MIDIMessageSet( message, MIDI_CHANNEL,  sizeof(MIDIChannel),  &channel );
  result += MIDIMessageSet( message, MIDI_PRESSURE, sizeof(MIDIPressure), &pressure );
  if( result != 0 ) return result;
  result  = MIDIDeviceSend( device, message );
  MIDIMessageRelease( message );
  return result;
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
  int result;
  result  = MIDIMessageSet( message, MIDI_CHANNEL, sizeof(MIDIChannel),   &channel );
  result += MIDIMessageSet( message, MIDI_VALUE,   sizeof(MIDILongValue), &value );
  if( result != 0 ) return result;
  result  = MIDIDeviceSend( device, message );
  MIDIMessageRelease( message );
  return result;
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
  int result;
  result  = MIDIMessageSet( message, MIDI_MANUFACTURER_ID, sizeof(MIDIManufacturerId), &manufacturer_id );
  result += MIDIMessageSet( message, MIDI_SYSEX_SIZE,      sizeof(size_t), &size );
  result += MIDIMessageSet( message, MIDI_SYSEX_DATA,      sizeof(void *), &data );
  result += MIDIMessageSet( message, MIDI_SYSEX_FRAGMENT,  sizeof(uint8_t), &fragment );
  if( result != 0 ) return result;
  result  = MIDIDeviceSend( device, message );
  MIDIMessageRelease( message );
  return result;
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
  int result;
  result  = MIDIMessageSet( message, MIDI_TIME_CODE_TYPE, sizeof(MIDIValue), &time_code_type );
  result += MIDIMessageSet( message, MIDI_VALUE,          sizeof(MIDIValue), &value );
  if( result != 0 ) return result;
  result  = MIDIDeviceSend( device, message );
  MIDIMessageRelease( message );
  return result;
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
  int result;
  result  = MIDIMessageSet( message, MIDI_VALUE, sizeof(MIDILongValue), &value );
  if( result != 0 ) return result;
  result  = MIDIDeviceSend( device, message );
  MIDIMessageRelease( message );
  return result;
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
  int result;
  result  = MIDIMessageSet( message, MIDI_VALUE, sizeof(MIDIValue), &value );
  if( result != 0 ) return result;
  result  = MIDIDeviceSend( device, message );
  MIDIMessageRelease( message );
  return result;
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
  int result = MIDIDeviceSend( device, message );
  MIDIMessageRelease( message );
  return result;
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
  int result = MIDIDeviceSend( device, message );
  MIDIMessageRelease( message );
  return result;
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
  struct MIDIMessage * message;
  int result;
  if( status < MIDI_STATUS_TIMING_CLOCK || status > MIDI_STATUS_RESET ) {
    return 1;
  }
  message = MIDIMessageCreate( status );
  result  = MIDIDeviceSend( device, message );
  MIDIMessageRelease( message );
  return result;
}

//@}
