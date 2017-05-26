#include <stdio.h>
#include <sys/time.h>
#include <time.h>

#include "busywait.h"


bool busy_wait_until(struct timeval timeout, try_fn fn, void* args, void* retval) {
  struct timeval curr_time; // For keeping track of timeout
  struct timeval start_time;
  struct timeval diff; // For calculating elapsed time

  gettimeofday(&start_time, NULL);

  while (!fn(args, retval)) {
    // Calculate elapsed time
    gettimeofday(&curr_time, NULL);
    timersub(&curr_time, &start_time, &diff);
    if (timercmp(&diff, &timeout, >)) {
      // Operation timed out
      return false;
    }
  }


  return true;
}


