# Fix of point 24b on armhf: `operator&=` raised FE_INVALID on an empty operand

Branch `fix-24b-armhf-fe-invalid`, base `origin/configure-clean` (6ef2387),
worktree `$SCR/wt/armhf`. Not pushed.

## Commits

- 375a822 On 32-bit ARM, operator&= tests isunordered() first: GCC made its quiet comparisons vcmpe

Authored and committed by Jordan08 <jordan.ninin@gmail.com>, one line, no
credit line. `check_branch fix-24b-armhf-fe-invalid origin/configure-clean`: OK
(1 commit, 1 file: `gaol/gaol_interval_fpu.h`).

## What was wrong and what the change does

### The CI failure

Run 36761037299 (PR #50, `c0e8bb2` merged into `e80819a`), three armhf jobs,
each `an operation with an empty operand raises no invalid-operation flag  58
checks, 6 failed` in `tests/rounding_direction.cpp`, everything else green:

- 110043350190 Debian 13 Trixie armhf, g++ 14.2.0-19;
- 110043349843 Debian 12 Bookworm armhf, g++ 12.2.0-14+deb12u1;
- 110043350332 Debian 13 Trixie armhf, `GAOL_PRESERVE_ROUNDING=ON`, g++ 14.2.0.

The logs show five of the six (the tests print at most five failures per
check name): `x & empty`, `gaol_ieee1788::intersection(x, empty)`,
`nth_root_rel(x, 1, empty)`, `asinh_rel(x, empty)`, `atanh_rel(x, empty)`; the
sixth, reproduced here with `GAOL_TESTS_FAILURES_SHOWN=100`, is
`invabs_rel(x, empty)`. `x &= empty` passes there. The jobs run on
`ubuntu-24.04-arm` runners, the armhf container natively (the qemu step is
skipped), with the configuration GAOL detects: FPU intervals (no SSE2),
`-mfpu=neon-vfpv4 -mfloat-abi=hard`, `-O3 -frounding-math -fno-fast-math
-ffp-contract=off`; "The processor does not trap an invalid operation here",
so only the in-process flag checks run.

### Reproduction

GCC 13.3 cross compiler (`arm-linux-gnueabihf-g++`, Ubuntu 13.3.0-6ubuntu2,
Thumb-2, armv7-a), CMake toolchain file with `CMAKE_CROSSCOMPILING_EMULATOR
"qemu-arm;-L;/usr/arm-linux-gnueabihf"` (qemu-arm 8.2.2): CMake detects the
same flags as the CI (`-mfpu=neon-vfpv4 -mfloat-abi=hard`, SSE2 intervals
OFF). `gaol_test_rounding_direction` under qemu fails the same six checks, and
only those (plus one unrelated failure, see Overlaps). qemu reproduces the
ARM flags faithfully enough: a control program shows `a < b` with a NaN `a`
raising FE_INVALID and `std::isless(a, b)` not.

The reviewer's survey of point 24b (`tmp/review-24b/survey.cpp`, 287
operations, 4845 combinations with an empty operand), cross-compiled and run
under qemu: **210 combinations raise FE_INVALID, all in 10 operations, all with
the empty set as the second operand of an intersection**: `x&y`, `x&=y`,
`ieee::intersection`, `nth_root_rel(x,1,y)`, `asinh_rel`, `atanh_rel`,
`invabs_rel`, `ieee::sinhRev(c,x)`, `ieee::tanhRev(c,x)`, `ieee::absRev(c,x)`.
Nothing else (relations, predicates, other functions) raises in these
contexts.

### The cause: GCC's if-conversion turns a quiet comparison into vcmpe

Not the constructor, `is_empty()`, a max/min nor a copy: one of the two quiet
comparisons of the FPU `operator&=` (`gaol/gaol_interval_fpu.h`), which GCC
compiles with the signaling `vcmpe` where it turns the choice of a bound into
a conditional move.

- In `asinh_rel` of the armhf `libgaol.a`: `vcmpe.f64 d16, d20; vmrs; it hi;
  vmovhi.f64 d16, d20` (the lower bound), the other comparisons being `vcmp`.
  In a replica of `&=` (`tmp/armhf/cg/and.cpp`, `iv(a) &= b` returned by
  value): `vcmpe.f64 d18, d1; vmrs; it lt; vmovlt.f64 d1, d18`, from
  `if (!std::isgreaterequal(I.right(), right())) rb_ = I.rb_;`. The same
  operator assigned in place (`a &= b`) keeps branches and `vcmp`: that is why
  `x &= empty` passes in the test and fails in the survey; it depends on the
  context GCC inlines it in.
