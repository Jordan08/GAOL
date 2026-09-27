<!-- Copyright (c) 2026 ENSTA, France
     Created 2026-09-27 by Jordan NININ -->
# Using intervals with GAOL v5: examples, review and comparison

Part of the documentation of [GAOL v5](../README.md#documentation).

This report has three parts:

- **The examples** in this directory: 15 programs that use GAOL v5 the way
  interval arithmetic is used in practice, as IBEX, Codac, C-XSC, INTLAB and
  the textbooks use it, plus the example of GAOL 4 (section 1).
- **A review of GAOL v5 started from zero**: how a newcomer meets it, how
  natural interval code reads with it, and what is right or wrong in it
  (sections 2 to 5).
- **A comparison** with IBEX, Codac and ibex-affine, which build on GAOL, and
  with about 45 interval libraries by other authors, a dozen of which were run
  on the same computations (sections 6 and 7).

Section 8 gathers the recommendations, by priority. Every number in this
report comes from a program that was run, on an Intel Core i7-1185G7 under
Linux (glibc 2.31) with GCC 9.4 and Clang 18, and every enclosure was checked
against values computed apart (mpmath at 60 to 400 digits, or exact rational
arithmetic). Appendix A says how.

Contents:

1. [The examples](#1-the-examples)
2. [How intervals are used, and what GAOL v5 gives for it](#2-how-intervals-are-used-and-what-gaol-v5-gives-for-it)
3. [First contact: GAOL v5 through a newcomer's eyes](#3-first-contact-gaol-v5-through-a-newcomers-eyes)
4. [What was checked and found right](#4-what-was-checked-and-found-right)
5. [Bugs and limitations found](#5-bugs-and-limitations-found)
6. [IBEX, Codac and ibex-affine](#6-ibex-codac-and-ibex-affine)
7. [Other interval libraries](#7-other-interval-libraries)
8. [Recommendations](#8-recommendations)

Appendices: [A. How the review was done](#appendix-a-how-the-review-was-done),
[B. The fixes of the confirmed bugs](#appendix-b-the-fixes-of-the-confirmed-bugs).

## In short

- **GAOL v5's numbers are right, and as tight as doubles allow.** On more than
  400 000 checked cases (arithmetic on the special values, elementary functions,
  reverse functions, output) and 14.7 million sampled points, no bound of the
  SSE2 build missed the exact result, and single operations gave the tightest
  enclosure. Among the fast
  libraries on doubles that were measured (Boost.Interval, filib++, kv), GAOL
  is the only one whose elementary functions are both sound and tightest
  (`sin([1e22])` is one double wide), and it is the fastest (x + y in 4 ns,
  sin in 81 ns, against 22 ns and 152 ns for Boost.Interval).
- **Natural interval code is short with GAOL alone.** The examples reproduce
  IBEX's labs and tutorial and Codac's examples with a box helper and an
  automatic differentiation header of under 200 lines each: SIVIA, forward-backward
  contractors, interval Newton, Krawczyk, branch and bound, validated ODE
  steps, affine arithmetic. GAOL's reverse functions (`sqrt_rel`, `div_rel`,
  `asin_rel`…), rare among interval libraries, are exactly what contractors
  need, and they were found sound and tight on 40 000 random cases.
- **What makes natural code awkward** is not the arithmetic but its
  surroundings: the rounding direction left upward for the whole program
  (printf, strtod, lrint and the program's own doubles change), the literal
  `0` that does not convert to an interval, `operator<` which is not an
  ordering (std::sort and std::set misbehave, and can read out of bounds),
  stream input and output that break the idioms of iostreams, and the missing
  pieces every algorithm needs: a box type, derivatives, a two-piece division,
  inflation, bisection at a ratio.
- **Confirmed bugs**: two give bounds that miss the exact result (`x -= x`,
  `x /= x` and `x %= x` in the FPU build used on ARM and with Visual C++; `pow`
  with a subnormal result when the x87 and SSE rounding directions differ),
  one environment makes every operation unsound (flush-to-zero set by a
  program linked with `-Ofast`), the parser hangs under a locale writing a
  decimal comma, and several crashes and I/O bugs follow (section 5). Each
  has a fix and a regression test that were validated on scratch builds;
  none was applied to the library (Appendix B).
- **The documentation** is precise and its 86 examples of the manual are
  right, but it has no tutorial and no example of an interval algorithm, and
  its FetchContent recipe fetches the public `master`, which is GAOL 4.2.3,
  121 commits behind GAOL v5 (section 3).

## 1. The examples

### 1.1 Building and running them

`examples/CMakeLists.txt` is a project of its own, which uses an installed
GAOL as any user's project would:

```bash
cmake -S examples -B build-examples -DCMAKE_PREFIX_PATH=<prefix of GAOL>
cmake --build build-examples
ctest --test-dir build-examples
```

The three builds of GAOL build them too, with the GAOL they build, where they
are asked to (`-DWITH_EXAMPLES=ON`, `./configure --with-examples`,
`meson setup -Dwith-examples=true`), and `make check` (`ninja check` with
meson) runs them with the unit tests (see
[Building GAOL](../doc/building.md#tests-examples-performance-and-the-parser)).
The autotools and meson builds compiled `16_Goldstein_Price` only, as they
compiled `Goldstein_Price` before.

Examples 01 to 15 **check what they print**: each enclosure is compared with
a value computed apart (mpmath, closed forms, exact fractions), each proof
(uniqueness, emptiness) is tested, and each claim of the output ("4 times too
wide", "the excess is divided by 100") is proved with interval arithmetic. A
failed check prints `FAILED: <claim>` and the program returns a failure, so
that ctest runs the examples as tests. The checks test containments and
qualitative facts, never digits: they pass on every build.

All 16 examples compile without a warning (`-Wall -Wextra`, GCC 9.4 and
Clang 18) and pass their checks with the four builds of GAOL tried: SSE2 (the
default on x86), FPU (`GAOL_SIMD=OFF`, the code of ARM and Visual C++),
`GAOL_PRESERVE_ROUNDING=ON`, and the library compiled by Clang 18. Each runs in
less than 0.3 s with the default build (06 takes 1.3 s with
`GAOL_PRESERVE_ROUNDING`). Their outputs are the same on every build, apart from the
sign of a zero bound (`[-0, 4]` with SSE2, `[0, 4]` with the FPU code), from
the report of the rounding policy in 13, and from a digit or two where a
program computes its own doubles (the preconditioner of 10).

Every program follows GAOL v5's rules: `#include <gaol/gaol.h>`, no call to
`gaol::init()`, and `gaol::cleanup()` right after the last use of GAOL.

### 1.2 The programs

| File | What it computes | What it shows | After |
|---|---|---|---|
| `01_first_steps.cpp` | Intervals built every way, their numbers, sets and relations | Containment; `interval(0.1)` vs `interval("0.1")`; `interval(0)` does not compile; `mid()` vs `midpoint()`; no `==`; the certainly relations | IBEX `doc-arithmetic.cpp`, Codac's manual, GAOL's manual |
| `02_decimals_and_rump.cpp` | 0.1 summed ten times; π by Machin; Rump's polynomial | Decimals are not doubles; a raw literal `0.1_iv`; `sqrt(2)` on a number is the C library's; the width as an alarm | Rump 1988, INTLAB demos |
| `03_dependency_problem.cpp` | x − x, x(1 − x) three ways, (x − 1)^5, subdivision, Goldstein-Price, rotations | The dependency problem, single-use forms, `sqr` vs `x*x`, linear convergence of subdivision, the wrapping effect | Moore et al. 2009, IBEX lab1, filib++ `horner.cc` |
| `04_centered_form.cpp` | x cos x and Chebyshev T5, natural and mean-value forms | Automatic differentiation over intervals, quadratic convergence of the centered form, monotonicity test | Codac's `AnalyticFunction`, `02_centered_form` |
| `05_interval_newton.cpp` | A root of √x + (x+1) cos x; all 7 roots of sin x − x/10; ±√2 | Interval Newton, uniqueness proofs, the extended (two-piece) division, the trap of `f(m)/F'(X)` when 0 ∈ F'(X) | C-XSC `inewton.cpp`, `allzeros.cpp` |
| `06_global_optimization.cpp` | Global minima of Goldstein-Price and of the six-hump camel | Moore-Skelboe branch and bound, a priority queue with an explicit comparator, the cluster effect, mean-value form and monotonicity test | IBEX `doc-optim.cpp`, Hansen & Walster |
| `07_contractors.cpp` | IBEX's backward arithmetic; x² + y² = 1 ∩ y = eˣ − 1.5 | Forward-backward contraction (HC4Revise) with GAOL's reverse functions, their names in GAOL, IEEE 1788 and IBEX | IBEX `doc-arithmetic.cpp`, `CtcHC4` |
| `08_sivia.cpp` | Codac's SIVIA example; IBEX lab2 and lab3; √x + √y ∈ [0, 1] | Set inversion, guaranteed area brackets, contractors in pavers, the domain trap of a library without decorations | Codac `03_sivia`, IBEX lab2, lab3 |
| `09_parameter_estimation.cpp` | IBEX lab5; range-only localisation from three beacons | Bounded-error estimation by set inversion, measurements read as text, a contractor with `sqrt_rel` | IBEX lab5, Codac `13_qinter` |
| `10_krawczyk.cpp` | Two circles of IBEX's solver; the roots of λ + e^−λ | A branch-and-prune solver with the Krawczyk operator, why IBEX bisects at 45 %, a false proof from a wrong Jacobian | IBEX `doc-solver.cpp`, Codac `main_evans.cpp` |
| `11_ode_enclosure.cpp` | x' = −x²; Codac's Lohner system; ∫ exp(−x²) | Picard's a priori enclosure, Taylor steps with a remainder, the mean-value form against the wrapping of x + h f(x), interval Riemann sums | Codac `14_lohner`, tubes, VNODE-LP |
| `12_affine_arithmetic.cpp` | x − x, x(1 − x), (x − 1)^5, (x − y)², rotations with affine forms | A sound affine arithmetic on GAOL alone (`affine.h`), when it beats intervals and when it loses to them | ibex-affine, YalAA, INTLAB `affari` |
| `13_rounding_environment.cpp` | The program's own doubles, printf, lrint, strtod, threads, `cleanup()` | What GAOL's rounding direction does to the rest of the program, and how to live with it | `doc/using.md` |
| `14_generic_programming.cpp` | One template evaluated with `double`, `interval` and `Dual<1>` | Argument-dependent lookup, `using gaol::pow`, `T(0.0)`, the standard algorithms and containers with intervals | Boost.Interval and kv style |
| `15_text_and_ieee1788.cpp` | Intervals read and written as text; one contractor step with the names of IEEE 1788 | The literals GAOL reads, reading a file, exact output, the namespace `gaol_ieee1788` | IEEE 1788-2015, clauses 12 and 13 |
| `16_Goldstein_Price.cpp` | A million evaluations of a Goldstein-Price function, timed | The example of GAOL 4, kept as it was | GAOL 4 |

Three headers are shared:

- `box.h`: `examples::Box`, a vector of intervals with the names IBEX and
  Codac use for boxes (`widest()`, `max_width()`, `mid()`, `volume()`,
  `bisect(i, ratio)`, `&`, `|`). GAOL has no box type; every algorithm on
  several variables needs one.
- `dual.h`: `examples::Dual<N>`, forward-mode automatic differentiation over
  intervals: a function written once as a template gives its natural
  extension with `interval` and its gradient enclosure with `Dual<N>`, as
  Codac's `AnalyticFunction` does.
- `affine.h`: the affine forms of example 12.

### 1.3 What each example shows

**01 — A first program.** Everything a first program meets, each line checked:

```text
  interval(0.1)                [0x1.999999999999ap-4, 0x1.999999999999ap-4] one double: misses 1/10
  interval("0.1")              [0x1.9999999999999p-4, 0x1.999999999999ap-4] two doubles: encloses 1/10
  interval(a, b)               [empty]                                      a = 3.5 > b = -1.25: empty
  interval(a) | interval(b)    [-1.25, 3.5]                                 the hull: from b to a
  x - x                        [-1, 1]                                      not [0, 0]: the dependency problem
  sqrt(interval(-1.0, 4.0))    [0, 2]                                       sqrt of [0, 4] only
  u < v, u >= v                false, false                                 overlapping: neither holds
  empty < 0.0, empty > 0.0     true, true                                   no point to contradict them
```

**02 — Decimals and Rump's example.** `interval(0.1)` is a point that is not
1/10, and `sqr(interval(0.1))` lies entirely above 1/100 (proved with `>=`);
`interval("0.1")`, or the three-line raw literal `0.1_iv` built on it, encloses
1/10. `sqrt(2.0)` is the C library's double, while `sqrt(interval(2.0))`
encloses √2. `sin(interval(M_PI))` is certainly positive, which proves that
`M_PI` is not π. Rump's polynomial at (77617, 33096) gives −1.18·10²¹ in
doubles, silently wrong, and [−5.9·10²¹, 4.7·10²¹] in intervals, which contains
the true −0.827396…: the width is the alarm, and more precision (MPFI, Arb) is
the cure.

**03 — The dependency problem.** On [0.4, 0.6], `x*(1-x)` is 20 times too
wide, `x - sqr(x)` 40 times, and `0.25 - sqr(x - 0.5)`, where x occurs once,
gives the range. Horner's scheme for the expanded (x − 1)⁵ on [0.99, 1.01] is
1.6·10⁹ times wider than `pow(x - 1, 5)`. Subdivision converges linearly (10
times more pieces, 10 times less excess). The true Goldstein-Price function on
[−2, 2]² has the range [3, 1015690.27…], its maximum lying on the edge y = 2,
not at a corner; its natural extension is [−8.8·10⁷, 1.5·10⁸]. Rotating
[−1, 1]² by 45° eight times gives [−16, 16]²: the wrapping effect.

**04 — Natural and centered forms.** For x cos x on [0, 1], the natural
extension is [0, 1], the mean-value form f(m) + f'(X)(X − m) is
[−0.0612088, 0.938792] (Codac's documented value) and their intersection
[0, 0.938792]; on boxes of radius r, the excess of the natural form is
divided by 10 when r is, that of the centered form by 100. T5 on [−1, 1] is
[−41, 41] expanded, [−15, 15] in Horner form and [−1, 1] as cos(5 acos y).

**05 — Interval Newton.** C-XSC's `inewton.cpp`, ported: `0.0 <= f` (element
of, in C-XSC) becomes `f.set_contains(0.0)`, `x != xOld` becomes
`!x.set_eq(xOld)`; the root is proved at the first step and enclosed in
[2.059045253415143, 2.059045253415145]. Then the trap: with
X = [−10, 10], f(m) = 0 and F'(X) = [−1.1, 0.9] ∋ 0, IEEE 1788's division gives
0 / F'(X) = [0], so m − f(m)/F'(X) lies inside X and "proves" one root where
there are seven. The relational division `%` (or `div_rel`) gives the whole
line, and the extended division, computed as two `div_rel` on the negative and
positive parts of F'(X) (GAOL has no `mulRevToPair`), finds and proves all 7
roots in 19 steps.

**06 — Global optimization.** A best-first branch and bound needs an ordering
of boxes: `operator<` of intervals is the certainly relation, not an ordering,
so the queue takes an explicit comparator on `left()`. On the six-hump camel,
the natural extension needs 25 057 bisections to bring the enclosure of the
minimum down to 10⁻³ and does not reach 10⁻⁶ within 120 000 (the cluster
effect); the mean-value form reaches 10⁻⁹ in 1 127, and 261 with the
monotonicity test. On Goldstein-Price, the natural extension needs 110 432
boxes to reach a width of 1.

**07 — Contractors.** GAOL's relational functions are the backward operators
of HC4Revise. IBEX's backward example (z = x + y, sin z = −1) gives
z = [4.712388980384689, 4.712388980384691], one double tighter than IBEX's
documented output with GAOL 4.2.3, and sin z = 1 gives the empty set, a proof
that no solution exists. The inverse functions are not the reverse ones:
`[4, 6] & asin(-1)` is empty (a false proof), `sqrt([1, 4])` forgets the
negative roots that `sqrt_rel` keeps, and `c / y` with 0 ∈ y contracts
nothing where `div_rel` gives [1, 10]. A comment table gives each reverse
function under its GAOL, IEEE 1788 and IBEX names; `div_rel(c, y, x)` is
`mulRev(y, c, x)`.

**08 — SIVIA.** Codac's example is paved in 0.03 s and the area of the set is
bracketed in [18.92, 20.21], which holds 19.564; IBEX's lab2 and lab3 give
11 867 and 5 075 boxes (IBEX documents 11 891 and 5 165), the contractors
narrowing the bracket from [93.19, 108.45] to [96.84, 104.96]. Part 3 shows the
trap of a library without decorations: `sqrt([-1, -0.5])` is empty, and the
empty set is both a subset of [0, 1] and disjoint from it, so a paver that
tests inclusion first claims an area of 3.16 for a set of area 1/6; testing
`is_empty()` and the domain explicitly gives [0.157, 0.186].

**09 — Parameter estimation.** IBEX lab5, with its data read as text
(`interval("[0.67, 4.6]")`, since the doubles 0.67 and 4.6 miss both ends):
the feasible set's hull and area bracket contain the values computed apart
(area 0.0661137, in [0.0655, 0.0667]). One of the four corners from which
lab5 says its data were generated, (0.6, 0.2), is proved inconsistent with the
published measurements. Part 2 localises a robot from three distances, a
contractor built on `sqrt_rel` reducing the paving from 3 317 to 1 997 boxes.

**10 — Krawczyk.** A branch-and-prune solver in about 100 lines proves both
solutions (1/2, ±√3/2) of IBEX's two circles, and the three roots W₀(−1),
W₁(−1), W₂(−1) of λ + e^−λ in Codac's box, and proves there are no others. It
shows why IBEX bisects at 45 %: cut at 50 %, the solutions lie on the cut and
nothing is proved. A Jacobian written by hand with one sign wrong makes
Krawczyk's test "prove" a root in a box that does not contain it; the
Jacobian of `Dual<2>` does not.

**11 — ODE.** For x' = −x² from [0.9, 1.1], Picard's test gives an a priori
enclosure and Taylor steps with a remainder enclose x(2) within 1.001 of the
exact width once the initial set is cut in 10. On Codac's Lohner system, the
natural step x + h f(x) ends with a width of 4408 for an exact 9.08·10⁻⁶ (the
dependency between x and f(x) multiplies the width by 1 + h at each step
instead of 1 − h), while its mean-value form stays within 0.3 %: the core idea
of Lohner's method.

**12 — Affine arithmetic.** `affine.h` builds sound affine forms on GAOL's
public API only: each coefficient is computed as an interval and settled as
its midpoint, its radius (`mid_rad()`) going to the error term, so nothing
depends on the rounding direction. x − x is 0 (intervals: [−5, 5]); x(1 − x)
is exact on [0.49, 0.51] where intervals are 200 times too wide; after 16
rotations by π/4 the affine box is still the true one, while the interval box
is 256 wide. Affine forms lose on wide boxes of strongly nonlinear functions,
hence the recipe of IBEX and INTLAB: `affine.to_interval() & interval
evaluation`.

**13 — The rounding direction.** In a GAOL program, before `cleanup()`, the
program's own doubles are rounded upward:

```text
  1.0 / 3.0                       0x1.5555555555556p-2, 0x1.5555555555555p-2  the double above 1/3, then the nearest
  std::lrint(2.3)                 3, 2                                        lrint() rounds in the current direction
  printf("%.2f", 2.675)           2.68, 2.67                                  the C library rounds its decimal output too
  strtod("0.3")                   0x1.3333333333334p-2, 0x1.3333333333333p-2  and its decimal input: strtod("0.3") != 0.3
```

GAOL's intervals, on the other hand, are bit for bit the same whatever
direction the program leaves, in every thread. The example writes a small
guard that computes a block to nearest, shows that GCC may reuse inside it a
value computed before it (GCC bug 34678), and that `cleanup()` sets back the
direction only once. It adapts itself to the `GAOL_PRESERVE_ROUNDING` build,
where none of this happens.

**14 — Generic code.** One template for `double`, `interval` and `Dual<1>`:
`using std::sqrt;` finds GAOL's functions by argument-dependent lookup, but
`pow` needs `using gaol::pow;` (GAOL's `pow` and IEEE 1788's differ:
`gaol::pow([-4,-1], 2)` is [1, 16], `gaol_ieee1788::pow` gives the empty set);
accumulators start at `T(0.0)`, since `interval()` is the whole line and `T(0)`
does not compile; `std::max([1,3], [2,4])` silently returns [1, 3] where
`gaol::max` gives [2, 4]; sorting, `std::set` and `std::priority_queue` take an
explicit comparator.

**15 — Text and IEEE 1788.** `interval("...")` reads decimals, `[1, 2]`,
`1/3`, `sqrt(2)`, `2*pi`, the uncertain form `3.56?1`, `[1,]`, `[entire]`,
`[empty]` and hexadecimal floating-point numbers, always enclosing the number
written; a malformed text throws `input_format_error`, whose `explanation()`
says why. A file is read line by line with `std::getline` and
`interval(line.c_str())` in a `try` of its own. `exact_string()` reads back bit
for bit. The second part writes one contractor step with the names of the
standard (`numsToInterval`, `pown`, `mulRev`, `sqrRev`, `precedes`…) in a
function that opens `gaol_ieee1788` alone.

**16 — The example of GAOL 4.** It times a million evaluations and prints
`z = [-56254330, 94177270]` and `Elapsed time: 197` (no unit). Its function
drops the +1 of the Goldstein-Price function ((x + y)² instead of
(x + y + 1)²), and GAOL's manual, which shows the same program, calls the
enclosure "the range" of f, whose true range is about 150 times narrower.
Examples 03 and 06 use the true function.

## 2. How intervals are used, and what GAOL v5 gives for it

This section follows what interval code does, in the order a program does
it, and says for each idiom how it reads with GAOL v5.

### 2.1 Building intervals

| Idiom | With GAOL v5 | Remark |
|---|---|---|
| `[a, b]` from doubles | `interval(a, b)` | `interval(b, a)` with b > a is the empty set (IEEE 1788, as IBEX); the hull of two numbers of unknown order is `interval(a) \| interval(b)` |
| A decimal constant | `interval("0.1")` | `interval(0.1)` is one double, which is not 1/10. A raw literal `interval operator""_iv(const char* s) { return interval(s); }` makes `0.1_iv` enclose 1/10 (example 02) |
| A constant | `interval::pi()`, `interval("sqrt(2)")`, `sqrt(interval(2.0))` | `sqrt(2)` on a number is the C library's, a double rounded upward, which misses √2 |
| Zero | `interval(0.0)`, `interval::zero()` | `interval(0)`, `interval(0, 0)`, `x = 0`, `x < 0`, `max(x, 0)`, `T(0)` do not compile: the literal 0 is a null pointer too, and `interval(const char*)` competes with `interval(double)` |
| The empty set, the whole line | `interval::emptyset()`, `interval::universe()` or `interval()` | `interval()` is the whole line, not 0: `std::accumulate(v, interval())` is the whole line |
| m ± r | `m + interval(-r, r)` | `interval(m - r, m + r)` computed with doubles misses the ends (the doubles are rounded upward); there is no mid-radius constructor |
| From an integer beyond 2⁵³ | — | `interval(9007199254740993LL)` is one double that does not contain the integer |

### 2.2 Arithmetic and functions

Arithmetic mixes doubles freely: `2.0*x`, `x/3.0`, `1.0/x`. `x^2` is a
compile error, `sqr(x)` is tighter and twice as fast as `x*x`, and `pow(x, n)`
is the tight integer power. Functions keep the part of their argument in their
domain (`sqrt([-1, 4]) = [0, 2]`, `log([0, 1]) = [-oo, 0]`), as IEEE 1788
wants, silently: GAOL has no decorations. Division by an interval holding 0
gives the hull of the result, often the whole line; `%` is not a modulo but
the relational division (the reverse of the product).

Against the **dependency problem**, GAOL gives `sqr`, `pow` and the
cancellative `cancel_minus(x, x) = [0]`, and nothing else: the examples add
the centered form with `Dual<N>` (04), subdivision (03), affine forms (12) and
the mean-value form of a flow (11). Measured on x(1 − x) on [0.4, 0.6], whose
range is [0.24, 0.25]: `x*(1-x)` gives [0.16, 0.36], the mean-value form
[0.23, 0.27], the affine form [0.2499, 0.25] on [0.49, 0.51] (intervals: 200
times wider), 1000 pieces [0.23992, 0.25010].

### 2.3 Sets, tests and comparisons

| Idiom | GAOL v5 | `gaol_ieee1788` | IBEX | Codac |
|---|---|---|---|---|
| Lower, upper bound | `left()`, `right()` | `inf`, `sup` | `lb()`, `ub()` | `lb()`, `ub()` |
| Midpoint (a double) | `midpoint()` | `mid` | `mid()` | `mid()` |
| Midpoint (an interval) | `mid()` | — | — | — |
| Width, radius | `width()`, `rad()` | `wid`, `rad` | `diam()`, `rad()` | `diam()`, `rad()` |
| Hull, intersection | `x \| y`, `x & y` | `convexHull`, `intersection` | `\|`, `&` | `\|`, `&` |
| Subset, interior | `x.set_leq(y)`, `y.set_strictly_contains(x)` | `subset`, `interior` | `is_subset`, `is_interior_subset` | `is_subset`, `is_interior_subset` |
| Contains a number | `x.set_contains(d)` | `isMember` | `contains` | `contains` |
| Disjoint | `set_disjoint` | `disjoint` | `is_disjoint` | `is_disjoint` |
| Equal | `set_eq` | `equal` | `==` | `==` |
| Certainly less | `x < y` | `strictPrecedes` | — | `x < y` gives a three-valued `BoolInterval` |
| Bisect | `split(l, r)`, at the midpoint | — | `bisect(ratio)` | `bisect(ratio = 0.49)` |
| Inflate | `x + interval(-r, r)` | — | `inflate(r)` | `inflate(r)` |
| Two-piece division | two `div_rel`, on `J & negative()` and `J & positive()` | `mulRevToPair`: not provided | `div2` | — |

Three points need care in natural code:

- **The relations `<`, `<=`, `>`, `>=` are the certainly relations** of IEEE
  1788 (strictPrecedes, precedes): true when true for every pair of points.
  False does not mean the opposite (`[1, 3] < [2, 4]` and `[1, 3] >= [2, 4]`
  are both false). There is no `==`. In C-XSC and PROFIL/BIAS, `<=` means
  subset and `0.0 <= f` means "0 is in f": code ported from them compiles and
  changes meaning. None of the relations is a strict weak ordering, so
  `std::sort`, `std::set`, `std::map`, `std::max`, `std::min` and
  `std::clamp` must not be given intervals without an explicit comparator:
  `std::set` drops overlapping intervals, and `std::sort` of a vector holding
  an empty interval reads past the end of the vector (the empty set satisfies
  `e < e`, as IEEE 1788 requires, and libstdc++'s unguarded loops rely on
  `!(a < a)`).
- **The empty set satisfies every certainly-relation and every inclusion
  test.** Without decorations, `sqrt` of a negative box is empty and passes
  both "inside" and "outside" tests; a paver must test `is_empty()` first, and
  decide itself what a box outside the domain of f means (example 08).
- **`width()` is an upper bound** (IEEE 1788's wid, rounded upward). A lower
  bound of a width, an area or a ratio of widths is computed with intervals:
  `interval(x.right()) - interval(x.left())` (examples 04, 08, 11).

### 2.4 Bisection

`split()` cuts at the midpoint of IEEE 1788, which is 0 for the whole line and
±DBL_MAX for a half-line: bisecting [1, +∞] twenty times gives
[DBL_MAX, +∞], whose width stays infinite, so a loop that stops on
`width() < eps` never ends on it; `is_canonical()` is the right stopping test
(one half is the interval itself exactly when it is canonical). Cutting in
the middle also puts the solutions of symmetric problems on the cut: IBEX cuts
at 0.45 and Codac at 0.49, as `Box::bisect` does (examples 05, 10).

### 2.5 Reverse functions and contractors

GAOL's relational functions are the reverse functions of IEEE 1788, and the
backward operators of IBEX and Codac, which call them:

| GAOL v5 | IEEE 1788 (`gaol_ieee1788`) | IBEX | Reverse of |
|---|---|---|---|
| `sqrt_rel(y, x)` | `sqrRev(y, x)` | `bwd_sqr` | x² |
| `div_rel(z, y, x)` | `mulRev(y, z, x)` | `bwd_mul` | x·y (the argument order differs) |
| `nth_root_rel(y, n, x)` | `pownRev(y, x, n)`, n ≥ 1 | `bwd_pow` | xⁿ |
| `asin_rel`, `acos_rel`, `atan_rel` | `sinRev`, `cosRev`, `tanRev` | `bwd_sin`, `bwd_cos`, `bwd_tan` | sin, cos, tan |
| `acosh_rel`, `asinh_rel`, `atanh_rel` | `coshRev`… | `bwd_cosh`… | cosh, sinh, tanh |
| `invabs_rel` | `absRev` | `bwd_abs` | \|x\| |
| `x &= z - y` | — | `bwd_add` | x + y |

With them, a forward-backward contractor is a dozen lines (07, 08, 09), and
IBEX's documented results are reproduced. What is missing for Newton-type
algorithms is the two-piece division `mulRevToPair`, and reverse functions
for `atan2`, `pow(x, y)`, `max`, `min`, `sign`, `floor` (which IBEX writes
itself).

### 2.6 Generic code

A function written once as `template<class T> T f(const T& x)` serves doubles,
intervals and automatic differentiation. With GAOL:

- `using std::sqrt; sqrt(x)` finds GAOL's `sqrt`, `exp`, `log`, `sin`… by
  argument-dependent lookup;
- `pow` is not found so, on purpose (`gaol::pow` and `gaol_ieee1788::pow`
  differ): generic code writes `using std::pow; using gaol::pow;`, and the
  error message otherwise names `gaol_core::interval`, which the user never
  wrote; `pow(x, 2L)` and `pow(x, size_t)` are ambiguous;
- constants are written `T(0.0)`, never `T(0)` or `T{}`;
- `Eigen::Matrix<gaol::interval, ...>` does not compile, Eigen writing
  `Scalar(0)` and `Scalar(1)`; Codac wraps GAOL's interval in a class of its
  own for this reason;
- `std::complex<interval>` compiles for `+ - * /` and `exp`, not for `abs`,
  `norm` or `sqrt`, which need `==`.

### 2.7 Text

GAOL's reader is an expression language that encloses what it reads, the
literals of IEEE 1788 included, and its decimal output is rounded outward, so
that the text read back encloses the interval; `exact_string()` (or the hexa
format) reads back bit for bit. Four points break the idioms of C++ streams
(section 5.2): `operator<<` leaves the stream's precision set to
`interval::precision()` (16) and applies `std::setw` to the `[` only;
`operator>>` reads a whole line and throws at the end of the input, so
`while (in >> x)` always ends with an exception; a point interval is written
`<0.1, 0.1000000000000001>`, which the reader refuses; and under a locale
writing a decimal comma the reader hangs.

### 2.8 The rounding direction and the rest of the program

Unless GAOL is built with `GAOL_PRESERVE_ROUNDING`, its initialization, which
runs before `main()` in every file that includes GAOL, sets the rounding
direction upward for the whole program, until `gaol::cleanup()`. GAOL's own
results do not depend on it (each operation checks the direction and sets it
upward when it is not, in every thread), but the program's doubles do:

| Computed by the program | While GAOL is initialized | After `gaol::cleanup()` |
|---|---|---|
| `1.0/3.0` (at run time) | 0.33333333333333337 | 0.33333333333333331 |
| `std::stod("0.3") == 0.3` | false | true |
| `std::lrint(2.3)`, `std::rint(-2.7)` | 3, −2 | 2, −3 |
| `printf("%.2f", 2.675)` | 2.68 | 2.67 |
| `std::cout << std::setprecision(3) << 3.14159265358979` | 3.15 | 3.14 |
| 0.1 added 10⁷ times | 1000000.0005483569 | 999999.99983897537 |
| `%.17g` then `strtod` of random doubles | 200 000 of 200 000 come back different | all come back |
| TwoSum (error-free transformation) | not exact (10 646 of 50 000 pairs) | exact |

Other consequences:

- threads created after GAOL's initialization start upward;
- `gaol::cleanup()` sets back the direction only on its first call: after
  another interval operation the direction is upward again, and a second
  `cleanup()` does nothing (documented; `std::fesetround(FE_TONEAREST)` or
  `gaol::round_nearest()` is the way back);
- with GCC, a double computed before `cleanup()` may be reused after it,
  still rounded upward (GCC bug 34678), as a double computed before any change
  of direction;
- the libraries linked into the program run upward too: glibc's libm stays
  within 1 ulp for exp, log and pow but reaches 3.9 ulp for cbrt; Dekker's
  product and TwoSum are no longer error-free, which makes ibex-affine's
  default affine forms slightly unsound in a GAOL program (section 6).

`GAOL_PRESERVE_ROUNDING` removes all of this, at a measured cost of 4.8 times
on x + y, 6.7 times on x/y, 1.4 to 1.6 times on exp and sin, and 12 times on
a Horner polynomial mixing intervals and doubles. Switching to nearest for a
block of the program's own code costs about 7 ns (example 13).

## 3. First contact: GAOL v5 through a newcomer's eyes

**Installing and using GAOL is easy.** `find_package(gaol)` and
`gaol::gaol` worked at once and carry the flags of interval arithmetic; the
include directory is passed as `-isystem`, so a CMake user sees no warning
from GAOL; `gaol.pc` is relocatable; `-ffast-math` is refused with a clear
`#error`.

**Finding a first program takes some digging.** `README.md` has no C++ code;
the first program is at line 92 of `doc/using.md`, after the compiler flags;
the manual (124 pages) says it "assumes a prior knowledge of interval
arithmetic" and is a reference, operation by operation, not a tutorial. No
document shows an interval algorithm: the manual has no occurrence of Newton,
bisection, branch and bound, contractor, HC4, centered form or Krawczyk,
although it presents the relational functions as what "interval constraint
arithmetic software" needs. The docs are exact but dense (31 to 40 words per
sentence on average in `using.md`, `accuracy.md` and `tests.md`), and the
manual carries 81 `[GAOL v5]` markers and 41 mentions of GAOL 4, which a
newcomer does not need. `doc/accuracy.md`, the tightness of each operation as
IEEE 1788 (12.10.3) asks, is rare among interval libraries.

**The manual's examples are right.** The 86 example blocks of
`manual/v5/gaol.tex` with an expected output (102 outputs) were extracted,
compiled with GCC and Clang in C++11 and C++17, and run: all match, apart from
three formatting slips (`true`/`false` printed without `std::boolalpha`, and
`nan` printed `-nan`). All 172 names the manual documents exist in the
headers.

**What a newcomer trips on:**

- The FetchContent recipe of `doc/using.md` and of the manual, and the
  `git clone` of `doc/building.md`, fetch the `master` branch of
  `Jordan08/GAOL`, which on 2026-09-27 is GAOL 4.2.3 (`fc47222`), 121 commits
  behind `MATH-CORE`: it has `3rd/mathlib`, no `gaol_ieee1788.h` and no
  namespace `gaol_core`, so the programs of the docs do not compile against
  it. There is no `v5.0.0` tag to pin.
- A program compiled with `-Wall -Wextra` and pkg-config's `-I`, or through
  FetchContent (whose include directory is not `SYSTEM`), gets about 35
  warnings from GAOL's headers: 30 `-Wunused-parameter` in
  `gaol_expr_visitor.h` (included by `gaol_ieee1788.h` even when expressions
  are not used) and `-Wdeprecated-copy`, which GCC repeats at every
  `x = ...;` of the user's code (the class has a user-provided copy
  constructor and an implicit copy assignment). A 13-line `main.cpp` built
  through FetchContent printed 38 warnings.
- `[[nodiscard]]` works in C++17 only, and `gaol::gaol` sets no language
  standard, so a CMake project with GCC 9 compiles in C++14 and gets no
  warning for `sqrt(x);`.
- `examples/` held one benchmark of 2006, which the recommended CMake build
  did not build and no document mentioned; `check/` holds the CppUnit tests of
  GAOL 4, not built by CMake, some for types no build compiles (they have been
  in `tests/` since, run by the three builds without CppUnit).
- The public headers put `using std::exception; using std::string;` at global
  scope, define unprefixed macros (`INLINE`, `HAVE_FENV_H`, `MEMALIGN`,
  `__HI`… 25 in all) and `#undef PACKAGE` (gone since: the configuration no
  longer defines `PACKAGE`).
- Smaller slips: the width output format is described as "midpoint and
  width" but prints the radius; the header comment of `chi()` says
  `chi([0,0]) = 0` while the code and the manual say −1; `tests/gaol_tests.h`
  says the references use 400 bits where the scripts use 2000; the header
  comment of `interval(const char*)` names a `jail_parser.h` that does not
  exist; the root file `version.h` is a Code::Blocks file of 2009 that nothing
  uses (removed since).

## 4. What was checked and found right

The numbers are those of the SSE2 build unless stated; the references are
exact rationals or mpmath at 60 to 400 digits.

| What | Cases | Result |
|---|---|---|
| `+ - * /` on a grid of 170 special intervals (±0, subnormals, DBL_MIN, DBL_MAX, ±∞), with `x op= x` | 116 280 | sound and tightest |
| Operations with a double operand, NaN and ±∞ included | 22 610 | tightest; NaN and ±∞ give the empty set |
| `sqr`, `inverse`, `abs`, `min`, `max`, `fma` | 43 310 | tightest |
| `div_rel` and `%` | 48 393 | sound and tightest |
| 17 elementary functions at special and random intervals | ~10 000 | tightest |
| `sin`, `cos` near kπ/2, k up to 10³⁰⁰; `sinpi`, `cospi`, `tanpi` up to 2¹⁰⁰ | 13 144 and 9 000 | tightest |
| Reverse functions against the set definition (random, with ±∞, ±0) | 40 000 | 0 unsound, tightest or within 2 doubles |
| Reverse functions, solutions sampled | 14.7 million points | 0 missed |
| 30 natural expressions on 400 random intervals, at 3 inner points and the bounds | 57 123 points | 0 outside; the 4 296 monotone single operations tightest |
| Decimal output, precisions 1 to 17, fixed and scientific | 42 252 | all enclose |
| Parsing, uncertain form and extreme exponents included | 75 strings | tightest |
| Library built by GCC against library built by Clang | 32 838 operations | bit-identical |
| Results after the caller set nearest, downward, toward zero, in other threads | 17 operations × 5 inputs × 5 directions | bit-identical |
| Leaks (LeakSanitizer, valgrind), parser error paths included | — | none |

GAOL v5 also handles natively the special cases that IBEX's wrapper had to
patch in GAOL 4: `[1,2] + ∞` and a disjoint intersection are empty,
`log([-1, 0])` is empty, `nth_root([-8, 27], 3)` is [−2, 3], `cosh` of a
half-line and `asinh` of a negative interval are right, `atan2` is defined,
and a real exponent of `pow` is no longer truncated to an integer.

## 5. Bugs and limitations found

The first round of review reported candidates; each was then given to a
separate reviewer asked to refute it, who reduced it to a minimal program,
found its cause, and wrote a fix and a regression test, validated on scratch
builds: the test fails on the original library (SSE2 and FPU builds) and
passes with the fix, and the existing ctest suite still passes. **No fix was
applied to the library**: the changes to the sources are left to the
maintainer. Appendix B gives each fix and its test.

### 5.1 Wrong bounds

| # | What | Where | Severity |
|---|---|---|---|
| 1 | In the FPU build (`GAOL_SIMD=OFF`, the code of ARM, Visual C++ and 32-bit Windows), `x -= x`, `x /= x` and `x %= x` give bounds that miss the result: `c = [-3,-1]; c -= c` is [−2, 1], `a = [0.25, 0.5]; a /= a` is [0.5, 1], and `b = [0.1]; b /= b` is empty. A natural loop meets it: normalising a row by its pivot, `row[j] /= row[i]` down to j = i. The compound operator writes one bound of `*this` before reading the operand's, which is `*this` itself. | `gaol/gaol_interval_fpu.cpp:187`, `:636`, `:832` | high |
| 2 | Flush-to-zero and denormals-are-zero make every operation with a subnormal result unsound: `[1e-300] * [1e-20]` is [0, 0]. They are set by `crtfastmath.o`, which GCC links into a program linked with `-Ofast`, `-ffast-math` or `-funsafe-math-optimizations`, even when GAOL's `-fno-fast-math` silenced the `#error` at compile time (GCC applies `-O` options first), and whose constructor runs after GAOL's initialization; also by loading a plug-in or Python module built with `-Ofast`. The check of each operation (1 + 2⁻⁶⁰ > 1) sees the rounding direction only. | `gaol/gaol_fpu.h:204` | high |
| 3 | On x86-64 with glibc, when the x87 rounding bits say nearest and MXCSR says upward (what `exactinit()` of Shewchuk's Triangle and predicates does), `pow` with a subnormal result misses the exact value (205 of 400 random cases): CORE-MATH rounds those results itself in the direction `fegetround()` gives, which glibc reads from the x87 unit. GAOL's own test lists that state (`tests/rounding_direction.cpp:104`) but no subnormal `pow`. | `3rd/math-core/src/binary64/pow/pow.h:236, 261, 408` | medium |
| 4 | `-ffinite-math-only` is not refused, and makes the empty set invisible: `([1,2] & [3,4]).is_empty()` is false, and the hull of the empty set with [1, 2] is empty. `-funsafe-math-optimizations` and `-ffast-math -fno-finite-math-only` are not refused either (no macro reveals them), and GCC then rewrites the probe `1 + tiny == 1` as `tiny == 0` in inline code, so that `width()` after a change of direction is below the exact width. | `gaol/gaol_config.h:199` | medium |
| 5 | `atanh([1, 5])` and `atanh([1])` are [DBL_MAX, +∞] instead of the empty set (atanh is defined on (−1, 1)); `atanh([-5, -1])` is empty. `atanh_rel` and `tanhRev` inherit it. | `gaol/gaol_interval.cpp:2558` | low |
| 6 | `interval("2", "1")` (and `textToInterval(sl, sr)`) keeps the bounds [2, 1]: `is_empty()` is true, but `interval("2","1") + [0, 1]` is [2, 2]. | `gaol/gaol_interval.cpp:519` | low |
| 7 | `pow(x, n)` for large n: the lower bound loses the square of the rest it carries, 8 doubles below the tightest at n = 2²⁸ − 1, 557 at 2³¹ − 1, 1962 at 2³² − 1 (the manual promises the tightest or one double beyond). When one bound's power under- or overflows, both bounds take the rounded products, and the SSE2 and FPU builds multiply them in different orders: their bounds differ in 190 of 1 600 random cases, against "the same bounds on every machine" of `doc/accuracy.md`. | `gaol/gaol_interval.cpp:160` | low |
| 8 | `tan(interval(-M_PI_2, M_PI_2))` is [−∞, +∞] where the tightest is ±1.63·10¹⁶: the width test uses `<` where `<=` is sound. | `gaol/gaol_interval.cpp:2387` | low (tightness) |

### 5.2 Crashes, hangs and input/output

| # | What | Where | Severity |
|---|---|---|---|
| 9 | The reader hangs under a locale writing a decimal comma (`setlocale(LC_ALL, "")` with `LANG=fr_FR.UTF-8`, what Qt and GTK programs do): `interval("0.1")` never returns, nor `textToInterval` nor `operator>>`. `strtod` stops at the `.`, and the lexer then walks from its value one double at a time (4.6·10¹⁸ steps for 0.1). Under the same locale, `exact_string()` writes `0x1,999999999999ap-4`, which does not read back. | `gaol/gaol_interval_lexer.lpp:226` (and the committed `.cpp:960`), `gaol/gaol_interval.cpp:756` | high |
| 10 | `interval((const char*)nullptr)`, `interval(std::getenv("UNSET"))` and `interval(1, 2) + nullptr` compile and crash (`strlen` of a null pointer). | `gaol/gaol_parser.cpp:58`, `gaol/gaol_interval.h:114` | medium |
| 11 | `gaol_core::expression` objects created with no argument in two threads crash within a million iterations: they share one global node whose reference count is a plain `unsigned`. | `gaol/gaol_expression.cpp:76` | medium |
| 12 | A point interval is written `<0.1, 0.1000000000000001>` in the default format, which the reader refuses ("bounds of degenerate interval do not evaluate to the same value"), although the manual says the bounds format reads back; `textToInterval(intervalToText(interval(0.1)))` is the empty set. | `gaol/gaol_interval.cpp:728` | medium |
| 13 | `operator<<` sets the stream's precision to `interval::precision()` and never restores it: after printing an interval, `std::cout << 1.0/3` prints 16 digits. `std::setprecision` is ignored for intervals, and `std::setw` pads the `[` only. IBEX and Codac both work around it. | `gaol/gaol_interval.cpp:780` | medium |
| 14 | `operator>>` reads a whole line and throws `input_format_error` at the end of the input and on a blank line, without setting `failbit`: `while (in >> x)` always ends with an exception, and leaves x empty. | `gaol/gaol_interval.cpp:556` | medium |
| 15 | `gaol_exception` does not override `what()`: `catch (const std::exception& e)` prints `std::exception`, and an uncaught error ends with `what(): std::exception`. | `gaol/gaol_exceptions.h:56` | medium |
| 16 | `gaol::sin(0.5)` (and every function of namespace `gaol` on a double) is ambiguous between the interval and the expression overloads. It compiled with GAOL 4, whose `<gaol/gaol>` did not include `gaol_expression.h`; GAOL v5's includes it through `gaol_ieee1788.h`. | `gaol/gaol_ieee1788.h:70` | low (regression) |
| 17 | Parsing a sum of 100 000 terms overflows the stack (one frame per term, in the evaluation and the destruction of the tree), and so do expressions built through the public API. | `gaol/gaol_interval_parser.ypp:422` | low |
| 18 | `expression::operator/=` is declared but not defined: a program using it does not link. | `gaol/gaol_expression.h:72` | low |
| 19 | `hausdorff(x, x)` is +∞ for x = [1, +∞], and so is `hausdorff([1, +∞], [2, +∞])`, whose distance is 1: a fixed-point loop on a box with an unbounded side never stops. | `gaol/gaol_interval_sse.cpp:95`, `gaol/gaol_interval_fpu.cpp:30` | low |
| 20 | `nb_fp_numbers(-0.0, 1.0)` wraps around to 13830554455654793217; `[1, 2] - 1` has the lower bound −0. | `gaol/gaol_interval.cpp:1451` | low |
| 21 | The width and center output formats print a center and a radius rounded to nearest, which need not enclose the interval ([1, 1 + 2⁻⁵²] is written `1 (+/- 1.11e-16)`), and a radius of 0 for [0, 5·10⁻³²⁴], against the manual. `gaol_ieee1788::intervalToText` follows the global format and the global locale, so its text is not always an interval literal. | `gaol/gaol_interval.cpp:789`, `gaol/gaol_ieee1788.h:375` | low |
| 22 | The installed `gaol_interval_parser.h` does not compile when included; it and `gaol_init_cleanup.h` are internal. `gaol_allocator.h` does not compile alone. | `CMakeLists.txt:650`, `gaol/meson.build:121`, `gaol/Makefile.am:61` | low |

### 5.3 Behaviours to document, or to decide on

These are GAOL's design, or follow from IEEE 1788, but surprised every
reviewer and every example writer:

- `operator<` is IEEE 1788's strictPrecedes, true for (∅, ∅): not an ordering
  (section 2.3). A named total order and a `std::less` specialization would
  make the standard containers safe.
- The literal `0` does not convert to `interval` (section 2.1). Constrained
  template constructors for the integer types, plus
  `interval(std::nullptr_t) = delete`, fix it without making any existing call
  ambiguous (checked against all the test programs and the library sources),
  make `Eigen::Matrix<interval>` possible, and enclose integers beyond 2⁵³.
- The rounding direction leaks into the program (section 2.8), `cleanup()`
  restores only once, there is no scoped guard and no documented barrier
  (`rnd_keep()` exists but is not presented as interface).
- With floating-point traps enabled (`feenableexcept`), `1/[0,1]`,
  `log([0,1])`, `[1e308]*10` and every `is_empty()` of a computed empty set
  die with SIGFPE: the unbounded results are computed as overflows or
  divisions by zero, and `is_empty()` compares NaN bounds with a signaling
  comparison. Every operation raises the inexact flag. IEEE 1788 leaves the
  flags unspecified; the docs say nothing.
- A zero bound may be written `-0`, and its sign differs between the SSE2
  and FPU builds (`sqr([-1, 2])` is [−0, 4] with SSE2 and [0, 4] with the FPU
  code; `interval::zero()` is [−0, 0] with SSE2).
- With both namespaces open, `pow(x, 2)` compiles and silently takes GAOL's
  `pow` (only `pow(x, 2.0)` and `pow(x, y)` are ambiguous, as the docs say).
- A text refused because `nth_root` has a non-integer order throws
  `invalid_action_error`, not `input_format_error`; a reading loop catching
  the latter lets it escape.
- `gaol_ieee1788::textToInterval` returns the empty set for a malformed text,
  which cannot be told from `"[empty]"` without decorations.
- `interval(DBL_MAX*10)`, `x + INFINITY` and `x * NAN` are silently empty: a
  double computation that overflowed feeds the empty set to the rest of the
  program.

## 6. IBEX, Codac and ibex-affine

The three projects were read for how they use intervals, not to check them
against changes of GAOL.

**IBEX** wraps `gaol::interval` in `ibex::Interval` and adds what natural
code needs above scalars: short names (`lb`, `ub`, `mid`, `diam`, `rad`),
named predicates, `==` as set equality and no ordering operators, `inflate`,
`bisect(ratio)`, `diff`, `div2`, the `IntervalVector` and `IntervalMatrix`
types, functions from strings (`Function f("x", "y", "sin(x+y)")`) with their
gradients and backward evaluation, and composable contractors. Its labs and
tutorial are the model of examples 07 to 10: lab2 and lab3 give 11 867 and
5 075 boxes with GAOL alone (IBEX documents 11 891 and 5 165), the
q-intersection of the tutorial gives IBEX's box to the printed digits, and the
backward arithmetic example is one double tighter than IBEX's documented
output. Two things met on the way: IBEX's wrapper saves and restores the
stream's precision around GAOL's `operator<<` ("Gaol fix precision to 16 and
does not restore it"), and the data of lab5 do not contain the outputs of one
of the four corners (0.6, 0.2) from which lab5 says they were generated
(example 09 proves the inconsistency).

**Codac** 2 derives its `Interval` from `gaol::interval` (protected) and
forwards almost every operation to GAOL; its backward operators call GAOL's
relational functions (`SqrOp::bwd` is `sqrt_rel`, `MulOp::bwd` is `div_rel`,
`CosOp::bwd` is `acos_rel`). It adds the vocabulary of IBEX, a default
bisection ratio of 0.49, a three-valued `BoolInterval` for comparisons, the
Eigen-based vectors and matrices, and `AnalyticFunction`, whose `eval()`
intersects the natural and the centered forms, the derivatives coming from
forward-mode differentiation in each operator. Its examples 02, 03, 13 and 14
are the models of examples 04, 08, 09 and 11. Codac never calls
`gaol::init()` or `gaol::cleanup()`, so a Codac program, and a Python session
that imports codac, compute their own doubles rounded upward. Read in its
sources, not run: its `LohnerAlgorithm` computes the new center and the
inverse of `B1` in doubles, and the lower bound of its tube integrals multiplies
`diam()` (rounded upward) by a lower bound in doubles; example 11 keeps every
rounding error in intervals.

**ibex-affine** implements affine forms on IBEX intervals in two models: dense
forms whose rounding errors are bounded by error-free transformations
(`Affine2`), and sparse forms whose coefficients are computed as intervals
(`Affine3`). It uses no rounding control of GAOL, so making GAOL's `*_dn()` and
`*_up()` private does not affect it. Three findings, measured against IBEX
2.8.9 with its GAOL:

- the error-free transformations of `Affine2` assume rounding to nearest, and
  in a program using GAOL, which rounds upward, `twoSum` returns an error of 0
  for a nonzero one: `v0 + (-2^-60)` with v0 centered at 2⁵² + 2 gives a lower
  bound above the exact minimum;
- the Chebyshev linearizations of `sqrt`, `exp` and `log` include the global
  extremum of the residual, so on narrow boxes they are 10⁵ to 10¹⁴ times
  wider than the interval result (sqrt on [10⁶, 10⁶ + 1.1·10⁻⁹]: 49.4 against
  5.7·10⁻¹³);
- `sqr` of a form with several noise symbols has a negative lower bound
  (`sqr(e0 + e1)` gives [−2, 4]).

Example 12 avoids all three by computing every coefficient as a GAOL interval
and settling it with `mid_rad()`; it passed 2 000 random expressions checked
at 300 bits on the SSE2, FPU and preserve builds.

## 7. Other interval libraries

About 45 libraries and tools were studied, from their sources or their
documentation; a dozen were run on the same computations as GAOL v5.
`doc/compare/` already compares GAOL with libieeep1788, filib++, PROFIL/BIAS
and Solaris Studio on 288 special cases and on speed; this section does not
repeat it.

### 7.1 C and C++

| Library | Authors | Bounds | Directed rounding | IEEE 1788 | Beyond scalars | Status |
|---|---|---|---|---|---|---|
| **GAOL v5** | J. Ninin, after F. Goualard | double, lower bound negated | upward once for the program, checked at each operation | set-based, no decorations | reverse functions, expression reader | active |
| Boost.Interval | H. Brönnimann, G. Melquiond, S. Pion | any type, by policies | policies, `save_state` RAII guard | before the standard | none | maintenance only |
| C-XSC | U. Karlsruhe, U. Wuppertal | double, staggered multiprecision, complex | assembly or `fesetround` per operation | before the standard; `<=` is subset | vectors, matrices, exact dot product, toolbox (linear and nonlinear systems, global optimization, AD) | 2.5.4, 2014 |
| filib++ | M. Lerch, J. Wolff von Gudenberg et al. | template | six strategies, from per-operation switching to none | containment sets | none | 3.0.2, 2011 |
| PROFIL/BIAS | O. Knüppel, C. Keil | double | switched per operation | no empty set; aborts on a domain error | vectors, matrices, linear systems, AD, global optimization | 2.0.8, 2009 |
| libieeep1788 | M. Nehmeier | MPFR-computed doubles | MPFR | P1788 flavours, decorations | none | 2015 |
| MPFI | N. Revol, F. Rouillier | MPFR, any precision | MPFR | partial; NaN bounds on domain errors | none (Sollya, Sage use it) | active |
| Arb (FLINT 3) | F. Johansson | balls, any precision | software | not applicable | polynomials, matrices, integration, special functions | active |
| kv | M. Kashiwagi | double, double-double, MPFR | `fesetround` per operation, or TwoSum, or AVX-512 | throws on a domain error | AD, affine arithmetic, ODE, Krawczyk, integration | active |
| CAPD | Jagiellonian University | double, long double, MPFR | per operation; backends | before the standard | Lohner integration, Poincaré maps | active |
| Moore | W. Mascarenhas | any endpoint, C++20 | mandatory `UpRounding` RAII object | "not compliant" | linear algebra, AD | paper, 2018 |
| CGAL `Interval_nt` | S. Pion et al. | double, lower bound negated | upward, `Protect_FPU_rounding` guard | not applicable | filtered predicates, `Uncertain<bool>` | active |
| Ariadne | P. Collins, L. Geretti | double, MPFR | rounding tags per operation | its own model | Taylor models, ODE, hybrid systems | active |
| cuinterval | N. Kichler | double on the GPU | CUDA intrinsics with a rounding per instruction | all set-based operations, no decorations | GPU kernels; has erf, erfc | active (2026) |
| YalAA | S. Kiel | affine forms | delegated to filib, PROFIL or C-XSC | — | affine arithmetic | 2012 |
| VNODE-LP | N. Nedialkov | filib++ or PROFIL | backend's | — | validated ODE solver | mature |

### 7.2 Other languages

| Library | Language | Bounds | Rounding | IEEE 1788 | Beyond scalars |
|---|---|---|---|---|---|
| INTLAB (S. M. Rump) | MATLAB, Octave | inf-sup, mid-rad for matrices | switched per operation | before the standard | verified linear and nonlinear systems, eigenvalues, AD, affine arithmetic, multiprecision |
| Octave interval (O. Heimlich) | Octave | double, bare and decorated | MPFR, crlibm | full conformance claimed | verified `\`, `fzero` (all roots), `fsolve` (pavings), contractors, exact dot |
| IntervalArithmetic.jl | Julia | any float type | error-free transformations, no hardware mode | conformant, decorations, "not guaranteed" flag | root finding, optimization, contractors, Taylor models, reachability |
| inari (M. Mizuno) | Rust | GAOL's layout [−a; b] in one register | local to each instruction (MXCSR in asm, or AVX-512 embedded rounding) | 1788.1 with decorations | none |
| mpmath `iv` | Python | software floats, any precision | software | none | matrices, `lu_solve` |
| python-flint | Python | Arb balls | software | not applicable | polynomials, integration, roots |
| pyinterval | Python | unions of intervals | crlibm | none | Newton returning all roots |
| JInterval | Java | set-based and Kaucher flavours | selectable | P1788 structure | linear systems |
| Sun Fortran 95 `-xia` | Fortran | containment sets | libsunimath | before the standard | intrinsic type and operators |

Also read: Sage RIF/RBF, Mathematica `Interval` and `CenteredInterval`,
Maple `RealBox`, Haskell `rounded-hw` and AERN2, OCaml `interval`, Rival
(Herbie), IntvalPy, INT4Scilab, INTLIB/GlobSol, COSY Infinity, Flow*,
DynIbex, dReal, Coq.Interval, IGen, Sollya, libaffa and aaflib, immrax (which
states it does not bound rounding errors), and the npm `interval-arithmetic`
package.

### 7.3 Measured side by side

**Elementary functions at points** (11 functions at 8 points each, 74 cases
in their domains, checked at 300 bits):

| Library | Results missing the value | Median width | Worst width | sin([10²²]) |
|---|---|---|---|---|
| GAOL v5 | 0 | 1 ulp | 1 ulp | [−0.8522008497671889, −0.85220084976718879] |
| MPFI, 53 bits | 0 | tightest | — | same as GAOL |
| filib++ | 0 | 19 ulp | 54 ulp | [−1, 1] |
| kv | 0 | 7 ulp | 221 886 ulp | [−1, 1] |
| Boost.Interval (`rounded_transc_std`) | 20, 3 of them empty | — | — | [−1, 1] |

Boost.Interval calls the libm under a directed rounding, which glibc's
functions do not honour: `atan([1])` is a point below π/4 and `tanh([0.5])` is
empty; its float type, which libfive uses, fails 653 of 14 000 cases.

**Speed** (GCC 9.4, `-O2`, one core, ns per operation):

| Operation | GAOL v5 | Boost.Interval | kv |
|---|---|---|---|
| x + y | **3.96** | 22.06 | 13.91 |
| x × y | **16.07** | 33.85 | 28.79 |
| x / y | **13.31** | 29.54 | 22.15 |
| exp | **29.6** | 36.5 | 1478 |
| sin | **81.3** | 152.2 | 1233 |
| sqrt | 9.82 | 23.56 | 41.42 (9.37 with `KV_FASTROUND`) |

`x*(1-x) + sin(x)` on a moving interval: GAOL 61 ns, IntervalArithmetic.jl
223 ns (decorations and flags included), python-flint 1.2 µs, pyinterval 30 µs,
mpmath 40 µs. inari, whose addition keeps its rounding inside the instruction
(AVX-512), adds in 1.4 ns in a dependency chain where GAOL's out-of-line
`operator+=` takes 4.5 ns.

**The same small tasks everywhere:**

| Task | GAOL v5 | Octave interval | IntervalArithmetic.jl | mpmath | python-flint | IntvalPy |
|---|---|---|---|---|---|---|
| 0.1 added ten times | [0.9999999999999997, 1.000000000000001] | [0.99999, 1.0001] | `_com_NG` with a float, `_com` with `I"0.1"` | contains 1 | [1 ± 1e−15] | prints `[1, 1]` |
| x(1 − x), x = [0, 1] | [−0, 1] | [0, 1] | [0, 1] | [0, 1] | [± 1.01] (a ball) | [0, 1] |
| Rump, doubles | [−5.9e21, 4.7e21] | same | same | same | [± 1.1e22] | [−4.7e21, 3.5e21] |
| Rump, more bits | — | — | [−0.827397, −0.827396] | −0.82739605994682136814… | same | — |
| sin([10²²]) | 1 ulp | tight | tight | 1 ulp | tight | [−1, 1] |
| 1/[−1, 2] | [−∞, +∞] | `mulrev` gives 2 pieces | `extended_div` gives 2 pieces | [−∞, +∞] | nan | error |
| sqrt([−4, 4]) | [0, 2], silent | [0, 2], decorated `trv` | [0, 2]`_trv` | error | nan | [−4.9e−324, 2] |
| [1, 3] < [2, 4] | false (certainly) | true (strictLess); `==` set equality | error: inconclusive | error | false | true |
| interval(2, 1) | empty, silent | warning, empty | warning, NaI | error | — | — |

Two libraries print intervals that do not enclose their values: mpmath rounds
printed bounds to nearest, and IntvalPy prints `[1, 1]` for
[0.9999999999999998, 1.0000000000000007]. GAOL's decimal output is rounded
outward.

### 7.4 How the libraries get their rounding

Five families appear: the rounding direction set upward once, with the lower
bound negated (GAOL, CGAL, filib++'s `native_onesided_global`, Boost's
`rounded_arith_opp`), the fastest; the direction switched and restored at each
operation (filib++'s `native_switched`, the mode IBEX uses; BIAS; kv; C-XSC;
INTLAB), two switches per operation; a scoped guard (Boost's `save_state`,
CGAL's `Protect_FPU_rounding`, Moore's mandatory `UpRounding`); no hardware
direction at all (error-free transformations in IntervalArithmetic.jl and kv's
`KV_NOHWROUND`, MPFR, Arb, INTLIB's widening), which runs where no rounding
mode exists; and the rounding carried by each instruction (AVX-512 embedded
rounding in inari and kv, CUDA intrinsics in cuinterval), with no global state.

GAOL is the only member of the first family that checks the direction at each
operation, which keeps it right after foreign code changed the direction. But
this family is the one C++ is moving away from: P2746 (H. Boehm, 2024,
targeting C++26) proposes to deprecate `fesetround()` in favour of operations
taking their rounding as an argument, and names interval arithmetic as one of
its only two known uses; C23 adds `#pragma STDC FENV_ROUND`.

### 7.5 What GAOL v5 does better, and what it can take from them

Better: the only fast library on doubles measured whose elementary functions
are sound and tightest; the fastest arithmetic measured; IEEE 1788's set-based
handling of domains, cleaner than NaN bounds (MPFI), exceptions (kv) or aborts
(PROFIL); reverse functions, which among the kernels only libieeep1788 and
Julia's IntervalContractors also have; enforced compiler flags; tests and
continuous integration on many platforms.

To take:

- **Rigorous literals**: `I"0.1"` (Julia), `intval('0.1')` (INTLAB),
  `interval!("[0.1]")` (inari); in C++, a raw literal `operator""_iv` on
  `interval(const char*)`.
- **Loud misuse**: Julia flags an interval that met a plain float as "not
  guaranteed", and refuses ambiguous comparisons; Octave warns on
  `interval(2, 1)`; decorations report a domain left.
- **Named comparisons**: Boost's `certain::`, `possible::`, `set::`
  namespaces, CGAL's `Uncertain<bool>`, Codac's `BoolInterval`,
  mbeutel/intervals' `set<bool>`.
- **Mid-radius construction**: `midrad(m, r)` in INTLAB and Octave, `m ± r` in
  Julia.
- **The two-piece division**: `mulrev` with two outputs (Octave),
  `extended_div` (Julia), `mul_rev_to_pair` (inari), `div2` (IBEX).
- **Scoped rounding**, and a path without global state for when P2746 lands:
  inari computes GAOL's very layout with the rounding carried by each
  instruction, 3 times faster on additions with AVX-512.
- **erf and erfc**, monotone, whose CORE-MATH versions are already vendored.
- **Test suites and benchmarks** where GAOL is absent: ITF1788 (all the
  operations of IEEE 1788), the cross-platform benchmark of Tang et al. (2021,
  which found Boost unsound and filib++ wrong on some expressions), and traits
  for YalAA (affine arithmetic) and VNODE-LP (validated ODEs).

## 8. Recommendations

### Priority 1: the bounds

1. Apply the fixes of the wrong bounds of section 5.1 and of the hang, the
   crashes and the link error of section 5.2 (numbers 1, 3 to 11, 18 to 20),
   with their regression tests (Appendix B). Each is small and was validated on
   the SSE2 and FPU builds.
2. Decide on flush-to-zero (number 2). The fused probe of Appendix B detects
   the rounding direction, FTZ and DAZ in one comparison, measured at no cost
   on an i7-1185G7; it should be measured on the other processors of the
   continuous integration (an addition with a subnormal operand can be slow on
   some x86 processors) before it replaces the probe of every operation. At
   the least, document that linking with `-Ofast` or `-ffast-math`, or loading
   code built so, breaks the bounds, and refuse `-ffinite-math-only`.
3. Make the SSE2 and FPU builds agree on the rounded-product powers, or say in
   `doc/accuracy.md` that they differ (number 7).

### Priority 2: the rounding direction and the rest of the program

4. Document concretely what upward rounding does to the program (the table of
   section 2.8: printf, strtod, lrint, text round trips, TwoSum, GCC's reuse
   across `cleanup()`), in `doc/using.md` and the manual's common errors.
5. Offer `gaol::restore_rounding()`, callable any number of times, a scoped
   guard computing a block to nearest, and a documented barrier (`rnd_keep()`
   exists); document `GAOL_PRESERVE_ROUNDING` as the way for a program to know
   the policy, and its measured cost.
6. Document the floating-point flags GAOL raises and that traps must be off;
   `is_empty()` with a quiet comparison (`std::islessequal`) would stop raising
   FE_INVALID at no cost.

### Priority 3: natural code

7. Accept the literal 0 and every integer (constrained template constructors,
   `interval(std::nullptr_t) = delete`), which also opens Eigen.
8. Give a total order for containers (`gaol::lexicographic_less`, and possibly
   a `std::less` specialization), and warn against `std::sort`, `std::set`,
   `std::max` with the certainly relations.
9. Fix the streams (numbers 12 to 15): restore the stream's state and honour
   `setw`, read one interval token and set `failbit`, write point intervals as
   `[a, b]` when their two texts differ, make `what()` the explanation.
10. Add the helpers every algorithm writes: `mulRevToPair`/`div_rel_pair`,
    `inflate`, `bisect(ratio)` and `is_bisectable()`, a mid-radius
    constructor, `hull()` and `intersect()` as functions, an enclosure of the
    width, and the `_iv` literal in a `gaol::literals` namespace; `erf` and
    `erfc`.
11. Remove the global `using` declarations and prefix the macros of the public
    headers; stop installing the internal headers; silence the header warnings
    (a defaulted copy assignment, unnamed unused parameters); restore
    `gaol::sin(0.5)` by not including `gaol_expression.h` from
    `gaol_ieee1788.h`.

### Priority 4: documentation

12. Tag `v5.0.0`, or merge `MATH-CORE` into `master`, and write the tag in the
    FetchContent recipes.
13. Put a ten-line program in `README.md`, and point to this directory. Add a
    short tutorial (range enclosure and subdivision, Newton with `%` or
    `div_rel`, a contractor with the `*_rel` functions, branch and bound),
    which examples 03, 05, 06 and 07 already contain.
14. Add a table of names for users of IBEX, Codac, C-XSC, Boost and IEEE 1788
    (section 2.3), with the meanings of `<`, `<=` and `==` in each.
15. List the pitfalls of section 2 in the manual's common errors: `sqrt(2)` on
    a number, `interval(m - r, m + r)`, the empty set passing every test, the
    domain without decorations, `split()` of canonical intervals,
    `/` against `%` in Newton's method.
16. Fix the Goldstein-Price function of the manual (the missing +1) and say
    "encloses the range", not "ranges over".

### Priority 5: longer term

17. Decorations, or at least a "domain left" flag.
18. A rounding that does not leak: per-instruction rounding (AVX-512, the
    FPCR of AArch64 in asm), as inari does, in the direction of P2746.
19. Optional headers above the scalar core, from the examples: a box type,
    forward differentiation over intervals, affine forms; or GAOL traits for
    YalAA and an interval backend for VNODE-LP.
20. Add Boost.Interval to `doc/compare/`, which is the library users pick
    first and whose elementary functions are unsound; run ITF1788 and Tang et
    al.'s benchmark on GAOL.

## Appendix A. How the review was done

The review started from the repository alone (`README.md`, `doc/`,
`manual/v5/`, the headers and sources of `gaol/`, `tests/`), with GAOL v5 at
`605728e` (branch `MATH-CORE`) built with CMake in Release and installed in a
scratch prefix; its 13 tests passed. Variants were built the same way: the FPU
code (`GAOL_SIMD=OFF`), `GAOL_PRESERVE_ROUNDING=ON`, and the library compiled
by Clang 18; the autotools and meson builds were run to check the example of
GAOL 4.

Each question was answered by probe programs compiled as a user compiles them
(the flags of `gaol.pc`, the include directory as `-isystem`), about 400 in
all. Enclosures were checked against mpmath (Python, 60 to 400 digits, or 2000
bits for the elementary functions) or exact rationals (`fractions`). IBEX
(`IBEX_LAST/ibex-lib`), Codac and ibex-affine were read from their local
copies; the other libraries from local archives (C-XSC 2.5.4, filib++ 3.0.2,
PROFIL/BIAS 2.0.8, INTLAB 9, YalAA 0.92, INTLIB, GlobSol) or fetched into the
scratch space (Boost 1.86, kv 0.4.62, MPFI, IntervalArithmetic.jl 0.22 under
Julia 1.9.3, Octave's interval package 3.2.1, inari 2.0, mpmath 1.4,
pyinterval, python-flint, IntvalPy), without installing anything on the
system. Timings were taken on one core of a shared machine and are
indicative; the ratios were stable from run to run.

Each bug candidate was then reduced by a second reader asked to refute it,
and its fix and test were written against a copy of the sources: the test was
shown to fail on the original library, SSE2 and FPU builds, and to pass with
the fix, the whole ctest suite passing too.

## Appendix B. The fixes of the confirmed bugs

The numbers are those of section 5. None is applied in the repository.

**1. `x -= x`, `x /= x`, `x %= x` in the FPU build.** In
`gaol_interval_fpu.cpp`, read the operand's bounds before writing:

```cpp
  interval& interval::operator-=(const interval& I)
  {
    // I may be *this (x -= x): its bounds are read before one is written
    const double ilb = I.lb_, irb = I.rb_;
    GAOL_RND_ENTER();
    lb_ += irb;
    rb_ += ilb;
    ...
```

and, in the P1-P1 cases of `operator/=` and `operator%=`,
`double tmp = lb_/I.rb_; rb_ /= -I.lb_; lb_ = tmp;`. Test
(`tests/arithmetic.cpp`, next to `-[x]`): `r += r`, `r -= r`, `r *= r`, and
without 0 in x `r /= r` and `r %= r`, each against the exact extremes; on the
original FPU build, 12 989 of these checks fail.

**2. Flush-to-zero and denormals-are-zero.** In `round_upward_if_needed()`,
where `GAOL_RND_SSE_REGISTER` is defined, probe with a subnormal, which also
shows FTZ and DAZ:

```cpp
    static const volatile double subnormal = 8.0947715414629834e-320; // 2^-1060
    // subnormal + 0 is 0 only under FTZ or DAZ; 1 + 2^-1060 > 1 only upward
    if (1.0 + (subnormal + 0.0) == 1.0) {
      _mm_setcsr(_mm_getcsr() & ~0x8040u); // FTZ and DAZ off
      round_upward();
    }
```

Measured: x + y 4.53 ns against 4.68 ns, exp 28.7 against 29.4, sin 46.0
against 47.3 (i7-1185G7): no difference. Test (`tests/rounding_direction.cpp`,
SSE2 only): under FTZ, DAZ and both, `[1e-300]*[1e-20]`,
`[3e-308]-[2.9e-308]`, `sqr([1e-160])` and `[100·2^-1074]*[1e10]` are the
tightest enclosures.

**3. `pow` with a subnormal result.** In `gaol/core_math_port.h`, which every
source of CORE-MATH includes first, on x86-64, define `fegetround()` as a read
of MXCSR (after including `<fenv.h>`), as CORE-MATH's own `get_rounding_mode()`
of `rsqrt.c`, `asinpi.c` and `cbrt.c` already does; the vendored sources stay
unchanged. Test: add to the operations of `tests/rounding_direction.cpp`
`gaol_ieee1788::pow(interval(0x1.4a1249ba9b6d5p-1), interval(0x1.97f9e85175cf7p+10))`
and two more with subnormal results; the direction "to nearest, and upward for
SSE" fails 6 checks without the fix.

**4. `-ffinite-math-only`.** In `gaol_config.h`, after the `-ffast-math`
check:

```cpp
#if defined(__FINITE_MATH_ONLY__) && __FINITE_MATH_ONLY__
#  error "GAOL cannot be compiled with -ffinite-math-only: its empty interval has NaN bounds and its unbounded ones infinite bounds, which the compiler then takes never to occur"
#endif
```

and a row in the refused options of `doc/three-builds.md`. Test: a compile
test with `PASS_REGULAR_EXPRESSION "ffinite-math-only"`, as
`tests/fp_strict` does for other options.

**5. `atanh([1, x])`.** In `atanh()`, return the empty set when
`J.left() == 1.0 || J.right() == -1.0` (J being `I & [-1, 1]`), and write the
domain (−1, 1) in `doc/accuracy.md:109` and the manual. Test: `atanh([1])`,
`atanh([1,5])`, `atanh_rel([1], entire)` in the table of empty results of
`tests/elementary.cpp`.

**6. `interval(sl, sr)`.** At the end of the constructor, set the empty set
when `tmpl.is_empty() || tmpr.is_empty() || !(tmpl.left() <= tmpr.right())`.
Test (`tests/expressions.cpp`): `interval("2", "1")` stays empty after adding
[0, 1] and in `max()`.

**7. `pow` with large exponents.** In `ipow_exact_dn()`:
`nl = std::fma(-h,h,p) + nl*(2.0*h - nl);` (keeping the square of the rest).
Test (`tests/arithmetic.cpp`): `pow([1.0000001], n)` for n = 2²⁸ − 1, 2³¹ − 1
and 2³² − 1 against the tightest bounds from mpmath.

**8. `tan`.** `const bool narrower_than_pi = (w <= pi_dn);`: w, the width
rounded upward, at most the double below π proves the exact width below π.
Test (`tests/elementary.cpp`): `tan([-M_PI_2, M_PI_2])` is
[−0x1.d02967c31cdb5p+53, 0x1.d02967c31cdb5p+53].

**9. The reader under a comma locale.** Replace the one-double-at-a-time walk
of `gaol_enclose_number()` (in `gaol_interval_lexer.lpp` and in the committed
`gaol_interval_lexer.cpp`, which carry the same code) by steps doubling from
`strtod()`'s value, then a bisection on the bits of the doubles (the doubles of
[0, +∞] are ordered as their bits): two comparisons when `strtod()` is right,
at most about 125 whatever it gives. Write the hexadecimal bounds of
`exact_string()` from the bits of the doubles rather than with `printf("%a")`
(checked identical to glibc's `%a` on 200 000 random doubles under the C
locale). Test (`tests/numbers.cpp`): under a comma locale if one is installed
(`fr_FR.UTF-8`, `de_DE.UTF-8`, skipped otherwise), `exact_string([1.5, 2.5])`
is `[0x1.8p+0, 0x1.4p+1]`, `interval("0.1")` and `interval("1.5")` are right,
and a `TIMEOUT` on the test so that a hang fails it.

**10. Null pointers.** `parse_interval()` returns false for a null pointer,
the error messages write `(null pointer)` instead of the string, and
`interval(std::nullptr_t) = delete;` makes `x + nullptr` a compile error.
Test (`tests/numbers.cpp`): `interval(std::getenv(...))` of an unset variable
throws `input_format_error`, and
`static_assert(!std::is_convertible<std::nullptr_t, interval>::value)`.

**11. Expressions in threads.** Make the shared null node immortal:
`inc_refcount()` and `dec_refcount()` leave it alone, and the constructors,
destructor and compound operators of `expression` go through them (no cost on
other nodes; `std::atomic` counts would make building expressions 2.4 times
slower). Test (`tests/expressions.cpp`, with no thread): the null node's count
is the same before and while `expression e, f = e; expression g; g = f;` live.

**12. Point intervals written `<a, b>`.** In `display_bounds()`, write `<a, a>`
only when the two texts are equal (`l == r && left == right`), `[l, r]`
otherwise; correct the manual where it shows `<0.1, 0.1000000000000001>`.
Test (`tests/numbers.cpp`): for special values and every precision, the text
of a point interval reads back and encloses it; in `tests/ieee1788.cpp`,
`textToInterval(intervalToText(interval(0.1)))` contains 0.1.

**13. `operator<<`.** Format into a local `std::ostringstream` that copies the
stream's flags and locale and takes `interval::precision()`, then write the
whole text at once: the stream keeps its precision, and `setw` and `left`
apply to the whole interval. Test: after `os.precision(3); os << interval(1, 2)`,
`os.precision()` is 3, and `std::setw(8) << interval(1, 2)` gives `  [1, 2]`.

**14. `operator>>`.** `if (!std::getline(is, buffer)) return is;` (failbit
set, the interval unchanged, as for a double at the end of the input), and set
`failbit` before throwing on a syntax error. Test: `while (in >> x)` over
`"[1, 2]\n[3, 4]\n"` reads 2 intervals and throws nothing.

**15. `what()`.** In `gaol_exception`:
`const char* what() const noexcept override { return explanation_.empty() ? "gaol_exception" : explanation_.c_str(); }`,
and in `operator<<` of the exceptions no longer print `what()` next to the
explanation. Test (`tests/expressions.cpp`): the `what()` of a refused string
is its explanation.

**16. `gaol::sin(0.5)`.** Stop including `gaol_expression.h` from
`gaol_ieee1788.h`, and move `gaol_ieee1788`'s two expression overloads
(`pown(e, n)`, `pow(e1, e2)`) to the end of `gaol_expression.h`; a program
using expressions includes `gaol_expression.h`, as with GAOL 4. Test
(`tests/other_functions.cpp`):
`static_assert(std::is_same<decltype(gaol::sin(0.5)), interval>::value, "")`.

**17. Long sums.** In `gaol_interval_parser.ypp`, fold each binary operation
when it is read (`$$ = gaol_leaf(gaol_value($1) + gaol_value($3))`), as calls
of functions already are, and regenerate the committed parser. Test
(`tests/expressions.cpp`): a sum and a product of 200 000 terms.

**18. `expression::operator/=`.** Define it as `operator*=` is, with a
`div_node`. Test: the in-place operators of `tests/expressions.cpp` with
`/=`.

**19. `hausdorff()`.** In both builds, `max(|a − c|, |b − d|)` rounded upward,
equal bounds (the infinite ones included) being at distance 0:
`fmax((a == c) ? 0.0 : fmax(a - c, c - a), (b == e) ? 0.0 : fmax(b - e, e - b))`.
Test (`tests/other_functions.cpp`): `hausdorff([1,+∞], [1,+∞]) == 0`,
`hausdorff([1,+∞], [2,+∞]) == 1`, `hausdorff([1,+∞], [1,2]) == +∞`.

**20. `nb_fp_numbers()`.** Number the doubles by the bits of |a| and |b|:
`ai.d = std::fabs(a); bi.d = std::fabs(b);` and `bi.i + ai.i + 1` when a < 0 < b.
Test: `nb_fp_numbers(-0.0, 1.0)` and `nb_fp_numbers(-1.0, 0.0)` are
4607182418800017409.

**21 and 22** (output formats, installed headers) are small changes of the
code or of the build files described in section 5.2; for the width and center
formats, documenting them as display formats is the smallest correct change.
