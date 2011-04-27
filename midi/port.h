#ifndef MIDIKIT_MIDI_PORT_H
#define MIDIKIT_MIDI_PORT_H
#include "type.h"

#define MIDI_PORT_IN      0x01
#define MIDI_PORT_OUT     0x02
#define MIDI_PORT_THRU    0x04
#define MIDI_PORT_INVALID 0x08

struct MIDIObjectSpec {
  int id;
  size_t size;
  void (*retain)( void * );
  void (*release)( void * );
};

struct MIDIPort;
extern struct MIDITypeSpec * MIDIPortType;

typedef int MIDIPortReceiveFn( void * target, void * source, struct MIDITypeSpec * type, void * object );
typedef int MIDIPortInterceptFn( void * observer, struct MIDIPort * port, int mode, struct MIDITypeSpec * type, void * object );

struct MIDIPort * MIDIPortCreate( char * name, int mode, void * target,
                                  MIDIPortReceiveFn * receive );

void MIDIPortDestroy( struct MIDIPort * port );
void MIDIPortRetain( struct MIDIPort * port );
void MIDIPortRelease( struct MIDIPort * port );

int MIDIPortConnect( struct MIDIPort * port, struct MIDIPort * target );
int MIDIPortDisconnect( struct MIDIPort * port, struct MIDIPort * target );
int MIDIPortDisconnectAll( struct MIDIPort * port );
int MIDIPortInvalidate( struct MIDIPort * port );

int MIDIPortSetObserver( struct MIDIPort * port, void * target, MIDIPortInterceptFn * intercept );
int MIDIPortGetObserver( struct MIDIPort * port, void ** target, MIDIPortInterceptFn ** intercept );

int MIDIPortReceiveFrom( struct MIDIPort * port, struct MIDIPort * source, struct MIDITypeSpec * type, void * object );
int MIDIPortReceive( struct MIDIPort * port, struct MIDITypeSpec * type, void * object );
int MIDIPortSendTo( struct MIDIPort * port, struct MIDIPort * target, struct MIDITypeSpec * type, void * object );
int MIDIPortSend( struct MIDIPort * port, struct MIDITypeSpec * type, void * object );

#endif
