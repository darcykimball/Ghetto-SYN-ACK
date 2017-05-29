#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <arpa/inet.h>
#include <netinet/ip.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>


#include "client_commands.h"
#include "packet.h"
#include "shell.h"
#include "client.h"


#define SHELL_ERROR(errmsg) fprintf(stderr, "%s\n", (errmsg));


const command_pair commands[] = {
  { "dump_config", &dump_config },
  { "send_string", &send_string },
};


//
// Helper fns
//

// TODO: pretty printing client info!

// Send a string to some destination
// Usage: send_string [dest_ip] [port] [string]
void send_string(size_t argc, char** argv) {
  struct sockaddr_in dest_addr; // To hold the destination address
  char* end = NULL; // For parsing port number
  packet_info pi; // For building the packet to send
  sequence_num seq_num; // User-supplied sequence number; usually not needed
                        // but in this allows explicit out-of-sequence sends
  struct addrinfo* aip; // For parsing address/port
  int gai_retval; // For return value of getaddressinfo()
  char ip_str[INET_ADDRSTRLEN]; // For message printing


  // Check for and validate arguments
  if (argc != 5) {
    SHELL_ERROR("Usage: send_string [dest_ip] [port] [seq_num] [string]");
    return;
  }


  // FIXME: not safe if IPv6...
  // TODO: debug/print!!
  if ((gai_retval = getaddrinfo(argv[1], argv[2], NULL, &aip))) {
    if (gai_retval == EAI_SYSTEM) {
      perror("send_string");
    }

    fprintf(stderr, "send_string: %s\n", gai_strerror(gai_retval));
    return;
  }

  assert(aip->ai_addrlen == sizeof(struct sockaddr_in));

  dest_addr = *(struct sockaddr_in*)(aip->ai_addr);
  freeaddrinfo(aip);

  //inet_pton(AF_INET, argv[1], &dest_addr.sin_addr);

  //dest_addr.sin_family = AF_INET;
  //dest_addr.sin_port = htons(strtoul(argv[2], &end, 10)); // FIXME dangerous??
  //if (end == argv[2] || *end != '\0') {
  //  SHELL_ERROR("send_string: Invalid port number!");
  //  return;
  //}


  seq_num = strtoul(argv[3], &end, 10);
  if (end == argv[3] || *end != '\0') {
    SHELL_ERROR("send_string: Invalid sequence number!");
    return;
  }
  
   
  // Construct the packet
  pi.type = DATA;
  pi.id = id;
  pi.cont.data_info.seq_num = seq_num;
  pi.cont.data_info.len = strlen(argv[4]) + 1; // XXX: Send the null byte too
  pi.cont.data_info.payload = argv[4];

  
  // Send
  fprintf(stderr, "Sending to IP %s, port %x\n",
    inet_ntop(AF_INET, &dest_addr.sin_addr, ip_str, INET_ADDRSTRLEN),
    ntohs(dest_addr.sin_port));

  client_send_packet(&the_client, &pi, &dest_addr);
}


void dump_config(size_t argc, char** argv) {
  // Unused params
  (void)argc;
  (void)argv;

  // TODO!!
}


//
// Helper fns
//

// TODO
