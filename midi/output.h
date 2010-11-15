#ifndef MIDIKIT_MIDI_OUTPUT_H
#define MIDIKIT_MIDI_OUTPUT_H

struct MIDIDriver;
struct MIDIDevice;
struct MIDIMessage;

struct MIDIOutput {
  size_t refs;
  struct MIDIDevice * device;
  struct MIDIDriver * driver;
};

struct MIDIOutput * MIDIOutputCreate( struct MIDIDevice * device, struct MIDIDriver * driver );
void MIDIOutputDestroy( struct MIDIOutput * output );
void MIDIOutputRetain( struct MIDIOutput * output );
void MIDIOutputRelease( struct MIDIOutput * output );
int MIDIOutputConnect( struct MIDIOutput * output );
int MIDIOutputDisconnect( struct MIDIOutput * output );
int MIDIOutputSend( struct MIDIOutput * output, struct MIDIMessage * message );
#endif
