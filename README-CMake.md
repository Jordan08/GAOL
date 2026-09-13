# GAOL with CMake

This fork of [GAOL](https://github.com/goualard-f/GAOL), the interval arithmetic
library written by [Frédéric Goualard](https://frederic.goualard.net), adds a
CMake build and the changes GAOL needs to compile and compute right with
Visual C++, MinGW, 32-bit ARM and other systems. It was written for
[Codac](https://github.com/codac-team/codac), whose intervals are built upon
GAOL. The autotools and meson builds of GAOL are kept.

## Building and installing

```bash
cmake -S . -B build -DCMAKE_INSTALL_PREFIX=<prefix>
cmake --build build --config Release
cmake --install build --config Release
```

GAOL computes its elementary functions with mathlib, the IBM Accurate Portable
Mathematical Library (libultim). The build looks for an installed mathlib
(`MathLib.h` and the `ultim` library, under `MATHLIB_DIR` or the usual paths);
when there is none, it downloads mathlib 2.1.1 from Frédéric Goualard's site
(`mathlib-2.1.1.tar.gz`, checked against its SHA256), builds it with the CMake
build of `cmake/mathlib/`, and installs it along with GAOL.

| Option | Default | |
|---|---|---|
| `GAOL_BUILD_MATHLIB` | `ON` | Download and build mathlib when no installed mathlib is found |
| `MATHLIB_DIR` | | Installation prefix of an installed mathlib |

Both libraries are static.

## Using GAOL from CMake

```cmake
find_package(gaol REQUIRED)
target_link_libraries(my_target PRIVATE gaol::gaol)
```

`gaol::gaol` carries the include directory, mathlib, and the compilation flags
interval arithmetic needs: `-frounding-math -ffloat-store -fno-fast-math
-ffp-contract=off` with GCC and Clang (the ones each compiler takes),
`-msse2 -mfpmath=sse` on 32-bit x86, `/fp:strict` with Visual C++. Code
including GAOL's headers has to be compiled with them, GAOL's interval
operations being inline.

GAOL is configured the way IBEX configures it: the rounding direction is set
upward once, at initialisation, and not restored after each operation
(`GAOL_PRESERVE_ROUNDING` undefined).

## What differs from GAOL

Each change is a commit of its own, and says where it comes from.

- **From the patch IBEX applies to GAOL** (Gilles Chabert,
  `interval_lib_wrapper/gaol/3rd/gaol-4.2.3alpha0.all.all.patch` of
  [IBEX](https://github.com/ibex-team/ibex-lib)):
  - the FPU is initialised with `fesetenv(FE_DFL_ENV)` and `round_upward()`
    rather than by writing an x87 control word into `fenv_t`, which on ARM64
    left the rounding to nearest and crashed an ARM64 Mac;
  - `interval::midpoint()` and `operator<<` set the rounding direction back
    upward, which they did not when the rounding direction is not preserved;
  - the macro `opposite()` is renamed `gaol_opposite()`.
- **For Visual C++, MinGW and the systems GAOL's configure does not know**,
  following the fork of GAOL by [Fabrice Le Bars](https://github.com/lebarsfa/GAOL):
  - `gaol/gaol_config_msvc.h`, `gaol/gaol_config_mingw.h` and
    `gaol/gaol_version_msvc.h` include the generated configuration;
  - memory allocated by `MEMALIGN()` can be freed by `free()`, as GAOL frees it;
  - `get_fpu_cw()` and `reset_fpu_cw()` save the rounding direction with
    `<fenv.h>` where the control word of `fenv_t` is not known;
  - `_MATHLIB_DLL_` is only defined when not already, and Visual C++ gets the
    `<fenv.h>` version of `get_inexact()` and `clear_inexact()`.
- **Square roots** are bounded whatever the rounding of the C library's
  `sqrt`, which Visual C++ for 32-bit x86 rounds to nearest in every rounding
  direction.
- **The CMake build**, derived from the CMake build of GAOL and mathlib in
  IBEX (Cyril Bouvier, Gilles Chabert), with the compilation flags of the IBEX
  fork of Fabrice Le Bars.

## Platforms

These sources are the ones Codac builds GAOL from, with a CMake build this one
derives from, and are checked by its continuous integration on Linux (x86_64,
arm64, armv6hf), macOS (arm64, x86_64) and Windows (Visual Studio x86, x64 and
arm64; MinGW x86 and x64). The bounds of the arithmetic and elementary
operations were also compared exactly with their true values on 32-bit x86,
32-bit ARM, s390x, riscv64 and ppc64le, and this build was used to build,
install and use GAOL through find_package(gaol) on Linux x86_64 (GCC, Clang,
CMake 3.14), 32-bit ARM (GCC) and MinGW x86 and x64.

Clang does not honour the rounding direction on 32-bit ARM processors, and the
build warns about it: use GCC there.

## Licences

GAOL is distributed under the GNU LGPL v2 (`COPYING.LIB`). mathlib, which the
build downloads, is distributed under the GNU GPL v2 or later.
