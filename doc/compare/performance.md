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
as filib++ (7.4 and 7.6 ns) and six times as fast as Solaris Studio (24 ns);
× and ÷ take GAOL and filib++ about the same time, 16 to 17 ns and 12 to
13 ns, half Solaris Studio's. An addition of intervals costs GAOL 3.5 times an
addition of doubles: two additions, and the check that the rounding direction
is upward, where filib++ sets the rounding direction and restores it. The
integer power `pow(x, 3)` takes GAOL 14 ns, less than `std::pow(x, 3)` on
doubles, which C++11 computes as `pow(double, double)`; filib++'s
`power(x, 3)` takes twice as long, Solaris Studio's `x**3` 15 times as long,
and its `x**2` 8.5 times as long as GAOL's `sqr`.

**Elementary functions.** filib++ is the fastest: sin and cos take 53 ns, log
29 ns, exp 46 ns and the real power `pow(x, y)` 92 ns. Solaris Studio follows,
with 59 ns for sin and cos, 56 ns for log, 55 ns for exp and 184 ns for
`pow(x, y)`, then GAOL, with 121 and 123 ns for sin and cos, 73 ns for log,
53 ns for exp and 145 ns for `pow(x, y)`. GAOL computes each bound with
mathlib, rounding to nearest, before moving it outward, and sets the rounding
direction four times for each function; sin and cos also divide the bounds by
an interval enclosing π to find where they are monotonic. filib++'s bounds are
the widest (below).

**Formulas.** GAOL is the fastest on the arithmetic line (29 ns, against 41
for filib++ and 88 for Solaris Studio), on the line of powers (100 ns, against
123 and 361) and on Shekel 5 (321 ns, against 580 and 2 383: Shekel 5 is 20
squares, 45 additions and subtractions and 5 divisions, and its squares cost
Solaris Studio 76 ns each). filib++ is the fastest on the line of sin and cos
(141 ns, against 232 for Solaris Studio and 274 for GAOL) and on the five-line
block (290 ns, against 404 for GAOL and 646 for Solaris Studio), where its
elementary functions weigh most.

**libieeep1788** is 14 to 68 times slower than GAOL: every bound is an MPFR
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
GAOL:            0536f71, CMake Release, mathlib 2.1.1
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
| `add` | 1.06 | 3.71 | 212 | 7.36 | 24.6 | 57.0 | 2.0 | 6.6 |
| `sub` | 1.08 | 3.83 | 208 | 7.64 | 24.0 | 54.3 | 2.0 | 6.3 |
| `mul` | 1.03 | 16.4 | 261 | 17.0 | 29.0 | 15.9 | 1.0 | 1.8 |
| `div` | 1.00 | 13.2 | 264 | 11.8 | 27.0 | 20.1 | 0.9 | 2.1 |
| `sqr` | 0.69 | 8.91 | 125 | 13.3 | 75.6 | 14.0 | 1.5 | 8.5 |
| `sqrt` | 1.96 | 9.02 | 160 | 16.8 | 36.8 | 17.8 | 1.9 | 4.1 |
| `exp` | 5.85 | 52.5 | 2 379 | 45.9 | 54.6 | 45.3 | 0.9 | 1.0 |
| `log` | 5.06 | 72.9 | 2 602 | 29.3 | 55.7 | 35.7 | 0.4 | 0.8 |
| `sin` | 18.8 | 121 | 8 252 | 52.7 | 59.1 | 68.2 | 0.4 | 0.5 |
| `cos` | 18.6 | 123 | 7 420 | 53.4 | 59.4 | 60.3 | 0.4 | 0.5 |
| `pow_int` | 19.9 | 14.1 | 330 | 26.6 | 213 | 23.5 | 1.9 | 15.2 |
| `pow_real` | 16.1 | 145 | 9 229 | 92.3 | 184 | 63.5 | 0.6 | 1.3 |
| `line_arith` | 1.34 | 29.4 | 1 016 | 40.5 | 88.3 | 34.6 | 1.4 | 3.0 |
| `line_trig` | 38.0 | 274 | 16 381 | 141 | 232 | 59.8 | 0.5 | 0.8 |
| `line_pow` | 29.4 | 100 | 3 761 | 123 | 361 | 37.5 | 1.2 | 3.6 |
| `shekel5` | 6.18 | 321 | 13 964 | 580 | 2 383 | 43.6 | 1.8 | 7.4 |
| `block5` | 67.7 | 404 | 26 084 | 290 | 646 | 64.5 | 0.7 | 1.6 |

#### Total time of the 1 000 000 operations (seconds)

| Operation | double (reference) | GAOL | libieeep1788 | filib++ | Solaris Studio f90 |
|---|---|---|---|---|---|
| `add` | 0.001 | 0.004 | 0.212 | 0.007 | 0.025 |
| `sub` | 0.001 | 0.004 | 0.208 | 0.008 | 0.024 |
| `mul` | 0.001 | 0.016 | 0.261 | 0.017 | 0.029 |
| `div` | 0.001 | 0.013 | 0.264 | 0.012 | 0.027 |
| `sqr` | 0.001 | 0.009 | 0.125 | 0.013 | 0.076 |
| `sqrt` | 0.002 | 0.009 | 0.160 | 0.017 | 0.037 |
| `exp` | 0.006 | 0.053 | 2.379 | 0.046 | 0.055 |
| `log` | 0.005 | 0.073 | 2.602 | 0.029 | 0.056 |
| `sin` | 0.019 | 0.121 | 8.252 | 0.053 | 0.059 |
| `cos` | 0.019 | 0.123 | 7.420 | 0.053 | 0.059 |
| `pow_int` | 0.020 | 0.014 | 0.330 | 0.027 | 0.213 |
| `pow_real` | 0.016 | 0.145 | 9.229 | 0.092 | 0.184 |
| `line_arith` | 0.001 | 0.029 | 1.016 | 0.040 | 0.088 |
| `line_trig` | 0.038 | 0.274 | 16.381 | 0.141 | 0.232 |
| `line_pow` | 0.029 | 0.100 | 3.761 | 0.123 | 0.361 |
| `shekel5` | 0.006 | 0.321 | 13.964 | 0.580 | 2.383 |
| `block5` | 0.068 | 0.404 | 26.084 | 0.290 | 0.646 |

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
