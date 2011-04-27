#include <stdlib.h>
#include <sys/select.h>
#include <sys/time.h>
#include <time.h>
#include "runloop.h"
#include "midi.h"

#define CURRENT_RUNLOOP( rl ) do { _current_runloop = (rl); } while(0)

#define MAX_RUNLOOP_SOURCES 16

static struct MIDIRunloop * _current_runloop = NULL;

struct MIDIRunloopSource {
  int    refs;
  int    nfds;
  fd_set readfds;
  fd_set writefds;
  struct timespec timeout_start;
  struct timespec timeout_time;
  struct MIDIRunloopSourceDelegate delegate;
  struct MIDIRunloop * runloop;
};

struct MIDIRunloop {
  int    refs;
  int    active;
  struct MIDIRunloopDelegate delegate;
  struct MIDIRunloopSource   master;
  struct MIDIRunloopSource * sources[MAX_RUNLOOP_SOURCES];
};

static int _fds_cmp( fd_set * a, fd_set * b, int nfds ) {
  int fd;
  for( fd=0; fd<nfds; fd++ ) {
    if( FD_ISSET( fd, a ) != FD_ISSET( fd, b ) ) {
      return 1;
    }
  }
  return 0;
}

static int _fds_check2( fd_set * fds, fd_set * chk, int nfds ) {
  int fd;
  for( fd=0; fd<nfds; fd++ ) {
    if( FD_ISSET( fd, chk ) && FD_ISSET( fd, fds ) ) {
      return 1;
    }
  }
  return 0;
}

static int _fds_check( fd_set * fds, int nfds ) {
  fd_set empty;
  FD_ZERO( &empty );
  return _fds_cmp( fds, &empty, nfds );
}

static int _fds_add( fd_set * lhs, fd_set * rhs, int nfds ) {
  int fd;
  for( fd=0; fd<nfds; fd++ ) {
    if( FD_ISSET( fd, rhs ) ) {
      FD_SET( fd, lhs );
    }
  }
  return 0;
}

static int _fds_cpy( fd_set * lhs, fd_set * rhs, int nfds ) {
  FD_ZERO( lhs );
  return _fds_add( lhs, rhs, nfds );
}

static int _fds_sub( fd_set * lhs, fd_set * rhs, int nfds ) {
  int fd;
  for( fd=0; fd<nfds; fd++ ) {
    if( FD_ISSET( fd, rhs ) ) {
      FD_CLR( fd, lhs );
    }
  }
  return 0;
}

static void _timespec_zero( struct timespec * ts ) {
  ts->tv_sec  = 0;
  ts->tv_nsec = 0;
}

static int _timespec_empty( struct timespec * ts ) {
  return ts->tv_sec == 0 && ts->tv_nsec == 0;
}

static void _timespec_cpy( struct timespec * lhs, struct timespec * rhs ) {
  if( lhs != rhs ) {
    lhs->tv_sec  = rhs->tv_sec;
    lhs->tv_nsec = rhs->tv_nsec;
  }
}

static int _timespec_cmp( struct timespec * lhs, struct timespec * rhs ) {
  if( lhs->tv_sec > rhs->tv_sec ) {
    return 2;
  } else if( lhs->tv_sec == rhs->tv_sec ) {
    if( lhs->tv_nsec > rhs->tv_nsec ) {
      return 1;
    } else if( lhs->tv_nsec == rhs->tv_nsec ) {
      return 0;
    } else {
      return -1;
    }
  } else {
    return -2;
  }
}

static void _timespec_add( struct timespec * lhs, struct timespec * rhs ) {
  lhs->tv_sec  += rhs->tv_sec;
  lhs->tv_nsec += rhs->tv_nsec;
  while( lhs->tv_nsec > 1000000000 ) {
    lhs->tv_sec  += 1;
    lhs->tv_nsec -= 1000000000;
  }
}

static void _timespec_sub( struct timespec * lhs, struct timespec * rhs ) {
  MIDILog( DEVELOP, "%ld.%06lds - %ld.%06lds\n",
    lhs->tv_sec, lhs->tv_nsec,
    rhs->tv_sec, rhs->tv_nsec );

  lhs->tv_sec  -= rhs->tv_sec;
  lhs->tv_nsec -= rhs->tv_nsec;
  if( lhs->tv_nsec >= 1000000000 ) {
    lhs->tv_sec  += lhs->tv_nsec / 1000000000;
    lhs->tv_nsec %= 1000000000;
  } else {
    while( lhs->tv_nsec < 0 ) {
      lhs->tv_sec  -= 1;
      lhs->tv_nsec += 1000000000;
    }
  }
  MIDILog( DEVELOP, "=> %ld.%06lds\n", lhs->tv_sec, lhs->tv_nsec );
}

