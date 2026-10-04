# Review of todo-06b-mingw-msvcrt-refused (against origin/configure-clean 0f2963c)

Reviewer worktree: `scratchpad/wt/review-06b` (detached at 878c808, now removed).
Notes, probes and logs: `scratchpad/tmp/review-06b/`. Builds:
`scratchpad/build/review-06b-{sse,clang-nofma,w64teeth,w64refused}`.

Verdict: **changes**. The x86-64 condition, the regression test and the CI jobs are right. One
blocking defect remains: the refusal outside x86-64 still gives false reasons, in the
`#error` and in the docs and the manual. That goes against the request to put the true reason
everywhere.

## Blocking

### B1. Outside x86-64, the refusal still gives false reasons (`#error`, manual, docs, CMake comment)

1. **The second `#error` gives a false reason on ARM.** It is at
   `gaol/gaol_config.h:264-266`, and its condition is
   `!defined(__x86_64__) && __MINGW64_VERSION_MAJOR < 11`. So it fires on 32-bit and 64-bit
   ARM mingw-w64 before 11, as well as on 32-bit x86 (for example an old llvm-mingw for
   aarch64).
   - It says there: "its fma() is that of mingw-w64's own math library, which is not correctly
     rounded".
   - This is false on ARM. In `mingw-w64-crt/math/fma.c` of v9 to v13 (checked: the same file
     byte for byte in v9–v12, and only an ARM64EC guard added in v13), ARM uses `fmacd` and
     ARM64 uses `fmadd`, which are hardware instructions.
   - The branch's own comment says so ("On 64-bit ARM, whose fma() is the instruction ... not
     tested"). The `#error` then tells these users something the code knows to be wrong. The
     advice `choco install ... --x86` also makes no sense there.
2. **The docs say 32-bit x86 has the `round()` problem, and it does not.**
   - The following texts give "that `round()` depends on the rounding direction" as a reason
     for the whole refused set, 32-bit x86 included:
     - `manual/v5/gaol.tex:331-336`;
     - `doc/three-builds.md:136` (the reason column);
     - `CMakeLists.txt:203-211`.
   - On 32-bit x86 this is false. There `res - x` is computed and compared in x87 extended
     precision, where it is exact:
     - I disassembled the `round.o` of WinLibs 11.2 i686 (mingw-w64 9), the refused toolchain.
       It is `fldl res; fsubl x; fldl 0.5; fcomip`, with no store to a double in between.
     - An exact emulation gives 0 wrong of the guard's 28 round checks on x87, against 6 of 28
       on SSE2.
   - `gaol_config.h` itself says "round() is right there". So the code comment and the docs
     now contradict each other.
3. **The docs say the refused MinGW-w64 are those whose `fma()` and `round()` come from
   mingw-w64's own math library. On 32-bit x86 that is not true.**
   - This wording is in `doc/three-builds.md:136`, `doc/continuous-integration.md:83`, the
     manual item, the CMake comment and the windows.yml comments. It suggests that the
     accepted 32-bit toolchains do not use that library.
   - They do. MinGW-Builds GCC 12/13 x86, Ubuntu i686 11 and MSYS2 MINGW32 all use mingw-w64's
     own x87 `fma()`. MinGW-Builds 12.2 and 13.2 i686 have the same x87 instruction sequence as
     Ubuntu's `fma.o` (I compared the disassembly). By the author's measurement, that `fma()`
     is not correctly rounded either (125 of 400,000 results off).
   - What actually separates the refused x86 toolchain is that WinLibs compiled that `fma()`
     without optimization, so every partial sum is rounded to a double.

**Suggested fix.** It changes only text and the preprocessor split:

- In `gaol/gaol_config.h`, split the second block:
  - `defined(__i386__) && __MINGW64_VERSION_MAJOR < 11`: keep the fma message.
  - A separate `#error` for the other (ARM) targets before 11. It should give an honest
    reason, for example "not tested with GAOL v5; its round() is that of mingw-w64's math
    library, computed in doubles, which depends on the rounding direction". That statement
    follows exactly from the x86-64 analysis, since it is the same C code in IEEE doubles. It
    should also drop `--x86`.
- In the manual, `three-builds.md` and `CMakeLists.txt`, scope "`round()` depends on the
  rounding direction" to x64.
- Describe the 32-bit x86 refusal as "before 11, whose `fma()` rounds each partial sum to a
  double (GCC 11 of WinLibs, mingw-w64 9)", not as "whose `fma()`/`round()` are mingw-w64's".
- Keep the accepted-x86 soundness question as an open question for the maintainer (author's
  question 1).

## Checked and correct

