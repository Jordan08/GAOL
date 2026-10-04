# Review of `configure-clean` (3 commits over `origin/configure-clean`, head b4fcb21)

Verdict: **changes requested**. The code of the three builds and of autoconf is right: all four readers agree
on 4000 random `VERSION.txt` files. `configure` is exactly what autoconf 2.72 generates. The tests pass and
have teeth, and the CI step works. There are four blocking findings. Three are false statements in the
documentation this branch writes. The fourth is the autoconf fix, which has no regression test.

## Blocking

1. **`doc/tests.md:401-404` and `tests/version_file.cmake:133-136`: "without that, these two checks fail with
   those versions only" is false.** "Those versions" are CMake 3.14.7 and 3.16.3. But `string(REGEX MATCH)`
   fails on an empty match in every CMake before 4.1: 3.28.3 (the CMake of Ubuntu 24.04), 3.30.0 and 4.0.0 fail
   too. The earlier review suggested this wording, and it was wrong.
   Evidence (a script of 4 lines, `cmake_minimum_required(VERSION 3.14)`, `string(REGEX MATCH "^.*" s "")`):
   ```
   3.14.7, 3.16.3, 3.28.3, 3.30.0, 4.0.0: CMake Error ... regex "^.*" matched an empty string.
   4.1.0: -- s=[]
   ```
   With the guard `if(_text MATCHES "^.")` removed from `gaol_read_version()`, `cmake -P tests/version_file.cmake`
   stops with `CMake Error at cmake/gaol_version.cmake:54 (string)` under 3.14.7, 3.16.3 **and 3.28.3**.
   Fix: "string(REGEX MATCH) stops with an error on an empty match in CMake before 4.1 (3.14.7, 3.16.3, 3.28.3,
   3.30.0 and 4.0.0 checked) ...; without that, these two checks fail with every CMake before 4.1". The
   same correction is optional for the base comment `cmake/gaol_version.cmake:51-53`.

2. **`examples/examples.md:526-528`: "the Goldstein-Price function it names" is false.** The overview chapter
   of the manual on this branch never names Goldstein-Price: `grep -ci goldstein manual/v5/gaol.tex` gives 0.
   It says "compute the range of the function f(x,y)=(1+(x+y)^2...)" (lines 721-726), and its program
   evaluates exactly that function. The name comes from GAOL 4's example `16_Goldstein_Price.cpp`. After
   `todo-39-goldstein-price` is merged, the manual names it but has the +1. So the sentence is not "true before
   and after the merge", as the report says. Fix: "But the function of the overview chapter's program is the
   Goldstein-Price function of GAOL 4's example (`16_Goldstein_Price.cpp`) without the +1 of (x + y + 1)²
   (recommendation 16)." The "In short" bullet is acceptable as it stands.

3. **`doc/tests.md:406-408`: "`.github/scripts/version-file.sh` runs them ... with the same cases and a few more"
   is false.** Four of the cases of `tests/version_file.cmake` are missing from the script: "two numbers",
   "four numbers", "an empty file" and "zeros". Evidence: I diffed the lists of case names of the two files.
   Fix: add the three refused cases. I checked that they pass with both configure and meson:
   ```
   refused "two numbers" '7.3\n' '37 2e 33 0a'
   refused "four numbers" '7.3.11.0\n' '37 2e 33 2e 31 31 2e 30 0a'
   refused "an empty file" '' ''
   ```
   Or reword to "with most of these cases (not 0.0.0, ...) and a few more".

4. **The autoconf bugs fixed in `configure.ac` have no regression test.** At generation time, autoconf accepted
   `7. 3.11`, `7.3.\r11`, a NUL byte and `5.0.0)`. It died in m4 on `[` and refused `\v\f` around the version.
   The report shows these only by manual `autoconf -f` runs on copies. Nothing committed or run by the CI
   checks them: `version-file.sh` runs `configure` and meson only. The CI step "configure generated for the
   version" only compares `--version` with a valid file. The house rule asks for a check with teeth for every
   bug fixed. Fix: add an `autoconf` mode to `version-file.sh`, and run it in the Linux autotools job after
   `apt-get install autoconf`. It runs `autoconf -f` in the copy, requires `configure.ac:NN: error:` and the
   bytes for the refused cases, and `configure --version` = `gaol configure 7.3.11` for the accepted ones.
   Show that it fails with the base `configure.ac`. A prototype run of the 24 cases with autoconf 2.72
   (autoconf, then the configure it generates) gave 24 ok with the new `configure.ac`.

## Non-blocking

- **MSYS2 meson job.** `refused()` now anchors its pattern with `$`. meson run by a Windows Python writes CR LF
  to `log`. I checked this under wine with python.org 3.12.7 and meson 1.4.1: `cat -A log` shows `...ASCII^M$`,
  and GNU grep 3.11 does not match `ASCII$`. It works today only because MSYS2's grep is `1~3.0-7`
  (GNU grep 3.0), whose `dosbuf.c` strips the CRs. That grep, run under wine, matched `ASCII$` on the same
  log. If MSYS2 updates grep, all 15 refused cases fail there. A more robust form is
  `LC_ALL=C tr -d '\r' < log | grep -a -q ...`.
