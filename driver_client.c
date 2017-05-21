#include <stdio.h>
#include <stddef.h>


#include "shell.h"
#include "client.h"
#include "client_commands.h"


client the_client; // The single, global client instance
client_id id; // The client ID for this session TODO


int main() {
  // Setup the client (configuration)
  if (!client_init(&the_client, NULL)) {
    fprintf(stderr, "Unable to start client! Exiting...\n");
    exit(1);
  }

  // Initialize ID
  id = 11;

  // Start the shell
  loop(commands, N_COMMANDS);
    

  return EXIT_SUCCESS;
}
