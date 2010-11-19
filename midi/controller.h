#ifndef MIDIKIT_MIDI_CONTROLLER_H
#define MIDIKIT_MIDI_CONTROLLER_H
#include "midi.h"
#include "device.h"

/**
 * Various MIDI control numbers, legal values for MIDIControl variables.
 */
//@{

#define MIDI_CONTROL_BANK_SELECT           0x00
#define MIDI_CONTROL_MODULATION_WHEEL      0x01
#define MIDI_CONTROL_BREATH_CONTROLLER     0x02
#define MIDI_CONTROL_UNDEFINED0            0x03
#define MIDI_CONTROL_FOOT_CONTROLLER       0x04
#define MIDI_CONTROL_PORTAMENTO_TIME       0x05
#define MIDI_CONTROL_DATA_ENTRY_MSB        0x06
#define MIDI_CONTROL_CHANNEL_VOLUME        0x07
#define MIDI_CONTROL_BALANCE               0x08
#define MIDI_CONTROL_UNDEFINED1            0x09
#define MIDI_CONTROL_PAN                   0x0a
#define MIDI_CONTROL_EXPRESSION_CONTROLLER 0x0b
#define MIDI_CONTROL_EFFECT_CONTROL1       0x0c
#define MIDI_CONTROL_EFFECT_CONTROL2       0x0d
#define MIDI_CONTROL_UNDEFINED2            0x0e
#define MIDI_CONTROL_UNDEFINED3            0x0f
#define MIDI_CONTROL_GENERAL_PURPOSE_CONTROLLER1 0x10
#define MIDI_CONTROL_GENERAL_PURPOSE_CONTROLLER2 0x11
#define MIDI_CONTROL_GENERAL_PURPOSE_CONTROLLER3 0x12
#define MIDI_CONTROL_GENERAL_PURPOSE_CONTROLLER4 0x13
#define MIDI_CONTROL_UNDEFINED4            0x14
#define MIDI_CONTROL_UNDEFINED5            0x15
#define MIDI_CONTROL_UNDEFINED6            0x16
#define MIDI_CONTROL_UNDEFINED7            0x17
#define MIDI_CONTROL_UNDEFINED8            0x18
#define MIDI_CONTROL_UNDEFINED9            0x19
#define MIDI_CONTROL_UNDEFINED10           0x1a
#define MIDI_CONTROL_UNDEFINED11           0x1b
#define MIDI_CONTROL_UNDEFINED12           0x1c
#define MIDI_CONTROL_UNDEFINED13           0x1d
#define MIDI_CONTROL_UNDEFINED14           0x1e
#define MIDI_CONTROL_UNDEFINED15           0x1f

#define MIDI_CONTROL_DAMPLER_PEDAL         0x40
#define MIDI_CONTROL_PORTAMENTO            0x41
#define MIDI_CONTROL_SOSTENUTO             0x42
#define MIDI_CONTROL_SOFT_PEDAL            0x43
#define MIDI_CONTROL_LEGATO_FOOTSWITCH     0x44
#define MIDI_CONTROL_HOLD2                 0x45
#define MIDI_CONTROL_SOUND_CONTROLLER1     0x46
#define MIDI_CONTROL_SOUND_CONTROLLER2     0x47
#define MIDI_CONTROL_SOUND_CONTROLLER3     0x48
#define MIDI_CONTROL_SOUND_CONTROLLER4     0x49
#define MIDI_CONTROL_SOUND_CONTROLLER5     0x4a
#define MIDI_CONTROL_SOUND_CONTROLLER6     0x4b
#define MIDI_CONTROL_SOUND_CONTROLLER7     0x4c
#define MIDI_CONTROL_SOUND_CONTROLLER8     0x4d
#define MIDI_CONTROL_SOUND_CONTROLLER9     0x4e
#define MIDI_CONTROL_SOUND_CONTROLLER10    0x4f
#define MIDI_CONTROL_GENERAL_PURPOSE_CONTROLLER5 0x50
#define MIDI_CONTROL_GENERAL_PURPOSE_CONTROLLER6 0x51
#define MIDI_CONTROL_GENERAL_PURPOSE_CONTROLLER7 0x52
#define MIDI_CONTROL_GENERAL_PURPOSE_CONTROLLER8 0x53
#define MIDI_CONTROL_PORTAMENTO_CONTROL    0x54

#define MIDI_CONTROL_ALL_SOUND_OFF         0x78
#define MIDI_CONTROL_RESET_ALL_CONTROLLERS 0x79
#define MIDI_CONTROL_LOCAL_CONTROL         0x7a
#define MIDI_CONTROL_ALL_NOTES_OFF         0x7b
#define MIDI_CONTROL_OMNI_MODE_OFF         0x7c
#define MIDI_CONTROL_OMNI_MODE_ON          0x7d
#define MIDI_CONTROL_MONO_MODE_ON          0x7e
#define MIDI_CONTROL_POLY_MODE_ON          0x7f

//@}


struct MIDIController;
struct MIDIControllerDelegate {
  int (*recv_b)( struct MIDIController * controller, struct MIDIDevice * device, MIDIControl control, MIDIBoolean value );
  int (*recv_v)( struct MIDIController * controller, struct MIDIDevice * device, MIDIControl control, MIDIValue value );
  int (*recv_lv)( struct MIDIController * controller, struct MIDIDevice * device, MIDIControl control, MIDILongValue value );
};

struct MIDIController * MIDIControllerCreate( struct MIDIControllerDelegate * delegate );
void MIDIControllerDestroy( struct MIDIController * controller );
void MIDIControllerRetain( struct MIDIController * controller );
void MIDIControllerRelease( struct MIDIController * controller );

#endif
