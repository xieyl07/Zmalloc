# 测试结果

波动较大, 仅供参考

### 结论是


###结论测完之后还要看还要改
*注. `memset`是因为有些情况下`pt`会有短的惊人的用时, 不清楚`pt`会这样原因. 测试的时候暂时关了吧.

目录
1. 基准
1. 大小的影响
1. `TCACHED_MAX`的影响
1. 随机下的测试
1. `first fit`和`next fit`对比
1. 线程数的影响
1. `inner_loop`和`outer_loop`数的影响

## 1. 基准

#### 1.1 分配固定大小(64字节), 按照分配顺序完全释放, 4线程, inner_loop = 30(TCACHED_MAX内), outer_loop = 100000, first_fit
> malloc    : 0.091774s per thread  
> my_alloc  : 0.062834s per thread  
> malloc    : 0.100022s per thread  
> my_alloc  : 0.066609s per thread  
> malloc    : 0.089587s per thread  
> my_alloc  : 0.068249s per thread  


## 2. 大小影响

*注. 发现ptmalloc2在分配1700字节时性能不佳

#### 2.1 分配随机大小(1-1024字节), 按照分配顺序完全释放, inner_loop = 30
malloc    : 0.061084s per thread  
my_alloc  : 0.082429s per thread  
malloc    : 0.077486s per thread  
my_alloc  : 0.077951s per thread  
malloc    : 0.066915s per thread  
my_alloc  : 0.080301s per thread  

#### 2.2 分配固定大小(1700字节), 按照分配顺序完全释放, inner_loop = 30
malloc    : 0.162452s per thread  
my_alloc  : 0.068985s per thread  
malloc    : 0.163255s per thread  
my_alloc  : 0.069169s per thread  
malloc    : 0.154430s per thread  
my_alloc  : 0.065085s per thread  

#### 2.3 分配随机大小(256B-256KB), 按照分配顺序完全释放, inner_loop = 30
malloc    : 3.738925s per thread  
my_alloc  : 2.230073s per thread  
malloc    : 3.857931s per thread  
my_alloc  : 2.268422s per thread  
malloc    : 3.850855s per thread  
my_alloc  : 2.252147s per thread  


#### 2.4 分配固定大小(64MiB), 按照分配顺序完全释放, inner_loop = 30
malloc    : 6.994040s per thread  
my_alloc  : 2.069994s per thread  
malloc    : 6.988520s per thread  
my_alloc  : 2.022938s per thread  
malloc    : 7.006715s per thread  
my_alloc  : 2.125248s per thread  


## 3. TCACHED_MAX的影响

#### 3.1 分配固定大小(64字节), 按照分配顺序完全释放, inner_loop = 100
malloc    : 0.260923s per thread  
my_alloc  : 0.417991s per thread  
malloc    : 0.265674s per thread  
my_alloc  : 0.417570s per thread  
malloc    : 0.276382s per thread  
my_alloc  : 0.421104s per thread  


#### .1 分配随机大小(1-1024字节), 按照分配顺序完全释放, inner_loop = 100
malloc    : 0.160264s per thread  
my_alloc  : 0.217643s per thread  
malloc    : 0.161851s per thread  
my_alloc  : 0.202134s per thread  
malloc    : 0.167909s per thread  
my_alloc  : 0.211136s per thread  


#### .1 分配固定大小(1700字节), 按照分配顺序完全释放, inner_loop = 100
malloc    : 4.360611s per thread  
my_alloc  : 0.575104s per thread  
malloc    : 3.568441s per thread  
my_alloc  : 0.553265s per thread  
malloc    : 3.605131s per thread  
my_alloc  : 0.629348s per thread  


## 4. 有碎片情况下的分配

#### 1.1 分配固定大小(64字节), 按照分配顺序完全释放, inner_loop = 30(TCACHED_MAX内)
> malloc    : 0.091774s per thread  
> my_alloc  : 0.062834s per thread  
> malloc    : 0.100022s per thread  
> my_alloc  : 0.066609s per thread  
> malloc    : 0.089587s per thread  
> my_alloc  : 0.068249s per thread  
malloc    : 0.226841s per thread  
my_alloc  : 0.311847s per thread  
malloc    : 0.236389s per thread  
my_alloc  : 0.244183s per thread  
malloc    : 0.222824s per thread  
my_alloc  : 0.243819s per thread  

