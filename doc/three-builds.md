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
  which GAOL compiles without warnings, `-Wsign-conversion` of Clang included;
- with the flags of interval arithmetic of [Using GAOL](using.md), and the code
  using GAOL, the tests included, is linked with `-mno-daz-ftz` where the
  compiler accepts it (GCC 13 and later on x86, and from 11.4 and 12.4 in the
  series 11 and 12), in `gaol::gaol` and in `gaol.pc`: each build tests the
  compiler with a link (GAOL v5);
- on x86 processors, the intervals are computed with SSE2 instructions
  (`-msse2 -msse3`), except with Visual C++ and on 32-bit Windows, where a
  `std::vector` of SSE2 intervals crashes: GCC takes the memory of `new` to be
  aligned on 16 bytes there, while the C runtime aligns it on 8;
- GAOL and CORE-MATH are compiled with the fused multiply-add
  instructions of the processor, where the compiler has a flag for them
  (`GAOL_FMA`, `--enable-fma`, `-Denable-fma`): `-mfma` on x86, except with GCC
  for Windows, which does not align the stack for the AVX that `-mfma` turns
  on; `/arch:AVX2` with Visual C++ for x64, not for 32-bit x86, where it broke
  the rounding direction; `-mfpu=neon-vfpv4 -mfloat-abi=hard` on 32-bit ARM;
  and only where the machine building runs a program so compiled, but when
  cross-compiling: under Rosetta on macOS 14, which has no AVX, the x86_64
  tests stopped on an illegal instruction.
  GAOL's exact products (`std::fma()`) and CORE-MATH's functions then compute
  `fma()` with one instruction rather than with a call to the math library: on
  an Intel i7-1185G7 with Clang 18.1, `pow(x, 3)` of an interval took 14.5 ns
  rather than 21.1, and `sin` and `cos` 2.4 times less, measured in
  [What the options are worth](#what-the-options-are-worth). The code using GAOL is given the flag
  too (`gaol::gaol`, `gaol.pc`), and compiled for the same processor.
  `-ffp-contract=off` stays, which forbids the compiler to contract a
  multiplication and an addition into a fused one, as CORE-MATH asks;
- with the AVX-512 path of `x + y`, `x - y`, `x * y`, `x / y` and `sqrt(x)`
  compiled into the library beside the SSE2 one, taken at run time by
  `gaol::init()` with `GAOL_PREFER_AVX512`
  (`--enable-prefer-avx512`, `-Denable-prefer-avx512=true`) on a processor
  that has the instructions: on by default where the rounding direction is
  preserved, whose every operation the path relieves of the store of that
  direction; no flag to compile, the path's functions alone
  carry the target attribute, and the library runs on any processor of the
  architecture (GAOL v5, see [Using GAOL](using.md#the-avx-512-path));
- without the intervals of floats, `gaol::intervalf` and `gaol::interval2f`
  (SSE3), unfinished, for which no build has an option: only a developer of
  GAOL compiles them, defining `GAOL_FLOAT_INTERVALS` in `gaol/gaol_config.h`;
- with exceptions, GAOL's assembly (`GAOL_USING_ASM`), the rounding direction left upward, and silent: no line
  on the standard error when GAOL initializes and cleans up, unless
  `GAOL_VERBOSE_MODE` (`--enable-verbose-mode`, `-Denable-verbose-mode=true`)
  is asked for, where configure wrote it by default;
- the processor and the system (`GAOL_IX86_LINUX`, `GAOL_AARCH64_LINUX`...),
  the sizes of the integer types and the byte order are read from the macros
  of the compiler, in `gaol/gaol_config.h`, rather than from the machine
  building;
- the three write the same macros into `gaol/gaol_configuration.h`, those
  GAOL's sources read and nothing else (see
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
optimization nor `NDEBUG`, and checks its assertions (`GAOL_DEBUGGING`). It was
`configure --enable-debug` alone that defined `GAOL_DEBUGGING`, adding `-g
-ansi -Weffc++ -pedantic` for `g++` alone to the optimizations of
`--enable-optimize`; the Debug build of CMake did not define it, and meson
defined it with its option `enable-debug`, now gone (GAOL v5).

`.github/audit/` configures the three builds and compares the macros GAOL
sees, the macros each writes into `gaol/gaol_configuration.h`, and the flags
GAOL is compiled with (`compare.py --check`); the continuous integration runs
it on each kind of machine.

## What the options are worth

Measured with `tests/performance.cpp` on an Intel i7-1185G7 with Clang 18.1,
in nanoseconds per operation, the best of five runs (GAOL v5):

| | x + y | x × y | sqr(x) | pow(x, 3) | exp(x) | log(x) | sin(x) | cos(x) |
|---|---:|---:|---:|---:|---:|---:|---:|---:|
| Default (`GAOL_FMA` and `GAOL_SIMD` on) | 2.68 | 3.87 | 2.61 | 14.5 | 29.3 | 32.9 | 59.4 | 58.0 |
| `-DGAOL_FMA=OFF` | 2.54 | 3.92 | 2.94 | 21.1 | 37.9 | 52.6 | 143.4 | 141.4 |
| `-DGAOL_SIMD=OFF` | 3.15 | 6.27 | 2.85 | 13.9 | 29.2 | 32.3 | 61.1 | 61.3 |

- **The fused multiply-add instructions (`GAOL_FMA`) are worth much more than
  they were**: sin and cos are **2.4 times faster** with them, log 1.6 times,
  pow(x, 3) 1.5 times and exp 1.3 times. CORE-MATH computes with
  `__builtin_fma` throughout, which is one instruction where the processor has
  it and a call to the math library where it has not. They are on by default wherever the compiler has a flag for them and the
  machine building runs a program compiled with it. They do nothing for the
  arithmetic, which does not multiply and add in one step.
- **The SSE2 intervals (`GAOL_SIMD`) pay on the products and the additions**:
  x × y takes 3.87 ns rather than 6.27, and x + y 2.68 rather than 3.15, the two
  bounds being computed in one instruction. They are on by default on x86
  processors. The elementary functions do not go through them.

### Interprocedural optimization is not offered

Compiled with it (`-flto`), GAOL's basic operations are two to three times
faster where the code using GAOL is compiled with it too — an addition of
intervals took 0.99 ns rather than 2.68 with Clang 18.1, `sqr(x)` 1.96 rather
than 2.61 — and the elementary functions are two to three times **slower**: cos
took 169 ns rather than 58, sin 147 rather than 59, log 61 rather than 33. The
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
builds refuse the compilers of the first two rows when configuring, with a
message naming what to use instead, and keep the options of the last four away
by giving GAOL the flags of [Using GAOL](using.md). `gaol/gaol_config.h`
refuses them again at compile time, for the code including GAOL's headers too,
all but the second row, which no macro of the compiler shows; the third row it
alone refuses, when compiling:

| Refused | Because |
|---|---|
| Clang for 32-bit ARM processors | It does not honour the rounding direction there: built by Clang 21, 4556 of 16000 random products, squares and cubes did not enclose their exact values. GCC does. |
| A compiler saying of `-frounding-math` "overriding currently unsupported rounding mode on this target", as Clang 14 for 64-bit ARM | Bounds of `pow()` and `nth_root()` did not enclose the exact values. Clang 18 honours the rounding direction there. |
| MinGW-w64 whose `fma()` or `round()` is wrong: on x64, before mingw-w64 12 (GCC 11 to 13 of Chocolatey) and linked with `msvcrt.dll` rather than the UCRT (MSYS2 MINGW64, the cross compilers of Debian and Ubuntu); on 32-bit x86, before 11 (GCC 11 of WinLibs); on ARM, before 11 (not tested) | On x64, the `fma()` and `round()` of mingw-w64's own math library, computed in doubles: that `fma()` is not correctly rounded (10.7 % of the error-free products `fma(a, b, -a*b)` wrong), and CORE-MATH computes with it; that `round()` depends on the rounding direction. On 32-bit x86, the `fma()` of the mingw-w64 9 of WinLibs rounds each of its partial sums to a double. Bounds of `tan()`, `asin()`, `atan()` and others did not enclose the exact values. MinGW-w64 GCC 14 and 15 on x64, 12 to 15 on 32-bit x86, and MSYS2 UCRT64 and CLANG64 are built and tested. |
| `-ffast-math`, `-Ofast`, `/fp:fast` | The compiler then rounds to nearest and drops the checks of NaN and infinities. Linking with them, or with `-funsafe-math-optimizations`, is a matter of its own (below). |
| `-ffinite-math-only`, which `-ffast-math` and `-Ofast` turn on | The compiler then takes NaN and infinities never to occur, in GAOL's inline functions too: the empty interval has NaN bounds, and `([1, 2] & [3, 4]).is_empty()` is false (GCC 9.4 and 13 at `-O0`, `-O2` and `-O3`; Clang 18 at `-O0`, and at `-O2` and `-O3` when the bounds are `volatile`). |
| Visual C++, and clang-cl from Clang 16, without `/fp:strict` (`/fp:precise`, their default) | They then assume rounding to nearest, and may evaluate or rewrite floating-point operations accordingly (clang-cl compiles with `-fno-rounding-math -ffp-contract=on`; GAOL v5 did not refuse it at first): no test gave a wrong bound so, but nothing certifies the bounds (see [What differs from GAOL](differences.md)). |
| Doubles computed on the x87 unit of 32-bit x86 processors (without `-msse2 -mfpmath=sse`, or `/arch:SSE2`) | CORE-MATH assumes every operation on doubles rounded to a double. Computed in extended precision, its results rounded to nearest are rounded twice, and 175 arguments gave the other neighbour of the exact value with GCC 12 on Debian 12 i386; with GCC 9, which rounds to nearest at compile time the constants CORE-MATH rounds in the direction in effect, GAOL's bounds of `exp2(-1075)`, `expm1(-800)` or `atan2()` of a tiny and a huge number did not enclose the exact values. `tests/extended_precision.cpp` checks these arguments. |

`-fno-fast-math`, one of the flags of interval arithmetic, turns `-ffast-math`
and `-ffinite-math-only` off when it comes after them on the command line, and
not when it comes before them: `gaol.pc` and `gaol::gaol` give it, and the code
including GAOL's headers is refused when `-ffast-math` or `-ffinite-math-only`
comes after `-fno-fast-math`. The CMake tests `refused_finite_math_only` and
`refused_fast_math` compile `tests/refused_options.cpp` with each of the two
options, after the flags of interval arithmetic, and check that
`gaol/gaol_config.h` refuses it, and `refused_positive` that it compiles
without them (GCC and Clang); `tests/refused_options.sh` does the same in the
autotools and meson builds.

No macro of the compiler shows the following, which `gaol/gaol_config.h`
cannot refuse and which give wrong results all the same: the code using GAOL
is not to be compiled with them.

- `-funsafe-math-optimizations`, and `-ffast-math -fno-finite-math-only`, with
  GCC (9.4): the compiler rewrites the addition `1.0 + tiny == 1.0`, by which
  GAOL sees the rounding direction, as `tiny == 0.0` (and, with GCC 13,
  `1.0 + (subnormal + 0.0) == 1.0`, which sees the modes flushing the
  subnormals to zero too, as `subnormal == 0.0`), so that an operation does
  not set the direction upward again after the code using GAOL left it to
  nearest, and `width()` is below the exact width. Clang 18 does not rewrite
  the first addition, but its `-funsafe-math-optimizations` implies
  `-fno-signed-zeros` (below): the second addition loses its `+ 0.0` there
  too, one `addsd` and one `ucomisd` being left.
- `-fno-signed-zeros`, with which GCC 13 and Clang 18 drop the `+ 0.0` of the
  second addition, which then misses the flush-to-zero mode (GAOL v5).
- `-fno-honor-nans` alone, with Clang, which does to the empty interval what
  `-ffinite-math-only` does: `__FINITE_MATH_ONLY__` is 1 only with
  `-fno-honor-infinities` too.

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
GCC 12 and 13), accepted, keeps its partial sums in extended precision and
passes the tests, though it is not correctly rounded; that of the mingw-w64 9
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
`gaol::gaol` does not prevent it: it cancels `-ffast-math` when it comes after
it, but not `-Ofast` (GCC 13 and Clang 18 link `crtfastmath.o` all the same,
and it silences the `#error` of `gaol/gaol_config.h` against `-ffast-math`,
which `-Ofast` would raise) nor, with GCC 13, `-funsafe-math-optimizations`.

**Linking with them, or loading code built with them, makes the bounds wrong.**
GAOL v5 defends itself:

- on x86 processors, and on ARM processors with GCC and Clang, each operation
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

The modes of other processors, and of ARM with Visual C++, are neither checked
nor cleared; GCC links `crtfastmath.o` for none of the other processors of the
continuous integration (POWER, s390x, RISC-V). `tests/rounding_direction.cpp`
sets each mode before operations with subnormal operands and results, and
checks that their bounds are the tightest ones, and that every operation that
checks the rounding direction gives, with a mode set just before it, what it
gives with the modes cleared (x86, and ARM with GCC and Clang), on what each
compiler emits; `tests/fast_math_link.cpp`, linked with `-ffast-math`, checks that the
modes are clear when `main()` starts where the build gives `-mno-daz-ftz`, and
that the operations clear them otherwise.
