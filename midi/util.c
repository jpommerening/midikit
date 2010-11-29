#include "util.h"

int MIDIUtilReadVarLen( void * buffer, size_t size MIDIVarLen * value, size_t * read ) {
  MIDIVarLen v = 0;
  size_t p = 0;
  if( buffer == NULL || value == NULL ) return 1;
  if( size < 1 ) return 1;
  do {
    v = v<<7 | (buffer[p++] & 0x7f);
    if( p>=size || p>4 ) return 1;
  } while( buffer[p] & 0xf0 );
  *value = v;
  *read  = p;
  return 0;
}

int MIDIUtilWriteVarLen( void * buffer, size_t size, MIDIVarLen * value, size_t * written ) {
  if( buffer == NULL || value == NULL ) return 1;
  if( size < 1 ) return 1;
}
