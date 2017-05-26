#include <stdio.h>
#include <time.h>
#include <string.h>
#include <netinet/ip.h>
#include <arpa/inet.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/time.h>

#include "client.h"
#include "packet.h"
#include "busywait.h"


bool client_init(client* cl, struct sockaddr_in const* addr) {
 cl->timeout.tv_sec = TIMEOUT;
 cl->timeout.tv_usec = 0;


  // Initialize address
  memset(cl, 0, sizeof(client));
  if (addr == NULL) {
    // FIXME: make more general??
    cl->addr.sin_family = AF_INET;
    cl->addr.sin_port = htons(0); // Let bind() choose a random port
    //inet_pton(AF_INET, "127.0.0.1", &cl->addr.sin_addr.s_addr);
    //inet_pton(AF_INET, "172.21.127.182", &cl->addr.sin_addr.s_addr);

    // FIXME: which approach??
    cl->addr.sin_addr.s_addr = htonl(INADDR_ANY);
  } else {
    memcpy(&cl->addr, addr, sizeof(struct sockaddr_in));
  }


  // Setup the socket
  if ((cl->sock_fd = socket(PF_INET, SOCK_DGRAM, 0)) < 0) {
    perror("client_init: Couldn't create socket!");
    return false;
  }

  if (bind(cl->sock_fd,
      (struct sockaddr*)&cl->addr, sizeof(struct sockaddr_in)) < 0) {
    perror("client_init: Couldn't bind address to socket!");
    return false;
  }


  return true;
}


void client_send_packet(client* cl, packet_info const* pi, struct sockaddr_in const* dest) {
  size_t raw_len; // Length of data to send
  packet_info reply_pi; // Packet data of any replies (ACKs)


  // Serialize packet data
  raw_len = flatten(pi, cl->send_buf, sizeof(cl->send_buf));


  
  // Send
  if (sendto(cl->sock_fd, cl->send_buf, raw_len, 0,
           (struct sockaddr*)dest, sizeof(struct sockaddr_in)) == -1) {
    perror("client_send_packet: Failed to send!");
    return;
  }

  
  // Start timer and wait for ACK if data was sent
  // TODO/FIXME: clean up control flow!!!
  if (pi->type == DATA) {
    sequence_num seq_num = pi->cont.data_info.seq_num; // Alias for readability


    while (cl->tries[seq_num] < MAX_TRIES) {
      // Wait...then check if timed out
      if (
               !client_recv_packet(cl) // XXX: The waiting happens here
            || !interpret_packet(cl->recv_buf, &reply_pi, sizeof(cl->recv_buf))
            || reply_pi.type != ACK
      ) {
        // Timed out or had an invalid packet; update tries and try again
        if (++cl->tries[seq_num] > MAX_TRIES) {
          // Max tries reached; give up and reset info for that sequence number
          fprintf(stderr,
            "client_send_packet: Maximum tries reached! Giving up.\n");

          cl->tries[seq_num] = 0;

          return;
        }
      } else {
        // Got an ACK after all
        fprintf(stderr,
          "client_send_packet: Got an ACK for sequence number: %u\n",
          pi->cont.data_info.seq_num);
        return;
      }
    }
  }
}


bool client_recv_packet(client* cl) {
  ssize_t n_recvd; // For retval of recv()
  DEFINE_ARGS(recv, sargs,
    cl->sock_fd,
    cl->recv_buf,
    sizeof(cl->recv_buf),
    0
  );


  return busy_wait_until(cl->timeout, &try_recv, &sargs, &n_recvd);
}
