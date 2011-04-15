#ifndef MIDIKIT_MIDI_RUNLOOP_H
#define MIDIKIT_MIDI_RUNLOOP_H

#define MIDI_RUNLOOP_READ       1
#define MIDI_RUNLOOP_WRITE      2
#define MIDI_RUNLOOP_IDLE       4
#define MIDI_RUNLOOP_INVALIDATE 8

struct MIDIRunloopSource;
struct MIDIRunloop;

struct MIDIRunloopSource * MIDIRunloopSourceCreate(
  void * info,
  int (*read)( void * info, int nfds, fd_set * readfds ),
  int (*write)( void * info, int nfds, fd_set * readfds ),
  int (*timeout)( void * info, struct timespec * elapsed )
);
void MIDIRunloopSourceDestroy( struct MIDIRunloopSource * source );
void MIDIRunloopSourceRetain( struct MIDIRunloopSource * source );
void MIDIRunloopSourceRelease( struct MIDIRunloopSource * source );

int MIDIRunloopSourceInvalidate( struct MIDIRunloopSource * source );
int MIDIRunloopSourceWait( struct MIDIRunloopSource * source );

int MIDIRunloopSourceScheduleRead( struct MIDIRunloopSource * source, int fd );
int MIDIRunloopSourceClearRead( struct MIDIRunloopSource * source, int fd );
int MIDIRunloopSourceScheduleWrite( struct MIDIRunloopSource * source, int fd );
int MIDIRunloopSourceClearWrite( struct MIDIRunloopSource * source, int fd );
int MIDIRunloopSourceScheduleTimeout( struct MIDIRunloopSource * source, struct timespec * timeout );
int MIDIRunloopSourceClearTimeout( struct MIDIRunloopSource * source );

struct MIDIRunloop * MIDIRunloopCreate();
void MIDIRunloopDestroy( struct MIDIRunloop * runloop );
void MIDIRunloopRetain( struct MIDIRunloop * runloop );
void MIDIRunloopRelease( struct MIDIRunloop * runloop );

int MIDIRunloopUpdateFromSource( struct MIDIRunloop * runloop, struct MIDIRunloopSource * source );
int MIDIRunloopAddSource( struct MIDIRunloop * runloop, struct MIDIRunloopSource * source );
int MIDIRunloopRemoveSource( struct MIDIRunloop * runloop, struct MIDIRunloopSource * source );

int MIDIRunloopStart( struct MIDIRunloop * runloop );
int MIDIRunloopStop( struct MIDIRunloop * runloop );
int MIDIRunloopStep( struct MIDIRunloop * runloop );

#endif
