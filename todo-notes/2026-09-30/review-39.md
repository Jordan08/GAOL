# Review of point 39: the Goldstein-Price function gets its +1 (`todo-39-goldstein-price`)

Branch head `969595e`, base `origin/configure-clean`. I reviewed three commits:
- `0e80c52`: example 16 evaluates the true function and checks it;
- `4ce59b6`: the manual;
- `969595e`: style, attribution and the time in ms.

I reviewed in a detached worktree (`$SCR/wt/review-39`), since removed. I built in `$SCR/build/review-39-*`. My scripts are in `$SCR/tmp/review-39/`.

**Verdict: approve.** I found no blocking defect. The numbers are right, and I derived them independently. The checks have teeth. Examples 03 and 16 pass in the sse, fpu, clang and preserve-rounding builds, and on the tree merged with the current `configure-clean`. The manual compiles. One thing must happen before any push: commit `969595e` has the author `Claude <noreply@anthropic.com>` (non-blocking item 1).

## Numbers, checked independently

### Natural extension

Script: `natext.py`. I evaluated the expression in exact rational interval arithmetic (Python `Fraction`), in the order of `goldstein_price()` in example 16. The program of the manual has the same order.

- `z = [-87881320, 147125080]`.
- The factors are `A = [-1524, 3076]` and `B = [-28570, 47830]`.
- The width is 235006400.
- Every bound, and every candidate product, is an integer. The largest in magnitude is 147125080, below 2^53. The comment "the largest is 147125080" is exact.
- f(0, -1) = [3, 3].
- The old function (without the +1) gives `[-56254330, 94177270]`, and 111 at (0, -1). The comment "111 at (0, -1), not 3" is right.

### True range

Script: `range.py`.

- **Expansion.** I expanded f exactly as an integer polynomial.
- **Edges.** On each of the four edges, I isolated the real roots of the derivative with Sturm sequences in exact rationals, then bisected them to 1e-45.
- **Interior.** I ran a rigorous branch and bound in exact rational interval arithmetic, using a centred expansion, a value test and a monotonicity test. It left **0 unresolved boxes**: no interior point beats the edge maximum.
- **Minimum.** A separate branch and bound (`rangemin.py`) proves f >= 3 - 1e-6 on the whole square. Also f(0, -1) = 3 exactly.
- **mpmath cross-check** (`mp.py`, 50 digits). It found the interior stationary values 3, 30, 35, 84, 99, 840, 980, 990 and 1155.

Results:
- M = 1015690.27179805890829884231208223310394647077, on the edge y = 2.
- x* = -1.73737253775830701152784568384260170940452952.
- These match 03 (`...0829884231208...`, within 2e-44) and the digits in 16, the manual and `examples.md`.
- The claim in 03 that an interval branch and bound confirms the maximum is now backed by mine. The author had not redone it.

### Ratio

235006400 / (M - 3) = 231.3767. This backs:
- "231 times wider" (16, the manual, `examples.md`);
- "231.4 times less" (16);
- the check `231 < ratio < 232`.

The old enclosure relative to the true range is 148.1, which `examples.md` rounds to "about 150".

### `range` in example 16

`textToInterval("[3, 1015690.2717980589082988423]")`:
- The decimal is 1.2e-20 **below** M.
- GAOL's lexer rounds outward exactly (the bracketing in `gaol_interval_lexer.lpp`), so the upper bound becomes 1015690.271798059, which is M + 8.7e-11.
- So `range` does enclose the true range, on every C library.

### The manual's program

I extracted it from `gaol.tex`, compiled it with `-std=c++11` and `-std=c++17` against the sse build, and ran it. It prints `z = [-87881320, 147125080]`, as the manual shows.

## Tests have teeth

The main new checks are the exact enclosure and f(0, -1) = 3. In my worktree I reverted only the fix: `pow(x + y + interval(1.0), 2)` became `pow(x + y, 2)`. Then I rebuilt `16_Goldstein_Price` (sse) and ran it:

```
z = [-56254330, 94177270]
FAILED: f(0, -1) = 3
FAILED: z is the natural extension of f
FAILED: the program of the manual gives the same enclosure
z is 148 times wider than the range of f
FAILED: z is 231 times wider than the range of f
exit 1
ctest -R 16_Goldstein_Price: 0% tests passed, 1 tests failed out of 1
```

After `git checkout examples/16_Goldstein_Price.cpp` and a rebuild, ctest reports 100% passed. As the author says, "z encloses the range" passes with the bug too. The other checks carry the teeth.

## Builds and runs

All commands went through `gcore`, with `-j1`.

