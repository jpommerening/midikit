#include "test.h"
#include "midi/message.h"
#include "midi/connector.h"

static struct {
  MIDIStatus status;
  size_t refs;
  struct MIDIConnector * connector;
} _target;

static int _relay( void * target, struct MIDIMessage * message ) {
  ASSERT_EQUAL( target, (void*) &_target, "Connector target points to unexpected address." );
  return MIDIMessageGet( message, MIDI_STATUS, sizeof(MIDIStatus), &(_target.status) );
}

static int _connect( void * target, struct MIDIConnector * connector ) {
  ASSERT_EQUAL( target, &_target, "Trying to connect to invalid target." );
  _target.refs++;
  _target.connector = connector;
  return 0;
}

static int _invalidate( void * target, struct MIDIConnector * connector ) {
  ASSERT_EQUAL( target, &_target, "Trying to connect to invalid target." );
  ASSERT_EQUAL( connector, _target.connector, "Trying to disconnect invalid connector." );
  _target.refs--;
  _target.connector = NULL;
  return 0;
}

static struct MIDIConnectorTargetDelegate _test_delegate = {
  &_relay,
  &_connect,
  &_invalidate
};

/**
 * Test that a MIDI connector can be created and passes messages.
 */
int test001_connector( void ) {
  struct MIDIConnector * connector;
  struct MIDIMessage * message;
  
  _target.status = 0;
  _target.refs = 0;
  
  connector = MIDIConnectorCreate();
  ASSERT_NOT_EQUAL( connector, NULL, "Could not create connector!" );
  message = MIDIMessageCreate( MIDI_STATUS_RESET );
  ASSERT_NOT_EQUAL( message, NULL, "Could not create reset message!" );
  ASSERT_NO_ERROR( MIDIConnectorAttachTargetWithDelegate( connector, &_target, &_test_delegate ), "Could not attach target to connector." );
  ASSERT_EQUAL( _target.refs, 1, "Connector did not retain target." );
  ASSERT_NO_ERROR( MIDIConnectorRelay( connector, message ), "Could not relay message." );
  ASSERT_EQUAL( _target.status, MIDI_STATUS_RESET, "Connector did not pass correct message." );
  MIDIConnectorRelease( connector );
  ASSERT_EQUAL( _target.refs, 0, "Connector did not release target." );
  MIDIMessageRelease( message );
  return 0;
}

