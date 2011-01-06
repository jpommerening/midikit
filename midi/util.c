#include <stdlib.h>
#include "util.h"

/**
 * @brief Read an encoded number with a variable number of bytes.
 * @param buffer The buffer to read from.
 * @param size   The number of available bytes in the buffer.
 * @param value  The location to store the read number in.
 * @param read   The location to store the number of read bytes in.
 *               (may be @c NULL)
 * @retval 0 on success.
 * @retval >0 if the number could not be read.
 */
int MIDIUtilReadVarLen( unsigned char * buffer, size_t size, MIDIVarLen * value, size_t * read ) {
  MIDIVarLen v = 0;
  size_t p = 0;
  if( buffer == NULL || value == NULL ) return 1;

  do {
    if( p>=size ) return 1;
    v = (v<<7) | (buffer[p] & 0x7f);
  } while( buffer[p++] & 0x80 );
  *value = v & 0x0fffffff;
  if( read != NULL ) *read  = p;
  return 0;
}

/**
 * @brief Write a number encoding it with a variable number of bytes.
 * @param buffer  The buffer to write to.
 * @param size    The number of available bytes in the buffer.
 * @param value   The location to read the number from.
 * @param written The location to store the number of written bytes in.
 *               (may be @c NULL)
 * @retval 0 on success.
 * @retval >0 if the number could not be read.
 */
int MIDIUtilWriteVarLen( unsigned char * buffer, size_t size, MIDIVarLen * value, size_t * written ) {
  MIDIVarLen v = 0;
  unsigned char tmp[4] = { 0x80, 0x80, 0x80, 0x00 };
  size_t p = 0, q = 0;
  if( buffer == NULL || value == NULL ) return 1;
  v = *value & 0x0fffffff;
  do {
    tmp[3-p] |= v & 0x7f;
    v >>= 7;
    p++;
  } while( v != 0 && p<4 );
  if( p>size ) return 1;
  q = 4-p;
  for( p=0; q+p<4; p++ ) {
    buffer[p] = tmp[q+p];
  }
  if( written != NULL ) *written = p;
  return 0;
}

#include <sys/select.h>
#include <time.h>

#define MAX_RUNLOOP_SOURCES 16 

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
  lhs->tv_sec  -= rhs->tv_sec;
  lhs->tv_nsec -= rhs->tv_nsec;
  if( lhs->tv_nsec >= 1000000 ) {
    lhs->tv_sec  += lhs->tv_nsec / 1000000;
    lhs->tv_nsec %= 1000000;
  } else {
    while( lhs->tv_nsec < 0 ) {
      lhs->tv_sec  -= 1;
      lhs->tv_nsec += 1000000;
    }
  }
}

int MIDIRunloopSourceWait( struct MIDIRunloopSource * source ) {
  int result = 0;
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

    result = select( source->nfds, &readfds, &writefds, NULL, &tv );
    if( result > 0 ) {
      source->remain.tv_sec  = source->timeout.tv_sec;
      source->remain.tv_nsec = source->timeout.tv_nsec;
      result = 0;
      if( source->read != NULL && _check_fds( &readfds, source->nfds ) ) {
        result += (source->read)( source->info, source->nfds, &readfds );
      }
      if( source->write != NULL && _check_fds( &writefds, source->nfds ) ) {
        result += (source->write)( source->info, source->nfds, &writefds );
      }
      return result;
    } else if( result == 0 && source->idle != NULL ) {
      source->remain.tv_sec  = source->timeout.tv_sec;
      source->remain.tv_nsec = source->timeout.tv_nsec;
      return (source->idle)( source->info, &(source->timeout) );
    }
  } else if( source->timeout.tv_sec > 0 || source->timeout.tv_nsec > 0 ) {
    /* nanosleep */
    result = nanosleep( &(source->timeout), &(source->remain) );
    if( result == 0 && source->idle != NULL ) {
      ts.tv_sec  = source->timeout.tv_sec;
      ts.tv_nsec = source->timeout.tv_nsec;
      _timespec_sub( &ts, &(source->remain) );
      source->remain.tv_sec  = source->timeout.tv_sec;
      source->remain.tv_nsec = source->timeout.tv_nsec;
      return (source->idle)( source->info, &ts );
    }
  }
  return 0;
}

