#pragma once

  extern void
kvell_init(const char * prefix, const char * nd, const char * wpd, const char * cgb, const char * qd);

  extern void
kvell_get_submit(const void * key, size_t klen, const uint64_t hash, void (*func)(void * item, uint64_t arg1, uint64_t arg2), uint64_t arg1, uint64_t arg2);

  extern void
kvell_put_submit(const void * key, size_t klen, const uint64_t hash, const void * value, size_t vlen, void (*func)(void * item, uint64_t arg1, uint64_t arg2), uint64_t arg1, uint64_t arg2);

  extern void
kvell_del_submit(const void * key, size_t klen, const uint64_t hash, void (*func)(void * item, uint64_t arg1, uint64_t arg2), uint64_t arg1, uint64_t arg2);

// XXX does 50 scans
  extern void
kvell_scan50(const void * key, size_t klen, const uint64_t hash, void (*func)(void * item, uint64_t arg1, uint64_t arg2), uint64_t arg1, uint64_t arg2);

  extern void
kvell_scan_n(const void * key, size_t klen, const uint64_t hash, void (*func)(void * item, uint64_t arg1, uint64_t arg2), uint64_t arg1, uint64_t arg2, unsigned int n);
