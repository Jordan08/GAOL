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

## The version of GAOL

The file `VERSION.txt` holds the version of GAOL, three numbers without
leading zeros such as `5.0.0`, and nothing else: the three builds read it, and
it is the only file to change for a new version (GAOL v5). It is not named
`VERSION`: the root of the sources is on the include path of the three builds,
and on file systems that ignore case (macOS, Windows), `#include <version>`,
which the headers of libc++ write, would find it instead of the standard
header.

- CMake reads it before `project()`, which gives it to `PROJECT_VERSION`, with
  `gaol_read_version()` of `cmake/gaol_version.cmake`; a change of
  `VERSION.txt` configures again. Where GAOL is the project built,
  it reads the line `PACKAGE_VERSION=` of `configure` too, and warns when it
  configures that `configure` was generated for another version than
  `VERSION.txt` holds: the archive of the sources that CPack makes holds
  `configure` as it is committed (see
  [below](#the-archive-of-the-sources-and-the-packages)). It says nothing for
  a tree without `configure`, or with a `configure` that has no such line.
- meson reads it in `project()`, with Python (see [With meson](#with-meson)
  for the Python it takes), which works with meson 0.53 as with later ones
  (`version: files()` needs meson 0.57); a change of `VERSION.txt` configures
  again.
- `configure` reads it when it runs, whence all that follows, and make runs it
  again when `VERSION.txt` changes (`CONFIG_STATUS_DEPENDENCIES` of
  `Makefile.am`). Only what `AC_INIT` writes when autoconf generates
  `configure` (`configure --version`, `configure --help`,
  `config.status --version` and the head of `config.log`) keeps the version of
  the last generation: `configure` is committed, and is to be generated again
  before a new `VERSION.txt` is committed, with the versions of autoconf,
  automake and libtool given [below](#with-autotools) and
  `AUTOHEADER=true autoreconf` (without `--force --install`, the other files
  stay as they are committed; `configure.ac` includes `VERSION.txt` with
  `m4_include`, so that autoconf sees its change despite its cache,
  `autom4te.cache`). Until then, `configure` configures GAOL with the version
  of `VERSION.txt` and warns, twice, that it was generated for another one, and
  the continuous integration fails (`.github/workflows/build-systems.yml`
  compares `configure --version` with `VERSION.txt`). With
  `--enable-maintainer-mode`, `make` generates `configure` again itself, and
  runs no autoheader over `gaol/gaol_configuration.h.in`, which is written by
  hand.

From `VERSION.txt` come the macros `GAOL_MAJOR_VERSION`, `GAOL_MINOR_VERSION`,
`GAOL_MICRO_VERSION` and `GAOL_VERSION` of `gaol/gaol_configuration.h`
(`gaol_core::version_major`... of `gaol/gaol_version.h`), the shared library
`libgaol.so.<major>.<minor>.<micro>` and its soname `libgaol.so.<major>`,
the `Version` of `gaol.pc`, `gaolConfigVersion.cmake` (`find_package(gaol
<version>)` takes a version of the same major number, not older than the one
asked for), the names of the archive and of the packages of CPack, and the
`\version` of the manuals (configure, meson). Each build refuses a
`VERSION.txt` that does not hold three numbers without leading zeros, which
the macros write as C integers, and the three read it by the same rules, as
autoconf does for `configure --version`. The blanks and empty lines around
the version are ignored, the blanks being the six of ASCII (space, tab, line
feed, vertical tab, form feed and carriage return, the CR of the line ends of
Windows), not those of Unicode such as a no-break space; so is a UTF-8 byte
order mark (the bytes EF BB BF) at the start of the file, which some editors
of Windows write. The message of a build that refuses the file quotes
what it read and gives the first bytes of the file in hexadecimal,
`VERSION.txt holds "5.0.x" (bytes in hexadecimal: 35 2e 30 2e 78 0a), where it
should hold the version of GAOL...`, which show what the quotation may hide:
CMake and configure quote a second byte order mark, or a zero-width space, as
it is, and it cannot be seen (meson writes it `\ufeff`, `\u200b`). A file of
UTF-16 characters, which Windows PowerShell 5 writes for a redirection
(`"5.0.0" > VERSION.txt`), is not decoded but refused, whether it starts with
its byte order mark (FF FE, FE FF) or not (it then holds NUL bytes, which a
text file does not hold): the message gives its bytes
(`ff fe 35 00 2e 00`...) and ends with
`the file is UTF-16: save it as UTF-8 or ASCII`, or, without the mark,
`the file holds a NUL byte as UTF-16 does: save it as UTF-8 or ASCII`. The
test `version_file` (`ctest -R version_file`) checks the reading of CMake,
and `.github/scripts/version-file.sh configure|meson|autoconf`, which the
continuous integration runs on a copy of the sources, those of configure,
meson and autoconf.

The editions of the manuals (`GAOL_V5_EDITION`, `GAOL_EDITION`) are their own,
set in `configure.ac` and `manual/meson.build`.

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
(`add_subdirectory`, FetchContent), GAOL and CORE-MATH are compiled as in
Release all the same: `-O3` (`/O2` with Visual C++), `NDEBUG` and the
optimizations of configure (GAOL v5: GAOL had `-O3` alone, and CORE-MATH no
optimization). With a generator of several configurations (Visual Studio,
Xcode, Ninja Multi-Config), GAOL built alone puts Release first among them,
which Ninja Multi-Config builds by default, unless `CMAKE_CONFIGURATION_TYPES`
is given; `cmake --build` with Visual Studio builds Debug all the same without
`--config` (GAOL v5). `GAOL_DEBUG` builds GAOL for debugging whatever the
build type or the configuration (below).

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
| `GAOL_DEBUG` | `OFF` | Build `libgaol` and CORE-MATH for debugging, without optimization nor `NDEBUG`, with debugging information and `GAOL_DEBUGGING`, whatever the build type or the configuration, a project bringing GAOL in included, as `enable-debug` of meson and `--enable-debug` of configure. Each configuration keeps its other flags (the C runtime of Visual C++, sanitizers...); the tests and the examples, and the code of that project, keep their build type; there is no target `perf` (GAOL v5) |
| `GAOL_PRESERVE_ROUNDING` | `OFF` | Restore the rounding direction found after each operation, rather than leaving it upward (see [The rounding direction](using.md#the-rounding-direction)) |
| `GAOL_PREFER_AVX512` | `OFF`, `ON` with `GAOL_PRESERVE_ROUNDING` | Have +, -, *, / and sqrt take the AVX-512 instructions and the rounding direction they carry in themselves, on a processor that has them, which sets neither the rounding direction nor the flush-to-zero modes (GAOL v5, see [The rounding direction](using.md#the-avx-512-path)) |

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
| `gaol-<version>.tar.gz` (`package_source`) | The sources, in `gaol-<version>/`, with `configure` and the `Makefile.in`: the three builds build them. CPack takes the source tree as it is, less what git ignores, the builds made within it, and the notes of the work on GAOL v5 (`TODO.md`, `process.md`, `todo-notes/`), so that made from a clean checkout, the archive holds the other files of the commit, `configure` as it was generated among them: if that was for another version than `VERSION.txt` holds, `configure --version` in the archive gives the old one, and CMake warns of it when it configures (see [The version of GAOL](#the-version-of-gaol)). |
| `gaol-<version>-<system>.tar.gz` (`package`) | What `cmake --install` installs, in `gaol-<version>-<system>/`: the headers, `libgaol.a` (or `libgaol.so*` with `BUILD_SHARED_LIBS`), the CMake package of GAOL and `gaol.pc`, to extract anywhere, `gaol.pc` finding its directories from where it is. |
| `libgaol-dev_<version>_<arch>.deb` (`package`, on Linux) | The same files in `/usr`, for `apt install ./libgaol-dev_<version>_<arch>.deb`: by default a static library and its headers, hence the name and the section `libdevel` of the development packages of Debian. |

The archive takes its name from the version CMake read when it configured, and
CMake warns of a `configure` generated for another version at that moment.
After a change of `VERSION.txt`, `package_source` configures the build
directory again first, whatever the generator: Ninja does before it runs CPack,
and CPack does when `VERSION.txt` holds another version than the build
directory was configured for (`cmake/gaol_package_source.cmake`, GAOL v5),
where the Makefile generators ran CPack at once.

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

`make clean` erases what `make` built, and keeps what configure made, so that
`make` builds again without configure: the `Makefile`, `config.status`,
`config.log`, `libtool`, `gaol.pc`, `gaol/gaol_configuration.h`, the
`manual/v*/gaol_version.tex`, the `.deps` directories of the dependencies and
the `.dirstamp` of the directories of CORE-MATH. `make distclean` erases them
too, the `.deps` directories included, which it left empty (GAOL v5), and
gives the source tree back as git has it, configure to be run again; the
continuous integration checks it (`build-systems.yml`).

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
| `--enable-debug` | `no` | The Debug build of CMake: `-g`, no optimization and no `NDEBUG` (`--enable-optimize` is then ignored), and GAOL's assertions (`GAOL_DEBUGGING`); brought into another project with `AC_CONFIG_SUBDIRS`, the `--enable-debug` of its configure, which passes on its options (and which that configure may read too), for the tests of GAOL as well, where `GAOL_DEBUG` of CMake and `enable-debug` of meson build the libraries alone so |
| `--enable-simd` | `yes` | The SSE2 intervals on x86 processors, as `GAOL_SIMD` |
| `--enable-fma` | `yes` | The fused multiply-add instructions of the processor, as `GAOL_FMA` |
| `--enable-asm` | `yes` | GAOL's assembly code, as `GAOL_ASM` |
| `--enable-verbose-mode` | `no` | The line on the standard error, as `GAOL_VERBOSE_MODE` |
| `--enable-preserve-rounding` | `no` | Restore the rounding direction after each operation, as `GAOL_PRESERVE_ROUNDING` |
| `--enable-prefer-avx512` | `no`, `yes` with `--enable-preserve-rounding` | Have +, -, *, / and sqrt take the AVX-512 instructions and the rounding direction they carry in themselves, on a processor that has them, as `GAOL_PREFER_AVX512` (GAOL v5) |
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
| `buildtype` | `release` | `-O3`, `NDEBUG` and `-funroll-loops -fomit-frame-pointer -fexpensive-optimizations`, as the Release build of CMake and configure; `debug` builds GAOL without optimization, with debugging information, and GAOL checks its assertions (`GAOL_DEBUGGING`), as the Debug build of CMake and `configure --enable-debug`. The option `enable-optimize` is gone. Brought in as a subproject, GAOL is built in `release` too where the project leaves the build type to its default (GAOL v5, see [Using GAOL](using.md#from-pkg-config)) |
| `enable-simd` | `true` | The SSE2 intervals on x86 processors, as `GAOL_SIMD` |
| `enable-fma` | `true` | The fused multiply-add instructions of the processor, as `GAOL_FMA` |
| `enable-asm` | `true` | GAOL's assembly code, as `GAOL_ASM` |
| `enable-debug` | `false` | Build `libgaol` and CORE-MATH for debugging, without optimization nor `NDEBUG` and with `GAOL_DEBUGGING`, whatever the build type, a subproject included (`-Dgaol:enable-debug=true`), as `GAOL_DEBUG` of CMake and `--enable-debug` of configure; the tests and the examples keep the build type, and there is no target `perf` (GAOL v5: it defined `GAOL_DEBUGGING` alone, and was gone) |
| `enable-verbose-mode` | `false` | The line on the standard error, as `GAOL_VERBOSE_MODE` |
| `enable-preserve-rounding` | `false` | Restore the rounding direction after each operation, as `GAOL_PRESERVE_ROUNDING` |
| `enable-prefer-avx512` | `false`, `true` with `enable-preserve-rounding` | Have +, -, *, / and sqrt take the AVX-512 instructions and the rounding direction they carry in themselves, on a processor that has them, as `GAOL_PREFER_AVX512`; meson cannot refuse the path under `enable-preserve-rounding`, which `-DGAOL_PREFER_AVX512=OFF` of CMake and `--disable-prefer-avx512` of configure do (GAOL v5) |
| `enable-exception` | `true` | Raise exceptions to signal errors, rather than abort |
| `with-tests` | `false` | Build the unit tests of `tests/`, which `meson test` and `ninja check` run, as `WITH_TESTS` (it was `with-test`) |
| `with-examples` | `false` | Build the examples of `examples/`, which `ninja check` runs, as `WITH_EXAMPLES` |
| `with-doc` | `false` | The target `pdf`, which builds the manuals, `manual/v5/gaol.pdf` for GAOL v5 and `manual/v4/gaol.pdf` for GAOL 4, with pdflatex, bibtex and makeindex (`make -C manual pdf` with autotools) |

meson builds `libgaol.a` and the shared library `libgaol.so.5.0.0`, whose
soname is `libgaol.so.5`, as configure and CMake name them; with Visual C++,
the static library alone, as CMake does, and `gaol_dep` and `gaol.pc` no
longer give the code using it `__GAOL_PUBLIC__` defined empty (GAOL v5, see
[Using GAOL](using.md#from-cmake)). The summary of `meson setup` gives the
address for the bug reports, jordan.ninin@ensta.fr, and the page of GAOL v5,
https://github.com/Jordan08/GAOL.
The option `check-perf` and the options `enable-relations` and `with-test`,
gone, are refused.

`meson setup` runs Python once, in `project()`, to read `VERSION.txt` (see
[The version of GAOL](#the-version-of-gaol)), and twice more for the message
of a file it refuses: the first of `python3` and `python` that it finds in
`PATH` or, when it finds neither, the Python that runs meson (the case of the
`meson.exe` of the Windows installer when no Python is installed).
`meson.build` names `python3` first because meson falls back on its own
Python for that name alone; the continuous integration checks it on Linux
with a `meson setup` whose `PATH` holds no Python.

On Windows, none of what follows was checked, but read in the sources of
meson. The directory `%USERPROFILE%\AppData\Local\Microsoft\WindowsApps` can
hold aliases of Python, `python3.exe` and perhaps `python.exe` too, which only
open the Microsoft Store when Python was not installed from it. meson 0.53.1
and later (the 0.53.2 of Ubuntu 20.04 and the meson of pip among them) leave
that directory out of their search for programs, as long as `PATH` names it
by that path (not for a profile whose directory differs from `USERPROFILE`).
meson 0.53.0 and earlier may take such an alias, and `meson setup` then stops
on the failure of the command that reads `VERSION.txt`: use a later meson
(`pip install meson`), or turn off the aliases of Python in the Windows
settings ("Manage app execution aliases").

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
  later), and runs the compile tests `refused_*` and `nodiscard_*` too. With
  meson before 0.57, `meson test` runs the examples too, and
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
| `GAOL_MAJOR_VERSION`, `GAOL_MINOR_VERSION`, `GAOL_MICRO_VERSION`, `GAOL_VERSION` | Always, from `VERSION.txt` (see [above](#the-version-of-gaol)): `5`, `0`, `0` and `"5.0.0"` for GAOL 5.0.0 | `project()` | `VERSION.txt`, read when configure runs | `project()` |
| `GAOL_DEBUGGING` | In a Debug build: GAOL checks its assertions (`GAOL_ASSERT`), and `GAOL_DEBUG_VERBOSE(level, command)` runs its commands | `CMAKE_BUILD_TYPE=Debug`, or `GAOL_DEBUG` | `--enable-debug` | `--buildtype=debug`, or `enable-debug` |
| `GAOL_EXCEPTIONS_ENABLED` | GAOL raises exceptions rather than abort | always | `--enable-exceptions` (default) | `enable-exception` (default) |
| `GAOL_PRESERVE_ROUNDING` | The operations restore the rounding direction they found | `GAOL_PRESERVE_ROUNDING` | `--enable-preserve-rounding` | `enable-preserve-rounding` |
| `GAOL_PREFER_AVX512` | +, -, *, / and sqrt take the AVX-512 instructions on a processor that has them | `GAOL_PREFER_AVX512` (with `GAOL_PRESERVE_ROUNDING`) | `--enable-prefer-avx512` (with `--enable-preserve-rounding`) | `enable-prefer-avx512` (with `enable-preserve-rounding`) |
| `GAOL_USING_ASM` | GAOL's assembly (32-bit x86 Linux and macOS); never with Visual C++ | `GAOL_ASM` (default) | `--enable-asm` (default) | `enable-asm` (default) |
| `GAOL_VERBOSE_MODE` | A line on the standard error when GAOL initializes and cleans up | `GAOL_VERBOSE_MODE` | `--enable-verbose-mode` | `enable-verbose-mode` |
| `GAOL_USING_SSE2_INSTRUCTIONS` | The intervals computed with SSE2 (x86, not 32-bit Windows nor Visual C++) | `GAOL_SIMD` (default) | `--enable-simd` (default) | `enable-simd` (default) |
| `GAOL_USING_SSE3_INSTRUCTIONS` | And compiled with `-msse3` | where the compiler takes it | where the compiler takes it | where the compiler takes it |
| `GAOL_HAVE_ROUNDING_MATH_OPTION` | The compiler takes `-frounding-math` | checked | checked | checked |
| `GAOL_HAVE_VISIBILITY_OPTIONS` | The library is compiled with `-fvisibility=hidden` | checked | checked | checked |
| `GAOL_HAVE_FENV_H` | The compiler has `<fenv.h>` | checked (required) | checked | checked |
| `GAOL_HAVE_GETRUSAGE` | `<sys/resource.h>` declares `getrusage()`, which `gaol/gaol_profile.cpp` measures the time with (`clock()` otherwise) | checked | checked | checked |

What GAOL needs to know of the processor and the system comes from the
compiler, in `gaol/gaol_config.h`: `GAOL_IX86_LINUX`, `GAOL_AARCH64_LINUX`,
`GAOL_IX86_MACOSX` and `GAOL_ARM_MACOSX`, the sizes of the integer types
(`GAOL_SIZEOF_INT`, `GAOL_SIZEOF_LONG_LONG_INT`, from `<limits.h>`) and the
order of the bytes (`GAOL_WORDS_BIGENDIAN`). These macros, and the
`GAOL_USING_SSE*` and `GAOL_HAVE_*` of the table, were named without `GAOL_`
before GAOL v5.

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
