#include <stdio.h>
#ifndef MIDI_LOG_CHANNELS
#define MIDI_LOG_CHANNELS MIDI_LOG_TEST
#endif
#include "midi/midi.h"

#define _EXPAND( v ) v
#define EXPAND( v ) _EXPAND( v )

#define ASSERT_BASE( expr, message, print_expr ) \
  if( ! (expr) ) { \
    MIDILogLocation( TEST, "error: assertion failed: %s\n  " print_expr "\n", message ); \
    return 1; \
  }

#define MIDI_LOG_TEST 0x20

#define ASSERT( expr, message ) ASSERT_BASE( expr, message, #expr );

#define ASSERT_CMP( x, op, y, message, print_expr ) ASSERT_BASE( (x) op (y), message, print_expr )

#define ASSERT_EQUAL( x, y, message )            ASSERT_CMP( x, ==, y, message, #x " has to be equal to " #y "!" )
#define ASSERT_NOT_EQUAL( x, y, message )        ASSERT_CMP( x, !=, y, message, #x " may not be equal to " #y "!" )
#define ASSERT_GREATER( x, y, message )          ASSERT_CMP( x, >, y, message, #x " has to be greater than " #y "!" )
#define ASSERT_LESS( x, y, message )             ASSERT_CMP( x, <, y, message, #x " has to be less than " #y "!" )
#define ASSERT_GREATER_OR_EQUAL( x, y, message ) ASSERT_CMP( x, >=, y, message, #x " has to be greater or equal " #y "!" )
#define ASSERT_LESS_OR_EQUAL( x, y, message )    ASSERT_CMP( x, <=, y, message, #x " has to be less or equal " #y "!" )

#define ASSERT_NEAR_DELTA 0.01
#define ASSERT_NEAR_LESS_NUM( y )    (y)*( ((y)>=0) ? (1.0-ASSERT_NEAR_DELTA) : (1.0+ASSERT_NEAR_DELTA) )
#define ASSERT_NEAR_GREATER_NUM( y ) (y)*( ((y)>=0) ? (1.0+ASSERT_NEAR_DELTA) : (1.0-ASSERT_NEAR_DELTA) )

#define ASSERT_NEAR_LESS( x, y, message )    ASSERT_BASE( (x) <= (y) && (x) >= ASSERT_NEAR_LESS_NUM(y), message, \
                                             #x " has to be near " #y " but not more!" )
#define ASSERT_NEAR_GREATER( x, y, message ) ASSERT_BASE( (x) >= (y) && (x) <= ASSERT_NEAR_GREATER_NUM(y), message, \
                                             #x " has to be near " #y " but not less!" )
#define ASSERT_NEAR( x, y, message )         ASSERT_BASE( (x) >= ASSERT_NEAR_LESS_NUM(y) && (x) <= ASSERT_NEAR_GREATER_NUM(y), message, \
                                             #x " has to be near " #y "!")

#define ASSERT_ERROR_CHECKER MIDIErrorNumber
#define ASSERT_NO_ERROR( command, message )  ASSERT_BASE( (command) == 0 && ASSERT_ERROR_CHECKER == 0, message, #command " returned an error!" ) 
#define ASSERT_ERROR( command, message )  ASSERT_BASE( (command) != 0 || ASSERT_ERROR_CHECKER != 0, message, #command " returned no error!" ) 
