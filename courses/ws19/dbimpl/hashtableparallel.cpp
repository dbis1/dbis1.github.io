#include <atomic>
#include <vector>
#include <cassert>
#include <cstdint>
#include <initializer_list>
#include <sys/mman.h>
#include <tbb/tbb.h>
#include "/opt/PerfEvent.hpp"

uint64_t hashKey(uint64_t k) {
   //MurmurHash64A
   const uint64_t m = 0xc6a4a7935bd1e995;
   const int r = 47;
   uint64_t h = 0x8445d61a4e774912 ^ (8*m);
   k *= m;
   k ^= k >> r;
   k *= m;
   h ^= k;
   h *= m;
   h ^= h >> r;
   h *= m;
   h ^= h >> r;
   return h;
}

void* allocZeros(uint64_t size) {
   return mmap(nullptr, size, PROT_READ|PROT_WRITE, MAP_PRIVATE|MAP_ANONYMOUS, -1, 0);
}

struct Hashtable {
   struct Entry {
      uint64_t key;
      uint64_t value;
      Entry* next;
   };

   uint64_t htSize;
   uint64_t mask;
   std::atomic<Entry*>* ht;

   Hashtable() {}

   void init(uint64_t size) {
      htSize = 1ull << ((8*sizeof(uint64_t)) - __builtin_clzl(size)); // next power of two
      mask = htSize - 1;
      ht = reinterpret_cast<std::atomic<Entry*>*>(allocZeros(sizeof(uint64_t) * htSize));
   }

   ~Hashtable() {
      munmap(ht, sizeof(uint64_t) * htSize);
   }

   Entry* lookup(uint64_t key) {
      for (Entry* e=ht[hashKey(key) & mask]; e; e=e->next)
         if (e->key==key)
            return e;
      return nullptr;
   }

   // insert with compare-and-swap
   bool insert(Entry* e) {
      uint64_t pos = hashKey(e->key) & mask;

      Entry* expected;
      do {
         expected = ht[pos];
         e->next = expected;
      } while (!ht[pos].compare_exchange_weak(expected, e));

      return true;
   }

   // insert with atomic exchange (faster)
   bool insertFast(Entry* e) {
      uint64_t pos = hashKey(e->key) & mask;

      Entry* next = ht[pos].exchange(e);
      e->next = next;

      return true;
   }

};

int main() {
   tbb::task_scheduler_init init(getenv("THREADS")?atoi(getenv("THREADS")):1);
   uint64_t n = 100e6;

   PerfEvent e;

   Hashtable h;
   tbb::enumerable_thread_specific<std::vector<Hashtable::Entry>> entries;

   {
      {
         e.setParam("op", "mat");
         PerfEventBlock b(e, n);

         // Phase 1: materialize
         tbb::parallel_for(tbb::blocked_range<uint64_t>(0, n),
                           [&](const tbb::blocked_range<uint64_t>& range) {
                              std::vector<Hashtable::Entry>& v = entries.local();
                              for (uint64_t i=range.begin(); i<range.end(); i++) {
                                 v.push_back({i, i, nullptr});
                              }
                           });
         // Phase 1.1: set HT size
         uint64_t count = 0;
         for (auto& v : entries)
            count += v.size();
         h.init(count);
         assert(count==n);
      }
      {
         e.setParam("op", "insert");
         PerfEventBlock b(e, n);
         // Phase 2: insert
         tbb::parallel_for(tbb::blocked_range<uint64_t>(0, entries.size(), 1),
                           [&](const tbb::blocked_range<uint64_t>& range) {
                              assert(range.end()-range.begin() == 1);
                              std::vector<Hashtable::Entry>& v = entries.local();
                              for (Hashtable::Entry& e : v) {
                                 h.insert(&e);
                              }
                           });
      }
   }

   {
      unsigned repeat = 1;
      e.setParam("op", "lookup");
      PerfEventBlock b(e, n*repeat);

      for (uint64_t j=0; j<repeat; j++) {

         std::atomic<uint64_t> count(0);

         tbb::parallel_for(tbb::blocked_range<uint64_t>(0, n),
                           [&](const tbb::blocked_range<uint64_t>& range) {
                              uint64_t localCount = 0;
                              for (uint64_t i=range.begin(); i<range.end(); i++) {
                                 Hashtable::Entry* e = h.lookup(i);
                                 assert(e);
                                 assert(e->value==i);
                                 localCount++;
                              }
                              count += localCount;
                           });

        assert(count==n);
      }

   }

   return 0;
}
