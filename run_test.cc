#include "bin.h"
#include "run.cc"

using namespace myAlloc;
int main() {
    for (int i = 0; i < NBINS; ++i) {
        printf("region_size: %d, page_num:%d, region_num:%d\n",
               bin_info[i].region_size, bin_info[i].page_num, bin_info[i].region_num);
    }
}