#pragma once

#include <cstring>
#include "misc.h"
#include "page.h"
#include "tuning.h"
#include "forward_declartion.h"

namespace myAlloc {

// 每次分配了 chunk 就 new 一个放到 map 里去
struct Chunk : NoCopy {
    Arena *arena;
    int num; // chunk number, 只有huge > 1

    PageInfo pages_i[1];
    // other PageInfo, and pages

    void init(Arena *a, int page_num) {
        arena = a;
        num = page_num;
    }
};

template <typename T>
struct sz_cmp { // 按大小排序的两个 chunk set
    // const 一定要! 不然 set 不能插入了
    bool operator()(const T *lhs, const T *rhs) const {
        return lhs->num < rhs->num || lhs < rhs;
    }
};

template <typename T>
struct sz_find {
    bool operator()(T *lhs, int rhs) const {
        return lhs->num == rhs;
    }
};

extern sz_find<Chunk> chunk_sz_find; // 偷懒定义在arena.cc里

static Chunk *alloc_chunk(int n) {
    void *addr = aligned_alloc(CHUNK_SIZE, CHUNK_SIZE * n);
    return (Chunk*)addr;
}

} // namespace
