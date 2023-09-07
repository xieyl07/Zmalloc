#include <thread>
#include <vector>
#include <cstdlib>
#include <sys/time.h>
#include <cstdio>
#include <functional>
#include <cstring>
#include "myalloc.h"
#include "mempool_example.h"

using namespace std;

int thread_num = 2;
int inner_loop = 10000;
int outer_loop = 10000;

u_int16_t *get_random_array() {
    int limit = 10000 * 10000 * 1;
    u_int16_t *ret = new u_int16_t[limit];
    for (int i = 0; i < limit; ++i) {
        ret[i] = random();
    }
    return ret;
}

const u_int16_t *ran_a = get_random_array();

void thread_func(void *v[], void*(*alloc_f)(size_t), void(*free_f)(void*)) {
    int cnt = 0;
    for (int i = 0; i < outer_loop; ++i) {
        for (int j = 0; j < inner_loop; ++j) {
//            int sz = (ran_a[cnt++] % 1024 + 1) * 256; // 小部分 small 和 large, KB级
            int sz = ran_a[cnt++] % 1024 + 1; // 别忘 +1
            v[j] = alloc_f(sz);
            memset(v[j], 0, sz);
        }

        for (int j = 0; j < inner_loop; ++j) {
            free_f(v[j]);
        }

//        for (int j = 0; j < inner_loop; ++j) {
//            int k = ran_a[cnt++] % inner_loop;
//            if (v[k]) {
//                free_f(v[k]);
//                v[k] = nullptr;
//            }
//        }
    }
}

void test(int type) {
    thread t[thread_num];
    void *v[thread_num][inner_loop];
    struct timeval start, end;
    long seconds, microseconds;
    double duration;

    const char *s;
    void*(*alloc_f)(size_t);
    void(*free_f)(void*);

    if (type == 0) {
        s = "malloc";
        alloc_f = malloc;
        free_f = free;
    } else if (type == 1) {
        s = "myalloc";
        alloc_f = myAlloc::myalloc;
        free_f = myAlloc::myfree;
    } else if (type == 2) {
        s = "mempool example";
        alloc_f = pool_alloc;
        free_f = pool_free;
    }


    gettimeofday(&start, NULL);
    for (int i = 0; i < thread_num; ++i) {
        t[i] = thread(std::bind(thread_func, (void **) v[i], alloc_f, free_f));
    }
    for (int i = 0; i < thread_num; ++i) {
        t[i].join();
    }
    gettimeofday(&end, NULL);
    seconds = end.tv_sec - start.tv_sec;
    microseconds = end.tv_usec - start.tv_usec;
    duration = seconds + microseconds / 1000000.0;
    printf("%s 执行时间: %f 秒\n", s, duration);
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
