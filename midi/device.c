#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>

#include "device.h"

#include "type.h"
#include "port.h"
#include "message.h"
#include "controller.h"
#include "timer.h"

#define N_CHANNEL 16


/**
 * @ingroup MIDI
 * @struct MIDIDeviceDelegate device.h
 * @brief Delegate to respond to received messages.
 * Every time the device receives a message, the device calls one of these
 * callbacks (if available) with the message properties.
 */
/**
 * @public @property MIDIDeviceDelegate::recv_nof
 * @brief Note off callback.
 */
/**
 * @public @property MIDIDeviceDelegate::recv_non
 * @brief Note on callback.
 */
/**
 * @public @property MIDIDeviceDelegate::recv_pkp
 * @brief Polyphonic key pressure callback.
 */
/**
 * @public @property MIDIDeviceDelegate::recv_cc
 * @brief Control change callback.
 */
/**
 * @public @property MIDIDeviceDelegate::recv_pc
 * @brief Program change callback.
 */
/**
 * @public @property MIDIDeviceDelegate::recv_cp
 * @brief Channel pressure callback.
 */
/**
 * @public @property MIDIDeviceDelegate::recv_pwc
 * @brief Pitch wheel change callback.
 */
/**
 * @public @property MIDIDeviceDelegate::recv_sx
 * @brief System exclusive callback.
 */
/**
 * @public @property MIDIDeviceDelegate::recv_tcqf
 * @brief Time code quarter frame callback.
 */
/**
 * @public @property MIDIDeviceDelegate::recv_spp
 * @brief Song position pointer callback.
 */
/**
 * @public @property MIDIDeviceDelegate::recv_ss
 * @brief Song select callback.
 */
/**
 * @public @property MIDIDeviceDelegate::recv_tr
 * @brief Tune request callback.
 */
/**
 * @public @property MIDIDeviceDelegate::recv_eox
 * @brief End of exclusive callback.
 */
/**
 * @public @property MIDIDeviceDelegate::recv_rt
 * @brief Real time callback.
 */

/**
 * @ingroup MIDI
 * @struct MIDIDevice device.h
 * @brief An abstract MIDI device.
 * The MIDIDevice class provides a skeleton for customized MIDI devices.
 * Standardized functionality can be plugged in by using MIDITimer,
 * MIDIController and MIDIInstrument objects.
 * Every MIDIDevice is equipped with an input, an output and a thru-port
 * that forwards every MIDIMessage received on the input.
 */
struct MIDIDevice {
  int    refs;
  struct MIDIDeviceDelegate * delegate;
  struct MIDIPort * in;
  struct MIDIPort * out;
  MIDIChannel base_channel;
  MIDIBoolean omni_mode;
  MIDIBoolean poly_mode;
  struct MIDITimer      * timer;
/*struct MIDIInstrument * instrument[N_CHANNEL];  **< @private */
  struct MIDIController * controller[N_CHANNEL];
};

#pragma mark Creation and destruction
/**
 * @name Creation and destruction
 * Creating, destroying and reference counting of MIDIDevice objects.
 * @{
 */

static int _recv( void * dev, void * source, struct MIDITypeSpec * type, void * data );

/**
 * @brief Create a MIDIDevice instance.
 * Allocate space and initialize a MIDIDevice instance.
 * @public @memberof MIDIDevice
 * @param delegate The delegate to use for the device. May be @c NULL.
 * @return a pointer to the created device structure on success.
 * @return a @c NULL pointer if the device could not created.
 */
struct MIDIDevice * MIDIDeviceCreate( struct MIDIDeviceDelegate * delegate ) {
  struct MIDIDevice * device = malloc( sizeof( struct MIDIDevice ) );
  MIDIChannel channel;

  device->refs = 1;
  device->in   = MIDIPortCreate( "Device IN",  MIDI_PORT_IN | MIDI_PORT_THRU, device, &_recv );
  device->out  = MIDIPortCreate( "Device OUT", MIDI_PORT_OUT, device, NULL );
/*device->in   = NULL;
  device->out  = NULL;
  device->thru = NULL;*/
  device->delegate     = delegate;
  device->base_channel = MIDI_CHANNEL_1;
  device->omni_mode    = MIDI_OFF;
  device->poly_mode    = MIDI_ON;
  device->timer        = NULL;
  for( channel=MIDI_CHANNEL_1; channel<=MIDI_CHANNEL_16; channel++ ) {
  /*device->instrument[(int)channel] = NULL;*/
    device->controller[(int)channel] = NULL;
  }
  return device;
}

/**
 * @brief Destroy a MIDIDevice instance.
 * Free all resources occupied by the device and release all referenced objects.
 * @public @memberof MIDIDevice
 * @param device The device.
 */
void MIDIDeviceDestroy( struct MIDIDevice * device ) {
  MIDIChannel channel;

/*MIDIDeviceDetachIn( device );
  MIDIDeviceDetachOut( device );
  MIDIDeviceDetachThru( device );*/

  MIDIPortInvalidate( device->in );
  MIDIPortInvalidate( device->out );
  MIDIPortRelease( device->in );
  MIDIPortRelease( device->out );

  if( device->timer != NULL ) MIDITimerRelease( device->timer );
  for( channel=MIDI_CHANNEL_1; channel<=MIDI_CHANNEL_16; channel++ ) {
  /*if( device->instrument[(int)channel] != NULL ) MIDIInstrumentRelease( device->instrument[(int)channel] );;*/
    if( device->controller[(int)channel] != NULL ) MIDIControllerRelease( device->controller[(int)channel] );;
  }
  free( device );
}

/**
 * @brief Retain a MIDIDevice instance.
 * Increment the reference counter of a device so that it won't be destroyed.
 * @public @memberof MIDIDevice
 * @param device The device.
 */
void MIDIDeviceRetain( struct MIDIDevice * device ) {
  device->refs++;
}

/**
 * @brief Release a MIDIDevice instance.
 * Decrement the reference counter of a device. If the reference count
 * reached zero, destroy the device.
 * @public @memberof MIDIDevice
 * @param device The device.
 */
void MIDIDeviceRelease( struct MIDIDevice * device ) {
  if( ! --device->refs ) {
    MIDIDeviceDestroy( device );
  }
}

/** @} */

#pragma mark Port access
/**
 * @name Port access
 * Methods to access the devices @c IN, @c OUT and @c THRU ports.
 * @{
 */

/**
 * @brief Get the device's @c IN port.
 * @public @memberof MIDIDevice
 * @param device The device.
 * @param port   The @c IN port.
 * @retval 0 on success.
 */
int MIDIDeviceGetInputPort( struct MIDIDevice * device, struct MIDIPort ** port ) {
  MIDIPrecond( device != NULL, EFAULT );
  MIDIPrecond( port != NULL, EINVAL );
  *port = device->in;
  return 0;
}

