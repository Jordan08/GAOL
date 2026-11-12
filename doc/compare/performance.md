<!-- Copyright (c) 2026 ENSTA, France

     By: Jordan NININ (created 2026-09-20)

     GAOL is a software distributed WITHOUT ANY WARRANTY. Read the associated
     COPYING.LIB file for information. -->
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
| GAOL V5.0.0, rounding direction preserved (this branch) | The same, but GAOL built with `GAOL_PRESERVE_ROUNDING=ON`; `GAOL_PREFER_AVX512`, a setting of the developers, left alone, so that the build takes the AVX-512 path with that option: **AVX-512 stays enabled** (the library takes the AVX-512 instructions where the compiler has them and the machine building runs them, as this one does) | The same program, in a column of the table of the times only; each operation restores the rounding direction it found instead of leaving it upward; `+`, `−`, `×`, `/`, `√`, `sqr` and the integer powers take the AVX-512 instructions, the rounding direction being in the instruction, so that they set no direction; the other operations read the direction, set it upward and restore it |
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
built with **Clang 21.1** — GMP, MPFR, libieeep1788, filib++ (from the archive
IBEX distributes) and PROFIL/BIAS as well as the two GAOL — so that one compiler
answers for every C and C++ library here; on 12 November, only the two GAOL of
this repository were built again, at the commit measured, the other libraries
being the builds of 13 October with the same Clang 21.1, which `setup.sh` keeps,
and every library was run again. Only Solaris Studio keeps its own, by nature.
Every library was measured in the same run, on 12 November 2026, at the commit
`2b5e4f9` of `configure-clean`, the machine running Ubuntu 26.04.1 (the tables
before, of 17 October, were measured on the same processor with the same Clang
21.1: the double reference took between 7 % less and 4 % more time in this run
than in that one, the other libraries between 7 % and 28 % more, GAOL V5.0.0
between 25 % less, for `cos`, which CORE-MATH rewrote in the import of 27
October (`0b2ae6d1`), and 17 % more, for `sqrt`, whose cause was not searched;
the way its operations compute and hand over the bounds has changed since too:
see below).

**What GAOL V5.0.0 changes.** The elementary functions, which mathlib used to
bound, are faster, and so is the integer power:

| | GAOL V5.0.0 | GAOL 4.2.3 | |
|---|---:|---:|---|
| `log` | 20.0 ns | 51.2 ns | 2.6 times faster |
| `exp` | 18.7 | 40.1 | 2.1 |
| `pow(x, y)` | 47.9 | 97.9 | 2.0 |
| five-line block | 141 | 273 | 1.9 |
| line of sin and cos | 86.9 | 166 | 1.9 |
| `sin` | 42.3 | 77.1 | 1.8 |
| `pow(x, 3)` | 5.75 | 9.74 | 1.7 |
| line of powers | 39.5 | 66.8 | 1.7 |
| `cos` | 40.5 | 69.5 | 1.7 |

Two things go at once with mathlib: the outward move of each bound, and the
two changes of rounding direction each function made, to nearest before
mathlib and back upward after. The integer power is computed from exact
products, both bounds at once in an SSE2 register, with an error bound proved
beforehand (point B of `TODO.md`). **And the bounds are tighter,
not looser**: in the last table below, GAOL V5.0.0's results are no wider than
libieeep1788's, which computes every bound with MPFR, for **every one of the
seventeen operations** (excess 0.0e+00), where GAOL 4.2.3 is wider on the
square root, `exp`, `log`, `sin`, `cos`, both powers and the lines that use
them.

**The arithmetic is faster than GAOL 4.2.3's**, which is what the operators
of GAOL V5.0.0 are written for (point ZZ of `TODO.md`):

| | GAOL V5.0.0 | GAOL 4.2.3 | |
|---|---:|---:|---|
| `mul` | 2.26 ns | 11.0 ns | 4.9 times faster |
| `line_arith` | 5.46 | 20.1 | 3.7 |
| `sqr` | 1.86 | 6.96 | 3.7 |
| `div` | 2.50 | 8.32 | 3.3 |
| `shekel5` | 84.5 | 202 | 2.4 |

