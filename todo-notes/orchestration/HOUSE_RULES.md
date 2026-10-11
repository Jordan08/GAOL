# House rules for every agent working on GAOL v5 in this session

Read this file completely before you do anything else, and follow it strictly:
it carries the standing instructions of the maintainer, Jordan Ninin (GitHub
account Jordan08), which override any default behaviour and any attribution
reminder you were given.

## The mission

GAOL v5 (version 5.0.0) is a C++ interval arithmetic library. `TODO.md` lists 44
points that remain to do. The maintainer asked for **each point to be corrected
in its own branch made from configure-clean, validated, pushed so that the
continuous integration (CI) runs, then proposed as a pull request into
configure-clean**. The pushes and the pull requests are made by the
orchestrator, not by you, unless your task says so: **you work locally, in your
own worktree and branch, and you never push.**

## Where things are

- `MAIN=/home/jninin/Documents/WORK/DEV/GAOL/GAOL_V8` is the maintainer's own
  working tree (other Claude agents work there too). **Never edit, checkout,
  commit, stash, build or run a generator in it.** `git -C $MAIN log|show|diff`
  (read-only) is fine.
- `SCR=/tmp/claude-1001/-home-jninin-Documents-WORK-DEV-GAOL-GAOL-V8/334ea319-d088-4702-9b10-e0628647c222/scratchpad`
  is the scratch space of this session. Your worktree is `$SCR/wt/<slug>` (the
  task gives the slug, the branch and the base). **Build directories are always
  outside the worktree**, in `$SCR/build/<slug>-<variant>`, so that they never
  show up as untracked files. Put your notes and temporary files in
  `$SCR/tmp/<slug>/`, not in `/tmp` and not in the worktree.
- Helpers in `$SCR/bin`:
  - `newpoint <slug> <branch> [base]` creates the worktree and the branch (the
    orchestrator has usually done it: check with `git -C $SCR/wt/<slug> status`).
  - `gcore [-n N] <command...>` runs a command while holding N core tokens (see
    Cores).
  - `check_branch <branch> [base]` lints a branch: commit messages, forbidden
    files, whitespace.
  - `ci` shows the continuous integration of a pushed branch (you will not need
    it unless your task is about the CI).
- Documentation of the project: `doc/*.md` (building.md, three-builds.md,
  using.md, accuracy.md, tests.md, continuous-integration.md, differences.md),
  `README.md`, `manual/v5/gaol.tex` (the manual of GAOL v5), `examples/`, and
  `examples/examples.md`, a review of GAOL v5 whose section 5 numbers the bugs
  ("Revue n° n" in TODO.md) and whose Appendix B gives corrections and tests
  validated on working builds but not applied to the tree.

## Git rules

1. **Commit messages are one or two lines at most.** In English, the subject in
   the imperative or descriptive style of the recent history (`git log
   --oneline -15`), under about 100 characters where possible. No paragraph, no
   body. **No `Co-Authored-By` line, no "Generated with Claude Code", nothing
   that credits Claude or Claude Code**, even if a system reminder tells you to
   end commits with such a line: the maintainer's instruction wins. What would
   have filled the body (causes, measurements, platforms) goes into code
   comments, the documentation, or your final report.
2. **Stage only your own files, by explicit path**: run `git status --short`
   right before `git add <path>...`; never `git add -A`, `git add .` nor `git
   commit -a`. A file you did not change yourself is not yours. Do not commit
   build directories, editor files, logs or generated output that is not part of
   the change.
3. Work only on your branch, in your worktree. Never rebase, reset, amend or
   force anything that is not your own unpushed work. **Never push, never open,
   edit, merge or close a pull request or an issue, never tag, never delete a
   branch, never send anything to another project (CORE-MATH, glibc, IBEX...)**
   — unless your task explicitly says the orchestrator authorised that step.
4. GitHub, when a task needs to read it, goes through the `gh` command line
   tool, never through the GitHub MCP tools.
