#ifndef MIDIKIT_TYPE_H
#define MIDIKIT_TYPE_H

typedef void MIDIRefFn( void * object );
typedef int MIDICoderFn( void * object, size_t size, void * buffer, size_t * bytes );

struct MIDITypeSpec {
  size_t type_id;
  char * type_name;
  size_t size;
  MIDIRefFn *retain;
  MIDIRefFn *release;
  MIDICoderFn *encode;
  MIDICoderFn *decode;
};

#define _CONCAT( a, b ) a ## b
#define CONCAT( a, b ) _CONCAT(a,b)

#define MIDI_MK_TYPE_SPEC( t ) \
  CONCAT( CONCAT( _type_spec_, t ), __LINE__ )

#define MIDI_TYPE_SPEC( t, id, ret, rel, enc, dec ) \
  static struct MIDITypeSpec MIDI_MK_TYPE_SPEC(t) = { \
    id, #t, sizeof( struct t ), \
    (MIDIRefFn*) ret, \
    (MIDIRefFn*) rel, \
    (MIDICoderFn*) enc, \
    (MIDICoderFn*) dec \
  }; \
  struct MIDITypeSpec * t ## Type = &MIDI_MK_TYPE_SPEC(t)

#define MIDI_TYPE_SPEC_CODING( t, id ) \
  MIDI_TYPE_SPEC( t, id, &t ## Retain, &t ## Release, &t ## Encode, &t ## Decode )

#define MIDI_TYPE_SPEC_OBJECT( t, id ) \
  MIDI_TYPE_SPEC( t, id, &t ## Retain, &t ## Release, NULL, NULL )

#endif
