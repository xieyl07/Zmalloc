#define r_no_d

#include <vector>
#include <sys/time.h>
#include "allocator.h"

using namespace std;

int main() {
//    for (int i = 0; i < 1000; ++i) {
//        vm.push_back(i);
//    }
//    vm.resize(2000);
//    while (!vm.empty()) {
//        vm.pop_back();
////    }
//    struct s {
//        explicit s(int num) {
//            i = num;
//            str[i] = 'a' + i;
//        }
//        int i;
//        double d;
//        char str[100];
//    };
    vector<int, myAlloc::Allocator<int>> vm;
    vector<int> vo;
    struct timeval start, end;
    gettimeofday(&start, NULL);

        for (int j = 0; j < 1000; ++j) {
            for (int i = 0; i < 1000; ++i) {
                vo.push_back(i * j);
            }
        }

    gettimeofday(&end, NULL);
    long seconds = end.tv_sec - start.tv_sec;
    long microseconds = end.tv_usec - start.tv_usec;
    double duration = seconds + microseconds / 1000000.0;

    printf("程序执行时间: %f 秒\n", duration);

    gettimeofday(&start, NULL);

    for (int j = 0; j < 1000; ++j) {
        for (int i = 0; i < 1000; ++i) {
            int k = i * j;
            vm.push_back(k);
        }
    }

    gettimeofday(&end, NULL);
    seconds = end.tv_sec - start.tv_sec;
    microseconds = end.tv_usec - start.tv_usec;
    duration = seconds + microseconds / 1000000.0;

    printf("程序执行时间: %f 秒\n", duration);

    return 0;
}
