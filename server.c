#include <stdio.h>
#include <stdint.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/ip.h>
#include <arpa/inet.h>
#include <string.h>
#include <assert.h>

#include "server.h"
#include "packet.h"


bool server_init(server* serv, struct sockaddr_in* addr) {
  char ip_str[INET_ADDRSTRLEN]; // For printing info


  fprintf(stderr, "server_init: Initializing server...\n");


  // Setup a socket
  if ((serv->sock_fd = socket(AF_INET, SOCK_DGRAM, 0)) < 0) {
    perror("server_init: Couldn't create socket");
    return false;
  }


  // Fill in with default address if needed
  if (addr == NULL) {
    memset(&serv->addr, 0, sizeof(serv->addr));
    serv->addr.sin_family = AF_INET;
    serv->addr.sin_addr.s_addr = htonl(INADDR_ANY);
    serv->addr.sin_port = htons(DEFAULT_PORT);
  } else {
    memcpy(&serv->addr, addr, sizeof(struct sockaddr_in));
  }


  // Bind the socket
  if ((bind(serv->sock_fd, (struct sockaddr*)&serv->addr,
        sizeof(struct sockaddr_in)) == -1)) {
    perror("server_init: Couldn't bind socket to address");
    return false;
  }


  // Initialize next expected sequence numbers
  memset(serv->expect_recv, 0, sizeof(serv->expect_recv));


  fprintf(stderr, "Done initializing!\n");
  fprintf(stderr, "Server IP: %s\n",
    inet_ntop(AF_INET, &serv->addr.sin_addr, ip_str, INET_ADDRSTRLEN));
  fprintf(stderr, "Server port: %u\n", ntohs(serv->addr.sin_port));


  return true;
}


// Process a received packet
void server_process_packet(server* serv, struct sockaddr_in const* ret) {
  packet_info pi; // For storing interpreted packet
  int code; // For return value of server_check_packet()
  char ip_str[INET_ADDRSTRLEN]; // For pretty-printing addresses


  // Validate
  code = server_check_packet(serv, &pi);


  // Notify of any errors if present
  if (code != 0) {
    alert_reject(&pi, (reject_code)code);
    
    // Reply to client with reject message
    fprintf(stderr, "server_run: Sending REJECT message\n");
    server_send_reject(serv, &pi, ret, (reject_code)code);

  } else { // All is well
    fprintf(stderr, "server_run: Received message: \"%s\"\n",
      (char const*)pi.cont.data_info.payload);
    fprintf(stderr, "server_run: from %s:%u\n", inet_ntop(AF_INET,
      &ret->sin_addr, ip_str, INET_ADDRSTRLEN), ntohs(ret->sin_port));


    // Send an ACK, and update next expected sequence number
    fprintf(stderr, "server_run: Sending ACK for sequence number %u\n",
      pi.cont.data_info.seq_num);
    server_send_ack(serv, pi.id, ret);
    ++serv->expect_recv[pi.id];
  }
}


// TODO
void server_send_ack(server* serv, client_id id, struct sockaddr_in const* ret) {
  char ip_str[INET_ADDRSTRLEN]; // For msg printing


  packet_info pi;
  pi.type = ACK;
  pi.id = id;
  // FIXME: check!!! for off by 1???
  pi.cont.ack_info.recvd_seq_num = serv->expect_recv[id];


  size_t flattened_len = flatten(&pi, serv->send_buf, sizeof(serv->send_buf));
  // FIXME: check number of sent bytes???
  // FIXME: remove!!
  
  fprintf(stderr, "server_send_ack: Sending ACK to %s:%u\n",
    inet_ntop(AF_INET,
    &ret->sin_addr, ip_str, INET_ADDRSTRLEN), ntohs(ret->sin_port));

  ssize_t retval = 
  sendto(
    serv->sock_fd,
    serv->send_buf,
    flattened_len,
    0,
    (struct sockaddr*)ret,
    sizeof(struct sockaddr_in)
  ); 

  // FIXME: remove!
  sendto(
    serv->sock_fd,
    serv->send_buf,
    flattened_len,
    0,
    (struct sockaddr*)ret,
    sizeof(struct sockaddr_in)
  ); 

  // FIXME: remove!!
  fprintf(stderr, "sendto() returned %ld\n", retval);
}

// FIXME: factor this a bit along with send_ack???
void server_send_reject(server* serv, packet_info const* bad_pi,
  struct sockaddr_in const* ret, reject_code code) {
  packet_info pi;
  pi.type = REJECT;
  pi.id = bad_pi->id;
  pi.cont.reject_info.code = code;
  pi.cont.reject_info.recvd_seq_num = bad_pi->cont.data_info.seq_num;


  size_t flattened_len = flatten(&pi, serv->send_buf, sizeof(serv->send_buf));
  // FIXME: check number of sent bytes???
  sendto(
    serv->sock_fd,
    serv->send_buf,
    flattened_len,
    0,
    (struct sockaddr*)ret,
    sizeof(struct sockaddr_in)
  );
}



// TODO
void server_run(server* serv) {
  struct sockaddr_in client_addr; // To store client IP address
  socklen_t addrlen = sizeof(struct sockaddr_in); // For length of client address
  ssize_t n_recvd; // To hold number of bytes received


  memset(&client_addr, 0, sizeof(client_addr));

  // FIXME: remove!
  fprintf(stderr, "addrlen = %u\n", addrlen);

  // Wait...
  fprintf(stderr, "server_run: Waiting for messages...\n");
  while ((n_recvd =
    recvfrom(serv->sock_fd, serv->recv_buf, sizeof(serv->recv_buf), 0,
      (struct sockaddr*)&client_addr, &addrlen))) // FIXME: this implicitly exits upon receiving a 0-byte packet!!
  {
    if (n_recvd == -1) {
      perror("server_run: recvfrom() failed");
      continue;
    }


    fprintf(stderr, "server_run: Got a packet: %ld bytes!\n", n_recvd);

    assert(addrlen == sizeof(struct sockaddr_in)); // FIXME: remove; debug
    assert(client_addr.sin_family == AF_INET);


    // Process and reply
    serv->last_recvd_len = n_recvd;
    server_process_packet(serv, &client_addr);
  }
}


int server_check_packet(server* serv, packet_info* pi) {
  // Read and check the packet for errors
  int code = interpret_packet(serv->recv_buf, pi, sizeof(serv->recv_buf));


  // Check for invalid length field: does the raw packet end on the expected
  // boundary?
  uint8_t* payload_end =
    serv->recv_buf
      + sizeof(PACKET_START)
      + sizeof(client_id)
      + sizeof(uint16_t) // XXX: packet type
      + sizeof(sequence_num)
      + sizeof(payload_len)
      + pi->cont.data_info.len;
  uint8_t* expected_payload_end =
    serv->recv_buf + serv->last_recvd_len - sizeof(PACKET_END);

  // FIXME: remove!!
  fprintf(stderr, "last_recvd_len = %lu\n", serv->last_recvd_len);
  fprintf(stderr, "payload_end, expected_payload_end = %p, %p\n",
    payload_end, expected_payload_end);
  
  if (payload_end != expected_payload_end) {
    return BAD_LEN;
  }


  // Check for additional errors
  if (code == 0) {
    // Check sequence number
    // FIXME: special case: both zero -> overflow!...or will it wrap around??
    if (pi->cont.data_info.seq_num != serv->expect_recv[pi->id]) {
      if (pi->cont.data_info.seq_num > serv->expect_recv[pi->id]) {
        return OUT_OF_SEQ;
      } else {
        return DUP_PACK;
      }
    }
  }


  return code; 
}
