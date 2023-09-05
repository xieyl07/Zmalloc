#pragma once

#define private public

#include "arena.h"
#include "tcache.h"

namespace myAlloc {

int cnt = 0;

void* myalloc(int size) {
    if (size <= 0) return nullptr;

    void *ret = tcache.alloc(size);
//    void *ret = tcache.arena->alloc(size);
    return ret;
}

void myfree(void *ptr) {
    tcache.free(ptr);
}

} // namespace