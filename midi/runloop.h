#ifndef MIDIKIT_MIDI_RUNLOOP_H
#define MIDIKIT_MIDI_RUNLOOP_H
#include <sys/select.h>

struct MIDIRunloopSource {
  int nfds;
  int flags;
  fd_set readfds;
  fd_set writefds;
  struct timespec timeout;
  struct timespec remain;
  int (*read)( void * info, int nfds, fd_set * readfds );
  int (*write)( void * info, int nfds, fd_set * writefds );
  int (*idle)( void * info, struct timespec * idletime );
  int (*schedule)( struct MIDIRunloopSource * source, int event );
  void * info;
};

int MIDIRunloopSourceWait( struct MIDIRunloopSource * source );
int MIDIRunloopSourceScheduleRead( struct MIDIRunloopSource * source );
int MIDIRunloopSourceScheduleWrite( struct MIDIRunloopSource * source );
int MIDIRunloopSourceScheduleIdle( struct MIDIRunloopSource * source );

struct MIDIRunloop;

struct MIDIRunloop * MIDIRunloopCreate();
void MIDIRunloopDestroy( struct MIDIRunloop * runloop );
void MIDIRunloopRetain( struct MIDIRunloop * runloop );
void MIDIRunloopRelease( struct MIDIRunloop * runloop );

int MIDIRunloopAddSource( struct MIDIRunloop * runloop, struct MIDIRunloopSource * source );
int MIDIRunloopRemoveSource( struct MIDIRunloop * runloop, struct MIDIRunloopSource * source );

int MIDIRunloopStart( struct MIDIRunloop * runloop );
int MIDIRunloopStop( struct MIDIRunloop * runloop );
int MIDIRunloopStep( struct MIDIRunloop * runloop );

#endif
