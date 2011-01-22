#ifdef __APPLE__
#include <arpa/inet.h>

#include "cfintegration.h"
#include "driver.h"

CFRunLoopSourceRef MIDIDriverCreateRunloopSource( struct MIDIDriver * driver ) {
  return NULL;
}

static int _netservice_get_first_addr( CFNetServiceRef netService, socklen_t * size, struct sockaddr ** addr ) {
  CFArrayRef addresses;
  CFDataRef data;
  
  CFNetServiceResolveWithTimeout( netService, 0.1, NULL );
  addresses = CFNetServiceGetAddressing( netService );
  if( CFArrayGetCount( addresses ) < 1 ) {
    return 1;
  }
  data  = CFArrayGetValueAtIndex( addresses, 0 );
  *size = CFDataGetLength( data );
  *addr = CFDataGetBytePtr( data );
  return 0;
}

int MIDIDriverAppleMIDIAddPeerWithCFNetService( struct MIDIDriverAppleMIDI * driver, CFNetServiceRef netService ) {
  socklen_t size;
  struct sockaddr * addr;
  if( _netservice_get_first_addr( netService, &size, &addr ) ) return 1;
  
  return MIDIDriverAppleMIDIAddPeerWithSockaddr( driver, size, addr );
}

int MIDIDriverAppleMIDIRemovePeerWithCFNetService( struct MIDIDriverAppleMIDI * driver, CFNetServiceRef netService ) {
  socklen_t size;
  struct sockaddr * addr;
  if( _netservice_get_first_addr( netService, &size, &addr ) ) return 1;
  
  return MIDIDriverAppleMIDIRemovePeerWithSockaddr( driver, size, addr );
}


#endif
