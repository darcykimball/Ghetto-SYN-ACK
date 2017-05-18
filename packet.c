#include <stdbool.h>
#include <stdlib.h>
#include <assert.h>

#include "packet.h"

//
// Constants
//


const unsigned MAX_IOVEC_LEN = 8;
const uint16_t PACKET_START = 0xFFFF;
const uint16_t PACKET_END = 0xFFFF;


//
// Functions
//


bool is_well_formed(void* flat_packet);


size_t build_iovec_list(packet_info pi, struct iovec* iovec_list) {
  size_t list_len = 0; // Actual size of the list, depending on packet type


  //
  // Build the list
  //
  
  
  // Start identifier

  // Client ID


  // Packet type

  // Packet contents
  switch (pi.type) {
    case DATA:
      // Sequence number

      // Payload length

      // Payload
     
      break;


    case ACK:
      // Received sequence number
      break;


    case REJECT:
      // Reject sub code
      // Received sequence number
      break;


    default:  
      // FIXME: proper error handling!!
      assert(false);
  }
  
  // End of packet  


  return list_len;
}


// TODO
size_t flatten(packet_info const* pi, void* buf) {
}


// TODO
bool interpret_packet(void const* buf, packet_info* pi) {
}

