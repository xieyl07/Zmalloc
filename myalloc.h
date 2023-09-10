#pragma once

//#define private public

#include "arena.h"
#include "tcache.h"

namespace myAlloc {

void* myalloc(size_t size) {
//    inO("%d", size)
    if (size == 0) {
        return nullptr;
    }
    assert(size < 100 MB);

    void *ret = tcache.alloc(size);
    // for test, 跳过 tcache. 记得把tcache 的构造函数里的预分配和析构函数注释掉,
    // 加上 #define private public
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
//        tcache.free(ptr, page_i);
        // 直接释放到 arena, 绕过 tcache.因为感觉浪费, 具体见Arena::bulk_free_small里面写的
        // 但是 gc 是不是要改改了, 多少次操作之后就分配或释放到标准线而不是释放3/4
        chunk->arena->free_small(a2c(ptr), page_i);
        break;
    case UNALLOCATED_TYPE:
    default:
        assert(false);
        exit(1);
    }
}

} // namespace