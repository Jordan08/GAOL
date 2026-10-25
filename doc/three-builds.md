<!-- Copyright (c) 2026 ENSTA, France
     Created 2026-09-20 by Jordan NININ -->
# The three builds

Part of the documentation of [GAOL v5](../README.md#documentation).

On a given machine with a given compiler, GAOL behaves the same whichever
build configured it: CMake, configure and meson give it the same macros and the
same flags. The reference is the autotools build of GAOL, then its meson
build, and the CMake build follows them, apart from the errors corrected (see
[What differs from GAOL](differences.md)). In every build, by default:

- GAOL is compiled in release: `-O3` and the optimizations configure adds
  (`-funroll-loops -fomit-frame-pointer -fexpensive-optimizations`, each where
  the compiler takes it), `NDEBUG`, `-std=c++11`, hidden visibility
  (`-fvisibility=hidden -fvisibility-inlines-hidden`) and `-Wall -Wconversion`,
  which GAOL compiles without warnings, `-Wsign-conversion` of Clang included.
  Its libraries are compiled in release too where a project brings GAOL in
  without choosing a build type, as by a configure calling GAOL's: a CMake
  project without `CMAKE_BUILD_TYPE`, with a generator of one configuration
  (with a generator of several, as Visual Studio, Release comes first among
  the configurations of GAOL built alone, and a project bringing GAOL in
  chooses its configuration), a meson project in the default build type of
  meson, with its ninja backend (GAOL v5);
- with the flags of interval arithmetic of [Using GAOL](using.md), and the code
  using GAOL, the tests included, is linked with `-mno-daz-ftz` where the
  compiler accepts it (GCC 13 and later on x86, and from 11.4 and 12.4 in the
  series 11 and 12), in `gaol::gaol` and in `gaol.pc`: each build tests the
  compiler with a link (GAOL v5);
- on x86 processors, the intervals are computed with SSE2 instructions
  (`-msse2 -msse3`; no flag with Visual C++ and clang-cl for x64, where SSE2 is
  the baseline of the processor). On 32-bit Windows the C runtime aligns the
  memory of `new` on 8 bytes, and an SSE2 interval needs 16: GCC took it to be
  aligned on 16, and a `std::vector` of SSE2 intervals crashed. GCC and Clang
  (MinGW-w64, MSYS2) are told what the runtime gives with `-faligned-new=8`, so
  that they ask for aligned memory for the intervals and for what holds one, and
  `gaol::gaol` and `gaol.pc` give the flag to the code using GAOL too, as it
  allocates them; `gaol/gaol_config.h` refuses the SSE2 intervals compiled
  without it. Visual C++ and clang-cl have no such flag, and keep the FPU
  intervals there (GAOL v5: Visual C++ x64 and clang-cl x64 take the SSE2
  intervals);
- on 64-bit ARM processors, the intervals are computed with NEON instructions
  (`GAOL_ARM64`, `--enable-arm64`, `-Denable-arm64`; no flag), on by default
  with every compiler of them: GCC, Clang and AppleClang under Linux and macOS,
  clang-cl and the Clang of MinGW-w64 and MSYS2, and Visual C++ (`cl`, whose
  registers are those of `_ReadStatusReg()` and `_WriteStatusReg()`; GAOL v5),
  whose compiler each build tests with the same program (CMake and meson for
  Visual C++, which configure does not build with): the intrinsics of
  `<arm_neon.h>` and the registers of the rounding mode. Not with 32-bit ARM,
  whose NEON has no vector of doubles, nor in a universal build of macOS: the
  option is then ignored, and
  the FPU computes the intervals, as before. Independent of `GAOL_SIMD`, which
  is for x86 processors (GAOL v5);
- GAOL and CORE-MATH are compiled with the fused multiply-add
  instructions of the processor, where the compiler has a flag for them
  (`GAOL_FMA`, `--enable-fma`, `-Denable-fma`): `-mfma` on x86; with GCC for
  Windows, on x86-64 and on 32-bit x86, `-mfma -mprefer-vector-width=128`
  (GAOL v5, see [GCC for Windows](#gcc-for-windows) below);
  `/arch:AVX2` with Visual C++ for x64, not for 32-bit x86, where it broke
  the rounding direction; `-mfpu=neon-vfpv4 -mfloat-abi=hard` on 32-bit ARM;
  and only where the machine building runs a program so compiled, but when
  cross-compiling: under Rosetta on macOS 14, which has no AVX, the x86_64
  tests stopped on an illegal instruction.
  GAOL's exact products (`fma_once()`) and CORE-MATH's functions then compute
  `fma()` with one instruction rather than with a call to the math library: on
  an Intel i7-1185G7 with Clang 21.1, `pow(x, 3)` of an interval takes 5.4 ns
  rather than 17.4, `sin` 1.3 times less and `cos` 2.5 times less, measured in
  [What the options are worth](#what-the-options-are-worth). The code using GAOL is given the flag
  too (`gaol::gaol`, `gaol.pc`), and compiled for the same processor.
  `-ffp-contract=off` stays, which forbids the compiler to contract a
  multiplication and an addition into a fused one, as CORE-MATH asks;
- with the AVX-512 path of `x + y`, `x - y`, `x * y`, `x / y`, `sqrt(x)`,
  `sqr(x)` and, with FMA, the integer powers beyond 1 and -1, asked for by
  `GAOL_PREFER_AVX512` (`--enable-prefer-avx512`,
  `-Denable-prefer-avx512=true`), on by default where the rounding direction
  is preserved, whose every operation the path relieves of the store of
  that direction, and taken (`GAOL_AVX512_ARITHMETIC`) where the machine
  building runs the instructions, which is checked as for the fused
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
  gave no version (GAOL v5).

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
GAOL is compiled with (`compare.py --check`); the continuous integration runs
it on each kind of machine.

## What the options are worth

Measured with `tests/performance.cpp` on an Intel i7-1185G7 with Clang 21.1,
in nanoseconds per operation, the best of nine runs, each program being run in
turn with the others (GAOL v5):

| | x + y | x × y | sqr(x) | pow(x, 3) | exp(x) | log(x) | sin(x) | cos(x) |
|---|---:|---:|---:|---:|---:|---:|---:|---:|
| Default (`GAOL_FMA` and `GAOL_SIMD` on) | 1.07 | 2.04 | 1.80 | 5.42 | 18.6 | 21.7 | 31.3 | 36.6 |
| `-DGAOL_FMA=OFF` | 1.14 | 2.06 | 1.81 | 17.4 | 21.6 | 34.5 | 41.4 | 90.3 |
| `-DGAOL_SIMD=OFF` | 2.30 | 4.32 | 2.46 | 7.65 | 19.9 | 23.6 | 32.8 | 39.0 |

- **The fused multiply-add instructions (`GAOL_FMA`) are worth much more than
  they were**: cos is **2.5 times faster** with them, pow(x, 3) 3.2 times, log
  1.6 times, sin 1.3 times and exp 1.2 times. CORE-MATH computes with
  `__builtin_fma` throughout, which is one instruction where the processor has
  it and a call to the math library where it has not. They are on by default wherever the compiler has a flag for them and the
  machine building runs a program compiled with it. They do nothing for the
  arithmetic, which does not multiply and add in one step.
- **The SSE2 intervals (`GAOL_SIMD`) pay on the products and the additions**:
  x × y takes 2.04 ns rather than 4.32, and x + y 1.07 rather than 2.30, the two
  bounds being computed in one instruction. They are on by default on x86
  processors. The elementary functions do not go through them.

On a 64-bit ARM processor, measured the same way on an Apple M2 with AppleClang
21.0, the median of three runs of each program (GAOL v5):

| | x + y | x − y | x × y | x ÷ y | sqr(x) | sqrt(x) | 13 operations in a line |
|---|---:|---:|---:|---:|---:|---:|---:|
| `-DGAOL_ARM64=OFF` (FPU intervals) | 1.18 | 1.18 | 3.33 | 3.30 | 1.82 | 4.60 | 51.7 |
| Default (NEON intervals) | 0.91 | 0.89 | 2.14 | 2.39 | 1.48 | 1.70 | 19.2 |
| `-DGAOL_ARM64=OFF -DGAOL_PRESERVE_ROUNDING=ON` | 38.8 | 38.7 | 44.4 | 45.2 | 43.7 | 48.5 | 537 |
| `-DGAOL_PRESERVE_ROUNDING=ON` (NEON intervals) | 26.9 | 27.3 | 29.3 | 34.0 | 27.3 | 34.3 | 377 |

- **The NEON intervals (`GAOL_ARM64`) pay on every operation of the table**:
  x − y takes 0.89 ns rather than 1.18, x × y 2.14 rather than 3.33 (of operands
  of random signs 2.18 rather than 3.38), x ÷ y 2.39 rather than 3.30, sqrt
  1.70 rather than 4.60, and a line of 13 operations 19.2 ns rather than 51.7
  (the FPU row is measured in a session of its own, and the NEON rows are those
  of #146).
  The two bounds are one vector, and the operations take them from one to the
  next in a register. The elementary functions, which CORE-MATH computes,
  go through them only for their arguments: no faster, no slower. They are on by
  default on 64-bit ARM processors with GCC, Clang or AppleClang.
- **The product, the quotient and the square of the FPU intervals take no
  branch on the signs of the bounds** (GAOL v5), and x − y reads its bounds
  after the check of the rounding direction. The row read before 3.56 for
  x − y, 3.37 for x × y (6.39 of operands of random signs), 4.00 for x ÷ y,
  2.08 for sqr(x) and 49.8 for the line of 13 operations. The product of a line
  of operands of one sign, which a processor predicts, takes more time than it
  did (a line of 7 products, 35.8 ns rather than 23.8), the largest of four
  products waiting for two comparisons. With `GAOL_PRESERVE_ROUNDING` the write
  of FPCR waits for the result as well, and the FPU row of that option gains
  nothing (x × y 44.4 ns rather than 42.2 for the former code, measured in the
  same session, and 44.3 rather than 49.1 for operands of random signs). These
  are the figures of this processor, with AppleClang. The processors of the
  continuous integration that run the FPU intervals (32-bit ARM, Visual C++ and
  clang-cl for x86, Ubuntu with GCC and Clang, macOS; and, when they were
  measured, Visual C++ and clang-cl for arm64, which have the NEON intervals
  now), measured in turn, give other ratios, losses included (the line of 7
  products 0.58 with GCC on Ubuntu, 0.57 with clang-cl for arm64); POWER,
  s390x and RISC-V are not measured. See
  [The FPU intervals](using.md#the-fpu-intervals).
- **The rounding direction is read with one `mrs` of FPCR** (0.8 ns on this
  processor) and written with `msr` only when the program's is not upward:
  `msr` takes 4 ns for the value FPCR holds and 11 ns when it changes the
  direction. With `GAOL_PRESERVE_ROUNDING` an operation of a program that
  computes to nearest writes it twice, which is the 27 ns above (39 ns with the
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
(`windows.yml`, GCC 14.4, CMake 4.4, Release, GAOL v5; not measured), whose
first runs found what Cygwin needed. Its `<stdlib.h>` does not declare
`posix_memalign()` under `-std=c++11` (the aligned memory is `_mm_malloc()`
there). Its `fma()` is not correctly rounded (`fma(1 + 2^-30, 1 - 2^-30, -1)`
gives 0), and `std::fma` is a call of it without optimization: GAOL calls
`__builtin_fma` (`fma_once()`), which is the instruction with `-mfma` at -O0 and
at -O3 (both checked), and refuses Cygwin without `-mfma`. Its
`nextafter(-DBL_MAX, -oo)` is `-DBL_MAX` in the rounding direction upward, which
`previous_float()` and `next_float()` now tell apart (the lower bound of
`sinh(-711)` did not enclose the value; `elementary` failed on it).
`is_bisectable()` and `bisect()` call no `nextafter()`: they tell the bounds
by their bits. A Debug
build of Cygwin is not in the job; `tests/core_math.cpp` and
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

Compiled with it (`-flto`), GAOL's basic operations are 1.1 to 1.5 times
faster where the code using GAOL is compiled with it too — an addition of
intervals took 0.73 ns rather than 1.07 with Clang 21.1, `x * y` 1.71 rather
than 2.04, `sqr(x)` 1.27 rather than 1.80 — and the elementary functions no
faster, a little **slower**: cos took 39.2 ns rather than 36.6, sin 32.6 rather
than 31.3, log 22.7 rather than 21.7. With Clang 18.1, before the operators
took their bounds in a register, the basic operations took two to three times
less time with it and the elementary functions two to three times more. The
sources of CORE-MATH are compiled apart, without it, and it reaches them
through the link all the same.

Worse, with **GCC** it breaks the bounds: GAOL then fails
`tests/rounding_direction.cpp`, which checks that an operation gives the same
result whatever the rounding direction the calling code left. With
`GAOL_PRESERVE_ROUNDING`, `textToInterval("sin(1)+exp(0.1)")` gave an upper bound
one double below the right one when the caller left the direction downward or
toward zero: the compiler moves floating-point operations across the changes of
rounding direction, which `-frounding-math` is meant to forbid and which it
does not do across translation units. Compiling GAOL's sources alone with it is
enough for the test to fail. Clang 18.1 passes the tests with it, so this is not
a property of the option but of one compiler's handling of it — which is reason
enough not to offer it.

An interval that does not enclose the exact value is worse than a slow one, so
there is no option for this, and `gaol.pc` and `gaol::gaol` carry no such flag.

## Compilers and options refused

Each of these gave bounds not enclosing the exact results, or worse. The three
builds refuse the compilers of the first two rows when configuring, the C
compiler, which builds CORE-MATH, as well as the C++ compiler, with a
message naming what to use instead, and keep the options of the last four away
by giving GAOL the flags of [Using GAOL](using.md). `gaol/gaol_config.h`
refuses them again at compile time, for the code including GAOL's headers too,
all but the second row, which no macro of the compiler shows; the third row it
alone refuses, when compiling:

| Refused | Because |
|---|---|
| Clang for 32-bit ARM processors | It does not honour the rounding direction there: built by Clang 21, 4556 of 16000 random products, squares and cubes did not enclose their exact values. GCC does. |
| A compiler saying of `-frounding-math` "overriding currently unsupported rounding mode on this target", as Clang 14 for 64-bit ARM | Bounds of `pow()` and `nth_root()` did not enclose the exact values. Clang 18 honours the rounding direction there. |
| MinGW-w64 whose `fma()` or `round()` is wrong: on x64, before mingw-w64 12 (GCC 11 to 13 of Chocolatey) and linked with `msvcrt.dll` rather than the UCRT (MSYS2 MINGW64, the cross compilers of Debian and Ubuntu); on 32-bit x86, before 11 (GCC 11 of WinLibs); on ARM, as on x64, before 12 or linked with `msvcrt.dll` (the refusal is not tested there; MSYS2 CLANGARM64, which links the UCRT, is built and tested) | On x64, the `fma()` and `round()` of mingw-w64's own math library, computed in doubles: that `fma()` is not correctly rounded (10.7 % of the error-free products `fma(a, b, -a*b)` wrong), and CORE-MATH computes with it wherever the compiler has no `-mfma` (`GAOL_FMA` off; GCC for Windows on x86-64 and on 32-bit x86 takes it by default, and these toolchains stay refused, their `round()` being wrong too); that `round()` depends on the rounding direction. On 32-bit x86, the `fma()` of the mingw-w64 9 of WinLibs rounds each of its partial sums to a double. On ARM, that `round()` too; GAOL v5 is not tested there. Bounds of `tan()`, `asin()`, `atan()` and others did not enclose the exact values. MinGW-w64 GCC 14 and 15 on x64, 12 to 15 on 32-bit x86 (GCC 13 also with `GAOL_FMA` off), and MSYS2 UCRT64 and CLANG64 are built and tested. |
| `-ffast-math`, `-Ofast`, `/fp:fast` | The compiler then rounds to nearest and drops the checks of NaN and infinities. `-Ofast` is not refused where `-fno-fast-math` turns its fast-math optimizations off (below). Linking with them, or with `-funsafe-math-optimizations`, is a matter of its own (below). |
| `-ffinite-math-only`, which `-ffast-math` and `-Ofast` turn on | The compiler then takes NaN and infinities never to occur, in GAOL's inline functions too: the empty interval has NaN bounds, and `([1, 2] & [3, 4]).is_empty()` is false (GCC 9.4 and 13 at `-O0`, `-O2` and `-O3`; Clang 18 at `-O0`, and at `-O2` and `-O3` when the bounds are `volatile`). |
| Visual C++, and clang-cl from Clang 16, without `/fp:strict` (`/fp:precise`, their default) | They then assume rounding to nearest, and may evaluate or rewrite floating-point operations accordingly (clang-cl compiles with `-fno-rounding-math -ffp-contract=on`; GAOL v5 did not refuse it at first): no test gave a wrong bound so, but nothing certifies the bounds (see [What differs from GAOL](differences.md)). |
| Cygwin without the fused multiply-add instructions (`GAOL_FMA=OFF`, or a processor without them) | **Bounds not certified.** The `fma()` of the C library of Cygwin is not correctly rounded: `fma(1 + 2^-30, 1 - 2^-30, -1)` gives 0 instead of -2^-60, and `std::fma` is a call of it without optimization. CORE-MATH computes with `fma()`. GAOL v5 has the instruction instead (`-mfma`, `__builtin_fma`), which every level of optimization keeps. | Refused by `gaol/gaol_config.h`; the Cygwin job of `windows.yml` checks the refusal, and builds and tests Cygwin with `-mfma` (GCC 14.4, Release, the tests passing but three skipped). |
| Doubles computed on the x87 unit of 32-bit x86 processors (without `-msse2 -mfpmath=sse`, or `/arch:SSE2`) | CORE-MATH assumes every operation on doubles rounded to a double. Computed in extended precision, its results rounded to nearest are rounded twice, and 175 arguments gave the other neighbour of the exact value with GCC 12 on Debian 12 i386; with GCC 9, which rounds to nearest at compile time the constants CORE-MATH rounds in the direction in effect, GAOL's bounds of `exp2(-1075)`, `expm1(-800)` or `atan2()` of a tiny and a huge number did not enclose the exact values. `tests/extended_precision.cpp` checks these arguments. |

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

No macro of the compiler shows the following, which `gaol/gaol_config.h`
cannot refuse and which give wrong results all the same: the code using GAOL
is not to be compiled with them.

- `-funsafe-math-optimizations`, and `-ffast-math -fno-finite-math-only`, with
  GCC: the compiler rewrites the addition `1.0 + tiny == 1.0`, by which GAOL
  sees the rounding direction, as `tiny == 0.0` (GCC 9.4), and
  `1.0 + (subnormal + 0.0) == 1.0`, which sees the modes flushing the
  subnormals to zero too, as `subnormal == 0.0` (GCC 9.4 from `-O0`, and
  GCC 13), so that an operation does not set the direction upward again
  after the code using GAOL left it to nearest, and `width()` is below the
  exact width. Clang 18 does not rewrite the first addition, but its
  `-funsafe-math-optimizations` implies `-fno-signed-zeros` (below): at `-O2`
  and `-O3`, the second addition loses its `+ 0.0` there too, one `addsd` and
  one `ucomisd` being left.
- `-fno-signed-zeros`, with which GCC (9.4 from `-O0`, and 13) and Clang 18
  (at `-O2` and `-O3`) drop the `+ 0.0` of the second addition, which then
  misses the flush-to-zero mode (GAOL v5).
- `-fno-honor-nans` alone, with Clang, which does to the empty interval what
  `-ffinite-math-only` does: `__FINITE_MATH_ONLY__` is 1 only with
  `-fno-honor-infinities` too.
- `-ffast-math` or `-Ofast` followed by `-fno-fast-math`, after the flags of
  interval arithmetic, with Clang (18): the macros show nothing, but
  `-frounding-math` stays off, and the compiler takes the rounding direction
  to be to nearest. `tests/rounding_direction.cpp`, compiled so, failed 54 of
  its 292728 checks after `-ffast-math -fno-fast-math` (operations with an
  empty operand raised the invalid-operation flag), and 126 after
  `-Ofast -fno-fast-math` (bounds with subnormal results under flush-to-zero
  too); with the flags of interval arithmetic given again after the option,
  none.

The probe of the modes is `!(1.0 + (subnormal + 0.0) > 1.0)` since point ZZ of
`TODO.md`, no longer `== 1.0`: with `-funsafe-math-optimizations`, GCC 15 does
not rewrite it as `!(subnormal > 0.0)`, and with `-fno-signed-zeros` GCC 15 and
Clang 21 drop its `+ 0.0` as they did that of the earlier form (GCC 9 to 14
were not tried with it). The code using GAOL is still not to be compiled with
these flags.

Two reasons that had GAOL refuse MinGW-w64 are gone with CORE-MATH (GAOL v5):
the math library of mingw-w64 older than version 12 gave `acosh()` near 1 up
to 25 million doubles away from the exact value, and the `fesetround()` of
mingw-w64 12 runs the instruction `cpuid` at each call. GAOL takes no
elementary function from the math library of the system any more, and no
longer changes the rounding direction for its elementary functions: the older
MinGW-w64 are refused for the reason of the third row only, the `fma()` and
`round()` that CORE-MATH and GAOL still take from the C library. Before
mingw-w64 12 they are those of mingw-w64 in every program; from 12 on, only in
the library of `msvcrt.dll`, so that a toolchain linking the UCRT takes those
of `ucrtbase.dll`, which pass the tests. `gaol/gaol_config.h` tells the two
runtimes apart by `_UCRT`, which the headers of a UCRT toolchain define, with
GCC and Clang alike, and `tests/core_math.cpp` checks the two functions first,
on every platform. On 32-bit x86, mingw-w64 computes them on the x87 unit:
there its `round()` is right, and the `fma()` of mingw-w64 11 (MinGW-Builds
GCC 12 and 13), accepted and called only with `GAOL_FMA` off (the default gives
GCC for Windows `-mfma` there too), keeps its partial sums in extended
precision and passes the tests, though it is not correctly rounded; that of the mingw-w64 9
of WinLibs, compiled without optimization, rounds each of them to a double.

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
  a later one, or by the compiler that built GAOL, of its major version and not
  older: GCC has the option from 11.4 and 12.4 in the series 11 and 12 only,
  and a GAOL built by GCC 11.4 gave it to GCC 12.3, which stops on it (GAOL v5,
  review of point 4). `gaol.pc`, written for that compiler, gives it to every
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