/**
 * @brief Get the device's @c OUT port.
 * @public @memberof MIDIDevice
 * @param device The device.
 * @param port   The @c OUT port.
 * @retval 0 on success.
 */
int MIDIDeviceGetOutputPort( struct MIDIDevice * device, struct MIDIPort ** port ) {
  MIDIPrecond( device != NULL, EFAULT );
  MIDIPrecond( port != NULL, EINVAL );
  *port = device->out;
  return 0;
}

/**
 * @brief Get the device's @c THRU port.
 * @public @memberof MIDIDevice
 * @param device The device.
 * @param port   The @c THRU port.
 * @retval 0 on success.
 */
int MIDIDeviceGetThroughPort( struct MIDIDevice * device, struct MIDIPort ** port ) {
  MIDIPrecond( device != NULL, EFAULT );
  MIDIPrecond( port != NULL, EINVAL );
  *port = device->in;
  return 0;
}

#pragma mark Deprecated connector attachment functions
/** @deprecated */
int MIDIDeviceDetachIn( struct MIDIDevice * device ) {
  MIDIPrecond( device != NULL, EFAULT );
  return 0;
}

/** @deprecated */
int MIDIDeviceAttachIn( struct MIDIDevice * device, struct MIDIPort * port ) {
  MIDIPrecond( device != NULL, EFAULT );
  MIDIPrecond( port != NULL, EINVAL );
  MIDIPortConnect( port, device->in );
  return 0;
}

/** @deprecated */
int MIDIDeviceDetachOut( struct MIDIDevice * device ) {
  MIDIPrecond( device != NULL, EFAULT );
  return MIDIPortDisconnectAll( device->out );
}

/** @deprecated */
int MIDIDeviceAttachOut( struct MIDIDevice * device, struct MIDIPort * port ) {
  MIDIPrecond( device != NULL, EFAULT );
  MIDIPrecond( port != NULL, EINVAL );
  MIDIPortConnect( device->out, port );
  return 0;
}

/** @deprecated */
int MIDIDeviceDetachThru( struct MIDIDevice * device ) {
  MIDIPrecond( device != NULL, EFAULT );
  return MIDIPortDisconnectAll( device->in );
}

/** @deprecated */
int MIDIDeviceAttachThru( struct MIDIDevice * device, struct MIDIPort * port ) {
  MIDIPrecond( device != NULL, EFAULT );
  MIDIPrecond( port != NULL, EINVAL );
  MIDIPortConnect( device->in, port );
  return 0;
}

/** @} */

#pragma mark Property access
/**
 * @name Property access
 * Get and set device properties.
 * @{
 */

/**
 * @brief Set the device's base channel.
 * Every MIDI device has a base channel. The base channel is used to reply to
 * certain messages like the "Control Change" message for MIDI-Modes "Omni" and
 * "Polyphonic". Even in Omni-Mode the device expects to receive those messages
 * only on the base channel.
 * @public @memberof MIDIDevice
 * @param device  The device.
 * @param channel The base channel.
 * @retval 0  on success.
 * @retval >0 if the base channel coult not be set.
 */
int MIDIDeviceSetBaseChannel( struct MIDIDevice * device, MIDIChannel channel ) {
  if( channel < MIDI_CHANNEL_1 || channel > MIDI_CHANNEL_16 ) return 1;
  device->base_channel = channel;
  return 0;
}

/**
 * @brief Get the device's base channel.
 * @see MIDIDeviceSetBaseChannel
 * @public @memberof MIDIDevice
 * @param device  The device.
 * @param channel The base channel.
 * @retval 0  on success.
 * @retval >0 if the base channel coult not be stored.
 */
int MIDIDeviceGetBaseChannel( struct MIDIDevice * device, MIDIChannel * channel ) {
  if( channel == NULL ) return 1;
  *channel = device->base_channel;
  return 0;
}

/**
 * @brief Set the device's timer.
 * The device's timer is responsible for following real time messages to
 * approximate the message sender's midi time and clock rate.
 * It is used to obtain timestamps for outgoing messages.
 * @see MIDIDeviceSetTimer
 * @public @memberof MIDIDevice
 * @param device The device.
 * @param timer  The timer.
 * @retval 0  on success.
 * @retval >0 if the timer could not be set.
 */
int MIDIDeviceSetTimer( struct MIDIDevice * device, struct MIDITimer * timer ) {
  if( device->timer != NULL ) MIDITimerRelease( device->timer );
  MIDITimerRetain( timer );
  device->timer = timer;
  return 0;
}

/**
 * @brief Get the device's timer.
 * @see MIDIDeviceSetTimer
 * @public @memberof MIDIDevice
 * @param device The device.
 * @param timer  The timer.
 * @retval 0  on success.
 * @retval >0 if the timer could not be stored.
 */
int MIDIDeviceGetTimer( struct MIDIDevice * device, struct MIDITimer ** timer ) {
  if( timer == NULL ) return 1;
  *timer = device->timer;
  return 0;
}

/*
 **
 * @brief Set the instrument for a specific channel.
 * Each channel can have it's own instrument. An instrument may be attached
 * to multiple channels, even multiple devices.
 *
 * The instrument is retained by the device, once for each channel it is
 * connected to. Thus if the instrument is attached to 5 channels it is
 * retained five times.
 * @public @memberof MIDIDevice
 * @param device     The midi device.
 * @param channel    The channel to attach the instrument to.
 *                   To attach the instrument to the device's base channel
 *                   use special MIDI_CHANNEL_BASE; to attach it to all
 *                   channels at once, use MIDI_CHANNEL_ALL.
 * @param instrument The instrument to attach.
 * @retval 0  on success.
 * @retval >0 if the instrument could not be attached to the channel(s).
 *
int MIDIDeviceSetChannelInstrument( struct MIDIDevice * device, MIDIChannel channel, struct MIDIInstrument * instrument ) {
  if( channel == MIDI_CHANNEL_ALL ) {
    for( channel=MIDI_CHANNEL_1; channel<=MIDI_CHANNEL_16; channel++ ) {
      if( MIDIDeviceSetChannelInstrument( device, channel, instrument ) ) return 1;
    }
    return 0;
  }
  if( channel == MIDI_CHANNEL_BASE ) channel = device->base_channel;
  if( channel < MIDI_CHANNEL_1 || channel > MIDI_CHANNEL_16 ) return 1;
  if( device->instrument[(int)channel] == instrument ) return 0;
  if( device->instrument[(int)channel] != NULL ) MIDIControllerRelease( device->instrument[(int)channel] );
  MIDIInstrumentRetain( controller );
  device->instrument[(int)channel] = instrument;
  return 0;
}

 **
 * @brief Get the instrument for a specific channel.
 * @see MIDIDeviceSetChannelInstrument
 * @public @memberof MIDIDevice
 * @param device     The midi device.
 * @param channel    The channel for which to get the instrument.
 * @param instrument The a pointer to the location to store the instrument
 *                   reference in.
 * @retval 0  on success.
 * @retval >0 if the instrument was not stored.
 *
int MIDIDeviceGetChannelInstrument( struct MIDIDevice * device, MIDIChannel channel, struct MIDIInstrument ** instrument ) {
  if( channel == MIDI_CHANNEL_BASE ) channel = device->base_channel;
  if( channel < MIDI_CHANNEL_1 || channel > MIDI_CHANNEL_16 ) return 1;
  if( instrument == NULL ) return 1;
  *instrument = device->instrument[(int)channel];
  return 0;
}
*/

