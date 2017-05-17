#include <stdio.h>
#include <stdint.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/ip.h>
#include <string.h>
#include <assert.h>

#include "server.h"
#include "packet.h"


bool server_init(server* serv, struct sockaddr_in* addr) {
  // Setup a socket
  if ((serv->sock_fd = socket(AF_INET, SOCK_DGRAM, 0)) < 0) {
    perror("server_init(): Couldn't create socket");
    return false;
  }


  // Fill in with default address if needed
  if (addr == NULL) {
    memset(&serv->addr, 0, sizeof(serv->addr));
    serv->addr.sin_addr.s_addr = htonl(INADDR_ANY);
    serv->addr.sin_port = htons(DEFAULT_PORT);
  } else {
    memcpy(&serv->addr, addr, sizeof(struct sockaddr_in));
  T}


  // Initialize next expected sequence numbers
  memset(serv->last_seq, 0, sizeof(serv->last_seq));


  return true;
}


// Process a received packet
void server_process_packet(server* serv, uint8_t const* raw_packet, struct sockaddr_in const* ret) {
  packet_info pi; // For storing interpreted packet
  reject_code code; // For packet checking


  // Check the packet for errors
  if (!server_check_packet(serv, raw_packet, &pi, &code)) {
    // Print out errors server-side
    fprintf(stderr, "server_check_packet(): From client %d:\n", pi.id);
    switch (code) {
      case OUT_OF_SEQ:
        fprintf(stderr, "server_process_packet(): Received out-of-sequence\n");
        break;
      case BAD_LEN:
        fprintf(stderr, "server_process_packet(): Length mismatch\n"); 
        break;
      case NO_END:
        fprintf(stderr, "server_process_packet(): No packet terminator\n");
        break;
      case DUP_PACK:
        fprintf(stderr, "server_process_packet(): Received duplicate\n"); 
        break;
      default:
        fprintf(stderr, "Something's horribly wrong\n");
        assert(true);
    }


    // Reply to client with reject message
    server_send_reject(serv, pi.id, ret, code);
  } else { // All is well
    // Send an ACK
    server_send_ack(serv, pi.id, ret);
  }
}


bool server_check_packet(server* serv, uint8_t const* raw_packet, size_t raw_packet_len, packet_info* pi, reject_code* code) {
  uint8_t const* ptr = raw_packet; // Temp pointer for traversing raw packet
  uint16_t start_id, end_id; // To hold raw packet data


  // Tentatively intepret
  start_id = *((uint16_t*)ptr);
  ptr += sizeof(PACKET_START);
  
  pi->id = *((uint8_t*)ptr);
  ptr += sizeof(pi->id);

  // FIXME: need to check? only one type allowed: DATA
  pi->type = *((packet_type*)ptr);
  ptr += sizeof(pi->type);

  pi->cont.data_info.seq_num = *((sequence_num*)ptr);
  ptr += sizeof(pi->cont.data_info.seq_num);

  pi->cont.data_info.len = *((payload_len*)ptr);
  ptr += sizeof(pi->cont.data_info.len);

  pi->cont.data_info.payload = (void*)ptr;
  ptr += pi->cont.data_info.len;

  end_id = *((uint16_t*)ptr);


  // Check for start of packet identifier
  // FIXME: same reject code for this and end of packet id??
  // FIXME: network order matters or no???



  // Check that packet length is consistent  
  // TODO

  // Check for end of packet identifier
  // TODO
  

  // Check sequence number for out-of-sequence/duplication
  // TODO

  return true;
}


// Send an ACK packet
// Precondition: The server just finished processing a valid received packet
// from 'client'.
// Postcondition: The last received sequence number for the client is
// incremented.
void server_send_ack(server* serv, client_id client);


// Run the server, i.e. wait indefinitely for packets, process, and reply with
// ACKs as appropriate.
void server_run(server* serv);
