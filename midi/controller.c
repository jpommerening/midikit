#include <stdlib.h>
#include "midi.h"
#include "controller.h"

#define N_CONTROLS 128
#define N_LV_CONTROLS 32
#define N_BV_CONTROLS 6
#define N_SV_CONTROLS 26

/*
static char * _control_names[] = {
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
*/

struct MIDINRPList;

struct MIDIController {
  size_t refs;
  struct MIDIControllerDelegate * delegate;

  MIDILongValue current_parameter;
  MIDIBoolean   current_parameter_registered;
  MIDIValue controls[N_CONTROLS];
  MIDIValue registered_parameters[6];
  struct MIDINRPList * list;
};

/**
 * @internal
 * Methods for accessing parameters, resetting controllers, etc.
 * @{
 */

struct MIDINRPList {
  struct MIDINonRegisteredParameter * parameter;
  struct MIDINRPList * next;
};

static int _load_non_registered_parameter( struct MIDIController * controller ) {
  MIDILongValue parameter = MIDI_LONG_VALUE( controller->controls[MIDI_CONTROL_NON_REGISTERED_PARAMETER_NUMBER+1],
                                             controller->controls[MIDI_CONTROL_NON_REGISTERED_PARAMETER_NUMBER] );
  struct MIDINRPList * list = controller->list;
  if( controller->current_parameter_registered == MIDI_ON ) return 1;
  if( parameter == controller->current_parameter )          return 0;
  if( parameter == MIDI_CONTROL_RPN_RESET ) {
    controller->controls[MIDI_CONTROL_DATA_ENTRY]    = 0x7f;
    controller->controls[MIDI_CONTROL_DATA_ENTRY+32] = 0x7f;
    controller->current_parameter = MIDI_CONTROL_RPN_RESET;
    controller->current_parameter_registered = MIDI_OFF;
    return 0;
  }
  while( list != NULL ) {
    if( list->parameter != NULL && list->parameter->number == parameter ) {
       controller->controls[MIDI_CONTROL_DATA_ENTRY]    = MIDI_MSB( list->parameter->value );
       controller->controls[MIDI_CONTROL_DATA_ENTRY+32] = MIDI_LSB( list->parameter->value );
       controller->current_parameter = parameter;
       controller->current_parameter_registered = MIDI_OFF;
       return 0;
    }
  }
  return 1;
}

static int _store_non_registered_parameter( struct MIDIController * controller ) {
  MIDILongValue parameter = controller->current_parameter;
  struct MIDINRPList * list = controller->list;
  if( controller->current_parameter_registered == MIDI_ON ) return 1;
  while( list != NULL ) {
    if( list->parameter != NULL && list->parameter->number == parameter ) {
       list->parameter->value = MIDI_LONG_VALUE( controller->controls[MIDI_CONTROL_DATA_ENTRY],
                                                 controller->controls[MIDI_CONTROL_DATA_ENTRY+32] );
       return 0;
    }
  }
  return 1;
}

static int _load_registered_parameter( struct MIDIController * controller ) {
  MIDILongValue parameter = MIDI_LONG_VALUE( controller->controls[MIDI_CONTROL_REGISTERED_PARAMETER_NUMBER+1],
                                             controller->controls[MIDI_CONTROL_REGISTERED_PARAMETER_NUMBER] );
  if( controller->current_parameter_registered == MIDI_OFF ) return 1;
  if( parameter == controller->current_parameter )           return 0;
  if( parameter == MIDI_CONTROL_RPN_RESET ) {
    controller->controls[MIDI_CONTROL_DATA_ENTRY]    = 0x7f;
    controller->controls[MIDI_CONTROL_DATA_ENTRY+32] = 0x7f;
    controller->current_parameter = MIDI_CONTROL_RPN_RESET;
    controller->current_parameter_registered = MIDI_ON;
    return 0;
  }
  if( parameter >= MIDI_CONTROL_RPN_PITCH_BEND_RANGE && parameter <= MIDI_CONTROL_RPN_COARSE_TUNING ) {
    controller->controls[MIDI_CONTROL_DATA_ENTRY]    = controller->registered_parameters[parameter*2];
    controller->controls[MIDI_CONTROL_DATA_ENTRY+32] = controller->registered_parameters[parameter*2+1];
    controller->current_parameter = parameter;
    controller->current_parameter_registered = MIDI_ON;
    return 0;
  }
  return 1;
}