/**
 * @brief Set the controller for a specific channel.
 * Each channel can have it's own controller. A controller may be attached
 * to multiple channels, even multiple devices.
 *
 * The controller is retained by the device, once for each channel it is
 * connected to. Thus if the controller is attached to 5 channels it is
 * retained five times.
 * @public @memberof MIDIDevice
 * @param device     The midi device.
 * @param channel    The channel to attach the controller to.
 *                   To attach the controller to the device's base channel
 *                   use special MIDI_CHANNEL_BASE; to attach it to all
 *                   channels at once, use MIDI_CHANNEL_ALL.
 * @param controller The controller to attach.
 * @retval 0  on success.
 * @retval >0 if the controller could not be attached to the channel(s).
 */
int MIDIDeviceSetChannelController( struct MIDIDevice * device, MIDIChannel channel, struct MIDIController * controller ) {
  if( channel == MIDI_CHANNEL_ALL ) {
    for( channel=MIDI_CHANNEL_1; channel<=MIDI_CHANNEL_16; channel++ ) {
      if( MIDIDeviceSetChannelController( device, channel, controller ) ) return 1;
    }
    return 0;
  }
  if( channel == MIDI_CHANNEL_BASE ) channel = device->base_channel;
  if( channel < MIDI_CHANNEL_1 || channel > MIDI_CHANNEL_16 ) return 1;
  if( device->controller[(int)channel] == controller ) return 0;
  if( device->controller[(int)channel] != NULL ) MIDIControllerRelease( device->controller[(int)channel] );
  MIDIControllerRetain( controller );
  device->controller[(int)channel] = controller;
  return 0;
}

/**
 * @brief Get the controller for a specific channel.
 * @see MIDIDeviceSetChannelController
 * @public @memberof MIDIDevice
 * @param device     The midi device.
 * @param channel    The channel for which to get the controller.
 * @param controller The a pointer to the location to store the controller
 *                   reference in.
 * @retval 0  on success.
 * @retval >0 if the controller was not stored.
 */
int MIDIDeviceGetChannelController( struct MIDIDevice * device, MIDIChannel channel, struct MIDIController ** controller ) {
  if( channel == MIDI_CHANNEL_BASE ) channel = device->base_channel;
  if( channel < MIDI_CHANNEL_1 || channel > MIDI_CHANNEL_16 ) return 1;
  if( controller == NULL ) return 1;
  *controller = device->controller[(int)channel];
  return 0;
}

/** @} */

#pragma mark Internal message routing
/**
 * @internal
 * Internal message routing.
 * @{
 */
 
static int _recv_cc_omni( struct MIDIDevice * device, MIDIChannel channel,
                          MIDIControl control, MIDIValue value ) {
  int result = 0, i;
  struct MIDIController * ctl;
  struct MIDIController * recv[N_CHANNEL] = { NULL };
  MIDIChannel c;
  MIDIPrecond( device != NULL, EFAULT );
  
  for( c=MIDI_CHANNEL_1; c<=MIDI_CHANNEL_16; c++ ) {
    if( device->controller[(int)c] != NULL ) {
      i   = 0;
      ctl = device->controller[(int)c];
      while( i<N_CHANNEL
          && recv[i]!=NULL
          && recv[i]!=ctl ) i++;
      if( recv[i] != ctl ) {
        recv[i]   = ctl;
        recv[i+1] = NULL;
        result   += MIDIControllerReceiveControlChange( ctl, device, channel,
                                                        control, value );
      }
    }
  }
  return result;
}

static int _recv_cc( struct MIDIDevice * device, MIDIChannel channel,
                     MIDIControl control, MIDIValue value ) {
  MIDIPrecond( device != NULL, EFAULT );

  if( device->omni_mode == MIDI_OFF ) {
    if( channel == device->base_channel &&
        device->controller[(int)channel] != NULL ) {
      return MIDIControllerReceiveControlChange( device->controller[(int)channel],
                                                 device, channel, control, value );
    }
  } else {
    return _recv_cc_omni( device, channel, control, value );
  }
  return 0;
}

/**
 * @brief Receive a generic MIDI message.
 * This is called by the @c IN port whenever it relays a MIDIMessage
 * to the device. Such a message may come from a MIDIDriver or the @c THRU
 * port of another device.
 *
 * The device determines the message type and calls the appropiate member
 * function. If something is attached to the device's @c THRU port, the message
 * is relayed via the port @em before the device processes the message.
 * This implies that, when multiple devices are daisy-chained with their
 * @c THRU ports, the last device in the chain will be the first device to process
 * the message.
 * @private @memberof MIDIDevice
 * @param device  The midi device.
 * @param message The received message.
 * @retval 0 on success.
 * @retval 1 if the message could not be processed.
 */
