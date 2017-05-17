#ifndef CLIENT_H
#define CLIENT_H


#include <netinet/ip.h>
#include <time.h>

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
  unsigned tries;   // Number of send attempts so far
} ack_timer;


// Struct for state of client
typedef struct {
  ack_timer timers[MAX_SEQ_NUM]; // Mapping from packet seq. numbers to ack_timer
  int send_sock_fd; // The socket to use for sending
  struct sockaddr_in addr; // The client's internet address
} client;


// Client initialization
// Args:
//   cl - The client object
//   addr - A desired address to use; if NULL, INADDR_ANY is used 
// Postcondition: The passed-in client is initialized with socket/address/timers
// Return value: true if everything was OK, false if not able to initialize
void client_init(client* cl, struct sockaddr_in* addr);


// Send a packet using the specified socket, timing out and retrying as
// necessary
void client_send_packet(client* cl, packet_info* pi, struct sockaddr_in dest);

  
#endif // CLIENT_H
