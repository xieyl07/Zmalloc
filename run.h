#pragma once

#include <bitset>
#include "misc.h"
#include "debug.h"
#include "tuning.h"

//#define use_next_fit

namespace myAlloc {

class RunInfo : NoCopy {
 public:
    int bin_id;
    int nfree; // free region number
    std::bitset<BITMAP_NUM_MAX> bitmap;

    void init(int b_id);
    char *get_region();
    bool is_allocated_region(char *addr);
    // 下面这三个都没有确认 region_id 有效
    void fetch_region_id(int region_id) {
//        inO("count before: %d", bitmap.count())
        bitmap.set(region_id);
//        inO("count after: %d", bitmap.count())
    }

 private:
#ifdef use_next_fit
    int idx = -1; // next fit
#endif
    int find_region_id();
    // 查看状况
    int get_region_id(int region_id) {
        return bitmap[region_id];
    }
    // 取出 region_id 的内容
    void take_region_id(int region_id) {
        bitmap.reset(region_id);
    }

    // used: 0, unused: 1, 为了能用 ffs. alloc_map[0] 从低位到高位
//    bitmap_t alloc_map[BITMAP_GROUPS_MAX];
};

} // namespace
