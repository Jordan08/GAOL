# Review of `todo-24b-quiet-empty-operands` (point 24 follow-up)

Branch head `bdec7d9`, base `origin/configure-clean` (`0f2963c`). Reviewed in a
detached worktree (`$SCR/wt/review-24b`, removed at the end); scratch files in
`$SCR/tmp/review-24b/`.

**Verdict: changes.** The code is right: no operation raises FE_INVALID on an
empty operand in any configuration I could build, and no result changes. What
has to change is one sentence of the docs, which claims more than the code does
(B1). The rest is non-blocking: the cost left on `floor`/`ceil`/`integer`, the
cost of `&=` with GCC, the Visual C++ question, and one existing bug I noticed
along the way.

## Blocking

### B1. "An empty operand raises no exception" is false (doc/using.md:339, manual/v5/gaol.tex:1000)

The bullet starts "An empty operand raises no exception, though the bounds of
the empty interval are NaN". The code only makes the **invalid-operation**
exception quiet. The test also checks only that one: it says itself "the other
exceptions (inexact, ...) are not checked". Empty operands still raise the
other exceptions:

```
$ ./inexact-sse    (g++ -O2, new sse lib; same with the fpu lib)
empty + empty: is_empty 1, FE_INEXACT 1, FE_INVALID 0, FE_DIVBYZERO 0, FE_OVERFLOW 0
atanh_rel([-1, 2], empty): is_empty 1, FE_DIVBYZERO 1, FE_INEXACT 1
```

My survey in `allflags` mode (`survey-new-sse-O3 allflags only`, where every
operand is empty) gives FE_INEXACT for `x+y`, `x-y`, `+=`, `-=`, `1.5+x`,
`1.5-x`, `ieee::add/sub` and the expression evaluator. The reason is the
rounding-direction probe `1 + 2^-60`, which runs before the emptiness test. The
bullet just above this one says that probe raises inexact, so the two bullets
now contradict each other. With a nonempty partner, `atanh_rel`, `tanhRev`,
`asinh_rel` and `textToInterval(sl, sr)` raise divide-by-zero, overflow or
underflow while computing on that partner.

Fix: in both files, write "An empty operand raises no invalid-operation
exception, though ...". The same overstatement is in the subject of `bdec7d9`
("... raises no floating-point exception"; the branch is not pushed yet, so the
author can reword it) and in the report's ledger texts ("For doc/differences.md":
"raises no floating-point exception anywhere").

## Non-blocking

