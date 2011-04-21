#include "test.h"
#include "midi/port.h"
#include "midi/message.h"
#include "midi/device.h"

MIDIStatus _status;

static int _receive_rt( struct MIDIDevice * device, MIDIStatus status, MIDITimestamp timestamp ) {
  _status = status;
  return 0;
}

static struct MIDIDeviceDelegate _test_device = {
  NULL, /* recv_nof  */
  NULL, /* recv_non  */
  NULL, /* recv_pkp  */
  NULL, /* recv_cc   */
  NULL, /* recv_pc   */
  NULL, /* recv_cp   */
  NULL, /* recv_pwc  */
  NULL, /* recv_sx   */
  NULL, /* recv_tcqf */
  NULL, /* recv_spp  */
  NULL, /* recv_ss   */
  NULL, /* recv_tr   */
  NULL, /* recv_eox  */
  &_receive_rt
};

/**
 * Test that a MIDI device can be created and receives messages.
 */
int test001_device( void ) {
  struct MIDIPort * port;
  struct MIDIMessage * message;
  struct MIDIDevice * device;
    
  port = MIDIPortCreate( "TestPort", MIDI_PORT_OUT, NULL, NULL );
  ASSERT_NOT_EQUAL( port, NULL, "Could not create port!" );
  message = MIDIMessageCreate( MIDI_STATUS_RESET );
  ASSERT_NOT_EQUAL( message, NULL, "Could not create reset message!" );
  device = MIDIDeviceCreate( &_test_device );
  ASSERT_NOT_EQUAL( device, NULL, "Could not create device!" );
  ASSERT_NO_ERROR( MIDIDeviceAttachIn( device, port ), "Could not attach input to device." );
  ASSERT_NO_ERROR( MIDIPortSend( port, MIDIMessageType, message ), "Could not send message." );
  ASSERT_EQUAL( _status, MIDI_STATUS_RESET, "Test device did not receive relayed reset message." );
  ASSERT_NO_ERROR( MIDIDeviceReceiveRealTime( device, MIDI_STATUS_TIMING_CLOCK, -1 ), "Could not simulate received message." );
  ASSERT_EQUAL( _status, MIDI_STATUS_TIMING_CLOCK, "Test device did not receive relayed clock message." );

  MIDIPortRelease( port );
  MIDIMessageRelease( message );
  MIDIDeviceRelease( device );
  return 0;
}

/**
 * Test that chaining works.
 */
int test002_device( void ) {
  struct MIDIPort * port;
  struct MIDIMessage * message;
  struct MIDIDevice * device_master;
  struct MIDIDevice * device_slave;

  message = MIDIMessageCreate( MIDI_STATUS_START );
  ASSERT_NOT_EQUAL( message, NULL, "Could not create reset message!" );
  device_master = MIDIDeviceCreate( NULL );
  ASSERT_NOT_EQUAL( device_master, NULL, "Could not create device!" );
  device_slave = MIDIDeviceCreate( &_test_device );
  ASSERT_NOT_EQUAL( device_slave, NULL, "Could not create device!" );

  ASSERT_NO_ERROR( MIDIDeviceGetThroughPort( device_master, &port ), "Could get master thru port." );
  ASSERT_NO_ERROR( MIDIDeviceAttachIn( device_slave, port ), "Could not attach port to slave in port." );
  ASSERT_NO_ERROR( MIDIDeviceReceive( device_master, message ), "Could not receive message on master device." );
  ASSERT_EQUAL( _status, MIDI_STATUS_START, "Slave device did not receive message from master thru." );

  ASSERT_NO_ERROR( MIDIDeviceDetachThru( device_master ), "Could not detach port from master thru port." );
  ASSERT_NO_ERROR( MIDIDeviceGetOutputPort( device_master, &port ), "Could get master out port." );
  ASSERT_NO_ERROR( MIDIDeviceAttachIn( device_slave, port ), "Could not attach port to slave in port." );

  ASSERT_NO_ERROR( MIDIMessageSetStatus( message, MIDI_STATUS_STOP ), "Could not change status of real time message." );
  ASSERT_NO_ERROR( MIDIDeviceSend( device_master, message ), "Could not receive message on master device." );
  ASSERT_EQUAL( _status, MIDI_STATUS_STOP, "Slave device did not receive message from master out." );

  MIDIDeviceDetachOut( device_master );

  MIDIPortRelease( port );
  MIDIMessageRelease( message );
  MIDIDeviceRelease( device_master );
  MIDIDeviceRelease( device_slave );
  return 0;
}
