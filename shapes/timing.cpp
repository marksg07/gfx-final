#include "timing.h"
#include <sys/time.h>
#include <stdlib.h>
double get_time() {
  struct timeval t;
  gettimeofday(&t, NULL);
  return (t.tv_usec * 0.000001 + t.tv_sec);
}