struct MIDIRunloop {
  size_t refs;
  int active;
  struct MIDIRunloopSource master;
  struct MIDIRunloopSource * sources[MAX_RUNLOOP_SOURCES];
};

static int _runloop_master_read( void * rl, int nfds, fd_set * readfds ) {
  int i, result = 0, cb = 0;
  struct MIDIRunloop * runloop = rl;
  struct MIDIRunloopSource * source;
  /* update internal idle timer(s) */
  for( i=0; i<MAX_RUNLOOP_SOURCES; i++ ) {
    source = runloop->sources[i];
    if( source == NULL ) continue;
    /* update / decrement remaining time
     * check if idle callback needs to be fired */
    if( source->read != NULL ) {
      cb++;
      if( _check_fds2( readfds, &(source->readfds), source->nfds ) ) {
        /* reset remaining time */
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
  /* update internal idle timer(s) */
  for( i=0; i<MAX_RUNLOOP_SOURCES; i++ ) {
    source = runloop->sources[i];
    if( source == NULL ) continue;
    /* update / decrement remaining time
     * check if idle callback needs to be fired */
    if( source->write != NULL ) {
      cb++;
      if( _check_fds2( writefds, &(source->writefds), source->nfds ) ) {
        /* reset remaining time */
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
  int i, result = 0, cb = 0;
  struct MIDIRunloop * runloop = rl;
  struct MIDIRunloopSource * source;
  /* update internal idle timer(s) */
  for( i=0; i<MAX_RUNLOOP_SOURCES; i++ ) {
    source = runloop->sources[i];
    if( source == NULL ) continue;
    /* update / decrement remaining time
     * check if idle callback needs to be fired */

    /* this can be removed (or be used to the thing stated above) */
    if( source->idle != NULL ) {
      cb++;
      if( source->remain.tv_sec < ts->tv_sec ||
        ( source->remain.tv_sec == ts->tv_sec && 
          source->remain.tv_nsec < ts->tv_nsec ) ) {
        source->remain.tv_sec  = source->timeout.tv_sec;
        source->remain.tv_nsec = source->timeout.tv_nsec;
        result += (source->idle)( source->info, ts );
      } else {
        _timespec_sub( &(source->remain), ts );
      }
    }
    if( source->read != NULL ) {
      runloop->master.read  = &_runloop_master_read;
    }
    if( source->write != NULL ) {
      runloop->master.write = &_runloop_master_write;
    }
  }
  printf( "master idle\n" );
  return result;
}

struct MIDIRunloop * MIDIRunloopCreate() {
  struct MIDIRunloop * runloop = malloc( sizeof( struct MIDIRunloop ) );
  int i;
  if( runloop == NULL ) return NULL;

  runloop->refs   = 1;
  runloop->active = 0;
  runloop->master.nfds = 0;
  FD_ZERO( &(runloop->master.readfds) );
  FD_ZERO( &(runloop->master.writefds) );
  runloop->master.timeout.tv_sec  = 0;
  runloop->master.timeout.tv_nsec = 100000; // 100 ms
  runloop->master.read  = NULL;
  runloop->master.write = NULL;
  runloop->master.idle  = NULL;
  runloop->master.info  = runloop;

  for( i=0; i<MAX_RUNLOOP_SOURCES; i++ ) {
    runloop->sources[i] = NULL;
  }
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
  if( source->idle != NULL ) {
    runloop->master.idle = &_runloop_master_idle;
  }
  if( source->read != NULL ) {
    runloop->master.read = &_runloop_master_read;
  }
  if( source->write != NULL ) {
    runloop->master.write = &_runloop_master_write;
  }
  printf( "master timeout %lu sec + %lu nsec\nnfds: %i\n", runloop->master.timeout.tv_sec, runloop->master.timeout.tv_nsec, runloop->master.nfds );
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