static void _timespec_now( struct timespec * ts ) {
  struct timeval  tv = { 0, 0 };
  gettimeofday( &tv, NULL );
  ts->tv_sec  = tv.tv_sec;
  ts->tv_nsec = tv.tv_usec * 1000;
}

static void _timeval_from_timespec( struct timeval * tv, struct timespec * ts ) {
  tv->tv_sec  = ts->tv_sec;
  tv->tv_usec = ts->tv_nsec / 1000;
}

struct MIDIRunloopSource * MIDIRunloopSourceCreate( struct MIDIRunloopSourceDelegate * delegate ) {
  struct MIDIRunloopSource * source = malloc( sizeof( struct MIDIRunloopSource ) );

  source->refs = 1;
  source->nfds = 0;

  FD_ZERO( &(source->readfds) );
  FD_ZERO( &(source->writefds) );

  _timespec_zero( &(source->timeout_start) );
  _timespec_zero( &(source->timeout_time) );

  if( delegate != NULL ) {
    source->delegate.info    = delegate->info;
    source->delegate.read    = delegate->read;
    source->delegate.write   = delegate->write;
    source->delegate.timeout = delegate->timeout;
  } else {
    source->delegate.info    = NULL;
    source->delegate.read    = NULL;
    source->delegate.write   = NULL;
    source->delegate.timeout = NULL;
  }
  source->runloop = NULL;

  return source;
}

void MIDIRunloopSourceDestroy( struct MIDIRunloopSource * source ) {
  free( source );
}

void MIDIRunloopSourceRetain( struct MIDIRunloopSource * source ) {
  MIDIPrecondReturn( source != NULL, EFAULT, (void)0 );
  source->refs++;
}

void MIDIRunloopSourceRelease( struct MIDIRunloopSource * source ) {
  MIDIPrecondReturn( source != NULL, EFAULT, (void)0 );
  if( ! --source->refs ) {
    MIDIRunloopSourceDestroy( source );
  }
}

/**
 * @brief Mark a runloop source as invalid.
 * Disable all callbacks of a runloop source and remove it from the runloop it is
 * scheduled in.
 * @public @memberof MIDIRunloopSource
 * @param source The source the should be invalidated.
 */
int MIDIRunloopSourceInvalidate( struct MIDIRunloopSource * source ) {
  MIDIPrecond( source != NULL, EFAULT );

  source->delegate.info    = NULL;
  source->delegate.read    = NULL;
  source->delegate.write   = NULL;
  source->delegate.timeout = NULL;
  
  if( source->runloop != NULL ) {
    return MIDIRunloopRemoveSource( source->runloop, source );
  } else {
    return 0;
  }
}

/**
 * @brief Start a new timeout.
 * @private @memberof MIDIRunloopSource
 * @param source The runloop source.
 * @param now    Must be set to the current time.
 */
static void _runloop_source_timeout_start( struct MIDIRunloopSource * source, struct timespec * now ) {
  _timespec_cpy( &(source->timeout_start), now );
}

/**
 * @brief Get the remaining time of the current timeout.
 * Zero if timeout is now set.
 * @private @memberof MIDIRunloopSource
 * @param source The runloop source.
 * @param remain Will be set to the remaining time.
 * @param now    Must be set to the current time.
 */
static void _runloop_source_timeout_remain( struct MIDIRunloopSource * source, struct timespec * remain, struct timespec * now ) {
  if( _timespec_empty( &(source->timeout_time) ) ) {
    _timespec_zero( remain );
  } else {
    _timespec_cpy( remain, now );
    _timespec_add( remain, &(source->timeout_time) );
    _timespec_sub( remain, &(source->timeout_start) );
  }
}

/**
 * @brief Check if the current timeout is over.
 * @private @memberof MIDIRunloopSource
 * @param source The runloop source.
 * @param now    Must be set to the current time.
 */
