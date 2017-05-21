#include <stdio.h>
#include <stdint.h>
#include <stddef.h>
#include <string.h>

#include "raw_iterator.h"


const char hello[] = "Hellooo";
const char there[] = "there";
const char world[] = "world!!!!!";


int main() {
  raw_iterator buf_rit;
  raw_iterator out_rit;
  uint8_t buf[BUFSIZ];
  char out[BUFSIZ];
  
  rit_init(&buf_rit, buf, sizeof(buf));
  rit_init(&out_rit, out, sizeof(out));


  rit_write(&buf_rit, sizeof(hello), hello);
  rit_write(&buf_rit, sizeof(there), there);
  rit_write(&buf_rit, sizeof(world), world);

  printf(buf);
  printf(buf + sizeof(hello));
  printf(buf + sizeof(there));

  rit_reset(&buf_rit);

  rit_read(&buf_rit, sizeof(hello), out);
  printf("out = %s\n", out);
    
  rit_read(&buf_rit, sizeof(there), out);
  printf("out = %s\n", out);

  rit_read(&buf_rit, sizeof(world), out);
  printf("out = %s\n", out);


  return 0;
}
