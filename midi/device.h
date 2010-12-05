#ifndef MIDIKIT_MIDI_DEVICE_H
#define MIDIKIT_MIDI_DEVICE_H
#include <stdint.h>
#include "midi.h"

struct MIDIConnector;
struct MIDIMessage;
struct MIDIController;
struct MIDITimer;

struct MIDIDevice;

struct MIDIDeviceDelegate {
  int (*recv_nof)( struct MIDIDevice * device, MIDIChannel channel, MIDIKey key, MIDIVelocity velocity );
  int (*recv_non)( struct MIDIDevice * device, MIDIChannel channel, MIDIKey key, MIDIVelocity velocity );
  int (*recv_pkp)( struct MIDIDevice * device, MIDIChannel channel, MIDIKey key, MIDIPressure pressute );
  int (*recv_cc)( struct MIDIDevice * device, MIDIChannel channel, MIDIControl control, MIDIValue value );
  int (*recv_pc)( struct MIDIDevice * device, MIDIChannel channel, MIDIProgram program );
  int (*recv_cp)( struct MIDIDevice * device, MIDIChannel channel, MIDIPressure value );
  int (*recv_pwc)( struct MIDIDevice * device, MIDIChannel channel, MIDILongValue value );
  int (*recv_sx)( struct MIDIDevice * device, MIDIManufacturerId manufacturer_id,
                  size_t size, void * data, uint8_t fragment );
  int (*recv_tcqf)( struct MIDIDevice * device, MIDIValue time_code_type, MIDIValue value );
  int (*recv_spp)( struct MIDIDevice * device, MIDILongValue value );
  int (*recv_ss)( struct MIDIDevice * device, MIDIValue value );
  int (*recv_tr)( struct MIDIDevice * device );
  int (*recv_eox)( struct MIDIDevice * device );
  int (*recv_rt)( struct MIDIDevice * device, MIDIStatus status, MIDITimestamp );
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

int MIDIDeviceSetBaseChannel( struct MIDIDevice * device, MIDIChannel channel );
int MIDIDeviceGetBaseChannel( struct MIDIDevice * device, MIDIChannel * channel );

int MIDIDeviceSetTimer( struct MIDIDevice * device, struct MIDITimer * timer );
int MIDIDeviceGetTimer( struct MIDIDevice * device, struct MIDITimer ** timer );

/*int MIDIDeviceSetChannelInstrument( struct MIDIDevice * device, MIDIChannel channel, struct MIDIInstrument * instrument );*/
/*int MIDIDeviceGetChannelInstrument( struct MIDIDevice * device, MIDIChannel channel, struct MIDIInstrument ** instrument );*/

int MIDIDeviceSetChannelController( struct MIDIDevice * device, MIDIChannel channel, struct MIDIController * controller );
int MIDIDeviceGetChannelController( struct MIDIDevice * device, MIDIChannel channel, struct MIDIController ** controller );

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

int MIDIDeviceReceiveSystemExclusive( struct MIDIDevice * device, MIDIManufacturerId manufacturer_id,
                                      size_t size, void * data, uint8_t fragment );
int MIDIDeviceSendSystemExclusive( struct MIDIDevice * device, MIDIManufacturerId manufacturer_id,
                                   size_t size, void * data, uint8_t fragment );
                                   
int MIDIDeviceReceiveTimeCodeQuarterFrame( struct MIDIDevice * device, MIDIValue time_code_type, MIDIValue value );
int MIDIDeviceSendTimeCodeQuarterFrame( struct MIDIDevice * device, MIDIValue time_code_type, MIDIValue value );

int MIDIDeviceReceiveSongPositionPointer( struct MIDIDevice * device, MIDILongValue value );
int MIDIDeviceSendSongPositionPointer( struct MIDIDevice * device, MIDILongValue value );

int MIDIDeviceReceiveSongSelect( struct MIDIDevice * device, MIDIValue value );
int MIDIDeviceSendSongSelect( struct MIDIDevice * device, MIDIValue value );

int MIDIDeviceReceiveTuneRequest( struct MIDIDevice * device );
int MIDIDeviceSendTuneRequest( struct MIDIDevice * device );

int MIDIDeviceReceiveEndOfExclusive( struct MIDIDevice * device );
int MIDIDeviceSendEndOfExclusive( struct MIDIDevice * device );

int MIDIDeviceReceiveRealTime( struct MIDIDevice * device, MIDIStatus status, MIDITimestamp timestamp );
int MIDIDeviceSendRealTime( struct MIDIDevice * device, MIDIStatus status, MIDITimestamp timestamp );

#endif
