#ifndef MIDIKIT_MIDI_CONNECTOR_H
#define MIDIKIT_MIDI_CONNECTOR_H

struct MIDIDevice;
struct MIDIDriver;
struct MIDIMessage;

struct MIDIConnector;
struct MIDIConnectorTargetDelegate {
  int (*relay)( void * target, struct MIDIMessage * message );
  int (*connect)( void * target, struct MIDIConnector * connector );
  int (*disconnect)( void * target, struct MIDIConnector * connector );
};
struct MIDIConnectorSourceDelegate {
  int (*connect)( void * target, struct MIDIConnector * connector );
  int (*disconnect)( void * source, struct MIDIConnector * connector );
};

struct MIDIConnector * MIDIConnectorCreate();
void MIDIConnectorDestroy( struct MIDIConnector * connector );
void MIDIConnectorRetain( struct MIDIConnector * connector );
void MIDIConnectorRelease( struct MIDIConnector * connector );

int MIDIConnectorDetachTarget( struct MIDIConnector * connector );
int MIDIConnectorAttachTargetWithDelegate( struct MIDIConnector * connector, void * target,
                                           struct MIDIConnectorTargetDelegate * delegate );

int MIDIConnectorDetachSource( struct MIDIConnector * connector );
int MIDIConnectorAttachSourceWithDelegate( struct MIDIConnector * connector, void * source,
                                           struct MIDIConnectorSourceDelegate * delegate );

int MIDIConnectorAttachToDeviceIn( struct MIDIConnector * connector, struct MIDIDevice * device );
int MIDIConnectorAttachToDriver( struct MIDIConnector * connector, struct MIDIDriver * driver );

int MIDIConnectorAttachFromDeviceOut( struct MIDIConnector * connector, struct MIDIDevice * device );
int MIDIConnectorAttachFromDeviceThru( struct MIDIConnector * connector, struct MIDIDevice * device );
int MIDIConnectorAttachFromDriver( struct MIDIConnector * connector, struct MIDIDriver * driver );

int MIDIConnectorRelay( struct MIDIConnector * connector, struct MIDIMessage * message );

#endif
