<!-- Copyright (c) 2026 ENSTA, France

     By: Jordan NININ (created 2026-09-20)

     GAOL is a software distributed WITHOUT ANY WARRANTY. Read the associated
     COPYING.LIB file for information. -->
# The three builds

Part of the documentation of [GAOL v5](../README.md#documentation).

On a given machine with a given compiler, GAOL behaves the same whichever
build configured it: CMake, configure and meson give it the same macros and the
same flags. The reference is the autotools build of GAOL, then its meson
build, and the CMake build follows them, apart from the errors corrected (see
[What differs from GAOL](differences.md)). In every build, by default:

- GAOL is compiled in release: `-O3`, `NDEBUG`, `-std=c++11` (`-std=c++14` for
  Clang with the target of Visual C++, whose standard library needs it), and,
  for the library alone (`gaol/` and `3rd/`: the tests, the examples and the
  benchmarks are compiled as the code using GAOL, with the flags of interval
  arithmetic below and without these), the optimizations configure adds
  (`-funroll-loops -fomit-frame-pointer -fexpensive-optimizations`, each where
  the compiler takes it), `-falign-functions=64`, hidden visibility
  (`-fvisibility=hidden -fvisibility-inlines-hidden`) and `-Wall -Wconversion`,
  which GAOL compiles without warnings, `-Wsign-conversion` of Clang included
  (meson's own `warning_level`, 1 by default, gives `-Wall` to everything it
  compiles).
  Its libraries are compiled in release too where a project brings GAOL in
  without choosing a build type, as by a configure calling GAOL's: a CMake
  project without `CMAKE_BUILD_TYPE`, with a generator of one configuration
  (with a generator of several, as Visual Studio, Release comes first among
  the configurations of GAOL built alone, and a project bringing GAOL in
  chooses its configuration), a meson project in the default build type of
  meson, with its ninja backend (GAOL v5);
- with the flags of interval arithmetic of [Using GAOL](using.md), and the code
  using GAOL, the tests included, is linked with `-mno-daz-ftz` where the
  compiler accepts it (GCC 13 and later on x86, from 11.4 and 12.4 in the
  series 11 and 12, and Clang 20 and later), in `gaol::gaol` and in `gaol.pc`:
  each build tests the compiler with a link (GAOL v5);
- on x86 processors, the intervals are computed with SSE2 instructions
  (`-msse2 -msse3`; no flag with Visual C++ and clang-cl for x64, where SSE2 is
  the baseline of the processor). On 32-bit Windows the C runtime aligns the
  memory of `new` on 8 bytes, and an SSE2 interval needs 16: GCC took it to be
  aligned on 16, and a `std::vector` of SSE2 intervals crashed. GCC and Clang
  (MinGW-w64, MSYS2) are told what the runtime gives with `-faligned-new=8`, so
  that they ask for aligned memory for the intervals and for what holds one, and
  `gaol::gaol` and `gaol.pc` give the flag to the code using GAOL too, as it
  allocates them; `gaol/gaol_config.h` refuses the SSE2 intervals compiled
  without it. Visual C++ and clang-cl have no such flag (`/Zc:alignedNew` does
  nothing in C++14 with `cl`, and breaks the STL of clang-cl, and only C++17 turns
  the aligned `new` on): their SSE2 intervals keep their two bounds as two
  doubles, whose alignment is 8, and load and store them with `movupd`
  (`GAOL_SSE2_UNALIGNED_STORAGE` of `gaol/gaol_config.h`, which derives it from
  the target: GAOL v5), so that they need no aligned memory, and the class
  allocators that aligned the intervals on 16 bytes are not compiled;
- on 64-bit ARM processors, the intervals are computed with NEON instructions
  (`GAOL_SIMD`, `--enable-simd`, `-Denable-simd`; no flag), on by default
  with every compiler of them: GCC, Clang and AppleClang under Linux and macOS,
  clang-cl and the Clang of MinGW-w64 and MSYS2, and Visual C++ (`cl`, whose
  registers are those of `_ReadStatusReg()` and `_WriteStatusReg()`; GAOL v5),
  whose compiler each build tests with the same program (CMake and meson for
  Visual C++, which configure does not build with): the intrinsics of
  `<arm_neon.h>` and the registers of the rounding mode. Not with 32-bit ARM,
  whose NEON has no vector of doubles: the FPU computes the intervals there, as
  before. A universal build of macOS (`CMAKE_OSX_ARCHITECTURES` with several
  architectures, or `-arch` several times in the flags of configure and meson)
  compiles each source once per architecture with one configuration, and takes
  the intervals of the architecture being compiled, NEON for arm64 and SSE2 for
  x86_64, where `GAOL_SIMD` allows them (`GAOL_UNIVERSAL_NEON` and
  `GAOL_UNIVERSAL_SSE2` of `gaol_configuration.h`; GAOL v5). `GAOL_SIMD` chooses
  between them and the FPU on every processor (GAOL v5: `GAOL_ARM64`,
  `--enable-arm64` and `-Denable-arm64` were an option of their own for the NEON
  intervals, independent of `GAOL_SIMD`);
- with GCC 12 for a 64-bit ARM target (Debian 12), `-fno-section-anchors`
  (GAOL v5), among the flags of interval arithmetic of [Using GAOL](using.md):
  each build tests the compiler with the same program (`__GNUC__ == 12` and
  `__aarch64__`, not Clang), and gives the flag to GAOL, CORE-MATH, the tests
  and the examples, and to the code using GAOL (`gaol::gaol`, `gaol.pc`). With
  `-frounding-math`, GCC 12.2 writes `std::numeric_limits<double>::denorm_min()`
  in a list of doubles as a 16-byte long double (the defect of
  [Using GAOL](using.md) of GCC 12.1 to 12.3 and 13.1 to 13.2, which reaches a
  list of doubles on 64-bit ARM), and the constants after it in the anchor of
  the section are read 8 bytes too early: `decimal_output()` of
  `tests/numbers.cpp` read the precisions `{1, 2, 4, 6, 15, 16, 17, 20}` as
  `{0, 1074790400, 1, 2, 4, 6, 15, 16}`, and the test never ended, writing 10^9
  digits in `snprintf` (the job `Debian 12 Bookworm arm64` of `containers.yml`
  stopped it at 300 s; `-O1`, `-fno-dse`, `-fno-store-merging` and
  `-fstack-reuse=none` do not mend it). The shift depends on the layout of the
  constants of the file, so that every change of `tests/numbers.cpp` moved the
  failure. `-fno-section-anchors` mends the offsets, and `tests/numbers.cpp`
  writes the double as a literal, which mends the list;
- GAOL and CORE-MATH are compiled with the fused multiply-add
  instructions of the processor, where the compiler has a flag for them
  (whenever the machine building runs a program compiled with them: no option
  of the user, `-DGAOL_FMA=OFF` of the developers leaves them out, see [Building
  GAOL](building.md#what-the-configuration-decides)): `-mfma` on x86; with GCC for
  Windows, on x86-64 and on 32-bit x86, `-mfma -mprefer-vector-width=128`
  (GAOL v5, see [GCC for Windows](#gcc-for-windows) below);
  `/arch:AVX2` with Visual C++ for x64, not for 32-bit x86, where it broke
  the rounding direction; `-mfpu=neon-vfpv4 -mfloat-abi=hard` on 32-bit ARM;
  and only where the machine building runs a program so compiled, but when
  cross-compiling: under Rosetta on macOS 14 (no longer in the CI), which had
  no AVX, the x86_64 tests stopped on an illegal instruction.
  GAOL's exact products (`fma_once()`) and CORE-MATH's functions then compute
  `fma()` with one instruction rather than with a call to the math library: on
  an Intel i7-1185G7 with Clang 21.1, `pow(x, 3)` of an interval takes 7.0 ns rather than 21.9, `sin` 1.4 times less and `cos` 1.4 times less, measured in
  [What the options are worth](#what-the-options-are-worth). The code using GAOL is given the flag
  too (`gaol::gaol`, `gaol.pc`), and compiled for the same processor.
  `-ffp-contract=off` stays, which forbids the compiler to contract a
  multiplication and an addition into a fused one, as CORE-MATH asks;
- with the AVX-512 path of `x + y`, `x - y`, `x * y`, `x / y`, `sqrt(x)`,
  `sqr(x)` and, with FMA, the integer powers beyond 1 and -1, taken where
  the rounding direction is preserved, whose every operation the path
  relieves of the store of that direction (`GAOL_PREFER_AVX512`, a setting
  of the developers, asks for the path without that option and refuses it
  with it, see [Building GAOL](building.md#the-settings-of-the-developers)),
  and where the machine building runs the instructions
  (`GAOL_AVX512_ARITHMETIC`), which is checked as for the fused
  multiply-add instructions, the path being left out when cross-compiling:
  the library then needs a processor with them, which `gaol::init()`
  checks, and the operations test nothing as they run; never with GCC for
  Windows (MinGW-w64, Cygwin), which does not align the stack for the
  AVX-512 registers (GCC bug 54412; nor for the 256-bit ones of AVX, which
  `-mprefer-vector-width=128` keeps MinGW-w64 GCC out of, see [GCC for
  Windows](#gcc-for-windows) below). No flag to compile,
  the path's functions alone carry the target attribute, which Visual C++
  has not and needs not, taking the intrinsics of any instruction set in any
  function. Visual C++ has no `__builtin_cpu_supports()`, and the one of
  clang-cl needs the runtime library of LLVM: both tell the processor by
  `cpuid` and `xgetbv` (GAOL v5, see
  [Using GAOL](using.md#the-avx-512-path));
- without the intervals of floats, `gaol::intervalf` and `gaol::interval2f`
  (SSE3), unfinished, for which no build has an option: only a developer of
  GAOL compiles them, defining `GAOL_FLOAT_INTERVALS` in `gaol/gaol_config.h`;
- with exceptions, the rounding direction left upward, and silent: no line
  on the standard error when GAOL initializes and cleans up, unless
  `GAOL_VERBOSE_MODE` (`--enable-verbose-mode`, `-Denable-verbose-mode=true`)
  is asked for, where configure wrote it by default;
- the processor and the system (`GAOL_IX86_MACOSX`, `GAOL_ARM_MACOSX`...),
  the sizes of the integer types and the byte order are read from the macros
  of the compiler, in `gaol/gaol_config.h`, rather than from the machine
  building;
- the three write the same macros into `gaol/gaol_configuration.h`, and
  nothing else (see
  [Building GAOL](building.md#the-configuration-of-gaol)) (GAOL v5);
- the three read the version of GAOL from the file `VERSION.txt`, the only one
  to change for a new version, by the same rules (see
  [Building GAOL](building.md#the-version-of-gaol)) (GAOL v5);
- the shared library is `libgaol.so.5.0.0`, whose soname is `libgaol.so.5`,
  where the build makes one (configure, meson, CMake with
  `BUILD_SHARED_LIBS`); it was `libgaol-5.0.so.0` with configure, and meson
  gave no version (GAOL v5). On macOS the three builds make `libgaol.5.dylib`
  (CMake keeps the file `libgaol.5.0.0.dylib` and links to it), but its
  Mach-O versions differ, as `otool -L` shows on an Apple M2 with macOS 27 and
  Apple clang 21: compatibility and current version 5.0.0 with CMake and
  meson, 6.0.0 with configure, where libtool adds 1 to the current version of
  `-version-info` on Darwin (`ltmain.sh`, "Darwin ld doesn't like 0 for these
  options"). It matters only to a program linked with the dylib of one build
  and run with the dylib of another (not tried).

In a Debug build (`CMAKE_BUILD_TYPE=Debug`, `configure --enable-debug`,
`meson setup --buildtype=debug`), GAOL is compiled with `-g`, without
optimization nor `NDEBUG`, so that `assert()` stays active. It was
`configure --enable-debug` alone that defined `GAOL_DEBUGGING`, which checked
GAOL's assertions (`GAOL_ASSERT`) in GAOL 4, adding `-g
-ansi -Weffc++ -pedantic` for `g++` alone to the optimizations of
`--enable-optimize`; the Debug build of CMake did not define it, and meson
defined it with its option `enable-debug` (GAOL v5). `GAOL_DEBUG` of CMake and
`enable-debug` of meson build the libraries of GAOL and CORE-MATH so whatever
the build type or the configuration, a project bringing GAOL in included, as
`configure --enable-debug` does for a configure calling GAOL's: the code of
that project, and the tests and the examples, keep their own (GAOL v5).

`.github/audit/` configures the three builds and compares the macros GAOL
sees, the macros each writes into `gaol/gaol_configuration.h`, and the flags
GAOL is compiled with, those of its C++ sources and those of a C source of
CORE-MATH (`compare.py --check`; GAOL v5: only the C++ flags were compared, and
the C sources had `-msse2 -msse3 -fvisibility=hidden` with CMake alone and the
standard of the compiler with configure, `gnu11` with meson, `gnu99` with CMake);
the continuous integration runs it on each kind of machine, with the default
options and with five sets of options: the rounding direction preserved, with and
without the AVX-512 path, the FPU intervals, no fused multiply-add instruction,
no exceptions and the verbose mode (GAOL v5: the default options alone).

## What the options are worth

Measured with `tests/performance.cpp` on an Intel i7-1185G7 with Clang 21.1,
in nanoseconds per operation, the best of nine runs, each program being run in turn with the others, on 12 November 2026 (GAOL v5; two sessions: the default build and `-DGAOL_FMA=OFF`, then the default build again and `-DGAOL_SIMD=OFF`, whose default build gave 1.24, 2.53, 2.06, 6.82, 24.0, 27.7, 35.7 and 34.5):

| | x + y | x × y | sqr(x) | pow(x, 3) | exp(x) | log(x) | sin(x) | cos(x) |
|---|---:|---:|---:|---:|---:|---:|---:|---:|
| Default (`GAOL_FMA` and `GAOL_SIMD` on) | 1.23 | 2.49 | 2.08 | 7.03 | 22.9 | 27.8 | 36.0 | 34.4 |
| `-DGAOL_FMA=OFF` | 1.25 | 2.52 | 2.03 | 21.9 | 26.1 | 43.4 | 50.2 | 49.4 |
| `-DGAOL_SIMD=OFF` | 2.52 | 4.17 | 2.63 | 8.94 | 23.6 | 27.6 | 34.4 | 39.2 |

- **The fused multiply-add instructions (`GAOL_FMA`) pay on the elementary
  functions**: pow(x, 3) is **3.1 times faster** with them, log 1.6 times, cos 1.4 times, sin 1.4 times and exp 1.1 times. CORE-MATH computes with
  `__builtin_fma` throughout, which is one instruction where the processor has
  it and a call to the math library where it has not. They are on by default wherever the compiler has a flag for them and the
  machine building runs a program compiled with it. They do nothing for the
  arithmetic, which does not multiply and add in one step.
- **The SSE2 intervals (`GAOL_SIMD`) pay on the products and the additions**:
  x × y takes 2.5 ns rather than 4.2, and x + y 1.24 rather than 2.52, the two
  bounds being computed in one instruction. They are on by default on x86
  processors. The elementary functions do not go through them.

On a 64-bit ARM processor, measured the same way on an Apple M2 with AppleClang
21.0, the median of nine runs of each program, 12 November 2026 (GAOL v5): the first two builds in turn in one session, the last two in another:

| | x + y | x − y | x × y | x ÷ y | sqr(x) | sqrt(x) | 13 operations in a line |
|---|---:|---:|---:|---:|---:|---:|---:|
| `-DGAOL_SIMD=OFF` (FPU intervals) | 1.15 | 1.15 | 3.25 | 3.24 | 1.78 | 5.15 | 50.4 |
| Default (NEON intervals) | 0.89 | 0.89 | 1.52 | 2.07 | 1.26 | 2.03 | 16.6 |
| `-DGAOL_SIMD=OFF -DGAOL_PRESERVE_ROUNDING=ON` | 38.2 | 38.7 | 43.6 | 44.9 | 43.6 | 47.8 | 532 |
| `-DGAOL_PRESERVE_ROUNDING=ON` (NEON intervals) | 26.5 | 27.1 | 27.7 | 35.0 | 27.1 | 35.6 | 370 |

- **The NEON intervals (`GAOL_SIMD`) pay on every operation of the table**:
  x − y takes 0.89 ns rather than 1.15, x × y 1.52 rather than 3.25 (of operands of random signs 1.59 rather than 3.31), x ÷ y 2.07 rather than 3.24, sqrt 2.03 rather than 5.15, and a line of 13 operations 16.6 ns rather than 50.4.
  The two bounds are one vector, and the operations take them from one to the
  next in a register. The elementary functions, which CORE-MATH computes,
  go through them only for their arguments: no faster, no slower. They are on by
  default on 64-bit ARM processors with GCC, Clang or AppleClang.
- **The product, the quotient and the square of the FPU intervals take no
  branch on the signs of the bounds** (GAOL v5), and x − y reads its bounds
  after the check of the rounding direction. The row read before 3.54 for
  x − y, 3.25 for x × y (6.39 of operands of random signs), 3.98 for x ÷ y,
  2.07 for sqr(x) and 49.6 for the line of 13 operations. The product of a line
  of operands of one sign, which a processor predicts, takes more time than it
  did (a line of 7 products, 35.0 ns rather than 23.3), the largest of four
  products waiting for two comparisons. With `GAOL_PRESERVE_ROUNDING` the write
  of FPCR waits for the result as well, and the FPU row of that option loses 6 to 7 %
  on operands of one sign (x × y 42.4 ns rather than 39.7 for the former code,
  measured in the same session) and gains on operands of random signs (42.4
  rather than 47.5). These
  are the figures of this processor, with AppleClang. The processors of the
  continuous integration that run the FPU intervals (32-bit ARM, Ubuntu with
  GCC and Clang, macOS; and, when they were measured, Visual C++ and clang-cl
  for x86 and for arm64, which have the SSE2 and the NEON intervals now: the
  columns "VS x86" and "clang-cl x86" of
  [The FPU intervals](using.md#the-fpu-intervals) are those of the FPU, which
  the same page measures again against the default intervals of these
  compilers: faster in 54 of 56 cells, up to 4.3 times),
  measured in turn, give other ratios, losses included (the line of 7 products
  0.58 with GCC on Ubuntu, 0.57 with clang-cl for arm64); POWER, s390x and
  RISC-V are not measured. See
  [The FPU intervals](using.md#the-fpu-intervals).
- **The rounding direction is read with one `mrs` of FPCR** (0.8 ns on this
  processor) and written with `msr` only when the program's is not upward:
  `msr` takes 4 ns for the value FPCR holds and 11 ns when it changes the
  direction. With `GAOL_PRESERVE_ROUNDING` an operation of a program that
  computes to nearest writes it twice, which is the 27 ns above (38 to 44 ns with the
  FPU intervals, which called `fegetround()` and `fesetround()`); a program that
  already computes upward writes nothing.

### GCC for Windows

With GCC for Windows (MinGW-w64, MSYS2, Cygwin) on x86-64 and on 32-bit x86,
`GAOL_FMA` gives `-mfma -mprefer-vector-width=128` (GAOL v5, for 32-bit x86
after the measures below). It gave nothing before, because of GCC bug 54412:
`-mfma` turns AVX on, the vectorizer of GCC then writes 256-bit registers
(`%ymm`), and GCC takes the stack to be aligned on 32 bytes for those it spills
there, where the ABI of Windows x64 aligns it on 16, so that an aligned
`vmovaps` to such a slot faults (as it did in the MinGW builds of Codac).
Without `-mfma`, CORE-MATH called a software `fma()` and the integer powers
computed no pairs of products. `-mprefer-vector-width=128` (a GCC option from
GCC 8) keeps the vectorizer to the 128-bit registers. `objdump -d
--no-show-raw-insn` found no line with `%ymm` in the objects of GAOL, CORE-MATH
and the tests built so by GCC 16.2 (MSYS2 UCRT64), against 30,598 lines with
`-mfma` alone, and none without a flag; the whole suite of tests passed with the
three. That is a count of instructions, and no proof that the defect cannot
occur: the continuous integration makes the count again on the objects it
builds, and the check of the fused multiply-add instructions with it
(`.github/scripts/check-gcc-windows-fma.sh`, in `windows.yml`). A GCC without
the option gets no flag. The code using GAOL is given the two flags
(`gaol::gaol`, `gaol.pc`); the 256-bit intrinsics it writes itself would stay
exposed to the defect, as with `-mavx`.

Measured with `tests/performance.cpp` on the Windows Server 2022 runner of
GitHub, MSYS2 UCRT64 with GCC 16.2, in nanoseconds per operation, the median of
three runs of each variant, alternated in one job (on these lines, the runs
of a variant differ by 1.16 times at most):

| | x × y | x ÷ y | pow(x, 3) | exp(x) | log(x) | sin(x) | cos(x) | floor(x) |
|---|---:|---:|---:|---:|---:|---:|---:|---:|
| No flag (`-DGAOL_FMA=OFF`, what GCC for Windows had) | 3.92 | 5.04 | 52.31 | 46.44 | 106.43 | 93.46 | 260.50 | 4.24 |
| `-mfma -mprefer-vector-width=128` (default) | 3.69 | 4.72 | 8.68 | 32.58 | 33.63 | 46.61 | 55.44 | 2.19 |

`-mfma` alone gave the same times. The lines of 15 additions and of 7 products
did not change (within 1.5 %; the line of 7 divisions took 6.3 % less). The
elementary functions and `pow(x, 3)` gain the most, as with Clang above.

For 32-bit x86, the same program with MinGW-w64 for i686 (MinGW-Builds, GCC
15.2 and 14.2, Windows Server 2022 runner of GitHub), built, tested (38 tests
in every variant, all passing, two skipped) and measured with each set of
flags in turn, in nanoseconds per operation (GCC 15.2; an earlier run of the
same variants gave times within 1.5 %):

| | x × y | pow(x, 3) | log(x) | cos(x) | lines with `%ymm` | lines with `vfmadd` |
|---|---:|---:|---:|---:|---:|---:|
| No flag (what GCC for Windows had) | 8.65 | 70.45 | 265.68 | 489.59 | 0 | 0 |
| `-mfma` | 8.76 | 12.23 | 81.24 | 138.19 | 4,709 | 850 |
| `-mfma -mprefer-vector-width=128` | 8.66 | 12.12 | 81.93 | 138.33 | 0 | 850 |
| `-mfma -mprefer-vector-width=128 -mstackrealign` | 8.67 | 12.14 | 82.12 | 139.20 | 0 | 850 |

A third run on another runner, faster, gave the same ratios (`pow(x, 3)` 56.6
against 10.05 ns, `log` 235.9 against 64.7, `cos` 430.1 against 121.3 ns, with
`-mfma -mprefer-vector-width=128`). With GCC 14.2 the ratios are similar
(`pow(x, 3)` 67.7 against 12.1 ns, `log` 268.6 against 103.0, `cos` 490.2
against 137.0 in the third run), and `-mfma` alone leaves 600 lines with
`%ymm`. GCC 13.2 (MinGW-w64 13.2 for x86) leaves 9 lines with `%ymm` in two test
objects even with both options: the check lists them and does not fail on
32-bit x86. `x × y`, which does not call `fma()`, did not change.

The stack of a thread on 32-bit Windows is aligned on 4 bytes (GCC bug 40838).
A program that keeps 16 intervals alive across calls in a loop of `+ - * /` and
`sqr`, then in a loop of `pow(x, 3)`, `pow(x, 5)`, `pow(x, -4)`, `pow(x, 7)`,
`exp`, `log`, `sin` and `cos` (which `-mfma` sends through the packed kernel of
the integer powers), on the stack of the main thread, of a `CreateThread`
thread and of a `std::thread`, gave the same results in the three, without a
flag, with `-mfma` and with `-mfma -mprefer-vector-width=128`, with GCC 15.2
and 14.2. That does not prove that a stack aligned on 4 bytes can never meet an
aligned access of 16 or 32 bytes. `-mstackrealign` changed no time with GCC
15.2 (1 % at most); `-mincoming-stack-boundary=2` was measured once, with the
times of the base. GAOL gives neither. It gives `-mprefer-vector-width=128`
with `-mfma` as on x86-64, which leaves no `%ymm` in the objects at the same
speed; that bug 54412 acts on 32-bit x86 was not shown (GCC 15.2 for 32-bit
Linux realigns the stack itself for the 256-bit spills it makes).

These flags are for GCC for Windows on x86-64 and on 32-bit x86: MinGW-w64,
MSYS2 and, since the tests of the three builds name it (`CYGWIN` of CMake,
`__CYGWIN__` of configure and meson; the AVX-512 path names `__CYGWIN__` too),
Cygwin (GAOL v5; before, Cygwin got `-mfma` alone). The MSYS environment of
MSYS2 (not its MinGW ones) is a Cygwin as far as these tests go, and gets the
same flags. If the user gives a `-mprefer-vector-width=` of his own, the builds
order it differently from GAOL's: CMake and configure put GAOL's flags after the
user's (`CMAKE_CXX_FLAGS`, `CXXFLAGS`), meson puts them before `cpp_args`. The
last of two such options is the one that counts (as tried with Clang).

Cygwin x86-64 is built and tested by a job of the continuous integration
(`windows.yml`, GCC 14.4, CMake 4.4, Release and Debug, GAOL v5; not measured), whose
first runs found what Cygwin needed. Its `<stdlib.h>` does not declare
`posix_memalign()` under `-std=c++11` (the aligned memory is `_mm_malloc()`
there). Its `fma()` is not correctly rounded (`fma(1 + 2^-30, 1 - 2^-30, -1)`
gives 0), and `std::fma` is a call of it without optimization: GAOL calls
`__builtin_fma` (`fma_once()`), which is the instruction with `-mfma` at -O0 and
at -O3 (both checked), and `gaol_fma_upward()` where the build has no `-mfma`
(`GAOL_SOFT_FMA`, see [the older MinGW-w64](#the-fma-and-round-of-mingw-w64)). Its
`nextafter(-DBL_MAX, -oo)` is `-DBL_MAX` in the rounding direction upward, which
`previous_float()` and `next_float()` now tell apart (the lower bound of
`sinh(-711)` did not enclose the value; `elementary` failed on it).
`is_bisectable()` and `bisect()` call no `nextafter()`: they tell the bounds
by their bits. The Debug
build of Cygwin is built and tested too; `tests/core_math.cpp` and
`tests/arithmetic.cpp` call `__builtin_fma` as GAOL does.

CPack of Cygwin could not make the archive of the sources in the job of 23
October 2026 (CMake 4.4.4, libarchive 3.8): TGZ, TBZ2 and ZIP all failed to add
`README`, a symbolic link to `README.md` in the repository, with "Couldn't list
extended attributes", so that `package_source` is not available there and the
test `cpack_stale_configure` says it is skipped. The cause was not found:
`cmake -E tar`, GNU `tar` and `git archive` make an archive of the checked-out
tree, and `listxattr()`, `llistxattr()` and `flistxattr()` find nothing wrong
with its link.

`-mprefer-vector-width=128` does not undo what other options of the user turn
on. With the Clang that cross-compiles for x86-64, the objects of GAOL, CORE-MATH
and the tests built with `-mavx512f -mfma -mprefer-vector-width=128` held 2
lines with `%ymm` and 2 887 with `%zmm`, and those built with
`-march=skylake-avx512 -mprefer-vector-width=128` 406 lines with `%ymm`; a second
reader counted none, and 4: the gap is not explained, and the `%zmm` may come
from the explicit intrinsics of the AVX-512 path rather than from the
vectorizer. Nothing was run with GCC but a small loop, which GCC 15.2 compiled
with no `%ymm` and no `%zmm` with `-mavx512f -mfma`, with
`-march=skylake-avx512` and with `-march=native`, all with
`-mprefer-vector-width=128`. GCC's own code (`ix86_option_override_internal`,
version 14.2, read, not run) applies the preferred width of an architecture
only if none was given, so that `-mprefer-vector-width=128` should win over a
`-march`. A program built with
such options is exposed to GCC bug 54412, as with `-mavx`.

### Interprocedural optimization is not offered

Compiled with it (`-flto`), GAOL's basic operations are 1.2 to 1.5 times
faster where the code using GAOL is compiled with it too — an addition of intervals took 0.73 ns rather than 1.07 with Clang 21.1 (in an earlier session than the table above, whose default build took 1.07, 2.04 and 1.80 ns for its first three columns then), `x * y` 1.71 rather
than 2.04, `sqr(x)` 1.27 rather than 1.80 — and the elementary functions no
faster, a little **slower**: cos took 39.2 ns rather than 36.6, sin 32.6 rather
than 31.3, log 22.7 rather than 21.7. With Clang 18.1, before the operators
took their bounds in a register, the basic operations took two to three times
less time with it and the elementary functions two to three times more. The
sources of CORE-MATH are compiled apart, without it.

On 20 September 2026, with **GCC** and the code of then, it broke the
bounds: GAOL failed `tests/rounding_direction.cpp`, which checks that an
operation gives the same result whatever the rounding direction the calling
code left. With `GAOL_PRESERVE_ROUNDING`, `textToInterval("sin(1)+exp(0.1)")`
gave an upper bound one double below the right one when the caller left the
direction downward or toward zero: the compiler moved floating-point
operations across the changes of rounding direction, which `-frounding-math`
is meant to forbid and which it did not do across translation units.
Compiling GAOL's sources alone with it was enough for the test to fail; Clang
18.1 passed the tests with it. With GCC 15.2 and the code of 13 November 2026,
the 63 tests pass with it (`-DCMAKE_INTERPROCEDURAL_OPTIMIZATION=ON`, `-flto`
in the flags of the target `gaol`), `rounding_direction` among them, with and
without `GAOL_PRESERVE_ROUNDING` (GAOL v5), and with GCC 13.3, the direction
preserved, which a job of `linux.yml` builds with it, all the tests run. They
ran on the processors with AVX-512 only, whose arithmetic takes the AVX-512
path: with the SSE2 path (`-DGAOL_PREFER_AVX512=OFF`, and the AMD EPYC 7763 of
some runners), `arithmetic` failed with GCC 12.4, 13.3 and 14.2 and `-flto`
(5240 checks of 7 365 456 with 13.3, about 24 700 with the others; 15.2
passed): `operator+=(double)` and `operator-=(double)` computed their sum after
the change of rounding direction that gives the caller's back, in the caller's
direction (GCC bug 34678, as above), and now keep it before, as do `*=`, `/=`
and `%=` with a double, `%=` between intervals and `divide_general()`, the
quotient by an interval with a zero (GCC 12.4 failed `[1]%[3]` with literal
operands until `%=` between intervals did; no test failed for the other three
alone): GCC 12.4, 13.3 and 14.2 then pass. A job of `linux.yml` builds it
with the SSE2 path whatever the runner is (GCC 13.3,
`-DGAOL_PREFER_AVX512=OFF`). The other compilers and
versions were not tried with it.

It is not refused either: the standard settings of the tools reach the
sources of GAOL, `CMAKE_INTERPROCEDURAL_OPTIMIZATION` of CMake, given to GAOL
or to a project taking it by FetchContent, `-Db_lto=true` of meson and
`-flto` in `CXXFLAGS`. CMake and meson keep their settings away from the
sources of CORE-MATH; a `-flto` given by hand in the flags of C
(`CMAKE_C_FLAGS`, `CFLAGS`, `c_args`) reaches them, in the three builds: the
`-fno-lto` configure gives them comes before `CFLAGS` on the command line, and
the last of the two wins.

An interval that does not enclose the exact value is worse than a slow one, so
there is no option for this, and `gaol.pc` and `gaol::gaol` carry no such flag.

## Compilers and options refused

Each of these gave bounds not enclosing the exact results, or worse. The three
builds refuse the compilers of the first two rows when configuring, the C
compiler, which builds CORE-MATH, as well as the C++ compiler, with a
message naming what to use instead, and keep the options of the last four away
by giving GAOL the flags of [Using GAOL](using.md). `gaol/gaol_config.h`
refuses them again at compile time, for the code including GAOL's headers too,
all but the second row, which no macro of the compiler shows; the third row, ARM,
it alone refuses, when compiling:

| Refused | Because |
|---|---|
| Clang for 32-bit ARM processors | It does not honour the rounding direction there: built by Clang 21, 4556 of 16000 random products, squares and cubes did not enclose their exact values. GCC does. |
| A compiler saying of `-frounding-math` "overriding currently unsupported rounding mode on this target", as Clang 14 for 64-bit ARM | Bounds of `pow()` and `nth_root()` did not enclose the exact values. Clang 18 honours the rounding direction there. |
| MinGW-w64 for 32-bit ARM, and for 64-bit ARM of version 12 or later linked with `msvcrt.dll` rather than the UCRT (no job builds these toolchains; the refusal is tested with the macros of such a toolchain defined by hand on an x86 system, by `refused_mingw_arm32` and `refused_mingw_arm64_msvcrt`, which check the logic of the refusal and not a real toolchain for ARM; mingw-w64 for 64-bit ARM before version 12 is accepted, below, and MSYS2 CLANGARM64, which links the UCRT, is built and tested) | Its `fma()` and `round()` are those of mingw-w64's own math library, which are wrong on x86 (below), and `round()` depends on the rounding direction there too, computed in doubles. GAOL v5 calls its own `gaol_round()`; the `fma()` of 32-bit ARM is not fused (`fmacd`, which is `vmla.f64`, a product and a sum each rounded) and Clang for 32-bit ARM is refused anyway, and no toolchain for 64-bit ARM links `msvcrt.dll`, so that this case was not built. |
| `-ffast-math`, `-Ofast`, `/fp:fast` | The compiler then rounds to nearest and drops the checks of NaN and infinities. `-Ofast` is not refused where `-fno-fast-math` turns its fast-math optimizations off (below). Linking with them, or with `-funsafe-math-optimizations`, is a matter of its own (below). |
| `-ffinite-math-only`, which `-ffast-math` and `-Ofast` turn on | The compiler then takes NaN and infinities never to occur, in GAOL's inline functions too: the empty interval has NaN bounds, and `([1, 2] & [3, 4]).is_empty()` is false (GCC 9.4 and 13 at `-O0`, `-O2` and `-O3`; Clang 18 at `-O0`, and at `-O2` and `-O3` when the bounds are `volatile`). |
| Visual C++, and clang-cl from Clang 16, without `/fp:strict` (`/fp:precise`, their default) | They then assume rounding to nearest, and may evaluate or rewrite floating-point operations accordingly (clang-cl compiles with `-fno-rounding-math -ffp-contract=on`; GAOL v5 did not refuse it at first): no test gave a wrong bound so, but nothing certifies the bounds (see [What differs from GAOL](differences.md)). |
| Doubles computed on the x87 unit of 32-bit x86 processors (without `-msse2 -mfpmath=sse`, or `/arch:SSE2`) | CORE-MATH assumes every operation on doubles rounded to a double. Computed in extended precision, its results rounded to nearest are rounded twice, and 175 arguments gave the other neighbour of the exact value with GCC 12 on Debian 12 i386; with GCC 9, which rounds to nearest at compile time the constants CORE-MATH rounds in the direction in effect, GAOL's bounds of `exp2(-1075)`, `expm1(-800)` or `atan2()` of a tiny and a huge number did not enclose the exact values. `tests/extended_precision.cpp` checks these arguments. |

`gaol/gaol_config.h` refuses ARM64EC too (`_M_ARM64EC`, GAOL v5), the 64-bit
ARM code of Windows that keeps the conventions of x64, which no build of GAOL
targets: GAOL would take its paths for x64 there, which set the rounding
direction through the MXCSR of the SSE instructions, not known to reach the
FPCR that rounds the ARM64 instructions the code runs. A build for ARM64
computes with the NEON instructions. The test `refused_arm64ec` and
`tests/refused_options.sh` define `_M_ARM64EC` and check the refusal.

`-fno-fast-math`, one of the flags of interval arithmetic, turns `-ffast-math`
and `-ffinite-math-only` off when it comes after them on the command line, and
not when it comes before them: `gaol.pc` and `gaol::gaol` give it, and the code
including GAOL's headers is refused when `-ffast-math` or `-ffinite-math-only`
comes after `-fno-fast-math`. With Clang, `-ffast-math` and `-Ofast` turn
`-frounding-math` off too, which `-fno-fast-math` does not turn on again: the
remedy is all the flags of interval arithmetic after the option, not
`-fno-fast-math` alone (below). The CMake tests `refused_finite_math_only` and
`refused_fast_math` compile `tests/refused_options.cpp` with each of the two
options, after the flags of interval arithmetic, and check that
`gaol/gaol_config.h` refuses it, and `refused_positive` that it compiles
without them (GCC and Clang); `tests/refused_options.sh` does the same in the
autotools and meson builds. With GCC, `-fno-fast-math` turns off all the
fast-math optimizations of `-Ofast` but `-fcx-limited-range`, which changes
the arithmetic of complex numbers alone (GCC 9.4), wherever it stands on the
command line, before `-Ofast` too: `-frounding-math` stays, and the header has
nothing to refuse. With Clang, `-fno-fast-math` turns them off only when it
comes after `-Ofast`, and `-frounding-math` stays off.

With GCC, `gaol/gaol_config.h` refuses `-fno-signed-zeros`,
`-fno-trapping-math`, `-fassociative-math` and `-freciprocal-math` too, which
`-funsafe-math-optimizations` turns on, as do `-ffast-math` and `-Ofast` with
`-fno-finite-math-only`: GCC shows them by `__NO_SIGNED_ZEROS__`,
`__NO_TRAPPING_MATH__`, `__ASSOCIATIVE_MATH__` and `__RECIPROCAL_MATH__` from
GCC 12 on (12.2 and 15.2 checked; GCC 9.4 and 11.4 define none of them, and
the options are not refused there; GAOL v5, which said no macro showed them).
`-fno-fast-math` turns them off
when it comes after them, as it does `-ffinite-math-only`. Compiled with
GCC 15.2, each option after all the flags of interval arithmetic:

- `-fno-signed-zeros` failed 6 of the 63 tests. GCC drops the `+ 0.0` of
  `!(1.0 + (subnormal + 0.0) > 1.0)`, by which GAOL sees the modes flushing
  the subnormals to zero, which then misses the flush-to-zero mode alone:
  `width()` of [2, 3]·2^-1074 is 0 under it, below the exact width. With
  `-funsafe-math-optimizations`, GCC rewrote the former probes
  `1.0 + tiny == 1.0`, by which GAOL sees the rounding direction, as
  `tiny == 0.0` (GCC 9.4), and `1.0 + (subnormal + 0.0) == 1.0` as
  `subnormal == 0.0` (GCC 9.4 from `-O0`, and GCC 13), so that an operation
  did not set the direction upward again after the code using GAOL left it to
  nearest.
- `-fno-trapping-math` failed 52 checks of `tests/rounding_direction.cpp`:
  operations with an empty operand, whose bounds are NaNs, raised the
  invalid-operation flag, which no operation of GAOL raises then.
- `-freciprocal-math` failed no test, but lets the compiler compute x / y as
  x · (1 / y), whose two roundings upward do not bound x / y from above
  where x is negative; `-fassociative-math`, which GCC takes only with
  `-fno-signed-zeros` and `-fno-trapping-math`, lets it reorder the
  operations whose rounding a bound depends on.

The CMake tests `refused_no_signed_zeros`, `refused_no_trapping_math`,
`refused_unsafe_math` and `refused_reciprocal_math`, and
`tests/refused_options.sh`, check these refusals where the compiler shows the
options by these macros, which the configuration and the script look for.

No macro of the compiler shows the following, which `gaol/gaol_config.h`
cannot refuse and which give wrong results all the same: the code using GAOL
is not to be compiled with them.

- These options with GCC before 12.
- With Clang, which defines none of the macros above: `-fno-signed-zeros`,
  with which Clang 18 (at `-O2` and `-O3`) and Clang 21 drop the `+ 0.0` of
  the probe of the modes as GCC does, one `addsd` and one `ucomisd` being left
  (Clang 18 does not rewrite the probe of the direction); `-fno-trapping-math`,
  `-fassociative-math` and `-freciprocal-math`; `-funsafe-math-optimizations`,
  which turns them on; and `-ffast-math` or `-Ofast` with
  `-fno-finite-math-only`, with which Clang defines `__NO_MATH_ERRNO__` alone
  (Clang 18 and 21).
- `-fno-honor-nans` alone, with Clang, which does to the empty interval what
  `-ffinite-math-only` does (the sum of the empty set and [1, 2] is not
  empty), and `-fno-honor-infinities` alone, which does it to the unbounded
  intervals (`interval::universe().is_entire()` is false, and [-oo, 5] is
  empty): `__FINITE_MATH_ONLY__` is 1 only with both.
- `-ffast-math` or `-Ofast` followed by `-fno-fast-math`, after the flags of
  interval arithmetic, with Clang (18): the macros show nothing, but
  `-frounding-math` stays off, and the compiler takes the rounding direction
  to be to nearest. `tests/rounding_direction.cpp`, compiled so, failed 54 of
  its 292728 checks after `-ffast-math -fno-fast-math` (operations with an
  empty operand raised the invalid-operation flag), and 126 after
  `-Ofast -fno-fast-math` (bounds with subnormal results under flush-to-zero
  too); with the flags of interval arithmetic given again after the option,
  none. With the FPU intervals, the relations in loops are among what fails
  (`certainly_le()`, `certainly_leq()`, `certainly_ge()`, `certainly_geq()`
  and the operators `<`, `<=`, `>`, `>=`): at `-O3 -march=native`, Clang 18
  and 20 vectorize them into the signaling comparisons `vcmplepd`, `vcmpnlepd`
  and `vcmpnltpd`, which they make for the empty operands too, whose bounds
  are NaNs (120 of 184 runs of such loops
  raised the flag, none with `-frounding-math`; `-fno-fast-math` or
  `-ffp-contract=off` alone change nothing). `tests/rounding_direction.cpp`
  runs such loops on empty operands, and fails when one raises the flag or
  gives a wrong result.

The probe of the modes is `!(1.0 + (subnormal + 0.0) > 1.0)`, not
`== 1.0`: with `-funsafe-math-optimizations`, GCC 15 does
not rewrite it as `!(subnormal > 0.0)`, and with `-fno-signed-zeros` GCC 15 and
Clang 21 drop its `+ 0.0` as they did that of the earlier form (GCC 9 to 14
were not tried with it). The code using GAOL is still not to be compiled with
these flags.

### The `fma()` and `round()` of mingw-w64

Two reasons that had GAOL refuse MinGW-w64 are gone with CORE-MATH (GAOL v5):
the math library of mingw-w64 older than version 12 gave `acosh()` near 1 up
to 25 million doubles away from the exact value, and the `fesetround()` of
mingw-w64 12 runs the instruction `cpuid` at each call. GAOL takes no
elementary function from the math library of the system any more, and no
longer changes the rounding direction for its elementary functions. A third
reason was left, the `fma()` and `round()` that CORE-MATH and GAOL took from the
C library, and GAOL takes them no more where they are wrong. They are those of
mingw-w64's own math library (`GAOL_MINGW_OWN_MATH`, which
`gaol/gaol_mingw_own_math.h` defines once for `gaol/gaol_config.h` and for the C
sources of CORE-MATH) in every program linked before mingw-w64 12, and from 12 on
only in the library of `msvcrt.dll`, so that a toolchain linking the UCRT takes
those of `ucrtbase.dll`, which pass the tests. `gaol/gaol_mingw_own_math.h`
tells the two runtimes apart by `_UCRT`, which the headers of a UCRT toolchain
define, with GCC and Clang alike. There, and for Cygwin, whose `fma()` is wrong
too:

- `round()` is `gaol_round()` (`gaol/math/gaol_round.h`), which reads the bits;
- `fma()` is the instruction, which `GAOL_FMA` gives GCC and Clang for Windows
  on a processor that has it: the three builds run a program compiled with
  `-mfma` and drop it where the processor does not run it, and the objects of
  the Release builds of the continuous integration are checked for the
  instructions (`.github/scripts/check-gcc-windows-fma.sh`);
- where the build has no instruction (`GAOL_FMA=OFF`, or a processor without the
  instructions, for which the builds do not take `-mfma`), `fma()` is
  `gaol_fma_upward()` (`gaol/math/gaol_fma.h`): `a*b + c` rounded once upward,
  the direction GAOL runs in, computed with integers (`GAOL_SOFT_FMA` of
  `gaol/gaol_config.h` is then 1); `fma_exact()`, which the `gaol_fma()` of
  GAOL's interface calls, is the `gaol_fma()` of that header, right in the four
  directions. The library calls no `fma()` of the C library in either case,
  which `.github/scripts/check-no-fma-call.sh` reads in its objects. Nothing
  changes where the library's `fma()` is right: Linux, macOS, Visual C++, the
  UCRT of mingw-w64 12 and later.

`gaol_fma_upward()`, which GAOL calls, is slower than the instruction: it is
there for the processors without it (before Intel Haswell and AMD Piledriver)
and for a library built for any processor of the architecture
(`-DGAOL_FMA=OFF`), which was refused before. It takes for granted the upward
direction that GAOL's code runs in, and has three fast paths on x86-64 (the
error of a product, `c` far above the product, and `c` of the size of the
product or below it), which `tan`, `log` and `pow(x, 3)` call most; `gaol_fma()` is right in the four directions, which the
tests that call CORE-MATH in another direction need: they are not run where
GAOL calls `gaol_fma_upward()`. `-DGAOL_FMA_SOFTWARE=ON` (`GAOL_FMA_FORCE_SOFTWARE=1` for the other two
builds) has GAOL call it on any machine, as the continuous integration does on
Ubuntu x86-64 with GCC and Clang.

`tests/core_math.cpp` checks the two functions first, on every platform: that of
the C library, and `gaol_round()`, `gaol_fma_upward()` and `gaol_fma()` where GAOL calls them. On
32-bit x86, mingw-w64 computes them on the x87 unit: there its `round()` is
right, and the `fma()` of mingw-w64 11 (MinGW-Builds GCC 12 and 13) keeps its
partial sums in extended precision and passes the tests, though it is not
correctly rounded (125 results of 400,000 were one double off); that of the
mingw-w64 9 of WinLibs, compiled without optimization, rounds each of them to a
double; that of a mingw-w64 12 or later linking `msvcrt.dll` (MSYS2 MINGW32) is
that of mingw-w64 too. GAOL calls none of them: the instruction, or
`gaol_fma_upward()`.

On 64-bit ARM, mingw-w64 before version 12 is accepted (GAOL v5): the `fma()`
of its `libmingwex` is not the code above but the instruction. In the
`libmingwex.a` of llvm-mingw 20230320 (Clang 16, mingw-w64 11) it is `fmadd d2,
d0, d1, d2`, a move and a return, which is the instruction that `__builtin_fma()`
of Clang is at every level of optimization; its `round()` is the code above,
computed in doubles. GAOL calls `gaol_round()` and the instruction, and
`gaol_fma_upward()` is for x86 only (`GAOL_SOFT_FMA` is 0), a processor for 64-bit ARM
having the instruction. That is read in the code, not taken from a comment:
`.github/scripts/check-arm64-fma.sh` reads the disassembly of the `fma()` of
the library and requires one `fmadd` and nothing else that computes, requires
fused instructions in the objects of the library, and fails on an object that
calls `fma()` or `round()`; `tests/core_math.cpp` calls the `fma()` of the
library as a function, through a pointer read at run time, against the exact
values of `tests/fma_values.py` in the four rounding directions, which a
product and a sum each rounded fails (the error-free products, the halfway
cases, the cancellations); and a job of `windows.yml` builds and tests GAOL with
that toolchain on the arm64 runners. The releases of llvm-mingw from June 2023
on call themselves mingw-w64 12, and the `libmingwex` of 20231128 and 20240130
has neither `fma.o` nor `round.o`: they take those of `ucrtbase.dll`. A toolchain
for 32-bit ARM, and one for 64-bit ARM of version 12 or later linking
`msvcrt.dll`, which no job builds, stay refused. The refusal is tested with the
macros of such a toolchain defined by hand on an x86 system
(`tests/refused_mingw_arm.cpp`): that checks the logic of the refusal, not a
real toolchain for ARM.

### Linking with `-Ofast`, `-ffast-math` or `-funsafe-math-optimizations`

The compilation of GAOL's headers refuses `-ffast-math`, but the link of the
program is not seen: GCC and Clang link `crtfastmath.o` into a program linked
with `-Ofast`, `-ffast-math` or `-funsafe-math-optimizations`, on x86 and on
ARM Linux, whose constructor sets the modes that flush the subnormal numbers
to zero, flush-to-zero and denormals-are-zero of the SSE instructions, FZ of
ARM, and loading a plug-in or a Python module built with `-Ofast` sets them
too (built by Clang 18, or by GCC before 13). With one of them, every
operation with a subnormal operand or result gives bounds that miss the exact
result: `[1e-300] * [1e-20]` is [0, 0]. The `-fno-fast-math` of `gaol.pc` and
`gaol::gaol` does not prevent it: it keeps `crtfastmath.o` out of the link
when it comes after `-ffast-math`, but not after `-Ofast` (GCC 9.4 and 13 and
Clang 18 link it all the same, and `-fno-fast-math` silences the `#error` of
`gaol/gaol_config.h` against `-ffast-math`, which `-Ofast` would raise) nor,
with GCC 9.4 and 13, after `-funsafe-math-optimizations`.

**Linking with them, or loading code built with them, makes the bounds wrong.**
GAOL v5 defends itself:

- on x86 processors, and on ARM processors with GCC and Clang and with the NEON
  intervals of Visual C++, each operation
  that computes bounds clears the modes when it starts, in the check that sets
  the rounding direction upward, which uses a subnormal number to see them as
  well as the direction (`round_upward_if_needed()` in `gaol/gaol_fpu.h`).
  Each operation makes it before it reads, compares or copies a bound, the
  test of the empty set aside, the compiler being kept from moving a
  comparison above it (a compiler barrier after the modes are cleared, and an
  empty asm statement for a double passed by value), and with
  `GAOL_PRESERVE_ROUNDING` makes its result before it sets the modes back. It
  is the only defence against the plug-in, and against the compilers without
  `-mno-daz-ftz` (see
  [Using GAOL](using.md#flush-to-zero-and-denormals-are-zero) for its cost, and
  for the functions that make no check);
- `-mno-daz-ftz` is given to the link (`gaol::gaol`, `gaol.pc`, the tests)
  where the compiler accepts it, which keeps `crtfastmath.o` out of it. Each
  build checks the compiler with a link, as the other flags are checked:
  `check_cxx_source_compiles()` with `CMAKE_REQUIRED_LINK_OPTIONS`,
  `AC_LINK_IFELSE` with `-Werror` and the `LDFLAGS` of the user,
  `has_link_argument()`. `gaol::gaol` gives it to a program built by GCC 13 or
  a later one, by Clang 20 or a later one (not on Windows, where the option was
  not tried with Clang: the compiler that built GAOL alone is trusted with it),
  or by the compiler that built
  GAOL, of its major version and not older: GCC has the option from 11.4 and
  12.4 in the series 11 and 12 only, and a GAOL built by GCC 11.4 gave it to
  GCC 12.3, which stops on it; a GAOL built by GCC gave it to no program built
  by Clang, which has it from 20 (GAOL v5).
  `gaol.pc`, written for that compiler, gives it to every
  program, and a program linked by a compiler that refuses it (Clang 18, a GCC
  before 11.4, GCC 12.0 to 12.3, GCC for ARM) is to be linked without it.

The modes of other processors, and of ARM with the FPU intervals of Visual C++,
are neither checked nor cleared; GCC links `crtfastmath.o` for none of the other processors of the
continuous integration (POWER, s390x, RISC-V). `tests/rounding_direction.cpp`
sets each mode before operations with subnormal operands and results, and
checks that their bounds are the tightest ones, and that every operation that
checks the rounding direction gives, with a mode set just before it, what it
gives with the modes cleared (x86, and ARM with GCC and Clang, and with the NEON
intervals of Visual C++), on what each
compiler emits; `tests/fast_math_link.cpp`, linked with `-ffast-math`, checks that the
modes are clear when `main()` starts where the build gives `-mno-daz-ftz`, and
that the operations clear them otherwise.
