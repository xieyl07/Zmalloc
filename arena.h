#pragma once

#include <cstdlib>
#include <cassert>
#include <set>
#include <unordered_set>
#include <thread>
#include "misc.h"
#include "debug.h"
#include "lock.h"
#include "tuning.h"
#include "chunk.h"
#include "page.h"
#include "run.h"
#include "bin.h"
#include "addr_convert.h"

using namespace std;

namespace myAlloc {

// ###貌似没必要
//class {
// public:
//    void insert(Chunk *chunk, int num) {
//        lock.lock();
//        for (int i = 0; i < num; ++i) {
//            chunks.insert(chunk);
//            chunk = (Chunk*)(a2c(chunk) + CHUNK_SIZE);
//        }
//        lock.unlock();
//    }
//    bool find(Chunk *chunk) {
//        lock.lock();
//        bool ret = chunks.find(chunk) != chunks.end();
//        lock.unlock();
//        return ret;
//    }
//
// private:
//    SpinLock lock;
//    unordered_set<Chunk*> chunks;
//} all_chunks; // 所有分配的 chunk 都在里面了

// debug things
void display_set(set<PageInfo*, page_sz_cmp> &s);


// Arena 是 new 出来的, 记得把成员变量都设置一下初始值
class Arena : NoCopy {
 public:
    ~Arena() {
        bool f = false;
        for (int i = 0; i < NBINS; ++i) {
            if (bins[i].cur_run) {
                f = true;
                cerr << i;
                waO(": memory leaking1, %d", bins[i].cur_run->nfree);
            }
            if (!bins[i].nonfull_runs.empty()) {
                f = true;
                cerr << i;
                waO(": memory leaking2, %d", bins[i].nonfull_runs.size());
            }
        }
        if (!avail_pages.empty()) {
            f = true;
            waO("memory leaking3, %d", avail_pages.size());
            display_set(avail_pages);
        }
        if (!f) {
            cerr << "no memory leaking" << endl;
            cerr << "release memory, cached_chunks.size() = " << cached_chunks_addr.size() << endl;
            for (void *p : cached_chunks_addr) {
                free(p);
            }
            if (spare) {
                free(spare);
            }
        }
    }

    // alloc 的接口, 就算用不到
    inline void* alloc(int size);

    inline char* alloc_small(int bin_id);
    inline void bulk_alloc_small(int bin_id, void **target, int num) {
        Bin *bin = bins + bin_id;
        int r_size = (bin_info + bin_id)->region_size;
        bin->lock.lock();

        while (num > 0) {
            RunInfo *run_i = get_run_i(bin_id, num);
            int tmp = run_i->get_nregion_id(target, num);
            char *run = run_i_to_page(a2c(run_i));

            // filling
            for (int i = 0; i < tmp; ++i) {
                target[i] = run + r_size * (int)(long)target[i];
            }
            target += tmp;
            num -= tmp;
        }

        bin->lock.unlock();
    }
    inline void* alloc_large(int size);
    inline void* alloc_huge(int size);

    // free 的接口没必要, 就是 myalloc 里那个 dispatch. 真想要就复制过来就行了
    inline void free_small(char *addr, PageInfo *page_i);
    // 现在的做法是释放时绕过tcache. 集中释放的话可能需要这个, 不过更需要改一改tbin.
    // 向tbin free的时候是有拿到run_i的, tcache放到tbin之后就扔掉了, 有点浪费.
    // 而且一些散的东西也没法集中释放
    inline void bulk_free_small(char **addr, int num);
    inline void free_large(PageInfo *page_i);
    inline void free_huge(Chunk *chunk);

    // 持有这个 arena 的线程数
    // public 的成员变量才需要锁, arena 只有这一个.
    // 改变值只在 arenas 操作时, 和 arenas 用同一把锁吧, 每次 arenas 操作完别急着 unlock()
    int nthread = 0;

