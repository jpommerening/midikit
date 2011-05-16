#ifdef __APPLE__
#include <arpa/inet.h>

#include "midi.h"
#include "cfintegration.h"
#include "driver.h"

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

/* MARK: -
 * MARK: CFMIDIRunloop */

struct CFMIDIRunloop {
  int refs;
  CFRunLoopRef          runloop;
  CFRunLoopTimerContext timer_context;
  CFSocketContext       socket_context;
  size_t nsrc;
  CFRunLoopTimerRef  cfrlt;
  CFRunLoopSourceRef cfrls[1];
  struct MIDIRunloopDelegate delegate;
};

/* MARK: Core Foundation callbacks *//**
 * @name Core Foundation callbacks
 * @{
 */
 
static const void * _cf_retain_callback( const void * runloop ) {
  struct CFMIDIRunloop * cf_runloop = runloop;
  /* CFMIDIRunloopRetain( cf_runloop ); */
  return cf_runloop;
}

static void _cf_release_callback( const void * runloop ) {
  struct CFMIDIRunloop * cf_runloop = runloop;
  /* CFMIDIRunloopRelease( cf_runloop ); */
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

/**
 * @}
 * @endcond
 */

/* MARK: Runloop callbacks *//**
 * @name Runloop callbacks
 * @cond INTERNAL
 * Methods that are part of the runloop delegate to schedule and unschedule (clear)
 * actions.
 * @{
 */

static int _cf_runloop_schedule_read( void * info, int fd ) {
  struct CFMIDIRunloop * runloop = info;
  CFSocketRef socket;
  MIDIAssert( runloop != NULL );

  socket = CFSocketCreateWithNative( NULL, fd, kCFSocketReadCallBack, &_cf_socket_callback, &(runloop->socket_context) );
  CFSocketEnableCallBacks( socket, kCFSocketReadCallBack );
  CFRelease( socket );
  return 0;
}

static int _cf_runloop_clear_read( void * info, int fd ) {
  struct CFMIDIRunloop * runloop = info;
  CFSocketRef socket;
  MIDIAssert( runloop != NULL );

  socket = CFSocketCreateWithNative( NULL, fd, kCFSocketReadCallBack, &_cf_socket_callback, &(runloop->socket_context) );
  CFSocketDisableCallBacks( socket, kCFSocketReadCallBack );
  CFRelease( socket );
  return 0;
}

static int _cf_runloop_schedule_write( void * info, int fd ) {
  struct CFMIDIRunloop * runloop = info;
  CFSocketRef socket;
  MIDIAssert( runloop != NULL );

  socket = CFSocketCreateWithNative( NULL, fd, kCFSocketWriteCallBack, &_cf_socket_callback, &(runloop->socket_context) );
  CFSocketEnableCallBacks( socket, kCFSocketWriteCallBack );
  CFRelease( socket );
  return 0;
}

static int _cf_runloop_clear_write( void * info, int fd ) {
  struct CFMIDIRunloop * runloop = info;
  CFSocketRef socket;
  MIDIAssert( runloop != NULL );

  socket = CFSocketCreateWithNative( NULL, fd, kCFSocketWriteCallBack, &_cf_socket_callback, &(runloop->socket_context) );
  CFSocketDisableCallBacks( socket, kCFSocketWriteCallBack );
  CFRelease( socket );
  return 0;
}

static int _cf_runloop_schedule_timeout( void * info, struct timespec * timeout ) {
  struct CFMIDIRunloop * runloop = info;
  MIDIAssert( runloop != NULL );

  return 0;
}

static int _cf_runloop_clear_timeout( void * info ) {
  struct CFMIDIRunloop * runloop = info;
  MIDIAssert( runloop != NULL );

  return 0;
}

/**
 * @}
 * @endcond
 */

/* MARK: -
 * MARK: Creation and destruction *//**
 * @name Creation and destruction
 * Creating, destroying and reference counting of MIDIClock objects.
 * @{
 */

struct CFMIDIRunloop * CFMIDIRunloopCreate( CFRunLoopRef runloop ) {
  struct CFMIDIRunloop * cf_runloop = malloc( sizeof( struct CFMIDIRunloop ) );
  MIDIPrecondReturn( cf_runloop != NULL, ENOMEM, NULL );
  
  cf_runloop->refs    = 1;
  cf_runloop->runloop = runloop;
  
  cf_runloop->timer_context.version = 0;
  cf_runloop->timer_context.info = cf_runloop;
  cf_runloop->timer_context.release = &_cf_release_callback;
  cf_runloop->timer_context.retain  = &_cf_retain_callback;
  cf_runloop->timer_context.copyDescription = NULL;

  cf_runloop->socket_context.version = 0;
  cf_runloop->socket_context.info = cf_runloop;
  cf_runloop->socket_context.release = &_cf_release_callback;
  cf_runloop->socket_context.retain  = &_cf_retain_callback;
  cf_runloop->socket_context.copyDescription = NULL;

  cf_runloop->delegate.info = cf_runloop;
  cf_runloop->delegate.schedule_read    = &_cf_runloop_schedule_read;
  cf_runloop->delegate.schedule_write   = &_cf_runloop_schedule_write;
  cf_runloop->delegate.schedule_timeout = &_cf_runloop_schedule_timeout;
  cf_runloop->delegate.clear_read    = &_cf_runloop_clear_read;
  cf_runloop->delegate.clear_write   = &_cf_runloop_clear_write;
  cf_runloop->delegate.clear_timeout = &_cf_runloop_clear_timeout;
  return cf_runloop;
}

/**
 * @brief Create a MIDIRunloop instance that uses a CFRunLoop as implementation.
 * Create a new CFMIDIRunloop instance and create a MIDIRunloop with the newly created
 * delegate.
 * @public @memberof CFMIDIRunloop
 * @param runloop The core foundation runloop to use as implementation.
 * @return a pointer to the created runloop structure on success.
 * @return a @c NULL pointer if the runloop could not created.
 */
struct MIDIRunloop * MIDIRunloopCreateWithCFRunloop( CFRunLoopRef runloop ) {
  struct MIDIRunloopDelegate * delegate;
  struct CFMIDIRunloop       * cf_runloop;
  cf_runloop = CFMIDIRunloopCreate( runloop );
  if( cf_runloop == NULL ) return NULL;
  return MIDIRunloopCreate( &(cf_runloop->delegate) );
}

void CFMIDIRunloopDestroy( struct CFMIDIRunloop * cf_runloop ) {
  MIDIPrecondReturn( cf_runloop, EFAULT, (void)0 );
  free( cf_runloop );
}

/**
 * @brief Retain a CFMIDIRunloop instance.
 * Increment the reference counter of a runloop so that it won't be destroyed.
 * @public @memberof CFMIDIRunloop
 * @param cf_runloop The core foundation runloop.
 */
void CFMIDIRunloopRetain( struct CFMIDIRunloop * cf_runloop ) {
  MIDIPrecondReturn( cf_runloop != NULL, EFAULT, (void)0 );
  cf_runloop->refs++;
}

/**
 * @brief Release a CFMIDIRunloop instance.
 * Decrement the reference counter of a runloop. If the reference count
 * reached zero, destroy the runloop.
 * @public @memberof CFMIDIRunloop
 * @param cf_runloop The core foundation runloop.
 */
void CFMIDIRunloopRelease( struct CFMIDIRunloop * cf_runloop ) {
  MIDIPrecondReturn( cf_runloop != NULL, EFAULT, (void)0 );
  if( ! --cf_runloop->refs ) {
    CFMIDIRunloopDestroy( cf_runloop );
  }
}

/** @} */

/* MARK: end */

static int _cf_source_schedule( struct MIDIRunloopSource * source, int event ) {
  int i, created;
  struct CFMIDIRunLoopSource * cf = source->scheduler;
  CFSocketContext socket_context;
  CFOptionFlags   socket_cb_types = kCFSocketNoCallBack;
  CFSocketRef     socket;

  if( event & MIDI_RUNLOOP_READ )  socket_cb_types |= kCFSocketReadCallBack;
  if( event & MIDI_RUNLOOP_WRITE ) socket_cb_types |= kCFSocketWriteCallBack;

  socket_context.version = 0;
  socket_context.info = source;
  socket_context.release = NULL;
  socket_context.retain  = NULL;
  socket_context.copyDescription = NULL;

  for( i=0; i<source->nfds; i++ ) {
    created = 0;
    if( FD_ISSET(i, &(source->readfds)) ) {
      created = 1;
    }
    if( FD_ISSET(i, &(source->writefds)) ) {
      created = 1;
    }
    if( created ) {
      socket = CFSocketCreateWithNative( NULL, i, socket_cb_types, &_cf_socket_callback, &socket_context );
      if( event & MIDI_RUNLOOP_INVALIDATE ) {
        CFSocketInvalidate( socket );
      } else if( socket_cb_types != kCFSocketNoCallBack ) {
        CFSocketEnableCallBacks( socket, socket_cb_types );
      }
      CFRelease( socket );
    }
  }
  return 0;
}

struct CFMIDIRunLoopSource {
  size_t refs;
  struct MIDIRunloopSource * source;
  size_t length;
  CFRunLoopTimerRef  cfrlt;
  CFRunLoopSourceRef cfrls[1];
};

struct CFMIDIRunLoopSource * CFMIDIRunLoopSourceCreate( struct MIDIRunloopSource * source ) {
  int i, create;
  CFRunLoopTimerContext timer_context;
  CFSocketContext       socket_context;
  CFOptionFlags         socket_cb_types;
  CFSocketRef           socket;
  struct CFMIDIRunLoopSource * cf = malloc( sizeof(struct CFMIDIRunLoopSource)
                                          + sizeof(CFRunLoopSourceRef)*(source->nfds-1) );
  MIDIPrecondReturn( cf != NULL, ENOMEM, NULL );
  
  timer_context.version = 0;
  timer_context.info = source;
  timer_context.release = NULL;
  timer_context.retain  = NULL;
  timer_context.copyDescription = NULL;

  socket_context.version = 0;
  socket_context.info = source;
  socket_context.release = NULL;
  socket_context.retain  = NULL;
  socket_context.copyDescription = NULL;
  
  cf->length = source->nfds;

  if( source->timeout.tv_sec > 0 || source->timeout.tv_nsec > 0 ) {
    cf->cfrlt = CFRunLoopTimerCreate( NULL, CFAbsoluteTimeGetCurrent(),
      (double) source->timeout.tv_sec + 0.000000001 * (double) source->timeout.tv_nsec,
      0, 1, &_cf_timer_callback, &timer_context );
  } else {
    cf->cfrlt = NULL;
  }

  for( i=0; i<cf->length; i++ ) {
    create = 0;
    socket_cb_types = kCFSocketNoCallBack;
    if( FD_ISSET(i, &(source->readfds)) ) {
      socket_cb_types |= kCFSocketReadCallBack;
      create = 1;
    }
    if( FD_ISSET(i, &(source->writefds)) ) {
      socket_cb_types |= kCFSocketWriteCallBack;
      create = 1;
    }
    if( create ) {
      socket       = CFSocketCreateWithNative( NULL, i, socket_cb_types, &_cf_socket_callback, &socket_context );
      cf->cfrls[i] = CFSocketCreateRunLoopSource( NULL, socket, 1 );
      CFRelease( socket );
    } else {
      cf->cfrls[i] = NULL;
    }
  }
  return cf;
}

/**
 * @brief Destroy a CFMIDIRunLoopSource instance.
 * Free all resources occupied by the runloop soruce and release all referenced objects.
 * @public @memberof CFMIDIRunLoopSource
 * @param source The runloop source.
 */
void CFMIDIRunLoopSourceDestroy( struct CFMIDIRunLoopSource * source ) {
  int i;
  MIDIPrecondReturn( source != NULL, EFAULT, (void)0 );
  if( source->cfrlt != NULL ) {
    CFRunLoopTimerInvalidate( source->cfrlt );
    CFRelease( source->cfrlt );
  }
  for( i=0; i<source->length; i++ ) {
    if( source->cfrls[i] != NULL ) {
      CFRunLoopSourceInvalidate( source->cfrls[i] );
      CFRelease( source->cfrls[i] );
    }
  }
  free( source );
}

/**
 * @brief Retain a CFMIDIRunLoopSource instance.
 * Increment the reference counter of a clock so that it won't be destroyed.
 * @public @memberof CFMIDIRunLoopSource
 * @param source The runloop source.
 */
void CFMIDIRunLoopSourceRetain( struct CFMIDIRunLoopSource * source ) {
  MIDIPrecondReturn( source != NULL, EFAULT, (void)0 );
  source->refs++;
}

/**
 * @brief Release a CFMIDIRunLoopSource instance.
 * Decrement the reference counter of a clock. If the reference count
 * reached zero, destroy the clock.
 * @public @memberof CFMIDIRunLoopSource
 * @param source The runloop source.
 */
void CFMIDIRunLoopSourceRelease( struct CFMIDIRunLoopSource * source ) {
  MIDIPrecondReturn( source != NULL, EFAULT, (void)0 );
  if( ! --source->refs ) {
    CFMIDIRunLoopSourceDestroy( source );
  }
}

void CFRunLoopAddMIDIRunLoopSource( CFRunLoopRef rl, struct CFMIDIRunLoopSource * source, CFStringRef mode ) {
  int i;
  if( source->cfrlt != NULL ) CFRunLoopAddTimer( rl, source->cfrlt, mode );
  for( i=0; i<source->length; i++ ) {
    if( source->cfrls[i] != NULL ) CFRunLoopAddSource( rl, source->cfrls[i], mode );
  }
}

void CFRunLoopRemoveMIDIRunLoopSource( CFRunLoopRef rl, struct CFMIDIRunLoopSource * source, CFStringRef mode ) {
  int i;
  if( source->cfrlt != NULL ) CFRunLoopRemoveTimer( rl, source->cfrlt, mode );
  for( i=0; i<source->length; i++ ) {
    if( source->cfrls[i] != NULL ) CFRunLoopRemoveSource( rl, source->cfrls[i], mode );
  }
}

#endif
