#include "headers.h"
#include "utils.h"

uint64_t cycles_to_us(uint64_t cycles) {
   return cycles / 1000lu;
}

void shuffle(size_t *array, size_t n) {
   if (n > 1) {
      size_t i;
      for (i = 0; i < n - 1; i++) {
         size_t j = i + rand() / (RAND_MAX / (n - i) + 1);
         size_t t = array[j];
         array[j] = array[i];
         array[i] = t;
      }
   }
}

void pin_me_on(int core) {
   if(!PINNING)
      return;

   cpu_set_t cpuset;
   pthread_t thread = pthread_self();

   CPU_ZERO(&cpuset);
   CPU_SET(core, &cpuset);

   int s = pthread_setaffinity_np(thread, sizeof(cpu_set_t), &cpuset);
   if (s != 0)
      die("Cannot pin thread on core %d\n", core);

}

  uint64_t
time_nsec(void)
{
  struct timespec ts;
  // MONO_RAW is 5x to 10x slower than MONO
  clock_gettime(CLOCK_MONOTONIC, &ts);
  return ((uint64_t)ts.tv_sec) * 1000000000lu + ((uint64_t)ts.tv_nsec);
}
