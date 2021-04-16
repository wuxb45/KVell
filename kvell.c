#include "headers.h"
#include "kvell.h"

size_t PAGE_CACHE_SIZE = 1lu << 30; // 1GB by default
size_t QUEUE_DEPTH = 64;

static void kvell_cb(struct slab_callback *cb, void *item) {
  struct item_metadata metacpy;
  if (item)
    memcpy((void *)&metacpy, item, sizeof(metacpy));
  if (cb->func) {
    cb->func(item, cb->arg1, cb->arg2);
  }

  free(cb->item);
  free(cb);
}

struct slab_callback *kvell_create_cb(void (*func)(void * item, uint64_t arg1, uint64_t arg2), uint64_t arg1, uint64_t arg2) {
  struct slab_callback *cb = calloc(1, sizeof(*cb));
  cb->cb = kvell_cb;
  cb->func = func;
  cb->arg1 = arg1;
  cb->arg2 = arg2;
  return cb;
}

  void
kvell_init(const char * prefix, const char * nd, const char * wpd, const char * cgb, const char * qd)
{
  kvell_prefix = strdup(prefix);
  int nb_disks = atoi(nd);
  int nb_workers_per_disk = atoi(wpd);
  PAGE_CACHE_SIZE = (1lu << 30) * atoi(cgb);
  QUEUE_DEPTH = atoi(qd);
  slab_workers_init(nb_disks, nb_workers_per_disk);
}

  static char *
kvell_create_item(const void * key, size_t klen, const uint64_t hash, const void * value, size_t vlen)
{
  size_t item_size = klen + vlen + sizeof(struct item_metadata);
  char *item = calloc(1, item_size);
  struct item_metadata *meta = (struct item_metadata *)item;

  meta->hash = hash;
  meta->key_size = klen;
  meta->value_size = vlen;
  char *item_key = &item[sizeof(*meta)];
  memcpy(item_key, key, klen);
  char *item_value = &item[sizeof(*meta) + klen];
  memcpy(item_value, value, vlen);
  return item;
}

  void
kvell_get_submit(const void * key, size_t klen, const uint64_t hash, void (*func)(void * item, uint64_t arg1, uint64_t arg2), uint64_t arg1, uint64_t arg2)
{
  struct slab_callback *cb = kvell_create_cb(func, arg1, arg2);
  cb->item = kvell_create_item(key, klen, hash, NULL, 0);
  kv_read_async(cb);
  // queued but the caller never know when does the request get processed
}

  void
kvell_set_submit(const void * key, size_t klen, const uint64_t hash, const void * value, size_t vlen,
    void (*func)(void * item, uint64_t arg1, uint64_t arg2), uint64_t arg1, uint64_t arg2)
{
  struct slab_callback *cb = kvell_create_cb(func, arg1, arg2);
  cb->item = kvell_create_item(key, klen, hash, value, vlen);
  kv_add_or_update_async(cb); // set
}

  void
kvell_del_submit(const void * key, size_t klen, const uint64_t hash, void (*func)(void * item, uint64_t arg1, uint64_t arg2), uint64_t arg1, uint64_t arg2)
{
  struct slab_callback *cb = kvell_create_cb(func, arg1, arg2);
  cb->item = kvell_create_item(key, klen, hash, NULL, 0);
  kv_remove_async(cb);
  // queued but the caller never know when does the request get processed
}


  void
kvell_scan50(const void * key, size_t klen, const uint64_t hash, void (*func)(void * item, uint64_t arg1, uint64_t arg2), uint64_t arg1, uint64_t arg2)
{
  char *item = kvell_create_item(key, klen, hash, NULL, 0);
  tree_scan_res_t scan_res = kv_init_scan(item, 50); // has to hardcode it
  free(item);
  for (size_t j = 0; j < scan_res.nb_entries; j++) {
    struct slab_callback *cb = kvell_create_cb(func, arg1, arg2);
    kv_read_async_no_lookup(cb, scan_res.entries[j].slab, scan_res.entries[j].slab_idx);
  }
  free(scan_res.hashes);
  free(scan_res.entries);
}
