#include <stdio.h>
#include <stddef.h>


#include "shell.h"
#include "client.h"
#include "client_commands.h"


client the_client; // The single, global client instance
client_id id; // The client ID for this session TODO


int main(int argc, char** argv) {
  // Setup the client (configuration)
  // TODO

  // Initialize ID
  id = 11;

  // Start the shell
  loop(commands, N_COMMANDS);
    

  return EXIT_SUCCESS;
}
