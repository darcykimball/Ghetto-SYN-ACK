#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <arpa/inet.h>
#include <netinet/ip.h>


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

static void dump_timers(ack_timer* const timers);


// Send a string to some destination
// Usage: send_string [dest_ip] [port] [string]
void send_string(size_t argc, char** argv) {
  struct sockaddr_in dest_addr; // To hold the destination address
  char* end = NULL; // For parsing port number
  packet_info pi; // For building the packet to send
  sequence_num seq_num; // User-supplied sequence number; usually not needed
                        // but in this allows explicit out-of-sequence sends


  // Check for and validate arguments
  if (argc != 3) {
    SHELL_ERROR("Usage: send_string [dest_ip] [port] [string]");
    return;
  }


  // FIXME: not safe if IPv6...
  // TODO: debug/print!!
  inet_pton(AF_INET, argv[0], &dest_addr);

  dest_addr.sin_port = htons(strtoul(argv[1], &end, 10)); // FIXME dangerous??
  if (end == argv[1] || *end != '\0') {
    SHELL_ERROR("send_string: Invalid port number!");
    perror("send_string:");
    return;
  }

  seq_num = strtoul(argv[2], &end, 10);
  if (end == argv[2] || *end != '\0') {
    SHELL_ERROR("send_string: Invalid sequence number!");
    perror("send_string:");
    return;
  }
  
   
  // Construct the packet
  pi.type = DATA;
  pi.id = id;
  pi.cont.data_info.seq_num = seq_num;
  pi.cont.data_info.len = strlen(argv[3]) + 1; // XXX: Send the null byte too
  pi.cont.data_info.payload = argv[3];

  
  // Send
  client_send_packet(&the_client, &pi, &dest_addr);
}


void dump_config(size_t argc, char** argv) {
  // Unused params
  (void)argc;
  (void)argv;

  dump_timers(the_client.timers);
}


//
// Helper fns
//

static void dump_timers(ack_timer* const timers) {
  printf("Timers:\n");
  for (size_t i = 0; i < MAX_SEQ_NUM; ++i) {
    printf("timers[%lu] = (%lu, %u)\n", i,
      timers[i].last_sent,
      timers[i].tries);
  }
}
