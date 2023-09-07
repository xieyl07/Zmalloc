# allocator

一个支持多线程的内存池, 借鉴了 jemalloc. 采用 bit map 减小分配器自身空间占用.

### 名词解释:
`run`: 即`slab`
`binind`: 这个 page 对应的 run 是 bins 里的第几个, 也就确定了一块 region 大小. 有必要?  
`page_id`: 是第几个 **存数据的** page  
`run_id`: run page_id  

三个容器: `arena`, `bin`, `tcache`.  
三种容量: `chunk`, `page`, `region`.  
`arena`里有空的chunk(spare, chunk_tree)和page(avail_pages), bin里有半空的run, tcache里有空的region

```text
region_size: 8, page_num:1, region_num:512
region_size: 16, page_num:1, region_num:256
region_size: 32, page_num:1, region_num:128
region_size: 48, page_num:3, region_num:256
region_size: 64, page_num:1, region_num:64
region_size: 80, page_num:5, region_num:256
region_size: 96, page_num:3, region_num:128
region_size: 112, page_num:7, region_num:256
region_size: 128, page_num:1, region_num:32
region_size: 160, page_num:5, region_num:128
region_size: 192, page_num:3, region_num:64
region_size: 224, page_num:7, region_num:128
region_size: 256, page_num:1, region_num:16
region_size: 320, page_num:5, region_num:64
region_size: 384, page_num:3, region_num:32
region_size: 448, page_num:7, region_num:64
region_size: 512, page_num:1, region_num:8
region_size: 640, page_num:5, region_num:32
region_size: 768, page_num:3, region_num:16
region_size: 896, page_num:7, region_num:32
region_size: 1024, page_num:1, region_num:4
region_size: 1280, page_num:5, region_num:16
region_size: 1536, page_num:3, region_num:8
region_size: 1792, page_num:7, region_num:16
region_size: 2048, page_num:2, region_num:4
region_size: 2560, page_num:5, region_num:8
region_size: 3072, page_num:3, region_num:4
region_size: 3584, page_num:7, region_num:8
region_size: 4096, page_num:4, region_num:4
region_size: 5120, page_num:5, region_num:4
region_size: 6144, page_num:6, region_num:4
region_size: 7168, page_num:7, region_num:4
region_size: 8192, page_num:8, region_num:4
region_size: 10240, page_num:10, region_num:4
region_size: 12288, page_num:12, region_num:4
region_size: 14336, page_num:14, region_num:4
```

### 调试技术
`CLion`可以方便的查看对象结构和内存(`x/8b 0x77777770000`)  
`gdb`可以方便的断点, `b arena.cc:207 if (chunk < 0x100)`, `watch *0x77777770000`(默认4字节内容), `watch`非常好用, `watch ((Arena*)0x5555555802b0)->bins[20].cur_run == 0x7ffff6c10020`(类成员就要把类地址写上不然gdb不认识), 可以看出越界访问, 忘记从cur_run中移除等等被意料之外的人写入的问题  
`gdb`的`p`可以执行函数, 非常好用  
printf "%p" chunk +  
不能后退, 就打日志吧  
`display avail_pages.empty() ? "empty" : (*avail_pages.begin())`

### 性能优化
最初写完的时候性能还比不上malloc. 用`perf record ./a.out`, `perf report`分析性能, 把几个对性能影响大的函数进行优化. 和想象的不同, 向系统分配内存上的速度不是问题, 但是在 run 的 bitmap 非常影响性能, 先用 `ffs` 代替自己的循环, 再用 `std::bitset`, 性能有了明显提升.

为了测试内存泄漏情况, 可以取消注释~TCache 里的 delete. 

内存泄漏和越界访问问题检测可以用`valgrind`, 可以编译时加上`-fsanitize=address`. #define memory_leaks_detecet就可以进行内存泄漏的测试