static int _runloop_source_timeout_check( struct MIDIRunloopSource * source, struct timespec * now ) {
  struct timespec ts; /* timeout limit */
  if( _timespec_empty( &(source->timeout_time) ) ) return 0; /* no timeout set */
  _timespec_cpy( &ts, &(source->timeout_start) );
  _timespec_add( &ts, &(source->timeout_time)  );
  return _timespec_cmp( now, &ts ) > 0; /* is "now" later/greater than "ts" ? */
}

/**
 * @brief Trigger a timeout.
 * @private @memberof MIDIRunloopSource
 * @param source The runloop source.
 * @param now    Must be set to the current time.
 */
static int _runloop_source_timeout( struct MIDIRunloopSource * source, struct timespec * now ) {
  if( source->delegate.info == NULL || source->delegate.timeout == NULL ) return 0;
  MIDIPrecond( source->delegate.info != NULL, EINVAL );
  _runloop_source_timeout_start( source, now );
  return (source->delegate.timeout)( source->delegate.info, now );
}

/**
 * @brief Trigger a read operation.
 * @private @memberof MIDIRunloopSource
 * @param source The runloop source.
 * @param now    Must be set to the current time.
 * @param fds    The fd_set from which to read.
 */
static int _runloop_source_read( struct MIDIRunloopSource * source, struct timespec * now, fd_set * fds ) {
  if( source->delegate.info == NULL || source->delegate.read == NULL ) return 0;
  if( _fds_check( fds, source->nfds ) ) {
    _runloop_source_timeout_start( source, now );
    return (source->delegate.read)( source->delegate.info, source->nfds, fds );
  } else {
    return 0;
  }
}

/**
 * @brief Trigger a write operation.
 * @private @memberof MIDIRunloopSource
 * @param source The runloop source.
 * @param now    Must be set to the current time.
 * @param fds    The fd_set to which to write.
 */
static int _runloop_source_write( struct MIDIRunloopSource * source, struct timespec * now, fd_set * fds ) {
  if( source->delegate.info == NULL || source->delegate.write == NULL ) return 0;
  if( _fds_check( fds, source->nfds ) ) {
    _fds_sub( &(source->writefds), fds, source->nfds );
    _runloop_source_timeout_start( source, now );
    return (source->delegate.write)( source->delegate.info, source->nfds, fds );
  }
  return 0;
}

/**
 * @brief Wait until any callback of the runloop source is triggered.
 * If no callbacks are scheduled return immediately.
 * @public @memberof MIDIRunloopSource
 * @param source The runloop source.
 */
int MIDIRunloopSourceWait( struct MIDIRunloopSource * source ) {
  int result = 0;
  struct timespec now, remain;
  struct timeval  remain_tv = { 0, 0 };
  fd_set readfds;
  fd_set writefds;

  /*printf( "RunloopSourceWait\n" );*/
  _timespec_now( &now );
  if( _runloop_source_timeout_check( source, &now ) ) {
    /* timed out before check */
    /*printf( "- timeout(sec:%li,nsec:%li)\n", source->timeout_time.tv_sec, source->timeout_time.tv_nsec );*/
    return _runloop_source_timeout( source, &now );
  } else if( source->nfds > 0 ) {
    /* select */
    _runloop_source_timeout_remain( source, &remain, &now );
    _timeval_from_timespec( &remain_tv, &remain );
    _fds_cpy( &readfds, &(source->readfds), source->nfds );
    _fds_cpy( &writefds, &(source->writefds), source->nfds );

    /*printf( "- select(nfds:%i)\n", source->nfds );*/
    result = select( source->nfds, &readfds, &writefds, NULL, &remain_tv );
    _timespec_now( &now );
    if( result > 0 ) {
      /*printf( "- read/write\n" );*/
      return _runloop_source_read( source, &now, &readfds )
           + _runloop_source_write( source, &now, &writefds );
    } else {
      /*printf( "- timeout\n" );*/
      return _runloop_source_timeout( source, &now );
    }
  } else if( ! _timespec_empty( &(source->timeout_time) ) ) {
    /* nanosleep */
    /*printf( "- sleep\n" );*/
    _runloop_source_timeout_remain( source, &remain, &now );
    result = nanosleep( &remain, NULL );
    _timespec_now( &now );
    /*printf( "- timeout\n" );*/
    return _runloop_source_timeout( source, &now );
  }
  return 0;
}

