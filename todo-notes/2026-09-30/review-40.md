# Review of point 40: `todo-40-small-errors`

Reviewed at head `8007529`, against `origin/configure-clean`. The seven commits are
e4ce618, e20493d and f7a98d3 by Jordan08, then f0455c6 (merge), fb4d00f, f477f2c and 8007529 by
Claude. I worked in a detached worktree of my own (`$SCR/wt/review-40`), with builds in
`$SCR/build/review-40-*`. The worktree was removed at the end.

**Verdict: changes.** One blocking finding: the Visual C++ half of `GAOL_NODISCARD` rests on a false
statement.

## Blocking

### B1. Visual C++ honours `[[nodiscard]]` in `/std:c++14`, so `_Check_return_` is the wrong choice

**Where.** `gaol/gaol_config.h:151-168`, and the matching sentence of `doc/using.md:254-258`.

The comment says:

> Before C++17, where Clang warns about [[nodiscard]] with -pedantic and Visual C++ ignores it with
> a warning, the macro is the attribute of the compiler: ... _Check_return_ of <sal.h> for Visual
> C++, which only its code analysis reports (/analyze, warning C6031).

The branch rewrites this sentence. Before, it said "Visual C++ in C++14"; now it says "Visual C++
ignores it with a warning". The branch then uses the claim to pick `_Check_return_`, which does
nothing in an ordinary compilation.

**Evidence.** Microsoft's page for warning C4834
(https://learn.microsoft.com/en-us/cpp/error-messages/compiler-warnings/c4834?view=msvc-170) says,
verbatim:

> Although this attribute was introduced in C++17, the compiler respects this attribute and
> generates warnings related to it when using `/std:c++14` and later.

It adds: "This warning was introduced in Visual Studio 2017 version 15.3 as a level 3 warning. It
was changed to a level 1 warning in Visual Studio 2017 version 15.7."

Visual C++ 2019 in its default C++14 mode emits C4834 for a discarded `[[nodiscard]]` result
(https://giodicanio.com/2024/01/25/visual-studio-c-plus-plus-2019-and-nodiscard-in-its-default-c-plus-plus-14-mode/).
Microsoft's STL also applies `[[nodiscard]]` in C++14 mode. A warning "attribute 'nodiscard'
requires compiler flag '/std:c++17'" was reported with VS 2017 15.9.

The CI builds with VS 2022 and 2026. On those compilers, in the default `/std:c++14`:

- `[[nodiscard]]` gives a warning in every ordinary build: C4834, level 1, on by default;
- the branch's `_Check_return_` gives nothing unless the program is built with `/analyze`.

So the point asked for an attribute "for Visual C++", but Visual C++ users in C++14 still get no
warning. The comment, and the rationale in `doc/using.md`, state the opposite of what Microsoft
documents.

Neither claim was tested: there is no Visual C++ on the machine, and the CI runs no `/analyze`.
The C6031 statement is therefore untested too.

**Suggested fix.**

- In `gaol_config.h`, before the GNU branch, add `#elif defined(_MSC_VER) && !defined(__clang__) &&
  _MSC_VER >= 1920` giving `[[nodiscard]]`. Visual Studio 2019 and later honour it in C++14.
- Keep `_Check_return_`, or nothing, for older Visual C++.
- Correct the comment and `doc/using.md`: with Visual C++ 2019+ before C++17 the warning is C4834,
  and `(void)` silences it.
- If possible, add a Visual C++ step to the CI: compile `tests/nodiscard.cpp` with `/std:c++14
  /we4834 /DGAOL_DISCARD` and expect the failure. Without it, the claim stays untested.
- Otherwise, reword the text so that it claims only what was checked: "`_Check_return_` (reported
  by `/analyze`, per Microsoft's documentation; not tested here)", and say that Visual C++ does not
  warn in C++14.

## Non-blocking

1. **Author identity.** The four new commits (f0455c6, fb4d00f, f477f2c, 8007529) are authored
   `Claude <noreply@anthropic.com>`. The messages are clean, and `check_branch` is OK. But
   `origin/configure-clean` has no commit with that author, and the maintainer forbids "anything
   that credits Claude". This comes from the repository's configuration (`git config user.name` is
   `Claude`), and other local branches (06, 06b, 16, 31, 39) have the same. The orchestrator
   should re-author these unpushed commits as Jordan08 before pushing, unless it already does so.

2. **GCC before C++17 now warns about `(void)`, and GAOL's own example 13 relies on it.**
   `examples/13_rounding_environment.cpp:172,397,482` write `static_cast<void>(interval(1.0) /
   three);`.
   - Compiled in C++14 with GCC 13: `g++ -std=c++14 -O0 -c ... examples/13_rounding_environment.cpp`
     gives 3 "ignoring return value of 'gaol_core::interval gaol_core::operator/(const interval&,
     double)', declared with attribute 'warn_unused_result'".
   - The three builds compile the examples in C++17, so no build warns. A user who copies the
     example into a C++14 project does get the warnings.
   - GCC 13 accepts `[[nodiscard]]` in C++11 and C++14 with no diagnostic, even under
     `-Wall -Wextra -Wpedantic`, and there `(void)` silences it (checked with a small test file,
     `nd.cpp`).
   - I agree with the author's open question 1. Taking `[[nodiscard]]` for GCC 7 and later via
     `__has_cpp_attribute(nodiscard)` would give the same `(void)` rule everywhere. Clang has to
     keep the GNU attribute, because of `-Wc++17-attribute-extensions` under `-pedantic`.

