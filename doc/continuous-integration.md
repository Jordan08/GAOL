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
  - Visual Studio 2022 and 2026, on x86, x64 and arm64, Release and Debug;
  - MinGW-w64 15 on x86 and x64, Release and Debug, 14 on x86 and x64, 12 and
    13 on x86;
  - MSYS2 UCRT64 (GCC) and CLANG64 (Clang).

They also build GAOL with autotools and meson on Ubuntu (x86_64, arm64),
Debian (i386, armhf), macOS (arm64, x86_64) and MSYS2, and the tests with the
GAOL they install, which is the only library installed, CORE-MATH being
compiled into it; check that configure and meson refuse the options that chose
a mathematical library before GAOL v5; build GAOL as a part of another
project, brought in by FetchContent (`tests/fetch_content`), and the tests with
the GAOL that project installs; check that the three builds agree on each of
these machines; check that the builds refuse Clang on 32-bit ARM and Clang 14
on 64-bit ARM, that `gaol/gaol_config.h` refuses MinGW-w64 GCC 11 to 13 on x64
and GCC 11 on x86, and Visual C++ without `/fp:strict`. Jobs of each build
restore the rounding direction (`GAOL_PRESERVE_ROUNDING`): Ubuntu x86_64 GCC
and arm64 Clang, Debian i386 and armhf, macOS arm64, Visual Studio x64,
autotools and meson. The jobs built in Release print the time per operation in
their summary.

The manual is built from `manual/gaol.tex` with the LaTeX of Ubuntu 24.04, by
the autotools and the meson builds, when `manual/` changes (`manual.yml`); the
PDF is an artifact of the run.

## Configurations refused or left out, and why

