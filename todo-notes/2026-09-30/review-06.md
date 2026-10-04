# Review of point 6: `todo-06-fegetround-mxcsr`

Branch `todo-06-fegetround-mxcsr` at `921359f`, against `origin/configure-clean` (`0f2963c`, already merged in by `ece2f2c`).
Detached worktree `scratchpad/wt/review-06`. Logs and programs are in `scratchpad/tmp/review-06/`, builds in `scratchpad/build/review-06-*`.

Commits reviewed:

- 1d97bab CORE-MATH's fegetround() reads MXCSR on x86-64: pow missed a subnormal result with the x87 unit to nearest
- 93fdc4f Document fegetround() read from MXCSR in 3rd/README.md and pow with a subnormal result in doc/tests.md
- ece2f2c Merge origin/configure-clean into todo-06-fegetround-mxcsr
- a1ddacb Comments of round_upward_if_needed() and core_math_port.h: CORE-MATH reads MXCSR on x86-64, mingw-w64 too
- 921359f cbrt.c maps mingw-w64's fegetround() to 0..3: nth_root(x, 3) missed the hardest cube roots on x64

## Verdict

**Approve.** I found no blocking defect. Both fixes are correct, and both regression tests fail without their fix and pass with it (I reproduced both). The references hold when recomputed independently. sse, fpu and clang build and pass on Linux. The MinGW-w64 x64 cross build passes under wine. The non-blocking points below are about wording, the order of merges, and a commit identity the orchestrator should decide on.

## Checks run

