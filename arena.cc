#include "arena.h"

namespace myAlloc {

sz_find<Chunk> chunk_sz_find;
sz_find_p page_sz_find;
int cpu_nthread = nthread_init();

void* Arena::alloc(int size) {
    if (size <= 0) {
        return nullptr;
    }
    if (size > LARGE_SIZE_MAX) { // huge
        return alloc_huge(size);
    } else if (size > SMALL_SIZE_MAX) { // large
        return alloc_large(size);
    } else { // small
        return alloc_small(size);
    }
}

inline void* Arena::alloc_huge(int size) {
    deO("")
    int chunk_num = size + CHUNK_SIZE + 1 + (map_bias << PAGE_SHIFT) >> CHUNK_SHIFT;

    Chunk *chunk = get_nchunk(chunk_num);
    chunk->init(this, chunk_num);

    return a2c(chunk) + (map_bias << PAGE_SHIFT);
}

inline Chunk* Arena::get_nchunk(int num) {
    deO("")
    if (spare && num == 1) {
        Chunk *chunk = spare;
        spare = nullptr;
        return chunk;

    } else if (!cached_chunks_sz.empty() &&
               num <= (*(cached_chunks_sz.rbegin()))->num) {
        Chunk *chunk = splice_tree_chunk(num);
        return chunk;

    } else {
        Chunk* chunk = alloc_chunk(num);
        all_chunks.insert(chunk, num);
        return chunk;
    }
}

// 确保的确能做到再调用
Chunk* Arena::splice_tree_chunk(int num) {
    deO("")
    auto it = lower_bound(cached_chunks_sz.begin(), cached_chunks_sz.end(), num, chunk_sz_find);
    if (it == cached_chunks_sz.end()) {
        deO("exit")
        exit(1);
    }
    Chunk* chunk = *it;
    cached_chunks_sz.erase(it);
    cached_chunks_addr.erase(cached_chunks_addr.find(chunk));

    if (num != chunk->num) {
        auto *tmp = chunk + num;
        tmp->num = chunk->num - num;
        cached_chunks_addr.insert(tmp);
        cached_chunks_sz.insert(tmp);
    }
    return chunk;
}

inline void* Arena::alloc_large(int size) {
    deO("")
    // 内存着色
    int random_offset = get_random_offset(63) << CACHE_LINE_SHIFT;
    int page_num = (size + PAGE_SIZE - 1 + random_offset) / PAGE_SIZE;
    PageInfo *page_i = get_npage(page_num);
    page_i->init_large_head(page_num);
    memset(page_i + 1, 0, (page_num - 1) * sizeof(PageInfo));
    return page_i_to_page(page_i) + random_offset;
}

// 确保的确能做到再调用
inline PageInfo* Arena::splice_tree_page(int page_num) {
    deO("")
    auto it = lower_bound(avail_pages.begin(), avail_pages.end(),
                          page_num, page_sz_find);
    if (it == avail_pages.end()) {
        deO("")
        exit(1);
    }
    PageInfo* page_i = *it;
    avail_pages.erase(it);

    int unallocated_num = page_i->get_page_num_lu();
    if (page_num != unallocated_num) {
        deal_remaining_pages(page_i + page_num, unallocated_num - page_num);
    }
    return page_i;
}

inline void Arena::deal_remaining_pages(PageInfo *page_i, int page_num) {
    deO("")
    init_unallocated(page_i, page_num);
    avail_pages.insert(page_i);
}

inline char* Arena::alloc_small(int size) {
    deO("")
    // 分配后判空
    int bin_id = get_bin_id(size);
    Bin *bin = bins + bin_id;
    RunInfo *run_i = bin->cur_run;
    if (run_i) {
        if (run_i->nfree == 1) {
            bin->cur_run = nullptr;
        }
    } else if (!bin->nonfull_runs.empty()) {
        run_i = bin->nonfull_runs.top();
        bin->nonfull_runs.pop();
        if (run_i->nfree > 1) {
            bin->cur_run = run_i;
        }
    } else {
        run_i = get_run(bin_id);
        if (run_i->nfree > 1) {
            bin->cur_run = run_i;
        }
    }
    char *ret = run_i->get_region();

    return ret;
}

inline PageInfo* Arena::get_npage(int page_num) {
    deO("")
    if (!avail_pages.empty() &&
        page_num <= (*avail_pages.rbegin())->get_page_num_lu()) {
        return splice_tree_page(page_num);
    } else {
        Chunk *chunk = get_nchunk(1);
        chunk->init(this, 1);
        PageInfo *page_i = chunk->pages_i;
        // 返回 page_num 个 page 就是了, 不必初始化
        if (page_num < CHUNK_PAGE_NUM - map_bias) {
            deal_remaining_pages(page_i + page_num,
                                 CHUNK_PAGE_NUM - map_bias - page_num);
        }
        return page_i;
    }

    // never get here
}

inline RunInfo* Arena::get_run(int bin_id) {
    deO("")
    int page_num = bin_info[bin_id].page_num;
    PageInfo *page_i = get_npage(page_num);
    init_small(page_i, page_num, bin_id);
    RunInfo &run_i = page_i->small.run_i;
    run_i.init(bin_id);
    return &run_i;
}

/************ 若干 free ************/

// free small & large
void Arena::free(void* addr) {
    deO("")

    // ###应该是全局的? 先检查在不在全局的 chunk_map(已分配的chunk) 里,
    // ###在的话再看num>0是huge, 不然是small或者large.
    // ###small或者large的话, 再去读取chunk头里对应的page_i,
    // ###确保allocated, 再看是
    // 这些 if 保证了地址有效, 后面分派的函数就不检查了
    // 没法检查所属内存的 chunk 是否是已分配的 chunk?

    PageInfo *page_i = addr_to_page_i(addr);
    if (!page_i->is_allocated()) {
        // do something for incorrect addr
        return;
    }

    if (page_i->is_large()) {
        free_large(page_i);
        return;
    }

    PageInfo *run_page_i = page_i_to_run_page_i(page_i);
    if (run_page_i->small.run_i.is_alloced_region(a2c(addr))) {
        free_small(a2c(addr), page_i);
        return;
    }

    // do something for incorrect addr
}

void Arena::free_huge(void *addr, Chunk *chunk) {
    deO("")
    // ###过程是拿到 chunk 并从 chunks 中释放, 再调用这个, 所以这个函数不管 chunks
    Arena *a = chunk->arena;
    if (a != this) {
        a->free_huge(addr, chunk);
        return;
    }

    cached_chunks_sz.insert(chunk);
    cached_chunks_addr.insert(chunk);
}

inline void Arena::free_large(PageInfo *page_i) {
    deO("")
    fetch_npage(page_i, page_i->get_page_num_lu());
}

inline void Arena::fetch_npage(PageInfo *page_i, int num) {
    deO("")
    init_unallocated(page_i, num);
    page_i = meld_unallocated_pages(page_i);
    if (page_i->get_page_num_lu() == CHUNK_PAGE_NUM - map_bias) {
        fetch_chunk(addr_to_chunk(page_i));
        return;
    }
    deO("%d", (*avail_pages.rbegin())->get_page_num_lu())
    avail_pages.insert(page_i);
}

inline PageInfo* Arena::meld_unallocated_pages(PageInfo *page_i_beg) {
    deO("")
    Chunk *chunk = addr_to_chunk(page_i_beg);
    int page_id = page_i_to_page_id(page_i_beg, chunk);
    int page_num = page_i_beg->get_page_num_lu();
    PageInfo *page_i_end = page_i_beg + page_num - 1;

    // 尝试合并前面的 page
    if (page_id != 0 && !(page_i_beg - 1)->is_allocated()) {
        deO("%p", &((page_i_beg - 1)->map))
        deO("%u", (page_i_beg - 1)->map)
        int prev_num = (page_i_beg - 1)->get_page_num_lu();
        page_i_beg -= prev_num;
        avail_pages.erase(page_i_beg);
        page_num += prev_num;
        page_i_beg->set_page_num_lu(page_num);
        page_i_end->set_page_num_lu(page_num);
    }

    // 尝试合并后面的 page
    if (page_id != CHUNK_PAGE_NUM - map_bias - 1 &&
        !(page_i_end + 1)->is_allocated())
    {
        int next_num = (page_i_end + 1)->get_page_num_lu();
        page_i_end += next_num;
        avail_pages.erase(page_i_end);
        page_num += next_num;
        page_i_beg->set_page_num_lu(page_num);
        page_i_end->set_page_num_lu(page_num);
    }

    return page_i_beg;
}

void Arena::fetch_chunk(Chunk *chunk) {
    deO("")
    // spare 放入 RB Tree
    if (spare) {
        if (!cached_chunks_addr.empty()) {

            auto next_it = upper_bound(cached_chunks_addr.begin(),
                                       cached_chunks_addr.end(), spare);
            if (next_it != cached_chunks_addr.begin()) {
                auto prev_it = --next_it;
                if (chunk - *prev_it == (*prev_it)->num) {
                    spare = *prev_it;
                    spare->num += 1;
                    cached_chunks_addr.erase(prev_it);
                    cached_chunks_sz.erase(prev_it);
                }
            }

            if (next_it != cached_chunks_addr.end()) {
                if (*next_it - chunk == spare->num) {
                    spare->num += (*next_it)->num;
                    cached_chunks_addr.erase(next_it);
                    cached_chunks_sz.erase(next_it);
                }
            }
        }
        cached_chunks_sz.insert(spare);
        cached_chunks_addr.insert(spare);
    }
    spare = chunk;
}

inline void Arena::free_small(char *addr, PageInfo *page_i) {
    deO("")
    PageInfo *run_page_i = page_i_to_run_page_i(page_i);
    char *run = page_i_to_page(run_page_i);
    RunInfo &run_i = run_page_i->small.run_i;
    BinInfo *bin_i = bin_info + run_i.bin_id;
    int region_id = (addr - run) / bin_i->region_size;
    run_i.unset_region_id(region_id); // ###其实run_i的所有操作都要上锁
    ++run_i.nfree;
    if (run_i.nfree == bin_i->region_num) {
        fetch_npage(run_page_i, bin_i->page_num);
    } else if (run_i.nfree == 1) {
        Bin &bin = bins[run_i.bin_id];
        if (!bin.cur_run) {
            bin.cur_run = &run_i;
        } else {
            bin.nonfull_runs.push(&run_i);
        }
    }
}

PairingHeap<Arena*, arena_cmp> arenas{new Arena};

} // namespace