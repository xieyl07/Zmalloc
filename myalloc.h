#pragma once

//#define private public

#include "arena.h"
#include "tcache.h"

namespace myAlloc {

void* myalloc(int size) {
//    inO("%d", size)
    if (size <= 0) {
        return nullptr;
    }

    void *ret = tcache.alloc(size);
    // ###for test, 跳过 tcache. 记得把tcache 的构造函数里的预分配和析构函数注释掉,
    // ###加上 #define private public
    //    void *ret = tcache.arena->alloc(size);
    return ret;
}

void myfree(void *ptr) {
    Chunk *chunk = addr_to_chunk(ptr);
    PageInfo *page_i = addr_to_page_i(ptr);
    switch (page_i->get_type()) {
    case LARGE_TYPE:
        chunk->arena->free_large(page_i);
        break;
    case CHUNKS_TYPE:
        chunk->arena->free_huge(chunk);
        break;
    case SMALL_TYPE:
        tcache.free(ptr, page_i);
//    chunk->arena->free_small(ptr, page_i); // ###for test, 跳过 tcache
        break;
    case UNALLOCATED_TYPE:
    default:
        assert(false);
        exit(1);
    }
}

} // namespace