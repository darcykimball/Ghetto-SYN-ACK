#ifndef CLIENT_COMMANDS_H
#define CLIENT_COMMANDS_H


#include "shell.h"
#include "client.h"


//
// Client shell commands
//


// Send a string to some destination
// Usage: send_string [dest_ip] [port]
void send_string(size_t argc, char** argv);


// Dump the current client configuration
// Usage: dump_config
void dump_config(size_t, char**);


#endif // CLIENT_COMMANDS_H
