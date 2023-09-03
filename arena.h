#pragma once

#include <cstdlib>
#include <set>
#include <unordered_set>
#include <thread>
#include "misc.h"
#include "debug.h"
#include "lock.h"
#include "tuning.h"
#include "pairing_heap.h"
#include "chunk.h"
#include "page.h"
#include "run.h"
#include "bin.h"
#include "addr_convert.h"

using namespace std;

namespace myAlloc {

class {
 public:
    void insert(Chunk *chunk, int num) {
        lock.lock();
        for (int i = 0; i < num; ++i) {
            chunks.insert(chunk);
            ++chunk;
        }
        lock.unlock();
    }
    bool find(Chunk *chunk) {
        lock.lock();
        bool ret = chunks.find(chunk) != chunks.end();
        lock.unlock();
        return ret;
    }

 private:
    SpinLock lock;
    unordered_set<Chunk*> chunks;
} all_chunks; // 所有分配的 chunk 都在里面了

class Arena : NoCopy {
 public:
    void* alloc(int size);
    void free(void* addr);
    void free_huge(void *addr, Chunk *chunk);

    int nthread;

 private: // ###还要排一下顺序
    char* alloc_small(int size);
    void* alloc_large(int size);
    void* alloc_huge(int size);

    void free_small(char *addr, PageInfo *page_i);
    void free_large(PageInfo *page_i);

    // ### 选择合适的指针类型, 像 set 用真实类型, cast 的次数很少. void* 还是 char*
    Chunk* splice_tree_chunk(int num);
    PageInfo* splice_tree_page(int page_num);
    PageInfo* meld_unallocated_pages(PageInfo *page_i_beg);
    void fetch_npage(PageInfo *page, int num);
    void fetch_chunk(Chunk *chunk);
    PageInfo* get_npage(int page_num);
    RunInfo* get_run(int bin_id);
    void deal_remaining_pages(PageInfo *page_i, int page_num);

    Chunk* get_nchunk(int num);

    // chunk & huge things
    Chunk *spare; // the most recently freed chunk(1 个). 不插入 cached_tree
    set<Chunk*> cached_chunks_addr; // rbtree. dirty
    set<Chunk*, sz_cmp<Chunk>> cached_chunks_sz; // rbtree. dirty
    SpinLock chunk_lock;

    // run things
    Bin bins[NBINS];
    set<PageInfo*, page_sz_cmp> avail_pages; // 大小排序. 地址排序不需要, 因为合并时候不看树
    SpinLock page_lock;
}; // class arena

static int nthread_init() {
    int thread_num = thread::hardware_concurrency();
    return thread_num;
}
extern int cpu_nthread; // arena.cc

struct arena_cmp {
    bool operator()(Arena *lhs, Arena *rhs) const {
        return lhs->nthread < rhs->nthread;
    }
};

extern PairingHeap<Arena*, arena_cmp> arenas; // 定义在 arena.cc

} // namespace