1. **`floor`/`ceil`/`integer` cost (reproduced).** `$SCR/tmp/review-24b/bench.cpp`,
   1024 random intervals, best of 15, three runs:
   - clang-18: floor 1.64–1.70 → 1.78–1.81 ns; ceil 1.63–1.66 → 1.87 ns.
   - GCC 13: floor 1.37–1.39 → 1.60–1.64 ns; ceil 1.37–1.39 → 1.60–1.62 ns.

   That is +0.1 to +0.25 ns (8–17%): the task asked for no cost on nonempty
   operands, and this is not quite zero. A form that costs nothing exists (the
   report's alternative (b)):
   - `floor` and `ceil` need no comparison at all, because `[floor(l), floor(r)]`
     is always valid and NaN bounds give the empty set by themselves.
   - `integer` needs only one quiet comparison, `std::islessequal(ceil(l), floor(r))`.

   The catch is that the result has to be built without the constructor:
   `interval(__m128d)` with the SSE2 intervals, friends or `lb_`/`rb_` with the
   FPU ones. The maintainer should choose between (a), (b) and keeping this.
   The author said so; I agree it is not a defect.
2. **`&=` with GCC is not always faster, and can be slower.** In the author's
   loop GCC went 6.43 → 2.45 ns. In my throughput loop
   (`bench2.cpp`, `z = v[i]; z &= w[i]; acc += z.right()`) it goes the other way:
   - SSE2 intervals: 1.82 → 2.05–2.33 ns.
   - FPU intervals: 1.53 → 1.74–1.98 ns.

   That is +13–29%, the same in three runs. Before, GCC compiled the branchless
   `vcmplesd`/`vblendvpd` (see the `loop` disassembly of `b2-base-gcc-fpu`).
   Now it emits `ucomisd` + `cmov` through general registers plus a branch:
   SSE2 has no quiet ordered `<=` compare predicate. clang-18, the reference
   compiler for performance, is unchanged (1.48–1.50 ns). The relations are
   unchanged with GCC in a loop: `set_contains` 0.95/0.95, `set_disjoint`
   1.29/1.29, `straddles_zero` 0.88/0.88. `set_leq` moves 0.97 → 1.04–1.08 ns.

   The report should say the effect of `&=` with GCC depends on the loop, not
   that it is a speed-up. Generated code (`cg.cpp`): GCC FPU `&=` standalone is
   23 instructions both ways, `comisd` → `ucomisd`. FPU unary minus drops from
   22 to 10 instructions (GCC) and from 18 to 5 (clang).
3. **Visual C++.** CI evidence that the quiet comparisons are quiet there: the
   point-24 Windows run 36537854690 (push, `d3a1c85`) passed `rounding_direction`
   on VS 2022/2026 x86, x64 and arm64, and its in-process "is_empty() raises no
   flag" check uses `std::islessequal`. The containers run of the same commit
   passed `rounding_direction` on i386, armhf, s390x, ppc64le, riscv64 and musl
   (its only failure is `numbers` on arm64, `stol`, unrelated).

   Performance is not known. MSVC builds the FPU intervals (`GAOL_SIMD AND NOT
   MSVC`). The FPU `&=` now makes four quiet comparisons where it made one
   (`is_empty()`) plus three `<=`. If the UCRT turns them into `_dpcomp`
   calls, `&=` slows down on Windows. `gaol_performance` measures neither `&=`
   nor any relation, so the CI would not show it. This is a known risk, stated
   in the report.
4. The docs say "which cost what `<=` costs" about `is_empty()` and the
   relations. That was measured with GCC and clang on x86-64 and holds there;
   it was not measured with MSVC. Consider "one instruction, as `<=`, with GCC
   and Clang on x86".
5. **Small gap in the tests.** `textToInterval(sl, sr)` is checked only with
   the left operand empty (`"[empty]", "[1, 2]"`). The right-empty case goes
   through the same `||` condition, so this is minor.
6. **Nit.** The comment above `straddles_zero()` says "as many instructions".
   That is not exactly true with GCC: `set_disjoint` has 21 → 24 instructions,
   `set_contains(I)` 19 → 16. `gaol_interval_sse.h:119` changes the leading
   whitespace of the `if` line (4 spaces + tab → tab); it looks the same.
7. **Existing bug, not from this branch (for the maintainer, a new TODO point).**
   `cancel_minus`/`cancel_plus` give wrong results in the GCC Release library.
   - `cancel_minus([0.5], [4.9e-324, 1e-300])` = `[0x1.fffffffffffffp-2, 0.5]`.
     It should be `[-oo, +oo]`: y is wider than x, and IEEE 1788 12.12.5 then
     asks for Entire. The Debug library and the clang build give Entire.
   - `cancel_minus([DBL_MAX], [0.5])` = Entire. It should be
     `[0x1.ffffffffffffep+1023, DBL_MAX]`, which Debug and clang give.

   The cause is in `difference_at_least()`: GCC -O3 moves the error terms of
   `two_sum` after `GAOL_RND_NEAREST_LEAVE()` (the objdump shows the
   `vaddsd`/`vsubsd` after the second `vldmxcsr`), so they are computed rounding
   upward. `GAOL_RND_KEEP` on `s1`/`e1`/`s2`/`e2` would pin them. The base has
   the same behaviour, and no test catches it (`cm.cpp`).

## Checks run

Worktree: detached at `bdec7d9`. Base sources from
`git archive origin/configure-clean` into `$SCR/tmp/review-24b/base-src`.

**Builds and tests**, all run through `gcore -j1`:
- **sse** (GCC 13, Release), full build + ctest: 49/49 passed (intervalf and
  interval2f skipped by design), no warning.
- **fpu** (`-DGAOL_SIMD=OFF`): 49/49, no warning.
- **clang-18**, full build + ctest: 49/49; the only warning is the old parser
  one.
- `gaol_test_rounding_direction`, 16972 checks, 0 failed, in each of: sse
  Debug, fpu Debug, clang-18 fpu, and `GAOL_PRESERVE_ROUNDING=ON` (16971
  checks there).
- **MinGW-w64 i686** cross build (mingw-w64 11, accepted for 32-bit x86) of
  the library and `rounding_direction`: compiles with no warning. It could not
  be run: the installed wine has no 32-bit support.
- Manual: `build-pdf.sh manual/v5` gives 126 pages, no undefined reference.
- `check_branch`: OK. Both commits authored by Jordan08, no Claude credit.
  `TODO.md`, `ChangeLog` and `doc/differences.md` are untouched. No build file
  changed.

**Independent survey** (`survey.cpp`, written from the headers, not from the
author's list): 287 operations and 4845 (operation, operands) combinations with
at least one empty operand. The partners are 23 nonempty intervals, including
±0 bounds, infinities, subnormals and DBL_MAX; ternary operations use a
subset. The survey covers:
- every member, relation, predicate and operator with an interval or a double;
- every function, the reverse functions and the `gaol_ieee1788` wrappers;
- the five output formats, the readers and textToInterval pairs;
- expression evaluation and printing.

Each combination is tested for the FE_INVALID flag in process. Each operation
is also run in a child with `feenableexcept(FE_INVALID)`; the control
`0.0/0.0` traps.

| configuration | base | branch |
|---|---|---|
| GCC SSE -O3 | 672 raising combos, 62 ops trap | 0 / 0 |
| GCC FPU -O3 | 723 raising combos, 67 ops trap | 0 / 0 |
| GCC SSE -O0 (Debug lib) | 672 raising combos, 62 ops trap | 0 / 0 |
| GCC FPU -O0 (Debug lib) | 723 raising combos, 67 ops trap | 0 / 0 |
| clang-18 SSE -O3 | not built | 0 / 0 |
| clang-18 FPU -O3 | 723 raising combos, 67 ops trap | 0 / 0 |
| GCC SSE `GAOL_PRESERVE_ROUNDING` | not built | 0 / 0 |

The ops raising on the base match the author's list, plus the readers and the
expression evaluation that reach them.

**Values:** every operation on every combination, empty or not (55348 lines),
with the bits of the results printed (NaN compared as NaN).
- Base and branch are **bit-identical** for GCC SSE -O3, GCC FPU -O3, GCC SSE
  -O0, GCC FPU -O0 and clang FPU -O3. This includes ±0 bounds, e.g. `sqrt` of
  `[-0, 2]` (the lower bound -0 that the intersection keeps is taken as +0).
- The IEEE 1788 truth values on the empty set, checked by hand against Table
  10.4: subset(E,X)=T, subset(X,E)=F, interior(E,E)=T, disjoint(E,·)=T,
  equal(E,E)=T, precedes/strictPrecedes(E,·)=T, less(E,E)=T and less(E,X)=F.

**Teeth:**
1. I reverted only `gaol/gaol_interval_sse.h` (the SSE2 `&=`), rebuilt
   `gaol_test_rounding_direction` in the sse build and ran it: exit 1,
   `16972 checks, 14 failed`. The failures are "x & empty", "x &= empty",
   "intersection(x, empty)", "nth_root_rel(x, 1, empty)", "asinh_rel(x, empty)"
   and others: no-flag and FE_INVALID-enabled "died on SIGFPE". Restored with
   `git checkout HEAD --`.
2. I reverted all four code files to the base: sse gives exit 1, `88 failed`
   (44 flag + 44 trap); fpu gives exit 1, `94 failed` (47 + 47); the 57 result
   checks pass. This confirms the "44 of 47 / all 47" of doc/tests.md.
   Restored, and the worktree is clean at `bdec7d9`.

**Generated code:** GCC 13 and clang-18 -O3, base against branch, for `&=`,
unary minus, `set_contains` (I and d), `set_disjoint`, `set_leq`,
`straddles_zero`, `floor`, `integer` and `split_left`. Every `comisd` on these
paths became `ucomisd`. `floor` and `integer` gain 2 instructions (`ucomisd` +
`jp`) and keep the constructor's 3 `comisd`. `split_left` gains 2 instructions
(GCC) or 5 (clang).

**Micro-benchmarks:** see non-blocking points 1 and 2.
