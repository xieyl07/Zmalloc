#pragma once

#include <cassert>
#include "misc.h"
#include "tuning.h"
#include "arena.h"

//#define memory_leaks_detecet

namespace myAlloc {

// tcache 的好处在线程数大于四倍的cpu线程数的时候才能体现出来
class TCache : NoCopy {
 public:
    TCache() {

        // get arena
        arenas_lock.lock();
        if ((*arenas.begin())->nthread == 0) {
            arena = *arenas.begin();
            arenas.erase(arenas.begin());
        } else if (arenas.size() < cpu_nthread * 4) {
            arena = new Arena;
        } else {
            arena = *arenas.begin();
            arenas.erase(arenas.begin());
        }
        ++arena->nthread;
        arenas.insert(arena);
        arenas_lock.unlock();

        // 预分配, 初始化
        for (int i = 0; i < NBINS; ++i) {
            for (int j = 0; j < NCACHED_MAX / 2; ++j) {
                tbins[i][j] = get_regions(i);
            }
            low_water[i] = bin_pointer[i] = NCACHED_MAX / 2 - 1;
            fill_shift[i] = 1;
            event_cnt[i] = 0;
        }
    }
    ~TCache() {
        arenas_lock.lock();
        arenas.erase(arena);
        --arena->nthread;
        arenas.insert(arena);
        arenas_lock.unlock();
        for (int i = 0; i < NBINS; ++i) {
            fetch_regions(i, bin_pointer[i] + 1);
        }
#ifdef memory_leaks_detecet
        delete arena; // for test###
#endif
    }

    // 算是一个总的入口, 外面不会直接访问 arena 的, 只会调用 tcache 的 alloc
    void* alloc(int size) {
        // size < 0 外面的 myallc 检查过了
        if (size > LARGE_SIZE_MAX) { // huge
            return arena->alloc_huge(size);
        } else if (size > SMALL_SIZE_MAX) { // large
            return arena->alloc_large(size);
        }

        int bin_id = get_bin_id(size);
        if (++event_cnt[bin_id] == TCACHE_GC_INCR) {
            gc(bin_id);
        }
        int &pointer = bin_pointer[bin_id];
        if (pointer < 0) {
            int fill_num = NCACHED_MAX >> fill_shift[bin_id];
            for (int i = 0; i < fill_num; ++i) {
                tbins[bin_id][i] = get_regions(bin_id);
            }
            pointer = fill_num - 1;
        }
        void *ptr = tbins[bin_id][pointer--];
        if (pointer < low_water[bin_id]) {
            low_water[bin_id] = pointer;
        }
        return ptr;
    }
    // 分的事大小让 myalloc 去做, tcache 只 free small
    void free(void *region, PageInfo *page_i) {
        deO("")
        int bin_id = page_i_to_run_page_i(page_i)->small.run_i.bin_id;
        if (++event_cnt[bin_id] == TCACHE_GC_INCR) {
            gc(bin_id);
        }
        if (bin_pointer[bin_id] == NCACHED_MAX - 1) {
            fetch_regions(bin_id, NCACHED_MAX / 2);
        }
        tbins[bin_id][++bin_pointer[bin_id]] = region;
    }

 private:
    void gc(int bin_id) {
        int remainint_water = low_water[bin_id];
        // 每次分配 ncached_max >> fill_shift
        if (remainint_water < 0) {
            fill_shift[bin_id] = std::max(fill_shift[bin_id] - 1, 0);
        } else if (remainint_water > 7) { // 比标准改了改
            ++fill_shift[bin_id];
            int free_num = remainint_water / 2; // 标准是 3 / 4
            fetch_regions(bin_id, free_num);
        }
    }
    void* get_regions(int bin_id) {
        // 调用 arena 的地方, 想想手上有什么, 是不是可以传过去一点, 免得重复计算
        return arena->alloc_small(bin_id);
    }
    void fetch_regions(int bin_id, int free_num) {
        for (int i = 0; i < free_num; ++i) {
            // 其实返还到自己的 arena 也没事的, 因为按地址找管理单元, 不会找错的
            // 就是可能会偷别的 arena 的 page, 搞得不均匀
            void *ptr = tbins[bin_id][bin_pointer[bin_id]--];
            Chunk *chunk = addr_to_chunk(ptr);
            chunk->arena->free_small(a2c(ptr), addr_to_page_i(ptr));
        }
    }

    Arena *arena; // 只是分配时候用用
    int fill_shift[NBINS]; // 每次分配 ncached_max >> fill_shift
    int low_water[NBINS];
    int event_cnt[NBINS]; // TCACHE_GC_INCR
    void *tbins[NBINS][NCACHED_MAX]; // bins 里的 bin 是一个指针数组
    int bin_pointer[NBINS]; // 指向非空的, 可以直接拿的
};

thread_local TCache tcache;

} // namespace
