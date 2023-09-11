# bulk_alloc 测试

测试方法
```shell
g++ -O2 run.cc myalloc_test.cc && for ((i=0; i<10; i++)); do ./a.out 0 && ./a.out 1; done
```

结论:
- 使用`bulk_alloc`能有效提高性能约30%
- 另外还可以看出, `malloc`在 随机释放随机数量 下的性能比 按分配的顺序完全释放 差很多
- 另外还可以看出, `malloc`在 块大小变大(64->1700) 的情况下 性能变差很快, `strace`查看发现, `futex`的用时占了了99.9%. 原因不明

## 结果展示

*注. 块较小的情况下结果波动大.

均采用如下配置
```c++
enum release_type{FULL, RAN}; // FULL: 完全释放, RAN: 随机释放一部分
enum size_type{FIX_S, RAN_S, FIX_M}; // FIX_S: 64 Byte, RAN_S: 1-1024 Byte, FIX_M: 1700 Byte
constexpr int thread_num = 1;
constexpr int inner_loop = 1000;
constexpr int outer_loop = 1000;
constexpr bool enable_memset = true;
```

### 测试1(FIX_S + FULL)

#### 未开启的测试结果:

```text
malloc    : 0.053594s per thread  
my_alloc  : 0.095776s per thread  
malloc    : 0.072742s per thread  
my_alloc  : 0.106248s per thread  
malloc    : 0.053572s per thread  
my_alloc  : 0.099129s per thread  
malloc    : 0.034862s per thread  
my_alloc  : 0.092891s per thread  
malloc    : 0.052588s per thread  
my_alloc  : 0.111607s per thread  
malloc    : 0.041407s per thread  
my_alloc  : 0.102602s per thread  
malloc    : 0.053565s per thread  
my_alloc  : 0.113423s per thread  
malloc    : 0.054056s per thread  
my_alloc  : 0.100214s per thread  
malloc    : 0.053550s per thread  
my_alloc  : 0.089263s per thread  
malloc    : 0.057135s per thread  
my_alloc  : 0.111011s per thread  
```
#### 开启后的测试结果:

```text
malloc    : 0.053594s per thread  
my_alloc  : 0.095776s per thread  
malloc    : 0.072742s per thread  
my_alloc  : 0.106248s per thread  
malloc    : 0.053572s per thread  
my_alloc  : 0.099129s per thread  
malloc    : 0.034862s per thread  
my_alloc  : 0.092891s per thread  
malloc    : 0.052588s per thread  
my_alloc  : 0.111607s per thread  
malloc    : 0.041407s per thread  
my_alloc  : 0.102602s per thread  
malloc    : 0.053565s per thread  
my_alloc  : 0.113423s per thread  
malloc    : 0.054056s per thread  
my_alloc  : 0.100214s per thread  
malloc    : 0.053550s per thread  
my_alloc  : 0.089263s per thread  
malloc    : 0.057135s per thread  
my_alloc  : 0.111011s per thread  
```

### 测试2(FIX_S + RAN)

#### 未开启的测试结果:

```text
malloc    : 0.103177s per thread  
my_alloc  : 0.115020s per thread  
malloc    : 0.111065s per thread  
my_alloc  : 0.114961s per thread  
malloc    : 0.086289s per thread  
my_alloc  : 0.113121s per thread  
malloc    : 0.107226s per thread  
my_alloc  : 0.133447s per thread  
malloc    : 0.083772s per thread  
my_alloc  : 0.128529s per thread  
malloc    : 0.099683s per thread  
my_alloc  : 0.116956s per thread  
malloc    : 0.080511s per thread  
my_alloc  : 0.115251s per thread  
malloc    : 0.089994s per thread  
my_alloc  : 0.109669s per thread  
malloc    : 0.100411s per thread  
my_alloc  : 0.133296s per thread  
malloc    : 0.100331s per thread  
my_alloc  : 0.119199s per thread  
```
#### 开启后的测试结果:

