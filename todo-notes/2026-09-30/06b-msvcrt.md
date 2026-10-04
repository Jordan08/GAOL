# 06b: mingw-w64 linked with msvcrt.dll refused on x86-64, and the true reason (fma() and round())

Branch `todo-06b-mingw-msvcrt-refused`, made from `origin/configure-clean`
(0f2963c). Worktree `scratchpad/wt/06b`. Logs, programs and toolchain files in
`scratchpad/tmp/06b/` (see "Where things are" at the end). Nothing pushed.

## Commits

- 3ab8c5e gaol_config.h refuses on x86-64 every mingw-w64 linked with msvcrt.dll: its fma() and round() are wrong, not fegetround()
- c6acef8 tests/core_math.cpp checks first the fma() and round() of the C library, against values computed apart
- 4dd16d6 The refused MinGW-w64 in the three builds, the docs and the manual: fma() and round(), msvcrt.dll on x86-64
- 6c9adf1 windows.yml: the refused MinGW-w64 jobs look for fma() and round() in the message, and MSYS2 MINGW64 is refused
- 446506f gaol_fpu_fenv.h: mingw-w64 for 32-bit x86 reads fegetround() from the x87 control word, not a state of its own
- 91e06d4 gaol_config.h and tests/core_math.cpp: the evidence on mingw-w64's fma() and round() made shorter and precise
- 878c808 doc/tests.md: core_math checks first the fma() and round() of the C library

`check_branch todo-06b-mingw-msvcrt-refused`: OK (7 commits, 12 files).

## What was wrong and what the change does

The refusal of mingw-w64 in `gaol/gaol_config.h` (before 12 on x86-64, before
11 on 32-bit x86) gave a false reason: "its `<fenv.h>` answers `fegetround()`
from a state of its own". The `fegetround()` of mingw-w64 9 to 12 is `fnstcw`
on x86 and x86-64 (report 06b). The true reason is the `fma()` and `round()`
of mingw-w64's own math library (`math/fma.c`, `math/round.c`, the same code
for x86 from v9 to v13):

- `fma()` adds the four products of the 26/27-bit halves of x and y to z with
  four roundings. It is not correctly rounded. CORE-MATH calls it through
  `__builtin_fma()` (GCC for Windows gets no `-mfma`), and GAOL's exact
  products call `std::fma()`.
- `round()` is `ceil(x)`, less 1 when that is more than 1/2 above x. That
  difference is rounded in the current direction, so on x86-64
  `round(0x1.fffffffffffffp-2)` is 1 in every direction except upward.

Before mingw-w64 12 both functions are in libmingwex, which every program links.
From 12 on they are only in `libmsvcrt*.a` (`src_msvcrt_common` of
`mingw-w64-crt/Makefile.am`, "Files included in all libmsvcr*.a"), so:

- a toolchain linking the UCRT takes the correct ones of `ucrtbase.dll`;
- a toolchain linking `msvcrt.dll` still takes mingw-w64's. This covers MSYS2
  MINGW64, the msvcrt builds of MinGW-Builds, and the cross compilers of
  Debian and Ubuntu. GAOL accepted these on x86-64: a soundness hole.