| Build | Scope | Result |
| --- | --- | --- |
| sse (GCC 13.3, Release) | full build, full `ctest -j1` | 100% of 45 passed (intervalf and interval2f skipped by design) |
| fpu (`-DGAOL_SIMD=OFF`) | gaol, 03, 16, and `ctest -R` on them | 2 of 2 passed; prints `<0, 0>` first |
| clang-18 | same | 2 of 2 passed; only the existing `gaol_nerrs` warning |
| `-DGAOL_PRESERVE_ROUNDING=ON` | same | 2 of 2 passed (16 takes 0.33 s) |
| branch merged with the current `origin/configure-clean`, sse | `git merge-tree` finds no conflict; gaol, 03, 16 | 2 of 2 passed, same output |
| mingw-w64 11, i686, `GAOL_SIMD=OFF` (cross) | gaol, 03, 16 | compiles; it could not run: this wine has no 32-bit prefix |
| 16 at `-O0` with ASan and UBSan, linked with the sse library | the program | exit 0, no report |

`-fsyntax-only -Wall -Wextra -Wconversion -Wshadow` on 16 gives **no warning** from the example itself. I ran it with g++ and clang++-18, in C++11, 17 and 20, with GAOL's headers as `-isystem`.

The manual compiles with `sh manual/build-pdf.sh manual/v5 $SCR/tmp/review-39/pdf`: 124 pages. Its only makeindex warning (`canonical interval`) predates this branch. The index has the new entry "dependency problem".

`check_branch todo-39-goldstein-price`: OK. It reports five files, all one-line messages, no ledger file, no PDF and no whitespace error.

## Portability

- The change touches no library code and no build file. `examples/CMakeLists.txt` changes in a comment only.
- The checks rest on arithmetic that is exact by construction: integers below 2^53, and `set_eq` treats -0 as 0. The ratio sits 0.38 away from the edges of its bracket. So x87 excess precision (i386), ARM, s390x, ppc64le, riscv64, musl and Visual C++ cannot change a result.
- `14*x` and friends meet only the `(double, const interval&)` overloads. The conversion from `int` is unambiguous for every compiler.
- `EXIT_FAILURE`, `<cstdlib>`, `<string>` and the anonymous namespace are C++11.
- No CI workflow builds the examples (grep `example` in `.github/workflows`: nothing). `make check` and `meson test --setup check` run them, and the loop has the same amount of work as before.
- CMake 3.14 and meson 0.53 are not affected.

## Documentation

I checked every added statement:
- 16's comments;
- 03's header and function comment;
- the `examples/CMakeLists.txt` comment;
- `examples.md` §1.1, the table, §16 and recommendation 16;
- the overview chapter of `gaol.tex`.

They are true. One of them is "the maximum is reached on the edge y = 2"; my branch and bound confirms it. So is the pointer to `examples/03_dependency_problem.cpp` for subdivision: it is in the source distribution. So is the unit ms: `elapsed_time()` is `getrusage` user time in ms, or `clock()*1000/CLOCKS_PER_SEC`. No `\newinvfive` mark was added, which is consistent: the mark "follows what behaves otherwise than in GAOL 4", and this is a correction of the text, not of the library.

## Blocking

None.

## Non-blocking

1. **The author of commit `969595e` is `Claude <noreply@anthropic.com>`**, the identity in `/root/.gitconfig`. The message is clean. Everything pushed so far is authored by Jordan08, including `0e80c52` and `4ce59b6` of this branch, whose identical author timestamps show they were re-authored. If this commit is pushed as is, it credits Claude on GitHub, which the house rules forbid. The orchestrator should re-author it before pushing, for example with `git commit --amend --no-edit --reset-author` under the maintainer's identity. The commit is unpushed, so this is allowed.
2. `examples/16_Goldstein_Price.cpp:97` writes the maximum as a decimal that is below the true maximum by 1.2e-20. The outward rounding of GAOL's exact lexer makes `range` enclose M anyway: its upper bound is M + 8.7e-11. Writing `...0829884232` would make the text itself an upper bound. This is cosmetic.
3. Not in this diff, confirming the author's open question 1:
   - `03_dependency_problem.cpp:237` (`interval(1015690.2717980589082988423120822331039464707651154)`) and `06_global_optimization.cpp:337` (`interval(3.0, 1015690.2717980589082989)`) both round to the double 1015690.2717980589, which is M - 2.97e-11.
   - So their "contains the range" references are 3e-11 narrower than the true range. It is harmless there, since the enclosures are far wider. It belongs to the examples point.
4. Not in this diff, in `examples.md` §1.1:
   - "Their outputs are the same on every build" ignores the CPU time that 16 prints.
   - "All 16 examples compile without a warning (`-Wall -Wextra`...)" holds only with GAOL's headers as system headers, as earlier reviews noted.
5. `examples/CMakeLists.txt`: in "...a timing loop that checks its enclosure too, which the autotools and meson builds of GAOL also compile", the word "which" now reads as if it referred to the enclosure. This is a wording nit.
6. The manual's program is checked through a copy of it in 16 (`w`). An edit of `gaol.tex` alone would go unnoticed. That is acceptable, as for the rest of the manual.
7. The check that `universe() * [0]` is `{0}` is slightly outside point 39. It is harmless, and matches "every claim of the output is checked". `interval(0.,0.)` on that line keeps the GAOL 4 spelling (`interval(0.0)` would match the house style).
