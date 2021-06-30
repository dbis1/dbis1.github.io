#include <cassert>
#include <cstring>
#include <experimental/string_view>
#include <fcntl.h>
#include <iostream>
#include <string>
#include <sys/mman.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
#include <unordered_map>
#include <vector>

using namespace std;

#define check(expr) if (!(expr)) { perror(#expr); exit(1); }

template<class T>
struct Vector {
   uint64_t count;
   int fd;
   T* data;

   Vector() : data(nullptr) {}
   Vector(const char *pathname) { readBinary(pathname); }
   ~Vector() { if (data) check(munmap(data,count*sizeof(T))==0); }

   void readBinary(const char *pathname) {
      fd=open(pathname,O_RDONLY);
      check(fd!=-1);
      struct stat sb;
      check(fstat(fd, &sb)!=-1);
      count=static_cast<uint64_t>(sb.st_size)/sizeof(T);
      data=reinterpret_cast<T*>(mmap(nullptr, count*sizeof(T), PROT_READ, MAP_PRIVATE, fd, 0));
      check(data!=MAP_FAILED);
   }

   uint64_t size() { return count; }
   T operator[](std::size_t idx) { return data[idx]; }
};

typedef std::experimental::string_view str;

template<>
struct Vector<str> {
  struct Data {
     uint64_t count;
     struct {
        uint64_t size;
        uint64_t offset;
     } slot[];
  };

   uint64_t fileSize;
   int fd;
   Data* data;

   Vector() : data(nullptr) {}
   Vector(const char *pathname) { readBinary(pathname); }
   ~Vector() { if (data) check(munmap(data,fileSize)==0); }

   void readBinary(const char *pathname) {
      fd=open(pathname,O_RDONLY);
      check(fd!=-1);
      struct stat sb;
      check(fstat(fd, &sb)!=-1);
      fileSize=static_cast<uint64_t>(sb.st_size);
      data=reinterpret_cast<Data*>(mmap(nullptr, fileSize, PROT_READ, MAP_PRIVATE, fd, 0));
      check(data!=MAP_FAILED);
   }

   uint64_t size() { return data->count; }
   str operator[](std::size_t idx) { auto slot=data->slot[idx]; return str(reinterpret_cast<char*>(data)+slot.offset,slot.size); }
};


int main() {
   Vector<int> p_partkey("tpch1binary/p_partkey");
   Vector<str> p_name("tpch1binary/p_name");
   Vector<str> p_mfgr("tpch1binary/p_mfgr");
   Vector<str> p_brand("tpch1binary/p_brand");
   Vector<str> p_type("tpch1binary/p_type");
   Vector<int> p_size("tpch1binary/p_size");
   Vector<str> p_container("tpch1binary/p_container");
   Vector<float> p_retailprice("tpch1binary/p_retailprice");
   Vector<str> p_comment("tpch1binary/p_comment");

   Vector<int> s_suppkey("tpch1binary/s_suppkey");
   Vector<str> s_name("tpch1binary/s_name");
   Vector<str> s_address("tpch1binary/s_address");
   Vector<int> s_nationkey("tpch1binary/s_nationkey");
   Vector<str> s_phone("tpch1binary/s_phone");
   Vector<float> s_acctbal("tpch1binary/s_acctbal");
   Vector<str> s_comment("tpch1binary/s_comment");

   Vector<int> ps_partkey("tpch1binary/ps_partkey");
   Vector<int> ps_suppkey("tpch1binary/ps_suppkey");
   Vector<int> ps_availqty("tpch1binary/ps_availqty");
   Vector<float> ps_supplycost("tpch1binary/ps_supplycost");
   Vector<str> ps_comment("tpch1binary/ps_comment");

   Vector<int> c_custkey("tpch1binary/c_custkey");
   Vector<str> c_name("tpch1binary/c_name");
   Vector<str> c_address("tpch1binary/c_address");
   Vector<int> c_nationkey("tpch1binary/c_nationkey");
   Vector<str> c_phone("tpch1binary/c_phone");
   Vector<float> c_acctbal("tpch1binary/c_acctbal");
   Vector<str> c_mktsegment("tpch1binary/c_mktsegment");
   Vector<str> c_comment("tpch1binary/c_comment");

   Vector<int> o_orderkey("tpch1binary/o_orderkey");
   Vector<int> o_custkey("tpch1binary/o_custkey");
   Vector<str> o_orderstatus("tpch1binary/o_orderstatus");
   Vector<float> o_totalprice("tpch1binary/o_totalprice");
   Vector<str> o_orderdate("tpch1binary/o_orderdate");
   Vector<str> o_orderpriority("tpch1binary/o_orderpriority");
   Vector<str> o_clerk("tpch1binary/o_clerk");
   Vector<int> o_shippriority("tpch1binary/o_shippriority");
   Vector<str> o_comment("tpch1binary/o_comment");

   Vector<int> l_orderkey("tpch1binary/l_orderkey");
   Vector<int> l_partkey("tpch1binary/l_partkey");
   Vector<int> l_suppkey("tpch1binary/l_suppkey");
   Vector<int> l_linenumber("tpch1binary/l_linenumber");
   Vector<float> l_quantity("tpch1binary/l_quantity");
   Vector<float> l_extendedprice("tpch1binary/l_extendedprice");
   Vector<float> l_discount("tpch1binary/l_discount");
   Vector<float> l_tax("tpch1binary/l_tax");
   Vector<str> l_returnflag("tpch1binary/l_returnflag");
   Vector<str> l_linestatus("tpch1binary/l_linestatus");
   Vector<str> l_shipdate("tpch1binary/l_shipdate");
   Vector<str> l_commitdate("tpch1binary/l_commitdate");
   Vector<str> l_receiptdate("tpch1binary/l_receiptdate");
   Vector<str> l_shipinstruct("tpch1binary/l_shipinstruct");
   Vector<str> l_shipmode("tpch1binary/l_shipmode");
   Vector<str> l_comment("tpch1binary/l_comment");

   Vector<int> n_nationkey("tpch1binary/n_nationkey");
   Vector<str> n_name("tpch1binary/n_name");
   Vector<int> n_regionkey("tpch1binary/n_regionkey");
   Vector<str> n_comment("tpch1binary/n_comment");
   Vector<int> r_regionkey("tpch1binary/r_regionkey");
   Vector<str> r_name("tpch1binary/r_name");
   Vector<str> r_comment("tpch1binary/r_comment");

   /*
    download link: http://www-db.in.tum.de/~leis/tpch1binary.tgz

    implement TPC-H query 3, try to be as fast as possible:
select
        l_orderkey,
        sum(l_extendedprice * (1 - l_discount)) as revenue,
        o_orderdate,
        o_shippriority
from
        customer,
        orders,
        lineitem
where
        c_mktsegment = 'BUILDING'
        and c_custkey = o_custkey
        and l_orderkey = o_orderkey
        and o_orderdate < date '1995-03-15'
        and l_shipdate > date '1995-03-15'
group by
        l_orderkey,
        o_orderdate,
        o_shippriority
order by
        revenue desc,
        o_orderdate
limit 10
    */

   return 0;
}

