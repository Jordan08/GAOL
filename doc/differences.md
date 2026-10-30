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
  - **No build names `-lm`.** CORE-MATH is the only library GAOL takes its
    elementary functions from, but the C library still gives it `sqrt()`,
    `floor()`, the functions of `<fenv.h>` and, where the processor has no
    fused multiply-add instruction, `fma()`. g++ and clang++ link the math
    library themselves, so the autotools build (whose `LDADD` was `-lm` in
    GAOL 4), meson (which called `find_library('m')`) and the `gaol.pc` that
    the builds install do not name it, as the CMake build never did:
    `pkg-config --libs gaol` gives `-L<libdir> -lgaol` and the link options of
    interval arithmetic.
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
  - mingw-w64 is no longer refused for its version alone. Both former reasons
    are gone: the hyperbolic functions of its math library, which GAOL no
    longer uses, and the cost of its `fesetround()`, which GAOL no longer calls
    for its elementary functions. MinGW-w64 GCC 12 to 15 are built and tested
    again on 32-bit x86, GCC 14 and 15 on x86-64, and so are MSYS2 UCRT64 and
    CLANG64. Still refused (`gaol/gaol_config.h`) are the mingw-w64 whose
    `fma()` or `round()` is wrong, those of its own math library, computed in
    doubles: that `fma()` is not correctly rounded (10.7 % of the error-free
    products `fma(a, b, -a*b)` were wrong with mingw-w64 11 on x86-64), and
    CORE-MATH computes with it wherever the compiler has no `-mfma`; that
    `round()` depends on the rounding direction. On x86-64 these are the
    mingw-w64 before version 12 and those linked with `msvcrt.dll` rather than
    the UCRT (MSYS2 MINGW64, the cross compilers of Debian and Ubuntu); on
    32-bit x86, those before version 11 and, from 12 on, those linked with
    `msvcrt.dll` where the build has no `-mfma` (MSYS2 MINGW32); on ARM, as on
    x86-64 but for `round()` alone (`fma()` is an instruction there), a refusal
    that is not tested.
  - `tests/core_math.cpp` checks the `fma()` and `round()` of the C library in
    the four rounding directions, against exact rational arithmetic, then the
    bounds against CORE-MATH called in the downward and the upward rounding,
    `tests/expressions.cpp` the intervals read from a string, and
    `-DGAOL_COVERAGE=ON` writes the coverage of the tests in
    [coverage/README.md](../coverage/README.md) (87 % of the lines of GAOL).
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
  - **The reader of strings takes the new names.** `gaol::textToInterval("...")`
    reads `exp2`, `log2`, `cbrt`, `sign` and `trunc` besides the functions
    GAOL already had, in the direct grammar and in the tree of
    `gaol/parser/gaol_expression.h`; `cbrt(x)` is `nth_root(x, 3)`, as `sqrt(x)` is
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
- **For Visual C++, MinGW and the systems GAOL's configure does not know**,
  following the fork of GAOL by [Fabrice Le Bars](https://github.com/lebarsfa/GAOL):
  - The generated configuration is included with every compiler:
    `gaol/gaol_config.h` includes it itself, where it included
    `gaol/gaol_config_msvc.h` and `gaol/gaol_config_mingw.h`, which only
    included it, and `gaol/gaol_version.h` includes `gaol/gaol_config.h`,
    where it included `gaol/gaol_version_msvc.h` for Visual C++ and nothing for
    MinGW. The four headers are gone.
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
    as it is after the check of the function of intervals calling them (the
    functions of the same names outside it, which checked it again for the
    code calling them directly, are gone since: see below). `exp()` of an
    interval checked the direction three times,
    `sin()` and `cos()` up to four, the other elementary functions and
    `pow(x, y)` two or three: once now, the bounds being the same. `exp()`
    takes 5.7 % less time, `sin()` 3.6 %, `pow(x, y)` 3.4 % (Clang 18,
    i7-1185G7).
  - **Once in the operations calling others.** `tan()` checked the direction
    six times, calling `width()`, `+` and `/` on intervals after its own
    check, and `acos_rel()`, `asin_rel()` and `atan_rel()` six times too,
    with `acos()`, `asin()` or `atan()` of J and four operations of intervals;
    `pow(x, -n)` two to five times, with `inverse()` and `gaol_uipow()`;
    `pow(x, n)` twice where it falls back to the products rounded outward (a
    bound 0 or infinite, a power not finite or below 2^-968); `nth_root(x, n)`
    for n >= 4 twice; `gaol_ieee1788::pow(x, [n])` for an integer n beyond the
    ints three times. They compute on doubles, or with the bodies of the
    operations they called (`uipow_rounded_upward()`,
    `interval::inverse_upward()`), after their one check, and give the same
    bounds, bit for bit, in the SSE2 and FPU builds and with
    `GAOL_PRESERVE_ROUNDING`. `sqrt_rel()` no longer sets the direction
    downward then upward again, two changes per call in every build: its
    lower bound is computed upward, as `sqrt()` computes it. `tan()` takes
    22 % less time, `acos_rel()` 24 %, `sqrt_rel()` 29 %, `pow(x, -3)` 6 %;
    with `GAOL_PRESERVE_ROUNDING`, where each check saves and restores the
    direction, `tan()` 74 % and `acos_rel()` 83 % (Clang 18, i7-1185G7).
    Still checking more than once: `pow(x, y)` where it takes exp(y log x)
    (an infinite bound, or a base from 0 with an exponent not above 0),
    `nth_root(x, q)` for q < 0, the inverse of a root, and `modulo_k_pi()`.
  - **`tan()` keeps its bounds before setting the direction back**, with
    `GAOL_PRESERVE_ROUNDING`: it set it back first, and `GAOL_RND_KEEP()`,
    which writes the bounds to memory so that they are computed before, came
    too late.
  - **The bounds at doubles set the direction back**, with
    `GAOL_PRESERVE_ROUNDING`: `exp_dn()`, `sin_up()` and the thirty others
    of `gaol/gaol_double_op.h` saved the direction they found, but left it
    upward, which the compilers only showed by warning that `_save_state` was
    set but not used. They are gone since, with the header no longer
    installed (see below).
  - **`pow(x, y)` keeps its bounds before setting the direction back**, with
    `GAOL_PRESERVE_ROUNDING`: it set it back without `GAOL_RND_KEEP()`, which
    the rule of `gaol/gaol_fpu.h` asks of every value computed before and used
    after, so that GCC, which does not model the rounding direction, could
    compute them after. `pow(x, y)` gives the same bounds in every rounding
    direction (`tests/rounding_direction.cpp`). `ipow_up()` and `ipow_dn()`
    had the same defect, but no operation of GAOL called them: they are gone
    (see below).
  - **`gaol::cleanup()` sets back the rounding direction** that the first
    `gaol::init()` found, unless it is preserved: GAOL left it upward after
    its use, even after its automatic cleanup when the program ends or the
    library is unloaded, and a program computing to nearest after it had to
    set it back itself. Only the rounding direction is set back, on x86 the
    one of the x87 unit and the one of the SSE instructions (`rounding_state`,
    `gaol/gaol_fpu.h`), not the rest of the environment that `gaol::init()`
    replaced with `fesetenv(FE_DFL_ENV)`: the exception flags raised since are
    the program's. `tests/rounding_direction.cpp` checks the direction after
    `gaol::cleanup()`, and `tests/automatic_cleanup.cpp` after the end of
    `main()`, on Linux, where the priorities of constructors let a function
    registered with `std::atexit()` run after GAOL's automatic cleanup.
  - **GAOL initializes itself with every compiler, before the static objects
    of the program**: each file including `gaol/gaol_common.h` holds a static
    `gaol_core::gaol_initializer`, whose constructor initializes GAOL the first
    time, as `std::ios_base::Init` does the standard streams. GAOL declared
    `gaol_init()` with `__attribute__((constructor))`, for GCC and Clang only:
    Visual C++ had no automatic initialization (its `gaol/dllmain.cpp`, for
    a DLL, was compiled by none of the builds and is gone), and the
    constructor of the static
    library ran after those of the program (except with MinGW-w64, which runs
    the constructors from the last linked to the first): the static objects of
    the program computing intervals set the rounding direction upward before
    the first `gaol::init()` read it, so that `gaol::cleanup()` left it upward.
    `gaol_init()` is gone. `tests/rounding_direction.cpp` computes such an
    object.
  - **`gaol::cleanup()` no longer frees what the initialization allocated**:
    the automatic cleanup frees it, when the program ends or the library is
    unloaded, after the static objects of the program are destroyed.
    `gaol::cleanup()` deleted the node of the empty expression while
    expressions still referred to it, a local of `main()` destroyed after the
    call or a static object, whose destructors then wrote into freed memory:
    `tests/expressions.cpp` holds such a static expression, which the
    sanitizers of the continuous integration check.
- **The constants of GAOL are initialized when compiling**, so that the
  intervals computed in the initialization of the static objects of a program
  are right. They were computed by the dynamic initialization of GAOL's files,
  which, with the static library of the CMake build, comes after the static
  objects of the program, except with MinGW-w64: an interval computed there
  found them 0, and `interval::pi()` was [-0, 0], `[1, 2]/[-1, 1]` was [-0, 0]
  rather than [−∞, +∞] and `sin([0, 4])` was [-0, 0], without any warning (25
  of the 45 checks of `tests/static_initialization.cpp` failed with the SSE2
  intervals, 24 with the others).
  - `gaol_pi_dn`, `gaol_pi_up`, `gaol_half_pi_dn` and `gaol_half_pi_up`
    (`gaol/gaol_port.h`) are written in decimal, exactly, rather than read from
    the unions that give their bits, which is not a constant expression. They
    are in `gaol_core`, under these names (GAOL 4 had them in `gaol` as
    `pi_dn`...), and `gaol` and `gaol_ieee1788` name them too.
  - `interval::one()`, `pi()`, `universe()` and the other constants of
    intervals build their interval from these doubles, which the compiler
    folds, rather than copy the static intervals `interval::cst_one`,
    `cst_pi`..., which are gone.
  - The masks of the SSE2 intervals (`gaol/core/gaol_interval_sse.cpp`) are
    written as vectors, which GCC and Clang, the only compilers of the SSE2
    intervals, put in read-only memory, rather than computed with
    `_mm_set_pd()` and `_mm_cmpeq_pd()`.
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
- **`x -= x`, `x /= x` and `x %= x`** enclose their results in the FPU intervals
  (`GAOL_SIMD=OFF`, and the processors with neither SSE2 nor NEON: 32-bit ARM,
  POWER, s390x and RISC-V), the bounds of the operand being read before those of
  the interval are written. The operand being the interval itself, GAOL wrote
  one bound before reading it: `x -= x` was `[a - b, 2b - a]` for x = [a, b],
  `[-2, 1]` for `[-3, -1]`, and `x /= x` and `x %= x` were `[a/b, b²/a]` for 0 <
  a ≤ b, `[0.5, 1]` for `[0.25, 0.5]`, neither containing the exact result;
  `[0.1] /= [0.1]` was empty. Normalising a row by its pivot, `row[j] /= row[i]`
  down to j = i, did it. The SSE2 intervals were right.
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
- **Numbers are read exactly** (`gaol/parser/gaol_interval_lexer.lpp`). GAOL read them
  with `strtod()` rounding downward and upward, and relied on the inexact flag.
  The C runtime of Windows and musl on 64-bit ARM processors round to nearest in
  every direction and raise no flag, so `interval("0.1")` did not enclose 1/10
  there. Each number is now compared exactly with the doubles around it,
  whatever the number of digits of its significand and of its exponent.
- **Numbers are read, and exact texts written, whatever the locale** of the
  program. GAOL read numbers with `strtod()`, which reads the decimal point of
  the C locale: under a locale writing a decimal comma, as
  `setlocale(LC_ALL, "")` sets under a French one, it stopped at the `.` of
  `0.1`, and `interval("0.1")` was `[0, 0]`. The doubles around the number
  are now bracketed by steps doubling from the double `strtod()` gives, then
  found by bisection on their bits: two comparisons when `strtod()` gives the
  nearest double, and at most about 125 whatever it gives. `exact_string()`
  and the format `hexa` write the bounds from the bits of the doubles, in the
  form glibc's `%a` has, with a point whatever the locale and the same text
  whatever the C library. The decimal formats of `operator<<` keep the
  decimal point of the locale of the stream, as for a double.
  `tests/numbers.cpp` reads and writes under such a locale where one is
  installed.
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
- **`operator<<` leaves the stream as it found it.** The interval is written
  in a stream of its own, with the flags and the locale of the stream and the
  precision of `interval::precision()`, then into the stream at once. GAOL set
  the precision of the stream to `interval::precision()` and left it so, the
  doubles written afterwards getting 16 digits, and wrote the `[` apart, which
  `std::setw` padded alone: `std::setw(8) << interval(1, 2)` wrote 7 spaces
  before `[1, 2]` rather than 2.
- **`operator>>` ends with the input.** Where no line is left, it sets the
  `failbit` of the stream, leaves the interval as it was and throws nothing,
  as the reading of a double does, and `while (in >> x)` ends there. It read
  the empty text left, threw `input_format_error` and emptied the interval:
  such a loop always ended with an exception. A line that is no interval, a
  blank one included, sets the `failbit` too, and empties the interval; then
  `input_format_error` is thrown as before, a stream whose `exceptions()`
  include `failbit` included (`invalid_action_error` for a function called
  with an argument it does not take). A program that reads on after it calls
  `in.clear()` first: GAOL read the next line.
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
  symbols mean (`--enable-relations`, `-Denable-relations`), gone from
  configure and meson. The code is kept in comments in
  `gaol/core/gaol_interval_core.h`.
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
- **Numbers in base 16, 8 and 2** are read, `0x10` (16), `0xff.8` (255.5),
  `0o17` (15), `0b101` (5) and `0b1.1p3` (12), the exponent after the `p` being
  optional: the hexadecimal number of the standard needs it (9.7.2), which
  allows a more general syntax (12.11.5). A number that begins with 0 is
  decimal, as the standard has it: `010` is 10, and octal is written `0o10`,
  as in Python, rather than `010` as in C.
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
  (`gaol/parser/gaol_interval_parser.ypp`,
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
  (`gaol/parser/gaol_interval_parser.ypp`).
  - **Before.** GAOL read a string with two grammars: the numbers, the
    constants and the functions of them built the tree of
    `gaol/parser/gaol_expression.h`, while the intervals written between brackets,
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
  23 functions GAOL read (`gaol/parser/gaol_interval_parser.ypp`).
  - **The names of GAOL.** `gaol::textToInterval()`, which is new, from one
    string or from two, and `operator>>` read all the functions of GAOL on
    intervals: `exp10`, `log10`, `expm1`,
    `log1p`, `hypot`, `rsqrt`, `sinpi`, `atan2pi`, `sqr`, `abs`, `min`, `max`,
    `floor`, `integer`, `inverse`, `fma` and the others, which GAOL did not
    read.
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
  `interval(sl, sr)`, `operator>>` and the two `textToInterval()` go through,
  creates them. The grammar keeps the directives of Bison 2.3, the Bison of
  macOS, which Bison 3 reads with a warning. The generated lexer and parser
  stay in the repository, and the three builds compile them; `make parser`
  regenerates them (see
  [Building GAOL](building.md#tests-examples-performance-and-the-parser)). Four threads reading strings
  at once, with the names of GAOL and of the standard, crashed each time with
  the globals; ThreadSanitizer reports nothing with the reentrant reader. That
  test is commented out in `tests/expressions.cpp`, the tests running no
  thread.
- **The namespaces `gaol_core`, `gaol` and `gaol_ieee1788`** (see
  [Using GAOL](using.md#the-namespaces)). The type `interval`, GAOL's
  functions and its expressions are in `gaol_core`; `gaol` names them as GAOL
  did, and `gaol_ieee1788` as IEEE 1788-2015 does. `gaol` names them one by one,
  with using-declarations, rather than opening `gaol_core` whole: the names
  GAOL 4 had in `gaol` that GAOL v5 still has, the functions on intervals GAOL
  v5 adds (`exp2`, `hypot`, `fma`, `cancel_minus`, `trunc`... and the nodes of
  their expressions), `restore_rounding()` and `exact_string()`. The two
  namespaces differ mainly in the power, the standard having two where GAOL 4
  had one: the `pow` of `gaol` takes the integer power for an integer exponent,
  a negative base included, and the real power otherwise, while in
  `gaol_ieee1788` `pow` is the real power, on the part of x in [0, +oo], and
  `pown` the integer power (see [The powers in the two
  namespaces](using.md#the-powers-in-the-two-namespaces)). `pow` and
  `textToInterval`, the functions of the same name whose meaning differs
  between the two, are in each of them and not in `gaol_core`, where
  argument-dependent lookup looks for a call on an interval: each namespace
  finds its own only. The powers of
  `gaol_core` are named apart: `gaol_pown()`, `gaol_uipow()`, `gaol_pown_ll()`,
  `gaol_pow_real()`, `gaol_pow_hybrid()`, `gaol_pown_exp()` and
  `gaol_pow_exp()`, and the node of a power of expressions keeps the function
  that computes it. The reader of strings, `parse_interval()` and
  `parsing_names`, is in `gaol_core`, and its code takes GAOL's `pow`.
  `gaol` does not name the exceptions, `NaN_val`, `eval_stack` nor
  `parse_interval()`, which GAOL 4 had in `gaol`, nor `rnd_keep()` and
  `parsing_names`, which GAOL v5 adds, nor what GAOL uses for itself: the
  functions that get and set the `rounding_state`, the rounding direction and
  the flush-to-zero modes (`get_rounding()`, `set_rounding()`,
  `get_flush_modes()`...), and the internals of GAOL 4 that its manual did not
  document (`gaol_signbit()`, `uintdouble`, `ullidouble`, `Interval_struct`,
  `the_null_expr`, `prec_t`, `modulo_k_pi()`, `round_upward_sse()` and the other
  `round_*_sse()`). A program names them in `gaol_core`, as
  `gaol_core::get_rounding()`. `f_negate()`, `f_negate_simple()`, the macro
  `opposite()`, `get_fpu_cw()` and `reset_fpu_cw()`, which GAOL v5 no longer
  calls, are gone. `gaol` and `gaol_ieee1788` both name `version_major`,
  `version_minor`, `version_micro` and `version`, and `gaol_pi_dn`,
  `gaol_pi_up`, `gaol_half_pi_dn` and `gaol_half_pi_up`.
  - **`gaol/gaol.h`** no longer opens `gaol`: a program adds
    `using namespace gaol;` or `using namespace gaol_ieee1788;`, one of the
    two, `pow(x, y)` being ambiguous with both.
  - **The symbols** of the library name `gaol_core`: a program compiled
    against the headers of GAOL 4 is compiled again.
- **`pow(e, n)` on an expression links.** The library defined it with an
  `unsigned int` exponent, where the header declares an `int`, and a program
  calling it did not link; `tests/expressions.cpp` builds it.
- **`expression::operator/=` links**: it was declared and not defined. The
  operators `+=`, `-=`, `*=` and `/=` of expressions build the new node before
  giving up their reference to the old root, which a `std::bad_alloc` left
  one reference short, and deleted while another expression held it.
- **Empty expressions can be created in several threads.** They all point to
  one node, `the_null_expr`, whose count of references, a plain
  `unsigned int` that two threads changed at once, lost updates and came down
  to 0 while expressions still pointed to it: the node was deleted twice, and
  two threads each creating and destroying a million empty expressions
  crashed each time. The references to that node are no longer counted: it
  lives from the initialization of GAOL to its automatic cleanup, when the
  program ends or the library is unloaded, which deletes it.
  `tests/expressions.cpp` checks, with `expr_node::references()`, that its
  count no longer changes, the tests running no thread. Other expressions
  still share the nodes of their copies and clones, whose counts are not
  atomic: such an expression is not copied or destroyed in two threads at
  once; expressions built apart in each thread are independent.
- **`uipow()`**, the `pown` of IEEE 1788-2015 for an unsigned exponent, is
  `pow(I, n)` for an `unsigned` n in `gaol`, `gaol_uipow()` in `gaol_core`,
  declared in `gaol/core/gaol_interval_core.h` and exported with the SSE2 intervals too,
  where it was `INLINE` ([issue #10](https://github.com/Jordan08/GAOL/issues/10)).
  - **Before.** `gaol::uipow()` was not found, and `uipow()` did not link.
  - **Now.** It gives `[1]` for an exponent 0, and the empty set for an empty
    interval. `sqr()` and `pow()` inline the same code as before, and take the
    same time.
  - **Removed declarations.** `uipow_upup()` and `uipow_dnup()` computed parts
    of it on the stored bounds, and the SSE2 intervals did not define them.
    They are no longer declared.
- **The bounds at doubles are no longer part of the interface.**
  `gaol/gaol_double_op.h` declared the functions of doubles `exp_dn()`,
  `sin_up()` and the thirty others of the elementary functions, `ipow_up()`,
  `ipow_dn()`, `pow_up()`, `pow_dn()`, and those of the namespaces `upward`
  and `nearest`; `gaol/gaol` and `gaol/gaol_interval.h` included it, and the
  three builds installed it. It is now a header of the sources, which no
  installed header includes, and `gaol_ieee1788::pow()`, which called two of
  its functions, calls `gaol_core::pow_standard()`, which the library compiles
  and exports, rather than compute inline in `gaol/gaol_ieee1788.h`. The
  thirty-two functions that checked the rounding direction for the code calling
  them from outside GAOL are gone, and `tests/rounding_direction.cpp` no longer
  calls them; `gaol_core::upward`, which the operations of intervals call,
  stays. `ipow_up()`, `ipow_dn()`, `pow_up()` and `pow_dn()`, which no operation
  of GAOL called, are removed from the header.
- **The `double` constants of `gaol/gaol_port.h` are no longer declared for the
  code using GAOL.** GAOL 4 declared `two_pi`, `pi`, `half_pi`, `pi_dn`,
  `pi_up`, `half_pi_dn`, `half_pi_up`, `ln2_dn`, `ln2_up`, `two_power_51` and
  `two_power_52` in the namespace `gaol`: a program that opened it with `using
  namespace gaol;` found them among its own names, and a `pi` of its own was
  ambiguous. The bounds of π, π/2 and 2π are those of `interval::pi()`,
  `interval::half_pi()` and `interval::two_pi()` (`interval::pi().left()` is π
  rounded downward), and those of ln 2 are those of `log(interval(2.0))`. The
  library of GAOL v5 uses none of `two_pi`, `pi`, `half_pi`, `ln2_dn`, `ln2_up`
  and `two_power_51`, which are removed; `two_power_52` is a constant of
  `gaol/core/gaol_interval_core.cpp`; the four doubles around π and π/2 that the
  library does use are `gaol_pi_dn`, `gaol_pi_up`, `gaol_half_pi_dn` and
  `gaol_half_pi_up` (see the namespaces above).
- **The headers GAOL installs are the ones a program includes.** The three
  builds install the same list: `gaol/gaol`, `gaol/gaol.h`, the headers they
  include, and `gaol/parser/gaol_expression.h` and
  `gaol/parser/gaol_expr_eval.h` for the expressions. They install no header of
  GAOL's sources: neither `gaol/parser/gaol_interval_parser.h`
  (`gaol/gaol_interval_parser.h` in GAOL 4), which did not compile when
  included, nor `gaol/gaol_init_cleanup.h`, both of which GAOL 4 installed, nor
  `gaol/gaol_core_math.h` and `gaol/gaol_u128.h`, which no installed header
  includes (nor `gaol/gaol_double_op.h`, see above). Each installed header
  compiles alone with `-std=c++11` and the flags of `gaol.pc`
  (`.github/scripts/headers.sh` checks it on the installed GAOL), but four:
  `gaol/core/gaol_interval_fpu.h`, `gaol/core/gaol_interval_neon.h` and
  `gaol/core/gaol_interval_sse.h`, which other headers include in the middle of
  their code, and `gaol/gaol_fpu_msvc.h`, for Visual C++ only.
- **The expressions and the reader of strings are in `gaol/parser/`.** GAOL 4
  installed the headers of the expressions (`gaol_expression.h`,
  `gaol_expr_eval.h`, `gaol_expr_visitor.h`, `gaol_eval_stack.h`) and of the
  reader of strings (`gaol_parser.h`) in `include/gaol/`, with the sources of
  the expressions, the lexer and the parser in `gaol/`. The sources are in
  `gaol/parser/` now (with `regenerate_parser.sh`), and the three builds
  install the five headers in `include/gaol/parser/`: a program that builds
  expressions includes `gaol/parser/gaol_expression.h`, with
  `gaol/parser/gaol_expr_eval.h` to evaluate them, where it included
  `gaol/gaol_expression.h`. No header is left in `include/gaol/` to include the
  new ones, and `gaol/gaol`, which did not include the expressions in GAOL 4,
  does not either.
- **`gaol/gaol_interval.h` is split in two, and the headers of the intervals are
  installed in `include/gaol/core/`.** GAOL 4 declared the class `interval`, its
  functions and the namespace `gaol` in `gaol/gaol_interval.h`, defined them in
  `gaol/gaol_interval.cpp`, and installed `gaol_interval_fpu.h` and
  `gaol_interval_sse.h` beside it in `include/gaol/`. The class, the functions
  and the helpers of `gaol_core` are declared in
  `gaol/core/gaol_interval_core.h` and defined in
  `gaol/core/gaol_interval_core.cpp` now, which includes the code of the SSE2,
  NEON and FPU intervals; `gaol/gaol_interval.h` and `gaol/gaol_interval.cpp`
  hold the namespace `gaol` alone (its using-declarations, its `pow` and
  `textToInterval()`), as `gaol/gaol_ieee1788.h` and `gaol/gaol_ieee1788.cpp`
  hold `gaol_ieee1788`. The three builds install `gaol_interval_core.h`,
  `gaol_interval_fpu.h`, `gaol_interval_neon.h` and `gaol_interval_sse.h` in
  `include/gaol/core/`, as their sources are in `gaol/core/`, and no header is
  left at the old place. `gaol/gaol_interval.h` stays in `include/gaol/` and
  includes `gaol/core/gaol_interval_core.h`, so that a program that includes it,
  or `gaol/gaol`, is not touched by the move (see [Using
  GAOL](using.md#the-headers)).
- **The timing of GAOL 4 is gone: `gaol/gaol_profile.h` and
  `gaol/gaol_profile.cpp`.** `gaol/gaol` no longer includes the header, and
  the three builds neither install it nor compile the source. The class
  `timepiece` and the functions `get_time()`, `reset_time()`,
  `elapsed_time()` and `intermediate_elapsed_time()` leave the interface,
  and so does the macro `HAVE_GETRUSAGE` of
  `gaol/gaol_configuration.h`, which nothing read but
  `gaol/gaol_profile.cpp`. A program measures the time with the clocks of
  the C++ standard library, as `examples/16_Goldstein_Price.cpp` does.
- **`gaol/gaol_parameters.h` and `gaol/gaol_limits.h` are gone.** GAOL 4
  installed both. The first declared an empty namespace `gaol` ("None at
  present"); the second included `<limits>`, or, where the configuration did
  not find it, declared a `std::numeric_limits` of its own for `double` and
  `int`. A program that included them includes `<limits>` itself.
- **`gaol/gaol_assert.h` and `GAOL_ASSERT` are gone.** GAOL 4 installed the
  header, and its manual documented the macro: with `GAOL_DEBUGGING` it was
  `assert()`, with `NDEBUG` undefined to keep it active, and it was nothing
  otherwise. GAOL used it only in `ipow_up()` and `ipow_dn()`, which are gone
  too (see above). A program that wants the check writes `assert()`;
  `GAOL_DEBUGGING` only turns on `GAOL_DEBUG_VERBOSE`, and the Debug builds
  define no `NDEBUG` (see below).
- **`gaol/gaol_exact.h`, `get_inexact()` and `clear_inexact()` are gone**, with
  the C file that defined them and `gaol/sysdeps/`, which held the versions for
  C99 and for Visual C++. GAOL 4 installed them, and its lexer called them to
  know whether `strtod()` had rounded a number. GAOL v5 reads numbers exactly,
  without the inexact flag (see above), so nothing called them any longer: the
  two C functions are no longer in `libgaol`, nor exported by its shared
  library.
- **`gaol/gaol_flags.h` and `s_nextafter()` are gone.** `gaol_flags.h` declared
  a single class, `prec_t`, the priorities of the operators of the expressions,
  which only `gaol/parser/gaol_expression.cpp` uses: it is in
  `gaol/parser/gaol_expression.h` now, in `gaol_core` (`gaol` does not name it).
  `s_nextafter()`, the function of `gaol/s_nextafter.c`, which GAOL 4 compiled
  into its library and which `gaol/gaol_port.h` substituted for `nextafter` with
  Visual C++ where the configuration found none, has no caller since that
  substitution is gone (see below).
- **`gaol/gaol_allocator.h` is gone**, with `gaol::aligned_allocator`, an
  allocator of the containers of the standard library that aligned memory on
  16 bytes for the SSE2 intervals. `gaol/gaol` included it with SSE3 and the
  builds installed it; neither GAOL, Codac nor IBEX used it. The `operator new`
  of the SSE2 intervals still aligns their memory (`GAOL_MEMALIGN` and
  `GAOL_MEMFREE` of `gaol/gaol_port.h`), but for Visual C++ and clang-cl on
  32-bit Windows, whose intervals need none.
- **`__GAOL_PUBLIC__` is `GAOL_PUBLIC`, and is empty with Visual C++ and
  clang-cl**, with which GAOL is always a static library. GAOL 4 declared its
  functions `__declspec(dllimport)` there, or `__declspec(dllexport)` with
  `_COMPILING__GAOL_PUBLIC__`, which no build defined, so that the code using
  GAOL had to define `__GAOL_PUBLIC__` empty. GAOL v5 needs no definition: the
  CMake build, the meson build, `gaol::gaol`, `gaol_dep` and `gaol.pc` define
  none, no header reads `__GAOL_PUBLIC__` any longer (a definition left in a
  project changes nothing), and a program may define `GAOL_PUBLIC` before
  including GAOL to replace it. With GCC and Clang nothing changes:
  `GAOL_PUBLIC` is the default visibility where the library is compiled with
  `-fvisibility=hidden`, so that the shared library exports the declarations,
  and nothing otherwise.
- **`gaol/gaol_exceptions.h` no longer declares `string` and `exception` in the
  global namespace, and the macros of GAOL's headers start with `GAOL_`**, so as
  not to meet the names of the program or of another library.
  - **Names.** `string` and `exception` were there by `using std::string;` and
    `using std::exception;`: a program that counted on them writes `std::string`
    and `std::exception`.
  - **Macros.** `INLINE`, `MEMALIGN`, `__HI`, `__LO`, `HAVE_FENV_H`,
    `USING_SSE2_INSTRUCTIONS`, `IX86_LINUX`, `WORDS_BIGENDIAN`,
    `ULONGLONGINT`... are now `GAOL_INLINE`, `GAOL_MEMALIGN`, `GAOL_HI`,
    `GAOL_LO`, `GAOL_HAVE_FENV_H`, `GAOL_USING_SSE2_INSTRUCTIONS`,
    `GAOL_IX86_LINUX`, `GAOL_WORDS_BIGENDIAN`, `GAOL_ULONGLONGINT`..., in the
    sources, in the three `gaol/gaol_configuration.h` and in the builds. The
    macros that `gaol/gaol`, `gaol/parser/gaol_expression.h` and
    `gaol/parser/gaol_expr_eval.h` define, beyond those of the system headers
    they include, all start with `GAOL_` or `gaol_`
    (`.github/scripts/headers.sh`).
  - **`_XOPEN_SOURCE`** is no longer redefined to 600 by `gaol/gaol_port.h` (x86
    and ARM on Linux): g++ and clang++ define `_GNU_SOURCE`, under which
    `<stdlib.h>` declares `posix_memalign()`.
  - **`nb_fp_numbers()`** returns an `unsigned long long`, where GAOL 4 returned
    `ULONGLONGINT`, `unsigned long` where `long` has 64 bits.
- **The headers compile without a warning** under `-Wall -Wextra`, and under
  `-Wold-style-cast -Wundef`, in the code that includes them. With `-Wextra`,
  every assignment `x = y` of intervals warned that the implicit assignment was
  deprecated, the class having a copy constructor of its own: `interval&
  operator=(const interval&) = default;` is declared (the class is still not
  trivially copyable). The continuous integration compiles GAOL, its tests and
  its examples with `-Wall -Wextra -Werror`, with GCC and Clang.
- **The constructor from one string, `interval(const char*)`, is gone.**
  `gaol::textToInterval()` reads the string instead, with the same names of
  functions, and throws the same `input_format_error` for a string that is no
  interval: `interval x("0.1")` becomes `interval x = textToInterval("0.1")`.
  The constructor took any `const char*`, and converted it implicitly:
  `interval(0)` was ambiguous, 0 being a null pointer as well as a number, and
  `x + nullptr` compiled, and crashed. `tests/numbers.cpp` checks that
  neither a `const char*` nor `nullptr` converts to an interval, and that
  `interval(0)` is [0].
- **The constructor from two strings, `interval(const char*, const char*)`,
  is gone** too: `interval("0.1", "0.3")` becomes
  `textToInterval("0.1", "0.3")`, which takes the left bound of the first
  string and the right bound of the second, and gives the empty set for
  bounds in the wrong order. The constructor kept the bounds [2, 1] of
  `interval("2", "1")`, which `is_empty()` called empty, but whose sum with
  [0, 1] was [2, 2] (review #6 of `examples/examples.md`, checked by
  `tests/expressions.cpp`). The literal 0 being a null pointer too, it made
  `interval(0, 0)` ambiguous, which now compiles.
- **The constructor from one `double`, `interval(double)`, is explicit**, and
  so are `expression(double)` and `expression(const interval&)`. A `double`
  becomes an interval where the program writes `interval(d)`:
  `interval x = 0.5;`, `return 0.5;` in a function returning an interval,
  `min(x, 0.5)`, `hypot(3.0, x)` and `interval r = sqrt(2.0);` do not
  compile, where GAOL 4 took the double for a point interval without saying
  so, the `sqrt(2.0)` of the C library among them; `gaol::sin(0.5)`, which
  was ambiguous between the sin of an interval and that of an expression,
  does not compile either. The assignment `x = d`, `x &= d` and `x |= d`,
  and the relations `<`, `<=`, `>` and `>=` between an interval and a
  `double`, in either order, have overloads of their own, which take
  `interval(d)` as the conversion did, empty for an infinite d or a NaN:
  `x = 0.5` and `x < 0.0` still compile, and give the same results. So do the
  arithmetic operators of an interval and a `double` (`x + 0.5`, `x *= 2.0`)
  and `set_contains(d)`. `interval(double, double)` is not explicit.
  `tests/other_functions.cpp`, `tests/expressions.cpp` and
  `tests/ieee1788.cpp` check that a `double` converts neither to an interval
  nor to an expression, `tests/assignment.cpp` and `tests/relations.cpp` the
  overloads of a `double`.
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
  matching the one that allocated it (`GAOL_MEMFREE()` in `gaol/gaol_port.h`):
  `_aligned_malloc()` and `_aligned_free()` on Windows, where `malloc()` aligns
  on 8 bytes only on 32-bit systems, so that the SSE2 intervals can be used with
  MinGW-w64 and MSYS2 on x64. The placement `delete` of `interval` and
  `interval2f`, called when a constructor throws (`interval("1", "x")`), freed
  the caller's memory; it now leaves it alone.
- **Visual C++ and clang-cl without `/fp:strict`** are refused by
  `gaol/gaol_config.h`, as `/fp:fast` is. Built with `/fp:strict` for GAOL and
  without it for the tests, `rounding_direction` and `other_functions` crashed
  with Visual Studio 2022 on x86 and x64: `round_upward_if_needed()` initialized
  its 2^-60 as `1.0/2^60`, which Visual C++ computed when the function first ran
  with `/fp:strict`, and at compile time, in read-only memory, without; the
  linker kept one copy, and GAOL wrote into it. 2^-60 is now a literal, and the
  tests pass so. Built without `/fp:strict` everywhere, they passed too, but
  Visual C++ then assumes rounding to nearest, and nothing certifies the bounds.
  clang-cl from Clang 16 defines the same macros of the floating-point model as
  Visual C++ (`_M_FP_STRICT` with `/fp:strict` alone, `_M_FP_PRECISE` by
  default): without `/fp:strict` it compiles with
  `-fno-rounding-math -ffp-contract=on`, assuming rounding to nearest as Visual
  C++ does, and was not refused. Before Clang 16 it does not tell its model, and
  is not checked.
- **clang-cl for 32-bit x86 is given `-msse2`.** clang-cl 18 computes the
  doubles on the x87 unit there, even with `/arch:SSE2` (`_M_IX86_FP` is 1 and
  `__SSE2_MATH__` is not defined), which `gaol/gaol_config.h` refuses, CORE-MATH
  rounding some results twice. `-msse2` makes it compute them in SSE2
  (`_M_IX86_FP` 2, `FLT_EVAL_METHOD` 0), as Visual C++ does by default. CMake
  and meson give it to GAOL and, with the other flags of interval arithmetic, to
  the code using GAOL (`gaol::gaol`, `gaol.pc`).
- **`-ffinite-math-only` is refused** by `gaol/gaol_config.h`, as `-ffast-math`
  and `/fp:fast` are; GAOL 4 refused none of them. `-ffast-math` and `-Ofast`
  turn it on, and the compiler then takes NaN and infinities never to occur, in
  the inline functions of the headers too: the empty interval has NaN bounds,
  `is_empty()` reads it as `!std::islessequal(left(), right())`, and
  `([1, 2] & [3, 4]).is_empty()` is false once the compiler has folded the test
  away (GCC 9.4 and 13 at `-O0`, `-O2` and `-O3`; Clang 18 at `-O0`, and at
  `-O2` and `-O3` when the bounds are `volatile`). The check reads
  `__FINITE_MATH_ONLY__`, so it also refuses what `__FAST_MATH__` does not show:
  with Clang, `-Ofast` or `-ffast-math` followed by `-frounding-math` leave
  `__FAST_MATH__` undefined and `__FINITE_MATH_ONLY__` at 1.
  - **The remedy** is the flags of interval arithmetic that `gaol.pc` and
    `gaol::gaol` give, after the option: `-fno-fast-math` turns `-ffast-math`
    and `-ffinite-math-only` off when it comes after them, and does nothing
    when they come after it, where the compilation stops. With Clang,
    `-ffast-math` and `-Ofast` turn `-frounding-math` off too, which
    `-fno-fast-math` does not turn on again, and Clang 18 compiled GAOL's
    operations as if rounding to nearest (54 of the 292728 checks of
    `tests/rounding_direction.cpp` failed after `-ffast-math -fno-fast-math`,
    126 after `-Ofast -fno-fast-math`, none with all the flags after the
    option).
  - **What no macro shows is not refused**, and the code using GAOL is not to
    be compiled with it: `-funsafe-math-optimizations` and
    `-ffast-math -fno-finite-math-only` with GCC, which rewrite the addition by
    which GAOL tells the rounding direction, so that an operation does not set
    the direction upward again after the code using GAOL left it to nearest, and
    `width()` is below the exact width; `-fno-signed-zeros` with GCC and Clang,
    which drops the `+ 0.0` of the addition that tells the modes flushing the
    subnormals to zero; and `-fno-honor-nans` alone with Clang, which does to
    the empty interval what `-ffinite-math-only` does (see [Compilers and
    options refused](three-builds.md#compilers-and-options-refused)).
  - **Tests.** The CMake tests `refused_finite_math_only` and
    `refused_fast_math` compile `tests/refused_options.cpp` with each option
    after the flags of interval arithmetic and check that the header refuses it,
    `refused_positive` that it compiles without them (GCC and Clang);
    `tests/refused_options.sh` does the same with configure and meson.
- **The compilers that do not honour the rounding direction are refused**,
  the C compiler as well as the C++ compiler, by the three builds when they
  configure (see [Compilers and options
  refused](three-builds.md#compilers-and-options-refused)): Clang for 32-bit
  ARM processors, and any compiler that says of `-frounding-math` "overriding
  currently unsupported rounding mode on this target", as Clang 14 does for
  64-bit ARM. GAOL 4 accepted them; built by Clang 21 for 32-bit ARM, GAOL v5
  gave 4556 of 16000 random products, squares and cubes that did not enclose
  their exact values. The C compiler builds CORE-MATH and the C++ compiler
  the rest of GAOL, and neither may be one of them: `CC=clang CXX=g++` on
  armhf is refused. `gaol/gaol_config.h` refuses Clang for 32-bit ARM again
  when the code including GAOL's headers is compiled.
- **`GAOL_INFINITY` is `std::numeric_limits<double>::infinity()`**, a constant
  of the compiler, where GAOL 4 took the `HUGE_VAL` of the C library. The UCRT
  of Windows (SDK 10.0.26100) writes `HUGE_VAL` as `((double)(float)1e+300)`:
  Visual C++ makes the conversion when compiling, but clang-cl under
  `/fp:strict` makes it when the program runs, in the rounding direction of the
  moment, and got `FLT_MAX` when that direction was downward or toward zero,
  with the overflow flag. `interval()` was then `[-FLT_MAX, FLT_MAX]`,
  `interval(1e300)` was empty and `[1] / [-1, 1]` did not contain 1e300;
  `GAOL_INFINITY` being in the inline code of the public headers, the program
  using GAOL got the same value.
- **GCC for Windows is given the fused multiply-add instructions** (MinGW-w64,
  MSYS2 and Cygwin, on x86-64 and on 32-bit x86): the three builds give it
  `-mfma -mprefer-vector-width=128` under the conditions of the other compilers,
  and a GCC older than 8, which has no `-mprefer-vector-width`, none. It had no
  flag: `-mfma` turns AVX on, GCC then vectorizes with 256-bit registers and
  spills them to a stack it takes to be aligned on 32 bytes, where the ABI of
  Windows x64 guarantees 16 (GCC bug 54412, which crashed the MinGW builds of
  Codac and is fixed in GCC 17), so that CORE-MATH called a software `fma()` and
  the integer powers had no FMA kernel. With the second flag `objdump` finds no
  `%ymm` in the objects of GAOL, CORE-MATH and the tests, against 30598 lines
  with `-mfma` alone (GCC 16.2, MSYS2 UCRT64), and a step of the continuous
  integration fails on any of them in the objects built for x86-64. On 32-bit
  x86 the defect was not shown to act, and the two options go together so that
  the objects hold no 256-bit register either (4709 lines with `-mfma` alone,
  none with both, GCC 15.2; GCC 13.2 leaves 9 lines in two test objects with
  both, which the step reports without failing). `-mstackrealign`, for the
  4-byte stack of the threads of Windows, changed no time with GCC 15.2, and
  GAOL does not give it. On the runner of GitHub (same compiler, x86-64),
  `pow(x, 3)` takes 8.7 ns rather than 52.3, `log(x)` 33.6 rather than 106.4 and
  `cos(x)` 55.4 rather than 260.5; with MinGW-w64 15.2 for 32-bit x86, 12.1 ns
  rather than 70.5, 81.9 rather than 265.7 and 138.3 rather than 489.6; `x * y`
  and `x / y` change by 7 % at most, within the noise of the runner. The
  library, and the code using it through `gaol::gaol` or `gaol.pc`, then need a
  processor with the FMA and AVX instructions (Intel Haswell, AMD Piledriver,
  2012-2013, and later), as with the other x86 compilers; `-DGAOL_FMA=OFF`, a
  setting of the developers, builds for any x86 processor with a MinGW-w64 that
  links the UCRT (MSYS2 MINGW32, which links `msvcrt.dll`, is refused without
  the instructions). The 256-bit intrinsics a program writes itself stay exposed
  to the defect.
- **Cygwin on x86-64 is built and tested**, with GCC, by the three builds in the
  continuous integration, and **it is refused without the fused multiply-add
  instructions**: the `fma()` of its C library is not correctly rounded
  (`fma(1 + 2^-30, 1 - 2^-30, -1)` is 0 rather than -2^-60) and CORE-MATH
  computes with `fma()`, so `gaol/gaol_config.h` stops the compilation without
  `-mfma`, which the builds give it as they do to the other GCC for Windows. The
  exact products of GAOL (`fma_once()`) and the program that the configuration
  runs to know whether the machine runs `-mfma` call `__builtin_fma`, the
  instruction at every level of optimization, where `std::fma` without
  optimization is a call of that `fma()`. Three more things came with it:
  - **`previous_float()` and `next_float()`** give `-oo` and `+oo` at `-DBL_MAX`
    and `DBL_MAX` with every C library, where GAOL 4 called `nextafter()` as it
    came. The `nextafter()` of Cygwin computes that overflow as `x + x` in the
    rounding direction of the program, upward for GAOL: `nextafter(-DBL_MAX,
    -oo)` was `-DBL_MAX`, and the lower bound of `sinh(-711)`, whose value is
    below `-DBL_MAX`, did not enclose it.
  - **The aligned memory** of the SSE2 intervals is `_mm_malloc()` and
    `_mm_free()` of GCC there: the `<stdlib.h>` of Cygwin does not declare
    `posix_memalign()` with `-std=c++11`.
  - **`package_source` is not available there**: in the job of the continuous
    integration (CMake 4.4.4, 23 October 2026) CPack, with TGZ, TBZ2 and ZIP
    alike, failed to add `README`, a symbolic link to `README.md`, to the
    archive ("Couldn't list extended attributes"). The cause was not found, and
    `tests/cpack_stale_configure.cmake` says it is skipped.
- **The intervals of floats are compiled by a developer of GAOL only**:
  `gaol::intervalf`, intervals of floats computed on the x87 unit, and
  `gaol::interval2f`, two of them in an SSE3 register, were compiled and
  installed by every build, and `gaol/gaol` included their headers. Neither
  IBEX nor Codac uses them, the manual leaves them undocumented, and they are
  unfinished: `sqrt(intervalf)` returns its argument, `interval2f::inverse()`
  aborts, and `pow(interval2f, int)` does not handle the empty set. Their
  code is within `#ifdef GAOL_FLOAT_INTERVALS`, a macro that no build
  defines and that no build has an option for: the options
  `GAOL_FLOAT_INTERVALS` of CMake, `--enable-float-intervals` of configure
  and `enable-float-intervals` of meson, which compiled them for a while,
  are gone. A developer working on them defines the macro in
  `gaol/gaol_config.h`: `gaol/core/gaol_interval_core.cpp` then includes their
  sources, `gaol/float/gaol_intervalf.cpp` and `gaol/float/gaol_interval2f.cpp`,
  as it includes those of the SSE2 and FPU intervals. Their headers, in
  `gaol/float/` too with `gaol_generator_mul_i2f.py`, the script that writes the
  cases of the product of `interval2f`, are not installed, `gaol/gaol` does not
  include them, and their tests, `tests/intervalf.cpp` and
  `tests/interval2f.cpp`, are skipped.
- **The archive of the sources and the packages are made by CMake**
  (CPack, see [Building GAOL](building.md#the-archive-of-the-sources-and-the-packages)):
  `cmake --build <build> --target package_source` gives `gaol-<version>.tar.gz`,
  the files of the repository less the notes of the work on GAOL v5 (`TODO.md`,
  `process.md` and `todo-notes/`) and the files of the maintainers' tools, and
  `--target package` an archive of what `cmake --install` installs and, on
  Linux, `libgaol-dev_<version>_<arch>.deb`.
  `make dist` and `make distcheck` of the autotools build are gone
  (`no-dist` in `configure.ac`), with the lists of files they put in the
  archive and `make rpm`, which built an RPM from it: `make dist` stopped on
  `m4/libtool.m4`, which it could not copy. The `gaol.pc` of the CMake build
  finds its directories from where it is (`${pcfiledir}`), where it named
  those of `CMAKE_INSTALL_PREFIX`, so that it stays right in the archive and
  the `.deb`, or after `cmake --install --prefix`.
  The archive holds `configure` as it is committed, and `configure --version`
  gives the version autoconf read from `VERSION.txt` when it generated it: after
  a change of `VERSION.txt` without a new generation of `configure`, the archive
  of the new version held a `configure` that said the old one, which only the
  autotools job of the continuous integration reported. CMake now warns, while
  it configures GAOL as the project built, naming both versions (nothing is
  said without `configure`, or without the line `PACKAGE_VERSION=`), and
  `package_source` configures the build directory again before it makes the
  archive when `VERSION.txt` holds another version than the one it was
  configured for, whatever the generator (`cmake/gaol_package_source.cmake`,
  which CPack includes): the Makefile generators ran CPack at once, and the
  archive was named after the old version and held the new `VERSION.txt`
  (`tests/cpack_stale_configure.cmake`; see
  [Building GAOL](building.md#the-version-of-gaol)). CPack reads the regular
  expressions of `CPACK_SOURCE_IGNORE_FILES` with their backslashes
  (`CPACK_VERBATIM_VARIABLES`): CMake 3 warned of them, and CMake 4.4 read `\.`
  as `.`, which matches any character, so that a file such as `examples/halo`
  would have been left out of the archive.
- **`is_finite()`** is `std::isfinite()`, in every build: `finite()` of the C
  library was used where the build system found it, and is not declared by
  every C library.
- **The meson build** did not compile on Linux without `GETRUSAGE_IN_HEADER`,
  which configure defined: `gaol/gaol_profile.cpp` declared `getrusage()`
  itself, against the declaration of the C library. The file is gone (see
  above), and the macro with it. meson installs the headers for MinGW and
  Visual C++, as configure now does too. Both install
  a `gaol.pc` carrying the flags of interval arithmetic. The meson build no
  longer requires flex and bison: it compiles the lexer and the parser
  committed, as CMake does, and its generators, whose output nothing compiled,
  are gone.
- **The three builds are alike** (GAOL v5, see [Building GAOL](building.md)
  and [The three builds](three-builds.md)):
  - **The same macros in `gaol/gaol_configuration.h`**, and nothing else (but
    for a setting of the developers that CMake alone writes, see [Building
    GAOL](building.md#the-configuration-of-gaol)). configure and meson checked
    some fifty headers, functions, types and sizes nothing read
    (`AC_FUNC_MALLOC` and `AC_FUNC_REALLOC` among them, which replaced
    `malloc()` and `realloc()` by `rpl_malloc()` and `rpl_realloc()` in a
    cross-compilation), and the header of configure defined `PACKAGE`, `VERSION`
    and `PACKAGE_*`, which `gaol/gaol_config.h` undefined before including it,
    undefining those of the code including GAOL. The template of configure,
    `gaol/gaol_configuration.h.in`, is written by hand.
  - **The macros that the installed headers read with `#if` are 1 or 0, never
    undefined**, so that GAOL's headers compile under `-Wundef -Werror`: the
    options of `gaol/gaol_configuration.h` (`GAOL_PRESERVE_ROUNDING`,
    `GAOL_DEBUGGING`, `GAOL_VERBOSE_MODE`, `GAOL_USING_SSE2_INSTRUCTIONS`, the
    `GAOL_HAVE_*`...) and the macros of the target in `gaol/gaol_config.h`,
    `gaol/gaol_fpu.h` and `gaol/gaol_fpu_fenv.h` (`GAOL_WORDS_BIGENDIAN`,
    `GAOL_RND_SSE_REGISTER`, `GAOL_IX86_LINUX`...) are defined to 1 where they
    hold and to 0 elsewhere, with the three builds. GAOL 4 left its options
    undefined when they were off, and its headers read them with `#if`, which
    warns under `-Wundef`. A program has to test these macros with `#if` as
    well, not with `#ifdef`: `#ifdef GAOL_PRESERVE_ROUNDING` is now true
    whatever the build. `.github/scripts/headers.sh` compiles each installed
    header with `-Wundef` (see [Building
    GAOL](building.md#the-configuration-of-gaol)).
  - **The same options** (see [The options](building.md#the-options)):
    `GAOL_PRESERVE_ROUNDING`, `GAOL_SIMD`, `GAOL_COVERAGE`, `WITH_TESTS`,
    `WITH_EXAMPLES`, `BUILD_SHARED_LIBS`, the build type, the
    position-independent code and `CMAKE_OSX_ARCHITECTURES` have their
    counterpart in configure (`--enable-preserve-rounding`, `--enable-simd`,
    `--enable-coverage`, `--with-tests`, `--with-examples`, `--enable-shared`,
    `--enable-debug`, `--enable-pic`) and in meson (`enable-preserve-rounding`,
    `enable-simd`, `enable-coverage`, `with-tests`, `with-examples`,
    `default_library`, `buildtype`, `b_staticpic`); a universal build of macOS
    is made with configure and meson as with CMake, by several `-arch` in the
    flags. Each part of a universal build computes with the intervals of its
    architecture, NEON for `arm64` and SSE2 for `x86_64`, where `GAOL_SIMD`
    allows them (`GAOL_UNIVERSAL_NEON` and `GAOL_UNIVERSAL_SSE2` of
    `gaol_configuration.h`, which the header turns into the macros of the
    architecture being compiled); the fused multiply-add instructions and the
    AVX-512 path are left out of it, the probe that decides them failing for one
    of the two architectures (see [Building GAOL](building.md#the-options)).
    With the generator of Xcode and no `CMAKE_OSX_ARCHITECTURES`, which would
    build `arm64` and `x86_64` while the configuration of GAOL is that of one
    architecture, CMake gives Xcode the architecture of the machine; the list
    `arm64;x86_64` asks for the universal build.
    `GAOL_SIMD` chooses the intervals of every processor: SSE2 on x86,
    NEON on 64-bit ARM (`GAOL_ARM64`, `--enable-arm64` and `enable-arm64` were
    an option of their own and are gone), the FPU if it is off. The rest is
    decided by the configuration, the same with the three builds: by checks that
    compile, link and run a program for the fused multiply-add instructions and
    the AVX-512 path (taken with `GAOL_PRESERVE_ROUNDING`, where it computes 2.8
    times faster at the median on an Intel i7-1185G7, and left out without it,
    where it costs 17 %; see [SIMD, FMA and the rounding
    direction](simd.md#when-the-build-takes-the-avx-512-path)), by
    `gaol/gaol_u128.h`, which a check compiles, for the 128-bit integer, and by
    the build type for the debugging.
    `GAOL_FMA`, `GAOL_PREFER_AVX512`, `GAOL_U128_EMULATION`, `GAOL_DEBUG`,
    `GAOL_EXCEPTIONS_ENABLED` and `GAOL_VERBOSE_MODE`, and the options of
    configure and meson that matched them (`--enable-fma`,
    `--enable-prefer-avx512`, `--enable-verbose-mode`, `--disable-exceptions`,
    `enable-debug` of meson...), are no longer options of the user, `configure
    --help` not listing them and `meson configure` marking them
    \[developers\], the developers of GAOL keeping them as settings (see
    [Building GAOL](building.md#the-settings-of-the-developers)); in a
    cross-compilation, where no program can be run, the fused multiply-add
    instructions and the AVX-512 path are left out unless the builder says
    that the machine runs them (`GAOL_MACHINE_RUNS_FMA`,
    `gaol_cv_machine_runs_fma`, `machine-runs-fma`): the fused multiply-add
    instructions were taken on the word of the compiler there.
  - **The C sources of CORE-MATH** are compiled with `-std=gnu99` (the C99
    that the inline assembly of `atan2` needs, with the extensions of GNU),
    `-msse2 -msse3` where the SSE2 intervals are used and
    `-fvisibility=hidden` by the three builds, and with `-ffloat-store` where
    the x87 unit computes the doubles. Before, CMake alone gave the second and
    the third flags, configure left the standard to the compiler (C23 with
    GCC 15) and meson took `gnu11`, and the audit of the three builds compared
    the flags of the C++ sources only (see [The three builds](three-builds.md)).
    A shared `libgaol` exports from these C sources only the 38 functions
    `gaol_cr_*` of `gaol/gaol_core_math.h`, the same with the three builds.
  - **The Debug build** of each (`CMAKE_BUILD_TYPE=Debug`,
    `configure --enable-debug`, `meson setup --buildtype=debug`) compiles
    GAOL with `-g`, without optimization nor `NDEBUG`, so that `assert()`
    stays active. `--enable-debug` added `-g -ansi -Weffc++
    -pedantic` for `g++` alone to the optimizations of `--enable-optimize`,
    and GAOL did not compile with it: `gaol/parser/gaol_expression.h` wrote on
    `std::cout` in `GAOL_DEBUG(level, command)` (now `GAOL_DEBUG_VERBOSE`)
    without including `<iostream>` (`tests/debugging.cpp`). meson's option
    `enable-optimize` is gone: its build type gives the optimizations and the
    debugging, as CMake's. So is the `--enable-optimize` of configure, which
    gave `-O3` (`-O` with `--disable-optimize`): the Release build does. The
    `enable-debug` of meson is a setting of the developers now (see below).
  - **GAOL as a subproject of meson.** GAOL can be a subproject of a meson
    project (`dependency('gaol', fallback: ['gaol', 'gaol_dep'])`, see [Using
    GAOL](using.md#from-pkg-config)); `meson setup` stopped on the
    `add_global_arguments()` that `meson.build` called, which meson refuses in
    a subproject. `meson.build` gives its flags with `add_project_arguments()`
    now, `gaol_dep` gives the code using GAOL the flags of interval arithmetic
    and the link option of `gaol.pc`, and a subproject declares no test setup
    nor target `check`, which would clash with those of the project. GAOL is
    built in release there, as it is alone: meson before 1.8.4 gives a
    subproject the build type of the project, `debug` by default, and GAOL
    compiles its libraries as in release all the same, except with Visual C++
    and meson before 0.57 and with the Visual Studio backend, which keep the
    build type of the project. `tests/meson_subproject` builds GAOL so and
    checks its flags.
  - **CMake without a build type**, with `add_subdirectory` or FetchContent and
    a generator of one configuration: `libgaol` had `-O3` alone and CORE-MATH no
    optimization at all; both are compiled as in Release now, with `-O3` (`/O2`
    with Visual C++), `NDEBUG` and the optimizations of configure
    (`tests/release_flags.py` checks the commands). With a generator of several
    configurations (Visual Studio, Xcode, Ninja Multi-Config), GAOL built alone
    puts Release first among them unless `CMAKE_CONFIGURATION_TYPES` is given:
    Ninja Multi-Config builds it by default.
  - **`GAOL_DEBUG` (CMake) and `enable-debug` (meson)**, settings of the
    developers (see [Building
    GAOL](building.md#the-settings-of-the-developers)), build `libgaol` and
    CORE-MATH for debugging whatever the build type or the configuration, a
    project bringing GAOL in included: no optimization, no `NDEBUG`,
    `GAOL_DEBUGGING`. The tests, the examples and the code of that project keep
    their own flags. The macro `GAOL_DEBUG(level, command)` of the debug build
    is named `GAOL_DEBUG_VERBOSE`, `GAOL_DEBUG` being that setting of CMake; the
    old name is not kept.
  - **The tests and the examples:** `WITH_TESTS` and `WITH_EXAMPLES` of
    CMake (`GAOL_BUILD_TESTS` before), `--with-tests` and `--with-examples`
    of configure, `with-tests` and `with-examples` of meson (`with-test`
    before) build them, `make test` runs the unit tests and `make check` the
    unit tests and the examples, with the three builds. The unit tests of
    GAOL 4, which were in `check/` and needed CppUnit, are in `tests/`, and
    run without it (see [Tests](tests.md)).
  - **`make perf`** measures GAOL v5 on the benchmark of
    [performance.md](compare/performance.md) and writes its times into the
    tables, the other libraries keeping theirs; it ran the benchmark of GAOL
    4 (`check/performances.cpp`) with autotools, and meson had an option
    `check-perf`.
  - **`make parser`** regenerates the lexer and the parser with the flex and
    the bison of the system, as `gaol/parser/regenerate_parser.sh` does. The rules of
    `gaol/Makefile.am` that ran flex and bison where make found the sources
    newer, and `AC_PROG_YACC`, are gone, and so is the deletion of the lexer
    and the parser by `make maintainer-clean`, which no build could make
    again without flex and bison.
  - **The shared library** is `libgaol.so.5.0.0`, whose soname is
    `libgaol.so.5`, with configure (`libgaol-5.0.so.0` before), meson (no
    version before) and CMake, which builds it with `BUILD_SHARED_LIBS` (a
    static library only before). The three builds make the static library
    alone by default, compiled as position-independent code, and the shared
    one when asked (`BUILD_SHARED_LIBS=ON`, `--enable-shared`,
    `-Ddefault_library=shared`); configure and meson made both, so that
    `make install` and `meson install` put `libgaol.a` alone under the prefix
    unless asked. A shared `libgaol` exports the classes and
    functions of the expressions (`gaol/parser/gaol_expression.h`), which it did not.
  - **meson with Visual C++** installs `gaol/gaol_configuration.h` in the
    directory of the headers (in `include/gaol` under the prefix before,
    whatever `includedir`) and `gaol_fpu_msvc.h`, and builds a static
    library, as CMake, `GAOL_PUBLIC` being empty with Visual C++ (nothing
    defines `__GAOL_PUBLIC__` any more); it no longer requires a math library
    apart and gives `/fp:strict` to CORE-MATH too. `nextafter` and `isnan` are
    no longer redefined for Visual C++ (`s_nextafter`, `_isnan`) where the
    build did not define `HAVE_NEXTAFTER` and `HAVE_ISNAN`, which only CMake
    did.
  - **Clang for the target of Visual C++**, that is `clang++` with the driver
    of GCC for `x86_64-pc-windows-msvc` or `i686-pc-windows-msvc` (not
    clang-cl), is supported by the three builds. It defines `_MSC_VER`, which
    they read (`CMAKE_CXX_SIMULATE_ID` for CMake), and is treated as Visual C++
    is: the libraries are compiled in C++14, where the builds asked for C++11
    and the standard library of Visual C++ does not compile with it ("deduced
    return types are a C++14 extension"); the lexer gets the `unistd.h` of
    `cmake/msvc`, with the `_CRT_*` definitions, as Visual C++ and clang-cl
    do; and CMake and meson build the static library alone, a DLL exporting
    nothing since `GAOL_PUBLIC` is empty (configure builds it by default).
  - **The version is 5.0.0 everywhere**: configure gave `"5.0.0"`, its quotes
    included, to `PACKAGE_VERSION`. `configure --help` and the summary of
    meson give the address for the bug reports, jordan.ninin@ensta.fr, and
    the page of GAOL v5, https://github.com/Jordan08/GAOL.
    The version is in `VERSION.txt` alone, where GAOL 4 wrote it in
    `configure.ac`, in `meson.build` and, for Visual C++ and MinGW, in two
    headers, and the three builds read it by the same rules: the six blanks of
    ASCII around the version and a UTF-8 byte order mark at the start of the
    file (which some editors of Windows write) are ignored, a NUL byte is
    refused, and a file of UTF-16 characters is refused with a message that says
    to save it as UTF-8 or ASCII. A file that is refused gets a message that
    gives its first bytes in hexadecimal. The test `version_file` (CMake) and
    `.github/scripts/version-file.sh` (configure, meson, autoconf) check it (see
    [Building GAOL](building.md#the-version-of-gaol)).
  - **`minimum()` and `maximum()` of two zeros** give -0 and +0 with every
    compiler: Visual C++ compiled `(b <= a) ? b : a` as the instruction
    `minsd`, which gives its second operand for two zeros, and
    `minimum(0.0, -0.0)` was +0, `maximum(0.0, -0.0)` -0. The unit test of
    GAOL 4 `tests/float_functions.cpp` found it, once built with Visual C++.
  - **`--enable-asm` and `HAVE_ROUNDING_MATH_OPTION` are gone.**
    `GAOL_USING_ASM` (`--enable-asm` of configure, `enable-asm` of meson,
    `GAOL_ASM` of CMake) chose the x87 assembly of `f_negate()` on 32-bit x86
    Linux and macOS and with Visual C++, which GAOL v5 no longer calls and which
    is gone (see the namespaces above), so that the option did nothing. The macro
    `HAVE_ROUNDING_MATH_OPTION` of `gaol/gaol_configuration.h`
    (`GAOL_HAVE_ROUNDING_MATH_OPTION`, with the prefix of the other macros of
    GAOL v5) is gone too: no code of GAOL read it, and the three builds still
    test by themselves whether the compiler takes `-frounding-math` and refuse
    the compilers that do not honour the rounding direction. Passing a removed
    option changes nothing with CMake, which warns that the variable was not
    used, and with configure, which warns of an unrecognized option, but `meson
    setup -Denable-asm=true` stops with an error, the option being unknown. A
    program that tests either macro sees it undefined (`-Wundef` warns).
  - **Gone:** the `Doxyfile` of the root, which named the sources of
    Frédéric Goualard's machine, and the documentation Doxygen wrote from
    the sources (`manual/gaol_doxygen.cfg.in`, `make -C manual html`),
    `version.h`, a file of Code::Blocks of 2009, `gaol.spec.in`, of `make
    rpm`, `stamp-h.in` and `mkinstalldirs`, of old versions of automake, the
    option `--enable-relations`, and the lines of CVS and Subversion (`$Id$`,
    `$Revision$`) and "Last modified ... on pc-goualard" of the headers.
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