- **Preprocessor condition.** I checked it on real headers with `-fsyntax-only` of
  `gaol/gaol_config.h`:
  - Ubuntu mingw-w64 11 x64 (msvcrt, 0x700): refused, with GCC and in C. Also refused with
    `-D__MSVCRT_VERSION__=0xE00`, because the version is below 12.
  - The latest MSYS2 packages from repo.msys2.org, headers-git and crt-git 13.0.0.r380 (they
    report `__MINGW64_VERSION_MAJOR` 14):
    - MINGW64 (0x600): refused with GCC 13 and with clang-18.
    - UCRT64 (0xE00): accepted with GCC and with clang.
    - CLANG64: accepted, in C++ and in C.
    - UCRT64 with `-D__MSVCRT_VERSION__=0x700` (what `-mcrtdll=msvcrt-os` gives): refused.
    - MINGW64 with `-D_UCRT` (what `-mcrtdll=ucrt` gives): accepted.
  - Debian `mingw-w64-common` 12.0.0-5 (0x700) and 14.0.0-1 (0x600), from deb.debian.org:
    msvcrt by default, so refused. Debian's `mingw-w64-ucrt64-dev` 14.0.0 is 0xE00, so
    accepted.
  - Ubuntu i686 mingw-w64 11: accepted. 32-bit x86 is unchanged, because the condition for
    `!__x86_64__` is the same as before. Linux GCC and Clang: accepted.
- **Which library supplies `fma` and `round`.** Checked with `nm`:
  - MSYS2 MINGW64 r380: in `libmsvcrt.a` and `libmsvcrt-os.a`, from mingw-w64's own
    `lib64_libmsvcrt_common_a-{fma,round}.o`. Upstream has not fixed the algorithm: the
    disassembly shows the same 4-rounding sum as in mingw-w64 11.
  - Debian mingw-w64 14.0.0 msvcrt: the same.
  - UCRT64, CLANG64 and Debian ucrt64: imported from `api-ms-win-crt-math`, and
    `libmingwex.a` defines neither function.
  - v12 `Makefile.am` does move `math/fma.c` and `math/round.c` into `src_msvcrt_common`.
- **`_UCRT` logic.** Checked in `_mingw.h.in` v11–v13 (0xE00–0xFFF or ≥0x1400), and in the
  GCC 15 `mingw-w64.h` spec (`-mcrtdll=ucrt*` → `-D_UCRT`, `msvcrt-os` → 0x700).
- **The guard's reference values.** I recomputed them with exact rationals (Python
  `fractions`) in the four directions: all 24 fma values and all 7 round values match. The
  three "error-free" triples really are `c = -RN(a*b)`, with exact results.
- **Teeth.**
  - Setup: the branch was exported with `git archive`, and `#if 0 &&` was put before the x86-64
    condition in the copy only. It was built with `tmp/06b/mingw64-toolchain.cmake` (Ubuntu
    mingw-w64 11, GCC 13, msvcrt, -static) and run under wine 9.0 with a private
    `WINEPREFIX`.
  - Result: `gaol_test_core_math.exe` fails the fma check 21 of 24 times and the round check
    6 of 28 times, with 136,618 of 1,472,122 checks failed in all
    (`tmp/review-06b/core_math-w64teeth.log`).
  - An exact emulation of mingw-w64's algorithm gives the same 21/6 on SSE2. On x87
    (extended precision, then stored to a double) it gives 0/0. So the accepted 32-bit x86
    jobs stay green, as the author says.
- **The refusal itself.**
  - The branch as it is, with the same toolchain: configure passes, and the build stops on the
    new `#error` (`build-w64refused.log`).
  - The CI grep strings match the real output of GCC (x64, MSYS2 MINGW64 headers), Clang
    (MINGW64 headers) and GCC i686 with the WinLibs 9 headers.
- **Linux.**
  - sse (GCC 13.3, -mfma): full build with examples, and `ctest -j1` passes 49/49 (intervalf
    and interval2f skipped).
  - clang-18 with `-DGAOL_FMA=OFF -DGAOL_PRESERVE_ROUNDING=ON`: `gaol_test_core_math` has 877
    `call fma@plt`, and all 1,472,122 checks pass. This is glibc's `fma()` in the four
    directions.
- **CI platforms (reasoning).**
  - The round part cannot be flaky: `tests/elementary.cpp` `integer_functions()` already
    checks `std::round` at ±nextafter(0.5), ±2.5 and similar values in the four directions on
    every CI platform.
  - For fma, the CI jobs call either a hardware FMA instruction or a library `fma()` believed
    correctly rounded in every direction:
    - GCC/Clang with -mfma use `vfmadd` on x86-64 and on i386, since the check runs on FMA3
      runners.
    - s390x, ppc64le and riscv64 inline the hardware instruction; qemu's softfloat honours the
      dynamic mode.
    - armhf gets `-mfpu=neon-vfpv4`.
    - glibc's `fma()` in software, on i386 without -mfma: its round-to-odd algorithm needs x87
      64-bit precision. GAOL keeps that precision: it writes only the RC bits, and
      `FE_DFL_ENV` gives 0x37f.
    - musl's generic `fma()` does one int64→double conversion with a sticky bit.
    - `tests/arithmetic.cpp` already calls `std::fma` in directed modes the same way, with
      volatile operands and `rnd_keep()`.
  - Not checked here, CI only:
    - Apple's x86_64 libm `fma()` under Rosetta;
    - ucrtbase's x86 and x64 `fma()`.

    CORE-MATH already depends on each of these in jobs that pass. A failure there would
    point to real unsoundness, not flakiness.
