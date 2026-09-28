# TODO

What is left to do on GAOL v5. What is done is in
[What differs from GAOL](doc/differences.md).

Points 4 to 32 and 36 to 42 come from the review of 2026-09-27,
[examples/examples.md](examples/examples.md), and from the check of the fixes
that followed it. "Review #n" is number n of its section 5, whose Appendix B
gives the fix and a regression test, validated on scratch SSE2 and FPU builds
but not applied. Its numbers 1, 9, 11, 13, 14 and 18 are fixed.

## Code

1. **The pow of the standard is written twice.** `gaol_ieee1788::pow`
   (`gaol/gaol_interval.cpp`) intersects x with [0, +oo] and takes x = {0}
   apart, and `gaol_pow_hybrid()` does it again. Fix: make the standard half
   of `gaol_pow_hybrid()`, the pow of Table 9.1 for an interval exponent, a
   function of `gaol/gaol_interval.cpp`, have `gaol_pow_hybrid()` add only the
   pown case of a degenerate integer exponent, and have `gaol_ieee1788::pow`
   call that half directly, apart from its integer exponents beyond the ints.

2. **The rounding direction is still checked more than once** by `pow(x, y)`
   where it takes exp(y log x) (an infinite bound, or a base from 0 with an
   exponent not above 0): `log()`, `*` and `exp()` check it three times; by
   `nth_root(x, q)` for q < 0, `inverse()` of `nth_root(x, -q)`, twice; and
   by `modulo_k_pi()`, twice. Fix: call the bodies of these operations after
   one check, as `tan()`, the relational functions and the negative powers
   now do (`uipow_rounded_upward()`, `interval::inverse_upward()`); bodies of
   `log()`, `exp()` and `*` without the check are to write.

3. **`pow(x, y)` is one double wide where the power at a corner is a double.**
   The lower bound is the double below CORE-MATH's value rounded upward, even
   where that value is exact: `pow([4], 0.5)` is [2 − 2^-52, 2], and cases
   149 to 152 and 163 of [doc/compare/special_cases.md](doc/compare/special_cases.md)
   are wider than IEEE 1788's result for this reason alone. Fix: keep the
   value as the lower bound where the power is exact, as `exp2`, `log2`,
   `nth_root(x, 3)` and the functions of Table 10.5 do.

## Wrong bounds

