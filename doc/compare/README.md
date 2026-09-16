# Comparison of GAOL with libieeep1788, filib++ and Solaris Studio

Part of the documentation of [this fork of GAOL](../../README.md#documentation).

Four implementations of interval arithmetic on doubles are compared:

| | GAOL | libieeep1788 | filib++ | Solaris Studio |
|---|---|---|---|---|
| What | This fork of GAOL, the library of Frédéric Goualard | [libieeep1788](https://github.com/nehmeier/libieeep1788), by Marco Nehmeier (University of Würzburg), last commit in 2015 | [filib++](https://www2.math.uni-wuppertal.de/wrswt/software/filib.html) 3.0.2.2 (University of Wuppertal), as IBEX distributes it, which IBEX computes with by default on Windows | The `interval(8)` type of Sun's Fortran 95 compiler, `f90 -xia`, in Solaris Studio 12.4 (2014) |
| Language | C++ | C++11, header-only | C++, templates and a small library | Fortran 95, intervals built into the compiler |
| Bounds | Computed with the rounding direction set upward; elementary functions with mathlib | Computed by MPFR, correctly rounded | Computed with the rounding direction set and restored by each operation (`native_switched`); elementary functions of its own | Computed by `libsunimath` |
| Arithmetic | Set-based, following IEEE 1788-2015 in most of its special cases, with the deviations the reports list | The set-based flavor of the preliminary IEEE P1788, the prototype of IEEE 1788-2015 | The extended mode of filib++ (`i_mode_extended_flag`, as IBEX uses it), where the empty set and the infinities are handled | The containment sets of Sun's interval arithmetic (G. W. Walster), where the infinities are values |

The comparison has two parts:

- [Special cases](special_cases.md): 226 special cases taken from GAOL's tests
  (infinities, zeros, NaN, empty sets, divisions by zero, `pow` and `pown`,
  `+=` and the other operators with doubles, reading from text, midpoints and
  widths), computed by the four libraries and compared with the results of
  IEEE 1788-2015.
- [Performance](performance.md): the time of a million additions,
  subtractions, multiplications, divisions, sines, cosines, powers,
  one-line combinations, evaluations of Shekel 5 and five-line blocks.

[code/](code/README.md) holds the scripts and programs, in C++, Fortran,
Python and bash, that run both parts again and rewrite the tables of the
reports.

## In short

| | GAOL | libieeep1788 | filib++ | Solaris Studio |
|---|---|---|---|---|
| Special cases with IEEE 1788's result | 185 of 224 | 217 of 217 | 111 of 210 | 117 of 205 |
| … or an interval enclosing it | 31 | 0 | 50 | 36 |
| … or another result | 8 | 0 | 49 | 52 |
| Cases it has no operation for | 0 | 8 | 15 | 20 |
| `x + y` | 3.7 ns | 213 ns | 7.6 ns | 24 ns |
| `x * y` | 16 ns | 262 ns | 17 ns | 29 ns |
| `sin(x)` | 130 ns | 8.6 µs | 53 ns | 59 ns |
| `pow(x, 3)` | 14 ns | 332 ns | 27 ns | 214 ns |
| Shekel 5 | 334 ns | 14 µs | 584 ns | 2.4 µs |

- **libieeep1788** gives the result of IEEE 1788 in every case it can compute,
  as tightly as possible, and is 13 to 73 times slower than GAOL.
- **GAOL** is the fastest on the arithmetic and on most formulas, and gives
  IEEE 1788's result, or an interval enclosing it, in all but 8 special
  cases: its hybrid `pow`, which takes `pown` for integer exponents, and two
  forms of literals it does not read, `[entire]` and the uncertain form.
- **filib++** is the fastest on the elementary functions, 2.3 to 2.5 times as
  fast as GAOL on sin and cos, and as fast as GAOL on × and ÷, but twice as
  slow on + and −, and its bounds of elementary functions and real powers are
  up to 36 doubles wider than the tightest. Its extended mode gives other results than
  IEEE 1788 wherever an infinity or a division by zero is involved, much as
  Solaris Studio: `interval(+∞)` is [MAX, +∞] and `[0] * [1, +∞]` is
  [−∞, +∞]; its `operator>>` rounds the bounds it reads to nearest.
- **Solaris Studio** is twice as fast as GAOL on sin and cos, but slow on
  integer powers and squares. Its containment sets give other results than
  IEEE 1788 wherever an infinity, a division by zero or an invalid argument is
  involved: `interval(+∞)` is [MAX, +∞], `[1, 2] / [0, 1]` is [−∞, +∞] and
  `interval(2, 1)` is [−∞, +∞].
