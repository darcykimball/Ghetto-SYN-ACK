#include <stdio.h>
#include <sys/time.h>

#include "busywait.h"


bool foo(void* bar, void* baz) {
  (void)bar;
  (void)baz;
  return false;
}


int main() {
  struct timeval timeout = { 3, 0 };


  busy_wait_until(timeout, &foo, NULL, NULL);


  return 0;
}
