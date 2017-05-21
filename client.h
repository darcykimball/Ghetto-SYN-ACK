#ifndef CLIENT_H
#define CLIENT_H


#include <stdio.h>
#include <time.h>
#include <netinet/ip.h>

#include "packet.h"


#define MAX_SEQ_NUM  0xFF // Maximum sequence number
#define MAX_TRIES    3    // Maximum times to try sending
#define TIMEOUT      3    // Amount of time, in seconds, to wait for ACK


// Controls verbosity of client-side output
// FIXME: pass as param??
extern bool verbose;


// Ack timer
typedef struct {
  time_t last_sent; // Time of the most recent send attempt
                    // XXX: Not needed, but here for debugging purposes
  uint8_t tries;   // Number of send attempts so far (max 3? FIXME)
} ack_timer;


// Struct for state of client
typedef struct {
  ack_timer timers[MAX_SEQ_NUM]; // Mapping from packet seq. numbers to ack_timer
  int sock_fd; // The socket to use for sending
  struct sockaddr_in addr; // The client's internet address
  uint8_t send_buf[BUFSIZ]; // Buffer for preparing packets to send
  uint8_t recv_buf[BUFSIZ]; // Buffer for received packets
} client;


// Client initialization
// Args:
//   cl - The client object
//   addr - A desired address to use; if NULL, INADDR_ANY is used 
// Postcondition: The passed-in client is initialized with socket/address/timers
// Return value: true if everything was OK, false if not able to initialize
bool client_init(client* cl, struct sockaddr_in const* addr);


// Send a packet using the specified socket, timing out and retrying as
// necessary
void client_send_packet(client* cl, packet_info const* pi, struct sockaddr_in const* dest);


// Receive (blocking) and process packets (can only be ACKs in current context)
bool client_recv_packet(client* cl);

  
#endif // CLIENT_H
