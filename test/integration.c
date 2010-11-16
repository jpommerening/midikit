#include <stdint.h>
#include "test.h"
#include "midi/midi.h"
#include "midi/connector.h"
#include "midi/message.h"
#include "midi/device.h"
#include "midi/driver.h"

static int _receive_nof( struct MIDIDevice * device, MIDIChannel channel, MIDIKey key, MIDIVelocity velocity ) {
  return 0;
}

static int _receive_non( struct MIDIDevice * device, MIDIChannel channel, MIDIKey key, MIDIVelocity velocity ) {
  return 0;
}

static int _receive_pkp( struct MIDIDevice * device, MIDIChannel channel, MIDIKey key, MIDIPressure pressure ) {
  return 0;
}

static int _receive_cc( struct MIDIDevice * device, MIDIChannel channel, MIDIControl control, MIDIValue value ) {
  return 0;
}

static int _receive_pc( struct MIDIDevice * device, MIDIChannel channel, MIDIProgram program ) {
  return 0;
}

static int _receive_cp( struct MIDIDevice * device, MIDIChannel channel, MIDIPressure pressure ) {
  return 0;
}

static int _receive_pwc( struct MIDIDevice * device, MIDIChannel channel, MIDILongValue value ) {
  return 0;
}

static int _receive_sx( struct MIDIDevice * device, MIDIManufacturerId manufacturer_id, size_t size, void * data, uint8_t fragment ) {
//uint8_t * values = data;
  return 0;
}

static int _receive_tcqf( struct MIDIDevice * device, MIDIValue time_code_type, MIDIValue value ) {
  return 0;
}

static int _receive_spp( struct MIDIDevice * device, MIDILongValue value ) {
  return 0;
}

static int _receive_ss( struct MIDIDevice * device, MIDIValue value ) {
  return 0;
}

static int _receive_tr( struct MIDIDevice * device ) {
  return 0;
}

static int _receive_eox( struct MIDIDevice * device ) {
  return 0;
}

static int _receive_rt( struct MIDIDevice * device, MIDIStatus status ) {
  return 0;
}

static struct MIDIDeviceDelegate _test_device = {
  &_receive_nof,
  &_receive_non,
  &_receive_pkp,
  &_receive_cc,
  &_receive_pc,
  &_receive_cp,
  &_receive_pwc,
  &_receive_sx,
  &_receive_tcqf,
  &_receive_spp,
  &_receive_ss,
  &_receive_tr,
  &_receive_eox,
  &_receive_rt
};

static int _send( struct MIDIDriver * driver, struct MIDIMessage * message ) {
  return 0;
}

static struct MIDIDriverDelegate _test_driver = {
  &_send
};

/**
 * Integration test.
 */
int test001_integration( void ) {
  return 0;
}
