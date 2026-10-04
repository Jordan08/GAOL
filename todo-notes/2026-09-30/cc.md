# cc: three follow-ups of points 39, 42 and 43, committed on configure-clean

Branch `configure-clean` (worktree `$SCR/wt/cc`), three commits over
`origin/configure-clean` (0f2963c), not pushed. Head: `b4fcb21aaec780cde523661be664f42b9b3d122e`.

## Commits

- `3167072` examples.md: the manual's examples print what it shows, but its Goldstein-Price lacked the +1
- `1b37c90` CI: meson setup with no Python on PATH; building.md: the residual case of WindowsApps
- `b4fcb21` The three builds and autoconf strip the same blanks of VERSION.txt, refuse NUL bytes, name UTF-16

## What was wrong and what the change does

**(a) Point 39, remark of the review.** `examples/examples.md` said in "In short" and in section 3 that the
86 examples of the manual "are right". Their outputs match what the manual shows, but the overview program
evaluated (x + y)² where the Goldstein-Price function has (x + y + 1)². Section 3 is now titled "The manual's
examples print what it shows" and adds one sentence: the overview program evaluated another function than
the Goldstein-Price function it names, because the +1 was missing (recommendation 16). The "In short" bullet
says the same in a parenthesis. The text stays true before and after `todo-39-goldstein-price` is merged. It
says what the review found and points to recommendation 16, which that branch marks "applied". Sections
1.1, 1.3 and recommendation 16 are not touched, and `git merge-tree` of the new `configure-clean` with
`origin/todo-39-goldstein-price` is clean.

**(b) Point 42, open question 2, and the review.** There is a new step "meson setup with no Python on PATH"
in the job `meson` of `.github/workflows/build-systems.yml`, run on Linux only (`if: runner.os == 'Linux'`).
The step:
- builds `$RUNNER_TEMP/no-python`, a directory of symbolic links to every program of `PATH` except those
  whose name starts with `python`, taking the first match of each name, as a lookup in `PATH` does;
- checks that neither `python3` nor `python` can be found there;
- runs `PATH=$RUNNER_TEMP/no-python meson setup $RUNNER_TEMP/build-no-python`.

meson itself still runs, because its script names its interpreter by an absolute path. The job installs meson
with pipx, whose script starts with `#!.../pipx/venvs/meson/bin/python -E`, and Debian's starts with
`#!/usr/bin/python3`. The step guards two things. The first is the order `find_program('python3', 'python')`.
The second is meson's fallback on its own Python, which `find_program_impl` takes only when
`args[0].endswith('python3')`: see meson 1.4.1, `interpreter.py:1683`.

`doc/building.md`, section "With meson", takes the review's rewording: "the Python that runs meson (the case
of the `meson.exe` of the Windows installer when no Python is installed)". It also says the CI checks the
fallback. For the residual case it adds one clause: meson 0.53.1 and later leave WindowsApps out "as long as
`PATH` names it by that path (not for a profile whose directory differs from `USERPROFILE`)". meson compares
each entry with `Path(USERPROFILE)/AppData/Local/Microsoft/WindowsApps`. `doc/continuous-integration.md`
names the new check.

**(c) Point 43, open questions and reviews.** The three builds, and autoconf when it generates `configure`,
now read `VERSION.txt` by the same rules.

- **Blanks.** `string(STRIP)` of CMake removes exactly the six ASCII blanks: bytes 9 to 13 and 32. I measured
  this over bytes 1 to 255 with CMake 3.14.7, 3.16.3, 3.28.3 and 4.4.3, under the C and fr_FR.UTF-8 locales.
  - meson now strips `string.whitespace`, which is that same set. `str.strip()` also stripped a no-break
    space, an em space, and the separators 1C to 1F.
  - configure strips `[[:space:]]` in the C locale, where it is the same set. It no longer runs `tr -d '\r'`,
    which also deleted a CR inside the version: configure accepted `7.3.\r11`.
  - autoconf used `tr -d ' \t\r\n'`. That removed blanks inside the text, so it accepted `5. 0.0` and
    `7.3.\r11` when it generated `configure`. It also refused a vertical tab or a form feed around the
    version, which configure accepts. autoconf now runs the same `sed` as configure. Then it joins the lines
    left with a blank, because an m4 regular expression matches at the end of any line and would accept a
    second line.
  - m4 reads the text again after the command. There, a `)` ended the argument, so autoconf took `5.0.0)`
    for 5.0.0. A `[` stopped m4, and a word such as `dnl` was expanded. Characters other than digits, dots
    and blanks are therefore written `?` before m4 sees them. Such a file is refused anyway, and the
    hexadecimal bytes of the message show the real characters.
