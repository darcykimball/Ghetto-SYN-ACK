#include <stdio.h>
#include <stddef.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>


#include "shell.h"
#include "server.h"


int main() {
  server serv; // The uniue server instance


  // Setup the server
  struct sockaddr_in serv_addr;
  memset(&serv_addr, 0, sizeof(struct sockaddr_in));
  serv_addr.sin_family = AF_INET;
  serv_addr.sin_port = htons(DEFAULT_PORT);
  serv_addr.sin_addr.s_addr = INADDR_ANY;
  
  if (!server_init(&serv, &serv_addr)) {
    fprintf(stderr, "Failed to initialize server! Exiting...\n");
    exit(1);
  }


  // Run...
  server_run(&serv);


  return EXIT_SUCCESS;
}
