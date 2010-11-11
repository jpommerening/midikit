#include <time.h>
#include "clock.h"

static MIDISamplingRate _sampling_rate = MIDI_SAMPLING_RATE_DEFAULT;
static double _divisor = CLOCKS_PER_SEC / MIDI_SAMPLING_RATE_DEFAULT;
static MIDIClock _now_offset = 0;

static MIDIClock _get_real_time() {
  return ( (double) clock() ) / _divisor;
}

int MIDIClockSetNow( MIDIClock now ) {
  MIDIClock current = _get_real_time();
  _now_offset = now - current;
  return 0;
}

int MIDIClockGetNow( MIDIClock * now ) {
  *now = _get_real_time() + _now_offset;
  return 0;
}

int MIDIClockSetSamplingRate( MIDISamplingRate rate ) {
  _sampling_rate = rate;
  _divisor = CLOCKS_PER_SEC / ( _sampling_rate );
  return 0;
}

int MIDIClockGetSamplingRate( MIDISamplingRate * rate ) {
  *rate = _sampling_rate;
  return 0;
}

int MIDIClockToSeconds( MIDIClock clock, double * seconds ) {
  *seconds = clock / _sampling_rate;
  return 0;
}

int MIDIClockFromSeconds( MIDIClock * clock, double seconds ) {
  *clock = seconds * _sampling_rate;
  return 0;
}
