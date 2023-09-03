//#define release_no_debug

#include "pairing_heap.h"
#include "debug.h"

int main() {
    PairingHeap<int> ph;
    for (int i  = 0; i < 30; ++i) {
        ph.push(i);
    }
    deO("size: %d", ph.size())

    while (!ph.empty()) {
         deO("top: %d", ph.top());
        ph.pop();
    }

    deO("size: %d", ph.size())

    return 0;
}