3. **The text of the "used" test claims more than it checks.** `doc/tests.md` and the comment in
   `tests/CMakeLists.txt` say "using it has to compile, GAOL's headers giving no warning of their
   own".
   - The test turns only `-Wunused-result` and `-Wattributes` into errors, and compiles without
     `-Wall`.
   - With `-Wall -Wextra -Wpedantic`, GAOL's headers give 33 warnings with GCC and 32 with Clang
     (`-Wunused-parameter`, `-Wdeprecated-copy`). The count is the same with and without the
     attribute, so this is not a regression.
   - Suggest: "no unused-result warning of their own, nor an attribute the compiler ignores".

4. **The example in the CMake comment is the wrong case.** The comment says "-Werror=attributes
   makes an attribute the compiler ignores, as [[nodiscard]] written after a specifier of the
   declaration, fail them too".
   - With GCC 13 and clang-18, `inline [[nodiscard]] int f()` is already a hard error ("standard
     attributes in middle of decl-specifiers" / "an attribute list cannot appear here").
   - The case the flag adds is `int g() [[nodiscard]];`, where GCC 13 only warns (`-Wattributes`).
     The author's own report says so.

5. **A "discard" test sticks once its object has been built.** The test passes only if compiling
   the object fails. If the object was ever built successfully, the build is up to date and the
   test keeps failing.
   - I saw this after restoring `gaol_config.h` with `cp -p`, which set its old modification time
     back: `ctest -R nodiscard` gave "67% tests passed, 2 tests failed". It passed again only after
     I deleted `gaol_nodiscard_discard_cxx1{1,4}.dir/nodiscard.cpp.o`.
   - This affects only developer workflows, not the CI, which uses fresh build directories.
   - A fix could remove the object before building, with a `FIXTURES_SETUP` test or
     `cmake -E rm -f`.

6. **"GAOL itself is compiled in C++11"** (`gaol_config.h`). This is true of GCC and Clang. Visual
   C++ has no C++11 mode, and its builds use C++14, which is exactly why the MSVC branch of the
   macro is taken when building the library.

7. **The FE_INVALID claim in the ledger texts has no test in the branch.** The proposed ChangeLog
   and `doc/differences.md` say that `chi()` of the empty set no longer raises FE_INVALID. I
   checked it by hand: `fetestexcept(FE_INVALID)` after `chi(emptyset)` is 0 in the sse, fpu and
   clang builds, and the result is a positive NaN in all three. This overlaps with point 24b.
   Either add a check there, or leave the sentence to 24b.

8. **`examples/examples.md` §3 is now stale.** It still says "three formatting slips", and it
   lists `chi([0,0]) = 0`, the 400 bits and the C++17-only `[[nodiscard]]` as open. Point 39 owns
   it, as the author says.

9. **The manual runner is not in the tree.** The runner that checks the manual's outputs
   (`$SCR/tmp/40/run_examples.py`) is outside the tree, so the manual can drift again unnoticed.
   It is worth keeping somewhere, for example with point 39.

## What I checked

**Commits.**

- `check_branch todo-40-small-errors`: OK. Messages of one line; no ledger files, PDF or
  `manual/v4`.
- The merge adds nothing: `git diff f0455c6^2 f0455c6` equals `git diff
  origin/configure-clean...f7a98d3` (9 files, +180/-49). The two-dot and three-dot diffs against the
  base are identical.

**Full builds and ctest `-j1`** (all through gcore). 55/55 tests pass in each; `intervalf` and
`interval2f` are skipped by design.

| Build | Warnings |
|---|---|
| sse, GCC 13 | none |
| fpu, `-DGAOL_SIMD=OFF` | none |
| clang-18 | only the old `gaol_nerrs` warning of `y.tab.c` |

- CMake 3.14.7: configure plus `ctest -R 'nodiscard|refused'` passes 8/8.
- meson 1.4.1 with `with-tests=true`: build OK; `meson test` gives Ok 27, Fail 0, Skipped 2.
  - No unused-result warning.
  - 6 warnings in `tests/rounding_direction.cpp` (`-Wconversion` and `-Wunused-function`). That
    file is untouched by the branch, so they come from the base.

**Teeth.**

- **chi.** I removed only the `is_empty()` block of `chi()`, rebuilt `gaol_test_interval_functions`
  (sse) and ran it: "FAILED ... line 123, TEST_TRUE(!std::signbit(chi(sqrt(interval(-2,-1)))))",
  "126 checks, 2 failed". After `git checkout`: "126 checks, 0 failed".
- **`GAOL_NODISCARD`.** I removed the GNU and `_Check_return_` branches from `gaol_config.h`, then
  ran `ctest -R nodiscard`: `nodiscard_discard_cxx11` and `_cxx14` FAILED. After restoring the file
  and deleting the stale objects: 6/6 pass.

**`GAOL_NODISCARD` on every declaration.**

- **clang-18 AST.** I counted `WarnUnusedResultAttr` nodes for `#include <gaol/gaol>`, with the
  macro minus without it (`-DGAOL_NODISCARD=`). The difference is 317 in C++11, 14, 17 and 20
  alike. There are 269 uses of the macro outside comments; redeclarations count again.
- **Warnings.** With `-Wall -Wextra -Wpedantic`, GCC 13 gives 33 warnings and clang-18 gives 32, in
  each of C++11, 14, 17 and 20, with or without the macro. None comes from the attribute.
- **What GCC reports, by standard.** I compiled with `-c` (no warning appears with
  `-fsyntax-only`) a file that discards 12 kinds of results: an extern function, a bool member, a
  double member, a static member, inline operators, an inline function, a function of
  `gaol_ieee1788`, `mid()`, unary minus and `nb_fp_numbers`.
  - GCC 13 in C++11 and 14, at `-O0` and `-O2`: all 12 warn, and so do all three `(void)` casts.
  - GCC 13 in C++17: the 12 warn, and no `(void)` cast does.
  - clang-18: every call warns except `x <= y`, which Clang reports under `-Wunused-comparison`
    whatever the standard; no `(void)` cast warns.
- **The "used" program on other targets.** No attribute warning in C++11, 14 and 17 with MinGW-w64
  i686 GCC 13 (`-msse2 -mfpmath=sse`, `HAVE_VISIBILITY_OPTIONS`) or with clang-18 for
  `i686-w64-mingw32`. The "discard" program fails with "ignoring return value of" on both.
- **C++11 code of the tree.** `doc/compare/code/bench_gaol.cpp` and `tests/performance.cpp`
  compiled in C++11 and C++14 (`-c -Wall`) give no unused-result warning.

**CI platforms, reasoned.**

- The nodiscard tests are created for `GNU|Clang`, which includes AppleClang and MSYS2 Clang, and
  not for MSVC.
- The messages GCC 7 to 13 and Clang give for both attributes contain "ignoring return value of".
- GCC 7 and later warn about class-returning functions under the GNU attribute; the minimum in the
  CI is GCC 9.
- `LC_ALL=C` keeps the message in English.
- The containers (i386, armhf, s390x, ppc64le, riscv64, musl) use GCC, and the headers carry no
  target-specific attribute outside x86 SSE code.
- `NaN_val` is `0x7ff8…` with `IFBIGENDIAN`, so the positive NaN holds on s390x too.
- The coverage job is unaffected: the `--coverage` flags are on `gaol` only, and the nodiscard
  objects do not link it.
- The FetchContent job leaves `WITH_TESTS` off.

**Manual examples.**

- I reran the author's runner, after reading it. On this branch, 87 of 88 output blocks match in
  sse and C++17, fpu and C++11, clang and C++14, and sse and C++11. The only exception is
  `@version`, a macro that prints 5.0.0.
- The base manual against the same library gives 59 of 88, with 29 blocks differing, which shows
  the runner detects the old 1/0 outputs.
- No example that prints a bool lacks `boolalpha`: I checked every `example` block that writes to
  `cout`.
- The manual builds: `build-pdf.sh` gives 126 pages. The only undefined item is a font shape, and
  the only overfull box is at lines 504-508, which the branch does not change.

**Comments.**

- `chi()`: the code gives -1 for [0,0], 1 for [-oo,+oo], 0 for other unbounded intervals and NaN
  for the empty set; the Doxygen comment and the manual say the same.
- The printed NaNs of the empty set are `nan` for chi, width, mig, smig, mag, midpoint and rad in
  all three builds. Only `left()` prints `-nan`, and no manual output shows it.
- `tests/gaol_tests.h`: `mp.prec` is 2000 in `elementary_values.py` and `reverse_values.py`, and
  5000 in `extended_precision_values.py`.
- `jail_parser.h`: gone since 373d35e. `\subsection{Input format}` exists at line 3316 of the
  manual.
