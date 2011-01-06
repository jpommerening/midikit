#include <stdio.h>
#include "midi/util.h"
#include "driver/applemidi/applemidi.h"

struct MIDIKeyboard {
  size_t refs;
  struct MIDIRunloop * runloop;
};

static int _keyboard_read_fds( void * kbd, int nfds, fd_set * fds ) {
  struct MIDIKeyboard * keyboard = kbd;
  int chr = getc( stdin );
  switch( chr ) {
    case 'q':
      printf( "quit\n" );
      if( keyboard->runloop != NULL ) {
        MIDIRunloopStop( keyboard->runloop );
      }
      break;
    default:
      printf( "%i\n", chr );
      break;
  }
  return 0;
}

struct MIDIKeyboard * MIDIKeyboardCreate() {
  struct MIDIKeyboard * keyboard = malloc( sizeof( struct MIDIKeyboard ) );
  keyboard->refs = 1;
  return keyboard;
}

void MIDIKeyboardRelease( struct MIDIKeyboard * keyboard ) {
  free( keyboard );
}

int MIDIKeyboardSetRunloop( struct MIDIKeyboard * keyboard, struct MIDIRunloop * runloop ) {
  keyboard->runloop = runloop;
  return 0;
}

int MIDIKeyboardCreateRunloopSource( struct MIDIKeyboard * keyboard, struct MIDIRunloopSource * source ) {
  source->nfds = 1; // stdin + 1
  FD_ZERO( &(source->readfds) );
  FD_ZERO( &(source->writefds) );
  FD_SET( 0, &(source->readfds) ); // stdin
  source->timeout.tv_sec = 0;
  source->timeout.tv_nsec = 0;
  source->read = &_keyboard_read_fds;
  source->write = NULL;
  source->idle = NULL;
  source->info = keyboard;
  return 0;
}

int main( int argc, char *argv[] ) {
  struct MIDIKeyboard * keyboard = MIDIKeyboardCreate();
  struct MIDIDriverAppleMIDI * applemidi = MIDIDriverAppleMIDICreate( "My AppleMIDI session", 5004 );
  struct MIDIRunloopSource keyboard_rls;
  struct MIDIRunloopSource driver_rls;
  struct MIDIRunloop * runloop = MIDIRunloopCreate();

  MIDIKeyboardCreateRunloopSource( keyboard, &keyboard_rls );
  MIDIDriverAppleMIDICreateRunloopSource( applemidi, &driver_rls );

  MIDIKeyboardSetRunloop( keyboard, runloop );

  MIDIRunloopAddSource( runloop, &keyboard_rls );
  MIDIRunloopAddSource( runloop, &driver_rls );

  MIDIRunloopStart( runloop );

  MIDIRunloopRelease( runloop );
  MIDIKeyboardRelease( keyboard );
  MIDIDriverAppleMIDIRelease( applemidi );

  return 0;
}