static int _recv_msg( struct MIDIDevice * device, struct MIDIMessage * message ) {
  MIDIStatus     status;
  MIDITimestamp  timestamp;
  MIDIValue      v[3];
  MIDILongValue  lv;
  uint8_t b;
  size_t  s;
  void *  vp;
  MIDIPrecond( device != NULL, EFAULT );
  MIDIPrecond( message != NULL, EINVAL );
  
  MIDIMessageGetStatus( message, &status );
  switch( status ) {
    case MIDI_STATUS_NOTE_OFF:
      MIDIMessageGet( message, MIDI_CHANNEL,  sizeof(MIDIChannel),  &v[0] );
      MIDIMessageGet( message, MIDI_KEY,      sizeof(MIDIKey),      &v[1] );
      MIDIMessageGet( message, MIDI_VELOCITY, sizeof(MIDIVelocity), &v[2] );
      return MIDIDeviceReceiveNoteOff( device, v[0], v[1], v[2] );
    case MIDI_STATUS_NOTE_ON:
      MIDIMessageGet( message, MIDI_CHANNEL,  sizeof(MIDIChannel),  &v[0] );
      MIDIMessageGet( message, MIDI_KEY,      sizeof(MIDIKey),      &v[1] );
      MIDIMessageGet( message, MIDI_VELOCITY, sizeof(MIDIVelocity), &v[2] );
      return MIDIDeviceReceiveNoteOn( device, v[0], v[1], v[2] );
    case MIDI_STATUS_POLYPHONIC_KEY_PRESSURE:
      MIDIMessageGet( message, MIDI_CHANNEL,  sizeof(MIDIChannel),  &v[0] );
      MIDIMessageGet( message, MIDI_KEY,      sizeof(MIDIKey),      &v[1] );
      MIDIMessageGet( message, MIDI_PRESSURE, sizeof(MIDIPressure), &v[2] );
      return MIDIDeviceReceivePolyphonicKeyPressure( device, v[0], v[1], v[2] );
    case MIDI_STATUS_CONTROL_CHANGE:
      MIDIMessageGet( message, MIDI_CHANNEL, sizeof(MIDIChannel), &v[0] );
      MIDIMessageGet( message, MIDI_CONTROL, sizeof(MIDIControl), &v[1] );
      MIDIMessageGet( message, MIDI_VALUE,   sizeof(MIDIValue),   &v[2] );
      return MIDIDeviceReceiveControlChange( device, v[0], v[1], v[2] );
    case MIDI_STATUS_PROGRAM_CHANGE:
      MIDIMessageGet( message, MIDI_CHANNEL, sizeof(MIDIChannel), &v[0] );
      MIDIMessageGet( message, MIDI_PROGRAM, sizeof(MIDIProgram), &v[1] );
      return MIDIDeviceReceiveProgramChange( device, v[0], v[1] );
    case MIDI_STATUS_CHANNEL_PRESSURE:
      MIDIMessageGet( message, MIDI_CHANNEL,  sizeof(MIDIChannel),  &v[0] );
      MIDIMessageGet( message, MIDI_PRESSURE, sizeof(MIDIPressure), &v[1] );
      return MIDIDeviceReceiveChannelPressure( device, v[0], v[1] );
    case MIDI_STATUS_PITCH_WHEEL_CHANGE:
      MIDIMessageGet( message, MIDI_CHANNEL, sizeof(MIDIChannel),   &v[0] );
      MIDIMessageGet( message, MIDI_VALUE,   sizeof(MIDILongValue), &lv );
      return MIDIDeviceReceivePitchWheelChange( device, v[0], lv );
    case MIDI_STATUS_SYSTEM_EXCLUSIVE:
      MIDIMessageGet( message, MIDI_MANUFACTURER_ID, sizeof(MIDIManufacturerId), &v[0] );
      MIDIMessageGet( message, MIDI_SYSEX_SIZE,      sizeof(size_t), &s );
      MIDIMessageGet( message, MIDI_SYSEX_DATA,      sizeof(void*), &vp );
      MIDIMessageGet( message, MIDI_SYSEX_FRAGMENT,  sizeof(uint8_t), &b );
      return MIDIDeviceReceiveSystemExclusive( device, v[0], s, vp, b );
    case MIDI_STATUS_TIME_CODE_QUARTER_FRAME:
      MIDIMessageGet( message, MIDI_TIME_CODE_TYPE, sizeof(MIDIValue), &v[0] );
      MIDIMessageGet( message, MIDI_VALUE,          sizeof(MIDIValue), &v[1] );
      return MIDIDeviceReceiveTimeCodeQuarterFrame( device, v[0], v[1] );
    case MIDI_STATUS_SONG_POSITION_POINTER:
      MIDIMessageGet( message, MIDI_VALUE, sizeof(MIDILongValue), &lv );
      return MIDIDeviceReceiveSongPositionPointer( device, lv );
    case MIDI_STATUS_SONG_SELECT:
      MIDIMessageGet( message, MIDI_VALUE, sizeof(MIDIValue), &v[0] );
      return MIDIDeviceReceiveSongSelect( device, v[0] );
    case MIDI_STATUS_TUNE_REQUEST:
      return MIDIDeviceReceiveTuneRequest( device );
    case MIDI_STATUS_END_OF_EXCLUSIVE:
      return MIDIDeviceReceiveEndOfExclusive( device );
    case MIDI_STATUS_TIMING_CLOCK:
    case MIDI_STATUS_START:
    case MIDI_STATUS_CONTINUE:
    case MIDI_STATUS_STOP:
    case MIDI_STATUS_ACTIVE_SENSING:
    case MIDI_STATUS_RESET:
      MIDIMessageGetTimestamp( message, &timestamp );
      return MIDIDeviceReceiveRealTime( device, status, timestamp );
    default:
      break;
  }
  return 0;
}

static int _recv_rt( struct MIDIDevice * device, MIDIStatus status, MIDITimestamp timestamp ) {
  MIDIPrecond( device != NULL, EFAULT );
  if( device->timer == NULL ) return 0;
  return MIDITimerReceiveRealTime( device->timer, device, status, timestamp );
}

static int _recv( void * dev, void * source, struct MIDITypeSpec * type, void * data ) {
  MIDIPrecond( dev != NULL, EFAULT );
  if( type == MIDIMessageType ) {
    MIDIPrecond( data != NULL, EINVAL );
    return _recv_msg( dev, data );
  } else {
    return 0;
  }
}


/** @} */

#pragma mark Message passing
/**
 * @name Message passing
 * Receiving and and sending MIDIMessage objects.
 * @{
 */

/**
 * @brief Receive a generic MIDI message.
 * This simulates a received message on the device's @c IN port.
 * @public @memberof MIDIDevice
 * @param device  The midi device.
 * @param message The received message.
 * @retval 0 on success.
 * @retval 1 if the message could not be processed.
 */
int MIDIDeviceReceive( struct MIDIDevice * device, struct MIDIMessage * message ) {
  MIDIPrecond( device != NULL, EFAULT );
  MIDIPrecond( message != NULL, EINVAL );
  return MIDIPortReceive( device->in, MIDIMessageType, message );
}


/**
 * @brief Send a generic MIDI message.
 * This calls the the relay method of the @c OUT connector.
 * @public @memberof MIDIDevice
 * @param device  The device.
 * @param message The message.
 * @retval 0 on success.
 * @retval 1 if the message could not be sent.
 */
int MIDIDeviceSend( struct MIDIDevice * device, struct MIDIMessage * message ) {
  MIDIPrecond( device != NULL, EFAULT );
  MIDIPrecond( message != NULL, EINVAL );
  return MIDIPortSend( device->out, MIDIMessageType, message );
}

/**
 * @brief Receive a "Note Off" message.
 * This is called whenever the device receives a "Note Off" message.
 * It can be used to simulate the reception of such a message.
 * @public @memberof MIDIDevice
 * @param device   The midi device.
 * @param channel  The channel on which the note ended.
 * @param key      The key that ended.
 * @param velocity The velocity with which the "key" was released.
 * @retval 0 on success.
 * @retval 1 if the message could not be processed.
 */
