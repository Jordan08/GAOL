# Performance: GAOL, libieeep1788, filib++ and Solaris Studio

Part of the [comparison](README.md) of GAOL with libieeep1788, filib++ and
Solaris Studio.

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
| GAOL | GCC 9.4, `-O3`, the flags of interval arithmetic; GAOL built by CMake in Release | Inline SSE2 operations, the rounding direction set upward; elementary functions computed by mathlib and moved outward |
| libieeep1788 | GCC 9.4, `-O3` | Each bound computed by MPFR (4.2.1), correctly rounded |
| filib++ | GCC 9.4, `-O3`, the flags of interval arithmetic; filib++ 3.0.2.2 | `interval<double, native_switched, i_mode_extended_flag>`, as in IBEX: inline operations, the rounding direction set and restored by each; elementary functions of its own |
| Solaris Studio | Sun Fortran 95 8.7 (Solaris Studio 12.4, 2014), `-O3 -xia` | Calls to `libsunimath` |

## What the timings show

**Arithmetic.** GAOL is the fastest on + and −, 3.7 and 3.8 ns, twice as fast
as filib++ (7.6 and 7.4 ns) and six times as fast as Solaris Studio (24 ns);
× and ÷ take GAOL and filib++ about the same time, 16 to 17 ns and 12 to
13 ns, half Solaris Studio's. An addition of intervals costs GAOL 3.6 times an
addition of doubles: two additions, and the check that the rounding direction
is upward, where filib++ sets the rounding direction and restores it. The
integer power `pow(x, 3)` takes GAOL 14 ns, less than `std::pow(x, 3)` on
doubles, which C++11 computes as `pow(double, double)`; filib++'s
`power(x, 3)` takes twice as long, Solaris Studio's `x**3` 15 times as long,
and its `x**2` 8 times as long as GAOL's `sqr`.

**Elementary functions.** filib++ is the fastest: sin and cos take 53 ns, log
29 ns, exp 46 ns and the real power `pow(x, y)` 91 ns. Solaris Studio follows,
with 59 ns for sin and cos, 56 ns for log, 55 ns for exp and 184 ns for
`pow(x, y)`, then GAOL, with 112 and 113 ns for sin and cos, 68 ns for log,
47 ns for exp and 129 ns for `pow(x, y)`. GAOL computes each bound with
mathlib, rounding to nearest, before moving it outward, and sets the rounding
direction twice for each function, to nearest before mathlib and upward
after; sin and cos also divide the bounds by an interval enclosing π to find
where they are monotonic. filib++'s bounds are
the widest (below).

**Formulas.** GAOL is the fastest on the arithmetic line (30 ns, against 41
for filib++ and 88 for Solaris Studio), on the line of powers (91 ns, against
122 and 361) and on Shekel 5 (331 ns, against 580 and 2 366: Shekel 5 is 20
squares, 45 additions and subtractions and 5 divisions, and its squares cost
Solaris Studio 76 ns each). filib++ is the fastest on the line of sin and cos
(141 ns, against 229 for Solaris Studio and 252 for GAOL) and on the five-line
block (288 ns, against 370 for GAOL and 645 for Solaris Studio), where its
elementary functions weigh most.

**libieeep1788** is 13 to 78 times slower than GAOL: every bound is an MPFR
computation, about 210 ns for an addition and 7 to 9 µs for sin, cos and the
real power. Its own README warns that its focus is correctness, not speed.

**The results** are the same: the sums of the midpoints of the million results
agree to 4e-15 relatively, and the arithmetic operations give the tightest
intervals in the four libraries, and so do the square roots, except
filib++'s. On average, the elementary functions and powers are wider than
libieeep1788's by 1e-15 to 6e-14 relatively with GAOL, 4e-15 to 2.4e-13 with
filib++ (1.4e-13 for log, 2.4e-13 for the real power), and at most 5e-15 with
Solaris Studio.

The timings were measured on a laptop, the program pinned to one processor,
each time being the best of several runs spread over three rounds: the
machine slowed down now and then for a few seconds, which made some operations
up to four times slower in a single round. Solaris Studio's operations are
calls into a library compiled in 2014, where GCC inlines GAOL's and filib++'s.

## The timings

<!-- BEGIN GENERATED TABLES (doc/compare/code/bench.py) -->

Measured on:

