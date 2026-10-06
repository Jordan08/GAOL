<!-- Copyright (c) 2026 ENSTA, France
     Created 2026-09-20 by Jordan NININ -->
# Continuous integration

Part of the documentation of [GAOL v5](../README.md#documentation).

The workflows of `.github/workflows/` build GAOL with CMake and run the tests,
with `make test` (`cmake --build <build> --target test`, `RUN_TESTS` with
Visual Studio). They run for each pull request, and for each push to the
branches `master`, `MATH-CORE` and `configure-clean` only: a push to another
branch runs nothing until it is proposed, which halves the jobs of a pull
request (GAOL v5); `workflow_dispatch` runs them on any branch. The badges of
[README.md](../README.md) show their state on `configure-clean`. They build on:

- **Linux:** Ubuntu 24.04 and 26.04 on x86_64 and arm64, with GCC and Clang,
  also with the address and undefined behaviour sanitizers, with `-Wall
  -Wextra -Werror` (see below), with CMake 3.14,
  with the compilers of Ubuntu 22.04 (GCC 11 and Clang 14 on x86_64, GCC 11
  on arm64), whose images GitHub retires by April 2027, with GCC 9, which has
  no `__builtin_roundeven()` (see [3rd/README.md](../3rd/README.md)), and as a
  shared library (`BUILD_SHARED_LIBS`); the test `cpack_stale_configure` also
  runs with `CC="ccache gcc"` and a Ninja outside `PATH`, which it has to pass
  on to the copies of the tree it configures.
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
    any more);
  - clang-cl, the Clang of Visual Studio (toolset `ClangCL`): of Visual
    Studio 2022 on x64, Release and Debug, and of Visual Studio 2026 on x86,
    x64 and arm64, Release. On x64 it defines `__x86_64__` and `_WIN32` but
    not `__WIN32__`, which the sources of CORE-MATH test to read the rounding
    direction in `cbrt`, `rsqrt` and `asinpi` (see
    [3rd/README.md](../3rd/README.md), item 7); on x86, GAOL gives it
    `-msse2`, without which it computes doubles on the x87 unit;
  - MinGW-w64 15 on x86 and x64, Release and Debug, 14 on x86 and x64, 12 and
    13 on x86;
  - MSYS2 UCRT64 (GCC) and CLANG64 (Clang).