#### 2.1 分配随机大小(1-1024字节), 按照分配顺序完全释放, inner_loop = 30
malloc    : 1.243445s per thread  
my_alloc  : 0.182761s per thread  
malloc    : 2.210330s per thread  
my_alloc  : 0.223464s per thread  
malloc    : 1.388952s per thread  
my_alloc  : 0.236132s per thread  

#### 2.2 分配固定大小(1700字节), 按照分配顺序完全释放, inner_loop = 30


#### 2.3 分配随机大小(256-256KB), 按照分配顺序完全释放, inner_loop = 30


## 3. TCACHED_MAX的影响

#### 3.1 分配固定大小(64字节), 按照分配顺序完全释放, inner_loop = 100
malloc    : 0.260923s per thread  
my_alloc  : 0.417991s per thread  
malloc    : 0.265674s per thread  
my_alloc  : 0.417570s per thread  
malloc    : 0.276382s per thread  
my_alloc  : 0.421104s per thread  


#### .1 分配随机大小(1-1024字节), 按照分配顺序完全释放, inner_loop = 100
malloc    : 0.160264s per thread  
my_alloc  : 0.217643s per thread  
malloc    : 0.161851s per thread  
my_alloc  : 0.202134s per thread  
malloc    : 0.167909s per thread  
my_alloc  : 0.211136s per thread  

#### .1 分配固定大小(1700字节), 按照分配顺序完全释放, inner_loop = 100
malloc    : 4.360611s per thread  
my_alloc  : 0.575104s per thread  
malloc    : 3.568441s per thread  
my_alloc  : 0.553265s per thread  
malloc    : 3.605131s per thread  
my_alloc  : 0.629348s per thread  

#### .1 分配固定大小(1700字节), 按照分配顺序完全释放, inner_loop = 100, 打开memset(因为感觉不真实不知道为什么)
malloc    : 4.436878s per thread  
my_alloc  : 1.383312s per thread  
malloc    : 4.484728s per thread  
my_alloc  : 1.353482s per thread  
malloc    : 4.293252s per thread  
my_alloc  : 1.344247s per thread  


## 5. 线程数影响

5个对比可以看出, 随着线程数增长, 单个线程的用时开始增长缓慢, 之后机器性能成为瓶颈, 几乎线性增长.

#### .1 分配固定大小(64字节), 按照分配顺序完全释放, 1线程, inner_loop = 100, outer_loop = 100000, next_fit

#### .3 分配固定大小(64字节), 按照分配顺序完全释放, 8线程, inner_loop = 100, outer_loop = 100000, next_fit

#### .4 分配固定大小(64字节), 按照分配顺序完全释放, 16线程, inner_loop = 100, outer_loop = 100000, next_fit

#### .5 分配固定大小(64字节), 按照分配顺序完全释放, 60线程, inner_loop = 100, outer_loop = 100000, next_fit

## first fit 和 next fit 的影响

#### 1.1 分配固定大小(64字节), 按照分配顺序完全释放, inner_loop = 30(TCACHED_MAX内), outer_loop = 100000, next_fit


## 7. inner_loop和outer_loop影响

和基准对比可以看出, 超过`TCACHED_MAX`之后, 不同`inner_loop`和`outer_loop`和用时是线性相关, 所以超过`TCACHED_MAX`之后, `outer_loop`和`outer_loop`对性能的影响只是单纯的倍数关系

#### .1 分配固定大小(8字节), 按照分配顺序完全释放, inner_loop = 10000, outer_loop = 100000, next_fit

#### .2 分配固定大小(8字节), 按照分配顺序完全释放, inner_loop = 30, outer_loop = 10000000, next_fit



###调一下顺序, 冗长没意思的(my_alloc里的三个参数), 超出TCACHED_MAX的放后面