Each of these was built by the continuous integration at first, or asked for,
and gave wrong bounds, crashed, was far slower than the others, or could not
be built. The builds now refuse the compilers among them (see
[Compilers and options refused](three-builds.md#compilers-and-options-refused)),
and the workflows check the refusal where they built before. On 21 September
2026, Clang on 32-bit ARM, Clang 14 on 64-bit ARM, the SSE2 intervals on 32-bit
Windows and doubles computed on the x87 unit were built again with GAOL v5, past
their refusal, and Debian 11 was tried again: the Problem column says what they
gave.

| Configuration | Problem | Now |
|---|---|---|
| Clang for 32-bit ARM processors (Debian armhf) | **Wrong bounds.** Clang does not honour the rounding direction there: built by Clang 21, 4556 of 16000 random products, squares and cubes did not enclose their exact values. Built past the refusal by Clang 19 (Debian 13) and Clang 21 (Debian sid), GAOL v5 fails five of its nine tests: bounds of `atan2()`, `sin()`, `cos()`, `pow()`, `nth_root()` and `div_rel()` do not enclose the exact values, and the products are not the tightest ones in any rounding direction. | Refused by the three builds; a job checks that CMake refuses it (`containers.yml`). GCC builds the armhf jobs. |
| Clang 14 on 64-bit ARM (Ubuntu 22.04 arm64) | **Wrong bounds.** It says of `-frounding-math` "overriding currently unsupported rounding mode on this target". Built past the refusal, GAOL v5 fails four of its nine tests in Release, bounds of `nth_root()`, `pow()`, `atan2()`, `cos()`, `div_rel()` and `mid()` not enclosing the exact values, and one in Debug, a bound of `atan2()`. Clang 18 honours the rounding direction there. | Refused; a job checks the refusal (`linux.yml`). Ubuntu 22.04 arm64 is built with GCC only, Ubuntu 24.04 and 26.04 arm64 with Clang too. |
| MinGW-w64 GCC 11.2, 12.2 and 13.2 (mingw-w64 before version 12), x86 and x64 | **Wrong bounds.** Their math library gave `acosh()` near 1 up to 25 million doubles from the exact value, and `asinh()` of large negative numbers NaN, which no number of doubles moved outward repairs ([issue #1](https://github.com/Jordan08/GAOL/issues/1); an inverted interval of the other solution, not explained, is [issue #11](https://github.com/Jordan08/GAOL/issues/11)). | **Not the reason any more:** CORE-MATH gives every elementary function, and `acosh` and `asinh` no longer come from the math library of the system. What is refused now is a mingw-w64 whose `<fenv.h>` answers `fegetround()` from a state of its own rather than from the registers, which CORE-MATH's functions read to know the direction GAOL set by writing them (`gaol/gaol_config.h`): the runtime before 12 on x86-64, which is GCC 11 to 13, and before 11 on 32-bit x86, which is GCC 11. Four jobs check the refusal (`windows.yml`), and GCC 12, 13 and 14 build and pass on 32-bit x86. |
| MinGW-w64 GCC 14.2 (mingw-w64 12) | **Speed.** The bounds were right, but its `fesetround()` runs the instruction `cpuid` at each call, which exits to the hypervisor in a virtual machine: 3.5 µs per call against 0.13 µs with mingw-w64 13, and `exp()`, `log()`, `sin()` and `cos()` of an interval took 10.6 to 13.8 µs rather than 0.5 to 0.7. | **Not the reason any more:** GAOL does not call `fesetround()` for its elementary functions, CORE-MATH computing in the direction in effect. MinGW-w64 GCC 14.2 and 15.2 build and pass, on x64 and on 32-bit x86, and MSYS2 is built too. |
| Visual C++ without `/fp:strict` | **Crash, and bounds not certified.** Built with `/fp:strict` for GAOL and without it for the tests, `rounding_direction` and `other_functions` crashed with Visual Studio 2022 (a constant computed at compile time in read-only memory, which GAOL wrote); without it everywhere the tests passed, but Visual C++ then assumes rounding to nearest. | Refused by `gaol/gaol_config.h`; each Visual Studio job checks it (`tests/fp_strict`). |
| The SSE2 intervals on 32-bit Windows | **Crash.** A `std::vector` of SSE2 intervals crashed on its first `movaps` in `rounding_direction` with MinGW-w64 15.2 for x86: GCC takes the memory of `new` to be aligned on 16 bytes, which the C runtime aligns on 8. Forced with GAOL v5, they still crash four of the nine tests (`arithmetic`, `other_functions`, `reverse`, `rounding_direction`) with MinGW-w64 14.2 and 15.2, in Release and in Debug. With `-faligned-new=8`, which makes `new` align them itself, the tests pass, but the code using GAOL would need the flag too. | The FPU intervals are compiled there, in the three builds, as with Visual C++, for which IBEX and the fork of Fabrice Le Bars build GAOL without them. On 64-bit Windows (MinGW-w64, MSYS2) the SSE2 intervals pass the tests. |
| `fesetexceptflag()` of mingw-w64 for 32-bit x86 | **Crash.** CORE-MATH's `cbrt`, `pow` and `atan2` keep the exception flags around their work, with `_mm_getcsr()` and `_mm_setcsr()` on x86-64 and with `fegetexceptflag()` and `fesetexceptflag()` everywhere else. On a 32-bit target mingw-w64 writes the flags into the mask bits of MXCSR as well: the register went from 0x5fb2 to 0x5932, which unmasks the invalid, divide-by-zero and overflow exceptions. The first comparison of the bounds of an empty interval, which are NaN and which `is_empty()` compares with `<=`, then trapped rather than raised a flag, and the tests died without a message. | The two are written on MXCSR in `gaol/core_math_port.h`, force-included into CORE-MATH's sources, on a 32-bit x86 Windows only; the mask bits and the rounding bits are left exactly as they are. |
| Doubles computed on the x87 unit of 32-bit x86 | **Wrong results rounded to nearest, and wrong bounds.** The x87 unit computes with 64-bit significands and 15-bit exponents, and CORE-MATH assumes every operation on doubles rounded to a double (`FLT_EVAL_METHOD` 0, see `3rd/math-core/README.md`). Built past the refusal on Debian 12 i386 with GCC 12, GAOL v5 passes its other tests, but CORE-MATH rounded to nearest gives the other neighbour of the exact value at 175 of the arguments compared with SSE2, its results being rounded twice: `exp(-0x1.74910d52d3051p+9)` is 0 rather than 2^-1074, `tanh(0x1.30fc1931f09c9p+4)` 1 rather than 1 − 2^-53. On x86-64 with `-mfpmath=387` and GCC 9, which turns into doubles at compile time, rounded to nearest, the constants CORE-MATH rounds in the direction in effect (`0x1p-1074 * 0.5`, `-1.0 + 0x1p-54`), GAOL's bounds of `exp2(-1075)`, `exp10(-400)`, `expm1(-800)` and `atan2()` of a tiny and a huge number do not enclose the exact values. | Refused by `gaol/gaol_config.h`; the i386 jobs compile with `-msse2 -mfpmath=sse`, which the builds give. `tests/extended_precision.cpp` checks these arguments, CORE-MATH in the four rounding directions and GAOL's bounds: built on the x87 unit, it fails with GCC 12 on i386 as with GCC 9 on x86-64. |
| Debian 11 Bullseye (amd64, arm64, armhf) | **Could not be installed.** Out of support since August 2026: its security repository lists packages that can no longer be downloaded (`libc-dev-bin 2.31-13+deb11u14`...), so that `g++` cannot be installed in the container; without that repository the packages are broken, and archive.debian.org does not have it yet. GAOL v5 itself builds and passes its tests there, with GCC 10.2 and CMake 3.18.4 installed from snapshot.debian.org. | Left out. Debian 12 and 13 are built. |
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
