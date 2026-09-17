# Tests

Part of the documentation of [this fork of GAOL](../README.md#documentation).

The programs of `tests/` compare the bounds GAOL computes with the exact results
of the operations, independently of GAOL and of the floating-point environment.
The exact results are computed with integers, or were computed with 2000 bits of
precision by [mpmath](https://mpmath.org). They follow the rounding tests of
Codac.

- **`arithmetic`:** on doubles and intervals of every magnitude (subnormal
  doubles and overflows included), sums, differences, products, quotients,
  relational divisions, squares, inverses, `abs`, `min`, `max`, `&`, `|` have to
  be the tightest enclosures, and so do square roots. Integer powers and n-th
  roots have to be enclosures, within a few doubles, negative powers whose
  x^n is beyond the largest double included, the odd roots of negative
  numbers being the opposites of the roots of their magnitudes, and the n-th
  roots of intervals the roots of their bounds (of their part in `[0, +oo]`
  for an even n). `pow([10], -400)`, `pow([2], -1050)`, the negative powers of
  intervals containing 0, and the roots of 0, 1 and −1 have to be the tightest
  enclosures. `gaol::uipow(x, n)` has to give what `pow(x, n)` gives, `[1]`
  for n = 0 and the empty set for an empty x. The operators of an interval with a double have to give, on
  bounds and doubles of special values (zeros of both signs, infinities, NaN),
  the sets the operators with `interval(d)` give.
  Products of intervals with zero and infinite bounds have to be the hull of
  the products of the bounds, a zero bound times an infinite one counting as 0.
  The constructors have to give the empty set for a lower bound of `+oo`, an
  upper bound of `-oo`, bounds in the wrong order and NaN bounds.
- **`elementary`:** `exp`, `log`, `sin`, `cos`, `tan`, `asin`, `acos`, `atan`,
  `sinh`, `cosh`, `tanh`, `asinh`, `acosh`, `atanh`, `sqrt` and `pow` at doubles,
  at intervals, and at intervals whose images are known exactly (extrema,
  poles, domains, `log` of intervals holding no positive number being empty,
  `exp(0)`, `log(1)` and `1^y` exact). The functions have to be the tightest
  enclosures where their value is 0, 1, ±π/4, ±π/2 or π (`sin(0)`, `cos(0)`,
  `acos(1)`, `acos(-1)`, `asin(1)`, `atan(1)`, `atan([-oo, +oo])`,
  `acosh(1)`...), and `cosh`, `sinh` and `tanh` beyond the largest double and
  near 1. sin and cos have to be within one
  double of the tightest bounds up to 2^25, `sin([1e-10])` included. The
  values are in `elementary_values.h`, which `elementary_values.py` generates.
- **`rounding_direction`:** about 100 operations of GAOL's interface, called
  with the rounding direction upward, to nearest, downward and toward zero (and
  on x86, with the x87 and SSE directions differing), have to give the results
  they give when called rounding upward, and leave the rounding direction
  upward, or as they found it with `GAOL_PRESERVE_ROUNDING`. Products and sums
  have to be the tightest enclosures, also when computed in a loop that changes
  the rounding direction before each of them.
- **`numbers`:** `interval("0.1")` has to be the tightest interval enclosing the
  number read, and the number itself when it is a double. The constants have to
  be the tightest enclosures of π, 2π and π/2. The literals of IEEE 1788-2015
  (`[entire]`, `[ ]`, `[1,]`, `3.56?1`, hexadecimal numbers...) have to be read
  as the tightest intervals enclosing them, whatever the case of their letters,
  and `[inf]` and the like as the empty set. Expressions read again and again
  have to give the same interval, and those GAOL cannot read or compute
  (`nth_root(8, 1.5)`, `sin(1)+`, `1+pow(2, atan2(1,1))`...) have to throw,
  the exception of `atan2()` going through the parser. Built with
  LeakSanitizer, the tests check that the parser frees the nodes of all of
  them. The intervals written in decimal (`interval_format::bounds`), in the
  general, scientific and fixed formats with 1 to 20 digits, have to enclose
  the intervals, each bound less than one unit of its last digit away, near
  the powers of ten too, where a digit moved outward changes the exponent;
  read back, they have to enclose the intervals written. In hexadecimal, the
  bits of the bounds have to be written.
- **`other_functions`:** midpoints (of subnormal bounds, and of `intervalf`
  when GAOL is built with the float intervals), widths, radii (`rad()`, `mid_rad()`), magnitudes, mignitudes, Hausdorff
  distances, splitting, integer parts, the comparisons of IEEE 1788-2015
  (`precedes`, `interior`, `subset`, `equal`, `disjoint`, from Tables 10.3 and
  10.4, on intervals of zero, infinite and small bounds and the empty set), and
  the relational functions (`sqrt_rel`, `div_rel`...).

The CMake build compiles them with `GAOL_BUILD_TESTS` (`OFF` by default: no
build compiles tests unless asked to, as `make check` and `with-test` for
GAOL's own check programs). `tests/find_package` builds the same tests with
an installed GAOL, and `.github/scripts/tests.sh` with a GAOL installed by
configure or meson.

`tests/performance.cpp` (`gaol_performance`) measures the time per operation of
GAOL's arithmetic and elementary functions, and of the same operations on
doubles. It is not a test: the continuous integration prints its table in the
summary of the jobs.

What they show of GAOL, beyond the fixes below:

- `atan2()` is not implemented: it throws `unavailable_feature_error`.
- `sin` and `cos` tell the pieces of their argument where they are monotonic
  by dividing it by an interval enclosing π: beyond 2^25, an argument within
  about 2^-51·|x| of an extremum may be taken as reaching it. `tan` adds an
  interval enclosing π/2 to its argument to tell its branch, and gives
  [-oo, +oo] when it cannot. (See [Accuracy of the operations](accuracy.md).)
- `pow(x, y)` is `exp(y log x)`, whose relative width grows with `|y log x|`.
