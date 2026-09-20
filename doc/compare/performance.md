<!-- Copyright (c) 2026 ENSTA, France
     Created 2026-09-20 by Jordan NININ -->
# Performance: GAOL, libieeep1788, filib++, PROFIL/BIAS and Solaris Studio

Part of the [comparison](README.md) of GAOL with libieeep1788, filib++,
PROFIL/BIAS and Solaris Studio.

One million operations of each kind are computed by each library on ordinary
intervals: bounded, of widths from 10^-6 to 1, drawn once and read from the
same file by every program. Each operation is timed on the whole array, its
results being stored in another one; the time per operation is the best time
divided by the million. The same operations on doubles, computed at the
midpoints of the intervals without any rounding, give the cost of the
arithmetic itself. [code/README.md](code/README.md) says how to run the
benchmark again.

| Library | Compiled with | Operations |
|---|---|---|
| GAOL V5.0.0 (this branch) | GCC 9.4, `-O3 -mfma`, the flags of interval arithmetic; GAOL built by CMake in Release with `-mfma` (`GAOL_FMA`), the sources of CORE-MATH compiled into it | Inline SSE2 operations, the rounding direction set upward; **every** elementary function CORE-MATH's, correctly rounded in that direction, so the bounds are the tightest ones and the direction is never switched |
| GAOL 4.3.1 (the master branch) | the same, with mathlib 2.1.1 of `3rd/mathlib` | The same operations; elementary functions computed by mathlib, correctly rounded to nearest, then moved one double outward, the rounding direction set to nearest and back for each; log CORE-MATH's |
| libieeep1788 | GCC 9.4, `-O3 -mfma`; MPFR 4.2.1 and GMP 6.3.0 built by GCC 9.4 with `-O3 -mfma` | Each bound computed by MPFR, correctly rounded |
| filib++ | GCC 9.4, `-O3 -mfma`, the flags of interval arithmetic; filib++ 3.0.2.2 built by GCC 9.4 in C++11 with `-O3 -mfma` | `interval<double, native_switched, i_mode_extended_flag>`, as in IBEX: inline operations, the rounding direction set and restored by each; elementary functions of its own |
| PROFIL/BIAS | GCC 9.4, `-O3 -mfma`, the flags of interval arithmetic; PROFIL/BIAS 2.0.8 built with its `x86-64-Linux-compat-gcc` configuration, by GCC 9.4 in C++11 with `-O3 -mfma -ffp-contract=off` | `INTERVAL`, whose operations are calls to the BIAS library, each setting the rounding direction downward then upward and back to nearest; elementary functions from the libm, moved outward |
| Solaris Studio | Sun Fortran 95 8.7 (Solaris Studio 12.4, 2014), `-O3 -xia` | Calls to `libsunimath` |

## What the timings show

The two versions of GAOL are measured side by side: **GAOL V5.0.0**, this
branch, which bounds every elementary function with CORE-MATH, and **GAOL
4.3.1**, the master branch, which bounds them with mathlib. Everything was
built and run again for this table, with GCC 9.4: the numbers are not those of
the earlier table, measured with Clang 18, but the seven columns are
comparable with one another.

**What GAOL V5.0.0 changes.** The arithmetic is untouched, and the timings say
so: +, −, ×, ÷, `sqr` and `sqrt` are the same to within the noise. The
elementary functions are between 1.3 and 2.3 times faster:

| | GAOL V5.0.0 | GAOL 4.3.1 | |
|---|---:|---:|---|
| `log` | 29.1 ns | 67.6 ns | 2.3 times faster |
| `pow(x, y)` | 71.7 | 138 | 1.9 |
| `exp` | 29.1 | 46.7 | 1.6 |
| `pow(x, 3)` | 22.7 | 32.0 | 1.4 |
| `sin` | 87.3 | 113 | 1.3 |
| `cos` | 84.8 | 113 | 1.3 |
| five-line block | 308 | 399 | 1.3 |

