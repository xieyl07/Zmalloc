#pragma once

#include <cstring>
#include "misc.h"
#include "page.h"
#include "tuning.h"
#include "forward_declartion.h"

namespace myAlloc {

struct Chunk : NoCopy {
    Arena *arena;

    PageInfo pages_i[1];
    // other PageInfo, and pages

    void init(int chunk_num, Arena *a) {
        arena = a;
        pages_i[0].init_chunks_head(chunk_num);
    }
    void init2(int chunk_num) {
        pages_i[0].init_chunks_head(chunk_num);
    }
};

struct chunk_sz_cmp { // 按大小排序的两个 chunk set_zero
    // const 一定要! 不然 set_zero 不能插入了
    bool operator()(const Chunk *lhs, const Chunk *rhs) const {
        return lhs->pages_i[0].get_num_luc() < rhs->pages_i[0].get_num_luc() ||
               lhs->pages_i[0].get_num_luc() == rhs->pages_i[0].get_num_luc() && lhs < rhs;
    }
};

struct sz_find_c {
    bool operator()(Chunk *lhs, int rhs) const {
        return lhs->pages_i[0].get_num_luc() < rhs;
    }
};

extern sz_find_c chunk_sz_find; // 偷懒定义在 arena.h 里

inline Chunk *alloc_chunk(int n) {
    assert(n > 0);
    void *chunk = aligned_alloc(CHUNK_SIZE, CHUNK_SIZE * n);
    return (Chunk*)chunk;
}

} // namespace