static int _store_registered_parameter( struct MIDIController * controller ) {
  MIDILongValue parameter = controller->current_parameter;
  if( controller->current_parameter_registered == MIDI_OFF ) return 1;
  if( parameter == MIDI_CONTROL_RPN_RESET ) return 0;
  if( parameter >= MIDI_CONTROL_RPN_PITCH_BEND_RANGE && parameter <= MIDI_CONTROL_RPN_COARSE_TUNING ) {
    controller->registered_parameters[parameter*2]   = controller->controls[MIDI_CONTROL_DATA_ENTRY];
    controller->registered_parameters[parameter*2+1] = controller->controls[MIDI_CONTROL_DATA_ENTRY+32];
    return 0;
  }
  return 1;
}

static int _load_current_parameter( struct MIDIController * controller ) {
  if( controller->current_parameter_registered == MIDI_OFF ) {
    return _load_non_registered_parameter( controller );
  } else {
    return _load_registered_parameter( controller );
  }
}

static int _store_current_parameter( struct MIDIController * controller ) {
  if( controller->current_parameter_registered == MIDI_OFF ) {
    return _store_non_registered_parameter( controller );
  } else {
    return _store_registered_parameter( controller );
  }
}

static int _reset_controls_for_gm( struct MIDIController * controller ) {
  controller->controls[MIDI_CONTROL_CHANNEL_VOLUME]        = 100;
  controller->controls[MIDI_CONTROL_EXPRESSION_CONTROLLER] = 127;
  controller->controls[MIDI_CONTROL_PAN]                   =  64;
  return 0;
}

static int _reset_controls( struct MIDIController * controller ) {
  controller->controls[MIDI_CONTROL_EXPRESSION_CONTROLLER] = 127;

  controller->controls[MIDI_CONTROL_DATA_ENTRY]    = 0x7f;
  controller->controls[MIDI_CONTROL_DATA_ENTRY+32] = 0x7f;
  controller->controls[MIDI_CONTROL_NON_REGISTERED_PARAMETER_NUMBER]   = 0x7f;
  controller->controls[MIDI_CONTROL_NON_REGISTERED_PARAMETER_NUMBER+1] = 0x7f;
  controller->controls[MIDI_CONTROL_REGISTERED_PARAMETER_NUMBER]       = 0x7f;
  controller->controls[MIDI_CONTROL_REGISTERED_PARAMETER_NUMBER+1]     = 0x7f;

  controller->current_parameter             = MIDI_CONTROL_RPN_RESET;
  controller->current_parameter_registered  = MIDI_OFF;

  controller->registered_parameters[MIDI_CONTROL_RPN_PITCH_BEND_RANGE_SEMITONES] = 2;
  controller->registered_parameters[MIDI_CONTROL_RPN_PITCH_BEND_RANGE_CENTS]     = 0;
  controller->registered_parameters[MIDI_CONTROL_RPN_FINE_TUNING_MSB]            = 0x40;
  controller->registered_parameters[MIDI_CONTROL_RPN_FINE_TUNING_LSB]            = 0;
  controller->registered_parameters[MIDI_CONTROL_RPN_COARSE_TUNING]              = 0x40;
  controller->registered_parameters[MIDI_CONTROL_RPN_COARSE_TUNING+1]            = 0;
  return 0;
}

static int _initialize_controls( struct MIDIController * controller ) {
  int i;
  for( i=0; i<N_CONTROLS; i++ ) {
    controller->controls[i] = 0;
  }
  return _reset_controls( controller ) + _reset_controls_for_gm( controller );
}

