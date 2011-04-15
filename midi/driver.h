#ifndef MIDIKIT_MIDI_DRIVER_H
#define MIDIKIT_MIDI_DRIVER_H
#include "midi.h"

struct MIDIMessage;
struct MIDIConnector;
struct MIDIPort;

struct MIDIDriver;

#define MIDI_DRIVER_WILL_SEND_MESSAGE 0
#define MIDI_DRIVER_WILL_RECEIVE_MESSAGE 1
#define MIDI_DRIVER_NUM_EVENT_TYPES 2
/*
struct MIDIDriverDelegate {
  int (*send)( void * implementation, struct MIDIMessage * message );
  int (*receive)( void * interface, struct MIDIMessage * message );
  int (*event)( void * observer, void * interface, void * implementation, int event, void * info );
  void * implementation;
  void * interface;
  void * observer;
};
*/
#ifdef MIDI_DRIVER_INTERNALS
#include "runloop.h"
struct MIDIDriver {
  size_t refs;                             /**< @private */
  struct MIDIRunloopSource * rls;          /**< @private */
  struct MIDIPort * port;                  /**< @private */
  struct MIDIClock * clock;                /**< @private */
  int (*send)( void * driver, struct MIDIMessage * message );
  void (*destroy)( void * driver );
};
#endif
/*
int MIDIDriverDelegateSendMessage( struct MIDIDriverDelegate * delegate, struct MIDIMessage * message );
int MIDIDriverDelegateReceiveMessage( struct MIDIDriverDelegate * delegate, struct MIDIMessage * message );
int MIDIDriverDelegateTriggerEvent( struct MIDIDriverDelegate * delegate, int event, void * info );

#define MIDI_DRIVER_DELEGATE_INITIALIZER { NULL, NULL, NULL, NULL, NULL, NULL }
*/

struct MIDIDriver * MIDIDriverCreate( char * name, MIDISamplingRate rate );
void MIDIDriverInit( struct MIDIDriver * driver, char * name, MIDISamplingRate rate );
void MIDIDriverDestroy( struct MIDIDriver * driver );
void MIDIDriverRetain( struct MIDIDriver * driver );
void MIDIDriverRelease( struct MIDIDriver * driver );

int MIDIDriverMakeLoopback( struct MIDIDriver * driver );

int MIDIDriverGetInputPort( struct MIDIDriver * driver, struct MIDIPort ** port );
int MIDIDriverGetOutputPort( struct MIDIDriver * driver, struct MIDIPort ** port );
int MIDIDriverProvideReceiveConnector( struct MIDIDriver * driver, struct MIDIConnector ** receive );
int MIDIDriverProvideSendConnector( struct MIDIDriver * driver, struct MIDIConnector ** send );

int MIDIDriverSend( struct MIDIDriver * driver, struct MIDIMessage * message );
int MIDIDriverReceive( struct MIDIDriver * driver, struct MIDIMessage * message );
int MIDIDriverTriggerEvent( struct MIDIDriver * driver, int type, size_t size, void * data );

int MIDIDriverStartProfiling( struct MIDIDriver * driver );
int MIDIDriverGetProfilingStats( struct MIDIDriver * driver, void * stats );
int MIDIDriverStopProfiling( struct MIDIDriver * driver );

#endif
