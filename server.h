#ifndef SERVER_H
#define SERVER_H


#include <netinet/ip.h>

#include "packet.h"

#define DEFAULT_PORT 4321

// Get the size of the range of an unsigned type
// XXX: taken from http://stackoverflow.com/questions/2053843/min-and-max-value-of-data-type-in-c
#define urange(t) ((((0x1ULL << ((sizeof(t) * 8ULL) - 1ULL)) - 1ULL) | \
                    (0xFULL << ((sizeof(t) * 8ULL) - 4ULL))) + 1)

// Server state
typedef struct {
  sequence_num last_seq[urange(client_id)]; // Mapping of clients to next
                                            // expected sequence numbers  
  int sock_fd; // The server's send/recv socket
  struct sockaddr_in addr;  // The server's IP address/port
} server;


// Initialize server
// Args:
//   serv - the server object
//   addr - A desired address to use; if NULL, INADDR_ANY is used, with
//   DEFAULT_PORT
// Return value: True if initialization was OK, false otherwise.
bool server_init(server* serv, struct sockaddr_in* addr);


// Process a received packet
void server_process_packet(server* serv, uint8_t const* raw_packet, struct sockaddr_in* ret);


// Check if a (presumably received) packet is well-formed, setting the given
// packet_info if interpretation succeeded (packet was well-formed)
// XXX: The length of the packet is deduced from the (expected) length field
// Args:
//   raw_packet - The buffer with the raw, flattened packet data
//   pi - The packet info for storing interpretation of the packet. If NULL,
//        it's unused.
// Return: True if the packet was OK, false otherwise
// Postconditions: Argument packet_info (if non-NULL) is set to the info in 
// the interpreted packet; as much data as possible is set, even if the packet
// is rejected. 'code' is set to the appropriate reject code, if needed.
bool server_check_packet(server* serv, uint8_t const* raw_packet, size_t raw_packet_len, packet_info* pi, reject_code* code);


// Send an ACK packet
// Precondition: The server just finished processing a valid received packet
// from 'client'.
// Postcondition: The last received sequence number for the client is
// incremented.
void server_send_ack(server* serv, client_id client, struct sockaddr_in const* ret);


// Send a reject (error) packet
void server_send_reject(server* serv, client_id client, struct sockaddr_in const* ret, reject_code code);



// Run the server, i.e. wait indefinitely for packets, process, and reply with
// ACKs as appropriate.
void server_run(server* serv);


#endif // SERVER_H
