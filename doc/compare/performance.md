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
| GAOL | GCC 9.4, `-O3`, the flags of interval arithmetic; GAOL built by CMake in Release | Inline SSE2 operations, the rounding direction set upward; elementary functions computed by mathlib and moved outward |
| libieeep1788 | GCC 9.4, `-O3` | Each bound computed by MPFR (4.2.1), correctly rounded |
| filib++ | GCC 9.4, `-O3`, the flags of interval arithmetic; filib++ 3.0.2.2 | `interval<double, native_switched, i_mode_extended_flag>`, as in IBEX: inline operations, the rounding direction set and restored by each; elementary functions of its own |
| PROFIL/BIAS | GCC 9.4, `-O3`, the flags of interval arithmetic; PROFIL/BIAS 2.0.8 built with its `x86-64-Linux-compat-gcc` configuration (`-O2`) | `INTERVAL`, whose operations are calls to the BIAS library, each setting the rounding direction downward then upward and back to nearest; elementary functions from the libm, moved outward |
| Solaris Studio | Sun Fortran 95 8.7 (Solaris Studio 12.4, 2014), `-O3 -xia` | Calls to `libsunimath` |

## What the timings show

**Arithmetic.** GAOL is the fastest on + and −, 3.7 and 3.8 ns, twice as fast
as filib++ (7.5 ns) and six times as fast as PROFIL/BIAS (22 ns) and Solaris
Studio (24 ns); × and ÷ take GAOL and filib++ about the same time, 16 and
12 to 13 ns, against 22 ns for PROFIL/BIAS and 27 to 29 ns for Solaris Studio.
An addition of intervals costs GAOL 3.7 times an addition of doubles: two
additions, and the check that the rounding direction is upward, where filib++
sets the rounding direction and restores it, and PROFIL/BIAS calls a function
of BIAS that sets it downward, then upward, then back to nearest — which is
why its four arithmetic operations all take about the same time. The integer
power `pow(x, 3)` takes GAOL 32 ns, computed from exact products since
[issue #7](https://github.com/Jordan08/GAOL/issues/7) (14 ns before, with the
products rounded one by one, for bounds up to 4 doubles wider):
filib++'s `power(x, 3)` takes 27 ns, PROFIL/BIAS's 50 ns and Solaris Studio's
`x**3` 213 ns; `sqr` takes GAOL 9 ns, against 13 for filib++, 28 for
PROFIL/BIAS and 76 for Solaris Studio.