The sums and the differences, 2.2 ns against 2.6, and the square root, 7.4
against 7.4, take what they took: a sum is one instruction, and what it waits
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
differences within 9 % of what they are, 2.3 ns against 2.2, and costs 1.7 times
on the products (3.8 ns) and 1.6 times on the quotients (3.9 ns), 1.2 times on
`sqr` (2.2 ns against 1.9) and 1.3 times on `pow(x, 3)` (7.2 ns against 5.8):
these operations take the AVX-512 instructions, whose rounding direction is in
the instruction, and set none, so that nothing is saved and restored. The square
root, 4.4 ns, is faster than without the preservation (7.4 ns): the instruction
rounds each root in its direction, where the SSE2 path checks the root of the C
library with a product. The elementary functions read the control register of
the direction, set it upward and write it back, once each, and take 1.2 to 1.5
times longer (`exp` 27.7 ns against 18.7, `log` 25.1 against 20.0, `sin` and
`cos` 1.3 times, `pow(x, y)` 1.2 times). They paid more, as `sqr` and the integer powers did, until point ZZ of
`TODO.md`: the two writes of the control register, each one waiting for the
operations in flight, took 6 ns a `sqr` (8.3 ns, 4.4 times); with the C
library's `fegetround()` and `fesetround()` and the writes of the control word
of the x87 unit as well, `pow(x, 3)` took 14 ns more (19.0 ns, 3.7 times; 8.4 ns
with the writes of the SSE register alone, where the machine has no AVX-512) and
the elementary functions 1.2 to 1.7 times longer. Shekel 5 takes 166 ns against
84.5, and every operation of the table is faster than with GAOL 4.2.3, which does
not preserve the direction either (Shekel 5: 202 ns). The results are the same:
the sums of the midpoints and of the widths of the million results are those of
GAOL V5.0.0, to 2e-16 relatively.

**Arithmetic.** GAOL is the fastest on the four operations: + and − take GAOL
V5.0.0 2.2 ns (2.6 for GAOL 4.2.3), 2.5 times as fast as filib++ (5.5 ns) and
8.0 to 9.3 times as fast as PROFIL/BIAS (17 ns) and Solaris Studio (20 ns);
× and ÷ take GAOL V5.0.0 2.3 and 2.5 ns (11 and 8.3 for GAOL 4.2.3), against
14 and 10 ns for filib++, 18 and 18 ns for PROFIL/BIAS, 25 and 23 ns for
Solaris Studio and 24 and 21 ns for Boost.Interval. An addition of intervals
costs GAOL V5.0.0 2.4 times an addition of doubles: one vector addition for
both bounds, and the check that the rounding direction is upward, where filib++
sets the rounding direction and restores it, and PROFIL/BIAS calls a function of
BIAS that sets it downward, then upward, then back to nearest — which is why its
four arithmetic operations all take about the same time. The integer power
`pown(x, 3)` takes GAOL V5.0.0 5.8 ns and GAOL 4.2.3 9.7 ns: filib++'s
`power(x, 3)` takes 23 ns, Boost.Interval's 23 ns, PROFIL/BIAS's 38 ns and
Solaris Studio's `x**3` 162 ns; `sqr` takes GAOL V5.0.0 1.9 ns, against 9.9
for filib++, 18 for Boost.Interval, 23 for PROFIL/BIAS and 63 for Solaris
Studio.

