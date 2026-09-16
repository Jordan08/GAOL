# Performance: GAOL, libieeep1788 and Solaris Studio

Part of the [comparison](README.md) of GAOL with libieeep1788 and Solaris
Studio.

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
| Solaris Studio | Sun Fortran 95 8.7 (Solaris Studio 12.4, 2014), `-O3 -xia` | Calls to `libsunimath` |

## What the timings show

**Arithmetic.** GAOL is the fastest: + and − take 3.8 ns, × 16 ns and ÷ 13 ns,
6 times less than Solaris Studio for + and −, and 2 times less for × and ÷.
An addition of intervals costs GAOL 3.8 times an addition of doubles: two
additions, and the check that the rounding direction is upward. The integer
power `pow(x, 3)` takes 14 ns, less than `std::pow(x, 3)` on doubles, which
C++11 computes as `pow(double, double)`; Solaris Studio's `x**3` takes 15 times
longer, and its `x**2` 8 times longer than GAOL's `sqr`.

**Elementary functions.** Solaris Studio is the fastest: sin and cos take
59 ns, half GAOL's time (129 and 122 ns), log 56 ns against 71, and exp is on
par (55 and 53 ns). GAOL computes each bound with mathlib, rounding to
nearest, before moving it outward, and sets the rounding direction four times
for each function; sin and cos reduce their argument modulo an interval
enclosing π. The real power `pow(x, y)` takes 141 ns with GAOL and 184 ns with
Solaris Studio.

**Formulas.** The one-line combinations and the five-line block follow from
the above: GAOL is 3 to 4 times faster than Solaris Studio on the arithmetic
line and on the line of powers, and 7 times faster on Shekel 5, whose 20
squares cost Solaris Studio 76 ns each; Solaris Studio is slightly faster on
the line of sin and cos, and GAOL 1.6 times faster on the five-line block.

**libieeep1788** is 13 to 90 times slower than GAOL: every bound is an MPFR
computation, about 210 ns for an addition and 7 to 10 µs for sin, cos and the
real power. Its own README warns that its focus is correctness, not speed.

**The results** are the same: the sums of the midpoints of the million results
agree to 1e-13 relatively, and the arithmetic operations give the tightest
intervals in the three libraries. On average, GAOL's square roots, elementary
functions and powers are wider than libieeep1788's by 2e-15 to 6e-14
relatively, and Solaris Studio's by at most 5e-15.

The timings were measured on a laptop, the program pinned to one processor,
each time being the best of several runs spread over three rounds: the
machine slowed down now and then for a few seconds, which made some operations
up to four times slower in a single round. Solaris Studio's operations are
calls into a library compiled in 2014, where GCC inlines GAOL's.

## The timings

<!-- BEGIN GENERATED TABLES (doc/compare/code/bench.py) -->

Measured on:

```
Date:            2026-09-16
Processor:       11th Gen Intel(R) Core(TM) i7-1185G7 @ 3.00GHz (taskset -c 2)
System:          Linux 5.15.0-191-generic, Ubuntu 20.04.6 LTS
C++ compiler:    g++ (Ubuntu 9.4.0-1ubuntu1~20.04.3) 9.4.0
C++ flags:       -std=c++11 -O3 -DNDEBUG (GAOL: -frounding-math -fno-fast-math -ffp-contract=off -msse2 -msse3; libieeep1788: -frounding-math -fno-fast-math -ffp-contract=off)
Fortran:         f90: Sun Fortran 95 8.7 Linux_i386 2014/10/20, flags: -O3 -xia
GAOL:            a694cdc, CMake Release, mathlib 2.1.1
libieeep1788:    1f10b89, MPFR 4.2.1, GMP 6.3.0
```

1 000 000 operations of each kind, on the same intervals. Each time is the best of 3 rounds, each program being run in turn with the others: 15 runs for double (reference), 15 runs for GAOL, 3 runs for libieeep1788, 15 runs for Solaris Studio f90 in all.

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
| `pow_int` | a³ (`pow(x, int)` in GAOL, `pown`, `x**3`) |
| `pow_real` | p^e (`pow(x, y)`, `x**y`) |
| `line_arith` | (a + b)(a − b) / p |
| `line_trig` | sin(a) cos(b) + a² |
| `line_pow` | √p · a³ − exp(b / p) |
| `shekel5` | Shekel 5: −Σᵢ 1 / (Σⱼ (xⱼ − aᵢⱼ)² + cᵢ), 5 terms, 4 variables |
| `block5` | five lines: t1 = ab + p; t2 = sin(t1) cos(b); t3 = a² + t2/p; t4 = exp(t2) − b³; t3 t4 + √p |

#### Time per operation (nanoseconds)

