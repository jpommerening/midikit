#include "test.h"
#include "midi/util.h"

/**
 * Test that the read of varlen numbers works
 */
int test001_util( void ) {
  unsigned char buffer[10] = {
    0x7f,
    0x81, 0x01,       // = 0x81
    0x81, 0x82, 0x03, // = 0x4103
    0x80, 0x80, 0x80, 0x00 // = 0
  };
  MIDIVarLen out[4] = { 0 };
  MIDIVarLen test[4] = { 0x7f, 0x81, 0x4103, 0x00 };
  size_t bytes = 0;
  size_t ptr = 0;


  ASSERT_NO_ERROR( MIDIUtilReadVarLen( &out[0], 10-ptr, buffer+ptr, &bytes ), "Could not read var length number from buffer." );
  ASSERT_EQUAL( out[0], test[0], "Read wrong number." );
  ASSERT_EQUAL( bytes, 1, "Read wrong number of bytes." );
  ptr+=bytes;
  ASSERT_NO_ERROR( MIDIUtilReadVarLen( &out[1], 10-ptr, buffer+ptr, &bytes ), "Could not read var length number from buffer." );
  ASSERT_EQUAL( out[1], test[1], "Read wrong number." );
  ASSERT_EQUAL( bytes, 2, "Read wrong number of bytes." );
  ptr+=bytes;
  ASSERT_NO_ERROR( MIDIUtilReadVarLen( &out[2], 10-ptr, buffer+ptr, &bytes ), "Could not read var length number from buffer." );
  ASSERT_EQUAL( out[2], test[2], "Read wrong number." );
  ASSERT_EQUAL( bytes, 3, "Read wrong number of bytes." );
  ptr+=bytes;
  ASSERT_ERROR( MIDIUtilReadVarLen( &out[3], 9-ptr, buffer+ptr, &bytes ), "Read number that reached out of the buffer." );
  ASSERT_NO_ERROR( MIDIUtilReadVarLen( &out[3], 10-ptr, buffer+ptr, &bytes ), "Could not read var length number from buffer." );
  ASSERT_EQUAL( out[3], test[3], "Read wrong number." );
  ASSERT_EQUAL( bytes, 4, "Read wrong number of bytes." );
  return 0;
}

/**
 * Test that the write of varlen numbers works
 */
int test002_util( void ) {
  MIDIVarLen buffer[4] = { 0x7f, 0x81, 0x4103, 1<<21 };
  unsigned char out[10] = { 0 };
  unsigned char test[10] = {
    0x7f,
    0x81, 0x01,       // = 0x81
    0x81, 0x82, 0x03, // = 0x4103
    0x81, 0x80, 0x80, 0x00 // = 0
  };
  size_t bytes = 0;
  size_t ptr = 0;


  ASSERT_NO_ERROR( MIDIUtilWriteVarLen( &buffer[0], 10-ptr, out+ptr, &bytes ), "Could not write var length number to buffer." );
  ASSERT_EQUAL( out[0], test[0], "Wrote wrong byte (0) of number." );
  ASSERT_EQUAL( bytes, 1, "Wrote wrong number of bytes." );
  ptr+=bytes;
  ASSERT_NO_ERROR( MIDIUtilWriteVarLen( &buffer[1], 10-ptr, out+ptr, &bytes ), "Could not write var length number to buffer." );
  ASSERT_EQUAL( out[1], test[1], "Wrote wrong byte (0) of number." );
  ASSERT_EQUAL( out[2], test[2], "Wrote wrong byte (1) of number." );
  ASSERT_EQUAL( bytes, 2, "Wrote wrong number of bytes." );
  ptr+=bytes;
  ASSERT_NO_ERROR( MIDIUtilWriteVarLen( &buffer[2], 10-ptr, out+ptr, &bytes ), "Could not write var length number to buffer." );
  ASSERT_EQUAL( out[3], test[3], "Wrote wrong byte (0) of number." );
  ASSERT_EQUAL( out[4], test[4], "Wrote wrong byte (1) of number." );
  ASSERT_EQUAL( out[5], test[5], "Wrote wrong byte (2) of number." );
  ASSERT_EQUAL( bytes, 3, "Wrote wrong number of bytes." );
  ptr+=bytes;
  ASSERT_ERROR( MIDIUtilWriteVarLen( &buffer[3], 9-ptr, out+ptr, &bytes ), "Wrote number that reached out of the buffer." );
  ASSERT_NO_ERROR( MIDIUtilWriteVarLen( &buffer[3], 10-ptr, out+ptr, &bytes ), "Could not write var length number to buffer." );
  ASSERT_EQUAL( out[6], test[6], "Wrote wrong byte (0) of number." );
  ASSERT_EQUAL( out[7], test[7], "Wrote wrong byte (1) of number." );
  ASSERT_EQUAL( out[8], test[8], "Wrote wrong byte (2) of number." );
  ASSERT_EQUAL( out[9], test[9], "Wrote wrong byte (3) of number." );
  ASSERT_EQUAL( bytes, 4, "Wrote wrong number of bytes." );
  ptr+=bytes;
  return 0;
}

/**
 * Test the turnaround. Conversion from and to var length numbers.
 */
int test003_util( void ) {
  MIDIVarLen in[6]  = { 0, 123, 1234, 123456, 1234<<13, 123<<21 };
  MIDIVarLen out[6] = { 0xcafebabe, 0 };
  unsigned char buffer[20];
  size_t bytes = 0;
  size_t ptr   = 0;
  int i;
  for( i=0; i<6; i++ ) {
    ASSERT_NO_ERROR( MIDIUtilWriteVarLen( &in[i], 20-ptr, buffer+ptr, &bytes ), "Could not write var length number to buffer." );
    ASSERT_NO_ERROR( MIDIUtilReadVarLen( &out[i], 20-ptr, buffer+ptr, &bytes ), "Could not read var length number from buffer." );
    ASSERT_EQUAL( in[i], out[i], "Variable length number did not survive turnaround unharmed." );
    ptr+=bytes;
  }
  return 0;
}

/**
 * Test that even bad streams can be read and no bad streams are created.
 * (So that the reader can stay in sync, even if it does not understand
 *  one number and the writer can't be fooled into creating invalid streams.)
 */
int test004_util( void ) {
  MIDIVarLen n;
  unsigned char buffer[10] = { 0x81, 0x82, 0x83, 0x84, 0x05 };
  size_t bytes;

  ASSERT_NO_ERROR( MIDIUtilReadVarLen( &n, 10, buffer, &bytes ), "Could not read broken stream." );
  ASSERT_LESS_OR_EQUAL( n, 0x0fffffff, "Over-long number was not truncated." );
  ASSERT_EQUAL( bytes, 5, "Read wrong number of bytes." );
  n = 0x80<<21;
  ASSERT_NO_ERROR( MIDIUtilWriteVarLen( &n, 4, buffer, &bytes ), "Invalid number could not be written." );
  ASSERT_LESS_OR_EQUAL( bytes, 4, "Wrote wrong number of bytes." );
  ASSERT_EQUAL( buffer[bytes-1], 0x00, "Wrote wrong last byte of number." );
  return 0;
}