static int _runloop_schedule_read( struct MIDIRunloop * runloop, int fd );
static int _runloop_schedule_write( struct MIDIRunloop * runloop, int fd );
static int _runloop_schedule_timeout( struct MIDIRunloop * runloop, struct timespec * ts );
static int _runloop_clear_read( struct MIDIRunloop * runloop, int fd );
static int _runloop_clear_write( struct MIDIRunloop * runloop, int fd );

/**
 * @brief Schedule the read callback of a runloop source.
 * Enable the read callback of a runloop source. The callback will be invoked the
 * next time any of the runloop source's "read" file descriptors have new data to
 * be read. The read callback will stay enabled until it is cleared using
 * MIDIRunloopSourceClearRead.
 * @public @memberof MIDIRunloopSource
 * @param source The source that should be scheduled.
 * @param fd     The file descriptor from which to read.
 */
int MIDIRunloopSourceScheduleRead( struct MIDIRunloopSource * source, int fd ) {
  MIDIPrecond( source != NULL, EFAULT );
  FD_SET( fd, &(source->readfds) );
  if( source->nfds <= fd ) source->nfds = fd + 1;
  if( source->runloop != NULL ) {
    return _runloop_schedule_read( source->runloop, fd );
  } else {
    return 0;
  }
}

/**
 * @brief Stop a scheduled read callback of a runloop source.
 * Unschedule a previously scheduled read operation.
 * @public @memberof MIDIRunloopSource
 * @param source The source that should be scheduled.
 * @param fd     The file descriptor from which to read.
 */
int MIDIRunloopSourceClearRead( struct MIDIRunloopSource * source, int fd ) {
  MIDIPrecond( source != NULL, EFAULT );
  FD_CLR( fd, &(source->readfds) );
  if( source->runloop != NULL ) {
    return _runloop_clear_read( source->runloop, fd );
  } else {
    return 0;
  }
}

/**
 * @brief Schedule the write callback of a runloop source.
 * Enable the write callback of a runloop source. The callback will be invoked the
 * next time any of the runloop source's "write" file descriptors can accept new
 * data. After that the callback will be disabled until this function is called
 * again.
 * @public @memberof MIDIRunloopSource
 * @param source The source that should be scheduled.
 * @param fd     The file descriptor to which to write.
 */
int MIDIRunloopSourceScheduleWrite( struct MIDIRunloopSource * source, int fd ) {
  MIDIPrecond( source != NULL, EFAULT );
  FD_SET( fd, &(source->writefds) );
  if( source->nfds <= fd ) source->nfds = fd + 1;
  if( source->runloop != NULL ) {
    return _runloop_schedule_write( source->runloop, fd );
  } else {
    return 0;
  }
}

/**
 * @brief Stop a scheduled write callback of a runloop source.
 * Unschedule a previously scheduled write operation.
 * @public @memberof MIDIRunloopSource
 * @param source  The source that should be scheduled.
 * @param timeout The timeout to be scheduled.
 * @param fd     The file descriptor to which to write.
 */
int MIDIRunloopSourceClearWrite( struct MIDIRunloopSource * source, int fd ) {
  MIDIPrecond( source != NULL, EFAULT );
  FD_CLR( fd, &(source->writefds) );
  if( source->runloop != NULL ) {
    return _runloop_clear_write( source->runloop, fd );
  } else {
    return 0;
  }
}

/**
 * @brief Schedule the idle callback of a runloop source.
 * Enable the idle callback of a runloop source. The callback will be invoked the
 * next time any of the runloop source's timeout expired. After that the callback
 * will be disabled until this function is called again.
 * @public @memberof MIDIRunloopSource
 * @param source  The source that should be scheduled.
 * @param timeout The timeout to be scheduled.
 */
int MIDIRunloopSourceScheduleTimeout( struct MIDIRunloopSource * source, struct timespec * timeout ) {
  MIDIPrecond( source != NULL, EFAULT );
  MIDIPrecond( timeout != NULL, EINVAL );
  _timespec_now( &(source->timeout_start) );
  _timespec_cpy( &(source->timeout_time), timeout );
  if( source->timeout_time.tv_sec == 0 && source->timeout_time.tv_nsec == 0 ) {
    /* We use timeout == 0 to signal that no timeout was scheduled.
     * Use a minimal timeout instead. */
    source->timeout_time.tv_nsec = 1;
  }
  if( source->runloop != NULL ) {
    return _runloop_schedule_timeout( source->runloop, timeout );
  } else {
    return 0;
  }
}

