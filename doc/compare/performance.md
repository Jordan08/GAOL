<!-- Copyright (c) 2026 ENSTA, France
     Created 2026-09-20 by Jordan NININ -->
# Performance: GAOL, libieeep1788, filib++, PROFIL/BIAS, Solaris Studio and Boost.Interval

Part of the [comparison](README.md) of GAOL with libieeep1788, filib++,
PROFIL/BIAS, Solaris Studio and Boost.Interval.

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
| GAOL 4.2.3 (the last version of Frédéric Goualard) | Clang 18.1, `-O3 -mfma`, the flags of interval arithmetic; GAOL built by its configure from [his repository](https://github.com/goualard-f/GAOL) (cd0ee1a), static, with the flags of GAOL V5.0.0, which its configure gives to `g++` alone, the rounding direction not preserved, its verbose mode off; [mathlib 2.1.1](https://frederic.goualard.net/software/mathlib-2.1.1.tar.gz) from his site, built by Clang 18.1 with `-O3 -mfma -ffp-contract=off` | Inline SSE2 operations, which take the rounding direction upward, as `gaol::init()` sets it, without checking it; elementary functions computed by mathlib, correctly rounded to nearest, then moved one double outward, the rounding direction set to nearest and back upward for each; `pow(x, y)` as exp(y·log(x)) |
| libieeep1788 | Clang 18.1, `-O3 -mfma`; MPFR 4.2.1 and GMP 6.3.0 built by Clang 18.1 with `-O3 -mfma` | Each bound computed by MPFR, correctly rounded |
| filib++ | Clang 18.1, `-O3 -mfma`, the flags of interval arithmetic; filib++ 3.0.2.2, the archive IBEX distributes, built by Clang 18.1 in C++11 with `-O3 -mfma` | `interval<double, native_switched, i_mode_extended_flag>`, as in IBEX: inline operations, the rounding direction set and restored by each; elementary functions of its own |
| PROFIL/BIAS | Clang 18.1, `-O3 -mfma`, the flags of interval arithmetic; PROFIL/BIAS 2.0.8 built with its `x86-64-Linux-compat-gcc` configuration, by Clang 18.1 in C++11 with `-O3 -mfma -ffp-contract=off` | `INTERVAL`, whose operations are calls to the BIAS library, each setting the rounding direction downward then upward and back to nearest; elementary functions from the libm, moved outward |
| Solaris Studio | Sun Fortran 95 8.7 (Solaris Studio 12.4, 2014), `-O3 -xia` | Calls to `libsunimath` |
| Boost.Interval | Clang 18.1, `-O3 -mfma`, the flags of interval arithmetic; the headers of Boost 1.92.0 | `interval<double>` with its default policies, `rounded_transc_opp` added for the elementary functions (see [Enclosure](enclosure.md#the-policies-of-boostinterval)): inline operations, each reading the rounding direction, setting it upward and setting the one it read back (`save_state`), the lower bounds computed negated; elementary functions from the C library, glibc 2.31, called with the rounding direction downward, then upward, neither correctly rounded nor always enclosing; sin(x) computed as cos(x − π/2); `pow(x, y)` written exp(y·log(x)), Boost.Interval having no real power |

## What the timings show

The two versions of GAOL are measured side by side: **GAOL V5.0.0**, this
branch, which bounds every elementary function with CORE-MATH, and **GAOL
4.2.3**, the last version of Frédéric Goualard, which GAOL V5.0.0 continues
and which bounds them with mathlib. Both are built without the preservation of
the rounding direction, the default of GAOL V5.0.0. Everything was built and
run again for this table with **Clang 18.1** — GMP, MPFR, libieeep1788,
filib++ (from the archive IBEX distributes) and PROFIL/BIAS as well as the two
GAOL — so that one compiler answers for every C and C++ library here. Only
Solaris Studio keeps its own, by nature. Every library was measured in the
same run, on 13 October 2026, the machine running Ubuntu 26.04.1: the
previous tables, of 27 September 2026, were measured under Ubuntu 20.04, on the
same processor with the same Clang 18.1.8, and every library, the doubles
included, takes about 30 % less time under the new system.

**What GAOL V5.0.0 changes.** The elementary functions, which mathlib used to
bound, are faster:

| | GAOL V5.0.0 | GAOL 4.2.3 | |
|---|---:|---:|---|
| `log` | 21.8 ns | 49.4 ns | 2.3 times faster |
| `pow(x, y)` | 53.0 | 94.4 | 1.8 |
| `exp` | 21.0 | 36.7 | 1.7 |
| `sin` | 50.4 | 73.4 | 1.5 |
| line of sin and cos | 129 | 161 | 1.2 |
| five-line block | 202 | 244 | 1.2 |
| `cos` | 60.3 | 71.0 | 1.2 |
| line of powers | 59.1 | 65.2 | 1.1 |

Two things go at once with mathlib: the outward move of each bound, and the
two changes of rounding direction each function made, to nearest before
mathlib and back upward after. **And the bounds are tighter, not looser**: in
the last table below, GAOL V5.0.0's results are no wider than libieeep1788's,
which computes every bound with MPFR, for **every one of the seventeen
operations** (excess 0.0e+00), where GAOL 4.2.3 is wider on the square root,
`exp`, `log`, `sin`, `cos`, both powers and the lines that use them.

The arithmetic costs GAOL V5.0.0 more: + and − take 2.7 ns against 2.5, ×
12.4 against 11.0 and ÷ 9.7 against 8.4 (10 to 15 % more), the square 7.6
against 6.1, the square root 7.7 against 6.9, `pow(x, 3)` 13.8 against 9.7,
the arithmetic line 21.3 against 19.4, and Shekel 5 277 ns against 185, half
as much again, which the costs of its 20 squares, 45 additions and
subtractions and 5 divisions do not account for. What GAOL V5.0.0 does more
there: each operation checks that the rounding
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

**Arithmetic.** GAOL is the fastest on + and −, 2.7 ns for GAOL V5.0.0 (2.5
and 2.4 for GAOL 4.2.3), 2.1 times as fast as filib++ (5.6 ns) and 5.6 to 6.5
times as fast as PROFIL/BIAS (15 ns) and Solaris Studio (17.5 ns); × and ÷
take GAOL V5.0.0 12.4 and 9.7 ns, against 16.7 and 9.7 ns for filib++, 15.4 ns
for PROFIL/BIAS and 20 to 21 ns for Solaris Studio. An addition of intervals
costs GAOL V5.0.0 2.7 times an addition of doubles: two additions, and the check
that the rounding direction is upward, where filib++ sets the rounding direction
and restores it, and PROFIL/BIAS calls a function of BIAS that sets it downward,
then upward, then back to nearest — which is why its four arithmetic operations
all take about the same time. The integer power `pow(x, 3)` takes GAOL V5.0.0
13.8 ns, computed from exact products, whose `fma()` is one instruction with
`-mfma`, and GAOL 4.2.3 9.7 ns: filib++'s `power(x, 3)` takes 19 ns,
PROFIL/BIAS's 35 ns and Solaris Studio's `x**3` 146 ns; `sqr` takes GAOL V5.0.0
7.6 ns, against 9.0 for filib++, 19.7 for PROFIL/BIAS and 55 for Solaris Studio.

**Elementary functions.** PROFIL/BIAS is the fastest on the square root
(3.9 ns), exp (10.6 ns), log (11.1 ns) and the real power `pow(x, y)` (34 ns),
which it computes from the libm of the system, moved outward, without correct
rounding: its bounds are among the widest (below), and its sin and cos, computed
by its own argument reduction, are the slowest after libieeep1788's, 122 and
136 ns. GAOL V5.0.0 comes next on exp and log, 21 and 22 ns — faster than
Boost.Interval (29 ns each), filib++ (34 and 29 ns) and Solaris Studio (38 ns
each) — with **correctly rounded** values, where the others are not. filib++ is
the fastest on sin and cos (36.6 and 37.6 ns) with its own polynomials, then
Solaris Studio (41 and 43 ns), then GAOL V5.0.0 (50 and 60 ns): GAOL pays there for correct
rounding over the whole range, and for dividing the bounds by an interval
enclosing π to find where the function is monotonic, asking CORE-MATH for the
signs of the derivative where the division cannot tell
([issue #6](https://github.com/Jordan08/GAOL/issues/6)). GAOL V5.0.0's sin
became faster than its cos when CORE-MATH rewrote `sin.c` (upstream 6b84457).
On `pow(x, y)` GAOL V5.0.0 takes 53 ns, against 72 for filib++, 73 for
Boost.Interval, 94 for GAOL 4.2.3 and 125 for Solaris Studio.

**Formulas.** GAOL is the fastest on the arithmetic line (21.3 ns for GAOL
V5.0.0 and 19.4 for GAOL 4.2.3, against 29 for filib++ and about 62 for Solaris
Studio, Boost.Interval and PROFIL/BIAS), on the line of powers (59 and 65 ns,
against 96 for filib++, 252 for Solaris Studio and 92 for PROFIL/BIAS) and on
Shekel 5 (277 and 185 ns, against 415, 1 711 and 1 078: Shekel 5 is 20 squares,
45 additions and subtractions and 5 divisions, and its squares cost Solaris
Studio 55 ns each and PROFIL/BIAS 20). GAOL V5.0.0 alone is the fastest on the
five-line block (202 ns, against 226 for filib++, 244 for GAOL 4.2.3, 425 for
PROFIL/BIAS and 462 for Solaris Studio). filib++ is the fastest on the line of
sin and cos (101 ns, against 129 for GAOL V5.0.0, 161 for GAOL 4.2.3, 165 for
Solaris Studio and 306 for PROFIL/BIAS), where its sin and cos weigh most.

**libieeep1788** is 12 to 123 times slower than GAOL V5.0.0: every bound is an
MPFR computation, about 140 ns for an addition and 5 to 6.5 µs for sin, cos
and the real power. Its own README warns that its focus is correctness, not
speed. GAOL V5.0.0 now gives **the same bounds as it does**, tightest
everywhere, 12 to 123 times faster.

**Boost.Interval** is slower than GAOL V5.0.0 on every operation: 4.7 times
on + and −, 1.7 to 2.9 times on ×, ÷, the square and the square root, 1.3 to
1.4 times on exp, log, sin and the powers, 1.1 times on cos, 2.9 times on the
arithmetic line and 3.3 times on Shekel 5. Each of its
operations reads the rounding direction (`fegetround`), sets it upward
(`fesetround`) and, at its end, sets the one it read back: two changes of
direction where GAOL checks the direction and changes nothing. Its elementary
functions add a change to the downward direction and back for the lower bound,
and its sin goes through the reduction of cos(x − π/2) by an interval
enclosing 2π. Its bounds are those of the C library, which are not correctly
rounded, and of which 27 to 41 % missed the value of the function with glibc
2.31 ([Enclosure](enclosure.md)), which has not been run under glibc 2.43 yet.

**The results** are the same: the sums of the midpoints of the million results
agree to 4e-15 relatively, apart from PROFIL/BIAS's (below) and the sines of
GAOL 4.2.3 and Boost.Interval, 1.4e-13 and 1.3e-13 apart, their sum being
small, 373 for a million values, which makes 5e-17 per result. The arithmetic
operations give the tightest intervals in every library, and so do the square
roots, except those of GAOL 4.2.3, filib++ and PROFIL/BIAS. Of the elementary
functions, **only GAOL V5.0.0's are the tightest on every operation**: on
average the others are wider than libieeep1788's by up to 5.9e-14 relatively
with GAOL 4.2.3 (the real power, computed as exp(y·log(x))), 4e-15 to 2.4e-13
with filib++ (1.4e-13 for log, 2.4e-13 for the real power), 9e-15 to 1.2e-13
with PROFIL/BIAS, and at most 5e-15 with Solaris Studio; with Boost.Interval,
by 1.4e-14 and 2.5e-14 on cos and sin and 3.7e-14 on the real power, its exp
and log being as tight as libieeep1788's on average: the sums cannot show that
its upper bound of exp is one double too low in 2 to 3 % of the results of the
test of [enclosure](enclosure.md). PROFIL/BIAS's integer power is far wider,
1.1e-05 on average: it computes `Power(x, n)` as exp(n log x), where the others
multiply. Its arithmetic results have midpoints that differ by 8e-15 to 4e-14
relatively while their widths are the tightest: the intervals are the same, but
its `Mid` computes inf + (sup − inf)/2 **rounded upward**, one double above the
midpoint for 4 % of the sums of the benchmark, where the others round to
nearest.

The timings were measured on a laptop, on 13 October 2026, each time being the
best of the runs of three rounds, each program being run in turn with the
others, on a processor of its own (`taskset -c 2`). libieeep1788's times, from
a single run of each operation in each round, are the least steady. Solaris
Studio's and PROFIL/BIAS's operations are calls into libraries, where the
compiler inlines GAOL's, filib++'s and Boost.Interval's.

## The timings

<!-- BEGIN GENERATED TABLES (doc/compare/code/bench.py) -->

Measured on:

```
Date:            2026-10-13
Processor:       11th Gen Intel(R) Core(TM) i7-1185G7 @ 3.00GHz (taskset -c 2)
System:          Linux 7.0.0-38-generic, Ubuntu 26.04.1 LTS
C++ compiler:    Ubuntu clang version 18.1.8 (20ubuntu8)
C++ flags:       -std=c++11 -O3 -DNDEBUG -mfma (GAOL: -frounding-math -fno-fast-math -ffp-contract=off -msse2 -msse3 -mfma; libieeep1788, filib++, PROFIL/BIAS and Boost.Interval: -frounding-math -fno-fast-math -ffp-contract=off)
Fortran:         f90: Sun Fortran 95 8.7 Linux_i386 2014/10/20, flags: -O3 -xia
GAOL V5.0.0:     the branch of this checkout (v4.3.2-777-g641ca81), CMake Release, CORE-MATH of 3rd/math-core compiled into the library, the rounding direction not preserved
GAOL 4.2.3:      the last version of Frédéric Goualard (cd0ee1a of https://github.com/goualard-f/GAOL), its configure, the rounding direction not preserved, mathlib 2.1.1
libieeep1788:    1f10b89, MPFR 4.2.1, GMP 6.3.0
filib++:         3.0.2.2, interval<double, native_switched, i_mode_extended_flag>
PROFIL/BIAS:     2.0.8, x86-64-Linux-compat-gcc configuration, built by clang-18 and clang++-18
Boost.Interval:  Boost 1.92.0, interval<double> with save_state<rounded_transc_opp<double> > and checking_strict<double>, the elementary functions of the C library, glibc 2.43
```

1 000 000 operations of each kind, on the same intervals. Each time is the best of 3 rounds, each program being run in turn with the others: 15 runs for double (reference), 3 runs for libieeep1788, 15 runs for GAOL V5.0.0, 15 runs for GAOL 4.2.3, 15 runs for filib++, 15 runs for Solaris Studio f90, 15 runs for PROFIL/BIAS, 15 runs for Boost.Interval in all.

#### The operations

a and b are intervals centred in [−10, 10], p in [1, 10], e in [0.5, 2.5], the arguments of Shekel 5 in [0, 10]; their widths are 10^u, u uniform in [−6, 0] ([−6, −1] for e).

| Operation | Computes |
|---|---|
| `add` | a + b |
| `sub` | a − b |
| `mul` | a × b |
| `div` | a / p |
| `sqr` | a² (`sqr`, `square` in Boost.Interval, `x**2` in Fortran) |
| `sqrt` | √p |
| `exp` | exp(a) |
| `log` | log(p) |
| `sin` | sin(a) |
| `cos` | cos(a) |
| `pow_int` | a³ (`pow(x, int)` in GAOL and Boost.Interval, `pown`, `power(x, int)` in filib++, `x**3`) |
| `pow_real` | p^e (`pow(x, y)`, `x**y`; exp(e log p) with Boost.Interval, which has no real power) |
| `line_arith` | (a + b)(a − b) / p |
| `line_trig` | sin(a) cos(b) + a² |
| `line_pow` | √p · a³ − exp(b / p) |
| `shekel5` | Shekel 5: −Σᵢ 1 / (Σⱼ (xⱼ − aᵢⱼ)² + cᵢ), 5 terms, 4 variables |
| `block5` | five lines: t1 = ab + p; t2 = sin(t1) cos(b); t3 = a² + t2/p; t4 = exp(t2) − b³; t3 t4 + √p |

#### Time per operation (nanoseconds)

| Operation | double (reference) | libieeep1788 | GAOL V5.0.0 | GAOL 4.2.3 | filib++ | Solaris Studio f90 | PROFIL/BIAS | Boost.Interval | libieeep1788 / GAOL V5.0.0 | GAOL 4.2.3 / GAOL V5.0.0 | filib++ / GAOL V5.0.0 | Solaris Studio f90 / GAOL V5.0.0 | PROFIL/BIAS / GAOL V5.0.0 | Boost.Interval / GAOL V5.0.0 |
|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|
| `add` | 1.00 | 136 | 2.72 | 2.46 | 5.63 | 17.7 | 15.3 | 12.7 | 49.9 | 0.9 | 2.1 | 6.5 | 5.6 | 4.7 |
| `sub` | 0.93 | 142 | 2.70 | 2.44 | 5.62 | 17.3 | 15.0 | 12.7 | 52.6 | 0.9 | 2.1 | 6.4 | 5.6 | 4.7 |
| `mul` | 0.90 | 174 | 12.4 | 11.0 | 16.7 | 21.1 | 15.4 | 21.4 | 14.0 | 0.9 | 1.3 | 1.7 | 1.2 | 1.7 |
| `div` | 0.92 | 168 | 9.72 | 8.43 | 9.71 | 19.8 | 15.4 | 18.8 | 17.3 | 0.9 | 1.0 | 2.0 | 1.6 | 1.9 |
| `sqr` | 0.59 | 87.7 | 7.56 | 6.13 | 8.97 | 55.4 | 19.7 | 16.3 | 11.6 | 0.8 | 1.2 | 7.3 | 2.6 | 2.2 |
| `sqrt` | 1.44 | 99.0 | 7.72 | 6.85 | 16.5 | 26.6 | 3.90 | 22.0 | 12.8 | 0.9 | 2.1 | 3.4 | 0.5 | 2.9 |
| `exp` | 3.74 | 1 463 | 21.0 | 36.7 | 33.8 | 38.2 | 10.6 | 29.2 | 69.8 | 1.7 | 1.6 | 1.8 | 0.5 | 1.4 |
| `log` | 3.68 | 1 915 | 21.8 | 49.4 | 29.1 | 38.1 | 11.1 | 29.2 | 88.0 | 2.3 | 1.3 | 1.7 | 0.5 | 1.3 |
| `sin` | 15.4 | 5 463 | 50.4 | 73.4 | 36.6 | 40.8 | 122 | 69.5 | 108 | 1.5 | 0.7 | 0.8 | 2.4 | 1.4 |
| `cos` | 14.2 | 4 849 | 60.3 | 71.0 | 37.6 | 42.7 | 136 | 66.8 | 80.4 | 1.2 | 0.6 | 0.7 | 2.3 | 1.1 |
| `pow_int` | 14.7 | 240 | 13.8 | 9.73 | 19.2 | 146 | 34.8 | 19.7 | 17.4 | 0.7 | 1.4 | 10.6 | 2.5 | 1.4 |
| `pow_real` | 12.0 | 6 489 | 53.0 | 94.4 | 72.4 | 125 | 33.9 | 73.2 | 123 | 1.8 | 1.4 | 2.4 | 0.6 | 1.4 |
| `line_arith` | 1.15 | 662 | 21.3 | 19.4 | 29.0 | 61.7 | 62.7 | 62.0 | 31.1 | 0.9 | 1.4 | 2.9 | 2.9 | 2.9 |
| `line_trig` | 29.4 | 10 861 | 129 | 161 | 101 | 165 | 306 | 192 | 84.1 | 1.2 | 0.8 | 1.3 | 2.4 | 1.5 |
| `line_pow` | 21.8 | 2 382 | 59.1 | 65.2 | 95.6 | 252 | 92.4 | 115 | 40.3 | 1.1 | 1.6 | 4.3 | 1.6 | 1.9 |
| `shekel5` | 2.56 | 9 119 | 277 | 185 | 415 | 1 711 | 1 078 | 917 | 32.9 | 0.7 | 1.5 | 6.2 | 3.9 | 3.3 |
| `block5` | 49.3 | 13 810 | 202 | 244 | 226 | 462 | 425 | 347 | 68.5 | 1.2 | 1.1 | 2.3 | 2.1 | 1.7 |

#### Total time of the 1 000 000 operations (seconds)

| Operation | double (reference) | libieeep1788 | GAOL V5.0.0 | GAOL 4.2.3 | filib++ | Solaris Studio f90 | PROFIL/BIAS | Boost.Interval |
|---|---|---|---|---|---|---|---|---|
| `add` | 0.001 | 0.136 | 0.003 | 0.002 | 0.006 | 0.018 | 0.015 | 0.013 |
| `sub` | 0.001 | 0.142 | 0.003 | 0.002 | 0.006 | 0.017 | 0.015 | 0.013 |
| `mul` | 0.001 | 0.174 | 0.012 | 0.011 | 0.017 | 0.021 | 0.015 | 0.021 |
| `div` | 0.001 | 0.168 | 0.010 | 0.008 | 0.010 | 0.020 | 0.015 | 0.019 |
| `sqr` | 0.001 | 0.088 | 0.008 | 0.006 | 0.009 | 0.055 | 0.020 | 0.016 |
| `sqrt` | 0.001 | 0.099 | 0.008 | 0.007 | 0.016 | 0.027 | 0.004 | 0.022 |
| `exp` | 0.004 | 1.463 | 0.021 | 0.037 | 0.034 | 0.038 | 0.011 | 0.029 |
| `log` | 0.004 | 1.915 | 0.022 | 0.049 | 0.029 | 0.038 | 0.011 | 0.029 |
| `sin` | 0.015 | 5.463 | 0.050 | 0.073 | 0.037 | 0.041 | 0.122 | 0.069 |
| `cos` | 0.014 | 4.849 | 0.060 | 0.071 | 0.038 | 0.043 | 0.136 | 0.067 |
| `pow_int` | 0.015 | 0.240 | 0.014 | 0.010 | 0.019 | 0.146 | 0.035 | 0.020 |
| `pow_real` | 0.012 | 6.489 | 0.053 | 0.094 | 0.072 | 0.125 | 0.034 | 0.073 |
| `line_arith` | 0.001 | 0.662 | 0.021 | 0.019 | 0.029 | 0.062 | 0.063 | 0.062 |
| `line_trig` | 0.029 | 10.861 | 0.129 | 0.161 | 0.101 | 0.165 | 0.306 | 0.192 |
| `line_pow` | 0.022 | 2.382 | 0.059 | 0.065 | 0.096 | 0.252 | 0.092 | 0.115 |
| `shekel5` | 0.003 | 9.119 | 0.277 | 0.185 | 0.415 | 1.711 | 1.078 | 0.917 |
| `block5` | 0.049 | 13.810 | 0.202 | 0.244 | 0.226 | 0.462 | 0.425 | 0.347 |

#### Same results?

The sum of the midpoints of the results, relative to libieeep1788's, and the mean width of the results, with the relative excess of the other libraries over libieeep1788, whose bounds are the tightest:

| Operation | Σ midpoints (libieeep1788) | Δ GAOL V5.0.0 | Δ GAOL 4.2.3 | Δ filib++ | Δ Solaris Studio f90 | Δ PROFIL/BIAS | Δ Boost.Interval | mean width (libieeep1788) | excess GAOL V5.0.0 | excess GAOL 4.2.3 | excess filib++ | excess Solaris Studio f90 | excess PROFIL/BIAS | excess Boost.Interval |
|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|
| `add` | 6215.245078 | 0.0e+00 | 0.0e+00 | 0.0e+00 | 0.0e+00 | 1.2e-14 | 0.0e+00 | 0.144602 | 0.0e+00 | 0.0e+00 | 0.0e+00 | 0.0e+00 | 0.0e+00 | 0.0e+00 |
| `sub` | 15708.8058 | 0.0e+00 | 0.0e+00 | 0.0e+00 | 0.0e+00 | 8.3e-15 | 0.0e+00 | 0.144602 | 0.0e+00 | 0.0e+00 | 0.0e+00 | 0.0e+00 | 0.0e+00 | 0.0e+00 |
| `mul` | -22357.9813 | 0.0e+00 | 0.0e+00 | 0.0e+00 | 0.0e+00 | 4.2e-14 | 0.0e+00 | 0.721669 | 0.0e+00 | 0.0e+00 | 0.0e+00 | 0.0e+00 | 0.0e+00 | 0.0e+00 |
| `div` | 1150.348145 | 0.0e+00 | 0.0e+00 | 0.0e+00 | 0.0e+00 | 4.1e-14 | 0.0e+00 | 0.0556511 | 0.0e+00 | 0.0e+00 | 0.0e+00 | 0.0e+00 | 0.0e+00 | 0.0e+00 |
| `sqr` | 33351536.53 | 0.0e+00 | 0.0e+00 | 0.0e+00 | 0.0e+00 | 0.0e+00 | 0.0e+00 | 0.723458 | 0.0e+00 | 0.0e+00 | 0.0e+00 | 0.0e+00 | 0.0e+00 | 0.0e+00 |
| `sqrt` | 2269319.522 | 0.0e+00 | -2.1e-16 | 0.0e+00 | 0.0e+00 | 0.0e+00 | 0.0e+00 | 0.0173328 | 0.0e+00 | 1.4e-14 | 2.4e-14 | 0.0e+00 | 9.7e-14 | 0.0e+00 |
| `exp` | 1113373157 | 0.0e+00 | -2.1e-16 | 2.1e-16 | 0.0e+00 | 0.0e+00 | 0.0e+00 | 81.332 | 0.0e+00 | 2.6e-15 | 2.1e-14 | 7.3e-16 | 9.0e-15 | 0.0e+00 |
| `log` | 1559051.81 | 0.0e+00 | 0.0e+00 | 4.5e-16 | 0.0e+00 | 1.5e-16 | 0.0e+00 | 0.0185316 | 0.0e+00 | 1.5e-14 | 1.4e-13 | 0.0e+00 | 5.9e-14 | 0.0e+00 |
| `sin` | 372.7406966 | 0.0e+00 | 1.4e-13 | 4.0e-15 | -9.2e-16 | -1.6e-14 | 1.3e-13 | 0.0471741 | 0.0e+00 | 1.3e-14 | 3.7e-14 | 0.0e+00 | 7.4e-14 | 2.5e-14 |
| `cos` | -55217.80218 | 0.0e+00 | -4.0e-16 | -1.3e-16 | 0.0e+00 | 2.5e-15 | 2.6e-16 | 0.04418 | 0.0e+00 | 2.1e-15 | 4.6e-14 | 6.6e-16 | 9.7e-14 | 1.4e-14 |
| `pow_int` | 763027.2411 | 0.0e+00 | -3.1e-16 | -3.1e-16 | 1.8e-15 | 2.4e-06 | -3.1e-16 | 7.24104 | 0.0e+00 | 4.2e-15 | 4.2e-15 | 4.6e-15 | 1.1e-05 | 4.2e-15 |
| `pow_real` | 24977959.15 | 0.0e+00 | 0.0e+00 | 1.2e-15 | 0.0e+00 | 0.0e+00 | 0.0e+00 | 0.963128 | 0.0e+00 | 5.9e-14 | 2.4e-13 | 4.1e-15 | 1.2e-13 | 3.7e-14 |
| `line_arith` | 11641.1468 | 1.6e-16 | 1.6e-16 | 0.0e+00 | 0.0e+00 | 3.5e-14 | 0.0e+00 | 0.742132 | 0.0e+00 | 0.0e+00 | 0.0e+00 | 0.0e+00 | 0.0e+00 | 0.0e+00 |
| `line_trig` | 33352445.64 | 0.0e+00 | 0.0e+00 | 0.0e+00 | 0.0e+00 | 0.0e+00 | 0.0e+00 | 0.781627 | 0.0e+00 | 3.0e-16 | 3.4e-15 | 0.0e+00 | 6.4e-15 | 8.9e-16 |
| `line_pow` | -200633868.5 | 0.0e+00 | 0.0e+00 | -8.9e-16 | 0.0e+00 | 2.1e-08 | 0.0e+00 | 391.862 | 0.0e+00 | 3.0e-16 | 1.4e-15 | 1.5e-16 | 4.5e-07 | 1.5e-16 |
| `shekel5` | -157759.691 | 0.0e+00 | 0.0e+00 | 0.0e+00 | 0.0e+00 | 0.0e+00 | 0.0e+00 | 0.00992931 | 0.0e+00 | 0.0e+00 | 0.0e+00 | 0.0e+00 | -1.8e-16 | 0.0e+00 |
| `block5` | 54765440.5 | 0.0e+00 | 2.7e-16 | 5.4e-16 | -2.7e-16 | -7.1e-07 | 5.4e-16 | 456.259 | 0.0e+00 | 3.3e-15 | 3.1e-15 | 2.2e-15 | 5.6e-06 | 4.0e-15 |

<!-- END GENERATED TABLES -->