4. **Flush-to-zero and denormals-are-zero make the bounds wrong** (review
   #2): under them, `[1e-300] * [1e-20]` is [0, 0]. A program linked with
   `-Ofast`, `-ffast-math` or `-funsafe-math-optimizations` gets them from
   `crtfastmath.o`, which GCC then links in and whose constructor runs after
   GAOL's initialization. The `-fno-fast-math` of `gaol.pc` and `gaol::gaol`
   does not prevent it, and silences the `#error` of `gaol_config.h` at
   compile time, GCC applying the `-O` options first. Loading a plug-in or a
   Python module built with `-Ofast` sets them too. The probe of each
   operation, `1.0 + tiny == 1.0` in `round_upward_if_needed()`
   (`gaol/gaol_fpu.h`), sees the rounding direction only: `tiny` is 2^-60, a
   normal double. Fix:
   - probe with a subnormal, `1.0 + (subnormal + 0.0) == 1.0` with 2^-1060,
     which sees the direction, FTZ and DAZ in one comparison, and then clear
     FTZ and DAZ in MXCSR (`& ~0x8040`) as well. Measured at no cost on an
     i7-1185G7; to measure first on the processors of the continuous
     integration, an addition with a subnormal operand being slow on some x86
     processors. It is the only defence against plug-ins;
   - pass `-mno-daz-ftz`, which keeps GCC from linking `crtfastmath.o`, in
     the link options of `gaol.pc` and `gaol::gaol` where the compiler takes
     it (GCC 12 and later on x86; not checked: GCC 9.4 and Clang 18 refuse
     it);
   - say in `doc/using.md`, `doc/three-builds.md` and the manual that linking
     with these options, or loading code compiled with them, breaks the
     bounds.

5. **`-ffinite-math-only` is not refused** (review #4). The compiler then
   takes NaNs and infinities never to occur, in the inline functions of the
   headers too, which are compiled with the options of the program: the empty
   set having NaN bounds, `([1,2] & [3,4]).is_empty()` is false, and the hull
   of the empty set with [1, 2] is empty. The `-fno-fast-math` of `gaol.pc`
   protects only when it comes after the option on the command line.
   `-funsafe-math-optimizations`, and `-ffast-math -fno-finite-math-only`,
   which no macro reveals, let GCC fold the probe `1 + tiny == 1` into
   `tiny == 0` in inline code, so that `width()` after a change of direction
   is below the exact width. Fix: an `#error` on `__FINITE_MATH_ONLY__` in
   `gaol_config.h`, next to the one on `__FAST_MATH__`, a row in the refused
   options of `doc/three-builds.md`, and a compile test as in
   `tests/fp_strict`.

6. **`pow` misses a subnormal result where the x87 unit and MXCSR disagree**
   (review #3): on x86-64 with the glibc, the x87 unit rounding to nearest and
   MXCSR upward, as `exactinit()` of Shewchuk's predicates and of Triangle
   leaves them, in 205 of 400 random cases. CORE-MATH rounds those results
   itself, in the direction `fegetround()` gives, which the glibc reads from
   the x87 unit. Fix: in `gaol/core_math_port.h`, on x86-64, define
   `fegetround()` as a read of MXCSR, as CORE-MATH's own `get_rounding_mode()`
   of `rsqrt.c`, `asinpi.c` and `cbrt.c` does, and add `pow` with subnormal
   results to `tests/rounding_direction.cpp`, which already lists that state.

7. **`atanh([1, x])` is [DBL_MAX, +oo]** instead of the empty set (review #5),
   atanh being defined on (−1, 1); so is `atanh([1])`, and `atanh_rel` and
   `tanhRev` inherit it, while `atanh([-5, -1])` is empty. Fix: the empty set
   when `J.left() == 1.0 || J.right() == -1.0`, J being `I & [-1, 1]`, and
   the domain (−1, 1) in `doc/accuracy.md` and the manual.

8. **`interval("2", "1")` keeps the bounds [2, 1]** (review #6), as does
   `textToInterval(sl, sr)`: `is_empty()` is true, but adding [0, 1] gives
   [2, 2]. Fix: the canonical empty set at the end of the constructor when
   either text is empty or `!(tmpl.left() <= tmpr.right())`.

9. **`pow(x, n)` for large n** (review #7). The lower bound drops the square
   of the rest `ipow_exact_dn()` carries: 8 doubles below the tightest at
   n = 2^28 − 1, 1962 at 2^32 − 1, where the manual promises the tightest or
   one double beyond. Fix: `nl = std::fma(-h,h,p) + nl*(2.0*h - nl);`. Where
   the power of one bound under- or overflows, both bounds take the rounded
   products, which the SSE2 and FPU builds multiply in different orders:
   their bounds differ in 190 of 1600 random cases, against "the same bounds
   on every machine" of `doc/accuracy.md`. Make them agree, or say so there.

10. **`tan(interval(-M_PI_2, M_PI_2))` is [-oo, +oo]** where the tightest is
    ±1.63·10^16 (review #8). Fix: `narrower_than_pi = (w <= pi_dn)`: w, the
    width rounded upward, at most the double below π proves the exact width
    below π.

11. **`hausdorff()` on infinite bounds, `nb_fp_numbers()` on −0** (review #19
    and #20). `hausdorff(x, x)` is +oo for x = [1, +oo], and so is
    `hausdorff([1, +oo], [2, +oo])`, whose distance is 1: a fixed-point loop
    on a box with an unbounded side never stops. Fix, in both builds: equal
    bounds, the infinite ones included, are at distance 0.
    `nb_fp_numbers(-0.0, 1.0)` wraps round to 13830554455654793217, and
    `[1, 2] - 1` has the lower bound −0. Fix: number the doubles by the bits
    of `std::fabs(a)` and `std::fabs(b)`.

12. **The reader gives a wrong subnormal enclosure under flush-to-zero and
    denormals-are-zero**: `interval("1e-310")` is
    [0x0.fffffffffffffp-1022, 0x1p-1022], which misses 1e-310 (GAOL hung
    there before the bisection on bits). `gaol_compare_number()`
    (`gaol/gaol_interval_lexer.lpp`) tests `!(x > 0.0)` and takes
    `std::frexp()` of the candidate double, which DAZ reads as 0. Fix: take
    the exponent and the significand of the candidate from its bits.

## Crashes

13. **A null `const char*` crashes** (review #10):
    `interval((const char*)nullptr)`, `interval(std::getenv("UNSET"))` and
    `interval(1, 2) + nullptr` compile, the constructor not being `explicit`,
    and give a null pointer to `strlen` in the initialization of the lexer.
    Fix: `parse_interval()` returns false for a null pointer, the error
    messages write `(null pointer)`, and `interval(std::nullptr_t) = delete;`
    refuses `x + nullptr` at compile time.

14. **Long sums overflow the stack** (review #17): a sum of 100 000 terms read
    from a string, and expressions as long built through the API, recurse once
    per term in the evaluation and in the destruction of the tree. Fix: in
    `gaol_interval_parser.ypp`, fold each binary operation when it is read, as
    calls of functions already are, and regenerate the committed parser; for
    the expressions of the API, an evaluation and a destruction without
    recursion.

## Streams and text

15. **A point interval is written in a form the reader refuses** (review
    #12): `<0.1, 0.1000000000000001>` in the default format, which the reader
    refuses ("bounds of degenerate interval do not evaluate to the same
    value"), so that `textToInterval(intervalToText(interval(0.1)))` is the
    empty set, against the manual. Fix: in `display_bounds()`, write `<a, a>`
    only when the two texts are equal and `[l, r]` otherwise, and correct the
    manual where it shows `<0.1, 0.1000000000000001>`.

16. **`gaol_exception` does not override `what()`** (review #15):
    `catch (const std::exception& e)` prints `std::exception`, and so does an
    uncaught error. Fix: `what()` returns the explanation, and `operator<<`
    of the exceptions no longer prints both.

17. **`operator>>` throws on a blank line**, and on `in >> d >> x`, where the
    interval reads the empty rest of the line of the number: a file ending
    with a blank line ends `while (in >> x)` with `input_format_error`. Fix:
    read with `std::getline(is >> std::ws, buffer)`, which skips blank space
    as the reading of a number does, or say in the manual that a blank line
    is ill-formed. Reading one interval per token rather than per line would
    also read back what `os << x << ' ' << y` writes.

18. **The width and center formats** (review #21) print a center and a radius
    rounded to nearest, which need not enclose the interval ([1, 1 + 2^-52]
    is written `1 (+/- 1.11e-16)`), and a radius of 0 for [0, 5·10^-324],
    against the manual; the manual and `gaol/gaol_interval.h` describe the
    width format as the center and the width, where it prints the radius.
    Both formats write the empty set `empty`, where the manual says every
    format writes `[empty]`. `gaol_ieee1788::intervalToText` follows the
    global format and the locale of the stream, so its text is not always a
    literal of IEEE 1788. Fix: document the two formats as display formats,
    or round their radius upward; have `intervalToText` write the bounds
    format with a point.

19. **`operator<<` is about 450 ns slower per interval** since it writes in a
    `std::ostringstream` of its own (13 % for the bounds format, 2.2 times for
    the center format), and `std::internal` now pads as `std::right` does. To
    measure on programs that write many intervals; formatting in a buffer of
    characters rather than in a stream would save most of it.

20. **Reading a long number under a comma locale is slow**: each of the up to
    125 comparisons of `gaol_enclose_number()` parses the text again, in a
    time quadratic in its length: 2.5 s for 20 000 characters, against
    0.08 s under the C locale. Fix: parse the significand and the exponent
    once and compare them with each double, or give `strtod()` a copy of the
    text with the decimal point of `localeconv()`, so that its value is right
    again and two comparisons suffice.

## Headers

21. **`gaol::sin(0.5)` no longer compiles** (review #16): every function of
    namespace `gaol` on a double is ambiguous between the interval and the
    expression overloads, since `gaol_ieee1788.h` includes
    `gaol_expression.h`; GAOL 4 compiled it. Fix: stop including
    `gaol_expression.h` from `gaol_ieee1788.h`, and move the two expression
    overloads of `gaol_ieee1788` (`pown(e, n)`, `pow(e1, e2)`) to the end of
    `gaol_expression.h`.

22. **The public headers leak into the program** (review #22). The installed
    `gaol_interval_parser.h` does not compile when included, and it and
    `gaol_init_cleanup.h` are internal; `gaol_allocator.h` does not compile
    alone. They put `using std::exception; using std::string;` at global
    scope, define 25 unprefixed macros (`INLINE`, `HAVE_FENV_H`, `MEMALIGN`,
    `__HI`…) and `#undef PACKAGE`. With `-Wall -Wextra` and a plain `-I`
    (pkg-config, or FetchContent, whose include directory is not `SYSTEM`), a
    program gets about 35 warnings: 30 `-Wunused-parameter` in
    `gaol_expr_visitor.h`, and a `-Wdeprecated-copy` at every `x = ...;` (a
    user-provided copy constructor with an implicit copy assignment). Fix:
    stop installing the internal headers, remove the `using`s, prefix the
    macros, default the copy assignment and leave the unused parameters
    unnamed.

## Interface

23. **The literal 0 does not convert to an interval**: `interval(0)`,
    `interval(0, 0)`, `x = 0`, `x < 0`, `max(x, 0)` and `T(0)` in a template
    are ambiguous, 0 being a null pointer constant too, as close to
    `const char*` as to `double`. So `Eigen::Matrix<interval>`, which writes
    `Scalar(0)`, does not compile, and an integer beyond 2^53 becomes a double
    that does not contain it. Fix: constructors templated on the integer
    types, constrained, with `interval(std::nullptr_t) = delete;` (checked
    against every test program and the sources of the library; in the
    headers, no change of ABI). Not a plain `interval(int)`, which makes
    `interval(5L)` ambiguous.

24. **No total order for the containers.** `<`, `<=`, `>` and `>=` are the
    certainly relations of IEEE 1788, true for (∅, ∅): `std::set` drops
    overlapping intervals, and `std::sort` of a vector holding an empty
    interval reads past its end. Fix: a `gaol::lexicographic_less`, possibly
    a `std::less` specialization, and a warning in the docs against
    `std::sort`, `std::set`, `std::max`, `std::min` and `std::clamp` without a
    comparator.

25. **Handling the rounding direction.** `cleanup()` restores it only once,
    there is no scoped guard, and `rnd_keep()` is not presented as a barrier.
    Fix: a `gaol::restore_rounding()` callable any number of times, a guard
    computing a block to nearest (about 7 ns), and `rnd_keep()` documented.

26. **Floating-point flags and traps.** With traps enabled
    (`feenableexcept`), `1/[0,1]`, `log([0,1])`, `[1e308]*10` and every
    `is_empty()` of a computed empty set die with SIGFPE: the unbounded
    results come from overflows or divisions by zero, and `is_empty()`
    compares NaN bounds with a signaling comparison. Every operation raises
    the inexact flag. Fix: `is_empty()` with `std::islessequal`, at no cost,
    and a word in the docs that traps must be off.

27. **The helpers every algorithm writes**: `mulRevToPair` (a `div_rel` in
    two pieces), `inflate`, `bisect(ratio)` and `is_bisectable()` (`split()`
    cuts at the midpoint, ±DBL_MAX for a half-line), a mid-radius
    constructor, `hull()` and `intersect()` as functions, an enclosure of the
    width (`width()` is an upper bound), a `_iv` literal in a
    `gaol::literals` namespace, `erf` and `erfc` (CORE-MATH's are vendored),
    and the reverse functions of `atan2`, `pow(x, y)`, `max`, `min`, `sign`
    and `floor`, which IBEX writes itself.

28. **Decorations**, or at least a flag telling that an argument left the
    domain: `sqrt` of a negative box is empty and passes every inclusion
    test, and `interval(DBL_MAX*10)`, `x + INFINITY` and `x * NAN` are
    silently empty.

29. **A rounding direction that does not leak**: the rounding carried by each
    instruction (AVX-512, the FPCR of AArch64 in assembly), as inari does,
    three times faster on additions; in the direction of P2746.

30. **Optional headers above the scalar core**, from `examples/`: a box type
    (`box.h`), forward differentiation over intervals (`dual.h`), affine forms
    (`affine.h`); or GAOL traits for YalAA and an interval backend for
    VNODE-LP.

## Tests and continuous integration

31. **The test under a comma locale does not run in the continuous
    integration**: `tests/numbers.cpp` reads under `fr_FR.UTF-8` or
    `de_DE.UTF-8` only where one is installed. Fix:
    `sudo locale-gen fr_FR.UTF-8` in `.github/workflows/linux.yml`.

32. **Test suites and benchmarks where GAOL is absent**: run ITF1788 (all the
    operations of IEEE 1788) and the benchmark of Tang et al. (2021) on GAOL
    v5, and add Boost.Interval, the library users pick first, whose
    elementary functions are unsound, to `doc/compare/`.

## CORE-MATH

33. **Propose the fixes of the vendored sources upstream.**
    [3rd/README.md](3rd/README.md) lists five changes of the CORE-MATH sources;
    four of them are fixes rather than adaptations to GAOL: the masks `~0ull`
    of `sinh.c`, `cosh.c` and `tanh.c`, the signed shifts of `cospi.c`, the
    128-bit shift of `asinpi.c`, and the 64-bit `__builtin_expect` of
    `rsqrt.c`, which took subnormals for +0 wherever `long` has 32 bits.
    A fifth one is made in `gaol/core_math_port.h` rather than in the
    sources: `sin.c` calls `__builtin_roundeven()` unguarded since its rewrite
    (upstream commit `6b84457`), which GCC before 10 does not have, and GCC 9.4
    did not link it; the other sources take the builtin only from GCC 10 and
    Clang 17 and round by themselves before (`roundeven_finite()` of
    `exp.c`), which `sin.c` could do too.
    This is also how the work of GAOL v5 reaches the glibc, which imports
    CORE-MATH's functions (glibc 2.41 to 2.44; `cosh`, `sinh` and `tanh` in
    2.44) and runs on 32-bit Linux targets, where `long` has 32 bits too.
    To check: whether the glibc's copies still carry the masks `~0ul` and the
    64-bit `__builtin_expect`; if so, the same fixes are sent to the glibc
    (libc-alpha, with a `Signed-off-by` line: no copyright assignment to the
    FSF since August 2021).

## Documentation

34. **The coverage report** ([coverage/README.md](coverage/README.md)) was
    written on 2026-09-20: its lines not run no longer match the sources, which
    changed since (one grammar for the reader of strings, the lexer and the
    parser made reentrant, the relations `possibly_*`, `certainly_eq` and
    `certainly_neq` removed). To write again with `-DGAOL_COVERAGE=ON` and the
    target `coverage`, which need gcovr.

35. **The timings of [doc/compare/performance.md](doc/compare/performance.md)**
    were measured at `bb6f7e4` with files outside the sources changed
    (`bb6f7e4-dirty`), before the rounding direction was checked once per
    function of intervals (`exp()` 5.7 % faster since, `sin()` 3.6 %,
    `pow(x, y)` 3.4 %), and once in `tan()`, the relational functions, the
    negative powers and `sqrt_rel()`. To measure again on a clean commit, the
    machine doing nothing else (`doc/compare/code/run_bench.sh`).

36. **The FetchContent recipes fetch GAOL 4.** Those of `doc/using.md` and of
    the manual, and the `git clone` of `doc/building.md`, take the `master`
    branch of `Jordan08/GAOL`, which on 2026-09-27 is GAOL 4.2.3, 121 commits
    behind `MATH-CORE`: the programs of the docs do not compile against it,
    and there is no `v5.0.0` tag to pin. Fix: tag `v5.0.0`, or merge
    `MATH-CORE` into `master`, and write the tag in the recipes.

37. **A first program before the details.** `README.md` has no C++ code, the
    first program is at line 92 of `doc/using.md`, and no document shows an
    interval algorithm. Fix: a ten-line program in `README.md` that points to
    `examples/`, and a short tutorial (range enclosure and subdivision,
    Newton with `%` or `div_rel`, a contractor with the `*_rel` functions,
    branch and bound), which examples 03, 05, 06 and 07 already contain.

38. **What upward rounding does to the program**, in `doc/using.md` and in the
    common errors of the manual, with the table of section 2.8 of the review:
    `printf`, `strtod`, `lrint`, text round trips, TwoSum, and GCC reusing
    after `cleanup()` a double computed before it. `GAOL_PRESERVE_ROUNDING`
    as the way to keep the rounding of the program, with its measured cost
    (4.8 times on x + y, 1.4 to 1.6 times on exp and sin).

39. **A table of names** for the users of IBEX, Codac, C-XSC, Boost and IEEE
    1788 (section 2.3 of the review), with what `<`, `<=` and `==` mean in
    each: in C-XSC and PROFIL/BIAS, `<=` is the inclusion, and code ported
    from them compiles and changes meaning.

40. **The pitfalls in the common errors of the manual**: `sqrt(2)` on a
    number; `interval(m - r, m + r)` with doubles; the empty set passing every
    test; the domain without decorations; `split()` of a canonical interval;
    `/` against `%` in Newton's method; `pow(x, 2)` taking GAOL's pow with
    both namespaces open; a text refused for a non-integer order of
    `nth_root`, which throws `invalid_action_error` and not
    `input_format_error`; `textToInterval` giving the empty set for a
    malformed text; a zero bound written `-0`, whose sign differs between the
    SSE2 and FPU builds (`sqr([-1, 2])`, `interval::zero()`).

41. **The Goldstein-Price function of the manual drops the +1**
    ((x + y)² instead of (x + y + 1)²), as `examples/16_Goldstein_Price.cpp`,
    the example of GAOL 4, does, and the manual says that f "ranges over" the
    enclosure, about 150 times wider than the range of the true function.
    Fix: say that the enclosure contains the range, and give the true
    function or say that it is not.

42. **Small slips.** The header comment of `chi()` (`gaol/gaol_interval.h`)
    says `chi([0,0]) = 0` where the code and the manual say −1;
    `tests/gaol_tests.h` says the references use 400 bits where
    `tests/elementary_values.py` uses 2000; the comment of
    `interval(const char*)` names a `jail_parser.h` that does not exist; the
    root `version.h` is a Code::Blocks file of 2009 that nothing uses;
    `GAOL_NODISCARD` works from C++17 and `gaol::gaol` sets no language
    standard, so that a CMake project with GCC 9, in C++14, gets no warning
    for `sqrt(x);`; three examples of the manual show `true`/`false` where the
    program prints 1/0 (no `std::boolalpha`), and `nan` where it prints
    `-nan`; `check/` holds the CppUnit tests of GAOL 4, which CMake does not
    build, some of them for types no build compiles.

## Licence

43. **Move GAOL v5 to the MIT licence, as CORE-MATH.** GAOL v5 is under the
    GNU LGPL v2 of GAOL (`COPYING.LIB`), and only the holders of the rights can
    change that.
    - **Whose code it is** (`git blame` on 2026-09-22, outside `3rd/` and the
      parser written by Bison): in `gaol/`, 13 592 lines of Frédéric Goualard,
      5 650 of Jordan Ninin and 2 of Raphaël Chenouard; in `check/`, 3 148 and
      41; `tests/`, 11 242 lines, all of Jordan Ninin; the meson files, about
      740 lines of Raphaël Chenouard and 360 of Jordan Ninin.
    - **Who has to agree.** Frédéric Goualard, and the establishments the
      headers name: the EPFL (2001), the IRIN and the LINA (2002-2011, now the
      LS2N of Nantes Université); in France, software written by an agent in
      the course of their duties belongs to their employer (article L113-9 of
      the Code de la propriété intellectuelle), so the agreement goes through
      the technology transfer offices of these establishments. ENSTA, which
      holds the copyright of the files GAOL v5 adds. Raphaël Chenouard for the
      meson files, or they are written again. Nothing to ask for CORE-MATH
      (already MIT),
      `gaol/s_nextafter.c` (Sun's licence, permissive: its notice stays) and
      the parser written by Bison (its exception leaves the licence free).
      Rewriting Goualard's code instead is no way round: code rewritten from
      it remains a derived work.
    - **If the agreement is refused:** the files of ENSTA alone (`tests/` and
      the files GAOL v5 adds) under MIT, with ENSTA's agreement, reusable
      anywhere; the library as a whole stays under the LGPL.
    - **Once agreed:** `COPYING.LIB` replaced by a `LICENSE`, a line
      `SPDX-License-Identifier: MIT` in the headers, the section Licences of
      `README.md`, the `License:` of `gaol.spec.in`. The releases already
      published (GAOL 4.2.2) stay under the LGPL.
    - **What MIT brings.** One licence for GAOL v5 and CORE-MATH. No more
      doubt for software that is not free: the LGPL v2 (section 5) leaves a
      program compiled with the library free only if it takes from it "small
      inline functions (ten lines or less in length)", and
      `gaol_interval.h`, `gaol_interval_sse.h` and `gaol_interval_fpu.h`
      define about 120 `INLINE` functions. Code
      reusable by any project, which is how CORE-MATH entered the glibc. And
      MIT is in the list of licences the French administrations may use
      (article D323-2-1 of the Code des relations entre le public et
      l'administration), where the LGPL v2 is not (the LGPL-3.0-or-later is).
    - **What MIT loses.** The reciprocity: whoever improves GAOL v5 may
      distribute the improvements without their sources. No clause on patents
      (Apache 2.0 has one); a small risk here.
    - **GCC.** Its runtime libraries (libstdc++, libgcc) are under the GPLv3
      with the GCC Runtime Library Exception. Code under MIT can go in (libffi
      is); code under the LGPL v2 can be made GPL (section 3 of `COPYING.LIB`)
      but cannot receive the exception without its holders. A `Signed-off-by`
      line has been enough since June 2021. But libstdc++ holds only what the
      C++ standard defines, and interval arithmetic is not in it: N2137
      (Brönnimann, Melquiond, Pion, 2006) was not adopted. A new proposal to
      WG21, which could build on IEEE 1788-2015, would have to come first.
    - **glibc.** It is not part of GCC: it is the C library of Linux systems,
      whatever the compiler, and its libm computes the `exp` that `std::exp`
      of libstdc++ calls. It is under the LGPL-2.1-or-later and holds only C
      code for what ISO C, POSIX or GNU define, so the C++ classes of GAOL v5
      cannot go in, whatever their licence. The work of GAOL v5 reaches it
      through CORE-MATH (point 33), or through C patches for standard
      functions, sent to libc-alpha with ENSTA's agreement.
    - **References:**
      [Contributing to GCC](https://gcc.gnu.org/contribute.html),
      [glibc copyright assignment policy](https://sourceware.org/pipermail/libc-alpha/2021-July/129577.html),
      [licences of the French administrations](https://www.data.gouv.fr/pages/legal/licences),
      [NEWS of the glibc](https://sourceware.org/git/?p=glibc.git;a=blob;f=NEWS).