int MIDIDeviceReceiveNoteOff( struct MIDIDevice * device, MIDIChannel channel, MIDIKey key, MIDIVelocity velocity ) {
  MIDIPrecond( device != NULL, EFAULT );
  if( device->delegate == NULL || device->delegate->recv_nof == NULL ) {
    return 0;
  }
  return (*device->delegate->recv_nof)( device, channel, key, velocity );
}

/**
 * @brief Send a "Note Off" message.
 * This is called when the device wants to send a "Note Off" message.
 * It can be used to trigger the sending of such a message.
 * @public @memberof MIDIDevice
 * @param device   The midi device.
 * @param channel  The channel on which the note ended.
 * @param key      The key that ended.
 * @param velocity The velocity with which the "key" was released.
 * @retval 0 on success.
 * @retval 1 if the message could not be sent.
 */
int MIDIDeviceSendNoteOff( struct MIDIDevice * device, MIDIChannel channel, MIDIKey key, MIDIVelocity velocity ) {
  struct MIDIMessage * message;
  int result;
  MIDIPrecond( device != NULL, EFAULT );
  message = MIDIMessageCreate( MIDI_STATUS_NOTE_OFF );
  result  = MIDIMessageSet( message, MIDI_CHANNEL,  sizeof(MIDIChannel),  &channel );
  result += MIDIMessageSet( message, MIDI_KEY,      sizeof(MIDIKey),      &key );
  result += MIDIMessageSet( message, MIDI_VELOCITY, sizeof(MIDIVelocity), &velocity );
  if( result != 0 ) return result;
  result  = MIDIDeviceSend( device, message );
  MIDIMessageRelease( message );
  return result;
}

/**
 * @brief Receive a "Note On" message.
 * This is called whenever the device receives a "Note On" message.
 * It can be used to simulate the reception of such a message.
 * @public @memberof MIDIDevice
 * @param device   The midi device.
 * @param channel  The channel on which the note occurred.
 * @param key      The key that was played.
 * @param velocity The velocity with which the "key" was pressed.
 * @retval 0 on success.
 * @retval 1 if the message could not be processed.
 */
int MIDIDeviceReceiveNoteOn( struct MIDIDevice * device, MIDIChannel channel, MIDIKey key, MIDIVelocity velocity ) {
  MIDIPrecond( device != NULL, EFAULT );
  if( device->delegate == NULL || device->delegate->recv_non == NULL ) {
    return 0;
  }
  return (*device->delegate->recv_non)( device, channel, key, velocity );
}

/**
 * @brief Send a "Note On" message.
 * This is called when the device wants to send a "Note On" message.
 * It can be used to trigger the sending of such a message.
 * @public @memberof MIDIDevice
 * @param device   The midi device.
 * @param channel  The channel on which the note occurred.
 * @param key      The key that was played.
 * @param velocity The velocity with which the "key" was pressed.
 * @retval 0 on success.
 * @retval 1 if the message could not be sent.
 */
int MIDIDeviceSendNoteOn( struct MIDIDevice * device, MIDIChannel channel, MIDIKey key, MIDIVelocity velocity ) {
  struct MIDIMessage * message;
  int result;
  MIDIPrecond( device != NULL, EFAULT );
  message = MIDIMessageCreate( MIDI_STATUS_NOTE_ON );
  result  = MIDIMessageSet( message, MIDI_CHANNEL,  sizeof(MIDIChannel),  &channel );
  result += MIDIMessageSet( message, MIDI_KEY,      sizeof(MIDIKey),      &key );
  result += MIDIMessageSet( message, MIDI_VELOCITY, sizeof(MIDIVelocity), &velocity );
  if( result != 0 ) return result;
  result  = MIDIDeviceSend( device, message );
  MIDIMessageRelease( message );
  return result;
}

/**
 * @brief Receive a "Polyphonic Key Pressure" message.
 * This is called whenever the device receives a "Polyphonic Key Pressure" message.
 * It can be used to simulate the reception of such a message.
 * @public @memberof MIDIDevice
 * @param device   The midi device.
 * @param channel  The channel on which the pressure change occured.
 * @param key      The key that was played.
 * @param pressure The pressure applied to the "key".
 * @retval 0 on success.
 * @retval 1 if the message could not be processed.
 */
int MIDIDeviceReceivePolyphonicKeyPressure( struct MIDIDevice * device, MIDIChannel channel, MIDIKey key, MIDIPressure pressure ) {
  MIDIPrecond( device != NULL, EFAULT );
  if( device->delegate == NULL || device->delegate->recv_pkp == NULL ) {
    return 0;
  }
  return (*device->delegate->recv_pkp)( device, channel, key, pressure );
}

/**
 * @brief Send a "Polyphonic Key Pressure" message.
 * This is called when the device wants to send a "Polyphonic Key Pressure" message.
 * It can be used to trigger the sending of such a message.
 * @public @memberof MIDIDevice
 * @param device   The midi device.
 * @param channel  The channel on which the note occurred.
 * @param key      The key that was played.
 * @param pressure The pressure applied to the "key".
 * @retval 0 on success.
 * @retval 1 if the message could not be sent.
 */
int MIDIDeviceSendPolyphonicKeyPressure( struct MIDIDevice * device, MIDIChannel channel, MIDIKey key, MIDIPressure pressure ) {
  struct MIDIMessage * message;
  int result;
  MIDIPrecond( device != NULL, EFAULT );
  message = MIDIMessageCreate( MIDI_STATUS_POLYPHONIC_KEY_PRESSURE );
  result  = MIDIMessageSet( message, MIDI_CHANNEL,  sizeof(MIDIChannel),  &channel );
  result += MIDIMessageSet( message, MIDI_KEY,      sizeof(MIDIKey),      &key );
  result += MIDIMessageSet( message, MIDI_PRESSURE, sizeof(MIDIPressure), &pressure );
  if( result != 0 ) return result;
  result  = MIDIDeviceSend( device, message );
  MIDIMessageRelease( message );
  return result;
}

/**
 * @brief Receive a "Control Change" message.
 * This is called whenever the device receives a "Control Change" message.
 * It can be used to simulate the reception of such a message.
 * @public @memberof MIDIDevice
 * @param device  The midi device.
 * @param channel The channel on which the control change occured.
 * @param control The control that was changed.
 * @param value   The new value of the control.
 * @retval 0 on success.
 * @retval 1 if the message could not be processed.
 */
int MIDIDeviceReceiveControlChange( struct MIDIDevice * device, MIDIChannel channel, MIDIControl control, MIDIValue value ) {
  MIDIPrecond( device != NULL, EFAULT );
  int result = _recv_cc( device, channel, control, value );
  if( device->delegate == NULL || device->delegate->recv_cc == NULL ) {
    return result;
  }
  return result + (*device->delegate->recv_cc)( device, channel, control, value );
}

