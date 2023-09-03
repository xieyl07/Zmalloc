#pragma once

#include "arena.h"
#include "tcache.h"

namespace myAlloc {

void* myalloc(int size) {
    if (size <= 0) return nullptr;

    void *ret = tcache.alloc(size);
//    void *ret = malloc(size);
    return ret;
}

void myfree(void *ptr) {
    tcache.free(ptr);
//    free(ptr);
}

} // namespace