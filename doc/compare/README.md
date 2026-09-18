# Comparison of GAOL with libieeep1788, filib++, PROFIL/BIAS and Solaris Studio

Part of the documentation of [this fork of GAOL](../../README.md#documentation).

Five implementations of interval arithmetic on doubles are compared:

| | GAOL | libieeep1788 | filib++ | PROFIL/BIAS | Solaris Studio |
|---|---|---|---|---|---|
| What | This fork of GAOL, the library of Frédéric Goualard | [libieeep1788](https://github.com/nehmeier/libieeep1788), by Marco Nehmeier (University of Würzburg), last commit in 2015 | [filib++](https://www2.math.uni-wuppertal.de/wrswt/software/filib.html) 3.0.2.2 (University of Wuppertal), as IBEX distributes it, which IBEX computes with by default on Windows | [PROFIL/BIAS](https://www.tuhh.de/ti3/keil/profil/) 2.0.8 (2009), by Olaf Knüppel and Christian Keil (TU Hamburg-Harburg) | The `interval(8)` type of Sun's Fortran 95 compiler, `f90 -xia`, in Solaris Studio 12.4 (2014) |
| Language | C++ | C++11, header-only | C++, templates and a small library | C++ (PROFIL) over C (BIAS) | Fortran 95, intervals built into the compiler |
| Bounds | Computed with the rounding direction set upward; elementary functions with mathlib | Computed by MPFR, correctly rounded | Computed with the rounding direction set and restored by each operation (`native_switched`); elementary functions of its own | Computed by the BIAS routines, which set the rounding direction downward, then upward, then back to nearest; elementary functions from the libm, moved outward | Computed by `libsunimath` |
| Arithmetic | Set-based, following IEEE 1788-2015 in most of its special cases, with the deviations the reports list | The set-based flavor of the preliminary IEEE P1788, the prototype of IEEE 1788-2015 | The extended mode of filib++ (`i_mode_extended_flag`, as IBEX uses it), where the empty set and the infinities are handled | The interval arithmetic before IEEE 1788: no empty set, and an argument outside the domain of a function is an error that aborts the program | The containment sets of Sun's interval arithmetic (G. W. Walster), where the infinities are values |

The comparison has two parts:

- [Special cases](special_cases.md): 291 special cases taken from GAOL's tests
  (infinities, zeros, NaN, empty sets, divisions by zero, `pow` and `pown`,
  `+=` and the other operators with doubles, reading from text, midpoints,
  widths, radii, comparisons and `atan2`), computed by the five libraries and compared
  with the results of IEEE 1788-2015.
- [Performance](performance.md): the time of a million additions,
  subtractions, multiplications, divisions, sines, cosines, powers,
  one-line combinations, evaluations of Shekel 5 and five-line blocks.

[code/](code/README.md) holds the scripts and programs, in C++, Fortran,
Python and bash, that run both parts again and rewrite the tables of the
reports.

## In short

| | GAOL | libieeep1788 | filib++ | PROFIL/BIAS | Solaris Studio |
|---|---|---|---|---|---|
| Special cases with IEEE 1788's result | 258 of 286 | 279 of 279 | 126 of 243 | 56 of 221 | 150 of 261 |
| … or an interval enclosing it | 22 | 0 | 50 | 22 | 41 |
| … or another result | 6 | 0 | 67 | 143 | 70 |
| Cases it has no operation for | 0 | 11 | 44 | 66 | 26 |
| `x + y` | 3.7 ns | 213 ns | 7.5 ns | 22 ns | 24 ns |
| `x * y` | 16 ns | 261 ns | 17 ns | 22 ns | 29 ns |
| `sin(x)` | 113 ns | 8.0 µs | 53 ns | 175 ns | 59 ns |
| `pow(x, 3)` | 32 ns | 331 ns | 27 ns | 50 ns | 213 ns |
| Shekel 5 | 325 ns | 14 µs | 581 ns | 1.6 µs | 2.4 µs |

- **libieeep1788** gives the result of IEEE 1788 in every case it can compute,
  as tightly as possible, and is 10 to 72 times slower than GAOL.
- **GAOL** is the fastest on the arithmetic and on most formulas, and gives
  IEEE 1788's result, or an interval enclosing it, in all but 6 special
  cases, all from its hybrid `pow`, which takes `pown` for integer
  exponents.
- **filib++** is the fastest on the elementary functions, twice as fast as
  GAOL on sin and cos, and as fast as GAOL on × and ÷, but twice as slow on +
  and −, and its bounds of elementary functions and real powers are up to 36
  doubles wider than the tightest. Its extended mode gives other results than
  IEEE 1788 wherever an infinity or a division by zero is involved, much as
  Solaris Studio: `interval(+∞)` is [MAX, +∞] and `[0] * [1, +∞]` is
  [−∞, +∞]; its `operator>>` rounds the bounds it reads to nearest, and its
  comparisons with the empty set and the infinities differ from IEEE 1788's.
- **PROFIL/BIAS** is a library of the interval arithmetic before IEEE 1788: it
  has no empty set, and an operation whose argument leaves the domain of the
  function, a division by an interval containing zero included, prints an error
  and aborts the program. Its exponential, logarithm and real power, taken
  from the libm, are the fastest of the five (16, 17 and 53 ns), its arithmetic
  is 6 times slower than GAOL's on + and −, its sin and cos are the slowest
  (175 and 196 ns), and its integer power, computed as exp(n log x), is 1.1e−05
  wider than the tightest on average.
- **Solaris Studio** is almost twice as fast as GAOL on sin and cos, but slow on
  integer powers and squares. Its containment sets give other results than
  IEEE 1788 wherever an infinity, a division by zero or an invalid argument is
  involved: `interval(+∞)` is [MAX, +∞], `[1, 2] / [0, 1]` is [−∞, +∞] and
  `interval(2, 1)` is [−∞, +∞].