They also build GAOL with autotools and meson on Ubuntu (x86_64, arm64),
Debian (i386, armhf), macOS (arm64, x86_64) and MSYS2, and with meson and
Visual Studio 2022 and 2026 (x64, x86), run the tests (`make test`,
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
and arm64 Clang, Debian i386 and armhf, macOS arm64, Visual Studio x64,
autotools and meson. Jobs of autotools and of meson on Ubuntu x86_64 build GAOL
without exceptions (`--disable-exceptions`, `-Denable-exception=false`),
where an error of GAOL ends the program, and run the tests, which leave out
there the checks of an exception (GAOL v5). The jobs built in Release print the
time per operation in their summary.

Two jobs of `linux.yml`, with the GCC and the Clang of Ubuntu 26.04, and one of
`macos.yml`, with the AppleClang of macOS 26 arm64, build the library, the
sources of CORE-MATH included, the tests and the examples with
`-Wall -Wextra -Werror`, and two of `windows.yml` with `/W4 /WX`, with the
Visual C++ of Visual Studio 2026 x64 and x86. The first three check the headers
GAOL installs with `.github/scripts/headers.sh` (GAOL v5): each compiles alone
with `-std=c++11`, the oldest standard GAOL takes, and
`-Wall -Wextra -Wold-style-cast -Werror` (they have no cast of C), but the
three that other headers include in the middle of their code or for Visual C++
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
| Clang for 32-bit ARM processors (Debian armhf) | **Wrong bounds.** Clang does not honour the rounding direction there. Built by Clang 19 (Debian 13) and Clang 21 (Debian sid), GAOL v5 fails `arithmetic`, `elementary`, `reverse`, `other_functions` and `rounding_direction`: bounds of `atan2()`, `sin()`, `cos()`, `pow()`, `nth_root()` and `div_rel()` do not enclose the exact values, and the products are not the tightest ones in any rounding direction. | Refused by the three builds and by `gaol/gaol_config.h`; a job checks that CMake refuses it (`containers.yml`). GCC builds the armhf jobs. |
| Clang 14 on 64-bit ARM (Ubuntu 22.04 arm64, and Clang 14 on Ubuntu 24.04 arm64), and any compiler saying of `-frounding-math` "overriding currently unsupported rounding mode on this target" | **Wrong bounds.** Clang 14 does not honour the rounding direction on 64-bit ARM, and says so. Built by it, GAOL v5 fails `arithmetic`, `elementary`, `reverse` and `other_functions` in Release, bounds of `nth_root()`, `pow()`, `atan2()`, `cos()`, `div_rel()` and `mid()` not enclosing the exact values, and `elementary` in Debug, a bound of `atan2()`. Clang 18 honours the rounding direction there. | Refused by the three builds; a job checks the refusal (`linux.yml`). Ubuntu 22.04 arm64 is built with GCC only, Ubuntu 24.04 and 26.04 arm64 with Clang too. |
| MinGW-w64 whose `fma()` or `round()` is wrong: on x64, GCC 11 to 13 of Chocolatey (mingw-w64 before 12) and the mingw-w64 linked with `msvcrt.dll` rather than the UCRT (MSYS2 MINGW64, the cross compilers of Debian and Ubuntu); on 32-bit x86, GCC 11 of WinLibs (mingw-w64 9); on ARM, mingw-w64 before 11 (not tested) | **Wrong bounds.** On x64, the `fma()` and `round()` are those of mingw-w64's own math library, computed in doubles. That `fma()` adds the products of the halves of its arguments with four roundings: it is not correctly rounded (10.7 % of the error-free products `fma(a, b, -a*b)` and 25 to 73 % of other triples wrong, depending on the rounding direction), and CORE-MATH computes with it, GCC for Windows having no `-mfma`. That `round()` depends on the rounding direction: `round(0x1.fffffffffffffp-2)` is 1 except rounding upward. On 32-bit x86 the same `fma()` computes on the x87 unit, and the one of the mingw-w64 9 of WinLibs, compiled without optimization, rounds each of its partial sums to a double. GAOL v5 fails `elementary`, `reverse` and `core_math` there: bounds of `tan()`, `asin()`, `atan()`, `sinh()`, `cosh()`, `atanh()` and `atan2()` of small arguments do not enclose the exact values (the continuous integration, and mingw-w64 11 and the objects of MSYS2 MINGW64 under wine). From mingw-w64 12 on, a toolchain linking the UCRT takes the `fma()` and `round()` of `ucrtbase.dll`. | Refused by `gaol/gaol_config.h`; six jobs check the refusal, two of them MSYS2 MINGW64 with GCC and Clang (`windows.yml`). MinGW-w64 GCC 14 and 15 on x64, GCC 12 to 15 on 32-bit x86 (12 and 13 with the `fma()` of mingw-w64 11, which keeps its partial sums in extended precision and passes the tests, though not correctly rounded), and MSYS2 UCRT64 and CLANG64 are built and tested; `tests/core_math.cpp` checks `fma()` and `round()` first. |
| Visual C++, and clang-cl from Clang 16, without `/fp:strict` (`/fp:precise`, their default) | **Bounds not certified.** They then assume rounding to nearest, and may evaluate or rewrite floating-point operations accordingly, while GAOL computes with the rounding direction set upward. | Refused by `gaol/gaol_config.h`; `gaol::gaol` gives `/fp:strict` to the code linking it, and each Visual Studio job checks the refusal (`tests/fp_strict`), those of clang-cl included (GAOL v5). |
| The SSE2 intervals on 32-bit Windows | **Crash.** GCC takes the memory of `new` to be aligned on 16 bytes, which the C runtime of 32-bit Windows aligns on 8, and a `std::vector` of SSE2 intervals crashes on its first `movaps`: with MinGW-w64 14.2 and 15.2 for x86, in Release and in Debug, `arithmetic`, `other_functions`, `reverse` and `rounding_direction` crash. With `-faligned-new=8` the tests pass, but the code using GAOL would need that flag too. | The FPU intervals are compiled there by the three builds, as with Visual C++. On 64-bit Windows (MinGW-w64, MSYS2) the SSE2 intervals are compiled and pass the tests. |
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
    `gaol/gaol_expression.h` wrote on `std::cout` without `<iostream>`. The
    Debug jobs of CMake, which now define `GAOL_DEBUGGING` too, and the test
    `debugging` compile that code.
  - `minimum()` and `maximum()` of two zeros of different signs gave +0 and
    -0 with Visual C++ in Release, rather than -0 and +0, which the unit
    test of GAOL 4 `float_functions` checks, built with Visual C++ since the
    tests of `check/` are in `tests/`.
  - A shared `libgaol` (configure, meson) did not export the classes and
    functions of the expressions (`gaol/gaol_expression.h`), and a program
    building one did not link with it. The shared library job of CMake and
    the meson jobs, whose tests link with the installed `libgaol.so`, build
    `tests/expressions.cpp`.
- **Reports of the sanitizers that were suppressed at first**, and are fixed
  now: the nodes the parser did not free (`lsan.supp`,
  [issue #4](https://github.com/Jordan08/GAOL/issues/4)). Nothing is
  suppressed any more.
