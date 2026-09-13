
# GAOL
<em>Not Just Another Interval Library</em>

GAOL is a C++ [Interval Arithmetic](https://en.wikipedia.org/wiki/Interval_arithmetic) library that strives to offer fast and reliable operators for constraint solvers. 

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

If you want to run tests setup the build folder with option `with-test` to `true`

If you want to install gaol to a specify folder use the meson argument `--prefix`

For instance you can run:

```bash
meson setup build --prefix=/opt/homebrew/Cellar/gaol/4.2.2 -Dwith-mathlib=apmathlib -Dwith-test=true
```

---

## This fork: CMake build, tests and fixes

This fork of [GAOL](https://github.com/goualard-f/GAOL), the interval arithmetic
library written by [Frédéric Goualard](https://frederic.goualard.net), adds a
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
(`MathLib.h` and the `ultim` library, under `MATHLIB_DIR` or the usual paths).
When there is none, it downloads mathlib 2.1.1 from Frédéric Goualard's site
(`mathlib-2.1.1.tar.gz`, checked against its SHA256), builds it with the CMake
build of `cmake/mathlib/`, and installs it along with GAOL.

| Option | Default | |
|---|---|---|
| `GAOL_BUILD_MATHLIB` | `ON` | Download and build mathlib when no installed mathlib is found |
| `MATHLIB_DIR` | | Installation prefix of an installed mathlib |
| `GAOL_BUILD_TESTS` | `ON` when GAOL is the main project | Build the tests, which `ctest` runs |

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

GAOL is configured the way IBEX configures it: the rounding direction is set
upward once, by `gaol::init()`, and not restored after each operation
(`GAOL_PRESERVE_ROUNDING` undefined).

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
- **`rounding_direction`:** after each operation of GAOL's interface, the
  rounding direction has to be upward still, and the bounds of a product and a
  sum the tightest ones.
- **`numbers`:** `interval("0.1")` has to be the tightest interval enclosing the
  number read, and the number itself when it is a double. The constants have to
  be the tightest enclosures of π, 2π and π/2.
- **`other_functions`:** midpoints, widths, magnitudes, mignitudes, Hausdorff
  distances, splitting, integer parts, and the relational functions
  (`sqrt_rel`, `div_rel`...).

`tests/find_package` builds the same tests with an installed GAOL.

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
- **Clang is refused for 32-bit ARM processors**, where it does not honour the
  rounding direction: built by Clang 21, 4556 of 16000 random products, squares
  and cubes did not enclose their exact values.
- **The meson build** defines `GETRUSAGE_IN_HEADER`, as configure does, without
  which it did not compile on Linux. It defines `USING_SSE3_INSTRUCTIONS` only
  with `enable-simd`, which installs the header `gaol/gaol` then includes.
- **The CMake build**, derived from the CMake build of GAOL and mathlib in IBEX
  (Cyril Bouvier, Gilles Chabert), with the compilation flags of the IBEX fork
  of Fabrice Le Bars.
- **The tests** of `tests/`, after the rounding tests of Codac.

### Other builds

The autotools build (`./configure && make`, see `INSTALL`) and the meson build
described above are kept. They are built by the continuous integration against
a mathlib installed from Frédéric Goualard's archive, and the tests are built
with the GAOL they install. Unlike the CMake build, both preserve the rounding
direction after each operation by default (`--enable-preserve-rounding`,
`enable-preserve-rounding`).

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
MinGW-w64 11 to 13, and builds GAOL with autotools and meson.

### Licences

GAOL is distributed under the GNU LGPL v2 (`COPYING.LIB`). mathlib, which the
build downloads, is distributed under the GNU GPL v2 or later.
