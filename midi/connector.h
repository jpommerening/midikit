#ifndef MIDIKIT_MIDI_CONNECTOR_H
#define MIDIKIT_MIDI_CONNECTOR_H

struct MIDIDevice;
struct MIDIDriver;
struct MIDIMessage;

struct MIDIConnector;
struct MIDIConnectorDelegate {
  int (*relay)( void * target, struct MIDIMessage * message );
  void (*retain)( void * target );
  void (*release)( void * target );
};

struct MIDIConnector * MIDIConnectorCreate();
void MIDIConnectorDestroy( struct MIDIConnector * connector );
void MIDIConnectorRetain( struct MIDIConnector * connector );
void MIDIConnectorRelease( struct MIDIConnector * connector );

int MIDIConnectorDetach( struct MIDIConnector * connector );
int MIDIConnectorAttachWithDelegate( struct MIDIConnector * connector, void * target,
                                     struct MIDIConnectorDelegate * delegate );
int MIDIConnectorAttachDevice( struct MIDIConnector * connector, struct MIDIDevice * device );
int MIDIConnectorAttachDriver( struct MIDIConnector * connector, struct MIDIDriver * driver );

int MIDIConnectorRelay( struct MIDIConnector * connector, struct MIDIMessage * message );

#endif
