#include <stdlib.h>
#include <sys/select.h>
#include <sys/time.h>
#include <time.h>
#include "runloop.h"
#include "midi.h"

#define MAX_RUNLOOP_SOURCES 16 

#define MIDI_RUNLOOP_READ  1
#define MIDI_RUNLOOP_WRITE 2
#define MIDI_RUNLOOP_IDLE  4

static int _cmp_fds( fd_set * a, fd_set * b, int nfds ) {
  int fd;
  for( fd=0; fd<nfds; fd++ ) {
    if( FD_ISSET( fd, a ) != FD_ISSET( fd, b ) ) {
      return 1;
    }
  }
  return 0;
}

static int _check_fds( fd_set * fds, int nfds ) {
  fd_set empty;
  FD_ZERO( &empty );
  return _cmp_fds( fds, &empty, nfds );
}

static int _check_fds2( fd_set * fds, fd_set * chk, int nfds ) {
  int fd;
  for( fd=0; fd<nfds; fd++ ) {
    if( FD_ISSET( fd, chk ) && FD_ISSET( fd, fds ) ) {
      return 1;
    }
  }
  return 0;
}

static int _cpy_fds2( fd_set * lhs, fd_set * rhs, int nfds ) {
  int fd;
  for( fd=0; fd<nfds; fd++ ) {
    if( FD_ISSET( fd, rhs ) ) {
      FD_SET( fd, lhs );
    }
  }
  return 0;
}

