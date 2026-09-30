# Review of `todo-16-display-formats` (point 16, and the point 13 decision `[a]`)

Reviewed head: `24b048e`, against `origin/configure-clean` (`0f2963c`).
Worktree: `scratchpad/wt/review-16` (detached, removed at the end). Nothing
was committed or pushed.

**Verdict: changes requested.** There are three blocking findings:

1. Under a decimal-comma locale, `[a]` is read back silently as a different
   interval that may not contain the point.
2. The new DAZ output test will very likely fail on the macOS x86_64 CI job.
3. `doc/accuracy.md` says something false about the agreeing format.

The rest of the change is sound:

- The width and center formats are correct.
- `intervalToText` is correct.
- The zero rule is correct.
- The tests have teeth.
- The sse, fpu, clang and preserve builds all pass.

## Blocking

### B1. Under a decimal-comma locale, `[a]` is read back silently as a different interval, which may not contain the point

Where: `gaol/gaol_interval.cpp:771`, in `display_bounds()`. The docs that say
otherwise are `manual/v5/gaol.tex:3559-3562` and `gaol/gaol_interval.h:60`.

`operator<<` writes the decimal point of the stream's locale. This is by
design, and `tests/ieee1788.cpp` checks it with `[0,25, 0,5]`. Under a
decimal-comma locale, the new one-number literal `[a]` becomes a
two-number literal, and the reader (whose point is always `.`) accepts it
silently:

| interval | written by the branch | read back | on configure-clean |
| --- | --- | --- | --- |
| `interval(-2.5)` | `[-2,5]` | [-2, 5]: **does not contain -2.5** | `<-2,5, -2,5>`: refused (input_format_error) |
| `interval(-0.5)` | `[-0,5]` | [-0, 5]: **does not contain -0.5** | refused |
| `interval(12.5)` | `[12,5]` | the empty set: **does not contain 12.5** | refused |
| `interval(0.5)` | `[0,5]` | [0, 5] (encloses, but it is not the point) | refused |
| `interval(-denorm_min)`, fixed, 1074 digits | `[-0,000…447265625]` | [-0, +oo]: **does not contain it** | refused |

The same happens in three other cases:

- through `std::string(I)` under `std::locale::global(fr_FR)`;
- with `stream << I` then `stream >> J`;
- with the FPU build.

Other intervals are still refused loudly under a comma locale, as before: for
example `[-2,5, 3,5]` and `[0,1, 0,1000000000000001]`.

So the change turns a loud failure into a silent unsound read, in the
bounds format, which the manual calls the format read back as an input.

The manual says, with no condition on the locale, that `[a]` is read back
"as the same point". The header says it is read back "as that point".

No test writes a point with `operator<<` under the comma locale:
`tests/ieee1788.cpp` has that locale set, but it checks only
`[0,25, 0,5]`.

Evidence (probe `scratchpad/tmp/review-16/comma.cpp`, linked against the
branch's `libgaol.a`, then against `origin/configure-clean`'s):

```
branch:  imbue fr: -2.5  written [-2,5]   read [-0x1p+1, 0x1.4p+2]   DOES NOT ENCLOSE
branch:  imbue fr: 12.5  written [12,5]   read [-nan, nan]           DOES NOT ENCLOSE
base:    imbue fr: -2.5  written <-2,5, -2,5>  refused (exception)
```

`scratchpad/tmp/review-16/rt.cpp` gives the full table.

Suggested fix:

- In `display_bounds()`, write `[a]` only when the text cannot be read as two
  numbers: `if (lbits == rbits && left == right && left.find(',') == std::string::npos)`,
  or test that the `numpunct` decimal point of `os` is not `','`.
  Otherwise keep `[l, r]` (`[-2,5, -2,5]`), which the reader refuses, as it
  refuses every other non-integer interval under that locale.
- The zero text needs the same guard. With `fixed|showpoint` and
  precision 0, it is `[0,]`, which is read as [0, +oo].
- Add a check to `text_independent_of_the_output_settings()`, where the
  comma locale is already set: `os << interval(-2.5)` must be either refused
  or read back as an interval containing -2.5. It fails on the branch today.
