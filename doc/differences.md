# What differs from GAOL

Part of the documentation of [this fork of GAOL](../README.md#documentation).

Each change is a commit of its own, and says where it comes from.

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
  - `_MATHLIB_DLL_` is only defined when not already, and Visual C++ gets the
    `<fenv.h>` version of `get_inexact()` and `clear_inexact()`.
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
- **The rounding direction is set on x86 processors by writing the control
  registers** of the x87 and SSE units (`fnstcw`/`fldcw`, `stmxcsr`/`ldmxcsr`)
  rather than through `fesetround()`, which cost 130 ns per call with
  mingw-w64 13, 50 ns with the C runtime of Visual C++ for x64 and 250 ns for
  x86, 8.5 ns with glibc. GAOL changed the direction four times for each
  elementary function of an interval (to nearest before mathlib and upward
  after, for each bound; twice now, see below), where mathlib itself takes
  about 10 ns. On the
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
  correctly rounded, whose value rounded to nearest is moved one double
  outward, as for the functions of mathlib
  ([issue #1](https://github.com/Jordan08/GAOL/issues/1)): the bounds are
  within one double of the tightest, on every system.
  - **Before.** GAOL took them from the libm of the system, mathlib having
    none, and moved their values one float outward. The libms of glibc 2.31,
    musl and MinGW-w64 are sometimes a float further, which gave bounds not
    enclosing the exact values. This fork first moved them three floats
    outward, which holds as long as the libm is within two floats of the exact
    value: the `acosh` of MinGW-w64 11 to 13 is millions of doubles away next
    to 1, and its `asinh` NaN for large negative numbers. The branch
    `hyperbolic-rigorous` bounded them without the libm, from `exp` and `log`,
    6 to 45 times slower.
  - **The sources** are `gaol/core_math_*.c`, one file for each function, as
    CORE-MATH distributes them (MIT licence, [3rd/core-math](../3rd/core-math/README.md)),
    compiled into GAOL's library under the names `gaol_cr_sinh()`...
    (`gaol/gaol_core_math.h`). Two changes: `gaol/core_math_port.h` is included,
    which gives Visual C++ the builtins of GCC they use; and `~0ul` is written
    `~(u64)0` in three of them, `unsigned long` having 32 bits on Windows.
  - **Tightness and time** (Intel i7-1185G7, GCC 9.4, glibc 2.31), per
    interval: within 1 double rather than 3 or 4; `sinh()` 84 ns rather than
    131, `cosh()` 76 rather than 77, `tanh()` 105 rather than 127, `asinh()`
    86 rather than 108, `acosh()` 83 rather than 99, `atanh()` 86 rather than
    128: three calls of `nextafter()` less for each bound.
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
- **`pow(I, J)` with finite bounds is within one double of the tightest
  bounds** ([issue #8](https://github.com/Jordan08/GAOL/issues/8)). x^y
  increases with y for x > 1 and decreases for x < 1, increases with x for
  y > 0 and decreases for y < 0: over a box of bases above 0 its extrema are
  at corners, which the places of the bounds about 1 and 0 give, and the pow
  of the mathematical library (mathlib's `upow()`, correctly rounded, which
  `nth_root()` used already) is taken there and moved one double outward;
  a base from 0 with exponents above 0 has 0 for lower bound.
  - **Before.** `exp(J*log(I))` multiplied the relative width of `log(I)`, a
    few 2^-52, by |y log x|: `pow([2], [1023.5])` was 1425 doubles below the
    exact value and 748 above. It is kept where a bound is infinite, or for
    a base from 0 with an exponent that is not above 0, whose limits it gives.
  - **Time.** `pow(x, y)` takes 138 ns rather than 128 on an Intel i7-1185G7
    (GCC 9.4), for bounds 16 times closer to the tightest: over the million
    powers of the benchmark of `doc/compare`, its intervals are 3.6e-15 wider
    than the tightest relatively, against 5.9e-14.
  - **Checked.** `upow()` was compared with mpmath at 240000 arguments,
    results near the overflow, subnormal results, bases next to 1 with large
    exponents, and exact powers with their neighbouring doubles, which take
    its slow path, included: all correctly rounded.
- **`nth_root(I, n)`** is the `rootn` of IEEE 1788-2015 (Table 10.5): for an
  odd `n`, it is defined on the whole real line, the root of a negative number
  being the opposite of the root of its magnitude, and `nth_root([-8, 27], 3)`
  encloses `[-2, 3]`; for an even `n`, it takes the roots of the part of `I` in
  `[0, +oo]`, as before. GAOL took that part for every `n`, and gave
  `[-2^-1074, 3.000000000000001]`, or the empty set for `nth_root([-8, -1], 3)`.
  The root of 0 is 0, rather than `[-2^-1074, 2^-1074]`. IBEX and Codac, which
  add the roots of the negative part themselves (`nth_root(x) | -nth_root(-x)`),
  get the same intervals as before. `check/non_arithmetic.cpp` wanted
  `[0, 1.2457]` for `nth_root([-4, 3], 5)`, and now wants `[-1.3195, 1.2457]`.
  libieeep1788 has no `rootn`: its `pown_rev([-8, 27], 3)` is `[-2, 3]`.
- **`sin()`** is computed as `cos()` is: the bounds of the interval divided by
  an enclosure of π, minus 1/2, tell the pieces where the sine is monotonic,
  and mathlib's sine is taken at the bounds, moved one double outward. GAOL
  computed `cos(x - [pi/2])`, whose subtraction widened the argument by about
  2^-52 max(2, |x|): `sin([1e-10])` was 4.4e-16 wide, billions of doubles,
  `sin([1, 2])` two doubles wider than the tightest, and `sin()` was not
  accurate in the sense of IEEE 1788-2015 (12.10.1). The bounds of `sin()` and
  `cos()` are now within one double of the tightest up to 2^25, and kept
  within [-1, 1]. On an Intel i7-1185G7 (GCC 9.4), `sin()` takes 121 ns rather
  than 130.
- **`atan2(y, x)`** is implemented, as the `atan2` of IEEE 1788-2015
  (Table 9.1), defined on the plane but (0, 0) with values in (−π, π]
  ([issue #2](https://github.com/Jordan08/GAOL/issues/2)). GAOL declared it,
  and its parser read it, but it raised `unavailable_feature_error`.
  - **Algorithm.** In each quadrant the angle is monotonic in y and in x: its
    least and greatest values over a box are at two of its corners, which the
    signs of the bounds give, and mathlib's `uatan2()`, correctly rounded, is
    taken there and moved one double outward. With CRlibm, which has no
    atan2, and with the math library of the system, the `atan2()` of the
    libm is taken, moved outward as the hyperbolic functions are.
  - **Special cases.** A box with points on the half-line y = 0, x < 0, where
    the angle is π, and points below it, whose angles are next to −π, gives
    `[-pi, pi]`; `atan2([0], [0])` is empty; a corner on an axis, on a
    diagonal of the right half-plane or at an infinity gives 0, ±π/4, ±π/2
    or π as the tightest bounds GAOL knows of them:
    `atan2([1, +oo], [1, +oo])` is `[0, pi/2]`.
  - **Time.** 100 ns on an Intel i7-1185G7 (GCC 9.4) for boxes within a
    quadrant, against 68 ns for `atan()`.
- **`exp(0)` = 1 and `log(1)` = 0 exactly**, the bounds of mathlib moved one
  double outward giving `exp([0])` and `log([1])` a width: `log([0, 1])` was
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
  π and ±π/4, the constants of GAOL. The values of mathlib moved one double
  outward gave these intervals a width: `acos([1, 3])` was
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
- **The rounding direction is set once for both bounds** of an elementary
  function: `exp()`, `log()`, `sin()`, `cos()`, `tan()`, `asin()`, `acos()`,
  `atan()`, the hyperbolic functions and `nth_root()` set it to nearest
  before evaluating the mathematical library at both bounds, and upward
  after (`GAOL_RND_NEAREST_ENTER()`, with the functions of `gaol::nearest`),
  rather than around each bound. The bounds are the same, and `exp_dn()`,
  `cos_up()` and the like still set the direction themselves. On an Intel
  i7-1185G7 (GCC 9.4, glibc 2.31), `sin()` and `cos()` take 113 and 114 ns
  rather than 124 and 126, `tan()` 106 ns rather than 120, the hyperbolic
  functions 9 to 12 ns less, `exp()` 47 ns rather than 53, `log()` 69 ns
  rather than 73, and `pow(x, y)` 129 ns rather than 147. With the math
  library of the system (`--with-mathlib=m`), the elementary functions left
  the direction to nearest, and so did `cosh_up()` with CRlibm, where GAOL
  leaves it upward: they leave it upward too.
- **`log()`** gives the empty set for an interval holding no positive number:
  `log` is defined on `(0, +oo)` (IEEE 1788-2015, Table 9.1). GAOL kept the part
  of the interval in `[0, +oo]`, and gave `[-oo, -MAX]` for `log([-4, 0])` and
  `log([0])`, which `check/non_arithmetic.cpp` wanted; IBEX and Codac returned
  the empty set themselves before calling it.
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
  `certainly_eq()`, true for two intervals that are the same double, or both
  empty, ignored the lower bound of its argument: `[2] == [1, 2]` was true
  with the certainly relations, the default of every build.
  `check/relations.cpp` wanted the former results.
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
- **`uipow()`**, the `pown` of IEEE 1788-2015 for an unsigned exponent, is
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
  Codac's `diam()` returned NaN for the empty set without calling `width()`;
  IBEX's `diam()`, which its documentation says is 0 for the empty set, returns
  what `width()` returns, NaN now.
- **The cosine of mathlib** (`3rd/mathlib/src/sincos32.c`): for the arguments
  hardest to round, mathlib computes cos(x) with multiple-precision numbers, as
  sin(π/2 − x) when x > 0.8, and `mpcos()` returned the cosine of π/2 − x
  instead, which is sin(x). `cos()` then gave bounds not enclosing cos(x), off
  by up to 9%, at 54 of the hard-to-round arguments of cos of
  [CORE-MATH](https://gitlab.inria.fr/core-math/core-math), all between 0.80
  and 0.853 ([dreal-deps/mathlib#2](https://github.com/dreal-deps/mathlib/issues/2)).
  The call to `c32()` in `mpcos()` is fixed, the line glibc fixed in its copy
  of the same code in 2003.
- **The arctangent of mathlib** (`3rd/mathlib/src/mpsqrt.c`): `fastiroot()`,
  which starts the multiple-precision square roots mathlib computes the
  arctangent with at the arguments hardest to round, read the halves of a double
  through `long`s. Where `long` has 64 bits (Linux and macOS on 64-bit
  processors), `atan()` returned values far from atan(x), or had not returned
  after 20 ms, at 15970 of the 55190 hard-to-round arguments of atan of
  CORE-MATH: `atan(1.016527294692847)` was 0.082 instead of 0.794. They are
  `int`s, as glibc made them in 2003
  ([commit](https://sourceware.org/git/?p=glibc.git;a=commit;h=bb3f4825c411e676c51479fea59643af540810b5));
  [Debian bug 210613](https://bugs.debian.org/210613) is the same bug on Alpha.
- **The logarithm of mathlib at subnormal arguments**
  (`3rd/mathlib/src/ulog.c`): `ulog()` scales a subnormal argument by 2^54,
  but its last, multiple-precision stage computed the logarithm from the scaled
  argument and from an approximation of the logarithm of the unscaled one.
  `log()` returned about 2^54 at 26 of the 53 subnormal hard-to-round arguments
  of log of CORE-MATH: `log(0x0.8819864d7985dp-1022)` was 1.8e16 instead of
  −709.03. That stage is given the unscaled argument. glibc had
  the same code until it
  [removed that stage](https://sourceware.org/git/?p=glibc.git;a=commit;h=b7c83ca30ef8e85b6642151d95600a36535f8d97)
  in 2018.
- **`#pragma STDC FENV_ACCESS ON` in mathlib's configuration**
  (`3rd/mathlib/src/mathlib_config.h`): the pragma of C99 (7.6.1) that tells the
  compiler the code may be executed with a rounding direction other than the
  default, and that it must not fold nor reorder its floating-point operations
  as if the rounding were to nearest, is written at the end of
  `src/mathlib_config.h`, which every source of mathlib includes. GAOL calls
  mathlib with the rounding direction set to nearest and sets it back upward
  afterwards, and mathlib is compiled with the flags of interval arithmetic:
  the pragma states for the compiler what those flags ask of it. It comes from
  the fork of mathlib by Fabrice Le Bars
  ([commit](https://github.com/lebarsfa/mathlib/commit/5ac52c2bd817e44d33d4f9af6c1045d4b8577449)),
  with the uppercase `ON` that macOS warns about in the lowercase. Clang 18
  honours it, and the elementary functions of an interval took the same time
  with it as without (exp, log, sin and cos within 0.8%, below the dispersion
  of the measures); GCC 13 ignores it and gave a `libultim.a` identical byte
  for byte; Visual C++ is given `/fp:strict`, which its documentation says
  makes it behave as if `fenv_access(on)` were set, and reads the pragma for
  Visual C++ rather than the one of C99, which it does not know.
- **The warnings of mathlib's tables** (`3rd/mathlib/src/uatan.tbl`,
  `ulog.tbl` and `utan.tbl`): the
  entries of the tables, of the union type `number`, are written
  `{0x3ff6a13c, 0xd1537290 }` where the union holds an array, and GCC and Clang
  warn about each of them with `-Wall` (`-Wmissing-braces`), 15777 times over
  mathlib's sources. Printing them is slow enough to stop a build: compiling
  `src/atnat.c`, which includes `src/uatan.tbl` and its 6027 of them, had not
  finished after ten minutes, against 0.22 s without `-Wall`. The three tables
  carrying 15165 of the 15777, `uatan.tbl`, `ulog.tbl` and `utan.tbl`, are
  given the pragma that turns the warning off, as Fabrice Le Bars does in his
  fork of mathlib
  ([commit](https://github.com/lebarsfa/mathlib/commit/daa4f21874f76785988426f03a5f651ac5a6cf4e)):
  434 warnings are left, from the tables of the other sources, which are kept
  as they are, and `src/atnat.c` compiles in 0.22 s with `-Wall`. The pragma
  changes no code: `libultim.a` is the same, byte for byte, with it and without
  it. The builds of GAOL compile mathlib with `-w` besides, whatever warning
  flags the project building GAOL gives.
- **The shifts of mathlib's `halfulp()`** (`3rd/mathlib/src/halfulp.c`,
  [issue #5](https://github.com/Jordan08/GAOL/issues/5)): `halfulp()`, which
  `upow()` calls for the powers that may be exact, and so `nth_root()`,
  shifted a positive `int` into its sign bit, undefined behaviour in C that
  the UndefinedBehaviorSanitizer of Clang reports. It shifts an
  `unsigned int`, and returns the same doubles (4 million arguments compared,
  228644 of them exact powers). The continuous integration suppressed the
  report (`.github/sanitizers/ubsan.supp`, now removed): it suppresses nothing
  any more.
- **`Init_Lib()` and `Exit_Lib()` of mathlib**
  (`3rd/mathlib/src/AARCH64_DPChange.c`,
  `3rd/mathlib/mathlib_configuration.h.in`): `Init_Lib()` sets the rounding
  direction to nearest, which mathlib's algorithms need, returns the one it
  found, and `Exit_Lib()` sets that one back, from the fork of mathlib by
  Fabrice Le Bars
  ([commit](https://github.com/lebarsfa/mathlib/commit/40c8a25ad855830db7688ef7dd32bb667ff0eb25)).
  `src/AARCH64_DPChange.c`, which does it through `<fenv.h>` alone, is the
  implementation chosen for every target: the ones mathlib's own build chooses
  on x86_64 Linux, on Intel Macs and on 32-bit x86 save the control word of the
  x87 unit in global variables, which two threads calling `gaol::init()` would
  write at once, to set a precision that GAOL loses right after by restoring
  the default floating-point environment, and that the doubles of 32-bit x86,
  computed in SSE2, do not depend on.
  - **Before.** `Init_Lib()` of `src/AARCH64_DPChange.c` restored the default
    floating-point environment and returned 0, and `Exit_Lib()` did nothing,
    where the comment of `Init_Lib()` says its result is what `Exit_Lib()` takes
    to restore what it found. On x86_64 Linux, `src/LINUX64_DPChange.c` wrote
    back the control word of the x87 unit alone: after `gaol::cleanup()`, the
    x87 unit rounded to nearest again, as `fegetround()` reported, while the
    SSE unit, which computes the doubles there, was left rounding upward.
  - **Now.** Both are set back, on every target. The rest of the floating-point
    environment, whose exception flags and masks `FE_DFL_ENV` also reset, is
    left as it is; with `GAOL_PRESERVE_ROUNDING`, where `gaol::init()` does not
    restore the default environment itself, the rounding direction of the caller
    is no longer lost either.
  - **Not taken.** The same version of that fork also sets the precision of the
    x87 unit to 53 bits on 32-bit Windows (`_controlfp(_PC_53, _MCW_PC)`).
    Windows sets it already, 32-bit Visual C++ computes doubles in SSE2, and
    `Init_Lib()` is called once, outside everything GAOL times: measured with
    Visual C++ 2022 and 2026, in x86 and in x64, it changed neither the bounds
    nor the time of an operation.
- **A mathlib found installed whose cosine of 2^52 − 1 is wrong** is refused by
  the three builds (see
  [Compilers and options refused](three-builds.md#compilers-and-options-refused)). A TODO of
  GAOL's check `reverse_mappings` said that `acos_rel()` failed at
  [2^52 − 1, 2^52 − 1/2] under AArch64: mathlib compiled there without
  `-ffp-contract=off`, as its own configure compiles it, gives that cosine far
  from its value. `tests/elementary.cpp` checks sin and cos at arguments where
  such a mathlib fails.
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
  configure also put the directories of `--with-mathlib-include` and
  `--with-mathlib-lib` before those of GAOL's sources and of the library just
  built: with mathlib in a prefix holding an installed GAOL, as the prefix of a
  GAOL installed by CMake, the sources included the headers installed, and
  `make check` and the examples linked the library installed.
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
  a `gaol.pc` carrying the flags of interval arithmetic, and meson takes
  `with-mathlib-include` and `with-mathlib-lib` as configure does.
- **mathlib is in the sources** (`3rd/mathlib`, see
  [3rd/README.md](../3rd/README.md)): the archive of mathlib 2.1.1, with the
  fixes above. The three builds compile it themselves, with the flags of
  interval arithmetic, and install it along with GAOL, unless told to use an
  installed one. The CMake build downloaded the archive from Frédéric
  Goualard's site in each new build directory and patched it; the autotools
  and meson builds needed mathlib installed beforehand, with
  `scripts/install-mathlib.sh`, which downloaded it too. GAOL can now be built
  as a part of another project, brought in by FetchContent, with no network
  access beyond its own sources (`tests/fetch_content`).
- **The CMake build**, derived from the CMake build of GAOL and mathlib in IBEX
  (Cyril Bouvier, Gilles Chabert), with the compilation flags of the IBEX fork
  of Fabrice Le Bars.
- **The tests** of `tests/`, after the rounding tests of Codac.
