#include "bitmap.h"

u_int16_t *get_random_array() {
    u_int16_t *ret = new u_int16_t[10000000];
    for (int i = 0; i < 10000000; ++i) {
        ret[i] = random();
    }
    return ret;
}

const u_int16_t *ran_a = get_random_array();

static void simple_test() {
    bitmap32<4> b32;
    bitmap64<2> b64;
    bitset<128> bstd;
    bstd.set();
    for (int i = 0; i < 128; ++i) {
        int ret = b32.first_find();
        assert(ret == i);
    }
    b32.init();
    for (int i = 0; i < 128; ++i) {
        int ret = b32.next_find();
        assert(ret == i);
    }
    b32.set_one(70);
    assert(b32.first_find() == 70);
    b32.init();
    b32.set_zero(0);
    assert(b32.first_find() == 1);
    cout << "--------b32 test finish----------\n";
    for (int i = 0; i < 128; ++i) {
        int ret = b64.first_find();
        assert(ret == i);
    }
    b64.init();
    for (int i = 0; i < 128; ++i) {
        int ret = b64.next_find();
        assert(ret == i);
    }
    b64.set_one(70);
    assert(b64.first_find() == 70);
    b64.init();
    b64.set_zero(0);
    assert(b64.first_find() == 1);
    cout << "--------b64 test finish----------\n";
    for (int i = 0; i < 128; ++i) {
        int ret = bitset_first_find<128>(bstd);
        assert(ret == i);
    }
    bstd.set();
    for (int i = 0; i < 128; ++i) {
        int ret = bitset_next_find<128>(bstd);
        assert(ret == i);
    }
    cout << "--------bstd test finish----------\n";
}

// 结论是, 极端数据(按顺序拿)的情况下, 对于first fit, bitset 比 ffs 慢了超多, 有十多倍
// ffs和ffsll几乎相等, ffsll略快
// 对于极端数据下的next fit, ffsll真的可以当做一点没快,
// ffs比之前稍微快了一点, 从比不过ffsll到稍稍稍微有优势. bitset那个纯作弊,
// 从first fit的9s到了0.27s, 还不如直接return ++idx呢, 这样是0.15s
static void bench_linear() {
    timeval start, end;
    long seconds, microseconds;
    double b32_time = 0, b64_time = 0, bstd_time = 0;

    bitmap32<16> b32;
    bitmap64<8> b64;
    bitset<512> bstd;
    bstd.set();

    constexpr int N = 1000000; // 加了个0, 有点大了其实

    for (int j = 0; j < N; ++j) {

        // ffs
        gettimeofday(&start, nullptr);
        for (int i = 0; i < 512; ++i) {
            b32.next_find();
        }
        gettimeofday(&end, nullptr);
        seconds = end.tv_sec - start.tv_sec;
        microseconds = end.tv_usec - start.tv_usec;
        b32_time += seconds + microseconds / 1000000.0;
        b32.init();

        // ffsll
        gettimeofday(&start, nullptr);
        for (int i = 0; i < 512; ++i) {
            b64.next_find();
        }
        gettimeofday(&end, nullptr);
        seconds = end.tv_sec - start.tv_sec;
        microseconds = end.tv_usec - start.tv_usec;
        b64_time += seconds + microseconds / 1000000.0;
        b64.init();

        // bitset
        gettimeofday(&start, nullptr);
        for (int i = 0; i < 512; ++i) {
            bitset_next_find<512>(bstd);
        }
        gettimeofday(&end, nullptr);
        seconds = end.tv_sec - start.tv_sec;
        microseconds = end.tv_usec - start.tv_usec;
        bstd_time += seconds + microseconds / 1000000.0;
        bstd.set();
        idx = -1;

        if (N > 100000 && (j + 1) % (N / 10) == 0) {
            printf("loading... %d0%%\n", (j + 1) / (N / 10));
        }
    }
    printf("b32: %lf\n", b32_time);
    printf("b64: %lf\n", b64_time);
    printf("bitset: %lf\n", bstd_time);
}