- Say in the manual and the header that `[a]` is read back as the point where
  the stream writes a decimal point.

### B2. The new `subnormal_output()` test will very likely fail on the macOS 15 x86_64 job (native Intel), and possibly on Visual C++ x86 and x64

Where: `tests/numbers.cpp:313-343`.

How the test reaches the C library:

- It writes `[0, u]`, `[u]`, `[21u, 22u]` and similar intervals
  (u = 5e-324) under denormals-are-zero, on every x86 platform whose
  processor honours DAZ.
- Under DAZ, `x == 0.0` is true for a subnormal. So `bound_to_text()` sends
  the bound to the C library (`as_it_was << x`).

Why it fails on macOS:

- Apple's Libc printf comes from FreeBSD's. FreeBSD's `vfprintf.c:762` calls
  gdtoa's `dtoa()`.
- `dtoa()` begins with a floating-point test,
  `if (!dval(&d)) return nrv_alloc("0", ...)` (`contrib/gdtoa/dtoa.c:181`).
  Under DAZ that test is true for a subnormal.
- So on that platform every subnormal bound is written `0`, and the texts
  are read back as [0, 0].

Evidence:

- `scratchpad/tmp/review-16/fbsd_dtoa.c:181` and `fbsd_vfprintf.c:762`,
  fetched from cgit.freebsd.org.
- `dazdtoa.c` runs that exact test. It prints `without DAZ: not 0`, then
  `with DAZ: 0 (dtoa returns "0")`.
- `macsim.cpp` simulates such a C library: a `num_put` doing gdtoa's zero
  test, which `bound_to_text()` reaches because `copyfmt` copies the locale.
  Linked against the branch, it gives:

```
[0x0p+0, 0x0.0000000000001p-1022]   written [0, 0]  read [0x0p+0, 0x0p+0]  DOES NOT ENCLOSE
[0x0.0000000000001p-1022, …]        written [0]     read [0x0p+0, 0x0p+0]  DOES NOT ENCLOSE
... 7 of 7 DOES NOT ENCLOSE
```

Other platforms:

- The Universal C Runtime of Windows estimates the decimal exponent with a
  floating-point `log10(value)`, as far as I recall its `cfout.cpp`. Under
  DAZ this is suspect, but I have not verified it.
- glibc (the test passes here) and musl on x86_64 (it formats through x87
  long double, which MXCSR does not affect) are fine.
- The CI has not run this test yet: the last CI run was on `122e0f0`, and the
  test came with `65c2b58`.

The defect underneath is in `bound_to_text()` under DAZ. It is point 4's
domain, and was already there before this branch. The house rule is that
what a test needs from the platform is checked at run time, and the test says
that it skipped.

Suggested fix (either one):

- Guard the test. Under DAZ, write `denorm_min()` with a plain classic
  stream at 16 digits. If the text is not `4.940656458412465e-324`, print
  "the C library does not write subnormals under denormals-are-zero: not
  checked" and return.
- Or clear FTZ and DAZ in MXCSR for the duration of `operator<<` and
  `intervalToText` (point 4).

### B3. `doc/accuracy.md` says that `operator<<` in the agreeing format writes the same text as `intervalToText`, which is false

Where: `doc/accuracy.md:199`.

The row gives `operator<<` "in the formats `bounds`, `agreeing` and `hexa`"
as the counterpart of `intervalToText`, then says "`operator<<`: the same
[`[l, r]`, `[a]`, `[0]`, `[empty]`], with the digits, the flags and the
locale of the stream". It then makes an exception for the hexadecimal format,
but not for agreeing.

In the agreeing format, the branch writes (probe `agree.cpp`):

- `interval(4)` as `4.000000000000000`;
- `interval(1, 2)` as `~[1., 2.]`;
- `interval::zero()` as `~[-0., 0.]`;
- `interval(-0.0)` as `-0.000000000000000`.

Only negative intervals go through `display_bounds()`.

Suggested fix: "`operator<<` in the format `bounds` (and `agreeing` for a
negative interval): the same, …".

