#include "run.h"
#include "page.h"
#include "addr_convert.h"
#include "bin.h"

namespace myAlloc {

static BinInfo* bin_info_init() {
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
        int head_size = sizeof(Arena*) + sizeof(int) + (CHUNK_PAGE_NUM - bias) * sizeof(PageInfo);
        bias = (head_size + PAGE_SIZE - 1) / PAGE_SIZE;
    }
    return bias;
}

constexpr int map_bias = map_bias_init();

// free 来 后 对对应的管理单元 init
void RunInfo::init(int b_id) {
    bin_id = b_id;
    nfree = bin_info[bin_id].region_num;
    memset(alloc_map, 0, BITMAP_GROUPS_MAX * sizeof(bitmap_t));
}

int RunInfo::find_region_id_set() {
    // ###检查了 nfree, 看看逻辑, 调用的地方是不是要检查返回值
    if (nfree < 1) {
        deO("")
        return -1;
    }

    BinInfo * bin_i = bin_info + bin_id;
    int group_num = bin_i->region_num / BITS_NUM;
    int spare_bits = bin_i->region_num % BITS_NUM;
    bitmap_t map;

    for (int i = 0; i < group_num; ++i) {
        map = alloc_map[i];
        if (map != BITMAP_MAX) {
            int idx = find_first_zero(map);
            set_bit(alloc_map[i], idx);
            return BITS_NUM * i + idx;
        }
    }

    if (spare_bits != 0 &&
        (map = alloc_map[group_num]) != (1U << (spare_bits + 1)) - 1)
    {
        int idx = find_first_zero(map);
        set_bit(alloc_map[group_num], idx);
        return BITS_NUM * group_num + idx;
    }

    return -1;
}

char* RunInfo::get_region() {
    int region_id = find_region_id_set();
    if (region_id == -1) {
        deO("region_id == -1, exit")
        exit(1);
    }
    --nfree;

    PageInfo *page_i = run_i_to_run_page_i(a2c(this));
    Chunk *chunk = addr_to_chunk(this);
    int page_id = page_i_to_page_id(page_i, chunk);
    char *run = page_id_to_page(page_id, a2c(chunk));
    BinInfo *bin_i = bin_info + bin_id;
    return run + bin_i->region_size * region_id;
}

bool RunInfo::is_alloced_region(char *addr) {
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

int RunInfo::get_region_id(int region_id) {
    bitmap_t map = alloc_map[region_id / BITS_NUM];
    int shift = region_id % BITS_NUM;
    return get_bit(map, shift);
}

void RunInfo::set_region_id(int region_id) {
    bitmap_t &map = alloc_map[region_id / BITS_NUM];
    int shift = region_id % BITS_NUM;
    set_bit(map, shift);
}

void RunInfo::unset_region_id(int region_id) {
    bitmap_t &map = alloc_map[region_id / BITS_NUM];
    int shift = region_id % BITS_NUM;
    unset_bit(map, shift);
}

}