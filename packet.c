#include <stdbool.h>
#include <stdlib.h>
#include <assert.h>

#include "packet.h"
#include "raw_iterator.h"

//
// Constants
//


const unsigned MAX_IOVEC_LEN = 8;
const uint16_t PACKET_START = 0xFFFF;
const uint16_t PACKET_END = 0xFFFF;


//
// Functions
//


bool is_well_formed(void* flat_packet);


// TODO
size_t flatten(packet_info const* pi, void* buf) {
}


// TODO
bool interpret_packet(void const* buf, packet_info* pi) {
}

