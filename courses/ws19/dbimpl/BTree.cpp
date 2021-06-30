/*
 * Implement a standard B+ Tree (single threaded, synchronization is a plus).
 */
#include <iostream>
#include <algorithm>
#include <vector>
#include <map>
#include <cassert>

using namespace std;

enum class PageType : uint8_t { BTreeInner=1, BTreeLeaf=2 };

struct NodeBase {
   uint16_t count;
   PageType type;
};

struct BTreeLeafBase : public NodeBase {
   static const PageType typeMarker=PageType::BTreeLeaf;
};

template<class Key,class Payload>
struct BTreeLeaf : public BTreeLeafBase {
   struct Entry {
      Key k;
      Payload p;
   };

   static const uint64_t pageSizeLeaf=16*1024;
   static const uint64_t maxEntries=/* TODO */0;

   BTreeLeaf() {
      count=0;
      type=typeMarker;
   }


   void insert(Key k,Payload p) {
     //TODO
   }

   BTreeLeaf* split(Key& sep) {
     //TODO
   }
};

struct BTreeInnerBase : public NodeBase {
   static const PageType typeMarker=PageType::BTreeInner;
};

template<class Key>
struct BTreeInner : public BTreeInnerBase {
   static const uint64_t pageSizeInner=16*1024;
   static const uint64_t maxEntries= /* TODO */0;

  BTreeInner() {
      count=0;
      type=typeMarker;
   }

   unsigned lowerBound(Key k) {
     // TODO
     return 0;
   }

   BTreeInner* split(Key& sep) {
     // TODO
   }

   void insert(Key k,NodeBase* child) {
     // TODO
   }

};


template<class Key,class Value>
struct BTree {
   NodeBase* root;

   BTree() {
      root = new BTreeLeaf<Key,Value>();
   }

   void makeRoot(Key k,NodeBase* leftChild,NodeBase* rightChild) {
     // TODO
   }

   void insert(Key k,Value v) {
     // TODO
   }

   Value lookup(Key k) {
     //TODO
     return 0;
   }
};


typedef uint32_t keytype;

int main(int argc,char** argv) {
  uint64_t n=getenv("N") ? atoi(getenv("N")) : 10e4;

   vector<keytype> v(n);
   for (uint64_t i=0; i<n; i++)
      v[i]=i;
   random_shuffle(v.begin(),v.end());

   {
      BTree<keytype,uint64_t> tree;
      for (uint64_t i=0; i<n; i++)
          tree.insert(v[i],i);

      for (uint64_t i=0; i<n; i++)
        if (tree.lookup(v[i])!=i){
              throw;
        }
   }


   return 0;
}
