# Continuous integration

Part of the documentation of [this fork of GAOL](../README.md#documentation).

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
  - MinGW-w64 15, on x86 and x64;
  - MSYS2 UCRT64 (GCC) and CLANG64 (Clang).

They also build GAOL with autotools and meson, with the mathlib of
`3rd/mathlib`, on Ubuntu (x86_64, arm64), Debian (i386, armhf), macOS (arm64,
x86_64) and MSYS2, and the tests with the GAOL they install; build GAOL with
each of the three builds against a mathlib installed apart, on Ubuntu x86_64
(`MATHLIB_DIR`, `--with-mathlib-include`, `-Dwith-mathlib-include`); build GAOL
as a part of another project, brought in by FetchContent
(`tests/fetch_content`), and the tests with the GAOL that project installs;
check that the three builds agree on each of these machines; check
that Clang is refused on 32-bit ARM, Clang 14 on 64-bit ARM, and MinGW-w64 11
to 14 (13 and 14 with autotools and meson too), and Visual C++ without
`/fp:strict`. Jobs of each build restore the
rounding direction (`GAOL_PRESERVE_ROUNDING`): Ubuntu x86_64 GCC and arm64
Clang, Debian i386 and armhf, macOS arm64, Visual Studio x64, autotools and
meson. The jobs built in Release print the time per operation in their summary.

## Configurations refused or left out, and why

Each of these was built by the continuous integration at first, or asked for,
and gave wrong bounds, crashed, or was far slower than the others. The builds
now refuse the compilers among them (see
[Compilers and options refused](three-builds.md#compilers-and-options-refused)),
and the workflows check the refusal where they built before.

| Configuration | Problem | Now |
|---|---|---|
| Clang for 32-bit ARM processors (Debian armhf) | **Wrong bounds.** Clang does not honour the rounding direction there: built by Clang 21, 4556 of 16000 random products, squares and cubes did not enclose their exact values. | Refused by the three builds; a job checks that CMake refuses it (`containers.yml`). GCC builds the armhf jobs. |
| Clang 14 on 64-bit ARM (Ubuntu 22.04 arm64) | **Wrong bounds.** It says of `-frounding-math` "overriding currently unsupported rounding mode on this target", and bounds of `pow()` and `nth_root()` did not enclose the exact values. Clang 18 honours the rounding direction there. | Refused; a job checks the refusal (`linux.yml`). Ubuntu 22.04 arm64 is built with GCC only, Ubuntu 24.04 and 26.04 arm64 with Clang too. |
| MinGW-w64 GCC 11.2, 12.2 and 13.2 (mingw-w64 before version 12), x86 and x64 | **Wrong bounds.** Their math library gave `acosh()` near 1 up to 25 million doubles from the exact value, and `asinh()` of large negative numbers NaN, which no number of doubles moved outward repairs ([issue #1](https://github.com/Jordan08/GAOL/issues/1); an inverted interval of the other solution, not explained, is [issue #11](https://github.com/Jordan08/GAOL/issues/11)). | Refused by the three builds; eight jobs check that CMake refuses 11.2 to 14.2, and two that configure and meson refuse 13.2 and 14.2. |
| MinGW-w64 GCC 14.2 (mingw-w64 12) | **Speed.** The bounds were right, but its `fesetround()` runs the instruction `cpuid` at each call, which exits to the hypervisor in a virtual machine: 3.5 µs per call against 0.13 µs with mingw-w64 13, and `exp()`, `log()`, `sin()` and `cos()` of an interval took 10.6 to 13.8 µs rather than 0.5 to 0.7. | Refused, as above. MinGW-w64 is built with GCC 15.2 (mingw-w64 13) only, and with MSYS2. |
| Visual C++ without `/fp:strict` | **Crash, and bounds not certified.** Built with `/fp:strict` for GAOL and without it for the tests, `rounding_direction` and `other_functions` crashed with Visual Studio 2022 (a constant computed at compile time in read-only memory, which GAOL wrote); without it everywhere the tests passed, but Visual C++ then assumes rounding to nearest. | Refused by `gaol/gaol_config.h`; each Visual Studio job checks it (`tests/fp_strict`). |
| The SSE2 intervals on 32-bit Windows | **Crash.** A `std::vector` of SSE2 intervals crashed on its first `movaps` in `rounding_direction` with MinGW-w64 15.2 for x86: GCC takes the memory of `new` to be aligned on 16 bytes, which the C runtime aligns on 8. | The FPU intervals are compiled there, in the three builds, as with Visual C++, for which IBEX and the fork of Fabrice Le Bars build GAOL without them. On 64-bit Windows (MinGW-w64, MSYS2) the SSE2 intervals pass the tests. |
| Doubles computed on the x87 unit of 32-bit x86 | **Wrong bounds.** In extended precision, `exp`, `sin` and `cos` missed the exact value for most arguments. | The i386 jobs compile with `-msse2 -mfpmath=sse`, which the builds give; `gaol/gaol_config.h` refuses the x87 unit. |
| An installed mathlib compiled with fused multiply-adds | **Wrong bounds.** On 64-bit ARM, where GCC contracts by default, mathlib's cosine of 2^52 − 1 was −0.4855 rather than 0.4733. | Refused by the three builds, which run a program linked with it. |
| Debian 11 Bullseye (amd64, arm64, armhf) | **Could not be installed.** Out of support since August 2026: its security repository lists packages that can no longer be downloaded (`libc-dev-bin 2.31-13+deb11u14`...), so that `g++` cannot be installed in the container. | Left out. Debian 12 and 13 are built. |
| mathlib downloaded by each job | **Every job failed.** The CMake build downloaded `mathlib-2.1.1.tar.gz` from Frédéric Goualard's site in each new build directory, and `scripts/install-mathlib.sh` for configure and meson: on 17 September 2026 the site stopped answering, and all the jobs of the five workflows failed on a connection timed out, after 15 minutes each. | mathlib is in the sources (`3rd/mathlib`), and no job downloads anything but its tools. |
| The intervals of floats (`gaol::intervalf`, `gaol::interval2f`) | **Unfinished.** `sqrt(intervalf)` returns its argument and `interval2f::inverse()` aborts; neither IBEX nor Codac uses them. | Off by default in the three builds (`GAOL_FLOAT_INTERVALS`); the audit compares the option between them. |

## What these platforms showed

What building and testing GAOL on these platforms showed, while the workflows
were written and since their first run on 13 September 2026, and what was
changed in GAOL for it; [What differs from GAOL](differences.md) gives each
change with its measures.

- **Bounds that were wrong on some platforms only.**
  - The hyperbolic functions, taken from the libm of the system: the libms
    of glibc 2.31, musl and MinGW-w64 are sometimes more than one double from
    the exact value, and their values are moved three doubles outward
    ([issue #1](https://github.com/Jordan08/GAOL/issues/1)).
  - The numbers read from text: the C runtime of Windows and musl on 64-bit
    ARM round `strtod()` to nearest in every direction, and
    `interval("0.1")` did not enclose 1/10 there. Numbers are now read
    exactly. The decimal output still relies on the C library
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
  - `libgaol.dylib` of the autotools build on macOS was linked without
    mathlib, and a program linking it crashed at its first elementary
    function.
  - configure and meson did not install the headers for MinGW and Visual
    C++, and meson did not compile on Linux (`GETRUSAGE_IN_HEADER`).
  - The agreement jobs passed while a build had not configured in the
    containers, a pipe hiding the failure: they now fail, and print the
    logs.
- **Reports of the sanitizers that were suppressed at first**, and are fixed
  now: the nodes the parser did not free (`lsan.supp`,
  [issue #4](https://github.com/Jordan08/GAOL/issues/4)), and the shift of
  an `int` into its sign bit in mathlib's `halfulp()` (`ubsan.supp`,
  [issue #5](https://github.com/Jordan08/GAOL/issues/5)). Nothing is
  suppressed any more.
- **Tried and not taken.** Setting the precision of the x87 unit to 53 bits
  in mathlib's `Init_Lib()` on 32-bit Windows, as the fork of mathlib by
  Fabrice Le Bars does, was measured by a workflow of its own with Visual
  C++ 2022 and 2026, in x86 and x64: it changed neither the bounds nor the
  time of an operation.
