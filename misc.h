#pragma once

#include <ctime>
#include <cstring>
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

// 要么 inline 要么 static
// 为了看看哪些函数影响大, 把 inline 去掉. 记得注释掉###
#define inline static

// 最大公因数
inline int gcd(int a, int b) {
    int temp;
    while (b != 0) {
        temp = a % b;
        a = b;
        b = temp;
    }
    return a;
}

// 最小公倍数
inline int lcm(const int a, const int b) {
    return b * a / gcd(a, b);
}

// 默认调用是正确的, 因为 run_i 调用的, 调用前先检查 nfree.
inline int find_first_non_zero(bitmap_t map) {

    return ffs((int64_t )map) - 1;
}

inline int get_bit(bitmap_t map, int shift) {
    // 别对有符号数右移, 高位可能会补1!
    return map >> shift & 1;
}

// 就不检查了
inline void set_bit(bitmap_t &map, int shift) {
    map = map | 1U << shift;
}

// 就不检查了
inline void unset_bit(bitmap_t &map, int shift) {
    map = map & ~(1U << shift);
}

inline int get_random_high_quailty(int limit) {
    // 使用 random_device 作为种子
    std::random_device rd;
    std::mt19937 generator(rd());

    // 定义随机数分布范围为0到63
    std::uniform_int_distribution<int> distribution(0, limit);

    // 生成随机数
    return distribution(generator);
}

inline int get_random(int limit) {
//    srand(time(NULL)); // uncommented for test
    return rand() % (limit + 1);
}

#undef inline

} // namespace