| What | Outcome |
|---|---|
| `check_branch todo-06-fegetround-mxcsr` | OK: 5 commits, 7 files, messages one line each, no ledger file, no whitespace error |
| sse (GCC 13.3), full build and `ctest -j1` | 49/49 passed (intervalf, interval2f skipped by design); no compiler warning |
| fpu (`-DGAOL_SIMD=OFF`), full build and ctest | 49/49 passed |
| clang-18, full build and ctest | 49/49 passed; only the existing `y.tab.c` warning (`gaol_nerrs`) |
| Debug with ASan and UBSan: `gaol_test_core_math`, `gaol_test_rounding_direction` | both exit 0 (1,472,280 and 16963 checks, 0 failed), no sanitizer report |
| Teeth of the MXCSR fix (Linux, sse) | only `gaol/core_math_port.h` taken from `origin/configure-clean`: `gaol_test_rounding_direction` exits 1, **16963 checks, 9 failed**, all in "to nearest, and upward for SSE", e.g. `[0x0.001539f0d791ap-1022, 0x0.001539f0d791bp-1022] rather than [0x0.001539f0d791bp-1022, 0x0.001539f0d791cp-1022]`. After `git checkout` and a rebuild: exit 0, 16963 checks, 0 failed |
| Teeth of the cbrt.c fix (mingw-w64 11 x64, wine 9.0) | exported copy (`git archive HEAD`), with the refusal in `gaol_config.h` relaxed in the copy only (`< 12` changed to `< 11`), `tmp/06b/mingw64-toolchain.cmake`, Release, `tmp/06b/fma_hw.o` (FMA3) linked to remove mingw-w64 11's inexact `fma`. With the fix: `gaol_test_core_math.exe` exits 0, **1,472,280 checks, 0 failed**. With only `cbrt.c` taken from `origin/configure-clean`: exit 1, **210 failed** (the three new checks fail 70/70 each), e.g. `nth_root([0x1.3a9ccd7f022dbp+0], 3) = [0x1.1236160ba9b92p+0, 0x1.1236160ba9b93p+0]` |
| Same cross build, other tests | `gaol_test_rounding_direction.exe`: 16953 checks, 0 failed (the subnormal `pow` checks pass on mingw-w64 x64). `gaol_test_elementary.exe`: 2 failed, the known `round()` of mingw-w64 at ±0x1.fffffffffffffp-2 (report 06b), not related to this branch |
| References of the cbrt test | recomputed with exact rationals (`tmp/review-06/ref.py`): for all seven, below³ < x < above³, the two are adjacent doubles, and ((below+above)/2)³ > x (so RN = below); the cube roots lie 1.9e-17 to 6.7e-16 ulp above `below` |
| References of the subnormal `pow` test | mpmath at 1000 bits: 1458662373659.2900063…, 567863.2560668449…, 1.3000000000970768… times 2^-1074; floor and ceiling equal the `lo`, `hi` of the test |
| MinGW x86 CI jobs (no wine32 here) | the i686 `fma` of mingw-w64 (Dekker products summed on the x87 unit, the emulation of report 06b) linked under CORE-MATH's `cbrt.c` on x86-64: **0 of 280** hard-case results wrong in the four directions. The x64 SSE Dekker `fma` gives 5 of 280 wrong (`-0x1.fe18a044a5501p+1·8^k` downward), matching the author's report. So the new check should pass on the accepted MinGW x86 toolchains (`tmp/review-06/cbrt_x87fma.c`) |
| Macros | `clang-18 --target=x86_64-w64-mingw32` defines `__x86_64__` and `__WIN32__` (MSYS2 CLANG64 takes the fixed branch); `--target=x86_64-pc-windows-msvc` defines `__x86_64__` and `_M_X64` but no `__WIN32__` (clang-cl, see non-blocking 6) |
| Object code | in the sse `libgaol.a`, no CORE-MATH object references `fegetround` (only `gaol_common.cpp.o`, GAOL's C++); `pow.c.o` has 4 `stmxcsr`, `cos.c.o`, `tan.c.o` and `cbrt.c.o` have 1 each |

## Portability reasoning (platforms not run here)

- **Visual C++ x64**: `_M_X64` enables the block, and `<xmmintrin.h>`/`_mm_getcsr`/`_MM_ROUND_*` exist in C. `cbrt.c` takes its generic branch (`fegetround()` is now GAOL's MXCSR read, which gives the UCRT's `FE_*`, and the switch maps them), so the result is the same as before, since the UCRT's `fegetround()` reads MXCSR on x64. `static inline` in C is already used by the 32-bit Windows block. The CI will confirm it compiles.
- **Visual C++ x86 and arm64, MinGW x86, Debian i386, armhf, s390x, ppc64le, riscv64, macOS arm64**: the new block is excluded by its guard, and `cbrt.c` is unchanged there. On i386, `round_upward_if_needed()` reads both units and sets both (the `#elif` of `gaol_fpu.h`, since `GAOL_RND_PROBE` is undefined for `__i386__`/`_M_IX86`), so the exclusion is sound.
- **MinGW-w64 and MSYS2 UCRT64/CLANG64 x64**: the cbrt fix applies, and was run under wine with GCC.
- **macOS x86_64, Alpine musl**: `FE_*` are 0/0x400/0x800/0xc00. The switch returns the libc's own constants, and musl already reads MXCSR.
- **Minimum CMake 3.14, meson 0.53, autotools**: no build file, source list or installed header changed. The port header was already force-included by all three builds, only into the CORE-MATH C sources (`CMakeLists.txt:462-464`, `gaol/meson.build:38-42`, `3rd/Makefile.am:54`), so the `fegetround` macro cannot reach C++.
- **Sanitizers**: the new test code does only exact `ldexp` and comparisons.

## Blocking findings

None.

## Non-blocking findings

1. **Commit identity (for the orchestrator).** `ece2f2c`, `a1ddacb` and `921359f` are authored and committed by `Claude <noreply@anthropic.com>`, while `1d97bab` and `93fdc4f`, and all the history of `origin/configure-clean`, are `Jordan08`. The messages credit nobody, and `check_branch` only checks messages. But pushed as they are, these would be the first commits on origin showing Claude as author, which goes against the spirit of "nothing that credits Claude". The commits are unpushed (`origin/todo-06-fegetround-mxcsr` is still `93fdc4f`), so the orchestrator can re-author them before the push if the maintainer wants that. The same holds for every branch of this session: `git config user.name` is `Claude` in the shared `.git`.
2. **The stated reason for refusing mingw-w64 before 12 is now untrue on x86-64.** The texts are the `#error` and the comment of `gaol/gaol_config.h:221-238`, `doc/three-builds.md:136`, `doc/continuous-integration.md:82` and `manual/v5/gaol.tex:331-334`. They say CORE-MATH reads `fegetround()` to know the direction; after this branch it reads MXCSR on x86-64. Report 06b shows the reason was already wrong before (mingw-w64 v9 to v12 share the same `fnstcw` `fegetround()`), so this is not introduced here. `todo-06b-mingw-msvcrt-refused`, built on this branch, rewrites these texts. Merge it right after this one, or together with it.
3. **`examples/examples.md` table 5.1 item 3 and Appendix B item 3 still describe the bug as open.** They say "GAOL's own test lists that state (`tests/rounding_direction.cpp:104`) but no subnormal `pow`", which is false after this branch. Point 39 (`todo-39-goldstein-price`) does not touch item 3, and other merged fixes (e.g. item 4, point 5) left the table unmarked too, so the bookkeeping pull request should mark item 3 **Fixed**.
4. **`3rd/README.md`, "How the changes are checked".** The first bullet says each function whose file changed was compared bit for bit with the pristine upstream over 40 million arguments. `cbrt.c` is now such a file, but its changed line is compiled only with mingw-w64 on x86-64, where the result differs from upstream by design. Suggest one sentence: change 6 is checked by `tests/core_math.cpp` in the MinGW/MSYS2 x64 jobs, and was shown under wine with mingw-w64 11 (210 checks failed without it).
5. **`gaol/gaol_fpu.h` comment of `round_upward_if_needed()`**: "it [the x87 unit] computes none of GAOL's doubles". With mingw-w64 on x86-64, `ldexp()` is x87 `fscale` (`math/x86/ldexp.c` of v11 to v13, in `src_libmingwex_x86`, which `src_libmingwex64` includes; `x86_64-w64-mingw32-objdump -d libmingwex.a` shows `fldl`/`fscale` in `<ldexp>`). `gaol_interval.cpp` (`odd_significand`, `difference_of_squares`, `pi_lo_*`) and `exp2m1.c` (`-1.0 + ldexp(1.0, i)`) call it, but only for exact results or for comparisons a subnormal cannot pass. So the conclusion (the bounds do not depend on the x87 direction) holds, but the premise is not literally true. Suggest "computes none of GAOL's doubles but exact ones".
6. **clang-cl x64** (author's open question A): the three `get_rounding_mode()` still compare `RC >> 3` with the UCRT's `FE_*`. It is not in the CI, not documented as supported, and not refused by `gaol_config.h`. Keep it in the TODO remarks.
7. **Where the new cbrt checks can fail.** They can fail only with mingw-w64 on x86-64: on Linux the fixed line is not compiled, so their teeth exist only in the MinGW/MSYS2 x64 jobs. The direct `cbrt` check also fails (5 of 70) with any x64 mingw-w64 whose `fma` is libmingwex's. That is mingw-w64 before 12, and 12 or later linked with msvcrt (MSYS2 MINGW64, the Debian/Ubuntu/Fedora cross compilers), which `gaol_config.h` accepts today but no CI job uses. The failure there is real (CORE-MATH's `cbrt` is then wrong in a directed rounding), not flakiness, and `todo-06b-mingw-msvcrt-refused` refuses those toolchains.
8. **What was observed with which toolchain.** The failure described in change 6 ("with MinGW-w64 and MSYS2 on x64") was observed with mingw-w64 11 (GCC 13) under wine. For MSYS2 it is inferred from the same headers and macros, which is sound, but it was not run.
9. **Overlap with `todo-31-upstream-patches`.** `3rd/README.md` there rewrites "What differs from upstream" and proposes upstream a more general patch 6 (read the RC field of MXCSR whatever `FE_*` are, in `cbrt`, `rsqrt`, `asinpi`), where the tree carries the one-line `mode = fegetround();`. Expect a conflict on the numbering and on "the six changes". The two are consistent, because in the tree `fegetround()` is already the MXCSR read.
10. **Wording in `core_math_port.h`.** "as the get_rounding_mode() of cbrt.c, rsqrt.c and asinpi.c does" holds for GCC and Clang (`__x86_64__`), not for Visual C++ x64, where these call the generic `fegetround()`, now GAOL's MXCSR read. The effect is the same, so this is a precision of wording only.

## Sanitizers

Debug build with `-fsanitize=address,undefined -fno-omit-frame-pointer` (GCC 13.3), `UBSAN_OPTIONS=halt_on_error=1`: `gaol_test_core_math` exits 0, 1,472,280 checks, 0 failed (the three hard-case checks 70/70 pass); `gaol_test_rounding_direction` exits 0, 16963 checks, 0 failed; no sanitizer report.

## Cleanup

The worktree `scratchpad/wt/review-06` was removed at the end (`git worktree remove --force`). Nothing was committed or pushed, and the branch is unchanged.
