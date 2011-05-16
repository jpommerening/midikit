#include <stdlib.h>
#include "midi.h"
#include "list.h"

struct MIDIEnumerator;

/**
 * @ingroup MIDI
 * @brief A list of MIDI objects of a common type.
 * I realize that this a general purpose linked list and should not be
 * restricted to MIDIKit use as the MIDI-prefix might indicate.
 * I will evaluate and extract general purpose code at a later point.
 * @todo allow iteration, assure ordering (insert before, insert after, etc.)
 */
struct MIDIList {
/**
 * @privatesections
 * @cond INTERNALS
 */
  int    refs;
  struct MIDITypeSpec   * type;
  struct MIDIEnumerator * first;
  struct MIDIEnumerator * last;
/** @endcond */
};

/* MARK: Internals *//**
 * @name Internals
 * @cond INTERNALS
 * @{
 */

struct MIDIEnumerator {
  void * object;
  struct MIDIEnumerator * next;
};

/**
 * @brief Use the callback to retain a list item.
 * @private @memberof MIDIList
 * @param list The list.
 * @param item The item to retain.
 */
static void _list_item_retain( struct MIDIList * list, void * item ) {
  MIDIAssert( list != NULL );
  if( item != NULL && list->type != NULL && list->type->retain != NULL ) {
    (*list->type->retain)( item );
  }
}

/**
 * @brief Use the callback to release a list item.
 * @private @memberof MIDIList
 * @param list The list.
 * @param item The item to release.
 */
static void _list_item_release( struct MIDIList * list, void * item ) {
  MIDIAssert( list != NULL );
  if( item != NULL && list->type != NULL && list->type->release ) {
    (*list->type->release)( item );
  }
}

/**
 * @}
 * @endcond
 */

/* MARK: -
 * MARK: Creation and destruction *//**
 * @name Creation and destruction
 * Creating, destroying and reference counting of MIDIConnector objects.
 * @{
 */

/**
 * @brief Create a MIDIList instance.
 * Allocate space and initialize a MIDIList instance.
 * @public @memberof MIDIList
 * @param type The type of the elements to be stored.
 * @return a pointer to the created list structure on success.
 * @return a @c NULL pointer if the list could not created.
 */
struct MIDIList * MIDIListCreate( struct MIDITypeSpec * type ) {
  struct MIDIList * list = malloc( sizeof( struct MIDIList ) );
  MIDIPrecondReturn( list != NULL, ENOMEM, NULL );

  list->refs  = 1;
  list->type  = type;
  list->first = NULL;
  list->last  = NULL;

  return list;
}

/**
 * @brief Destroy a MIDIList instance.
 * Free all resources occupied by the list and release all
 * list items.
 * @public @memberof MIDIList
 * @param list The list.
 */
void MIDIListDestroy( struct MIDIList * list ) {
  struct MIDIEnumerator * entry;
  struct MIDIEnumerator * next;
  MIDIPrecondReturn( list != NULL, EFAULT, (void)0 );
  entry = list->first;
  list->first = NULL;
  list->last  = NULL;
  while( entry != NULL ) {
    next = entry->next;
    _list_item_release( list, entry->object );
    free( entry );
    entry = next;
  }
  free( list );
}

/**
 * @brief Retain a MIDIList instance.
 * Increment the reference counter of a list so that it won't be destroyed.
 * @public @memberof MIDIList
 * @param list The list.
 */
void MIDIListRetain( struct MIDIList * list ) {
  MIDIPrecondReturn( list != NULL, EFAULT, (void)0 );
  list->refs++;
}

/**
 * @brief Release a MIDIList instance.
 * Decrement the reference counter of a list. If the reference count
 * reached zero, destroy the list.
 * @public @memberof MIDIList
 * @param list The list.
 */
void MIDIListRelease( struct MIDIList * list ) {
  MIDIPrecondReturn( list != NULL, EFAULT, (void)0 );
  if( ! --list->refs ) {
    MIDIListDestroy( list );
  }
}

/** @} */

/* MARK: List management *//**
 * @name List management
 * Functions for working with lists.
 * @{
 */

