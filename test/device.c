#include "test.h"
#include "midi/message.h"
#include "midi/connector.h"
#include "midi/device.h"

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
  device = MIDIDeviceCreate( NULL );
  ASSERT_NOT_EQUAL( device, NULL, "Could not create device!" );
  ASSERT_NO_ERROR( MIDIDeviceAttachIn( device, connector ), "Could not attach input to device." );
  ASSERT_NO_ERROR( MIDIConnectorRelay( connector, message ), "Could not relay message." );
  MIDIConnectorRelease( connector );
  MIDIDeviceRelease( device );
  MIDIMessageRelease( message );
  return 0;
}

