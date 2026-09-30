<!-- Copyright (c) 2026 ENSTA, France
     Created 2026-09-20 by Jordan NININ -->
# Tests

Part of the documentation of [GAOL v5](../README.md#documentation).

The programs of `tests/` compare the bounds GAOL computes with the exact results
of the operations, independently of GAOL and of the floating-point environment.
The exact results are computed with integers, or were computed with 2000 bits of
precision by [mpmath](https://mpmath.org). They follow the rounding tests of
Codac.

Under the Debug C runtime of Visual C++, a failed assertion of the runtime or of
the checked iterators of its library, and an invalid parameter, are written to
stderr, where the test goes on, and make it fail (`tests/gaol_tests.h`): the
runtime showed them in a dialog box, which nobody closes on a machine of the
CI, and the test hung, without output, until ctest stopped it at 300 s. That
runtime reports such an assertion of its own when it writes a subnormal number
under denormals-are-zero ("unexpected input value; log10 failed"), and writes it
right: `numbers` does not check the output of subnormals with that mode there.

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
  enclosures. `gaol::pow(x, n)` for an unsigned n has to give what it gives
  for an int n, `[1]` for n = 0 and the empty set for an empty x. The operators of an interval with a double have to give, on
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
  poles, domains, `log` of intervals holding no positive number and `atanh` of
  intervals with no point of (-1, 1) being empty, `exp(0)`, `log(1)` and `1^y`
  exact). The functions have to be the tightest
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
  -1, 1 and `[-oo, +oo]` being exact. `tan([-M_PI_2, M_PI_2])` has to be the
  tightest enclosure, ±1.63e16: its width is the double below π, and it holds
  no pole. tan has to be within one double of the tightest bounds over 158
  intervals whose bounds are next to two consecutive poles, of a width within
  two doubles of the one below π or drawn at random, five of which,
  `[-M_PI_2, M_PI_2]` included, hold no pole and have a width that rounds up to
  the double below π (GAOL v5). `exp2`, `exp10`, `log2` and `log10` have
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
- **`rounding_direction`:** about 110 operations of GAOL's interface,
  called with the rounding direction upward, to nearest, downward and toward zero (and
  on x86, with the x87 and SSE directions differing), have to give the results
  they give when called rounding upward, and leave the rounding direction
  upward, or as they found it with `GAOL_PRESERVE_ROUNDING`. Products and sums
  have to be the tightest enclosures, also when computed in a loop that changes
  the rounding direction before each of them. `pow` with a subnormal result,
  which CORE-MATH rounds by itself in the direction `fegetround()` gives, has to
  be the tightest enclosure, computed apart with mpmath, in each of them, the
  x87 unit to nearest and the SSE instructions upward included: `fegetround()`
  of glibc reads the x87 unit, and the upper bound was below the exact value
  (GAOL v5). `cbrt`, `pow` and `atan2` have to leave the exception masks of the
  SSE control register as they found them, and an empty interval to be told
  empty after them: the `fesetexceptflag()` of mingw-w64 for 32-bit Windows
  unmasked the exceptions, and the comparison of the NaN bounds of an empty
  interval then killed the program (GAOL v5).
  `is_empty()` has to be true for six empty sets (`interval::emptyset()`,
  `[3, 2]`, `sqrt([-2, -1])`, `log([-2, -1])` and the two orders of
  `[1, 2] & [3, 4]`) and false for three nonempty intervals, and to raise no
  exception flag, as `interval::emptyset()` (GAOL v5). With glibc, each is
  also computed and told empty in a child process that enabled the
  invalid-operation exception, which must not die on SIGFPE: the comparison
  of the NaN bounds with `<=` did, and `interval::emptyset()` in a build
  without optimization. The operations of the interface with an empty operand
  on either side (about 190 calls, in 58 checks: one for each of the 48 calls
  that compared the NaN bounds of the empty set, the others in ten groups)
  have to give the result of the empty set and raise no invalid-operation
  flag, and, with glibc, not die in a child process that enabled the
  exception (GAOL v5): 45 of the 48 died with the SSE2 intervals, and all 48
  with the FPU ones (`x & y` for an empty `y`, `sqrt`, `exp`, `min`, `max`,
  `floor`, `set_contains()`, the output...).
  Where the processor does not trap an invalid operation, the test says so
  and skips that part.
  `gaol::cleanup()` has to set back the direction the first `gaol::init()`
  found, to nearest, or to leave it as it is with `GAOL_PRESERVE_ROUNDING`,
  although an interval computed in the initialization of a static object set
  it upward before `main()`: GAOL has to initialize itself before.
- **`automatic_cleanup`** (Linux only): after the end of `main()`, which leaves
  the rounding direction downward, GAOL's automatic cleanup has to set back
  the direction to nearest, as the program started, or to leave it downward
  with `GAOL_PRESERVE_ROUNDING`. The direction is read by a function a
  constructor of priority 101 registers with `std::atexit()`, before GAOL
  initializes itself in the initialization of the static objects, so that it
  runs after GAOL's automatic cleanup: the priorities of constructors are those
  of ELF, and GAOL is linked statically, as the CMake build makes it.
- **`static_initialization`:** about 40 operations computed before `main()`, in
  the initialization of a static object of the program, have to give the
  intervals they give in `main()`, bit for bit: the constants of intervals
  (`pi()`, `one()`, `universe()`...), products and quotients, which use the
  masks of the SSE2 intervals, the elementary functions, the trigonometric ones
  using π, and the reader of strings. The static library of the CMake build is
  initialized after the program, except with MinGW-w64, and GAOL's constants
  were computed by its dynamic initialization: 25 of the 45 checks failed with
  the SSE2 intervals, 24 with the others, `pi()` giving [-0, 0] (GAOL v5). With
  MinGW-w64, which initializes GAOL's files first, the test passes with or
  without the fix. `pi_dn`, `pi_up` and
  the other doubles of `gaol/gaol_port.h`, now written in decimal, have to be
  those their unions write in bits.
- **`numbers`:** `textToInterval("0.1")` has to be the tightest interval
  enclosing the number read, and the number itself when it is a double. The
  constants have to be the tightest enclosures of π, 2π and π/2. No
  `const char*` nor `nullptr` may convert to an interval, and `interval(0)` has
  to be `[0, 0]`: the constructor from one string, which made it ambiguous, is
  gone (GAOL v5). The literals of IEEE 1788-2015
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
  numbers the format of the agreeing digits stands for. The text of a point
  interval, written with 1 to 25 digits in each of these formats, has to be
  read back as an interval enclosing it: the literal `[a]` of IEEE 1788-2015
  is written for a number that is the point itself, and read back as the
  point (`[4]`, and `[0]` for a zero, whatever the signs of its bounds), and
  the two bounds `[l, r]` otherwise, neither of them the point
  (`[0.1, 0.1000000000000001]` for `interval(0.1)`); the largest doubles and
  the subnormals, written with all their digits (309 digits, 1074 after the
  point), are written `[a]` and read back as the point where the C++ library
  writes them exactly. GAOL wrote `<a, b>` for every point interval, and the
  reader refused most of them (GAOL v5). In hexadecimal, the
  bounds have to be written in the hexadecimal-significand form of
  IEEE 1788-2015 (13.4.1) and read back bit for bit, which is the recovery
  requirement of 13.4: over random intervals, and over the empty set, the
  infinite bounds, the signed zeros, the subnormals, the largest doubles and
  point intervals.
  `operator<<` has to leave the precision of the stream as it was, and
  `std::setw` to pad the whole interval, adjusted to the right or to the left.
  The width and center formats have to write the `midpoint()` and the `rad()`
  of IEEE 1788-2015: with every precision and flag, over the special values,
  the subnormals, the largest doubles, unbounded intervals and random
  intervals, the radius written has to be `rad()` rounded upward, less than
  one unit of its last digit above it, so that midpoint plus or minus it
  contains the interval, and never 0 for an interval that is not a point,
  which is written as its midpoint alone; the midpoint has to be written
  rounded to nearest, at most half a unit of its last digit away. GAOL wrote
  (l+r)/2 and (r-l)/2 rounded to nearest, which do not contain the interval,
  a radius 0 for [0, 5·10^-324] and `inf` for the midpoint of [10^308,
  1.7·10^308]. The empty set has to be `[empty]` in the five formats, and in
  the conversion to a string: two of them wrote `empty` (GAOL v5).
  `while (in >> x)` has to stop at the end of the input with `failbit` set,
  nothing thrown and the interval unchanged, and a line that is no interval,
  refused at its end (`[1, 2`) or as the reader reads it (`<3, 4>`), has to set
  `failbit` and throw `input_format_error`, on a stream throwing on `failbit`
  too (GAOL v5). Blank lines have to be skipped, as the blanks before a number
  are, with or without `std::noskipws`: a file ending with an empty line, an
  empty line or a line of blanks between two intervals, and `in >> d >> x` over
  the two lines `1.5` and `[1, 2]` have to read what is there, and to end at the
  end of the input as `while (in >> d)` does over numbers, on a stream throwing
  on nothing, on `failbit`, or on `badbit` and `failbit`; an interval written on
  two lines is still no interval (GAOL v5). The skipping must not replace the
  checks that open any extraction: a stream that is not good, one without buffer
  included, is not read and keeps all its text, and the stream tied to the input
  is flushed before the first character is read, as for a number, so that a
  prompt shows before the user types (`std::ws` alone does none of this).
  Numbers with a million zeros after their point and an exponent of 7 digits
  (`0.00…01e1000001`, and in hexadecimal and in the uncertain form) have to be
  read exactly: the exponent was cut at 100000 (GAOL v5). Under a locale
  writing a decimal comma, where one is installed
  (`fr_FR.UTF-8`, `de_DE.UTF-8`, `French_France.1252`...), numbers have to be
  read as in the C locale, and `exact_string()` has to write points and read
  back bit for bit: the reading never ended there, and the test, which ctest
  would otherwise let run with no limit, fails after 5 minutes should it hang
  again (GAOL v5). Numbers of 5000 to 20000 characters, in decimal, in
  hexadecimal and in the uncertain form, have to be read as the tightest
  enclosures, known without reading them (`1.5` followed by zeros is 1.5, and
  followed by zeros and a 1 is between 1.5 and the next double...), under the C
  locale and under a locale writing a decimal comma; and under the comma
  locale, the number of 20000 characters has to take at most 10 times the
  time it takes under the C locale, and 50 ms more: the reader read the text
  again for each of its 125 comparisons or so, in a time quadratic in its
  length, and took 50 times as long (GAOL v5).
  The Ubuntu runners of the continuous integration have no
  such locale: its Linux jobs generate `fr_FR.UTF-8` for the test, and fail if
  it did not check under it (see
  [Continuous integration](continuous-integration.md)).
  With flush-to-zero, denormals-are-zero or both set in MXCSR
  (x86 only, and where the processor honours them), the numbers from 0 to the
  least normal double, in decimal and in hexadecimal, alone, in intervals and
  in the uncertain form, have to be read as the tightest intervals enclosing
  them, and the doubles among them, 0 included, as themselves: the reader
  compared each number with the doubles around it as doubles, and
  denormals-are-zero reads a subnormal as 0, so that 1e-310 was read as the
  interval from the greatest subnormal to the least normal double, which does
  not enclose it, and 0 as the greatest subnormal (GAOL v5). Intervals of
  subnormals (`[0, 5e-324]`, `[5e-324]`, `[-5e-324, 0]`...) written with 16
  digits under denormals-are-zero have to be read back, the mode restored, as
  intervals enclosing them: `operator<<` compares their bounds by their bits,
  and does not write them `[0]` (GAOL v5). A subnormal bound is then written
  by the C library, which compares it with 0 too where it uses gdtoa (FreeBSD,
  macOS), and writes 0: nothing is checked where the C library does not write
  the bounds of the test under the mode as it does without it.
- **`other_functions`:** midpoints (of subnormal bounds, and of `intervalf`
  where a developer of GAOL compiles the float intervals, `gaol/gaol_config.h`), widths, radii (`rad()`, `mid_rad()`), magnitudes, mignitudes, Hausdorff
  distances (of intervals with infinite bounds too, equal bounds being at
  distance 0), `nb_fp_numbers()` (across the two zeros), splitting, integer parts, the comparisons of IEEE 1788-2015
  (`precedes`, `interior`, `subset`, `equal`, `disjoint`, from Tables 10.3 and
  10.4, on intervals of zero, infinite and small bounds and the empty set), and
  the relational functions (`sqrt_rel`, `div_rel`...): `acos_rel`, `asin_rel`
  and `atan_rel` have to keep their value within 6 doubles from 1 to 2^50,
  and decide an interval of a single double beyond 2^53. `less`,
  `strictly_less`, `is_entire` and `is_common_interval` have to give the values
  of Tables 10.3 and 10.4, and `==` and `!=` must not compile on intervals,
  `certainly_neq()` and the possibly relations being gone. Each name of `gaol_ieee1788` has to be the operation
  of the standard it names, which a wrong translation would not show at
  compilation: the eight comparisons against the bounds of Table 10.3 and the
  empty cases of Table 10.4, over 20 000 pairs; `inf`, `sup` and the numeric
  functions against Table 10.2; the reverse functions with the arguments in
  the order of the standard, `mulRev(b, c, x)` being `div_rel(c, b, x)`; `pow`
  against the pow of Table 9.1, which GAOL's own `pow` is not for a negative
  base, and at integer exponents beyond the ints, where it has to give the
  tightest bounds and GAOL's own `pow` gives [-oo, +oo] (GAOL v5).
- **`ieee1788`:** `gaol_ieee1788` as a program uses it, under
  `using namespace gaol_ieee1788;` alone. Every name of the standard it
  provides is called unqualified, which compiles only if none of them is
  ambiguous with a function of `gaol_core`. `pow` has to be the standard's with an interval, an
  `int` or a `double` exponent: on a negative base, at `[0]`, and at infinite,
  NaN and beyond-the-ints exponents; `pown` and `gaol::pow` the integer power;
  the bounds of `gaol_ieee1788::pow` and of `gaol::pow` on 89 boxes, each
  reaching a branch of the pow of Table 9.1, which the two share, or of what
  `gaol::pow` adds to it (the integer power, [-oo, +oo] beyond the ints):
  they have to be bit for bit those the two functions gave when each had its
  own copy of the pow, each an enclosure of the exact power within one double
  of the tightest bounds, and the same with the exponent given as a double;
  and the expressions `pow(e1, e2)` and `pown(e, n)`, evaluated, the
  standard's too. GAOL's functions on intervals and on an interval and a
  number; the functions of C on numbers, by `static_assert`.
  `intervalToExact()` has to be `exact_string()`, read back bit for bit, and
  to leave the global output format alone; the check of it by a second
  thread writing intervals meanwhile is commented out, the tests running no
  thread. `textToInterval(intervalToText(x))` has to contain x for a point
  interval x, `interval(0.1)` first: it was the empty set (GAOL v5); and to be
  x itself for a text `[a]`.
  `intervalToText(x)` has to be an interval literal of the standard, `[l, r]`,
  `[a]` (`[4]`, `[0]`) or `[empty]`, whatever the global format, the precision
  of the intervals and the locale: in each of the five formats, with 1, 3, 8,
  16, 17 and 30 digits, and under a locale writing a decimal comma where one
  is installed, where `operator<<` writes `1.5 (+/- 0.5)` and `[0,25, 0,5]`; a
  grammar of the literals of Tables 9.5 and 12.2 checks the text, and
  `textToInterval` has to read it back as an interval containing x (GAOL v5).
  Under that locale, the text `operator<<` writes for a point (`-2.5`, `12.5`,
  `0`, minus the least subnormal..., with 16 digits, and in the fixed format
  with no digit and the showpoint flag, and with 1074 digits) has to be refused
  by `gaol::textToInterval`, or read back as an interval containing the point,
  and as the point itself when it is one number: the literal `[a]` wrote
  `[-2,5]`, read as `[-2, 5]`, `[12,5]`, read as the empty set, and `[0,]`,
  read as `[0, +oo]` (GAOL v5).
  `textToInterval` has to read each name of
  Tables 9.1 and 10.5 as the function of that name, in any case of letters,
  `pow([-4,-1],2)` being the empty set, and to give the empty set for the
  names of GAOL alone (`nth_root`, `cbrt`, `log1p`...) and the calls that are
  wrong, where `gaol::textToInterval` reads the names of GAOL (GAOL v5).
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
  undefined behaviour comes back. `sin` is also checked, in the four
  rounding directions, against the values of the upstream sources of
  CORE-MATH compiled unchanged, at 19 arguments that take its accurate path in
  the upward rounding, which computes with the 128-bit integer GAOL ports
  (see [3rd/README.md](../3rd/README.md)): comparing CORE-MATH with itself,
  the rest of the test cannot see a fault of that port, which the jobs
  computing with the two 64-bit halves would then find here. First of all,
  the test checks the `fma()` and `round()` of the C library, which CORE-MATH
  and GAOL call (`fma()` wherever the compiler has no fused multiply-add
  instruction, as GCC for Windows): `fma()` correctly rounded in the four
  rounding directions at six triples, three error-free products
  `fma(a, b, -a*b)` among them and `x + x*2^-54` as `cr_tan()` computes it, and
  `round()` the same in every direction at ±0x1.fffffffffffffp-2, ±0.5, ±2.5
  and 2<sup>52</sup> − 1/2, against values computed with exact rational
  arithmetic. The `fma()` and `round()` of mingw-w64's own math library on
  x64, which `gaol/gaol_config.h` refuses there, fail 21 and 6 of these checks
  under wine (GAOL v5). `cbrt` is checked in the four rounding directions, and
  `nth_root(x, 3)` and `rootn` have to be the tightest enclosures, at the
  seven arguments `cbrt.c` rounds apart (its `wlist`), scaled by powers of 8
  and on both signs, against mpmath: with mingw-w64 on x86-64, its
  `get_rounding_mode()` gave `FE_UPWARD` where 0 to 3 were expected, and the
  upper bound was below the cube root (GAOL v5).
- **`expressions`:** `textToInterval("...")` lexes the string, parses it into
  the tree of `gaol/gaol_expression.h` and evaluates that tree, so this test
  goes through every node of the tree and every way the string can be wrong:
  the numbers in every form the lexer takes (decimal, exponent, hexadecimal, the
  bounds given apart), the operators and the functions alone and nested, and
  the strings the parser has to refuse with an exception rather than an
  interval. Every string being an expression, the intervals have to be read
  wherever a number may stand, after a number and in a bound (`1+[1,2]`,
  `[1,2]+1+[1,2]`, `[cos([0,1]), 2]`), which the two grammars of GAOL refused,
  a newline as a space, and an error anywhere to stop the reading
  (`[nth_root(8,1.5)]+[1,2]` gave `[-oo, +oo]`); `pow(2,-1050)` has to be
  `gaol::pow`, whose power the reader computed as an inverse that overflowed
  (GAOL v5). The names GAOL v5 adds — `exp2`, `log2`, `cbrt`, `sign` and
  `trunc` — are read alone and in bounds, in any case of letters, the lexer
  taking the longest name so that `exp2` is not read as `exp` followed by 2.
  A negative exponent of `nth_root` has to give the root of C++,
  1/x<sup>1/|q|</sup>, alone and in a bound: the reader converted it to an
  unsigned int (GAOL v5). The expressions built in C++ go through every node
  too, `pow(e, 3)` and `/=` included, which did not link, and have to be
  printed as written, `x/(y*z)` with its `/` and `(-2)^2` with its parentheses (GAOL v5).
  Every function of GAOL has to be read under its name, those the reader did
  not know included (`exp10`, `hypot`, `sinpi`, `fma`...), the names of IEEE
  1788-2015 alone (`pown`, `rootn`, `recip`...) and the calls with a wrong
  number of arguments have to be refused, and `gaol::textToInterval` has to
  read the names of GAOL, and with two strings to take the left bound of the
  first and the right bound of the second. What the exceptions of GAOL say
  has to be their explanation, or `gaol_exception` where there is none: the
  `what()` of a `gaol_exception`, an `input_format_error`, an
  `unavailable_feature_error` and an `invalid_action_error` built with a known
  explanation, and of what the reader, `operator>>` and `nb_fp_numbers()`
  throw, read through a `std::exception`; and `operator<<` has to write the
  explanation once. `what()` was `std::exception`, which a handler of
  `std::exception` printed and an exception that nothing catches ended the
  program with, and `operator<<` wrote it next to the explanation (GAOL v5).
  The reading of
  strings by four threads at once, where the reader, whose state was global,
  crashed before its lexer became reentrant and its parser pure, is commented
  out: the tests run no thread
  (GAOL v5). Each value is compared with the same computation written in C++,
  which the other tests check against the exact results: what is tested here is
  the lexer, the parser and the evaluation, not the operations. An empty
  expression of static storage, destroyed after `main()` has called
  `gaol::cleanup()`, has to stay valid: `gaol::cleanup()` deleted the node it
  refers to, and its destructor wrote into freed memory, which the sanitizers
  of the continuous integration report (GAOL v5). The references of the empty
  expressions to that node, `the_null_expr`, must not be counted
  (`expr_node::references()` does not change as they are built, copied,
  assigned and extended): the count, changed by several threads at once, came
  down to 0 and the node was deleted twice (GAOL v5).
- **`u128`:** the accurate phases of CORE-MATH's `log`, `sin`, `cos`, `tan`,
  `atan2` and `pow`, and of `log10` and seven functions of Table 10.5, compute
  with a 128-bit unsigned integer, which Visual C++
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

- **`debugging`:** GAOL's headers compiled with `GAOL_DEBUGGING`, which the
  Debug builds define (`CMAKE_BUILD_TYPE=Debug`, `configure --enable-debug`,
  `meson setup --buildtype=debug`), whatever the build: an expression is built
  and evaluated, its nodes calling `GAOL_DEBUG`, which has to run its command
  at the level given to `gaol::init()` and not above. `gaol/gaol_expression.h`
  did not compile with `GAOL_DEBUGGING`: its `GAOL_DEBUG` wrote on `std::cout`,
  which no header included (GAOL v5).
- **`version_file`** (CMake only: a script of CMake, not a program): the
  reading of `VERSION.txt` by `CMakeLists.txt`, `gaol_read_version()` of
  `cmake/gaol_version.cmake`, has to ignore a UTF-8 byte order mark at the
  start of the file, which some editors of Windows write, the line ends of
  Windows (CR LF), and the blanks and empty lines around the version (the six
  blanks of ASCII, not those of Unicode), to refuse anything but three numbers
  without leading zeros, and to give the first bytes of a refused file in
  hexadecimal in its message. It refused a file starting with a byte order
  mark as `VERSION.txt holds "5.0.0"`, the mark being a character that is not
  seen; configure and meson did the same. A file of UTF-16 characters
  (`tests/version_file/`, which CMake could not write: it holds NUL bytes) has
  to be refused with all of its message, a NUL byte having cut the text
  quoted, and a NUL byte after `5.0.0` has to make the file refused, whether
  `file(READ)` cuts the text at the NUL byte (CMake 3.14.7 and 3.16.3) or keeps
  it and a regular expression stops there (3.28.3, 4.4.3); the message has to
  end with `the file is UTF-16: save it as UTF-8 or ASCII` for a file that
  starts with the byte order mark of UTF-16 (FF FE, FE FF), and with
  `the file holds a NUL byte as UTF-16 does: save it as UTF-8 or ASCII` for
  one that holds a NUL byte without it, and with neither for the others. A
  file holding a mark alone, or nothing, has to be refused with its message
  too: `string(REGEX MATCH)` stops with an error on an empty match in CMake
  before 4.1 (3.14.7, 3.16.3, 3.28.3, 3.30.0 and 4.0.0 checked), which is why
  `gaol_read_version()` runs no regular expression on an empty text; without
  that, the test stops with that error on these checks with every CMake
  before 4.1. The files are written by the script, which expects the
  versions it writes, not the ones the code reads back. configure and meson,
  which read the file their own way, and autoconf, which reads it for
  `configure --version`, have no such test: `.github/scripts/version-file.sh`
  runs them on a copy of the sources, in the continuous integration, with
  the same cases and a few more (configure dropped a CR within the version
  and read a file holding NUL bytes, meson stripped the blanks of Unicode,
  autoconf dropped the blanks within the version and let NUL bytes through,
  took `5.0.0)` for 5.0.0 and stopped on a bracket) (GAOL v5).
- **`refused_finite_math_only` and `refused_fast_math`:** compile tests, made
  by the CMake build where the compiler is GCC or Clang. `tests/refused_options.cpp`,
  a program including `<gaol/gaol>`, is compiled with `-ffinite-math-only` and
  with `-ffast-math`, put after the flags of interval arithmetic, and the
  compilation has to fail with the message of `gaol/gaol_config.h`, which the
  test looks for in the output of the build. With `-ffinite-math-only` the
  compiler takes NaN and infinities never to occur, in the inline functions of
  the headers too, and the empty interval, whose bounds are NaN, is no longer
  told empty: `([1, 2] & [3, 4]).is_empty()` was false with GCC 9 and Clang 18.
  The header did not refuse `-ffinite-math-only`, nor `-Ofast` or `-ffast-math`
  followed by `-frounding-math` with Clang, which leave `__FAST_MATH__`
  undefined and `__FINITE_MATH_ONLY__` at 1 (GAOL v5). The autotools and meson
  builds have no such test, the header being the same; `tests/fp_strict` is
  the check of Visual C++ without `/fp:strict`.
- **`cpack_stale_configure`** (CMake build, on a Unix system that builds for
  itself, where the tree has a `configure`): a script, not a program
  (`tests/cpack_stale_configure.cmake`). CPack puts `configure` in the archive
  of the sources as it is committed, and `configure --version` gives the version
  it was generated for: after a change of `VERSION.txt` without a new
  generation of `configure`, the archive of the new version holds a
  `configure` that says the old one. The script configures copies of the tree
  with the compiler that built GAOL, and CMake has to warn of it, naming the
  two versions, whether `configure` has the line ends of Unix or of Windows,
  and not otherwise: not when `configure` was generated for the version
  `VERSION.txt` holds, not when it has no line `PACKAGE_VERSION=`, and not
  when there is no `configure`, which CMake has to configure all the same. The
  version `configure` was generated for is asked of `configure --version`, not
  read as `CMakeLists.txt` reads it. The copies are made of symbolic links to
  the files of the tree, but for `CMakeLists.txt`, `VERSION.txt` and
  `configure`, which are copied; the test takes a few seconds on an ordinary
  machine, a configuration and four that reuse its checks (GAOL v5).
- **The unit tests of GAOL 4:** `arithmetic_operators` (`check/arithmetic.cpp`),
  `assignment`, `constants`, `constructor`, `float_functions`,
  `generic_functions`, `input_output`, `interval_functions`, `misc`,
  `non_arithmetic`, `relations`, `reverse_mappings` and `trigonometric` check
  the results of the operations on chosen intervals, as Frédéric Goualard wrote
  them with CppUnit in `check/`. They are in `tests/` now and run with the
  checks of `unit_tests.h` rather than CppUnit, which no build needs any more:
  each suite keeps its class and its tests, a `TEST_...` counts a check named
  after its test and describes a failure by its line, its expression and the
  values compared, and a failure no longer ends its test. `reverse_mappings`
  draws its random intervals from a seed of its own rather than from the
  process identifier (`srand48(getpid())`, which Visual C++ does not have), so
  that its checks are the same at each run. `intervalf` and `interval2f` test
  the intervals of floats where a developer of GAOL compiles them
  (`GAOL_FLOAT_INTERVALS`, see `gaol/gaol_config.h`), and are skipped otherwise
  (exit status 77). `check/fpu.cpp`, an empty test, and `check/essai.cpp`,
  which printed an interval, are gone, as is `check/performances.cpp`, the
  benchmark of GAOL 4, which `make perf` replaces (see
  [Building GAOL](building.md#tests-examples-performance-and-the-parser)).
- **`nodiscard_discard_cxx11`, `nodiscard_discard_cxx14` and
  `nodiscard_discard_cxx17`, `nodiscard_used_cxx11`, `nodiscard_used_cxx14` and
  `nodiscard_used_cxx17`:** compile tests, made by the CMake build where the
  compiler is GCC, Clang, or Visual C++ 2019 16.4 or later.
  `tests/nodiscard.cpp`, a program including `<gaol/gaol>`, is compiled in
  each of the three standards with `-Werror=unused-result` and
  `-Werror=attributes` (`/we4834`, `/we5030` and `/we5051` with Visual C++):
  throwing away the result of `sqrt(x)` has to fail the compilation with the
  warning of the compiler, which the test looks for in the output of the
  build, and using it has to compile, GAOL's headers throwing no result away,
  nor carrying an attribute the compiler ignores where it is written.
  `GAOL_NODISCARD` was empty before C++17, so that a CMake project with GCC 9,
  which compiles in C++14 unless it says otherwise, got no warning for
  `sqrt(x);`; it is now `[[nodiscard]]` there with GCC 7 and later and Visual
  C++ 2019 16.4 and later, and the attribute of Clang with Clang (see
  [Using GAOL](using.md#a-result-thrown-away)). The autotools and meson
  builds have no such test, the header being the same.

The three builds compile them with `WITH_TESTS` (CMake), `--with-tests`
(configure) and `with-tests` (meson), all off by default, and run them with
`make test` (`meson test`, or `ninja test`); `make check` (`ninja check`) runs
the examples of `examples/` too, where they are built (see
[Building GAOL](building.md#tests-examples-performance-and-the-parser)). The
continuous integration runs `make test` in every job. `tests/find_package`
builds some of the same tests with an installed GAOL, and
`.github/scripts/tests.sh` with a GAOL installed by configure or meson.

`tests/performance.cpp` (`gaol_performance`) measures the time per operation of
GAOL's arithmetic and elementary functions, and of the same operations on
doubles. It is not a test: the continuous integration prints its table in the
summary of the jobs.

What they show of GAOL, beyond the fixes of
[What differs from GAOL](differences.md):

- `sin`, `cos` and `tan` tell the pieces of their argument where they are
  monotonic by dividing it by an interval enclosing π, and, where the quotients
  cannot tell, next to an extremum or a pole and at the large magnitudes, from
  the signs of their derivative at the bounds, which CORE-MATH gives exactly:
  their bounds are within one double of the tightest at every magnitude. (See
  [Accuracy of the operations](accuracy.md).)
