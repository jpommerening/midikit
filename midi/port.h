#ifndef MIDIKIT_MIDI_PORT_H
#define MIDIKIT_MIDI_PORT_H

struct MIDIPort;

struct MIDIPort * MIDIPortCreate( char * name, void * target,
                                  int (*receive)( void * target, void * source,
                                                  int type, size_t size, void * data ) );
void MIDIPortDestroy( struct MIDIPort * port );
void MIDIPortRetain( struct MIDIPort * port );
void MIDIPortRelease( struct MIDIPort * port );

int MIDIPortConnect( struct MIDIPort * port, struct MIDIPort * target );
int MIDIPortDisconnect( struct MIDIPort * port, struct MIDIPort * target );
int MIDIPortInvalidate( struct MIDIPort * port );

int MIDIPortReceiveFrom( struct MIDIPort * port, struct MIDIPort * source, int type, size_t size, void * data );
int MIDIPortReceive( struct MIDIPort * port, int type, size_t size, void * data );
int MIDIPortSendTo( struct MIDIPort * port, struct MIDIPort * target, int type, size_t size, void * data );
int MIDIPortSend( struct MIDIPort * port, int type, size_t size, void * data );

#endif
