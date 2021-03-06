#include "test.h"
#include "midi/port.h"

struct TestPort {
  int value;
};
MIDI_TYPE_SPEC( TestPort, 123, NULL, NULL, NULL, NULL );

int _receive( void * target, void * source, struct MIDITypeSpec * type, void * data ) {
  if( type == TestPortType ) {
    *(int*)target = *(int*)data;
  }
  return 0;
}

/**
 * Test that ports can be connected released, etc.
 */
int test001_port( void ) {
  int a, b, v;
  struct MIDIPort * port_a = MIDIPortCreate( "port a", MIDI_PORT_IN | MIDI_PORT_OUT, &a, &_receive );
  struct MIDIPort * port_b = MIDIPortCreate( "port b", MIDI_PORT_IN | MIDI_PORT_OUT, &b, &_receive );

  ASSERT_NOT_EQUAL( port_a, NULL, "Could not create port a!" );
  ASSERT_NOT_EQUAL( port_b, NULL, "Could not create port b!" );

  ASSERT_NO_ERROR( MIDIPortConnect( port_a, port_b ), "Could not connect MIDI ports!" );
  ASSERT_NO_ERROR( MIDIPortDisconnect( port_a, port_b ), "Could not disconnect MIDI ports!" );

  v = 123;
  ASSERT_NO_ERROR( MIDIPortSendTo( port_a, port_b, TestPortType, &v ), "Could not send without connection." );
  ASSERT_EQUAL( b, v, "Did not receive correct message." );

  v = 234;
  ASSERT_NO_ERROR( MIDIPortConnect( port_b, port_a ), "Could not connect MIDI ports!" );
  ASSERT_NO_ERROR( MIDIPortSend( port_b, TestPortType, &v ), "Could not send with connection." );
  ASSERT_EQUAL( a, v, "Did not receive correct message." );

  /* create a retainment cycle */
  ASSERT_NO_ERROR( MIDIPortConnect( port_a, port_b ), "Could not connect MIDI ports (bidirectional)!" );

  /* invalidating one port should suffice to break the retainment cycle */
  ASSERT_NO_ERROR( MIDIPortInvalidate( port_a ), "Could not invalidate MIDI port!" );

  MIDIPortRelease( port_a );
  MIDIPortRelease( port_b );

  return 0;
}

/**
 * Test something else ..
 */
