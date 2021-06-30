/*
Implement a basic buffer manager that manages buffer frames and controls concurrent access to these frames. The buffer manager should offer the following functionality:

    BufferManager::BufferManager(unsigned pageCount)
    Create a new instance that keeps up to pageCount frames in main memory.

    BufferFrame& BufferManager::fixPage(uint64_t pageId, bool exclusive)
    A method to retrieve frames given a page ID and indicating whether the page will be held exclusively by this thread or not. The method can fail (by throwing an exception) if no free frame is available and no used frame can be freed.

    void BufferManager::unfixPage(BufferFrame& frame, bool isDirty)
    Return a frame to the buffer manager indicating whether it is dirty or not. If dirty, the page manager must write it back to disk. It does not have to write it back immediately, but must not write it back before unfixPage is called.

    void* BufferFrame::getData()
    A buffer frame should offer a method giving access to the buffered page. Except for the buffered page, BufferFrame objects can also store control information (page ID, dirtyness, ...).

    BufferManager::~BufferManager()
    Destructor. Write all dirty frames to disk and free all resources.

Your buffer manager should have the following features:

    High performance: Release locks as early as possible.
    Concurrency: It should be able to handle concurrent method invocations efficiently (e.g. using pthread_rwlock_t). Requests to fixPage should block until the requested access (exclusive or shared) can be fulfilled.
    Buffering: It should use a buffer of size frames to keep pages in memory as long as possible. If no free frames are available, old frames should be reclaimed using some reasonable strategy (e.g. Random, LRU, Second Chance).

Your buffer manager does not need to flush of pages to disk asynchronously or prefetch pages that are likely to be accessed in the near future. Use this test program to validate your implementation.

 */

#include <iostream>
#include <vector>
#include <atomic>
#include <stdlib.h>
#include <cstdint>
#include <cassert>
#include <pthread.h>

using namespace std;

class BufferFrame {
   private:
   // TODO

   public:
   void* getData();
};

class BufferManager {
   private:
   // TODO

   public:
   BufferManager(unsigned pageCount);
   ~BufferManager();

   BufferFrame& fixPage(uint64_t pageId, bool exclusive);
   void unfixPage(BufferFrame& frame, bool isDirty);
};


// Test code

BufferManager* bm;
unsigned pagesOnDisk;
unsigned pagesInRAM;
unsigned threadCount;
unsigned* threadSeed;
atomic<bool> stop(false);

unsigned randomPage(unsigned threadNum) {
   // pseudo-gaussian, causes skewed access pattern
   unsigned page=0;
   for (unsigned  i=0; i<20; i++)
      page+=rand_r(&threadSeed[threadNum])%pagesOnDisk;
   return page/20;
}

static void* scan(void *arg) {
   // scan all pages and check if the counters are not decreasing
   vector<unsigned> counters(pagesOnDisk, 0);

   while (!stop) {
      unsigned start = random()%(pagesOnDisk-10);
      for (unsigned page=start; page<start+10; page++) {
         BufferFrame& bf = bm->fixPage(page, false);
         unsigned newcount = reinterpret_cast<unsigned*>(bf.getData())[0];
         assert(counters[page]<=newcount);
         counters[page]=newcount;
         bm->unfixPage(bf, false);
      }
   }

   return NULL;
}

static void* readWrite(void *arg) {
   // read or write random pages
   uintptr_t threadNum = reinterpret_cast<uintptr_t>(arg);

   uintptr_t count = 0;
   for (unsigned i=0; i<100000/threadCount; i++) {
      bool isWrite = (rand_r(&threadSeed[threadNum])%128) < 10;
      BufferFrame& bf = bm->fixPage(randomPage(threadNum), isWrite);

      if (isWrite) {
         count++;
         reinterpret_cast<unsigned*>(bf.getData())[0]++;
      }
      bm->unfixPage(bf, isWrite);
   }

   return reinterpret_cast<void*>(count);
}

int main(int argc, char** argv) {
   if (argc==4) {
      pagesOnDisk = atoi(argv[1]);
      pagesInRAM = atoi(argv[2]);
      threadCount = atoi(argv[3]);
   } else {
      cerr << "usage: " << argv[0] << " <pagesOnDisk> <pagesInRAM> <threads>" << endl;
      exit(1);
   }

   threadSeed = new unsigned[threadCount];
   for (unsigned i=0; i<threadCount; i++)
      threadSeed[i] = i*97134;

   bm = new BufferManager(pagesInRAM);

   vector<pthread_t> threads(threadCount);
   pthread_attr_t pattr;
   pthread_attr_init(&pattr);

   // set all counters to 0
   for (unsigned i=0; i<pagesOnDisk; i++) {
      BufferFrame& bf = bm->fixPage(i, true);
      reinterpret_cast<unsigned*>(bf.getData())[0]=0;
      bm->unfixPage(bf, true);
   }

   // start scan thread
   pthread_t scanThread;
   pthread_create(&scanThread, &pattr, scan, NULL);

   // start read/write threads
   for (unsigned i=0; i<threadCount; i++)
      pthread_create(&threads[i], &pattr, readWrite, reinterpret_cast<void*>(i));

   // wait for read/write threads
   unsigned totalCount = 0;
   for (unsigned i=0; i<threadCount; i++) {
      void *ret;
      pthread_join(threads[i], &ret);
      totalCount+=reinterpret_cast<uintptr_t>(ret);
   }

   // wait for scan thread
   stop = true;
   pthread_join(scanThread, NULL);

   // restart buffer manager
   delete bm;
   bm = new BufferManager(pagesInRAM);

   // check counter
   unsigned totalCountOnDisk = 0;
   for (unsigned i=0; i<pagesOnDisk; i++) {
      BufferFrame& bf = bm->fixPage(i,false);
      totalCountOnDisk+=reinterpret_cast<unsigned*>(bf.getData())[0];
      bm->unfixPage(bf, false);
   }
   if (totalCount==totalCountOnDisk) {
      cout << "test successful" << endl;
      delete bm;
      return 0;
   } else {
      cerr << "error: expected " << totalCount << " but got " << totalCountOnDisk << endl;
      delete bm;
      return 1;
   }
}