- The RTL dumps (`-fdump-rtl-all`, `tmp/armhf/cg/dump/`): up to `cse_local`
  the comparison is `(set (reg:CCFP cc) (compare:CCFP ...))`
  (`*cmpdf_split_vfp`, vcmp) and a jump on `(unlt (reg:CCFP cc) 0)` into the
  block `rb_ = I.rb_`. The first if-conversion pass (`ce1`: "if-conversion
  succeeded through noce_try_cmove") replaces them by `(set (reg:CCFPE cc)
  (compare:CCFPE ...))` (`*cmpdf_trap_split_vfp`, **vcmpe**) and
  `(if_then_else (ge (reg:CCFPE cc) 0) rb_ I.rb_)` (`*thumb2_movdfcc_vfp`).
- Why: the jump branches into the then-block, so if-conversion takes the
  reversed condition. The ARM backend reverses a condition on the flags of a
  floating-point comparison with `reverse_condition_maybe_unordered`
  (`REVERSE_CONDITION` in `arm.h`): the quiet UNLT becomes the ordered GE,
  which is right as a test of the flags. But the conditional move is expanded
  again from the operands (`movdfcc` calls `arm_gen_compare_reg (code, ...)`),
  and `arm_select_cc_mode` gives CCFPEmode, the signaling `vcmpe`, to LT, LE,
  GT, GE, and CCFPmode, `vcmp`, to EQ, NE, UNORDERED, ORDERED and the UN*
  codes. So every quiet ordering comparison (`std::isless()`...,
  `!std::islessequal()`...) that GCC reverses this way becomes signaling;
  UNORDERED/ORDERED (`std::isunordered()`, `std::isnan()`) stay `vcmp` both
  ways.
- GCC 12, 13, 14: `arm_select_cc_mode`, `REVERSE_CONDITION` and the `movdfcc`
  expander are the same in the three release branches (read in
  `gcc/config/arm/arm.cc`, `arm.h`, `arm.md` of `releases/gcc-12`, `-13`,
  `-14`, `tmp/armhf/gccsrc/`); GCC 13.3 reproduces here, and the CI shows GCC
  12.2 and 14.2 failing the same six checks. Known GCC bugs of this kind on ARM:
  PR 52258 "__builtin_isgreaterequal is sometimes signaling on ARM" (NEW since
  2012, last changed 2024-01-04) and PR 102018 "gcc.dg/torture/pr82692.c
  execution fails on arm cortex-m7" (UNCONFIRMED, vcmpe); the x86 analogue,
  PR 82692, was fixed in GCC 8 in the i386 backend (quiet comparisons marked
  UNSPEC_NOTRAP), which is why x86 is not affected. I did not find a GCC bug
  report for this exact path (if-conversion, `noce_try_cmove`).
- AArch64 is not affected: GCC 13.3 for AArch64 compiles the same `&=` with
  `fcmp` and `fcsel ..., lt` (quiet), the survey under qemu-aarch64 finds 0 of
  4845 combinations raising, and `rounding_direction` passes its 58 flag
  checks; the CI's arm64 jobs were green.

### The change

`gaol/gaol_interval_fpu.h`, `interval::operator&=`, on 32-bit ARM only
(`#if defined(__arm__) && !defined(__aarch64__)`, the targets where GCC is the
only compiler GAOL accepts): the empty operands are told first with
`std::isunordered(lb_, rb_)` (`this` empty: returned as it is) and
`std::isunordered(I.lb_, I.rb_)` (`I` empty: the result is `[NaN, NaN]`, the
bits the base code gave), whose comparison stays `vcmp` whether GCC reverses
it or not; then the bounds, none of them NaN any more, are compared with plain
`>` and `<`, which cannot raise whatever instruction GCC picks; the final test
of disjoint bounds is unchanged. Other processors keep the code of #50 in the
`#else` branch, byte for byte, so that x86 code generation is unchanged. A
comment says why, with the GCC mechanism.

Why not the same code everywhere: I measured it. A form robust everywhere with
no extra comparison (one `std::isunordered(lb_, I.lb_)`, then plain
comparisons) is faster with GCC 13 on x86-64 (throughput loop 1.95 -> 1.21 ns)
but not uniformly with clang-18, the reference compiler (+9% in the random
throughput loop, 1.73-1.80 -> 1.89-1.97 ns, -8% in the random in-place loop,
within 2-3% in the others; `tmp/armhf/cg/bench_variants.cpp`), and it
assumes both bounds of an empty set are NaN; the two-guard form used on ARM
costs one comparison on x86 (+20-25% in two clang loops). The task asked for no
cost on nonempty operands off ARM, so these stay an open question.