Two things are gone at once: the outward move of each bound, and the two
changes of rounding direction each function made, to nearest before mathlib
and back upward after. **And the bounds are tighter, not looser**: in the last
table below, GAOL V5.0.0's results are no wider than libieeep1788's, which
computes every bound with MPFR, for **every one of the seventeen operations**
(excess 0.0e+00), where GAOL 4.3.1 was wider by up to 1.5e-14 relatively on
log and 3.6e-15 on the real power.

**Arithmetic.** GAOL is the fastest on + and −, 3.8 ns, about twice as fast as
filib++ (7.3 to 7.5 ns) and six times as fast as PROFIL/BIAS (22 ns) and
Solaris Studio (24 ns); × and ÷ take GAOL 16 and 13 ns, against 17 and 12 ns
for filib++, 22 ns for PROFIL/BIAS and 27 to 29 ns for Solaris Studio. An
addition of intervals costs GAOL 3.5 times an addition of doubles: two
additions, and the check that the rounding direction is upward, where filib++
sets the rounding direction and restores it, and PROFIL/BIAS calls a function
of BIAS that sets it downward, then upward, then back to nearest — which is
why its four arithmetic operations all take about the same time. The integer
power `pow(x, 3)` takes GAOL 23 ns, computed from exact products since
[issue #7](https://github.com/Jordan08/GAOL/issues/7), whose `fma()` is one
instruction with `-mfma`: filib++'s `power(x, 3)` takes 27 ns, PROFIL/BIAS's
50 ns and Solaris Studio's `x**3` 214 ns; `sqr` takes GAOL 9.6 ns, against 13
for filib++, 28 for PROFIL/BIAS and 76 for Solaris Studio.

**Elementary functions.** PROFIL/BIAS is the fastest on exp (16 ns), log
(17 ns) and the real power `pow(x, y)` (54 ns), which it computes from the libm
of the system, moved outward, without correct rounding: its bounds are the
widest of the five (below), and its sin and cos, computed by its own argument
reduction, are the slowest, 174 and 196 ns. GAOL V5.0.0 comes next on exp and
log, 29.1 ns each — faster than filib++ (46 and 31 ns) and than Solaris Studio
(55 and 56 ns) — with **correctly rounded** values, where the others are not.
filib++ is the fastest on sin and cos (54 ns) with its own polynomials, then
Solaris Studio (59 ns), then GAOL V5.0.0 (87 and 85 ns): GAOL pays there for
correct rounding over the whole range, and for dividing the bounds by an
interval enclosing π to find where the function is monotonic, asking CORE-MATH
for the signs of the derivative where the division cannot tell
([issue #6](https://github.com/Jordan08/GAOL/issues/6)). On `pow(x, y)` GAOL
V5.0.0 takes 72 ns, against 92 for filib++ and 184 for Solaris Studio.

**Formulas.** GAOL is the fastest on the arithmetic line (29 ns, against 41
for filib++, 88 for Solaris Studio and 91 for PROFIL/BIAS), on the line of
powers (84 ns, against 121, 360 and 133) and on Shekel 5 (310 ns, against
582, 2 367 and 1 597: Shekel 5 is 20 squares, 45 additions and subtractions
and 5 divisions, and its squares cost Solaris Studio 76 ns each and
PROFIL/BIAS 28). filib++ is the fastest on the line of sin and cos (138 ns,
against 204 for GAOL, 228 for Solaris Studio and 440 for PROFIL/BIAS) and, by
a little, on the five-line block (283 ns, against 308 for GAOL, 611 for
PROFIL/BIAS and 641 for Solaris Studio), where its elementary functions weigh
most.

**libieeep1788** is 14 to 93 times slower than GAOL: every bound is an MPFR
computation, about 210 ns for an addition and 7 to 9 µs for sin, cos and the
real power. Its own README warns that its focus is correctness, not speed.
GAOL V5.0.0 now gives **the same bounds as it does**, tightest everywhere, at
between a fourteenth and a ninety-third of the time.

**The results** are the same: the sums of the midpoints of the million results
agree to 4e-15 relatively, apart from PROFIL/BIAS's integer powers (2.4e-06,
below). The arithmetic operations and the square roots give the tightest
intervals in the five libraries, except filib++'s and PROFIL/BIAS's square
roots. Of the elementary functions, **only GAOL V5.0.0's are the tightest on
every operation**: on average the others are wider than libieeep1788's by up
to 3.6e-15 relatively with GAOL 4.3.1, 4e-15 to 2.4e-13 with filib++ (1.4e-13
for log, 2.4e-13 for the real power), 9e-15 to 1.2e-13 with PROFIL/BIAS, and
at most 5e-15 with Solaris Studio. PROFIL/BIAS's integer power is far wider,
1.1e-05 on average: it computes `Power(x, n)` as exp(n log x), where the four
others multiply. Its arithmetic results have midpoints that differ by about
1e-14 relatively while their widths are the tightest: the intervals are the
same, but its `Mid` computes inf + (sup − inf)/2 **rounded upward**, one double
above the midpoint for 4 % of the sums of the benchmark, where the others round
to nearest.

The timings were measured on a laptop, each time being the best of several runs
spread over three rounds: the machine slowed down now and then for a few
seconds, which made some operations up to 2.5 times slower in a single round.
libieeep1788's times, from a single run of each operation in each round, are
the least steady. Solaris Studio's and PROFIL/BIAS's operations are calls into
libraries, where the compiler inlines GAOL's and filib++'s.

## The timings

<!-- BEGIN GENERATED TABLES (doc/compare/code/bench.py) -->

Measured on:

```
Date:            2026-09-20
Processor:       11th Gen Intel(R) Core(TM) i7-1185G7 @ 3.00GHz
System:          Linux 5.15.0-191-generic, Ubuntu 20.04.6 LTS
C++ compiler:    Ubuntu clang version 18.1.8 (11~20.04.2)
C++ flags:       -std=c++11 -O3 -DNDEBUG -mfma (GAOL: -frounding-math -fno-fast-math -ffp-contract=off -msse2 -msse3 -mfma; libieeep1788, filib++ and PROFIL/BIAS: -frounding-math -fno-fast-math -ffp-contract=off)
Fortran:         f90: Sun Fortran 95 8.7 Linux_i386 2014/10/20, flags: -O3 -xia
GAOL V5.0.0:     the branch of this checkout (v4.3.2-35-g5bfc3f9), CMake Release, CORE-MATH of 3rd/math-core compiled into the library
GAOL 4.3.2:      the master branch, CMake Release, mathlib 2.1.1 of 3rd/mathlib
libieeep1788:    1f10b89, MPFR 4.2.1, GMP 6.3.0
filib++:         3.0.2.2, interval<double, native_switched, i_mode_extended_flag>
PROFIL/BIAS:     2.0.8, x86-64-Linux-compat-gcc configuration, built by clang-18 and clang++-18
```

1 000 000 operations of each kind, on the same intervals. Each time is the best of 3 rounds, each program being run in turn with the others: 15 runs for double (reference), 3 runs for libieeep1788, 15 runs for GAOL V5.0.0, 15 runs for GAOL 4.3.2, 15 runs for filib++, 15 runs for Solaris Studio f90, 15 runs for PROFIL/BIAS in all.

#### The operations

a and b are intervals centred in [−10, 10], p in [1, 10], e in [0.5, 2.5], the arguments of Shekel 5 in [0, 10]; their widths are 10^u, u uniform in [−6, 0] ([−6, −1] for e).

| Operation | Computes |
|---|---|
| `add` | a + b |
| `sub` | a − b |
| `mul` | a × b |
| `div` | a / p |
| `sqr` | a² (`sqr`, `x**2` in Fortran) |
| `sqrt` | √p |
| `exp` | exp(a) |
| `log` | log(p) |
| `sin` | sin(a) |
| `cos` | cos(a) |
| `pow_int` | a³ (`pow(x, int)` in GAOL, `pown`, `power(x, int)` in filib++, `x**3`) |
| `pow_real` | p^e (`pow(x, y)`, `x**y`) |
| `line_arith` | (a + b)(a − b) / p |
| `line_trig` | sin(a) cos(b) + a² |
| `line_pow` | √p · a³ − exp(b / p) |
| `shekel5` | Shekel 5: −Σᵢ 1 / (Σⱼ (xⱼ − aᵢⱼ)² + cᵢ), 5 terms, 4 variables |
| `block5` | five lines: t1 = ab + p; t2 = sin(t1) cos(b); t3 = a² + t2/p; t4 = exp(t2) − b³; t3 t4 + √p |

#### Time per operation (nanoseconds)

| Operation | double (reference) | libieeep1788 | GAOL V5.0.0 | GAOL 4.3.2 | filib++ | Solaris Studio f90 | PROFIL/BIAS | libieeep1788 / GAOL V5.0.0 | GAOL 4.3.2 / GAOL V5.0.0 | filib++ / GAOL V5.0.0 | Solaris Studio f90 / GAOL V5.0.0 | PROFIL/BIAS / GAOL V5.0.0 |
|---|---|---|---|---|---|---|---|---|---|---|---|---|
| `add` | 1.20 | 210 | 3.23 | 3.24 | 7.91 | 23.9 | 21.5 | 65.0 | 1.0 | 2.4 | 7.4 | 6.6 |
| `sub` | 1.20 | 205 | 3.43 | 3.41 | 7.93 | 23.9 | 21.7 | 59.7 | 1.0 | 2.3 | 7.0 | 6.3 |
| `mul` | 1.22 | 259 | 16.3 | 16.3 | 23.3 | 28.9 | 22.0 | 15.9 | 1.0 | 1.4 | 1.8 | 1.4 |
| `div` | 1.20 | 263 | 12.8 | 12.7 | 13.7 | 26.9 | 21.7 | 20.6 | 1.0 | 1.1 | 2.1 | 1.7 |
| `sqr` | 0.76 | 134 | 9.68 | 9.61 | 12.5 | 75.8 | 28.1 | 13.8 | 1.0 | 1.3 | 7.8 | 2.9 |
| `sqrt` | 1.97 | 142 | 11.7 | 12.1 | 22.8 | 36.8 | 5.58 | 12.1 | 1.0 | 1.9 | 3.1 | 0.5 |
| `exp` | 5.90 | 2 093 | 29.7 | 47.5 | 46.4 | 54.6 | 14.9 | 70.5 | 1.6 | 1.6 | 1.8 | 0.5 |
| `log` | 5.14 | 2 941 | 30.5 | 29.8 | 41.3 | 55.7 | 15.4 | 96.4 | 1.0 | 1.4 | 1.8 | 0.5 |
| `sin` | 19.9 | 8 323 | 89.5 | 103 | 50.8 | 59.2 | 175 | 93.0 | 1.2 | 0.6 | 0.7 | 2.0 |
| `cos` | 19.6 | 7 678 | 85.6 | 105 | 52.2 | 59.5 | 195 | 89.7 | 1.2 | 0.6 | 0.7 | 2.3 |
| `pow_int` | 19.7 | 407 | 20.8 | 20.9 | 27.5 | 215 | 49.3 | 19.5 | 1.0 | 1.3 | 10.3 | 2.4 |
| `pow_real` | 16.3 | 16 679 | 69.7 | 137 | 107 | 184 | 48.2 | 239 | 2.0 | 1.5 | 2.6 | 0.7 |
| `line_arith` | 1.61 | 984 | 29.2 | 29.4 | 41.7 | 87.9 | 88.5 | 33.7 | 1.0 | 1.4 | 3.0 | 3.0 |
| `line_trig` | 39.8 | 17 674 | 206 | 238 | 142 | 234 | 443 | 85.8 | 1.2 | 0.7 | 1.1 | 2.1 |
| `line_pow` | 29.2 | 3 544 | 88.8 | 101 | 139 | 363 | 130 | 39.9 | 1.1 | 1.6 | 4.1 | 1.5 |
| `shekel5` | 3.54 | 14 419 | 302 | 301 | 608 | 2 396 | 1 612 | 47.7 | 1.0 | 2.0 | 7.9 | 5.3 |
| `block5` | 66.5 | 25 944 | 328 | 383 | 325 | 650 | 615 | 79.0 | 1.2 | 1.0 | 2.0 | 1.9 |

#### Total time of the 1 000 000 operations (seconds)

| Operation | double (reference) | libieeep1788 | GAOL V5.0.0 | GAOL 4.3.2 | filib++ | Solaris Studio f90 | PROFIL/BIAS |
|---|---|---|---|---|---|---|---|
| `add` | 0.001 | 0.210 | 0.003 | 0.003 | 0.008 | 0.024 | 0.021 |
| `sub` | 0.001 | 0.205 | 0.003 | 0.003 | 0.008 | 0.024 | 0.022 |
| `mul` | 0.001 | 0.259 | 0.016 | 0.016 | 0.023 | 0.029 | 0.022 |
| `div` | 0.001 | 0.263 | 0.013 | 0.013 | 0.014 | 0.027 | 0.022 |
| `sqr` | 0.001 | 0.134 | 0.010 | 0.010 | 0.013 | 0.076 | 0.028 |
| `sqrt` | 0.002 | 0.142 | 0.012 | 0.012 | 0.023 | 0.037 | 0.006 |
| `exp` | 0.006 | 2.093 | 0.030 | 0.047 | 0.046 | 0.055 | 0.015 |
| `log` | 0.005 | 2.941 | 0.031 | 0.030 | 0.041 | 0.056 | 0.015 |
| `sin` | 0.020 | 8.323 | 0.089 | 0.103 | 0.051 | 0.059 | 0.175 |
| `cos` | 0.020 | 7.678 | 0.086 | 0.105 | 0.052 | 0.060 | 0.195 |
| `pow_int` | 0.020 | 0.407 | 0.021 | 0.021 | 0.027 | 0.215 | 0.049 |
| `pow_real` | 0.016 | 16.679 | 0.070 | 0.137 | 0.107 | 0.184 | 0.048 |
| `line_arith` | 0.002 | 0.984 | 0.029 | 0.029 | 0.042 | 0.088 | 0.089 |
| `line_trig` | 0.040 | 17.674 | 0.206 | 0.238 | 0.142 | 0.234 | 0.443 |
| `line_pow` | 0.029 | 3.544 | 0.089 | 0.101 | 0.139 | 0.363 | 0.130 |
| `shekel5` | 0.004 | 14.419 | 0.302 | 0.301 | 0.608 | 2.396 | 1.612 |
| `block5` | 0.067 | 25.944 | 0.328 | 0.383 | 0.325 | 0.650 | 0.615 |

#### Same results?

The sum of the midpoints of the results, relative to libieeep1788's, and the mean width of the results, with the relative excess of the other libraries over libieeep1788, whose bounds are the tightest:

| Operation | Σ midpoints (libieeep1788) | Δ GAOL V5.0.0 | Δ GAOL 4.3.2 | Δ filib++ | Δ Solaris Studio f90 | Δ PROFIL/BIAS | mean width (libieeep1788) | excess GAOL V5.0.0 | excess GAOL 4.3.2 | excess filib++ | excess Solaris Studio f90 | excess PROFIL/BIAS |
|---|---|---|---|---|---|---|---|---|---|---|---|---|
| `add` | 6215.245078 | 0.0e+00 | 0.0e+00 | 0.0e+00 | 0.0e+00 | 1.2e-14 | 0.144602 | 0.0e+00 | 0.0e+00 | 0.0e+00 | 0.0e+00 | 0.0e+00 |
| `sub` | 15708.8058 | 0.0e+00 | 0.0e+00 | 0.0e+00 | 0.0e+00 | 8.3e-15 | 0.144602 | 0.0e+00 | 0.0e+00 | 0.0e+00 | 0.0e+00 | 0.0e+00 |
| `mul` | -22357.9813 | 0.0e+00 | 0.0e+00 | 0.0e+00 | 0.0e+00 | 4.2e-14 | 0.721669 | 0.0e+00 | 0.0e+00 | 0.0e+00 | 0.0e+00 | 0.0e+00 |
| `div` | 1150.348145 | 0.0e+00 | 0.0e+00 | 0.0e+00 | 0.0e+00 | 4.1e-14 | 0.0556511 | 0.0e+00 | 0.0e+00 | 0.0e+00 | 0.0e+00 | 0.0e+00 |
| `sqr` | 33351536.53 | 0.0e+00 | 0.0e+00 | 0.0e+00 | 0.0e+00 | 0.0e+00 | 0.723458 | 0.0e+00 | 0.0e+00 | 0.0e+00 | 0.0e+00 | 0.0e+00 |
| `sqrt` | 2269319.522 | 0.0e+00 | 0.0e+00 | 0.0e+00 | 0.0e+00 | 0.0e+00 | 0.0173328 | 0.0e+00 | 0.0e+00 | 2.4e-14 | 0.0e+00 | 9.7e-14 |
| `exp` | 1113373157 | 0.0e+00 | -2.1e-16 | 2.1e-16 | 0.0e+00 | 0.0e+00 | 81.332 | 0.0e+00 | 2.6e-15 | 2.1e-14 | 7.3e-16 | 9.0e-15 |
| `log` | 1559051.81 | 0.0e+00 | 0.0e+00 | 4.5e-16 | 0.0e+00 | 1.5e-16 | 0.0185316 | 0.0e+00 | 0.0e+00 | 1.4e-13 | 0.0e+00 | 5.9e-14 |
| `sin` | 372.7406966 | 0.0e+00 | 1.5e-15 | 3.4e-15 | -9.2e-16 | -1.7e-14 | 0.0471741 | 0.0e+00 | 1.4e-15 | 3.7e-14 | 0.0e+00 | 7.4e-14 |
| `cos` | -55217.80218 | 0.0e+00 | -4.0e-16 | -1.3e-16 | 0.0e+00 | 2.5e-15 | 0.04418 | 0.0e+00 | 1.8e-15 | 4.6e-14 | 3.3e-16 | 9.7e-14 |
| `pow_int` | 763027.2411 | 0.0e+00 | 0.0e+00 | -9.2e-16 | 1.5e-15 | 2.4e-06 | 7.24104 | 0.0e+00 | 0.0e+00 | 4.2e-15 | 4.6e-15 | 1.1e-05 |
| `pow_real` | 24977959.15 | 0.0e+00 | 0.0e+00 | 1.2e-15 | 0.0e+00 | 0.0e+00 | 0.963128 | 0.0e+00 | 3.6e-15 | 2.4e-13 | 4.1e-15 | 1.2e-13 |
| `line_arith` | 11641.1468 | 0.0e+00 | 0.0e+00 | 0.0e+00 | 0.0e+00 | 3.5e-14 | 0.742132 | 0.0e+00 | 0.0e+00 | 0.0e+00 | 0.0e+00 | 0.0e+00 |
| `line_trig` | 33352445.64 | 0.0e+00 | 0.0e+00 | 0.0e+00 | 0.0e+00 | 0.0e+00 | 0.781627 | 0.0e+00 | 0.0e+00 | 3.4e-15 | 0.0e+00 | 6.4e-15 |
| `line_pow` | -200633868.5 | 0.0e+00 | 0.0e+00 | -5.9e-16 | 0.0e+00 | 2.1e-08 | 391.862 | 0.0e+00 | 1.5e-16 | 1.4e-15 | 1.5e-16 | 4.5e-07 |
| `shekel5` | -157759.691 | 0.0e+00 | 0.0e+00 | 0.0e+00 | 0.0e+00 | 0.0e+00 | 0.00992931 | 0.0e+00 | 0.0e+00 | 0.0e+00 | 0.0e+00 | -1.8e-16 |
| `block5` | 54765440.5 | 0.0e+00 | 2.7e-16 | 5.4e-16 | -2.7e-16 | -7.1e-07 | 456.259 | 0.0e+00 | 0.0e+00 | 3.1e-15 | 2.2e-15 | 5.6e-06 |

<!-- END GENERATED TABLES -->
