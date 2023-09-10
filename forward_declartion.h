#pragma once

namespace myAlloc {

struct Chunk;
struct PageInfo;
struct Bin;
struct BinInfo;
extern const int NBINS;
extern const BinInfo *bin_info;
extern const int map_bias;
class Arena;
class TCache;
class RunInfo;

} // namespace