| Operation | double (reference) | GAOL | libieeep1788 | Solaris Studio f90 | libieeep1788 / GAOL | Solaris Studio f90 / GAOL |
|---|---|---|---|---|---|---|
| `add` | 1.00 | 3.83 | 211 | 24.2 | 55.2 | 6.3 |
| `sub` | 1.01 | 3.79 | 208 | 24.0 | 55.0 | 6.3 |
| `mul` | 1.01 | 16.3 | 260 | 29.0 | 16.0 | 1.8 |
| `div` | 1.00 | 13.1 | 263 | 27.0 | 20.1 | 2.1 |
| `sqr` | 0.70 | 9.31 | 123 | 75.7 | 13.2 | 8.1 |
| `sqrt` | 1.96 | 8.98 | 147 | 36.7 | 16.4 | 4.1 |
| `exp` | 5.86 | 52.7 | 2 237 | 54.6 | 42.4 | 1.0 |
| `log` | 5.16 | 70.9 | 2 618 | 55.7 | 36.9 | 0.8 |
| `sin` | 18.8 | 129 | 9 774 | 59.1 | 75.7 | 0.5 |
| `cos` | 18.7 | 122 | 7 229 | 59.8 | 59.1 | 0.5 |
| `pow_int` | 19.8 | 14.1 | 336 | 214 | 23.8 | 15.1 |
| `pow_real` | 16.1 | 141 | 10 079 | 184 | 71.4 | 1.3 |
| `line_arith` | 1.30 | 29.5 | 1 016 | 87.6 | 34.4 | 3.0 |
| `line_trig` | 37.9 | 276 | 24 408 | 229 | 88.4 | 0.8 |
| `line_pow` | 29.3 | 95.8 | 3 618 | 361 | 37.8 | 3.8 |
| `shekel5` | 6.23 | 342 | 21 432 | 2 366 | 62.6 | 6.9 |
| `block5` | 67.7 | 398 | 33 319 | 643 | 83.7 | 1.6 |

#### Total time of the 1 000 000 operations (seconds)

| Operation | double (reference) | GAOL | libieeep1788 | Solaris Studio f90 |
|---|---|---|---|---|
| `add` | 0.001 | 0.004 | 0.211 | 0.024 |
| `sub` | 0.001 | 0.004 | 0.208 | 0.024 |
| `mul` | 0.001 | 0.016 | 0.260 | 0.029 |
| `div` | 0.001 | 0.013 | 0.263 | 0.027 |
| `sqr` | 0.001 | 0.009 | 0.123 | 0.076 |
| `sqrt` | 0.002 | 0.009 | 0.147 | 0.037 |
| `exp` | 0.006 | 0.053 | 2.237 | 0.055 |
| `log` | 0.005 | 0.071 | 2.618 | 0.056 |
| `sin` | 0.019 | 0.129 | 9.774 | 0.059 |
| `cos` | 0.019 | 0.122 | 7.229 | 0.060 |
| `pow_int` | 0.020 | 0.014 | 0.336 | 0.214 |
| `pow_real` | 0.016 | 0.141 | 10.079 | 0.184 |
| `line_arith` | 0.001 | 0.030 | 1.016 | 0.088 |
| `line_trig` | 0.038 | 0.276 | 24.408 | 0.229 |
| `line_pow` | 0.029 | 0.096 | 3.618 | 0.361 |
| `shekel5` | 0.006 | 0.342 | 21.432 | 2.366 |
| `block5` | 0.068 | 0.398 | 33.319 | 0.643 |

#### Same results?

The sum of the midpoints of the results, relative to libieeep1788's, and the mean width of the results, with the relative excess of the other libraries over libieeep1788, whose bounds are the tightest:

| Operation | Σ midpoints (libieeep1788) | Δ GAOL | Δ Solaris Studio f90 | mean width (libieeep1788) | excess GAOL | excess Solaris Studio f90 |
|---|---|---|---|---|---|---|
| `add` | 6215.245078 | 0.0e+00 | 0.0e+00 | 0.144602 | 0.0e+00 | 0.0e+00 |
| `sub` | 15708.8058 | 0.0e+00 | 0.0e+00 | 0.144602 | 0.0e+00 | 0.0e+00 |
| `mul` | -22357.9813 | 0.0e+00 | 0.0e+00 | 0.721669 | 0.0e+00 | 0.0e+00 |
| `div` | 1150.348145 | 0.0e+00 | 0.0e+00 | 0.0556511 | 0.0e+00 | 0.0e+00 |
| `sqr` | 33351536.53 | 0.0e+00 | 0.0e+00 | 0.723458 | 0.0e+00 | 0.0e+00 |
| `sqrt` | 2269319.522 | -2.1e-16 | 0.0e+00 | 0.0173328 | 1.4e-14 | 0.0e+00 |
| `exp` | 1113373157 | -2.1e-16 | 0.0e+00 | 81.332 | 2.6e-15 | 7.3e-16 |
| `log` | 1559051.81 | 0.0e+00 | 0.0e+00 | 0.0185316 | 1.5e-14 | 0.0e+00 |
| `sin` | 372.7406966 | 1.4e-13 | -9.2e-16 | 0.0471741 | 1.3e-14 | 0.0e+00 |
| `cos` | -55217.80218 | -4.0e-16 | 0.0e+00 | 0.04418 | 1.8e-15 | 3.3e-16 |
| `pow_int` | 763027.2411 | -9.2e-16 | 1.5e-15 | 7.24104 | 4.2e-15 | 4.6e-15 |
| `pow_real` | 24977959.15 | 0.0e+00 | 0.0e+00 | 0.963128 | 5.9e-14 | 4.1e-15 |
| `line_arith` | 11641.1468 | 0.0e+00 | 0.0e+00 | 0.742132 | 0.0e+00 | 0.0e+00 |
| `line_trig` | 33352445.64 | 0.0e+00 | 0.0e+00 | 0.781627 | 3.0e-16 | 0.0e+00 |
| `line_pow` | -200633868.5 | 0.0e+00 | 0.0e+00 | 391.862 | 4.6e-16 | 1.5e-16 |
| `shekel5` | -157759.691 | 0.0e+00 | 0.0e+00 | 0.00992931 | 0.0e+00 | 0.0e+00 |
| `block5` | 54765440.5 | 2.7e-16 | -2.7e-16 | 456.259 | 3.3e-15 | 2.2e-15 |

<!-- END GENERATED TABLES -->