## Tests

No new test: the existing checks of `tests/rounding_direction.cpp`, "an
operation with an empty operand raises no invalid-operation flag" (58 entries
of point 24b), are the regression test; six of them fail on armhf without the
fix, which is what the CI reported. No test file changed.

## How the test was shown to fail without the fix

On the final tree, in the armhf cross build: `git show
HEAD~1:gaol/gaol_interval_fpu.h > gaol/gaol_interval_fpu.h` (the fix only
reverted), `gcore cmake --build build/armhf-cross -j1 --target
gaol_test_rounding_direction`, run under `qemu-arm -L
/usr/arm-linux-gnueabihf` with `GAOL_TESTS_FAILURES_SHOWN=100`: exit 1,
`an operation with an empty operand raises no invalid-operation flag  58
checks, 6 failed` (`x & empty`, `gaol_ieee1788::intersection(x, empty)`,
`nth_root_rel(x, 1, empty)`, `asinh_rel(x, empty)`, `atanh_rel(x, empty)`,
`invabs_rel(x, empty)`). Restored with `git checkout HEAD --
gaol/gaol_interval_fpu.h` (worktree clean), rebuilt, run: `58 checks, 0
failed`. Logs `tmp/armhf/rd-teeth-revert.log`, `rd-teeth-restore.log`.

## Local validation