```
Date:            2026-09-16
Processor:       11th Gen Intel(R) Core(TM) i7-1185G7 @ 3.00GHz (taskset -c 2)
System:          Linux 5.15.0-191-generic, Ubuntu 20.04.6 LTS
C++ compiler:    g++ (Ubuntu 9.4.0-1ubuntu1~20.04.3) 9.4.0
C++ flags:       -std=c++11 -O3 -DNDEBUG (GAOL: -frounding-math -fno-fast-math -ffp-contract=off -msse2 -msse3; libieeep1788 and filib++: -frounding-math -fno-fast-math -ffp-contract=off)
Fortran:         f90: Sun Fortran 95 8.7 Linux_i386 2014/10/20, flags: -O3 -xia
GAOL:            98055b6, CMake Release, mathlib 2.1.1
libieeep1788:    1f10b89, MPFR 4.2.1, GMP 6.3.0
filib++:         3.0.2.2 , interval<double, native_switched, i_mode_extended_flag>
```

1 000 000 operations of each kind, on the same intervals. Each time is the best of 3 rounds, each program being run in turn with the others: 15 runs for double (reference), 15 runs for GAOL, 3 runs for libieeep1788, 15 runs for filib++, 15 runs for Solaris Studio f90 in all.

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

| Operation | double (reference) | GAOL | libieeep1788 | filib++ | Solaris Studio f90 | libieeep1788 / GAOL | filib++ / GAOL | Solaris Studio f90 / GAOL |
|---|---|---|---|---|---|---|---|---|
| `add` | 1.02 | 3.69 | 213 | 7.61 | 23.8 | 57.6 | 2.1 | 6.5 |
| `sub` | 1.01 | 3.81 | 212 | 7.41 | 23.9 | 55.6 | 1.9 | 6.3 |
| `mul` | 1.04 | 16.7 | 263 | 17.0 | 28.9 | 15.8 | 1.0 | 1.7 |
| `div` | 1.05 | 13.2 | 263 | 11.7 | 26.8 | 19.9 | 0.9 | 2.0 |
| `sqr` | 0.68 | 9.30 | 123 | 13.3 | 75.7 | 13.2 | 1.4 | 8.1 |
| `sqrt` | 1.96 | 9.15 | 146 | 16.7 | 36.7 | 16.0 | 1.8 | 4.0 |
| `exp` | 5.87 | 46.7 | 2 249 | 45.9 | 54.6 | 48.1 | 1.0 | 1.2 |
| `log` | 5.07 | 68.2 | 2 612 | 29.2 | 55.6 | 38.3 | 0.4 | 0.8 |
| `sin` | 18.7 | 112 | 8 727 | 52.6 | 59.1 | 77.9 | 0.5 | 0.5 |
| `cos` | 18.6 | 113 | 7 646 | 53.4 | 59.4 | 67.8 | 0.5 | 0.5 |
| `pow_int` | 19.8 | 14.5 | 330 | 26.7 | 214 | 22.8 | 1.8 | 14.7 |
| `pow_real` | 16.0 | 129 | 9 206 | 91.4 | 184 | 71.2 | 0.7 | 1.4 |
| `line_arith` | 1.37 | 29.6 | 1 042 | 40.6 | 87.7 | 35.2 | 1.4 | 3.0 |
| `line_trig` | 37.8 | 252 | 18 873 | 141 | 229 | 75.0 | 0.6 | 0.9 |
| `line_pow` | 29.2 | 90.6 | 3 800 | 122 | 361 | 41.9 | 1.4 | 4.0 |
| `shekel5` | 6.22 | 331 | 13 656 | 580 | 2 366 | 41.3 | 1.8 | 7.1 |
| `block5` | 67.6 | 370 | 25 674 | 288 | 645 | 69.5 | 0.8 | 1.7 |

#### Total time of the 1 000 000 operations (seconds)

| Operation | double (reference) | GAOL | libieeep1788 | filib++ | Solaris Studio f90 |
|---|---|---|---|---|---|
| `add` | 0.001 | 0.004 | 0.213 | 0.008 | 0.024 |
| `sub` | 0.001 | 0.004 | 0.212 | 0.007 | 0.024 |
| `mul` | 0.001 | 0.017 | 0.263 | 0.017 | 0.029 |
| `div` | 0.001 | 0.013 | 0.263 | 0.012 | 0.027 |
| `sqr` | 0.001 | 0.009 | 0.123 | 0.013 | 0.076 |
| `sqrt` | 0.002 | 0.009 | 0.146 | 0.017 | 0.037 |
| `exp` | 0.006 | 0.047 | 2.249 | 0.046 | 0.055 |
| `log` | 0.005 | 0.068 | 2.612 | 0.029 | 0.056 |
| `sin` | 0.019 | 0.112 | 8.727 | 0.053 | 0.059 |
| `cos` | 0.019 | 0.113 | 7.646 | 0.053 | 0.059 |
| `pow_int` | 0.020 | 0.014 | 0.330 | 0.027 | 0.214 |
| `pow_real` | 0.016 | 0.129 | 9.206 | 0.091 | 0.184 |
| `line_arith` | 0.001 | 0.030 | 1.042 | 0.041 | 0.088 |
| `line_trig` | 0.038 | 0.252 | 18.873 | 0.141 | 0.229 |
| `line_pow` | 0.029 | 0.091 | 3.800 | 0.122 | 0.361 |
| `shekel5` | 0.006 | 0.331 | 13.656 | 0.580 | 2.366 |
| `block5` | 0.068 | 0.370 | 25.674 | 0.288 | 0.645 |