**Elementary functions.** PROFIL/BIAS is the fastest on the square root (4.5 ns, against 4.4 for GAOL V5.0.0 with the direction preserved), exp (12 ns), log (13 ns) and the real power `pow(x, y)` (37 ns),
which it computes from the libm of the system, moved outward, without correct
rounding: its bounds are among the widest (below), and its sin and cos,
computed by its own argument reduction, are the slowest after libieeep1788's,
136 and 149 ns. GAOL V5.0.0 comes next on exp and log, 19 and 20 ns — faster
than Boost.Interval (33 ns each), filib++ (37 and 33 ns) and Solaris Studio
(42 and 43 ns) — with **correctly rounded** values, where the others are not.
On sin and cos GAOL V5.0.0 (42 and 41 ns) and filib++ (42 and 43 ns), with its
own polynomials, are level, GAOL V5.0.0 ahead by 5 % on cos, and Solaris Studio
follows (46 ns each): GAOL pays there for correct rounding over the whole range,
and for dividing the bounds by an interval enclosing π to find where the
function is monotonic, asking CORE-MATH for the signs of the derivative where
the division cannot tell ([issue #6](https://github.com/Jordan08/GAOL/issues/6)).
GAOL V5.0.0's sin had become faster than its cos when CORE-MATH rewrote `sin.c`
(upstream 6b84457); the import of CORE-MATH of 27 October (upstream 0b2ae6d1)
rewrote both, and the cos, 54 ns on 17 October, takes 41 ns. On `pow(x, y)`
GAOL V5.0.0 takes 48 ns, against 78 for filib++, 83 for Boost.Interval, 98 for
GAOL 4.2.3 and 143 for Solaris Studio.

**Formulas.** GAOL V5.0.0 is the fastest on every formula: the arithmetic line
(5.5 ns, and 20 for GAOL 4.2.3, against 33 for filib++ and 68 to 70 for
Boost.Interval, PROFIL/BIAS and Solaris Studio), the line of powers (40 and 67
ns, against 97 for filib++, 106 for PROFIL/BIAS and 285 for Solaris Studio), the
line of sin and cos (87 ns, against 110 for filib++, 166 for GAOL 4.2.3, 186 for
Solaris Studio and 344 for PROFIL/BIAS), Shekel 5 (84.5 and 202 ns, against 464
for filib++, 1 026 for Boost.Interval, 1 209 for PROFIL/BIAS and 1 930 for
Solaris Studio: Shekel 5 is 20 squares, 45 additions and subtractions and 5
divisions, and its squares cost Solaris Studio 63 ns each and PROFIL/BIAS 23)
and the five-line block (141 ns, against 232 for filib++, 273 for GAOL 4.2.3,
470 for PROFIL/BIAS and 513 for Solaris Studio). filib++ comes second on the
line of sin and cos and on the five-line block, where its sin and cos weigh
most.

**libieeep1788** is 16 to 156 times slower than GAOL V5.0.0: every bound is an
MPFR computation, about 150 ns for an addition and 5.7 to 7.5 µs for sin, cos
and the real power. Its own README warns that its focus is correctness, not
speed. GAOL V5.0.0 now gives **the same bounds as it does**, tightest
everywhere, 16 to 156 times faster.

**Boost.Interval** is slower than GAOL V5.0.0 on every operation: 6.4 times
on + and −, 3.4 to 10.6 times on ×, ÷, the square, the square root and the
integer power, 1.7 to 1.9 times on exp, log, sin, cos and the real power, 12.5
times on the arithmetic line and 12 times on Shekel 5. Each of its
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
with GAOL 4.2.3 (the real power, computed as exp(y·log(x))), 2.1e-14 to
2.4e-13 with filib++ (1.4e-13 for log, 2.4e-13 for the real power), 9.0e-15 to
1.2e-13 with PROFIL/BIAS, and at most 4.1e-15 with Solaris Studio; with
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

The timings were measured on a laptop, on 12 November 2026, on mains power
(an editor and a browser open beside the benchmark, a load average of 1.7 to
2.3 during the run, the benchmark being one of it), each time being the best of
the runs of three rounds, each program being run in turn with the others, on a
processor of its own (`taskset -c 2`). libieeep1788's times, from
a single run of each operation in each round, are the least steady. Solaris
Studio's and PROFIL/BIAS's operations are calls into libraries, as GAOL
V5.0.0's `+`, `-`, `*`, `/` and `sqr` are (the bounds in registers), where the
compiler inlines GAOL 4.2.3's, filib++'s and Boost.Interval's.

## The timings

<!-- BEGIN GENERATED TABLES (doc/compare/code/bench.py) -->

Measured on:

```
Date:            2026-11-12
Processor:       11th Gen Intel(R) Core(TM) i7-1185G7 @ 3.00GHz (taskset -c 2)
System:          Linux 7.0.0-43-generic, Ubuntu 26.04.1 LTS
C++ compiler:    Ubuntu clang version 21.1.8 (6ubuntu1)
C++ flags:       -std=c++11 -O3 -DNDEBUG -mfma (GAOL: -frounding-math -fno-fast-math -ffp-contract=off -msse2 -msse3 -mfma; libieeep1788, filib++, PROFIL/BIAS and Boost.Interval: -frounding-math -fno-fast-math -ffp-contract=off)
Fortran:         f90: Sun Fortran 95 8.7 Linux_i386 2014/10/20, flags: -O3 -xia
GAOL V5.0.0:     the branch of this checkout (v4.3.2-2005-g2b5e4f9), CMake Release, CORE-MATH of 3rd/math-core compiled into the library, the rounding direction not preserved
GAOL V5.0.0 preserved: the same, built with GAOL_PRESERVE_ROUNDING=ON, the rounding direction restored after each operation, AVX-512 path of + - * / sqrt sqr and pown on, the AVX-512 instructions with the rounding direction in them (GAOL_PREFER_AVX512 left at its default)
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
| `add` | 0.90 | 151 | 2.17 | 2.31 | 2.55 | 5.45 | 20.2 | 17.4 | 13.8 | 69.7 | 1.1 | 1.2 | 2.5 | 9.3 | 8.0 | 6.4 |
| `sub` | 0.87 | 155 | 2.17 | 2.35 | 2.56 | 5.50 | 20.1 | 17.4 | 14.0 | 71.6 | 1.1 | 1.2 | 2.5 | 9.3 | 8.0 | 6.5 |
| `mul` | 0.84 | 189 | 2.26 | 3.77 | 11.0 | 14.2 | 24.5 | 17.9 | 24.0 | 83.3 | 1.7 | 4.9 | 6.3 | 10.8 | 7.9 | 10.6 |
| `div` | 0.90 | 194 | 2.50 | 3.94 | 8.32 | 10.3 | 23.0 | 17.6 | 21.1 | 77.6 | 1.6 | 3.3 | 4.1 | 9.2 | 7.1 | 8.5 |
| `sqr` | 0.59 | 104 | 1.86 | 2.21 | 6.96 | 9.92 | 63.4 | 22.9 | 18.2 | 55.8 | 1.2 | 3.7 | 5.3 | 34.1 | 12.3 | 9.8 |
| `sqrt` | 1.29 | 120 | 7.35 | 4.36 | 7.43 | 16.6 | 30.2 | 4.48 | 25.0 | 16.3 | 0.6 | 1.0 | 2.3 | 4.1 | 0.6 | 3.4 |
| `exp` | 3.40 | 1 711 | 18.7 | 27.7 | 40.1 | 37.2 | 42.2 | 12.3 | 33.3 | 91.3 | 1.5 | 2.1 | 2.0 | 2.3 | 0.7 | 1.8 |
| `log` | 3.25 | 2 199 | 20.0 | 25.1 | 51.2 | 33.3 | 43.2 | 12.9 | 33.2 | 110 | 1.3 | 2.6 | 1.7 | 2.2 | 0.6 | 1.7 |
| `sin` | 13.2 | 6 325 | 42.3 | 54.0 | 77.1 | 42.0 | 45.9 | 136 | 79.7 | 149 | 1.3 | 1.8 | 1.0 | 1.1 | 3.2 | 1.9 |
| `cos` | 11.9 | 5 694 | 40.5 | 50.9 | 69.5 | 42.8 | 46.4 | 149 | 74.2 | 141 | 1.3 | 1.7 | 1.1 | 1.1 | 3.7 | 1.8 |
| `pow_int` | 12.5 | 267 | 5.75 | 7.23 | 9.74 | 22.8 | 162 | 37.7 | 23.0 | 46.5 | 1.3 | 1.7 | 4.0 | 28.2 | 6.6 | 4.0 |
| `pow_real` | 10.1 | 7 489 | 47.9 | 58.7 | 97.9 | 78.0 | 143 | 37.2 | 83.3 | 156 | 1.2 | 2.0 | 1.6 | 3.0 | 0.8 | 1.7 |
| `line_arith` | 1.18 | 759 | 5.46 | 9.06 | 20.1 | 33.4 | 70.3 | 69.1 | 68.4 | 139 | 1.7 | 3.7 | 6.1 | 12.9 | 12.6 | 12.5 |
| `line_trig` | 25.2 | 12 688 | 86.9 | 115 | 166 | 110 | 186 | 344 | 208 | 146 | 1.3 | 1.9 | 1.3 | 2.1 | 4.0 | 2.4 |
| `line_pow` | 19.0 | 2 760 | 39.5 | 51.2 | 66.8 | 96.8 | 285 | 106 | 131 | 69.8 | 1.3 | 1.7 | 2.4 | 7.2 | 2.7 | 3.3 |
| `shekel5` | 2.46 | 10 369 | 84.5 | 166 | 202 | 464 | 1 930 | 1 209 | 1 026 | 123 | 2.0 | 2.4 | 5.5 | 22.8 | 14.3 | 12.1 |
| `block5` | 43.9 | 15 913 | 141 | 180 | 273 | 232 | 513 | 470 | 384 | 113 | 1.3 | 1.9 | 1.6 | 3.6 | 3.3 | 2.7 |

GAOL V5.0.0, rounding direction preserved, is the same GAOL built with `GAOL_PRESERVE_ROUNDING=ON`: each operation restores the rounding direction it found, where GAOL V5.0.0 leaves it upward. AVX-512 stays enabled in it: `GAOL_PREFER_AVX512`, which that option turns on by default, is left on, so that `+`, `−`, `×`, `/`, `√`, `sqr` and the integer powers take the AVX-512 instructions, the rounding direction being in the instruction (the machine description above says whether the library took them); the other operations read the direction, set it upward and restore it. Its column is in this table only: its results are those of GAOL V5.0.0.

#### Total time of the 1 000 000 operations (seconds)

| Operation | double (reference) | libieeep1788 | GAOL V5.0.0 | GAOL 4.2.3 | filib++ | Solaris Studio f90 | PROFIL/BIAS | Boost.Interval |
|---|---|---|---|---|---|---|---|---|
| `add` | 0.001 | 0.151 | 0.002 | 0.003 | 0.005 | 0.020 | 0.017 | 0.014 |
| `sub` | 0.001 | 0.155 | 0.002 | 0.003 | 0.005 | 0.020 | 0.017 | 0.014 |
| `mul` | 0.001 | 0.189 | 0.002 | 0.011 | 0.014 | 0.024 | 0.018 | 0.024 |
| `div` | 0.001 | 0.194 | 0.002 | 0.008 | 0.010 | 0.023 | 0.018 | 0.021 |
| `sqr` | 0.001 | 0.104 | 0.002 | 0.007 | 0.010 | 0.063 | 0.023 | 0.018 |
| `sqrt` | 0.001 | 0.120 | 0.007 | 0.007 | 0.017 | 0.030 | 0.004 | 0.025 |
| `exp` | 0.003 | 1.711 | 0.019 | 0.040 | 0.037 | 0.042 | 0.012 | 0.033 |
| `log` | 0.003 | 2.199 | 0.020 | 0.051 | 0.033 | 0.043 | 0.013 | 0.033 |
| `sin` | 0.013 | 6.325 | 0.042 | 0.077 | 0.042 | 0.046 | 0.136 | 0.080 |
| `cos` | 0.012 | 5.694 | 0.041 | 0.069 | 0.043 | 0.046 | 0.149 | 0.074 |
| `pow_int` | 0.012 | 0.267 | 0.006 | 0.010 | 0.023 | 0.162 | 0.038 | 0.023 |
| `pow_real` | 0.010 | 7.489 | 0.048 | 0.098 | 0.078 | 0.143 | 0.037 | 0.083 |
| `line_arith` | 0.001 | 0.759 | 0.005 | 0.020 | 0.033 | 0.070 | 0.069 | 0.068 |
| `line_trig` | 0.025 | 12.688 | 0.087 | 0.166 | 0.110 | 0.186 | 0.344 | 0.208 |
| `line_pow` | 0.019 | 2.760 | 0.040 | 0.067 | 0.097 | 0.285 | 0.106 | 0.131 |
| `shekel5` | 0.002 | 10.369 | 0.085 | 0.202 | 0.464 | 1.930 | 1.209 | 1.026 |
| `block5` | 0.044 | 15.913 | 0.141 | 0.273 | 0.232 | 0.513 | 0.470 | 0.384 |

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
Date:            2026-11-12
Processor:       11th Gen Intel(R) Core(TM) i7-1185G7 @ 3.00GHz (taskset -c 2)
System:          Linux 7.0.0-43-generic, Ubuntu 26.04.1 LTS
C++ compiler:    Ubuntu clang version 21.1.8 (6ubuntu1)
C++ flags:       -std=c++11 -O3 -DNDEBUG -mfma (GAOL: -frounding-math -fno-fast-math -ffp-contract=off -msse2 -msse3 -mfma; libieeep1788, filib++, PROFIL/BIAS and Boost.Interval: -frounding-math -fno-fast-math -ffp-contract=off)
GAOL V5.0.0:     the branch of this checkout (v4.3.2-2005-g2b5e4f9), CMake Release, CORE-MATH of 3rd/math-core compiled into the library, the rounding direction not preserved
GAOL V5.0.0 preserved: the same, built with GAOL_PRESERVE_ROUNDING=ON, the rounding direction restored after each operation, AVX-512 path of + - * / sqrt sqr and pown on, the AVX-512 instructions with the rounding direction in them (GAOL_PREFER_AVX512 left at its default)
GAOL 4.2.3:      the last version of Frédéric Goualard (cd0ee1a of https://github.com/goualard-f/GAOL), its configure, the rounding direction not preserved, mathlib 2.1.1
libieeep1788:    1f10b89, MPFR 4.2.1, GMP 6.3.0
filib++:         3.0.2.2, interval<double, native_switched, i_mode_extended_flag>
PROFIL/BIAS:     2.0.8, x86-64-Linux-compat-gcc configuration, built by clang-21 and clang++-21
Boost.Interval:  Boost 1.92.0, interval<double> with save_state<rounded_transc_opp<double> > and checking_strict<double>, the elementary functions of the C library, glibc 2.43
```

1 000 000 evaluations of each line, on the same intervals. Each time is the best of 3 rounds, each program being run in turn with the others: 15 runs for double (reference), 3 runs for libieeep1788, 15 runs for GAOL V5.0.0, 15 runs for GAOL V5.0.0, rounding direction preserved, 15 runs for GAOL 4.2.3, 15 runs for filib++, 15 runs for PROFIL/BIAS, 15 runs for Boost.Interval in all.

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

| Line | double (reference) | libieeep1788 | GAOL V5.0.0 | GAOL V5.0.0, rounding direction preserved | GAOL 4.2.3 | filib++ | PROFIL/BIAS | Boost.Interval | libieeep1788 / GAOL V5.0.0 | GAOL V5.0.0, rounding direction preserved / GAOL V5.0.0 | GAOL 4.2.3 / GAOL V5.0.0 | filib++ / GAOL V5.0.0 | PROFIL/BIAS / GAOL V5.0.0 | Boost.Interval / GAOL V5.0.0 |
|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|
| `sum16` | 2.52 | 2 282 | 16.7 | 35.2 | 30.4 | 89.9 | 244 | 193 | 137 | 2.1 | 1.8 | 5.4 | 14.6 | 11.6 |
| `diff16` | 2.55 | 2 402 | 17.3 | 35.3 | 30.7 | 88.0 | 233 | 192 | 139 | 2.0 | 1.8 | 5.1 | 13.5 | 11.1 |
| `mul8` | 2.45 | 1 323 | 20.4 | 32.0 | 34.6 | 53.7 | 110 | 123 | 64.8 | 1.6 | 1.7 | 2.6 | 5.4 | 6.0 |
| `mul8_tree` | 2.46 | 1 361 | 17.0 | 30.0 | 33.5 | 56.3 | 113 | 122 | 80.1 | 1.8 | 2.0 | 3.3 | 6.7 | 7.2 |
| `quot8` | 3.91 | 1 384 | 23.0 | 40.3 | 39.4 | 55.3 | 114 | 122 | 60.1 | 1.7 | 1.7 | 2.4 | 5.0 | 5.3 |
| `mix13` | 2.47 | 2 429 | 24.3 | 38.7 | 55.2 | 92.4 | 208 | 204 | 100 | 1.6 | 2.3 | 3.8 | 8.6 | 8.4 |
| `mix_tree` | 2.51 | 3 466 | 32.4 | 50.4 | 71.5 | 129 | 322 | 295 | 107 | 1.6 | 2.2 | 4.0 | 10.0 | 9.1 |
| `pos_sum16` | 1.10 | 2 257 | 17.6 | 33.6 | 28.3 | 83.4 | 233 | 196 | 128 | 1.9 | 1.6 | 4.7 | 13.3 | 11.1 |
| `pos_mul8` | 0.96 | 1 320 | 21.4 | 31.0 | 22.0 | 44.9 | 115 | 119 | 61.8 | 1.5 | 1.0 | 2.1 | 5.4 | 5.6 |
| `pos_quot8` | 3.91 | 1 351 | 23.8 | 38.6 | 41.4 | 49.2 | 116 | 126 | 56.9 | 1.6 | 1.7 | 2.1 | 4.9 | 5.3 |

GAOL V5.0.0, rounding direction preserved, is the same GAOL built with `GAOL_PRESERVE_ROUNDING=ON`, as in the table of the operations above: each operation of the line restores the rounding direction it found, where GAOL V5.0.0 leaves it upward, and AVX-512 stays enabled in it, so that `+`, `−`, `×` and `/` take the AVX-512 instructions, the rounding direction being in the instruction (the machine description above says whether the library took them). Its column is in this table only: its results are those of GAOL V5.0.0.

#### Time per operation of a line (nanoseconds)

The time of the line divided by its number of operations.

| Line | double (reference) | libieeep1788 | GAOL V5.0.0 | GAOL 4.2.3 | filib++ | PROFIL/BIAS | Boost.Interval |
|---|---|---|---|---|---|---|---|
| `sum16` | 0.17 | 152 | 1.11 | 2.03 | 5.99 | 16.3 | 12.9 |
| `diff16` | 0.17 | 160 | 1.15 | 2.05 | 5.87 | 15.5 | 12.8 |
| `mul8` | 0.35 | 189 | 2.92 | 4.94 | 7.67 | 15.8 | 17.6 |
| `mul8_tree` | 0.35 | 194 | 2.43 | 4.78 | 8.04 | 16.2 | 17.4 |
| `quot8` | 0.56 | 198 | 3.29 | 5.62 | 7.89 | 16.3 | 17.4 |
| `mix13` | 0.19 | 187 | 1.87 | 4.24 | 7.11 | 16.0 | 15.7 |
| `mix_tree` | 0.13 | 182 | 1.70 | 3.76 | 6.77 | 17.0 | 15.5 |
| `pos_sum16` | 0.07 | 150 | 1.17 | 1.89 | 5.56 | 15.5 | 13.1 |
| `pos_mul8` | 0.14 | 189 | 3.05 | 3.15 | 6.42 | 16.4 | 17.1 |
| `pos_quot8` | 0.56 | 193 | 3.39 | 5.92 | 7.03 | 16.5 | 18.0 |

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
line, and faster than GAOL 4.2.3 on all ten, 1.03 to 2.3 times as fast: a line
of 15 additions takes 16.7 ns, 1.1 ns an addition, against 30.4 ns (2.0 ns),
and a line of 7 products 20.4 ns (2.9 ns a product) against 34.6 ns (4.9 ns);
the quotients, 23.0 ns for 7 (3.3 ns each) against 39.4 ns (5.6 ns), 1.7 times
as fast, and the lines that mix the four operations, `mix13` and `mix_tree`,
24.3 and 32.4 ns against 55.2 and 71.5 ns, 2.3 and 2.2 times. The line of
products of intervals of one sign, `pos_mul8`, is where GAOL 4.2.3 comes
nearest, 22.0 ns against 21.4 ns: with the signs the same from one evaluation
to the next, its branches are predicted, and what it still pays is the bounds
that go through memory between two operations; its additions and its quotients
of intervals of one sign take 1.6 and 1.7 times as long (28.3 ns against
17.6 ns, and 41.4 ns against 23.8 ns). An operation of a line costs GAOL V5.0.0
less than the same operation timed alone above, 1.1 ns an addition against
2.2: the two doubles of the result stay in a register for the next operation,
and the benchmark of a single operation, which reads two arrays and writes a
third for each, waits for the memory, where a line reads eight arrays at most
for its 7 to 19 operations.

**Preserving the rounding direction** (the column of that name, `GAOL_PRESERVE_ROUNDING=ON`
with the AVX-512 path: the first measure of the lines on x86) makes the lines
1.5 to 2.1 times as long as without, 30.0 to 50.4 ns for lines of 7 to 19
operations, 35.2 ns for the 15 additions against 16.7: each operation restores
the direction it found. It is faster than filib++, which sets and restores the
direction too, on all ten lines (1.3 to 2.6 times as fast), and faster than
GAOL 4.2.3, which does not preserve it, on five (`mul8`, `mul8_tree`, `mix13`,
`mix_tree` and `pos_quot8`, 1.07 to 1.43 times as fast); it is as fast on
`quot8` (40.3 ns against 39.4) and slower on the additions (`sum16`, `diff16`,
`pos_sum16`: 35.2, 35.3 and 33.6 ns against 30.4, 30.7 and 28.3) and on the
products of intervals of one sign (`pos_mul8`: 31.0 ns against 22.0), where
GAOL 4.2.3's branches are predicted.

Against the other libraries, filib++ takes 2.1 to 5.4 times as long as GAOL
V5.0.0 on the ten lines, Boost.Interval 5.3 to 11.6 times, PROFIL/BIAS 4.9 to
14.6 times and libieeep1788 57 to 139 times. The bounds are the tightest:
the mean widths of the results of GAOL V5.0.0 are those of libieeep1788, which
computes every bound with MPFR, to 1.1e-16 relatively on the lines of additions
and exactly on the others, where GAOL 4.2.3's differ from them by 2.3e-16 on the
lines of quotients; PROFIL/BIAS's sums of midpoints differ because its `Mid`
rounds upward (see above), by a lot where the sum of a line, `diff16`, is as
small as 10^-12 for a million values.

## The same lines on a 64-bit ARM processor

<!-- BEGIN GENERATED ARM64 (doc/compare/code/bench.py) -->

Measured on:

```
Date:            2026-11-12
Processor:       Apple M2 (not pinned to a processor: no taskset on macOS), power: AC Power
System:          Darwin 27.0.0, macOS 27.0.1
C++ compiler:    Apple clang version 21.0.0 (clang-2100.4.1.1)
C++ flags:       -std=c++11 -O3 -DNDEBUG (GAOL: -frounding-math -fno-fast-math -ffp-contract=off; GAOL 4.2.3: -frounding-math -fno-fast-math -ffp-contract=off)
GAOL V5.0.0:     the branch of this checkout (v4.3.2-2005-g2b5e4f92), CMake Release, NEON intervals, CORE-MATH of 3rd/math-core compiled into the library, the rounding direction not preserved
GAOL V5.0.0 preserved: the same, built with GAOL_PRESERVE_ROUNDING=ON, the rounding direction restored after each operation (the FPCR register, read and written by each operation)
GAOL 4.2.3:      the last version of Frédéric Goualard (cd0ee1a of https://github.com/goualard-f/GAOL), its configure generated again by autoreconf and told --build=arm-apple-darwin (the one it commits does not know macOS on ARM), the rounding direction not preserved, the FPU intervals (it has no instructions of 64-bit ARM to compute them with), mathlib 2.1.1 configured with --host=aarch64-unknown-linux-gnu (its configure knows no macOS on ARM; no line here calls it)
```

1 000 000 evaluations of each line, on the same intervals (drawn by the same program as above). Each time is the best of 3 rounds, each program being run in turn with the others: 15 runs for GAOL V5.0.0, 15 runs for GAOL V5.0.0, rounding direction preserved, 15 runs for GAOL 4.2.3 in all.

#### Time per evaluation of a line (nanoseconds)

| Line | Operations | GAOL V5.0.0 | GAOL V5.0.0, rounding direction preserved | GAOL 4.2.3 | GAOL V5.0.0, rounding direction preserved / GAOL V5.0.0 | GAOL 4.2.3 / GAOL V5.0.0 |
|---|---:|---|---|---|---|---|
| `sum16` | 15 | 9.05 | 383 | 41.8 | 42.3 | 4.6 |
| `diff16` | 15 | 9.20 | 389 | 77.4 | 42.3 | 8.4 |
| `mul8` | 7 | 10.6 | 187 | 27.8 | 17.6 | 2.6 |
| `mul8_tree` | 7 | 9.95 | 190 | 24.5 | 19.1 | 2.5 |
| `quot8` | 7 | 18.1 | 229 | 37.3 | 12.7 | 2.1 |
| `mix13` | 13 | 14.3 | 352 | 61.4 | 24.6 | 4.3 |
| `mix_tree` | 19 | 18.5 | 515 | 82.0 | 27.9 | 4.4 |
| `pos_sum16` | 15 | 9.11 | 385 | 39.6 | 42.3 | 4.3 |
| `pos_mul8` | 7 | 9.50 | 188 | 16.1 | 19.8 | 1.7 |
| `pos_quot8` | 7 | 17.9 | 231 | 46.9 | 12.9 | 2.6 |

#### Same results?

The sum of the midpoints of the results and their mean width, with the relative difference of the other versions from GAOL V5.0.0:

| Line | Σ midpoints (GAOL V5.0.0) | Δ GAOL V5.0.0, rounding direction preserved | Δ GAOL 4.2.3 | mean width (GAOL V5.0.0) | excess GAOL V5.0.0, rounding direction preserved | excess GAOL 4.2.3 |
|---|---|---|---|---|---|---|
| `sum16` | 54024197.3 | 0.0e+00 | 0.0e+00 | 1.03024 | -1.1e-16 | 0.0e+00 |
| `diff16` | -4.690193546e-13 | 0.0e+00 | 0.0e+00 | 1.03024 | -1.1e-16 | 0.0e+00 |
| `mul8` | 255576459.8 | 0.0e+00 | 0.0e+00 | 13602.3 | 0.0e+00 | 0.0e+00 |
| `mul8_tree` | 255576459.8 | 0.0e+00 | 0.0e+00 | 13602.3 | 0.0e+00 | 0.0e+00 |
| `quot8` | -1016.492029 | -1.1e-16 | 0.0e+00 | 0.0625416 | 0.0e+00 | 0.0e+00 |
| `mix13` | 738436463 | 0.0e+00 | 0.0e+00 | 79.6756 | 0.0e+00 | 0.0e+00 |
| `mix_tree` | -155881.5083 | 0.0e+00 | 0.0e+00 | 1.24303 | 0.0e+00 | 0.0e+00 |
| `pos_sum16` | 56040038.03 | 0.0e+00 | 0.0e+00 | 0.64606 | 0.0e+00 | 0.0e+00 |
| `pos_mul8` | 2.178309922e+10 | 0.0e+00 | 0.0e+00 | 1161.99 | 0.0e+00 | 0.0e+00 |
| `pos_quot8` | 138457.8704 | 0.0e+00 | 0.0e+00 | 0.0326453 | 0.0e+00 | -1.1e-16 |

<!-- END GENERATED ARM64 -->

GAOL V5.0.0 takes 9.05 to 18.5 ns for a line of 7 to 19 operations on this
processor, 0.6 to 2.6 ns an operation, where GAOL 4.2.3, which has no
instructions of 64-bit ARM to compute its FPU intervals with, takes 1.7 to 8.4
times as long. With the rounding direction preserved the same lines take 187 to
515 ns, 25.5 to 33.0 ns an operation, 12.7 to 42.3 times as long as without:
the program computes to nearest, so each operation sets the direction upward and
puts nearest back, two `msr` of FPCR of 11 ns each that change it, with the
`mrs` of 0.8 ns that reads it, the 27 ns an operation of
[The three builds](../three-builds.md). The sums of the midpoints and of the
widths of the three agree to 1.1e-16 relatively.

The processor is not pinned and the machine was on mains power, the low power
mode off, with a load average of 1.6 at the start and 1.7 at the end (the run
took 121 s, the build of the three GAOLs and of the three programs included), and
not idle: an editor and other programs ran on it, with a load average of 1.6 to
2.2 at the readings taken every 30 s. The three rounds of a line differ by 1.0 %
at the median and by 3.4 % at most (`mul8` of GAOL V5.0.0, whose second round is
the slowest); the best of the three is kept.
