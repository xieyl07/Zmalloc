#include <vector>
#include <sys/time.h>
#include <thread>
#include <cstdlib>
#include <cstdio>
#include <functional>
#include "misc.h"

using namespace std;

int inner_loop = 1000, outer_loop = 1000;

void malloc_time_test(void *v[]) {
    for (int i = 0; i < outer_loop; ++i) {
        for (int j = 0; j < inner_loop; ++j) {
            int sz = random() % 1024 + 1;
            v[j] = malloc(sz);
            memset(v[j], 0xFF, sz);
        }
        for (int j = 0; j < inner_loop; ++j) {
            free(v[j]);
        }
    }
}

int main() {
    int n = 6;
    thread t[n];
    void *v[n][inner_loop];
    struct timeval start, end;
    long seconds, microseconds;
    double duration;

    // 2
    gettimeofday(&start, NULL);
    for (int i = 0; i < n; ++i) {
        t[i] = thread(std::bind(malloc_time_test, (void**)v[i]));
    }
    for (int i = 0; i < n; ++i) {
        t[i].join();
    }
    gettimeofday(&end, NULL);
    seconds = end.tv_sec - start.tv_sec;
    microseconds = end.tv_usec - start.tv_usec;
    duration = seconds + microseconds / 1000000.0;
    printf("malloc  执行时间: %f 秒\n", duration);

    return 0;

}