/**
 * @brief Add an item to the list.
 * Add one item to the list and retain it.
 * @public @memberof MIDIList
 * @param list The list.
 * @param item The item to add.
 * @retval 0 on success.
 * @retval >0 otherwise.
 */
int MIDIListAdd( struct MIDIList * list, void * item ) {
  struct MIDIEnumerator * entry;
  MIDIPrecond( list != NULL, EFAULT );
  MIDIPrecond( item != NULL, EINVAL );

  entry = malloc( sizeof( struct MIDIEnumerator ) );
  if( entry == NULL ) {
    MIDIError( ENOMEM, "Failed to allocate space for enumerator." );
  }

  entry->object = item;
  entry->next   = list->first;
  _list_item_retain( list, entry->object );
  list->first   = entry;
  if( list->last == NULL ) {
    list->last = entry;
  }
  return 0;
}

/**
 * @brief Remove an item from the list.
 * Remove one item from the list and release it.
 * @public @memberof MIDIList
 * @param list The list.
 * @param item The item to remove.
 * @retval 0 on success.
 * @retval >0 otherwise.
 */
int MIDIListRemove( struct MIDIList * list, void * item ) {
  struct MIDIEnumerator * entry;
  struct MIDIEnumerator ** eptr;
  MIDIPrecond( list != NULL, EFAULT );
  MIDIPrecond( item != NULL, EINVAL );

  eptr  = &(list->first);
  entry = list->first;
  while( entry != NULL ) {
    if( entry->object == item ) {
      if( entry == list->last ) {
        list->last = *eptr;
      }
      _list_item_release( list, entry->object );
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

/**
 * @brief Check if an item is contained inside the list.
 * Step through all items and check if any item has the given address.
 * @public @memberof MIDIList
 * @param list The list.
 * @param item The item to search for.
 * @retval 0 if the item is present in the list.
 * @retval -1 if the item is not present in the list.
 * @retval >0 if an error occurred.
 */
int MIDIListContains( struct MIDIList * list, void * item ) {
  struct MIDIEnumerator * entry;
  MIDIPrecond( list != NULL, EFAULT );
  MIDIPrecond( item != NULL, EINVAL );
  
  entry = list->first;
  while( entry != NULL ) {
    if( entry->object == item ) {
      return 0;
    }
  }
  return -1;
}

/**
 * @brief Find a given item using a comparator function.
 * Step through all items and check if the given comparator returns 0.
 * @public @memberof MIDIList
 * @param list The list.
 * @param item The result-item.
 * @param info The pointer to pass as the first parameter.
 * @param func The comparator function.
 * @retval 0 if the item is present in the list.
 * @retval -1 if the item is not present in the list.
 * @retval >0 if an error occurred.
 */
int MIDIListFind( struct MIDIList * list, void ** item, void * info, int (*func)( void *, void *) ) {
  struct MIDIEnumerator * entry;
  MIDIPrecond( list != NULL, EFAULT );
  MIDIPrecond( item != NULL, EINVAL );
  
  entry = list->first;
  while( entry != NULL ) {
    if( (*func)( entry->object, info ) == 0 ) {
      *item = entry->object;
      return 0;
    }
  }
  return -1;
}

/**
 * @brief Apply a function to all items in the list.
 * Call the given function once for every item in the list.
 * Use the given @c info pointer as the first parameter when
 * calling the function and the item as the second parameter.
 * @public @memberof MIDIList
 * @param list The list.
 * @param info The pointer to pass as the first parameter.
 * @param func The function to apply.
 * @retval 0 on success.
 * @retval >0 otherwise.
 */
int MIDIListApply( struct MIDIList * list, void * info, int (*func)( void *, void * ) ) {
  struct MIDIEnumerator * entry;
  void * item;
  int result = 0;
  MIDIPrecond( list != NULL, EFAULT );
  MIDIPrecond( func != NULL, EINVAL );

  entry = list->first;
  while( entry != NULL ) {
    item  = entry->object;
    entry = entry->next;
    if( item != NULL ) {
      result += (*func)( item, info );
    }
  }
  return result;
}

/** @} */
