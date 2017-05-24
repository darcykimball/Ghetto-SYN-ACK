#include <stdio.h>
#include <stdint.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/ip.h>
#include <string.h>
#include <assert.h>

#include "server.h"
#include "packet.h"


// Helper fns
static void alert_reject(packet_info* pi, reject_code code);


bool server_init(server* serv, struct sockaddr_in* addr) {
  fprintf(stderr, "Initializing server...\n");


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
  }


  // Initialize next expected sequence numbers
  memset(serv->last_recvd, 0, sizeof(serv->last_recvd));



  fprintf(stderr, "Done initializing!\n");
  return true;
}


// Process a received packet
void server_process_packet(server* serv, struct sockaddr_in const* ret) {
  packet_info pi; // For storing interpreted packet
  int code; // For return value of interpret_packet()


  // Check the packet for errors
  // FIXME: need both interpret_packet and server_check_packet (TBD) to
  // coordinate reject_code!!
  code = interpret_packet(serv->recv_buf, &pi, sizeof(serv->recv_buf));


  // Check for additional errors
  if (code == 0) {
    // Check sequence number
    if (pi.cont.data_info.seq_num == serv->last_recvd[pi.id]) {
      code = DUP_PACK;
    } else if (pi.cont.data_info.seq_num != (serv->last_recvd[pi.id] + 1)) {
      code = OUT_OF_SEQ;
    }
  }


  // Notify of any errors if present
  if (code != 0) {
    alert_reject(&pi, (reject_code)code);
    //
    // Reply to client with reject message
    server_send_reject(serv, pi.id, ret, (reject_code)code);
  } else { // All is well
    // Send an ACK
    server_send_ack(serv, pi.id, ret);
  }
}


// TODO
void server_send_ack(server* serv, client_id id, struct sockaddr_in const* ret) {
  packet_info pi;
  pi.type = ACK;
  pi.id = id;
  // FIXME: check!!! for off by 1???
  pi.cont.ack_info.recvd_seq_num = serv->last_recvd[id];


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


// FIXME: factor this a bit along with send_ack???
void server_send_reject(server* serv, client_id id, struct sockaddr_in const* ret, reject_code code) {
  packet_info pi;
  pi.type = REJECT;
  pi.id = id;
  pi.cont.reject_info.code = code;
  pi.cont.reject_info.recvd_seq_num = serv->last_recvd[id];


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
  socklen_t addrlen; // For storing length of client address

  // Wait...
  while (true) {
    // Get a message
    recvfrom(serv->sock_fd, serv->recv_buf, sizeof(serv->recv_buf), 0,
      (struct sockaddr*)&client_addr, &addrlen);

    assert(addrlen == sizeof(struct sockaddr_in));


    // Process and reply
    server_process_packet(serv, &client_addr);
  }
}


static void alert_reject(packet_info* pi, reject_code code) {
  // Print out errors server-side
  fprintf(stderr, "server_check_packet: From client %d:\n", pi->id);
  switch (code) {
    case NO_END:
      fprintf(stderr, "server_process_packet: No packet terminator\n");
      break;
    case DUP_PACK:
      fprintf(stderr, "server_process_packet: Received duplicate\n"); 
      break;
    case OUT_OF_SEQ:
      fprintf(stderr, "server_process_packet: Received out of sequence!\n");
      break;
    case BAD_TYPE:
      fprintf(stderr, "server_process_packet: Bad type field!\n");
      break;
    case BAD_LEN:
      fprintf(stderr, "server_process_packet: Bad length field!\n");
      break;
    default:
      fprintf(stderr, "server_process_packet: Unrecognized reject code...\n");
  }
}