```text
malloc    : 0.103302s per thread  
my_alloc  : 0.081456s per thread  
malloc    : 0.087359s per thread  
my_alloc  : 0.083306s per thread  
malloc    : 0.100142s per thread  
my_alloc  : 0.083267s per thread  
malloc    : 0.104431s per thread  
my_alloc  : 0.078098s per thread  
malloc    : 0.106878s per thread  
my_alloc  : 0.063637s per thread  
malloc    : 0.084574s per thread  
my_alloc  : 0.061239s per thread  
malloc    : 0.084608s per thread  
my_alloc  : 0.072779s per thread  
malloc    : 0.112863s per thread  
my_alloc  : 0.058177s per thread  
malloc    : 0.080961s per thread  
my_alloc  : 0.082806s per thread  
malloc    : 0.107772s per thread  
my_alloc  : 0.081457s per thread  
```

### 测试3(RAN_S + RAN)

#### 未开启的测试结果:

```text
malloc    : 0.417778s per thread  
my_alloc  : 0.294233s per thread  
malloc    : 0.679326s per thread  
my_alloc  : 0.349040s per thread  
malloc    : 0.463199s per thread  
my_alloc  : 0.208187s per thread  
malloc    : 0.339218s per thread  
my_alloc  : 0.203110s per thread  
malloc    : 0.331969s per thread  
my_alloc  : 0.209229s per thread  
malloc    : 0.349963s per thread  
my_alloc  : 0.231014s per thread  
malloc    : 0.357440s per thread  
my_alloc  : 0.232300s per thread  
malloc    : 0.440766s per thread  
my_alloc  : 0.183951s per thread  
malloc    : 0.379994s per thread  
my_alloc  : 0.244011s per thread  
malloc    : 0.346965s per thread  
my_alloc  : 0.202919s per thread  
```
#### 开启后的测试结果:

```text
malloc    : 0.376280s per thread  
my_alloc  : 0.168575s per thread  
malloc    : 0.379926s per thread  
my_alloc  : 0.150780s per thread  
malloc    : 0.416929s per thread  
my_alloc  : 0.165615s per thread  
malloc    : 0.359104s per thread  
my_alloc  : 0.165632s per thread  
malloc    : 0.394583s per thread  
my_alloc  : 0.149274s per thread  
malloc    : 0.423680s per thread  
my_alloc  : 0.189218s per thread  
malloc    : 0.385032s per thread  
my_alloc  : 0.145840s per thread  
malloc    : 0.376827s per thread  
my_alloc  : 0.140085s per thread  
malloc    : 0.344936s per thread  
my_alloc  : 0.150629s per thread  
malloc    : 0.395582s per thread  
my_alloc  : 0.179485s per thread  
```

### 测试4(RAN_S + FULL)

#### 未开启的测试结果:

```text
malloc    : 0.075693s per thread  
my_alloc  : 0.185423s per thread  
malloc    : 0.046012s per thread  
my_alloc  : 0.127394s per thread  
malloc    : 0.086556s per thread  
my_alloc  : 0.123714s per thread  
malloc    : 0.073384s per thread  
my_alloc  : 0.114494s per thread  
malloc    : 0.076661s per thread  
my_alloc  : 0.101647s per thread  
malloc    : 0.064412s per thread  
my_alloc  : 0.113436s per thread  
malloc    : 0.062730s per thread  
my_alloc  : 0.103539s per thread  
malloc    : 0.068836s per thread  
my_alloc  : 0.125089s per thread  
malloc    : 0.076099s per thread  
my_alloc  : 0.118966s per thread  
malloc    : 0.070389s per thread  
my_alloc  : 0.168513s per thread  
```

#### 开启后的测试结果:

