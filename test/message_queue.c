#include "test.h"
#include "midi/message.h"
#include "midi/message_queue.h"

/**
 * Test that items can be pushed and popped to and from
 * the MIDIMessageQueue.
 */
int test001_message_queue( void ) {
  struct MIDIMessageQueue * queue = MIDIMessageQueueCreate();
  struct MIDIMessage * message[3] = {
    MIDIMessageCreate( MIDI_STATUS_NOTE_ON ),
    MIDIMessageCreate( MIDI_STATUS_POLYPHONIC_KEY_PRESSURE ),
    MIDIMessageCreate( MIDI_STATUS_NOTE_OFF )
  };
  struct MIDIMessage * m;
  size_t length;
  MIDIKey key = 60;
  
  ASSERT_NOT_EQUAL( queue, NULL, "Could not create message queue." );
  ASSERT_NOT_EQUAL( message[0], NULL, "Could not create message 0." );
  ASSERT_NOT_EQUAL( message[1], NULL, "Could not create message 1." );
  ASSERT_NOT_EQUAL( message[2], NULL, "Could not create message 2." );
  
  ASSERT_NO_ERROR( MIDIMessageSet( message[0], MIDI_KEY, sizeof(MIDIKey), &key ),
    "Could not set key for message 0." );
  ASSERT_NO_ERROR( MIDIMessageSet( message[1], MIDI_KEY, sizeof(MIDIKey), &key ),
    "Could not set key for message 1." );
  ASSERT_NO_ERROR( MIDIMessageSet( message[2], MIDI_KEY, sizeof(MIDIKey), &key ),
    "Could not set key for message 2." );
  
  ASSERT_NO_ERROR( MIDIMessageQueuePush( queue, message[0] ),
    "Could not enqueue message 0." );
  ASSERT_NO_ERROR( MIDIMessageQueuePush( queue, message[1] ),
    "Could not enqueue message 1." );
  ASSERT_NO_ERROR( MIDIMessageQueuePush( queue, message[2] ),
    "Could not enqueue message 2." );
  
  ASSERT_NO_ERROR( MIDIMessageQueuePeek( queue, &m ),
    "Could not peek into queue." );
  ASSERT_EQUAL( m, message[0], "Queue returned wrong message." );
  
  ASSERT_NO_ERROR( MIDIMessageQueueGetLength( queue, &length),
    "Could not determine queue length." );
  ASSERT_EQUAL( length, 3, "Message queue returned wrong length." );
  
  ASSERT_NO_ERROR( MIDIMessageQueuePop( queue, &m), "Could not pop message." );
  ASSERT_EQUAL( m, message[0], "Queue returned wrong message." );
  MIDIMessageRelease( m );
  
  ASSERT_NO_ERROR( MIDIMessageQueuePop( queue, &m), "Could not pop message." );
  ASSERT_EQUAL( m, message[1], "Queue returned wrong message." );
  MIDIMessageRelease( m );
  
  ASSERT_NO_ERROR( MIDIMessageQueueGetLength( queue, &length),
    "Could not determine queue length." );
  ASSERT_EQUAL( length, 1, "Message queue returned wrong length." );
  
  MIDIMessageRelease( message[0] );
  MIDIMessageRelease( message[1] );
  MIDIMessageRelease( message[2] );
  MIDIMessageQueueRelease( queue );
  return 0;
}
