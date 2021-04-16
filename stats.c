#include "headers.h"
#include "utils.h"
#include "slab.h"

#define MAX_STATS 10000000LU

struct stats {
   uint64_t *timing_time;
   uint64_t *timing_value;
   size_t timing_idx;
   size_t max_timing_idx;
} stats;

void add_timing_stat(uint64_t elapsed) {
   if(!stats.timing_value) {
      stats.timing_time = malloc(MAX_STATS * sizeof(*stats.timing_time));
      stats.timing_value = malloc(MAX_STATS * sizeof(*stats.timing_value));
      stats.timing_idx = 0;
      stats.max_timing_idx = MAX_STATS;
   }
   if(stats.timing_idx >= stats.max_timing_idx)
      stats.timing_idx = 0; // wrap around

   rdtscll(stats.timing_time[stats.timing_idx]);
   stats.timing_value[stats.timing_idx] = elapsed;
   stats.timing_idx++;
}

int cmp_uint(const void *_a, const void *_b) {
   uint64_t a = *(uint64_t*)_a;
   uint64_t b = *(uint64_t*)_b;
   if(a > b)
      return 1;
   else if(a < b)
      return -1;
   else
      return 0;
}

void print_stats(void) {
   uint64_t avg = 0;

   if(stats.timing_idx == 0) {
      printf("#No stat has been collected\n");
      return;
   }

   size_t last = stats.timing_idx;
   qsort(stats.timing_value, last, sizeof(*stats.timing_value), cmp_uint);
   for(size_t i = 0; i < last; i++)
      avg += stats.timing_value[i];
   for(size_t i = 0; i < 1000; i++) {
      printf("CDF p1000 %4lu us %lu\n", i, cycles_to_us(stats.timing_value[last * i / 1000]));
   }

   printf("#Latency: avg %lu 95 %lu 99 %lu 99.5 %lu 99.9 %lu max %lu\n", cycles_to_us(avg/last),
      cycles_to_us(stats.timing_value[last*95/100]),
      cycles_to_us(stats.timing_value[last*99/100]),
      cycles_to_us(stats.timing_value[last*995/1000]),
      cycles_to_us(stats.timing_value[last*999/1000]),
      cycles_to_us(stats.timing_value[last-1]));

   stats.timing_idx = 0;
}



struct timing_s {
   size_t origin;
   size_t time;
};

void *allocate_payload(void) {
#if DEBUG
   return calloc(20, sizeof(struct timing_s));
#else
   return NULL;
#endif
}

void add_time_in_payload(struct slab_callback *c, size_t origin) {
#if DEBUG
   struct timing_s *payload = c->payload;
   if(!payload)
      return;

   uint64_t t, pos = 0;
   t = time_nsec();
   while(pos < 20 && payload[pos].time)
      pos++;
   if(pos == 20)
      die("Too many times added!\n");
   payload[pos].time = t;
   payload[pos].origin = origin;
#else
   if(origin != 0)
      return;
   uint64_t t;
   t = time_nsec();
   c->payload = (void*)t;
#endif
}

uint64_t get_origin_from_payload(struct slab_callback *c, size_t pos) {
#if DEBUG
   struct timing_s *payload = c->payload;
   if(!payload)
      return 0;
   return payload[pos].origin;
#else
   return 0;
#endif
}

uint64_t get_time_from_payload(struct slab_callback *c, size_t pos) {
#if DEBUG
   struct timing_s *payload = c->payload;
   if(!payload)
      return 0;
   return payload[pos].time;
#else
   return (uint64_t)c->payload;
#endif
}

void free_payload(struct slab_callback *c) {
#if DEBUG
   free(c->payload);
#endif
}