```text
malloc    : 0.057331s per thread  
my_alloc  : 0.079922s per thread  
malloc    : 0.059716s per thread  
my_alloc  : 0.081361s per thread  
malloc    : 0.073547s per thread  
my_alloc  : 0.091318s per thread  
malloc    : 0.055275s per thread  
my_alloc  : 0.099439s per thread  
malloc    : 0.074574s per thread  
my_alloc  : 0.081602s per thread  
malloc    : 0.069137s per thread  
my_alloc  : 0.093499s per thread  
malloc    : 0.079004s per thread  
my_alloc  : 0.090813s per thread  
malloc    : 0.074541s per thread  
my_alloc  : 0.080820s per thread  
malloc    : 0.083206s per thread  
my_alloc  : 0.092401s per thread  
malloc    : 0.077731s per thread  
my_alloc  : 0.076239s per thread  
```

### 测试5(FIX_M + FULL)

#### 未开启的测试结果:

```text
malloc    : 1.667380s per thread  
my_alloc  : 0.093257s per thread  
malloc    : 1.717308s per thread  
my_alloc  : 0.101452s per thread  
malloc    : 1.441534s per thread  
my_alloc  : 0.075845s per thread  
malloc    : 1.469260s per thread  
my_alloc  : 0.079139s per thread  
malloc    : 1.374004s per thread  
my_alloc  : 0.097052s per thread  
malloc    : 1.522090s per thread  
my_alloc  : 0.095036s per thread  
malloc    : 1.439549s per thread  
my_alloc  : 0.076064s per thread  
malloc    : 1.460276s per thread  
my_alloc  : 0.068107s per thread  
malloc    : 2.312381s per thread  
my_alloc  : 0.175017s per thread  
malloc    : 2.408959s per thread  
my_alloc  : 0.182729s per thread  
```

#### 开启后的测试结果:

```text
malloc    : 1.329719s per thread  
my_alloc  : 0.081163s per thread  
malloc    : 1.327566s per thread  
my_alloc  : 0.069114s per thread  
malloc    : 1.307578s per thread  
my_alloc  : 0.070208s per thread  
malloc    : 1.327169s per thread  
my_alloc  : 0.068355s per thread  
malloc    : 1.305414s per thread  
my_alloc  : 0.070756s per thread  
malloc    : 1.267545s per thread  
my_alloc  : 0.069703s per thread  
malloc    : 1.256985s per thread  
my_alloc  : 0.058201s per thread  
malloc    : 1.285240s per thread  
my_alloc  : 0.070153s per thread  
malloc    : 1.252274s per thread  
my_alloc  : 0.074049s per thread  
malloc    : 1.261480s per thread  
my_alloc  : 0.050988s per thread  
```

由于差距过大, 使用`strace -c ./a.out 0`查看. 结果如下:

```text
% time     seconds  usecs/call     calls    errors syscall
------ ----------- ----------- --------- --------- ----------------
 99.85    1.107467      553733         2           futex
  0.05    0.000609         609         1           execve
  0.04    0.000467          18        25           mmap
  0.01    0.000118          19         6           openat
  0.01    0.000103          14         7           mprotect
  0.01    0.000085          17         5           read
  0.01    0.000067          11         6           newfstatat
  0.00    0.000040           6         6           close
  0.00    0.000036          36         1           munmap
  0.00    0.000029           9         3           brk
  0.00    0.000027          27         1           write
  0.00    0.000015          15         1         1 access
  0.00    0.000015           7         2         1 arch_prctl
  0.00    0.000012           6         2           pread64
  0.00    0.000008           8         1           getrandom
  0.00    0.000007           7         1           prlimit64
  0.00    0.000006           6         1           set_tid_address
  0.00    0.000005           5         1           set_robust_list
  0.00    0.000005           5         1           rseq
  0.00    0.000004           4         1           clone3
  0.00    0.000000           0         1           rt_sigaction
  0.00    0.000000           0         3           rt_sigprocmask
------ ----------- ----------- --------- --------- ----------------
100.00    1.109125       14219        78         2 total
```

`strace ./a.out 0`的输出在[这里](malloc.log)
