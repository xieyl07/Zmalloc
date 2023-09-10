#pragma once

#include <cassert>
#include <set>
#include "misc.h"
#include "tuning.h"
#include "pairing_heap.h"
#include "lock.h"
#include "forward_declartion.h"

namespace myAlloc {

struct Bin : NoCopy {
    RunInfo *cur_run;
    // non-full free runs, 按 RunInfo 的地址排序,
    // RunInfo 在 PageInfo 里面, 也就是按照起始 page 的地址排序
    set<RunInfo*> nonfull_runs;
    SpinLock lock;
};

struct BinInfo : NoCopy {
    int region_size;
    int page_num;
    int region_num;
};

extern const BinInfo* bin_info; // 偷懒放到 run.cpp 里调用bin_info_init初始化他

extern char size_class[]; // 偷懒放到 run.cpp

// small 才用 bin
inline int get_bin_id(int size) {
    int bin_id = -1;
//    if (size <= 8) {
//        bin_id = 0;
//    } else if (size <= 128) {
//        bin_id = (size + 16 - 1) / 16;
//    } else if (size <= 256) {
//        bin_id = (size + 32 - 1) / 32 + 4;
//    } else if (size <= 512) {
//        bin_id = (size + 64 - 1) / 64 + 8;
//    } else if (size <= 1024) {
//        bin_id = (size + 128 - 1) / 128 + 12;
//    } else if (size <= 2048) {
//        bin_id = (size + 256 - 1) / 256 + 16;
//    } else if (size <= 4096) {
//        bin_id = (size + 512 - 1) / 512 + 20;
//    } else if (size <= 8 KiB) {
//        bin_id = (size + 1 KiB - 1) / (1 KiB) + 24; // 括号! 不然成了 / 1 * 1024
//    } else if (size <= SMALL_SIZE_MAX) {
//        bin_id = (size + 2 KB -1) / (2 KB) + 28;
//    } else {
//        assert(false);
//        exit(1);
//    }
    if (size <= 512) {
        bin_id = size_class[size];
    } else if (size <= 1024) {
        bin_id = (size + 128 - 1) / 128 + 12;
    } else if (size <= 2048) {
        bin_id = (size + 256 - 1) / 256 + 16;
    } else if (size <= 4096) {
        bin_id = (size + 512 - 1) / 512 + 20;
    } else if (size <= SMALL_SIZE_MAX) {
        bin_id = (size + 1 KiB - 1) / (1 KiB) + 24; // 括号! 不然成了 / 1 * 1024
    } else {
        assert(false);
        exit(1);
    }

    return bin_id;
}


} // namespace
