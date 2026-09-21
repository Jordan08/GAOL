<!-- Copyright (c) 2026 ENSTA, France
     Created 2026-09-20 by Jordan NININ -->
# Tests

Part of the documentation of [GAOL v5](../README.md#documentation).

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
  upper bound of `-oo`, bounds in the wrong order and NaN bounds. `fma(x, y, z)`
  has to be the tightest enclosure over random boxes, against `std::fma` called
  in the downward and the upward rounding directly: GCC had folded the
  negations GAOL rounds its lower bound with, and the bound came one double
  above the exact one. `cancel_minus` and `cancel_plus` have to be the tightest
  and `y + cancel_minus(x, y)` to enclose x, over intervals of the same width
  and one double wider, which rounding cannot tell apart (GAOL v5).
- **`elementary`:** `exp`, `log`, `sin`, `cos`, `tan`, `asin`, `acos`, `atan`,
  `sinh`, `cosh`, `tanh`, `asinh`, `acosh`, `atanh`, `sqrt` and `pow` at doubles,
  at intervals, and at intervals whose images are known exactly (extrema,
  poles, domains, `log` of intervals holding no positive number being empty,
  `exp(0)`, `log(1)` and `1^y` exact). The functions have to be the tightest
  enclosures where their value is 0, 1, ±π/4, ±π/2 or π (`sin(0)`, `cos(0)`,
  `acos(1)`, `acos(-1)`, `asin(1)`, `atan(1)`, `atan([-oo, +oo])`,
  `acosh(1)`...), and `cosh`, `sinh` and `tanh` beyond the largest double and
  near 1. `pow(x, y)` has to be within one double of the tightest bounds at
  points, `pow([2], [1023.5])` among them, and over 385 boxes of bases and
  exponents in every position about the base 1 and the exponent 0. `atan2`
  has to be within one double of the tightest bounds at
  points of the four quadrants and over 324 boxes in every position about the
  axes, and the tightest over the boxes with infinite bounds or on an axis,
  `[-pi, pi]` across the half-line y = 0, x < 0, and empty at (0, 0). sin, cos and tan have to be within one
  double of the tightest bounds at every magnitude, `sin([1e-10])` and
  `cos([2^60])` included, and over 649 intervals next to their extrema and
  poles, of width about π and 2π, and of consecutive doubles up to the largest,
  -1, 1 and `[-oo, +oo]` being exact. `exp2`, `exp10`, `log2` and `log10` have
  to be the tightest enclosures, and the exact values themselves where they are
  doubles (`exp2` of a whole number, `exp10` of 0 to 22, `log2` of a power of
  two, `log10` of a power of ten up to 10^22). `nth_root(I, q)` has to be an
  enclosure for a negative q too, which is `inverse(nth_root(I, -q))`, and
  `nth_root(I, 3)` the cube root CORE-MATH gives. The integer functions of
  Table 9.1 — `sign`, `trunc`, `round_ties_to_even` and `round_ties_to_away` —
  are exact, and have to give the same result whatever rounding direction the
  calling code left, which `std::nearbyint` and `std::rint` would not: they are
  compared with a reference computed arithmetically, over the halfway values,
  the whole numbers and the doubles on either side of them, and over the
  magnitudes beyond 2^52, on both signs and on the empty set. The
  values are in `elementary_values.h`, which `elementary_values.py` generates.
- **`rounding_direction`:** about 100 operations of GAOL's interface, called
  with the rounding direction upward, to nearest, downward and toward zero (and
  on x86, with the x87 and SSE directions differing), have to give the results
  they give when called rounding upward, and leave the rounding direction
  upward, or as they found it with `GAOL_PRESERVE_ROUNDING`. Products and sums
  have to be the tightest enclosures, also when computed in a loop that changes
  the rounding direction before each of them. `cbrt`, `pow` and `atan2` have to
  leave the exception masks of the SSE control register as they found them, and
  an empty interval to be told empty after them: the `fesetexceptflag()` of
  mingw-w64 for 32-bit Windows unmasked the exceptions, and the comparison of
  the NaN bounds of an empty interval then killed the program (GAOL v5).
- **`numbers`:** `interval("0.1")` has to be the tightest interval enclosing the
  number read, and the number itself when it is a double. The constants have to
  be the tightest enclosures of π, 2π and π/2. The literals of IEEE 1788-2015
  (`[entire]`, `[ ]`, `[1,]`, `3.56?1`, hexadecimal numbers...) have to be read
  as the tightest intervals enclosing them, whatever the case of their letters,
  and `[inf]` and the like as the empty set. Expressions read again and again
  have to give the same interval, and those GAOL cannot read or compute
  (`nth_root(8, 1.5)`, `sin(1)+`, `atan2(1)`...) have to throw. Built with
  LeakSanitizer, the tests check that the parser frees the nodes of all of
  them. The intervals written in decimal (`interval_format::bounds`), in the
  general, scientific and fixed formats with 1 to 20 digits, have to enclose
  the intervals, each bound less than one unit of its last digit away, near
  the powers of ten too, where a digit moved outward changes the exponent;
  read back, they have to enclose the intervals written, and so do the two
  numbers the format of the agreeing digits stands for. In hexadecimal, the
  bounds have to be written in the hexadecimal-significand form of
  IEEE 1788-2015 (13.4.1) and read back bit for bit, which is the recovery
  requirement of 13.4: over random intervals, and over the empty set, the
  infinite bounds, the signed zeros, the subnormals and the largest doubles.
- **`other_functions`:** midpoints (of subnormal bounds, and of `intervalf`
  when GAOL is built with the float intervals), widths, radii (`rad()`, `mid_rad()`), magnitudes, mignitudes, Hausdorff
  distances, splitting, integer parts, the comparisons of IEEE 1788-2015
  (`precedes`, `interior`, `subset`, `equal`, `disjoint`, from Tables 10.3 and
  10.4, on intervals of zero, infinite and small bounds and the empty set), and
  the relational functions (`sqrt_rel`, `div_rel`...): `acos_rel`, `asin_rel`
  and `atan_rel` have to keep their value within 6 doubles from 1 to 2^50,
  and decide an interval of a single double beyond 2^53. `less`,
  `strictly_less`, `is_entire` and `is_common_interval` have to give the values
  of Tables 10.3 and 10.4. Each name of `gaol_ieee1788` has to be the operation
  of the standard it names, which a wrong translation would not show at
  compilation: the eight comparisons against the bounds of Table 10.3 and the
  empty cases of Table 10.4, over 20 000 pairs; `inf`, `sup` and the numeric
  functions against Table 10.2; the reverse functions with the arguments in
  the order of the standard, `mulRev(b, c, x)` being `div_rel(c, b, x)`; `pow`
  against the pow of Table 9.1, which GAOL's own `pow` is not for a negative
  base. The names are then called unqualified under
  `using namespace gaol_ieee1788;`, beside `using namespace gaol;`, which
  compiles only if none of them is ambiguous with a function of `gaol`, and
  `pow(x, y)` has to be the standard's there (GAOL v5).
- **`core_math`:** the bounds of the elementary functions against CORE-MATH
  itself. CORE-MATH is correctly rounded in the rounding direction in effect,
  so the tightest bounds of f at a double x are the values it gives rounding
  downward and upward, which the test computes by setting the direction itself,
  independently of GAOL. Computing in the upward direction it keeps, GAOL takes
  the upper bound from CORE-MATH and the double below it as the lower one: its
  bounds have to enclose the tightest ones, to be equal above and within one
  double below, and to be the tightest ones where `gaol_interval.cpp` gives the
  exact value itself (`log(1)`, `sin(0)`, the bounds of π/2 at `asin(1)`...).
  Each function is tried at the ends of its domain and next to them, at the
  values GAOL treats apart, at the powers of two and their neighbours, at the
  subnormals, and at random doubles of every magnitude. The functions of
  Table 10.5 GAOL provides have to be the tightest enclosures over intervals
  too: the hull of their image, computed from the values at the bounds of the
  part of the interval in the domain (`expm1`, `exp2m1`, `exp10m1`, `log1p`,
  `log2p1`, `log10p1`, `rsqrt`, `atanpi`, `asinpi`, `acospi`), at the points of
  the box nearest to the origin and farthest from it (`hypot`), at the
  corners of the box (`atan2pi`), and, for `sinpi`, `cospi` and `tanpi`, at
  every multiple of 1/2 within, enumerated one by one, where their extrema
  and poles are (GAOL v5). The bounds are drawn among the points where the
  value is a double (2<sup>k</sup> − 1, 10<sup>k</sup> − 1, the powers of 4,
  Pythagorean triples, the diagonals) and their neighbours, so that a value
  taken for exact when it is not, which gives a bound that no longer encloses
  the image, fails as a double missed does. The arguments of `asinpi` next to
  ±1 where CORE-MATH shifted a 64-bit integer by 65 bits are among them: the
  jobs of the continuous integration with the sanitizers stop there if that
  undefined behaviour comes back.
- **`expressions`:** `interval("...")` lexes the string, parses it into the
  tree of `gaol/gaol_expression.h` and evaluates that tree, so this test goes
  through every node of the tree and every way the string can be wrong: the
  numbers in every form the lexer takes (decimal, exponent, hexadecimal, the
  bounds given apart), the operators and the functions alone and nested, and
  the strings the parser has to refuse with an exception rather than an
  interval. The names GAOL v5 adds — `exp2`, `log2`, `cbrt`, `sign` and
  `trunc` — are read by both paths of the grammar, the direct one and the tree
  of `gaol/gaol_expression.h` that the bounds given apart go through, in any
  case of letters, the lexer taking the longest name so that `exp2` is not read
  as `exp` followed by 2. Each value is compared with the same computation written in C++,
  which the other tests check against the exact results: what is tested here is
  the lexer, the parser and the evaluation, not the operations.
- **`ieee1788`:** `gaol_ieee1788` as a program uses it: `using namespace
  gaol_ieee1788;` at file scope, without `using namespace gaol`, and
  `gaol/gaol_expression.h` included before `gaol/gaol`, so that the overloads
  of GAOL's expressions are in sight. The names of the standard are called
  unqualified, which compiles only if none is ambiguous with a function of
  `gaol`; `pow` has to be the pow of Table 9.1 with an interval, an `int` or a
  `double` as exponent, empty for a negative base where `gaol::pow` and `pown`
  give the integer power, and the tightest for an integer exponent beyond the
  ints; `min`, `atan2` and `hypot` take an interval and a number; `sqrt`,
  `floor`, `atan2` and `abs` on numbers remain those of C, which
  `static_assert` checks; `intervalToExact` does not change the output format
  and writes what `operator<<` writes in `interval_format::hexa` (GAOL v5).
- **`u128`:** the accurate phases of CORE-MATH's `log`, `sin`, `cos`, `tan`,
  `atan2` and `pow` compute with a 128-bit unsigned integer, which Visual C++
  has on no architecture and GCC has on no 32-bit target; there GAOL computes
  with the two 64-bit halves of `gaol/gaol_u128.h`. The test compiles those
  halves (`GAOL_U128_FORCE_EMULATION`) and compares every operation with the
  same operation on the native type of the compiler, at the ends of the ranges,
  at the powers of two and their neighbours, at random values, and over every
  shift count from 0 to 127: the two have to give the same 128 bits, so that
  the bounds GAOL computes with Visual C++ and on a 32-bit target are those it
  computes elsewhere. Where the compiler has no 128-bit type of its own, which
  is where the halves run in earnest, there is nothing to compare with, and the
  test checks the identities the operations satisfy instead (a + b − b = a,
  shifting left then right, the product of the halves against the schoolbook
  product, and the order).
- **`extended_precision`:** the results that doubles computed in extended
  precision, on the x87 unit of an x86 processor, round wrongly, which is why
  `gaol/gaol_config.h` refuses that unit. CORE-MATH in the four rounding
  directions, and GAOL's bounds of [x, x], are checked at the arguments where
  CORE-MATH, built past that refusal, gave the wrong double. Rounded to
  nearest, its results are rounded twice there, to 64 bits and then to a
  double: `exp(-0x1.74910d52d3051p+9)` gave 0 rather than 2^-1074, and
  `tanh(0x1.30fc1931f09c9p+4)` 1 rather than 1 − 2^-53. In the directed
  roundings, GCC 9 rounded to nearest at compile time the constants CORE-MATH
  rounds in the direction in effect, and GAOL's bounds of `exp2(-1075)`,
  `expm1(-800)` or `atan2()` of a tiny and a huge number did not enclose the
  exact values. The values are computed by `extended_precision_values.py`,
  with mpmath at 5000 bits. Built on the x87 unit, the test fails on Debian 12
  i386 with GCC 12 (22 checks rounded to nearest, tried without `expm1`,
  `exp2m1`, `exp10m1`, `sinpi`, `cospi` and `tanpi`), and on x86-64 with
  `-mfpmath=387` and GCC 9 (76 of its 300 checks).
- **`reverse`:** the relational functions against the reverse functions of
  IEEE 1788-2015 (10.5.4, Table 10.1): `sqrt_rel` (`sqrRev`), `invabs_rel`
  (`absRev`), `nth_root_rel` (`pownRev`), `asin_rel`, `acos_rel`, `atan_rel`
  (`sinRev`, `cosRev`, `tanRev`), `acosh_rel` (`coshRev`), and `div_rel`,
  `%` and `%=` (`mulRev`). Over the cases of the minimal tests of
  libieeep1788 and random ones, each result has to be, as 12.10.2 asks:
  empty when an argument is; **valid**, enclosing the hull of the x of I
  whose image is in J, which the tests hold the tightest enclosure of; and
  **accurate**, which the standard recommends for inf-sup types, within
  `nextOut(tightest(nextOut(arguments)))` (12.10.1). The bounds of `asin_rel`,
  `acos_rel` and `atan_rel` are allowed one double further, their reduction
  modulo π rounding twice, and the bound of x beyond 2^52, which they keep
  (see [Accuracy of the operations](accuracy.md)). The values are in
  `reverse_values.h`, which `reverse_values.py` generates: the hulls are
  computed from the definition of each function with exact rational
  arithmetic, and with mpmath for the periodic ones, not from GAOL nor from
  the results libieeep1788 expects, which the generator checks are
  enclosures of them.

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

- `sin`, `cos` and `tan` tell the pieces of their argument where they are
  monotonic by dividing it by an interval enclosing π, and, where the quotients
  cannot tell, next to an extremum or a pole and at the large magnitudes, from
  the signs of their derivative at the bounds, which CORE-MATH gives exactly:
  their bounds are within one double of the tightest at every magnitude. (See
  [Accuracy of the operations](accuracy.md).)
