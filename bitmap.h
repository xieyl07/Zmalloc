#pragma once

#include <cstdlib>
#include <cstring>
#include <cassert>
#include <sys/time.h>
#include <iostream>
#include <bitset>

using namespace std;

/*说明
 * 用bitset一定要开O2, 开不开差很多的
 * 每个find没找到都返回-1. 如果是判断有了再去找, 建议改成assert
 */

template <typename T>
constexpr int bitsof(T val) {
    return 8 * sizeof(T);
}

// 因为共用的代码太多了, 使用template模式重写了bitmap32和bitmap64
// 主要问题是用了纯虚函数接口, 有虚函数开销
template <unsigned N, typename T>
class bitmap_template {
 public:
    bitmap_template() {
        init();
    }
    int first_find() {
        for (int i = 0; i < N; ++i) {
            if (bmap[i] != 0) {
                return i * bitsof(bmap[0]) + find_and_set(i);
            }
        }
        return -1;
//        assert(false);
    }
    int next_find() {
        int tmp = idx;
        for (; idx < N; ++idx) {
            if (bmap[idx] != 0) {
                return idx * bitsof(bmap[0]) + find_and_set();
            }
        }
        for (idx = 0; idx < tmp; ++idx) {
            if (bmap[idx] != 0) {
                return idx * bitsof(bmap[0]) + find_and_set();
            }
        }
        return -1;
//        assert(false);
    }
    void init() {
        memset(bmap, 0xFF, sizeof(bmap));
        idx = 0;
        nfree = 8 * bitsof(bmap);
    }
    void set_zero(int i) {
        int ind = i / bitsof(bmap[0]);
        int offset = i % bitsof(bmap[0]);
        bmap[ind] = bmap[ind] & ~(1LL << offset);
    }
    void set_one(int i) {
        int ind = i / bitsof(bmap[0]);
        int offset = i % bitsof(bmap[0]);
        bmap[ind] = bmap[ind] | (1LL << offset);
    }

 protected:
    virtual int find_and_set(int i) = 0;
    virtual int find_and_set() = 0;

    T bmap[N];
    int nfree = 8 * bitsof(bmap);
    int idx = 0; // NEXT fit
};

template<int N>
class bitmap64_template : public bitmap_template<N, int64_t> {
 private:
    int find_and_set(int i) {
        int j = ffsll(bitmap_template<N, int64_t>::bmap[i]) - 1;
        bitmap_template<N, int64_t>::bmap[i] =
                bitmap_template<N, int64_t>::bmap[i] & ~(1LL << j);
        --bitmap_template<N, int64_t>::nfree;
        return j;
    }
    int find_and_set() {
        int j = ffsll(bitmap_template<N, int64_t>::bmap[bitmap_template<N, int64_t>::idx]) - 1;
        bitmap_template<N, int64_t>::bmap[bitmap_template<N, int64_t>::idx] =
                bitmap_template<N, int64_t>::bmap[bitmap_template<N, int64_t>::idx] & ~(1LL << j);
        --bitmap_template<N, int64_t>::nfree;
        return j;
        }
};

template<int N>
class bitmap32_template : public bitmap_template<N, int32_t> {
    using father_scope = bitmap_template<N, int32_t>;
 private:
    int find_and_set(int i) {
        int j = ffs(father_scope::bmap[i]) - 1;
        father_scope::bmap[i] =
                father_scope::bmap[i] & ~(1 << j);
        --father_scope::nfree;
        return j;
    }
    int find_and_set() {
        int j = ffs(father_scope::bmap[father_scope::idx]) - 1;
        father_scope::bmap[father_scope::idx] =
                father_scope::bmap[father_scope::idx] & ~(1 << j);
        --father_scope::nfree;
        return j;
    }
};

// 32和64的区别只有 int64_t map[N];变成了int32_t map[N];
// 1 << j 在 64上要写成 1LL, ffs改成ffsll. 主要是 1LL真的会错!