/**
 * @brief Stop a scheduled write callback of a runloop source.
 * Unschedule a previously scheduled write operation.
 * @public @memberof MIDIRunloopSource
 * @param source The source that should be scheduled.
 */
int MIDIRunloopSourceClearTimeout( struct MIDIRunloopSource * source ) {
  MIDIPrecond( source != NULL, EFAULT );
  _timespec_zero( &(source->timeout_time) );
  return 0;
}

static int _runloop_master_read( void * rl, int nfds, fd_set * readfds ) {
  int i, result = 0, cb = 0;
  struct MIDIRunloop * runloop = rl;
  struct MIDIRunloopSource * source;
  struct timespec now;
  
  CURRENT_RUNLOOP( runloop );

  /* _timespec_now( &now ); */
  _timespec_cpy( &now, &(runloop->master.timeout_start) );
  
  for( i=0; i<MAX_RUNLOOP_SOURCES; i++ ) {
    source = runloop->sources[i];
    if( source == NULL ) continue;

    if( source->delegate.read != NULL ) {
      cb++;
      if( _fds_check2( readfds, &(source->readfds), source->nfds ) ) {
        result += _runloop_source_read( source, &now, readfds );
      } else if( _runloop_source_timeout_check( source, &now ) ) {
        result += _runloop_source_timeout( source, &now );
      }
    } else {
      if( _runloop_source_timeout_check( source, &now ) ) {
        result += _runloop_source_timeout( source, &now );
      }
    }
  }
  if( cb == 0 ) {
    runloop->master.delegate.read = NULL;
  }
  return result;
}

static int _runloop_master_write( void * rl, int nfds, fd_set * writefds ) {
  int i, result = 0, cb = 0;
  struct MIDIRunloop * runloop = rl;
  struct MIDIRunloopSource * source;  
  struct timespec now;
  
  CURRENT_RUNLOOP( runloop );

  /* _timespec_now( &now ); */
  _timespec_cpy( &now, &(runloop->master.timeout_start) );
  
  for( i=0; i<MAX_RUNLOOP_SOURCES; i++ ) {
    source = runloop->sources[i];
    if( source == NULL ) continue;

    if( source->delegate.write != NULL ) {
      cb++;
      if( _fds_check2( writefds, &(source->writefds), source->nfds ) ) {
        result += _runloop_source_write( source, &now, writefds );
      } else if( _runloop_source_timeout_check( source, &now ) ) {
        result += _runloop_source_timeout( source, &now );
      }
    } else {
      if( _runloop_source_timeout_check( source, &now ) ) {
        result += _runloop_source_timeout( source, &now );
      }
    }
  }
  if( cb == 0 ) {
    runloop->master.delegate.write = NULL;
  }
  return result;
}

static int _runloop_master_timeout( void * rl, struct timespec * ts ) {
  int i, result = 0;
  struct MIDIRunloop * runloop = rl;
  struct MIDIRunloopSource * source;
  struct timespec now;
  
  CURRENT_RUNLOOP( runloop );

  /* _timespec_now( &now ); */
  _timespec_cpy( &now, &(runloop->master.timeout_start) );

  for( i=0; i<MAX_RUNLOOP_SOURCES; i++ ) {
    source = runloop->sources[i];
    if( source == NULL ) continue;

    if( _runloop_source_timeout_check( source, &now ) ) {
      result += _runloop_source_timeout( source, &now );
    }
    
    if( source->delegate.read != NULL ) {
      runloop->master.delegate.read  = &_runloop_master_read;
    }
    if( source->delegate.write != NULL ) {
      runloop->master.delegate.write = &_runloop_master_write;
    }
  }
  return result;
}

struct MIDIRunloop * MIDIRunloopCreate( struct MIDIRunloopDelegate * delegate ) {
  int i;
  struct MIDIRunloop * runloop = malloc( sizeof( struct MIDIRunloop ) );
  MIDIPrecondReturn( runloop != NULL, ENOMEM, NULL );

  runloop->refs   = 1;
  runloop->active = 0;
  runloop->master.nfds = 0;
  FD_ZERO( &(runloop->master.readfds) );
  FD_ZERO( &(runloop->master.writefds) );
  _timespec_now( &(runloop->master.timeout_start) );
  _timespec_zero( &(runloop->master.timeout_time) );
  runloop->master.delegate.read    = NULL;
  runloop->master.delegate.write   = NULL;
  runloop->master.delegate.timeout = NULL;
  runloop->master.delegate.info    = runloop;

