#include <stdio.h>
#include <time.h>
#include <string.h>
#include <netinet/ip.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/time.h>

#include "client.h"
#include "packet.h"


bool client_init(client* cl, struct sockaddr_in const* addr) {
  struct timeval timeout; // For setting recv() timeout value
  timeout.tv_sec = TIMEOUT;
  timeout.tv_usec = 0;


  memset(cl, 0, sizeof(client));
  if (addr == NULL) {
    cl->addr.sin_family = AF_INET;
    cl->addr.sin_port = htons(0); // Let bind() choose a random port
    cl->addr.sin_addr.s_addr = htonl(INADDR_ANY);
  } else {
    memcpy(&cl->addr, addr, sizeof(struct sockaddr_in));
  }


  // Setup the socket
  if (bind(cl->sock_fd,
      (struct sockaddr*)&cl->addr, sizeof(cl->addr)) < 0) {
    perror("client_init: Couldn't bind address to socket!");
    return false;
  }


  // Set timeout value for receiving
  // FIXME: renamd sock_fd!! as we use it for both sending/receiving
  setsockopt(cl->sock_fd, SOL_SOCKET, SO_RCVTIMEO, 
    (const void*)&timeout, sizeof(struct timeval));


  return true;
}


void client_send_packet(client* cl, packet_info const* pi, struct sockaddr_in const* dest) {
  size_t raw_len; // Length of data to send
  packet_info reply_pi; // Packet data of any replies (ACKs)


  // Serialize packet data
  raw_len = flatten(pi, cl->send_buf);

  
  // Send
  if (sendto(cl->sock_fd, cl->send_buf, raw_len, 0,
           (struct sockaddr*)dest, sizeof(struct sockaddr_in)) == -1) {
    perror("client_send_packet: Failed to send!");
    return;
  }


  // Start timer and wait for ACK if data was sent
  // FIXME: looping over tries and keeping track of tries is REDUNDANT!!
  // FIXME: so is using a timeout! while keeping track of times!!
  if (pi->type == DATA) {
    while (true) {
      cl->timers[pi->cont.data_info.seq_num].last_sent = time(NULL);

      // Wait...then check if timed out
      if (
               !client_recv_packet(cl)
            || !interpret_packet(cl->recv_buf, &reply_pi)
            || reply_pi.type != ACK
      ) {
        // Timed out or had an invalid packet; update tries and try again
        if (++cl->timers[pi->cont.data_info.seq_num].tries > MAX_TRIES) {
          // Max tries reached; give up
          fprintf(stderr,
            "client_send_packet: Maximum tries reached! Giving up.\n");
          return;
        }


        // Keep trying
        continue;
      }


      // Got an ACK after all
      fprintf(stderr,
        "client_send_packet: Got an ACK for sequence number: %u\n",
        pi->cont.data_info.seq_num);
      return;
    }
  }
}


bool client_recv_packet(client* cl) {
  if (recv(cl->sock_fd, cl->recv_buf, sizeof(cl->recv_buf), 0) == -1) {
    perror("client_recv_packet:");
    return false;
  }


  return true;
}
