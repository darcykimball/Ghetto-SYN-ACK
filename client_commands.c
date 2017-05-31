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


// Command map
const command_pair commands[] = {
  { "dump_config", &dump_config },
  { "send_string", &send_string },
  { "send_bs", &send_bs },
};


// Helper for parsing IP/port/sequence number
static bool parse_ip_args(char** argv, struct sockaddr_in* dest_addr, sequence_num* seq_num) {
  char* end = NULL; // For parsing port number
  struct addrinfo* aip; // For parsing address/port
  int gai_retval; // For return value of getaddressinfo()


  // XXX: not safe if IPv6...
  if ((gai_retval = getaddrinfo(argv[1], argv[2], NULL, &aip))) {
    if (gai_retval == EAI_SYSTEM) {
      perror("parse_ip_args");
    }

    fprintf(stderr, "parse_ip_args: %s\n", gai_strerror(gai_retval));
    return false;
  }

  assert(aip->ai_addrlen == sizeof(struct sockaddr_in));

  *dest_addr = *(struct sockaddr_in*)(aip->ai_addr);
  freeaddrinfo(aip);


  // Validate seq_num param
  *seq_num = strtoul(argv[3], &end, 10);
  if (end == argv[3] || *end != '\0') {
    SHELL_ERROR("parse_ip_args: Invalid sequence number!");
    return false;
  }


  return true;
}


void send_string(size_t argc, char** argv) {
  struct sockaddr_in dest_addr; // To hold the destination address
  packet_info pi; // For building the packet to send
  sequence_num seq_num; // User-supplied sequence number; usually not needed
                        // but in this allows explicit out-of-sequence sends
  char ip_str[INET_ADDRSTRLEN]; // For message printing


  // Check for and validate arguments
  if (argc != 5) {
    SHELL_ERROR("Usage: send_string [dest_ip] [port] [seq_num] [string]");
    return;
  }

  if (!parse_ip_args(argv, &dest_addr, &seq_num)) {
    return;
  }
  
   
  // Construct the packet
  pi.type = DATA;
  pi.id = the_client.id;
  pi.cont.data_info.seq_num = seq_num;
  pi.cont.data_info.len = strlen(argv[4]) + 1; // XXX: Send the null byte too
  pi.cont.data_info.payload = argv[4];

  
  // Send
  fprintf(stderr, "Sending to IP %s, port %u\n",
    inet_ntop(AF_INET, &dest_addr.sin_addr, ip_str, INET_ADDRSTRLEN),
    ntohs(dest_addr.sin_port));

  client_send_packet(&the_client, &pi, &dest_addr);
}


void dump_config(size_t argc, char** argv) {
  // Unused params
  (void)argc;
  (void)argv;


  char ip_str[INET_ADDRSTRLEN];

  fprintf(stderr,
    "Client IP: %s\nClient Port: %u\nClient ID: %u Client Socket: %d\n",
      inet_ntop(AF_INET, &the_client.addr.sin_addr.s_addr, ip_str,
        INET_ADDRSTRLEN),
      ntohs(the_client.addr.sin_port),
      the_client.id,
      the_client.sock_fd);

}


void send_bs(size_t argc, char** argv) {
  struct sockaddr_in dest_addr; // To hold the destination address
  packet_info pi; // For making the packet to send
  sequence_num seq_num; // User-supplied sequence number


  if (argc != 5) {
    SHELL_ERROR("Usage: send_bs [dest_ip] [port] [seq_num] [l|e]\n\t"
      "l - Bad length\n\te - Missing end marker\n");
    return;
  }
  
  parse_ip_args(argv, &dest_addr, &seq_num);

  
  // Make the packet 
  pi.id = the_client.id;
  pi.type = DATA;
  size_t raw_len;

  switch (*argv[4]) {
    case 'l':
      pi.cont.data_info.len = 3; // XXX: 10 is not the length of the payload
      pi.cont.data_info.payload = "bs";
      pi.cont.data_info.seq_num = seq_num;

      raw_len =
        flatten(&pi, the_client.send_buf, sizeof(the_client.send_buf));

      // Mess up the length field
      the_client.send_buf[
          sizeof(PACKET_START) +
          sizeof(client_id) +
          sizeof(uint16_t) +
          sizeof(sequence_num)
        ] = 1;

      break;

    case 'e':
      pi.cont.data_info.len = strlen("hello") + 1;
      pi.cont.data_info.payload = "hello";
      pi.cont.data_info.seq_num = seq_num;

      raw_len =
        flatten(&pi, the_client.send_buf, sizeof(the_client.send_buf));
    
      // Mess up the end flag
      the_client.send_buf[raw_len - 1] = 0xAA;

      break;
    default:
      SHELL_ERROR("send_bs: Invalid argument!\n"); 
      return;
  }

  
  // Send out the bs
  sendto(the_client.sock_fd, the_client.send_buf, raw_len, 0,
    (struct sockaddr*)&dest_addr, sizeof(dest_addr));


  // Expecting a reject packet...
  client_recv_packet(&the_client);
}
