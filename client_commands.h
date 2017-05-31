#ifndef CLIENT_COMMANDS_H
#define CLIENT_COMMANDS_H


#include "shell.h"
#include "client.h"


#define N_COMMANDS 3


// The single client instance
extern client the_client;


// Command table
extern const command_pair commands[];


//
// Client shell commands
//


// Send a string to some destination
// Usage: send_string [dest_ip] [port] [seq_num] [string]
void send_string(size_t argc, char** argv);


// Dump the current client configuration
// Usage: dump_config
void dump_config(size_t, char**);


// Send an invalid packet
void send_bs(size_t argc, char** argv);


#endif // CLIENT_COMMANDS_H
