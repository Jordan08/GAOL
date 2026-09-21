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
| GAOL V5.0.0 (this branch) | Clang 18.1, `-O3 -mfma`, the flags of interval arithmetic; GAOL built by CMake in Release with `-mfma` (`GAOL_FMA`), the sources of CORE-MATH compiled into it, the rounding direction not preserved (`GAOL_PRESERVE_ROUNDING` off, its default) | Inline SSE2 operations, each checking that the rounding direction is upward and setting it when it is not; **every** elementary function CORE-MATH's, correctly rounded in that direction, so the bounds are the tightest ones and the direction is never switched |
| GAOL 4.2.3 (the last version of Frédéric Goualard) | Clang 18.1, `-O3 -mfma`, the flags of interval arithmetic; GAOL built by its configure from [his repository](https://github.com/goualard-f/GAOL) (cd0ee1a), static, with the flags of GAOL V5.0.0, which its configure gives to `g++` alone, the rounding direction not preserved; [mathlib 2.1.1](https://frederic.goualard.net/software/mathlib-2.1.1.tar.gz) from his site, built by Clang 18.1 with `-O3 -mfma -ffp-contract=off` | Inline SSE2 operations, which take the rounding direction upward, as `gaol::init()` sets it, without checking it; elementary functions computed by mathlib, correctly rounded to nearest, then moved one double outward, the rounding direction set to nearest and back upward for each; `pow(x, y)` as exp(y·log(x)) |
| libieeep1788 | Clang 18.1, `-O3 -mfma`; MPFR 4.2.1 and GMP 6.3.0 built by Clang 18.1 with `-O3 -mfma` | Each bound computed by MPFR, correctly rounded |
| filib++ | Clang 18.1, `-O3 -mfma`, the flags of interval arithmetic; filib++ 3.0.2.2, the archive IBEX distributes, built by Clang 18.1 in C++11 with `-O3 -mfma` | `interval<double, native_switched, i_mode_extended_flag>`, as in IBEX: inline operations, the rounding direction set and restored by each; elementary functions of its own |
| PROFIL/BIAS | Clang 18.1, `-O3 -mfma`, the flags of interval arithmetic; PROFIL/BIAS 2.0.8 built with its `x86-64-Linux-compat-gcc` configuration, by Clang 18.1 in C++11 with `-O3 -mfma -ffp-contract=off` | `INTERVAL`, whose operations are calls to the BIAS library, each setting the rounding direction downward then upward and back to nearest; elementary functions from the libm, moved outward |
| Solaris Studio | Sun Fortran 95 8.7 (Solaris Studio 12.4, 2014), `-O3 -xia` | Calls to `libsunimath` |

## What the timings show

The two versions of GAOL are measured side by side: **GAOL V5.0.0**, this
branch, which bounds every elementary function with CORE-MATH, and **GAOL
4.2.3**, the last version of Frédéric Goualard, which GAOL V5.0.0 continues
and which bounds them with mathlib. Both are built without the preservation of
the rounding direction, the default of GAOL V5.0.0. Everything was built and
run again for this table with **Clang 18.1** — GMP, MPFR, libieeep1788,
filib++ (from the archive IBEX distributes) and PROFIL/BIAS as well as the two
GAOL — so that one compiler answers for every C and C++ library here. Only
Solaris Studio keeps its own, by nature.

**What GAOL V5.0.0 changes.** The elementary functions, which mathlib used to
bound, are faster:

| | GAOL V5.0.0 | GAOL 4.2.3 | |
|---|---:|---:|---|
| `log` | 29.8 ns | 69.8 ns | 2.3 times faster |
| `pow(x, y)` | 69.5 | 136 | 2.0 |
| `exp` | 29.9 | 52.5 | 1.8 |
| `cos` | 84.7 | 102 | 1.2 |
| `sin` | 88.1 | 105 | 1.2 |
| line of sin and cos | 204 | 230 | 1.1 |
| five-line block | 320 | 343 | 1.07 |
| line of powers | 85.5 | 90.8 | 1.06 |

Two things go at once with mathlib: the outward move of each bound, and the
two changes of rounding direction each function made, to nearest before
mathlib and back upward after. **And the bounds are tighter, not looser**: in
the last table below, GAOL V5.0.0's results are no wider than libieeep1788's,
which computes every bound with MPFR, for **every one of the seventeen
operations** (excess 0.0e+00), where GAOL 4.2.3 is wider on the square root,
`exp`, `log`, `sin`, `cos`, both powers and the lines that use them.

The arithmetic costs GAOL V5.0.0 a little more: Shekel 5 takes 301 ns against
263, the arithmetic line 29.1 against 27.6, the square root 11.8 ns against 8.8
and `pow(x, 3)` 20.7 against 13.9, the operators themselves being within 5 %.
What GAOL V5.0.0 does more there: each operation checks that the rounding
direction is upward, with an addition, and sets it when it is not, where GAOL
4.2.3 takes it upward on trust; the square root is the tightest interval
whatever the rounding of the C library's `sqrt`, where GAOL 4.2.3's is wider
(excess 1.4e-14); and the integer power is computed from exact products
([issue #7](https://github.com/Jordan08/GAOL/issues/7)), the tightest, where
GAOL 4.2.3 rounds each product outward (excess 4.2e-15). The check is what a
program needs that changes the rounding direction: the benchmark sums the
results rounding to nearest, and GAOL 4.2.3's own `midpoint()` leaves the
direction to nearest. Left so, the operations of GAOL 4.2.3 timed next gave
bounds narrower than libieeep1788's, which did not enclose the exact results;
the benchmark now sets the direction back after its sums
(`code/bench_common.h`).

**Arithmetic.** GAOL is the fastest on + and −, 3.2 and 3.4 ns for GAOL V5.0.0
(3.1 and 3.2 for GAOL 4.2.3), about twice as fast as filib++ (7.9 ns) and six
to seven times as fast as PROFIL/BIAS (22 ns) and Solaris Studio (24 ns); × and
÷ take GAOL V5.0.0 16 and 12.5 ns, against 23 and 14 ns for filib++, 22 ns for
PROFIL/BIAS and 27 to 29 ns for Solaris Studio. An addition of intervals costs
GAOL V5.0.0 2.3 times an addition of doubles: two additions, and the check that
the rounding direction is upward, where filib++ sets the rounding direction and
restores it, and PROFIL/BIAS calls a function of BIAS that sets it downward,
then upward, then back to nearest — which is why its four arithmetic operations
all take about the same time. The integer power `pow(x, 3)` takes GAOL V5.0.0
21 ns, computed from exact products, whose `fma()` is one instruction with
`-mfma`, and GAOL 4.2.3 14 ns: filib++'s `power(x, 3)` takes 26.5 ns,
PROFIL/BIAS's 49 ns and Solaris Studio's `x**3` 214 ns; `sqr` takes GAOL
V5.0.0 9.5 ns, against 12 for filib++, 28 for PROFIL/BIAS and 76 for Solaris
Studio.

**Elementary functions.** PROFIL/BIAS is the fastest on the square root
(5.5 ns), exp (15 ns), log (15.5 ns) and the real power `pow(x, y)` (48 ns),
which it computes from the libm of the system, moved outward, without correct
rounding: its bounds are among the widest (below), and its sin and cos,
computed by its own argument reduction, are the slowest, 173 and 194 ns. GAOL
V5.0.0 comes next on exp and log, about 30 ns each — faster than filib++ (46.5
and 41 ns) and than Solaris Studio (55 and 56 ns) — with **correctly rounded**
values, where the others are not. filib++ is the fastest on sin and cos (51 and
52 ns) with its own polynomials, then Solaris Studio (59 ns), then GAOL V5.0.0
(88 and 85 ns): GAOL pays there for correct rounding over the whole range, and
for dividing the bounds by an interval enclosing π to find where the function is
monotonic, asking CORE-MATH for the signs of the derivative where the division
cannot tell ([issue #6](https://github.com/Jordan08/GAOL/issues/6)). On
`pow(x, y)` GAOL V5.0.0 takes 70 ns, against 107 for filib++, 136 for GAOL
4.2.3 and 184 for Solaris Studio.

**Formulas.** GAOL is the fastest on the arithmetic line (29 ns for GAOL V5.0.0
and 28 for GAOL 4.2.3, against 41 for filib++, 88 for Solaris Studio and 89 for
PROFIL/BIAS), on the line of powers (86 and 91 ns, against 134, 360 and 130)
and on Shekel 5 (301 and 263 ns, against 600, 2 365 and 1 590: Shekel 5 is 20
squares, 45 additions and subtractions and 5 divisions, and its squares cost
Solaris Studio 76 ns each and PROFIL/BIAS 28). filib++ is the fastest on the
line of sin and cos (142 ns, against 204 for GAOL V5.0.0, 229 for Solaris
Studio, 230 for GAOL 4.2.3 and 436 for PROFIL/BIAS) and, by one nanosecond, on
the five-line block (319 ns, against 320 for GAOL V5.0.0, 343 for GAOL 4.2.3,
608 for PROFIL/BIAS and 642 for Solaris Studio), where its elementary functions
weigh most.

**libieeep1788** is 12 to 130 times slower than GAOL V5.0.0: every bound is an
MPFR computation, about 200 ns for an addition and 8 to 9 µs for sin, cos and
the real power. Its own README warns that its focus is correctness, not speed.
GAOL V5.0.0 now gives **the same bounds as it does**, tightest everywhere, at
between a twelfth and a hundred-and-thirtieth of the time.

**The results** are the same: the sums of the midpoints of the million results
agree to 4e-15 relatively, apart from PROFIL/BIAS's (below) and the sines of
GAOL 4.2.3, 1.4e-13 apart, their sum being small, 373 for a million values,
which makes 5e-17 per result. The arithmetic operations give the tightest
intervals in every library, and so do the square roots, except those of GAOL
4.2.3, filib++ and PROFIL/BIAS. Of the elementary functions, **only GAOL
V5.0.0's are the tightest on every operation**: on average the others are wider
than libieeep1788's by up to 5.9e-14 relatively with GAOL 4.2.3 (the real
power, computed as exp(y·log(x))), 4e-15 to 2.4e-13 with filib++ (1.4e-13 for
log, 2.4e-13 for the real power), 9e-15 to 1.2e-13 with PROFIL/BIAS, and at
most 5e-15 with Solaris Studio. PROFIL/BIAS's integer power is far wider,
1.1e-05 on average: it computes `Power(x, n)` as exp(n log x), where the others
multiply. Its arithmetic results have midpoints that differ by about 1e-14
relatively while their widths are the tightest: the intervals are the same, but
its `Mid` computes inf + (sup − inf)/2 **rounded upward**, one double above the
midpoint for 4 % of the sums of the benchmark, where the others round to
nearest.

The timings were measured on a laptop, each time being the best of several runs
spread over three rounds: the machine slowed down now and then for a few
seconds, which made some operations up to 2.5 times slower in a single round.
libieeep1788's times, from a single run of each operation in each round, are the
least steady. Solaris Studio's and PROFIL/BIAS's operations are calls into
libraries, where the compiler inlines GAOL's and filib++'s.

## The timings

<!-- BEGIN GENERATED TABLES (doc/compare/code/bench.py) -->

Measured on:

```
Date:            2026-09-21
Processor:       11th Gen Intel(R) Core(TM) i7-1185G7 @ 3.00GHz (taskset -c 2)
System:          Linux 5.15.0-191-generic, Ubuntu 20.04.6 LTS
C++ compiler:    Ubuntu clang version 18.1.8 (11~20.04.2)
C++ flags:       -std=c++11 -O3 -DNDEBUG -mfma (GAOL: -frounding-math -fno-fast-math -ffp-contract=off -msse2 -msse3 -mfma; libieeep1788, filib++ and PROFIL/BIAS: -frounding-math -fno-fast-math -ffp-contract=off)
Fortran:         f90: Sun Fortran 95 8.7 Linux_i386 2014/10/20, flags: -O3 -xia
GAOL V5.0.0:     the branch of this checkout (v4.3.2-84-gbb6f7e4-dirty), CMake Release, CORE-MATH of 3rd/math-core compiled into the library, the rounding direction not preserved
GAOL 4.2.3:      the last version of Frédéric Goualard (cd0ee1a of https://github.com/goualard-f/GAOL), its configure, the rounding direction not preserved, mathlib 2.1.1
libieeep1788:    1f10b89, MPFR 4.2.1, GMP 6.3.0
filib++:         3.0.2.2, interval<double, native_switched, i_mode_extended_flag>
PROFIL/BIAS:     2.0.8, x86-64-Linux-compat-gcc configuration, built by clang-18 and clang++-18
```

1 000 000 operations of each kind, on the same intervals. Each time is the best of 3 rounds, each program being run in turn with the others: 15 runs for double (reference), 3 runs for libieeep1788, 15 runs for GAOL V5.0.0, 15 runs for GAOL 4.2.3, 15 runs for filib++, 15 runs for Solaris Studio f90, 15 runs for PROFIL/BIAS in all.

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

| Operation | double (reference) | libieeep1788 | GAOL V5.0.0 | GAOL 4.2.3 | filib++ | Solaris Studio f90 | PROFIL/BIAS | libieeep1788 / GAOL V5.0.0 | GAOL 4.2.3 / GAOL V5.0.0 | filib++ / GAOL V5.0.0 | Solaris Studio f90 / GAOL V5.0.0 | PROFIL/BIAS / GAOL V5.0.0 |
|---|---|---|---|---|---|---|---|---|---|---|---|---|
| `add` | 1.39 | 207 | 3.24 | 3.11 | 7.90 | 24.0 | 21.5 | 63.9 | 1.0 | 2.4 | 7.4 | 6.6 |
| `sub` | 1.29 | 204 | 3.38 | 3.22 | 7.91 | 23.7 | 21.5 | 60.4 | 1.0 | 2.3 | 7.0 | 6.4 |
| `mul` | 1.33 | 260 | 16.1 | 15.8 | 23.4 | 29.1 | 22.0 | 16.1 | 1.0 | 1.5 | 1.8 | 1.4 |
| `div` | 1.26 | 245 | 12.5 | 12.3 | 13.7 | 27.0 | 21.7 | 19.6 | 1.0 | 1.1 | 2.2 | 1.7 |
| `sqr` | 0.89 | 128 | 9.52 | 9.07 | 12.4 | 75.7 | 27.5 | 13.4 | 1.0 | 1.3 | 8.0 | 2.9 |
| `sqrt` | 1.97 | 146 | 11.8 | 8.78 | 22.8 | 36.7 | 5.52 | 12.3 | 0.7 | 1.9 | 3.1 | 0.5 |
| `exp` | 5.86 | 2 021 | 29.9 | 52.5 | 46.5 | 54.5 | 15.4 | 67.6 | 1.8 | 1.6 | 1.8 | 0.5 |
| `log` | 5.11 | 2 688 | 29.8 | 69.8 | 41.3 | 55.6 | 15.5 | 90.1 | 2.3 | 1.4 | 1.9 | 0.5 |
| `sin` | 19.9 | 7 797 | 88.1 | 105 | 51.2 | 59.1 | 173 | 88.5 | 1.2 | 0.6 | 0.7 | 2.0 |
| `cos` | 19.3 | 8 029 | 84.7 | 102 | 51.8 | 59.5 | 194 | 94.8 | 1.2 | 0.6 | 0.7 | 2.3 |
| `pow_int` | 19.7 | 342 | 20.7 | 13.9 | 26.5 | 214 | 49.3 | 16.6 | 0.7 | 1.3 | 10.3 | 2.4 |
| `pow_real` | 16.2 | 9 158 | 69.5 | 136 | 107 | 184 | 47.6 | 132 | 2.0 | 1.5 | 2.6 | 0.7 |
| `line_arith` | 1.35 | 972 | 29.1 | 27.6 | 41.4 | 87.6 | 88.8 | 33.4 | 0.9 | 1.4 | 3.0 | 3.0 |
| `line_trig` | 39.0 | 15 513 | 204 | 230 | 142 | 229 | 436 | 75.9 | 1.1 | 0.7 | 1.1 | 2.1 |
| `line_pow` | 29.1 | 3 420 | 85.5 | 90.8 | 134 | 360 | 130 | 40.0 | 1.1 | 1.6 | 4.2 | 1.5 |
| `shekel5` | 3.53 | 13 254 | 301 | 263 | 600 | 2 365 | 1 590 | 44.1 | 0.9 | 2.0 | 7.9 | 5.3 |
| `block5` | 69.2 | 24 507 | 320 | 343 | 319 | 642 | 608 | 76.6 | 1.1 | 1.0 | 2.0 | 1.9 |

#### Total time of the 1 000 000 operations (seconds)

| Operation | double (reference) | libieeep1788 | GAOL V5.0.0 | GAOL 4.2.3 | filib++ | Solaris Studio f90 | PROFIL/BIAS |
|---|---|---|---|---|---|---|---|
| `add` | 0.001 | 0.207 | 0.003 | 0.003 | 0.008 | 0.024 | 0.022 |
| `sub` | 0.001 | 0.204 | 0.003 | 0.003 | 0.008 | 0.024 | 0.022 |
| `mul` | 0.001 | 0.260 | 0.016 | 0.016 | 0.023 | 0.029 | 0.022 |
| `div` | 0.001 | 0.245 | 0.012 | 0.012 | 0.014 | 0.027 | 0.022 |
| `sqr` | 0.001 | 0.128 | 0.010 | 0.009 | 0.012 | 0.076 | 0.027 |
| `sqrt` | 0.002 | 0.146 | 0.012 | 0.009 | 0.023 | 0.037 | 0.006 |
| `exp` | 0.006 | 2.021 | 0.030 | 0.052 | 0.046 | 0.054 | 0.015 |
| `log` | 0.005 | 2.688 | 0.030 | 0.070 | 0.041 | 0.056 | 0.015 |
| `sin` | 0.020 | 7.797 | 0.088 | 0.105 | 0.051 | 0.059 | 0.173 |
| `cos` | 0.019 | 8.029 | 0.085 | 0.102 | 0.052 | 0.059 | 0.194 |
| `pow_int` | 0.020 | 0.342 | 0.021 | 0.014 | 0.027 | 0.214 | 0.049 |
| `pow_real` | 0.016 | 9.158 | 0.070 | 0.136 | 0.107 | 0.184 | 0.048 |
| `line_arith` | 0.001 | 0.972 | 0.029 | 0.028 | 0.041 | 0.088 | 0.089 |
| `line_trig` | 0.039 | 15.513 | 0.204 | 0.230 | 0.142 | 0.229 | 0.436 |
| `line_pow` | 0.029 | 3.420 | 0.085 | 0.091 | 0.134 | 0.360 | 0.130 |
| `shekel5` | 0.004 | 13.254 | 0.301 | 0.263 | 0.600 | 2.365 | 1.590 |
| `block5` | 0.069 | 24.507 | 0.320 | 0.343 | 0.319 | 0.642 | 0.608 |

#### Same results?

The sum of the midpoints of the results, relative to libieeep1788's, and the mean width of the results, with the relative excess of the other libraries over libieeep1788, whose bounds are the tightest:

| Operation | Σ midpoints (libieeep1788) | Δ GAOL V5.0.0 | Δ GAOL 4.2.3 | Δ filib++ | Δ Solaris Studio f90 | Δ PROFIL/BIAS | mean width (libieeep1788) | excess GAOL V5.0.0 | excess GAOL 4.2.3 | excess filib++ | excess Solaris Studio f90 | excess PROFIL/BIAS |
|---|---|---|---|---|---|---|---|---|---|---|---|---|
| `add` | 6215.245078 | 0.0e+00 | 0.0e+00 | 0.0e+00 | 0.0e+00 | 1.2e-14 | 0.144602 | 0.0e+00 | 0.0e+00 | 0.0e+00 | 0.0e+00 | 0.0e+00 |
| `sub` | 15708.8058 | 0.0e+00 | 0.0e+00 | 0.0e+00 | 0.0e+00 | 8.3e-15 | 0.144602 | 0.0e+00 | 0.0e+00 | 0.0e+00 | 0.0e+00 | 0.0e+00 |
| `mul` | -22357.9813 | 0.0e+00 | 0.0e+00 | 0.0e+00 | 0.0e+00 | 4.2e-14 | 0.721669 | 0.0e+00 | 0.0e+00 | 0.0e+00 | 0.0e+00 | 0.0e+00 |
| `div` | 1150.348145 | 0.0e+00 | 0.0e+00 | 0.0e+00 | 0.0e+00 | 4.1e-14 | 0.0556511 | 0.0e+00 | 0.0e+00 | 0.0e+00 | 0.0e+00 | 0.0e+00 |
| `sqr` | 33351536.53 | 0.0e+00 | 0.0e+00 | 0.0e+00 | 0.0e+00 | 0.0e+00 | 0.723458 | 0.0e+00 | 0.0e+00 | 0.0e+00 | 0.0e+00 | 0.0e+00 |
| `sqrt` | 2269319.522 | 0.0e+00 | -2.1e-16 | 0.0e+00 | 0.0e+00 | 0.0e+00 | 0.0173328 | 0.0e+00 | 1.4e-14 | 2.4e-14 | 0.0e+00 | 9.7e-14 |
| `exp` | 1113373157 | 0.0e+00 | -2.1e-16 | 2.1e-16 | 0.0e+00 | 0.0e+00 | 81.332 | 0.0e+00 | 2.6e-15 | 2.1e-14 | 7.3e-16 | 9.0e-15 |
| `log` | 1559051.81 | 0.0e+00 | 0.0e+00 | 4.5e-16 | 0.0e+00 | 1.5e-16 | 0.0185316 | 0.0e+00 | 1.5e-14 | 1.4e-13 | 0.0e+00 | 5.9e-14 |
| `sin` | 372.7406966 | 0.0e+00 | 1.4e-13 | 3.4e-15 | -9.2e-16 | -1.7e-14 | 0.0471741 | 0.0e+00 | 1.3e-14 | 3.7e-14 | 0.0e+00 | 7.4e-14 |
| `cos` | -55217.80218 | 0.0e+00 | -4.0e-16 | -1.3e-16 | 0.0e+00 | 2.5e-15 | 0.04418 | 0.0e+00 | 1.8e-15 | 4.6e-14 | 3.3e-16 | 9.7e-14 |
| `pow_int` | 763027.2411 | 0.0e+00 | -9.2e-16 | -9.2e-16 | 1.5e-15 | 2.4e-06 | 7.24104 | 0.0e+00 | 4.2e-15 | 4.2e-15 | 4.6e-15 | 1.1e-05 |
| `pow_real` | 24977959.15 | 0.0e+00 | 0.0e+00 | 1.2e-15 | 0.0e+00 | 0.0e+00 | 0.963128 | 0.0e+00 | 5.9e-14 | 2.4e-13 | 4.1e-15 | 1.2e-13 |
| `line_arith` | 11641.1468 | 1.6e-16 | 1.6e-16 | 0.0e+00 | 0.0e+00 | 3.5e-14 | 0.742132 | 0.0e+00 | 0.0e+00 | 0.0e+00 | 0.0e+00 | 0.0e+00 |
| `line_trig` | 33352445.64 | 0.0e+00 | 0.0e+00 | 0.0e+00 | 0.0e+00 | 0.0e+00 | 0.781627 | 0.0e+00 | 3.0e-16 | 3.4e-15 | 0.0e+00 | 6.4e-15 |
| `line_pow` | -200633868.5 | 0.0e+00 | 0.0e+00 | -5.9e-16 | 0.0e+00 | 2.1e-08 | 391.862 | 0.0e+00 | 3.0e-16 | 1.4e-15 | 1.5e-16 | 4.5e-07 |
| `shekel5` | -157759.691 | 0.0e+00 | 0.0e+00 | 0.0e+00 | 0.0e+00 | 0.0e+00 | 0.00992931 | 0.0e+00 | 0.0e+00 | 0.0e+00 | 0.0e+00 | -1.8e-16 |
| `block5` | 54765440.5 | 0.0e+00 | 2.7e-16 | 5.4e-16 | -2.7e-16 | -7.1e-07 | 456.259 | 0.0e+00 | 3.3e-15 | 3.1e-15 | 2.2e-15 | 5.6e-06 |

<!-- END GENERATED TABLES -->
