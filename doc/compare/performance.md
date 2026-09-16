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
as filib++ (7.6 ns) and six times as fast as Solaris Studio (24 ns); × and ÷
take GAOL and filib++ about the same time, 16 to 17 ns and 12 to 13 ns, half
Solaris Studio's. An addition of intervals costs GAOL 3.6 times an addition of
doubles: two additions, and the check that the rounding direction is upward,
where filib++ sets the rounding direction and restores it. The integer power
`pow(x, 3)` takes GAOL 14 ns, less than `std::pow(x, 3)` on doubles, which
C++11 computes as `pow(double, double)`; filib++'s `power(x, 3)` takes twice as
long, Solaris Studio's `x**3` 15 times as long, and its `x**2` 8 times as long
as GAOL's `sqr`.

**Elementary functions.** filib++ is the fastest: sin and cos take 53 ns, log
29 ns, exp 47 ns and the real power `pow(x, y)` 92 ns. Solaris Studio follows,
with 59 to 60 ns for sin and cos, 56 ns for log, 55 ns for exp and 184 ns for
`pow(x, y)`, then GAOL, with 130 and 124 ns for sin and cos, 71 ns for log,
53 ns for exp and 143 ns for `pow(x, y)`. GAOL computes each bound with
mathlib, rounding to nearest, before moving it outward, and sets the rounding
direction four times for each function; sin and cos reduce their argument
modulo an interval enclosing π. filib++'s bounds are the widest (below).

**Formulas.** GAOL is the fastest on the arithmetic line (30 ns, against 40
for filib++ and 88 for Solaris Studio), on the line of powers (96 ns, against
122 and 365) and on Shekel 5 (334 ns, against 584 and 2 386: Shekel 5 is 20
squares, 45 additions and subtractions and 5 divisions, and its squares cost
Solaris Studio 76 ns each). filib++ is the fastest on the line of sin and cos
(142 ns, against 230 for Solaris Studio and 278 for GAOL) and on the five-line
block (289 ns, against 400 for GAOL and 649 for Solaris Studio), where its
elementary functions weigh most.

**libieeep1788** is 13 to 73 times slower than GAOL: every bound is an MPFR
computation, about 210 ns for an addition and 7 to 10 µs for sin, cos and the
real power. Its own README warns that its focus is correctness, not speed.

**The results** are the same: the sums of the midpoints of the million results
agree to 2e-13 relatively, and the arithmetic operations give the tightest
intervals in the four libraries. On average, the square roots, elementary
functions and powers are wider than libieeep1788's by 2e-15 to 6e-14
relatively with GAOL, 4e-15 to 2.4e-13 with filib++ (1.4e-13 for log, 2.4e-13
for the real power), and at most 5e-15 with Solaris Studio.

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
GAOL:            1026f39, CMake Release, mathlib 2.1.1
libieeep1788:    1f10b89, MPFR 4.2.1, GMP 6.3.0
filib++:         3.0.2.2 (/home/jninin/Logiciel/filib), interval<double, native_switched, i_mode_extended_flag>
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
| `add` | 1.04 | 3.71 | 213 | 7.62 | 24.3 | 57.3 | 2.1 | 6.5 |
| `sub` | 1.04 | 3.83 | 209 | 7.57 | 23.9 | 54.5 | 2.0 | 6.3 |
| `mul` | 1.15 | 16.5 | 262 | 17.0 | 29.1 | 15.9 | 1.0 | 1.8 |
| `div` | 1.10 | 13.1 | 264 | 11.8 | 26.9 | 20.2 | 0.9 | 2.1 |
| `sqr` | 0.79 | 9.23 | 124 | 13.4 | 75.8 | 13.5 | 1.4 | 8.2 |
| `sqrt` | 1.96 | 8.92 | 147 | 16.8 | 36.8 | 16.5 | 1.9 | 4.1 |
| `exp` | 5.87 | 52.5 | 2 270 | 46.5 | 54.6 | 43.2 | 0.9 | 1.0 |
| `log` | 5.11 | 70.9 | 2 626 | 29.2 | 55.7 | 37.1 | 0.4 | 0.8 |
| `sin` | 18.7 | 130 | 8 648 | 52.5 | 59.3 | 66.4 | 0.4 | 0.5 |
| `cos` | 18.7 | 124 | 7 433 | 53.3 | 59.7 | 60.2 | 0.4 | 0.5 |
| `pow_int` | 20.0 | 14.1 | 332 | 26.7 | 214 | 23.5 | 1.9 | 15.1 |
| `pow_real` | 16.0 | 143 | 9 687 | 91.5 | 184 | 67.7 | 0.6 | 1.3 |
| `line_arith` | 1.39 | 29.7 | 1 019 | 40.4 | 88.4 | 34.3 | 1.4 | 3.0 |
| `line_trig` | 37.8 | 278 | 18 679 | 142 | 230 | 67.3 | 0.5 | 0.8 |
| `line_pow` | 29.3 | 96.2 | 3 799 | 122 | 365 | 39.5 | 1.3 | 3.8 |
| `shekel5` | 6.20 | 334 | 13 876 | 584 | 2 386 | 41.6 | 1.7 | 7.1 |
| `block5` | 67.7 | 400 | 29 171 | 289 | 649 | 72.9 | 0.7 | 1.6 |

#### Total time of the 1 000 000 operations (seconds)

