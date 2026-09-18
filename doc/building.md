# Building GAOL

Part of the documentation of [this fork of GAOL](../README.md#documentation).

GAOL computes its elementary functions with mathlib, the IBM Accurate Portable
Mathematical Library (libultim), whose sources are in `3rd/mathlib`:
[mathlib-2.1.1.tar.gz](https://frederic.goualard.net/software/mathlib-2.1.1.tar.gz)
of Frédéric Goualard's site, with the fixes of this fork (see
[3rd/README.md](../3rd/README.md) and [What differs from GAOL](differences.md):
as it is, `cos()`, `atan()` and `log()` of mathlib are wrong at some
arguments). Each build compiles it with the flags of interval arithmetic and
installs it along with GAOL, unless told to use a mathlib installed already.
The CMake build is the one to use; the autotools and meson builds of GAOL are
kept for those who use them. The math library of the system (`-lm`) needs no
installation. The autotools and meson builds can also build GAOL with
[CRlibm](https://github.com/taschini/crlibm) instead (`crlibm`), which
`sh scripts/install-crlibm.sh <prefix>` builds and installs; GAOL's headers
then include `crlibm.h`, whose directory `gaol.pc` carries when
`with-mathlib-include` gives it.

## With CMake

```bash
cmake -S . -B build -DCMAKE_INSTALL_PREFIX=<prefix> -DGAOL_BUILD_TESTS=ON
cmake --build build --config Release
ctest --test-dir build -C Release
cmake --install build --config Release
```

CMake 3.14 or later. The build compiles the mathlib of `3rd/mathlib`
(`3rd/mathlib/CMakeLists.txt`) and installs it along with GAOL, unless
`GAOL_FIND_MATHLIB` is `ON`, which giving `MATHLIB_DIR` makes it by default:
it then looks for an installed mathlib, under `MATHLIB_DIR` and in the usual
paths, refuses one whose cosine is wrong (see
[Compilers and options refused](three-builds.md#compilers-and-options-refused)),
and compiles the one of `3rd/mathlib` when none is found. Both libraries are
static. The build type is Release unless another is given; brought in by a
project that gives none (`add_subdirectory`, FetchContent), GAOL, CORE-MATH
and mathlib are compiled with `-O3` all the same (`/O2` with Visual C++).

| Option | Default | |
|---|---|---|
| `CMAKE_BUILD_TYPE` | `Release` | `Debug` builds GAOL without optimization, with debugging information |
| `CMAKE_INSTALL_PREFIX` | the system's | Where `cmake --install` puts GAOL |
| `CMAKE_POSITION_INDEPENDENT_CODE` | `ON` | Compile GAOL and mathlib as position-independent code (`-fPIC`), which linking them into a shared library needs; a project building GAOL with FetchContent that sets it is followed |
| `MATHLIB_DIR` | | Installation prefix of an installed mathlib, which turns `GAOL_FIND_MATHLIB` on by default |
| `GAOL_FIND_MATHLIB` | `OFF`; `ON` with `MATHLIB_DIR` | Use an installed mathlib, under `MATHLIB_DIR` or in the usual paths, rather than the one of `3rd/mathlib` |
| `GAOL_BUILD_MATHLIB` | `ON` | Build the mathlib of `3rd/mathlib` when no installed mathlib is used; `OFF`, with `GAOL_FIND_MATHLIB` `OFF`, leaves mathlib to the code linking GAOL |
| `GAOL_BUILD_TESTS` | `OFF` | Build the tests of `tests/`, which `ctest` runs; no build compiles tests by default |
| `GAOL_SIMD` | `ON` | Compute the intervals with SSE2 instructions on x86 processors (`-msse2 -msse3`); not with Visual C++ nor on 32-bit Windows |
| `GAOL_FLOAT_INTERVALS` | `OFF` | Compile the intervals of floats, `gaol::intervalf`, and `gaol::interval2f` where `GAOL_SIMD` gives SSE3: both are unfinished, and neither IBEX nor Codac uses them |
| `GAOL_FMA` | `ON` | Compile GAOL, CORE-MATH and mathlib with the fused multiply-add instructions of the processor, where the compiler has a flag for them and the machine building runs a program compiled with it (not checked when cross-compiling): `-mfma` on x86 (not with GCC for Windows), `/arch:AVX2` with Visual C++ for x64, `-mfpu=neon-vfpv4 -mfloat-abi=hard` on 32-bit ARM; 64-bit ARM, POWER, s390x and RISC-V have them without a flag. The library then needs a processor with them (on x86, Intel Haswell and AMD Piledriver, 2012-2013, and later); `OFF` builds it for any processor of the architecture. The code using GAOL is given the flag too, in `gaol::gaol` and `gaol.pc`, and `-ffp-contract=off` stays (see [The three builds](three-builds.md)) |
| `GAOL_ASM` | `ON` | Use GAOL's assembly code where it has some (`GAOL_USING_ASM`) |
| `GAOL_VERBOSE_MODE` | `OFF` | Write a line on the standard error when GAOL initializes and cleans up (`GAOL_VERBOSE_MODE`); GAOL is silent by default |
| `GAOL_PRESERVE_ROUNDING` | `OFF` | Restore the rounding direction found after each operation, rather than leaving it upward (see [The rounding direction](using.md#the-rounding-direction)) |

## With autotools

```bash
./configure --prefix=<prefix>
make
make install
```

configure builds the mathlib of `3rd/mathlib` (`3rd/Makefile.am`) with the
flags it gives GAOL's C code and installs it along with GAOL, unless
`--with-mathlib-include` or `--with-mathlib-lib` says where an installed
mathlib is: `./configure --with-mathlib-include=<mathlib>/include
--with-mathlib-lib=<mathlib>/lib` uses that one (a GAOL installed by any of
the builds provides it so, under its own prefix), and refuses one whose cosine
is wrong. `sh scripts/install-mathlib.sh <mathlib>` installs the mathlib of
`3rd/mathlib` alone, under `<mathlib>`, with cmake and a C compiler.

See also `INSTALL`. `configure` is committed, generated by autoconf 2.69 from
`configure.ac`; `make` does not regenerate it as long as the files keep the
dates of the checkout. The options, with their defaults:

| Option | Default | |
|---|---|---|
| `--with-mathlib=apmathlib\|crlibm\|m` | `apmathlib` | The mathematical library: mathlib (`ultim`) or CRlibm, whose bounds are certified, or `m`, the math library of the system, whose bounds are not (below) |
| `--with-mathlib-include=DIR`, `--with-mathlib-lib=DIR` | | Where the header and the library of an installed mathlib are, given to use it rather than the one of `3rd/mathlib`; for CRlibm, when not in the usual paths |
| `--enable-optimize` | `yes` | `-O3 -funroll-loops -fomit-frame-pointer -fexpensive-optimizations` and `NDEBUG`, for the C++ and C sources alike (GAOL, CORE-MATH, mathlib); `--disable-optimize` compiles with `-O` |
| `--enable-debug` | `no` | `-g` and GAOL's assertions (`GAOL_DEBUGGING`) |
| `--enable-simd` | `yes` | The SSE2 intervals on x86 processors, as `GAOL_SIMD` |
| `--enable-float-intervals` | `no` | `gaol::intervalf` and `gaol::interval2f`, as `GAOL_FLOAT_INTERVALS` |
| `--enable-fma` | `yes` | The fused multiply-add instructions of the processor, as `GAOL_FMA` |
| `--enable-asm` | `yes` | GAOL's assembly code, as `GAOL_ASM` |
| `--enable-verbose-mode` | `no` | The line on the standard error, as `GAOL_VERBOSE_MODE` |
| `--enable-preserve-rounding` | `no` | Restore the rounding direction after each operation, as `GAOL_PRESERVE_ROUNDING` |
| `--enable-relations=set\|certainly\|possibly` | `certainly` | What the relation symbols (`<`, `==`...) mean on intervals |
| `--enable-exceptions` | `yes` | Raise exceptions to signal errors, rather than abort |
| `--with-cppunit-include=DIR`, `--with-cppunit-lib=DIR` | | CppUnit, for GAOL's own check programs (`make check`) |

With `--with-mathlib=m` (`-Dwith-mathlib=default` for meson), GAOL computes its
elementary functions with the math library of the system, whose results it
widens slightly, and configure and meson warn that the bounds are not
certified: they are valid only where the math library is within about one
double of the exact value. Built so with glibc 2.31 on x86-64, every bound the
tests check encloses the exact value, but 840 of the 4735 checks of
`elementary` fail, their bounds being further from the tightest than the tests
allow, `sin`, `cos`, `pow(x, y)` and `atan2` being up to 4 doubles away where
the tests want one; 960 of the checks of `other_functions` fail, `acos_rel`,
`asin_rel` and `atan_rel` keeping the values they had to but being further
than 2^-49 from them. The checks of `arithmetic`, `numbers` and
`rounding_direction` pass, and the continuous integration checks that no other
check than those fails. mathlib, the default of every build, and CRlibm give
certified bounds, and the five tests pass with both.

## With meson

```bash
meson setup build --prefix=<prefix>
meson compile -C build
meson install -C build
```

meson builds the mathlib of `3rd/mathlib` (`3rd/mathlib/meson.build`) and
installs it along with GAOL, unless `with-mathlib-include` or
`with-mathlib-lib` says where an installed mathlib is, as for autotools above.
`meson compile` needs meson 0.54; with an older one, as the meson 0.53 of
Ubuntu 20.04, `ninja -C build` builds GAOL as well. The options
(`-D<option>=<value>`), with their defaults:

| Option | Default | |
|---|---|---|
| `buildtype` | `release` | `-O3` and `NDEBUG`; `debug` builds GAOL without optimization, with debugging information |
| `with-mathlib` | `apmathlib` | `apmathlib` (mathlib, `ultim`), `crlibm`, or `default`, the math library of the system, whose bounds are not certified (see `--with-mathlib=m` above) |
| `with-mathlib-include`, `with-mathlib-lib` | | Where the header and the library of an installed mathlib are, as with configure |
| `enable-optimize` | `true` | `-funroll-loops -fomit-frame-pointer -fexpensive-optimizations`, as configure |
| `enable-debug` | `false` | GAOL's assertions (`GAOL_DEBUGGING`) |
| `enable-simd` | `true` | The SSE2 intervals on x86 processors, as `GAOL_SIMD` |
| `enable-float-intervals` | `false` | `gaol::intervalf` and `gaol::interval2f`, as `GAOL_FLOAT_INTERVALS` |
| `enable-fma` | `true` | The fused multiply-add instructions of the processor, as `GAOL_FMA` |
| `enable-asm` | `true` | GAOL's assembly code, as `GAOL_ASM` |
| `enable-verbose-mode` | `false` | The line on the standard error, as `GAOL_VERBOSE_MODE` |
| `enable-preserve-rounding` | `false` | Restore the rounding direction after each operation, as `GAOL_PRESERVE_ROUNDING` |
| `enable-relations` | `certainly` | `set`, `certainly` or `possibly`, as configure |
| `enable-exception` | `true` | Raise exceptions to signal errors, rather than abort |
| `with-test` | `false` | Build GAOL's own check programs, which need CppUnit |
| `check-perf`, `with-doc` | `false` | GAOL's performance programs; the documentation: the target `pdf` builds the manual, `manual/gaol.pdf`, with pdflatex, bibtex and makeindex (`make -C manual pdf` with autotools) |
