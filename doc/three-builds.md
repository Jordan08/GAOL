<!-- Copyright (c) 2026 ENSTA, France
     Created 2026-09-20 by Jordan NININ -->
# The three builds

Part of the documentation of [this fork of GAOL](../README.md#documentation).

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
- with the flags of interval arithmetic of [Using GAOL](using.md);
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
  an Intel i7-1185G7, `pow(x, 3)` of an interval took 21 ns rather than 29, and
  `sin` and `cos` 9 % less with GCC 9.4. The code using GAOL is given the flag
  too (`gaol::gaol`, `gaol.pc`), and compiled for the same processor.
  `-ffp-contract=off` stays, which forbids the compiler to contract a
  multiplication and an addition into a fused one, as CORE-MATH asks and as
  mathlib needed before it (compiled by GCC with `-mfma` and the contraction
  allowed, mathlib gave 3.8 million of 30 million results differently, nearly
  half of those of sin, cos, tan and cot);
- without the intervals of floats, `gaol::intervalf` and `gaol::interval2f`
  (SSE3), unfinished, which `GAOL_FLOAT_INTERVALS`
  (`--enable-float-intervals`, `-Denable-float-intervals=true`) compiles;
- with exceptions, the "certainly" relations, GAOL's assembly
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

## What the options are worth

Measured with `tests/performance.cpp` on an Intel i7-1185G7 with GCC 9, in
nanoseconds per operation (fork of GAOL):

| | x + y | sqr(x) | pow(x, 3) | exp(x) | log(x) | sin(x) | cos(x) |
|---|---:|---:|---:|---:|---:|---:|---:|
| Default (`GAOL_FMA` and `GAOL_SIMD` on) | 3.02 | 3.31 | 15.7 | 28.7 | 33.1 | 59.0 | 59.1 |
| `-DGAOL_FMA=OFF` | 3.03 | 3.58 | 25.5 | 37.9 | 52.5 | 143.7 | 142.0 |
| `-DGAOL_SIMD=OFF` | 3.30 | 3.46 | 15.9 | 28.2 | 32.8 | 59.6 | 60.3 |
| Default, whole build with interprocedural optimization | 1.48 | 2.28 | 12.0 | 26.4 | 33.0 | 57.9 | 60.8 |

- **The fused multiply-add instructions (`GAOL_FMA`) are worth much more than
  they were**: sin and cos are **2.4 times faster** with them, log 1.5 times and
  exp 1.3 times. CORE-MATH computes with `__builtin_fma` throughout, which is
  one instruction where the processor has it and a call to the math library
  where it has not; with mathlib the gain was 9 %. They are on by default
  wherever the compiler has a flag for them and the machine building runs a
  program compiled with it.
- **The SSE2 intervals (`GAOL_SIMD`) pay on the additions and the products**
  (x + y 3.02 ns rather than 3.30), and cost a little on the division (6.4
  rather than 5.8). They are on by default on x86 processors. The elementary
  functions do not go through them.
- **Interprocedural optimization is not offered, because it breaks the
  bounds.** Compiled with it (`-flto`), GAOL's basic operations are about twice
  as fast where the code using GAOL is compiled with it too — an addition of
  intervals took 1.48 ns rather than 3.02, a subtraction 1.35 rather than 3.11,
  `sqr(x)` 2.28 rather than 3.32 at `-O3` — and GAOL then **fails
  `tests/rounding_direction.cpp`**: with `GAOL_PRESERVE_ROUNDING`,
  `interval("sin(1)+exp(0.1)")` gave an upper bound one double below the right
  one when the calling code left the rounding direction downward or toward
  zero. The compiler moves floating-point operations across the changes of
  rounding direction, which `-frounding-math` is meant to forbid and which it
  does not do across translation units; compiling GAOL's sources alone with it
  is enough for the test to fail. An interval that does not enclose the exact
  value is worse than a slow one, so there is no option for this, and `gaol.pc`
  and `gaol::gaol` carry no such flag. The sources of CORE-MATH are compiled
  apart all the same, which keeps a `-flto` given by hand away from them.

## Compilers and options refused

Each of these gave bounds not enclosing the exact results, or worse. The three
builds refuse the compilers of the first four rows when configuring, with a
message naming what to use instead, and keep the options of the last three away
by giving GAOL the flags of [Using GAOL](using.md). `gaol/gaol_config.h`
refuses them again at compile time, for the code including GAOL's headers too,
all but the second row, which no macro of the compiler shows:

| Refused | Because |
|---|---|
| Clang for 32-bit ARM processors | It does not honour the rounding direction there: built by Clang 21, 4556 of 16000 random products, squares and cubes did not enclose their exact values. GCC does. |
| A compiler saying of `-frounding-math` "overriding currently unsupported rounding mode on this target", as Clang 14 for 64-bit ARM | Bounds of `pow()` and `nth_root()` did not enclose the exact values. Clang 18 honours the rounding direction there. |
| `-ffast-math`, `-Ofast`, `/fp:fast` | The compiler then rounds to nearest and drops the checks of NaN and infinities. |
| Visual C++ without `/fp:strict` (`/fp:precise`, its default) | Visual C++ then assumes rounding to nearest, and may evaluate or rewrite floating-point operations accordingly: no test gave a wrong bound so, but nothing certifies the bounds (see [What differs from GAOL](differences.md)). |
| Doubles computed on the x87 unit of 32-bit x86 processors (without `-msse2 -mfpmath=sse`, or `/arch:SSE2`) | In extended precision, GAOL's bounds are wrong: built for an i686 computing on the x87, `exp`, `sin` and `cos` missed the exact value for most arguments. |

Two rows are gone with mathlib (fork of GAOL): mingw-w64 older than version 12,
whose math library gave `acosh()` near 1 up to 25 million floats away from the
exact value, and mingw-w64 12, whose `fesetround()` runs the instruction
`cpuid` at each call. GAOL takes no function from the math library of the
system any more, and no longer changes the rounding direction for its
elementary functions: MinGW-w64 11 to 15 are built and tested again. A mathlib
found installed was refused when its cosine of 2^52 − 1 was wrong, which no
longer applies either: there is no mathlib to find.