/**
 * @brief Send a "Control Change" message.
 * This is called when the device wants to send a "Control Change" message.
 * It can be used to trigger the sending of such a message.
 * @public @memberof MIDIDevice
 * @param device  The midi device.
 * @param channel The channel on which the control change occured.
 * @param control The control that was changed.
 * @param value   The new value of the control.
 * @retval 0 on success.
 * @retval 1 if the message could not be sent.
 */
int MIDIDeviceSendControlChange( struct MIDIDevice * device, MIDIChannel channel, MIDIControl control, MIDIValue value ) {
  struct MIDIMessage * message;
  int result;
  MIDIPrecond( device != NULL, EFAULT );
  message = MIDIMessageCreate( MIDI_STATUS_CONTROL_CHANGE );
  result  = MIDIMessageSet( message, MIDI_CHANNEL, sizeof(MIDIChannel), &channel );
  result += MIDIMessageSet( message, MIDI_CONTROL, sizeof(MIDIControl), &control );
  result += MIDIMessageSet( message, MIDI_VALUE,   sizeof(MIDIValue),   &value );
  if( result != 0 ) return result;
  result  = MIDIDeviceSend( device, message );
  MIDIMessageRelease( message );
  return result;
}

/**
 * @brief Receive a "Program Change" message.
 * This is called whenever the device receives a "Program Change" message.
 * It can be used to simulate the reception of such a message.
 * @public @memberof MIDIDevice
 * @param device  The midi device.
 * @param channel The channel on which the program change occured.
 * @param program The new program.
 * @retval 0 on success.
 * @retval 1 if the message could not be processed.
 */
int MIDIDeviceReceiveProgramChange( struct MIDIDevice * device, MIDIChannel channel, MIDIProgram program ) {
  MIDIPrecond( device != NULL, EFAULT );
  if( device->delegate == NULL || device->delegate->recv_pc == NULL ) {
    return 0;
  }
  return (*device->delegate->recv_pc)( device, channel, program );
}

/**
 * @brief Send a "Program Change" message.
 * This is called when the device wants to send a "Program Change" message.
 * It can be used to trigger the sending of such a message.
 * @public @memberof MIDIDevice
 * @param device  The midi device.
 * @param channel The channel on which the program change occured.
 * @param program The new program.
 * @retval 0 on success.
 * @retval 1 if the message could not be sent.
 */
int MIDIDeviceSendProgramChange( struct MIDIDevice * device, MIDIChannel channel, MIDIProgram program ) {
  struct MIDIMessage * message;
  int result;
  MIDIPrecond( device != NULL, EFAULT );
  message = MIDIMessageCreate( MIDI_STATUS_PROGRAM_CHANGE );
  result  = MIDIMessageSet( message, MIDI_CHANNEL, sizeof(MIDIChannel), &channel );
  result += MIDIMessageSet( message, MIDI_PROGRAM, sizeof(MIDIProgram), &program );
  if( result != 0 ) return result;
  result  = MIDIDeviceSend( device, message );
  MIDIMessageRelease( message );
  return result;
}

/**
 * @brief Receive a "Channel Pressure" message.
 * This is called whenever the device receives a "Channel Pressure" message.
 * It can be used to simulate the reception of such a message.
 * @public @memberof MIDIDevice
 * @param device   The midi device.
 * @param channel  The channel on which the pressure change occured.
 * @param pressure The pressure applied to all notes currently played on the channel.
 * @retval 0 on success.
 * @retval 1 if the message could not be processed.
 */
int MIDIDeviceReceiveChannelPressure( struct MIDIDevice * device, MIDIChannel channel, MIDIPressure pressure ) {
  MIDIPrecond( device != NULL, EFAULT );
  if( device->delegate == NULL || device->delegate->recv_cp == NULL ) {
    return 0;
  }
  return (*device->delegate->recv_cp)( device, channel, pressure );
}

/**
 * @brief Send a "Channel Pressure" message.
 * This is called when the device wants to send a "Channel Pressure" message.
 * It can be used to trigger the sending of such a message.
 * @public @memberof MIDIDevice
 * @param device   The midi device.
 * @param channel  The channel on which the pressure change occured.
 * @param pressure The pressure applied to all notes currently played on the channel.
 * @retval 0 on success.
 * @retval 1 if the message could not be sent.
 */
int MIDIDeviceSendChannelPressure( struct MIDIDevice * device, MIDIChannel channel, MIDIPressure pressure ) {
  struct MIDIMessage * message;
  int result;
  MIDIPrecond( device != NULL, EFAULT );
  message = MIDIMessageCreate( MIDI_STATUS_CHANNEL_PRESSURE );
  result  = MIDIMessageSet( message, MIDI_CHANNEL,  sizeof(MIDIChannel),  &channel );
  result += MIDIMessageSet( message, MIDI_PRESSURE, sizeof(MIDIPressure), &pressure );
  if( result != 0 ) return result;
  result  = MIDIDeviceSend( device, message );
  MIDIMessageRelease( message );
  return result;
}

/**
 * @brief Receive a "Pitch Wheel Change" message.
 * This is called whenever the device receives a "Pitch Wheel Change" message.
 * It can be used to simulate the reception of such a message.
 * @public @memberof MIDIDevice
 * @param device  The midi device.
 * @param channel The channel on which the pitch wheel change occured.
 * @param value   The amount of pitch-change of all notes played on the channel.
 *                (@c 0x2000 = Original pitch)
 * @retval 0 on success.
 * @retval 1 if the message could not be processed.
 */
int MIDIDeviceReceivePitchWheelChange( struct MIDIDevice * device, MIDIChannel channel, MIDILongValue value ) {
  MIDIPrecond( device != NULL, EFAULT );
  if( device->delegate == NULL || device->delegate->recv_pwc == NULL ) {
    return 0;
  }
  return (*device->delegate->recv_pwc)( device, channel, value );
}

/**
 * @brief Send a "Pitch Wheel Change" message.
 * This is called when the device wants to send a "Pitch Wheel Change" message.
 * It can be used to trigger the sending of such a message.
 * @public @memberof MIDIDevice
 * @param device  The midi device.
 * @param channel The channel on which the pitch wheel change occured.
 * @param value   The amount of pitch-change of all notes played on the channel.
 *                (@c 0x2000 = Original pitch)
 * @retval 0 on success.
 * @retval 1 if the message could not be sent.
 */
int MIDIDeviceSendPitchWheelChange( struct MIDIDevice * device, MIDIChannel channel, MIDILongValue value ) {
  struct MIDIMessage * message;
  int result;
  MIDIPrecond( device != NULL, EFAULT );
  message = MIDIMessageCreate( MIDI_STATUS_PITCH_WHEEL_CHANGE );
  result  = MIDIMessageSet( message, MIDI_CHANNEL, sizeof(MIDIChannel),   &channel );
  result += MIDIMessageSet( message, MIDI_VALUE,   sizeof(MIDILongValue), &value );
  if( result != 0 ) return result;
  result  = MIDIDeviceSend( device, message );
  MIDIMessageRelease( message );
  return result;
}

