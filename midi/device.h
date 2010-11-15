#ifndef MIDIKIT_MIDI_DEVICE_H
#define MIDIKIT_MIDI_DEVICE_H
#include "midi.h"

struct MIDIConnector;
struct MIDIMessage;

struct MIDIDevice;
struct MIDIDeviceDelegate {
  int (*recv_nof)( struct MIDIDevice * device, MIDIChannel channel, MIDIKey key, MIDIVelocity velocity );  ///< Note on callback
  int (*recv_non)( struct MIDIDevice * device, MIDIChannel channel, MIDIKey key, MIDIVelocity velocity );  ///< Note off callback
  int (*recv_pkp)( struct MIDIDevice * device, MIDIChannel channel, MIDIKey key, MIDIPressure pressute );  ///< Polyphonic key pressure callback
  int (*recv_cc)( struct MIDIDevice * device, MIDIChannel channel, MIDIControl control, MIDIValue value ); ///< Control change callback
  int (*recv_pc)( struct MIDIDevice * device, MIDIChannel channel, MIDIProgram program );                  ///< Program change callback
  int (*recv_cp)( struct MIDIDevice * device, MIDIChannel channel, MIDIPressure value );                   ///< Channel pressure callback
  int (*recv_pwc)( struct MIDIDevice * device, MIDIChannel channel, MIDILongValue value );                 ///< Pitch wheel change callback
};

struct MIDIDevice * MIDIDeviceCreate( struct MIDIDeviceDelegate * delegate );
void MIDIDeviceDestroy( struct MIDIDevice * device );
void MIDIDeviceRetain( struct MIDIDevice * device );
void MIDIDeviceRelease( struct MIDIDevice * device );

int MIDIDeviceDetachIn( struct MIDIDevice * device );
int MIDIDeviceAttachIn( struct MIDIDevice * device, struct MIDIConnector * in );

int MIDIDeviceDetachOut( struct MIDIDevice * device );
int MIDIDeviceAttachOut( struct MIDIDevice * device, struct MIDIConnector * out );

int MIDIDeviceDetachThru( struct MIDIDevice * device );
int MIDIDeviceAttachThru( struct MIDIDevice * device, struct MIDIConnector * thru );

int MIDIDeviceReceive( struct MIDIDevice * device, struct MIDIMessage * message );
int MIDIDeviceSend( struct MIDIDevice * device, struct MIDIMessage * message );

int MIDIDeviceReceiveNoteOff( struct MIDIDevice * device, MIDIChannel channel, MIDIKey key, MIDIVelocity velocity );
int MIDIDeviceSendNoteOff( struct MIDIDevice * device, MIDIChannel channel, MIDIKey key, MIDIVelocity velocity );

int MIDIDeviceReceiveNoteOn( struct MIDIDevice * device, MIDIChannel channel, MIDIKey key, MIDIVelocity velocity );
int MIDIDeviceSendNoteOn( struct MIDIDevice * device, MIDIChannel channel, MIDIKey key, MIDIVelocity velocity );

int MIDIDeviceReceivePolyphonicKeyPressure( struct MIDIDevice * device, MIDIChannel channel, MIDIKey key, MIDIPressure pressure );
int MIDIDeviceSendPolyphonicKeyPressure( struct MIDIDevice * device, MIDIChannel channel, MIDIKey key, MIDIPressure pressure );

int MIDIDeviceReceiveControlChange( struct MIDIDevice * device, MIDIChannel channel, MIDIControl control, MIDIValue value );
int MIDIDeviceSendControlChange( struct MIDIDevice * device, MIDIChannel channel, MIDIControl control, MIDIValue value );

int MIDIDeviceReceiveProgramChange( struct MIDIDevice * device, MIDIChannel channel, MIDIProgram program );
int MIDIDeviceSendProgramChange( struct MIDIDevice * device, MIDIChannel channel, MIDIProgram program );

int MIDIDeviceReceiveChannelPressure( struct MIDIDevice * device, MIDIChannel channel, MIDIPressure pressure );
int MIDIDeviceSendChannelPressure( struct MIDIDevice * device, MIDIChannel channel, MIDIPressure pressure );

int MIDIDeviceReceivePitchWheelChange( struct MIDIDevice * device, MIDIChannel channel, MIDILongValue value );
int MIDIDeviceSendPitchWheelChange( struct MIDIDevice * device, MIDIChannel channel, MIDILongValue value );
#endif
