<!-- Copyright (c) 2026 ENSTA, France
     Created 2026-09-20 by Jordan NININ -->
# Using GAOL

Part of the documentation of [GAOL v5](../README.md#documentation).

GAOL's interval operations are inline: the code that includes GAOL's headers
has to be compiled with the flags of interval arithmetic, not only GAOL
itself. With GCC and Clang, each where the compiler takes it:

- `-frounding-math -fno-fast-math -ffp-contract=off`, so that the compiler
  rounds each operation in the direction in effect, as written, and neither
  evaluates it at compile time in the default rounding nor contracts a
  multiplication and an addition into a fused one;
- `-msse2 -mfpmath=sse` on 32-bit x86, so that doubles are computed in double
  precision rather than on the x87 unit;
- `-msse2 -msse3` on x86 processors with the SSE2 intervals (`GAOL_SIMD`);
- `-ffloat-store` where doubles are still computed on the x87 unit;
- `-mfma` on x86, and `-mfpu=neon-vfpv4 -mfloat-abi=hard` on 32-bit ARM, where
  GAOL is compiled with the fused multiply-add instructions of the processor
  (`GAOL_FMA`, see [Building GAOL](building.md)): the code using GAOL runs only
  on a processor with them, as GAOL, and is compiled for it.

With Visual C++, `/fp:strict`, and `/arch:AVX2` for x64 with `GAOL_FMA`. Each build installs them with GAOL, in
`gaol::gaol` and `gaol.pc` (below). `gaol/gaol_config.h` refuses code compiled
by Visual C++ without `/fp:strict`. GCC and Clang do not tell the code whether
`-frounding-math` and `-ffp-contract=off` were given: there, it only refuses
what contradicts them, `-ffast-math` and doubles computed on the x87 unit (see
[Compilers and options refused](three-builds.md#compilers-and-options-refused)).

## From CMake

```cmake
find_package(gaol REQUIRED)
target_link_libraries(my_target PRIVATE gaol::gaol)
```

`gaol::gaol` carries the include directory, the flags above, mathlib
(`gaol::ultim` when it was built along with GAOL) and, for Visual C++,
`__GAOL_PUBLIC__=`, GAOL being a static library. A library whose headers
include GAOL's, as Codac's, links `gaol::gaol` `PUBLIC`, so that its own users
get the flags, and its CMake package finds GAOL again (`find_dependency(gaol)`).
`tests/find_package` is a project using an installed GAOL this way.

A project can also build GAOL for itself, with the options it wants:

```cmake
include(FetchContent)
FetchContent_Declare(gaol GIT_REPOSITORY https://github.com/Jordan08/GAOL.git GIT_TAG master)
FetchContent_MakeAvailable(gaol)
target_link_libraries(my_target PUBLIC gaol::gaol)
```

GAOL and the mathlib of `3rd/mathlib` are then targets of the project
(`gaol::gaol`, `gaol::ultim`), built with it, and `cmake --install` of the
project installs them with it, CMake package and `gaol.pc` included; nothing
is downloaded beyond GAOL's sources. `tests/fetch_content` is a project
building GAOL this way.

GAOL and mathlib, static libraries, are compiled as position-independent code
(`-fPIC`), so that `gaol::gaol` can be linked into a shared library, such as
Python bindings. A project that sets `CMAKE_POSITION_INDEPENDENT_CODE`, `ON` or
`OFF`, before `FetchContent_MakeAvailable(gaol)`, or on the command line, is
followed instead.

## From pkg-config

Each build installs `gaol.pc` in the `pkgconfig` directory of its library
directory (`<prefix>/lib/pkgconfig`, or `lib64` or `lib/<multiarch>` rather
than `lib` on the systems whose libraries go there), except the CMake build
with Visual C++. Its `Cflags` carries the flags above with the include
directory, and `Libs` GAOL and mathlib:

```bash
export PKG_CONFIG_PATH=<prefix>/lib/pkgconfig
c++ -std=c++17 -O2 $(pkg-config --cflags gaol) program.cpp $(pkg-config --libs gaol)
```

In a meson project, `dependency('gaol')`.

## The rounding direction

Each operation of GAOL sets the rounding direction upward when it is not, and
leaves it upward, whichever way GAOL is built. The bounds are then right
whatever rounding direction the calling code left. The check is an addition,
1 + 2^-60, above 1 only when rounded upward, rather than a reading of the
rounding direction, which cost far more under Rosetta 2 and with 32-bit Visual
C++ (see [What differs from GAOL](differences.md)). Code that needs
its own rounding direction after GAOL's operations builds GAOL with
`GAOL_PRESERVE_ROUNDING` (`--enable-preserve-rounding`,
`-Denable-preserve-rounding=true`): each operation then also restores the
rounding direction it found, which makes the arithmetic operations several
times slower.