## Non-blocking

1. **Denormals-are-zero at low precision: a loud failure becomes a silent unsound read.**
   - This is the author's own Risks item. At 1 digit, `[22u]` is written
     `[1e-322]` and read as [20u, 21u]. `[21u, 22u]` is written
     `[1e-322, 1e-322]`.
   - The angles of the base were refused.
   - The cause is `bound_to_text()` (`x == 0.0`), which is point 4 and
     point 11. The comment "two equal texts are the double itself" in
     `display_bounds()` is false under DAZ for a subnormal.
   - To be settled with point 4. Note it in TODO.md.
2. **Commit identity.**
   - The six new commits (`8885870` … `24b048e`) have the author and
     committer `Claude <noreply@anthropic.com>`.
   - All of `configure-clean` and the first three commits of this branch are
     `Jordan08`.
   - `check_branch` does not flag it, and other agent branches of the session
     have the same identity. This is for the orchestrator to decide, since
     house rule 1 says nothing may credit Claude.
3. **Subject lengths.** The subjects of `a1699aa` (118 characters), `aee103e`
   (103) and `6798042` (102) are over the "about 100" of the house rules.
4. **Scope of the `Random` change.** The change to `tests/gaol_tests.h`
   (`Random`) goes beyond point 16. It changes the random population of every
   test program for Clang, and possibly for MSVC and for GCC targets that
   evaluate arguments in another order. It is justified, but watch the CI of
   all programs.
5. **Manual, input-format paragraph** (`gaol.tex:3443-3446`). It says
   "operator<< writes them [a]", but only the points whose digits are exact
   are written `[a]`. It also says GAOL 4 wrote "the form … which the reader
   still takes", but GAOL 4's `<0.1, 0.1000000000000001>` is not taken. The
   cross-reference explains it. Tighten the wording.
6. **`doc/tests.md` (ieee1788 entry).** It says "with 1 to 30 digits", but
   six precisions are tested: 1, 3, 8, 16, 17 and 30.
7. **Zeros in the agreeing format.** The agreeing format still writes a zero
   with the signs of its bounds (`~[-0., 0.]`, `-0.000000000000000`). This
   was already so before the branch. It does not match the "[0] whatever the
   signs" rule of the bounds format, which the manual justifies by one text
   on every build. The maintainer may want to note it.
8. **MinGW.** I could not check it locally: `gaol_config.h` refuses the local
   mingw-w64 11 with an `#error` (point 06b). The all-digits test
   (309 to 1080 digits) checks at run time that the C library writes the text
   exactly, so it is guarded.
9. **Unchanged open questions of the author,** which I agree are not
   regressions:
   - the width format under a comma locale, with `showpos`, and with digit
     grouping;
   - the center written `-0`;
   - the precision of `intervalToText`.

## What was checked and passed

- **Correctness of the width and center formats.**
  - `midpoint()` is written to nearest and `rad()` upward.
  - A point interval is written without a radius.
  - Unbounded intervals are right: `[3, inf]` is
    `1.797693134862316e+308 (+/- inf)`, the universe is `0 (+/- inf)`.
  - Pi is `3.141592653589793 (+/- 4.440892098500627e-16)`, which is 2^-51, as
    the manual says.
  - Interval `[0, 5e-324]` is written `0 (+/- 4.940656458412466e-324)`.
  - Both builds were checked: sse and fpu.
- **The zero rule.** `[-0,+0]`, `[+0,+0]`, `[-0,-0]` and `[+0,-0]` are all
  written `[0]`. With the flags, zero is `[+0]` under showpos, `[0.000]` in
  fixed with 3 digits, and `[0x0p+0]` with the hexfloat flags. All read back
  as {0}.
  - `interval::zero()` is `[-0x0p+0, 0x0p+0]` with SSE2 and
    `[0x0p+0, 0x0p+0]` with the FPU build, as the manual says.
  - The hexa format keeps the signs.
