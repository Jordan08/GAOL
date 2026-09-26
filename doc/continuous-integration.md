<!-- Copyright (c) 2026 ENSTA, France
     Created 2026-09-20 by Jordan NININ -->
# Continuous integration

Part of the documentation of [GAOL v5](../README.md#documentation).

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
  - Visual Studio 2022 and 2026, on x86, x64 and arm64, Release and Debug (on
    arm64, the compiler of Visual Studio 2022, MSVC 14.44, installed with
    Visual Studio 2026: GitHub has no image with Visual Studio 2022 on arm64
    any more);
  - MinGW-w64 15 on x86 and x64, Release and Debug, 14 on x86 and x64, 12 and
    13 on x86;
  - MSYS2 UCRT64 (GCC) and CLANG64 (Clang).

They also build GAOL with autotools and meson on Ubuntu (x86_64, arm64),
Debian (i386, armhf), macOS (arm64, x86_64) and MSYS2, and the tests with the
GAOL they install, which is the only library installed, CORE-MATH being
compiled into it; build GAOL as a part of another project, brought in by
FetchContent (`tests/fetch_content`), and the tests with the GAOL that project
installs; check that the three builds agree on each of these machines; check
that the builds refuse Clang on 32-bit ARM and Clang 14 on 64-bit ARM, that
`gaol/gaol_config.h` refuses MinGW-w64 GCC 11 to 13 on x64 and GCC 11 on x86,
and Visual C++ without `/fp:strict`. Jobs of each build
restore the rounding direction (`GAOL_PRESERVE_ROUNDING`): Ubuntu x86_64 GCC
and arm64 Clang, Debian i386 and armhf, macOS arm64, Visual Studio x64,
autotools and meson. The jobs built in Release print the time per operation in
their summary.

The manuals, that of GAOL v5 (`manual/v5/gaol.tex`) and that of GAOL 4
(`manual/v4/gaol.tex`), are built with the LaTeX of Ubuntu 24.04, by the
autotools and the meson builds, when `manual/` changes (`manual.yml`); the
PDFs are artifacts of the run.

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
| Clang 14 on 64-bit ARM (Ubuntu 22.04 arm64), and any compiler saying of `-frounding-math` "overriding currently unsupported rounding mode on this target" | **Wrong bounds.** Clang 14 does not honour the rounding direction on 64-bit ARM, and says so. Built by it, GAOL v5 fails `arithmetic`, `elementary`, `reverse` and `other_functions` in Release, bounds of `nth_root()`, `pow()`, `atan2()`, `cos()`, `div_rel()` and `mid()` not enclosing the exact values, and `elementary` in Debug, a bound of `atan2()`. Clang 18 honours the rounding direction there. | Refused by the three builds; a job checks the refusal (`linux.yml`). Ubuntu 22.04 arm64 is built with GCC only, Ubuntu 24.04 and 26.04 arm64 with Clang too. |
| MinGW-w64 whose `<fenv.h>` answers `fegetround()` from a state of its own: GCC 11 to 13 of MinGW-Builds on x64 (mingw-w64 before 12), GCC 11 on 32-bit x86 (mingw-w64 before 11) | **Wrong bounds.** GAOL sets the rounding direction by writing the registers of the processor, and CORE-MATH's functions read `fegetround()` to know it, which these mingw-w64 answer from a state of their own: the elementary functions would be rounded in another direction than the one GAOL set. | Refused by `gaol/gaol_config.h`; four jobs check the refusal (`windows.yml`). MinGW-w64 GCC 14 and 15 on x64, GCC 12 to 15 on 32-bit x86, and MSYS2 are built and tested. |
| Visual C++ without `/fp:strict` (`/fp:precise`, its default) | **Bounds not certified.** Visual C++ then assumes rounding to nearest, and may evaluate or rewrite floating-point operations accordingly, while GAOL computes with the rounding direction set upward. | Refused by `gaol/gaol_config.h`; `gaol::gaol` gives `/fp:strict` to the code linking it, and each Visual Studio job checks the refusal (`tests/fp_strict`). |
| The SSE2 intervals on 32-bit Windows | **Crash.** GCC takes the memory of `new` to be aligned on 16 bytes, which the C runtime of 32-bit Windows aligns on 8, and a `std::vector` of SSE2 intervals crashes on its first `movaps`: with MinGW-w64 14.2 and 15.2 for x86, in Release and in Debug, `arithmetic`, `other_functions`, `reverse` and `rounding_direction` crash. With `-faligned-new=8` the tests pass, but the code using GAOL would need that flag too. | The FPU intervals are compiled there by the three builds, as with Visual C++. On 64-bit Windows (MinGW-w64, MSYS2) the SSE2 intervals are compiled and pass the tests. |
| Doubles computed on the x87 unit of 32-bit x86 | **Wrong results rounded to nearest, and wrong bounds.** The x87 unit computes with 64-bit significands and 15-bit exponents, and CORE-MATH assumes every operation on doubles rounded to a double (`FLT_EVAL_METHOD` 0, see `3rd/math-core/README.md`). With GCC 12 on Debian 12 i386, CORE-MATH rounded to nearest gives the other neighbour of the exact value at 175 of the arguments compared with SSE2, its results being rounded twice: `exp(-0x1.74910d52d3051p+9)` is 0 rather than 2^-1074, `tanh(0x1.30fc1931f09c9p+4)` 1 rather than 1 − 2^-53. With GCC 9 on x86-64 (`-mfpmath=387`), which turns into doubles at compile time, rounded to nearest, the constants CORE-MATH rounds in the direction in effect (`0x1p-1074 * 0.5`, `-1.0 + 0x1p-54`), GAOL's bounds of `exp2(-1075)`, `exp10(-400)`, `expm1(-800)` and of `atan2()` of a tiny and a huge number do not enclose the exact values. | Refused by `gaol/gaol_config.h`; the builds compile with `-msse2 -mfpmath=sse` on 32-bit x86. `tests/extended_precision.cpp` checks these arguments, CORE-MATH in the four rounding directions and GAOL's bounds, and fails on the x87 unit. |
| Debian 11 Bullseye (amd64, arm64, armhf) | **Cannot be installed.** Out of support since August 2026: deb.debian.org lists packages of its security repository that it no longer serves (`libc-dev-bin 2.31-13+deb11u14`...), so that `g++` cannot be installed in the container; without that repository the packages are broken, and archive.debian.org does not have it yet. GAOL v5 itself builds and passes its tests there, with GCC 10.2 and CMake 3.18.4 installed from snapshot.debian.org. | Left out of the continuous integration. Debian 12 and 13 are built. |
| The intervals of floats (`gaol::intervalf`, `gaol::interval2f`) | **Unfinished.** `sqrt(intervalf)` returns its argument and `interval2f::inverse()` aborts; neither IBEX nor Codac uses them. | Off by default in the three builds (`GAOL_FLOAT_INTERVALS`); the audit compares the option between them. |

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
- **Reports of the sanitizers that were suppressed at first**, and are fixed
  now: the nodes the parser did not free (`lsan.supp`,
  [issue #4](https://github.com/Jordan08/GAOL/issues/4)). Nothing is
  suppressed any more.
