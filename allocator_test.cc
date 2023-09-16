#include <vector>
#include <sys/time.h>
#include <thread>
#include <utility>
#include "allocator.h"

using namespace std;

int inner_loop = 500;
int outer_loop = 1000;

constexpr int ran_array_size = 1024 * 1024 * 8;

// 不让生成随机数成为性能影响最大的地方. 不然看不出各个分配器之间的区别
u_int16_t *get_random_array() {
    u_int16_t *ret = new u_int16_t[ran_array_size];
    for (int i = 0; i < ran_array_size; ++i) {
        ret[i] = random();
    }
    return ret;
}

const u_int16_t *ran_a = get_random_array();

void std_time_test() {
    for (int j = 0; j < outer_loop; ++j) {
        int cnt = 0;
//        vector<vector<int>> vv(inner_loop); // test 2
        vector<int> v; // test 1
        for (int i = 0; i < inner_loop; ++i) {
            v.push_back(i); // test 1
//            vv[i] = vector<int>((ran_a[cnt] % 1024 + 1)); // test 2
            if (++cnt >= ran_array_size) cnt = 0;
        }
    }
}

void myalloc_time_test() {
    for (int j = 0; j < outer_loop; ++j) {
        int cnt = 0;
//        vector<vector<int>> vv(inner_loop);
        vector<int> v; // test 1
        for (int i = 0; i < inner_loop; ++i) {
            v.push_back(i); // test 1
//            vv[i] = vector<int>((ran_a[cnt] % 1024 + 1));
            if (++cnt >= ran_array_size) cnt = 0;
        }
    }
}

void malloc_time_test() {
    for (int j = 0; j < outer_loop; ++j) {
        int cnt = 0;
//        vector<vector<int>> vv(inner_loop);
        vector<int> v; // test 1
        for (int i = 0; i < inner_loop; ++i) {
            v.push_back(i); // test 1
//            vv[i] = vector<int>((ran_a[cnt] % 1024 + 1));
            if (++cnt >= ran_array_size) cnt = 0;
        }
    }
}

int main() {
    int n = 8;
    thread t[n];
    struct timeval start, end;
    long seconds, microseconds;
    double duration;

    // 1
    gettimeofday(&start, NULL);
    for (int i = 0; i < n; ++i) {
        t[i] = thread(std_time_test);
    }
    for (int i = 0; i < n; ++i) {
        t[i].join();
    }
    gettimeofday(&end, NULL);
    seconds = end.tv_sec - start.tv_sec;
    microseconds = end.tv_usec - start.tv_usec;
    duration = seconds + microseconds / 1000000.0;
    printf("std::allocator: %lfs\n", duration);

    // 2
    gettimeofday(&start, NULL);
    for (int i = 0; i < n; ++i) {
        t[i] = thread(myalloc_time_test);
    }
    for (int i = 0; i < n; ++i) {
        t[i].join();
    }
    gettimeofday(&end, NULL);
    seconds = end.tv_sec - start.tv_sec;
    microseconds = end.tv_usec - start.tv_usec;
    duration = seconds + microseconds / 1000000.0;
    printf("      my_alloc: %lfs\n", duration);

    // 3
    gettimeofday(&start, NULL);
    for (int i = 0; i < n; ++i) {
        t[i] = thread(malloc_time_test);
    }
    for (int i = 0; i < n; ++i) {
        t[i].join();
    }
    gettimeofday(&end, NULL);
    seconds = end.tv_sec - start.tv_sec;
    microseconds = end.tv_usec - start.tv_usec;
    duration = seconds + microseconds / 1000000.0;
    printf("        malloc: %lfs\n\n", duration);

    return 0;
}
