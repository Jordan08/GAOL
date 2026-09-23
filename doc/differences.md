<!-- Copyright (c) 2026 ENSTA, France
     Created 2026-09-20 by Jordan NININ -->
# What differs from GAOL

Part of the documentation of [GAOL v5](../README.md#documentation).

What GAOL v5, written by Jordan Ninin, changes from the GAOL 4.2.2 of
Frédéric Goualard it continues. Each change is a commit of its own, and says
where it comes from.

- **Every elementary function is bounded with
  [CORE-MATH](https://core-math.gitlabpages.inria.fr/)**, on every architecture
  and with every compiler, and it is the only mathematical library GAOL uses
  (`3rd/math-core`, see [3rd/README.md](../3rd/README.md)):
  - The other mathematical libraries GAOL could be built with are gone, with
    their sources, their headers and the scripts that installed them, and so
    are the options of the builds that chose one: there is nothing to choose,
    to find, to build apart or to link along with GAOL, whose library holds
    the functions.
  - The bounds are **the tightest ones** rather than one double beyond.
    CORE-MATH is correctly rounded in the rounding direction in effect, so GAOL
    takes the value at a bound, computed in the upward rounding it keeps, as
    the upper bound, and the double below it as the lower one: GAOL 4 took
    values correctly rounded to nearest only, so each bound had to be moved one
    double outward, and the direction had to be set to nearest and back around
    every call. On an Intel i7-1185G7 with Clang 18, the bounds of `log` took
    30 ns rather than 62 (see [Accuracy of the operations](accuracy.md)).
  - The accurate phases of `log`, `sin`, `cos`, `tan`, `atan2` and `pow`
    compute with a 128-bit integer, which Visual C++ has on no architecture and
    GCC has on no 32-bit target: `gaol/gaol_u128.h` gives them the type of the
    compiler where it has one and a structure of two 64-bit halves where it has
    none, and the sources of CORE-MATH call its functions there. The two paths
    were compared with the pristine upstream sources over about 40 million
    arguments in the four rounding directions, and give the same bits;
    `tests/u128.cpp` compares the halves with `unsigned __int128`, and a job of
    the continuous integration builds the whole of GAOL with the halves forced.
  - mingw-w64 is no longer refused for its version. Both reasons are gone: the
    hyperbolic functions of its math library, which GAOL no longer uses, and
    the cost of its `fesetround()`, which GAOL no longer calls for its
    elementary functions. MinGW-w64 GCC 12 to 15 are built and tested again
    on 32-bit x86, and 14 and 15 on x86-64; only those whose `<fenv.h>` answers
    `fegetround()` from a state of its own are still refused, CORE-MATH's
    functions reading it to know the direction GAOL sets by writing the
    registers (`gaol/gaol_config.h`).
  - `tests/core_math.cpp` checks the bounds against CORE-MATH called in the
    downward and the upward rounding, `tests/expressions.cpp` the intervals
    read from a string, and `-DGAOL_COVERAGE=ON` writes the coverage of the
    tests in [coverage/README.md](../coverage/README.md) (87 % of the lines of
    GAOL).
  - **`exp2`, `exp10`, `log2` and `log10`**, which IEEE 1788-2015 requires
    among the forward elementary functions (Table 9.1) and which GAOL did not
    provide, are bounded with CORE-MATH's, the tightest bounds. The value that
    is a double is kept as a bound rather than moved: 2<sup>3</sup> = 8,
    10<sup>22</sup>, log<sub>2</sub>(1/4) = −2 and log<sub>10</sub>(100) = 2
    are exact. Only `log10` needed the 128-bit integer of `gaol/gaol_u128.h`,
    its `dint.h` being that of `log` but for a constant.
  - **Fifteen functions of Table 10.5**, which IEEE 1788-2015 recommends:
    `expm1`, `exp2m1`, `exp10m1`, `log1p` (logp1), `log2p1`, `log10p1`,
    `hypot`, `rsqrt` (rSqrt), `sinpi`, `cospi`, `tanpi`, `asinpi`, `acospi`,
    `atanpi` and `atan2pi`, the tightest bounds. `sinpi`, `cospi` and `tanpi`
    find their extrema and poles exactly, at the multiples of 1/2, which are
    doubles: `sinpi([1e17])` is 0, where `sin(pi*x)` gave [-1, 1], the width
    of pi times 10^17 being more than a period. Seven of them compute with
    CORE-MATH's 128-bit integer, which was ported to `gaol/gaol_u128.h` first
    (see [3rd/README.md](../3rd/README.md)). Only `compoundm1` is missing,
    CORE-MATH having no binary64 version of it.
  - **`fma`, `cancelMinus` and `cancelPlus`**, which IEEE 1788-2015 requires the
    tightest, and **`less`, `strictLess`, `isEntire` and `isCommonInterval`**.
    Rounding the lower bound of `fma` as -fma(-a, b, -c) came one double above
    the exact bound under GCC, which folded the negations into one
    instruction rounded upward: the result goes through `rnd_keep()` first.
  - **`gaol_ieee1788`**: the operations of the standard GAOL provides, under
    the names and in the argument order of the standard, with the type
    `interval`, so that `using namespace gaol_ieee1788;` is enough to use them
    (see [Using GAOL](using.md#the-names-of-ieee-1788-2015)). Its `pow` is
    the standard's, `pow(x, 2)` being `pow(x, [2])` and the integer power
    `pown(x, 2)`, and an integer exponent beyond the ints gives the pow of
    CORE-MATH at the bounds of x, where the `pow` of `gaol` gives [-oo, +oo].
  - **`cospi.c` of CORE-MATH shifted a signed integer** out of its range,
    undefined behaviour that the tests run with UBSan reported; it shifts it
    unsigned now, as `sinpi.c` and `tanpi.c` do (see
    [3rd/README.md](../3rd/README.md)).
  - **The hexadecimal output is an interval literal.** Written with
    `interval_format::hexa` and read again, an interval now gives the same
    bounds bit for bit, which is the recovery requirement of IEEE 1788-2015
    (13.4). GAOL wrote the sixteen hexadecimal digits of each double
    (`[3fb999999999999a, ...]`), which is no interval literal at all and which
    the parser refused; the note of 13.4.1 gives that very form as the one
    failing its readability test. The bounds are written in the
    hexadecimal-significand form instead (`[0x1.999999999999ap-4, ...]`), which
    the lexer already read. The decimal formats, and the default one, are
    unchanged. `exact_string(I)` gives that text without the global output
    format, and `gaol_ieee1788::intervalToExact()` is it: switching the format
    to hexa and back, it showed hexa to the other threads meanwhile.
  - **The reader of strings takes the new names.** `interval("...")` reads
    `exp2`, `log2`, `cbrt`, `sign` and `trunc` besides the functions GAOL
    already had, in the direct grammar and in the tree of
    `gaol/gaol_expression.h`; `cbrt(x)` is `nth_root(x, 3)`, as `sqrt(x)` is
    `nth_root(x, 2)`, so it needs no node of its own, and the four others have
    one each, which `gaol::exp2(expression)` and its companions build from C++
    too. Read from a string, they gave a syntax error before.
  - **`nth_root(x, 3)` is CORE-MATH's `cbrt`**, at the magnitudes of the bounds,
    the root of a negative number being the opposite of the root of its
    magnitude: the tightest bounds, and the exact value where the cube root is
    a double, which cubing the value tells. It took 50 ns rather than 189 with
    the search by bisection the other roots use (Intel i7-1185G7, Clang 18.1).
  - **`nth_root(x, q)` takes a negative q**, which IEEE 1788-2015 recommends
    (rootn over ℤ∖{0}, Table 10.5): x<sup>1/q</sup> is 1/x<sup>1/|q|</sup>,
    whose domain is ℝ∖{0} for an odd q and (0, +∞) for an even one. The
    reader of strings takes it too: it converted the exponent to an unsigned
    int, and `interval("nth_root(16, -2)")` was the 4294967294-th root of 16,
    `[1.000000000645543, 1.000000000645544]`, rather than `[0.25]`.
  - **`sign`, `trunc`, `round_ties_to_even` and `round_ties_to_away`**, the
    integer functions IEEE 1788-2015 requires beside `ceil` and `floor`
    (Table 9.1), which GAOL did not provide. Each is non-decreasing, so the
    bounds of the result are its values at the bounds, and each is exact. They
    take nothing from CORE-MATH, which has none of them and needs none: their
    results are integers, with no rounding to get right. What they do need is
    to ignore the rounding direction, which `std::nearbyint` and `std::rint`
    read: `round_ties_to_even` therefore reads the bits
    (`gaol/gaol_roundeven.h`, which also gives CORE-MATH the `roundeven()` the
    math library of Windows has not).

- **From the patch IBEX applies to GAOL** (Gilles Chabert,
  `interval_lib_wrapper/gaol/3rd/gaol-4.2.3alpha0.all.all.patch` of
  [IBEX](https://github.com/ibex-team/ibex-lib)):
  - The FPU is initialised with `fesetenv(FE_DFL_ENV)` and `round_upward()`,
    rather than by writing an x87 control word into `fenv_t`, which on ARM64 left
    the rounding to nearest and crashed an ARM64 Mac.
  - `interval::midpoint()` and `operator<<` set the rounding direction back
    upward, which they did not when the rounding direction is not preserved.
  - The macro `opposite()` is renamed `gaol_opposite()`.
- **For Visual C++, MinGW and the systems GAOL's configure does not know**,
  following the fork of GAOL by [Fabrice Le Bars](https://github.com/lebarsfa/GAOL):
  - `gaol/gaol_config_msvc.h`, `gaol/gaol_config_mingw.h` and
    `gaol/gaol_version_msvc.h` include the generated configuration.
  - `get_fpu_cw()` and `reset_fpu_cw()` save the rounding direction with
    `<fenv.h>` where the control word of `fenv_t` is not known.
  - Visual C++ gets the `<fenv.h>` version of `get_inexact()` and
    `clear_inexact()`.
- **The rounding direction** is set upward by each operation when it is not,
  in every build. Built with CMake, GAOL set it once, in `gaol::init()`, and
  computed in the direction the calling code left: rounding to nearest,
  downward or toward zero, 30 operations gave other bounds, and sums and
  products did not enclose their exact values (3199 of the 13825 checks of
  `rounding_direction` failed with GCC 9.4 on x86-64). Built with autotools or
  meson, GAOL restored the rounding direction after each operation, but only
  the x87 control word on x86-64, where GAOL computes with SSE (43 operations
  left the SSE instructions rounding to nearest or upward, whatever direction
  they found), and without the rounding bits of the FPCR register on arm64.
  `GAOL_PRESERVE_ROUNDING`, now an option, saves and restores the direction with
  `fegetround()` and `fesetround()` and the SSE register. Each value computed
  before the rounding direction changes is then written to a `volatile`
  variable: GCC does not implement `#pragma STDC FENV_ACCESS`, and moved the
  computation of the midpoint after `fesetround()`
  ([GCC bug 34678](https://gcc.gnu.org/bugzilla/show_bug.cgi?id=34678)).
  Whether the direction is upward is shown by an addition, 1 + 2^-60 with
  2^-60 read from `volatile` memory, wherever doubles are computed in double
  precision.
  Reading it instead, with the SSE register and `fegetround()`, made `x + y`
  take 800 ns rather than 2.8 ns under Rosetta 2, 82 to 98 ns rather than 2.6
  to 4 ns with 32-bit Visual C++, and 48 ns rather than 6.4 ns with 32-bit
  MinGW-w64 15.2; and Clang 18 read MXCSR once for a whole loop that changed
  the rounding direction.
  - **Once per function of intervals.** The bounds of the elementary
    functions at doubles are computed by the functions of namespace `upward`
    (`gaol/gaol_double_op.h`), which take the direction to be upward already,
    as it is after the check of the function of intervals calling them; the
    functions of the same names outside it check it too, for the code calling
    them directly. `exp()` of an interval checked the direction three times,
    `sin()` and `cos()` up to four, the other elementary functions and
    `pow(x, y)` two or three: once now, the bounds being the same. `exp()`
    takes 5.7 % less time, `sin()` 3.6 %, `pow(x, y)` 3.4 % (Clang 18,
    i7-1185G7). `tan()`, the relational functions of the trigonometric
    functions and the negative integer powers still check it again within the
    operations of intervals they call.
  - **`tan()` keeps its bounds before setting the direction back**, with
    `GAOL_PRESERVE_ROUNDING`: it set it back first, and `GAOL_RND_KEEP()`,
    which writes the bounds to memory so that they are computed before, came
    too late.
  - **The bounds at doubles set the direction back**, with
    `GAOL_PRESERVE_ROUNDING`: `exp_dn()`, `sin_up()` and the thirty others
    of `gaol/gaol_double_op.h` saved the direction they found, but left it
    upward, which the compilers only showed by warning that `_save_state` was
    set but not used. `tests/rounding_direction.cpp` calls them.
- **The rounding direction is set on x86 processors by writing the control
  registers** of the x87 and SSE units (`fnstcw`/`fldcw`, `stmxcsr`/`ldmxcsr`)
  rather than through `fesetround()`, which cost 130 ns per call with
  mingw-w64 13, 50 ns with the C runtime of Visual C++ for x64 and 250 ns for
  x86, 8.5 ns with glibc. GAOL changed the direction four times for each
  elementary function of an interval (to nearest before the math library and
  upward after, for each bound), where the math library itself took about
  10 ns; it no longer changes it there, CORE-MATH computing upward. On the
  runners of the continuous integration, `exp()`, `log()`, `sin()` and `cos()`
  of an interval took 535 to 630 ns with MinGW-w64 and MSYS2, and take 48 to
  209 ns; 250 to 340 ns with Visual C++ for x64, and 45 to 160 ns; 1050 to
  1750 ns with Visual C++ for x86, and 150 to 650 ns. Both registers are set,
  as `fesetround()` sets them, and `fegetround()` reads the direction set;
  with Visual C++ for x64, MXCSR only, the x87 unit being unused there.
  Elsewhere (ARM, POWER, s390x, RISC-V), `fesetround()` still.
- **`hausdorff()`** returns the tightest upper bound of the distance. It computed
  `fabs(a - c)` in the rounding direction of the caller, below the exact
  distance when rounded upward with a < c.
- **`mid()`** returns the tightest interval enclosing the midpoint, computed as
  `midpoint()` computes it with the rounding outward: the half of the sum of the
  bounds, or the sum of their halves when the sum overflows (F. Goualard, *How
  do you compute the midpoint of an interval?*). It computed `a/2 + b/2` rounded
  upward, and for its lower bound subtracted the half of b rounded upward, which
  is not exact when |b| < 2^-1021: `mid([0, 2^-1074])` was `[2^-1074, 2^-1074]`,
  and 45150 of the 180901 intervals whose bounds are multiples of 2^-1074 from
  -300·2^-1074 to 300·2^-1074 did not enclose their midpoints;
  `mid([2^-1074, 2^-1074])` was `[2^-1074, 2^-1073]`, beyond the interval.
- **`intervalf::midpoint()`** is rounded to nearest, ties to even, as
  `interval::midpoint()` is and IEEE 1788-2015 has it. It computed
  `a + (b/2 - a/2)` rounded upward: the midpoint of `[1, 1 + 2^-23]` was
  `1 + 2^-23` rather than 1. The comments of both `midpoint()` gave a formula
  that neither computed.
- **The operators of an SSE2 interval with a double** (`+=`, `-=`, `*=`, `/=`,
  `%=`, and `+`, `-`, `*`, `/`, `%` of an interval and a double, which call
  them) compute as the FPU intervals do, by the sign of the double, rather than
  through `interval(d)` and the operators between intervals. On an Intel
  i7-1185G7 (GCC 9.4, CMake Release), `x += d` takes 3.9 ns rather than
  13.6 ns, `x *= d` 6.3 ns rather than 25.2 ns and `x /= d` 4.1 ns rather than
  18.0 ns, giving the same sets.
- **The constructors give the empty set** where their arguments are not an
  interval, as IBEX does: `interval(+oo)`, `interval(-oo)`,
  `interval(+oo, +oo)`, `interval(-oo, -oo)`, `interval(2, 1)` and bounds that
  are NaN are all the empty set, and assigning `+oo` to an interval empties it.
  IEEE 1788-2015 has no interval `[+oo, +oo]` nor `[-oo, -oo]`: its constructor
  fails there, and gives the empty set (10.5.8, 12.12.7). GAOL built them as
  intervals of one infinite point, whose midpoint, `realmax`, was outside them.
  The intervals with one infinite bound are unchanged, `[a, +oo]` among them,
  and their midpoint is still `realmax`. On an Intel i7-1185G7 (GCC 9.4, CMake
  Release), `interval(a, b)` takes 1.28 ns rather than 1.24 ns. Its bounds are
  set in a register and behind a branch: written to a pair in memory, their
  16-byte load stalls on the two 8-byte stores, and conditional moves make them
  depend on the comparison, which lengthens the loops accumulating intervals;
  either one costs several nanoseconds per construction.
- **A zero bound times an infinite bound** counts as 0 in the products of the
  FPU intervals, as in those of the SSE2 intervals: `[0, 1] * [1, +oo]` is
  `[0, +oo]`, `[-1, 0] * [1, +oo]` is `[-oo, 0]` and `[0, 0] * [1, +oo]` is
  `[0, 0]` in every build, with the operators of an interval and of a double.
  The FPU intervals gave NaN bounds there, the empty set. The operators of a
  double give the empty set for an infinite double, as `interval(d)` is empty:
  they no longer repair NaN bounds, and `x *= d` takes 3.6 ns rather than
  4.8 ns.
- **Square roots** are bounded whatever the rounding of the C library's `sqrt`,
  which Visual C++ for 32-bit x86 rounds to nearest in every rounding direction.
  Where `sqrt` rounds as it should, the results are unchanged.
- **Square roots are the tightest intervals**, as IEEE 1788-2015 requires of
  its basic operations (12.10.2): the lower bound is the square root rounded
  upward when its square is the argument, the double below it otherwise.
  GAOL computed it as x/sqrt(x) rounded downward, one double below the
  tightest bound for half of the doubles. The square roots of the C library
  are checked with a product rounded the other way rather than a division,
  and `sqrt()` takes the same time as before (9.0 ns on an Intel i7-1185G7,
  GCC 9.4).
- **Numbers are read exactly** (`gaol/gaol_interval_lexer.lpp`). GAOL read them
  with `strtod()` rounding downward and upward, and relied on the inexact flag.
  The C runtime of Windows and musl on 64-bit ARM processors round to nearest in
  every direction and raise no flag, so `interval("0.1")` did not enclose 1/10
  there. Each number is now compared exactly with the doubles around it.
- **Intervals are written in decimal rounded outward**, whatever the C
  library ([issue #3](https://github.com/Jordan08/GAOL/issues/3)).
  - **Before.** `operator<<` set the rounding direction downward, then upward,
    and let the C library write each bound (formats `bounds` and `agreeing`).
    The C runtime of Windows rounds the magnitude, and wrote -2/3 rounded
    downward `-0.6666`; musl on 64-bit ARM processors rounds to nearest
    whatever the direction, and wrote `1.235e+05` for the lower bound of
    123456.789. Read back, such a text did not enclose the interval written.
  - **Now.** The magnitude of each bound is written rounding to nearest,
    compared exactly with the bound, as the numbers read are, and its last
    digit is moved by one when it is on the wrong side. The general format is
    made from the scientific one by the rules of `%g`, glibc 2.31 writing
    999999.5 with six digits and `showpoint` `1.e+06`. The flags, the
    precision and the decimal point of the stream are kept.
  - **Same text with glibc**, which rounds as asked: 1077546 bounds written
    in the general, scientific and fixed formats with 1 to 18 digits were the
    same, character for character.
- **The format of the agreeing digits** (`interval_format::agreeing`) keeps
  the digits of each bound. GAOL dropped from both bounds what followed the
  last character of the left one that is not a zero: `[1.25, 1.2567]` was
  written `1.25~[, ]`, and `[100, 100.47]` `100.~[, ]`, which say nothing of
  the right bound. They are written `1.25~[0, 67]` and `100.~[0, 47]`; the
  zeros ending an exponent are no longer dropped, and the search for the
  common characters stops at the end of the shorter text.
- **Hyperbolic functions:** `sinh`, `cosh`, `tanh`, `asinh`, `acosh` and
  `atanh` are those of [CORE-MATH](https://core-math.gitlabpages.inria.fr/),
  correctly rounded upward, as the other elementary functions
  ([issue #1](https://github.com/Jordan08/GAOL/issues/1)): the bounds are
  the tightest, on every system.
  - **Before.** GAOL took them from the libm of the system, and moved their
    values one float outward. The libms of glibc 2.31, musl and MinGW-w64 are
    sometimes a float further, which gave bounds not enclosing the exact
    values; the `acosh` of MinGW-w64 11 to 13 is millions of doubles away next
    to 1, and its `asinh` NaN for large negative numbers.
  - **The sources.** The whole tree of CORE-MATH is vendored in
    `3rd/math-core` (MIT licence), compiled into GAOL's library under the
    names `gaol_cr_sinh()`... (`gaol/gaol_core_math.h`). The sources are kept as
    CORE-MATH wrote them, `gaol/core_math_port.h` being force-included into each
    of them by the compiler: [3rd/README.md](../3rd/README.md) lists what that
    header gives and the changes the vendored sources carry.
  - **Tests.** `tests/elementary.cpp` requires one double of every function,
    rather than 8.
  - At the overflow, the bounds were two floats wider than the
    tightest, against three assertions of GAOL's own check `trigonometric`
    (`make check`); they are the tightest there (see the exact values
    below).
- **Powers with a real exponent**, ported from the fix of Codac:
  - `pow(I, e)` for a floating-point `e` called `pow(I, int)`, which truncated
    the exponent: `pow([4], 0.5)` returned `[1]`. It now computes an integer `e`
    with `pow(I, int)` and any other `e` with `pow(I, J)`.
  - `pow(I, J)` computed the powers of the negative part of `I` on its
    magnitude: `pow([-4,-1], [0.5])` returned `[-1, 2]`. It is now hybrid: a
    degenerate integer exponent `[n]` always takes `pow(I, int)`, the `pown` of
    IEEE 1788, which keeps the negative part of `I` and gives `0^0 = 1`, and
    `[-oo, +oo]` for an `n` beyond the ints; any other exponent takes the `pow`
    of IEEE 1788, on the part of `I` in `[0, +oo]`, where `0^y` is `0` for
    `y > 0` and has no value for `y <= 0`: `pow([0], [0.5])` was
    `[0, 4.9e-324]`, and `pow([0], [-0.5])` was `[MAX, +oo]`. An exponent
    `[+oo]` or `[-oo]`, which contains no real number, gives the empty set.
- **`pow(I, J)` with finite bounds takes the tightest bounds at the corners
  of the box**, but for a lower bound one double below where the power there
  is a double ([issue #8](https://github.com/Jordan08/GAOL/issues/8)). x^y
  increases with y for x > 1 and decreases for x < 1, increases with x for
  y > 0 and decreases for y < 0: over a box of bases above 0 its extrema are
  at corners, which the places of the bounds about 1 and 0 give, and the pow
  of CORE-MATH, correctly rounded upward, is taken there; a base from 0 with
  exponents above 0 has 0 for lower bound.
  - **Before.** `exp(J*log(I))` multiplied the relative width of `log(I)`, a
    few 2^-52, by |y log x|: `pow([2], [1023.5])` was 1425 doubles below the
    exact value and 748 above. It is kept where a bound is infinite, or for
    a base from 0 with an exponent that is not above 0, whose limits it gives.
  - **Tightness.** Over the million powers of the benchmark of `doc/compare`,
    its intervals are no wider than those of libieeep1788, which computes
    each bound with MPFR, where `exp(J*log(I))` made them 5.9e-14 wider than
    the tightest relatively.
- **Integer powers are computed from exact products** (issue #7): `pow(x, n)`,
  for an `int` or an `unsigned` n, gives the tightest bounds for n ≥ 3, where
  GAOL rounded each product of its binary exponentiation outward and was up to
  n + 1 doubles from them: 2, 3, 5, 6 and 8 doubles for n = 3 to 7, `pow([1.1], 3)` being two
  doubles wide.
  - **How.** Each product h·y is rounded to p, and `fma(h, y, -p)` gives its
    rest h·y − p exactly: the power is carried as h + l, l being the sum of
    the rests, and rounded outward once at the end. The upper bound keeps l
    rounded upward, the lower bound keeps −l rounded upward and leaves out the
    square of l, which would raise it: the bounds are proved, and the tightest
    unless the power is within n·2^-104 of a double, where they are one double
    beyond. The power of a double that is a double is exact.
  - **Where not.** Below 2^-968 the rest of a product is no double, and at the
    overflow neither is the product: the rounded products of before serve
    there, as for the square, which is the tightest already. Over 2.4 million
    powers of exponents 3, 4, 5, 8, 17, 100, −3 and −4 at every magnitude,
    the bounds were never wider than before, and narrower for 47 %.
  - **Time** (Intel i7-1185G7, GCC 9.4, x86_64 without FMA instructions
    compiled in, `fma()` being the one of glibc): `pow(x, 3)` takes 32 ns
    rather than 14.4 in the benchmark of `doc/compare`, and the line
    `sqrt(p) * x^3 - exp(b/p)` 113 ns rather than 94; `sqr()` and Shekel 5 are
    unchanged. Compiled with FMA instructions (`-mfma`, or on 64-bit ARM,
    which always has them), the exact products cost about 1 ns more than the
    rounded ones.
  - **Tests.** `tests/arithmetic.cpp` requires the tightest bounds for
    x between 2^-30 and 2^30, and n doubles over all the doubles, as before.
- **`nth_root(I, n)`** is the `rootn` of IEEE 1788-2015 (Table 10.5): for an
  odd `n`, it is defined on the whole real line, the root of a negative number
  being the opposite of the root of its magnitude, and `nth_root([-8, 27], 3)`
  encloses `[-2, 3]`; for an even `n`, it takes the roots of the part of `I` in
  `[0, +oo]`, as before. GAOL took that part for every `n`, and gave
  `[-2^-1074, 3.000000000000001]`, or the empty set for `nth_root([-8, -1], 3)`.
  The root of 0 is 0, rather than `[-2^-1074, 2^-1074]`.
  `check/non_arithmetic.cpp` wanted `[0, 1.2457]` for `nth_root([-4, 3], 5)`,
  and now wants `[-1.3195, 1.2457]`.
  libieeep1788 has no `rootn`: its `pown_rev([-8, 27], 3)` is `[-2, 3]`.
- **The n-th roots are proved with integer powers**, and are the tightest
  bounds or one double beyond, for every n and every double (issue #7). GAOL
  took the power of the mathematical library with the exponent 1/n rounded,
  moved one double outward: the rounded exponent moves the root by
  |log x|·2^-53/n relatively, and the bounds were up to 8 doubles from the
  tightest between 2^-30 and 2^30, and 234 over all the doubles;
  `nth_root([27], 3)` was `[0x1.7ffffffffffffp+1, 0x1.8000000000002p+1]`.
  - **The proof.** l is below the root when l^n, rounded upward, is at most x,
    and u above it when u^n, rounded downward, is at least x. The lower bound
    is the largest double so proved, the upper bound the smallest one.
  - **The search.** It starts from the pow of CORE-MATH with the exponent 1/n
    rounded, brought next to the root by a step of Newton's method,
    r - r (r^n - x)/(n r^n), and goes by steps that double until a proved and
    an unproved double are found, then by bisection: two powers from a start
    next to the root. It ends from any start, and its result is proved
    whatever the start.
  - **Exact roots.** The root of a double that is the n-th power of a double is
    that double: `nth_root([27], 3)` is `[3]`.
  - **`nth_root_rel()`** takes the roots of `nth_root()`: it took the same
    powers, and kept a value within 23 doubles, now 2.
  - **Time.** A point interval takes 152 ns rather than 130, its only root
    being looked for once; an interval 194 ns rather than 136 (Intel i7-1185G7,
    GCC 9.4).
- **`sin()`** is computed as `cos()` is: the bounds of the interval divided by
  an enclosure of π, minus 1/2, tell the pieces where the sine is monotonic,
  and the sine of CORE-MATH is taken at the bounds, correctly rounded upward.
  GAOL computed `cos(x - [pi/2])`, whose subtraction widened the argument by about
  2^-52 max(2, |x|): `sin([1e-10])` was 4.4e-16 wide, billions of doubles,
  `sin([1, 2])` two doubles wider than the tightest, and `sin()` was not
  accurate in the sense of IEEE 1788-2015 (12.10.1). The bounds of `sin()` and
  `cos()` are kept within [-1, 1]. On an Intel i7-1185G7 (GCC 9.4), `sin()`
  takes 121 ns rather than 130.
- **`sin()`, `cos()` and `tan()` are within one double of the tightest bounds
  at every magnitude** (issue #6). The bounds divided by an enclosure of π only
  tell on which pieces the interval lies while the quotients are not within
  their rounding errors of an integer: a bound within about |x|·2^-52 of an
  extremum was taken as reaching it, up to |x|²·2^-103 from the tightest bound,
  `cos([2^60])` was `[-1, 1]`, and `tan()` gave `[-oo, +oo]` next to a pole, as
  for the double below π/2, whose tangent is `0x1.9153d9443ed0bp+51`, and
  beyond 2^52.
  - **The signs of the derivative.** CORE-MATH being correctly rounded, and no
    double but 0 being a multiple of π/2 (the closest has a cosine of 4.7e-19),
    the signs of its sine and cosine are the exact ones. For an interval
    narrower than 2π, the derivative has at most two zeros within it: signs
    that differ at the bounds show one extremum, a minimum or a maximum
    according to their order; the same signs show none below the width π, and
    none or two beyond, which the sign at the middle tells. For `tan()`,
    narrower than π, a pole is within the interval exactly when the signs of
    the cosine at the bounds differ.
  - **Only where the quotients cannot tell.** The quotients rounded outward
    still tell the intervals on one piece, and rounded inward those that hold
    an extremum or a pole for sure: the signs are only asked for next to an
    extremum or a pole, and at the large magnitudes. `cos([2^60])` is
    `[-0x1.1d146047d6948p-1, -0x1.1d146047d6946p-1]`.
  - **Time** (Intel i7-1185G7, GCC 9.4): unchanged in the benchmark of
    `doc/compare` (113 ns for `sin()` and `cos()`); 125 ns rather than
    121 for intervals holding an extremum, 67 rather than 62 for those holding
    both; `tan()` of a narrow interval 143 ns rather than 150. At magnitudes
    from 1e8 to 1e15, `cos()` of a narrow interval takes 250 ns rather than
    224, and `tan()` 260 rather than 217, where they gave `[-1, 1]` and
    `[-oo, +oo]` more often.
  - **Tests.** `tests/elementary.cpp` requires one double at every magnitude,
    rather than |x|²·2^-103 + 2^-51 beyond 2^25 for `sin()` and `cos()` and
    2^-49·max(1, |x|)·(1 + tan²) for `tan()`, and checks 649 intervals for
    each function: a few doubles around the doubles nearest to kπ/2, k up to
    2^55, widths about π and 2π from one extremum to the next, and consecutive
    doubles up to the largest.
- **`acos_rel()`, `asin_rel()` and `atan_rel()`** enclose the multiples of
  π of the pieces of the preimage with π in double-double (issue #6): k·π_hi
  is p + e exactly, e being the rest of the product from `fma()`, and
  k·(π − π_hi) is bounded by the products with the two doubles around
  π − π_hi. The inverse function of J is added to that sum before it is
  rounded, one rounding at the magnitude of the result rather than two:
  `atan_rel()` was up to 49 doubles from the tightest bounds over random
  cases where it is now within 2, and `acos_rel()` up to 34 where it is now
  within 2. GAOL computed k·[π_dn, π_up], and `asin_rel()` as
  π/2 + `acos_rel(J, I − π/2)`, two additions of an enclosure of π/2 more:
  the values they have to keep are within 4, 5 and 3 doubles of their
  bounds from 1 to 2^50, where they were within 4, 7 and 7. The three
  functions share one algorithm (`periodic_rel()`), each with its pieces:
  i·π ± acos(J), i·π ± asin(J), i·π + atan(J). An I of a single double is
  decided at every magnitude by whether f(I) meets J, where a bound beyond
  2^52 was kept as it was: `acos_rel(J, [2^60])` is `[2^60]` or empty. An I
  that meets no piece gives the empty set, where the hull of two empty
  intersections was taken.
- **`atan2(y, x)`** is implemented, as the `atan2` of IEEE 1788-2015
  (Table 9.1), defined on the plane but (0, 0) with values in (−π, π]
  ([issue #2](https://github.com/Jordan08/GAOL/issues/2)). GAOL declared it,
  and its parser read it, but it raised `unavailable_feature_error`.
  - **Algorithm.** In each quadrant the angle is monotonic in y and in x: its
    least and greatest values over a box are at two of its corners, which the
    signs of the bounds give, and the atan2 of CORE-MATH, correctly rounded
    upward, is taken there.
  - **Special cases.** A box with points on the half-line y = 0, x < 0, where
    the angle is π, and points below it, whose angles are next to −π, gives
    `[-pi, pi]`; `atan2([0], [0])` is empty; a corner on an axis, on a
    diagonal of the right half-plane or at an infinity gives 0, ±π/4, ±π/2
    or π as the tightest bounds GAOL knows of them:
    `atan2([1, +oo], [1, +oo])` is `[0, pi/2]`.
  - **Time.** 100 ns on an Intel i7-1185G7 (GCC 9.4) for boxes within a
    quadrant, against 68 ns for `atan()`.
- **`exp(0)` = 1 and `log(1)` = 0 exactly**, the bounds moved one double
  outward giving `exp([0])` and `log([1])` a width: `log([0, 1])` was
  `[-oo, 2^-1074]`, and `pow([1], [-oo, +oo])`, exp(y log 1), was `[0, +oo]`
  rather than `[1]`, and `pow([0, 1], [1, +oo])` `[0, +oo]` rather than
  `[0, 1]`. On an Intel i7-1185G7 (GCC 9.4), `log()` takes 1.5 ns more
  (72.5 ns rather than 71) and `pow(x, y)` 2 ns more (145 ns rather than
  143); `exp()` takes the same time.
- **The other elementary functions are exact where their value is a double**,
  as `exp()` and `log()`, and the tightest where it is π/4, π/2 or π:
  `sin(0)`, `tan(0)`, `asin(0)`, `atan(0)`, `sinh(0)`, `tanh(0)`, `asinh(0)`
  and `atanh(0)` are 0, `cos(0)` and `cosh(0)` 1, `acos(1)` and `acosh(1)` 0,
  the n-th roots of 0, 1 and −1 are themselves, and `asin(±1)`, `acos(0)`,
  `acos(-1)`, `atan(±1)` and `atan(±oo)` are the tightest enclosures of ±π/2,
  π and ±π/4, the constants of GAOL. The values moved one double outward
  gave these intervals a width: `acos([1, 3])` was
  `[-2^-1074, 2^-1074]`, `acos(-1)` one double wider than the tightest, and
  `nth_root([1], 3)` `[1 - 2^-53, 1 + 2^-52]`; those of the libm moved three
  doubles, `acosh([1])` and `sinh([0])` `[-3·2^-1074, 3·2^-1074]`. `cosh()`
  and `sinh()` are `[MAX, +oo]` from 711 on, and `tanh()` has the double
  below 1 for lower bound from 20 on, as the three assertions of
  `check/trigonometric.cpp` want: `cosh([MAX, +inf])` was
  `[0x1.ffffffffffffdp+1023, +inf]`, likewise `sinh([-inf, -MAX])`, and
  `tanh([MAX, +inf])` had `0x1.ffffffffffffdp-1` for left bound. `make check`
  passes all its 15 checks. On an Intel i7-1185G7 (GCC 9.4), the functions
  take the same time as before, within 1.5 ns (1 %).
- **Negative integer powers** `pow(x, -n)` are computed as `(1/x)^n` where
  `x^n` is beyond the largest double, rather than as `1/x^n`, whose `x^n`
  overflowed before the inversion: `pow([10], -400)` was `[0, 5.6e-309]`
  rather than `[0, 2^-1074]`, `pow([2], -1050)` `[0, 5.6e-309]` rather than
  `[2^-1050]`, and `pow([-2, 2], -1050)` `[0, +oo]` rather than
  `[2^-1050, +oo]`. Elsewhere `1/x^n` is kept, exact where `x^n` is exact, and
  `pow(x, -3)` takes 0.8 ns more (14.6 ns rather than 13.8).
- **`log()`** gives the empty set for an interval holding no positive number:
  `log` is defined on `(0, +oo)` (IEEE 1788-2015, Table 9.1). GAOL kept the part
  of the interval in `[0, +oo]`, and gave `[-oo, -MAX]` for `log([-4, 0])` and
  `log([0])`, which `check/non_arithmetic.cpp` wanted.
- **The intersection of disjoint intervals** (`operator&`, `operator&=`) is the
  empty set of `interval::emptyset()`, whose bounds are NaN. GAOL kept the
  largest left bound and the smallest right bound, `[3, 2]` for
  `[1, 2] & [3, 4]`: `is_empty()` took it for empty, but the operations that
  compute on the bounds did not, and `([1, 2] & [3, 4]) + [0, 1]` was `[3, 3]`.
  `div_rel(K, J, I)`, which intersects I with the quotients, returned such
  empty sets: `div_rel([5, 6], [1, 2], [10, 20])` was `[10, 6]`.
- **`div_rel()` rounds the lower bound of the quotients downward** where K is
  below 0 and J straddles it, the one branch of the function left without SSE
  code: `K.right()/J.left()` was computed in the rounding direction of the
  library, upward, one double above the bound of the hull where the quotient
  is no double, and the result left out points of the preimage.
  `div_rel([-1], [-3, 1], [0, 10])`, the x of `[0, 10]` with an x·y in
  `[-1]` for a y of `[-3, 1]`, was `[0x1.5555555555556p-2, 10]`, above 1/3,
  rather than `[0x1.5555555555555p-2, 10]`: `mulRev` of IEEE 1788-2015 has to
  be valid (12.10.2), and the numbers from 1/3 to the double above it were
  lost. The bounds of the other branches, computed on the representation
  `<-l, r>` of the SSE2 intervals or negated as `-(K.rb_/J.lb_)` with the
  x87, were rounded outward already.
- **`invabs_rel()`** (`absRev` of IEEE 1788-2015, Table 10.1) takes no
  magnitude from the negative part of J: it is the hull of the x of I whose
  magnitude is in J. GAOL took that part as magnitudes, so that
  `invabs_rel([-2, -1], [-10, 10])`, where no x has a magnitude in
  `[-2, -1]`, was `[-2, 2]` rather than the empty set, and
  `invabs_rel([-2, 0], [-10, 10])` `[-2, 2]` rather than `[0]`. The function
  is otherwise the same: the bounds of J, of −J and of I, hence tightest.
- **`rad()` and `mid_rad()`**, `rad` and `midRad` of IEEE 1788-2015 (12.12.8):
  the radius, the smallest double r such that the interval is in
  `[m - r, m + r]`, m being `midpoint()`, and both at once.
- **The comparisons with the empty set and the infinities** follow IEEE
  1788-2015 (Tables 10.3 and 10.4). `certainly_leq()` and `certainly_le()`,
  `precedes` and `strictPrecedes`, and `certainly_geq()` and
  `certainly_ge()`, are true when either interval is empty: GAOL gave false
  when the second one only was. `set_strictly_contains()` and `set_le()`,
  `interior`, take an infinite bound as beyond the same infinite bound:
  `interior(Entire, Entire)` and `[2, +oo]` interior to `[1, +oo]` were
  false, as `set_le()` of the empty set in the empty set.
  `check/relations.cpp` wanted the former results.
- **The possibly relations, `certainly_eq()`, `certainly_neq()`, `==` and
  `!=` are removed**, and so is the option that chose what the relation
  symbols mean (`--enable-relations`, `-Denable-relations`), which configure
  and meson refuse with a message. The code is kept in comments in
  `gaol/gaol_interval.h`.
  - **Why.** `certainly_neq()` was `!certainly_eq()`, which is "possibly not
    equal": `[3, 4]` was certainly not equal to `[3, 4]`, and so was `!=` with
    the certainly relations, the default of every build. IEEE 1788-2015 has
    neither the possibly relations nor a certain equality (Table 10.3): its
    equality is `equal`, `set_eq()`, and "for all x and y, x ≠ y" is
    `disjoint`, `set_disjoint()`, which `gaol_ieee1788` names already.
    `possibly_eq(y)` was `!set_disjoint(y)`.
  - **The relation symbols.** `<`, `<=`, `>` and `>=` are the certainly
    relations in every build: `strictPrecedes`, `precedes` and their
    converses. The set relations of the option were never taken, configure
    and meson defining `GAOL_SET_RELATIONS` where the header read
    `GAOL_SET_RELATION`: they gave the possibly relations.
  - **Tests.** `tests/other_functions.cpp` checks that `==` and `!=` do not
    compile on intervals.
- **The interval literals of IEEE 1788-2015** are read (9.7, 12.11), whatever
  the case of their letters (`[Empty]`, `[1, Inf]`): `[ ]`, `[entire]`, the
  bounds left out (`[1,]`, `[,]`), `infinity`, the hexadecimal numbers
  (`[0x1.3p-1, 2/3]`) and the uncertain form (`3.56?1`, `-10??u`), all as the
  tightest intervals of doubles enclosing them. A lower bound written `inf`,
  or an upper bound written `-inf`, leaves no interval, as `numsToInterval`
  has it: `[inf]` and `[inf, inf]` are the empty set, where GAOL gave
  `[MAX, +oo]`, which `check/input_output.cpp` wanted; `inf` alone, an
  expression, is still `[MAX, +oo]`. The lexer and the parser are regenerated
  with flex 2.6.4 and bison 3.5.1.
- **The parser frees the nodes of the expressions it reads**
  (`gaol/gaol_interval_parser.ypp`,
  [issue #4](https://github.com/Jordan08/GAOL/issues/4)).
  - **The leak.** The parser kept a reference to each node, which it did not
    give up when the node went into another one. The nodes of every
    `interval("...")` were therefore never freed: LeakSanitizer reported them
    in the tests, and the continuous integration suppressed them
    (`.github/sanitizers/lsan.supp`, now removed).
  - **After an error.** The nodes left by a syntax error are freed too
    (`%destructor`), and so are those of an expression whose exponent throws,
    as `1+pow(2, atan2(1, 1))` did before `atan2()` was implemented: the
    parser aborts, and `parse_interval()` throws the exception again. `parse_interval()` also frees the lexer's buffer
    whatever the parser throws.
  - **A crash.** The parser deleted the null node it gives `nth_root()` and
    `pow()` for an exponent it cannot use, which later expressions still
    used: reading `nth_root(8, 1.5)`, then `nth_root(8, 1.5)+1`, crashed.
    Both now throw `input_format_error`.
- **One grammar reads every string as an expression**
  (`gaol/gaol_interval_parser.ypp`).
  - **Before.** GAOL read a string with two grammars: the numbers, the
    constants and the functions of them built the tree of
    `gaol/gaol_expression.h`, while the intervals written between brackets,
    and the operations on them, were computed at once. Bison chose between the
    two at each token (124 shift/reduce and 14 reduce/reduce conflicts): once
    a number had started the tree, no interval could come, and `1+[1,2]`,
    `2*cos([0,1])` and `[1,2]+1+[1,2]` were refused while `[1,2]*2` was read,
    as was an interval in a bound, `[cos([0,1]), 2]`. Each literal set the flag
    of success, which the next one set again: `[nth_root(8,1.5)]+[1,2]` gave
    `[-oo, +oo]` rather than an error.
  - **Now.** The literals of intervals, the uncertain numbers, `empty` and
    `<a,b>` are leaves of the tree, computed when they are read, and the
    operators and the functions are nodes over any expression. The grammar has
    no conflict, and an error found in an action stops the reading at once. An
    uncertain number is still no bound of a literal: `[5?1]` and `[1, 3.56?1]`
    throw `input_format_error`, IEEE 1788-2015 giving `[5?1]` as a string that
    is not an interval literal (12.11.4).
  - **`pow(x, n)`** is the `pow` of `gaol` for an integer exponent too: the
    parser took 1/x<sup>|n|</sup> for n < 0, whose x<sup>|n|</sup>
    overflowed (`pow(2,-1050)` was `[0, 5.6e-309]` rather than
    `[2^-1050]`), and 1 for n = 0, which is no value for an empty x.
  - **A newline is a space.** It fell to the default rule of flex, which wrote
    it on the standard output and read on.
  - **Expressions are printed as written**: a division with `/`, where GAOL
    wrote `*` (`x/(y*z)` was printed `x*(y*z)`), and a negative number within
    the parentheses a power of it needs (`(-2)^2` was printed `-2^2`).
  - **Tests.** `tests/expressions.cpp` reads the strings GAOL refused, checks
    that an error anywhere stops the reading, and prints both expressions; 9 of
    its checks fail with the two grammars.
- **The reader of strings knows every function of GAOL, and those of IEEE
  1788-2015 under their names.** The lexer takes a name as a whole and looks it
  up in a table of names, rather than in a token of the grammar for each of the
  23 functions GAOL read (`gaol/gaol_interval_parser.ypp`).
  - **The names of GAOL.** `interval("...")`, `operator>>` and
    `gaol::textToInterval()`, which is new, with the two-string form of the
    constructor as `textToInterval(sl, sr)`, read all the functions of GAOL on
    intervals: `exp10`, `log10`, `expm1`, `log1p`, `hypot`, `rsqrt`, `sinpi`,
    `atan2pi`, `sqr`, `abs`, `min`, `max`, `floor`, `integer`, `inverse`, `fma`
    and the others, which GAOL did not read.
  - **The names of the standard.** `gaol_ieee1788::textToInterval()` reads the
    names of Tables 9.1 and 10.5 (`pown`, `rootn`, `recip`, `logp1`, `rSqrt`,
    `sinPi`, `roundTiesToEven`...), whose `pow` is the pow of the standard. It
    read those of GAOL: `textToInterval("pow([-4,-1],2)")` was [1, 16], where
    `gaol_ieee1788::pow([-4,-1], 2)` is the empty set, and `pown`, `rootn` and
    `sinPi` gave the empty set. As for `pow`, a program calls the
    `textToInterval` of the namespace it opens.
  - **Calls.** A call is computed when it is read and gives a leaf of the
    tree; a wrong number of arguments throws `input_format_error`.
  - **Tests.** `tests/expressions.cpp` reads each name of GAOL and refuses
    those of the standard alone, and `tests/ieee1788.cpp` reads each name of
    the standard and gives the empty set for those of GAOL alone.
- **Several threads can read strings at once.** The lexer of flex kept its
  buffer and its position in globals, the parser of bison its token and its
  value, and GAOL the interval read and the names of the functions: two
  threads each building an interval from a string crashed, with "fatal flex
  scanner internal error" or a segmentation fault. The lexer is now a
  reentrant one (`%option reentrant bison-bridge`) and the parser a pure one
  (`%pure-parser`, `%parse-param`, `%lex-param`): each reading of a string has
  its own scanner and its own context, which holds the interval read, the
  exception an action raised and the names of the functions, and the strings
  are read in parallel, with no lock. `parse_interval()`, which
  `interval("...")`, `operator>>` and the two `textToInterval()` go through,
  creates them. The grammar keeps the directives of Bison 2.3, the Bison of
  macOS, which Bison 3 reads with a warning: autotools regenerates the parser
  with the `bison -y` it finds when `gaol_interval_parser.ypp` is newer than
  `gaol_interval_parser.cpp`. The generated lexer and parser stay in the
  repository, and CMake and meson compile them. Four threads reading strings
  at once, with the names of GAOL and of the standard, crashed each time with
  the globals; ThreadSanitizer reports nothing with the reentrant reader. That
  test is commented out in `tests/expressions.cpp`, the tests running no
  thread.
- **The namespaces `gaol_core`, `gaol` and `gaol_ieee1788`** (see
  [Using GAOL](using.md#the-namespaces)). The type `interval`, GAOL's
  functions and its expressions are in `gaol_core`; `gaol` names them as GAOL
  did, and `gaol_ieee1788` as IEEE 1788-2015 does. `pow`, the one function of
  the same name whose meaning differs between the two, is in each of them and
  not in `gaol_core`, where argument-dependent lookup looks for a call on an
  interval: each namespace finds its own `pow` only. The powers of
  `gaol_core` are named apart: `gaol_pown()`, `gaol_uipow()`,
  `gaol_pow_real()`, `gaol_pow_hybrid()`, `gaol_pown_exp()` and
  `gaol_pow_exp()`, and the node of a power of expressions keeps the function
  that computes it. The parser is in `gaol`, and takes GAOL's `pow`.
  - **`gaol/gaol.h`** no longer opens `gaol`: a program adds
    `using namespace gaol;` or `using namespace gaol_ieee1788;`, one of the
    two, `pow(x, y)` being ambiguous with both.
  - **The symbols** of the library name `gaol_core`: a program compiled
    against the headers of GAOL 4 is compiled again.
- **`pow(e, n)` on an expression links.** The library defined it with an
  `unsigned int` exponent, where the header declares an `int`, and a program
  calling it did not link; `tests/expressions.cpp` builds it.
- **`uipow()`**, the `pown` of IEEE 1788-2015 for an unsigned exponent, is
  `pow(I, n)` for an `unsigned` n in `gaol`, `gaol_uipow()` in `gaol_core`,
  declared in `gaol/gaol_interval.h` and exported with the SSE2 intervals too,
  where it was `INLINE` ([issue #10](https://github.com/Jordan08/GAOL/issues/10)).
  - **Before.** `gaol::uipow()` was not found, and `uipow()` did not link.
  - **Now.** It gives `[1]` for an exponent 0, and the empty set for an empty
    interval. `sqr()` and `pow()` inline the same code as before, and take the
    same time.
  - **Removed declarations.** `uipow_upup()` and `uipow_dnup()` computed parts
    of it on the stored bounds, and the SSE2 intervals did not define them.
    They are no longer declared.
- **`width()`** of the empty set is NaN, as `wid` of IEEE 1788-2015 (12.12.8),
  rather than -1, which the manual and `check/interval_functions.cpp` gave.
- **The three builds agree** (see [The three builds](three-builds.md)).
  Before, each had its own idea: configure optimized only when the compiler
  was named `g++` exactly (`clang++` compiled without optimization), computed
  the doubles of 32-bit x86 on the x87 unit, read the processor of the machine
  building rather than the target of the compiler (SSE with `cpuid`, the
  system with `uname`: an armhf container was an `AARCH64_LINUX`), and refused
  macOS, Windows, s390x and riscv64; meson did not optimize, compiled with the
  assertions of libstdc++, refused i686 and armv7l, linked CRlibm by default,
  and its `enable-optimize` did nothing; CMake left out `-msse3` and
  `gaol::interval2f`, the assembly, the verbose mode and the hidden visibility
  of configure, and `-Wconversion`. `-fno-fast-math`, `-ffp-contract=off` and
  the refused compilers were in CMake only. Without `-ffp-contract=off`, the
  compilers fused the multiplications and additions of `mid()`, whose two
  bounds were then computed alike: `mid()` returned a single double, not
  enclosing the midpoint of 9735 of the 10000 random intervals of
  `other_functions`, with meson on macOS arm64 and with autotools on Ubuntu
  arm64. Without `-frounding-math`, GAOL built by `clang++` at `-O2` without
  SSE2 intervals gave integer powers not enclosing their exact values.
- **`-ffloat-store`** is added only where doubles are still computed on the x87
  unit (`FLT_EVAL_METHOD` not 0), whose 80-bit registers keep more digits than
  a double. CMake gave it to GCC on every target, and configure wherever SSE2
  was not used, 64-bit ARM included: GCC then stored every double variable in
  memory rather than in a register. On an Intel i7-1185G7 (GCC 9.4, CMake
  Release), `x + y` takes 3.0 ns rather than 9.1 ns, `x * y` 4.9 ns rather than
  18.3 ns, `sqrt(x)` 8.3 ns rather than 41.3 ns, and `log(x)` 73 ns rather than
  110 ns.
- **The memory of GAOL's SSE2 intervals** is released with the function
  matching the one that allocated it (`MEMFREE()` in `gaol/gaol_port.h`):
  `_aligned_malloc()` and `_aligned_free()` on Windows, where `malloc()` aligns
  on 8 bytes only on 32-bit systems, so that the SSE2 intervals can be used with
  MinGW-w64 and MSYS2 on x64. The placement `delete` of `interval` and
  `interval2f`, called when a constructor throws (`interval("1/0")`), freed
  the caller's memory; it now leaves it alone.
- **Visual C++ without `/fp:strict`** is refused by `gaol/gaol_config.h`, as
  `/fp:fast` was. Built with `/fp:strict` for GAOL and without it for the tests,
  `rounding_direction` and `other_functions` crashed with Visual Studio 2022 on
  x86 and x64: `round_upward_if_needed()` initialized its 2^-60 as `1.0/2^60`,
  which Visual C++ computed when the function first ran with `/fp:strict`, and
  at compile time, in read-only memory, without; the linker kept one copy, and
  GAOL wrote into it. 2^-60 is now a literal, and the tests pass so. Built
  without `/fp:strict` everywhere, they passed too, but Visual C++ then assumes
  rounding to nearest, and nothing certifies the bounds.
- **The intervals of floats are compiled only when asked**, with
  `GAOL_FLOAT_INTERVALS` (`--enable-float-intervals`,
  `-Denable-float-intervals=true`), off by default in the three builds:
  `gaol::intervalf`, intervals of floats computed on the x87 unit, and
  `gaol::interval2f`, two of them in an SSE3 register, were compiled and
  installed by every build, and `gaol/gaol` included their headers. Neither
  IBEX nor Codac uses them, the manual leaves them undocumented, and they are
  unfinished: `sqrt(intervalf)` returns its argument, `interval2f::inverse()`
  aborts, and `pow(interval2f, int)` does not handle the empty set. Without
  the option, their sources are not compiled, their headers are neither
  installed nor included, and their check programs are not built.
- **`is_finite()`** is `std::isfinite()`, in every build: `finite()` of the C
  library was used where the build system found it, and is not declared by
  every C library.
- **The meson build** defines `GETRUSAGE_IN_HEADER`, as configure does, without
  which it did not compile on Linux, and installs the headers for MinGW and
  Visual C++, as configure now does too (and `gaol/gaol_interval2f.h`, which
  it left out, with the intervals of floats). Both install
  a `gaol.pc` carrying the flags of interval arithmetic.
- **CORE-MATH is in the sources** (`3rd/math-core`, see
  [3rd/README.md](../3rd/README.md)): GAOL can be built as a part of another
  project, brought in by FetchContent, with no network access beyond its own
  sources (`tests/fetch_content`).
- **The manual of GAOL v5**, `manual/v5`, follows the manual of GAOL 4 and
  describes GAOL v5: the three builds and their options, the flags of
  interval arithmetic, the namespaces and the rounding direction, each
  operation of GAOL, the new ones included, with examples whose outputs are
  those of GAOL v5, the names of IEEE 1788-2015, the accuracy of each
  operation, and what a program written for GAOL 4 has to change. The manual
  of GAOL 4 is kept as it was in `manual/v4`, and `make -C manual pdf` and the
  target `pdf` of the meson build build both.
- **The manual of GAOL 4 compiles again** (issue #13): `manual/v4/gaol.pdf`
  was the PDF of 2009, and `gaol.tex` no longer compiled. The manual still
  describes the mathematical libraries and the options of GAOL 4.
  `marginbib`, a package of 2000 kept with the manual, patches the output
  routine of LaTeX and stops with the LaTeX of today; the references in the
  margin are now printed by `bibentry` (`\margincite`, `\margincite*` and
  `\marginnocite` in `manual.cls`), and listed at the end of the manual. The
  `multicol.sty` of 2006 kept with the manual is removed for the one of LaTeX,
  and the fonts are Latin Modern, the PDF embedding bitmaps otherwise where
  the cm-super fonts are not installed. `manual/build-pdf.sh` builds it, for
  `make -C manual pdf` and for the target `pdf` of the meson build, which had
  none.
- **The CMake build**, derived from the CMake build of GAOL in IBEX
  (Cyril Bouvier, Gilles Chabert), with the compilation flags of the IBEX fork
  of Fabrice Le Bars.
- **The tests** of `tests/`, after the rounding tests of Codac.