- **The rest.**
  - `configure` regenerates byte for byte with autoconf 2.72.
  - The manual compiles (126 pages, 1 overfull box, which predates the change).
  - `check_branch`: OK (7 one-line commits, 12 files).
  - No forbidden files: `TODO.md`, `ChangeLog`, `doc/differences.md` and `gaol.pdf` are
    untouched.
  - The three builds need no change: only comments changed, and there is no new source, test
    or option.
  - `git merge-tree` against the pushed `todo-*` branches finds no conflict introduced by
    this branch. The conflicts with todo-04, todo-06, todo-12, todo-16, todo-17 and todo-36
    exist against configure-clean already.

## Non-blocking

1. **CI diagnostic that never fires** (`windows.yml:250`). `grep -E '...|define _UCRT$'`
   cannot match: `-dM` prints an empty macro as `#define _UCRT ` with a trailing space (checked
   with GCC and clang). So the job never shows `_UCRT`, even when it is defined, although the
   author's report says it prints it. The pass/fail result is not affected. Fix:
   `define _UCRT ?$`.
2. **`windows.yml:187`.** "GCC 11 to 13 of Chocolatey on x86-64 are mingw-w64 before 12
   (UCRT, ...)": 11.2.0.07112021 is WinLibs mingw-w64 9.0.0 **msvcrt**. The asset name has no
   `ucrt`, and the i686 headers of the same release default to 0x700.
3. **"which are right" (ucrtbase).** "the right ones of ucrtbase.dll" (`gaol_config.h`, the
   manual, the docs) is an inference from CI tests passing on FMA3 runners. ucrtbase's software
   path on CPUs without FMA3 was never checked. Suggest "which pass the tests". The guard now
   checks them on each CI run.
4. **Cygwin.** The x86-64 condition does not require `__MINGW32__`.
   - On Cygwin x86-64, code that includes `<windows.h>` (w32api = mingw-w64 headers, `_UCRT`
     undefined) before GAOL's headers is now refused whatever the w32api version. The message
     would be about mingw-w64's `fma()`, which Cygwin does not link.
   - Before, this happened only for w32api before 12.
   - Fix: add `defined(__MINGW32__) &&`, as `GAOL_RND_MINGW_FENV_ONLY` in `gaol_fpu_fenv.h`
     already does.
5. **The x86 `#error` recommends "GCC 12 to 15"** of MinGW-Builds. GCC 12 and 13 x86 use the
   x87 mingw-w64 `fma()`, which the same comment says is not correctly rounded. It would be
   better to recommend 14 and 15 (UCRT) first, pending the maintainer's decision on 32-bit x86.
6. **`doc/continuous-integration.md:83`.** "`round(0x1.fffffffffffffp-2)` is 1" should say
   "except rounding upward": upward, GAOL's own direction, it is 0.
7. **"mingw-w64 13" for the MSYS2 MINGW64 objects** (`gaol_config.h` comment). The package is
   crt-git 13.0.0.r91, whose headers report `__MINGW64_VERSION_MAJOR` 14. At r380, the
   current version, the objects are unchanged.
8. **Scope, outside the x64 request: ARM64 mingw-w64 linked with msvcrt.dll** (e.g. llvm-mingw
   msvcrt aarch64), and ARM64 11 UCRT. They get mingw-w64's `round()` computed in doubles, the
   same direction-dependent code as x86-64, and are accepted. This extends the author's open
   question 3.
9. **Pre-existing, not introduced here:** a mingw-w64 git snapshot that reports 12 but predates
   the move of `fma.c`/`round.c` would pass with UCRT headers. The guard catches it.

## Commands of note

- Probes: `scratchpad/tmp/review-06b/probe/`, with headers in `tmp/review-06b/msys2/x/` and
  `tmp/review-06b/debian/`.
- Exact checks: `python3 tmp/review-06b/check_table.py` (reference values) and
  `python3 tmp/review-06b/emul.py` (SSE2 against x87 emulation of mingw-w64's `fma`/`round`).
- Teeth: `tmp/review-06b/src-teeth` (exported copy, refusal disabled), built in
  `build/review-06b-w64teeth`, run as
  `WINEPREFIX=tmp/review-06b/wine WINEDEBUG=-all wine gaol_test_core_math.exe`.
