#include <stdint.h>
#include "test.h"
#include "midi/midi.h"
#include "midi/connector.h"
#include "midi/message.h"
#include "midi/device.h"
#include "midi/driver.h"

uint8_t _test_values[16] = { 0 };

static int _receive_nof( struct MIDIDevice * device, MIDIChannel channel, MIDIKey key, MIDIVelocity velocity ) {
  printf( "Note Off ( channel=%i, key=%i, velocity=%i )\n", (int) channel, (int) key, (int) velocity );
  ASSERT_EQUAL( channel,  _test_values[0], "Received unexpected channel in note off message." );
  ASSERT_EQUAL( key,      _test_values[1], "Received unexpected key in note off message." );
  ASSERT_EQUAL( velocity, _test_values[2], "Received unexpected velocity in note off message." );
  return 0;
}

static int _receive_non( struct MIDIDevice * device, MIDIChannel channel, MIDIKey key, MIDIVelocity velocity ) {
  printf( "Note On ( channel=%i, key=%i, velocity=%i )\n", (int) channel, (int) key, (int) velocity );
  ASSERT_EQUAL( channel,  _test_values[0], "Received unexpected channel in note on message." );
  ASSERT_EQUAL( key,      _test_values[1], "Received unexpected key in note on message." );
  ASSERT_EQUAL( velocity, _test_values[2], "Received unexpected velocity in note on message." );
  return 0;
}

static int _receive_pkp( struct MIDIDevice * device, MIDIChannel channel, MIDIKey key, MIDIPressure pressure ) {
  printf( "Polyphonic Key Pressure ( channel=%i, key=%i, pressure=%i )\n", (int) channel, (int) key, (int) pressure );
  ASSERT_EQUAL( channel,  _test_values[0], "Received unexpected channel in polyphonic key pressure message." );
  ASSERT_EQUAL( key,      _test_values[1], "Received unexpected key in polyphonic key pressure message." );
  ASSERT_EQUAL( pressure, _test_values[2], "Received unexpected pressure in polyphonic key pressure message." );
  return 0;
}

static int _receive_cc( struct MIDIDevice * device, MIDIChannel channel, MIDIControl control, MIDIValue value ) {
  printf( "Control Change ( channel=%i, control=0x%02x, value=%i )\n", (int) channel, (int) control, (int) value );
  ASSERT_EQUAL( channel, _test_values[0], "Received unexpected channel in control change message." );
  ASSERT_EQUAL( control, _test_values[1], "Received unexpected control in control change message." );
  ASSERT_EQUAL( value,   _test_values[2], "Received unexpected value in control change message." );
  return 0;
}

static int _receive_pc( struct MIDIDevice * device, MIDIChannel channel, MIDIProgram program ) {
  printf( "Program Change ( channel=%i, program=%i )\n", (int) channel, (int) program );
  ASSERT_EQUAL( channel, _test_values[0], "Received unexpected channel in program change message." );
  ASSERT_EQUAL( program, _test_values[1], "Received unexpected program in program change message." );
  return 0;
}

static int _receive_cp( struct MIDIDevice * device, MIDIChannel channel, MIDIPressure pressure ) {
  printf( "Channel Pressure ( channel=%i, pressure=%i )\n", (int) channel, (int) pressure );
  ASSERT_EQUAL( channel,  _test_values[0], "Received unexpected channel in channel pressure message." );
  ASSERT_EQUAL( pressure, _test_values[1], "Received unexpected pressure in channel pressure message." );
  return 0;
}

static int _receive_pwc( struct MIDIDevice * device, MIDIChannel channel, MIDILongValue value ) {
  printf( "Pitch Wheel Change ( channel=%i, value=%i )\n", (int) channel, (int) value );
  ASSERT_EQUAL( channel,         _test_values[0], "Received unexpected channel in pitch wheel change message." );
  ASSERT_EQUAL( MIDI_MSB(value), _test_values[1], "Received unexpected value in pitch wheel change message." );
  ASSERT_EQUAL( MIDI_LSB(value), _test_values[2], "Received unexpected value in pitch wheel change message." );
  return 0;
}

static int _receive_sx( struct MIDIDevice * device, MIDIManufacturerId manufacturer_id, size_t size, void * data, uint8_t fragment ) {
  printf( "System Exclusive ( manufacturer_id=0x%02x, size=%i, fragment=%i )\n", (int) manufacturer_id, (int) size, (int) fragment );
  ASSERT_EQUAL( manufacturer_id, _test_values[0], "Received unexpected manufacturer id in system exclusive message." );
  ASSERT_EQUAL( size,            _test_values[1], "Received unexpected size in system exclusive message." );
  ASSERT_EQUAL( fragment,        _test_values[2], "Received unexpected fragment in system exclusive message." );
/*uint8_t * values = data;*/
  return 0;
}

static int _receive_tcqf( struct MIDIDevice * device, MIDIValue time_code_type, MIDIValue value ) {
  return 0;
}

static int _receive_spp( struct MIDIDevice * device, MIDILongValue value ) {
  return 0;
}

static int _receive_ss( struct MIDIDevice * device, MIDIValue value ) {
  return 0;
}

static int _receive_tr( struct MIDIDevice * device ) {
  return 0;
}

static int _receive_eox( struct MIDIDevice * device ) {
  return 0;
}

static int _receive_rt( struct MIDIDevice * device, MIDIStatus status, MIDITimestamp timestamp ) {
  return 0;
}

static struct MIDIDeviceDelegate _test_device = {
  &_receive_nof,
  &_receive_non,
  &_receive_pkp,
  &_receive_cc,
  &_receive_pc,
  &_receive_cp,
  &_receive_pwc,
  &_receive_sx,
  &_receive_tcqf,
  &_receive_spp,
  &_receive_ss,
  &_receive_tr,
  &_receive_eox,
  &_receive_rt
};

