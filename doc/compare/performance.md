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
| GAOL V5.0.0 (this branch) | Clang 21.1, `-O3 -mfma`, the flags of interval arithmetic; GAOL built by CMake in Release with `-mfma` (`GAOL_FMA`), the sources of CORE-MATH compiled into it, the rounding direction not preserved (`GAOL_PRESERVE_ROUNDING` off, its default) | Called under the names of IEEE 1788-2015, the namespace `gaol_ieee1788` (`pown(x, n)`, `pow(x, y)`), as the other libraries of the standard (GAOL v5); SSE2 operations that are calls into the library, the bounds passed and returned in registers, each kernel checking that the rounding direction is upward and setting it when it is not; **every** elementary function CORE-MATH's, correctly rounded in that direction, so the bounds are the tightest ones and the direction is never switched |
| GAOL V5.0.0, rounding direction preserved (this branch) | The same, but GAOL built with `GAOL_PRESERVE_ROUNDING=ON`; `GAOL_PREFER_AVX512` left at its default, which is on with that option: **AVX-512 stays enabled** (the library takes the AVX-512 instructions where the compiler has them and the machine building runs them, as this one does) | The same program, in a column of the table of the times only; each operation restores the rounding direction it found instead of leaving it upward; `+`, `−`, `×`, `/`, `√`, `sqr` and the integer powers take the AVX-512 instructions, the rounding direction being in the instruction, so that they set no direction; the other operations read the direction, set it upward and restore it |
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
the rounding direction, the default of GAOL V5.0.0; GAOL V5.0.0 built with it,
AVX-512 left enabled, has a column of its own in the first table. GAOL V5.0.0
is called under the names of IEEE 1788-2015, `gaol_ieee1788`, as the other libraries
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
with the same Clang 21.1: the other libraries took between 7 % less and 18 %
more time then than in this run, GAOL V5.0.0 between 7 % more and 4.9 times as
much, the way its operations compute and hand over the bounds having changed
since: see below).

**What GAOL V5.0.0 changes.** The elementary functions, which mathlib used to
bound, are faster, and so is the integer power:

| | GAOL V5.0.0 | GAOL 4.2.3 | |
|---|---:|---:|---|
| `log` | 20.9 ns | 46.8 ns | 2.2 times faster |
| `exp` | 18.3 | 35.1 | 1.9 |
| `pow(x, y)` | 48.2 | 89.9 | 1.9 |
| `pow(x, 3)` | 5.12 | 9.07 | 1.8 |
| line of powers | 38.1 | 61.0 | 1.6 |
| `sin` | 43.2 | 68.0 | 1.6 |
| five-line block | 155 | 229 | 1.5 |
| line of sin and cos | 102 | 148 | 1.5 |
| `cos` | 54.0 | 62.8 | 1.2 |

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

**The arithmetic is faster than GAOL 4.2.3's**, which is what the operators
of GAOL V5.0.0 are written for (point ZZ of `TODO.md`):

| | GAOL V5.0.0 | GAOL 4.2.3 | |
|---|---:|---:|---|
| `mul` | 2.30 ns | 10.0 ns | 4.4 times faster |
| `line_arith` | 5.61 | 18.7 | 3.3 |
| `sqr` | 1.87 | 5.81 | 3.1 |
| `div` | 2.52 | 7.50 | 3.0 |
| `shekel5` | 108 | 189 | 1.7 |

