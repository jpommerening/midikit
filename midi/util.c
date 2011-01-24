#include <stdlib.h>
#include "util.h"

/**
 * @defgroup MIDI-fnc Utility functions
 * @ingroup MIDI
 * @{
 */
 
/**
 * @brief Read an encoded number with a variable number of bytes.
 * @param value  The location to store the read number in.
 * @param size   The number of available bytes in the buffer.
 * @param buffer The buffer to read from.
 * @param read   The location to store the number of read bytes in.
 *               (may be @c NULL)
 * @retval 0 on success.
 * @retval >0 if the number could not be read.
 */
int MIDIUtilReadVarLen( MIDIVarLen * value, size_t size, unsigned char * buffer, size_t * read ) {
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

/**
 * @brief Write a number encoding it with a variable number of bytes.
 * @param value   The location to read the number from.
 * @param size    The number of available bytes in the buffer.
 * @param buffer  The buffer to write to.
 * @param written The location to store the number of written bytes in.
 *               (may be @c NULL)
 * @retval 0 on success.
 * @retval >0 if the number could not be read.
 */
int MIDIUtilWriteVarLen( MIDIVarLen * value, size_t size, unsigned char * buffer, size_t * written ) {
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

/** @} */