# Third-party libraries

Part of the documentation of [this fork of GAOL](../README.md#documentation).

## mathlib

`mathlib/` holds the sources of mathlib 2.1.1, the IBM Accurate Portable
Mathematical Library (libultim), with which GAOL computes its elementary
functions, as Frédéric Goualard distributes it along with GAOL:
[mathlib-2.1.1.tar.gz](https://frederic.goualard.net/software/mathlib-2.1.1.tar.gz)
(SHA256 `f299848aa3e57ebb6248cd3cf54ecc7661a945aeac9e420e71db194965f87281`).
It is distributed under the GNU LGPL v2 or later, as the headers of its
sources state (`mathlib/COPYING` is the text of the GNU GPL v2). The whole
archive is kept, its own autotools build (`mathlib/configure`,
`mathlib/Makefile.am`) and its batch file for Visual C++
(`mathlib/install.bat`) included, which GAOL does not use.

### How GAOL builds it

The three builds of GAOL compile libultim from `mathlib/src` themselves and
install it along with GAOL, with `MathLib.h` and its configuration headers,
unless told to use a mathlib installed already (see
[Building GAOL](../doc/building.md)):

- CMake: `mathlib/CMakeLists.txt`, which `CMakeLists.txt` reaches with
  `add_subdirectory()` and which `scripts/install-mathlib.sh` builds alone;
- meson: `mathlib/meson.build`, reached with `subdir()`;
- autotools: `Makefile.am` of this directory, a subdirectory of GAOL's build.

They compile it with the flags of interval arithmetic GAOL is compiled with
(`-frounding-math -fno-fast-math -ffp-contract=off`, and `-msse2 -mfpmath=sse`
on 32-bit x86; `/fp:strict` with Visual C++). Compiled with the contraction of
multiplications and additions into fused multiply-adds, which GCC does by
default wherever the processor has them, 64-bit ARM processors included,
unless given `-ffp-contract=off`, mathlib reduces large arguments modulo π/2
wrongly (`branred()`): compiled so on x86_64, it gave cos(2^52 − 1) about
−0.4855 rather than 0.4733, and sin, cos and tan far from their values at 534
of 20080 random arguments from 2^26 to 2^54, which GAOL's bounds did not
enclose. This is why the builds refuse an installed mathlib whose cosine of
2^52 − 1 is wrong (see [The three builds](../doc/three-builds.md)). mathlib
is also compiled with `-w`, whatever warning flags the project building GAOL
gives: with `-Wall -Wextra -Wconversion`, GCC 9 printed 25255 warnings over
its sources.

These build files were added for GAOL, with `mathlib/mathlib_configuration.h.in`,
the configuration header mathlib's own configure script would write, which
every build copies as it is: the macros describing the target are set by the
preprocessor there, from the macros of the compiler, which works the same in a
cross-compilation and with Visual C++. `src/mathlib_config_msvc.h` and
`src/mathlib_config_mingw.h`, the configuration files `src/mathlib_config.h`
includes for Visual C++ and MinGW, are replaced with files including that one.
`mathlib/CMakeLists.txt` and `mathlib/mathlib_configuration.h.in` derive from
the CMake build of the same mathlib in [IBEX](https://github.com/ibex-team/ibex-lib)
(`interval_lib_wrapper/gaol/3rd/mathlib-2.1.1/`, Cyril Bouvier and Gilles
Chabert).

### What is changed in mathlib's sources

The sources are those of the archive but for the fixes below, each a change
of a few lines in one file, with the pragma of the tables. Every one is
covered by the tests of `tests/` (see `tests/elementary_values.py`), and
[What differs from GAOL](../doc/differences.md) lists them among the changes
of the fork.

- **`src/sincos32.c`, the cosine.** For the arguments hardest to round,
  `ucos()` falls back on `mpcos()`, which computes cos(x) with
  multiple-precision numbers, as sin(π/2 − x) when x > 0.8. `c32(x, y, z)`
  sets y to cos(x) and z to sin(x), and `mpcos()` took the cosine of π/2 − x,
  which is sin(x), rather than its sine: `ucos()` returned sin(x), and GAOL
  bounds not enclosing cos(x), at 54 of the hard-to-round arguments of cos of
  [CORE-MATH](https://gitlab.inria.fr/core-math/core-math), all between 0.80
  and 0.853 ([dreal-deps/mathlib#2](https://github.com/dreal-deps/mathlib/issues/2)).
  The call is now `c32(&b,&c,&a,p)`. glibc, which took the same code from IBM,
  fixed the same line in 2003 (Debian bug 153548,
  [commit](https://sourceware.org/git/?p=glibc.git;a=commit;h=86583139a4d746743ccffcd72e25d96c5fb8d488)).
- **`src/mpsqrt.c`, the square root in multiple precision**, which the
  arctangent uses at the arguments hardest to round (`src/mpatan.c`,
  `src/mpatan2.c`). `fastiroot()`, which gives it a first approximation of
  1/√x, read and wrote the halves of a double through an array of two `long`s:
  where `long` has 64 bits (Linux and macOS on 64-bit processors),
  `p.i[HIGH_HALF]` is not the high half of the double, x was not scaled to
  [0.5, 2), and the approximation was wrong. `atan()` then returned values far
  from atan(x), or had not returned after 20 ms, at 12003 and 3967 of the
  55190 hard-to-round arguments of atan of CORE-MATH on x86_64:
  atan(1.016527294692847) was 0.082 instead of 0.794. They are `int`s, as
  glibc made them in 2003 (Michael Matz, "fastiroot: Fix 64-bit problem",
  [commit](https://sourceware.org/git/?p=glibc.git;a=commit;h=bb3f4825c411e676c51479fea59643af540810b5));
  [Debian bug 210613](https://bugs.debian.org/210613) is the same bug on Alpha,
  whose three arguments fail on x86_64 without this fix.
- **`src/ulog.c`, the logarithm at subnormal arguments.** `ulog()` scales a
  subnormal x by 2^54, which its exponent n accounts for, but its last,
  multiple-precision stage computed the logarithm from the scaled x and from
  an approximation of the logarithm of the unscaled one: `ulog()` returned
  about 2^54 at 26 of the 53 subnormal hard-to-round arguments of log of
  CORE-MATH on x86_64: log(0x0.8819864d7985dp-1022) was 1.8e16 instead of
  −709.03. That stage is given the unscaled x, kept in `x0`. glibc kept it in
  `__ieee754_log` until it removed the multiple-precision stages in 2018
  (Wilco Dijkstra, "Remove slow paths from log",
  [commit](https://sourceware.org/git/?p=glibc.git;a=commit;h=b7c83ca30ef8e85b6642151d95600a36535f8d97)).
- **`src/utan.c`, the tangent.** In 7 of the tests with which `utan()`
  decides whether its result is rounded right, the error bound `t4` was
  assigned in one operand of `==` and read in the other, with no sequence
  point between them: undefined behaviour, which Clang warns about
  (`-Wunsequenced`). GCC and Clang 18, from `-O0` to `-O3`, gave the same
  results as with `t4` assigned before the test, at the 1077348 hard-to-round
  arguments of tan of CORE-MATH and 10 million others. `t4` is assigned before
  the test, as glibc did in 2009 (Ulrich Drepper, "Fix -Wsequence-point
  warnings",
  [commit](https://sourceware.org/git/?p=glibc.git;a=commit;h=82a1a4dae1b699a394e213866e789eacef1728fc))
  and Fabrice Le Bars in his fork of mathlib
  ([commit](https://github.com/lebarsfa/mathlib/commit/04a3dfe75cd3f0e49e22bca9ed688462d18c6c52)).
- **`src/halfulp.c`, the shifts.** `halfulp()`, which `upow()` calls for the
  powers that may be exact, counts the significant bits of the high half of a
  double by shifting it leftward until it is 0, as an `int`: shifting a
  positive `int` into its sign bit is undefined behaviour in C, which the
  UndefinedBehaviorSanitizer of Clang reports ("left shift of 1070596096 by
  12 places cannot be represented in type 'int4'"), and which the continuous
  integration suppressed
  ([issue #5](https://github.com/Jordan08/GAOL/issues/5)). An `unsigned int`
  is shifted instead, which gives the bits the compilers gave: `halfulp()`
  returns the same doubles at 4 million arguments, 228644 of them exact
  powers, and the tests pass with Clang 18 and no suppression.
- **`src/sincos32.h`, the include guard**, which tested `SINCOS32_H` but
  defined `SINCCOS32_H`, which Clang warns about (`-Wheader-guard`): it
  defines `SINCOS32_H`, as Fabrice Le Bars fixed it
  ([commit](https://github.com/lebarsfa/mathlib/commit/ad40312506d2297c75e3169816d1878b791d74a6)).
- **`src/mathlib_config.h`, `#pragma STDC FENV_ACCESS ON`** at its end, which
  every source of mathlib includes: the pragma of C99 (7.6.1) that tells the
  compiler the code may be executed with a rounding direction other than the
  default, and that it must not fold nor reorder its floating-point operations
  as if the rounding were to nearest. GAOL sets the rounding direction to
  nearest before calling mathlib and back upward afterwards, and mathlib is
  compiled with the flags of interval arithmetic: the pragma states for the
  compiler what those flags ask of it, as Fabrice Le Bars does in his fork
  ([commit](https://github.com/lebarsfa/mathlib/commit/5ac52c2bd817e44d33d4f9af6c1045d4b8577449)),
  with the uppercase `ON` that macOS warns about in the lowercase. Clang 18
  honours it, and the elementary functions of an interval took the same time
  with it as without (exp, log, sin and cos within 0.8%, below the dispersion
  of the measures); GCC 13 ignores it and gave a `libultim.a` identical byte
  for byte; Visual C++ is given `/fp:strict`, which its documentation says
  makes it behave as if `fenv_access(on)` were set, and reads the pragma for
  Visual C++ rather than the one of C99, which it does not know.
- **`src/uatan.tbl`, `src/ulog.tbl` and `src/utan.tbl`,
  `#pragma GCC diagnostic ignored "-Wmissing-braces"`.** The entries of the
  tables, of the union type `number`, are written `{0x3ff6a13c, 0xd1537290 }`
  where the union holds an array: GCC and Clang warn about each of them with
  `-Wall` (`-Wmissing-braces`), 15777 times over mathlib's sources, and
  printing them is slow enough to stop a build: compiling `src/atnat.c` with
  `-Wall`, which includes `src/uatan.tbl` and its 6027 of them, had not
  finished after ten minutes, against 0.22 s without. The three tables carry
  15165 of the 15777 and are given the pragma, as Fabrice Le Bars does in his
  fork ([commit](https://github.com/lebarsfa/mathlib/commit/daa4f21874f76785988426f03a5f651ac5a6cf4e)):
  434 are left, from the tables of the other sources, kept as they are, which
  `-w` turns off in the builds of GAOL. The pragma changes no code:
  `libultim.a` is the same, byte for byte, with it and without it.
- **`src/AARCH64_DPChange.c`, `Init_Lib()` and `Exit_Lib()`**, which
  `mathlib/mathlib_configuration.h.in` chooses for every target
  (`MATHLIB_AARCH64`): nothing in it is specific to ARM64, and it goes through
  `<fenv.h>` alone. `Init_Lib()` restored the default floating-point
  environment (`FESETENV(FE_DFL_ENV)`) and returned 0, and `Exit_Lib()` did
  nothing: the rounding direction the caller had set was lost, where the
  comment of `Init_Lib()` says its result is what `Exit_Lib()` takes to restore
  it. `Init_Lib()` now sets the rounding direction to nearest, which mathlib's
  algorithms need, and returns the one it found, and `Exit_Lib()` sets that one
  back, as the fork of mathlib by Fabrice Le Bars does
  ([commit](https://github.com/lebarsfa/mathlib/commit/40c8a25ad855830db7688ef7dd32bb667ff0eb25));
  the rest of the floating-point environment, whose exception flags and masks
  `FE_DFL_ENV` also reset, is left as it is. Nothing is kept in the global
  variables of the file, which no longer has any use for them: two threads
  calling `gaol::init()` no longer write the same variables. The versions
  mathlib's own build chooses for the other processors are not used:
  `src/LINUX64_DPChange.c` (x86_64 Linux), `src/LINUX_DPChange.c` (32-bit
  x86) and `src/IX86MACOSX_DPChange.c` (Intel Macs) save the control word of
  the x87 unit in global variables, which two threads calling `gaol::init()`
  would write at once, to set a precision that GAOL loses right after by
  restoring the default floating-point environment, and that the doubles of
  32-bit x86, computed in SSE2, do not depend on; `src/LINUX_DPChange.c` and
  `src/MSVC_DPChange.c` are 32-bit x86 assembly besides.

### Updating mathlib

Should a new archive be taken, unpack it over `mathlib/`, keep
`mathlib/CMakeLists.txt`, `mathlib/meson.build` and
`mathlib/mathlib_configuration.h.in`, and apply the changes above again:
`git diff` of the files of `mathlib/src` before the update shows each of them
exactly.
