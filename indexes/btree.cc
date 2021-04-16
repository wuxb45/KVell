#include "cpp-btree/btree_map.h"
#include "btree.h"

using namespace std;
using namespace btree;

extern "C"
{
   btree_t *btree_create() {
      btree_map<string, struct index_entry> *b = new btree_map<string, struct index_entry>();
      return b;
   }

   int btree_find(btree_t *t, unsigned char* k, size_t len, struct index_entry *e) {
      btree_map<string, struct index_entry> *b = static_cast< btree_map<string, struct index_entry> * >(t);
      auto i = b->find(string((char *)k, len));
      if(i != b->end()) {
         *e = i->second;
         return 1;
      } else {
         return 0;
      }
   }

   void btree_delete(btree_t *t, unsigned char*k, size_t len) {
      btree_map<string, struct index_entry> *b = static_cast< btree_map<string, struct index_entry> * >(t);
      b->erase(string((char *)k, len));
   }

   void btree_insert(btree_t *t, unsigned char*k, size_t len, struct index_entry *e) {
      btree_map<string, struct index_entry> *b = static_cast< btree_map<string, struct index_entry> * >(t);
      b->insert(make_pair(string((char *)k, len), *e));
   }

   struct index_scan btree_find_n(btree_t *t, unsigned char* k, size_t len, size_t n) {
      struct index_scan res;
      res.hashes = (uint64_t*) malloc(n*sizeof(*res.hashes));
      res.entries = (struct index_entry*) malloc(n*sizeof(*res.entries));
      res.nb_entries = 0;

      btree_map<string, struct index_entry> *b = static_cast< btree_map<string, struct index_entry> * >(t);
      auto i = b->find_closest(string((char *)k, len));
      while(i != b->end() && res.nb_entries < n) {
         res.hashes[res.nb_entries] = *(uint64_t*)(i->first.c_str());
         res.entries[res.nb_entries] = i->second;
         res.nb_entries++;
         i++;
      }

      return res;
   }


   void btree_forall_keys(btree_t *t, void (*cb)(uint64_t h, void *data), void *data) {
      btree_map<string, struct index_entry> *b = static_cast< btree_map<string, struct index_entry> * >(t);
      auto i = b->begin();
      while(i != b->end()) {
         uint64_t hash = *(uint64_t*)(i->first.c_str());
         cb(hash, data);
         i++;
      }
      return;
   }


   void btree_free(btree_t *t) {
      btree_map<string, struct index_entry> *b = static_cast< btree_map<string, struct index_entry> * >(t);
      delete b;
   }
}
