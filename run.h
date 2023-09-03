#pragma once

#include "misc.h"
#include "debug.h"
#include "tuning.h"

namespace myAlloc {

class RunInfo : NoCopy {
 public:
    int bin_id;
    int nfree; // free region number
    // used: 1, unused: 0. alloc_map[0] 从低位到高位

    void init(int b_id);
    char *get_region();
    bool is_alloced_region(char *addr);
    // 下面这三个都没有确认 region_id 有效!
    int get_region_id(int region_id);
    void set_region_id(int region_id);
    void unset_region_id(int region_id);

 private:
    int find_region_id_set();

    bitmap_t alloc_map[BITMAP_GROUPS_MAX];
};

} // namespace
