<!-- Copyright (c) 2026 ENSTA, France
     Created 2026-09-20 by Jordan NININ -->
# Comparison of GAOL with libieeep1788, filib++, PROFIL/BIAS, Solaris Studio and Boost.Interval

Part of the documentation of [GAOL v5](../../README.md#documentation).

Six implementations of interval arithmetic on doubles are compared:

| | libieeep1788 | GAOL | filib++ | Solaris Studio | PROFIL/BIAS | Boost.Interval |
|---|---|---|---|---|---|---|
| What | [libieeep1788](https://github.com/nehmeier/libieeep1788), by Marco Nehmeier (University of Würzburg), last commit in 2015 | GAOL v5, by Jordan Ninin (ENSTA), which continues the GAOL of Frédéric Goualard | [filib++](https://www2.math.uni-wuppertal.de/wrswt/software/filib.html) 3.0.2.2 (University of Wuppertal), as IBEX distributes it, which IBEX computes with by default on Windows | The `interval(8)` type of Sun's Fortran 95 compiler, `f90 -xia`, in Solaris Studio 12.4 (2014) | [PROFIL/BIAS](https://www.tuhh.de/ti3/keil/profil/) 2.0.8 (2009), by Olaf Knüppel and Christian Keil (TU Hamburg-Harburg) | [Boost.Interval](https://www.boost.org/libs/numeric/interval) of Boost 1.92.0 (2026), by Hervé Brönnimann, Guillaume Melquiond and Sylvain Pion, the interval library C++ users find first |
| Language | C++11, header-only | C++ | C++, templates and a small library | Fortran 95, intervals built into the compiler | C++ (PROFIL) over C (BIAS) | C++03 templates, header-only |
| Bounds | Computed by MPFR, correctly rounded | Computed with the rounding direction set upward; elementary functions with CORE-MATH, correctly rounded upward | Computed with the rounding direction set and restored by each operation (`native_switched`); elementary functions of its own | Computed by `libsunimath` | Computed by the BIAS routines, which set the rounding direction downward, then upward, then back to nearest; elementary functions from the libm, moved outward | Computed with the rounding direction set upward by each operation, the lower bounds negated, and set back after it (the default policies, `save_state<rounded_arith_opp>`); elementary functions from the C library, called with the rounding direction set downward or upward (`rounded_transc_opp`), a direction glibc does not always honour |
| Arithmetic | The set-based flavor of the preliminary IEEE P1788, the prototype of IEEE 1788-2015 | Set-based, following IEEE 1788-2015 in most of its special cases, with the deviations the reports list | The extended mode of filib++ (`i_mode_extended_flag`, as IBEX uses it), where the empty set and the infinities are handled | The containment sets of Sun's interval arithmetic (G. W. Walster), where the infinities are values | The interval arithmetic before IEEE 1788: no empty set, and an argument outside the domain of a function is an error that aborts the program | The interval arithmetic before IEEE 1788, without empty set: its default checking (`checking_strict`) throws an exception where an operation would create one |

The comparison has three parts:

- [Special cases](special_cases.md): 290 special cases taken from GAOL's tests
  (infinities, zeros, NaN, empty sets, divisions by zero, `pow` and `pown`,
  `+=` and the other operators with doubles, reading from text, midpoints,
  widths, radii, comparisons and `atan2`), computed by the six libraries and compared
  with the results of IEEE 1788-2015, GAOL under its own names (`gaol`) and
  under those of the standard (`gaol_ieee1788`).
- [Enclosure](enclosure.md): whether the elementary functions of GAOL and
  Boost.Interval enclose their values, at 100 000 random arguments for each
  function, against mpmath, GAOL under both namespaces.
- [Performance](performance.md): the time of a million additions,
  subtractions, multiplications, divisions, sines, cosines, powers,
  one-line combinations, evaluations of Shekel 5 and five-line blocks, and
  the time of long lines of + − × ÷ between 8 intervals, GAOL v5 being called
  under the names of the standard (`gaol_ieee1788`), as the other libraries of
  IEEE 1788, and GAOL 4.2.3 under its own.

GAOL v5 gives its intervals under two namespaces: `gaol`, its own names,
compatible with GAOL 4, and `gaol_ieee1788`, the names of IEEE 1788-2015. They
compute with the same functions, and differ mainly in the power: the `pow` of
`gaol` takes the integer power for an integer exponent, a negative base
included, where the `pow` of `gaol_ieee1788` is the real power of the
standard, defined for x > 0 only, and `pown` the integer power (see
[Using GAOL](../using.md#the-powers-in-the-two-namespaces)).

[code/](code/README.md) holds the scripts and programs, in C++, Fortran,
Python and bash, that run the three parts again and rewrite the tables of the
reports.

## In short

| | libieeep1788 | GAOL | filib++ | Solaris Studio | PROFIL/BIAS | Boost.Interval |
|---|---|---|---|---|---|---|
| Special cases with IEEE 1788's result | 281 of 281 | 288 of 288 under `gaol_ieee1788`; 280 under `gaol` | 126 of 245 | 150 of 261 | 56 of 223 | 93 of 168 |
| … or an interval enclosing it | 0 | 0; 2 | 50 | 41 | 22 | 9 |
| … or another result | 0 | 0; 6 | 69 | 70 | 145 | 66 |
| Cases it has no operation for | 8 | 0 | 44 | 28 | 65 | 121 |
| Elementary functions at random points: results missing the value | — | 0 of 1 600 000 | — | — | — | 34 % (20 % with `rounded_transc_std`) |
| `x + y` | 137 ns | 2.2 ns | 5.1 ns | 18 ns | 14 ns | 12 ns |
| `x * y` | 171 ns | 2.3 ns | 12 ns | 21 ns | 15 ns | 21 ns |
| `sin(x)` | 5.5 µs | 43 ns | 33 ns | 41 ns | 115 ns | 71 ns |
| `log(x)` | 1.9 µs | 21 ns | 27 ns | 39 ns | 10 ns | 30 ns |
| `pown(x, 3)` | 240 ns | 5.1 ns | 18 ns | 145 ns | 35 ns | 20 ns |
| Shekel 5 | 9.0 µs | 108 ns | 397 ns | 1.7 µs | 1.1 µs | 920 ns |

The times were measured on 17 October 2026, every library in the same run,
compiled by Clang 21.1 (see [performance](performance.md)).

- **libieeep1788** gives the result of IEEE 1788 in every case it can compute,
  as tightly as possible, and is 17 to 135 times slower than GAOL.
- **GAOL** is the fastest on the arithmetic, the integer power and most
  formulas. Under the names of the standard, `gaol_ieee1788`, it gives IEEE
  1788's result in every special case; under its own, `gaol`, in all but 8,
  all from its hybrid `pow`, which takes `pown` for integer exponents, a
  negative base included. Its elementary functions, CORE-MATH's, give the
  tightest bounds; its exp and log are faster than filib++'s, its sin and cos
  slower.
- **filib++** is the fastest on sin and cos, 1.3 to 1.6 times as fast as GAOL,
  but 5.3 times as slow on ×, 3.5 times on ÷ and 2.3 times on + and −, and its bounds of elementary functions and real powers are up to 36
  doubles wider than the tightest. Its extended mode gives other results than
  IEEE 1788 wherever an infinity or a division by zero is involved, much as
  Solaris Studio: `interval(+∞)` is [MAX, +∞] and `[0] * [1, +∞]` is
  [−∞, +∞]; its `operator>>` rounds the bounds it reads to nearest, and its
  comparisons with the empty set and the infinities differ from IEEE 1788's.
- **PROFIL/BIAS** is a library of the interval arithmetic before IEEE 1788: it
  has no empty set, and an operation whose argument leaves the domain of the
  function, a division by an interval containing zero included, prints an error
  and aborts the program. Its exponential, logarithm and real power, taken
  from the libm, are the fastest of the five (9.9, 10.2 and 34 ns), and so is
  its square root (3.7 ns); its arithmetic is 6.4 times slower than GAOL's on
  + and −, its sin and cos are the slowest after libieeep1788's (115 and
  131 ns), and its integer power, computed as exp(n log x), is 1.1e−05 wider
  than the tightest on average.
- **Boost.Interval**, the library of Boost, is the interval library C++
  programmers find first. Its arithmetic gave the tightest intervals in the
  benchmark, but its elementary functions are the C library's, called with the
  rounding direction set downward or upward, which glibc does not always
  honour: at the points of the test of enclosure, 34 % of its results miss the
  value of the function with glibc 2.43, 20 % with its other policy, and
  `atan([−∞, +∞])` misses π/2. With its default checking it has no empty set:
  an operation that would create one throws an exception. It is 5.6 times
  slower than GAOL on + and −, 1.8 to 11 times on ×, ÷ and the formulas, 3.5
  times on the square root, 3.9 times on the integer power and 1.2 to 1.6
  times on the other elementary functions.
- **Solaris Studio** is 1.0 to 1.3 times as fast as GAOL on sin and cos, but
  slow on integer powers and squares. Its containment sets give other results
  than IEEE 1788 wherever an infinity, a division by zero or an invalid
  argument is involved: `interval(+∞)` is [MAX, +∞], `[1, 2] / [0, 1]` is
  [−∞, +∞] and `interval(2, 1)` is [−∞, +∞].

## Test suites

[ITF1788 on GAOL v5](itf1788.md) runs the 9542 cases of
[ITF1788](https://github.com/oheim/ITF1788), the test suite of the operations
of IEEE 1788-2015, on GAOL v5 under the names of the standard
(`gaol_ieee1788`), with the default and the FPU builds: 7285 cases pass, every
case of `pow` and `pown` among them, and the 10 that fail show no bug of GAOL
v5 (results valid and accurate but not the tightest, which the standard does
not require of these operations, and cases that expect a result that is not
the tightest); the others are of operations GAOL v5 does not provide, or of
the decorated intervals.
[itf1788/](itf1788/) holds the generator and the runner, which fetches
ITF1788.

[The benchmark of Tang et al. (2021) on GAOL v5](tang2021.md) runs the
benchmark of interval libraries of Tang, Ferguson, Schneider, Zorin, Kamil
and Panozzo (PPAM 2022) on GAOL v5 under the names of the standard
(`gaol_ieee1788`), with the default and the FPU builds: on
100 000 inputs of each of its 132 expressions, every interval of GAOL encloses
the value of the expression computed, the single operations are the tightest,
and the intervals are narrower than those of filib's C version, the paper's
reference; the 9884 results that fail the paper's own check come from decimal
constants that are not doubles, a defect of the benchmark (GAOL v5).
[tang2021/](tang2021/) holds the patch that adds GAOL to the benchmark, the
scripts that fetch and run it, and the checker that replaces Mathematica.
