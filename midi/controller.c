#include <stdlib.h>
#include "midi.h"
#include "controller.h"

struct MIDIController {
  size_t refs;
  struct MIDIControllerDelegate * delegate;
  MIDILongValue lv_controls[32];
  MIDIValue     sv_controls[32];
  MIDIBoolean   b_controls[8];
};

struct MIDIController * MIDIControllerCreate( struct MIDIControllerDelegate * delegate ) {
  struct MIDIController * controller = malloc( sizeof( struct MIDIController ) );
  controller->refs = 1;
  controller->delegate = delegate;
  return controller;
}

void MIDIControllerDestroy( struct MIDIController * controller ) {
  free( controller );
}

void MIDIControllerRetain( struct MIDIController * controller ) {
  controller->refs++;
}

void MIDIControllerRelease( struct MIDIController * controller ) {
  if( ! --controller->refs ) {
    MIDIControllerDestroy( controller );
  }
}

int MIDIControllerReceiveControlChange( struct MIDIController * controller, MIDIChannel channel,
                                        MIDIControl control, MIDIValue value ) {
  return 0;
}

int MIDIControllerSendControlChange( struct MIDIController * controller, MIDIChannel channel,
                                     MIDIControl control, MIDIValue value ) {
  return 0;
}
