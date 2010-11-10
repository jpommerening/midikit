#include <stdio.h>

extern int test_error;

#define ASSERT( expr, message ) \
  if( ! (expr) ) { \
    printf( "%s:%i: error: assert failed: %s\n", __FILE__, __LINE__, message ); \
    return 1; \
  }
