<!-- Copyright (c) 2026 ENSTA, France
     Created 2026-09-20 by Jordan NININ -->
# Building GAOL

Part of the documentation of [GAOL v5](../README.md#documentation).

GAOL computes its elementary functions with
[CORE-MATH](https://core-math.gitlabpages.inria.fr/), whose sources are in
`3rd/math-core` and are compiled into `libgaol` itself: **there is no
mathematical library to install, to find or to link along with GAOL**, and no
choice to make. See [3rd/README.md](../3rd/README.md) for the version taken and
what differs from it.

The bounds GAOL computes are the same on every architecture and with every
compiler, and they are the tightest ones (see
[Accuracy of the operations](accuracy.md)).

The CMake build is the one to use; the autotools and meson builds of GAOL are
kept for those who use them.

## With CMake

```bash
cmake -S . -B build -DCMAKE_INSTALL_PREFIX=<prefix> -DWITH_TESTS=ON
cmake --build build --config Release
cmake --build build --target test      # make test with the Makefiles of CMake
cmake --install build --config Release
```

CMake 3.14 or later. The build compiles the thirty-six sources of CORE-MATH into
`libgaol`, which is static unless `BUILD_SHARED_LIBS` is `ON`: there is
nothing else to build and nothing else to install. The build type is Release
unless another is given; brought in by a project that gives none
(`add_subdirectory`, FetchContent), GAOL and CORE-MATH are compiled with `-O3`
all the same (`/O2` with Visual C++).

| Option | Default | |
|---|---|---|
| `CMAKE_BUILD_TYPE` | `Release` | `Debug` builds GAOL without optimization, with debugging information, and GAOL checks its assertions (`GAOL_DEBUGGING`), as `configure --enable-debug` and `meson setup --buildtype=debug` |
| `CMAKE_INSTALL_PREFIX` | the system's | Where `cmake --install` puts GAOL |
| `BUILD_SHARED_LIBS` | `OFF` | `ON` builds the shared library `libgaol.so.5.0.0` (soname `libgaol.so.5`), as configure and meson do; always static with Visual C++ |
| `CMAKE_POSITION_INDEPENDENT_CODE` | `ON` | Compile GAOL as position-independent code (`-fPIC`), which linking them into a shared library needs; a project building GAOL with FetchContent that sets it is followed |
| `WITH_TESTS` | `OFF` | Build the unit tests of `tests/`, which `make test` and `make check` run (see [below](#tests-examples-performance-and-the-parser)) |
| `WITH_EXAMPLES` | `OFF` | Build the examples of `examples/`, which `make check` runs |
| `GAOL_SIMD` | `ON` | Compute the intervals with SSE2 instructions on x86 processors (`-msse2 -msse3`); not with Visual C++ nor on 32-bit Windows |
| `GAOL_COVERAGE` | `OFF` | Compile GAOL and its tests with the counters of gcov, and add the target `coverage`, which runs the tests and writes `coverage/coverage.html` and the page of conclusions [coverage/README.md](../coverage/README.md). GCC and Clang, with `-DCMAKE_BUILD_TYPE=Debug` |
| `GAOL_U128_EMULATION` | `OFF` | Compute the 128-bit integer of the accurate phases with two 64-bit halves, as where the compiler has no 128-bit type (Visual C++, and GCC for a 32-bit target): the way to run that code on an ordinary machine |
| `GAOL_FMA` | `ON` | Compile GAOL and CORE-MATH with the fused multiply-add instructions of the processor, where the compiler has a flag for them and the machine building runs a program compiled with it (not checked when cross-compiling): `-mfma` on x86 (not with GCC for Windows), `/arch:AVX2` with Visual C++ for x64, `-mfpu=neon-vfpv4 -mfloat-abi=hard` on 32-bit ARM; 64-bit ARM, POWER, s390x and RISC-V have them without a flag. The library then needs a processor with them (on x86, Intel Haswell and AMD Piledriver, 2012-2013, and later); `OFF` builds it for any processor of the architecture. The code using GAOL is given the flag too, in `gaol::gaol` and `gaol.pc`, and `-ffp-contract=off` stays (see [The three builds](three-builds.md)) |
| `GAOL_ASM` | `ON` | Use GAOL's assembly code where it has some (`GAOL_USING_ASM`) |
| `GAOL_VERBOSE_MODE` | `OFF` | Write a line on the standard error when GAOL initializes and cleans up (`GAOL_VERBOSE_MODE`); GAOL is silent by default |
| `GAOL_PRESERVE_ROUNDING` | `OFF` | Restore the rounding direction found after each operation, rather than leaving it upward (see [The rounding direction](using.md#the-rounding-direction)) |

### The archive of the sources and the packages

CPack makes them from the CMake build, where `make dist` of the autotools
build made the archive of the sources:

```bash
git clone https://github.com/Jordan08/GAOL.git gaol && cd gaol
cmake -S . -B build -DCMAKE_INSTALL_PREFIX=/usr -DGAOL_FMA=OFF
cmake --build build --config Release
cmake --build build --target package_source
cmake --build build --target package
```

| File | What it holds |
|---|---|
| `gaol-<version>.tar.gz` (`package_source`) | The sources, in `gaol-<version>/`, with `configure` and the `Makefile.in`: the three builds build them. CPack takes the source tree as it is, less what git ignores and the builds made within it, so that made from a clean checkout, the archive holds the files of the commit. |
| `gaol-<version>-<system>.tar.gz` (`package`) | What `cmake --install` installs, in `gaol-<version>-<system>/`: the headers, `libgaol.a` (or `libgaol.so*` with `BUILD_SHARED_LIBS`), the CMake package of GAOL and `gaol.pc`, to extract anywhere, `gaol.pc` finding its directories from where it is. |
| `libgaol-dev_<version>_<arch>.deb` (`package`, on Linux) | The same files in `/usr`, for `apt install ./libgaol-dev_<version>_<arch>.deb`: by default a static library and its headers, hence the name and the section `libdevel` of the development packages of Debian. |

The packages hold `libgaol` as the build compiled it: with `GAOL_FMA` `ON`,
the default, only the processors that have the fused multiply-add
instructions run it, and the code using it is compiled with `-mfma`;
`-DGAOL_FMA=OFF` makes packages for any processor of the architecture.
`-DCMAKE_INSTALL_PREFIX=/usr` gives the library directory of Debian and Ubuntu
(`lib/x86_64-linux-gnu`); with another prefix, the `.deb` puts the files in
`/usr/lib`.

## With autotools

```bash
./configure --prefix=<prefix> --with-tests
make
make test
make install
```

configure compiles the thirty-six sources of CORE-MATH into `libgaol`
(`gaol/Makefile.am`), with the flags it gives GAOL's C code: there is nothing
else to build, nothing else to install and nothing else to link.

See also `INSTALL`. `configure`, `aclocal.m4`, the `Makefile.in`, `ltmain.sh`,
`m4/*.m4` and the scripts `compile`, `config.guess`, `config.sub`, `depcomp`,
`install-sh`, `missing` and `test-driver` are committed, so that a checkout
builds without the autotools, and `make` never regenerates them
(`AM_MAINTAINER_MODE([disable])`). They are generated by autoconf 2.72,
automake 1.18.1 and libtool 2.5.4, with `AUTOHEADER=true autoreconf --force
--install` followed by `git checkout INSTALL`, which `--install` replaces by
the text of automake; `AUTOHEADER=true` keeps `gaol/gaol_configuration.h.in`,
written by hand, which autoheader 2.72 overwrites (2.73 leaves it). `config.guess`
and `config.sub` are those of 2026-05-17 from
[GNU config](https://git.savannah.gnu.org/cgit/config.git), newer than the ones
libtool installs. The versions matter: the files of GAOL 4.2.2 came from
libtool 2.4.2, which linked `libgaol` with `-flat_namespace -undefined
suppress` on macOS 11 and later, taking them for Mac OS X 10.0, and from a
`config.sub` of 2016, which refused `loongarch64`. There is no `make dist` nor
`make distcheck`: the archive of the sources is made with CMake (see
[above](#the-archive-of-the-sources-and-the-packages)). The options, with
their defaults:

| Option | Default | |
|---|---|---|
| `--enable-optimize` | `yes` | `-O3 -funroll-loops -fomit-frame-pointer -fexpensive-optimizations` and `NDEBUG`, for the C++ and C sources alike (GAOL and CORE-MATH), as the Release build of CMake; `--disable-optimize` compiles with `-O` |
| `--enable-debug` | `no` | The Debug build of CMake: `-g`, no optimization and no `NDEBUG` (`--enable-optimize` is then ignored), and GAOL's assertions (`GAOL_DEBUGGING`) |
| `--enable-simd` | `yes` | The SSE2 intervals on x86 processors, as `GAOL_SIMD` |
| `--enable-fma` | `yes` | The fused multiply-add instructions of the processor, as `GAOL_FMA` |
| `--enable-asm` | `yes` | GAOL's assembly code, as `GAOL_ASM` |
| `--enable-verbose-mode` | `no` | The line on the standard error, as `GAOL_VERBOSE_MODE` |
| `--enable-preserve-rounding` | `no` | Restore the rounding direction after each operation, as `GAOL_PRESERVE_ROUNDING` |
| `--enable-exceptions` | `yes` | Raise exceptions to signal errors, rather than abort |
| `--with-tests` | `no` | Build the unit tests of `tests/`, which `make test` and `make check` run, as `WITH_TESTS` |
| `--with-examples` | `no` | Build the examples of `examples/`, which `make check` runs, as `WITH_EXAMPLES` |

`make` builds `libgaol.a` and the shared library `libgaol.so.5.0.0`, whose
soname is `libgaol.so.5`, as CMake (`BUILD_SHARED_LIBS`) and meson name them;
it was `libgaol-5.0.so.0`. `configure --help` ends with the address for the bug
reports, jordan.ninin@ensta.fr, and the page of GAOL v5,
https://github.com/Jordan08/GAOL.

## With meson

```bash
meson setup build --prefix=<prefix> -Dwith-tests=true
meson compile -C build
meson test -C build
meson install -C build
```

meson compiles the thirty-six sources of CORE-MATH into `libgaol`
(`gaol/meson.build`), as CMake and configure do, and the lexer and the parser
committed (`gaol/gaol_interval_lexer.cpp`, `gaol/gaol_interval_parser.cpp`):
neither flex nor bison is needed.
`meson compile` needs meson 0.54; with an older one, as the meson 0.53 of
Ubuntu 20.04, `ninja -C build` builds GAOL as well. The options
(`-D<option>=<value>`), with their defaults:

| Option | Default | |
|---|---|---|
| `buildtype` | `release` | `-O3`, `NDEBUG` and `-funroll-loops -fomit-frame-pointer -fexpensive-optimizations`, as the Release build of CMake and configure; `debug` builds GAOL without optimization, with debugging information, and GAOL checks its assertions (`GAOL_DEBUGGING`), as the Debug build of CMake and `configure --enable-debug`. The options `enable-optimize` and `enable-debug` are gone |
| `enable-simd` | `true` | The SSE2 intervals on x86 processors, as `GAOL_SIMD` |
| `enable-fma` | `true` | The fused multiply-add instructions of the processor, as `GAOL_FMA` |
| `enable-asm` | `true` | GAOL's assembly code, as `GAOL_ASM` |
| `enable-verbose-mode` | `false` | The line on the standard error, as `GAOL_VERBOSE_MODE` |
| `enable-preserve-rounding` | `false` | Restore the rounding direction after each operation, as `GAOL_PRESERVE_ROUNDING` |
| `enable-exception` | `true` | Raise exceptions to signal errors, rather than abort |
| `with-tests` | `false` | Build the unit tests of `tests/`, which `meson test` and `ninja check` run, as `WITH_TESTS` (it was `with-test`) |
| `with-examples` | `false` | Build the examples of `examples/`, which `ninja check` runs, as `WITH_EXAMPLES` |
| `with-doc` | `false` | The target `pdf`, which builds the manuals, `manual/v5/gaol.pdf` for GAOL v5 and `manual/v4/gaol.pdf` for GAOL 4, with pdflatex, bibtex and makeindex (`make -C manual pdf` with autotools) |

meson builds `libgaol.a` and the shared library `libgaol.so.5.0.0`, whose
soname is `libgaol.so.5`, as configure and CMake name them; with Visual C++,
the static library alone, as CMake does, and `__GAOL_PUBLIC__` defined empty
for the code using it (`gaol.pc`, the dependency `gaol_dep` of a meson
project). The summary of `meson setup` gives the address for the bug reports,
jordan.ninin@ensta.fr, and the page of GAOL v5, https://github.com/Jordan08/GAOL.
The option `check-perf` and the options `enable-relations` and `with-test`,
gone, are refused.

## Tests, examples, performance and the parser

The three builds have the same targets:

| | CMake | configure | meson |
|---|---|---|---|
| The unit tests of `tests/` | `make test` (`cmake --build <build> --target test`; `RUN_TESTS` with Visual Studio), with `WITH_TESTS` | `make test`, with `--with-tests` | `meson test` or `ninja test`, with `with-tests` |
| The unit tests and the examples of `examples/` | `make check` (`--target check`), with `WITH_TESTS` and `WITH_EXAMPLES` | `make check`, with `--with-tests` and `--with-examples` | `ninja check` (`meson test --setup check`), with `with-tests` and `with-examples` |
| GAOL v5 measured on the benchmark of [performance.md](compare/performance.md) | `make perf` | `make perf` | `ninja perf` |
| The lexer and the parser regenerated with flex and bison | `make parser` | `make parser` | `ninja parser` |

- **The tests** are the unit tests of GAOL v5, which compare its bounds with the
  exact results, and those of GAOL 4, which were in `check/` and needed
  CppUnit: they are in `tests/` now, and need nothing (see
  [The tests](tests.md)). `ctest` runs the tests and the examples of a CMake
  build; `make test` leaves the examples out (label `example`, CMake 3.17 and
  later). With meson before 0.57, `meson test` runs the examples too, and
  `meson test --suite unit` the tests alone.
- **The examples** check what they print and fail otherwise (see
  [examples/examples.md](../examples/examples.md)).
- **`make perf`** compiles `doc/compare/code/bench_gaol.cpp` with the flags and
  the library of the build, runs it on the intervals of the benchmark, and
  writes its times into the tables of `doc/compare/performance.md`, with the
  times of the other libraries measured by the last whole run, which
  `doc/compare/code/results.csv` keeps: GAOL 4.2.3, filib++, libieeep1788,
  PROFIL/BIAS and Solaris Studio are not run (see
  [doc/compare/code/README.md](compare/code/README.md)). Linux, bash, and Python
  3 with numpy.
- **`make parser`** regenerates `gaol/gaol_interval_lexer.cpp`,
  `gaol/gaol_interval_parser.cpp` and `gaol/gaol_interval_parser.h` from
  `gaol/gaol_interval_lexer.lpp` and `gaol/gaol_interval_parser.ypp` with the
  flex and the bison of the system, where the CMake build finds them
  (`find_program`, not required). The three files are committed and no build
  needs flex nor bison otherwise; flex 2.6.4 and bison 3.5.1 give them again
  byte for byte. `sh gaol/regenerate_parser.sh` runs the same commands without
  a build (`FLEX` and `BISON` give other programs).

## The configuration of GAOL

Each build writes `gaol/gaol_configuration.h`, which `gaol/gaol_config.h`
includes with every compiler: CMake from `cmake/gaol_configuration.h.in`,
configure from `gaol/gaol_configuration.h.in` (written by hand, not by
autoheader), meson from `gaol/gaol_configuration_meson.h.in`. The three define
the same macros, those GAOL's sources read, and nothing else; the continuous
integration checks that they write the same header on each kind of machine
(`.github/audit`).

| Macro | Defined | CMake | configure | meson |
|---|---|---|---|---|
| `GAOL_MAJOR_VERSION`, `GAOL_MINOR_VERSION`, `GAOL_MICRO_VERSION`, `GAOL_VERSION` | Always: `5`, `0`, `0`, `"5.0.0"` | `project()` | `AC_INIT` | `project()` |
| `GAOL_DEBUGGING` | In a Debug build: GAOL checks its assertions (`GAOL_ASSERT`), and `GAOL_DEBUG` runs its commands | `CMAKE_BUILD_TYPE=Debug` | `--enable-debug` | `--buildtype=debug` |
| `GAOL_EXCEPTIONS_ENABLED` | GAOL raises exceptions rather than abort | always | `--enable-exceptions` (default) | `enable-exception` (default) |
| `GAOL_PRESERVE_ROUNDING` | The operations restore the rounding direction they found | `GAOL_PRESERVE_ROUNDING` | `--enable-preserve-rounding` | `enable-preserve-rounding` |
| `GAOL_USING_ASM` | GAOL's assembly (32-bit x86 Linux and macOS); never with Visual C++ | `GAOL_ASM` (default) | `--enable-asm` (default) | `enable-asm` (default) |
| `GAOL_VERBOSE_MODE` | A line on the standard error when GAOL initializes and cleans up | `GAOL_VERBOSE_MODE` | `--enable-verbose-mode` | `enable-verbose-mode` |
| `USING_SSE2_INSTRUCTIONS` | The intervals computed with SSE2 (x86, not 32-bit Windows nor Visual C++) | `GAOL_SIMD` (default) | `--enable-simd` (default) | `enable-simd` (default) |
| `USING_SSE3_INSTRUCTIONS` | And compiled with `-msse3` | where the compiler takes it | where the compiler takes it | where the compiler takes it |
| `HAVE_ROUNDING_MATH_OPTION` | The compiler takes `-frounding-math` | checked | checked | checked |
| `HAVE_VISIBILITY_OPTIONS` | The library is compiled with `-fvisibility=hidden` | checked | checked | checked |
| `HAVE_FENV_H` | The compiler has `<fenv.h>` | checked (required) | checked | checked |
| `HAVE_GETRUSAGE` | `<sys/resource.h>` declares `getrusage()`, which `gaol/gaol_profile.cpp` measures the time with (`clock()` otherwise) | checked | checked | checked |

What GAOL needs to know of the processor and the system comes from the
compiler, in `gaol/gaol_config.h`: `IX86_LINUX`, `AARCH64_LINUX`,
`IX86_MACOSX` and `ARM_MACOSX`, the sizes of the integer types (`SIZEOF_INT`,
`SIZEOF_LONG_LONG_INT`, from `<limits.h>`) and the order of the bytes
(`WORDS_BIGENDIAN`).

The configure and meson builds defined some fifty other macros, of GAOL 4 or
of autoconf, which nothing read, and they are gone (GAOL v5): the checks of
C headers (`HAVE_STDLIB_H`, `HAVE_UNISTD_H`, `STDC_HEADERS`...), of functions
(`HAVE_FLOOR`, `HAVE_POW`, `HAVE_MALLOC`, `HAVE_REALLOC`, `HAVE_FINITE`...), of
types and sizes (`HAVE__BOOL`, `SIZEOF_LONG`...), `const`, `inline`, `size_t`,
`malloc` and `realloc` (`rpl_malloc()` and `rpl_realloc()` of
`AC_FUNC_MALLOC` and `AC_FUNC_REALLOC` in a cross-compilation), `CLOCK_IN_HEADER`,
`LT_OBJDIR`, `PACKAGE`, `VERSION` and `PACKAGE_*`, which `gaol/gaol_config.h`
undefined, undefining those of the code including GAOL. `HAVE_LIMITS`,
`HAVE_CASSERT`, `HAVE_CLOCK` and `GETRUSAGE_IN_HEADER` were read, but are true
with every compiler of C++11, and `HAVE_NEXTAFTER` and `HAVE_ISNAN` were read
for Visual C++, which has both: the code that read them no longer does.
