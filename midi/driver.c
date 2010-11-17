#include <stdlib.h>
#include "driver.h"
#include "message.h"
#include "connector.h"

struct MIDIConnectorList;
  
struct MIDIDriver {
  size_t refs;
  struct MIDIDriverDelegate * delegate;
  struct MIDIConnectorList  * receivers;
  struct MIDIConnectorList  * senders;
  struct MIDIClock * clock;
};

struct MIDIConnectorList {
  struct MIDIConnector     * connector;
  struct MIDIConnectorList * next;
};

struct MIDIDriver * MIDIDriverCreate( struct MIDIDriverDelegate * delegate ) {
  struct MIDIDriver * driver = malloc( sizeof( struct MIDIDriver ) );
  driver->refs = 1;
  driver->delegate  = delegate;
  driver->receivers = NULL;
  driver->senders   = NULL;
  driver->clock     = NULL;
  return driver;
}

static void _list_destroy( struct MIDIConnectorList ** list, int (*fn)( struct MIDIConnector * ) ) {
  struct MIDIConnectorList * item;
  struct MIDIConnectorList * next;
  item = *list;
  *list = NULL;
printf( "destroy\n" );
  while( item != NULL ) {
    if( fn != NULL ) (*fn)( item->connector );
    MIDIConnectorRelease( item->connector );
    next = item->next;
    free( item );
    item = next;
  }
}

static void _list_push( struct MIDIConnectorList ** list, struct MIDIConnector * connector ) {
  struct MIDIConnectorList * item = malloc( sizeof( struct MIDIConnectorList ) );
  if( list == NULL ) return;
  if( item == NULL ) return;
  item->connector = connector;
  item->next = *list;
  MIDIConnectorRetain( connector );
  *list = item;
}

static void _list_remove( struct MIDIConnectorList ** list, struct MIDIConnector * connector, int (*fn)( struct MIDIConnector * ) ) {
  struct MIDIConnectorList * item;
  struct MIDIConnectorList * release;
  if( list == NULL ) return;
  while( *list != NULL ) {
    item = *list;
    if( item->connector == connector ) {
      *list = item->next;
      free( item );
      _list_push( &release, connector );
    } else {
      list = &(item->next);
    }
  }
printf( "release listed." );
  _list_destroy( &release, fn );
printf( "remove done\n" );
}

static int _receiver_connect( void * driverp, struct MIDIConnector * receiver ) {
  struct MIDIDriver * driver = driverp;
  _list_push( &(driver->receivers), receiver );
  return 0;
}

static int _receiver_invalidate( void * driverp, struct MIDIConnector * receiver ) {
  struct MIDIDriver * driver = driverp;
  _list_remove( &(driver->receivers), receiver, &MIDIConnectorDetachSource );
  return 0;
}

static int _sender_relay( void * driverp, struct MIDIMessage * message ) {
  return MIDIDriverSend( driverp, message );
}

static int _sender_connect( void * driverp, struct MIDIConnector * sender ) {
  struct MIDIDriver * driver = driverp;
  _list_push( &(driver->senders), sender );
  return 0;
}

static int _sender_invalidate( void * driverp, struct MIDIConnector * sender ) {
  struct MIDIDriver * driver = driverp;
  _list_remove( &(driver->senders), sender, &MIDIConnectorDetachTarget );
  return 0;
}

struct MIDIConnectorSourceDelegate MIDIDriverReceiveConnectorDelegate = {
  &_receiver_connect,
  &_receiver_invalidate
};

struct MIDIConnectorTargetDelegate MIDIDriverSendConnectorDelegate = {
  &_sender_relay,
  &_sender_connect,
  &_sender_invalidate
};

void MIDIDriverDestroy( struct MIDIDriver * driver ) {
  if( driver->clock != NULL ) {
    MIDIClockRelease( driver->clock );
  }
  _list_destroy( &(driver->receivers), &MIDIConnectorDetachSource );
  _list_destroy( &(driver->senders), &MIDIConnectorDetachTarget );
  free( driver );
}

void MIDIDriverRetain( struct MIDIDriver * driver ) {
  driver->refs++;
}

void MIDIDriverRelease( struct MIDIDriver * driver ) {
  if( ! --driver->refs ) {
    MIDIDriverDestroy( driver );
  }
}

int MIDIDriverProvideSendConnector( struct MIDIDriver * driver, struct MIDIConnector ** send ) {
  struct MIDIConnector * connector;
  if( send == NULL ) return 1;
  connector = MIDIConnectorCreate();
  if( connector == NULL ) return 1;
  MIDIConnectorAttachToDriver( connector, driver );
  _list_push( &(driver->senders), connector );
  *send = connector;
  MIDIConnectorRelease( connector ); // retained by list
  return 0;
}

int MIDIDriverProvideReceiveConnector( struct MIDIDriver * driver, struct MIDIConnector ** receive ) {
  struct MIDIConnector * connector;
  if( receive == NULL ) return 1;
  connector = MIDIConnectorCreate();
  if( connector == NULL ) return 1;
  MIDIConnectorAttachFromDriver( connector, driver );
  _list_push( &(driver->receivers), connector );
  *receive = connector;
  MIDIConnectorRelease( connector ); // retained by list
  return 0;
}

int MIDIDriverSend( struct MIDIDriver * driver, struct MIDIMessage * message ) {
  if( driver->delegate == NULL || driver->delegate->send == NULL ) {
    return 0;
  }
  return (*driver->delegate->send)( driver, message );
}

int MIDIDriverReceive( struct MIDIDriver * driver, struct MIDIMessage * message ) {
  struct MIDIConnectorList * item = driver->receivers;
  int result = 0;
  while( item != NULL ) {
    result += MIDIConnectorRelay( item->connector, message );
    item = item->next;
  }
  return result;
}
