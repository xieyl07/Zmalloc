#pragma once

#include "misc.h"
#include "run.h"

namespace myAlloc {

constexpr int UNALLOCATED_TYPE = 0b00;
constexpr int SMALL_TYPE = 0b01;
constexpr int LARGE_TYPE = 0b10;
constexpr int CHUNKS_TYPE = 0b11;

// 只设置要用到的部分, 其他不管, 炸了就炸了
// 对于 small, 所有 page_i 设置 run_offset 和 type, 第一个 page_i 设置 run_i
// 对于 large, 第一个 page_i 设置 page_num 和 type, 中间的全部 memset 0,
//            中间的 page_i 只要不被当成 small 就行了
// 对于 unallocated, 头尾两个 page_i 设置 page_num 和 type. 中间不管,
//            因为这货只在avail_pages里, 也就合并时候用用, 不会和 small 混的
// 对于 chunks, 头部第一个 page_i 设置一下 chunks_num 和 type,
//            分配出去的 huge 和在树上的 chunks 不需要区分
// 符合规则的 page, 只要按读取的顺序设置关心的字段就好了.
// 只在分配时初始化, 所有都这样
struct PageInfo : NoCopy {
    struct {
        RunInfo run_i; // 只设置第一块
    } small;

    int get_type() const {
        return map & 0b11;
    }
    int get_run_offset_s() const {
        return map >> 2;
    }
    int get_num_luc() const { // large & unallocated 的 page num 和 chunks 的 num
        return map >> 2;
    }

    // unallocated 初始化两端
    void init_unallocated_ends(int page_num) {
        map = page_num << 2 | UNALLOCATED_TYPE;
    }
    void init_small_normal(int run_offset) {
        map = run_offset << 2 | SMALL_TYPE;
    }
    void init_small_head(int bin_id) {
        small.run_i.init(bin_id);
        init_small_normal(0);
    }
    // head 初始化第一个
    void init_large_head(int page_num) {
        map = page_num << 2 | LARGE_TYPE;
    }
    void init_chunks_head(int chunk_num) {
        map = chunk_num << 2 | CHUNKS_TYPE;
    }

 private:
    // bits:
    // 最后两位是 type, 0b00: unallocated, 0b01: small, 0b10: large,
    // 0b11: chunks(不区分是 huge 还是 树上的, 没必要)
    // 剩余的位是 各个 type 不同的地方, 相当于 struct 里的 union
    // 哪些 page_i 要设置写在上面了
    bitmap_t map;
};

static void init_unallocated(PageInfo *begin_page_i, int page_num) {
    begin_page_i->init_unallocated_ends(page_num);
    (begin_page_i + page_num - 1)->init_unallocated_ends(page_num);
}

static void init_small(PageInfo *begin_page_i, int page_num, int bin_id) {
    begin_page_i->init_small_head(bin_id);
    PageInfo *page_i = begin_page_i + 1;
    for (int i = 1; i < page_num; ++i, ++page_i) {
        page_i->init_small_normal(i);
    }
}

// run_i 到 PageInfo 的距离
const int RUNINFO_OFFSET = (p_t)(&((PageInfo*)0)->small.run_i);

// chunk_sz_cmp 用在 set_zero 的类型制定, 不需要构造对象
struct page_sz_cmp {
    bool operator()(PageInfo *const lhs, PageInfo *const rhs) const {
        return lhs->get_num_luc() < rhs->get_num_luc() ||
               (lhs->get_num_luc() == rhs->get_num_luc() && lhs < rhs);
    }
};

struct sz_find_p {
    bool operator()(PageInfo *lhs, int rhs) const {
        return lhs->get_num_luc() < rhs;
    }
};

extern sz_find_p page_sz_find; // 偷懒定义在arena.h 里

} // namespace