static int _cpy_fds( fd_set * lhs, fd_set * rhs, int nfds ) {
  FD_ZERO( lhs );
  return _cpy_fds2( lhs, rhs, nfds );
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

static void _timespec_get( struct timespec * ts ) {
  struct timeval  tv = { 0, 0 };
  gettimeofday( &tv, NULL );
  ts->tv_sec  = tv.tv_sec;
  ts->tv_nsec = tv.tv_usec * 1000;
}

static void _timespec_elapsed( struct timespec * ts ) {
  struct timeval  tv = { 0, 0 };
  struct timespec td = { ts->tv_sec, ts->tv_nsec };
  gettimeofday( &tv, NULL );
  ts->tv_sec  = tv.tv_sec;
  ts->tv_nsec = tv.tv_usec * 1000;
  _timespec_sub( ts, &td );
}

static int _source_reset_remain( struct MIDIRunloopSource * source ) {
  source->remain.tv_sec  = source->timeout.tv_sec;
  source->remain.tv_nsec = source->timeout.tv_nsec;
  return 0;
}

/**
 * @brief Wait until any callback of the runloop source is triggered.
 * @public @memberof MIDIRunloopSource
 * @param source The runloop source.
 */
int MIDIRunloopSourceWait( struct MIDIRunloopSource * source ) {
  int result = 0, idle = 1;
  struct timeval  tv = { 0, 0 };
  struct timespec ts = { 0, 0 };
  fd_set readfds;
  fd_set writefds;

  if( source->nfds > 0 ) {
    /* select */
    tv.tv_sec  = source->timeout.tv_sec;
    tv.tv_usec = source->timeout.tv_nsec / 1000;

    _cpy_fds( &readfds, &(source->readfds), source->nfds );
    _cpy_fds( &writefds, &(source->writefds), source->nfds );
    _timespec_get( &ts );
    result = select( source->nfds, &readfds, &writefds, NULL, &tv );
    if( result > 0 ) {
      result = 0;
      if( source->read != NULL && _check_fds( &readfds, source->nfds ) ) {
        idle = 0;
        result += (source->read)( source->info, source->nfds, &readfds );
      }
      if( source->write != NULL && _check_fds( &writefds, source->nfds ) ) {
        idle = 0;
        result += (source->write)( source->info, source->nfds, &writefds );
      }
    }
    if( idle && source->idle != NULL ) {
      _timespec_elapsed( &ts );
      _source_reset_remain( source );
      result += (source->idle)( source->info, &ts );
    }
  } else if( source->timeout.tv_sec > 0 || source->timeout.tv_nsec > 0 ) {
    /* nanosleep */
    result = nanosleep( &(source->timeout), &(source->remain) );
    if( result == 0 && source->idle != NULL ) {
      ts.tv_sec  = source->timeout.tv_sec;
      ts.tv_nsec = source->timeout.tv_nsec;
      _timespec_sub( &ts, &(source->remain) );
      _source_reset_remain( source );
      result += (source->idle)( source->info, &ts );
    }
  }
  return result;
}

/**
 * @brief Schedule the read callback of a runloop source.
 * Enable the read callback of a runloop source. The callback will be invoked the
 * next time any of the runloop source's "read" file descriptors have new data to
 * be read. After that the callback will be disabled until this function is called
 * again.
 * @public @memberof MIDIRunloopSource
 * @param source The source that should be scheduled.
 */
int MIDIRunloopSourceScheduleRead( struct MIDIRunloopSource * source ) {

  if( source->schedule != NULL ) {
    (*source->schedule)( source, MIDI_RUNLOOP_READ );
  }
  return 0;
}

/**
 * @brief Schedule the write callback of a runloop source.
 * Enable the write callback of a runloop source. The callback will be invoked the
 * next time any of the runloop source's "write" file descriptors can accept new
 * data. After that the callback will be disabled until this function is called
 * again.
 * @public @memberof MIDIRunloopSource
 * @param source The source that should be scheduled.
 */
int MIDIRunloopSourceScheduleWrite( struct MIDIRunloopSource * source ) {

  if( source->schedule != NULL ) {
    (*source->schedule)( source, MIDI_RUNLOOP_WRITE );
  }
  return 0;
}

/**
 * @brief Schedule the idle callback of a runloop source.
 * Enable the idle callback of a runloop source. The callback will be invoked the
 * next time any of the runloop source's timeout expired. After that the callback
 * will be disabled until this function is called again.
 * @public @memberof MIDIRunloopSource
 * @param source The source that should be scheduled.
 */
int MIDIRunloopSourceScheduleIdle( struct MIDIRunloopSource * source ) {

  if( source->schedule != NULL ) {
    (*source->schedule)( source, MIDI_RUNLOOP_IDLE );
  }
  return 0;
}

/**
 * Reimplement the scheduling alltogether:
 * Replace the "master runloop source" with the
 * respective filedescriptors and a global timer.
 */

struct MIDIRunloop {
  size_t refs;
  int active;
  struct timespec ts;
  struct MIDIRunloopSource master;
  struct MIDIRunloopSource * sources[MAX_RUNLOOP_SOURCES];
};

static void _runloop_get_elapsed_time( struct MIDIRunloop * runloop, struct timespec * ts ) {
  if( ts != NULL ) {
    ts->tv_sec  = runloop->ts.tv_sec;
    ts->tv_nsec = runloop->ts.tv_nsec;
    _timespec_elapsed( ts );
  }
  _timespec_get( &(runloop->ts) );
}

static int _source_check_idle( struct MIDIRunloopSource * source, struct timespec * ts ) {
  _timespec_sub( &(source->remain), ts );
  if( ( source->remain.tv_sec < 0 ) ||
      ( source->remain.tv_sec == 0 &&
        source->remain.tv_nsec < 0 ) ) {
    if( source->idle != NULL ) {
      _source_reset_remain( source );
      return (source->idle)( source->info, ts );
    }
  }
  return 0;
}

static int _runloop_master_read( void * rl, int nfds, fd_set * readfds ) {
  int i, result = 0, cb = 0;
  struct MIDIRunloop * runloop = rl;
  struct MIDIRunloopSource * source;
  struct timespec ts = { 0, 0 };

  /* update internal idle timer(s) */
  _runloop_get_elapsed_time( runloop, &ts );
  
  for( i=0; i<MAX_RUNLOOP_SOURCES; i++ ) {
    source = runloop->sources[i];
    if( source == NULL ) continue;
    /* update / decrement remaining time
     * check if idle callback needs to be fired */
    result += _source_check_idle( source, &ts );
    if( source->read != NULL ) {
      cb++;
      if( _check_fds2( readfds, &(source->readfds), source->nfds ) ) {
        /* reset remaining time */
        _source_reset_remain( source );
        result += (source->read)( source->info, source->nfds, readfds );
      }
    }
  }
  if( cb == 0 ) {
    runloop->master.read = NULL;
  }
  return result;
}

static int _runloop_master_write( void * rl, int nfds, fd_set * writefds ) {
  int i, result = 0, cb = 0;
  struct MIDIRunloop * runloop = rl;
  struct MIDIRunloopSource * source;  
  struct timespec ts = { 0, 0 };

  /* update internal idle timer(s) */
  _runloop_get_elapsed_time( runloop, &ts );
  
  for( i=0; i<MAX_RUNLOOP_SOURCES; i++ ) {
    source = runloop->sources[i];
    if( source == NULL ) continue;
    /* update / decrement remaining time
     * check if idle callback needs to be fired */
    result += _source_check_idle( source, &ts );
    if( source->write != NULL ) {
      cb++;
      if( _check_fds2( writefds, &(source->writefds), source->nfds ) ) {
        /* reset remaining time */
        _source_reset_remain( source );
        result += (source->write)( source->info, source->nfds, writefds );
      }
    }
  }
  if( cb == 0 ) {
    runloop->master.write = NULL;
  }
  return result;
}

/* fix idle multiplexing:
 * the if *any* of the runloop sources fires (writefds) frequently,
 * no runloop source will ever be idle.
 * we should decrement the remaining time whenever the event
 * does not fire. whenever a callback is invoked we can reset
 * the remaining time. */

static int _runloop_master_idle( void * rl, struct timespec * ts ) {
  int i, result = 0;
  struct MIDIRunloop * runloop = rl;
  struct MIDIRunloopSource * source;

  /* update internal idle timer(s) */
  _runloop_get_elapsed_time( runloop, ts );
  
  for( i=0; i<MAX_RUNLOOP_SOURCES; i++ ) {
    source = runloop->sources[i];
    if( source == NULL ) continue;
    /* update / decrement remaining time
     * check if idle callback needs to be fired */
    result += _source_check_idle( source, ts );
    
    if( source->read != NULL ) {
      runloop->master.read  = &_runloop_master_read;
    }
    if( source->write != NULL ) {
      runloop->master.write = &_runloop_master_write;
    }
  }
  return result;
}

struct MIDIRunloop * MIDIRunloopCreate() {
  struct MIDIRunloop * runloop = malloc( sizeof( struct MIDIRunloop ) );
  int i;
  if( runloop == NULL ) return NULL;

  runloop->refs   = 1;
  runloop->active = 0;
  runloop->ts.tv_sec   = 0;
  runloop->ts.tv_nsec  = 0;
  runloop->master.nfds = 0;
  FD_ZERO( &(runloop->master.readfds) );
  FD_ZERO( &(runloop->master.writefds) );
  runloop->master.timeout.tv_sec  = 0;
  runloop->master.timeout.tv_nsec = 10000000; /* 10 ms */
  runloop->master.remain.tv_sec  = 0;
  runloop->master.remain.tv_nsec = 10000000; /* 10 ms */
  runloop->master.read  = NULL;
  runloop->master.write = NULL;
  runloop->master.idle  = NULL;
  runloop->master.info  = runloop;

  for( i=0; i<MAX_RUNLOOP_SOURCES; i++ ) {
    runloop->sources[i] = NULL;
  }
  _runloop_get_elapsed_time( runloop, NULL );
  return runloop;
}

void MIDIRunloopDestroy( struct MIDIRunloop * runloop ) {
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

int MIDIRunloopAddSource( struct MIDIRunloop * runloop, struct MIDIRunloopSource * source ) {
  int i;
  for( i=0; i<MAX_RUNLOOP_SOURCES; i++ ) {
    if( runloop->sources[i] == NULL ) {
      runloop->sources[i] = source;
      break;
    }
  }
  if( i == MAX_RUNLOOP_SOURCES ) {
    return 1;
  }
  if( ( source->timeout.tv_sec != 0 || source->timeout.tv_nsec != 0 ) &&
      ( source->timeout.tv_sec < runloop->master.timeout.tv_sec ||
      ( source->timeout.tv_sec == runloop->master.timeout.tv_sec &&
        source->timeout.tv_nsec < runloop->master.timeout.tv_nsec ) ) ) {
    runloop->master.timeout.tv_sec  = source->timeout.tv_sec;
    runloop->master.timeout.tv_nsec = source->timeout.tv_nsec;
  }
  if( source->nfds > 0 ) {
    _cpy_fds2( &(runloop->master.readfds), &(source->readfds), source->nfds );
    _cpy_fds2( &(runloop->master.writefds), &(source->writefds), source->nfds );
    if( source->nfds > runloop->master.nfds ) {
      runloop->master.nfds = source->nfds;
    }
  }
  if( source->read != NULL ) {
    runloop->master.read = &_runloop_master_read;
  }
  if( source->write != NULL ) {
    runloop->master.write = &_runloop_master_write;
  }
  runloop->master.idle = &_runloop_master_idle;
  MIDILog( DEVELOP, "master timeout %lu sec + %lu nsec\nnfds: %i\n",
    runloop->master.timeout.tv_sec, runloop->master.timeout.tv_nsec, runloop->master.nfds );
  return 0;
}

int MIDIRunloopRemoveSource( struct MIDIRunloop * runloop, struct MIDIRunloopSource * source ) {
  int i;
  for( i=0; i<MAX_RUNLOOP_SOURCES; i++ ) {
    if( runloop->sources[i] == source ) {
      runloop->sources[i] = NULL;
      return 0;
    }
  }
  return 1;
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
  return result;
}

int MIDIRunloopStop( struct MIDIRunloop * runloop ) {
  runloop->active = 0;
  return 0;
}

int MIDIRunloopStep( struct MIDIRunloop * runloop ) {
  return MIDIRunloopSourceWait( &(runloop->master) );
}

