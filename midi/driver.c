#include <stdlib.h>
#include "driver.h"
#include "message.h"
#include "input.h"
#include "output.h"

struct MIDISender;
struct MIDIReceiver;
  
struct MIDIDriver {
  size_t refs;
  struct MIDIDriverContext * context;
  struct MIDISender   * senders;
  struct MIDIReceiver * receivers;
  struct MIDIClock    * clock;
};

struct MIDISender {
  struct MIDIOutput * output;
  struct MIDISender * next;
};

struct MIDIReceiver {
  struct MIDIInput    * input;
  struct MIDIReceiver * next;
};

static struct MIDIDriverContext _loopback = {
  &MIDIDriverReceive
};

struct MIDIDriverContext * midiDriverLoopback = &_loopback;

struct MIDIDriver * MIDIDriverCreate() {
  struct MIDIDriver * driver = malloc( sizeof( struct MIDIDriver ) );
  driver->refs = 1;
  driver->context = NULL;
  driver->senders = NULL;
  driver->receivers = NULL;
  driver->clock = NULL;
  return driver;
}

void MIDIDriverDestroy( struct MIDIDriver * driver ) {
  struct MIDIReceiver * receiver = driver->receivers;
  struct MIDIReceiver * next_receiver;
  struct MIDISender * sender = driver->senders;
  struct MIDISender * next_sender;
  if( driver->clock != NULL ) {
    MIDIClockRelease( driver->clock );
  }
  while( receiver != NULL ) {
    MIDIInputRelease( receiver->input );
    next_receiver = receiver->next;
    free( receiver );
    receiver = next_receiver;
  }
  while( sender != NULL ) {
    MIDIOutputRelease( sender->output );
    next_sender = sender->next;
    free( sender );
    sender = next_sender;
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

int MIDIDriverAddSender( struct MIDIDriver * driver, struct MIDIOutput * sender ) {
  if( sender == NULL ) {
    return 1;
  }
  struct MIDISender * entry = malloc( sizeof( struct MIDISender ) );
  entry->output = sender;
  entry->next   = driver->senders;
  driver->senders = entry;
  MIDIOutputRetain( sender );
  return 0;
}

int MIDIDriverRemoveSender( struct MIDIDriver * driver, struct MIDIOutput * sender ) {
  if( sender == NULL ) {
    return 1;
  }
  struct MIDISender * entry = driver->senders;
  while( entry != NULL ) {
    if( entry->next != NULL && entry->next->output == sender ) {
      entry->next = entry->next->next;
      MIDIOutputRelease( sender );
      return 0;
    }
  }
  return 1;
}

int MIDIDriverAddReceiver( struct MIDIDriver * driver, struct MIDIInput * receiver ) {
  if( receiver == NULL ) {
    return 1;
  }
  struct MIDIReceiver * entry = malloc( sizeof( struct MIDIReceiver ) );
  entry->input = receiver;
  entry->next = driver->receivers;
  driver->receivers = entry;
  MIDIInputRetain( receiver );
  return 0;
}

int MIDIDriverRemoveReceiver( struct MIDIDriver * driver, struct MIDIInput * receiver ) {
  if( receiver == NULL ) {
    return 1;
  }
  struct MIDIReceiver * entry = driver->receivers;
  while( entry != NULL ) {
    if( entry->next != NULL && entry->next->input == receiver ) {
      entry->next = entry->next->next;
      MIDIInputRelease( receiver );
      return 0;
    }
  }
  return 1;
}

int MIDIDriverSend( struct MIDIDriver * driver, struct MIDIMessage * message ) {
  if( driver->context == NULL || driver->context->send == NULL ) {
    return 0;
  }
  return (*driver->context->send)( driver, message );
}

int MIDIDriverReceive( struct MIDIDriver * driver, struct MIDIMessage * message ) {
  struct MIDIReceiver * entry = driver->receivers;
  while( entry != NULL ) {
    MIDIInputReceive( entry->input, message );
  }
  return 0;
}
