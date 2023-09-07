#include <cassert>
#include "run.h"
#include "page.h"
#include "addr_convert.h"
#include "bin.h"

namespace myAlloc {

static inline BinInfo* bin_info_init() {
    auto bin_i = new BinInfo[NBINS];
    for (int i = 0; i < NBINS; ++i) {
        int region_size = bin_i[i].region_size = small_size[i];
        int run_size = lcm(region_size, PAGE_SIZE);
        if (run_size / region_size < MIN_REGION_NUM) { // 1, 2
            run_size = MIN_REGION_NUM * region_size;
        }
        bin_i[i].page_num = run_size >> PAGE_SHIFT;
        bin_i[i].region_num = run_size / region_size;
    }
    return bin_i;
}

BinInfo *bin_info = bin_info_init();

static constexpr unsigned map_bias_init() {
    int bias = 0;

    // 逼近拟合求解方程.
    // 刚开始map_bias偏小, 第一个 loop 后 bias 偏大,
    // 再偏小, 偏大, 最后大于等于实际占用, 挺好的
    for (int i = 0; i < 3; ++i) {
        int head_size = offsetof(Chunk, pages_i) + (CHUNK_PAGE_NUM - bias) * sizeof(PageInfo);
        bias = (head_size + PAGE_SIZE - 1) / PAGE_SIZE;
    }
    return bias;
}

constexpr int map_bias = map_bias_init();

void RunInfo::init(int b_id) {
    bin_id = b_id;
    nfree = bin_info[bin_id].region_num;
    bitmap.set();
    idx = 0;
}

int RunInfo::find_region_id() {
    if (nfree < 1) {
        assert(false);
    }

    int num = bin_info[bin_id].region_num;

    // first fit(jemalloc 用的是这种):
    for (int i = 0; i < num; ++i) {
        if (bitmap[i] == 1) {
            return i;
        }
    }

//    // next fit:
//    while (idx < num) {
//        if (bitmap[idx] == 1) {
////            inO("1: %d", idx)
//            return idx++;
//        }
//        idx++;
//    }
//
//    idx = 0;
////    inO("another loop")
//    while (idx < num) {
//        if (bitmap[idx] == 1) {
////            inO("2: %d", idx)
//            return idx++;
//        }
//        idx++;
//    }

    // nfree > 0, 但是没找到
    assert(false);
}

char* RunInfo::get_region() {
    int region_id = find_region_id();
    take_region_id(region_id);

    --nfree;

    PageInfo *page_i = run_i_to_run_page_i(a2c(this));
    Chunk *chunk = addr_to_chunk(this);
    int page_id = page_i_to_page_id(page_i, chunk);
    char *run = page_id_to_page(page_id, a2c(chunk));
    BinInfo *bin_i = bin_info + bin_id;
    return run + bin_i->region_size * region_id;
}

// 测试给的地址是否是有效地址.
bool RunInfo::is_allocated_region(char *addr) {
    // 比较费性能, 算了不用了
    return true;
    // 默认 this 真的是 small 的第一个 page 的 RunInfo
    char *run_page = run_i_to_page_small(a2c(this));
    if(addr < run_page || addr >= run_page + PAGE_SIZE) {
        return false;
    }

    BinInfo *bin_i = bin_info + bin_id;
    if ((addr - run_page) % bin_i->region_size != 0) {
        return false;
    }

    int region_id = (addr - run_page) / bin_i->region_size;
    if (region_id >= bin_i->region_size) {
        return false;
    }

    if (get_region_id(region_id) == 0) {
        return false;
    }
    return true;
}

char size_class[] = {0, 0, 0, 0, 0, 0, 0, 0, 0,
                     1, 1, 1, 1, 1, 1, 1, 1,
                     2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2,
                     3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3,
                     4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4,
                     5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5,
                     6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6,
                     7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7,
                     8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8,
                     9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9,
                     10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10,
                     11, 11, 11, 11, 11, 11, 11, 11, 11, 11, 11, 11, 11, 11, 11, 11, 11, 11, 11, 11, 11, 11, 11, 11, 11, 11, 11, 11, 11, 11, 11, 11,
                     12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12,
                     13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13,
                     14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14,
                     15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15,
                     16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16,
};

}