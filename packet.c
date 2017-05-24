#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <arpa/inet.h>

#include "packet.h"
#include "raw_iterator.h"


//
// Constants
//


const unsigned MAX_IOVEC_LEN = 8;
const uint16_t PACKET_START = 0xFFFF;
const uint16_t PACKET_END = 0xFFFF;


// TODO
// FIXME: network order!!!
size_t flatten(packet_info const* pi, void* buf, size_t size) {
  raw_iterator rit; // For writing to buffer
  rit_init(&rit, (uint8_t*)buf, size);
  size_t flattened_size = 0; // Size of the flattened packet in the buffer
  uint16_t network_short; // For converting shorts to network order
  

  // Add common field sizes to the total packet size
  flattened_size +=
    sizeof(PACKET_START) +
    sizeof(client_id) +
    sizeof(uint16_t) +  // Size of packet_type on the wire
    sizeof(PACKET_END);



  // Header
  rit_write(&rit, sizeof(PACKET_START), &PACKET_START);
  
  
  // Client ID
  rit_write(&rit, sizeof(client_id), &pi->id);


  // Packet type
  network_short = htons(pi->type);
  rit_write(&rit, sizeof(uint16_t), &network_short);


  // XXX: For 1-byte fields, conversion to network order is not necessary;
  // same for start and end flags b/c they'd be the same after conversion
  switch (pi->type) {
    case DATA:
      // Sequence number
      rit_write(&rit, sizeof(sequence_num), &pi->cont.data_info.seq_num);

      // Data length
      rit_write(&rit, sizeof(payload_len), &pi->cont.data_info.len);

      // Payload
      rit_write(&rit, pi->cont.data_info.len, pi->cont.data_info.payload);

      flattened_size +=
        sizeof(sequence_num) +
        sizeof(payload_len) +
        pi->cont.data_info.len;

      break;
    case ACK:
      // Received sequence number
      rit_write(&rit, sizeof(sequence_num), &pi->cont.ack_info.recvd_seq_num);

      flattened_size += sizeof(sequence_num);

      break;
    case REJECT:
      // Reject code
      network_short = htons(pi->cont.reject_info.code);
      rit_write(&rit, sizeof(uint16_t), &network_short);

      // Received sequence number
      rit_write(&rit, sizeof(sequence_num),
        &pi->cont.reject_info.recvd_seq_num);

      flattened_size +=
        sizeof(uint16_t) + // Reject code size
        sizeof(sequence_num);
              

      break;
    default:
      // Should really be unreachable
      fprintf(stderr, "flatten: Invalid type constant!\n");
      assert(0);
  }

  rit_write(&rit, sizeof(PACKET_END), &PACKET_END);


  return flattened_size;
}


// FIXME: How to check for length mismatch??
int interpret_packet(void const* buf, packet_info* pi, size_t size) {
  raw_iterator rit; // Raw iterator for reading buffer
  rit_init(&rit, (uint8_t*)buf, size); // XXX: no constness is OK; read only
  uint16_t network_short; // For reading shorts


  // Check header
  // XXX: no network to host conversion needed in this case
  rit_read(&rit, sizeof(uint16_t), &network_short);
  if (network_short != PACKET_START) {
    fprintf(stderr, "interpret_packet: Bad PACKET_START!\n");
    
    // XXX: I overloaded NO_END; see packet.h
    return NO_END;
  }


  // Read client ID
  rit_read(&rit, sizeof(client_id), &pi->id);


  // Read packet type
  rit_read(&rit, sizeof(uint16_t), &network_short);
  pi->type = (packet_type)ntohs(network_short);


  // Interpret based on type
  switch (pi->type) {
    case DATA:
      // Sequence number
      rit_read(&rit, sizeof(sequence_num), &pi->cont.data_info.seq_num);

      // Payload length
      rit_read(&rit, sizeof(payload_len), &pi->cont.data_info.len);

      // Payload; save ptr to it and skip over
      pi->cont.data_info.payload = rit.curr;
      rit.curr += pi->cont.data_info.len; // FIXME: use a method??
        
      break;

    case ACK:
      // Received sequence number
      rit_read(&rit, sizeof(sequence_num), &pi->cont.ack_info.recvd_seq_num);
      
      break;

    case REJECT:
      // Reject code
      rit_read(&rit, sizeof(uint16_t), &network_short);
      pi->cont.reject_info.code = (reject_code)ntohs(network_short);

      // Received sequence number
      rit_read(&rit, sizeof(sequence_num), &pi->cont.reject_info.recvd_seq_num);
      break;

    default:
      // Invalid packet type was found
      fprintf(stderr, "interpret_packet: Bad packet type!\n");
      return BAD_TYPE;
  }


  // Check packet end flag
  rit_read(&rit, sizeof(uint16_t), &network_short);
  if (network_short != PACKET_END) {
    fprintf(stderr, "interpret_packet: Bad PACKET_END!\n");
    return NO_END; 
  }

  
  return 0;
}

