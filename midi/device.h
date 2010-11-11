#ifndef MIDIKIT_MIDI_DEVICE_H
#define MIDIKIT_MIDI_DEVICE_H
#include "midi.h"

struct MIDIInput;
struct MIDIOutput;
struct MIDIMessage;

struct MIDIDevice;
struct MIDIDeviceContext {
  int (*recv_nof)( struct MIDIDevice * device, MIDIValue channel, MIDIValue key, MIDIValue velocity ); ///< Note on callback
  int (*recv_non)( struct MIDIDevice * device, MIDIValue channel, MIDIValue key, MIDIValue velocity ); ///< Note off callback
  int (*recv_pkp)( struct MIDIDevice * device, MIDIValue channel, MIDIValue key, MIDIValue velocity ); ///< Polyphonic key pressure callback
  int (*recv_cc)( struct MIDIDevice * device, MIDIValue channel, MIDIValue control, MIDIValue value ); ///< Control change callback
  int (*recv_pc)( struct MIDIDevice * device, MIDIValue channel, MIDIValue program );                  ///< Program change callback
  int (*recv_cp)( struct MIDIDevice * device, MIDIValue channel, MIDIValue value );                    ///< Channel pressure callback
  int (*recv_pwc)( struct MIDIDevice * device, MIDIValue channel, MIDILongValue value );               ///< Pitch wheel change callback
};

struct MIDIDevice * MIDIDeviceCreate( struct MIDIDeviceContext * context );
void MIDIDeviceDestroy( struct MIDIDevice * device );
void MIDIDeviceRetain( struct MIDIDevice * device );
void MIDIDeviceRelease( struct MIDIDevice * device );

int MIDIDeviceDetachIn( struct MIDIDevice * device );
int MIDIDeviceAttachIn( struct MIDIDevice * device, struct MIDIInput * in );

int MIDIDeviceDetachOut( struct MIDIDevice * device );
int MIDIDeviceAttachOut( struct MIDIDevice * device, struct MIDIOutput * out );

int MIDIDeviceDetachThru( struct MIDIDevice * device );
int MIDIDeviceAttachThru( struct MIDIDevice * device, struct MIDIOutput * thru );

int MIDIDeviceReceive( struct MIDIDevice * device, struct MIDIMessage * message );
int MIDIDeviceSend( struct MIDIDevice * device, struct MIDIMessage * message );

int MIDIDeviceReceiveNoteOff( struct MIDIDevice * device, MIDIValue channel, MIDIValue key, MIDIValue velocity );
int MIDIDeviceSendNoteOff( struct MIDIDevice * device, MIDIValue channel, MIDIValue key, MIDIValue velocity );

int MIDIDeviceReceiveNoteOn( struct MIDIDevice * device, MIDIValue channel, MIDIValue key, MIDIValue velocity );
int MIDIDeviceSendNoteOn( struct MIDIDevice * device, MIDIValue channel, MIDIValue key, MIDIValue velocity );

int MIDIDeviceReceivePolyphonicKeyPressure( struct MIDIDevice * device, MIDIValue channel, MIDIValue key, MIDIValue velocity );
int MIDIDeviceSendPolyphonicKeyPressure( struct MIDIDevice * device, MIDIValue channel, MIDIValue key, MIDIValue velocity );

int MIDIDeviceReceiveControlChange( struct MIDIDevice * device, MIDIValue channel, MIDIValue control, MIDIValue value );
int MIDIDeviceSendControlChange( struct MIDIDevice * device, MIDIValue channel, MIDIValue control, MIDIValue value );

int MIDIDeviceReceiveProgramChange( struct MIDIDevice * device, MIDIValue channel, MIDIValue program );
int MIDIDeviceSendProgramChange( struct MIDIDevice * device, MIDIValue channel, MIDIValue program );

int MIDIDeviceReceiveChannelPressure( struct MIDIDevice * device, MIDIValue channel, MIDIValue value );
int MIDIDeviceSendChannelPressure( struct MIDIDevice * device, MIDIValue channel, MIDIValue value );

int MIDIDeviceReceivePitchWheelChange( struct MIDIDevice * device, MIDIValue channel, MIDILongValue value );
int MIDIDeviceSendPitchWheelChange( struct MIDIDevice * device, MIDIValue channel, MIDILongValue value );
#endif
