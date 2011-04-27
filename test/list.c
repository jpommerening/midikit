#include "test.h"
#include "midi/list.h"
#include "midi/type.h"

struct Test {
  int refs;
};

static struct Test _testitem = { 1 };

static void _retain_item( struct Test * item ) {
  item->refs++;
}

static void _release_item( struct Test * item ) {
  item->refs--;
}

static int _apply_set( void * item, void * info ) {
  *(int*)item = *(int*)info;
  return 0;
}

MIDI_TYPE_SPEC( Test, 0x0000, &_retain_item, &_release_item, NULL, NULL );

/**
 * Test that lists can be created and items can be added and reference counting works.
 */
int test001_list( void ) {
  struct MIDIList * list = MIDIListCreate( TestType );

  ASSERT_NOT_EQUAL( list, NULL, "Could not create list!" );

  ASSERT_NO_ERROR( MIDIListAdd( list, &_testitem ), "Could not add item." );
  ASSERT_EQUAL( _testitem.refs, 2, "Item was not retained." );
  ASSERT_NO_ERROR( MIDIListRemove( list, &_testitem ), "Could not add item." );
  ASSERT_EQUAL( _testitem.refs, 1, "Item was not released." );
  ASSERT_NO_ERROR( MIDIListAdd( list, &_testitem ), "Could not add item." );
  ASSERT_EQUAL( _testitem.refs, 2, "Item was not retained." );
  MIDIListRelease( list );
  ASSERT_EQUAL( _testitem.refs, 1, "Item was not released on destruction." );
  return 0;
}

/**
 * Test that apply functionality works.
 */
int test002_list( void ) {
  int a, b, c;
  int v = 123;
  struct MIDIList * list = MIDIListCreate( TestType );

  ASSERT_NOT_EQUAL( list, NULL, "Could not create list!" );

  ASSERT_NO_ERROR( MIDIListAdd( list, &a ), "Could not add item." );
  ASSERT_NO_ERROR( MIDIListAdd( list, &b ), "Could not add item." );
  ASSERT_NO_ERROR( MIDIListAdd( list, &c ), "Could not add item." );

  ASSERT_NO_ERROR( MIDIListApply( list, &v, &_apply_set ), "Could not apply set function." );
  ASSERT_EQUAL( a, v, "Setter did not set list item a." );
  ASSERT_EQUAL( b, v, "Setter did not set list item b." );
  ASSERT_EQUAL( c, v, "Setter did not set list item c." );

  MIDIListRelease( list );
  return 0;
}
