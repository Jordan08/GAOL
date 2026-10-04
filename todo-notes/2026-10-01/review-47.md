# Review of `todo-47-clang-cl-rounding-mode` (point 47)

Reviewed head: `118c852`, against `origin/configure-clean` (`6ef2387`).
Worktree: `scratchpad/wt/review-47` (detached, removed at the end). Nothing was
committed or pushed. Work files are in `scratchpad/tmp/review-47/`:

- `emu/`: the independent emulation.
- `pp/`: the preprocessor table.
- `xwin/win/`: the real MSVC 14.44 STL/CRT and the UCRT 10.0.26100 headers and libs.
- `clangcl-toolchain.cmake`.
- `teeth/`: the clang-cl programs.
- Logs: `*-clangcl*.log`, `rd-clangcl.txt`.

The disk is at 99%, so the `.lib` files, the builds and the executables were
deleted. `xwin/win/inc` keeps the MSVC and UCRT 10.0.26100 headers. The libs
come back from the VS manifest:

- `Microsoft.VC.14.44.17.14.CRT.x64.Desktop.base.vsix`: `lib/x64`;
- Win11SDK_10.0.26100's `Universal CRT Headers Libraries and Sources` MSI, cab
  `a1e2a83a...`: `libucrt.lib`;
- Win11SDK_10.0.26100's `store_libs`: `kernel32.lib`.

**Verdict: changes requested.** There is one blocking finding. It concerns the
new CI job and what the report says that job will show. It is not about the
one-word fix, which is correct.

What is right:

- The one-word change in the three files is correct for every target. I checked
  that independently.
- GAOL, CORE-MATH and all the tests build with the real clang-cl. They also pass
  with it, except `rounding_direction`: see B1.

## Blocking

### B1. The clang-cl jobs will stay red after open question 3 is fixed: a real soundness bug of GAOL with clang-cl, not reported

Where:

- The cause is `gaol/gaol_port.h:145`, `#define GAOL_INFINITY HUGE_VAL`.
- The report is `reports/47.md`, § "What the CI must confirm", item 3. It says
  `RUN_TESTS` passes except `rounding_direction`, "which fails on every job until
  open question 3 is fixed".
- The comments of `windows.yml` and `doc/continuous-integration.md` present the
  job as the check of clang-cl.

**The bug.** The UCRT of the CI defines `HUGE_VAL` as `((double)INFINITY)`. In
Windows SDK 10.0.26100, `INFINITY` is `((float)(_HUGE_ENUF))`, that is
`(float)1e+300`. The legacy form is `(float)(1e300*1e300)` (checked in the SDK's
own `corecrt_math.h`, extracted from the Win11SDK_10.0.26100 package). Under
`/fp:strict`, clang-cl does not fold that inexact conversion. It emits
`vcvtsd2ss` + `vcvtss2sd` at run time (`llvm-objdump` of `double f(){return
HUGE_VAL;}`). Rounding downward or toward zero, the conversion gives FLT_MAX,
0x1.fffffep+127, and raises overflow and inexact. Visual C++ folds it, and GCC,
Clang and MinGW use `__builtin_huge_val()`. So only clang-cl is affected.

**The effect.** GAOL built by clang-cl, called while the rounding direction is
downward or toward zero, does not enclose. This is `teeth/uni.cpp`, built with
clang-cl 18 and GAOL's flags and run under wine:

```
fesetround(FE_DOWNWARD); gaol::interval u; ... interval(1.0) / interval(-1.0, 1.0)
interval() = [-0x1.fffffe0000000p+127, 0x1.fffffe0000000p+127], contains 1e300: 0
[1] / [-1, 1] = [-0x1.fffffe0000000p+127, 0x1.fffffe0000000p+127]
```

**How it was shown.** GAOL and every test were built with clang-cl 18 in
Release and in Debug. The build used:

- the real MSVC 14.44 STL/CRT;
- the UCRT 10.0.26100 headers and static `libucrt.lib`, all Microsoft code,
  downloaded from the VS manifest;
- `lld-link` and `/MT`.

The tests ran under wine. `rounding_direction` fails 38 checks. Only one of them
is the check of open question 3, "the other outputs". The other 37 are:

- `interval()`: 12 failures;
- `operator/` by an interval containing 0: 12 failures;
- `pow(x,n)` from the rounded products: 12 failures, with `[-nan(ind), nan]`
  rather than `[0x1.fffffffffffffp+1023, inf]`;
- `interval::emptyset()` raises no exception flag ("flags 5 raised").

