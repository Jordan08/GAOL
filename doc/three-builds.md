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
- on x86 processors, the intervals are computed with SSE2 instructions and
  `gaol::interval2f` with SSE3, except with Visual C++ and on 32-bit Windows,
  where a `std::vector` of SSE2 intervals crashes: GCC takes the memory of
  `new` to be aligned on 16 bytes there, while the C runtime aligns it on 8;
- with mathlib, exceptions, the "certainly" relations, GAOL's assembly
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
| mingw-w64 older than version 12 (MinGW-w64 GCC 11 to 13) | Its math library gave `acosh()` near 1 up to 25 million floats away from the exact value, and `asinh()` of large negative numbers NaN. |
| mingw-w64 12 (MinGW-w64 GCC 14.2, rt_v12) | Its `fesetround()` runs the instruction `cpuid` at each call: in a virtual machine, `exp()`, `log()`, `sin()` and `cos()` took 10.6 to 13.8 microseconds rather than 0.5 to 0.7 with mingw-w64 13 (MinGW-w64 GCC 15.2, MSYS2). |
| `-ffast-math`, `-Ofast`, `/fp:fast` | The compiler then rounds to nearest and drops the checks of NaN and infinities. |
| Visual C++ without `/fp:strict` (`/fp:precise`, its default) | Visual C++ then assumes rounding to nearest, and may evaluate or rewrite floating-point operations accordingly: no test gave a wrong bound so, but nothing certifies the bounds (see [What differs from GAOL](differences.md)). |
| Doubles computed on the x87 unit of 32-bit x86 processors (without `-msse2 -mfpmath=sse`, or `/arch:SSE2`) | In extended precision, GAOL's bounds and mathlib's results are wrong: built for an i686 computing on the x87, `exp`, `sin` and `cos` missed the exact value for most arguments. |

The three builds also refuse a mathlib they find installed whose cosine of
2^52 − 1 is wrong, running a program linked with it. mathlib compiled with the
contraction of multiplications and additions into fused multiply-adds, which
GCC does by default wherever the processor has them, 64-bit ARM processors
included, unless given `-ffp-contract=off`, reduces large arguments modulo π/2
wrongly (`branred()`): compiled so on x86_64, it gave cos(2^52 − 1) about
−0.4855 rather than 0.4733, and sin, cos and tan far from their values at 534 of
20080 random arguments from 2^26 to 2^54, which GAOL's bounds did not enclose.
The message says to build GAOL with the mathlib of `3rd/mathlib`, which the
three builds compile with `-ffp-contract=off` unless told to use an installed
one (`GAOL_FIND_MATHLIB`, `--with-mathlib-include`, `-Dwith-mathlib-include`),
or to install that one with `scripts/install-mathlib.sh`. When the program
cannot run, cross-compiling without an emulator, the builds only warn.