**The change (maintainer's decision).** On x86-64, `gaol/gaol_config.h` now
refuses a mingw-w64 when `__MINGW64_VERSION_MAJOR < 12 || !defined(_UCRT)`.
The `#error` names the true reason and the accepted toolchains:

- MinGW-Builds GCC 14 and 15 (`choco install mingw --version=15.2.0`);
- MSYS2 UCRT64 or CLANG64 (`pacman -S mingw-w64-ucrt-x86_64-gcc`);
- Visual Studio.

The refusal on 32-bit x86 (and on 64-bit ARM) before 11 is kept as it was, in
an `#error` of its own that now gives the true reason for x86: the `fma()`.

**Why `_UCRT`.** It is the macro mingw-w64 itself uses to select the UCRT
declarations. I checked it in `_mingw.h.in` of v11, v12 and v13 and in the
installed headers:

- `__MSVCRT_VERSION__` defaults to the configured runtime: 0xE00 for ucrt,
  0x600 or 0x700 for msvcrt.
- `_mingw.h` defines `_UCRT` when that value is 0xE00 to 0xFFF, or 0x1400 or
  more.
- `--with-default-msvcrt` defaults to ucrt from v12 (msvcrt in v11).
- GCC 14 and 15 `-mcrtdll=ucrt*` add `-D_UCRT` and link `-lucrt`. I read
  `gcc/config/i386/mingw-w64.h` of the gcc-14 and gcc-15 release branches;
  GCC 13, the one installed here, has no `-mcrtdll`.
- `-mcrtdll=msvcrt-os` gives `-D__MSVCRT_VERSION__=0x700`, so `_UCRT` stays
  undefined and the program is refused, which is right.
- Clang relies on the same headers, so the test covers Clang too.

`gaol_config.h` includes `<limits.h>` before the test, which brings in
`_mingw.h`.

A program that defines `_UCRT` (or `__MSVCRT_VERSION__=0x1400`) itself while
linking `msvcrt.dll` is not refused. No macro shows that case, but
`tests/core_math.cpp` then fails. A test on `__MSVCRT_VERSION__ >= 0xE00`
would behave the same, because `_mingw.h` derives `_UCRT` from it.

**32-bit x86: the true reason found.** I installed wine32 (see Risks) and
looked at the CI history and at the binaries:

- **The refused WinLibs GCC 11.2.0 x86 (mingw-w64 9.0.0).** Its libmingwex
  `fma` is compiled without optimization: each statement stores `ret` as a
  double (`fstpl`), so every partial sum is rounded to a double.
  - Its errors, measured with its `fma.o` linked into `fmatest.c` under wine:
    - error-free products: 0 wrong;
    - other triples: 24,763, 73,165, 73,176 and 69,621 wrong of 100,000
      (to nearest, upward, downward, toward zero), about the x86-64 rates.
  - In the CI, GAOL was accepted with it in run 119 (job 106095196810,
    2026-09-20) and run 122 (job 106101384755). The failures were those of
    x86-64:
    - `elementary`: 29 checks, including `tan(0x1.56e1fc2f8f359p-997)` two
      doubles above x;
    - `reverse`: 4 checks;
    - `core_math`: about 2,250 per function.
  - Run 122 already had the `<fenv.h>`-only path (`GAOL_RND_MINGW_FENV_ONLY`),
    and it changed nothing.
- **The accepted x86 toolchains.** These are MinGW-Builds 12.2.0 (rt_v10) and
  13.2.0 (rt_v11), which report `__MINGW64_VERSION_MAJOR` 11, and Ubuntu's
  mingw-w64 11.
  - Their `fma` is compiled with optimization and keeps the sums in x87
    extended precision. Its 800,000 results are bit-identical across the
    three toolchains:
    - the error-free products are exact;
    - 125 of the 400,000 other results are one double off (22, 41, 28 and 34
      per direction). It is still not correctly rounded.
  - Their `round()` is right: it is computed in extended precision. It does
    give `-0` for `round(0x1.fffffffffffffp-2)` rounding downward.
  - The full ctest of Ubuntu's i686 mingw-w64 11 under wine32 passes, on
    configure-clean and on this branch (31 tests, 2 skipped).

So what the x86 refusal really rests on is the `fma()` of the refused
toolchain's build. Mingw-w64 9 on x86 is not wrong as a version: WinLibs built
its `fma` without optimization. I did not widen or narrow the 32-bit refusal.
The accepted x86 toolchains are an open question (below).

**`GAOL_RND_MINGW_FENV_ONLY`** (`gaol/gaol_fpu_fenv.h`, x86 before 12). Its
comment repeated the false reason, and it is now corrected. With that path
disabled in an exported copy (registers written directly), the full ctest of
Ubuntu's i686 mingw-w64 11 under wine32 passes as well. The code is
unchanged; see open question 2.

**A run-time guard.** `tests/core_math.cpp` now first checks
`c_library_fma_and_round()`:

- `std::fma` in the four directions at six triples:
  - three error-free products `fma(a, b, -a*b)` that mingw-w64's x86-64 `fma`
    gets wrong in every direction;
  - two other triples it gets wrong in every direction;
  - `fma(x, 0x1p-54, x)` with x = 0x1.56e1fc2f8f359p-997, which is what
    `cr_tan` returns for a tiny x (wrong upward).
- `std::round` in the four directions at ±0x1.fffffffffffffp-2, ±0.5, ±2.5
  and 2^52 − 1/2.

The operands are volatile and the results go through `gaol::rnd_keep()`, as
in `tests/arithmetic.cpp`. The expected values were computed with Python
`fractions` (exact) and checked with mpmath (`tmp/06b/guard/table.py`).
Without `-mfma` (MinGW) the test calls the library `fma()`, as CORE-MATH
does. With `-mfma` it checks the instruction GAOL then uses. The triples were
chosen among those the x87 `fma` of mingw-w64 11 gets right, so the accepted
x86 CI jobs stay green (open question 1).

**CI.**

- The four `mingw-refused` jobs (Chocolatey 13.2.0, 12.2.0, 11.2.0 x64; 11.2.0
  x86) now also grep the reason from a matrix field, with `grep -F`.
- A new `msys2-refused` job (msys2/setup-msys2, `msystem: MINGW64`) has two
  entries, GCC (`mingw-w64-x86_64-toolchain`) and Clang
  (`mingw-w64-x86_64-clang`).
  - It prints `__MINGW64_VERSION_*`, `__MSVCRT_VERSION__` and `_UCRT`.
  - It configures with CMake and Ninja and expects the build to stop on the
    x86-64 `#error`.
- The comments of the workflow were corrected; the old one even ended in
  mid-sentence.

## Tests

The new checks are in `tests/core_math.cpp`: 24 `fma()` checks named "the fma()
of the C library: correctly rounded" and 28 `round()` checks named "the round()
of the C library: the same in every rounding direction". Results:

| Build | fma() | round() | core_math overall |
|---|---|---|---|
| Linux GCC 13, sse (`-mfma`, the instruction) | 0 of 24 failed | 0 of 28 failed | 1,472,122 checks, 0 failed |
| Linux, `GAOL_FMA=OFF` (glibc's `fma()` called, 778 `call fma@plt` in the test) | 0 of 24 failed | 0 of 28 failed | 1,472,122 checks, 0 failed |
| mingw-w64 11 x86-64 under wine, refusal disabled in an exported copy | 21 of 24 failed | 6 of 28 failed | 136,618 failed |
| same, `fma` and `round` objects of MSYS2 MINGW64 (mingw-w64 13, msvcrt) linked in | 21 of 24 failed | 6 of 28 failed | 136,618 failed |
| same, hardware `fma` (`fma_hw.o`) and a direction-free `round` linked in | 0 failed | 0 failed | 1,472,122 checks, 0 failed |
| mingw-w64 11 i686 (Ubuntu) under wine32, branch as is | 0 of 24 failed | 0 of 28 failed | 0 failed; full ctest passes |

The 21 failures are:

- the 12 checks of the three error-free products;
- the 8 checks of the two other triples;
- the tan triple, upward.

The 6 failures are ±0x1.fffffffffffffp-2 to nearest, downward and toward zero.
In the x86-64 build with the refusal disabled, `elementary` (29) and `reverse`
(4) fail too, as on configure-clean.

## How the test was shown to fail without the fix

The "fix" here is the refusal. The guard was shown to have teeth with the
refusal disabled only in an exported copy:

- `git archive HEAD` into `tmp/06b/src-teeth64`, with `#if 0 &&` put before
  the x86-64 condition. This was never committed.
- Built with `tmp/06b/mingw64-toolchain.cmake` (Ubuntu mingw-w64 11, GCC 13,
  msvcrt, `-static`), Release, `WITH_TESTS=ON`.
- `ctest -j1` under wine 9.0: `core_math` fails with the 21 + 6 guard
  failures above (`tmp/06b/teeth/ctest-64.log`).
- MSYS2's own objects (`lib64_libmsvcrt_common_a-{fma,round}.o` of
  `mingw-w64-x86_64-crt-git-13.0.0.r91`) relinked into the same test: same
  result (`tmp/06b/teeth/core_math_msys2_mingw64.log`).
- The Linux build passes, and so does the relinked test with correct `fma`
  and `round` under wine (`tmp/06b/teeth/core_math_fma_round_ok.log`).

The refusal itself:

- `cmake` with the toolchain file on this branch: configure passes, and the
  build stops on the new `#error` at `gaol_interval.cpp`
  (`tmp/06b/refused/build-w64.log`).
- `gaol_config.h` preprocessed (`-fsyntax-only`, `tmp/06b/probe`):

| Headers and compiler | Result |
|---|---|
| Ubuntu mingw-w64 11 x64 (msvcrt), GCC 13 | refused (x86-64 message) |
| same with `-D__MSVCRT_VERSION__=0x1400` | refused (version < 12) |
| MSYS2 MINGW64 headers-git 13.0.0.r91 (reports 14, `__MSVCRT_VERSION__` 0x600), GCC 13 `-nostdinc` | refused |
| same headers, clang-18 `--target=x86_64-w64-windows-gnu` | refused |
| MSYS2 UCRT64 headers (0xE00, `_UCRT`), GCC 13 | accepted |
| MSYS2 CLANG64 headers, clang-18 | accepted |
| UCRT64 headers with `-D__MSVCRT_VERSION__=0x700` (what `-mcrtdll=msvcrt-os` gives) | refused |
| MINGW64 headers with `-D_UCRT` (what `-mcrtdll=ucrt` gives) | accepted |
| WinLibs 11.2.0 i686 headers (mingw-w64 9) | refused (x86 message) |
| MinGW-Builds 12.2.0 rt_v10 i686 headers (11, UCRT) | accepted |
| Ubuntu mingw-w64 11 i686 | accepted |
| Linux GCC 13 | accepted |

The CI grep strings were matched against real compiler output: GCC x64,
WinLibs x86 and clang with MINGW64 headers.

## Local validation

- Linux sse (GCC 13.3, Release, examples): full build and `ctest -j1`, 49 of
  49 pass, with `intervalf` and `interval2f` skipped as usual. This was run
  on c6acef8 (`tmp/06b/linux/ctest.log`), then rebuilt and run again on
  91e06d4 (`ctest2.log`). 878c808 only changes `doc/tests.md`.
- Linux with `-DGAOL_FMA=OFF`: `gaol_test_core_math` calls glibc's `fma()`,
  and its 1,472,122 checks pass (`tmp/06b/linux/core_math-nofma.log`).
- i686 cross build of the branch (mingw-w64 11 Ubuntu), full ctest under
  wine32: 31 tests, 29 pass, 2 skipped (`tmp/06b/w32/ctest-head32.log`).
  Configure-clean gives the same (`ctest-base32.log`), and so does the
  `<fenv.h>`-only path disabled (`ctest-nofenv32.log`).
- The manual compiles: `sh manual/build-pdf.sh manual/v5 tmp/06b/pdf`, 126
  pages. The one overfull box, at lines 505–509, predates the change.
- `configure` was regenerated with autoconf 2.72 of `scratchpad/tools/bin`. I
  first checked that the unchanged base regenerates byte for byte. The only
  difference is the `#` comment copied from `configure.ac`.
- `windows.yml` parses with PyYAML. The matrix fields and the job list were
  checked (`mingw`, `mingw-refused`, `msys2`, `msys2-refused`,
  `visual-studio`).

Not tested here (CI only):

- the MSYS2 MINGW64 jobs themselves (package names checked in the MSYS2
  repository index: gcc 16.2.0, clang 22.1.8, cmake, ninja);
- Visual C++, macOS, musl, qemu platforms against the new guard. All their
  `fma()` and `round()` should be correct.

## Documentation changed

- `gaol/gaol_config.h`: the comment and both `#error` messages.
- `gaol/gaol_fpu_fenv.h`: the comment of `GAOL_RND_MINGW_FENV_ONLY`.
- `CMakeLists.txt` (around line 203), `configure.ac` (around 231) and
  `configure`, `meson.build` (line 73, which wrongly said "no longer
  refused").
- `doc/three-builds.md`: table row and last paragraph.
- `doc/continuous-integration.md`: the refusal sentence and the table row.
- `doc/tests.md`: `core_math`.
- `manual/v5/gaol.tex`: the platform list, and the refused item of the
  section on refused compilers and options.
- `.github/workflows/windows.yml`: the comments.
- `doc/differences.md` is not edited: see "For doc/differences.md".

## Behaviour change

- On x86-64, a mingw-w64 linked with `msvcrt.dll` is now refused when
  compiling, whatever its version, with GCC or Clang. This covers MSYS2
  MINGW64, the msvcrt builds of MinGW-Builds, and Debian's and Ubuntu's cross
  compilers.
- Before 12 it stays refused, with a new message.
- 32-bit x86 and other targets: the same set as before, with a new message.
- `gaol_test_core_math` has 52 more checks. It fails on any platform whose
  `fma()` or `round()` is wrong at these values.

## API and ABI

None.

## Overlaps with other points

- **Point 6** (`todo-06-fegetround-mxcsr`) touches other files. Its finding 1
  (the cbrt bound with mingw-w64 x64) is independent.
- **`todo-04-ftz-daz`** edits `doc/three-builds.md`:
  - the `-ffast-math` row right under the mingw row changed here;
  - an insertion after the paragraph rewritten here.
  - Expect a textual conflict, easy to resolve (keep both).
  - It also inserts in `manual/v5/gaol.tex` after the refused list, close to
    the changed item.
- **`todo-03-pow-exact-corner`** adds to `tests/core_math.cpp`, a function
  near line 302 and a call in `main()` after `sin_accurate_path()`. It should
  merge cleanly: the call added here is right after `gaol::init()`.
- **`todo-40-small-errors`** edits `gaol/gaol_config.h` around lines 144–165,
  away from the mingw block.
- **Point 31** (upstream patches): mingw-w64's `fma()` and `round()` are
  mingw-w64 bugs, still there for msvcrt in v13. They are worth reporting to
  mingw-w64. Nothing was sent.

## Risks

- **CI red on the new guard where a C library's `fma()` or `round()` is
  wrong.** That is the purpose, but it could hit a platform nobody suspected.
  The ones I expect to be correct are glibc (x86-64 ifunc and i386), musl,
  macOS libm, UCRT (x86, x64, arm64), qemu softfloat and the fma instructions.
  A failure there means that platform's bounds are unsound too.
- **The MSYS2 MINGW64 Clang entry** depends on `mingw-w64-x86_64-clang`
  linking CMake's test program with the GCC toolchain installed beside it. If
  MSYS2 changes that, the job fails before reaching the `#error`; its log
  shows which step failed.
- **Machine state.**
  - I added the i386 architecture and installed `wine32:i386`. To get it in, I
    needed a dummy `libgphoto2-6t64:i386`: the ondrej/php PPA's `libgd3:amd64`
    blocks the real one.
  - apt upgraded about 20 amd64 packages to their noble-updates versions (no
    compiler, libc, cmake, TeX or wine package).
  - With wine32 installed, `/usr/bin/wine` picks the 32-bit loader, which
    broke 64-bit programs in wine64-only prefixes. I renamed the loader to
    `/usr/lib/wine/wine.i386` at once, to restore `wine` for the other agents.
  - At the end I put the loader's name back and removed `wine32:i386`,
    `libwine:i386` and the dummy package. `wine` runs x86-64 programs again
    in the old prefixes (checked).
  - About 80 automatically installed i386 libraries stay, harmless. I did not
    run `apt-get autoremove`, which could remove more than mine.
  - To redo the 32-bit tests: `apt-get install --no-install-recommends
    wine32:i386`, after installing `tmp/06b/w32/libgphoto2-dummy_i386.deb`.
    Then rename `/usr/lib/wine/wine` to `wine.i386`, and use
    `tmp/06b/w32/mingw32-toolchain-i386loader.cmake` with the prefix
    `tmp/06b/wine32`.

## Open questions and decisions for the maintainer

1. **Are the accepted 32-bit x86 toolchains sound?** MinGW-Builds GCC 12 and
   13 x86 in the CI, and Ubuntu's i686 mingw-w64 11, use mingw-w64's x87
   `fma()`.
   - It is exact on the 100,000 error-free products tried, but 125 of 400,000
     other results are one double off. 41 of the 100,000 upward results are
     one double above the correctly rounded value, which is still an upper
     bound.
   - All of GAOL's tests pass with it, in the CI and here under wine32.
   - But CORE-MATH's proofs assume a correctly rounded `fma()`, and GAOL's
     lower bound (the double below the upward result) assumes CORE-MATH is
     correctly rounded.
   - Options:
     - (a) keep accepting them (today);
     - (b) refuse 32-bit x86 before 12 or without `_UCRT`, as on x86-64. The
       MinGW-Builds GCC 12 and 13 x86 jobs would become refused jobs; GCC 14
       and 15 x86 are UCRT and mingw-w64 12/13, whose `fma` is UCRT's.
   - With (b), the guard could also take triples the x87 `fma` gets wrong,
     for example:
     - a = −0x1.53d31ea5098bcp-4, b = −0x1.8366682864b75p+10,
       c = −0x1.9535d79821296p-3: the upward result should be
       0x1.00babe64a5209p+7, and mingw-w64 11 for x86 gives the double above;
     - a = −0x1.42d4192294c63p+11, b = −0x1.2c80de6befe77p+11,
       c = 0x1.98cee0aa846fdp-38: wrong downward and toward zero;
     - the 91 triples are in `tmp/06b/guard/rows.pkl`.
   - I left them out, so as not to turn the accepted x86 jobs red without
     your decision.
2. **`GAOL_RND_MINGW_FENV_ONLY`** (x86, mingw-w64 before 12, through
   `fesetround()`) is not needed as far as the tests show: with the registers
   written directly, Ubuntu's i686 mingw-w64 11 passes the full ctest under
   wine32. Keep it (cautious, and what the CI tests) or drop it (faster)?
   Unchanged here.
3. **64-bit ARM mingw-w64 before 11** stays refused, untested. Its `fma()` is
   the instruction, but its `round()` would be the libmingwex one computed in
   doubles, as on x86-64. ARM64 mingw-w64 11 (UCRT, e.g. an older
   llvm-mingw) is accepted today and would have that `round()` too. The guard
   catches it at test time; refusing ARM64 before 12 would be the analogue of
   x86-64.
4. **A program that defines `_UCRT` itself while linking `msvcrt.dll`** is not
   refused: no macro shows it, and `tests/core_math.cpp` catches it.
5. **A git snapshot of mingw-w64 reporting 12**, taken before `fma.c` and
   `round.c` moved to `src_msvcrt_common`, would pass with UCRT headers and
   still link libmingwex's. This is theoretical (I did not date the move),
   and the guard catches it.
6. **The x86 `#error` message** speaks of `fma()`. On 64-bit ARM before 11,
   which the same condition refuses, the problem would be `round()`.

## For TODO.md

(in French, as TODO.md)

- **06b** (fait) : sur x86-64, `gaol/gaol_config.h` refuse tout mingw-w64 lié à `msvcrt.dll` (`!defined(_UCRT)`) en plus des
  versions avant 12. La raison donnée est la vraie : `fma()` (non correctement arrondi, 10,7 % des produits exacts
  `fma(a, b, -a*b)` faux) et `round()` (dépend du sens d'arrondi) de la bibliothèque mathématique de mingw-w64, que CORE-MATH
  et GAOL appellent ; depuis mingw-w64 12, seule une chaîne liée à l'UCRT prend ceux, corrects, de `ucrtbase.dll`. Le
  `fegetround()` de mingw-w64 lit le mot de contrôle x87 : pas d'état propre. Sur x86 32 bits, le refus avant 11 vient du
  `fma()` de WinLibs GCC 11.2 (mingw-w64 9), compilé sans optimisation. `tests/core_math.cpp` vérifie d'abord `fma()` et
  `round()` de la bibliothèque C (21 et 6 échecs avec mingw-w64 11 x64 sous wine) ; la CI refuse aussi MSYS2 MINGW64 (GCC et
  Clang). Questions ouvertes : le `fma()` x87 des MinGW-w64 x86 acceptés (GCC 12 et 13, mingw-w64 11) n'est pas
  correctement arrondi (125 résultats sur 400 000 à un double près ; tous les tests passent) ; `GAOL_RND_MINGW_FENV_ONLY`
  n'est pas nécessaire d'après les tests ; mingw-w64 ARM64.

## For ChangeLog

- gaol/gaol_config.h: refuse on x86-64 every mingw-w64 linked with msvcrt.dll
  (`_UCRT` undefined), besides those before version 12. The reason given is
  now the true one: the fma() of mingw-w64's own math library is not
  correctly rounded and CORE-MATH computes with it, and its round() depends on
  the rounding direction. It is not fegetround(), which reads the x87 control
  word. The 32-bit x86 refusal before 11 names the fma() of that mingw-w64.
- tests/core_math.cpp: check first the fma() and round() of the C library in
  the four rounding directions, against values computed with exact rational
  arithmetic.
- .github/workflows/windows.yml: the refused MinGW-w64 jobs look for the
  reason in the message; MSYS2 MINGW64 (GCC and Clang) is checked refused.
- CMakeLists.txt, configure.ac, configure, meson.build, doc/three-builds.md,
  doc/continuous-integration.md, doc/tests.md, manual/v5/gaol.tex,
  gaol/gaol_fpu_fenv.h: the same reason in the comments and the docs.

## For doc/differences.md

Replace the item at line 37 ("mingw-w64 is no longer refused for its
version...") with:

```
  - mingw-w64 is no longer refused for its version alone. Both former reasons
    are gone: the hyperbolic functions of its math library, which GAOL no
    longer uses, and the cost of its `fesetround()`, which GAOL no longer calls
    for its elementary functions. MinGW-w64 GCC 12 to 15 are built and tested
    again on 32-bit x86, and GCC 14 and 15, MSYS2 UCRT64 and CLANG64 on x86-64.
    Still refused are the mingw-w64 whose `fma()` and `round()` are those of
    mingw-w64's own math library: on x86-64, before version 12 and those linked
    with `msvcrt.dll` rather than the UCRT (MSYS2 MINGW64, the cross compilers
    of Debian and Ubuntu); on 32-bit x86, before 11. That `fma()` is not
    correctly rounded, and CORE-MATH computes with it; that `round()` depends
    on the rounding direction (`gaol/gaol_config.h`).
```

and, in the next item, "`tests/core_math.cpp` checks the bounds against
CORE-MATH..." becomes "`tests/core_math.cpp` checks the `fma()` and `round()` of
the C library, then the bounds against CORE-MATH...".

## Where things are (scratchpad/tmp/06b)

- `mingw64-toolchain.cmake` is the x86-64 cross toolchain.
- `w32/mingw32-toolchain*.cmake` is the i686 one; the `-i386loader` variant
  uses `/usr/lib/wine/wine.i386`.
- `wine/` and `wine32/` are the wine prefixes.
- `guard/triples.py`, `guard/table.py`: the exact references of the guard.
- `w32/fma-*.txt`: the 800,000 fma results:
  - mingw-w64 11 i686 (Ubuntu);
  - MinGW-Builds rt_v10 and rt_v11 i686;
  - WinLibs mingw-w64 9 i686.
- `w32/roundtest.c` and its runs.
- `winlibs/`, `mgb/`, `mgb13/`: the downloaded toolchains' libs and headers.
- `msys2/`: the MSYS2 headers packages (MINGW64, UCRT64, CLANG64).
- `ci/run119-*.log`, `ci/run122-*.log`: the CI logs of the accepted WinLibs
  x86.
- `teeth/`: the proofs.
- `refused/`: the refused cmake build.
- `probe/`: the preprocessing checks.
- `linux/`: the Linux validation.
- `pdf/`: the manual.

## Review round

The review (`scratchpad/reports/review-06b.md`, verdict "changes") found one
blocking defect, B1: outside x86-64, the refusal still gave false reasons. It
also made nine non-blocking remarks. B1 is fixed. So are non-blocking 1 to 7;
8 and 9 are open questions. There is no new test (the only test change is a
comment), and x86-64 behaviour is unchanged. Nothing pushed.

### Commits of this round

- e5114ad gaol_config.h: the mingw-w64 refusal outside x86-64 split, 32-bit x86 for the fma() of WinLibs, ARM as not tested; Cygwin left out
- 14189de The refused MinGW-w64 in the builds, the docs and the manual: round() wrong on x64 only, the fma() of WinLibs on x86, ARM not tested
- ce5be89 windows.yml: the x86 refusal message, the toolchains of Chocolatey named right, and _UCRT shown when defined

`check_branch todo-06b-mingw-msvcrt-refused`: OK (10 commits, 12 files).

### B1: false reasons outside x86-64 (fixed)

**(a) The ARM message.** The second block was `!defined(__x86_64__) && < 11`.
It is now split in `gaol/gaol_config.h` into:

- **x86-64:** the same condition and the same message as before.
- **`defined(__i386__) && < 11`:**
  - The message says: "for 32-bit x86 before version 11, as the mingw-w64 9
    of WinLibs (GCC 11), whose fma(), which CORE-MATH computes with, rounds
    each of its partial sums to a double: the bounds of the elementary
    functions did not enclose the exact values".
  - It recommends MinGW-Builds GCC 14 or 15 (`--x86`, UCRT) or Visual Studio
    (non-blocking 5).
- **The rest (ARM) `< 11`:**
  - The message says: "GAOL v5 was not tested with it, and the round() of
    mingw-w64's own math library, computed in doubles, depends on the
    rounding direction. Build GAOL with Visual Studio". There is no `--x86`
    any more.
  - The CI has no ARM64 MinGW job, so Visual Studio is the only tested
    recommendation.

What I checked in the mingw-w64 sources (`tmp/06b/src`, v9 to v13):

- **Where the functions come from.**
  - `math/fma.c` and `math/round.c` are in the common `src_libmingwex` of v9,
    v10 and v11 (`Makefile.am`).
  - The ARM lists (`src_libmingwexarm32/arm64`) override neither.
  - `round.c` is byte-identical from v9 to v13.
- **ARM `fma()`.**
  - It is `fmadd` on ARM64, a fused instruction.
  - On 32-bit ARM it is `fmacd`. Assembled with clang-18
    (`tmp/06b/arm/a.s`), `fmacd` gives the same encoding as `vmla.f64`
    (`ee01 0b02`). `vfma.f64` is `eea1 0b02`. So on 32-bit ARM mingw-w64's
    `fma()` is not fused, contrary to what the review implies.
  - This does not matter in practice: Clang, the only mingw-w64 compiler for
    32-bit ARM, is refused there by the `#error` just before.
- **ARM64 `round()`.** Clang 18 for `aarch64-w64-windows-gnu` compiles
  `round()` to `frinta` and `fma()` to `fmadd`, even at `-O0` and with
  `-frounding-math` (`tmp/06b/arm/r.c`). So on ARM64 with Clang the library
  `round()` is not even called. The ARM refusal is therefore described as
  "not tested", with the `round()` fact given as a hazard, and not as a known
  failure.

**(b) Scope of `round()`.** The claim that `round()` depends on the rounding
direction is now limited to x64 in:

- the manual;
- `doc/three-builds.md`;
- `doc/continuous-integration.md` (it already was);
- `CMakeLists.txt`;
- `configure.ac`, `configure` and `meson.build`;
- `doc/tests.md` ("on x64").

For 32-bit x86, `gaol_config.h` states that `round()` is right, bar the sign
of a zero result when rounding downward. This holds in the four builds looked
at:

| Build | Evidence |
|---|---|
| Ubuntu mingw-w64 11 | tested under wine32 in the first round |
| WinLibs mingw-w64 9 | tested under wine32, and disassembled: `fldl; fsubl; fcomip` |
| MinGW-Builds rt_v10 | disassembled (`mgb/ex`): `frndint` with RC set upward, then `fsub` and `fcomip` in x87 registers, with no store to a double |
| MinGW-Builds rt_v11 | disassembled (`mgb13/ex`), same as rt_v10 |

**(c) "Whose fma() and round() are those of its own math library".** Every
place that described the refused set so now says "whose `fma()` or `round()`
is wrong". Each architecture then gets its own reason:

- on x64, mingw-w64's own functions, computed in doubles;
- on 32-bit x86, the `fma()` of WinLibs mingw-w64 9, which rounds each
  partial sum to a double;
- on ARM, not tested.

Where the text has room, it also says that the accepted 32-bit toolchains use
mingw-w64's own x87 `fma()`. That `fma()` keeps its partial sums in extended
precision and passes the tests, but it is not correctly rounded. The places
are:

- the `gaol_config.h` comment;
- `CMakeLists.txt`;
- `doc/three-builds.md` (the last paragraph);
- `doc/continuous-integration.md` (the "tested" column);
- the `windows.yml` comment of the `mingw` matrix;
- the comment in `tests/core_math.cpp`.

The soundness of those toolchains is still open question 1.

### Non-blocking remarks

1. **Fixed.** `define _UCRT$` became `define _UCRT( |$)`.
   - Checked with the MSYS2 UCRT64 headers: `-dM` prints `#define _UCRT `,
     with a trailing space. The old pattern printed nothing; the new one
     prints the line.
   - It also matches `-D_UCRT` (`#define _UCRT 1`, from `-mcrtdll=ucrt`).
     It does not match `_UCRTBASE`-like names.
2. **Fixed.** The `windows.yml` comment now names the toolchains correctly:
   - GCC 12.2.0 and 13.2.0 of Chocolatey on x86-64 are MinGW-Builds of
     mingw-w64 11 (UCRT);
   - GCC 11.2.0 is WinLibs mingw-w64 9 (msvcrt.dll);
   - on 32-bit x86, GCC 11.2.0 is WinLibs mingw-w64 9, whose `fma()` rounds
     each partial sum to a double.

   The Chocolatey pages in `tmp/06b/choco-*.html` give the assets: the ones
   for 12.2.0 and 13.2.0 have `ucrt` in their names, and the one for 11.2.0 is
   `winlibs-x86_64-posix-seh-gcc-11.2.0-mingw-w64-9.0.0-r1.7z`.
3. **Fixed.** "which are right" (ucrtbase) became "which pass the tests", in
   `gaol_config.h`, `CMakeLists.txt`, the manual and `three-builds.md`.
4. **Fixed.** The whole mingw-w64 block now sits under
   `#if defined(__MINGW32__) && defined(__MINGW64_VERSION_MAJOR)`.
   - Clang for `x86_64-pc-windows-cygnus` does not define `__MINGW32__`.
   - With `<_mingw.h>` of mingw-w64 11 or 9 included first, it was refused at
     878c808 and is accepted now.
   - `aarch64-w64-windows-gnu` and all the real mingw-w64 targets do define
     `__MINGW32__`.
5. **Fixed.** The x86 `#error` recommends MinGW-Builds GCC 14 or 15 (UCRT)
   and Visual Studio.
6. **Fixed.** `doc/continuous-integration.md` now says
   "`round(0x1.fffffffffffffp-2)` is 1 except rounding upward".
7. **Fixed.** The MSYS2 MINGW64 objects are described as those "whose headers
   say mingw-w64 14", not "mingw-w64 13".
8. **Open question.** ARM64 mingw-w64 linked with msvcrt.dll, or ARM64 11
   with UCRT: see open question 7 below.
9. **Open question.** A mingw-w64 git snapshot reporting 12: see open
   question 5 of the first round. It is unchanged, and the guard catches it.

### Validation of this round

- **Preprocessor matrix.** `tmp/06b/probe2/run.sh <source root>`
  preprocesses `gaol/gaol_config.h` with `-fsyntax-only`. It uses the real
  headers of each toolchain, which are merged for Debian. Results:
  - On this branch, all 26 cases are as expected (`out-branch.txt`):
    - **Refused with the x64 message:** Ubuntu 11 x64 in C++ and in C;
      Ubuntu 11 x64 with `-D__MSVCRT_VERSION__=0xE00`; MSYS2 r380 MINGW64
      with GCC and with clang-18; MSYS2 UCRT64 with
      `-D__MSVCRT_VERSION__=0x700`; Debian 12 msvcrt; Debian 14 msvcrt.
    - **Accepted:** MSYS2 MINGW64 with `-D_UCRT`; MSYS2 UCRT64 in C++ and
      in C; MSYS2 CLANG64; Debian 14 ucrt64.
    - **32-bit x86:** Ubuntu i686 11 and MinGW-Builds 12.2 rt_v10 i686 are
      accepted. WinLibs 9 i686, with GCC and with clang, is refused with the
      new x86 message.
    - **ARM:** aarch64 clang with the WinLibs 9 headers, in C++ and in C, is
      refused with the ARM message. armv7 clang is refused with the Clang
      32-bit ARM message and the ARM message. aarch64 with the Ubuntu 11 or
      MSYS2 CLANG64 headers is accepted.
    - **Cygwin x64, with the Ubuntu 11 or the WinLibs 9 `<_mingw.h>`
      first:** accepted.
    - **Linux GCC 13 and clang-18:** accepted.
  - On 878c808 (`out-878c808.txt`), only 7 cases differ. WinLibs 9 x86 and
    the ARM cases got the old message, whose fma reason is false on ARM, and
    the two Cygwin cases were refused.
- **The refusal on a real build.** CMake with `tmp/06b/mingw64-toolchain.cmake`
  (Ubuntu mingw-w64 11 x64), in `build/06b-w64refused2`:
  - configure passes, and the build stops on the x86-64 `#error`
    (`tmp/06b/round2/w64refused-build.log`);
  - both `mingw-refused` x64 grep strings match the log, and so does the
    `msys2-refused` one;
  - the new x86 grep string matches the GCC i686 output with the WinLibs 9
    headers (`round2/winlibs32.log`);
  - the msys2 string matches clang-18 with the MINGW64 headers
    (`round2/clang-mingw64.log`).
- **`configure`.** Regenerated with autoconf 2.72 of `scratchpad/tools/bin`
  from the new `configure.ac`: identical, byte for byte, to the committed one
  (`tmp/06b/regen2`).
- **`windows.yml`** parses with PyYAML. Its jobs are `visual-studio`,
  `mingw`, `msys2`, `mingw-refused` and `msys2-refused`.
- **The manual compiles:** 126 pages (`tmp/06b/round2/pdf`). Its one overfull
  box, now at lines 508–512 (the `package` item), predates the change.
- **Linux sse** (GCC 13.3, Release, examples): full rebuild and `ctest -j1`.
  49 of 49 pass, with `intervalf` and `interval2f` skipped as usual
  (`tmp/06b/round2/linux-ctest.log`). The rebuild was of the tree as
  committed at ce5be89.
- **Teeth.** No new test this round; the only change to
  `tests/core_math.cpp` is a comment. The guard's teeth were shown in the
  first round and again by the reviewer (21/24 and 6/28 failures with
  mingw-w64 11 x64 under wine). On x86-64 the refusal condition is unchanged,
  apart from the added `__MINGW32__`, which every mingw-w64 compiler defines.

### Open questions added by this round

7. **ARM64 mingw-w64 linked with msvcrt.dll, and ARM64 mingw-w64 11 (UCRT),
   are accepted** (review, non-blocking 8). Their `round()` is mingw-w64's,
   computed in doubles. But Clang 18, the usual compiler there (llvm-mingw),
   compiles `round()` to `frinta` and `fma()` to `fmadd` even at `-O0`. So
   GAOL and CORE-MATH do not call these functions, and `tests/core_math.cpp`
   would catch a toolchain that does. Refusing ARM64 before 12 or without
   `_UCRT`, as on x86-64, would be the analogue. It is outside the x64 scope
   of the request, so it is not done here.
8. **The ARM refusal before 11 is a carry-over.** The old condition
   `!__x86_64__ && < 11` was aimed at 32-bit x86. Nothing tested shows a
   wrong bound on ARM64 mingw-w64 before 11. Keeping the refusal (as now) is
   cautious; dropping it is the maintainer's call.
9. **The 32-bit x86 refusal is by version (< 11), but its evidence is one
   build**: WinLibs 9, whose `fma()` was compiled without optimization.
   Another i686 mingw-w64 before 11 built with optimization would have the
   x87 `fma()` of the accepted 11. The message says "as the mingw-w64 9 of
   WinLibs" for that reason.

### Ledger text, replacing that of the first round

**For TODO.md** (in French):

- **06b** (fait) : sur x86-64, `gaol/gaol_config.h` refuse tout mingw-w64 lié à `msvcrt.dll` (`!defined(_UCRT)`) en plus des
  versions avant 12, avec la vraie raison : `fma()` (non correctement arrondi) et `round()` (dépend du sens d'arrondi) de la
  bibliothèque mathématique de mingw-w64, calculés en doubles, que CORE-MATH et GAOL appellent ; depuis 12, une chaîne liée à
  l'UCRT prend ceux de `ucrtbase.dll`. Le `fegetround()` de mingw-w64 lit le mot de contrôle x87 : pas d'état propre. Sur x86
  32 bits, le refus avant 11 vient du `fma()` de WinLibs GCC 11.2 (mingw-w64 9), compilé sans optimisation, qui arrondit
  chaque somme partielle à un double ; `round()` y est juste (x87). Sur ARM, le refus avant 11 est gardé, « non testé ».
  Cygwin n'est plus visé (`__MINGW32__`). `tests/core_math.cpp` vérifie d'abord `fma()` et `round()` de la bibliothèque C
  (21 et 6 échecs avec mingw-w64 11 x64 sous wine) ; la CI refuse aussi MSYS2 MINGW64 (GCC et Clang). Questions ouvertes :
  le `fma()` x87 des MinGW-w64 x86 acceptés (mingw-w64 11) n'est pas correctement arrondi (125 résultats sur 400 000 à un
  double près ; tous les tests passent) ; `GAOL_RND_MINGW_FENV_ONLY` ; mingw-w64 ARM64 (msvcrt, 11, refus avant 11).

**For ChangeLog:**

- gaol/gaol_config.h: refuse on x86-64 every mingw-w64 linked with msvcrt.dll
  (`_UCRT` undefined), besides those before version 12. The reason given is
  now the true one: the fma() and round() of mingw-w64's own math library,
  computed in doubles. That fma() is not correctly rounded, and CORE-MATH
  computes with it; that round() depends on the rounding direction. It is not
  fegetround(), which reads the x87 control word. On 32-bit x86, the refusal
  before 11 names the fma() of the mingw-w64 9 of WinLibs, which rounds each
  of its partial sums to a double. On ARM, the refusal before 11 is kept, as
  not tested. Cygwin (no `__MINGW32__`) is no longer caught by these checks.
- tests/core_math.cpp: check first the fma() and round() of the C library in
  the four rounding directions, against values computed with exact rational
  arithmetic.
- .github/workflows/windows.yml: the refused MinGW-w64 jobs look for the
  reason in the message; MSYS2 MINGW64 (GCC and Clang) is checked refused.
- CMakeLists.txt, configure.ac, configure, meson.build, doc/three-builds.md,
  doc/continuous-integration.md, doc/tests.md, manual/v5/gaol.tex,
  gaol/gaol_fpu_fenv.h: the same reasons, per architecture, in the comments
  and the docs.

**For doc/differences.md.** Replace the item at line 37 ("mingw-w64 is no
longer refused for its version...") with:

```
  - mingw-w64 is no longer refused for its version alone. Both former reasons
    are gone: the hyperbolic functions of its math library, which GAOL no
    longer uses, and the cost of its `fesetround()`, which GAOL no longer calls
    for its elementary functions. MinGW-w64 GCC 12 to 15 are built and tested
    again on 32-bit x86, and GCC 14 and 15, MSYS2 UCRT64 and CLANG64 on x86-64.
    Still refused are the mingw-w64 whose `fma()` or `round()` is wrong
    (`gaol/gaol_config.h`). On x86-64, those before version 12 and those linked
    with `msvcrt.dll` rather than the UCRT (MSYS2 MINGW64, the cross compilers
    of Debian and Ubuntu) give the programs the `fma()` and `round()` of
    mingw-w64's own math library, computed in doubles: that `fma()` is not
    correctly rounded, and CORE-MATH computes with it; that `round()` depends
    on the rounding direction. On 32-bit x86, mingw-w64 before 11 is refused,
    as the mingw-w64 9 of WinLibs (GCC 11), whose `fma()` rounds each of its
    partial sums to a double. On ARM, mingw-w64 before 11 stays refused, not
    tested.
```

In the next item, "`tests/core_math.cpp` checks the bounds against
CORE-MATH..." becomes "`tests/core_math.cpp` checks the `fma()` and `round()` of
the C library, then the bounds against CORE-MATH...".
