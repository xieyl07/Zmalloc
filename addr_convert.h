#pragma once

#include <cstddef>
#include "chunk.h"

namespace myAlloc {

// 头部占用的 page 数量
extern const int map_bias; // 定义在 run.cc 里了

// 要么 inline 要么 static
// 为了看看哪些函数影响大, 把 inline 去掉. 跑分时候记得注释掉
#ifdef inline_expand_M
#define inline static
#endif

inline Chunk* addr_to_chunk(void *addr) {
    return (Chunk*)((p_t)addr & CHUNK_MASK);
}

/*******************************************************/

inline PageInfo* page_i_to_run_page_i(PageInfo *page_i) {
    return page_i - page_i->get_run_offset_s();
}

inline PageInfo* run_i_to_run_page_i(char *run_info) {
    return (PageInfo*)(run_info - RUNINFO_OFFSET);
}

inline int page_i_to_page_id(PageInfo *page_i, Chunk *chunk) {
    return page_i - chunk->pages_i;
}

inline char* page_id_to_page(const int page_id, char *chunk) {
    return ((page_id + map_bias) << PAGE_SHIFT) + chunk;
}

// 总的. 其实中间的 page_i 也行, 不过肯定是 run_page_i 来调用的
inline char* page_i_to_page(PageInfo * page_i) {
    Chunk* chunk = addr_to_chunk(page_i);
    int page_id = page_i_to_page_id(page_i, chunk);
    return page_id_to_page(page_id, a2c(chunk));
}

// 总的. 默认是 run 的第一个 page 的 RunInfo
inline char* run_i_to_page_small(char *run_info) {
    PageInfo *page_i = run_i_to_run_page_i(run_info);
    return page_i_to_page(page_i);
}

/*******************************************************/

inline int addr_to_page_id(char *addr, char* chunk) {
    return ((addr - chunk) / PAGE_SIZE) - map_bias;
}

inline PageInfo* page_id_to_page_i(const int page_id, Chunk *chunk) {
    return chunk->pages_i + page_id;
}

// 总的
inline PageInfo* addr_to_page_i(void *addr) {
    Chunk *chunk = addr_to_chunk(addr);
    int page_id = addr_to_page_id(a2c(addr), a2c(chunk));
    return page_id_to_page_i(page_id, chunk);
}

// 总的
inline RunInfo* addr_to_run_i(void *addr) {
    PageInfo *page_i = addr_to_page_i(addr);
    return &page_i_to_run_page_i(page_i)->small.run_i;
}

#ifdef inline_expand_M
#undef inline
#endif

} // namespace