The sums and the differences, 2.2 ns against 2.4, and the square root, 6.3
against 6.3, take what they took: a sum is one instruction, and what it waits
for is memory, the benchmark reading two arrays of 16 MB and writing a third,
and the square root is the tightest interval whatever the rounding of the C
library's `sqrt`, where GAOL 4.2.3's is wider (excess 1.4e-14). The products,
the quotients and the squares were nine cases of the signs of the bounds, told
apart by a switch in GAOL 4.2.3 and in GAOL V5.0.0 before, which the signs of
the benchmark, centred at random on both sides of 0, took at random: a
mispredicted branch costs more than the operation it chooses. They are now
computed with no branch on the signs: the larger of four products for a
product, one division of the two lanes by the denominators the signs choose,
by masks, for a quotient, two squares and a mask for a square. And an operation takes the two doubles of
each operand in a register, and gives the two doubles of its result in one, to
the function of the library that computes them, where GAOL 4.2.3's operators,
and the former ones of GAOL V5.0.0, read their operand and wrote their result
through memory, the result going back to memory for the operation that
followed. Each operation of GAOL V5.0.0 still checks that the rounding
direction is upward, with one comparison, and sets it when it is not, where
GAOL 4.2.3 takes it upward on trust. The check is what a program needs that
changes the rounding direction: the benchmark sums the results rounding to
nearest, and GAOL 4.2.3's own `midpoint()` leaves the direction to nearest.
Left so, the operations of GAOL 4.2.3 timed next gave bounds narrower than
libieeep1788's, which did not enclose the exact results; the benchmark now sets
the direction back after its sums (`code/bench_common.h`). The bounds are the
tightest, as before: the zero bound of a product or of a quotient may have the
other sign ([accuracy](../accuracy.md#the-same-bounds-on-every-machine)).

**Preserving the rounding direction** (the column "GAOL V5.0.0, rounding
direction preserved" of the first table: `GAOL_PRESERVE_ROUNDING=ON`, with
AVX-512 left enabled, as that option has it by default) leaves the sums and the
differences as they are, 2.2 ns, and costs 1.5 times on the products and the
quotients (3.4 and 3.6 ns), 1.1 times on `sqr` (2.0 ns against 1.9) and 1.2
times on `pow(x, 3)` (6.2 ns against 5.1): these operations take the AVX-512
instructions, whose rounding direction is in the instruction, and set none, so
that nothing is saved and restored. The square root, 3.75 ns, is faster than
without the preservation (6.3 ns): the instruction rounds each root in its
direction, where the SSE2 path checks the root of the C library with a product.
The elementary functions read the control register of the direction, set it
upward and write it back, once each, and take 1.0 to 1.3 times longer (`exp`
23.2 ns against 18.3, `log` 20.4 against 20.9, `sin`, `cos` and `pow(x, y)` 1.1
times). They paid more, as `sqr` and the integer powers did, until point ZZ of
`TODO.md`: the two writes of the control register, each one waiting for the
operations in flight, took 6 ns a `sqr` (8.3 ns, 4.4 times); with the C
library's `fegetround()` and `fesetround()` and the writes of the control word
of the x87 unit as well, `pow(x, 3)` took 14 ns more (19.0 ns, 3.7 times; 8.4 ns
with the writes of the SSE register alone, where the machine has no AVX-512) and
the elementary functions 1.2 to 1.7 times longer. Shekel 5 takes 147 ns against
108, and every operation of the table is faster than with GAOL 4.2.3, which does
not preserve the direction either (Shekel 5: 189 ns). The results are the same:
the sums of the midpoints and of the widths of the million results are those of
GAOL V5.0.0, to 2e-16 relatively.

**Arithmetic.** GAOL is the fastest on the four operations: + and − take GAOL
V5.0.0 2.2 ns (2.4 for GAOL 4.2.3), 2.3 times as fast as filib++ (5.1 ns) and
6.4 to 8.0 times as fast as PROFIL/BIAS (14 ns) and Solaris Studio (17.6 ns);
× and ÷ take GAOL V5.0.0 2.3 and 2.5 ns (10.0 and 7.5 for GAOL 4.2.3), against
12.1 and 8.7 ns for filib++, 14.7 and 14.4 ns for PROFIL/BIAS, 21 and 20 ns
for Solaris Studio and 21 and 19 ns for Boost.Interval. An addition of
intervals costs GAOL V5.0.0 2.3 times an addition of doubles: two additions,
and the check that the rounding direction is upward, where filib++ sets the
rounding direction and restores it, and PROFIL/BIAS calls a function of BIAS
that sets it downward, then upward, then back to nearest — which is why its
four arithmetic operations all take about the same time. The integer power
`pown(x, 3)` takes GAOL V5.0.0 5.1 ns and GAOL 4.2.3 9.1 ns: filib++'s
`power(x, 3)` takes 18 ns, Boost.Interval's 20 ns, PROFIL/BIAS's 35 ns and
Solaris Studio's `x**3` 145 ns; `sqr` takes GAOL V5.0.0 1.9 ns, against 8.2
for filib++, 16 for Boost.Interval, 18 for PROFIL/BIAS and 56 for Solaris
Studio.

**Elementary functions.** PROFIL/BIAS is the fastest on the square root
(3.7 ns), exp (9.9 ns), log (10.2 ns) and the real power `pow(x, y)` (34 ns),
which it computes from the libm of the system, moved outward, without correct
rounding: its bounds are among the widest (below), and its sin and cos,
computed by its own argument reduction, are the slowest after libieeep1788's,
115 and 131 ns. GAOL V5.0.0 comes next on exp and log, 18 and 21 ns — faster
than Boost.Interval (30 ns each), filib++ (30 and 27 ns) and Solaris Studio
(38 and 39 ns) — with **correctly rounded** values, where the others are not.
filib++ is the fastest on sin and cos (33.4 and 33.9 ns) with its own
polynomials, then Solaris Studio (41 and 43 ns), then GAOL V5.0.0 (43 and
54 ns): GAOL pays there for correct rounding over the whole range, and for
dividing the bounds by an interval enclosing π to find where the function is
monotonic, asking CORE-MATH for the signs of the derivative where the division
cannot tell ([issue #6](https://github.com/Jordan08/GAOL/issues/6)). GAOL
V5.0.0's sin became faster than its cos when CORE-MATH rewrote `sin.c`
(upstream 6b84457). On `pow(x, y)` GAOL V5.0.0 takes 48 ns, against 65 for
filib++, 73 for Boost.Interval, 90 for GAOL 4.2.3 and 125 for Solaris Studio.

**Formulas.** GAOL is the fastest on the arithmetic line (5.6 ns for GAOL
V5.0.0 and 18.7 for GAOL 4.2.3, against 27 for filib++ and 62 to 63 for
Solaris Studio, PROFIL/BIAS and Boost.Interval), on the line of powers (38 and
61 ns, against 82 for filib++, 93 for PROFIL/BIAS and 253 for Solaris Studio)
and on Shekel 5 (108 and 189 ns, against 397 for filib++, 920 for
Boost.Interval, 1 093 for PROFIL/BIAS and 1 720 for Solaris Studio: Shekel 5 is
20 squares, 45 additions and subtractions and 5 divisions, and its squares cost
Solaris Studio 56 ns each and PROFIL/BIAS 18). GAOL V5.0.0 alone is the
fastest on the five-line block (155 ns, against 197 for filib++, 229 for GAOL
4.2.3, 427 for PROFIL/BIAS and 464 for Solaris Studio). filib++ is the fastest
on the line of sin and cos (90.8 ns, against 102 for GAOL V5.0.0, 148 for GAOL
4.2.3, 166 for Solaris Studio and 311 for PROFIL/BIAS), where its sin and cos
weigh most.

**libieeep1788** is 17 to 135 times slower than GAOL V5.0.0: every bound is an
MPFR computation, about 140 ns for an addition and 5 to 7 µs for sin, cos and
the real power. Its own README warns that its focus is correctness, not speed.
GAOL V5.0.0 now gives **the same bounds as it does**, tightest everywhere, 17
to 135 times faster.

**Boost.Interval** is slower than GAOL V5.0.0 on every operation: 5.6 times
on + and −, 3.5 to 9.3 times on ×, ÷, the square, the square root and the
integer power, 1.2 to 1.6 times on exp, log, sin, cos and the real power, 11
times on the arithmetic line and 8.5 times on Shekel 5. Each of its
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
Studio's and PROFIL/BIAS's operations are calls into libraries, as GAOL
V5.0.0's `+`, `-`, `*`, `/` and `sqr` are (the bounds in registers), where the
compiler inlines GAOL 4.2.3's, filib++'s and Boost.Interval's.

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
GAOL V5.0.0:     the branch of this checkout (v4.3.2-902-g7c1ebb9), CMake Release, CORE-MATH of 3rd/math-core compiled into the library, the rounding direction not preserved
GAOL V5.0.0 preserved: the same, built with GAOL_PRESERVE_ROUNDING=ON, the rounding direction restored after each operation, AVX-512 path of + - * / sqrt sqr and pown on, the AVX-512 instructions with the rounding direction in them (GAOL_PREFER_AVX512 left at its default) (measured again on 2026-10-18)
GAOL 4.2.3:      the last version of Frédéric Goualard (cd0ee1a of https://github.com/goualard-f/GAOL), its configure, the rounding direction not preserved, mathlib 2.1.1
libieeep1788:    1f10b89, MPFR 4.2.1, GMP 6.3.0
filib++:         3.0.2.2, interval<double, native_switched, i_mode_extended_flag>
PROFIL/BIAS:     2.0.8, x86-64-Linux-compat-gcc configuration, built by clang-21 and clang++-21
Boost.Interval:  Boost 1.92.0, interval<double> with save_state<rounded_transc_opp<double> > and checking_strict<double>, the elementary functions of the C library, glibc 2.43
```

1 000 000 operations of each kind, on the same intervals. Each time is the best of 3 rounds, each program being run in turn with the others: 15 runs for double (reference), 3 runs for libieeep1788, 15 runs for GAOL V5.0.0, 15 runs for GAOL V5.0.0, rounding direction preserved, 15 runs for GAOL 4.2.3, 15 runs for filib++, 15 runs for Solaris Studio f90, 15 runs for PROFIL/BIAS, 15 runs for Boost.Interval in all.

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

| Operation | double (reference) | libieeep1788 | GAOL V5.0.0 | GAOL V5.0.0, rounding direction preserved | GAOL 4.2.3 | filib++ | Solaris Studio f90 | PROFIL/BIAS | Boost.Interval | libieeep1788 / GAOL V5.0.0 | GAOL V5.0.0, rounding direction preserved / GAOL V5.0.0 | GAOL 4.2.3 / GAOL V5.0.0 | filib++ / GAOL V5.0.0 | Solaris Studio f90 / GAOL V5.0.0 | PROFIL/BIAS / GAOL V5.0.0 | Boost.Interval / GAOL V5.0.0 |
|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|
| `add` | 0.96 | 137 | 2.23 | 2.19 | 2.38 | 5.10 | 17.6 | 14.2 | 12.4 | 61.4 | 1.0 | 1.1 | 2.3 | 7.9 | 6.4 | 5.6 |
| `sub` | 0.92 | 138 | 2.21 | 2.16 | 2.40 | 5.12 | 17.6 | 14.3 | 12.3 | 62.5 | 1.0 | 1.1 | 2.3 | 8.0 | 6.5 | 5.6 |
| `mul` | 0.90 | 171 | 2.30 | 3.42 | 10.0 | 12.1 | 21.3 | 14.7 | 21.5 | 74.6 | 1.5 | 4.4 | 5.3 | 9.3 | 6.4 | 9.3 |
| `div` | 0.91 | 171 | 2.52 | 3.60 | 7.50 | 8.70 | 20.3 | 14.4 | 18.9 | 68.1 | 1.4 | 3.0 | 3.5 | 8.1 | 5.7 | 7.5 |
| `sqr` | 0.59 | 88.4 | 1.87 | 2.04 | 5.81 | 8.25 | 55.5 | 18.5 | 16.2 | 47.3 | 1.1 | 3.1 | 4.4 | 29.7 | 9.9 | 8.7 |
| `sqrt` | 1.30 | 107 | 6.30 | 3.75 | 6.26 | 13.4 | 26.6 | 3.73 | 22.3 | 17.0 | 0.6 | 1.0 | 2.1 | 4.2 | 0.6 | 3.5 |
| `exp` | 3.44 | 1 491 | 18.3 | 23.2 | 35.1 | 30.1 | 38.0 | 9.87 | 29.8 | 81.4 | 1.3 | 1.9 | 1.6 | 2.1 | 0.5 | 1.6 |
| `log` | 3.32 | 1 919 | 20.9 | 20.4 | 46.8 | 26.6 | 38.5 | 10.2 | 29.9 | 92.0 | 1.0 | 2.2 | 1.3 | 1.8 | 0.5 | 1.4 |
| `sin` | 13.4 | 5 472 | 43.2 | 46.8 | 68.0 | 33.4 | 41.5 | 115 | 71.2 | 127 | 1.1 | 1.6 | 0.8 | 1.0 | 2.7 | 1.6 |
| `cos` | 12.0 | 4 890 | 54.0 | 57.6 | 62.8 | 33.9 | 42.7 | 131 | 65.7 | 90.6 | 1.1 | 1.2 | 0.6 | 0.8 | 2.4 | 1.2 |
| `pow_int` | 13.0 | 240 | 5.12 | 6.18 | 9.07 | 17.8 | 145 | 35.1 | 19.9 | 46.8 | 1.2 | 1.8 | 3.5 | 28.4 | 6.9 | 3.9 |
| `pow_real` | 10.3 | 6 517 | 48.2 | 51.4 | 89.9 | 64.5 | 125 | 33.6 | 73.2 | 135 | 1.1 | 1.9 | 1.3 | 2.6 | 0.7 | 1.5 |
| `line_arith` | 1.14 | 655 | 5.61 | 8.24 | 18.7 | 27.4 | 61.6 | 63.1 | 61.7 | 117 | 1.5 | 3.3 | 4.9 | 11.0 | 11.2 | 11.0 |
| `line_trig` | 25.8 | 10 920 | 102 | 113 | 148 | 90.8 | 166 | 311 | 187 | 107 | 1.1 | 1.5 | 0.9 | 1.6 | 3.1 | 1.8 |
| `line_pow` | 19.4 | 2 401 | 38.1 | 43.7 | 61.0 | 81.9 | 253 | 92.6 | 115 | 63.1 | 1.1 | 1.6 | 2.2 | 6.7 | 2.4 | 3.0 |
| `shekel5` | 2.43 | 8 993 | 108 | 147 | 189 | 397 | 1 720 | 1 093 | 920 | 83.4 | 1.4 | 1.7 | 3.7 | 15.9 | 10.1 | 8.5 |
| `block5` | 44.5 | 13 823 | 155 | 171 | 229 | 197 | 464 | 427 | 343 | 89.4 | 1.1 | 1.5 | 1.3 | 3.0 | 2.8 | 2.2 |

GAOL V5.0.0, rounding direction preserved, is the same GAOL built with `GAOL_PRESERVE_ROUNDING=ON`: each operation restores the rounding direction it found, where GAOL V5.0.0 leaves it upward. AVX-512 stays enabled in it: `GAOL_PREFER_AVX512`, which that option turns on by default, is left on, so that `+`, `−`, `×`, `/`, `√`, `sqr` and the integer powers take the AVX-512 instructions, the rounding direction being in the instruction (the machine description above says whether the library took them); the other operations read the direction, set it upward and restore it. Its column is in this table only: its results are those of GAOL V5.0.0.

#### Total time of the 1 000 000 operations (seconds)

| Operation | double (reference) | libieeep1788 | GAOL V5.0.0 | GAOL 4.2.3 | filib++ | Solaris Studio f90 | PROFIL/BIAS | Boost.Interval |
|---|---|---|---|---|---|---|---|---|
| `add` | 0.001 | 0.137 | 0.002 | 0.002 | 0.005 | 0.018 | 0.014 | 0.012 |
| `sub` | 0.001 | 0.138 | 0.002 | 0.002 | 0.005 | 0.018 | 0.014 | 0.012 |
| `mul` | 0.001 | 0.171 | 0.002 | 0.010 | 0.012 | 0.021 | 0.015 | 0.021 |
| `div` | 0.001 | 0.171 | 0.003 | 0.007 | 0.009 | 0.020 | 0.014 | 0.019 |
| `sqr` | 0.001 | 0.088 | 0.002 | 0.006 | 0.008 | 0.056 | 0.018 | 0.016 |
| `sqrt` | 0.001 | 0.107 | 0.006 | 0.006 | 0.013 | 0.027 | 0.004 | 0.022 |
| `exp` | 0.003 | 1.491 | 0.018 | 0.035 | 0.030 | 0.038 | 0.010 | 0.030 |
| `log` | 0.003 | 1.919 | 0.021 | 0.047 | 0.027 | 0.039 | 0.010 | 0.030 |
| `sin` | 0.013 | 5.472 | 0.043 | 0.068 | 0.033 | 0.041 | 0.115 | 0.071 |
| `cos` | 0.012 | 4.890 | 0.054 | 0.063 | 0.034 | 0.043 | 0.131 | 0.066 |
| `pow_int` | 0.013 | 0.240 | 0.005 | 0.009 | 0.018 | 0.145 | 0.035 | 0.020 |
| `pow_real` | 0.010 | 6.517 | 0.048 | 0.090 | 0.064 | 0.125 | 0.034 | 0.073 |
| `line_arith` | 0.001 | 0.655 | 0.006 | 0.019 | 0.027 | 0.062 | 0.063 | 0.062 |
| `line_trig` | 0.026 | 10.920 | 0.102 | 0.148 | 0.091 | 0.166 | 0.311 | 0.187 |
| `line_pow` | 0.019 | 2.401 | 0.038 | 0.061 | 0.082 | 0.253 | 0.093 | 0.115 |
| `shekel5` | 0.002 | 8.993 | 0.108 | 0.189 | 0.397 | 1.720 | 1.093 | 0.920 |
| `block5` | 0.044 | 13.823 | 0.155 | 0.229 | 0.197 | 0.464 | 0.427 | 0.343 |

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

## Long lines of arithmetic operators

The operations above are timed one at a time. A program that computes with
intervals writes lines of them, an operator after another, where what an
operation hands to the next matters as much as what it computes: the lines
below are expressions of + − × ÷ alone, of 7 to 19 operations between 8
intervals of the same million, evaluated for each of them. They are run by
[`code/run_lines.sh`](code/README.md) with every library but Solaris Studio,
which has no program for them. The seven first take intervals of both signs, at
random; the last three, `pos_sum16`, `pos_mul8` and `pos_quot8`, only p and e,
which are positive, so that the branches of an implementation that tells the
signs apart, as GAOL 4.2.3's does, are taken the same way from one evaluation
to the next. The chains give the next operation the result of the previous
one, the trees (`mul8_tree`, `mix_tree`) several independent ones.

<!-- BEGIN GENERATED LINES (doc/compare/code/bench.py) -->

Measured on:

```
Date:            2026-10-17
Processor:       11th Gen Intel(R) Core(TM) i7-1185G7 @ 3.00GHz (taskset -c 2)
System:          Linux 7.0.0-38-generic, Ubuntu 26.04.1 LTS
C++ compiler:    Ubuntu clang version 21.1.8 (6ubuntu1)
C++ flags:       -std=c++11 -O3 -DNDEBUG -mfma (GAOL: -frounding-math -fno-fast-math -ffp-contract=off -msse2 -msse3 -mfma; libieeep1788, filib++, PROFIL/BIAS and Boost.Interval: -frounding-math -fno-fast-math -ffp-contract=off)
GAOL V5.0.0:     the branch of this checkout (v4.3.2-902-g7c1ebb9), CMake Release, CORE-MATH of 3rd/math-core compiled into the library, the rounding direction not preserved
GAOL 4.2.3:      the last version of Frédéric Goualard (cd0ee1a of https://github.com/goualard-f/GAOL), its configure, the rounding direction not preserved, mathlib 2.1.1
libieeep1788:    1f10b89, MPFR 4.2.1, GMP 6.3.0
filib++:         3.0.2.2, interval<double, native_switched, i_mode_extended_flag>
PROFIL/BIAS:     2.0.8, x86-64-Linux-compat-gcc configuration, built by clang-21 and clang++-21
Boost.Interval:  Boost 1.92.0, interval<double> with save_state<rounded_transc_opp<double> > and checking_strict<double>, the elementary functions of the C library, glibc 2.43
```

1 000 000 evaluations of each line, on the same intervals. Each time is the best of 3 rounds, each program being run in turn with the others: 15 runs for double (reference), 3 runs for libieeep1788, 15 runs for GAOL V5.0.0, 15 runs for GAOL 4.2.3, 15 runs for filib++, 15 runs for PROFIL/BIAS, 15 runs for Boost.Interval in all.

#### The lines

a and b are intervals centred in [−10, 10], p in [1, 10], e in [0.5, 2.5], s1 to s4 in [0, 10]; their widths are 10^u, u uniform in [−6, 0] ([−6, −1] for e), the intervals of the table above.

| Line | Computes | Operations | What it shows |
|---|---|---:|---|
| `sum16` | a + b + p + e + s1 + s2 + s3 + s4 + a + b + p + e + s1 + s2 + s3 + s4 | 15 | additions, one after the other |
| `diff16` | a − b + p − e + s1 − s2 + s3 − s4 − a + b − p + e − s1 + s2 − s3 + s4 | 15 | additions and subtractions, one after the other |
| `mul8` | a · b · p · e · s1 · s2 · s3 · s4 | 7 | products, one after the other, of operands of both signs |
| `mul8_tree` | ((a · b)(p · e))((s1 · s2)(s3 · s4)) | 7 | the same products in a tree |
| `quot8` | a / p / e / p / e / p / e / p | 7 | quotients by intervals with no zero, one after the other |
| `mix13` | ((((((a + b) · p − e) · s1 + s2) · p − s3) / e + a) · b − s4) / p + e | 13 | the four operations, one after the other |
| `mix_tree` | ((a + b)(p − e) + (s1 − s2)(s3 + s4)) / (p + e) + ((a − b)(s1 + s2) − (p · e) / (p + e)) / (e + p) | 19 | the four operations, in a tree |
| `pos_sum16` | p + e + p + e + p + e + p + e + p + e + p + e + p + e + p + e | 15 | additions of intervals of one sign, which every line repeats |
| `pos_mul8` | p · e · p · e · p · e · p · e | 7 | products of intervals of one sign |
| `pos_quot8` | p / e / p / e / p / e / p / e | 7 | quotients of intervals of one sign |

#### Time per evaluation of a line (nanoseconds)

| Line | double (reference) | libieeep1788 | GAOL V5.0.0 | GAOL 4.2.3 | filib++ | PROFIL/BIAS | Boost.Interval | libieeep1788 / GAOL V5.0.0 | GAOL 4.2.3 / GAOL V5.0.0 | filib++ / GAOL V5.0.0 | PROFIL/BIAS / GAOL V5.0.0 | Boost.Interval / GAOL V5.0.0 |
|---|---|---|---|---|---|---|---|---|---|---|---|---|
| `sum16` | 2.45 | 2 001 | 22.8 | 27.4 | 76.5 | 216 | 179 | 87.9 | 1.2 | 3.4 | 9.5 | 7.9 |
| `diff16` | 2.44 | 2 091 | 23.3 | 27.1 | 76.5 | 215 | 177 | 89.6 | 1.2 | 3.3 | 9.2 | 7.6 |
| `mul8` | 2.42 | 1 166 | 17.9 | 30.9 | 48.6 | 104 | 111 | 65.3 | 1.7 | 2.7 | 5.8 | 6.2 |
| `mul8_tree` | 2.37 | 1 168 | 15.0 | 29.4 | 50.5 | 105 | 111 | 77.9 | 2.0 | 3.4 | 7.0 | 7.4 |
| `quot8` | 3.36 | 1 178 | 20.6 | 34.0 | 48.6 | 102 | 112 | 57.1 | 1.7 | 2.4 | 5.0 | 5.4 |
| `mix13` | 2.38 | 2 068 | 23.0 | 47.6 | 80.7 | 192 | 185 | 89.9 | 2.1 | 3.5 | 8.4 | 8.0 |
| `mix_tree` | 2.38 | 2 985 | 34.7 | 64.7 | 116 | 278 | 264 | 86.0 | 1.9 | 3.3 | 8.0 | 7.6 |
| `pos_sum16` | 1.03 | 1 930 | 23.1 | 24.5 | 73.5 | 214 | 179 | 83.5 | 1.1 | 3.2 | 9.2 | 7.8 |
| `pos_mul8` | 0.89 | 1 139 | 17.8 | 18.9 | 40.2 | 100 | 105 | 64.0 | 1.1 | 2.3 | 5.7 | 5.9 |
| `pos_quot8` | 3.27 | 1 160 | 20.2 | 37.6 | 44.3 | 101 | 109 | 57.3 | 1.9 | 2.2 | 5.0 | 5.4 |

#### Time per operation of a line (nanoseconds)

The time of the line divided by its number of operations.

| Line | double (reference) | libieeep1788 | GAOL V5.0.0 | GAOL 4.2.3 | filib++ | PROFIL/BIAS | Boost.Interval |
|---|---|---|---|---|---|---|---|
| `sum16` | 0.16 | 133 | 1.52 | 1.83 | 5.10 | 14.4 | 11.9 |
| `diff16` | 0.16 | 139 | 1.55 | 1.81 | 5.10 | 14.3 | 11.8 |
| `mul8` | 0.35 | 167 | 2.55 | 4.42 | 6.94 | 14.8 | 15.8 |
| `mul8_tree` | 0.34 | 167 | 2.14 | 4.19 | 7.22 | 15.0 | 15.9 |
| `quot8` | 0.48 | 168 | 2.95 | 4.86 | 6.94 | 14.6 | 16.0 |
| `mix13` | 0.18 | 159 | 1.77 | 3.66 | 6.21 | 14.8 | 14.2 |
| `mix_tree` | 0.13 | 157 | 1.83 | 3.41 | 6.08 | 14.6 | 13.9 |
| `pos_sum16` | 0.07 | 129 | 1.54 | 1.63 | 4.90 | 14.2 | 11.9 |
| `pos_mul8` | 0.13 | 163 | 2.54 | 2.70 | 5.74 | 14.4 | 14.9 |
| `pos_quot8` | 0.47 | 166 | 2.89 | 5.36 | 6.33 | 14.4 | 15.6 |

#### Same results?

The sum of the midpoints of the results, relative to libieeep1788's, and the mean width of the results, with the relative excess of the other libraries over libieeep1788, whose bounds are the tightest:

| Line | Σ midpoints (libieeep1788) | Δ GAOL V5.0.0 | Δ GAOL 4.2.3 | Δ filib++ | Δ PROFIL/BIAS | Δ Boost.Interval | mean width (libieeep1788) | excess GAOL V5.0.0 | excess GAOL 4.2.3 | excess filib++ | excess PROFIL/BIAS | excess Boost.Interval |
|---|---|---|---|---|---|---|---|---|---|---|---|---|
| `sum16` | 54024197.3 | 0.0e+00 | 0.0e+00 | 0.0e+00 | 0.0e+00 | 0.0e+00 | 1.03024 | 1.1e-16 | 1.1e-16 | 0.0e+00 | 0.0e+00 | 0.0e+00 |
| `diff16` | -9.729426466e-13 | 0.0e+00 | 0.0e+00 | 0.0e+00 | 1.4e+00 | 0.0e+00 | 1.03024 | 1.1e-16 | -1.1e-16 | 0.0e+00 | 0.0e+00 | 0.0e+00 |
| `mul8` | 255576459.8 | 0.0e+00 | 0.0e+00 | 0.0e+00 | 1.9e-14 | 0.0e+00 | 13602.3 | 0.0e+00 | 0.0e+00 | 0.0e+00 | 0.0e+00 | 0.0e+00 |
| `mul8_tree` | 255576459.8 | 0.0e+00 | 0.0e+00 | 0.0e+00 | 2.0e-14 | 0.0e+00 | 13602.3 | 0.0e+00 | 0.0e+00 | 0.0e+00 | 0.0e+00 | 0.0e+00 |
| `quot8` | -1016.492029 | 0.0e+00 | 0.0e+00 | 0.0e+00 | 1.2e-14 | 0.0e+00 | 0.0625416 | 0.0e+00 | -2.3e-16 | 0.0e+00 | 0.0e+00 | 0.0e+00 |
| `mix13` | 738436463 | 0.0e+00 | 0.0e+00 | 0.0e+00 | 1.6e-16 | 0.0e+00 | 79.6756 | 0.0e+00 | 0.0e+00 | 0.0e+00 | 0.0e+00 | 0.0e+00 |
| `mix_tree` | -155881.5083 | 0.0e+00 | 0.0e+00 | 0.0e+00 | 3.0e-15 | 0.0e+00 | 1.24303 | 0.0e+00 | 0.0e+00 | 0.0e+00 | 0.0e+00 | 0.0e+00 |
| `pos_sum16` | 56040038.03 | 0.0e+00 | 0.0e+00 | 0.0e+00 | 0.0e+00 | 0.0e+00 | 0.64606 | 0.0e+00 | 0.0e+00 | 0.0e+00 | 0.0e+00 | 0.0e+00 |
| `pos_mul8` | 2.178309922e+10 | 0.0e+00 | 0.0e+00 | 0.0e+00 | 1.8e-16 | 0.0e+00 | 1161.99 | 0.0e+00 | 0.0e+00 | 0.0e+00 | 0.0e+00 | 0.0e+00 |
| `pos_quot8` | 138457.8704 | 0.0e+00 | 0.0e+00 | 0.0e+00 | 0.0e+00 | 0.0e+00 | 0.0326453 | 0.0e+00 | -2.2e-16 | 0.0e+00 | 0.0e+00 | 0.0e+00 |

<!-- END GENERATED LINES -->

**What the lines show.** GAOL V5.0.0 is the fastest of the libraries on every
line, and faster than GAOL 4.2.3 on all ten, 1.06 to 2.1 times as fast: a line
of 15 additions takes 22.8 ns, 1.5 ns an addition, against 27.5 ns (1.8 ns),
and a line of 7 products 17.9 ns (2.6 ns a product) against 30.9 ns (4.4 ns); the
quotients, 20.6 ns for 7 (2.9 ns each) against 34.0 ns (4.9 ns), 1.7 times as
fast, and the lines that mix the four operations, `mix13` and `mix_tree`, 23.0
and 34.7 ns against 47.6 and 64.7 ns, 2.1 and 1.9 times. The three lines of intervals
of one sign are where GAOL 4.2.3 comes nearest, on the additions and the
products, 1.06 times as fast as it, 23.1 ns against 24.5 ns and 17.8 ns against
18.9 ns: with the signs the same from one evaluation to the next, its
branches are predicted, and what it still pays is the bounds that go through
memory between two operations; its quotients, on the other hand, take 1.9 times
as long (37.6 ns against 20.2 ns). An operation of a line costs GAOL V5.0.0
less than the same operation timed alone above, 1.5 ns an addition against
2.2: the two doubles of the result stay in a register for the next operation,
and the benchmark of a single operation, which reads two arrays and writes a
third for each, waits for the memory, where a line reads eight arrays at most
for its 7 to 19 operations.

Against the other libraries, filib++ takes 2.2 to 3.5 times as long as GAOL
V5.0.0 on the ten lines, Boost.Interval 5.4 to 8.0 times, PROFIL/BIAS 5.0 to
9.5 times and libieeep1788 57 to 90 times. The bounds are the tightest:
the mean widths of the results of GAOL V5.0.0 are those of libieeep1788, which
computes every bound with MPFR, to 1.1e-16 relatively on the lines of additions
and exactly on the others, where GAOL 4.2.3's differ from them by 2.3e-16 on the
lines of quotients; PROFIL/BIAS's sums of midpoints differ because its `Mid`
rounds upward (see above), by a lot where the sum of a line, `diff16`, is as
small as 10^-12 for a million values.