- armhf cross build (GCC 13.3, Release, the CI's flags), full build: no
  warning (only GCC's "parameter passing ... changed in GCC 7.1" notes, as in
  the CI). Full `ctest -j1` under qemu: 38 tests, 35 passed (among them
  `arithmetic`, `relations`, `reverse`, `ieee1788`, `elementary`,
  `core_math`, `numbers`), `intervalf` and `interval2f` skipped by design,
  `rounding_direction` failed only on "the other outputs" (unrelated,
  pre-existing, see Overlaps): its flag check `58 checks, 0 failed`.
- armhf with `GAOL_PRESERVE_ROUNDING=ON` (the third failing CI job):
  `rounding_direction` flag check `58 checks, 0 failed` (11916 checks, the one
  failure is "the other outputs").
- Survey (`tmp/armhf/survey/`) on armhf: before 210 of 4845 combinations raise
  FE_INVALID, after **0 of 4845**; the values mode (55348 lines, bits of every
  result of every operation on every combination, empty or not) is
  **identical** before and after.
- x86-64, natively: sse (GCC 13.3) full build and `ctest -j1`: 55 tests, all
  pass but `rounding_direction` ("the other outputs" only, as on the base),
  `intervalf`/`interval2f` skipped; fpu (`-DGAOL_SIMD=OFF`) full build and
  ctest: the same; clang-18, sse Debug and fpu Debug
  `gaol_test_rounding_direction`: `17281 checks, 2 failed`, the two being "the
  other outputs" (result, and result with FE_INVALID enabled), the flag check
  and the trap check of the 58 entries all passing. The base FPU build gives
  the same `17281 checks, 2 failed`.
- x86 code generation: the SSE2 build does not include
  `gaol_interval_fpu.h`: `libgaol.a` and `gaol_test_rounding_direction` are
  byte-identical to those built before the change. FPU build, base
  (`git archive origin/configure-clean`) against the branch: `objdump -d` of
  the 48 objects of `libgaol.a` and of `rounding_direction.cpp.o` identical;
  the only differing bytes are the source path string in
  `gaol_interval.cpp.o`.
- ARM cost: one comparison more (`vcmp`, `vmrs`, branch) per `&=` of nonempty
  operands, 35 instructions instead of 31 for `iv(a) &= b` returned by value,
  32 instead of 28 in place (GCC 13.3, replica `tmp/armhf/cg/variants.cpp`).
  No timing: qemu timings mean nothing.
- AArch64 (GCC 13.3 cross, qemu-aarch64), unchanged code: survey 0 of 4845,
  `rounding_direction` flag check 58/0.

## Documentation changed

None besides the code comment in `gaol/gaol_interval_fpu.h`: `doc/using.md`
and the manual already say that an empty operand raises no invalid-operation
exception; the change makes it true on 32-bit ARM for the intersection. (But
see open question 2: `is_empty()` in the program's own selections is not
covered there.)

## Behaviour change

On 32-bit ARM only: `x & y`, `x &= y`, `intersection()` and the functions
intersecting with an empty operand (`nth_root_rel(x, 1, y)`, `asinh_rel`,
`atanh_rel`, `invabs_rel`, `sinhRev`, `tanhRev`, `absRev`) no longer raise
FE_INVALID for an empty `y`. Results unchanged, bit for bit (survey). A
malformed `this` with inverted non-NaN bounds (which GAOL's FPU intervals never
build) now gives `[NaN, NaN]` instead of being returned as it is; both are
empty for `is_empty()`. Other processors: no change at all.

## API and ABI

None. `operator&=` is inline: programs get the change when recompiled, the
reverse functions of the library when it is rebuilt.

## Overlaps with other points

- **Unrelated failure on configure-clean, every platform**: `tests/rounding_direction.cpp`,
  entry "the other outputs" of `empty_operands` (added by #50, abc148a),
  expects `write(E(), interval_format::width) == "empty"` and the same for the
  center format, while point 16 (#57, fdeca24 "Width and center formats write
  midpoint(), rad() and [empty]") writes `[empty]` there
  (`gaol/gaol_interval.cpp`, `operator<<`). The two merged without a textual
  conflict; `rounding_direction` now fails on configure-clean 6ef2387 with
  `17281 checks, 2 failed` on x86 (and 1 on armhf, which cannot trap), the
  same on the base and on this branch. Not fixed here (not this point's bug);
  the likely fix is to expect `"[empty]"` for both formats. The CI of this
  branch will show it until then.
- Point 24b (#50): this completes it on armhf.

## Risks

- The ARM branch is compiled and tested only by the armhf CI jobs (Debian 12
  and 13, CMake; autotools and meson armhf in `build-systems.yml`); x86 never
  sees it.
- `#if defined(__arm__) && !defined(__aarch64__)` also covers a program
  compiled by Clang for 32-bit ARM, which GAOL refuses (`gaol_config.h`), and
  does not cover Visual C++ for 32-bit ARM (`_M_ARM`), not in the CI.
- The fix removes the one case the CI and the survey found; the mechanism is
  general (open question 2).

## Open questions and decisions for the maintainer

1. `rounding_direction` "the other outputs": fix the expectation (`[empty]`
   for the width and center formats) in its own change; until then the CI is
   red on every platform for that entry.
2. The same GCC mechanism reaches `is_empty()` (and potentially any quiet
   comparison) in the program's own code on 32-bit ARM: with GCC 13.3,
   `x.is_empty() ? p : q` for an empty `x` compiles to `vcmpe; it ls;
   vmovls` and raises FE_INVALID (`tmp/armhf/ctx/ctx.cpp`); the relations
   tried (`set_contains`, `set_disjoint`, `set_leq`, `subset`, `isMember`...)
   did not in that form. GAOL's own operations are all quiet (survey). Options:
   document it in `doc/using.md` and the manual; or, on 32-bit ARM only,
   `is_empty()` as `std::isunordered(lb_, rb_) || left() > right()` (one
   comparison more there, and to be checked that GCC does not fold it back
   into one reversible UNGT); or report it to GCC (PR 52258 is open).
3. A form robust on every platform (one `std::isunordered(lb_, I.lb_)`, then
   plain comparisons, four comparisons as now): GCC 13 x86-64 faster (1.95 ->
   1.21 ns in a throughput loop, which would undo the GCC slowdown of `&=`
   noted in the review of #50), clang-18 mixed (+9% in one loop, -8% in
   another, within 2-3% in the others). Not taken,
   the task asking for no x86 change.

## For TODO.md

Point 24b: the armhf CI failure of #50 (six "raises no invalid-operation
flag" checks, `x & empty` and the reverse functions) is fixed by branch
`fix-24b-armhf-fe-invalid`: GCC 12-14 for 32-bit ARM compiled a quiet
comparison of `operator&=` into `vcmpe` when it made a conditional move of it;
on 32-bit ARM `&=` now tells the empty sets with `std::isunordered()` first.
Remaining: `is_empty()` in the program's own selections on 32-bit ARM (open
question 2); "the other outputs" of `rounding_direction` after #57.

## For ChangeLog

gaol/gaol_interval_fpu.h: on 32-bit ARM, `operator&=` tells an empty operand
with `std::isunordered()` before comparing the bounds. GCC 12 to 14 compiled
its quiet comparisons into the signaling `vcmpe` where it turned the choice of
a bound into a conditional move, so that `x & y`, `intersection()` and the
reverse functions raised FE_INVALID for an empty `y` on armhf.

## For doc/differences.md

Nothing new: the entry of point 24b ("An empty operand raises no
invalid-operation exception anywhere") becomes true on 32-bit ARM for the
intersection; if the maintainer documents open question 2, a sentence there.
