<!-- Copyright (c) 2026 ENSTA, France
     Created 2026-09-20 by Jordan NININ -->
# Continuous integration

Part of the documentation of [GAOL v5](../README.md#documentation).

The workflows of `.github/workflows/` build GAOL with CMake and run the tests,
with `make test` (`cmake --build <build> --target test`, `RUN_TESTS` with
Visual Studio). They run for each pull request, and for each push to the
branches `master`, `MATH-CORE` and `configure-clean` only: a push to another
branch runs nothing until it is proposed, which halves the jobs of a pull
request (GAOL v5); `workflow_dispatch` runs them on any branch. `performance.yml`
is the exception: it runs for the pull requests that change what it measures
only (see [The speed of a pull request](#the-speed-of-a-pull-request)). The badges of
[README.md](../README.md) show their state on `configure-clean`. They build on:

- **Linux:** Ubuntu 24.04 and 26.04 on x86_64 and arm64, with GCC and Clang,
  also with the address and undefined behaviour sanitizers, with `-Wall
  -Wextra -Werror` (see below), with CMake 3.14,
  with the compilers of Ubuntu 22.04 (GCC 11 and Clang 14 on x86_64, GCC 11
  on arm64), whose images GitHub retires by April 2027, with GCC 9, which has
  no `__builtin_roundeven()` (see [3rd/README.md](../3rd/README.md)), and as a
  shared library (`BUILD_SHARED_LIBS`); the test `cpack_stale_configure` also
  runs with `CC="ccache gcc"` and a Ninja outside `PATH`, which it has to pass
  on to the copies of the tree it configures; and the tests run with the
  `FE_*` values of Cygwin (newlib, 0 to 3) put over those of glibc for the
  sources of CORE-MATH (`.github/scripts/newlib-fe-values.h`), Cygwin, which
  defines `__x86_64__` but not `__WIN32__`, as Linux (see
  [3rd/README.md](../3rd/README.md), item 6; and a job builds it, below).
- **Linux containers:**
  - Debian 12 and 13 on amd64, arm64 and armhf, and Debian 12 on i386;
  - manylinux_2_28 on x86_64 and aarch64;
  - Alpine (musl) on x86_64 and aarch64;
  - Debian 13 under qemu on s390x, ppc64le and riscv64, and Ubuntu 24.04
    (GCC 13) under qemu for POWER9 (`-mcpu=power9`);
  - built for size (`MinSizeRel`, `-Os`) on Debian 13 armhf and for POWER9,
    where GCC calls the intersection rather than inlining it, and
    `tests/rounding_direction.cpp` checks the choices a program makes on
    `is_empty()` of an empty interval there too.
- **macOS:** 14, 15 and 26, on arm64 and x86_64 (natively or under Rosetta), and
  with the sanitizers with AppleClang, LLVM's Clang and GCC.
- **Windows:**
  - Visual Studio 2022 and 2026, on x86, x64 and arm64, Release and Debug (on
    arm64, the compiler of Visual Studio 2022, MSVC 14.44, installed with
    Visual Studio 2026: GitHub has no image with Visual Studio 2022 on arm64
    any more). On x64 the intervals are the SSE2 ones (GAOL v5), on x86 and
    arm64 those of the FPU, which a step of each job checks. Two x64 jobs ask
    for the AVX-512 path (`GAOL_PREFER_AVX512`), as the one with the rounding
    direction preserved does by default: they take it where the runner has the
    instructions, which the step tells, with a warning where it has not. The
    runners have none, so that five more jobs run the unit tests that compute
    with intervals under Intel SDE, which emulates a processor with them
    (Visual Studio 2022 and 2026, Release and Debug, and clang-cl, with
    `GAOL_PRESERVE_ROUNDING` or `GAOL_PREFER_AVX512`);
  - clang-cl, the Clang of Visual Studio (toolset `ClangCL`): of Visual
    Studio 2022 on x64, Release and Debug, and of Visual Studio 2026 on x86,
    x64 and arm64, Release. On x64 it defines `__x86_64__` and `_WIN32` but
    not `__WIN32__`, which upstream CORE-MATH tests to read the rounding
    direction in `cbrt`, `rsqrt` and `asinpi` (see
    [3rd/README.md](../3rd/README.md), item 6); on x86, GAOL gives it
    `-msse2`, without which it computes doubles on the x87 unit;
  - MinGW-w64 15 on x86 and x64, Release and Debug (SSE2 intervals on both,
    `-faligned-new=8` on x86, which a step checks; GAOL v5), 14 on x86 and x64,
    12 and 13 on x86; on x86, Release, GCC 15.2 with `GAOL_PRESERVE_ROUNDING`
    (with `-mfma`, GAOL v5) and GCC 13.2 with `GAOL_FMA=OFF` (the `fma()` of
    mingw-w64 11 on the x87 unit);
  - MSYS2 UCRT64 (GCC, and GCC again with `GAOL_PRESERVE_ROUNDING`, with and
    without `GAOL_FMA`: with `-mfma`, `__FMA__` is defined, so that the rounding
    direction goes through MXCSR alone, which only Linux and clang-cl ran, and
    mingw-w64 13 and later read it with a `fegetround()` that joins the two
    units and set it with a `fesetround()` that clears the exception flags,
    which GAOL does not call there; GAOL v5) and CLANG64 (Clang, and Clang
    again with `GAOL_PRESERVE_ROUNDING`, which takes `-mfma` and the same path);
  - Cygwin, GCC for x86-64, Release (GAOL v5): `-mfma -mprefer-vector-width=128`
    in the flags, the objects read as below, all the tests passing but those
    that are skipped (`intervalf`, `interval2f`, and `cpack_stale_configure`,
    the CPack of Cygwin not making an archive), and a configuration with
    `GAOL_FMA=OFF` that `gaol/gaol_config.h` has to refuse;
  - With GCC on x64 and on x86, in Release, MinGW-w64 15, 14, 13 (x86) and 12
    (x86), MSYS2 UCRT64 and Cygwin, a step reads the objects with `objdump`
    (`.github/scripts/check-gcc-windows-fma.sh`, GAOL v5):
    `gaol_interval_core.cpp` and the `cos` of CORE-MATH have to hold fused
    multiply-add instructions (`GAOL_FMA` gives these compilers `-mfma
    -mprefer-vector-width=128`), and no object, the tests' included, an
    instruction on a `%ymm` or `%zmm` register, which GCC would spill to a stack
    Windows x64 aligns on 16 bytes (GCC bug 54412); on x86 such lines are
    listed and noticed, not an error.

They also build GAOL with autotools and meson on Ubuntu (x86_64, arm64),
Debian (i386, armhf), macOS (arm64, x86_64) and MSYS2, and with meson and
Visual Studio 2022 and 2026 (x64, x86), and with meson 0.53.2, the oldest the
meson build takes (Ubuntu x86_64, GAOL v5), run the tests (`make test`,
`ninja test`), and build them again with the
GAOL they install, which is the only library installed, CORE-MATH being
compiled into it; build GAOL as a part of another project, brought in by
FetchContent (`tests/fetch_content`), and the tests with the GAOL that project
installs, and check that GAOL and CORE-MATH are compiled as in Release where
that project gives no build type (`tests/release_flags.py`, GAOL v5); build
GAOL as a subproject of a meson project (`tests/meson_subproject`, Ubuntu
x86_64, with the meson of pipx and the meson 1.3 of Ubuntu), whose code
compiled with `gaol_dep` has to get the flags of `gaol.pc`, and GAOL to be
built in release in the default build type of meson (GAOL v5); check that
`GAOL_DEBUG` and `enable-debug` build GAOL for debugging, alone and brought
into another project, and that GAOL built alone with Ninja Multi-Config builds
Release by default (`linux.yml`, `build-systems.yml`, configured only, GAOL
v5); check that the three builds agree on each of these machines; check that
configure and meson, and autoconf when it generates configure (Linux), read a
`VERSION.txt` that starts with a byte order mark or has the line ends of
Windows (`.github/scripts/version-file.sh`), and that meson reads it with
no Python on `PATH` (Linux); check that `make distclean` gives the source tree
back as git has it, after the autotools have built, installed and tested GAOL
in it (Ubuntu, macOS); check that the builds
refuse Clang on 32-bit ARM and Clang 14 on 64-bit ARM, that
`gaol/gaol_config.h` refuses MinGW-w64 GCC 11 to 13 and MSYS2 MINGW64 (GCC
and Clang) on x64 and GCC 11 on x86,
`-ffinite-math-only` and `-ffast-math` with GCC and Clang (the tests
`refused_*` of the three builds, see [Tests](tests.md)), and Visual C++ and
clang-cl without `/fp:strict`. Jobs of each build
restore the rounding direction (`GAOL_PRESERVE_ROUNDING`): Ubuntu x86_64 GCC
and Clang (which take the AVX-512 path where the runner has it, Clang having
computed a difference of that path once flush-to-zero was set back, GAOL v5)
and arm64 GCC and Clang (GCC too since the NEON intervals, GAOL v5: it does not
model the direction they write into FPCR), Debian i386 and armhf, macOS arm64,
Visual Studio x86 and x64, autotools and meson.
Jobs of autotools and of meson on Ubuntu x86_64 build GAOL
without exceptions (`--disable-exceptions`, `-Denable-exception=false`),
where an error of GAOL ends the program, and run the examples and the tests,
which leave out there the checks of an exception (GAOL v5). The jobs built in
Release print the time per operation in their summary, a table that no other run
compares: `performance.yml` compares the base of a pull request with its head
(see [The speed of a pull request](#the-speed-of-a-pull-request)).

On 64-bit ARM processors the intervals are the NEON ones by default
(`GAOL_ARM64`, `--enable-arm64`, `-Denable-arm64`; GAOL v5), so that the jobs of
Ubuntu arm64, macOS arm64 and the aarch64 containers, and those of autotools
and meson on arm64, test them. A few jobs build the FPU intervals instead,
which the NEON ones replace on these processors, so that their code stays
covered, the preserved rounding direction included: the GCC and the Clang of
Ubuntu 24.04 arm64 (`-DGAOL_ARM64=OFF`), and both with the rounding direction
preserved too, macOS 15 arm64 (`-DGAOL_ARM64=OFF`), and autotools
(`--disable-arm64`) and meson (`-Denable-arm64=false`) on Ubuntu 24.04 arm64.
The GCC and the Clang of Ubuntu 24.04 x86-64 build them too
(`-DGAOL_SIMD=OFF`), GCC without and with the direction preserved and Clang
with it: the compound products and quotients of the FPU intervals hold the
direction only by the order of their stores, which the checks of
`tests/rounding_direction.cpp` run with the direction preserved make a rule
(GAOL v5). A build that fell back to
the FPU intervals without saying so would keep every test green, so the jobs
check which intervals the configuration took, in a step "The intervals"
(`.github/scripts/check-intervals.sh`, which reads
`GAOL_USING_NEON_INSTRUCTIONS` and `GAOL_USING_SSE2_INSTRUCTIONS` in
`gaol_configuration.h`, and fails if they are not the expected ones): the main
jobs of `linux.yml`, those of `macos.yml`, autotools and meson expect the NEON
intervals on 64-bit ARM, the SSE2 intervals on x86-64, and the FPU intervals
where the option (`GAOL_ARM64` or `GAOL_SIMD`) is off; the Visual Studio jobs
of `windows.yml` expect the SSE2 intervals on x64 (GAOL v5), the NEON intervals
on arm64 with Visual C++, and the FPU intervals on x86 and with clang-cl on
arm64, where NEON is not offered to it; the
Linux containers make the check before the build, from `uname -m`: the NEON
intervals on aarch64, the SSE2 intervals on x86_64, and the FPU intervals on
armhf, s390x, ppc64le and riscv64.

Four jobs of `linux.yml`, with the GCC and the Clang of Ubuntu 26.04 on x86_64
and on arm64, and one of `macos.yml`, with the AppleClang of macOS 26 arm64,
build the library, the sources of CORE-MATH included, the tests and the
examples with `-Wall -Wextra -Werror`, and two of `windows.yml` with `/W4 /WX`,
with the Visual C++ of Visual Studio 2026 x64 and x86. Those of arm64 compile
the header and the sources of the NEON intervals, with the warnings of the
headers (`-Wold-style-cast`, `-Wundef`) that the intrinsics of `<arm_neon.h>`
have to stay clear of (GAOL v5). The first five check the headers
GAOL installs with `.github/scripts/headers.sh` (GAOL v5): each compiles alone
with `-std=c++11`, the oldest standard GAOL takes, and
`-Wall -Wextra -Wold-style-cast -Wundef -Werror` (they have no cast of C,
nor any `#if` on a macro that may be undefined), but the
four that other headers include in the middle of their code or for Visual C++
only, and the macros they define start with `GAOL_` or `gaol_`.
`-Werror` and `/WX` go to the targets only (`CMAKE_COMPILE_WARNING_AS_ERROR`),
so that no check of CMake fails on a warning of its test program. Until then,
the library was compiled with `-Wall -Wconversion`, and no job failed on a
warning.

`tests/numbers.cpp` reads numbers and writes exact texts under a locale
writing a decimal comma where the system has one (see [Tests](tests.md)), and
says so and passes where it has none. The jobs on Ubuntu and in the Debian
containers generate `fr_FR.UTF-8` before the tests, with `localedef`
(`.github/scripts/comma-locale.sh`): their systems have no such locale; macOS
has `fr_FR.UTF-8`, and Windows `French_France.1252`. After the tests, each job
that runs them fails if the output of `numbers` holds no check under such a
locale: the output ctest keeps in `Testing/Temporary/LastTest.log`,
`tests/numbers.log` with the autotools, `meson-logs/testlog.txt` with meson
(GAOL v5: only the CMake jobs of `linux.yml` checked it). Alpine and
manylinux have no locale writing a decimal comma to generate, and check
nothing of the kind. `numbers` has 300 s (`GAOL_NUMBERS_TIMEOUT`), 600 in the
job of macOS 15 x86_64 GCC with the sanitizers, where it took 146 to 367 s on
the same code, and once more than 400.

The manuals, that of GAOL v5 (`manual/v5/gaol.tex`) and that of GAOL 4
(`manual/v4/gaol.tex`), are built with the LaTeX of Ubuntu 24.04, by the
autotools and the meson builds, when `manual/` changes (`manual.yml`); the
PDFs are artifacts of the run. The examples of the manual of GAOL v5 that show
what they print are compiled in C++11 with the GAOL that the job Ubuntu 24.04
x86_64 GCC of `linux.yml` installs, at each run, so that a change of the
library that changes one of their outputs shows, and have to print what the
manual shows (`manual/check_examples.py`, GAOL v5).

## The speed of a pull request

The times of the continuous integration cannot be compared from one run to the
next (GAOL v5). The runners are virtual machines shared with other jobs, whose
speed changes by several times between two runs of the same code: `x * y` with
Visual C++ x64 took 7.9 ns and then 16.3, and `sqrt` with MSYS2 and GCC 16
6.8 ns and then 170. The step "Performance" of the jobs of `linux.yml`,
`macos.yml`, `windows.yml`, `containers.yml` and `build-systems.yml` prints the
table of `gaol_performance` in their summary (and never fails a job): it tells
what a platform costs at that moment, and nothing else. Showing the best of
several passes would not help: a pull request is to be compared with its base,
which is not measured at the same moment.

`performance.yml` measures two builds in turn, in the same job, so that what the
runner does between two moments reaches both. It runs for a pull request that
changes what is measured, not for one that changes the documentation: `gaol/`,
`3rd/` (CORE-MATH), `tests/performance.cpp`, the files of the benchmark
`doc/compare/code/bench*` and `doc/compare/code/run_perf.sh`, the files
`CMakeLists.txt` and `cmake/`, and the scripts and the workflow themselves. A
job:

1. checks out the merge commit of the pull request and its first parent, which
   is the base it is merged into: the difference between the two is the pull
   request alone, whatever the base branch has gained since the pull request
   was started;
2. configures both with CMake in Release and builds `gaol_performance` alone,
   with the options of the job; the programs of the base are kept from one run
   to the next (`actions/cache`, keyed by the job, the image of the runner and
   the commit of the base, `--before-cache` of `performance-ab.sh`), so that
   the next push of the same pull request builds only the pull request (GAOL
   v5). What is kept is the programs, not the times, which are measured in
   turn in every run; the image of the runner and the model of the processor
   are in the key so that the two builds come from the same tools and the same
   choices (the AVX-512 path and the fused multiply-add instructions are taken
   where the machine building runs them, and a program built on a runner with
   AVX-512 stops on one that has none); a key has no comma, which the names of
   the jobs have, so they are written with underscores in it;
3. runs `gaol_performance` three times with each build, the base first in the
   first and third runs and the pull request first in the second
   (`.github/scripts/performance-ab.sh`);
4. writes in the summary of the job the table of the median of each row for the
   base and for the pull request, their ratio (base over pull request: above 1,
   the pull request is faster) and the spread of the runs, the largest ratio of
   the slowest run to the fastest of either build
   (`.github/scripts/compare-performance.py`).

The jobs are Ubuntu 24.04 x86_64 with GCC, with Clang, with GCC and the
rounding direction preserved, and with GCC and with Clang and the FPU
intervals; Ubuntu 24.04 arm64 with GCC and with Clang (the
NEON intervals) and with GCC and the FPU intervals (the kernels of #157); macOS
26 arm64 (the NEON intervals) and macOS 15 x86_64; Visual Studio 2026 x64, x86,
arm64, and x64 with the rounding direction preserved, and with clang-cl for
x64, x86 and arm64; MSYS2 UCRT64 (GCC) and CLANG64 (Clang); and Debian 13
containers for i386 and armhf, `gaol_performance` alone and no cache there (the
image of Debian changes under the job). The other platforms (POWER, s390x,
RISC-V, MinGW-w64 of Chocolatey, the 32-bit Windows of MSYS2) are not compared,
nor are the autotools and meson builds: the three builds give the same flags,
which `.github/audit` checks.

The table never fails a job because of a time. Only a difference of several
times, on rows that share their code, means something on a shared runner, and a
ratio within the spread of the runs is noise: under the table, the rows whose
ratio is 2 or more, or 0.5 or less, and beyond the spread of the runs (for a
spread of 3, a ratio of 0.5 is not listed, one of 0.25 is) are listed, or none
is; a row of two times of 0 has no ratio, and a run of 0 makes the spread
infinite. A job fails when a build or a program does, and when the tables cannot
be compared: an output with no row, or no row common to the base and the pull
request (the format of the output of `gaol_performance` changed), which
`compare-performance.py` says and fails on. The comparison has its test, which
needs Python alone and runs first in each job, before the builds
(`.github/scripts/test-performance-tools.py`): tables made for the purpose,
with rows on one side only, a `|` in a name, one run, times of 0, empty outputs
and the spread, and `processor_name()` of `doc/compare/code/run_perf.sh` with
other `/proc/cpuinfo` (not on Windows). It fails if the median is replaced by
the mean or the threshold is inverted.

On Linux, macOS and with Visual Studio the job also runs the ten long lines of
`doc/compare/code/run_lines.sh` with `gaol_bench`, the program of the benchmark
(the CMake target `gaol_bench`, which `make perf` builds too): 200 000 intervals
drawn by `bench.py` (Python 3 with numpy, which the script installs with pip
where it is missing), 3 repeats of each line, 3 runs of each build, and a second
table under the first. Both builds are tried: the lines are left out, with a
warning of the job, where one of them has no `gaol_bench` that builds (on macOS
and Windows, a base older than the target: the head is built all the same, so
that Visual C++ compiles `gaol_bench` from the first pull request) or numpy
cannot be installed, and that table is a notice, not an error, where it cannot
be made. `make perf` runs the same ten lines (see
[doc/compare/code/README.md](compare/code/README.md)).

The job `control-register-performance` of `containers.yml` (Debian 13 armhf and
arm64) uses the same scripts to compare two builds of one tree that differ by a
CMake option: the control register of the rounding direction written directly
with `GAOL_PRESERVE_ROUNDING`, or the functions of the C library, and only the
rows that the register can change.

The same comparison runs anywhere with CMake, a C++ compiler and, for the long
lines, Python 3 with numpy:

```sh
sh .github/scripts/performance-ab.sh --before-src ../base --after-src . --lines
python3 .github/scripts/compare-performance.py base perf-before-1.md perf-before-2.md perf-before-3.md \
  -- PR perf-after-1.md perf-after-2.md perf-after-3.md
python3 .github/scripts/compare-performance.py base perf-lines-before-1.csv perf-lines-before-2.csv perf-lines-before-3.csv \
  -- PR perf-lines-after-1.csv perf-lines-after-2.csv perf-lines-after-3.csv
```

(`--options` and `--before-options` give CMake options to both builds or to the
base alone; the script describes them all.)

## Configurations refused or left out, and why

The configurations GAOL v5 refuses, and those the continuous integration
leaves out. The builds refuse the compilers among them when configuring, or
`gaol/gaol_config.h` when compiling (see
[Compilers and options refused](three-builds.md#compilers-and-options-refused)),
and the last column says which job checks the refusal. What a configuration
gives with GAOL v5 was measured on 21 September 2026, building it past its
refusal.

| Configuration | Why | What GAOL v5 does |
|---|---|---|
| Clang for 32-bit ARM processors (Debian armhf) | **Wrong bounds.** Clang does not honour the rounding direction there. Built by Clang 19 (Debian 13) and Clang 21 (Debian sid), GAOL v5 fails `arithmetic`, `elementary`, `reverse`, `other_functions` and `rounding_direction`: bounds of `atan2()`, `sin()`, `cos()`, `pow()`, `nth_root()` and `div_rel()` do not enclose the exact values, and the products are not the tightest ones in any rounding direction. | Refused by the three builds and by `gaol/gaol_config.h`; a job checks that CMake, configure and meson refuse it, for the C compiler alone as for the C++ compiler alone (`containers.yml`). GCC builds the armhf jobs. |
| Clang 14 on 64-bit ARM (Ubuntu 22.04 arm64, and Clang 14 on Ubuntu 24.04 arm64), and any compiler saying of `-frounding-math` "overriding currently unsupported rounding mode on this target" | **Wrong bounds.** Clang 14 does not honour the rounding direction on 64-bit ARM, and says so. Built by it, GAOL v5 fails `arithmetic`, `elementary`, `reverse` and `other_functions` in Release, bounds of `nth_root()`, `pow()`, `atan2()`, `cos()`, `div_rel()` and `mid()` not enclosing the exact values, and `elementary` in Debug, a bound of `atan2()`. Clang 18 honours the rounding direction there. | Refused by the three builds; a job checks it for the C compiler alone, the C++ compiler alone and both (`linux.yml`). Ubuntu 22.04 arm64 is built with GCC only, Ubuntu 24.04 and 26.04 arm64 with Clang too. |
| MinGW-w64 whose `fma()` or `round()` is wrong: on x64, GCC 11 to 13 of Chocolatey (mingw-w64 before 12) and the mingw-w64 linked with `msvcrt.dll` rather than the UCRT (MSYS2 MINGW64, the cross compilers of Debian and Ubuntu); on 32-bit x86, GCC 11 of WinLibs (mingw-w64 9); on ARM, as on x64, mingw-w64 before 12 or linked with `msvcrt.dll` (not tested) | **Wrong bounds.** On x64, the `fma()` and `round()` are those of mingw-w64's own math library, computed in doubles. That `fma()` adds the products of the halves of its arguments with four roundings: it is not correctly rounded (10.7 % of the error-free products `fma(a, b, -a*b)` and 25 to 73 % of other triples wrong, depending on the rounding direction), and CORE-MATH computes with it wherever the compiler has no `-mfma` (`GAOL_FMA` off; GCC for Windows on x86-64 and on 32-bit x86 takes `-mfma` by default, and these toolchains stay refused, their `round()` being wrong too). That `round()` depends on the rounding direction: `round(0x1.fffffffffffffp-2)` is 1 except rounding upward. On 32-bit x86 the same `fma()` computes on the x87 unit, and the one of the mingw-w64 9 of WinLibs, compiled without optimization, rounds each of its partial sums to a double. GAOL v5 fails `elementary`, `reverse` and `core_math` there: bounds of `tan()`, `asin()`, `atan()`, `sinh()`, `cosh()`, `atanh()` and `atan2()` of small arguments do not enclose the exact values (the continuous integration, and mingw-w64 11 and the objects of MSYS2 MINGW64 under wine). From mingw-w64 12 on, a toolchain linking the UCRT takes the `fma()` and `round()` of `ucrtbase.dll`. | Refused by `gaol/gaol_config.h`; six jobs check the refusal, two of them MSYS2 MINGW64 with GCC and Clang (`windows.yml`). MinGW-w64 GCC 14 and 15 on x64, GCC 12 to 15 on 32-bit x86 (13 also with `GAOL_FMA=OFF`, the `fma()` of mingw-w64 11, which keeps its partial sums in extended precision and passes the tests, though not correctly rounded), and MSYS2 UCRT64 and CLANG64 are built and tested; `tests/core_math.cpp` checks `fma()` and `round()` first. |
| Visual C++, and clang-cl from Clang 16, without `/fp:strict` (`/fp:precise`, their default) | **Bounds not certified.** They then assume rounding to nearest, and may evaluate or rewrite floating-point operations accordingly, while GAOL computes with the rounding direction set upward. | Refused by `gaol/gaol_config.h`; `gaol::gaol` gives `/fp:strict` to the code linking it, and each Visual Studio job checks the refusal (`tests/fp_strict`), those of clang-cl included (GAOL v5). |
| The SSE2 intervals on 32-bit Windows, compiled without aligned `new` | **Crash.** GCC takes the memory of `new` to be aligned on 16 bytes, which the C runtime of 32-bit Windows aligns on 8, and a `std::vector` of SSE2 intervals crashes on its first `movaps`: with MinGW-w64 14.2 and 15.2 for x86, in Release and in Debug, `arithmetic`, `other_functions`, `reverse` and `rounding_direction` crash. With `-faligned-new=8` the tests pass, but the code using GAOL needs that flag too. Visual C++ and clang-cl have no such flag: `/Zc:alignedNew` does nothing in C++14 with `cl` (the warning C4316 stays, and `std::allocator` calls `operator new(size_t)`), and with clang-cl it makes the STL fail to compile (`align_val_t` is not an enumeration there); only `/std:c++17` turns the aligned `new` on (Compiler Explorer, MSVC 19.51 and clang-cl 22 for x86). | CMake, configure and meson take the SSE2 intervals on 32-bit Windows with GCC and Clang (MinGW-w64, MSYS2), with `-faligned-new=8` given to the library and to the code using it (`gaol::gaol`, `gaol.pc`, `gaol_dep`); `gaol/gaol_config.h` refuses them compiled without (the tests `refused_aligned_new*` check it); the FPU intervals are compiled with Visual C++ and clang-cl, and with a compiler that does not take the flag. On 64-bit Windows (MinGW-w64, MSYS2, Visual C++, clang-cl) the SSE2 intervals are compiled and pass the tests. |
| The NEON intervals with clang-cl, for a target of Windows with another compiler (MinGW-w64, MSYS2), and on 32-bit ARM | **Not offered.** The NEON intervals write the rounding direction into FPCR with `mrs` and `msr`, in the inline assembly of GCC and Clang, or through `_ReadStatusReg()` and `_WriteStatusReg()` with Visual C++ (`gaol/gaol_config.h`), and need the vectors of two doubles of 64-bit ARM, which the NEON of 32-bit ARM does not have. clang-cl, MinGW-w64 and MSYS2 for arm64 define `__aarch64__` (clang-cl without `__GNUC__`), and the jobs of clang-cl arm64 are the only ones of them in the continuous integration. | `GAOL_ARM64`, `--enable-arm64` and `-Denable-arm64` are ignored there, and the FPU computes the intervals, as before, in a universal build of macOS too; `gaol/gaol_config.h` stops the compilation of NEON intervals elsewhere (GAOL v5). The Visual Studio jobs check that clang-cl takes the FPU intervals, and Visual C++ for arm64 the NEON ones (`windows.yml`), and so do the armhf ones (`containers.yml`). |
| Cygwin without the fused multiply-add instructions (`GAOL_FMA=OFF`, or a processor without them) | **Bounds not certified.** The `fma()` of the C library of Cygwin is not correctly rounded: `fma(1 + 2^-30, 1 - 2^-30, -1)` gives 0 instead of -2^-60, and `std::fma` is a call of it without optimization. CORE-MATH computes with `fma()`. GAOL v5 has the instruction instead (`-mfma`, `__builtin_fma`), which every level of optimization keeps. | Refused by `gaol/gaol_config.h`; the Cygwin job of `windows.yml` checks the refusal, and builds and tests Cygwin with `-mfma` (GCC 14.4, Release, the tests passing but three skipped). |
| Doubles computed on the x87 unit of 32-bit x86 | **Wrong results rounded to nearest, and wrong bounds.** The x87 unit computes with 64-bit significands and 15-bit exponents, and CORE-MATH assumes every operation on doubles rounded to a double (`FLT_EVAL_METHOD` 0, see `3rd/math-core/README.md`). With GCC 12 on Debian 12 i386, CORE-MATH rounded to nearest gives the other neighbour of the exact value at 175 of the arguments compared with SSE2, its results being rounded twice: `exp(-0x1.74910d52d3051p+9)` is 0 rather than 2^-1074, `tanh(0x1.30fc1931f09c9p+4)` 1 rather than 1 − 2^-53. With GCC 9 on x86-64 (`-mfpmath=387`), which turns into doubles at compile time, rounded to nearest, the constants CORE-MATH rounds in the direction in effect (`0x1p-1074 * 0.5`, `-1.0 + 0x1p-54`), GAOL's bounds of `exp2(-1075)`, `exp10(-400)`, `expm1(-800)` and of `atan2()` of a tiny and a huge number do not enclose the exact values. | Refused by `gaol/gaol_config.h`; the builds compile with `-msse2 -mfpmath=sse` on 32-bit x86. `tests/extended_precision.cpp` checks these arguments, CORE-MATH in the four rounding directions and GAOL's bounds, and fails on the x87 unit. |
| Debian 11 Bullseye (amd64, arm64, armhf) | **Cannot be installed.** Out of support since August 2026: deb.debian.org lists packages of its security repository that it no longer serves (`libc-dev-bin 2.31-13+deb11u14`...), so that `g++` cannot be installed in the container; without that repository the packages are broken, and archive.debian.org does not have it yet. GAOL v5 itself builds and passes its tests there, with GCC 10.2 and CMake 3.18.4 installed from snapshot.debian.org. | Left out of the continuous integration. Debian 12 and 13 are built. |
| The intervals of floats (`gaol::intervalf`, `gaol::interval2f`) | **Unfinished.** `sqrt(intervalf)` returns its argument and `interval2f::inverse()` aborts; neither IBEX nor Codac uses them. | Compiled by none of the three builds, which have no option for them: only a developer of GAOL defines `GAOL_FLOAT_INTERVALS` (`gaol/gaol_config.h`). The audit still compares it between the builds, none of which defines it. |

## What these platforms showed

What building and testing GAOL on these platforms showed, while the workflows
were written and since their first run on 13 September 2026, and what was
changed in GAOL for it; [What differs from GAOL](differences.md) gives each
change with its measures.

- **Bounds that were wrong on some platforms only.**
  - The hyperbolic functions, taken from the libm of the system: the libms
    of glibc 2.31, musl and MinGW-w64 are sometimes more than one double from
    the exact value. They are now those of CORE-MATH, correctly rounded, on
    every platform ([issue #1](https://github.com/Jordan08/GAOL/issues/1)).
  - The numbers read from text: the C runtime of Windows and musl on 64-bit
    ARM round `strtod()` to nearest in every direction, and
    `interval("0.1")` did not enclose 1/10 there. Numbers are now read
    exactly, and intervals written in decimal are rounded outward by GAOL
    rather than by the C library
    ([issue #3](https://github.com/Jordan08/GAOL/issues/3)).
  - `mid()` with Visual C++ (`/O2 /fp:strict`), which rewrote
    `(-.5)*x + y` into `y - .5*x`: `mid([2^-1074, MAX])` was wrong in the
    seven Release jobs of Visual Studio.
  - `mid()` on 64-bit ARM with autotools and meson, which did not give
    `-ffp-contract=off`: the compilers fused its multiplications and
    additions, and it returned a single double. The three builds now give
    the same flags, which the audit jobs check on each kind of machine.
  - Square roots with Visual C++ for 32-bit x86, whose `sqrt` rounds to
    nearest in every rounding direction.
- **Operations that were slow on some platforms only**, which the table of
  `gaol_performance` in the summary of each Release job showed.
  - Reading the rounding direction before each operation made `x + y` take
    800 ns under Rosetta 2 and 82 to 98 ns with 32-bit Visual C++: an
    addition shows whether it is upward instead.
  - `fesetround()` cost 130 ns with mingw-w64 13, and 50 to 250 ns with
    Visual C++: on x86 processors the control registers are written
    directly.
  - `-ffloat-store`, given to GCC on every target, 64-bit ARM included,
    made `x * y` take 18.3 ns rather than 4.9 on x86-64: it is given only
    where doubles are still computed on the x87 unit.
- **Crashes and builds that failed.**
  - configure and meson did not install the headers for MinGW and Visual
    C++, and meson did not compile on Linux (`GETRUSAGE_IN_HEADER`).
  - The agreement jobs passed while a build had not configured in the
    containers, a pipe hiding the failure: they now fail, and print the
    logs.
  - `configure --enable-debug` did not compile: with `GAOL_DEBUGGING`,
    `gaol/parser/gaol_expression.h` wrote on `std::cout` without `<iostream>`. The
    Debug jobs of CMake, which now define `GAOL_DEBUGGING` too, and the test
    `debugging` compile that code.
  - `minimum()` and `maximum()` of two zeros of different signs gave +0 and
    -0 with Visual C++ in Release, rather than -0 and +0, which the unit
    test of GAOL 4 `float_functions` checks, built with Visual C++ since the
    tests of `check/` are in `tests/`.
  - A shared `libgaol` (configure, meson) did not export the classes and
    functions of the expressions (`gaol/parser/gaol_expression.h`), and a program
    building one did not link with it. The shared library job of CMake and
    the meson jobs, whose tests link with the installed `libgaol.so`, build
    `tests/expressions.cpp`.
- **Reports of the sanitizers that were suppressed at first**, and are fixed
  now: the nodes the parser did not free (`lsan.supp`,
  [issue #4](https://github.com/Jordan08/GAOL/issues/4)). Nothing is
  suppressed any more.
