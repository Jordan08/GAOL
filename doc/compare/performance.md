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
| GAOL V5.0.0 (this branch) | Clang 21.1, `-O3 -mfma`, the flags of interval arithmetic; GAOL built by CMake in Release with `-mfma` (`GAOL_FMA`), the sources of CORE-MATH compiled into it, the rounding direction not preserved (`GAOL_PRESERVE_ROUNDING` off, its default) | Called under the names of IEEE 1788-2015, the namespace `gaol_ieee1788` (`pown(x, n)`, `pow(x, y)`), as the other libraries of the standard (GAOL v5); inline SSE2 operations, each checking that the rounding direction is upward and setting it when it is not; **every** elementary function CORE-MATH's, correctly rounded in that direction, so the bounds are the tightest ones and the direction is never switched |
| GAOL 4.2.3 (the last version of Frédéric Goualard) | Clang 21.1, `-O3 -mfma`, the flags of interval arithmetic; GAOL built by its configure from [his repository](https://github.com/goualard-f/GAOL) (cd0ee1a), static, with the flags of GAOL V5.0.0, which its configure gives to `g++` alone, the rounding direction not preserved, its verbose mode off; [mathlib 2.1.1](https://frederic.goualard.net/software/mathlib-2.1.1.tar.gz) from his site, built by Clang 21.1 with `-O3 -mfma -ffp-contract=off` | Called under its own names, `gaol`; inline SSE2 operations, which take the rounding direction upward, as `gaol::init()` sets it, without checking it; elementary functions computed by mathlib, correctly rounded to nearest, then moved one double outward, the rounding direction set to nearest and back upward for each; `pow(x, y)` as exp(y·log(x)) |
| libieeep1788 | Clang 21.1, `-O3 -mfma`; MPFR 4.2.1 and GMP 6.3.0 built by Clang 21.1 with `-O3 -mfma` | Each bound computed by MPFR, correctly rounded |
| filib++ | Clang 21.1, `-O3 -mfma`, the flags of interval arithmetic; filib++ 3.0.2.2, the archive IBEX distributes, built by Clang 21.1 in C++11 with `-O3 -mfma` | `interval<double, native_switched, i_mode_extended_flag>`, as in IBEX: inline operations, the rounding direction set and restored by each; elementary functions of its own |
| PROFIL/BIAS | Clang 21.1, `-O3 -mfma`, the flags of interval arithmetic; PROFIL/BIAS 2.0.8 built with its `x86-64-Linux-compat-gcc` configuration, by Clang 21.1 in C++11 with `-O3 -mfma -ffp-contract=off` | `INTERVAL`, whose operations are calls to the BIAS library, each setting the rounding direction downward then upward and back to nearest; elementary functions from the libm, moved outward |
| Solaris Studio | Sun Fortran 95 8.7 (Solaris Studio 12.4, 2014), `-O3 -xia` | Calls to `libsunimath` |
| Boost.Interval | Clang 21.1, `-O3 -mfma`, the flags of interval arithmetic; the headers of Boost 1.92.0 | `interval<double>` with its default policies, `rounded_transc_opp` added for the elementary functions (see [Enclosure](enclosure.md#the-policies-of-boostinterval)): inline operations, each reading the rounding direction, setting it upward and setting the one it read back (`save_state`), the lower bounds computed negated; elementary functions from the C library, glibc 2.43, called with the rounding direction downward, then upward, neither correctly rounded nor always enclosing; sin(x) computed as cos(x − π/2); `pow(x, y)` written exp(y·log(x)), Boost.Interval having no real power |

## What the timings show

The two versions of GAOL are measured side by side: **GAOL V5.0.0**, this
branch, which bounds every elementary function with CORE-MATH, and **GAOL
4.2.3**, the last version of Frédéric Goualard, which GAOL V5.0.0 continues
and which bounds them with mathlib. Both are built without the preservation of
the rounding direction, the default of GAOL V5.0.0. GAOL V5.0.0 is called
under the names of IEEE 1788-2015, `gaol_ieee1788`, as the other libraries
of the standard are, and GAOL 4.2.3, which has only its own, under `gaol`: of
the operations here, only the powers have other names there, `pown(x, 3)`
and the real power `pow(x, y)` of the standard, which are the functions
`gaol::pow` takes for an integer exponent and, on the positive bases of the
benchmark, for an interval one (see
[Using GAOL](../using.md#the-powers-in-the-two-namespaces)). Everything was
built and run again for this table with **Clang 21.1** — GMP, MPFR,
libieeep1788, filib++ (from the archive IBEX distributes) and PROFIL/BIAS as
well as the two GAOL — so that one compiler answers for every C and C++
library here. Only Solaris Studio keeps its own, by nature. Every library was
measured in the same run, on 17 October 2026, the machine running Ubuntu
26.04.1 (the tables before, of 13 October, were measured on the same processor
with the same Clang 21.1: the other libraries took between 5 % less and 16 %
more time then than in this run, GAOL V5.0.0 9 to 34 % more, its square, its
additions and subtractions and the flag its operations tested for the AVX-512
path having changed since).

**What GAOL V5.0.0 changes.** The elementary functions, which mathlib used to
bound, are faster, and so is the integer power:

| | GAOL V5.0.0 | GAOL 4.2.3 | |
|---|---:|---:|---|
| `log` | 19.7 ns | 46.8 ns | 2.4 times faster |
| `pow(x, y)` | 47.4 | 90.2 | 1.9 |
| `exp` | 18.6 | 34.7 | 1.9 |
| `pow(x, 3)` | 5.34 | 9.06 | 1.7 |
| `sin` | 42.9 | 67.7 | 1.6 |
| line of sin and cos | 119 | 147 | 1.2 |
| five-line block | 190 | 224 | 1.2 |
| `cos` | 54.4 | 63.1 | 1.2 |
| line of powers | 53.7 | 61.7 | 1.1 |

Two things go at once with mathlib: the outward move of each bound, and the
two changes of rounding direction each function made, to nearest before
mathlib and back upward after. The integer power is computed from exact
products, both bounds at once in an SSE2 register, with an error bound proved
beforehand (point B of `TODO.md`): it took 13.8 ns before, with an exact
product and a rest carried apart for each bound. **And the bounds are tighter,
not looser**: in the last table below, GAOL V5.0.0's results are no wider than
libieeep1788's, which computes every bound with MPFR, for **every one of the
seventeen operations** (excess 0.0e+00), where GAOL 4.2.3 is wider on the
square root, `exp`, `log`, `sin`, `cos`, both powers and the lines that use
them.

The arithmetic costs GAOL V5.0.0 about what it costs GAOL 4.2.3: + and −
take 2.3 and 2.4 ns against 2.4 and 2.3, × 10.3 against 10.3 and ÷ 7.9
against 7.7, the square 6.2 against 5.9, and the arithmetic line 19.6 against
18.9. The square root takes 6.9 ns against 5.9, and Shekel 5 210 ns against
170, a quarter more, of which the costs of its 20 squares, 45 additions and
subtractions and 5 divisions explain about 7 ns of the 40. What GAOL V5.0.0
does more there: each operation checks that the rounding direction is upward,
with an addition, and sets it when it is not, where GAOL 4.2.3 takes it upward
on trust; and the square root is the tightest interval whatever the rounding
of the C library's `sqrt`, where GAOL 4.2.3's is wider (excess 1.4e-14). The check is what a
program needs that changes the rounding direction: the benchmark sums the
results rounding to nearest, and GAOL 4.2.3's own `midpoint()` leaves the
direction to nearest. Left so, the operations of GAOL 4.2.3 timed next gave
bounds narrower than libieeep1788's, which did not enclose the exact results;
the benchmark now sets the direction back after its sums
(`code/bench_common.h`).

**Arithmetic.** GAOL is the fastest on + and −, 2.3 and 2.4 ns for GAOL V5.0.0
(2.4 and 2.3 for GAOL 4.2.3), 2.0 to 2.2 times as fast as filib++ (5.1 and
4.9 ns) and 6.3 to 7.6 times as fast as PROFIL/BIAS (15 ns) and Solaris Studio
(17.6 and 17.8 ns); × and ÷ take GAOL V5.0.0 10.3 and 7.9 ns, against 12.3 and
8.8 ns for filib++, 15.4 ns for PROFIL/BIAS and 20 to 21 ns for Solaris Studio.
An addition of intervals costs GAOL V5.0.0 2.4 times an addition of doubles:
two additions,
and the check that the rounding direction is upward, where filib++ sets the
rounding direction and restores it, and PROFIL/BIAS calls a function of BIAS
that sets it downward, then upward, then back to nearest — which is why its
four arithmetic operations all take about the same time. The integer power
`pown(x, 3)` takes GAOL V5.0.0 5.3 ns and GAOL 4.2.3 9.1 ns: filib++'s
`power(x, 3)` takes 18 ns, Boost.Interval's 20 ns, PROFIL/BIAS's 35 ns and
Solaris Studio's `x**3` 145 ns; `sqr` takes GAOL V5.0.0 6.2 ns, against 8.2
for filib++, 19.6 for PROFIL/BIAS and 56 for Solaris Studio.

**Elementary functions.** PROFIL/BIAS is the fastest on the square root
(3.9 ns), exp (10.6 ns), log (11.5 ns) and the real power `pow(x, y)` (34 ns),
which it computes from the libm of the system, moved outward, without correct
rounding: its bounds are among the widest (below), and its sin and cos,
computed by its own argument reduction, are the slowest after libieeep1788's,
123 and 138 ns. GAOL V5.0.0 comes next on exp and log, 19 and 20 ns — faster
than Boost.Interval (30 ns each), filib++ (30 and 27 ns) and Solaris Studio
(39 and 38 ns) — with **correctly rounded** values, where the others are not.
filib++ is the fastest on sin and cos (33.1 and 34.1 ns) with its own
polynomials, then Solaris Studio (41 and 43 ns), then GAOL V5.0.0 (43 and
54 ns): GAOL pays there for correct rounding over the whole range, and for
dividing the bounds by an interval enclosing π to find where the function is
monotonic, asking CORE-MATH for the signs of the derivative where the division
cannot tell ([issue #6](https://github.com/Jordan08/GAOL/issues/6)). GAOL
V5.0.0's sin became faster than its cos when CORE-MATH rewrote `sin.c`
(upstream 6b84457). On `pow(x, y)` GAOL V5.0.0 takes 47 ns, against 66 for
filib++, 73 for Boost.Interval, 90 for GAOL 4.2.3 and 125 for Solaris Studio.

**Formulas.** GAOL is the fastest on the arithmetic line (19.6 ns for GAOL
V5.0.0 and 18.9 for GAOL 4.2.3, against 30 for filib++ and about 62 to 64 for
Solaris Studio, PROFIL/BIAS and Boost.Interval), on the line of powers (54 and
62 ns, against 88 for filib++, 93 for PROFIL/BIAS and 254 for Solaris Studio)
and on Shekel 5 (210 and 170 ns, against 416 for filib++, 921 for
Boost.Interval, 1 085 for PROFIL/BIAS and 1 720 for Solaris Studio: Shekel 5 is
20 squares, 45 additions and subtractions and 5 divisions, and its squares cost
Solaris Studio 56 ns each and PROFIL/BIAS 20). GAOL V5.0.0 alone is the
fastest on the five-line block (190 ns, against 215 for filib++, 224 for GAOL
4.2.3, 433 for PROFIL/BIAS and 464 for Solaris Studio). filib++ is the fastest
on the line of sin and cos (98.5 ns, against 119 for GAOL V5.0.0, 147 for GAOL
4.2.3, 166 for Solaris Studio and 310 for PROFIL/BIAS), where its sin and cos
weigh most.

**libieeep1788** is 14 to 138 times slower than GAOL V5.0.0: every bound is an
MPFR computation, about 140 ns for an addition and 5 to 7 µs for sin, cos and
the real power. Its own README warns that its focus is correctness, not speed.
GAOL V5.0.0 now gives **the same bounds as it does**, tightest everywhere, 14
to 138 times faster.

**Boost.Interval** is slower than GAOL V5.0.0 on every operation: 5.3 to 5.5
times on + and −, 2.1 to 3.7 times on ×, ÷, the square, the square root and the
integer power, 1.2 to 1.6 times on exp, log, sin, cos and the real power, 3.1
times on the arithmetic line and 4.4 times on Shekel 5. Each of its
operations reads the rounding direction (`fegetround`), sets it upward
(`fesetround`) and, at its end, sets the one it read back: two changes of
direction where GAOL checks the direction and changes nothing. Its elementary
functions add a change to the downward direction and back for the lower bound,
and its sin goes through the reduction of cos(x − π/2) by an interval
enclosing 2π. Its bounds are those of the C library, which are not correctly
rounded, and of which 20 to 34 % miss the value of the function with glibc
2.43 ([Enclosure](enclosure.md)).

**The results** are the same: the sums of the midpoints of the million results
agree to 4e-15 relatively, apart from PROFIL/BIAS's (below) and the sines of
GAOL 4.2.3 and Boost.Interval, 1.4e-13 and 1.3e-13 apart, their sum being
small, 373 for a million values, which makes 5e-17 per result. The arithmetic
operations give the tightest intervals in every library, and so do the square
roots, except those of GAOL 4.2.3, filib++ and PROFIL/BIAS. Of the elementary
functions, **only GAOL V5.0.0's are the tightest on every operation**: on
average the others are wider than libieeep1788's by up to 5.9e-14 relatively
with GAOL 4.2.3 (the real power, computed as exp(y·log(x))), 3.4e-15 to
2.4e-13 with filib++ (1.4e-13 for log, 2.4e-13 for the real power), 6.4e-15 to
1.2e-13 with PROFIL/BIAS, and at most 4.6e-15 with Solaris Studio; with
Boost.Interval, by 1.4e-14 and 2.5e-14 on cos and sin and 3.7e-14 on the real
power, its exp and log being as tight as libieeep1788's on average: the sums
cannot show that its upper bound of exp is one double too low in 2 to 3 % of
the results of the test of [enclosure](enclosure.md). PROFIL/BIAS's integer
power is far wider, 1.1e-05 on average: it computes `Power(x, n)` as
exp(n log x), where the others multiply. Its arithmetic results have midpoints
that differ by 8e-15 to 4e-14 relatively while their widths are the tightest:
the intervals are the same, but its `Mid` computes inf + (sup − inf)/2
**rounded upward**, one double above the midpoint for 4 % of the sums of the
benchmark, where the others round to nearest.

The timings were measured on a laptop, on 17 October 2026, each time being the
best of the runs of three rounds, each program being run in turn with the
others, on a processor of its own (`taskset -c 2`). libieeep1788's times, from
a single run of each operation in each round, are the least steady. Solaris
Studio's and PROFIL/BIAS's operations are calls into libraries, where the
compiler inlines GAOL's, filib++'s and Boost.Interval's.

## The timings

<!-- BEGIN GENERATED TABLES (doc/compare/code/bench.py) -->

Measured on:

```
Date:            2026-10-17
Processor:       11th Gen Intel(R) Core(TM) i7-1185G7 @ 3.00GHz (taskset -c 2)
System:          Linux 7.0.0-38-generic, Ubuntu 26.04.1 LTS
C++ compiler:    Ubuntu clang version 21.1.8 (6ubuntu1)
C++ flags:       -std=c++11 -O3 -DNDEBUG -mfma (GAOL: -frounding-math -fno-fast-math -ffp-contract=off -msse2 -msse3 -mfma; libieeep1788, filib++, PROFIL/BIAS and Boost.Interval: -frounding-math -fno-fast-math -ffp-contract=off)
Fortran:         f90: Sun Fortran 95 8.7 Linux_i386 2014/10/20, flags: -O3 -xia
GAOL V5.0.0:     the branch of this checkout (v4.3.2-897-g7716fec), CMake Release, CORE-MATH of 3rd/math-core compiled into the library, the rounding direction not preserved
GAOL 4.2.3:      the last version of Frédéric Goualard (cd0ee1a of https://github.com/goualard-f/GAOL), its configure, the rounding direction not preserved, mathlib 2.1.1
libieeep1788:    1f10b89, MPFR 4.2.1, GMP 6.3.0
filib++:         3.0.2.2, interval<double, native_switched, i_mode_extended_flag>
PROFIL/BIAS:     2.0.8, x86-64-Linux-compat-gcc configuration, built by clang-21 and clang++-21
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
| `add` | 0.98 | 135 | 2.31 | 2.39 | 5.11 | 17.6 | 15.3 | 12.7 | 58.4 | 1.0 | 2.2 | 7.6 | 6.6 | 5.5 |
| `sub` | 0.91 | 143 | 2.38 | 2.32 | 4.87 | 17.8 | 15.1 | 12.5 | 59.9 | 1.0 | 2.0 | 7.5 | 6.3 | 5.3 |
| `mul` | 0.90 | 175 | 10.3 | 10.3 | 12.3 | 21.4 | 15.4 | 22.0 | 17.0 | 1.0 | 1.2 | 2.1 | 1.5 | 2.1 |
| `div` | 0.89 | 171 | 7.88 | 7.67 | 8.76 | 19.9 | 15.4 | 19.2 | 21.7 | 1.0 | 1.1 | 2.5 | 2.0 | 2.4 |
| `sqr` | 0.61 | 88.9 | 6.20 | 5.88 | 8.22 | 55.5 | 19.6 | 16.6 | 14.3 | 0.9 | 1.3 | 9.0 | 3.2 | 2.7 |
| `sqrt` | 1.35 | 98.5 | 6.85 | 5.95 | 13.4 | 26.6 | 3.95 | 22.8 | 14.4 | 0.9 | 2.0 | 3.9 | 0.6 | 3.3 |
| `exp` | 3.46 | 1 489 | 18.6 | 34.7 | 30.4 | 38.9 | 10.6 | 29.8 | 80.2 | 1.9 | 1.6 | 2.1 | 0.6 | 1.6 |
| `log` | 3.65 | 1 930 | 19.7 | 46.8 | 26.7 | 37.9 | 11.5 | 29.8 | 97.9 | 2.4 | 1.4 | 1.9 | 0.6 | 1.5 |
| `sin` | 13.9 | 5 549 | 42.9 | 67.7 | 33.1 | 41.4 | 123 | 69.2 | 129 | 1.6 | 0.8 | 1.0 | 2.9 | 1.6 |
| `cos` | 12.4 | 4 942 | 54.4 | 63.1 | 34.1 | 43.1 | 138 | 64.7 | 90.8 | 1.2 | 0.6 | 0.8 | 2.5 | 1.2 |
| `pow_int` | 13.5 | 243 | 5.34 | 9.06 | 18.0 | 145 | 34.9 | 19.8 | 45.5 | 1.7 | 3.4 | 27.2 | 6.5 | 3.7 |
| `pow_real` | 10.5 | 6 547 | 47.4 | 90.2 | 66.0 | 125 | 34.4 | 73.3 | 138 | 1.9 | 1.4 | 2.6 | 0.7 | 1.5 |
| `line_arith` | 1.15 | 656 | 19.6 | 18.9 | 29.7 | 61.5 | 64.4 | 61.7 | 33.4 | 1.0 | 1.5 | 3.1 | 3.3 | 3.1 |
| `line_trig` | 25.8 | 11 091 | 119 | 147 | 98.5 | 166 | 310 | 187 | 93.1 | 1.2 | 0.8 | 1.4 | 2.6 | 1.6 |
| `line_pow` | 19.4 | 2 589 | 53.7 | 61.7 | 88.2 | 254 | 92.6 | 116 | 48.2 | 1.1 | 1.6 | 4.7 | 1.7 | 2.2 |
| `shekel5` | 2.38 | 9 075 | 210 | 170 | 416 | 1 720 | 1 085 | 921 | 43.1 | 0.8 | 2.0 | 8.2 | 5.2 | 4.4 |
| `block5` | 44.2 | 13 981 | 190 | 224 | 215 | 464 | 433 | 343 | 73.7 | 1.2 | 1.1 | 2.4 | 2.3 | 1.8 |

#### Total time of the 1 000 000 operations (seconds)

| Operation | double (reference) | libieeep1788 | GAOL V5.0.0 | GAOL 4.2.3 | filib++ | Solaris Studio f90 | PROFIL/BIAS | Boost.Interval |
|---|---|---|---|---|---|---|---|---|
| `add` | 0.001 | 0.135 | 0.002 | 0.002 | 0.005 | 0.018 | 0.015 | 0.013 |
| `sub` | 0.001 | 0.143 | 0.002 | 0.002 | 0.005 | 0.018 | 0.015 | 0.013 |
| `mul` | 0.001 | 0.175 | 0.010 | 0.010 | 0.012 | 0.021 | 0.015 | 0.022 |
| `div` | 0.001 | 0.171 | 0.008 | 0.008 | 0.009 | 0.020 | 0.015 | 0.019 |
| `sqr` | 0.001 | 0.089 | 0.006 | 0.006 | 0.008 | 0.056 | 0.020 | 0.017 |
| `sqrt` | 0.001 | 0.099 | 0.007 | 0.006 | 0.013 | 0.027 | 0.004 | 0.023 |
| `exp` | 0.003 | 1.489 | 0.019 | 0.035 | 0.030 | 0.039 | 0.011 | 0.030 |
| `log` | 0.004 | 1.930 | 0.020 | 0.047 | 0.027 | 0.038 | 0.011 | 0.030 |
| `sin` | 0.014 | 5.549 | 0.043 | 0.068 | 0.033 | 0.041 | 0.123 | 0.069 |
| `cos` | 0.012 | 4.942 | 0.054 | 0.063 | 0.034 | 0.043 | 0.138 | 0.065 |
| `pow_int` | 0.013 | 0.243 | 0.005 | 0.009 | 0.018 | 0.145 | 0.035 | 0.020 |
| `pow_real` | 0.010 | 6.547 | 0.047 | 0.090 | 0.066 | 0.125 | 0.034 | 0.073 |
| `line_arith` | 0.001 | 0.656 | 0.020 | 0.019 | 0.030 | 0.061 | 0.064 | 0.062 |
| `line_trig` | 0.026 | 11.091 | 0.119 | 0.147 | 0.099 | 0.166 | 0.310 | 0.187 |
| `line_pow` | 0.019 | 2.589 | 0.054 | 0.062 | 0.088 | 0.254 | 0.093 | 0.116 |
| `shekel5` | 0.002 | 9.075 | 0.210 | 0.170 | 0.416 | 1.720 | 1.085 | 0.921 |
| `block5` | 0.044 | 13.981 | 0.190 | 0.224 | 0.215 | 0.464 | 0.433 | 0.343 |

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
