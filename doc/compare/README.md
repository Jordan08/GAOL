# Comparison of GAOL with libieeep1788, filib++, PROFIL/BIAS and Solaris Studio

Part of the documentation of [this fork of GAOL](../../README.md#documentation).

Five implementations of interval arithmetic on doubles are compared:

| | libieeep1788 | GAOL | filib++ | Solaris Studio | PROFIL/BIAS |
|---|---|---|---|---|---|
| What | [libieeep1788](https://github.com/nehmeier/libieeep1788), by Marco Nehmeier (University of Würzburg), last commit in 2015 | This fork of GAOL, the library of Frédéric Goualard | [filib++](https://www2.math.uni-wuppertal.de/wrswt/software/filib.html) 3.0.2.2 (University of Wuppertal), as IBEX distributes it, which IBEX computes with by default on Windows | The `interval(8)` type of Sun's Fortran 95 compiler, `f90 -xia`, in Solaris Studio 12.4 (2014) | [PROFIL/BIAS](https://www.tuhh.de/ti3/keil/profil/) 2.0.8 (2009), by Olaf Knüppel and Christian Keil (TU Hamburg-Harburg) |
| Language | C++11, header-only | C++ | C++, templates and a small library | Fortran 95, intervals built into the compiler | C++ (PROFIL) over C (BIAS) |
| Bounds | Computed by MPFR, correctly rounded | Computed with the rounding direction set upward; elementary functions with mathlib, log and the hyperbolic functions with CORE-MATH | Computed with the rounding direction set and restored by each operation (`native_switched`); elementary functions of its own | Computed by `libsunimath` | Computed by the BIAS routines, which set the rounding direction downward, then upward, then back to nearest; elementary functions from the libm, moved outward |
| Arithmetic | The set-based flavor of the preliminary IEEE P1788, the prototype of IEEE 1788-2015 | Set-based, following IEEE 1788-2015 in most of its special cases, with the deviations the reports list | The extended mode of filib++ (`i_mode_extended_flag`, as IBEX uses it), where the empty set and the infinities are handled | The containment sets of Sun's interval arithmetic (G. W. Walster), where the infinities are values | The interval arithmetic before IEEE 1788: no empty set, and an argument outside the domain of a function is an error that aborts the program |

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

| | libieeep1788 | GAOL | filib++ | Solaris Studio | PROFIL/BIAS |
|---|---|---|---|---|---|
| Special cases with IEEE 1788's result | 279 of 279 | 258 of 286 | 126 of 243 | 150 of 261 | 56 of 221 |
| … or an interval enclosing it | 0 | 22 | 50 | 41 | 22 |
| … or another result | 0 | 6 | 67 | 70 | 143 |
| Cases it has no operation for | 11 | 0 | 44 | 26 | 66 |
| `x + y` | 205 ns | 3.3 ns | 7.9 ns | 24 ns | 22 ns |
| `x * y` | 259 ns | 16 ns | 23 ns | 29 ns | 22 ns |
| `sin(x)` | 7.9 µs | 103 ns | 51 ns | 59 ns | 172 ns |
| `log(x)` | 2.7 µs | 30 ns | 41 ns | 56 ns | 15 ns |
| `pow(x, 3)` | 351 ns | 21 ns | 28 ns | 214 ns | 49 ns |
| Shekel 5 | 15 µs | 301 ns | 599 ns | 2.4 µs | 1.6 µs |

- **libieeep1788** gives the result of IEEE 1788 in every case it can compute,
  as tightly as possible, and is 12 to 91 times slower than GAOL.
- **GAOL** is the fastest on the arithmetic and on most formulas, and gives
  IEEE 1788's result, or an interval enclosing it, in all but 6 special
  cases, all from its hybrid `pow`, which takes `pown` for integer
  exponents. Its log, CORE-MATH's, gives the tightest bounds, faster than
  filib++'s.
- **filib++** is the fastest on sin and cos, twice as fast as GAOL, as fast
  as GAOL on ÷, but 1.4 times as slow on × and 2.4 times on + and −, and its
  bounds of elementary functions and real powers are up to 36 doubles wider
  than the tightest. Its extended mode gives other results than IEEE 1788
  wherever an infinity or a division by zero is involved, much as Solaris
  Studio: `interval(+∞)` is [MAX, +∞] and `[0] * [1, +∞]` is
  [−∞, +∞]; its `operator>>` rounds the bounds it reads to nearest, and its
  comparisons with the empty set and the infinities differ from IEEE 1788's.
- **PROFIL/BIAS** is a library of the interval arithmetic before IEEE 1788: it
  has no empty set, and an operation whose argument leaves the domain of the
  function, a division by an interval containing zero included, prints an error
  and aborts the program. Its exponential, logarithm and real power, taken
  from the libm, are the fastest of the five (15, 15 and 48 ns), and so is its
  square root (5.6 ns); its arithmetic is 6 to 7 times slower than GAOL's on +
  and −, its sin and cos are the slowest (172 and 194 ns), and its integer
  power, computed as exp(n log x), is 1.1e−05 wider than the tightest on
  average.
- **Solaris Studio** is almost twice as fast as GAOL on sin and cos, but slow on
  integer powers and squares. Its containment sets give other results than
  IEEE 1788 wherever an infinity, a division by zero or an invalid argument is
  involved: `interval(+∞)` is [MAX, +∞], `[1, 2] / [0, 1]` is [−∞, +∞] and
  `interval(2, 1)` is [−∞, +∞].