/**
 * @brief Receive a "System Exclusive" message.
 * This is called whenever the device receives a "System Exclusive" message.
 * It can be used to simulate the reception of such a message.
 * @public @memberof MIDIDevice
 * @param device          The midi device.
 * @param manufacturer_id The ID of the manufacturer that defines the message syntax.
 *                        (@c 0x7d = Educational Use, @c 0x7e = Non-Realtime Universal,
 *                         @c 0x7f = Realtime Universal)
 * @param size            The size of the buffer pointed to by @c data.
 * @param data            The system exclusive data. (@c size bytes)
 * @param fragment        The fragment number of the system exclusive message, starting
 *                        with zero. A message may be fragmented due to various reasons.
 *                        A receiver should collect fragments until it receives an
 *                        "End of Exclusive" message.
 * @retval 0 on success.
 * @retval 1 if the message could not be processed.
 */
int MIDIDeviceReceiveSystemExclusive( struct MIDIDevice * device, MIDIManufacturerId manufacturer_id,
                                      size_t size, void * data, uint8_t fragment ) {
  MIDIPrecond( device != NULL, EFAULT );
  if( device->delegate == NULL || device->delegate->recv_sx == NULL ) {
    return 0;
  }
  return (*device->delegate->recv_sx)( device, manufacturer_id, size, data, fragment );
}

/**
 * @brief Send a "System Exclusive" message.
 * This is called when the device wants to send a "System Exclusive" message.
 * It can be used to trigger the sending of such a message.
 * @public @memberof MIDIDevice
 * @param device          The midi device.
 * @param manufacturer_id The ID of the manufacturer that defines the message syntax.
 *                        (@c 0x7d = Educational Use, @c 0x7e = Non-Realtime Universal,
 *                         @c 0x7f = Realtime Universal)
 * @param size            The size of the buffer pointed to by @c data.
 * @param data            The system exclusive data. (@c size bytes)
 * @param fragment        The fragment number of the system exclusive message, starting
 *                        with zero. A message may be fragmented due to various reasons.
 *                        A receiver should collect fragments until it receives an
 *                        "End of Exclusive" message.
 * @retval 0 on success.
 * @retval 1 if the message could not be sent.
 */
int MIDIDeviceSendSystemExclusive( struct MIDIDevice * device, MIDIManufacturerId manufacturer_id,
                                   size_t size, void * data, uint8_t fragment ) {
  struct MIDIMessage * message;
  int result;
  MIDIPrecond( device != NULL, EFAULT );
  message = MIDIMessageCreate( MIDI_STATUS_SYSTEM_EXCLUSIVE );
  result  = MIDIMessageSet( message, MIDI_MANUFACTURER_ID, sizeof(MIDIManufacturerId), &manufacturer_id );
  result += MIDIMessageSet( message, MIDI_SYSEX_SIZE,      sizeof(size_t), &size );
  result += MIDIMessageSet( message, MIDI_SYSEX_DATA,      sizeof(void *), &data );
  result += MIDIMessageSet( message, MIDI_SYSEX_FRAGMENT,  sizeof(uint8_t), &fragment );
  if( result != 0 ) return result;
  result  = MIDIDeviceSend( device, message );
  MIDIMessageRelease( message );
  return result;
}

/**
 * @brief Receive a "Time Code Quarter Frame" message.
 * This is called whenever the device receives a "Time Code Quarter Frame" message.
 * It can be used to simulate the reception of such a message.
 * @public @memberof MIDIDevice
 * @param device         The midi device.
 * @param time_code_type One of the eight code-types specified by the MIDI time code spec.
 * @param value          The 4-bit value for the given time code type.
 * @retval 0 on success.
 * @retval 1 if the message could not be processed.
 */
int MIDIDeviceReceiveTimeCodeQuarterFrame( struct MIDIDevice * device, MIDIValue time_code_type, MIDIValue value ) {
  MIDIPrecond( device != NULL, EFAULT );
  if( device->delegate == NULL || device->delegate->recv_tcqf == NULL ) {
    return 0;
  }
  return (*device->delegate->recv_tcqf)( device, time_code_type, value );
}

/**
 * @brief Send a "Time Code Quarter Frame" message.
 * This is called when the device wants to send a "Time Code Quarter Frame" message.
 * It can be used to trigger the sending of such a message.
 * @public @memberof MIDIDevice
 * @param device         The midi device.
 * @param time_code_type One of the eight code-types specified by the MIDI time code spec.
 * @param value          The 4-bit value for the given time code type.
 * @retval 0 on success.
 * @retval 1 if the message could not be sent.
 */
int MIDIDeviceSendTimeCodeQuarterFrame( struct MIDIDevice * device, MIDIValue time_code_type, MIDIValue value ) {
  struct MIDIMessage * message;
  int result;
  MIDIPrecond( device != NULL, EFAULT );
  message = MIDIMessageCreate( MIDI_STATUS_TIME_CODE_QUARTER_FRAME );
  result  = MIDIMessageSet( message, MIDI_TIME_CODE_TYPE, sizeof(MIDIValue), &time_code_type );
  result += MIDIMessageSet( message, MIDI_VALUE,          sizeof(MIDIValue), &value );
  if( result != 0 ) return result;
  result  = MIDIDeviceSend( device, message );
  MIDIMessageRelease( message );
  return result;
}

/**
 * @brief Receive a "Song Position Pointer" message.
 * This is called whenever the device receives a "Song Position Pointer" message.
 * It can be used to simulate the reception of such a message.
 * @public @memberof MIDIDevice
 * @param device The midi device.
 * @param value  A 14-bit value specifying a "beat" inside the current song or sequence.
 *               (1 beat = 1/16 note = 6 clocks)
 * @retval 0 on success.
 * @retval 1 if the message could not be processed.
 */
int MIDIDeviceReceiveSongPositionPointer( struct MIDIDevice * device, MIDILongValue value ) {
  MIDIPrecond( device != NULL, EFAULT );
  if( device->delegate == NULL || device->delegate->recv_spp == NULL ) {
    return 0;
  }
  return (*device->delegate->recv_spp)( device, value );
}

/**
 * @brief Send a "Song Position Pointer" message.
 * This is called when the device wants to send a "Song Position Pointer" message.
 * It can be used to trigger the sending of such a message.
 * @public @memberof MIDIDevice
 * @param device The midi device.
 * @param value  A 14-bit value specifying a "beat" inside the current song or sequence.
 *               (1 beat = 1/16 note = 6 clocks)
 * @retval 0 on success.
 * @retval 1 if the message could not be sent.
 */
