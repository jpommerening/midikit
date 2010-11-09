#ifndef MIDIKIT_MIDI_INPUT_H
#define MIDIKIT_MIDI_INPUT_H

struct MIDIDriver;
struct MIDIDevice;
struct MIDIMessage;

struct MIDIInput {
  size_t refs;
  struct MIDIDevice * device;
  struct MIDIDriver * driver;
};

struct MIDIInput * MIDIInputCreate( struct MIDIDevice * device, struct MIDIDriver * driver );
void MIDIInputDestroy( struct MIDIInput * input );
void MIDIInputRetain( struct MIDIInput * input );
void MIDIInputRelease( struct MIDIInput * input );
int MIDIInputConnect( struct MIDIInput * input );
int MIDIInputDisconnect( struct MIDIInput * input );
int MIDIInputReceive( struct MIDIInput * input, struct MIDIMessage * message );
#endif