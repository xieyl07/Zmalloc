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
malloc    : 0.053594s  0.095776s :    my_alloc (per thread)  
malloc    : 0.072742s  0.106248s :    my_alloc (per thread)  
malloc    : 0.053572s  0.099129s :    my_alloc (per thread)  
malloc    : 0.034862s  0.092891s :    my_alloc (per thread)  
malloc    : 0.052588s  0.111607s :    my_alloc (per thread)  
malloc    : 0.041407s  0.102602s :    my_alloc (per thread)  
malloc    : 0.053565s  0.113423s :    my_alloc (per thread)  
malloc    : 0.054056s  0.100214s :    my_alloc (per thread)  
malloc    : 0.053550s  0.089263s :    my_alloc (per thread)  
malloc    : 0.057135s  0.111011s :    my_alloc (per thread)  
```
#### 开启后的测试结果:

```text
malloc    : 0.053594s  0.024875s :    my_alloc (per thread)  
malloc    : 0.072742s  0.048930s :    my_alloc (per thread)  
malloc    : 0.053572s  0.026888s :    my_alloc (per thread)  
malloc    : 0.034862s  0.056419s :    my_alloc (per thread)  
malloc    : 0.052588s  0.059501s :    my_alloc (per thread)  
malloc    : 0.041407s  0.055562s :    my_alloc (per thread)  
malloc    : 0.053565s  0.053902s :    my_alloc (per thread)  
malloc    : 0.054056s  0.030326s :    my_alloc (per thread)  
malloc    : 0.053550s  0.055958s :    my_alloc (per thread)  
malloc    : 0.057135s  0.053504s :    my_alloc (per thread)  
```

### 测试2(FIX_S + RAN)

#### 未开启的测试结果:

```text
malloc    : 0.103177s  0.115020s :    my_alloc (per thread)  
malloc    : 0.111065s  0.114961s :    my_alloc (per thread)  
malloc    : 0.086289s  0.113121s :    my_alloc (per thread)  
malloc    : 0.107226s  0.133447s :    my_alloc (per thread)  
malloc    : 0.083772s  0.128529s :    my_alloc (per thread)  
malloc    : 0.099683s  0.116956s :    my_alloc (per thread)  
malloc    : 0.080511s  0.115251s :    my_alloc (per thread)  
malloc    : 0.089994s  0.109669s :    my_alloc (per thread)  
malloc    : 0.100411s  0.133296s :    my_alloc (per thread)  
malloc    : 0.100331s  0.119199s :    my_alloc (per thread)  
```
#### 开启后的测试结果:

```text
malloc    : 0.103302s  0.081456s :    my_alloc (per thread)  
malloc    : 0.087359s  0.083306s :    my_alloc (per thread)  
malloc    : 0.100142s  0.083267s :    my_alloc (per thread)  
malloc    : 0.104431s  0.078098s :    my_alloc (per thread)  
malloc    : 0.106878s  0.063637s :    my_alloc (per thread)  
malloc    : 0.084574s  0.061239s :    my_alloc (per thread)  
malloc    : 0.084608s  0.072779s :    my_alloc (per thread)  
malloc    : 0.112863s  0.058177s :    my_alloc (per thread)  
malloc    : 0.080961s  0.082806s :    my_alloc (per thread)  
malloc    : 0.107772s  0.081457s :    my_alloc (per thread)  
```

### 测试3(RAN_S + RAN)

#### 未开启的测试结果:

```text
malloc    : 0.417778s  0.294233s :    my_alloc (per thread)  
malloc    : 0.679326s  0.349040s :    my_alloc (per thread)  
malloc    : 0.463199s  0.208187s :    my_alloc (per thread)  
malloc    : 0.339218s  0.203110s :    my_alloc (per thread)  
malloc    : 0.331969s  0.209229s :    my_alloc (per thread)  
malloc    : 0.349963s  0.231014s :    my_alloc (per thread)  
malloc    : 0.357440s  0.232300s :    my_alloc (per thread)  
malloc    : 0.440766s  0.183951s :    my_alloc (per thread)  
malloc    : 0.379994s  0.244011s :    my_alloc (per thread)  
malloc    : 0.346965s  0.202919s :    my_alloc (per thread)  
```
#### 开启后的测试结果:

```text
malloc    : 0.376280s  0.168575s :    my_alloc (per thread)  
malloc    : 0.379926s  0.150780s :    my_alloc (per thread)  
malloc    : 0.416929s  0.165615s :    my_alloc (per thread)  
malloc    : 0.359104s  0.165632s :    my_alloc (per thread)  
malloc    : 0.394583s  0.149274s :    my_alloc (per thread)  
malloc    : 0.423680s  0.189218s :    my_alloc (per thread)  
malloc    : 0.385032s  0.145840s :    my_alloc (per thread)  
malloc    : 0.376827s  0.140085s :    my_alloc (per thread)  
malloc    : 0.344936s  0.150629s :    my_alloc (per thread)  
malloc    : 0.395582s  0.179485s :    my_alloc (per thread)  
```

### 测试4(RAN_S + FULL)

#### 未开启的测试结果:

```text
malloc    : 0.075693s  0.185423s :    my_alloc (per thread)  
malloc    : 0.046012s  0.127394s :    my_alloc (per thread)  
malloc    : 0.086556s  0.123714s :    my_alloc (per thread)  
malloc    : 0.073384s  0.114494s :    my_alloc (per thread)  
malloc    : 0.076661s  0.101647s :    my_alloc (per thread)  
malloc    : 0.064412s  0.113436s :    my_alloc (per thread)  
malloc    : 0.062730s  0.103539s :    my_alloc (per thread)  
malloc    : 0.068836s  0.125089s :    my_alloc (per thread)  
malloc    : 0.076099s  0.118966s :    my_alloc (per thread)  
malloc    : 0.070389s  0.168513s :    my_alloc (per thread)  
```

#### 开启后的测试结果:

```text
malloc    : 0.057331s  0.079922s :    my_alloc (per thread)  
malloc    : 0.059716s  0.081361s :    my_alloc (per thread)  
malloc    : 0.073547s  0.091318s :    my_alloc (per thread)  
malloc    : 0.055275s  0.099439s :    my_alloc (per thread)  
malloc    : 0.074574s  0.081602s :    my_alloc (per thread)  
malloc    : 0.069137s  0.093499s :    my_alloc (per thread)  
malloc    : 0.079004s  0.090813s :    my_alloc (per thread)  
malloc    : 0.074541s  0.080820s :    my_alloc (per thread)  
malloc    : 0.083206s  0.092401s :    my_alloc (per thread)  
malloc    : 0.077731s  0.076239s :    my_alloc (per thread)  
```

### 测试5(FIX_M + FULL)

#### 未开启的测试结果:

```text
malloc    : 1.667380s  0.093257s :    my_alloc (per thread)  
malloc    : 1.717308s  0.101452s :    my_alloc (per thread)  
malloc    : 1.441534s  0.075845s :    my_alloc (per thread)  
malloc    : 1.469260s  0.079139s :    my_alloc (per thread)  
malloc    : 1.374004s  0.097052s :    my_alloc (per thread)  
malloc    : 1.522090s  0.095036s :    my_alloc (per thread)  
malloc    : 1.439549s  0.076064s :    my_alloc (per thread)  
malloc    : 1.460276s  0.068107s :    my_alloc (per thread)  
malloc    : 2.312381s  0.175017s :    my_alloc (per thread)  
malloc    : 2.408959s  0.182729s :    my_alloc (per thread)  
```

#### 开启后的测试结果:

```text
malloc    : 1.329719s  0.081163s :    my_alloc (per thread)  
malloc    : 1.327566s  0.069114s :    my_alloc (per thread)  
malloc    : 1.307578s  0.070208s :    my_alloc (per thread)  
malloc    : 1.327169s  0.068355s :    my_alloc (per thread)  
malloc    : 1.305414s  0.070756s :    my_alloc (per thread)  
malloc    : 1.267545s  0.069703s :    my_alloc (per thread)  
malloc    : 1.256985s  0.058201s :    my_alloc (per thread)  
malloc    : 1.285240s  0.070153s :    my_alloc (per thread)  
malloc    : 1.252274s  0.074049s :    my_alloc (per thread)  
malloc    : 1.261480s  0.050988s :    my_alloc (per thread)  
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
