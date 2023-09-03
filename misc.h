#pragma once

#include <random>
#include "typedef.h"
#include "tuning.h"

namespace myAlloc {

// the same as offsetof in stddef.h
#define struct_member_offset(struct_type, member_name) \
    (u_int64_t)(&((struct_type*)nullptr)->member_name)

class NoCopy {
 public:
    NoCopy() = default;
    NoCopy(const NoCopy&) = delete;
    NoCopy& operator=(const NoCopy&) = delete;
};

// all to char*
template<class T>
inline char* a2c(T p) {
    return reinterpret_cast<char*>(p);
}

// 最大公因数
static int gcd(int a, int b) {
    int temp;
    while (b != 0) {
        temp = a % b;
        a = b;
        b = temp;
    }
    return a;
}

// 最小公倍数
static int lcm(const int a, const int b) {
    return b * a / gcd(a, b);
}

// 默认调用是正确的, 因为 run_i 调用的, 调用前先检查 nfree.
// 而且最后一个map可能不是满的, 有无效的0, run_i 里的逻辑也要改###
static int find_first_zero(bitmap_t map) {
    for (int j = 0; j < BITS_NUM; ++j, map >>= 1) {
        if ((map & 1) == 0) {
            return j;
        }
    }
    return -1;
}

static int get_bit(bitmap_t map, int shift) {
    // 别对有符号数右移, 高位可能会补1!
    return map >> shift & 1;
}

// 就不检查了
static void set_bit(bitmap_t &map, int shift) {
    map = map | 1U << shift;
}

// 就不检查了
static void unset_bit(bitmap_t &map, int shift) {
    map = map & ~(1U << shift);
}

static int get_random_offset(int limit) {
    // 使用 random_device 作为种子
    std::random_device rd;
    std::mt19937 generator(rd());

    // 定义随机数分布范围为0到63
    std::uniform_int_distribution<int> distribution(0, limit);

    // 生成随机数
    return distribution(generator);
}

} // namespace