static int _send( void * implementation, struct MIDIMessage * message ) {
  struct MIDIDriver * driver = implementation;
  size_t size, i;
  uint8_t buffer[32];
  MIDIMessageGetSize( message, &size );
  MIDIMessageEncode( message, sizeof(buffer), &(buffer[0]), NULL );
  for( i=0; i<size; i++ ) {
    if( i%16 == 15 || i==size-1 ) {
      printf( "%02x\n", buffer[i] );
    } else {
      printf( "%02x ", buffer[i] );
    }
  }
  return MIDIDriverReceive( driver, message );
}

static struct MIDIDriverDelegate _test_driver = MIDI_DRIVER_DELEGATE_INITIALIZER;

/**
 * Integration test.
 * Create a (first) device that is used as input & sends messages to another device.
 * The second device is connected to a driver that sends its messages.
 * The messages that are received by the driver are given to the first device:
 *
 * [device 1] -(out)-----(in)-> [device 2] -(out)-------> [driver] (loopback)
 *        ^-(in)--------------------------------------------´
 *
 * device 1 receives from driver
 * device 2 receives from device 1 out
 * driver receives from device 2 out, loopbacks to device 1
 *
 * When a message is sent on device 1 the only receiver is device 2
 * When a message is sent on device 2 the message passes the driver and reaches device 1
 */
int test001_integration( void ) {
  struct MIDIDevice * device_1;
  struct MIDIDevice * device_2;
  struct MIDIDriver * driver;
  struct MIDIConnector * connector;
  
  device_1 = MIDIDeviceCreate( &_test_device );
  device_2 = MIDIDeviceCreate( &_test_device );
  driver = MIDIDriverCreate( &_test_driver );
  _test_driver.send = &_send;
  _test_driver.implementation = driver;
  connector = MIDIConnectorCreate();
  
  ASSERT_NOT_EQUAL( device_1,  NULL, "Could not create device 1." );
  ASSERT_NOT_EQUAL( device_2,  NULL, "Could not create device 2." );
  ASSERT_NOT_EQUAL( device_2,  NULL, "Could not create driver." );
  ASSERT_NOT_EQUAL( connector, NULL, "Could not create connector." );
  ASSERT_NO_ERROR( MIDIDeviceAttachOut( device_1, connector ), "Could not attach connector to device 1 out port." );
  ASSERT_NO_ERROR( MIDIDeviceAttachIn( device_2, connector ), "Could not attach connector to device 2 in port." );
  MIDIConnectorRelease( connector ); /* connector is retained by the device 1 & 2. */
  connector = NULL;
  
  ASSERT_NO_ERROR( MIDIDriverProvideSendConnector( driver, &connector ), "Could not provide driver output." );
  ASSERT_NO_ERROR( MIDIDeviceAttachOut( device_2, connector ), "Could not attach connector to device 2 out port." );
  connector = NULL;
  
  ASSERT_NO_ERROR( MIDIDriverProvideReceiveConnector( driver, &connector ), "Could not provide driver input." );
  ASSERT_NO_ERROR( MIDIDeviceAttachIn( device_1, connector ), "Could not attach connector to device 1 in port." );
  connector = NULL;
  
  _test_values[0] = MIDI_CHANNEL_1;
  _test_values[1] = 60;
  _test_values[2] = 92;
  ASSERT_NO_ERROR( MIDIDeviceSendNoteOn( device_2, _test_values[0], _test_values[1], _test_values[2] ), "Could not send note on event." );

  _test_values[0] = MIDI_CHANNEL_1;
  _test_values[1] = 60;
  _test_values[2] = 127;
  ASSERT_NO_ERROR( MIDIDeviceSendPolyphonicKeyPressure( device_2, _test_values[0], _test_values[1], _test_values[2] ), "Could not send polyphonic key pressure event." );

  _test_values[0] = MIDI_CHANNEL_1;
  _test_values[1] = 60;
  _test_values[2] = 0;
  ASSERT_NO_ERROR( MIDIDeviceSendNoteOff( device_2, _test_values[0], _test_values[1], _test_values[2] ), "Could not send note off event." );
  
  _test_values[0] = MIDI_CHANNEL_2;
  _test_values[1] = 0x07; /* Volume controller */
  _test_values[2] = 64;
  ASSERT_NO_ERROR( MIDIDeviceSendControlChange( device_2, _test_values[0], _test_values[1], _test_values[2] ), "Could not send control change event." );

  _test_values[0] = MIDI_CHANNEL_3;
  _test_values[1] = 0x18;
  ASSERT_NO_ERROR( MIDIDeviceSendProgramChange( device_2, _test_values[0], _test_values[1] ), "Could not send program change event." );

  _test_values[0] = MIDI_CHANNEL_4;
  _test_values[1] = 0x18;
  ASSERT_NO_ERROR( MIDIDeviceSendChannelPressure( device_2, _test_values[0], _test_values[1] ), "Could not send channel pressure event." );

  _test_values[0] = MIDI_CHANNEL_5;
  _test_values[1] = 123;
  _test_values[2] = 45;
  ASSERT_NO_ERROR( MIDIDeviceSendPitchWheelChange( device_2, _test_values[0], MIDI_LONG_VALUE( _test_values[1], _test_values[2] ) ), "Could not send pitch wheel change event." );

  MIDIDriverRelease( driver );
  MIDIDeviceRelease( device_1 );
  MIDIDeviceRelease( device_2 );

  return 0;
}
