#pragma once

#include "misc.h"
#include "run.h"

namespace myAlloc {

// 对于 small, 所有 page_i设置run_offset, 两个bool, 第一个 page_i 设置 run_i
// 对于 large, 第一个 page_i 设置 page_num, 两个bool, 中间的全部 memset 0,
//            读到中间的 page_i 只有 free 的时候, 但是 free 一个中间块干嘛呢
// 对于 unallocated, 头尾两个 page_i 设置 page_num和unallocated. 中间不管,
//            因为这货只在avail_pages里, 也就合并时候用用
// 符合规则的 page, 只要按读取的顺序设置关心的字段就好了.
// 不合规的块(large第一个 page 以外), 按照读取顺序让他第一步就离开
// 只在分配时设置 Chunk 头部的簿记信息, 对应的 page 填充 0
struct PageInfo : NoCopy {
    union { // small 和 lu 不同的地方
        struct {
            RunInfo run_i; // 只设置第一块
            int run_offset; // 到第一个 run 还差几个, 每个 page 都设置
        } small;
        struct {
            int page_num; // large 只设置头, avail_pages 里的 unallocated pages 只设置头尾
        } lu; // large and unallocated
    };
    bool is_allocated;
    bool is_large;

    void init_large_head(int page_num) {
        lu.page_num = page_num;
        is_allocated = true;
        is_large = true;
    }
    void init_small_head(int bin_id) {
        small.run_i.init(bin_id);
        init_small_normal(0);
    }
    void init_small_normal(int run_offset) {
        is_allocated = true;
        is_large = false;
        small.run_offset = run_offset;
    }
    void init_unallocated_head_tail(int page_num) {
        is_allocated = false;
        lu.page_num = page_num;
    }
};

static void init_unallocated(PageInfo *begin_page_i, int page_num) {
    begin_page_i->init_unallocated_head_tail(page_num);
    (begin_page_i + page_num - 1)->init_unallocated_head_tail(page_num);
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

// sz_cmp 用在 set 的类型制定, 不需要构造对象
struct page_sz_cmp {
    bool operator()(PageInfo *const lhs, PageInfo *const rhs) const {
        return lhs->lu.page_num < rhs->lu.page_num || lhs < rhs;
    }
};

struct sz_find_p {
    bool operator()(PageInfo *lhs, int rhs) const {
        return lhs->lu.page_num == rhs;
    }
};

extern sz_find_p page_sz_find; // 偷懒定义在arena.cc里

} // namespace