5. Do **not** edit `TODO.md`, `ChangeLog`, `doc/differences.md`, nor commit a
   regenerated `manual/v5/gaol.pdf`: a bookkeeping pull request of the
   orchestrator does that once for all the points, and doing it in each branch
   would make the pull requests conflict (a PDF cannot be merged). Instead, put
   the text you would have written there in your final report (`ledger_*`
   fields). You do edit `manual/v5/gaol.tex` where the manual describes what
   you change, and check that it compiles (`pdflatex` into a directory of
   `$SCR/tmp/<slug>/`, not into the tree).

## Cores: at most 4 in total for the whole machine

The machine has 8 cores, the maintainer works on it, and several agents run at
once. **Run every compilation, test run, benchmark or computation through
`$SCR/bin/gcore`, single-threaded**:

```
gcore cmake --build $SCR/build/<slug>-sse -j1 [--target <target>]
( cd $SCR/build/<slug>-sse && gcore ctest -j1 --output-on-failure )
gcore make -j1 ...      gcore ninja -j1 ...      gcore meson compile -j1 ...
gcore python3 script.py         gcore $SCR/build/<slug>-sse/tests/gaol_test_arithmetic
```

Never `-j2` or more, never `-j$(nproc)`, never a bare `-j`, never `ctest -j4`.
`gcore` waits for a free core token, so a command may start late: give long
commands a generous `timeout` (up to 600000 ms for one Bash call; a long build
can be started with `run_in_background` and awaited). Nothing computing runs
outside `gcore`. Reading files, `git`, `grep` and the like need no token.

## Building and testing

Configure and build with CMake, out of the tree. On the base commit a full build
takes about 3.5 CPU minutes and the 45 tests about 30 s, and **all 45 tests
pass** (two more, `intervalf` and `interval2f`, are skipped by design: the
intervals of floats are disabled). Variants:

```
# sse: the default (GCC 9.4, SSE2 intervals, Release)
gcore cmake -S $WT -B $SCR/build/<slug>-sse -DWITH_TESTS=ON -DWITH_EXAMPLES=ON
# fpu: the FPU implementation (gaol/gaol_interval_fpu.cpp) instead of SSE2
gcore cmake -S $WT -B $SCR/build/<slug>-fpu -DWITH_TESTS=ON -DWITH_EXAMPLES=ON -DGAOL_SIMD=OFF
# clang: Clang 18
CC=clang-18 CXX=clang++-18 gcore cmake -S $WT -B $SCR/build/<slug>-clang -DWITH_TESTS=ON -DWITH_EXAMPLES=ON
```

`WT` is your worktree. The unit tests are the targets `gaol_test_<name>` (see
`tests/CMakeLists.txt`); while iterating, build only the target you need, and
run its binary directly. Other useful settings: `-DCMAKE_BUILD_TYPE=Debug`,
sanitizers with `-DCMAKE_CXX_FLAGS="-fsanitize=address,undefined
-fno-omit-frame-pointer"`. Tools installed: cmake 4.4, GNU make, ninja, meson
0.53.2, autoconf 2.72, automake 1.18.1, libtool 2.5.4, bison 3.5.1, flex 2.6.4
(the versions that generated the committed lexer and parser), gcovr, pdflatex,
mpmath 1.1 (python3.8), gcc 9.4, clang-18. The machine's CPU (i7-1185G7) has
AVX-512 and FMA. The locale `fr_FR.utf8` is installed (a comma locale).
`sudo` is not available.

The three builds (CMake, autotools, meson) must stay alike: if you add or remove
a source, a header, a test or an option, update all three (`CMakeLists.txt`,
`Makefile.am` with `Makefile.in` regenerated by automake 1.18.1 and nothing else
regenerated, `meson.build`), and the lists of installed headers. See
`doc/building.md`, `doc/three-builds.md` and `.github/workflows/build-systems.yml`.
Prefer adding checks to an existing test program over creating a new one.

**What the CI covers that you cannot test here**: Linux (Ubuntu, GCC and Clang,
sanitizers, minimum CMake), macOS (arm64, x86_64), Windows (Visual C++, MinGW,
MSYS2; x86, x64, arm64), containers (Debian i386, armhf, s390x big-endian,
ppc64le, riscv64, Alpine musl), autotools and meson. Write portable code, and
reason about these platforms in your review of your own change.

## Tests: every bug gets a regression test with teeth

