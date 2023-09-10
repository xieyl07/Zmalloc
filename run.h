#pragma once

#include <bitset>
#include "misc.h"
#include "debug.h"
#include "tuning.h"

namespace myAlloc {

class RunInfo : NoCopy {
 public:
    int bin_id;
    int nfree; // free region number

    void init(int b_id);
    char *get_region();
    int get_nregion_id(void **target, int num);
    bool is_allocated_region(char *addr);
    // 下面这三个都没有确认 region_id 有效
    void fetch_region(int region_id) {
//        inO("count before: %d", bitmap.count())
        bitmap.set(region_id);
//        inO("count after: %d", bitmap.count())
    }

 private:
    int get_region_id();

//    // used: 0, unused: 1, 为了能用 ffs. alloc_map[0] 从低位到高位
//    bitmap_t alloc_map[BITMAP_GROUPS_MAX];
    std::bitset<BITMAP_NUM_MAX> bitmap;
};

} // namespace
