# Review of point 31: `todo-31-upstream-patches`

Branch head `5bf9a23` (2 commits over `origin/configure-clean` `0f2963c`, which is
current). The only file changed is `3rd/README.md` (+814/-8). Reviewed in a detached
worktree. Nothing was committed or pushed.

**Verdict: changes requested (one blocking finding, a false statement in the new
text).** Everything else I checked against the real upstream sources holds.

## Blocking

1. **`3rd/README.md:772` and `:795-796` (patch 6, "The defect") say that only
   mingw-w64 and clang-cl get it wrong: "Two compilers get it wrong: ... Visual C++
   defines no `__x86_64__` and calls `fegetround()`, and the other systems are
   right."** This is false. Cygwin, and any other x86-64 target that uses newlib,
   defines `__x86_64__` but not `__WIN32__`/`__WIN64__`, so master takes the `>>3`
   branch. Newlib's `<fenv.h>` gives the rounding macros the values 0 to 3, so that
   branch fails there in the same way as with clang-cl.
   - Evidence:
     - newlib-cygwin HEAD `8f14542`, `newlib/libc/machine/shared_x86/sys/fenv.h`
       lines 124-127: `FE_DOWNWARD (1)`, `FE_TONEAREST (0)`, `FE_TOWARDZERO (3)`,
       `FE_UPWARD (2)`. It is Cygwin's header: it has `#ifdef __CYGWIN__` blocks,
       and its `fegetround()` in `newlib/libm/machine/shared_x86/fenv.c` returns 0
       to 3.
     - `clang-18 --target=x86_64-pc-cygwin -dM -E -x c /dev/null` prints
       `__CYGWIN__`, `__unix__` and `__x86_64__`, and no `__WIN32__`. GCC for
       Cygwin defines `_WIN32`/`__WIN32__` only with `-mwin32`.
     - An emulation on Linux: the upstream files were compiled with a header that
       includes `<fenv.h>` and redefines the four `FE_*` rounding macros as 0 to 3.
       The results were compared with an unmodified Linux build over 200,000 random
       arguments:

       ```
       == core-math (Cygwin/newlib FE_* emulated)
       cbrt(0x1.3a9ccd7f022dbp+0) dir 2: 0x1.1236160ba9b93p+0      <- upward, wrong
       differences cbrt/rsqrt/asinpi per dir N,D,U,Z:
         0 0 0 0 / 0 0 0 0 / 0 100034 99966 0
       == cm-patched
       cbrt(0x1.3a9ccd7f022dbp+0) dir 2: 0x1.1236160ba9b94p+0      <- right
         0 0 0 0 / 0 0 0 0 / 0 0 0 0
       ```

       The README's own example is wrong with master here. `asinpi` takes downward
       and upward for toward zero. The patch fixes both.
   - Suggested fix: make the list say "two compilers and Cygwin". For example,
     after the clang-cl bullet add: "**Cygwin** (GCC and Clang define `__x86_64__`
     but not `__WIN32__`, and the `<fenv.h>` of newlib gives `FE_*` the values 0
     to 3) is wrong in the same way: the downward and upward roundings are taken for
     toward zero." Then replace "and the other systems are right" with "and glibc,
     musl, macOS and the BSDs, whose `FE_*` are the values of the x87 control word,
     are right". Also update the intro at line 213 ("with mingw-w64 and with
     clang-cl"), and possibly the comment in the patch ("0x400 for FE_DOWNWARD with
     GNU libc and mingw-w64, 0x100 with the UCRT..., 1 with newlib"). This makes the
     case for the table form stronger, not weaker.

## Non-blocking