static int _all_sound_off( struct MIDIController * controller ) {
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

/** @} */

#pragma mark Creation and destruction
/**
 * @name Creation and destruction
 * Creating, destroying and reference counting of MIDIController objects.
 * @{
 */

/**
 * @brief Create a MIDIController instance.
 * Allocate space and initialize a MIDIController instance.
 * @public @memberof MIDIController
 * @param delegate The delegate to use for the controller. May be @c NULL.
 * @return a pointer to the created controller structure on success.
 * @return a @c NULL pointer if the controller could not created.
 */
struct MIDIController * MIDIControllerCreate( struct MIDIControllerDelegate * delegate ) {
  struct MIDIController * controller = malloc( sizeof( struct MIDIController ) );
  controller->refs = 1;
  controller->delegate = delegate;
  _initialize_controls( controller );
  return controller;
}

/**
 * @brief Destroy a MIDIController instance.
 * Free all resources occupied by the controller.
 * @public @memberof MIDIController
 * @param controller The controller.
 */
void MIDIControllerDestroy( struct MIDIController * controller ) {
  free( controller );
}

/**
 * @brief Retain a MIDIController instance.
 * Increment the reference counter of a controller so that it won't be destroyed.
 * @public @memberof MIDIController
 * @param controller The controller.
 */
void MIDIControllerRetain( struct MIDIController * controller ) {
  controller->refs++;
}

/**
 * @brief Release a MIDIController instance.
 * Decrement the reference counter of a controller. If the reference count
 * reached zero, destroy the controller.
 * @public @memberof MIDIController
 * @param controller The controller.
 */
void MIDIControllerRelease( struct MIDIController * controller ) {
  if( ! --controller->refs ) {
    MIDIControllerDestroy( controller );
  }
}

/** @} */

int MIDIControllerReceiveControlChange( struct MIDIController * controller, struct MIDIDevice * device,
                                        MIDIChannel channel, MIDIControl control, MIDIValue value ) {
  if( control == MIDI_CONTROL_DATA_ENTRY ||
      control == MIDI_CONTROL_DATA_ENTRY+32 ||
      control == MIDI_CONTROL_DATA_INCREMENT ||
      control == MIDI_CONTROL_DATA_DECREMENT ) {
    _load_current_parameter( controller );
    if( control == MIDI_CONTROL_DATA_INCREMENT ) {
      controller->controls[MIDI_CONTROL_DATA_ENTRY+32]++;
    } else if( control == MIDI_CONTROL_DATA_DECREMENT ) {
      controller->controls[MIDI_CONTROL_DATA_ENTRY+32]--;
    } else {
      controller->controls[(int)control] = value;
    }
    _store_current_parameter( controller );
  } else if( control < MIDI_CONTROL_ALL_SOUND_OFF ) {
    controller->controls[(int)control] = value;
    if( control == MIDI_CONTROL_NON_REGISTERED_PARAMETER_NUMBER ||
        control == MIDI_CONTROL_NON_REGISTERED_PARAMETER_NUMBER+1 ) {
      controller->current_parameter_registered = MIDI_OFF;
    }
    if( control == MIDI_CONTROL_REGISTERED_PARAMETER_NUMBER ||
        control == MIDI_CONTROL_REGISTERED_PARAMETER_NUMBER+1 ) {
      controller->current_parameter_registered = MIDI_ON;
    }
  } else {
    switch( control ) {
      case  MIDI_CONTROL_ALL_SOUND_OFF:
        return _all_sound_off( controller );
      case MIDI_CONTROL_RESET_ALL_CONTROLLERS:
        return _reset_controls( controller );
      case MIDI_CONTROL_LOCAL_CONTROL:
        return _local_control( controller, MIDI_BOOL(value) );
      case MIDI_CONTROL_ALL_NOTES_OFF:
        return _all_notes_off( controller );
      case MIDI_CONTROL_OMNI_MODE_OFF:
        return _omni_mode( controller, MIDI_OFF ) + _all_notes_off( controller );
      case MIDI_CONTROL_OMNI_MODE_ON:
        return _omni_mode( controller, MIDI_ON )  + _all_notes_off( controller );
    }
  }
  return 0;
}

int MIDIControllerSendControlChange( struct MIDIController * controller, struct MIDIDevice * device,
                                     MIDIChannel channel, MIDIControl control, MIDIValue value ) {
  return 0;
}
