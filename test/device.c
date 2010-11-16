#include "test.h"
#include "midi/message.h"
#include "midi/connector.h"
#include "midi/device.h"

MIDIStatus _status;

static int _receive_rt( struct MIDIDevice * device, MIDIStatus status ) {
  _status = status;
  return 0;
}

static struct MIDIDeviceDelegate _test_device = {
  NULL, // recv_nof
  NULL, // recv_non
  NULL, // recv_pkp
  NULL, // recv_cc
  NULL, // recv_pc
  NULL, // recv_cp
  NULL, // recv_pwc
  NULL, // recv_sx
  NULL, // recv_tcqf
  NULL, // recv_spp
  NULL, // recv_ss
  NULL, // recv_tr
  NULL, // recv_eox
  &_receive_rt
};

/**
 * Test that a MIDI device can be created and receives messages.
 */
int test001_device( void ) {
  struct MIDIConnector * connector;
  struct MIDIMessage * message;
  struct MIDIDevice * device;
    
  connector = MIDIConnectorCreate();
  ASSERT_NOT_EQUAL( connector, NULL, "Could not create connector!" );
  message = MIDIMessageCreate( MIDI_STATUS_RESET );
  ASSERT_NOT_EQUAL( message, NULL, "Could not create reset message!" );
  device = MIDIDeviceCreate( &_test_device );
  ASSERT_NOT_EQUAL( device, NULL, "Could not create device!" );
  ASSERT_NO_ERROR( MIDIDeviceAttachIn( device, connector ), "Could not attach input to device." );
  ASSERT_NO_ERROR( MIDIConnectorRelay( connector, message ), "Could not relay message." );
  ASSERT_EQUAL( _status, MIDI_STATUS_RESET, "Test device did not receive relayed reset message." );
  ASSERT_NO_ERROR( MIDIDeviceReceiveRealTime( device, MIDI_STATUS_TIMING_CLOCK ), "Could not simulate received message." );
  ASSERT_EQUAL( _status, MIDI_STATUS_TIMING_CLOCK, "Test device did not receive relayed clock message." );

  MIDIConnectorRelease( connector );
  MIDIMessageRelease( message );
  MIDIDeviceRelease( device );
  return 0;
}

/**
 * Test that chaining works.
 */
int test002_device( void ) {
  struct MIDIConnector * connector;
  struct MIDIMessage * message;
  struct MIDIDevice * device_master;
  struct MIDIDevice * device_slave;

  connector = MIDIConnectorCreate();
  ASSERT_NOT_EQUAL( connector, NULL, "Could not create connector!" );
  message = MIDIMessageCreate( MIDI_STATUS_START );
  ASSERT_NOT_EQUAL( message, NULL, "Could not create reset message!" );
  device_master = MIDIDeviceCreate( NULL );
  ASSERT_NOT_EQUAL( device_master, NULL, "Could not create device!" );
  device_slave = MIDIDeviceCreate( &_test_device );
  ASSERT_NOT_EQUAL( device_slave, NULL, "Could not create device!" );

  ASSERT_NO_ERROR( MIDIDeviceAttachThru( device_master, connector ), "Could not attach connector to master thru port." );
  ASSERT_NO_ERROR( MIDIDeviceAttachIn( device_slave, connector ), "Could not attach connector to slave in port." );
  ASSERT_NO_ERROR( MIDIDeviceReceive( device_master, message ), "Could not receive message on master device." );
  ASSERT_EQUAL( _status, MIDI_STATUS_START, "Slave device did not receive message from master thru." );

  ASSERT_NO_ERROR( MIDIDeviceDetachThru( device_master ), "Could not detach connector from master thru port." );
  ASSERT_NO_ERROR( MIDIDeviceAttachOut( device_master, connector ), "Could not attach connector to master out port." );
  ASSERT_NO_ERROR( MIDIMessageSetStatus( message, MIDI_STATUS_STOP ), "Could not change status of real time message." );
  ASSERT_NO_ERROR( MIDIDeviceSend( device_master, message ), "Could not receive message on master device." );
  ASSERT_EQUAL( _status, MIDI_STATUS_STOP, "Slave device did not receive message from master out." );

  MIDIDeviceDetachOut( device_master );

  MIDIConnectorRelease( connector );
  MIDIMessageRelease( message );
  MIDIDeviceRelease( device_master );
  MIDIDeviceRelease( device_slave );
  return 0;
}
