#include "headers.h"
size_t PAGE_CACHE_SIZE = 1lu << 30; // 1GB by default
size_t QUEUE_DEPTH = 64;

int main(int argc, char **argv) {
   int nb_disks, nb_workers_per_disk;
   declare_timer;

   /* Parsing of the options */
   if(argc < 9)
      die("Usage: ./main <path prefix> <nb disks> <nb workers per disk> <page cache size in GB> <queue-depth> <key size> <value size> <nr items>\n");
   kvell_prefix = argv[1];
   nb_disks = atoi(argv[2]);
   nb_workers_per_disk = atoi(argv[3]);
   PAGE_CACHE_SIZE = (1lu << 30) * atoi(argv[4]);
   QUEUE_DEPTH = atoi(argv[5]);
   wl_key_size = atoi(argv[6]);
   wl_value_size = atoi(argv[7]);
   int nr_items = atoi(argv[8]);

   /* Definition of the workload, if changed you need to erase the DB before relaunching */
   struct workload w = {
      .api = &YCSB,
      .nb_items_in_db = nr_items,
      .nb_load_injectors = 4,
      //.nb_load_injectors = 12, // For scans (see scripts/run-aws.sh and OVERVIEW.md)
   };



   /* Pretty printing useful info */
   printf("# Configuration:\n");
   printf("# \tPage cache size: %lu GB\n", PAGE_CACHE_SIZE/1024/1024/1024);
   printf("# \tWorkers: %d working on %d disks\n", nb_disks*nb_workers_per_disk, nb_disks);
   printf("# \tIO configuration: %lu queue depth (capped: %s, extra waiting: %s)\n", QUEUE_DEPTH, NEVER_EXCEED_QUEUE_DEPTH?"yes":"no", WAIT_A_BIT_FOR_MORE_IOS?"yes":"no");
   printf("# \tQueue configuration: %lu maximum pending callbaks per worker\n", MAX_NB_PENDING_CALLBACKS_PER_WORKER);
   printf("# \tDatastructures: %d (memory index) %d (pagecache)\n", MEMORY_INDEX, PAGECACHE_INDEX);
   printf("# \tThread pinning: %s\n", PINNING?"yes":"no");
   printf("# \tBench: %s (%lu elements)\n", w.api->api_name(), w.nb_items_in_db);

   /* Initialization of random library */
   start_timer {
      printf("Initializing random number generator (Zipf) -- this might take a while for large databases...\n");
      init_zipf_generator(0, w.nb_items_in_db - 1); /* This takes about 3s... not sure why, but this is legacy code :/ */
   } stop_timer("Initializing random number generator (Zipf)");

   /* Recover database */
   start_timer {
      slab_workers_init(nb_disks, nb_workers_per_disk);
   } stop_timer("Init found %lu elements", get_database_size());

   /* Add missing items if any */
   repopulate_db(&w);

   /* Launch benchs */
   bench_t workload, workloads[] = {
      ycsb_a_zipfian, ycsb_b_zipfian, ycsb_c_zipfian,
      ycsb_d_zipfian, ycsb_e_zipfian, ycsb_f_zipfian,
   };
   foreach(workload, workloads) {
      if (workload == ycsb_e_uniform || workload == ycsb_e_zipfian) {
         w.nb_requests = nr_items / 50; // requests for YCSB E are longer (scans) so we do less
      } else {
         w.nb_requests = nr_items;
      }
      run_workload(&w, workload);
   }
   return 0;
}