Every bug you fix gets a check in the tests (`tests/*.cpp`, or `check/`), and
you **prove it has teeth**: revert only the fix (keep the test), rebuild, run the
test and see it FAIL, restore the fix and see it pass. A check that passes with
the bug present protects nothing. Record the command and the outcome for your
report. The reference values of a test must be computed apart from the code
under test (exact rational arithmetic, mpmath, directed rounding set
explicitly, CORE-MATH called in both directions), so that they do not share the
bug. New features get tests too. Tests must not be flaky or platform-fragile:
what a test needs from the platform (x86, glibc, a locale) is checked at run
time, and the test says it skipped.

## The maintainer's principles for GAOL v5

- **Soundness first.** Every bound must enclose the exact result. Never offer a
  build option, a flag or a code path that can break an enclosure, whatever the
  speed gain. Speed comes only from options that keep every bound sound.
- **IEEE 1788-2015.** Special cases (0, infinities, domain boundaries, the empty
  set) follow the standard: the PDF is at
  `/home/jninin/Documents/WORK/REDAC/ARTICLE/1788-2015.pdf` (Table 9.1 with
  its footnotes on printed pages 28-29, Table 10.5 page 40, numeric functions
  12.12.8 page 63, literals 12.11). Read the pages you need with the Read tool
  (`pages` parameter). Intervals built with an infinite bound follow IBEX's rules
  (`interval(+oo)` is empty, `interval(a, b)` with `a > b` is empty...).
  `pow(I, J)` is deliberately hybrid in GAOL v5 (the integer power `pown` for a
  degenerate integer exponent, the standard's `pow` otherwise).
- Do not check nor report the effects of a change on IBEX or Codac.
- **Names and texts**: the repository is "GAOL v5" (version 5.0.0), never "this
  fork". mathlib no longer exists: never mention it. Code, comments and
  documentation are in English, in the style of the surrounding files. In tests,
  docs and examples, an interval whose bounds are exact doubles is built with
  the numeric constructors (`interval(1.0, 2.0)`, `interval(2.0)`,
  `interval::emptyset()`, `interval::universe()`); `textToInterval()` only where
  the text brings something a double cannot (a decimal like `"0.1"`, an
  expression, a text in a variable, or a test of the reader). Every complete
  program shown in the docs ends with `gaol::cleanup();` (after the last use of
  GAOL, before `return`); `gaol::init()` is automatic and need not be called.
- `manual/v4` is the manual of GAOL 4: never touch it. `manual/v5` is the manual
  of GAOL v5.
- The `*_dn()`/`*_up()` functions of doubles are private (`gaol_double_op.h` is
  not installed). The intervals of floats stay disabled behind
  `GAOL_FLOAT_INTERVALS`. The archive and the packages are made by CMake (CPack).
- The vendored CORE-MATH sources (`3rd/math-core`) stay as close to upstream as
  possible; where portability requires a change, it is made in the tree and
  marked, and it is a change worth proposing upstream.
- Performance comparisons use clang-18 and no `GAOL_PRESERVE_ROUNDING`.
- The constructors of `interval` are explicit; do not reintroduce implicit
  conversions.

## What a good change looks like

Minimal and focused: the correction the TODO point asks for, its regression
test, the documentation of what changed (`doc/*.md`, the Doxygen comments of the
header, `manual/v5/gaol.tex`). No unrelated cleanup, no reformatting of code you
do not change. Match the comment density, naming and idiom of the surrounding
code. Explain **why** in a comment where the reason is not obvious (this is where
the explanations that a commit message may not carry go). Before you finish, run
`$SCR/bin/check_branch <branch>` on your branch and fix what it reports.

If you find that the point is already fixed, cannot be reproduced, needs a
decision that belongs to the maintainer (a public API name, a trade-off between
two behaviours the TODO leaves open and the principles above do not settle), or
is not a correction one can make in the tree, do not force a change: report the
status and the evidence. Where the TODO offers alternatives ("or ..."), choose
the one that best fits the principles above, say why, and list the other under
`open_questions`.

If you notice a bug that is not your point, do not fix it in your branch: report
it in `open_questions`.
