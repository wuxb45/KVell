#pragma once

  extern void
kvell_init(const char * prefix, const char * nd, const char * wpd, const char * cgb, const char * qd);

  extern void
kvell_get_submit(const void * key, size_t klen, const uint64_t hash, void (*func)(void * item, void * opaque), void * opaque);

  extern void
kvell_set_submit(const void * key, size_t klen, const uint64_t hash, const void * value, size_t vlen, void (*func)(void * item, void * opaque), void * opaque);

  extern void
kvell_del_submit(const void * key, size_t klen, const uint64_t hash, void (*func)(void * item, void * opaque), void * opaque);

// XXX does 50 scans
  extern void
kvell_scan50(const void * key, size_t klen, const uint64_t hash, void (*func)(void * item, void * opaque), void * opaque);