**Elementary functions.** PROFIL/BIAS is the fastest on exp (16 ns), log
(17 ns) and the real power `pow(x, y)` (53 ns), which it computes from the
libm of the system, moved outward, without correct rounding: its bounds are
the widest of the five (below), and its sin and cos, computed by its own
argument reduction, are the slowest, 175 and 196 ns. filib++ is the fastest on
sin and cos (53 ns) with its own polynomials. Solaris Studio follows, with
59 ns for sin and cos, 56 ns for log, 55 ns for exp and 184 ns for
`pow(x, y)`, then GAOL, with 113 ns for sin and cos, 68 ns for log, 47 ns for
exp and 138 ns for `pow(x, y)`. GAOL computes each bound with mathlib,
correctly rounded to nearest, before moving it one double outward, and sets
the rounding direction twice for each function, to nearest before mathlib and
upward after; sin and cos also divide the bounds by an interval enclosing π to
find where they are monotonic, and ask mathlib for the signs of the derivative
where the division cannot tell
([issue #6](https://github.com/Jordan08/GAOL/issues/6)), and `pow(x, y)` takes
mathlib's pow at two corners of the box.

**Formulas.** GAOL is the fastest on the arithmetic line (29 ns, against 41
for filib++, 88 for Solaris Studio and 91 for PROFIL/BIAS), on the line of
powers (113 ns, against 122, 361 and 133) and on Shekel 5 (325 ns, against
581, 2 381 and 1 599: Shekel 5 is 20 squares, 45 additions and subtractions
and 5 divisions, and its squares cost Solaris Studio 76 ns each and
PROFIL/BIAS 28). filib++ is the fastest on the line of sin and cos (141 ns,
against 229 for Solaris Studio, 252 for GAOL and 442 for PROFIL/BIAS) and on
the five-line block (290 ns, against 398 for GAOL, 614 for PROFIL/BIAS and
644 for Solaris Studio), where its elementary functions weigh most.

**libieeep1788** is 10 to 72 times slower than GAOL: every bound is an MPFR
computation, about 210 ns for an addition and 7 to 9 µs for sin, cos and the
real power. Its own README warns that its focus is correctness, not speed.

**The results** are the same: the sums of the midpoints of the million results
agree to 4e-15 relatively, apart from PROFIL/BIAS's integer powers (2.4e-06,
below), and the arithmetic operations give the tightest intervals in the five
libraries, and so do the square roots, except filib++'s and PROFIL/BIAS's. On
average, the elementary functions and powers are wider than libieeep1788's by
1e-15 to 3.6e-15 relatively with GAOL, 4e-15 to 2.4e-13 with filib++ (1.4e-13
for log, 2.4e-13 for the real power), 9e-15 to 1.2e-13 with PROFIL/BIAS, and
at most 5e-15 with Solaris Studio. PROFIL/BIAS's integer power is far wider,
1.1e-05 on average: it computes `Power(x, n)` as exp(n log x), where the four
others multiply. Its arithmetic results have midpoints that differ by about
1e-14 relatively while their widths are the tightest: the intervals are the
same, but its `Mid` computes inf + (sup − inf)/2 **rounded upward**, one
double above the midpoint for 4 % of the sums of the benchmark, where the
others round to nearest.

The timings were measured on a laptop, the program pinned to one processor,
each time being the best of several runs spread over three rounds: the
machine slowed down now and then for a few seconds, which made some operations
up to four times slower in a single round. Solaris Studio's and PROFIL/BIAS's
operations are calls into libraries, where GCC inlines GAOL's and filib++'s.

## The timings

<!-- BEGIN GENERATED TABLES (doc/compare/code/bench.py) -->

Measured on:

```
Date:            2026-09-18
Processor:       11th Gen Intel(R) Core(TM) i7-1185G7 @ 3.00GHz (taskset -c 2)
System:          Linux 5.15.0-191-generic, Ubuntu 20.04.6 LTS
C++ compiler:    g++ (Ubuntu 9.4.0-1ubuntu1~20.04.3) 9.4.0
C++ flags:       -std=c++11 -O3 -DNDEBUG (GAOL: -frounding-math -fno-fast-math -ffp-contract=off -msse2 -msse3; libieeep1788, filib++ and PROFIL/BIAS: -frounding-math -fno-fast-math -ffp-contract=off)
Fortran:         f90: Sun Fortran 95 8.7 Linux_i386 2014/10/20, flags: -O3 -xia
GAOL:            v4.3.0-42-gf58597f, CMake Release, mathlib 2.1.1 of 3rd/mathlib
libieeep1788:    1f10b89, MPFR 4.2.1, GMP 6.3.0
filib++:         3.0.2.2, interval<double, native_switched, i_mode_extended_flag>
PROFIL/BIAS:     2.0.8, x86-64-Linux-compat-gcc
```

1 000 000 operations of each kind, on the same intervals. Each time is the best of 3 rounds, each program being run in turn with the others: 15 runs for double (reference), 15 runs for GAOL, 3 runs for libieeep1788, 15 runs for filib++, 15 runs for PROFIL/BIAS, 15 runs for Solaris Studio f90 in all.

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

| Operation | double (reference) | GAOL | libieeep1788 | filib++ | PROFIL/BIAS | Solaris Studio f90 | libieeep1788 / GAOL | filib++ / GAOL | PROFIL/BIAS / GAOL | Solaris Studio f90 / GAOL |
|---|---|---|---|---|---|---|---|---|---|---|
| `add` | 1.00 | 3.68 | 213 | 7.50 | 21.7 | 24.1 | 57.8 | 2.0 | 5.9 | 6.6 |
| `sub` | 0.99 | 3.79 | 207 | 7.45 | 21.7 | 23.8 | 54.7 | 2.0 | 5.7 | 6.3 |
| `mul` | 0.99 | 16.3 | 261 | 17.0 | 22.2 | 29.0 | 16.0 | 1.0 | 1.4 | 1.8 |
| `div` | 1.01 | 13.0 | 263 | 11.7 | 21.9 | 27.0 | 20.3 | 0.9 | 1.7 | 2.1 |
| `sqr` | 0.68 | 9.33 | 124 | 13.3 | 28.2 | 75.6 | 13.3 | 1.4 | 3.0 | 8.1 |
| `sqrt` | 1.96 | 9.00 | 147 | 16.7 | 10.5 | 36.7 | 16.4 | 1.9 | 1.2 | 4.1 |
| `exp` | 5.85 | 46.7 | 2 241 | 45.9 | 16.2 | 54.5 | 48.0 | 1.0 | 0.3 | 1.2 |
| `log` | 5.06 | 67.6 | 2 601 | 29.3 | 16.9 | 55.7 | 38.4 | 0.4 | 0.3 | 0.8 |
| `sin` | 18.8 | 113 | 8 049 | 52.6 | 175 | 59.1 | 71.2 | 0.5 | 1.5 | 0.5 |
| `cos` | 18.6 | 113 | 7 205 | 53.3 | 196 | 59.7 | 63.6 | 0.5 | 1.7 | 0.5 |
| `pow_int` | 19.7 | 32.1 | 331 | 26.6 | 49.9 | 213 | 10.3 | 0.8 | 1.6 | 6.7 |
| `pow_real` | 16.1 | 138 | 9 229 | 91.4 | 53.1 | 184 | 66.9 | 0.7 | 0.4 | 1.3 |
| `line_arith` | 1.32 | 29.2 | 1 015 | 40.5 | 91.4 | 88.4 | 34.7 | 1.4 | 3.1 | 3.0 |
| `line_trig` | 37.7 | 252 | 16 676 | 141 | 442 | 229 | 66.1 | 0.6 | 1.8 | 0.9 |
| `line_pow` | 29.3 | 113 | 3 606 | 122 | 133 | 361 | 31.9 | 1.1 | 1.2 | 3.2 |
| `shekel5` | 6.22 | 325 | 13 531 | 581 | 1 599 | 2 381 | 41.6 | 1.8 | 4.9 | 7.3 |
| `block5` | 67.7 | 398 | 22 188 | 290 | 614 | 644 | 55.7 | 0.7 | 1.5 | 1.6 |

#### Total time of the 1 000 000 operations (seconds)

| Operation | double (reference) | GAOL | libieeep1788 | filib++ | PROFIL/BIAS | Solaris Studio f90 |
|---|---|---|---|---|---|---|
| `add` | 0.001 | 0.004 | 0.213 | 0.007 | 0.022 | 0.024 |
| `sub` | 0.001 | 0.004 | 0.207 | 0.007 | 0.022 | 0.024 |
| `mul` | 0.001 | 0.016 | 0.261 | 0.017 | 0.022 | 0.029 |
| `div` | 0.001 | 0.013 | 0.263 | 0.012 | 0.022 | 0.027 |
| `sqr` | 0.001 | 0.009 | 0.124 | 0.013 | 0.028 | 0.076 |
| `sqrt` | 0.002 | 0.009 | 0.147 | 0.017 | 0.010 | 0.037 |
| `exp` | 0.006 | 0.047 | 2.241 | 0.046 | 0.016 | 0.054 |
| `log` | 0.005 | 0.068 | 2.601 | 0.029 | 0.017 | 0.056 |
| `sin` | 0.019 | 0.113 | 8.049 | 0.053 | 0.175 | 0.059 |
| `cos` | 0.019 | 0.113 | 7.205 | 0.053 | 0.196 | 0.060 |
| `pow_int` | 0.020 | 0.032 | 0.331 | 0.027 | 0.050 | 0.213 |
| `pow_real` | 0.016 | 0.138 | 9.229 | 0.091 | 0.053 | 0.184 |
| `line_arith` | 0.001 | 0.029 | 1.015 | 0.040 | 0.091 | 0.088 |
| `line_trig` | 0.038 | 0.252 | 16.676 | 0.141 | 0.442 | 0.229 |
| `line_pow` | 0.029 | 0.113 | 3.606 | 0.122 | 0.133 | 0.361 |
| `shekel5` | 0.006 | 0.325 | 13.531 | 0.581 | 1.599 | 2.381 |
| `block5` | 0.068 | 0.398 | 22.188 | 0.290 | 0.614 | 0.644 |

#### Same results?

The sum of the midpoints of the results, relative to libieeep1788's, and the mean width of the results, with the relative excess of the other libraries over libieeep1788, whose bounds are the tightest:

| Operation | Σ midpoints (libieeep1788) | Δ GAOL | Δ filib++ | Δ PROFIL/BIAS | Δ Solaris Studio f90 | mean width (libieeep1788) | excess GAOL | excess filib++ | excess PROFIL/BIAS | excess Solaris Studio f90 |
|---|---|---|---|---|---|---|---|---|---|---|
| `add` | 6215.245078 | 0.0e+00 | 0.0e+00 | 1.2e-14 | 0.0e+00 | 0.144602 | 0.0e+00 | 0.0e+00 | 0.0e+00 | 0.0e+00 |
| `sub` | 15708.8058 | 0.0e+00 | 0.0e+00 | 8.3e-15 | 0.0e+00 | 0.144602 | 0.0e+00 | 0.0e+00 | 0.0e+00 | 0.0e+00 |
| `mul` | -22357.9813 | 0.0e+00 | 0.0e+00 | 4.2e-14 | 0.0e+00 | 0.721669 | 0.0e+00 | 0.0e+00 | 0.0e+00 | 0.0e+00 |
| `div` | 1150.348145 | 0.0e+00 | 0.0e+00 | 4.1e-14 | 0.0e+00 | 0.0556511 | 0.0e+00 | 0.0e+00 | 0.0e+00 | 0.0e+00 |
| `sqr` | 33351536.53 | 0.0e+00 | 0.0e+00 | 0.0e+00 | 0.0e+00 | 0.723458 | 0.0e+00 | 0.0e+00 | 0.0e+00 | 0.0e+00 |
| `sqrt` | 2269319.522 | 0.0e+00 | 0.0e+00 | 0.0e+00 | 0.0e+00 | 0.0173328 | 0.0e+00 | 2.4e-14 | 9.7e-14 | 0.0e+00 |
| `exp` | 1113373157 | -2.1e-16 | 2.1e-16 | 0.0e+00 | 0.0e+00 | 81.332 | 2.6e-15 | 2.1e-14 | 9.0e-15 | 7.3e-16 |
| `log` | 1559051.81 | 0.0e+00 | 4.5e-16 | 1.5e-16 | 0.0e+00 | 0.0185316 | 1.5e-14 | 1.4e-13 | 5.9e-14 | 0.0e+00 |
| `sin` | 372.7406966 | 1.5e-15 | 3.4e-15 | -1.7e-14 | -9.2e-16 | 0.0471741 | 1.4e-15 | 3.7e-14 | 7.4e-14 | 0.0e+00 |
| `cos` | -55217.80218 | -4.0e-16 | -1.3e-16 | 2.5e-15 | 0.0e+00 | 0.04418 | 1.8e-15 | 4.6e-14 | 9.7e-14 | 3.3e-16 |
| `pow_int` | 763027.2411 | 0.0e+00 | -9.2e-16 | 2.4e-06 | 1.5e-15 | 7.24104 | 0.0e+00 | 4.2e-15 | 1.1e-05 | 4.6e-15 |
| `pow_real` | 24977959.15 | 0.0e+00 | 1.2e-15 | 0.0e+00 | 0.0e+00 | 0.963128 | 3.6e-15 | 2.4e-13 | 1.2e-13 | 4.1e-15 |
| `line_arith` | 11641.1468 | 0.0e+00 | 0.0e+00 | 3.5e-14 | 0.0e+00 | 0.742132 | 0.0e+00 | 0.0e+00 | 0.0e+00 | 0.0e+00 |
| `line_trig` | 33352445.64 | 0.0e+00 | 0.0e+00 | 0.0e+00 | 0.0e+00 | 0.781627 | 0.0e+00 | 3.4e-15 | 6.4e-15 | 0.0e+00 |
| `line_pow` | -200633868.5 | 0.0e+00 | -5.9e-16 | 2.1e-08 | 0.0e+00 | 391.862 | 1.5e-16 | 1.4e-15 | 4.5e-07 | 1.5e-16 |
| `shekel5` | -157759.691 | 0.0e+00 | 0.0e+00 | 0.0e+00 | 0.0e+00 | 0.00992931 | 0.0e+00 | 0.0e+00 | -1.8e-16 | 0.0e+00 |
| `block5` | 54765440.5 | 2.7e-16 | 5.4e-16 | -7.1e-07 | -2.7e-16 | 456.259 | 0.0e+00 | 3.1e-15 | 5.6e-06 | 2.2e-15 |

<!-- END GENERATED TABLES -->
