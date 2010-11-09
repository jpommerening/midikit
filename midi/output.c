#include <stdlib.h>
#include "output.h"
#include "device.h"
#include "driver.h"

struct MIDIOutput * MIDIOutputCreate( struct MIDIDevice * device, struct MIDIDriver * driver ) {
  struct MIDIOutput * output = malloc( sizeof( struct MIDIOutput ) );
  if( output != NULL ) {
    output->refs = 1;
    output->device = device;
    output->driver = driver;
  }
  return output;
}

void MIDIOutputDestroy( struct MIDIOutput * output ) {
  MIDIOutputDisconnect( output );
  free( output );
}

void MIDIOutputRetain( struct MIDIOutput * output ) {
  output->refs++;
}

void MIDIOutputRelease( struct MIDIOutput * output ) {
  if( ! --output->refs ) {
    MIDIOutputDestroy( output );
  }
}

int MIDIOutputConnect( struct MIDIOutput * output ) {
  return MIDIDriverAddSender( output->driver, output );
}

int MIDIOutputDisconnect( struct MIDIOutput * output ) {
  return MIDIDriverRemoveSender( output->driver, output );
}

int MIDIOutputSend( struct MIDIOutput * output, struct MIDIMessage * message ) {
  return MIDIDriverSend( output->driver, message );
}