template <unsigned N>
class bitmap64 {
 public:
    bitmap64() {
        init();
    }
    int first_find() {
        for (int i = 0; i < N; ++i) {
            if (map[i] != 0) {
                return i * bitsof(map[0]) + find_and_set(i);
            }
        }
        return -1;
//        assert(false);
    }
    int next_find() {
        int tmp = idx;
        for (; idx < N; ++idx) {
            if (map[idx] != 0) {
                return idx * bitsof(map[0]) + find_and_set();
            }
        }
        for (idx = 0; idx < tmp; ++idx) {
            if (map[idx] != 0) {
                return idx * bitsof(map[0]) + find_and_set();
            }
        }
        return -1;
//        assert(false);
    }
    void init() {
        memset(map, 0xFF, sizeof(map));
        idx = 0;
        nfree = 8 * bitsof(map);
    }
    void set_zero(int i) {
        int ind = i / bitsof(map[0]);
        int offset = i % bitsof(map[0]);
        map[ind] = map[ind] & ~(1LL << offset);
    }
    void set_one(int i) {
        int ind = i / bitsof(map[0]);
        int offset = i % bitsof(map[0]);
        map[ind] = map[ind] | (1LL << offset);
    }

 private:
    int find_and_set(int i) {
        int j = ffsll(map[i]) - 1;
        map[i] = map[i] & ~(1LL << j);
        --nfree;
        return j;
    }
    int find_and_set() {
        int j = ffsll(map[idx]) - 1;
        map[idx] = map[idx] & ~(1LL << j);
        --nfree;
        return j;
    }

    int64_t map[N];
    int nfree = 8 * bitsof(map);
    int idx = 0; // NEXT fit
};


template <unsigned N>
class bitmap32 {
 public:
    bitmap32() {
        init();
    }
    int first_find() {
        for (int i = 0; i < N; ++i) {
            if (map[i] != 0) {
                return i * bitsof(map[0]) + find_and_set(i);
            }
        }
        return -1;
//        assert(false);
    }
    int next_find() {
        int tmp = idx;
        for (; idx < N; ++idx) {
            if (map[idx] != 0) {
                return idx * bitsof(map[0]) + find_and_set();
            }
        }
        for (idx = 0; idx < tmp; ++idx) {
            if (map[idx] != 0) {
                return idx * bitsof(map[0]) + find_and_set();
            }
        }
        return -1;
//        assert(false);
    }
    void init() {
        memset(map, 0xFF, sizeof(map));
        idx = 0;
        nfree = 8 * bitsof(map);
    }
    void set_zero(int i) {
        int ind = i / bitsof(map[0]);
        int offset = i % bitsof(map[0]);
        map[ind] = map[ind] & ~(1 << offset);
    }
    void set_one(int i) {
        int ind = i / bitsof(map[0]);
        int offset = i % bitsof(map[0]);
        map[ind] = map[ind] | (1 << offset);
    }

 private:
    int find_and_set(int i) {
        int j = ffs(map[i]) - 1;
        map[i] = map[i] & ~(1 << j);
        --nfree;
        return j;
    }
    int find_and_set() {
        int j = ffs(map[idx]) - 1;
        map[idx] = map[idx] & ~(1 << j);
        --nfree;
        return j;
    }

    int32_t map[N];
    int nfree = 8 * bitsof(map);
    int idx = 0; // NEXT fit
};

template <int N>
inline int bitset_first_find(bitset<N> &bstd) {
    for (int i = 0; i < N; ++i) {
        if (bstd[i] == 1) {
            bstd.reset(i);
            return i;
        }
    }
    return -1;
//    assert(false);
}

int bstd_idx = -1;
template <int N>
inline int bitset_next_find(bitset<N> &bstd) {
//    if (bstd_idx == N) bstd_idx = -1;
//    return ++bstd_idx; // 如果按顺序分配的话
    int tmp = bstd_idx;
    while (++bstd_idx < N) {
        if (bstd[bstd_idx] == 1) {
            bstd.reset(bstd_idx); // 原来是忘了reset了, 居然能过next find的case, 真厉害
            return bstd_idx;
        }
    }

    bstd_idx = -1;
    while (++bstd_idx < tmp) {
        if (bstd[bstd_idx] == 1) {
            bstd.reset(bstd_idx);
            return bstd_idx;
        }
    }
    return -1;
//    assert(false);
}
