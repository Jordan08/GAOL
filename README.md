
# GAOL
<em>Not Just Another Interval Library</em>

GAOL is a C++ [Interval Arithmetic](https://en.wikipedia.org/wiki/Interval_arithmetic) library that strives to offer fast and reliable operators for constraint solvers. 

## Author

GAOL is written by [Frédéric Goualard](https://frederic.goualard.net/), Associate
Professor in Computing Science at Nantes Université (LS2N, UMR CNRS 6004), who
has been its main developer since 2001. Its page is the
[GAOL section](https://frederic.goualard.net/#research-software-gaol) of
Frédéric Goualard's site, and its original repository is
[goualard-f/GAOL](https://github.com/goualard-f/GAOL).

Publications of Frédéric Goualard related to GAOL (see the
[full list](https://frederic.goualard.net/#Publications)):

- [Fast and Correct SIMD Algorithms for Interval Arithmetic](https://frederic.goualard.net/publications/interval-sse2_goualard_para08.pdf).
  PARA '08, Lecture Notes in Computer Science 6126–6127, Springer, 2012.
- [Interval Extensions of Multivalued Inverse Functions](https://hal.archives-ouvertes.fr/hal-00288457v1).
  Research report hal-00288457, 2008.
- [How do you compute the midpoint of an interval?](https://hal.archives-ouvertes.fr/hal-00576641v2).
  ACM Transactions on Mathematical Software 40(2), 2014.

## Building GAOL

### Pre-requisites

A supported math library: [apmathlib](https://frederic.goualard.net/software/mathlib-2.1.1.tar.gz) or [crlibm](https://github.com/taschini/crlibm) (since crlibm github repo seems to have missing files, you can take get it here: [pycrlibm](https://github.com/taschini/pycrlibm))

### Linux users

Look at INSTALL file to use autotools

### MacOS ARM users

[Meson build system](https://mesonbuild.com/index.html) can be used:

```bash
meson setup build -Dwith-mathlib=crlibm
cd build
meson compile
```

GAOL is built optimized by default (`buildtype=release`); `meson setup build
--buildtype=debug` builds it for debugging.

If you want to run tests setup the build folder with option `with-test` to `true`

If you want to install gaol to a specify folder use the meson argument `--prefix`

For instance you can run:

```bash
meson setup build --prefix=/opt/homebrew/Cellar/gaol/4.2.2 -Dwith-mathlib=apmathlib -Dwith-test=true
```

---

## This fork: CMake build, tests and fixes

This fork of [GAOL](https://github.com/goualard-f/GAOL), the interval arithmetic
library written by [Frédéric Goualard](https://frederic.goualard.net/), adds a
CMake build, tests of the bounds GAOL computes, and the changes GAOL needs to
compile and compute right with Visual C++, MinGW, 32-bit ARM and other systems.
It was written for [Codac](https://github.com/codac-team/codac), whose intervals
are built upon GAOL. The autotools and meson builds above are kept, and still
work (see [Other builds](#other-builds)).

### Building and installing with CMake

```bash
cmake -S . -B build -DCMAKE_INSTALL_PREFIX=<prefix>
cmake --build build --config Release
ctest --test-dir build -C Release
cmake --install build --config Release
```

GAOL computes its elementary functions with mathlib, the IBM Accurate Portable
Mathematical Library (libultim). The build looks for an installed mathlib
(`MathLib.h` and the `ultim` library, under `MATHLIB_DIR` or the usual paths),
unless `GAOL_FIND_MATHLIB` is `OFF`. When there is none, or none is looked for,
it downloads mathlib 2.1.1 from
[Frédéric Goualard's site](https://frederic.goualard.net/)
(`mathlib-2.1.1.tar.gz`, checked against its SHA256), fixes bugs of it (see
[What differs from GAOL](#what-differs-from-gaol)), builds it with the CMake
build of `cmake/mathlib/`, and installs it along with GAOL. An installed mathlib
is used as it is. A project building GAOL for itself, as Codac does, sets
`GAOL_FIND_MATHLIB` to `OFF` to find mathlib under the installation prefix it
gives, whatever mathlib the machine has.

| Option | Default | |
|---|---|---|
| `GAOL_BUILD_MATHLIB` | `ON` | Download and build mathlib when no installed mathlib is found |
| `GAOL_FIND_MATHLIB` | `ON` | Look for an installed mathlib before building one; `OFF` builds mathlib even where one is installed |
| `MATHLIB_DIR` | | Installation prefix of an installed mathlib |
| `GAOL_BUILD_TESTS` | `ON` when GAOL is the main project | Build the tests, which `ctest` runs |
| `GAOL_PRESERVE_ROUNDING` | `OFF` | Restore the rounding direction found after each operation, rather than leaving it upward (see [Using GAOL from CMake](#using-gaol-from-cmake)) |

Both libraries are static. CMake 3.14 or later is needed.

GAOL cannot be built with a compiler that does not honour the rounding direction
on the target: Clang for 32-bit ARM processors, and compilers that say so of
`-frounding-math`, such as Clang 14 for 64-bit ARM processors. Nor with a
MinGW-w64 older than version 12 (MinGW-w64 GCC 11 to 13), whose math library
gave hyperbolic functions far from their exact values. The configuration stops
with a message naming the compilers to use instead (GCC, a later Clang, a later
MinGW-w64).

### Using GAOL from CMake

```cmake
find_package(gaol REQUIRED)
target_link_libraries(my_target PRIVATE gaol::gaol)
```

`gaol::gaol` carries the include directory, mathlib, and the compilation flags
interval arithmetic needs:

- `-frounding-math -ffloat-store -fno-fast-math -ffp-contract=off` with GCC and
  Clang (the ones each compiler takes);
- `-msse2 -mfpmath=sse` on 32-bit x86;
- `/fp:strict` with Visual C++.

Code including GAOL's headers has to be compiled with them, GAOL's interval
operations being inline.

Each operation of GAOL sets the rounding direction upward when it is not, and
leaves it upward, whichever way GAOL is built (CMake, autotools or meson). The
bounds are then right whatever rounding direction the calling code left. The
check is an addition, 1 + 2^-60, above 1 only when rounded upward, rather than
a reading of the rounding direction, which cost far more under Rosetta 2 and
with 32-bit Visual C++ (see [What differs from GAOL](#what-differs-from-gaol)).
Code that needs its own rounding direction
after GAOL's operations builds GAOL with `GAOL_PRESERVE_ROUNDING` `ON`
(`--enable-preserve-rounding` with autotools, `-Denable-preserve-rounding=true`
with meson): each operation then also restores the rounding direction it found,
which makes the arithmetic operations several times slower.

### Tests

The programs of `tests/` compare the bounds GAOL computes with the exact results
of the operations, independently of GAOL and of the floating-point environment.
The exact results are computed with integers, or were computed with 2000 bits of
precision by [mpmath](https://mpmath.org). They follow the rounding tests of
Codac.

- **`arithmetic`:** on doubles and intervals of every magnitude (subnormal
  doubles and overflows included), sums, differences, products, quotients,
  relational divisions, squares, inverses, `abs`, `min`, `max`, `&`, `|` have to
  be the tightest enclosures. Integer powers, square roots and n-th roots have
  to be enclosures, within a few doubles.
- **`elementary`:** `exp`, `log`, `sin`, `cos`, `tan`, `asin`, `acos`, `atan`,
  `sinh`, `cosh`, `tanh`, `asinh`, `acosh`, `atanh`, `sqrt` and `pow` at doubles,
  at intervals, and at intervals whose images are known exactly (extrema,
  poles, domains). The values are in `elementary_values.h`, which
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
  be the tightest enclosures of π, 2π and π/2.
- **`other_functions`:** midpoints, widths, magnitudes, mignitudes, Hausdorff
  distances, splitting, integer parts, and the relational functions
  (`sqrt_rel`, `div_rel`...).

`tests/find_package` builds the same tests with an installed GAOL.

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

### What differs from GAOL

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
  - Memory allocated by `MEMALIGN()` can be freed by `free()`, as GAOL frees it.
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
- **`hausdorff()`** returns the tightest upper bound of the distance. It computed
  `fabs(a - c)` in the rounding direction of the caller, below the exact
  distance when rounded upward with a < c.
- **Square roots** are bounded whatever the rounding of the C library's `sqrt`,
  which Visual C++ for 32-bit x86 rounds to nearest in every rounding direction.
  Where `sqrt` rounds as it should, the results are unchanged.
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
  the two.
- **Powers with a real exponent**, ported from the fix of Codac:
  - `pow(I, e)` for a floating-point `e` called `pow(I, int)`, which truncated
    the exponent: `pow([4], 0.5)` returned `[1]`. It now computes an integer `e`
    with `pow(I, int)` and any other `e` with `pow(I, J)`.
  - `pow(I, J)` computed the powers of the negative part of `I` on its
    magnitude: `pow([-4,-1], [0.5])` returned `[-1, 2]`. It now keeps the
    negative part of `I` only for an integer exponent.
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
- **Clang is refused for 32-bit ARM processors**, where it does not honour the
  rounding direction: built by Clang 21, 4556 of 16000 random products, squares
  and cubes did not enclose their exact values.
- **The autotools and meson builds** compile GAOL with `-ffp-contract=off`, as
  the CMake build does, and `configure` checks `-frounding-math` with any
  compiler, not only `g++`. Without `-ffp-contract=off`, the compilers fused
  the multiplications and additions of `mid()`, whose two bounds were then
  computed alike: `mid()` returned a single double, not enclosing the midpoint
  of 9735 of the 10000 random intervals of `other_functions`, with meson on
  macOS arm64 (Apple Clang) and with autotools on Ubuntu arm64 (GCC 13,
  rounding direction preserved). Without `-frounding-math`, GAOL built by
  `clang++` at `-O2` without SSE2 intervals (`--disable-simd`) gave integer
  powers not enclosing their exact values.
- **The meson build** defines `GETRUSAGE_IN_HEADER`, as configure does, without
  which it did not compile on Linux. It defines `USING_SSE3_INSTRUCTIONS` only
  with `enable-simd`, which installs the header `gaol/gaol` then includes. It
  builds GAOL in release by default, where meson's own default, debug, compiled
  it without optimization, and `enable-optimize`, which did nothing, adds the
  optimization flags of configure where the compiler takes them.
- **The CMake build**, derived from the CMake build of GAOL and mathlib in IBEX
  (Cyril Bouvier, Gilles Chabert), with the compilation flags of the IBEX fork
  of Fabrice Le Bars.
- **The tests** of `tests/`, after the rounding tests of Codac.

### Other builds

The autotools build (`./configure && make`, see `INSTALL`) and the meson build
described above are kept. They are built by the continuous integration against
a mathlib installed from
[Frédéric Goualard's archive](https://frederic.goualard.net/software/mathlib-2.1.1.tar.gz),
and the tests are built
with the GAOL they install. As the CMake build, both leave the rounding
direction upward by default; `--enable-preserve-rounding` and
`-Denable-preserve-rounding=true` restore it after each operation.

### Platforms

The continuous integration of this fork (`.github/workflows/`) builds GAOL with
CMake and runs the tests on:

- **Linux:** Ubuntu 22.04, 24.04 and 26.04 on x86_64 and arm64, with GCC and
  Clang, also with the address and undefined behaviour sanitizers, and with
  CMake 3.14.
- **Linux containers:**
  - Debian 12 and 13 on amd64, arm64 and armhf, and Debian 12 on i386;
  - manylinux_2_28 on x86_64 and aarch64;
  - Alpine (musl) on x86_64 and aarch64;
  - Debian 13 under qemu on s390x, ppc64le and riscv64.
- **macOS:** 14, 15 and 26, on arm64 and x86_64 (natively or under Rosetta), and
  with the sanitizers with AppleClang, LLVM's Clang and GCC.
- **Windows:**
  - Visual Studio 2022 and 2026, on x86, x64 and arm64, Release and Debug;
  - MinGW-w64 14 and 15, on x86 and x64;
  - MSYS2 UCRT64 (GCC) and CLANG64 (Clang).

It also checks that Clang is refused on 32-bit ARM, Clang 14 on 64-bit ARM, and
MinGW-w64 11 to 13, and builds GAOL with autotools and meson. Jobs of each build
restore the rounding direction (`GAOL_PRESERVE_ROUNDING`): Ubuntu x86_64 GCC and
arm64 Clang, Debian i386 and armhf, macOS arm64, Visual Studio x64, autotools and
meson. The jobs built in Release print the time per operation in their summary.

### Licences

GAOL, by [Frédéric Goualard](https://frederic.goualard.net/), is distributed
under the GNU LGPL v2 (`COPYING.LIB`). mathlib, which the
build downloads, is distributed under the GNU GPL v2 or later.
