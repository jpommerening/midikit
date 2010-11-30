#include <stdlib.h>
#include "util.h"

int MIDIUtilReadVarLen( unsigned char * buffer, size_t size, MIDIVarLen * value, size_t * read ) {
  MIDIVarLen v = 0;
  size_t p = 0;
  if( buffer == NULL || value == NULL ) return 1;

  do {
    if( p>=size ) return 1;
    v = (v<<7) | (buffer[p] & 0x7f);
  } while( buffer[p++] & 0x80 );
  *value = v & 0x0fffffff;
  if( read != NULL ) *read  = p;
  return 0;
}

int MIDIUtilWriteVarLen( unsigned char * buffer, size_t size, MIDIVarLen * value, size_t * written ) {
  MIDIVarLen v = 0;
  unsigned char tmp[4] = { 0x80, 0x80, 0x80, 0x00 };
  size_t p = 0, q = 0;
  if( buffer == NULL || value == NULL ) return 1;
  v = *value & 0x0fffffff;
  do {
    tmp[3-p] |= v & 0x7f;
    v >>= 7;
    p++;
  } while( v != 0 && p<4 );
  if( p>size ) return 1;
  q = 4-p;
  for( p=0; q+p<4; p++ ) {
    buffer[p] = tmp[q+p];
  }
  if( written != NULL ) *written = p;
  return 0;
}
