#ifdef __APPLE__
#include <arpa/inet.h>

#include "cfintegration.h"
#include "driver.h"

CFRunLoopSourceRef MIDIDriverCreateRunloopSource( struct MIDIDriver * driver ) {
  return NULL;
}

static int _netservice_get_first_addr( CFNetServiceRef netService, socklen_t * size, struct sockaddr ** addr ) {
  CFArrayRef addresses;
  CFDataRef data;
  CFNetServiceCancel( netService );
  CFNetServiceResolveWithTimeout( netService, 0.1, NULL );
  addresses = CFNetServiceGetAddressing( netService );
  if( addresses == NULL || CFArrayGetCount( addresses ) < 1 ) {
    return 1;
  }
  data  = CFArrayGetValueAtIndex( addresses, 0 );
  *size = CFDataGetLength( data );
  *addr = CFDataGetBytePtr( data );
  
  return 0;
}

int MIDIDriverAppleMIDIAddPeerWithCFNetService( struct MIDIDriverAppleMIDI * driver, CFNetServiceRef netService ) {
  socklen_t size;
  struct sockaddr * addr;
  if( _netservice_get_first_addr( netService, &size, &addr ) ) return 1;
  
  return MIDIDriverAppleMIDIAddPeerWithSockaddr( driver, size, addr );
}

int MIDIDriverAppleMIDIRemovePeerWithCFNetService( struct MIDIDriverAppleMIDI * driver, CFNetServiceRef netService ) {
  socklen_t size;
  struct sockaddr * addr;
  if( _netservice_get_first_addr( netService, &size, &addr ) ) return 1;
  
  return MIDIDriverAppleMIDIRemovePeerWithSockaddr( driver, size, addr );
}

static void _cf_socket_callback( CFSocketRef s, CFSocketCallBackType callbackType, CFDataRef address, const void *data, void *info ) {
  struct MIDIRunloopSource * source = info;
  int i = CFSocketGetNative( s );
  fd_set fds;
  FD_ZERO(&fds);
  FD_SET(i, &fds);
  switch( callbackType ) {
    case kCFSocketReadCallBack:
      if( FD_ISSET(i, &(source->readfds)) && source->read != NULL ) {
        (*source->read)( source->info, source->nfds, &(fds) );
      }
      break;
    case kCFSocketWriteCallBack:
      if( FD_ISSET(i, &(source->writefds)) && source->write != NULL ) {
        (*source->write)( source->info, source->nfds, &(fds) );
      }
      break;
  }
}

static void _cf_timer_callback( CFRunLoopTimerRef timer, void *info ) {
  struct MIDIRunloopSource * source = info;
  struct timespec ts;
  source->remain.tv_sec = 0;
  source->remain.tv_nsec = 0;
  ts.tv_sec  = source->timeout.tv_sec;
  ts.tv_nsec = source->timeout.tv_nsec;
  if( source->idle != NULL ) {
    (*source->idle)( source->info, &ts );
  }
}

struct CFMIDIRunloopSource {
  struct MIDIRunloopSource * source;
  size_t length;
  CFRunLoopTimerRef  cfrlt;
  CFRunLoopSourceRef cfrls[1];
};

void CFRunLoopAddMIDIRunloopSource( CFRunLoopRef rl, struct MIDIRunloopSource * source, CFStringRef mode ) {
  int i, create;
  CFSocketRef socket;
  CFSocketContext socket_context;
  CFSocketCallBackType types = kCFSocketNoCallBack;
  CFRunLoopTimerRef timer;
  CFRunLoopTimerContext timer_context;
  CFRunLoopSourceRef runloopsource;

  socket_context.version = 0;
  socket_context.info = source;
  socket_context.release = NULL;
  socket_context.retain  = NULL;
  socket_context.copyDescription = NULL;
  
  timer_context.version = 0;
  timer_context.info = source;
  timer_context.release = NULL;
  timer_context.retain  = NULL;
  timer_context.copyDescription = NULL;
  
  if( source->timeout.tv_sec > 0 || source->timeout.tv_nsec > 0 ) {
    timer = CFRunLoopTimerCreate( NULL, CFAbsoluteTimeGetCurrent(),
      (double) source->timeout.tv_sec + 0.000000001 * (double) source->timeout.tv_nsec,
      0, 1, &_cf_timer_callback, &timer_context );
    CFRunLoopAddTimer( rl, timer, mode );
    CFRelease( timer );
  }

  for( i=0; i<source->nfds; i++ ) {
    create = 0;
    if( FD_ISSET(i, &(source->readfds)) ) {
      types |= kCFSocketReadCallBack;
      create = 1;
    }
    if( FD_ISSET(i, &(source->writefds)) ) {
      types |= kCFSocketWriteCallBack;
      create = 1;
    }
    if( create ) {
      socket = CFSocketCreateWithNative( NULL, i, types, &_cf_socket_callback, &socket_context );
      runloopsource = CFSocketCreateRunLoopSource( NULL, socket, 1 );
      CFRunLoopAddSource( rl, runloopsource, mode );
      CFRelease( socket );
      CFRelease( runloopsource );
    }
  }
}


#endif
