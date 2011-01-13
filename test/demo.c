#include <stdio.h>
#include <unistd.h>
#include "midi/util.h"
#include "driver/applemidi/applemidi.h"

struct MIDIKeyboard {
  size_t refs;
  struct MIDIRunloopSource runloop_source;
  struct MIDIRunloop * runloop;
};

static int _keyboard_read_fds( void * kbd, int nfds, fd_set * fds ) {
  struct MIDIKeyboard * keyboard = kbd;
  int chr = fgetc( stdin );
  switch( chr ) {
    case 'q':
      printf( "quit\n" );
      if( keyboard->runloop != NULL ) {
        MIDIRunloopStop( keyboard->runloop );
      }
      break;
    default:
      break;
  }
  return 0;
}

static int _keyboard_timeout( void * kbd, struct timespec * ts ) {
  printf( "kbd timeout %i.%06i s\n", (int) ts->tv_sec, (int) ts->tv_nsec );
  return 0;
}

int _keyboard_init_runloop_source( struct MIDIKeyboard * keyboard ) {
  struct MIDIRunloopSource * source = &(keyboard->runloop_source);
  
  source->nfds = 1; /* stdin + 1 */
  FD_ZERO( &(source->readfds) );
  FD_ZERO( &(source->writefds) );
  FD_SET( 0, &(source->readfds) ); /* stdin */
  source->timeout.tv_sec = 5;
  source->timeout.tv_nsec = 0;
  source->remain.tv_sec = 5;
  source->remain.tv_nsec = 0;
  source->read = &_keyboard_read_fds;
  source->write = NULL;
  source->idle = &_keyboard_timeout;
  source->info = keyboard;
  return 0;
}

struct MIDIKeyboard * MIDIKeyboardCreate() {
  struct MIDIKeyboard * keyboard = malloc( sizeof( struct MIDIKeyboard ) );
  keyboard->refs = 1;
  _keyboard_init_runloop_source( keyboard );
  return keyboard;
}

void MIDIKeyboardRelease( struct MIDIKeyboard * keyboard ) {
  free( keyboard );
}

int MIDIKeyboardSetRunloop( struct MIDIKeyboard * keyboard, struct MIDIRunloop * runloop ) {
  keyboard->runloop = runloop;
  return 0;
}

int MIDIKeyboardGetRunloopSource( struct MIDIKeyboard * keyboard, struct MIDIRunloopSource ** source ) {
  if( source == NULL ) return 1;
  *source = &(keyboard->runloop_source);
  return 0;
}

int main( int argc, char *argv[] ) {
  struct MIDIKeyboard * keyboard;
  struct MIDIDriverAppleMIDI * applemidi;
  struct MIDIRunloopSource * keyboard_rls;
  struct MIDIRunloopSource * driver_rls;
  struct MIDIRunloop * runloop;

  int i;
  char client_addr[16] = { '\0' };
  unsigned short client_port = 0;
  unsigned short port;

  if( argc > 4 || argc < 3 ) {
    printf( "Usage:\n  %s <session name> <session port> [ <client address>:<client port> ]\n", argv[0] );
    return 1;
  }

  sscanf( argv[2], "%hu", &port );

  if( argc == 4 ) {
    for( i=0; i<16 && argv[3][i] != '\0' && argv[3][i] != ':'; i++ ) {
      if( ( argv[3][i] >= '0' && argv[3][i] <= '9' ) || argv[3][i] == '.' ) {
        client_addr[i] = argv[3][i];
      } else {
        fprintf( stderr, "Unexpected character '%c' in address.\n", argv[3][i] );
        return 1;
      }
    }
    if( argv[3][i] == ':' ) {
      i++;
      sscanf( argv[3]+i, "%hu", &client_port );
    } else {
      fprintf( stderr, "Missing client port.\n" );
      return 1;
    }
  }

  printf( "Starting session '%s' on port %hu.\n", argv[1], port );
  keyboard  = MIDIKeyboardCreate();
  applemidi = MIDIDriverAppleMIDICreate( NULL, argv[1], port );
  runloop   = MIDIRunloopCreate();

  if( client_addr[0] != '\0' && client_port != 0 ) {
    printf( "Connecting to client %s:%hu.\n", client_addr, client_port );
    MIDIDriverAppleMIDIAddPeer( applemidi, client_addr, client_port );
  } else {
    MIDIDriverAppleMIDIAcceptFromAny( applemidi );
  }

  MIDIKeyboardGetRunloopSource( keyboard, &keyboard_rls );
  MIDIDriverAppleMIDIGetRunloopSource( applemidi, &driver_rls );

  MIDIKeyboardSetRunloop( keyboard, runloop );

  MIDIRunloopAddSource( runloop, keyboard_rls );
  MIDIRunloopAddSource( runloop, driver_rls );

  MIDIRunloopStart( runloop );

  MIDIRunloopRelease( runloop );
  MIDIKeyboardRelease( keyboard );
  MIDIDriverAppleMIDIRelease( applemidi );

  return 0;
}
