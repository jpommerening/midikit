#include <stdlib.h>
#include "midi.h"
#include "controller.h"

#define N_LV_CONTROLS 32
#define N_BV_CONTROLS 6
#define N_SV_CONTROLS 26

static char * _lv_control_names[] = {
  "Bank Select",
  "Modulation Wheel",
  "Breath Controller",
  "Undefined (0)",
  "Foot Controller",
  "Portamento Time",
  "Data Entry",
  "Channel Volume",
  "Balance",
  "Undefined (1)",
  "Pan",
  "Expression Controller",
  "Effect Control 1",
  "Effect Control 2"
  "Undefined (2)",
  "Undefined (3)",
  "General Purpose Controller 1",
  "General Purpose Controller 2",
  "General Purpose Controller 3",
  "General Purpose Controller 4",
  "Undefined (4)",
  "Undefined (5)",
  "Undefined (6)",
  "Undefined (7)",
  "Undefined (8)",
  "Undefined (9)",
  "Undefined (10)",
  "Undefined (11)",
  "Undefined (12)",
  "Undefined (13)",
  "Undefined (14)",
  "Undefined (15)"
};

struct MIDIController {
  size_t refs;
  struct MIDIControllerDelegate * delegate;
  struct MIDIDevice * device;
  MIDIChannel   channel;
  MIDILongValue lv_controls[N_LV_CONTROLS];
  MIDIBoolean   bv_controls[N_BV_CONTROLS];
  MIDIValue     sv_controls[N_SV_CONTROLS];
  MIDILongValue nrpnumber;
  MIDILongValue rpnumber;
};

static int _load_non_registered_parameter( struct MIDIController * controller, MIDILongValue parameter ) {
  controller->lv_controls[MIDI_CONTROL_DATA_ENTRY] = 0;
  controller->nrpnumber = parameter;
  return 0;
}

static int _load_registered_parameter( struct MIDIController * controller, MIDILongValue parameter ) {
  controller->lv_controls[MIDI_CONTROL_DATA_ENTRY] = 0;
  controller->rpnumber = parameter;
  return 0;
}

static int _reset_all_controllers( struct MIDIController * controller ) {
  int i;
  for( i=0; i<N_LV_CONTROLS; i++ ) {
    controller->lv_controls[i] = 0;
  }
  controller->lv_controls[MIDI_CONTROL_CHANNEL_VOLUME] = MIDI_LONG_VALUE(90,0); // GM requirement
  for( i=0; i<N_BV_CONTROLS; i++ ) {
    controller->bv_controls[i] = MIDI_OFF;
  }
  for( i=0; i<N_SV_CONTROLS; i++ ) {
    controller->sv_controls[i] = 0;
  }
  return 0;
}

static int _local_control( struct MIDIController * controller, MIDIBoolean value ) {
  return 0;
}

static int _all_notes_off( struct MIDIController * controller ) {
  return 0;
}

static int _omni_mode( struct MIDIController * controller, MIDIBoolean value ) {
  return 0;
}

struct MIDIController * MIDIControllerCreate( struct MIDIControllerDelegate * delegate, MIDIChannel channel ) {
  struct MIDIController * controller = malloc( sizeof( struct MIDIController ) );
  controller->refs = 1;
  controller->delegate = delegate;
  controller->channel  = channel;
  _reset_all_controllers( controller );
  controller->rpnumber  = 0;
  controller->nrpnumber = 0;
  return controller;
}

void MIDIControllerDestroy( struct MIDIController * controller ) {
  free( controller );
}

void MIDIControllerRetain( struct MIDIController * controller ) {
  controller->refs++;
}

void MIDIControllerRelease( struct MIDIController * controller ) {
  if( ! --controller->refs ) {
    MIDIControllerDestroy( controller );
  }
}

static int _recv_lv( struct MIDIController * controller, int entry, MIDIControl control ) {
  if( controller->delegate != NULL &&
      controller->delegate->recv_lv != NULL )
    return (controller->delegate->recv_lv)( controller, controller->device,
                                            control, controller->lv_controls[entry] );
  return 0;
}

static int _recv_bv( struct MIDIController * controller, int entry, MIDIControl control ) {
  if( controller->delegate != NULL &&
      controller->delegate->recv_bv != NULL )
    return (controller->delegate->recv_bv)( controller, controller->device,
                                            control, controller->bv_controls[entry] );
  return 0;
}

