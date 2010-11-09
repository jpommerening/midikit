#include <stdlib.h>
#include "input.h"
#include "device.h"
#include "driver.h"

struct MIDIInput * MIDIInputCreate( struct MIDIDevice * device, struct MIDIDriver * driver ) {
  struct MIDIInput * input = malloc( sizeof( struct MIDIInput ) );
  if( input != NULL ) {
    input->refs = 1;
    input->device = device;
    input->driver = driver;
  }
  return input;
}

void MIDIInputDestroy( struct MIDIInput * input ) {
  MIDIInputDisconnect( input );
  free( input );
}

void MIDIInputRetain( struct MIDIInput * input ) {
  input->refs++;
}

void MIDIInputRelease( struct MIDIInput * input ) {
  if( ! --input->refs ) {
    MIDIInputDestroy( input );
  }
}

int MIDIInputConnect( struct MIDIInput * input ) {
  return MIDIDriverAddReceiver( input->driver, input );
}

int MIDIInputDisconnect( struct MIDIInput * input ) {
  return MIDIDriverRemoveReceiver( input->driver, input );
}

int MIDIInputReceive( struct MIDIInput * input, struct MIDIMessage * message ) {
  return MIDIDeviceReceive( input->device, message );
}