- The three commits have the author `Claude <noreply@anthropic.com>` (the machine's gitconfig). The pushed
  history is authored by Jordan08, so the orchestrator must set the author before pushing. The messages
  themselves are fine and `check_branch` is OK.
- `examples/examples.md:522`: the heading "The manual's examples print what it shows" is followed at once by
  "apart from three formatting slips".
- `doc/building.md`: "NUL bytes, which no text of UTF-8 or ASCII holds" is loose, because NUL is an ASCII and
  UTF-8 character. "which a text file does not hold" is more accurate.
- `doc/building.md` (and the manual, l. 645) say that `meson setup` runs Python "once". On a refused file it
  now runs three times (twice on the base). This is true for a valid file.
- The CMake, meson and configure hint "the file is UTF-16" is given for any file that starts with FF FE or
  FE FF, including UTF-32LE (FF FE 00 00) or `\xff\xfe5.0.0`. This is a harmless heuristic.
- `doc/building.md`, "the continuous integration checks it": the step runs on Linux only.
  `doc/continuous-integration.md` says so.
- `gaol.pc.in:8` now conflicts textually with `origin/todo-04-ftz-daz`. The report already says so.

## Checks run

- **Branch.** Worktree `$SCR/wt/review-cc` (detached at b4fcb21). It was left clean after each revert.
  `check_branch configure-clean origin/configure-clean`: OK. `git diff --check`: clean. Subjects are 85 to
  97 characters.
- **Generated `configure`.** `AUTOHEADER=true autoreconf` with autoconf 2.72 (`$SCR/tools/bin`) on `git archive
  HEAD`. No tracked file differs from the committed tree, so `configure` is exactly what autoconf 2.72
  generates.
- **autoconf, generation time.** `autoconf -f` on copies with 18 contents: accepted `5.0.0`, `\v\f5.0.0\v\f`,
  and BOM with CRLF and blanks. Refused, with the right bytes and hint: `5. 0.0`, `5.0.0)`, `5.0.0[`,
  `5.0.0 dnl`, `5,0,0`, NUL, UTF-16 with and without its mark, two lines, CR inside, `#`, empty, `'`, `$1`,
  and a backquote.
- **Differential fuzz of the four readers.** The readers are the real `gaol_read_version()` under CMake 3.28.3
  and 3.14.7, meson.build's Python logic, `configure`'s code under dash, and `configure.ac`'s esyscmd commands.
  Two seeds gave 1507 and 2507 files, 429 of them accepted. The four readers gave identical results
  everywhere, hints included.
- **`tests/version_file.cmake`.** "28 checks, 0 failed" with CMake 3.14.7, 3.16.3 (from cmake.org) and 3.28.3.
  `file(READ)` on the NUL fixture gives length 5 on 3.14.7 and 3.16.3 and length 7 on 3.28.3, as the doc says.
- **Teeth.**
  - With `${_utf16}` removed from the CMake message: "4 of 28 checks failed" (3.14.7).
  - With `configure` and `configure.ac` reset to the base: `version-file.sh configure` FAILED on "a CR inside:
    accepted", "two lines", "UTF-16", "UTF-16, big-endian", "UTF-16 without a mark: accepted" and "a NUL
    byte after the version: accepted" (exit 1).
  - With meson.build back to `.strip()`: "a no-break space after", "an em space after" and "a file separator
    after" were accepted (exit 1).
  - With `find_program('python', 'python3')`: the no-Python CI step fails with
    `ERROR: Program 'python python3' not found`.
  - All were restored with `git checkout`.
- **Scripts under dash.** `dash .github/scripts/version-file.sh configure` and `... meson` (meson 1.4.1):
  24 ok each, exit 0.
- **CI step.** Extracted from the YAML with `yaml.safe_load`, which also validates the file. Run with
  `bash --noprofile --norc -eo pipefail`: 1726 links, "Program python3 found: YES", "Project version: 5.0.0",
  exit 0. The meson script's interpreter is named by an absolute path: pipx on the runners, `/usr/bin/python3`
  for pip or apt. meson's fallback `args[0].endswith('python3')` is at `interpreter.py:1683` in 1.4.1.
- **Builds.**
  - sse, full CMake build: 100% of 49 passed; intervalf and interval2f skipped; version_file passed.
  - CMake 3.14.7 configure of the tree: GAOL_VERSION "5.0.0", `ctest -R version_file` passed.
  - meson 1.4.1, full build and `meson test`: 27 ok, 2 skipped.
  - autotools VPATH `configure --with-tests` under `LANG=fr_FR.UTF-8`: "from VERSION.txt: 5.0.0", with no
    warning. Then `make` and `make check`: 27 PASS, 2 SKIP, 0 FAIL.
  - fpu and clang not run: no C or C++ file changed.
- **Not run here: macOS.** By reasoning, BSD `od -An -v -tx1 [-N 2]` prints lowercase bytes in wider columns,
  and `tr -d ' \n'` and `grep ' 00'` are insensitive to that. BSD `tr -d '\000'`, `tr -c` and `sed
  [[:space:]]` under `LC_ALL=C` are POSIX. configure exports `LC_ALL=C` itself (lines 60-61).