static int _recv_sv( struct MIDIController * controller, int entry, MIDIControl control ) {
  if( controller->delegate != NULL &&
      controller->delegate->recv_sv != NULL )
    return (controller->delegate->recv_sv)( controller, controller->device,
                                            control, controller->sv_controls[entry] );
  return 0;
}

int MIDIControllerReceiveControlChange( struct MIDIController * controller, MIDIChannel channel,
                                        MIDIControl control, MIDIValue value ) {
  MIDILongValue lv;
  if( channel != controller->channel ) return 0;
  if( control >= MIDI_CONTROL_BANK_SELECT && control <= MIDI_CONTROL_UNDEFINED15 ) {
    lv = MIDI_LONG_VALUE( value, MIDI_LSB(controller->lv_controls[(int)control]) );
    controller->lv_controls[(int)control] = lv;
    _recv_lv( controller, control, control );
  } else if( control >= MIDI_CONTROL_BANK_SELECT+32 && control <= MIDI_CONTROL_UNDEFINED15+32 ) {
    lv = MIDI_LONG_VALUE( MIDI_MSB(controller->lv_controls[(int)control-32]), value );
    controller->lv_controls[(int)control-32] = lv;
    _recv_lv( controller, control-32, control );
  } else if( control >= MIDI_CONTROL_DAMPER_PEDAL && control <= MIDI_CONTROL_HOLD2 ) {
    controller->bv_controls[(int)control-MIDI_CONTROL_DAMPER_PEDAL] = MIDI_BOOL( value );
    _recv_bv( controller, control-MIDI_CONTROL_DAMPER_PEDAL, control );
  } else if( control >= MIDI_CONTROL_SOUND_CONTROLLER1 && control <= MIDI_CONTROL_EFFECTS5_DEPTH ) {
    if( control < MIDI_CONTROL_UNDEFINED16 || control > MIDI_CONTROL_UNDEFINED21 ) {
      controller->sv_controls[(int)control-MIDI_CONTROL_EFFECTS1_DEPTH] = value;
      _recv_sv( controller, control-MIDI_CONTROL_EFFECTS1_DEPTH, control );
    }
  } else {
    switch( control ) {
      case MIDI_CONTROL_DATA_INCREMENT:
        controller->lv_controls[MIDI_CONTROL_DATA_ENTRY]++;
        return 0;
      case MIDI_CONTROL_DATA_DECREMENT:
        controller->lv_controls[MIDI_CONTROL_DATA_ENTRY]--;
        return 0;
      case MIDI_CONTROL_NON_REGISTERED_PARAMETER_NUMBER:
        lv = MIDI_LONG_VALUE( MIDI_MSB(controller->nrpnumber), value );
        return _load_non_registered_parameter( controller, lv );
      case (MIDI_CONTROL_NON_REGISTERED_PARAMETER_NUMBER+1):
        lv = MIDI_LONG_VALUE( value, MIDI_LSB(controller->nrpnumber) );
        return _load_non_registered_parameter( controller, lv );
      case MIDI_CONTROL_REGISTERED_PARAMETER_NUMBER:
        lv = MIDI_LONG_VALUE( MIDI_MSB(controller->rpnumber), value );
        return _load_registered_parameter( controller, lv );
      case (MIDI_CONTROL_REGISTERED_PARAMETER_NUMBER+1):
        lv = MIDI_LONG_VALUE( value, MIDI_LSB(controller->rpnumber) );
        return _load_registered_parameter( controller, lv );
      case  MIDI_CONTROL_ALL_SOUND_OFF:
        return 0;
      case MIDI_CONTROL_RESET_ALL_CONTROLLERS:
        return _reset_all_controllers( controller );
      case MIDI_CONTROL_LOCAL_CONTROL:
        return _local_control( controller, MIDI_BOOL(value) );
      case MIDI_CONTROL_ALL_NOTES_OFF:
        return _all_notes_off( controller );
      case MIDI_CONTROL_OMNI_MODE_OFF:
        return _omni_mode( controller, MIDI_OFF ) + _all_notes_off( controller );
      case MIDI_CONTROL_OMNI_MODE_ON:
        return _omni_mode( controller, MIDI_ON ) + _all_notes_off( controller );
      default:
        return 1; // unhandled control
    }
  }
  return 0;
}

int MIDIControllerSendControlChange( struct MIDIController * controller, MIDIChannel channel,
                                     MIDIControl control, MIDIValue value ) {
  return 0;
}
