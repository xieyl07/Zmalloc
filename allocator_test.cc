#include <vector>
#include "allocator.h"

using namespace std;

int main() {
//    for (int i = 0; i < 1000; ++i) {
//        v.push_back(i);
//    }
//    v.resize(2000);
//    while (!v.empty()) {
//        v.pop_back();
//    }
    struct s {
        explicit s(int num) {
            i = num;
            str[i] = 'a' + i;
        }
        int i;
        double d;
        char str[100];
    };
    vector<s, myAlloc::Allocator<int>> v;
    for (int i = 0; i < 20; ++i) {
        v.push_back(s(i));
    }
    cout << v.back().i << "|" << v.back().str[v.back().i];
    while (v.empty()) {
        v.pop_back();
    }
    return 0;
}