The cause is confirmed: rebuilt with `HUGE_VAL` forced to `__builtin_huge_val()`
(`/FI tmp/review-47/hugeval.h`), `rounding_direction` fails only the one check
of open question 3.

**Fix.** Per the house rules, this is not point 47's bug to fix. But it has to
reach the maintainer before the PR is proposed. The PR presents the job as the
check of clang-cl.

1. Correct item 3 of "What the CI must confirm": `rounding_direction` will fail
   on the clang-cl jobs for this second reason too.
2. Add an open question, or a new TODO point, with the evidence above.
3. Mention it in the PR text.

A fix in its own point would let the job turn green once open question 3 is
fixed, for example:

```c
#if defined(__GNUC__) || defined(__clang__)
#  define GAOL_INFINITY __builtin_huge_val()
#else
#  define GAOL_INFINITY HUGE_VAL
#endif
```

`std::numeric_limits<double>::infinity()` also works. The comment there worries
about old libc++. `tests/rounding_direction.cpp` already has teeth for this under
clang-cl. Leave the choice to the maintainer.

## Checked and correct

- **Preprocessor logic** (`tmp/review-47/pp/`). `get_rounding_mode()` of each
  file was preprocessed before and after, with the real predefined macros of
  each target (`clang-18 --target=... -dM -E`, the real mingw-w64 GCCs) and the
  `FE_*` of each libc.
  - **Unchanged**:
    - Linux x86_64 with GCC and Clang, and i386;
    - musl;
    - macOS x86_64 and arm64;
    - mingw-w64 GCC x64 and x86;
    - clang x86_64/aarch64-w64-windows-gnu (MSYS2 CLANG64);
    - Cygwin (`x86_64-pc-cygwin`: `__CYGWIN__`, no `_WIN32`);
    - clang-cl x86 and arm64 (no `__x86_64__`);
    - Visual C++ x64/x86/arm64 (cl macros emulated: no `__x86_64__`, so
      `fegetround()` or FPCR).
  - **Changed**, with the new branch:
    - clang-cl x64 with modern `FE_*` → `>>5`;
    - clang-cl x64 with the legacy `FE_UPWARD` 0x100 → `lut[..>>13]`;
    - any clang for x86_64 with `_WIN32` but no `__WIN32__` (e.g.
      windows-itanium): changed as well, and right, since it uses the UCRT.
  - The lut is right whatever the values: it maps MXCSR RC 0..3 onto the four
    macros. `>>5` gives 0/0x100/0x200/0x300, which are the modern UCRT values.
- **The real UCRT values.** The UCRT 10.0.26100 `fenv.h` defines `FE_* = _RC_*`
  (`FE_DOWNWARD` 0x100, `FE_UPWARD` 0x200), as the author assumed. For the
  record:
  - The legacy UCRT 10.0.10240 (the VS component "Windows Universal CRT SDK")
    has `FE_UPWARD` 0x100.
  - Its `fesetround(FE_UPWARD)` really sets downward (MXCSR RC 0x2000, checked
    under wine). This is the pre-14393 swap CORE-MATH's comment refers to.
  - CMake and MSBuild take the latest SDK, so the CI gets 0x200.
