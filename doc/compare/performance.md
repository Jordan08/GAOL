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
| GAOL | Clang 18.1, `-O3 -mfma`, the flags of interval arithmetic; GAOL, CORE-MATH and mathlib built by CMake in Release, with `-mfma` (`GAOL_FMA`) | Inline SSE2 operations, the rounding direction set upward; elementary functions computed by mathlib and moved outward, but log, CORE-MATH's, correctly rounded upward |
| libieeep1788 | Clang 18.1, `-O3 -mfma`; MPFR 4.2.1 and GMP 6.3.0 built by Clang 18.1 with `-O3 -mfma` | Each bound computed by MPFR, correctly rounded |
| filib++ | Clang 18.1, `-O3 -mfma`, the flags of interval arithmetic; filib++ 3.0.2.2 built by Clang 18.1 in C++11 with `-O3 -mfma` | `interval<double, native_switched, i_mode_extended_flag>`, as in IBEX: inline operations, the rounding direction set and restored by each; elementary functions of its own |
| PROFIL/BIAS | Clang 18.1, `-O3 -mfma`, the flags of interval arithmetic; PROFIL/BIAS 2.0.8 built with its `x86-64-Linux-compat-gcc` configuration, by Clang 18.1 in C++11 with `-O3 -mfma -ffp-contract=off` | `INTERVAL`, whose operations are calls to the BIAS library, each setting the rounding direction downward then upward and back to nearest; elementary functions from the libm, moved outward |
| Solaris Studio | Sun Fortran 95 8.7 (Solaris Studio 12.4, 2014), `-O3 -xia` | Calls to `libsunimath` |

## What the timings show