  for( i=0; i<MAX_RUNLOOP_SOURCES; i++ ) {
    runloop->sources[i] = NULL;
  }
  
  if( delegate != NULL ) {
    runloop->delegate.info             = delegate->info;
    runloop->delegate.schedule_read    = delegate->schedule_read;
    runloop->delegate.schedule_write   = delegate->schedule_write;
    runloop->delegate.schedule_timeout = delegate->schedule_timeout;
    runloop->delegate.clear_read       = delegate->clear_read;
    runloop->delegate.clear_write      = delegate->clear_write;
    runloop->delegate.clear_timeout    = delegate->clear_timeout;
  } else {
    runloop->delegate.info             = NULL;
    runloop->delegate.schedule_read    = NULL;
    runloop->delegate.schedule_write   = NULL;
    runloop->delegate.schedule_timeout = NULL;
    runloop->delegate.clear_read       = NULL;
    runloop->delegate.clear_write      = NULL;
    runloop->delegate.clear_timeout    = NULL;
  }

  return runloop;
}

void MIDIRunloopDestroy( struct MIDIRunloop * runloop ) {
  int i;
  for( i=0; i<MAX_RUNLOOP_SOURCES; i++ ) {
    if( runloop->sources[i] != NULL ) {
      runloop->sources[i]->runloop = NULL;
    }
  }
  free( runloop );
}

void MIDIRunloopRetain( struct MIDIRunloop * runloop ) {
  runloop->refs++;
}

void MIDIRunloopRelease( struct MIDIRunloop * runloop ) {
  if( ! --runloop->refs ) {
    MIDIRunloopDestroy( runloop );
  }
}

static int _runloop_schedule_read( struct MIDIRunloop * runloop, int fd ) {
  MIDIAssert( runloop != NULL );
  
  if( fd >= runloop->master.nfds ) {
    runloop->master.nfds = fd + 1;
  }
  FD_SET( fd, &(runloop->master.readfds) );
  runloop->master.delegate.read = &_runloop_master_read;

  if( runloop->delegate.info != NULL && runloop->delegate.schedule_read != NULL ) {
    return (runloop->delegate.schedule_read)( runloop->delegate.info, fd );
  } else {
    return 0;
  }
}

static int _runloop_clear_read( struct MIDIRunloop * runloop, int fd ) {
  int i;
  MIDIAssert( runloop != NULL );

  for( i=0; i<MAX_RUNLOOP_SOURCES; i++ ) {
    if( runloop->sources[i] != NULL ) {
      if( FD_ISSET( fd, &(runloop->sources[i]->readfds) ) ) {
        return 0;
      }
    }
  }
  
  if( fd == runloop->master.nfds - 1 ) {
    runloop->master.nfds = fd;
  }
  FD_CLR( fd, &(runloop->master.readfds) );

  if( runloop->delegate.info != NULL && runloop->delegate.clear_read != NULL ) {
    return (runloop->delegate.clear_read)( runloop->delegate.info, fd );
  } else {
    return 0;
  }
}

static int _runloop_schedule_write( struct MIDIRunloop * runloop, int fd ) {
  MIDIAssert( runloop != NULL );
  
  if( fd >= runloop->master.nfds ) {
    runloop->master.nfds = fd + 1;
  }
  FD_SET( fd, &(runloop->master.writefds) );
  runloop->master.delegate.write = &_runloop_master_write;

  if( runloop->delegate.info != NULL && runloop->delegate.schedule_write != NULL ) {
    return (runloop->delegate.schedule_write)( runloop->delegate.info, fd );
  } else {
    return 0;
  }
}

static int _runloop_clear_write( struct MIDIRunloop * runloop, int fd ) {
  int i;
  MIDIAssert( runloop != NULL );

  for( i=0; i<MAX_RUNLOOP_SOURCES; i++ ) {
    if( runloop->sources[i] != NULL ) {
      if( FD_ISSET( fd, &(runloop->sources[i]->writefds) ) ) {
        return 0;
      }
    }
  }
  
  if( fd == runloop->master.nfds - 1 ) {
    runloop->master.nfds = fd;
  }
  FD_CLR( fd, &(runloop->master.writefds) );

  if( runloop->delegate.info != NULL && runloop->delegate.clear_write != NULL ) {
    return (runloop->delegate.clear_write)( runloop->delegate.info, fd );
  } else {
    return 0;
  }
}


