# Comparison of GAOL with libieeep1788 and Solaris Studio

Part of the documentation of [this fork of GAOL](../../README.md#documentation).

Three implementations of interval arithmetic on doubles are compared:

| | GAOL | libieeep1788 | Solaris Studio |
|---|---|---|---|
| What | This fork of GAOL, the library of Frédéric Goualard | [libieeep1788](https://github.com/nehmeier/libieeep1788), by Marco Nehmeier (University of Würzburg), last commit in 2015 | The `interval(8)` type of Sun's Fortran 95 compiler, `f90 -xia`, in Solaris Studio 12.4 (2014) |
| Language | C++ | C++11, header-only | Fortran 95, intervals built into the compiler |
| Bounds | Computed with the rounding direction set upward; elementary functions with mathlib | Computed by MPFR, correctly rounded | Computed by `libsunimath` |
| Arithmetic | Set-based, following IEEE 1788-2015 in most of its special cases, with the deviations the reports list | The set-based flavor of the preliminary IEEE P1788, the prototype of IEEE 1788-2015 | The containment sets of Sun's interval arithmetic (G. W. Walster), where the infinities are values |

The comparison has two parts:

- [Special cases](special_cases.md): 226 special cases taken from GAOL's tests
  (infinities, zeros, NaN, empty sets, divisions by zero, `pow` and `pown`,
  `+=` and the other operators with doubles, reading from text, midpoints and
  widths), computed by the three libraries and compared with the results of
  IEEE 1788-2015.
- [Performance](performance.md): the time of a million additions,
  subtractions, multiplications, divisions, sines, cosines, powers,
  one-line combinations, evaluations of Shekel 5 and five-line blocks.

[code/](code/README.md) holds the scripts and programs, in C++, Fortran,
Python and bash, that run both parts again and rewrite the tables of the
reports.

## In short

| | GAOL | libieeep1788 | Solaris Studio |
|---|---|---|---|
| Special cases with IEEE 1788's result | 185 of 224 | 217 of 217 | 117 of 205 |
| … or an interval enclosing it | 31 | 0 | 36 |
| … or another result | 8 | 0 | 52 |
| Cases it has no operation for | 0 | 8 | 20 |
| `x + y` | 3.8 ns | 211 ns | 24 ns |
| `x * y` | 16 ns | 260 ns | 29 ns |
| `sin(x)` | 129 ns | 9.8 µs | 59 ns |
| `pow(x, 3)` | 14 ns | 336 ns | 214 ns |
| Shekel 5 | 342 ns | 21 µs | 2.4 µs |

- **libieeep1788** gives the result of IEEE 1788 in every case it can compute,
  as tightly as possible, and is 13 to 90 times slower than GAOL.
- **GAOL** is the fastest on the arithmetic and on most formulas, and gives
  IEEE 1788's result, or an interval enclosing it, in all but 8 special
  cases: its hybrid `pow`, which takes `pown` for integer exponents, and two
  forms of literals it does not read, `[entire]` and the uncertain form.
- **Solaris Studio** is the fastest on the elementary functions, twice as
  fast as GAOL on sin and cos, but slow on integer powers and squares. Its
  containment sets give other results than IEEE 1788 wherever an infinity, a
  division by zero or an invalid argument is involved: `interval(+∞)` is
  [MAX, +∞], `[1, 2] / [0, 1]` is [−∞, +∞] and `interval(2, 1)` is [−∞, +∞].
