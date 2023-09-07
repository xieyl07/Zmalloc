#define r_no_d

#include <vector>
#include <sys/time.h>
#include <thread>
#include <utility>
#include "allocator.h"

using namespace std;

void std_time_test() {
    for (int j = 0; j < 100000; ++j) {
        vector<int> v;
        for (int i = 0; i < 1000; ++i) {
            v.push_back(0);
        }
        while (!v.empty()) v.pop_back();
    }
}

void myalloc_time_test() {
    for (int j = 0; j < 100000; ++j) {
        vector<int, myAlloc::Allocator<int>> v;
        for (int i = 0; i < 1000; ++i) {
            v.push_back(0);
        }
        while (!v.empty()) v.pop_back();
    }
}

void malloc_time_test() {
    for (int j = 0; j < 100000; ++j) {
    vector<int, myAlloc::AllocatorWARP<int>> v;
        for (int i = 0; i < 1000; ++i) {
            v.push_back(0);
        }
        while (!v.empty()) v.pop_back();
    }
}

int main() {
    int n = 1;
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
    printf("std::allocator 执行时间: %f 秒\n", duration);

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
    printf("myalloc 执行时间: %f 秒\n", duration);

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
    printf("malloc 执行时间: %f 秒\n", duration);

    return 0;
}