- **NUL bytes.**
  - configure accepted `7.3.11` followed by a NUL, and UTF-16LE without a mark, because the shell drops NUL
    bytes from `$(...)`. configure now finds NUL bytes with `od -An -v -tx1 | grep ' 00'` and refuses the
    file. It removes them with `tr -d '\000'` before reading the text, so bash prints no warning.
  - autoconf does the same.
  - CMake was already fixed on the base by the third commit of point 43. It finds the NUL byte in
    `file(READ ... HEX)`, because `file(READ)` cuts the text at a NUL byte in 3.14.7 and 3.16.3 but keeps it
    in 3.28.3 and 4.4.3 (checked with the policies of 3.14). The test `version_file` passes with all four.
- **UTF-16.** The file is still refused, not decoded. When it starts with `FF FE` or `FE FF`, the message of
  all three builds and of autoconf ends with `; the file is UTF-16: save it as UTF-8 or ASCII`. When it holds
  a NUL byte without that mark, it ends with `; the file holds a NUL byte as UTF-16 does: save it as UTF-8 or
  ASCII`. That wording names the NUL byte, which CMake's quotation cut: `VERSION.txt holds "5.0.0" (... 00
  78)` looked contradictory. The hints have no comma because a comma would end an m4 argument.
- **Line ends in configure.** configure now joins the lines with a blank, as autoconf does, so that its
  message stays on one line.
- **CI step.** The step "configure generated for the version of VERSION.txt" computes the version with the
  commands of `configure.ac`.
- **Documentation.**
  - `doc/building.md`: the blanks now read "the six of ASCII". The sentence "a second byte order mark, or a
    zero-width space, does not show in the quotation" was not true for meson, which writes them `﻿` and
    `​`; it now says so. The paragraph gives UTF-16 with and without a mark and both hints, and is
    re-wrapped.
  - `doc/tests.md`: the new checks are listed, and "these two checks fail only there" is reworded as the
    review asked.
  - `doc/three-builds.md` says "by the same rules".
  - `gaol.pc.in` line 8 now names `VERSION.txt`.
- **`configure`** is regenerated with the tools of `$SCR/tools/bin` (autoconf 2.72,
  `AUTOHEADER=true autoreconf`).

## Tests

- **`tests/version_file.cmake` (CTest test `version_file`).** It has 28 checks, up from 22.
  - New accepted case: vertical tabs and form feeds around the version.
  - New refused cases: a CR inside, a no-break space after (C2 A0), a file separator after (1C), and the
    UTF-16 marks FF FE and FE FF before ASCII text (hint `utf-16`).
  - The two binary fixtures now expect their hint: `utf16le_bom.txt` expects `utf-16` and
    `nul_after_version.txt` expects `nul`.
  - Each refused message must end exactly with `such as 5.0.0` followed by the expected hint, or with no
    hint. So a hint is checked to be present where it belongs and absent elsewhere.
  - The comment on the "mark alone" and "empty file" checks is reworded as the review asked.
- **`.github/scripts/version-file.sh configure|meson`.** It has 24 cases, up from 15.
  - New accepted case: vertical tabs and form feeds around the version.
  - New refused cases: a blank inside, a CR inside, two lines, a no-break space after, an em space after,
    a file separator after, UTF-16 big-endian with its mark, UTF-16 without a mark, and 7.3.11 followed by
    a NUL byte.
  - The existing UTF-16 case now expects its hint.
  - `refused` takes an optional fourth argument (`utf-16|nul`) and anchors the message at its end.
- **The new CI step (b)** is itself the test of the python3-first order.

## How the test was shown to fail without the fix

- **CMake.** With only the hint removed from `gaol_read_version()` (the `${_utf16}` suffix dropped, on a copy
  of the file), `cmake -P tests/version_file.cmake` gave "4 of 28 checks failed" with CMake 3.14.7 and with
  3.28.3. The failures were the two UTF-16 marks, the UTF-16 fixture and the NUL fixture. With the fix, all
  four CMake versions (3.14.7, 3.16.3, 3.28.3 and 4.4.3) gave "28 checks, 0 failed".
- **configure and meson.** I ran the new `version-file.sh` on a copy of the tree at commit (b), with the old
  `configure`, `configure.ac` and `meson.build`.
  - configure gave exit 1 with 6 FAILED: "a CR inside: accepted", "two lines" (the message ran over two
    lines), "UTF-16" and "UTF-16, big-endian" (no hint), "UTF-16 without a mark: accepted", and "a NUL byte
    after the version: accepted". This was under C.UTF-8 and under fr_FR.UTF-8. configure sets `LC_ALL=C`
    itself, so the em space was refused before too.
  - meson 1.4.1 gave exit 1 with 7 FAILED: "a no-break space after: accepted", "an em space after:
    accepted", "a file separator after: accepted", and four missing hints.
  - With the fix, configure had 24 ok under dash, under bash (`CONFIG_SHELL=/bin/bash`, where no "ignored
    null byte" warning appears), and under LANG=C.UTF-8. meson had 24 ok with meson 1.4.1 and 0.53.2.
- **autoconf, at generation time.** I ran `autoconf -f` on copies of the tree with 24 contents of
  `VERSION.txt`.
  - Old `configure.ac`: accepted `7. 3.11`, `7.3.\r11`, `7.3.11` plus a NUL, and `7.3.11)`. Refused the
    vertical tab and form feed case. Died in m4 on `[`. Quoted `"5"` for `5,0,0`.
  - New `configure.ac`: accepts exactly the five valid cases and refuses the others with a well-formed
    message and the right hint.
- **CI step (b).** I extracted the step from the YAML and ran it with `bash --noprofile --norc -eo pipefail`.
  - On the tree it passes with meson 1.4.1 (pip), 0.53.2 (venv) and 1.12.1. The last one was installed with
    pipx, as the job does: `pipx install --backend pip meson`.
  - On a copy whose `meson.build` has `find_program('python', 'python3')` it fails with all three:
    "Program 'python python3' not found or not executable", and with 0.53.2 "Program(s) ['python',
    'python3'] not found".
  - With Python out of `PATH`, meson 0.53.2 logs "Program python3 found: NO", "Program python found: NO" and
    "Project version: 5.0.0". meson 1.x logs "Program python3 found: YES" with no path.

## Local validation

- **Regeneration.** First, `AUTOHEADER=true autoreconf` on `git archive` of the unchanged base reproduced
  `configure`, `aclocal.m4` and every `Makefile.in` byte for byte (only `autom4te.cache` and `configure~`
  were added). After the change, a fresh regeneration of the final tree differs from the committed tree in
  no tracked file, so only `configure` changed. The last edit of `configure.ac`, the m4 sanitization, does
  not change `configure`.
- **sse build.** Full build with GCC 13.3, Release, WITH_TESTS and WITH_EXAMPLES, `-j1` through gcore, then
  `ctest -j1`: "100% tests passed, 0 tests failed out of 49". `intervalf` and `interval2f` were skipped as
  expected, and `version_file` passed.
- **CMake versions.** `cmake -P tests/version_file.cmake` passes with 3.14.7 and 3.16.3 (from cmake.org),
  3.28.3 and 4.4.3 (from pip).
- **meson and autotools.** `meson setup` of the worktree (meson 1.4.1) succeeds: "Project version: 5.0.0" and
  `GAOL_VERSION "5.0.0"`. A VPATH `configure --with-tests` succeeds: "the version of GAOL, from VERSION.txt:
  5.0.0", with no warning.
- **Workflow.** The YAML loads with `yaml.safe_load`. The step "configure generated for the version of
  VERSION.txt" passes on the tree. It detects `5.0.1` and `5. 0.0` against a `configure` generated for
  5.0.0, and accepts a mark followed by blanks and CR LF.
- **Not run.** The fpu and clang variants: no C or C++ file changed. Windows, macOS and MSYS2 are left to the
  CI. The shell code uses only POSIX options: `od -An -v -tx1 [-N]`, `tr -d '\000'`, `tr -c`, `sed`
  `[[:space:]]` under `LC_ALL=C`, and `printf` octal escapes.
- **Lint.** `check_branch configure-clean origin/configure-clean`: OK. `git diff --check` is clean.

## Documentation changed

`examples/examples.md` (In short, section 3); `doc/building.md` (The version of GAOL, With meson);
`doc/continuous-integration.md`; `doc/tests.md` (bullet `version_file`); `doc/three-builds.md`; comments of
`meson.build`, `configure.ac`, `cmake/gaol_version.cmake`, `tests/version_file.cmake`,
`.github/scripts/version-file.sh` and `.github/workflows/build-systems.yml`; `gaol.pc.in` line 8.
`manual/v5/gaol.tex` does not describe how `VERSION.txt` is read. Its short meson paragraph keeps "use meson
0.53.1 or later, or turn off the aliases", whose second remedy covers the residual case.

## Behaviour change

- **Blanks.** Only the six ASCII blanks around the version are ignored, in all three builds and in autoconf:
  - meson now refuses a no-break space, an em space, or 1C to 1F at the ends, which `str.strip()` removed;
  - configure refuses a CR inside the version;
  - autoconf refuses blanks and CRs inside the version, and accepts a vertical tab or form feed around it.
- **NUL bytes.** configure and autoconf refuse a file that holds a NUL byte, including UTF-16 without a mark.
- **Messages.**
  - The message of every build ends with a hint for UTF-16 or for a NUL byte.
  - autoconf writes `?` for characters other than digits, dots and blanks in the text it quotes.
  - configure quotes several lines joined by a blank.
- **Nothing changes for a valid `VERSION.txt`.**

## API and ABI

None. Build-internal only.

## Overlaps with other points

- `gaol.pc.in` line 8 now conflicts textually with `origin/todo-04-ftz-daz`, which edits the line above. The
  resolution is trivial: keep both, with `VERSION.txt`. That branch already conflicts with `configure-clean`
  in other files.
- The other unmerged branches (fix-path-core-math, 03, 06, 08, 12, 16, 17, 36, 39, 40, todo-status) have
  exactly the same conflicts with the new head as with `origin/configure-clean`, which I checked with
  `git merge-tree`.
- 39 merges cleanly.
- A branch that edits `configure.ac` must regenerate `configure` after merging.

## Risks

- The CI step builds a directory of about 2000 symbolic links, which is fast. It depends on meson finding
  every other program through `PATH`, and a program that meson needs under a name starting with `python`
  would be missing. On the GitHub runners, meson (pipx) lives in `/opt/pipx_bin` or `~/.local/bin`, both on
  `PATH`.
- BSD `od` prints wider columns. `tr -d ' \n'` and `grep ' 00'` are insensitive to that, but this was not run
  on macOS here.
- The meson error path now runs Python twice more (the bytes and the UTF-16 test). It still runs once when
  the file is accepted.
- Python's text mode turns a lone CR into LF, so meson's quotation shows `\n` where the file has `0d`. The
  hexadecimal bytes are right, and the rules are unchanged: CR is a blank at the ends and refused inside.

## Open questions and decisions for the maintainer

1. **m4 quotation.** autoconf's generation-time message writes `?` for any character other than digits, dots
   and blanks. This is the price of making m4 safe against `,`, `(`, `)`, `[`, `#` and macro names such as
   `dnl`. An exact quotation would need quadrigraphs. The bytes in hexadecimal are exact.
2. **Hint wording.** "the file holds a NUL byte as UTF-16 does" has no comma, because m4 would cut the text
   at a comma. The same wording is used in all builds so they stay alike.
3. **Manual.** `manual/v5/gaol.tex` does not carry the residual WindowsApps case. Its remedy "turn off the
   aliases" covers it.
4. **Commit identity.** The commits carry this machine's git identity (`Claude <noreply@anthropic.com>`,
   from `/root/.gitconfig`), as other agents' commits of this session do. The messages credit no one, and
   `check_branch` is OK. The orchestrator may want to set the author before pushing.
5. **`m4_include([VERSION.txt])`.** It feeds the file to m4, in a KILL diversion, only after the `m4_fatal`
   check. By then the file holds only digits, dots and blanks, so it is harmless: `7.3.11[` now gets the
   normal message.

## For TODO.md

(TODO.md is in French)

- **39** : `examples/examples.md` (section 3 et « In short ») dit maintenant que les exemples du manuel
  impriment ce qu'il montre, mais que le programme de la vue d'ensemble calculait une autre fonction que
  Goldstein-Price (le +1 manquait, recommandation 16). Reste, pour la branche `todo-39-goldstein-price` :
  le style GAOL 4 de l'exemple 16 et sa première ligne `<-0, 0>`.
- **42** : le pas de CI « meson setup with no Python on PATH » (job meson, Linux) est fait. `doc/building.md`
  a la reformulation de la revue et le cas résiduel (un profil dont le répertoire diffère de `USERPROFILE`).
  Retirer la question ouverte.
- **43** : les trois constructions et autoconf lisent `VERSION.txt` par les mêmes règles :
  - les six blancs ASCII autour de la version, jamais dedans ;
  - un octet NUL refusé partout ;
  - UTF-16 refusé, avec « the file is UTF-16: save it as UTF-8 or ASCII » ;
  - `gaol.pc.in` nomme `VERSION.txt`.

  Retirer la question ouverte.

## For ChangeLog

	* examples/examples.md: the manual's examples print what it shows, but
	its overview program evaluated Goldstein-Price without its +1.
	* .github/workflows/build-systems.yml: meson setup with no Python on
	PATH, which guards find_program('python3', 'python') and meson's
	fallback on its own Python.
	* configure.ac, meson.build, cmake/gaol_version.cmake: VERSION.txt is
	read by the same rules everywhere (the six blanks of ASCII around the
	version, a NUL byte refused, UTF-16 named in the message); autoconf no
	longer removes the blanks inside the version.

## For doc/differences.md

- **`VERSION.txt` is read by the same rules by CMake, meson, configure and autoconf.** They ignore the six
  ASCII blanks around the version (meson stripped Unicode spaces too, configure dropped a CR anywhere, and
  autoconf dropped blanks inside the version). They refuse a NUL byte, which configure and autoconf used to
  let through. A UTF-16 file is refused with a message that says to save it as UTF-8 or ASCII.

## Review round

The review (`$SCR/reports/review-cc.md`) asked for changes: four blocking findings and eight non-blocking ones.
One more commit answers it. The branch now has four commits over `origin/configure-clean` (0f2963c), not
pushed. Head: `646db619a6b92212aa688ffe3e22344cfb5e7c9f`.

- `646db61` version-file.sh: an autoconf mode run by the CI, the cases of version_file; docs corrected after review

### Blocking findings, all fixed

1. **The empty match of `string(REGEX MATCH)`.** The finding is right. I checked it again with a 3-line script
   (`string(REGEX MATCH "^.*" s "")` under `cmake_minimum_required(VERSION 3.14)`):
   - 3.14.7, 3.16.3, 3.28.3, 3.30.0 and 4.0.0 stop with "matched an empty string";
   - 4.1.0 gives `s=[]`.

   3.30.0, 4.0.0 and 4.1.0 came from the pip wheels, unpacked in `$SCR/tmp/cc/r2`.
   - `doc/tests.md` and the comment of `tests/version_file.cmake` now say: "stops with an error on an empty
     match in CMake before 4.1 (3.14.7, 3.16.3, 3.28.3, 3.30.0 and 4.0.0 checked)". They say that without
     the guard, "the test stops with that error on these checks with every CMake before 4.1". The script does
     stop, which is more exact than "the two checks fail".
   - The base comment in `cmake/gaol_version.cmake` now says "of CMake before 4.1".
   - Proof: with the guard `if(_text MATCHES "^.")` removed on a copy, `cmake -P tests/version_file.cmake`
     stops with `CMake Error at .../gaol_version.cmake:54 (string)` under 3.14.7, 3.16.3, 3.28.3, 3.30.0
     and 4.0.0. Under 4.1.0 it gives "28 checks, 0 failed". With the guard, all six versions give "28
     checks, 0 failed".
2. **Goldstein-Price in `examples/examples.md`.** The finding is right: `grep -ci goldstein
   manual/v5/gaol.tex` gives 0. Section 3 now takes the review's sentence: "But the function of the overview
   chapter's program is the Goldstein-Price function of GAOL 4's example (`16_Goldstein_Price.cpp`) without
   the +1 of (x + y + 1)² (recommendation 16)."
   - The "In short" bullet is unchanged, as the review allows.
   - I withdraw the earlier claim that the text is "true before and after the merge". After
     `todo-39-goldstein-price` is merged, the sentence describes the state that was reviewed, and
     recommendation 16 there says "applied". That is how the rest of the document reads.
   - `git merge-tree` with `origin/todo-39-goldstein-price` is still clean.
3. **"The same cases and a few more".** The finding is right. `.github/scripts/version-file.sh` now has the
   cases of `tests/version_file.cmake` that it lacked:
   - refused: "two numbers", "four numbers" and "an empty file";
   - accepted: "zeros" (0.0.0) and "numbers of several digits" (12.345.6789).

   `accepted` takes an optional third argument, the expected version (7.3.11 by default). It checks the
   four macros against that version. The sentence of `doc/tests.md` is now true. The UTF-16 marks are
   covered by the UTF-16 texts, which start with the same marks.
4. **No regression test for the autoconf fixes.** The finding is right, and there is now one.
   - `version-file.sh autoconf` runs `autoconf -f` in the copy after removing `autom4te.cache`.
     - An accepted case must give `gaol configure <version>` as the first line of `configure --version`.
     - A refused case must give `^configure.ac:NN: error: VERSION.txt holds ... (bytes in hexadecimal:
       ...), where ... such as 5.0.0<hint>$`. That is the `m4_fatal` of `configure.ac`, not an error of m4.
   - Two new refused cases hold the characters m4 reads: "a parenthesis after" (`7.3.11)`) and "a bracket
     after" (`7.3.11[`).
   - The CI job `autotools` has a new step "VERSION.txt read by autoconf, as by configure", on Linux only.
     It runs `sudo apt-get install -y -q autoconf` and then `sh .github/scripts/version-file.sh autoconf`.
     Ubuntu 24.04's autoconf is 2.71. It warns that `aclocal.m4` was generated for 2.72, and it works.

### Non-blocking findings

Fixed:
- **CR LF on MSYS2.** `refused()` now runs `LC_ALL=C tr -d '\r' < log | LC_ALL=C grep -a -q ...`.
  - A comment says why: meson run by a Windows Python writes CR LF, and GNU grep's `$` does not match
    before a CR. MSYS2's grep 3.0 strips the CRs itself.
  - I checked it on a CR LF log with GNU grep 3.11. The old form does not match, and the new form does.
- **The heading of section 3** is now "The manual's examples print almost exactly what it shows".
- **`doc/building.md`, NUL bytes.** The text now says "which a text file does not hold".
- **Python "once".**
  - `doc/building.md` now says "once, in `project()`, ..., and twice more for the message of a file it
    refuses".
  - The manual (`manual/v5/gaol.tex`) says "runs Python to read the version" and drops "once". It compiles:
    126 pages, and the only warning is the font warning that was there before.
- **The no-Python step runs on Linux only.** `doc/building.md` now says "checks it on Linux".

Not changed (see the open questions): the commit author, the UTF-16 hint for UTF-32LE or `FF FE` followed by
ASCII, and the conflict of `gaol.pc.in` with `todo-04-ftz-daz`.

### Tests and teeth

- **`version-file.sh`.** It has 31 cases (10 accepted, 21 refused), up from 24. It gave 31 ok and exit 0:
  - `autoconf`: autoconf 2.71 (`/usr/bin`, the version of Ubuntu 24.04) under dash and bash, and autoconf
    2.72 (`$SCR/tools/bin`);
  - `meson`: meson 1.4.1, 1.12.1 (pipx) and 0.53.2;
  - `configure`: dash.
- **Teeth of the autoconf mode.** On a copy of the tree whose `configure.ac` is the one of
  `origin/configure-clean`, `version-file.sh autoconf` gives exit 1 and 9 FAILED, the same with 2.71 and
  2.72:
  - "vertical tabs and form feeds around: refused";
  - "a blank inside: accepted";
  - "a CR inside: accepted";
  - "a parenthesis after: accepted";
  - "a bracket after" (m4 died: `ERROR: end of file in string`);
  - "UTF-16" and "UTF-16, big-endian" (no hint);
  - "UTF-16 without a mark" (quoted "7", no hint);
  - "a NUL byte after the version: accepted".
- **The CI step.** I extracted it from the YAML with `yaml.safe_load` and ran it with
  `bash --noprofile --norc -eo pipefail`. apt reported "autoconf is already the newest version (2.71-3)",
  and the script gave 31 ok, exit 0.
- **Teeth of the empty-text guard.** See blocking finding 1.

### Local validation

- **sse.** Full build with `--clean-first` (GCC 13.3, Release, WITH_TESTS, WITH_EXAMPLES, `-j1` through
  gcore). `ctest -j1`: "100% tests passed, 0 tests failed out of 49". `intervalf` and `interval2f` were
  skipped, and `version_file` passed.
- **`tests/version_file.cmake`.** "28 checks, 0 failed" with CMake 3.14.7, 3.16.3, 3.28.3, 3.30.0, 4.0.0 and
  4.1.0.
- **Manual.** `sh manual/build-pdf.sh manual/v5 $SCR/tmp/cc/r2/pdf` gives 126 pages.
- **Lint.** `git diff --check` is clean. `check_branch configure-clean origin/configure-clean`: OK.
- **Merges.** `git merge-tree` with every `origin/todo-*` and `origin/fix-*` branch gives the same conflicts
  as with `origin/configure-clean`, except `todo-04-ftz-daz` (`gaol.pc.in`, already reported). `todo-39`
  merges cleanly.
- **Not run.** fpu and clang: no C or C++ file changed. `configure`, `configure.ac` and `meson.build` did not
  change in this round.

### Documentation changed in this round

`doc/tests.md` (bullet `version_file`), `doc/building.md` (The version of GAOL, With meson),
`doc/continuous-integration.md` (autoconf checked on Linux), `examples/examples.md` (section 3),
`manual/v5/gaol.tex` (the meson paragraph), and the comments of `tests/version_file.cmake`,
`cmake/gaol_version.cmake`, `.github/scripts/version-file.sh` and `.github/workflows/build-systems.yml`.

### Open questions after the review

1. **Commit author.** The four commits carry `Claude <noreply@anthropic.com>`, from `/root/.gitconfig`. The
   orchestrator should set the author (Jordan08) before pushing. The messages credit no one.
2. **The UTF-16 hint** is given for any file that starts with `FF FE` or `FE FF`, including UTF-32LE and
   `FF FE` followed by ASCII. The review calls this a harmless heuristic. I left it as it is, because
   telling UTF-32 apart would add code to all four readers for a case nobody writes.
3. **`gaol.pc.in` line 8** conflicts textually with `origin/todo-04-ftz-daz`. The resolution is to keep both
   changes. That branch already conflicts with `configure-clean` elsewhere.
4. **Cost of the autoconf step in the CI.** It runs on the four Linux entries of the job `autotools`,
   including the two with the rounding direction preserved. It takes about 10 s each. It could be limited
   to `matrix.cfg.configure == ''` if the maintainer prefers.
5. **Wording of examples.md after todo-39.** Section 3 describes the program that was reviewed. After
   todo-39 is merged, the manual has the +1, and recommendation 16 says "applied".

### For TODO.md (additions)

- **43** : le script `.github/scripts/version-file.sh` a un mode `autoconf` (autoconf -f dans une copie,
  puis `configure --version`), exécuté par le job autotools sur Linux. Il a aussi les cas de
  `tests/version_file.cmake` qui lui manquaient (deux et quatre nombres, un fichier vide, 0.0.0,
  12.345.6789), et `)` et `[`.

### For ChangeLog (addition)

	* .github/scripts/version-file.sh: an autoconf mode, run by the
	autotools job on Linux, which checks the version autoconf reads
	for configure --version; the cases of tests/version_file.cmake.