- **Independent emulation** (`tmp/review-47/emu/`, different from the author's).
  - The three sources were compiled natively on Linux with Clang 18, with the
    UCRT `FE_*` put over glibc's and `-D_WIN32`, so no clang-cl and no wine. They
    were compared bit for bit with the Linux build, in the 4 directions:

    | function | args | cur: down / up | pat | pat, legacy 0x100 values (lut branch) |
    |---|---|---|---|---|
    | cbrt | 2,009,534 | 4,767 / 4,767 | 0 / 0 | 0 |
    | rsqrt | 2,006,294 | 0 / 1,026 | 0 | 0 |
    | asinpi | 2,240,000 | 1,022,537 / 1,022,551 | 0 | 0 |

    The cbrt arguments include the `wlist` × 8^k for all k, on both signs.
    Nearest and toward zero show no difference anywhere.
  - mpmath, 600 bits, on 400 differences per function and direction: the Linux
    and patched value is the correctly rounded, tightest one, and the cur value
    does not enclose, in all of them.
- **The real clang-cl.**
  - The actual `tests/core_math.cpp`, built by clang-cl 18 with the UCRT
    10.0.26100, passes: 1,474,378 checks, 0 failed, Release and Debug.
  - Linked with `configure-clean`'s three objects instead (`teeth/`), it fails
    26,558 checks. The same total as the author's mirrored run.
- **Object code.** Byte-identical `.text` before and after, for GCC 13 Linux and
  for mingw-w64 GCC 13, in the three files.
- **The new rsqrt test on other targets.** 0 of 4,092 wrong on:
  - Linux with `GAOL_U128_FORCE_EMULATION` (the Visual C++ and 32-bit path);
  - mingw-w64 x64 under wine.
- **The CI job.**
  - CMake 3.28 `CMakeDetermineCompilerId.cmake` sets `FRONTEND_VARIANT=MSVC` for
    a Visual Studio generator with a Clang that simulates MSVC. It prints `The
    <LANG> compiler identification is Clang <ver> with MSVC-like command-line`,
    so the 2-line grep is right.
  - `matrix.cfg.clang_cl` renders as `true` or as empty.
  - The YAML parses (15 entries).
  - `-T ClangCL` is passed to `find_package` and `fp_strict` too.
  - `MSVC` is true for clang-cl, which gives `/fp:strict`
    (`-frounding-math -ffp-exception-behavior=strict -ffp-contract=off`),
    `/arch:AVX2` and `/FI`. It gives no SSE2 intervals, no `GAOL_USING_ASM`, no
    `gaol_exact.c`, no refused/nodiscard compile tests, and passes the ARM32
    check.
  - Nothing in GAOL's C++ breaks clang-cl:
    - the `_MSC_VER` branches;
    - the GNU asm of `gaol_fpu_fenv.h`;
    - native `unsigned __int128`: no division anywhere, so no `__udivti3`;
    - `/Zc:strictStrings-`, which is only an "unused argument" warning.
  - The whole build links. `tests/fp_strict` needs the change made: with `NOT
    MSVC` it would fail for clang-cl.
  - What could not be checked here: MSBuild itself, and the presence of the
    ClangCL toolset in the `windows-2022` image. The toolset is expected there
    (components `VC.Llvm.Clang` and `VC.Llvm.ClangToolset`). VS 17.14 bundles
    Clang 19, which the STL 14.44 requires. Locally, clang 18 needed
    `_ALLOW_COMPILER_AND_STL_VERSION_MISMATCH`.
- **House rules.**
  - Author and committer are Jordan08, no trailers, `check_branch` OK.
  - The three builds are unaffected: no source, header, test or option was added.
  - `TODO.md`, `ChangeLog` and `doc/differences.md` are untouched. The manual
    is untouched, which is correct.

## Nonblocking

1. **The docs say "the upper bounds" did not enclose, which is incomplete.** For
   `nth_root(x, 3)` at negative x, the lower bound did not enclose either:
   - `cube_root()` takes `-cube_root_up(-a)`.
   - With clang-cl before the fix, `nth_root([-0x1.3a9ccd7f022dbp+0], 3)` =
     `[-0x1.1236160ba9b93p+0, ...]`, above the exact −1.0711377886270596615…
     (`teeth/neg.cpp`).
   - The raw `cr_cbrt` and `cr_asinpi` gave downward results above the exact
     value at negative arguments: the "D" column above.

   Reword these: 3rd/README.md item 7, the report, and the ChangeLog and
   differences.md suggestions. For example: "the bound of `nth_root(x, 3)`
   farther from zero, and the upper bounds of `rsqrt` and `asinpi`".
2. **The report says `recommended_tightest()` "could not see the defect". That is
   wrong.** With clang-cl before the fix, "rsqrt over an interval: the tightest
   bounds" fails 7,963 of 36,396 checks, and "asinpi over an interval" 16,317 of
   31,190. These counts come from the real clang-cl build, and are part of the
   author's own 26,558. `rsqrt_hard_cases()` is still worth having, because it
   checks against analytic values. Correct the justification.
3. **Commit c437e80's subject is 114 characters**, over the "about 100". For
   example: `cbrt.c, rsqrt.c, asinpi.c: take the Windows branch under _WIN32 too
   (clang-cl x64)`.
4. **3rd/README.md item 7 describes only the `>>5`, table and `>>3` branches.**
   It could say that other `FE_*` values take `fegetround()`, which is
   mingw-w64's path (item 6).
5. **Open question 1 (clang-cl not refused without `/fp:strict`) is more pressing
   now that clang-cl is a CI platform.** Confirmed with Clang 18:
   `-ffp-model=strict` defines `_M_FP_STRICT _M_FP_EXCEPT`, precise gives
   `_M_FP_PRECISE`, and fast gives `_M_FP_FAST`.
6. **Noise in the logs.** Each clang-cl build gives 11 "argument unused:
   `/Zc:strictStrings-`" warnings. `$<$<CXX_COMPILER_ID:MSVC>:...>` would avoid
   them. This is not this point's concern.
