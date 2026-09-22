# TODO

What is left to do on GAOL v5. What is done is in
[What differs from GAOL](doc/differences.md).

## Code

1. **The pow of the standard is written twice.** `gaol_ieee1788::pow`
   (`gaol/gaol_ieee1788.h`) intersects x with [0, +oo] and takes x = {0}
   apart, and `gaol_pow_hybrid()` does it again. Fix: export the standard half
   of `gaol_pow_hybrid()`, the pow of Table 9.1 for an interval exponent, have
   `gaol_pow_hybrid()` add only the pown case of a degenerate integer
   exponent, and have `gaol_ieee1788::pow` call that half directly, apart
   from its integer exponents beyond the ints.

2. **The rounding direction is still checked more than once** by `tan()`, the
   relational functions of the trigonometric functions (`acos_rel()`,
   `asin_rel()`, `atan_rel()`) and the negative integer powers `pow(x, -n)`,
   which call operations of intervals (`+`, `/`, `inverse()`, `atan()`...)
   that check it again: six times per call of `tan()` and of the relational
   functions. Fix: cores of these operations without the check, in
   `gaol/gaol_interval_sse.cpp` and `gaol/gaol_interval_fpu.cpp`, called
   after the check of the function calling them, as the functions of
   namespace `upward` are for the bounds at doubles.

3. **`pow(x, y)` is one double wide where the power at a corner is a double.**
   The lower bound is the double below CORE-MATH's value rounded upward, even
   where that value is exact: `pow([4], 0.5)` is [2 − 2^-52, 2], and cases
   149 to 152 and 163 of [doc/compare/special_cases.md](doc/compare/special_cases.md)
   are wider than IEEE 1788's result for this reason alone. Fix: keep the
   value as the lower bound where the power is exact, as `exp2`, `log2`,
   `nth_root(x, 3)` and the functions of Table 10.5 do.

## CORE-MATH

4. **Propose the fixes of the vendored sources upstream.**
   [3rd/README.md](3rd/README.md) lists five changes of the CORE-MATH sources;
   four of them are fixes rather than adaptations to GAOL: the masks `~0ull`
   of `sinh.c`, `cosh.c` and `tanh.c`, the signed shifts of `cospi.c`, the
   128-bit shift of `asinpi.c`, and the 64-bit `__builtin_expect` of
   `rsqrt.c`, which took subnormals for +0 wherever `long` has 32 bits.

## Documentation

5. **The coverage report** ([coverage/README.md](coverage/README.md)) was
   written on 2026-09-20: its lines not run no longer match the sources. To
   write again with `-DGAOL_COVERAGE=ON` and the target `coverage`, which
   need gcovr.

6. **The timings of [doc/compare/performance.md](doc/compare/performance.md)**
   were measured at `bb6f7e4` with files outside the sources changed
   (`bb6f7e4-dirty`), before the rounding direction was checked once per
   function of intervals (`exp()` 5.7 % faster since, `sin()` 3.6 %,
   `pow(x, y)` 3.4 %). To measure again on a clean commit, the machine doing
   nothing else (`doc/compare/code/run_bench.sh`).