 private:
    // 选择合适的指针类型, 像 set_zero 用真实类型, cast 的次数很少.
    // void* 还是 char* 还是真实类型. 真实类型的话当心加减! 就算大部分对的, 错用一次就够烦的

    // 总的分配大小单元的函数
    inline RunInfo* get_run(int bin_id);
    inline PageInfo* get_npage(int page_num);
    inline Chunk* get_nchunk(int num);

    // 总的释放大小单元的函数
    inline void fetch_npage(PageInfo *page, int page_num);
    inline void fetch_chunk(Chunk *chunk);

    inline RunInfo* get_run_i(int bin_id, int acquired_num);
    inline PageInfo* splice_page_tree(int page_num);
    inline Chunk* splice_chunk_tree(int num);

    // 其余辅助函数, 都是处理 page_avails 用的
    inline void deal_remaining_pages(PageInfo *page_i, int page_num);
    inline void meld_unallocated_pages(PageInfo *&page_i_beg, int &page_num);

    // chunk things
    Chunk *spare = nullptr; // the most recently freed chunk(1 个). 不插入 cached_tree
    set<Chunk*> cached_chunks_addr; // rbtree. dirty
    set<Chunk*, chunk_sz_cmp> cached_chunks_sz; // rbtree. dirty
    SpinLock chunk_lock; // for above 3 chunks container

    // page things
    set<PageInfo*, page_sz_cmp> avail_pages; // 大小排序. 地址排序不需要, 因为合并时候不看树
    SpinLock page_lock;

    // run things
    Bin bins[NBINS] = {};