static int _runloop_schedule_timeout( struct MIDIRunloop * runloop, struct timespec * timeout ) {
  MIDIAssert( runloop != NULL );

  if( ! _timespec_empty( timeout ) ) {
    if( (  _timespec_cmp( timeout, &(runloop->master.timeout_time) ) < 0 )
        || _timespec_empty( &(runloop->master.timeout_time) ) ) {
      /* fix this:
       * The start time should not be changed. Instead the remaining time of the source
       * timeout should be computed and checked against the remaining time of the master
       * source.
       */
      _timespec_now( &(runloop->master.timeout_start) );
      _timespec_cpy( &(runloop->master.timeout_time), timeout );
    }
    runloop->master.delegate.timeout = &_runloop_master_timeout;
  }

  if( runloop->delegate.info != NULL && runloop->delegate.schedule_timeout != NULL ) {
    return (runloop->delegate.schedule_timeout)( runloop->delegate.info, timeout );
  } else {
    return 0;
  }
}

static int _runloop_update_from_source( struct MIDIRunloop * runloop, struct MIDIRunloopSource * source ) {
  if( ! _timespec_empty( &(source->timeout_time) ) ) {
    if( (  _timespec_cmp( &(source->timeout_time), &(runloop->master.timeout_time) ) < 0 )
        || _timespec_empty( &(runloop->master.timeout_time) ) ) {
      /* fix this:
       * The start time should not be changed. Instead the remaining time of the source
       * timeout should be computed and checked against the remaining time of the master
       * source.
       */
      _timespec_now( &(runloop->master.timeout_start) );
      _timespec_cpy( &(runloop->master.timeout_time), &(source->timeout_time) );
    }
  }
  if( source->nfds > 0 ) {
    _fds_add( &(runloop->master.readfds), &(source->readfds), source->nfds );
    _fds_add( &(runloop->master.writefds), &(source->writefds), source->nfds );
    if( source->nfds > runloop->master.nfds ) {
      runloop->master.nfds = source->nfds;
    }
  }
  if( source->delegate.read != NULL ) {
    runloop->master.delegate.read = &_runloop_master_read;
  }
  if( source->delegate.write != NULL ) {
    runloop->master.delegate.write = &_runloop_master_write;
  }
  if( source->delegate.timeout != NULL ) {
    runloop->master.delegate.timeout = &_runloop_master_timeout;
  }
  return 0;
}

int MIDIRunloopAddSource( struct MIDIRunloop * runloop, struct MIDIRunloopSource * source ) {
  int i;
  for( i=0; i<MAX_RUNLOOP_SOURCES; i++ ) {
    if( runloop->sources[i] == NULL ) {
      runloop->sources[i] = source;
      source->runloop = runloop;
      MIDIRunloopSourceRetain( source );
      break;
    }
  }
  if( i == MAX_RUNLOOP_SOURCES ) {
    return 1;
  }
  _runloop_update_from_source( runloop, source );
  MIDILog( DEVELOP, "master timeout %lu sec + %lu nsec\nnfds: %i\n",
    runloop->master.timeout_time.tv_sec, runloop->master.timeout_time.tv_nsec, runloop->master.nfds );
  return 0;
}

int MIDIRunloopRemoveSource( struct MIDIRunloop * runloop, struct MIDIRunloopSource * source ) {
  int i;
  for( i=0; i<MAX_RUNLOOP_SOURCES; i++ ) {
    if( runloop->sources[i] == source ) {
      runloop->sources[i] = NULL;
      MIDIRunloopSourceRelease( source );
      return 0;
    }
  }
  return 1;
}

int MIDIRunloopStep( struct MIDIRunloop * runloop ) {
  return MIDIRunloopSourceWait( &(runloop->master) );
}

int MIDIRunloopStart( struct MIDIRunloop * runloop ) {
  int result = 0;
  runloop->active = 1;
  do {
    result = MIDIRunloopStep( runloop );
    if( result != 0 ) {
      runloop->active = 0;
    }
  } while( runloop->active );
  CURRENT_RUNLOOP(NULL);
  return result;
}

int MIDIRunloopStop( struct MIDIRunloop * runloop ) {
  runloop->active = 0;
  return 0;
}
