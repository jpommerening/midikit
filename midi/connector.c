#include <stdlib.h>
#include <stdio.h>
#include "connector.h"

typedef int (*RelayFn)( void *, struct MIDIMessage *  );
typedef void (*RetainFn)( void * );
typedef void (*ReleaseFn)( void * );

extern int MIDIDeviceReceive( struct MIDIDevice * device, struct MIDIMessage * message );
extern void MIDIDeviceRetain( struct MIDIDevice * device );
extern void MIDIDeviceRelease( struct MIDIDevice * device );

extern int MIDIDriverSend( struct MIDIDriver * driver, struct MIDIMessage * message );
extern void MIDIDriverRetain( struct MIDIDriver * driver );
extern void MIDIDriverRelease( struct MIDIDriver * driver );

static struct MIDIConnectorDelegate _device_delegate = {
  (RelayFn)   &MIDIDeviceReceive,
  (RetainFn)  &MIDIDeviceRetain,
  (ReleaseFn) &MIDIDeviceRelease
};

static struct MIDIConnectorDelegate _driver_delegate = {
  (RelayFn)   &MIDIDriverSend,
  (RetainFn)  &MIDIDriverRetain,
  (ReleaseFn) &MIDIDriverRelease
};

struct MIDIConnector {
  size_t refs;
  void * target;
  struct MIDIConnectorDelegate * delegate;
};

struct MIDIConnector * MIDIConnectorCreate() {
  struct MIDIConnector * connector = malloc( sizeof( struct MIDIConnector ) );
  if( connector != NULL ) {
    connector->refs = 1;
    connector->delegate = NULL;
    connector->target   = NULL;
  }
  return connector;
}

void MIDIConnectorDestroy( struct MIDIConnector * connector ) {
  MIDIConnectorDetach( connector );
  free( connector );
}

void MIDIConnectorRetain( struct MIDIConnector * connector ) {
  connector->refs++;
}

void MIDIConnectorRelease( struct MIDIConnector * connector ) {
  if( ! --connector->refs ) {
    MIDIConnectorDestroy( connector );
  }
}

static void _retain_target( struct MIDIConnector * connector ) {
  if( connector->target != NULL &&
      connector->delegate != NULL &&
      connector->delegate->retain != NULL )
    (connector->delegate->retain)( connector->target );
}

static void _release_target( struct MIDIConnector * connector ) {
  if( connector->target != NULL &&
      connector->delegate != NULL &&
      connector->delegate->release != NULL )
    (connector->delegate->release)( connector->target );
}

int MIDIConnectorDetach( struct MIDIConnector * connector ) {
  _release_target( connector );
  connector->target   = NULL;
  connector->delegate = NULL;
  return 0;
}

int MIDIConnectorAttachWithDelegate( struct MIDIConnector * connector, void * target, struct MIDIConnectorDelegate * delegate ) {
  _release_target( connector );
  connector->target   = target;
  connector->delegate = delegate;
  _retain_target( connector );
  return 0;
}

int MIDIConnectorAttachDevice( struct MIDIConnector * connector, struct MIDIDevice * device ) {
  return MIDIConnectorAttachWithDelegate( connector, device, &_device_delegate );
}

int MIDIConnectorAttachDriver( struct MIDIConnector * connector, struct MIDIDriver * driver ) {
  return MIDIConnectorAttachWithDelegate( connector, driver, &_driver_delegate );
}

int MIDIConnectorRelay( struct MIDIConnector * connector, struct MIDIMessage * message ) {
  if( connector->target == NULL ||
      connector->delegate == NULL ||
      connector->delegate->relay == NULL )
    return 1;
  return (connector->delegate->relay)( connector->target, message );
}