static void bench_random() {
    timeval start, end;
    long seconds, microseconds;
    double b32_time = 0, b64_time = 0, bstd_time = 0;
    int ran_a_idx = 0;

    bitmap32<16> b32;
    bitmap64<8> b64;
    bitset<512> bstd;
    bstd.set();

    constexpr int N = 100000;

//    // ffs
//    ran_a_idx = 0;
//    for (int j = 0; j < N; ++j) {
//
//        gettimeofday(&start, nullptr);
//        // 随机拿一点
//        for (int k = 0; k < 512 / 3; ++k) {
//            b32.set_zero(ran_a[ran_a_idx++] % 512);
//            ran_a_idx %= 10000000;
//        }
//        // 找 二分之一个
//        for (int i = 0; i < 512 / 2; ++i) {
//            b32.first_find();
//        }
//        // 再随机还一点
//        for (int k = 0; k < 512 / 2; ++k) {
//            b32.set_one(ran_a[ran_a_idx++] % 512);
//            ran_a_idx %= 10000000;
//        }
//        // 拿完
//        while (b32.first_find() != -1) {
//            ;
//        }
//        gettimeofday(&end, nullptr);
//        seconds = end.tv_sec - start.tv_sec;
//        microseconds = end.tv_usec - start.tv_usec;
//        b32_time += seconds + microseconds / 1000000.0;
//        b32.init();
//    }
//
//    ran_a_idx = 0;
//    // ffsll
//    for (int j = 0; j < N; ++j) {
//
//        gettimeofday(&start, nullptr);
//        // 随机拿一点
//        for (int k = 0; k < 512 / 3; ++k) {
//            b64.set_zero(ran_a[ran_a_idx++] % 512);
//            ran_a_idx %= 10000000;
//        }
//        // 找 二分之一个
//        for (int i = 0; i < 512 / 2; ++i) {
//            b64.first_find();
//        }
//        // 再随机还一点
//        for (int k = 0; k < 512 / 2; ++k) {
//            b64.set_one(ran_a[ran_a_idx++] % 512);
//            ran_a_idx %= 10000000;
//        }
//        // 拿完
//        while (b64.first_find() != -1) {
//            ;
//        }
//        gettimeofday(&end, nullptr);
//        seconds = end.tv_sec - start.tv_sec;
//        microseconds = end.tv_usec - start.tv_usec;
//        b64_time += seconds + microseconds / 1000000.0;
//        b64.init();
//    }

    ran_a_idx = 0;
    // bitset
    for (int j = 0; j < N; ++j) {

        gettimeofday(&start, nullptr);
        // 随机拿一点
        for (int k = 0; k < 512 / 3; ++k) {
            bstd.reset(ran_a[ran_a_idx++] % 512);
            if (++ran_a_idx == 10000000) ran_a_idx = 0;
        }
        // 找 二分之一个
        for (int i = 0; i < 512 / 2; ++i) {
            bitset_first_find<512>(bstd);
        }
        // 再随机还一点
        for (int k = 0; k < 512 / 2; ++k) {
            bstd.set(ran_a[ran_a_idx++] % 512);
            if (++ran_a_idx == 10000000) ran_a_idx = 0;
        }
        // 拿完
        while (bitset_next_find<512>(bstd) != -1) {
            ;
        }
        gettimeofday(&end, nullptr);
        seconds = end.tv_sec - start.tv_sec;
        microseconds = end.tv_usec - start.tv_usec;
        bstd_time += seconds + microseconds / 1000000.0;
        bstd.set();
    }

    printf("b32: %lf\n", b32_time);
    printf("b64: %lf\n", b64_time);
    printf("bitset: %lf\n", bstd_time);
}

int main() {

    simple_test();
//    bench_linear();
    bench_random();

    return 0;
}
