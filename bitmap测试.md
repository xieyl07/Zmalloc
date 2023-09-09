# bitmap 测试

结论:
- 使用`first fit`下, bitset明显快而且稳定(必须O2). 肯定是编译器优化, `next fit`下不如另外两个. 

```text
first fit
b32 linear: 0.663058
b64 linear: 0.567891
bitset linear: 0.178909
b32 random: 1.058494
b64 random: 0.895482
bitset random: 0.175797
b32 linear: 0.746841
b64 linear: 0.619092
bitset linear: 0.186554
b32 random: 1.016871
b64 random: 0.855250
bitset random: 0.206301
b32 linear: 0.869871
b64 linear: 0.707933
bitset linear: 0.196700
b32 random: 1.200858
b64 random: 0.771659
bitset random: 0.174105


next fit
b32 linear: 0.470794
b64 linear: 0.469409
bitset linear: 0.458250
b32 random: 0.737791
b64 random: 0.696092
bitset random: 0.973291
b32 linear: 0.476162
b64 linear: 0.475553
bitset linear: 0.463254
b32 random: 0.735317
b64 random: 0.689656
bitset random: 0.973294
b32 linear: 0.472280
b64 linear: 0.471166
bitset linear: 0.462289
b32 random: 0.740999
b64 random: 0.694978
bitset random: 0.979599
```