#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "packet.h"
#include "test.h"


const char str_data[] = "Hello, this is some arbitrary data!";
const client_id id = 0xAB;


int main() {
  uint8_t buf[BUFSIZ];


  // Make some packets and check serialization functions

 
  // A data packet
  packet_info pi_orig;
  pi_orig.type = DATA;
  pi_orig.id = id;
  pi_orig.cont.data_info.seq_num = 0xCD;
  pi_orig.cont.data_info.len = sizeof(str_data);
  pi_orig.cont.data_info.payload = str_data;

  size_t flattened_size = 2 + 1 + 2 + 1 + 1 + sizeof(str_data) + 2;

  // Is the size as expected?
  TEST(flatten(&pi_orig, buf, sizeof(buf)) == flattened_size);

  // Does interpreting the raw packet yield the original?
  packet_info pi_interp;
  interpret_packet(buf, &pi_interp, sizeof(buf));

  TEST(pi_interp.type == pi_orig.type);
  TEST(pi_interp.id == pi_orig.id);
  TEST(pi_interp.cont.data_info.len == pi_orig.cont.data_info.len);
  TEST(pi_interp.cont.data_info.seq_num == pi_orig.cont.data_info.seq_num);
  TEST(
    memcmp(pi_interp.cont.data_info.payload,
           pi_orig.cont.data_info.payload, sizeof(str_data)) == 0
  );
 


  return 0;
}