int MIDIDeviceSendSongPositionPointer( struct MIDIDevice * device, MIDILongValue value ) {
  struct MIDIMessage * message;
  int result;
  MIDIPrecond( device != NULL, EFAULT );
  message = MIDIMessageCreate( MIDI_STATUS_SONG_POSITION_POINTER );
  result  = MIDIMessageSet( message, MIDI_VALUE, sizeof(MIDILongValue), &value );
  if( result != 0 ) return result;
  result  = MIDIDeviceSend( device, message );
  MIDIMessageRelease( message );
  return result;
}

/**
 * @brief Receive a "Song Select" message.
 * This is called whenever the device receives a "Song Select" message.
 * It can be used to simulate the reception of such a message.
 * @public @memberof MIDIDevice
 * @param device The midi device.
 * @param value  The song or sequence number to select.
 * @retval 0 on success.
 * @retval 1 if the message could not be processed.
 */
int MIDIDeviceReceiveSongSelect( struct MIDIDevice * device, MIDIValue value ) {
  MIDIPrecond( device != NULL, EFAULT );
  if( device->delegate == NULL || device->delegate->recv_ss == NULL ) {
    return 0;
  }
  return (*device->delegate->recv_ss)( device, value );
}

/**
 * @brief Send a "Song Select" message.
 * This is called when the device wants to send a "Song Select" message.
 * It can be used to trigger the sending of such a message.
 * @public @memberof MIDIDevice
 * @param device The midi device.
 * @param value  The song or sequence number to select.
 * @retval 0 on success.
 * @retval 1 if the message could not be sent.
 */
int MIDIDeviceSendSongSelect( struct MIDIDevice * device, MIDIValue value ) {
  struct MIDIMessage * message;
  int result;
  MIDIPrecond( device != NULL, EFAULT );
  message = MIDIMessageCreate( MIDI_STATUS_SONG_SELECT );
  result  = MIDIMessageSet( message, MIDI_VALUE, sizeof(MIDIValue), &value );
  if( result != 0 ) return result;
  result  = MIDIDeviceSend( device, message );
  MIDIMessageRelease( message );
  return result;
}

/**
 * @brief Receive a "Tune Request" message.
 * This is called whenever the device receives a "Tune Request" message.
 * It can be used to simulate the reception of such a message.
 * @public @memberof MIDIDevice
 * @param device The midi device.
 * @retval 0 on success.
 * @retval 1 if the message could not be processed.
 */
int MIDIDeviceReceiveTuneRequest( struct MIDIDevice * device ) {
  MIDIPrecond( device != NULL, EFAULT );
  if( device->delegate == NULL || device->delegate->recv_tr == NULL ) {
    return 0;
  }
  return (*device->delegate->recv_tr)( device );
}

/**
 * @brief Send a "Tune Request" message.
 * This is called when the device wants to send a "Tune Request" message.
 * It can be used to trigger the sending of such a message.
 * @public @memberof MIDIDevice
 * @param device The midi device.
 * @retval 0 on success.
 * @retval 1 if the message could not be sent.
 */
int MIDIDeviceSendTuneRequest( struct MIDIDevice * device ) {
  struct MIDIMessage * message;
  int result;
  MIDIPrecond( device != NULL, EFAULT );
  message = MIDIMessageCreate( MIDI_STATUS_TUNE_REQUEST );
  result  = MIDIDeviceSend( device, message );
  MIDIMessageRelease( message );
  return result;
}

/**
 * @brief Receive an "End Of Exclusive" message.
 * This is called whenever the device receives an "End Of Exclusive" message.
 * It can be used to simulate the reception of such a message.
 * @see MIDIDeviceReceiveSystemExclusive
 * @public @memberof MIDIDevice
 * @param device The midi device.
 * @retval 0 on success.
 * @retval 1 if the message could not be processed.
 */
int MIDIDeviceReceiveEndOfExclusive( struct MIDIDevice * device ) {
  MIDIPrecond( device != NULL, EFAULT );
  if( device->delegate == NULL || device->delegate->recv_eox == NULL ) {
    return 0;
  }
  return (*device->delegate->recv_eox)( device );
}

/**
 * @brief Send an "End Of Exclusive" message.
 * This is called when the device wants to send an "End Of Exclusive" message.
 * It can be used to trigger the sending of such a message.
 * @see MIDIDeviceReceiveSystemExclusive
 * @public @memberof MIDIDevice
 * @param device The midi device.
 * @retval 0 on success.
 * @retval 1 if the message could not be sent.
 */
int MIDIDeviceSendEndOfExclusive( struct MIDIDevice * device ) {
  struct MIDIMessage * message;
  int result;
  MIDIPrecond( device != NULL, EFAULT );
  message = MIDIMessageCreate( MIDI_STATUS_END_OF_EXCLUSIVE );
  result  = MIDIDeviceSend( device, message );
  MIDIMessageRelease( message );
  return result;
}

/**
 * @brief Receive a "Real-Time" message.
 * This is called whenever the device receives a "Real-Time" message.
 * It can be used to simulate the reception of such a message.
 * @public @memberof MIDIDevice
 * @param device    The midi device.
 * @param status    The actual status of the message.
 * @param timestamp The message timestamp.
 * @retval 0 on success.
 * @retval 1 if the message could not be processed.
 */
int MIDIDeviceReceiveRealTime( struct MIDIDevice * device, MIDIStatus status, MIDITimestamp timestamp ) {
  int result;
  MIDIPrecond( device != NULL, EFAULT );
  MIDIPrecond( status >= MIDI_STATUS_TIMING_CLOCK, EINVAL );
  MIDIPrecond( status <= MIDI_STATUS_RESET, EINVAL );
  
  result = _recv_rt( device, status, timestamp );
  if( device->delegate == NULL || device->delegate->recv_rt == NULL ) {
    return result;
  }
  return result + (*device->delegate->recv_rt)( device, status, timestamp );
}

/**
 * @brief Send a "Real-Time" message.
 * This is called when the device wants to send a "Real-Time" message.
 * It can be used to trigger the sending of such a message.
 * @public @memberof MIDIDevice
 * @param device    The midi device.
 * @param status    The actual status of the message.
 * @param timestamp The message timestamp.
 * @retval 0 on success.
 * @retval 1 if the message could not be sent.
 */
int MIDIDeviceSendRealTime( struct MIDIDevice * device, MIDIStatus status, MIDITimestamp timestamp ) {
  struct MIDIMessage * message;
  int result;
  MIDIPrecond( device != NULL, EFAULT );
  MIDIPrecond( status >= MIDI_STATUS_TIMING_CLOCK, EINVAL );
  MIDIPrecond( status <= MIDI_STATUS_RESET, EINVAL );
  
  message = MIDIMessageCreate( status );
  result  = MIDIMessageSetTimestamp( message, timestamp );
  result += MIDIDeviceSend( device, message );
  MIDIMessageRelease( message );
  return result;
}

/** @} */
