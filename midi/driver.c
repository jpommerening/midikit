#include <stdlib.h>
#include "driver.h"
#include "message.h"
#include "connector.h"

struct MIDIReceiver;
  
struct MIDIDriver {
  size_t refs;
  struct MIDIDriverDelegate * delegate;
  struct MIDIReceiver * receivers;
  struct MIDIClock * clock;
};

struct MIDIReceiver {
  struct MIDIConnector * target;
  struct MIDIReceiver * next;
};

static struct MIDIDriverDelegate _loopback = {
  &MIDIDriverReceive
};

struct MIDIDriverDelegate * midiDriverLoopback = &_loopback;

struct MIDIDriver * MIDIDriverCreate() {
  struct MIDIDriver * driver = malloc( sizeof( struct MIDIDriver ) );
  driver->refs = 1;
  driver->delegate = NULL;
  driver->receivers = NULL;
  driver->clock = NULL;
  return driver;
}

void MIDIDriverDestroy( struct MIDIDriver * driver ) {
  struct MIDIReceiver * receiver = driver->receivers;
  struct MIDIReceiver * next_receiver;
  if( driver->clock != NULL ) {
    MIDIClockRelease( driver->clock );
  }
  while( receiver != NULL ) {
    MIDIConnectorRelease( receiver->target );
    next_receiver = receiver->next;
    free( receiver );
    receiver = next_receiver;
  }
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

int MIDIDriverProvideOutput( struct MIDIDriver * driver, struct MIDIConnector ** output ) {
  struct MIDIConnector * connector;
  if( output == NULL ) return 1;
  connector = MIDIConnectorCreate();
  if( connector == NULL ) return 1;
  MIDIConnectorAttachDriver( connector, driver );
  *output = connector;
  return 0;
}

int MIDIDriverProvideInput( struct MIDIDriver * driver, struct MIDIConnector ** input ) {
  struct MIDIConnector * connector;
  struct MIDIReceiver * entry;
  if( input == NULL ) return 1;
  connector = MIDIConnectorCreate();
  if( connector == NULL ) return 1;
  entry = malloc( sizeof( struct MIDIReceiver ) );
  entry->target = connector;
  entry->next = driver->receivers;
  driver->receivers = entry;
  *input = connector;
  return 0;
}

int MIDIDriverSend( struct MIDIDriver * driver, struct MIDIMessage * message ) {
  if( driver->delegate == NULL || driver->delegate->send == NULL ) {
    return 0;
  }
  return (*driver->delegate->send)( driver, message );
}

int MIDIDriverReceive( struct MIDIDriver * driver, struct MIDIMessage * message ) {
  struct MIDIReceiver * entry = driver->receivers;
  while( entry != NULL ) {
    MIDIConnectorRelay( entry->target, message );
  }
  return 0;
}
