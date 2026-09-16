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
  x86, 8.5 ns with glibc. GAOL changes the direction four times for each
  elementary function of an interval (to nearest before mathlib and upward
  after, for each bound), where mathlib itself takes about 10 ns. On the
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
- **Hyperbolic functions:** the values GAOL takes from the libm of the system
  are moved three floats outward rather than one. The libms of glibc 2.31, musl
  and MinGW-w64 are sometimes a float further, which gave bounds not enclosing
  the exact values. The branch `hyperbolic-rigorous` bounds them without the
  libm instead; [issue #1](https://github.com/Jordan08/GAOL/issues/1) compares
  the two. At the overflow, the bounds are then two floats wider than the
  tightest, which three assertions of GAOL's own check `trigonometric`
  (`make check`) want: `cosh([MAX, +inf])` is `[0x1.ffffffffffffdp+1023, +inf]`
  rather than `[MAX, +inf]`, likewise `sinh([-inf, -MAX])`, and
  `tanh([MAX, +inf])` has `0x1.ffffffffffffdp-1` for left bound rather than
  `previous_float(1)`.
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
- **`exp(0)` = 1 and `log(1)` = 0 exactly**, the bounds of mathlib moved one
  double outward giving `exp([0])` and `log([1])` a width: `log([0, 1])` was
  `[-oo, 2^-1074]`, and `pow([1], [-oo, +oo])`, exp(y log 1), was `[0, +oo]`
  rather than `[1]`, and `pow([0, 1], [1, +oo])` `[0, +oo]` rather than
  `[0, 1]`. On an Intel i7-1185G7 (GCC 9.4), `log()` takes 1.5 ns more
  (72.5 ns rather than 71) and `pow(x, y)` 2 ns more (145 ns rather than
  143); `exp()` takes the same time.
- **`log()`** gives the empty set for an interval holding no positive number:
  `log` is defined on `(0, +oo)` (IEEE 1788-2015, Table 9.1). GAOL kept the part
  of the interval in `[0, +oo]`, and gave `[-oo, -MAX]` for `log([-4, 0])` and
  `log([0])`, which `check/non_arithmetic.cpp` wanted; IBEX and Codac returned
  the empty set themselves before calling it. `log([0, 1])` is still
  `[-oo, 2^-1074]`.
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
- **`width()`** of the empty set is NaN, as `wid` of IEEE 1788-2015 (12.12.8),
  rather than -1, which the manual and `check/interval_functions.cpp` gave.
  Codac's `diam()` returned NaN for the empty set without calling `width()`;
  IBEX's `diam()`, which its documentation says is 0 for the empty set, returns
  what `width()` returns, NaN now.
- **The cosine of mathlib** (`cmake/mathlib/prepare.cmake`): for the arguments
  hardest to round, mathlib computes cos(x) with multiple-precision numbers, as
  sin(π/2 − x) when x > 0.8, and `mpcos()` returned the cosine of π/2 − x
  instead, which is sin(x). `cos()` then gave bounds not enclosing cos(x), off
  by up to 9%, at 54 of the hard-to-round arguments of cos of
  [CORE-MATH](https://gitlab.inria.fr/core-math/core-math), all between 0.80
  and 0.853 ([dreal-deps/mathlib#2](https://github.com/dreal-deps/mathlib/issues/2)).
  The CMake build fixes the call to `c32()` in `mpcos()` in the sources it
  downloads, the line glibc fixed in its copy of the same code in 2003.
- **The arctangent of mathlib** (`cmake/mathlib/prepare.cmake`): `fastiroot()`,
  which starts the multiple-precision square roots mathlib computes the
  arctangent with at the arguments hardest to round, read the halves of a double
  through `long`s. Where `long` has 64 bits (Linux and macOS on 64-bit
  processors), `atan()` returned values far from atan(x), or had not returned
  after 20 ms, at 15970 of the 55190 hard-to-round arguments of atan of
  CORE-MATH: `atan(1.016527294692847)` was 0.082 instead of 0.794. The CMake
  build makes them `int`s, as glibc did in 2003
  ([commit](https://sourceware.org/git/?p=glibc.git;a=commit;h=bb3f4825c411e676c51479fea59643af540810b5));
  [Debian bug 210613](https://bugs.debian.org/210613) is the same bug on Alpha.
- **The logarithm of mathlib at subnormal arguments**
  (`cmake/mathlib/prepare.cmake`): `ulog()` scales a subnormal argument by 2^54,
  but its last, multiple-precision stage computed the logarithm from the scaled
  argument and from an approximation of the logarithm of the unscaled one.
  `log()` returned about 2^54 at 26 of the 53 subnormal hard-to-round arguments
  of log of CORE-MATH: `log(0x0.8819864d7985dp-1022)` was 1.8e16 instead of
  −709.03. The CMake build gives that stage the unscaled argument. glibc had
  the same code until it
  [removed that stage](https://sourceware.org/git/?p=glibc.git;a=commit;h=b7c83ca30ef8e85b6642151d95600a36535f8d97)
  in 2018.
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
- **`is_finite()`** is `std::isfinite()`, in every build: `finite()` of the C
  library was used where the build system found it, and is not declared by
  every C library.
- **The meson build** defines `GETRUSAGE_IN_HEADER`, as configure does, without
  which it did not compile on Linux, and installs `gaol/gaol_interval2f.h` and
  the headers for MinGW and Visual C++, as configure now does too. Both install
  a `gaol.pc` carrying the flags of interval arithmetic, and meson takes
  `with-mathlib-include` and `with-mathlib-lib` as configure does.
- **The CMake build**, derived from the CMake build of GAOL and mathlib in IBEX
  (Cyril Bouvier, Gilles Chabert), with the compilation flags of the IBEX fork
  of Fabrice Le Bars.
- **The tests** of `tests/`, after the rounding tests of Codac.
