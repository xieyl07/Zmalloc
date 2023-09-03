#pragma once

#include <cstdlib>
#include "typedef.h"

namespace myAlloc {

// 这些大小的宏括号都没有, 别拿到算式里
#define KiB * (1<<10)
#define KB KiB
#define MiB * (1<<20)
#define MB MiB

// 最高位是 2 的几次
constexpr int get_equal_shift(unsigned num) {
    int cnt = 0;
    while (num > 1) {
        num >>= 1;
        ++cnt;
    }
    return cnt;
}

constexpr int PAGE_SIZE = 4 KiB;
constexpr int CHUNK_SIZE = 2 MiB;
constexpr int CHUNK_PAGE_NUM = CHUNK_SIZE / PAGE_SIZE;
constexpr int PAGE_SHIFT = get_equal_shift(PAGE_SIZE); // x/PAGE_SIZE == (x>>PAGE_SHIFT)
constexpr int CHUNK_SHIFT = get_equal_shift(CHUNK_SIZE);
constexpr p_t PAGE_MASK = ~(PAGE_SIZE - 1); // SIZE 是 int 所以可以赋值后高位补 1. ~(SIZE - 1) 是 int 负数, 所以赋值给无符号数高位补 1
constexpr p_t CHUNK_MASK = ~(CHUNK_SIZE - 1); // 别搞成 int 了! 要和 8 字节地址操作的
constexpr int small_size[] = {8, // lu 和 huge 的档位有什么用?
                              16, 32, 48, 64, 80, 96, 112, 128,
                              160, 192, 224, 256,
                              320, 384, 448, 512,
                              640, 768, 896, 1024,
                              1280, 1536, 1792, 2048,
                              2560, 3072, 3584, 4096,
                              5 KB, 6 KB, 7 KB, 8 KB,
                              10 KB, 12 KB, 14 KB
};
constexpr int NBINS = sizeof(small_size) / sizeof(int); // 只有 small 去 bin?
constexpr int MIN_REGION_NUM = 4;
constexpr int BITS_NUM = 8 * sizeof(bitmap_t);
constexpr int BITMAP_GROUPS_MAX = PAGE_SIZE / small_size[0] / 8;
constexpr int SMALL_SIZE_MAX = small_size[NBINS - 1];
constexpr int LARGE_SIZE_MAX = 1792 KiB; // < CHUNK_SIZE
constexpr int NCACHED_MAX = 16;
constexpr int TCACHE_GC_INCR = 50; // 多少次事件(alloc, free)后tcache执行GC. 待更改###
constexpr int CACHE_LINE_SIZE = 64;
constexpr int CACHE_LINE_SHIFT = get_equal_shift(CACHE_LINE_SIZE);

} // namespace
