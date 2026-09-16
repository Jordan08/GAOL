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
  roots have to be enclosures, within a few doubles, the odd roots of negative
  numbers being the opposites of the roots of their magnitudes, and the n-th
  roots of intervals the roots of their bounds (of their part in `[0, +oo]`
  for an even n). The operators of an interval with a double have to give, on
  bounds and doubles of special values (zeros of both signs, infinities, NaN),
  the sets the operators with `interval(d)` give.
  Products of intervals with zero and infinite bounds have to be the hull of
  the products of the bounds, a zero bound times an infinite one counting as 0.
  The constructors have to give the empty set for a lower bound of `+oo`, an
  upper bound of `-oo`, bounds in the wrong order and NaN bounds.
- **`elementary`:** `exp`, `log`, `sin`, `cos`, `tan`, `asin`, `acos`, `atan`,
  `sinh`, `cosh`, `tanh`, `asinh`, `acosh`, `atanh`, `sqrt` and `pow` at doubles,
  at intervals, and at intervals whose images are known exactly (extrema,
  poles, domains, `log` of intervals holding no positive number being empty).
  The values are in `elementary_values.h`, which
  `elementary_values.py` generates.
- **`rounding_direction`:** about 95 operations of GAOL's interface, called
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
  and `[inf]` and the like as the empty set.
- **`other_functions`:** midpoints (of subnormal bounds and of `intervalf`
  too), widths, magnitudes, mignitudes, Hausdorff distances, splitting, integer
  parts, and the relational functions (`sqrt_rel`, `div_rel`...).

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
- `sin`, `cos` and `tan` reduce their argument modulo an interval enclosing π.
  Their bounds take on its width for each multiple of π subtracted, about
  2^-52·|x|.
- `pow(x, y)` is `exp(y log x)`, whose relative width grows with `|y log x|`.
- The decimal output of intervals (`interval_format::bounds`) relies on the C
  library to round the bounds outward, which the C runtime of Windows and musl
  on 64-bit ARM processors do not do. The hexadecimal format
  (`interval_format::hexa`) gives the bounds exactly.
- GAOL's parser does not free the nodes of the expressions it reads, a few dozen
  bytes for each `interval("...")`.
