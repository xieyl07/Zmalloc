#define private public

#include "bin.h"
#include "run.cc"

using namespace myAlloc;

void test_bin_info() {
    for (int i = 0; i < NBINS; ++i) {
        printf("region_size: %d, page_num:%d, region_num:%d\n",
               bin_info[i].region_size, bin_info[i].page_num, bin_info[i].region_num);
    }
}

void test_get_nregion() {
    RunInfo run_i;
    int b_id = 3;
    run_i.init(b_id);
    run_i.bitmap.reset(2);
    void *res[3];
    int ret = run_i.get_nregion_id(res, 3);

    assert(res[0] == (void*)0);
    assert(res[1] == (void*)1);
    assert(res[2] == (void*)3);
    assert(ret == 3);

    run_i.nfree = 2;
    ret = run_i.get_nregion_id(res, 3);
    assert(ret == 2);

    cout << "get_nregion_id test passed\n";
}

int main() {
    test_bin_info();
    test_get_nregion();

    return 0;
}