#pragma once

#include "misc.h"
#include "tuning.h"
#include "arena.h"

namespace myAlloc {

class TCache : NoCopy {
 public:
    TCache() {
        // get arena
        if (arenas.top()->nthread == 0) {
            arena = arenas.top();
            arenas.pop();
        } else if (arenas.size() < cpu_nthread * 4) {
            static Arena arena1;
            arena = &arena1;
        } else {
            arena = arenas.top();
            arenas.pop();
        }
        ++arena->nthread;
        arenas.push(arena);

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
        for (int i = 0; i < NBINS; ++i) {
            fetch_regions(i, bin_pointer[i] + 1);
        }
    }

    // 算是一个总的入口, 外面不会直接访问 arena 的, 只会调用 tcache 的 alloc
    void* alloc(int size) {
        if (size > SMALL_SIZE_MAX) {
            return arena->alloc(size);
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
    // 总的入口
    void free(void *region) {

        if (!addr_to_page_i(region)->is_large()/* is small### */) {
            Chunk *chunk = addr_to_chunk(region);
            chunk->arena->free(region);
            return;
        }
        int bin_id = addr_to_run_i(region)->bin_id;
        if (++event_cnt[bin_id] == TCACHE_GC_INCR) {
            gc(bin_id);
        }
        if (bin_pointer[bin_id] == NCACHED_MAX) {
            fetch_regions(bin_id, NCACHED_MAX / 2);
        }
        tbins[bin_id][++bin_pointer[bin_id]] = region;
    }

 private:
    void gc(int bin_id) {
        int remainint_water = low_water[bin_id];
        if (remainint_water < 0) {
            --fill_shift[bin_id];
        } else if (remainint_water > 0) {
            ++fill_shift[bin_id];
            int free_num = 3 * remainint_water / 4;
            fetch_regions(bin_id, free_num);
        }
    }
    void* get_regions(int bin_id) {
        return arena->alloc(bin_info[bin_id].region_size);
    }
    void fetch_regions(int bin_id, int free_num) {
        for (int i = 0; i < free_num; ++i) {
            arena->free(tbins[bin_id][bin_pointer[bin_id]--]);
        }
    }

    Arena *arena;
    int fill_shift[NBINS]; // 每次分配 ncached_max >> fill_shift
    int low_water[NBINS];
    int event_cnt[NBINS]; // TCACHE_GC_INCR
    void *tbins[NBINS][NCACHED_MAX]; // bins 里的 bin 是一个指针数组
    int bin_pointer[NBINS]; // 指向非空的, 可以直接拿的
};

thread_local TCache tcache;

} // namespace