| Operation | double (reference) | GAOL | libieeep1788 | filib++ | Solaris Studio f90 |
|---|---|---|---|---|---|
| `add` | 0.001 | 0.004 | 0.213 | 0.008 | 0.024 |
| `sub` | 0.001 | 0.004 | 0.209 | 0.008 | 0.024 |
| `mul` | 0.001 | 0.017 | 0.262 | 0.017 | 0.029 |
| `div` | 0.001 | 0.013 | 0.264 | 0.012 | 0.027 |
| `sqr` | 0.001 | 0.009 | 0.124 | 0.013 | 0.076 |
| `sqrt` | 0.002 | 0.009 | 0.147 | 0.017 | 0.037 |
| `exp` | 0.006 | 0.053 | 2.270 | 0.046 | 0.055 |
| `log` | 0.005 | 0.071 | 2.626 | 0.029 | 0.056 |
| `sin` | 0.019 | 0.130 | 8.648 | 0.053 | 0.059 |
| `cos` | 0.019 | 0.124 | 7.433 | 0.053 | 0.060 |
| `pow_int` | 0.020 | 0.014 | 0.332 | 0.027 | 0.214 |
| `pow_real` | 0.016 | 0.143 | 9.687 | 0.091 | 0.184 |
| `line_arith` | 0.001 | 0.030 | 1.019 | 0.040 | 0.088 |
| `line_trig` | 0.038 | 0.278 | 18.679 | 0.142 | 0.230 |
| `line_pow` | 0.029 | 0.096 | 3.799 | 0.122 | 0.365 |
| `shekel5` | 0.006 | 0.334 | 13.876 | 0.584 | 2.386 |
| `block5` | 0.068 | 0.400 | 29.171 | 0.289 | 0.649 |

#### Same results?

The sum of the midpoints of the results, relative to libieeep1788's, and the mean width of the results, with the relative excess of the other libraries over libieeep1788, whose bounds are the tightest:

| Operation | Σ midpoints (libieeep1788) | Δ GAOL | Δ filib++ | Δ Solaris Studio f90 | mean width (libieeep1788) | excess GAOL | excess filib++ | excess Solaris Studio f90 |
|---|---|---|---|---|---|---|---|---|
| `add` | 6215.245078 | 0.0e+00 | 0.0e+00 | 0.0e+00 | 0.144602 | 0.0e+00 | 0.0e+00 | 0.0e+00 |
| `sub` | 15708.8058 | 0.0e+00 | 0.0e+00 | 0.0e+00 | 0.144602 | 0.0e+00 | 0.0e+00 | 0.0e+00 |
| `mul` | -22357.9813 | 0.0e+00 | 0.0e+00 | 0.0e+00 | 0.721669 | 0.0e+00 | 0.0e+00 | 0.0e+00 |
| `div` | 1150.348145 | 0.0e+00 | 0.0e+00 | 0.0e+00 | 0.0556511 | 0.0e+00 | 0.0e+00 | 0.0e+00 |
| `sqr` | 33351536.53 | 0.0e+00 | 0.0e+00 | 0.0e+00 | 0.723458 | 0.0e+00 | 0.0e+00 | 0.0e+00 |
| `sqrt` | 2269319.522 | -2.1e-16 | 0.0e+00 | 0.0e+00 | 0.0173328 | 1.4e-14 | 2.4e-14 | 0.0e+00 |
| `exp` | 1113373157 | -2.1e-16 | 2.1e-16 | 0.0e+00 | 81.332 | 2.6e-15 | 2.1e-14 | 7.3e-16 |
| `log` | 1559051.81 | 0.0e+00 | 4.5e-16 | 0.0e+00 | 0.0185316 | 1.5e-14 | 1.4e-13 | 0.0e+00 |
| `sin` | 372.7406966 | 1.4e-13 | 3.4e-15 | -9.2e-16 | 0.0471741 | 1.3e-14 | 3.7e-14 | 0.0e+00 |
| `cos` | -55217.80218 | -4.0e-16 | -1.3e-16 | 0.0e+00 | 0.04418 | 1.8e-15 | 4.6e-14 | 3.3e-16 |
| `pow_int` | 763027.2411 | -9.2e-16 | -9.2e-16 | 1.5e-15 | 7.24104 | 4.2e-15 | 4.2e-15 | 4.6e-15 |
| `pow_real` | 24977959.15 | 0.0e+00 | 1.2e-15 | 0.0e+00 | 0.963128 | 5.9e-14 | 2.4e-13 | 4.1e-15 |
| `line_arith` | 11641.1468 | 0.0e+00 | 0.0e+00 | 0.0e+00 | 0.742132 | 0.0e+00 | 0.0e+00 | 0.0e+00 |
| `line_trig` | 33352445.64 | 0.0e+00 | 0.0e+00 | 0.0e+00 | 0.781627 | 3.0e-16 | 3.4e-15 | 0.0e+00 |
| `line_pow` | -200633868.5 | 0.0e+00 | -5.9e-16 | 0.0e+00 | 391.862 | 4.6e-16 | 1.4e-15 | 1.5e-16 |
| `shekel5` | -157759.691 | 0.0e+00 | 0.0e+00 | 0.0e+00 | 0.00992931 | 0.0e+00 | 0.0e+00 | 0.0e+00 |
| `block5` | 54765440.5 | 2.7e-16 | 5.4e-16 | -2.7e-16 | 456.259 | 3.3e-15 | 3.1e-15 | 2.2e-15 |

<!-- END GENERATED TABLES -->
