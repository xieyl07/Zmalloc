#include <thread>
#include <vector>
#include <cstdlib>
#include <sys/time.h>
#include <cstdio>
#include <functional>
#include <cstring>
#include "myalloc.h"

using namespace std;

enum release_type{FULL, RANDOM};
enum size_type{FIX_SMALL, FIX_MID, FIX_LARGE, RAN_SMALL, RAN_MID};

// 要调的
constexpr int thread_num = 1000;
constexpr int inner_loop = 1000;
constexpr int outer_loop = 100;
constexpr size_type size_switch = RAN_MID;
constexpr bool enable_memset = true;
constexpr release_type release_type_switch = RANDOM;
// #define use_next_fit 在 run.h里


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
double *durations = new double[thread_num];

void thread_func(void *v[], int tid, void*(*alloc_f)(size_t), void(*free_f)(void*)) {

    timeval start, end;
    long seconds, microseconds;
    double duration;
    int cnt = 0;
    int sz;

    gettimeofday(&start, nullptr);

    for (int i = 0; i < outer_loop; ++i) {
        for (int j = 0; j < inner_loop; ++j) {
            switch (size_switch) {
            case FIX_SMALL:
                sz = 64;
                break;
            case FIX_MID:
            {
                sz = 1700;
                break;
            }
            case FIX_LARGE:
            {
                sz = 64 * 1024;
                break;
            }
            case RAN_SMALL:
            {
                sz = ran_a[cnt] % 1024 + 1; // 别忘 +1
                cnt = (cnt + 1) % ran_array_size;
                break;
            }
            case RAN_MID:
            {
                sz = (ran_a[cnt] % 1024 + 1) * 256; // [256, 256KB] 小部分 small 和 large
                cnt = (cnt + 1) % ran_array_size;
                break;
            }
            } // switch
            v[j] = alloc_f(sz);
            if (enable_memset && i == 1) memset(v[j], 0xff, sz);
        }

        switch (release_type_switch) {
        case FULL:
        {
            for (int j = 0; j < inner_loop; ++j) {
                free_f(v[j]);
            }
            break;
        }
        case RANDOM:
        {
            for (int j = 0; j < inner_loop; ++j) {
                int k = ran_a[cnt] % inner_loop;
                cnt = (cnt + 1) % ran_array_size;
                if (v[k]) {
                    free_f(v[k]);
                    v[k] = nullptr;
                }
            }
            break;
        }
        } // switch (release_type_switch)
    }

    gettimeofday(&end, nullptr);
    seconds = end.tv_sec - start.tv_sec;
    microseconds = end.tv_usec - start.tv_usec;
    durations[tid] = seconds + microseconds / 1000000.0;
}

void test(int type) {
    thread t[thread_num];
    void *v[thread_num][inner_loop];

    const char *s;
    void*(*alloc_f)(size_t);
    void(*free_f)(void*);

    if (type == 0) {
        s = "malloc";
        alloc_f = malloc;
        free_f = free;
    } else if (type == 1) {
        s = "my_alloc";
        alloc_f = myAlloc::myalloc;
        free_f = myAlloc::myfree;
    }
    assert(type != 2); // 本来 2 是留给另一个别人的内存池的


    for (int i = 0; i < thread_num; ++i) {
        t[i] = thread(bind(thread_func, (void **) v[i], i, alloc_f, free_f));
    }
    for (int i = 0; i < thread_num; ++i) {
        t[i].join();
    }

    double duration = 0;
    for (int i = 0; i < thread_num; ++i) {
        duration += durations[i];
    }
    printf("%-10s: %lfs per thread  \n", s, duration / thread_num);
}

int main(int argc, char*argv[]) {

    if (argc < 2) {
        cout << "no argument, 0: malloc, 1: myalloc, 2: mempool example\n";
        return 1;
    }
    if (strcmp(argv[1], "0") != 0 && strcmp(argv[1], "1") != 0 && strcmp(argv[1], "2") != 0) {
        cout << "argument error, 0: malloc, 1: myalloc, 2: mempool example\n";
        return 2;
    }
    test(argv[1][0]-'0');

    return 0;
}