**Arithmetic.** GAOL is the fastest on + and −, 3.3 and 3.4 ns, more than
twice as fast as filib++ (7.9 ns) and six to seven times as fast as
PROFIL/BIAS (22 ns) and Solaris Studio (24 ns); × and ÷ take GAOL 16 and
13 ns, against 23 and 14 ns for filib++, 22 ns for PROFIL/BIAS and 27 to
29 ns for Solaris Studio. An addition of intervals costs GAOL 2.5 times an
addition of doubles: two additions, and the check that the rounding direction
is upward, where filib++ sets the rounding direction and restores it, and
PROFIL/BIAS calls a function of BIAS that sets it downward, then upward, then
back to nearest — which is why its four arithmetic operations all take about
the same time. The integer power `pow(x, 3)` takes GAOL 21 ns, computed from
exact products since [issue #7](https://github.com/Jordan08/GAOL/issues/7),
whose `fma()` is one instruction with `-mfma` (29 ns without it; 14 ns before
issue #7, measured with GCC 9.4, with the products rounded one by one, for
bounds up to 4 doubles wider): filib++'s `power(x, 3)` takes 28 ns,
PROFIL/BIAS's 49 ns and Solaris Studio's `x**3` 214 ns; `sqr` takes GAOL
10 ns, against 13 for filib++, 28 for PROFIL/BIAS and 76 for Solaris Studio.

**Elementary functions.** PROFIL/BIAS is the fastest on the square root
(5.6 ns), exp (15 ns), log (15 ns) and the real power `pow(x, y)` (48 ns),
which it computes from the libm of the system, moved outward, without correct
rounding: its bounds are the widest of the five (below), and its sin and cos,
computed by its own argument reduction, are the slowest, 172 and 194 ns. It is
compiled with `-ffp-contract=off`: with `-mfma` and the contraction Clang does
by default, its outward roundings x(1 + ε) + η became fused multiply-adds whose
addend η is subnormal, and these functions took 90 ns more. filib++ is the
fastest on sin and cos (51 and 52 ns) with its own polynomials, and takes 46 ns
for exp, 41 ns for log and 107 ns for `pow(x, y)`. Solaris Studio follows,
with 59 to 60 ns for sin and cos, 56 ns for log, 55 ns for exp and 184 ns for
`pow(x, y)`, then GAOL, with 103 and 105 ns for sin and cos, 47 ns for exp and
135 ns for `pow(x, y)`, but 30 ns for log, faster than filib++'s and Solaris
Studio's. GAOL's log is CORE-MATH's, correctly rounded in the upward rounding
GAOL computes in, which gives the tightest bounds without switching the
rounding direction (62 ns with mathlib's log). Its other functions compute
each bound with mathlib, correctly rounded to nearest, before moving it one
double outward, and set the rounding direction twice for each function, to
nearest before mathlib and upward after; sin and cos also divide the bounds
by an interval enclosing π to find where they are monotonic, and ask mathlib
for the signs of the derivative where the division cannot tell
([issue #6](https://github.com/Jordan08/GAOL/issues/6)), and `pow(x, y)` takes
mathlib's pow at two corners of the box.

**Formulas.** GAOL is the fastest on the arithmetic line (29 ns, against 42
for filib++, 88 for Solaris Studio and 88 for PROFIL/BIAS), on the line of
powers (99 ns, against 135, 361 and 130) and on Shekel 5 (301 ns, against
599, 2 365 and 1 592: Shekel 5 is 20 squares, 45 additions and subtractions
and 5 divisions, and its squares cost Solaris Studio 76 ns each and
PROFIL/BIAS 28). filib++ is the fastest on the line of sin and cos (140 ns,
against 230 for Solaris Studio, 237 for GAOL and 439 for PROFIL/BIAS) and on
the five-line block (320 ns, against 379 for GAOL, 611 for PROFIL/BIAS and
643 for Solaris Studio), where its elementary functions weigh most.

**libieeep1788** is 12 to 91 times slower than GAOL: every bound is an MPFR
computation, about 200 ns for an addition and 7 to 9 µs for sin, cos and the
real power. Its own README warns that its focus is correctness, not speed.

**The results** are the same: the sums of the midpoints of the million results
agree to 4e-15 relatively, apart from PROFIL/BIAS's integer powers (2.4e-06,
below), and the arithmetic operations give the tightest intervals in the five
libraries, and so do the square roots, except filib++'s and PROFIL/BIAS's, and
GAOL's log. On average, the other elementary functions and powers are wider
than libieeep1788's by at most 3.6e-15 relatively with GAOL, 4e-15 to 2.4e-13
with filib++ (1.4e-13 for log, 2.4e-13 for the real power), 9e-15 to 1.2e-13
with PROFIL/BIAS, and at most 5e-15 with Solaris Studio. PROFIL/BIAS's integer
power is far wider, 1.1e-05 on average: it computes `Power(x, n)` as
exp(n log x), where the four others multiply. Its arithmetic results have
midpoints that differ by about 1e-14 relatively while their widths are the
tightest: the intervals are the same, but its `Mid` computes
inf + (sup − inf)/2 **rounded upward**, one double above the midpoint for 4 %
of the sums of the benchmark, where the others round to nearest.

The timings were measured on a laptop, the program pinned to one processor,
each time being the best of several runs spread over three rounds: the
machine slowed down now and then for a few seconds, which made some operations
up to 2.5 times slower in a single round. libieeep1788's times, from a single
run of each operation in each round, are the least steady: its Shekel 5 took
15.4 µs here, and 13.4 µs in the run just before. Solaris Studio's and
PROFIL/BIAS's operations are calls into libraries, where Clang inlines GAOL's
and filib++'s.

## The timings

<!-- BEGIN GENERATED TABLES (doc/compare/code/bench.py) -->

Measured on:

```
Date:            2026-09-18
Processor:       11th Gen Intel(R) Core(TM) i7-1185G7 @ 3.00GHz (taskset -c 2)
System:          Linux 5.15.0-191-generic, Ubuntu 20.04.6 LTS
C++ compiler:    Ubuntu clang version 18.1.8 (11~20.04.2)
C++ flags:       -std=c++11 -O3 -DNDEBUG -mfma (GAOL: -frounding-math -fno-fast-math -ffp-contract=off -msse2 -msse3 -mfma; libieeep1788, filib++ and PROFIL/BIAS: -frounding-math -fno-fast-math -ffp-contract=off)
Fortran:         f90: Sun Fortran 95 8.7 Linux_i386 2014/10/20, flags: -O3 -xia
GAOL:            v4.3.0-58-g86a332b, CMake Release, mathlib 2.1.1 of 3rd/mathlib
libieeep1788:    1f10b89, MPFR 4.2.1, GMP 6.3.0
filib++:         3.0.2.2, interval<double, native_switched, i_mode_extended_flag>
PROFIL/BIAS:     2.0.8, x86-64-Linux-compat-gcc configuration, built by clang-18 and clang++-18
```

1 000 000 operations of each kind, on the same intervals. Each time is the best of 3 rounds, each program being run in turn with the others: 15 runs for double (reference), 3 runs for libieeep1788, 15 runs for GAOL, 15 runs for filib++, 15 runs for Solaris Studio f90, 15 runs for PROFIL/BIAS in all.

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

| Operation | double (reference) | libieeep1788 | GAOL | filib++ | Solaris Studio f90 | PROFIL/BIAS | libieeep1788 / GAOL | filib++ / GAOL | Solaris Studio f90 / GAOL | PROFIL/BIAS / GAOL |
|---|---|---|---|---|---|---|---|---|---|---|
| `add` | 1.31 | 205 | 3.28 | 7.94 | 23.9 | 21.5 | 62.5 | 2.4 | 7.3 | 6.6 |
| `sub` | 1.28 | 204 | 3.40 | 7.88 | 23.9 | 21.5 | 60.1 | 2.3 | 7.0 | 6.3 |
| `mul` | 1.30 | 259 | 16.2 | 23.4 | 28.9 | 22.0 | 16.0 | 1.4 | 1.8 | 1.4 |
| `div` | 1.19 | 254 | 12.5 | 13.6 | 26.8 | 21.7 | 20.3 | 1.1 | 2.1 | 1.7 |
| `sqr` | 0.91 | 129 | 9.64 | 12.5 | 75.6 | 27.8 | 13.4 | 1.3 | 7.8 | 2.9 |
| `sqrt` | 1.97 | 140 | 11.9 | 22.8 | 36.7 | 5.55 | 11.7 | 1.9 | 3.1 | 0.5 |
| `exp` | 5.87 | 2 027 | 47.4 | 46.2 | 54.6 | 14.7 | 42.7 | 1.0 | 1.2 | 0.3 |
| `log` | 5.10 | 2 716 | 29.8 | 41.0 | 55.6 | 15.4 | 91.3 | 1.4 | 1.9 | 0.5 |
| `sin` | 19.9 | 7 917 | 103 | 50.6 | 59.2 | 172 | 76.9 | 0.5 | 0.6 | 1.7 |
| `cos` | 19.3 | 6 949 | 105 | 51.8 | 59.5 | 194 | 66.0 | 0.5 | 0.6 | 1.8 |
| `pow_int` | 19.6 | 351 | 20.6 | 27.5 | 214 | 49.3 | 17.0 | 1.3 | 10.4 | 2.4 |
| `pow_real` | 16.2 | 9 356 | 135 | 107 | 184 | 47.9 | 69.4 | 0.8 | 1.4 | 0.4 |
| `line_arith` | 1.44 | 962 | 29.1 | 41.9 | 87.7 | 88.4 | 33.1 | 1.4 | 3.0 | 3.0 |
| `line_trig` | 38.9 | 15 630 | 237 | 140 | 230 | 439 | 65.8 | 0.6 | 1.0 | 1.8 |
| `line_pow` | 29.1 | 3 420 | 98.9 | 135 | 361 | 130 | 34.6 | 1.4 | 3.6 | 1.3 |
| `shekel5` | 3.55 | 15 431 | 301 | 599 | 2 365 | 1 592 | 51.3 | 2.0 | 7.9 | 5.3 |
| `block5` | 66.4 | 23 536 | 379 | 320 | 643 | 611 | 62.1 | 0.8 | 1.7 | 1.6 |

#### Total time of the 1 000 000 operations (seconds)

| Operation | double (reference) | libieeep1788 | GAOL | filib++ | Solaris Studio f90 | PROFIL/BIAS |
|---|---|---|---|---|---|---|
| `add` | 0.001 | 0.205 | 0.003 | 0.008 | 0.024 | 0.022 |
| `sub` | 0.001 | 0.204 | 0.003 | 0.008 | 0.024 | 0.022 |
| `mul` | 0.001 | 0.259 | 0.016 | 0.023 | 0.029 | 0.022 |
| `div` | 0.001 | 0.254 | 0.013 | 0.014 | 0.027 | 0.022 |
| `sqr` | 0.001 | 0.129 | 0.010 | 0.012 | 0.076 | 0.028 |
| `sqrt` | 0.002 | 0.140 | 0.012 | 0.023 | 0.037 | 0.006 |
| `exp` | 0.006 | 2.027 | 0.047 | 0.046 | 0.055 | 0.015 |
| `log` | 0.005 | 2.716 | 0.030 | 0.041 | 0.056 | 0.015 |
| `sin` | 0.020 | 7.917 | 0.103 | 0.051 | 0.059 | 0.172 |
| `cos` | 0.019 | 6.949 | 0.105 | 0.052 | 0.059 | 0.194 |
| `pow_int` | 0.020 | 0.351 | 0.021 | 0.028 | 0.214 | 0.049 |
| `pow_real` | 0.016 | 9.356 | 0.135 | 0.107 | 0.184 | 0.048 |
| `line_arith` | 0.001 | 0.962 | 0.029 | 0.042 | 0.088 | 0.088 |
| `line_trig` | 0.039 | 15.630 | 0.237 | 0.140 | 0.230 | 0.439 |
| `line_pow` | 0.029 | 3.420 | 0.099 | 0.135 | 0.361 | 0.130 |
| `shekel5` | 0.004 | 15.431 | 0.301 | 0.599 | 2.365 | 1.592 |
| `block5` | 0.066 | 23.536 | 0.379 | 0.320 | 0.643 | 0.611 |

#### Same results?

The sum of the midpoints of the results, relative to libieeep1788's, and the mean width of the results, with the relative excess of the other libraries over libieeep1788, whose bounds are the tightest:

| Operation | Σ midpoints (libieeep1788) | Δ GAOL | Δ filib++ | Δ Solaris Studio f90 | Δ PROFIL/BIAS | mean width (libieeep1788) | excess GAOL | excess filib++ | excess Solaris Studio f90 | excess PROFIL/BIAS |
|---|---|---|---|---|---|---|---|---|---|---|
| `add` | 6215.245078 | 0.0e+00 | 0.0e+00 | 0.0e+00 | 1.2e-14 | 0.144602 | 0.0e+00 | 0.0e+00 | 0.0e+00 | 0.0e+00 |
| `sub` | 15708.8058 | 0.0e+00 | 0.0e+00 | 0.0e+00 | 8.3e-15 | 0.144602 | 0.0e+00 | 0.0e+00 | 0.0e+00 | 0.0e+00 |
| `mul` | -22357.9813 | 0.0e+00 | 0.0e+00 | 0.0e+00 | 4.2e-14 | 0.721669 | 0.0e+00 | 0.0e+00 | 0.0e+00 | 0.0e+00 |
| `div` | 1150.348145 | 0.0e+00 | 0.0e+00 | 0.0e+00 | 4.1e-14 | 0.0556511 | 0.0e+00 | 0.0e+00 | 0.0e+00 | 0.0e+00 |
| `sqr` | 33351536.53 | 0.0e+00 | 0.0e+00 | 0.0e+00 | 0.0e+00 | 0.723458 | 0.0e+00 | 0.0e+00 | 0.0e+00 | 0.0e+00 |
| `sqrt` | 2269319.522 | 0.0e+00 | 0.0e+00 | 0.0e+00 | 0.0e+00 | 0.0173328 | 0.0e+00 | 2.4e-14 | 0.0e+00 | 9.7e-14 |
| `exp` | 1113373157 | -2.1e-16 | 2.1e-16 | 0.0e+00 | 0.0e+00 | 81.332 | 2.6e-15 | 2.1e-14 | 7.3e-16 | 9.0e-15 |
| `log` | 1559051.81 | 0.0e+00 | 4.5e-16 | 0.0e+00 | 1.5e-16 | 0.0185316 | 0.0e+00 | 1.4e-13 | 0.0e+00 | 5.9e-14 |
| `sin` | 372.7406966 | 1.5e-15 | 3.4e-15 | -9.2e-16 | -1.7e-14 | 0.0471741 | 1.4e-15 | 3.7e-14 | 0.0e+00 | 7.4e-14 |
| `cos` | -55217.80218 | -4.0e-16 | -1.3e-16 | 0.0e+00 | 2.5e-15 | 0.04418 | 1.8e-15 | 4.6e-14 | 3.3e-16 | 9.7e-14 |
| `pow_int` | 763027.2411 | 0.0e+00 | -9.2e-16 | 1.5e-15 | 2.4e-06 | 7.24104 | 0.0e+00 | 4.2e-15 | 4.6e-15 | 1.1e-05 |
| `pow_real` | 24977959.15 | 0.0e+00 | 1.2e-15 | 0.0e+00 | 0.0e+00 | 0.963128 | 3.6e-15 | 2.4e-13 | 4.1e-15 | 1.2e-13 |
| `line_arith` | 11641.1468 | 0.0e+00 | 0.0e+00 | 0.0e+00 | 3.5e-14 | 0.742132 | 0.0e+00 | 0.0e+00 | 0.0e+00 | 0.0e+00 |
| `line_trig` | 33352445.64 | 0.0e+00 | 0.0e+00 | 0.0e+00 | 0.0e+00 | 0.781627 | 0.0e+00 | 3.4e-15 | 0.0e+00 | 6.4e-15 |
| `line_pow` | -200633868.5 | 0.0e+00 | -5.9e-16 | 0.0e+00 | 2.1e-08 | 391.862 | 1.5e-16 | 1.4e-15 | 1.5e-16 | 4.5e-07 |
| `shekel5` | -157759.691 | 0.0e+00 | 0.0e+00 | 0.0e+00 | 0.0e+00 | 0.00992931 | 0.0e+00 | 0.0e+00 | 0.0e+00 | -1.8e-16 |
| `block5` | 54765440.5 | 2.7e-16 | 5.4e-16 | -2.7e-16 | -7.1e-07 | 456.259 | 0.0e+00 | 3.1e-15 | 2.2e-15 | 5.6e-06 |

<!-- END GENERATED TABLES -->
