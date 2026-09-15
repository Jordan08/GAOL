
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

## This fork

This fork of [GAOL](https://github.com/goualard-f/GAOL) adds a CMake build,
tests of the bounds GAOL computes, and the changes GAOL needs to compile and
compute right on every system it can: Linux, macOS and Windows, on x86, x86_64,
ARM, arm64 and the other processors of Debian. It was written for
[Codac](https://github.com/codac-team/codac), whose intervals are built upon
GAOL. The autotools and meson builds of GAOL are kept, and the three builds
configure GAOL the same way (see [The three builds](#the-three-builds)).

Contents: [Building GAOL](#building-gaol) (the three builds and their options)
· [Using GAOL](#using-gaol) (CMake, pkg-config, the flags)
· [The three builds](#the-three-builds) (what they agree on, the refused
compilers) · [Tests](#tests) · [What differs from GAOL](#what-differs-from-gaol)
· [Continuous integration](#continuous-integration) · [Licences](#licences).

## Building GAOL

GAOL computes its elementary functions with mathlib, the IBM Accurate Portable
Mathematical Library (libultim):
[mathlib-2.1.1.tar.gz](https://frederic.goualard.net/software/mathlib-2.1.1.tar.gz)
on Frédéric Goualard's site. The CMake build is the one to use: it downloads
mathlib, fixes it (see [What differs from GAOL](#what-differs-from-gaol):
as it is, `cos()`, `atan()` and `log()` of mathlib are wrong at some
arguments), builds it with the flags of interval arithmetic and installs it
along with GAOL. The autotools and meson builds of GAOL are kept for those
who use them; they need mathlib installed beforehand (`MathLib.h` and
`libultim.a`). The math library of the system (`-lm`) needs no installation.
The autotools and meson builds can also build GAOL with
[CRlibm](https://github.com/taschini/crlibm) instead (`crlibm`).

### With CMake

```bash
cmake -S . -B build -DCMAKE_INSTALL_PREFIX=<prefix> -DGAOL_BUILD_TESTS=ON
cmake --build build --config Release
ctest --test-dir build -C Release
cmake --install build --config Release
```

CMake 3.14 or later. The build looks for an installed mathlib (under
`MATHLIB_DIR` or the usual paths) unless `GAOL_FIND_MATHLIB` is `OFF`, and
refuses one whose cosine is wrong (see
[Compilers and options refused](#compilers-and-options-refused)); when
there is none, or none is looked for, it downloads mathlib 2.1.1 (checked
against its SHA256), fixes bugs of it (see
[What differs from GAOL](#what-differs-from-gaol)), builds it with
`cmake/mathlib/` and installs it along with GAOL. Both libraries are static.
The build type is Release unless another is given.

| Option | Default | |
|---|---|---|
| `CMAKE_BUILD_TYPE` | `Release` | `Debug` builds GAOL without optimization, with debugging information |
| `CMAKE_INSTALL_PREFIX` | the system's | Where `cmake --install` puts GAOL |
| `GAOL_BUILD_MATHLIB` | `ON` | Download and build mathlib when no installed mathlib is found |
| `GAOL_FIND_MATHLIB` | `ON` | Look for an installed mathlib before building one; `OFF` builds mathlib even where one is installed, as a project building GAOL for itself does |
| `MATHLIB_DIR` | | Installation prefix of an installed mathlib |
| `GAOL_BUILD_TESTS` | `OFF` | Build the tests of `tests/`, which `ctest` runs; no build compiles tests by default |
| `GAOL_SIMD` | `ON` | Compute the intervals with SSE2 instructions on x86 processors, and `gaol::interval2f` with SSE3 (`-msse2 -msse3`); not with Visual C++ nor on 32-bit Windows |
| `GAOL_ASM` | `ON` | Use GAOL's assembly code where it has some (`GAOL_USING_ASM`) |
| `GAOL_VERBOSE_MODE` | `OFF` | Write a line on the standard error when GAOL initializes and cleans up (`GAOL_VERBOSE_MODE`); GAOL is silent by default |
| `GAOL_PRESERVE_ROUNDING` | `OFF` | Restore the rounding direction found after each operation, rather than leaving it upward (see [The rounding direction](#the-rounding-direction)) |

### With autotools

```bash
sh scripts/install-mathlib.sh <mathlib>   # unless mathlib is installed already
./configure --prefix=<prefix> --with-mathlib-include=<mathlib>/include --with-mathlib-lib=<mathlib>/lib
make
make install
```

`scripts/install-mathlib.sh <mathlib>` installs mathlib as the CMake build
does, for configure: it downloads mathlib 2.1.1 (checked against its SHA256),
applies the fixes of `cmake/mathlib/prepare.cmake`, builds it with the flags
of interval arithmetic and installs it under `<mathlib>`. It needs cmake, a C
compiler, curl and tar. `--with-mathlib-include` and `--with-mathlib-lib` then
tell configure where mathlib is (a GAOL installed by the CMake build provides
it the same way, under its own prefix).

See also `INSTALL`. `configure` is committed, generated by autoconf 2.69 from
`configure.ac`; `make` does not regenerate it as long as the files keep the
dates of the checkout. The options, with their defaults:

| Option | Default | |
|---|---|---|
| `--with-mathlib=apmathlib\|crlibm\|m` | `apmathlib` | The mathematical library: mathlib (`ultim`) or CRlibm, whose bounds are certified, or `m`, the math library of the system, whose bounds are not (below) |
| `--with-mathlib-include=DIR`, `--with-mathlib-lib=DIR` | | Where its header and its library are, when not in the usual paths |
| `--enable-optimize` | `yes` | `-O3 -funroll-loops -fomit-frame-pointer -fexpensive-optimizations` and `NDEBUG`; `--disable-optimize` compiles with `-O` |
| `--enable-debug` | `no` | `-g` and GAOL's assertions (`GAOL_DEBUGGING`) |
| `--enable-simd` | `yes` | The SSE2 intervals and `gaol::interval2f` on x86 processors, as `GAOL_SIMD` |
| `--enable-asm` | `yes` | GAOL's assembly code, as `GAOL_ASM` |
| `--enable-verbose-mode` | `no` | The line on the standard error, as `GAOL_VERBOSE_MODE` |
| `--enable-preserve-rounding` | `no` | Restore the rounding direction after each operation, as `GAOL_PRESERVE_ROUNDING` |
| `--enable-relations=set\|certainly\|possibly` | `certainly` | What the relation symbols (`<`, `==`...) mean on intervals |
| `--enable-exceptions` | `yes` | Raise exceptions to signal errors, rather than abort |
| `--with-cppunit-include=DIR`, `--with-cppunit-lib=DIR` | | CppUnit, for GAOL's own check programs (`make check`) |

With `--with-mathlib=m` (`-Dwith-mathlib=default` for meson), GAOL computes its
elementary functions with the math library of the system, whose results it
widens slightly, and configure and meson warn that the bounds are not
certified. Built so with glibc 2.31 on x86-64, 11 of the 2993 checks of
`elementary` gave bounds not enclosing the exact values, near the overflow of
`exp`, `sinh` and `cosh`; 1144 of the checks of `other_functions` failed,
`acos_rel`, `asin_rel` and `atan_rel` not containing the values they had to;
and 630 checks of `rounding_direction`, the elementary functions, `pow` and
`nth_root` leaving the rounding direction to nearest. mathlib, the default of
every build, and CRlibm give certified bounds.

### With meson

```bash
meson setup build --prefix=<prefix> -Dwith-mathlib-include=<mathlib>/include -Dwith-mathlib-lib=<mathlib>/lib
meson compile -C build
meson install -C build
```

With mathlib installed under `<mathlib>`, as for autotools above. The options
(`-D<option>=<value>`), with their defaults:

| Option | Default | |
|---|---|---|
| `buildtype` | `release` | `-O3` and `NDEBUG`; `debug` builds GAOL without optimization, with debugging information |
| `with-mathlib` | `apmathlib` | `apmathlib` (mathlib, `ultim`), `crlibm`, or `default`, the math library of the system, whose bounds are not certified (see `--with-mathlib=m` above) |
| `with-mathlib-include`, `with-mathlib-lib` | | Where its header and its library are, when not in the usual paths, as with configure |
| `enable-optimize` | `true` | `-funroll-loops -fomit-frame-pointer -fexpensive-optimizations`, as configure |
| `enable-debug` | `false` | GAOL's assertions (`GAOL_DEBUGGING`) |
| `enable-simd` | `true` | The SSE2 intervals and `gaol::interval2f` on x86 processors, as `GAOL_SIMD` |
| `enable-asm` | `true` | GAOL's assembly code, as `GAOL_ASM` |
| `enable-verbose-mode` | `false` | The line on the standard error, as `GAOL_VERBOSE_MODE` |
| `enable-preserve-rounding` | `false` | Restore the rounding direction after each operation, as `GAOL_PRESERVE_ROUNDING` |
| `enable-relations` | `certainly` | `set`, `certainly` or `possibly`, as configure |
| `enable-exception` | `true` | Raise exceptions to signal errors, rather than abort |
| `with-test` | `false` | Build GAOL's own check programs, which need CppUnit |
| `check-perf`, `with-doc` | `false` | GAOL's performance programs, the documentation |

## Using GAOL

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
- `-ffloat-store` where doubles are still computed on the x87 unit.

With Visual C++, `/fp:strict`. Each build installs them with GAOL, in
`gaol::gaol` and `gaol.pc` (below). `gaol/gaol_config.h` refuses code compiled
by Visual C++ without `/fp:strict`. GCC and Clang do not tell the code whether
`-frounding-math` and `-ffp-contract=off` were given: there, it only refuses
what contradicts them, `-ffast-math` and doubles computed on the x87 unit (see
[Compilers and options refused](#compilers-and-options-refused)).

### From CMake

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
set(GAOL_FIND_MATHLIB OFF)   # mathlib downloaded and built along, whatever the machine has
FetchContent_MakeAvailable(gaol)
target_link_libraries(my_target PUBLIC gaol::gaol)
```

`cmake --install` of the project then installs GAOL and mathlib with it.

### From pkg-config

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

### The rounding direction

Each operation of GAOL sets the rounding direction upward when it is not, and
leaves it upward, whichever way GAOL is built. The bounds are then right
whatever rounding direction the calling code left. The check is an addition,
1 + 2^-60, above 1 only when rounded upward, rather than a reading of the
rounding direction, which cost far more under Rosetta 2 and with 32-bit Visual
C++ (see [What differs from GAOL](#what-differs-from-gaol)). Code that needs
its own rounding direction after GAOL's operations builds GAOL with
`GAOL_PRESERVE_ROUNDING` (`--enable-preserve-rounding`,
`-Denable-preserve-rounding=true`): each operation then also restores the
rounding direction it found, which makes the arithmetic operations several
times slower.

## The three builds

On a given machine with a given compiler, GAOL behaves the same whichever
build configured it: CMake, configure and meson give it the same macros and the
same flags. The reference is the autotools build of GAOL, then its meson
build, and the CMake build follows them, apart from the errors corrected (see
[What differs from GAOL](#what-differs-from-gaol)). In every build, by default:

- GAOL is compiled in release: `-O3` and the optimizations configure adds
  (`-funroll-loops -fomit-frame-pointer -fexpensive-optimizations`, each where
  the compiler takes it), `NDEBUG`, `-std=c++11`, hidden visibility
  (`-fvisibility=hidden -fvisibility-inlines-hidden`) and `-Wall -Wconversion`,
  which GAOL compiles without warnings, `-Wsign-conversion` of Clang included;
- with the flags of interval arithmetic of [Using GAOL](#using-gaol);
- on x86 processors, the intervals are computed with SSE2 instructions and
  `gaol::interval2f` with SSE3, except with Visual C++ and on 32-bit Windows,
  where a `std::vector` of SSE2 intervals crashes: GCC takes the memory of
  `new` to be aligned on 16 bytes there, while the C runtime aligns it on 8;
- with mathlib, exceptions, the "certainly" relations, GAOL's assembly
  (`GAOL_USING_ASM`), the rounding direction left upward, and silent: no line
  on the standard error when GAOL initializes and cleans up, unless
  `GAOL_VERBOSE_MODE` (`--enable-verbose-mode`, `-Denable-verbose-mode=true`)
  is asked for, where configure wrote it by default;
- the processor and the system (`IX86_LINUX`, `AARCH64_LINUX`...), the sizes
  of the integer types and the byte order are read from the macros of the
  compiler, in `gaol/gaol_config.h`, rather than from the machine building.

`.github/audit/` configures the three builds and compares the macros GAOL
sees and the flags it is compiled with (`compare.py --check`); the continuous
integration runs it on each kind of machine.

### Compilers and options refused

Each of these gave bounds not enclosing the exact results, or worse. The three
builds refuse the compilers of the first four rows when configuring, with a
message naming what to use instead, and keep the options of the last three away
by giving GAOL the flags of [Using GAOL](#using-gaol). `gaol/gaol_config.h`
refuses them again at compile time, for the code including GAOL's headers too,
all but the second row, which no macro of the compiler shows:

| Refused | Because |
|---|---|
| Clang for 32-bit ARM processors | It does not honour the rounding direction there: built by Clang 21, 4556 of 16000 random products, squares and cubes did not enclose their exact values. GCC does. |
| A compiler saying of `-frounding-math` "overriding currently unsupported rounding mode on this target", as Clang 14 for 64-bit ARM | Bounds of `pow()` and `nth_root()` did not enclose the exact values. Clang 18 honours the rounding direction there. |
| mingw-w64 older than version 12 (MinGW-w64 GCC 11 to 13) | Its math library gave `acosh()` near 1 up to 25 million floats away from the exact value, and `asinh()` of large negative numbers NaN. |
| mingw-w64 12 (MinGW-w64 GCC 14.2, rt_v12) | Its `fesetround()` runs the instruction `cpuid` at each call: in a virtual machine, `exp()`, `log()`, `sin()` and `cos()` took 10.6 to 13.8 microseconds rather than 0.5 to 0.7 with mingw-w64 13 (MinGW-w64 GCC 15.2, MSYS2). |
| `-ffast-math`, `-Ofast`, `/fp:fast` | The compiler then rounds to nearest and drops the checks of NaN and infinities. |
| Visual C++ without `/fp:strict` (`/fp:precise`, its default) | Visual C++ then assumes rounding to nearest, and may evaluate or rewrite floating-point operations accordingly: no test gave a wrong bound so, but nothing certifies the bounds (see [What differs from GAOL](#what-differs-from-gaol)). |
| Doubles computed on the x87 unit of 32-bit x86 processors (without `-msse2 -mfpmath=sse`, or `/arch:SSE2`) | In extended precision, GAOL's bounds and mathlib's results are wrong: built for an i686 computing on the x87, `exp`, `sin` and `cos` missed the exact value for most arguments. |

The three builds also refuse a mathlib they find installed whose cosine of
2^52 − 1 is wrong, running a program linked with it. mathlib compiled with the
contraction of multiplications and additions into fused multiply-adds, which
GCC does by default wherever the processor has them, 64-bit ARM processors
included, unless given `-ffp-contract=off`, reduces large arguments modulo π/2
wrongly (`branred()`): compiled so on x86_64, it gave cos(2^52 − 1) about
−0.4855 rather than 0.4733, and sin, cos and tan far from their values at 534 of
20080 random arguments from 2^26 to 2^54, which GAOL's bounds did not enclose.
The message says to let CMake build mathlib (`-DGAOL_FIND_MATHLIB=OFF`) or to
install it with `scripts/install-mathlib.sh`, both of which compile it with
`-ffp-contract=off`. When the program cannot run, cross-compiling without an
emulator, the builds only warn.

## Tests

The programs of `tests/` compare the bounds GAOL computes with the exact results
of the operations, independently of GAOL and of the floating-point environment.
The exact results are computed with integers, or were computed with 2000 bits of
precision by [mpmath](https://mpmath.org). They follow the rounding tests of
Codac.

- **`arithmetic`:** on doubles and intervals of every magnitude (subnormal
  doubles and overflows included), sums, differences, products, quotients,
  relational divisions, squares, inverses, `abs`, `min`, `max`, `&`, `|` have to
  be the tightest enclosures. Integer powers, square roots and n-th roots have
  to be enclosures, within a few doubles. The operators of an interval with a
  double have to give, on bounds and doubles of special values (zeros of both
  signs, infinities, NaN), the sets the operators with `interval(d)` give.
  Products of intervals with zero and infinite bounds have to be the hull of
  the products of the bounds, a zero bound times an infinite one counting as 0.
  The constructors have to give the empty set for a lower bound of `+oo`, an
  upper bound of `-oo`, bounds in the wrong order and NaN bounds.
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
- **`other_functions`:** midpoints (of subnormal bounds and of `intervalf`
  too), widths, magnitudes, mignitudes, Hausdorff distances, splitting, integer
  parts, and the relational functions (`sqrt_rel`, `div_rel`...).

The CMake build compiles them with `GAOL_BUILD_TESTS` (`OFF` by default: no
build compiles tests unless asked to, as `make check` and `with-test` for
GAOL's own check programs). `tests/find_package` builds the same tests with
an installed GAOL, and `.github/scripts/tests.sh` with a GAOL installed by
configure or meson.

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

## What differs from GAOL

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
  [Compilers and options refused](#compilers-and-options-refused)). A TODO of
  GAOL's check `reverse_mappings` said that `acos_rel()` failed at
  [2^52 − 1, 2^52 − 1/2] under AArch64: mathlib compiled there without
  `-ffp-contract=off`, as its own configure compiles it, gives that cosine far
  from its value. `tests/elementary.cpp` checks sin and cos at arguments where
  such a mathlib fails.
- **The three builds agree** (see [The three builds](#the-three-builds)).
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

## Continuous integration

The workflows of `.github/workflows/` build GAOL with CMake and run the tests
on:

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
  - MinGW-w64 15, on x86 and x64;
  - MSYS2 UCRT64 (GCC) and CLANG64 (Clang).

They also build GAOL with autotools and meson, against an installed mathlib,
on Ubuntu (x86_64, arm64), Debian (i386,
armhf), macOS (arm64, x86_64) and MSYS2, and the tests with the GAOL they
install; check that the three builds agree on each of these machines; build
GAOL with CMake against an installed mathlib (`MATHLIB_DIR`); check
that Clang is refused on 32-bit ARM, Clang 14 on 64-bit ARM, and MinGW-w64 11
to 14 (13 and 14 with autotools and meson too), and Visual C++ without
`/fp:strict`. Jobs of each build restore the
rounding direction (`GAOL_PRESERVE_ROUNDING`): Ubuntu x86_64 GCC and arm64
Clang, Debian i386 and armhf, macOS arm64, Visual Studio x64, autotools and
meson. The jobs built in Release print the time per operation in their summary.

## Licences

GAOL, by [Frédéric Goualard](https://frederic.goualard.net/), is distributed
under the GNU LGPL v2 (`COPYING.LIB`). mathlib, which the
CMake build downloads, is distributed under the GNU LGPL v2 or later, as the
headers of its sources state (its archive carries the text of the GNU GPL v2).