#### Same results?

The sum of the midpoints of the results, relative to libieeep1788's, and the mean width of the results, with the relative excess of the other libraries over libieeep1788, whose bounds are the tightest:

| Operation | Σ midpoints (libieeep1788) | Δ GAOL | Δ filib++ | Δ Solaris Studio f90 | mean width (libieeep1788) | excess GAOL | excess filib++ | excess Solaris Studio f90 |
|---|---|---|---|---|---|---|---|---|
| `add` | 6215.245078 | 0.0e+00 | 0.0e+00 | 0.0e+00 | 0.144602 | 0.0e+00 | 0.0e+00 | 0.0e+00 |
| `sub` | 15708.8058 | 0.0e+00 | 0.0e+00 | 0.0e+00 | 0.144602 | 0.0e+00 | 0.0e+00 | 0.0e+00 |
| `mul` | -22357.9813 | 0.0e+00 | 0.0e+00 | 0.0e+00 | 0.721669 | 0.0e+00 | 0.0e+00 | 0.0e+00 |
| `div` | 1150.348145 | 0.0e+00 | 0.0e+00 | 0.0e+00 | 0.0556511 | 0.0e+00 | 0.0e+00 | 0.0e+00 |
| `sqr` | 33351536.53 | 0.0e+00 | 0.0e+00 | 0.0e+00 | 0.723458 | 0.0e+00 | 0.0e+00 | 0.0e+00 |
| `sqrt` | 2269319.522 | 0.0e+00 | 0.0e+00 | 0.0e+00 | 0.0173328 | 0.0e+00 | 2.4e-14 | 0.0e+00 |
| `exp` | 1113373157 | -2.1e-16 | 2.1e-16 | 0.0e+00 | 81.332 | 2.6e-15 | 2.1e-14 | 7.3e-16 |
| `log` | 1559051.81 | 0.0e+00 | 4.5e-16 | 0.0e+00 | 0.0185316 | 1.5e-14 | 1.4e-13 | 0.0e+00 |
| `sin` | 372.7406966 | 1.5e-15 | 3.4e-15 | -9.2e-16 | 0.0471741 | 1.4e-15 | 3.7e-14 | 0.0e+00 |
| `cos` | -55217.80218 | -4.0e-16 | -1.3e-16 | 0.0e+00 | 0.04418 | 1.8e-15 | 4.6e-14 | 3.3e-16 |
| `pow_int` | 763027.2411 | -9.2e-16 | -9.2e-16 | 1.5e-15 | 7.24104 | 4.2e-15 | 4.2e-15 | 4.6e-15 |
| `pow_real` | 24977959.15 | 0.0e+00 | 1.2e-15 | 0.0e+00 | 0.963128 | 5.9e-14 | 2.4e-13 | 4.1e-15 |
| `line_arith` | 11641.1468 | 0.0e+00 | 0.0e+00 | 0.0e+00 | 0.742132 | 0.0e+00 | 0.0e+00 | 0.0e+00 |
| `line_trig` | 33352445.64 | 0.0e+00 | 0.0e+00 | 0.0e+00 | 0.781627 | 0.0e+00 | 3.4e-15 | 0.0e+00 |
| `line_pow` | -200633868.5 | 0.0e+00 | -5.9e-16 | 0.0e+00 | 391.862 | 3.0e-16 | 1.4e-15 | 1.5e-16 |
| `shekel5` | -157759.691 | 0.0e+00 | 0.0e+00 | 0.0e+00 | 0.00992931 | 0.0e+00 | 0.0e+00 | 0.0e+00 |
| `block5` | 54765440.5 | 2.7e-16 | 5.4e-16 | -2.7e-16 | 456.259 | 2.5e-15 | 3.1e-15 | 2.2e-15 |

<!-- END GENERATED TABLES -->
