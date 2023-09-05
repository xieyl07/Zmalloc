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
    struct {
        RunInfo run_i; // 只设置第一块
    } small;

    bool is_allocated() const {
        return map & 1;
    }
    bool is_large() const {
        return map & 2;
    }
    int get_run_offset_small() const {
        return map >> 2;
    }
    int get_page_num_lu() const { // large / unallocated
        return map >> 2;
    }

    void set_allocated_flag(bool flag) {
        if (flag) {
            map = map | 1;
        } else {
            map = map & ~1U;
        }
    }
    void set_large_flag(bool flag) {
        if (flag) {
            map = map | 2;
        } else {
            map = map & ~2U;
        }
    }
    void set_run_offset_small(int val) {
        map = val << 2 | map & 3;
    }
    void set_page_num_lu(int val) { // large / unallocated
        map = val << 2 | map & 3;
    }

    void init_large_head(int page_num) {
        set_page_num_lu(page_num);
        set_allocated_flag(true);
        set_large_flag(true);
    }
    void init_small_head(int bin_id) {
        small.run_i.init(bin_id);
        init_small_normal(0);
    }
    void init_small_normal(int run_offset) {
        set_allocated_flag(true);
        set_large_flag(false);
        set_run_offset_small(run_offset);
    }
    // unallocated 初始化两端
    void init_unallocated_ends(int page_num) {
        set_allocated_flag(false);
        set_page_num_lu(page_num);
    }

 private:
    // bits:
    // 最低位是 is_allocated 标志, 倒数第二位是 is_large 标志
    // 剩余的位是 small 和 large / unallocated 不同的地方, 相当于 struct 里的一个union
    // 对于small, 是 run_offset, 到 run_page_i 还差几个, 每个 page 都设置
    // 对于 large 和 unallocated, 是 page_num, large 只设置头, avail_pages 里的 unallocated pages 只设置头尾
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

// sz_cmp 用在 set 的类型制定, 不需要构造对象
struct page_sz_cmp {
    bool operator()(PageInfo *const lhs, PageInfo *const rhs) const {
        return lhs->get_page_num_lu() < rhs->get_page_num_lu()
            || lhs < rhs;
    }
};

struct sz_find_p {
    bool operator()(PageInfo *lhs, int rhs) const {
        return lhs->get_page_num_lu() < rhs;
    }
};

extern sz_find_p page_sz_find; // 偷懒定义在arena.cc里

} // namespace
