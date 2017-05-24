#ifndef SERVER_H
#define SERVER_H


#include <stddef.h>
#include <stdlib.h>
#include <netinet/ip.h>

#include "packet.h"


#define DEFAULT_PORT 4321

// Get the size of the range of an unsigned type
// XXX: taken from http://stackoverflow.com/questions/2053843/min-and-max-value-of-data-type-in-c
#define urange(t) ((((0x1ULL << ((sizeof(t) * 8ULL) - 1ULL)) - 1ULL) | \
                    (0xFULL << ((sizeof(t) * 8ULL) - 4ULL))) + 1)


// Server state
typedef struct {
  sequence_num expect_recv[urange(client_id)]; // Mapping of clients to next
                                            // expected sequence numbers  
  int sock_fd; // The server's send/recv socket
  struct sockaddr_in addr;  // The server's IP address/port
  uint8_t send_buf[BUFSIZ]; // Buffer for packets to be sent
  uint8_t recv_buf[BUFSIZ]; // Buffer for received packets
} server;


// Initialize server
// Args:
//   serv - the server object
//   addr - A desired address to use; if NULL, INADDR_ANY is used, with
//   DEFAULT_PORT
// Return value: True if initialization was OK, false otherwise.
bool server_init(server* serv, struct sockaddr_in* addr);


// Process a received packet
void server_process_packet(server* serv, struct sockaddr_in const* ret);


// Send an ACK packet
// Precondition: The server just finished processing a valid received packet
// from 'client'.
// Postcondition: The last received sequence number for the client is
// incremented.
void server_send_ack(server* serv, client_id client, struct sockaddr_in const* ret);


// Send a reject (error) packet
void server_send_reject(server* serv, packet_info const* bad_pi,
  struct sockaddr_in const* ret, reject_code code);


// Run the server, i.e. wait indefinitely for packets, process, and reply with
// ACKs as appropriate.
void server_run(server* serv);


#endif // SERVER_H