    // 还有些全局变量, 不在这里
};

// 总结一下, arena 里的成员变量要锁上, chunk 一把锁, page 一把锁, 每个 Bin 一把锁, arenas 一把锁
// chunk 的锁是 chunk 的三个缓存块共用, page 的锁就 avail_pages用,
// Bin 里的锁是给外面的人用的, 所以不是private. run 是基于 bin 的, bin 顺带把 run 锁上
// nthread 和 arenas 密切相关, arenas 的锁顺带把 nthread锁上
// 因为太乱, 直接把三个总的函数锁上了. chunk: get_chunks, page: get_npage, run: alloc_small

static inline int nthread_init() {
    int thread_num = thread::hardware_concurrency();
    return thread_num;
}
const int cpu_nthread = nthread_init();

struct arena_cmp {
    bool operator()(Arena *lhs, Arena *rhs) const {
        return lhs->nthread < rhs->nthread;
    }
};

// 帮别的文件初始化的
sz_find_c chunk_sz_find;
sz_find_p page_sz_find;

set<Arena*, arena_cmp> arenas{new Arena};
SpinLock arenas_lock;

/******** 类实现, 为了能 inline 放一个文件里. 按照思路排的, 不像类定义里按分类排 ********/

// 只是留着, 不用
void* Arena::alloc(int size) {
    if (size <= 0) {
        return nullptr;
    }

    if (size > LARGE_SIZE_MAX) { // huge
        return alloc_huge(size);
    } else if (size > SMALL_SIZE_MAX) { // large
        return alloc_large(size);
    } else { // small
        return alloc_small(get_bin_id(size));
    }
}

void* Arena::alloc_huge(int size) {
    deO("")
    int chunk_num = (size + (map_bias << PAGE_SHIFT) + CHUNK_SIZE - 1) >> CHUNK_SHIFT;

    // 以后一直是 get 块, 再 init. get 里不对要返回的块进行分割
    Chunk *chunk = get_nchunk(chunk_num);
    chunk->init(chunk_num, this);

    return a2c(chunk) + (map_bias << PAGE_SHIFT);
}

Chunk* Arena::get_nchunk(int num) {
    deO("")
    Chunk *chunk;
    chunk_lock.lock();
    if (spare && num == spare->pages_i[0].get_num_luc()) {
        chunk = spare;
        spare = nullptr;

    } else if (spare && num < spare->pages_i[0].get_num_luc()) {
        chunk = spare;
        spare = (Chunk*)(a2c(chunk) + (num << CHUNK_SHIFT));
        spare->init(spare->pages_i[0].get_num_luc() - num, this);

    } else if (!cached_chunks_sz.empty() &&
               num <= (*(cached_chunks_sz.rbegin()))->pages_i[0].get_num_luc()) {
        chunk = splice_chunk_tree(num);

    } else {
        chunk = alloc_chunk(num);
//        all_chunks.insert(chunk, num);
    }
    chunk_lock.unlock();
    return chunk;
}

// 确保的确能做到再调用
Chunk* Arena::splice_chunk_tree(int num) {
    deO("")
    auto it = lower_bound(cached_chunks_sz.begin(), cached_chunks_sz.end(), num, chunk_sz_find);
    if (it == cached_chunks_sz.end()) {
        assert(false);
        exit(1);
    }
    Chunk* chunk = *it;
    cached_chunks_sz.erase(it);
    int ret2 = cached_chunks_addr.erase(chunk);
    // 删除某个元素还是要检查一下是不是真的删掉了. 删除迭代器就别了吧
    assert(ret2 == 1);

    int ori_num = chunk->pages_i[0].get_num_luc();
    if (num != ori_num) {
        Chunk *tmp = (Chunk*)(a2c(chunk) + (num << CHUNK_SHIFT)); // 别 << CHUNK_SIZE !
        tmp->pages_i[0].init_chunks_head(ori_num - num);
        cached_chunks_addr.insert(tmp);
        cached_chunks_sz.insert(tmp);
    }
    return chunk;
}

void* Arena::alloc_large(int size) {
    deO("")
    // 内存着色
    int random_offset = get_random(63) << CACHE_LINE_SHIFT;
    int page_num = (size + PAGE_SIZE - 1 + random_offset) / PAGE_SIZE;
    PageInfo *page_i = get_npage(page_num);
    page_i->init_large_head(page_num);
    memset(page_i + 1, 0, (page_num - 1) * sizeof(PageInfo));
    return page_i_to_page(page_i) + random_offset;
}

// 确保的确能做到再调用
PageInfo* Arena::splice_page_tree(int page_num) {
    deO("")
    auto it = lower_bound(avail_pages.begin(), avail_pages.end(),
                          page_num, page_sz_find);
    if (it == avail_pages.end()) {
        assert(false);
        exit(1);
    }
    PageInfo* page_i = *it;
    avail_pages.erase(it);

    int unallocated_num = page_i->get_num_luc();
    if (page_num != unallocated_num) {
        deal_remaining_pages(page_i + page_num, unallocated_num - page_num);
    }
    return page_i;
}

void Arena::deal_remaining_pages(PageInfo *page_i, int page_num) {
    deO("")
    init_unallocated(page_i, page_num);
    // 就不单独锁了
    avail_pages.insert(page_i);
}

// 其实 bin_id 就可以代表 size
char* Arena::alloc_small(int bin_id) {
    Bin *bin = bins + bin_id;
    bin->lock.lock();

    RunInfo *run_i = get_run_i(bin_id, 1);

    // run_i 还留在 bin 里, 所以要么对 run_i 上锁, 要么继续持有 bin 的锁
    char *ret = run_i->get_region();
    bin->lock.unlock();

    return ret;
}

// 具体的分配途径. acquired_num 变成还差多少
RunInfo* Arena::get_run_i(int bin_id, int acquired_num) {
    Bin *bin = bins + bin_id;
    RunInfo *run_i = bin->cur_run;
    if (run_i) {
        // 如果不够大就从 cur_run拿掉
        if (run_i->nfree <= acquired_num) {
            bin->cur_run = nullptr;
        }
    } else if (!bin->nonfull_runs.empty()) {
        run_i = *bin->nonfull_runs.begin();
        bin->nonfull_runs.erase(bin->nonfull_runs.begin());
        // 如果有多就放到 cur_run
        if (run_i->nfree > acquired_num) {
            bin->cur_run = run_i;
        }
    } else {
        run_i = get_run(bin_id);
        // 如果有多就放到 cur_run
        if (run_i->nfree > acquired_num) {
            bin->cur_run = run_i;
        }
    }
    return run_i;
}

PageInfo* Arena::get_npage(int page_num) {
    deO("")
    PageInfo *page_i;
    page_lock.lock();
    if (!avail_pages.empty() &&
        page_num <= (*avail_pages.rbegin())->get_num_luc()) {
        page_i = splice_page_tree(page_num);

    } else {
        Chunk *chunk = get_nchunk(1);
        chunk->init(1, this);
        page_i = chunk->pages_i;
        // 返回 page_num 个 page 就是了, 不必初始化
        if (page_num < CHUNK_PAGE_NUM - map_bias) {
            deal_remaining_pages(page_i + page_num,
                                 CHUNK_PAGE_NUM - map_bias - page_num);
        }
    }
    page_lock.unlock(); // 仔细锁好复杂, 随便一把大锁算了
    return page_i;
}

RunInfo* Arena::get_run(int bin_id) {
    deO("")
    int page_num = bin_info[bin_id].page_num;
    PageInfo *page_i = get_npage(page_num);
    init_small(page_i, page_num, bin_id);
    return (RunInfo*)(a2c(page_i) + RUNINFO_OFFSET);
}


/************ 若干 free ************/

// 是 Chunk 的指针, 不是 huge 的开头的指针!
void Arena::free_huge(Chunk *chunk) {
    assert((p_t)chunk % CHUNK_SIZE == 0);
    fetch_chunk(chunk);
}

// 大的函数, 合并也放在里面了
void Arena::fetch_chunk(Chunk *chunk) {
    deO("")

    chunk_lock.lock();

    if (spare) { // spare 放入 RB Tree
        if (!cached_chunks_addr.empty()) {
            int spare_num = spare->pages_i[0].get_num_luc();
            auto next_it = upper_bound(cached_chunks_addr.begin(),
                                       cached_chunks_addr.end(), spare);
            if (next_it != cached_chunks_addr.begin()) {
                auto prev_it = --next_it;
                int prev_num = (*prev_it)->pages_i[0].get_num_luc();
                // 总结经验, 别对 Chunk* 加减!
                if ((a2c(spare) - a2c(*prev_it)) >> CHUNK_SHIFT == prev_num) {
                    spare = *prev_it;
                    spare_num += prev_num;
                    cached_chunks_addr.erase(prev_it);
                    cached_chunks_sz.erase(prev_it);
                }
            }

            if (next_it != cached_chunks_addr.end()) {
                if ((a2c(*next_it) - a2c(spare)) >> CHUNK_SHIFT == spare_num) {
                    spare_num += (*next_it)->pages_i[0].get_num_luc();
                    cached_chunks_addr.erase(next_it);
                    cached_chunks_sz.erase(next_it);
                }
            }
            spare->pages_i[0].init_chunks_head(spare_num);
        } // !chunk_tree.empty()

        cached_chunks_sz.insert(spare);
        cached_chunks_addr.insert(spare);
    } // if (!spare)

    // 插入 chunk 就是这么简单
    spare = chunk;

    chunk_lock.unlock();
}

void Arena::free_large(PageInfo *page_i) {
    deO("")
    fetch_npage(page_i, page_i->get_num_luc());
}

void Arena::fetch_npage(PageInfo *page_i, int page_num) {
//    inO("page_i: %p, page_num: %d", page_i, page_num)
    page_lock.lock();
    // 引用传值!
    meld_unallocated_pages(page_i, page_num);
//    inO("page_i: %p, page_num: %d", page_i, page_num)
    if (page_num == CHUNK_PAGE_NUM - map_bias) {
        page_lock.unlock();
        Chunk *chunk = addr_to_chunk(page_i);
        chunk->init2(1); // 因为和small的长度信息是用的同一个, 会覆盖的!
        fetch_chunk(chunk);
    } else {
        avail_pages.insert(page_i);
        page_lock.unlock();
    }
}

// 引用传值. 传进来的不需要是 unallocated, 在这里 init_unallocated
void Arena::meld_unallocated_pages(PageInfo *&page_i_beg, int &page_num) {
    deO("")
    Chunk *chunk = addr_to_chunk(page_i_beg);
    int page_id = page_i_to_page_id(page_i_beg, chunk);
    PageInfo *page_i_end = page_i_beg + page_num - 1;

    // 尝试合并前面的 page
    if (page_id != 0 && (page_i_beg - 1)->get_type() == UNALLOCATED_TYPE) {
        int prev_num = (page_i_beg - 1)->get_num_luc();
        if (prev_num != 0) { // large 就不走下面这些了吧
            page_num += prev_num;
            page_i_beg -= prev_num;
            page_id -= prev_num;
            int ret = avail_pages.erase(page_i_beg); // 真没删掉
//        assert(ret == 1);
            if (ret == 0) {
                display_set(avail_pages);
                waO("%d", avail_pages.find(page_i_beg) != avail_pages.end());
            }
        }
    }

    // 尝试合并后面的 page
    if (page_id + page_num != CHUNK_PAGE_NUM - map_bias &&
        (page_i_end + 1)->get_type() == UNALLOCATED_TYPE)
    {
        int ret= avail_pages.erase(page_i_end + 1);
//        assert(ret == 1);
        if (ret == 0) {
            display_set(avail_pages);
            PageInfo *p = page_i_end + 1;
            waO("%d", avail_pages.find(p) != avail_pages.end());
        }
        int next_num = (page_i_end + 1)->get_num_luc();
        page_num += next_num;
        page_i_end += next_num;
    }
    page_i_beg->init_unallocated_ends(page_num);
    page_i_end->init_unallocated_ends(page_num);
}

// page_i 真的指的是普通的 page_i
void Arena::free_small(char *addr, PageInfo *page_i) {
//    inO("")
    PageInfo *run_page_i = page_i_to_run_page_i(page_i);
    char *run = page_i_to_page(run_page_i);
    RunInfo &run_i = run_page_i->small.run_i;
    assert(run_i.bin_id < NBINS);
    auto bin_i = bin_info + run_i.bin_id;
    int region_id = (addr - run) / bin_i->region_size;
    assert(region_id < 512);
    run_i.fetch_region(region_id);

    // run 的所有操作都要上 bin 的锁
    Bin &bin = bins[run_i.bin_id];
    bin.lock.lock();
    ++run_i.nfree;
    if (run_i.nfree == bin_i->region_num) {
        // 归还前先从树上删了!
        if (bin.cur_run == &run_i) {
            bin.cur_run = nullptr;
        } else {
            int ret = bin.nonfull_runs.erase(&run_i);
            assert(ret == 1);
        }
        bin.lock.unlock();
        fetch_npage(run_page_i, bin_i->page_num);
    } else if (run_i.nfree == 1) {
        if (!bin.cur_run) {
            bin.cur_run = &run_i;
        } else {
            bin.nonfull_runs.insert(&run_i);
        }
        bin.lock.unlock();
    } else {
        bin.lock.unlock();
    }
}

void display_set(set<PageInfo*, page_sz_cmp> &s) {
    int cnt = 0;
    for (PageInfo *p : s) {
        waO("%p page_id: %d, page_num: %d", p,
            page_i_to_page_id(p, addr_to_chunk(p)), p->get_num_luc())
        cnt += p->get_num_luc();
    }
    waO("total %d in %d", cnt, s.size());
}

} // namespace
