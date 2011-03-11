#include <stdlib.h>
#include "midi.h"
#include "list.h"

/**
 * I realize that this a general purpose linked list and should not be
 * restricted to MIDIKit use as the MIDI-prefix might indicate.
 * I will evaluate and extract general purpose code at a later point.
 */

struct MIDIListEntry;

struct MIDIListEntry {
  void * item;
  struct MIDIListEntry * next;
};

struct MIDIList {
  int refs;
  void (*retain)( void * item );
  void (*release)( void * item );
  struct MIDIListEntry * data;
};

static void _list_item_retain( struct MIDIList * list, void * item ) {
  if( item != NULL && list->retain != NULL ) {
    (*list->retain)( item );
  }
}

static void _list_item_release( struct MIDIList * list, void * item ) {
  if( item != NULL && list->release != NULL ) {
    (*list->release)( item );
  }
}

struct MIDIList * MIDIListCreate( void (*retain)( void * ), void (*release)( void * ) ) {
  struct MIDIList * list = malloc( sizeof( struct MIDIList ) );
  MIDIPrecondReturn( list != NULL, ENOMEM, NULL );

  list->refs = 1;
  list->retain  = retain;
  list->release = release;
  list->data = NULL;

  return list;
}

void MIDIListDestroy( struct MIDIList * list ) {
  struct MIDIListEntry * entry;
  struct MIDIListEntry * next;
  MIDIPrecondReturn( list != NULL, EFAULT, (void)0 );
  entry = list->data;
  list->data = NULL;
  while( entry != NULL ) {
    next = entry->next;
    _list_item_release( list, entry->item );
    free( entry );
    entry = next;
  }
  free( list );
}

void MIDIListRetain( struct MIDIList * list ) {
  MIDIPrecondReturn( list != NULL, EFAULT, (void)0 );
  list->refs++;
}

void MIDIListRelease( struct MIDIList * list ) {
  MIDIPrecondReturn( list != NULL, EFAULT, (void)0 );
  if( ! --list->refs ) {
    MIDIListDestroy( list );
  }
}

int MIDIListAdd( struct MIDIList * list, void * item ) {
  struct MIDIListEntry * entry;
  MIDIPrecond( list != NULL, EFAULT );
  MIDIPrecond( item != NULL, EINVAL );

  entry       = malloc( sizeof( struct MIDIListEntry ) );
  entry->item = item;
  entry->next = list->data;
  _list_item_retain( list, entry->item );
  list->data  = entry;
  return 0;
}

int MIDIListRemove( struct MIDIList * list, void * item ) {
  struct MIDIListEntry * entry;
  struct MIDIListEntry ** eptr;
  MIDIPrecond( list != NULL, EFAULT );
  MIDIPrecond( item != NULL, EINVAL );

  eptr  = &(list->data);
  entry = list->data;
  while( entry != NULL ) {
    if( entry->item == item ) {
      _list_item_release( list, entry->item );
      *eptr = entry->next;
      free( entry );
      entry = *eptr;
    } else {
      eptr  = &(entry->next);
      entry = entry->next;
    }
  }
  return 0;
}


int MIDIListApply( struct MIDIList * list, void * info, int (*func)( void *, void * ) ) {
  struct MIDIListEntry * entry;
  void * item;
  MIDIPrecond( list != NULL, EFAULT );
  MIDIPrecond( func != NULL, EINVAL );

  entry = list->data;
  while( entry != NULL ) {
    item  = entry->item;
    entry = entry->next;
    if( item != NULL ) {
      (*func)( item, info );
    }
  }

  return 0;
}
