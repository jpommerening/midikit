#include <stdio.h>

extern int test_error;

#define ASSERT( expr, message ) \
  if( ! (expr) ) { \
    printf( "%s:%i: error: assert failed: %s\n", __FILE__, __LINE__, message ); \
    return 1; \
  }

#define ASSERT_EQUAL( x, y, message ) \
  if( (x) != (y) ) { \
    printf( "%s:%i: error: assert failed ( " #x " is not equal to " #y "): %s\n", __FILE__, __LINE__, message ); \
    return 1; \
  }

