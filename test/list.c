#include "test.h"
#include "midi/list.h"

static int _testitem_refs = 1;

static void _retain_item( void * item ) {
  if( item == &_testitem_refs ) {
    _testitem_refs++;
  }
}

static void _release_item( void * item ) {
  if( item == &_testitem_refs ) {
    _testitem_refs--;
  }
}


/**
 * Test that lists can be created and items can be added and reference counting works.
 */
int test001_list( void ) {
  struct MIDIList * list = MIDIListCreate( &_retain_item, &_release_item );

  ASSERT_NOT_EQUAL( list, NULL, "Could not create list!" );

  ASSERT_NO_ERROR( MIDIListAdd( list, &_testitem_refs ), "Could not add item." );
  ASSERT_EQUAL( _testitem_refs, 2, "Item was not retained." );
  ASSERT_NO_ERROR( MIDIListRemove( list, &_testitem_refs ), "Could not add item." );
  ASSERT_EQUAL( _testitem_refs, 1, "Item was not released." );
  ASSERT_NO_ERROR( MIDIListAdd( list, &_testitem_refs ), "Could not add item." );
  ASSERT_EQUAL( _testitem_refs, 2, "Item was not retained." );
  MIDIListRelease( list );
  ASSERT_EQUAL( _testitem_refs, 1, "Item was not released on destruction." );
  return 0;
}