- **Line 247, "the function the programs reach under wine is not"** correctly
  rounded. The `fma()` in question is the software `fma()` of mingw-w64 11's
  `libmingwex.a` (`lib64_libmingwex_a-fma.o`: a `mulsd`/`addsd` and a split with
  mask `0xfffffffff8000000`). It is linked statically, so real Windows gets it too;
  it is not a wine artefact. I measured 5,416 wrong results out of 20,000 random
  triples, checked with exact rationals. Report open question 4 ("Real Windows may
  differ from wine") is therefore misleading.
  - Suggested wording: "the `fma()` of the `libmingwex.a` of mingw-w64 11 is not".
  - This matters beyond this branch. GAOL compiles CORE-MATH without `-mfma` for
    MinGW (`CMakeLists.txt:334`). Whether the mingw-w64 versions GAOL accepts
    (≥ 12 on x86-64, 12.2 on i686) have a correctly rounded `fma()` is a question
    for the maintainer (point 6 / 6b).
- **Line 222, table row "masks": "absent: its copies write `MANTISSA_MASK`".**
  glibc's `lgamma` writes `~UINT64_C(0) >> 12` (`e_lgamma_r.c:1003`). The text
  below says so, but the cell does not.
- **Rows "shift of `asinpi_acc()`" and "signed shifts of `cospi.c`": "no `asinpi` /
  `cospi` of CORE-MATH".** glibc does have CORE-MATH's `asinpif` and `cospif`
  (`sysdeps/ieee754/flt-32/s_asinpif.c`, `s_cospif.c`, since 2.42). The cells are
  right only for the double functions. Suggest "no `asinpi` of doubles from
  CORE-MATH".
- **Lines 517-518, "1 − |x| from 2<sup>−54</sup>" and "from 2<sup>−55</sup>".** No
  double below 1 is nearer to 1 than 2<sup>−53</sup>. Say that such samples round
  to 1 − 2<sup>−53</sup> or to ±1, or start at 2<sup>−53</sup>.
- **Line 232, "Each patch is a commit of its own on upstream master".** Block 6's
  `index` lines (`asinpi.c ce089f18..`, `rsqrt.c 6bc8cf64..`) are the blobs after
  patches 2 and 3, so the six are a series. Each block still `git apply`s alone on
  `b1a4badf`, but `git apply --3way` or `git am` of block 6 alone would not. Say
  "a series of six commits on ..., in this order".
- **Patch 1, `lgamma.c:753`.** `unsigned ft = (tl.u+2)&(~0ull>>12)` still truncates
  to 32 bits, so the test uses 32 bits (or 20), never 52, with or without the
  patch. The sentence "where 52 bits decide ... 20 bits decide it more often" does
  not quite fit this line. A note for the authors could say so.
- **Merge with point 6 (`todo-06-fegetround-mxcsr`, textually clean).** After both
  are merged:
  - Line 206 ("Four of the changes above are fixes ... (2 to 5)") becomes stale,
    because point 6 adds item 6, "a fix to propose".
  - Point 6's item 6 says `rsqrt.c`/`asinpi.c` "are right", which patch 6 here
    qualifies (x87-only `fegetround()` with mingw-w64; clang-cl; Cygwin).
  - Patch 6's status should say what GAOL's copy does.

  The orchestrator should reconcile these at the second merge.
- **House rules.**
  - Both commit subjects are slightly over 100 characters (104 and 107).
  - The commits are authored `Claude <noreply@anthropic.com>` (the git config of
    this session). No pushed todo branch has Claude-authored commits, so the author
    must be rewritten before pushing, as for the others.
  - `check_branch`: OK. No ledger files. The three builds are untouched.

## What I verified (all passed unless stated)

- **Upstream CORE-MATH.** Fresh clone of gitlab.inria.fr; master is still
  `b1a4badf` (29 Sep 2026).
  - The six diff blocks, extracted from the README: `git apply --check` of each
    alone on `b1a4badf` passes, and `patch -p1 --dry-run` gives no offset or fuzz.
    Applied in sequence, every resulting blob matches the `index` line of the
    README (e.g. `asinpi.c e6b13ece`, `cbrt.c 9323ab21`, `pow.h 5f21bdf3`,
    `sin.c 35fa7c0c`).
  - Every line reference is correct:
    - the masks in sinh 113/353/413, cosh 107/115/314/374, tanh 136/385, sinpi
      99/109, log1p 483 and lgamma 753;
    - `1l <<` in cos 401, tan 530 and sincos 392 (shift 0..51);
    - asinpi 273, rsqrt 172, exp2 362, sin 829;
    - pow.h 236/261/408;
    - the `get_rounding_mode()` branches: cbrt 84-101, rsqrt 84-101, asinpi 93-110;
    - cbrt `off[rm]` 204/205/215/216 and `rm+sign` 235;
    - cospi 179-181 at `6b84457`.
  - Commits `49d77f99` (5 Feb 2026, John Mather), `b1a4badf` (`m` made
    `uint64_t`, equivalent to GAOL's casts), `8c2bc708` (22 Sep) and `708e86ef`
    are as described.
  - `sin.c` is the only binary64 source calling `__builtin_roundeven` without the
    guard. Patch 4's block is identical to `exp.c`'s.
  - `pow.c` includes `<x86intrin.h>` before `pow.h`, so `_MM_GET_ROUNDING_MODE()`
    is available.
- **Reproductions.**
  - UBSan (GCC 13): master gives `asinpi.c:273:26: runtime error: shift exponent
    65 is too large...` for `0x1.ffffffffff0c7p-1`; the patched file is silent,
    and its results match mpmath.
  - MinGW-w64 11 x64 under wine, master:
    - `rsqrt(2^-1042..2^-1024)` = `inf` for the 10 powers of 4;
    - upward `cbrt(0x1.3a9ccd7f022dbp+0)` = `...b93p+0`.
  - The same with the patches: `0x1p+521..0x1p+512` and `...b94p+0` (mpmath: the
    root is 4.2e-16 ulp above `b93`).
  - `mingw fegetround` is `fnstcw` only (disassembled from `libmingwex.a`).
  - pow with the x87 unit to nearest and MXCSR upward, GCC and Clang: master gives
    `0x0.001539f0d791bp-1022` and the patch `...91cp-1022`, which is right
    (exact: 1458662373659.29·2^-1074).
  - pow downward, (-0x1.10a688680a753p-93)^11: master and patched both give
    `-0x1p-1022` with the underflow flag (spurious: the exact value is
    -0.99999999999999995·2^-1022).
  - Predefined macros: clang-cl target gives `__x86_64__ _WIN32 _WIN64`, no
    `__WIN32__`; the mingw GCC and Clang targets give `__WIN32__`; mingw
    `FE_*` = 0/0x400/0x800/0xc00.
- **asinpi next to ±1.** Exhaustive over [1−2^-37, 1), both signs, 4 directions
  (524,288 results), patched `cr_asinpi` with the 128-bit subtraction
  instrumented:
  - 0 wrong against mpmath (256 bits);
  - `cr_asinpi` reaches `asinpi_acc` 622 times, max ss 69, no wrap;
  - `asinpi_acc` called directly wraps exactly at ±0x1.fffffffffffffp-1,
    downward and toward zero (ss 76).

  This confirms the "for the authors" caveat and that `cr_asinpi` (and GAOL's
  port) never hits it in that range.
- **Object code (GCC 13, x86-64).** Unchanged by patch 1 for all nine files,
  by patch 3 alone for rsqrt, and by patch 4 for sin.
- **`sin.c` fallback forced** (portable, SSE4.1 and AVX forms): hash of
  300,000×4 results identical to master's.
- **Warnings.** `-W -Wall -Wextra -Wshadow`: 0 for the patched cbrt, rsqrt, asinpi,
  pow and sin with GCC 13 and Clang 18. MinGW: master's `#warning` fires, the
  patched files are clean.
- **CORE-MATH `./check.sh --worst`** on the patched tree (MPFR 4.2.1,
  `CORE_MATH_NO_OPENMP=1`), 4 directions, 0 failures: cbrt 212,508, rsqrt 19,844,
  asinpi 116,822 arguments.
- **glibc** (shallow master `30f988fbe795` plus the tags 2.40-2.44):
  - CORE-MATH files: floats from 2.41; acosh, asinh, atanh, erf, erfc, lgamma
    and tgamma of doubles in 2.43; cosh/sinh/tanh in 2.44; cbrt on master only.
  - `MANTISSA_MASK` = `UINT64_C(0x000fffffffffffff)`. Line numbers e_cosh
    97/106/222/295, e_sinh 105/213/286, s_tanh 104/284 (master), e_lgamma_r 1003
    are right.
  - None of the CORE-MATH files has `~0ul`, `1l <<` or a non-comparison
    `__glibc_(un)likely`.
  - asinpi/rsqrt/cospi of doubles are the generic templates; `s_sin.c` is IBM and
    `e_pow.c` is 2018 code.
  - `get-rounding-mode.h` (generic, used on x86) reads `_FPU_GETCW` = `fnstcw` and
    aborts otherwise. `fegetround.c` has the quoted comment.
  - `s_cbrt.c:37` `cbrt_rounding_index()` with `__builtin_unreachable()`; it is
    the only CORE-MATH double function reading the direction.

  "glibc needs nothing" holds.
- **Nothing sent.**
  - The author's CORE-MATH clone has no remote ref other than upstream's own
    branches, and no push in its reflog.
  - There is no `origin/todo-31*`.
- **GAOL.**
  - sse build (GCC 13.3, `-DWITH_TESTS=ON -DWITH_EXAMPLES=ON`, `-j1` through
    gcore) and `ctest -j1`: 100% of 49 passed (`intervalf` and `interval2f`
    skipped by design).
  - `git diff --check` is clean, all 15 internal anchors resolve, the fences are
    balanced, and `check_branch` is OK.
  - `git merge-tree` with every other local todo branch (including point 6) is
    clean.
  - Other variants were not built: only `3rd/README.md` changes, so no build or
    platform can be affected (the Visual C++, MinGW, macOS and container builds
    see nothing new).
- **Tests have teeth.** The branch adds no GAOL test, and needs none: it is
  documentation only and fixes no GAOL bug. For the upstream patches, the defects
  were shown on master and gone with the patch: UBSan for asinpi, wine for rsqrt
  and cbrt, x87/MXCSR for pow, and the newlib emulation for cbrt/asinpi.
