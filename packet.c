#include <stdbool.h>
#include <stdlib.h>

#include "packet.h"

//
// Constants
//


const unsigned MAX_IOVEC_LEN = 8;
const uint16_t PACKET_START = 0xFFFF;
const uint16_t PACKET_END = 0xFFFF;


//
// Helper function decls
// FIXME


static struct iovec make_iovec(void* data, size_t len);


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
  *iovec_list++ = make_iovec((void*)PACKET_START, sizeof(PACKET_START));
  ++list_len;

  // Client ID
  *iovec_list++ = make_iovec(&pi.id, sizeof(pi.id));
  ++list_len;


  // Packet type
  *iovec_list++ = make_iovec(&pi.type, sizeof(pi.type));
  ++list_len;

  // Packet contents
  switch (pi.type) {
    case DATA:
      // Sequence number
      *iovec_list++ = make_iovec(&pi.cont.data_info.seq_num,
        sizeof(sequence_num));
      ++list_len;

      // Payload length
      *iovec_list++ = make_iovec(&pi.cont.data_info.len, sizeof(payload_len));
      ++list_len;

      // Payload
      *iovec_list++ = make_iovec(pi.cont.data_info.payload,
        pi.cont.data_info.len);
      ++list_len;
     
      break;


    case ACK:
      // Received sequence number
      *iovec_list++ = make_iovec(&pi.cont.ack_info.recvd_seq_num,
        sizeof(sequence_num));
      ++list_len;

      break;


    case REJECT:
      // Reject sub code
      *iovec_list++ = make_iovec(&pi.cont.reject_info.code, 
        sizeof(reject_code));
      ++list_len;
      
      // Received sequence number
      *iovec_list++ = make_iovec(&pi.cont.reject_info.recvd_seq_num,
        sizeof(sequence_num));
      ++list_len;

      break;


    default:  
      // FIXME: proper error handling!!
      exit(1);
  }
  
  // End of packet  
  *iovec_list++ = make_iovec((void*)PACKET_END, sizeof(PACKET_END));
  ++list_len;


  return list_len;
}


//
// Helper functions
//


// FIXME: check that SEND really enforces constness on data!!!
static struct iovec make_iovec(void* data, size_t len) {
  return { .iov_base = data, .iov_len = len }; 
}