- **Flag variants of `[a]` all read back as the point:**
  - `[0x1p+2]`, `[0X1P+2]` and `[0x0.0000000000001p-1022]` (hexfloat);
  - `[+4]` (showpos);
  - `[4.]` (fixed with showpoint, precision 0);
  - `[4e+00]` and `[4.e+00]` (scientific, precision 0);
  - `[1.0000000000000000E+22]` (uppercase);
  - `[2e+22]` (1 digit).
  - `std::setw` pads the whole literal: `|     [4]|[0]     |`.
- **Round trips under the comma locale:**
  - the hexa format is bit-exact for zeros of both signs, subnormals, the
    empty set and the universe;
  - `intervalToText` is enclosing for all of them;
  - the bounds format is right for zeros, and refuses non-points as before;
  - the only failure is B1.
- **Portability, by reasoning:**
  - `memcpy` to `uint64_t` and `<< 1` behave the same on big-endian s390x,
    i386 (the store to double drops the excess precision), armhf and
    arm64.
  - `GAOL_NODISCARD extern __GAOL_PUBLIC__` follows the pattern of
    `gaol_ieee1788::pow`, so it is fine for Visual C++ DLLs.
  - No source, header, test or option is added, so the three builds, CMake
    3.14 and meson 0.53 are unaffected.
  - The locale probe of `ieee1788.cpp` catches `std::runtime_error` and
    skips.
- **House rules.**
  - `check_branch todo-16-display-formats`: OK.
  - No ledger files, no PDF, no `manual/v4`.
  - Commit messages are one line each, with no Claude trailer (but see
    non-blocking 2).
  - The manual compiles: `build-pdf.sh`, 126 pages, no undefined reference.
    The only overfull box is at line 501, which the branch does not touch.

## Tests have teeth

Each mutant was applied in my worktree, the tests were rebuilt (sse) and run,
and the tree was restored with `git checkout`.

| mutant | result |
| --- | --- |
| A: `display_bounds()` of configure-clean (`<a, a>`) | numbers exit 1, 5489 of 393034 failed ("in square brackets" 4573, "two bounds, or one for a point" 886, "all its digits" 9, the six forms, agreeing, DAZ 7+7); ieee1788 exit 1, 720 of 4109 |
| B: radius written with `bound_to_text(w, false, …)` (downward) | numbers exit 1, 76611 failed (contains 28161, rad() upward 31923, at least half the width 16277, not 0 246, four forms) |
| C: `intervalToText` without `imbue(classic)` | ieee1788 exit 1, 931 failed (literal 300, read back 300, text of the manual 330, the comma check 1) |

After the restore, all three variants were rebuilt (clean tree, HEAD
`24b048e`).

## Local validation (all through gcore, -j1, out-of-tree)

| variant | build | ctest |
| --- | --- | --- |
| sse (GCC 13.3) | 0 warnings | 49/49 passed (intervalf, interval2f skipped); numbers 405119, ieee1788 4113 checks, 0 failed |
| fpu (`-DGAOL_SIMD=OFF`) | 0 warnings | 49/49 passed, 2 skipped |
| clang-18 | only the old `gaol_nerrs` warning | 49/49 passed, 2 skipped |
| preserve (`-DGAOL_PRESERVE_ROUNDING=ON`), 4 targets | ok | rounding_direction 16944, numbers 405119, ieee1788 4113, input_output 23: 0 failed |
| mingw-w64 11 (cross, wine) | refused by `gaol_config.h` `#error` (point 06b) | not run |

Note on method: my first fpu run went through the mutants while they were
applied, and failed exactly like mutant A. I rebuilt all three variants after
the restore; the table shows the clean runs.

## Probes

All in `scratchpad/tmp/review-16/`:

| probe | what it shows |
| --- | --- |
| `comma.cpp` | B1, against the branch and the base |
| `rt.cpp` | round trips under the comma locale |
| `flags.cpp` | flag variants of `[a]` |
| `agree.cpp` | the five formats |
| `dazdtoa.c` | the gdtoa zero test under DAZ |
| `macsim.cpp` | B2 |
| `fbsd_dtoa.c`, `fbsd_vfprintf.c` | FreeBSD sources |
