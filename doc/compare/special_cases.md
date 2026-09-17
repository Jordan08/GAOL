# Special cases: GAOL, libieeep1788, filib++ and Solaris Studio

Part of the [comparison](README.md) of GAOL with libieeep1788, filib++ and
Solaris Studio.

The special cases of GAOL's tests (`tests/*.cpp`, and `check/*.cpp` for the
integer powers) are computed by the four libraries: zeros, infinities and NaN
as bounds or operands, empty sets, divisions by intervals containing zero,
arguments at the edges of the domains, the powers `pow` and `pown`, the
operators `+=`, `-=`, `*=`, `/=` with special doubles, the reading of
intervals from text, the numeric functions of intervals, the comparisons and
`atan2`. Each result is
compared with the result IEEE 1788-2015 defines for the set-based flavor, the
tightest interval of doubles, computed with mpmath:

- ✓ the result of IEEE 1788;
- ⊃ an interval enclosing it, wider;
- ✗ another result: a set not enclosing it, a set where IEEE 1788 has the empty
  set, another boolean, or an exception;
- n/a: the library has no such operation;
- no mark: IEEE 1788 defines no result (five cases).

Signed zeros are not told apart. The bounds are written with the shortest
decimal that reads back as the same double, MAX being the largest double and
pred(MAX) the one below it.

| Library | Version | Intervals |
|---|---|---|
| GAOL | this fork | `gaol::interval`, CMake Release build, SSE2 intervals, mathlib 2.1.1 |
| libieeep1788 | last commit, 30 March 2015 | `interval<double, mpfr_bin_ieee754_flavor>`: the set-based flavor of the preliminary IEEE P1788, on doubles, computed with MPFR |
| filib++ | 3.0.2.2, as IBEX distributes it | `interval<double, native_switched, i_mode_extended_flag>`, the intervals of IBEX built with filib++: the extended mode, where the infinities and the empty set are intervals |
| Solaris Studio | Sun Fortran 95 8.7 (Solaris Studio 12.4), Linux x86-64 | `interval(8)` of `f90 -xia`, computed by `libsunimath` |

Each case is written once, in [code/cases.py](code/cases.py), and translated
for each library: libieeep1788 has no operators of an interval and a double
nor compound assignments, which are computed with `II(d, d)` there; Solaris
Studio's are written `x = x + d`; filib++ reads intervals from text with its
`operator>>`. [code/README.md](code/README.md) says how to run the comparison
again.

## What the cases show

**libieeep1788** gives the result of IEEE 1788 in each of the 279 cases
where IEEE 1788 defines one and libieeep1788 has the operation: it computes
every bound with MPFR, correctly rounded. It has no n-th root (`rootn`), no
operators with doubles and no compound assignments.

**GAOL** gives the result of IEEE 1788, or an interval enclosing it, in 280
cases out of 286, and something else in 6, all from the hybrid `pow(x, y)` of
this fork, which takes the integer power `pown` for a degenerate integer
exponent, where IEEE 1788's `pow` only takes the part of x in [0, +∞] (see
[What differs from GAOL](../differences.md)): `pow([−2], 2.0)` is [4],
`pow([0], [0])` is [1], `pow([−1], [2^31−1])` is [−1] and
`pow([−2, 0], [−1])` is [−∞, −0.5], where IEEE 1788 has ∅ (cases 158, 178 to
180, 183); `pow([−2, −1], [1e10])`, an integer beyond the ints, is [−∞, +∞]
(173). Five more cases enclose IEEE 1788's result for the same reason (157,
161, 174 to 176). GAOL reads a bare number, `interval("0.1")`, as the interval
enclosing it, an extension of the literals IEEE 1788 allows (19).

Its wider results are a few doubles off: sin, cos, tan and `pow(x, y)` by one
double, as `pow([−15], 17)`, whose products are rounded one by one, and the
odd n-th roots, computed as powers with a rounded
exponent (`nth_root([−8, 27], 3)` is [−2.0000000000000004, 3.000000000000001])
([accuracy](../accuracy.md) gives the tightness of each operation). acos,
acosh and the negative integer powers are the tightest: `acos([1, 3])` and
`acosh([0, 1])` are [0], and `pow([10], −400)` is [0, 2^-1074]. `atan2` is
within one double of the tightest bounds, and the tightest where a bound is 0,
±π/4, ±π/2 or ±π: across the half-line y = 0, x < 0, where the angle jumps
from π to −π, it is [−π, π], as in libieeep1788 (263 to 291).

**filib++** gives the result of IEEE 1788, or an interval enclosing it, in
176 cases out of 243, and something else in 67. Its extended mode takes the
infinities much as Solaris Studio does:

- The intervals built from an infinity are not empty: `interval(+∞)`,
  `x = +∞` and `x + ∞` give [MAX, +∞] (1 to 4, 11, 12, 61 to 64, 79, 80),
  whereas `interval(2, 1)` and NaN bounds give ∅, as in IEEE 1788 (5 to 10).
- A product of a zero and an infinite bound is [−∞, +∞] (50 to 53, 57, 60,
  65, 66, 74, 75, 81, 82), and so are the quotients of an interval containing
  0 by an interval containing 0 (36 to 38, 40) and the divisions by the double
  0 (68 to 70, 83), whereas `[1, 2] / [0]` and `1 / [0]` are [MAX, +∞] (39,
  42).
- `log([−4, 0])` and `log([0])` are [−∞, −MAX] (97, 98). `pow(x, y)`, which is
  exp(y log x), follows: `pow([0], [0])` is [0, +∞], `pow([0], [−1])` is
  [MAX, +∞], `pow([0], [0.5])` is [0, 2.2e−308], and `pow([4], [+∞])` is
  [MAX, +∞] (166, 167, 169, 170, 179 to 193). The integer power
  `power([0], −1)` is [MAX, +∞] (138), and `power([−3, 2], −2)` is [0, +∞]
  rather than [1/9, +∞] (134, 141).
- `mid([−∞, 1])` is −∞ and `mid([1, +∞])` is +∞, as in Solaris Studio (207,
  208), and `imax([1, 2], ∅)` is [1, 2] rather than ∅ (225).
- Its `operator>>` reads only the form `[l, u]`, with finite or infinite
  bounds, and rounds the bounds to nearest: `[0.1, 0.3]` gives the doubles
  nearest 0.1 and 0.3, which do not enclose [0.1, 0.3] (21); `[0.1]`,
  `[1e309]`, `[ ]`, `[empty]`, `[entire]`, the hexadecimal numbers, the
  uncertain form and the others throw `interval_io_exception` (20, 22, 24,
  25, 27 to 29, 233, 234, 237 to 242), `[,]` is [0] and `[1,]` ∅ (235, 236),
  and `[inf, inf]` is [MAX, +∞] (243). It reads `[ EMPTY ]`. Its constructor
  from two strings, `interval("0.1", "0.1")`, rounds them outward, one double
  wider than the tightest.
- Its certainly-less comparisons `cle` and `clt` are false when an interval is
  empty, where `precedes` and `strictPrecedes` are true (245, 246, 248);
  `interior` is false for the same infinite bound (251, 252); `ceq` is false
  for two empty intervals (262).
- It has no n-th root, no relational division and no `atan2`.

Its elementary functions are wider than the tightest by up to 25 doubles (6 to
11 for sin and cos, 15 to 19 for asin, acos and atan, 25 for tan), its real
powers by up to 36, `cos([2^52 − 1])` is [−1, 1], `exp([−800])` is
[0, 2.2e−308] and `sqrt([−4, 4])` has −2^-1074 for lower bound.

**Solaris Studio** gives the result of IEEE 1788, or an interval enclosing it,
in 191 cases out of 261, and something else in 70. It follows the containment
sets of Sun's interval arithmetic (G. W. Walster), whose values include the
infinities: 1/0 is {−∞, +∞}, 0 × ∞ is every extended real, and +∞ is a point.
So:

- The intervals built from an infinity are not empty: `interval(+∞)` is
  [MAX, +∞], `x + ∞` and `x = +∞` give [MAX, +∞], and `[1, 2] / ∞` gives
  [0, 1.1e−308] (1 to 4, 11, 12, 61 to 64, 79, 80); `[inf]` and `[inf, inf]`
  are read as [MAX, +∞] (242, 243).
- The arguments that are not an interval give [−∞, +∞] rather than ∅:
  `interval(2, 1)`, NaN bounds, `x + NaN` (5 to 10, 67).
- A division by an interval containing zero is [−∞, +∞], a division by [0]
  too (30 to 43, 68 to 70, 83), and so is a product of a zero and an infinite
  bound (50 to 53, 57, 60, 65, 66, 74, 75, 81, 82).
- `pow` includes the limits: `pow([0], [0])` is [0, +∞], `pow([0], [−1])` is
  [+∞] and `pow([4], [+∞])` is [MAX, +∞] (166 to 170, 179 to 181, 183, 187 to
  193), and `x**(−n)` is [−∞, +∞] when x contains 0 (138 to 140). The powers
  are often one double wider than the tightest, `x**n` included (132, 134,
  136, 137, 141, 146, 150, 152, 157, 159, 161, 163).
- `mid([−∞, 1])` is −∞ and `mid([1, +∞])` is +∞, rather than −MAX and MAX
  (207, 208).
- Reading `0.1` gives [0, 0.2]: a single number read as an interval carries an
  uncertainty of one unit of its last digit, as the uncertain form `0.1?1` of
  IEEE 1788 would. Bounds in the wrong order are a read error rather than ∅,
  and neither a rational bound (`1/3`), `[entire]`, `[ ]`, bounds left out,
  hexadecimal numbers nor the uncertain form are read (19, 22, 23, 28, 29,
  233, 235 to 241).
- Its certainly-less operators `.cle.` and `.clt.` are false when an interval
  is empty (245, 246, 248), `.int.` is false for the same infinite bound (251,
  252), and `.ceq.` false for two empty intervals (262).
- `atan2` is continued across the half-line y = 0, x < 0 rather than given
  the hull of its values: `atan2([−1, 1], [−2, −1])` is [3π/4, 5π/4], beyond
  π, `atan2([−1, 0], [−2, −1])` is [−π, −3π/4], without the angle π of its
  points on the half-line, and `atan2([0], [0])` is [−π, π] (263, 282, 283).
  Where a box touches an axis at (0, 0), it gives [−π, π] (275, 276, 279,
  280).
- It has no `asinh`, `acosh`, `atanh`, n-th root, relational division
  (`mul_rev`), radius (`rad`), nor `sqr`, computed as `x**2`.

Its elementary functions are the tightest intervals, or one double wider, and
give IEEE 1788's results at the edges of their domains, `log([−4, 0])` and
`log([0])` aside, which are [−∞, −MAX] (97, 98).

## The cases

<!-- BEGIN GENERATED TABLES (doc/compare/code/cases.py) -->

291 cases. Each result is marked against the result IEEE 1788-2015 defines (the tightest interval of doubles, computed with mpmath):

| | GAOL | libieeep1788 | filib++ | Solaris Studio |
|---|---|---|---|---|
| ✓ the result of IEEE 1788 | 255 | 279 | 126 | 150 |
| ⊃ encloses it, wider | 25 | 0 | 50 | 41 |
| ✗ differs | 6 | 0 | 67 | 70 |
| n/a no such operation | 0 | 11 | 44 | 26 |

### 1. Constructors and assignment

From tests/numbers.cpp (constructors).

| # | Operation | IEEE 1788 | GAOL | libieeep1788 | filib++ | Solaris Studio | Notes |
|---|---|---|---|---|---|---|---|
| 001 | `interval(+∞)` | ∅ | ∅ ✓ | ∅ ✓ | [MAX, +∞] ✗ | [MAX, +∞] ✗ | numsToInterval(+∞, +∞) fails (12.12.7) |
| 002 | `interval(−∞)` | ∅ | ∅ ✓ | ∅ ✓ | [−∞, −MAX] ✗ | [−∞, −MAX] ✗ |  |
| 003 | `interval(+∞, +∞)` | ∅ | ∅ ✓ | ∅ ✓ | [MAX, +∞] ✗ | [MAX, +∞] ✗ |  |
| 004 | `interval(−∞, −∞)` | ∅ | ∅ ✓ | ∅ ✓ | [−∞, −MAX] ✗ | [−∞, −MAX] ✗ |  |
| 005 | `interval(+∞, 1)` | ∅ | ∅ ✓ | ∅ ✓ | ∅ ✓ | [−∞, +∞] ✗ |  |
| 006 | `interval(1, −∞)` | ∅ | ∅ ✓ | ∅ ✓ | ∅ ✓ | [−∞, +∞] ✗ |  |
| 007 | `interval(2, 1)` | ∅ | ∅ ✓ | ∅ ✓ | ∅ ✓ | [−∞, +∞] ✗ | l > u: the constructor fails |
| 008 | `interval(NaN)` | ∅ | ∅ ✓ | ∅ ✓ | ∅ ✓ | [−∞, +∞] ✗ |  |
| 009 | `interval(NaN, 1)` | ∅ | ∅ ✓ | ∅ ✓ | ∅ ✓ | [−∞, +∞] ✗ |  |
| 010 | `interval(1, NaN)` | ∅ | ∅ ✓ | ∅ ✓ | ∅ ✓ | [−∞, +∞] ✗ |  |
| 011 | `x = [1, 2]; x = +∞` | ∅ | ∅ ✓ | ∅ ✓ | [MAX, +∞] ✗ | [MAX, +∞] ✗ | libieeep1788 has no assignment of a double: II(d, d) |
| 012 | `x = [1, 2]; x = −∞` | ∅ | ∅ ✓ | ∅ ✓ | [−∞, −MAX] ✗ | [−∞, −MAX] ✗ |  |
| 013 | `[−∞, 1]` | [−∞, 1] | [−∞, 1] ✓ | [−∞, 1] ✓ | [−∞, 1] ✓ | [−∞, 1] ✓ |  |
| 014 | `[1, +∞]` | [1, +∞] | [1, +∞] ✓ | [1, +∞] ✓ | [1, +∞] ✓ | [1, +∞] ✓ |  |
| 015 | `[−∞, +∞]` | [−∞, +∞] | [−∞, +∞] ✓ | [−∞, +∞] ✓ | [−∞, +∞] ✓ | [−∞, +∞] ✓ |  |
| 016 | `[−0]` | [0] | [−0] ✓ | [−0] ✓ | [−0] ✓ | [−0] ✓ |  |
| 017 | `[−MAX, MAX]` | [−MAX, MAX] | [−MAX, MAX] ✓ | [−MAX, MAX] ✓ | [−MAX, MAX] ✓ | [−MAX, MAX] ✓ |  |
| 018 | `empty set` | ∅ | ∅ ✓ | ∅ ✓ | ∅ ✓ | ∅ ✓ | Solaris Studio has no empty constant: [1, 2] .ix. [3, 4] |

### 2. Reading intervals from text

From tests/numbers.cpp (numbers).

| # | Operation | IEEE 1788 | GAOL | libieeep1788 | filib++ | Solaris Studio | Notes |
|---|---|---|---|---|---|---|---|
| 019 | `interval("0.1")` | — | [0.09999999999999999, 0.1] | ∅ | exception interval_io_exception | [0, 0.2] | not an interval literal (9.7.4): ∅, unless the implementation extends the literals (9.7.1), as GAOL does; Solaris Studio reads 0.1 ± 0.1 |
| 020 | `interval("[0.1]")` | [0.09999999999999999, 0.1] | [0.09999999999999999, 0.1] ✓ | [0.09999999999999999, 0.1] ✓ | exception interval_io_exception ✗ | [0.09999999999999999, 0.1] ✓ | filib++: operator>>, which reads [l, u] only, and rounds the bounds to nearest |
| 021 | `interval("[0.1, 0.3]")` | [0.09999999999999999, 0.30000000000000004] | [0.09999999999999999, 0.30000000000000004] ✓ | [0.09999999999999999, 0.30000000000000004] ✓ | [0.1, 0.3] ✗ | [0.09999999999999999, 0.30000000000000004] ✓ |  |
| 022 | `interval("[1/3, 0.3]")` | ∅ | ∅ ✓ | ∅ ✓ | exception interval_io_exception ✗ | read error, iostat 1210 ✗ | rational literal, l > u |
| 023 | `interval("[0.3, 0.1]")` | ∅ | ∅ ✓ | ∅ ✓ | ∅ ✓ | read error, iostat 1211 ✗ | l > u |
| 024 | `interval("[1e309]")` | [MAX, +∞] | [MAX, +∞] ✓ | [MAX, +∞] ✓ | exception interval_io_exception ✗ | [MAX, +∞] ✓ | the real number 1e309, beyond the doubles |
| 025 | `interval("[1e-400]")` | [0, 2^-1074] | [0, 2^-1074] ✓ | [−0, 2^-1074] ✓ | exception interval_io_exception ✗ | [0, 2^-1074] ✓ |  |
| 026 | `interval("[1, inf]")` | [1, +∞] | [1, +∞] ✓ | [1, +∞] ✓ | [1, +∞] ✓ | [1, +∞] ✓ | a literal of the set-based flavor (10.5.1) |
| 027 | `interval("[empty]")` | ∅ | ∅ ✓ | ∅ ✓ | exception interval_io_exception ✗ | ∅ ✓ | a literal of the set-based flavor (10.5.1); filib++ reads [ EMPTY ] |
| 028 | `interval("[entire]")` | [−∞, +∞] | [−∞, +∞] ✓ | [−∞, +∞] ✓ | exception interval_io_exception ✗ | read error, iostat 1210 ✗ |  |
| 029 | `interval("3.56?1")` | [3.55, 3.5700000000000003] | [3.55, 3.5700000000000003] ✓ | [3.55, 3.5700000000000003] ✓ | exception interval_io_exception ✗ | read error, iostat 1210 ✗ | uncertain form (9.7.4) |

### 3. Division by an interval containing zero

From tests/arithmetic.cpp (divisions_by_zero).

| # | Operation | IEEE 1788 | GAOL | libieeep1788 | filib++ | Solaris Studio | Notes |
|---|---|---|---|---|---|---|---|
| 030 | `[1, 2] / [0, 1]` | [1, +∞] | [1, +∞] ✓ | [1, +∞] ✓ | [1, +∞] ✓ | [−∞, +∞] ⊃ |  |
| 031 | `[1, 2] / [−1, 0]` | [−∞, −1] | [−∞, −1] ✓ | [−∞, −1] ✓ | [−∞, −1] ✓ | [−∞, +∞] ⊃ |  |
| 032 | `[−2, −1] / [0, 1]` | [−∞, −1] | [−∞, −1] ✓ | [−∞, −1] ✓ | [−∞, −1] ✓ | [−∞, +∞] ⊃ |  |
| 033 | `[−2, −1] / [−1, 0]` | [1, +∞] | [1, +∞] ✓ | [1, +∞] ✓ | [1, +∞] ✓ | [−∞, +∞] ⊃ |  |
| 034 | `[1, 2] / [−1, 1]` | [−∞, +∞] | [−∞, +∞] ✓ | [−∞, +∞] ✓ | [−∞, +∞] ✓ | [−∞, +∞] ✓ |  |
| 035 | `[−1, 2] / [0, 1]` | [−∞, +∞] | [−∞, +∞] ✓ | [−∞, +∞] ✓ | [−∞, +∞] ✓ | [−∞, +∞] ✓ |  |
| 036 | `[0, 2] / [0, 1]` | [0, +∞] | [0, +∞] ✓ | [−0, +∞] ✓ | [−∞, +∞] ⊃ | [−∞, +∞] ⊃ |  |
| 037 | `[−2, 0] / [0, 1]` | [−∞, 0] | [−∞, 0] ✓ | [−∞, 0] ✓ | [−∞, +∞] ⊃ | [−∞, +∞] ⊃ |  |
| 038 | `[0] / [−1, 1]` | [0] | [−0] ✓ | [−0] ✓ | [−∞, +∞] ⊃ | [−∞, +∞] ⊃ |  |
| 039 | `[1, 2] / [0]` | ∅ | ∅ ✓ | ∅ ✓ | [MAX, +∞] ✗ | [−∞, +∞] ✗ |  |
| 040 | `[0] / [0]` | ∅ | ∅ ✓ | ∅ ✓ | [−∞, +∞] ✗ | [−∞, +∞] ✗ |  |
| 041 | `inverse([0, 1])` | [1, +∞] | [1, +∞] ✓ | [1, +∞] ✓ | [1, +∞] ✓ | [−∞, +∞] ⊃ | recip in IEEE 1788, 1/x in filib++ and Solaris Studio |
| 042 | `inverse([0])` | ∅ | ∅ ✓ | ∅ ✓ | [MAX, +∞] ✗ | [−∞, +∞] ✗ |  |
| 043 | `inverse([−1, 1])` | [−∞, +∞] | [−∞, +∞] ✓ | [−∞, +∞] ✓ | [−∞, +∞] ✓ | [−∞, +∞] ✓ |  |

### 4. Relational division (GAOL's %, mulRev of IEEE 1788)

From tests/arithmetic.cpp (divisions_by_zero).

| # | Operation | IEEE 1788 | GAOL | libieeep1788 | filib++ | Solaris Studio | Notes |
|---|---|---|---|---|---|---|---|
| 044 | `[1, 2] % [0, 1]` | [1, +∞] | [1, +∞] ✓ | [1, +∞] ✓ | n/a | n/a | mul_rev(y, x) in libieeep1788 |
| 045 | `[1, 2] % [−1, 0]` | [−∞, −1] | [−∞, −1] ✓ | [−∞, −1] ✓ | n/a | n/a |  |
| 046 | `[1, 2] % [−1, 1]` | [−∞, +∞] | [−∞, +∞] ✓ | [−∞, +∞] ✓ | n/a | n/a |  |
| 047 | `[0] % [−1, 1]` | [−∞, +∞] | [−∞, +∞] ✓ | [−∞, +∞] ✓ | n/a | n/a |  |
| 048 | `[−1, 2] % [0]` | [−∞, +∞] | [−∞, +∞] ✓ | [−∞, +∞] ✓ | n/a | n/a |  |
| 049 | `[1, 2] % [0]` | ∅ | ∅ ✓ | ∅ ✓ | n/a | n/a |  |

### 5. Products with zero and infinite bounds

From tests/arithmetic.cpp (products_with_infinite_bounds).

| # | Operation | IEEE 1788 | GAOL | libieeep1788 | filib++ | Solaris Studio | Notes |
|---|---|---|---|---|---|---|---|
| 050 | `[0, 1] * [1, +∞]` | [0, +∞] | [0, +∞] ✓ | [−0, +∞] ✓ | [−∞, +∞] ⊃ | [−∞, +∞] ⊃ |  |
| 051 | `[−1, 0] * [1, +∞]` | [−∞, 0] | [−∞, 0] ✓ | [−∞, 0] ✓ | [−∞, +∞] ⊃ | [−∞, +∞] ⊃ |  |
| 052 | `[0] * [1, +∞]` | [0] | [0] ✓ | [−0] ✓ | [−∞, +∞] ⊃ | [−∞, +∞] ⊃ |  |
| 053 | `[0] * [−∞, +∞]` | [0] | [−0] ✓ | [−0] ✓ | [−∞, +∞] ⊃ | [−∞, +∞] ⊃ |  |
| 054 | `[0, 1] * [−∞, +∞]` | [−∞, +∞] | [−∞, +∞] ✓ | [−∞, +∞] ✓ | [−∞, +∞] ✓ | [−∞, +∞] ✓ |  |
| 055 | `[1, +∞] * [1, +∞]` | [1, +∞] | [1, +∞] ✓ | [1, +∞] ✓ | [1, +∞] ✓ | [1, +∞] ✓ |  |
| 056 | `[−∞, −1] * [1, +∞]` | [−∞, −1] | [−∞, −1] ✓ | [−∞, −1] ✓ | [−∞, −1] ✓ | [−∞, −1] ✓ |  |
| 057 | `[−∞, 0] * [−∞, 0]` | [0, +∞] | [−0, +∞] ✓ | [−0, +∞] ✓ | [−∞, +∞] ⊃ | [−∞, +∞] ⊃ |  |
| 058 | `[−∞, +∞] * [−∞, +∞]` | [−∞, +∞] | [−∞, +∞] ✓ | [−∞, +∞] ✓ | [−∞, +∞] ✓ | [−∞, +∞] ✓ |  |
| 059 | `[1, 2] * ∅` | ∅ | ∅ ✓ | ∅ ✓ | ∅ ✓ | ∅ ✓ |  |
| 060 | `[0, 1] * interval(+∞)` | ∅ | ∅ ✓ | ∅ ✓ | [−∞, +∞] ✗ | [−∞, +∞] ✗ | interval(+∞) is empty |

### 6. Operators of an interval and a double (+=, -=, *=, /=)

From tests/arithmetic.cpp (operations_with_special_doubles).

| # | Operation | IEEE 1788 | GAOL | libieeep1788 | filib++ | Solaris Studio | Notes |
|---|---|---|---|---|---|---|---|
| 061 | `x = [1, 2]; x += +∞` | ∅ | ∅ ✓ | ∅ ✓ | [MAX, +∞] ✗ | [MAX, +∞] ✗ | libieeep1788 has neither mixed operators nor op=: x op II(d, d); Solaris Studio: x = x op d |
| 062 | `x = [1, 2]; x -= +∞` | ∅ | ∅ ✓ | ∅ ✓ | [−∞, −pred(MAX)] ✗ | [−∞, −pred(MAX)] ✗ |  |
| 063 | `x = [1, 2]; x *= +∞` | ∅ | ∅ ✓ | ∅ ✓ | [MAX, +∞] ✗ | [MAX, +∞] ✗ |  |
| 064 | `x = [1, 2]; x /= +∞` | ∅ | ∅ ✓ | ∅ ✓ | [0, 1.112536929253601e−308] ✗ | [0, 1.112536929253601e−308] ✗ |  |
| 065 | `x = [0]; x *= +∞` | ∅ | ∅ ✓ | ∅ ✓ | [−∞, +∞] ✗ | [−∞, +∞] ✗ |  |
| 066 | `x = [0, 1]; x *= −∞` | ∅ | ∅ ✓ | ∅ ✓ | [−∞, +∞] ✗ | [−∞, +∞] ✗ |  |
| 067 | `x = [1, 2]; x += NaN` | ∅ | ∅ ✓ | ∅ ✓ | ∅ ✓ | [−∞, +∞] ✗ |  |
| 068 | `x = [1, 2]; x /= 0` | ∅ | ∅ ✓ | ∅ ✓ | [−∞, +∞] ✗ | [−∞, +∞] ✗ |  |
| 069 | `x = [1, 2]; x /= −0` | ∅ | ∅ ✓ | ∅ ✓ | [−∞, +∞] ✗ | [−∞, +∞] ✗ |  |
| 070 | `x = [−1, 2]; x /= 0` | ∅ | ∅ ✓ | ∅ ✓ | [−∞, +∞] ✗ | [−∞, +∞] ✗ |  |
| 071 | `x = [1, 2]; x %= 0` | ∅ | ∅ ✓ | ∅ ✓ | n/a | n/a | GAOL's %=, mul_rev in libieeep1788 |
| 072 | `x = [1, 2]; x += −0` | [1, 2] | [1, 2] ✓ | [1, 2] ✓ | [1, 2] ✓ | [1, 2] ✓ |  |
| 073 | `x = [1, 2]; x *= −0` | [0] | [−0] ✓ | [−0] ✓ | [−0] ✓ | [−0] ✓ |  |
| 074 | `x = [1, +∞]; x *= 0` | [0] | [−0] ✓ | [−0] ✓ | [−∞, +∞] ⊃ | [−∞, +∞] ⊃ |  |
| 075 | `x = [−∞, +∞]; x *= 0` | [0] | [−0] ✓ | [−0] ✓ | [−∞, +∞] ⊃ | [−∞, +∞] ⊃ |  |
| 076 | `x = [1, +∞]; x -= MAX` | [−MAX, +∞] | [−MAX, +∞] ✓ | [−MAX, +∞] ✓ | [−MAX, +∞] ✓ | [−MAX, +∞] ✓ |  |
| 077 | `x = [MAX]; x += MAX` | [MAX, +∞] | [MAX, +∞] ✓ | [MAX, +∞] ✓ | [MAX, +∞] ✓ | [MAX, +∞] ✓ |  |
| 078 | `x = ∅; x += 1` | ∅ | ∅ ✓ | ∅ ✓ | ∅ ✓ | ∅ ✓ |  |
| 079 | `x = [1, 2]; x += interval(+∞)` | ∅ | ∅ ✓ | ∅ ✓ | [MAX, +∞] ✗ | [MAX, +∞] ✗ |  |
| 080 | `[1, 2] + +∞` | ∅ | ∅ ✓ | ∅ ✓ | [MAX, +∞] ✗ | [MAX, +∞] ✗ |  |
| 081 | `[0, 1] * +∞` | ∅ | ∅ ✓ | ∅ ✓ | [−∞, +∞] ✗ | [−∞, +∞] ✗ |  |
| 082 | `0 * [1, +∞]` | [0] | [0] ✓ | [−0] ✓ | [−∞, +∞] ⊃ | [−∞, +∞] ⊃ |  |
| 083 | `[1, 2] / 0` | ∅ | ∅ ✓ | ∅ ✓ | [−∞, +∞] ✗ | [−∞, +∞] ✗ |  |

### 7. Elementary functions at the edges of their domains

From tests/elementary.cpp (at_known_intervals), check/.

| # | Operation | IEEE 1788 | GAOL | libieeep1788 | filib++ | Solaris Studio | Notes |
|---|---|---|---|---|---|---|---|
| 084 | `sin([1, 2])` | [0.8414709848078965, 1] | [0.8414709848078964, 1] ⊃ | [0.8414709848078965, 1] ✓ | [0.8414709848078953, 1] ⊃ | [0.8414709848078964, 1] ⊃ |  |
| 085 | `sin([4, 5])` | [−1, −0.7568024953079282] | [−1, −0.7568024953079281] ⊃ | [−1, −0.7568024953079282] ✓ | [−1, −0.7568024953079271] ⊃ | [−1, −0.7568024953079282] ✓ |  |
| 086 | `sin([0, 7])` | [−1, 1] | [−1, 1] ✓ | [−1, 1] ✓ | [−1, 1] ✓ | [−1, 1] ✓ |  |
| 087 | `sin([−∞, +∞])` | [−1, 1] | [−1, 1] ✓ | [−1, 1] ✓ | [−1, 1] ✓ | [−1, 1] ✓ |  |
| 088 | `cos([3, 4])` | [−1, −0.6536436208636118] | [−1, −0.6536436208636118] ✓ | [−1, −0.6536436208636118] ✓ | [−1, −0.653643620863611] ⊃ | [−1, −0.6536436208636118] ✓ |  |
| 089 | `cos([−1, 1])` | [0.5403023058681397, 1] | [0.5403023058681397, 1] ✓ | [0.5403023058681397, 1] ✓ | [0.540302305868139, 1] ⊃ | [0.5403023058681397, 1] ✓ |  |
| 090 | `cos([0, 7])` | [−1, 1] | [−1, 1] ✓ | [−1, 1] ✓ | [−1, 1] ✓ | [−1, 1] ✓ |  |
| 091 | `cos([2^52−1])` | [0.473292885954309, 0.4732928859543091] | [0.473292885954309, 0.47329288595430913] ⊃ | [0.473292885954309, 0.4732928859543091] ✓ | [−1, 1] ⊃ | [0.473292885954309, 0.4732928859543091] ✓ | a wrong mathlib gives −0.4855 (see manual) |
| 092 | `tan([1, 2])` | [−∞, +∞] | [−∞, +∞] ✓ | [−∞, +∞] ✓ | [−∞, +∞] ✓ | [−∞, +∞] ✓ |  |
| 093 | `tan([−1, 1])` | [−1.5574077246549023, 1.5574077246549023] | [−1.5574077246549025, 1.5574077246549025] ⊃ | [−1.5574077246549023, 1.5574077246549023] ✓ | [−1.5574077246549078, 1.5574077246549078] ⊃ | [−1.5574077246549023, 1.5574077246549023] ✓ |  |
| 094 | `tan([−∞, +∞])` | [−∞, +∞] | [−∞, +∞] ✓ | [−∞, +∞] ✓ | [−∞, +∞] ✓ | [−∞, +∞] ✓ |  |
| 095 | `log([−1, 1])` | [−∞, 0] | [−∞, 0] ✓ | [−∞, 0] ✓ | [−∞, 0] ✓ | [−∞, 0] ✓ |  |
| 096 | `log([0, 1])` | [−∞, 0] | [−∞, 0] ✓ | [−∞, 0] ✓ | [−∞, 0] ✓ | [−∞, 0] ✓ |  |
| 097 | `log([−4, 0])` | ∅ | ∅ ✓ | ∅ ✓ | [−∞, −MAX] ✗ | [−∞, −MAX] ✗ | no x > 0 in [−4, 0]: Solaris Studio takes log(0) = −∞ |
| 098 | `log([0])` | ∅ | ∅ ✓ | ∅ ✓ | [−∞, −MAX] ✗ | [−∞, −MAX] ✗ |  |
| 099 | `log([−2, −1])` | ∅ | ∅ ✓ | ∅ ✓ | ∅ ✓ | ∅ ✓ |  |
| 100 | `log([1, +∞])` | [0, +∞] | [0, +∞] ✓ | [−0, +∞] ✓ | [0, +∞] ✓ | [−0, +∞] ✓ |  |
| 101 | `exp([−∞, 0])` | [0, 1] | [0, 1] ✓ | [−0, 1] ✓ | [0, 1] ✓ | [0, 1] ✓ |  |
| 102 | `exp([740])` | [MAX, +∞] | [MAX, +∞] ✓ | [MAX, +∞] ✓ | [MAX, +∞] ✓ | [MAX, +∞] ✓ |  |
| 103 | `exp([−800])` | [0, 2^-1074] | [0, 2^-1074] ✓ | [−0, 2^-1074] ✓ | [0, 2.2250738585072014e−308] ⊃ | [0, 2^-1074] ✓ |  |
| 104 | `exp([−∞, +∞])` | [0, +∞] | [0, +∞] ✓ | [−0, +∞] ✓ | [0, +∞] ✓ | [0, +∞] ✓ |  |
| 105 | `exp(∅)` | ∅ | ∅ ✓ | ∅ ✓ | ∅ ✓ | ∅ ✓ |  |
| 106 | `sqrt([−4, 4])` | [0, 2] | [0, 2] ✓ | [−0, 2] ✓ | [−2^-1074, 2.0000000000000004] ⊃ | [−0, 2] ✓ |  |
| 107 | `sqrt([−4, −1])` | ∅ | ∅ ✓ | ∅ ✓ | ∅ ✓ | ∅ ✓ |  |
| 108 | `sqrt([−∞, +∞])` | [0, +∞] | [0, +∞] ✓ | [−0, +∞] ✓ | [−2^-1074, +∞] ⊃ | [−0, +∞] ✓ |  |
| 109 | `sqr([−∞, +∞])` | [0, +∞] | [−0, +∞] ✓ | [−0, +∞] ✓ | [0, +∞] ✓ | [−0, +∞] ✓ | x**2 in Solaris Studio |
| 110 | `sqr([−∞, −MAX])` | [MAX, +∞] | [MAX, +∞] ✓ | [MAX, +∞] ✓ | [MAX, +∞] ✓ | [MAX, +∞] ✓ |  |
| 111 | `asin([−2, 2])` | [−1.5707963267948968, 1.5707963267948968] | [−1.5707963267948968, 1.5707963267948968] ✓ | [−1.5707963267948968, 1.5707963267948968] ✓ | [−1.570796326794901, 1.570796326794901] ⊃ | [−1.5707963267948968, 1.5707963267948968] ✓ |  |
| 112 | `asin([2, 3])` | ∅ | ∅ ✓ | ∅ ✓ | ∅ ✓ | ∅ ✓ |  |
| 113 | `acos([−2, 2])` | [0, 3.1415926535897936] | [0, 3.1415926535897936] ✓ | [−0, 3.1415926535897936] ✓ | [0, 3.141592653589802] ⊃ | [−0, 3.1415926535897936] ✓ |  |
| 114 | `acos([1, 3])` | [0] | [0] ✓ | [−0] ✓ | [0] ✓ | [−0] ✓ |  |
| 115 | `acos([−3, −1])` | [3.141592653589793, 3.1415926535897936] | [3.141592653589793, 3.1415926535897936] ✓ | [3.141592653589793, 3.1415926535897936] ✓ | [3.141592653589785, 3.141592653589802] ⊃ | [3.141592653589793, 3.1415926535897936] ✓ |  |
| 116 | `acos([−3, −2])` | ∅ | ∅ ✓ | ∅ ✓ | ∅ ✓ | ∅ ✓ |  |
| 117 | `atan([−∞, +∞])` | [−1.5707963267948968, 1.5707963267948968] | [−1.5707963267948968, 1.5707963267948968] ✓ | [−1.5707963267948968, 1.5707963267948968] ✓ | [−1.5707963267949, 1.5707963267949] ⊃ | [−1.5707963267948968, 1.5707963267948968] ✓ |  |
| 118 | `sinh([−∞, +∞])` | [−∞, +∞] | [−∞, +∞] ✓ | [−∞, +∞] ✓ | [−∞, +∞] ✓ | [−∞, +∞] ✓ |  |
| 119 | `cosh([−∞, +∞])` | [1, +∞] | [1, +∞] ✓ | [1, +∞] ✓ | [1, +∞] ✓ | [1, +∞] ✓ |  |
| 120 | `tanh([−∞, +∞])` | [−1, 1] | [−1, 1] ✓ | [−1, 1] ✓ | [−1, 1] ✓ | [−1, 1] ✓ |  |
| 121 | `asinh([−∞, +∞])` | [−∞, +∞] | [−∞, +∞] ✓ | [−∞, +∞] ✓ | [−∞, +∞] ✓ | n/a | Solaris Studio has no asinh, acosh, atanh |
| 122 | `acosh([0, 1])` | [0] | [0] ✓ | [−0] ✓ | [0] ✓ | n/a |  |
| 123 | `acosh([−1, 0.5])` | ∅ | ∅ ✓ | ∅ ✓ | ∅ ✓ | n/a |  |
| 124 | `atanh([−1, 1])` | [−∞, +∞] | [−∞, +∞] ✓ | [−∞, +∞] ✓ | [−∞, +∞] ✓ | n/a |  |
| 125 | `atanh([2, 3])` | ∅ | ∅ ✓ | ∅ ✓ | ∅ ✓ | n/a |  |
| 126 | `abs([−∞, −1])` | [1, +∞] | [1, +∞] ✓ | [1, +∞] ✓ | [1, +∞] ✓ | [1, +∞] ✓ |  |
| 127 | `abs([−∞, 1])` | [0, +∞] | [0, +∞] ✓ | [−0, +∞] ✓ | [0, +∞] ✓ | [−0, +∞] ✓ |  |

### 8. Integer powers (GAOL's pow(x, n), pown of IEEE 1788, filib++'s power(x, n), x**n of Fortran)

From check/non_arithmetic.cpp (test_pow_int), tests/arithmetic.cpp.

| # | Operation | IEEE 1788 | GAOL | libieeep1788 | filib++ | Solaris Studio | Notes |
|---|---|---|---|---|---|---|---|
| 128 | `pow([0], 0)` | [1] | [1] ✓ | [1] ✓ | [1] ✓ | [1] ✓ | pown(x, 0) = 1 for every x (Table 9.1, b) |
| 129 | `pow(∅, 0)` | ∅ | ∅ ✓ | ∅ ✓ | ∅ ✓ | ∅ ✓ |  |
| 130 | `pow([−∞, +∞], 0)` | [1] | [1] ✓ | [1] ✓ | [1] ✓ | [1] ✓ |  |
| 131 | `pow([−3, 5], 2)` | [0, 25] | [−0, 25] ✓ | [−0, 25] ✓ | [0, 25] ✓ | [−0, 25] ✓ |  |
| 132 | `pow([−2, 3], 3)` | [−8, 27] | [−8, 27] ✓ | [−8, 27] ✓ | [−8, 27] ✓ | [−8.000000000000002, 27.000000000000004] ⊃ |  |
| 133 | `pow([−3, 2], −1)` | [−∞, +∞] | [−∞, +∞] ✓ | [−∞, +∞] ✓ | [−∞, +∞] ✓ | [−∞, +∞] ✓ |  |
| 134 | `pow([−3, 2], −2)` | [0.1111111111111111, +∞] | [0.1111111111111111, +∞] ✓ | [0.1111111111111111, +∞] ✓ | [0, +∞] ⊃ | [0.11111111111111109, +∞] ⊃ |  |
| 135 | `pow([−3, 2], −3)` | [−∞, +∞] | [−∞, +∞] ✓ | [−∞, +∞] ✓ | [−∞, +∞] ✓ | [−∞, +∞] ✓ |  |
| 136 | `pow([−3, −2], −3)` | [−0.125, −0.037037037037037035] | [−0.125, −0.037037037037037035] ✓ | [−0.125, −0.037037037037037035] ✓ | [−0.125, −0.037037037037037035] ✓ | [−0.12500000000000003, −0.03703703703703703] ⊃ |  |
| 137 | `pow([2, 3], −4)` | [0.012345679012345678, 0.0625] | [0.012345679012345678, 0.0625] ✓ | [0.012345679012345678, 0.0625] ✓ | [0.012345679012345678, 0.0625] ✓ | [0.012345679012345677, 0.06250000000000001] ⊃ |  |
| 138 | `pow([0], −1)` | ∅ | ∅ ✓ | ∅ ✓ | [MAX, +∞] ✗ | [−∞, +∞] ✗ | pown(0, p) has no value for p < 0 |
| 139 | `pow([0, 2], −1)` | [0.5, +∞] | [0.5, +∞] ✓ | [0.5, +∞] ✓ | [0.5, +∞] ✓ | [−∞, +∞] ⊃ |  |
| 140 | `pow([−2, 0], −1)` | [−∞, −0.5] | [−∞, −0.5] ✓ | [−∞, −0.5] ✓ | [−∞, −0.5] ✓ | [−∞, +∞] ⊃ |  |
| 141 | `pow([−2, 0], −2)` | [0.25, +∞] | [0.25, +∞] ✓ | [0.25, +∞] ✓ | [0, +∞] ⊃ | [0.24999999999999997, +∞] ⊃ |  |
| 142 | `pow([0, +∞], −2)` | [0, +∞] | [0, +∞] ✓ | [−0, +∞] ✓ | [0, +∞] ✓ | [−0, +∞] ✓ |  |
| 143 | `pow([−∞, −MAX], 3)` | [−∞, −MAX] | [−∞, −MAX] ✓ | [−∞, −MAX] ✓ | [−∞, −MAX] ✓ | [−∞, −MAX] ✓ |  |
| 144 | `pow([−∞, −MAX], 4)` | [MAX, +∞] | [MAX, +∞] ✓ | [MAX, +∞] ✓ | [MAX, +∞] ✓ | [MAX, +∞] ✓ |  |
| 145 | `pow([MAX, +∞], 3)` | [MAX, +∞] | [MAX, +∞] ✓ | [MAX, +∞] ✓ | [MAX, +∞] ✓ | [MAX, +∞] ✓ |  |
| 146 | `pow([−15], 17)` | [−9.852612533569336e+19, −9.852612533569334e+19] | [−9.852612533569338e+19, −9.852612533569334e+19] ⊃ | [−9.852612533569336e+19, −9.852612533569334e+19] ✓ | [−9.852612533569338e+19, −9.852612533569334e+19] ⊃ | [−9.852612533569338e+19, −9.852612533569334e+19] ⊃ |  |
| 147 | `pow([10], 400)` | [MAX, +∞] | [MAX, +∞] ✓ | [MAX, +∞] ✓ | [MAX, +∞] ✓ | [MAX, +∞] ✓ |  |
| 148 | `pow([10], −400)` | [0, 2^-1074] | [0, 2^-1074] ✓ | [−0, 2^-1074] ✓ | [0, 5.56268464626801e−309] ⊃ | [−0, 2^-1074] ✓ |  |

### 9. Real powers (GAOL's pow(x, d) and pow(x, y), pow of IEEE 1788 and of filib++, x**y of Fortran)

From tests/elementary.cpp (powers).

| # | Operation | IEEE 1788 | GAOL | libieeep1788 | filib++ | Solaris Studio | Notes |
|---|---|---|---|---|---|---|---|
| 149 | `pow([4], 0.5)` | [2] | [1.9999999999999998, 2.0000000000000004] ⊃ | [2] ✓ | [1.9999999999999978, 2.0000000000000036] ⊃ | [2] ✓ | libieeep1788 and filib++: pow(x, [d]) |
| 150 | `pow([4], 1.5)` | [8] | [7.999999999999999, 8.000000000000002] ⊃ | [8] ✓ | [7.999999999999982, 8.000000000000028] ⊃ | [7.999999999999999, 8.000000000000002] ⊃ |  |
| 151 | `pow([4, 9], 0.5)` | [2, 3] | [1.9999999999999998, 3.0000000000000004] ⊃ | [2, 3] ✓ | [1.9999999999999978, 3.000000000000007] ⊃ | [2, 3] ✓ |  |
| 152 | `pow([4], −0.5)` | [0.5] | [0.49999999999999994, 0.5000000000000001] ⊃ | [0.5] ✓ | [0.4999999999999993, 0.5000000000000008] ⊃ | [0.49999999999999994, 0.5000000000000001] ⊃ |  |
| 153 | `pow([0, 4], 0.5)` | [0, 2] | [0, 2.0000000000000004] ⊃ | [−0, 2] ✓ | [0, 2.0000000000000036] ⊃ | [−0, 2] ✓ |  |
| 154 | `pow([−4, 9], 0.5)` | [0, 3] | [0, 3.0000000000000004] ⊃ | [−0, 3] ✓ | [0, 3.000000000000007] ⊃ | [−0, 3] ✓ |  |
| 155 | `pow([−2, 3], [1, 2])` | [0, 9] | [0, 9.000000000000002] ⊃ | [−0, 9] ✓ | [0, 9.000000000000032] ⊃ | [−0, 9] ✓ |  |
| 156 | `pow([−10, 10], −2)` | [0.009999999999999998, +∞] | [0.009999999999999998, +∞] ✓ | [0.009999999999999998, +∞] ✓ | [0.009999999999999936, +∞] ⊃ | [0.009999999999999998, +∞] ✓ |  |
| 157 | `pow([−2, 3], 3)` | [0, 27] | [−8, 27] ⊃ | [−0, 27] ✓ | [0, 27.000000000000124] ⊃ | [−0, 27.000000000000004] ⊃ | GAOL: pown for an integer exponent (choice of the fork), IEEE 1788's pow: x > 0 only |
| 158 | `pow([−2], 2)` | ∅ | [4] ✗ | ∅ ✓ | ∅ ✓ | ∅ ✓ | GAOL: pown for an integer exponent (choice of the fork), IEEE 1788's pow: x > 0 only |
| 159 | `pow([2, 3], 4)` | [16, 81] | [16, 81] ✓ | [16, 81] ✓ | [15.99999999999996, 81.00000000000047] ⊃ | [15.999999999999998, 81.00000000000001] ⊃ |  |
| 160 | `pow([4], 0)` | [1] | [1] ✓ | [1] ✓ | [1] ✓ | [1] ✓ |  |
| 161 | `pow([−2, 3], [3])` | [0, 27] | [−8, 27] ⊃ | [−0, 27] ✓ | [0, 27.000000000000124] ⊃ | [−0, 27.000000000000004] ⊃ | GAOL: pown for an integer exponent (choice of the fork), IEEE 1788's pow: x > 0 only |
| 162 | `pow([−2, 3], [2])` | [0, 9] | [−0, 9] ✓ | [−0, 9] ✓ | [0, 9.000000000000032] ⊃ | [−0, 9] ✓ |  |
| 163 | `pow([3, 4], [2, 3])` | [9, 64] | [8.999999999999998, 64.00000000000001] ⊃ | [9, 64] ✓ | [8.999999999999984, 64.00000000000038] ⊃ | [9, 64.00000000000001] ⊃ |  |
| 164 | `pow([−4, −1], 0.5)` | ∅ | ∅ ✓ | ∅ ✓ | ∅ ✓ | ∅ ✓ |  |
| 165 | `pow([−4, −1], [0.5])` | ∅ | ∅ ✓ | ∅ ✓ | ∅ ✓ | ∅ ✓ |  |
| 166 | `pow([4], +∞)` | ∅ | ∅ ✓ | ∅ ✓ | [MAX, +∞] ✗ | [MAX, +∞] ✗ | the exponent interval(+∞) is empty |
| 167 | `pow([4], −∞)` | ∅ | ∅ ✓ | ∅ ✓ | [0, 2.2250738585072014e−308] ✗ | [−0, 2^-1074] ✗ |  |
| 168 | `pow([4], NaN)` | ∅ | ∅ ✓ | ∅ ✓ | ∅ ✓ | [−0, +∞] ✗ |  |
| 169 | `pow([4], interval(+∞))` | ∅ | ∅ ✓ | ∅ ✓ | [MAX, +∞] ✗ | [MAX, +∞] ✗ |  |
| 170 | `pow([4], interval(−∞))` | ∅ | ∅ ✓ | ∅ ✓ | [0, 2.2250738585072014e−308] ✗ | [−0, 2^-1074] ✗ |  |
| 171 | `pow(∅, 1.5)` | ∅ | ∅ ✓ | ∅ ✓ | ∅ ✓ | ∅ ✓ |  |
| 172 | `pow([4], ∅)` | ∅ | ∅ ✓ | ∅ ✓ | ∅ ✓ | ∅ ✓ |  |
| 173 | `pow([−2, −1], [1e10])` | ∅ | [−∞, +∞] ✗ | ∅ ✓ | ∅ ✓ | ∅ ✓ | GAOL: pown, [−∞, +∞] for an integer beyond the ints |
| 174 | `pow([2, 3], [2^31])` | [MAX, +∞] | [−∞, +∞] ⊃ | [MAX, +∞] ✓ | [MAX, +∞] ✓ | [MAX, +∞] ✓ | GAOL: [−∞, +∞] for an integer beyond the ints |
| 175 | `pow([0.25, 0.5], [−(2^31+1)])` | [MAX, +∞] | [−∞, +∞] ⊃ | [MAX, +∞] ✓ | [MAX, +∞] ✓ | [MAX, +∞] ✓ |  |
| 176 | `pow([−2, 3], 1e10)` | [0, +∞] | [−∞, +∞] ⊃ | [−0, +∞] ✓ | [0, +∞] ✓ | [−0, +∞] ✓ |  |
| 177 | `pow(∅, [1e10])` | ∅ | ∅ ✓ | ∅ ✓ | ∅ ✓ | ∅ ✓ |  |
| 178 | `pow([−1], [2^31−1])` | ∅ | [−1] ✗ | ∅ ✓ | ∅ ✓ | ∅ ✓ | GAOL: pown for an integer exponent (choice of the fork), IEEE 1788's pow: x > 0 only |
| 179 | `pow([0], [0])` | ∅ | [1] ✗ | ∅ ✓ | [0, +∞] ✗ | [−0, +∞] ✗ | pow(0, y) has no value for y ≤ 0 (Table 9.1, c); GAOL: pown(0, 0) = 1 |
| 180 | `pow([0], 0)` | ∅ | [1] ✗ | ∅ ✓ | [0, +∞] ✗ | [−0, +∞] ✗ |  |
| 181 | `pow([−∞, +∞], [0])` | [1] | [1] ✓ | [1] ✓ | [0, +∞] ⊃ | [−0, +∞] ⊃ |  |
| 182 | `pow([0, 2], [−1])` | [0.5, +∞] | [0.5, +∞] ✓ | [0.5, +∞] ✓ | [0.4999999999999993, +∞] ⊃ | [0.5, +∞] ✓ |  |
| 183 | `pow([−2, 0], [−1])` | ∅ | [−∞, −0.5] ✗ | ∅ ✓ | [MAX, +∞] ✗ | [+∞] ✗ | GAOL: pown for an integer exponent (choice of the fork), IEEE 1788's pow: x > 0 only |
| 184 | `pow([0], [0.5])` | [0] | [−0] ✓ | [−0] ✓ | [0, 2.2250738585072014e−308] ⊃ | [−0] ✓ | pow(0, y) = 0 for y > 0 |
| 185 | `pow([0], 0.5)` | [0] | [−0] ✓ | [−0] ✓ | [0, 2.2250738585072014e−308] ⊃ | [−0] ✓ |  |
| 186 | `pow([−2, 0], [2.5])` | [0] | [−0] ✓ | [−0] ✓ | [0, 2.2250738585072014e−308] ⊃ | [−0] ✓ |  |
| 187 | `pow([0], [0, 1])` | [0] | [−0] ✓ | [−0] ✓ | [0, +∞] ⊃ | [−0, +∞] ⊃ |  |
| 188 | `pow([0], [−1, 1])` | [0] | [−0] ✓ | [−0] ✓ | [0, +∞] ⊃ | [−0, +∞] ⊃ |  |
| 189 | `pow([0], [−1])` | ∅ | ∅ ✓ | ∅ ✓ | [MAX, +∞] ✗ | [+∞] ✗ |  |
| 190 | `pow([0], [−0.5])` | ∅ | ∅ ✓ | ∅ ✓ | [MAX, +∞] ✗ | [MAX, +∞] ✗ |  |
| 191 | `pow([0], −0.5)` | ∅ | ∅ ✓ | ∅ ✓ | [MAX, +∞] ✗ | [MAX, +∞] ✗ |  |
| 192 | `pow([−2, 0], [−0.5])` | ∅ | ∅ ✓ | ∅ ✓ | [MAX, +∞] ✗ | [MAX, +∞] ✗ |  |
| 193 | `pow([0], [−1, 0])` | ∅ | ∅ ✓ | ∅ ✓ | [0, +∞] ✗ | [−0, +∞] ✗ |  |
| 194 | `pow([0.5], [−∞, +∞])` | [0, +∞] | [0, +∞] ✓ | [−0, +∞] ✓ | [0, +∞] ✓ | [−0, +∞] ✓ |  |
| 195 | `pow([1], [−∞, +∞])` | [1] | [1] ✓ | [1] ✓ | [0, +∞] ⊃ | [−0, +∞] ⊃ |  |
| 196 | `pow([0, 1], [1, +∞])` | [0, 1] | [0, 1] ✓ | [−0, 1] ✓ | [0, +∞] ⊃ | [−0, +∞] ⊃ |  |
| 197 | `pow([2, +∞], [−1, 1])` | [0, +∞] | [0, +∞] ✓ | [−0, +∞] ✓ | [0, +∞] ✓ | [−0, +∞] ✓ |  |

### 10. n-th roots (GAOL's nth_root, rootn of IEEE 1788)

From check/non_arithmetic.cpp, tests/arithmetic.cpp.

| # | Operation | IEEE 1788 | GAOL | libieeep1788 | filib++ | Solaris Studio | Notes |
|---|---|---|---|---|---|---|---|
| 198 | `nth_root([−8, 27], 3)` | [−2, 3] | [−2.0000000000000004, 3.000000000000001] ⊃ | n/a | n/a | n/a | libieeep1788, filib++ and Solaris Studio have no rootn; pown_rev([−8, 27], 3) of libieeep1788 is [−2, 3] |
| 199 | `nth_root([−4, 9], 2)` | [0, 3] | [0, 3] ✓ | n/a | n/a | n/a |  |
| 200 | `nth_root([−4, −1], 2)` | ∅ | ∅ ✓ | n/a | n/a | n/a |  |
| 201 | `nth_root([−8, −1], 3)` | [−2, −1] | [−2.0000000000000004, −1] ⊃ | n/a | n/a | n/a |  |
| 202 | `nth_root([0], 5)` | [0] | [0] ✓ | n/a | n/a | n/a |  |
| 203 | `nth_root([−∞, +∞], 3)` | [−∞, +∞] | [−∞, +∞] ✓ | n/a | n/a | n/a |  |
| 204 | `nth_root([−∞, +∞], 0)` | — | ∅ | n/a | n/a | n/a | rootn(x, q) is for q ≠ 0 only (Table 10.5) |
| 205 | `nth_root([0, +∞], 2)` | [0, +∞] | [0, +∞] ✓ | n/a | n/a | n/a |  |

### 11. Numeric and set functions

From tests/other_functions.cpp, tests/numbers.cpp.

| # | Operation | IEEE 1788 | GAOL | libieeep1788 | filib++ | Solaris Studio | Notes |
|---|---|---|---|---|---|---|---|
| 206 | `[−∞, +∞].midpoint()` | 0 | 0 ✓ | 0 ✓ | 0 ✓ | 0 ✓ | mid (12.12.8) |
| 207 | `[−∞, 1].midpoint()` | −MAX | −MAX ✓ | −MAX ✓ | −∞ ✗ | −∞ ✗ |  |
| 208 | `[1, +∞].midpoint()` | MAX | MAX ✓ | MAX ✓ | +∞ ✗ | +∞ ✗ |  |
| 209 | `∅.midpoint()` | NaN | NaN ✓ | NaN ✓ | NaN ✓ | NaN ✓ |  |
| 210 | `[MAX/2, MAX].midpoint()` | 1.3482698511467367e+308 | 1.3482698511467367e+308 ✓ | 1.3482698511467367e+308 ✓ | 1.3482698511467367e+308 ✓ | 1.3482698511467367e+308 ✓ |  |
| 211 | `[0, 2^-1074].midpoint()` | 0 | 0 ✓ | 0 ✓ | 0 ✓ | 0 ✓ | the tie rounded to even |
| 212 | `[−2^-1074, 4·2^-1074].midpoint()` | 2·2^-1074 | 2·2^-1074 ✓ | 2·2^-1074 ✓ | 2·2^-1074 ✓ | 2·2^-1074 ✓ | 1.5·2^-1074, to even |
| 213 | `[−∞, 1].width()` | +∞ | +∞ ✓ | +∞ ✓ | +∞ ✓ | +∞ ✓ | wid (12.12.8), diam in filib++ |
| 214 | `[−MAX, MAX].width()` | +∞ | +∞ ✓ | +∞ ✓ | +∞ ✓ | +∞ ✓ |  |
| 215 | `∅.width()` | NaN | NaN ✓ | NaN ✓ | NaN ✓ | NaN ✓ |  |
| 216 | `[−3, 2].mag()` | 3 | 3 ✓ | 3 ✓ | 3 ✓ | 3 ✓ |  |
| 217 | `∅.mag()` | NaN | NaN ✓ | NaN ✓ | NaN ✓ | NaN ✓ |  |
| 218 | `[−3, 2].mig()` | 0 | 0 ✓ | 0 ✓ | 0 ✓ | 0 ✓ |  |
| 219 | `[−3, −2].mig()` | 2 | 2 ✓ | 2 ✓ | 2 ✓ | 2 ✓ |  |
| 220 | `∅.mig()` | NaN | NaN ✓ | NaN ✓ | NaN ✓ | NaN ✓ |  |
| 221 | `[1, 2] \| ∅` | [1, 2] | [1, 2] ✓ | [1, 2] ✓ | [1, 2] ✓ | [1, 2] ✓ | convex_hull in libieeep1788, hull in filib++, .ih. in Solaris Studio |
| 222 | `[1, 2] & [3, 4]` | ∅ | ∅ ✓ | ∅ ✓ | ∅ ✓ | ∅ ✓ | intersection in libieeep1788, intersect in filib++, .ix. in Solaris Studio |
| 223 | `[1, 2] & [2, 3]` | [2] | [2] ✓ | [2] ✓ | [2] ✓ | [2] ✓ |  |
| 224 | `min([−∞, 1], [0, 2])` | [−∞, 1] | [−∞, 1] ✓ | [−∞, 1] ✓ | [−∞, 1] ✓ | [−∞, 1] ✓ | imin and imax in filib++ |
| 225 | `max([1, 2], ∅)` | ∅ | ∅ ✓ | ∅ ✓ | [1, 2] ✗ | ∅ ✓ |  |
| 226 | `−[−∞, 1]` | [−1, +∞] | [−1, +∞] ✓ | [−1, +∞] ✓ | [−1, +∞] ✓ | [−1, +∞] ✓ |  |
| 227 | `[1, 2].rad()` | 0.5 | 0.5 ✓ | 0.5 ✓ | 0.5 ✓ | n/a | rad (12.12.8): the smallest r with x in [m − r, m + r]; Solaris Studio has none |
| 228 | `[1, 1.0000000000000007].rad()` | 4.440892098500626e−16 | 4.440892098500626e−16 ✓ | 4.440892098500626e−16 ✓ | 3.3306690738754696e−16 ✗ | n/a | m = 1 + 2^-51, the tie rounded to even |
| 229 | `[0, 2^-1074].rad()` | 2^-1074 | 2^-1074 ✓ | 2^-1074 ✓ | 0 ✗ | n/a |  |
| 230 | `[−MAX, MAX].rad()` | MAX | MAX ✓ | MAX ✓ | MAX ✓ | n/a |  |
| 231 | `[−∞, 1].rad()` | +∞ | +∞ ✓ | +∞ ✓ | +∞ ✓ | n/a |  |
| 232 | `∅.rad()` | NaN | NaN ✓ | NaN ✓ | NaN ✓ | n/a |  |

### 12. Interval literals of the set-based flavor

From tests/numbers.cpp (ieee_literals).

| # | Operation | IEEE 1788 | GAOL | libieeep1788 | filib++ | Solaris Studio | Notes |
|---|---|---|---|---|---|---|---|
| 233 | `interval("[ ]")` | ∅ | ∅ ✓ | ∅ ✓ | exception interval_io_exception ✗ | read error, iostat -1 ✗ | 12.11.3 |
| 234 | `interval("[Empty]")` | ∅ | ∅ ✓ | ∅ ✓ | exception interval_io_exception ✗ | ∅ ✓ | the case of the letters is ignored (9.7.1) |
| 235 | `interval("[,]")` | [−∞, +∞] | [−∞, +∞] ✓ | [−∞, +∞] ✓ | [0] ✗ | read error, iostat 1210 ✗ | bounds left out are infinite |
| 236 | `interval("[1,]")` | [1, +∞] | [1, +∞] ✓ | [1, +∞] ✓ | ∅ ✗ | read error, iostat 1210 ✗ |  |
| 237 | `interval("[-Inf, 2/3]")` | [−∞, 0.6666666666666667] | [−∞, 0.6666666666666667] ✓ | [−∞, 0.6666666666666667] ✓ | exception interval_io_exception ✗ | read error, iostat 1210 ✗ |  |
| 238 | `interval("[0x1.3p-1, 2/3]")` | [0.59375, 0.6666666666666667] | [0.59375, 0.6666666666666667] ✓ | [0.59375, 0.6666666666666667] ✓ | exception interval_io_exception ✗ | read error, iostat 1210 ✗ | hexadecimal number (9.7.2) |
| 239 | `interval("[0x1.00000000000001p0]")` | [1, 1.0000000000000002] | [1, 1.0000000000000002] ✓ | [1, 1.0000000000000002] ✓ | exception interval_io_exception ✗ | read error, iostat 1210 ✗ |  |
| 240 | `interval("-10??u")` | [−10, +∞] | [−10, +∞] ✓ | [−10, +∞] ✓ | exception interval_io_exception ✗ | read error, iostat 1210 ✗ | uncertain form with an infinite radius |
| 241 | `interval("-10?12")` | [−22, 2] | [−22, 2] ✓ | [−22, 2] ✓ | exception interval_io_exception ✗ | read error, iostat 1210 ✗ |  |
| 242 | `interval("[inf]")` | ∅ | ∅ ✓ | ∅ ✓ | exception interval_io_exception ✗ | [MAX, +∞] ✗ | not a literal: numsToInterval(+∞, +∞) has no value |
| 243 | `interval("[inf, inf]")` | ∅ | ∅ ✓ | ∅ ✓ | [MAX, +∞] ✗ | [MAX, +∞] ✗ |  |

### 13. Comparisons (Tables 10.3 and 10.4)

From tests/other_functions.cpp (comparisons).

| # | Operation | IEEE 1788 | GAOL | libieeep1788 | filib++ | Solaris Studio | Notes |
|---|---|---|---|---|---|---|---|
| 244 | `precedes([1, 2], [2, 3])` | true | true ✓ | true ✓ | true ✓ | true ✓ | GAOL: certainly_leq, certainly_le, set_strictly_contains, set_contains, set_eq, set_disjoint; filib++: cle, clt, interior, subset, seq, disjoint; Solaris Studio: .cle., .clt., .int., .sb., .seq., .dj. |
| 245 | `precedes([1, 2], ∅)` | true | true ✓ | true ✓ | false ✗ | false ✗ | true when either interval is empty (Table 10.4) |
| 246 | `precedes(∅, [1, 2])` | true | true ✓ | true ✓ | false ✗ | false ✗ |  |
| 247 | `strictPrecedes([1, 2], [2, 3])` | false | false ✓ | false ✓ | false ✓ | false ✓ |  |
| 248 | `strictPrecedes([1, 2], ∅)` | true | true ✓ | true ✓ | false ✗ | false ✗ |  |
| 249 | `interior([1.5], [1, 2])` | true | true ✓ | true ✓ | true ✓ | true ✓ |  |
| 250 | `interior([1, 2], [1, 3])` | false | false ✓ | false ✓ | false ✓ | false ✓ |  |
| 251 | `interior([−∞, +∞], [−∞, +∞])` | true | true ✓ | true ✓ | false ✗ | false ✗ | −∞ <0 −∞ and +∞ <0 +∞ (Table 10.3) |
| 252 | `interior([2, +∞], [1, +∞])` | true | true ✓ | true ✓ | false ✗ | false ✗ |  |
| 253 | `interior(∅, ∅)` | true | true ✓ | true ✓ | true ✓ | true ✓ |  |
| 254 | `subset(∅, [1, 2])` | true | true ✓ | true ✓ | true ✓ | true ✓ |  |
| 255 | `subset([1, 2], ∅)` | false | false ✓ | false ✓ | false ✓ | false ✓ |  |
| 256 | `equal(∅, ∅)` | true | true ✓ | true ✓ | true ✓ | true ✓ |  |
| 257 | `equal([1, +∞], [1, +∞])` | true | true ✓ | true ✓ | true ✓ | true ✓ |  |
| 258 | `disjoint([1, 2], ∅)` | true | true ✓ | true ✓ | true ✓ | true ✓ |  |
| 259 | `disjoint([1, 2], [2, 3])` | false | false ✓ | false ✓ | false ✓ | false ✓ |  |
| 260 | `certainly_eq([2], [1, 2])` | — | false | n/a | false | false | not in IEEE 1788: for all x, y, x = y (false); filib++: ceq, Solaris Studio: .ceq. |
| 261 | `certainly_eq([2], [2])` | — | true | n/a | true | true |  |
| 262 | `certainly_eq(∅, ∅)` | — | true | n/a | false | false |  |

### 14. atan2(y, x), defined on the plane but (0, 0), with values in (−π, π] (Table 9.1)

From tests/elementary.cpp (atan2_of_boxes).

| # | Operation | IEEE 1788 | GAOL | libieeep1788 | filib++ | Solaris Studio | Notes |
|---|---|---|---|---|---|---|---|
| 263 | `atan2([0], [0])` | ∅ | ∅ ✓ | ∅ ✓ | n/a | [−3.1415926535897936, 3.1415926535897936] ✗ | atan2(0, 0) has no value; filib++ has no atan2 |
| 264 | `atan2(∅, [1])` | ∅ | ∅ ✓ | ∅ ✓ | n/a | ∅ ✓ |  |
| 265 | `atan2([1], ∅)` | ∅ | ∅ ✓ | ∅ ✓ | n/a | ∅ ✓ |  |
| 266 | `atan2([1], [1])` | [0.7853981633974483, 0.7853981633974484] | [0.7853981633974483, 0.7853981633974484] ✓ | [0.7853981633974483, 0.7853981633974484] ✓ | n/a | [0.7853981633974483, 0.7853981633974484] ✓ |  |
| 267 | `atan2([1, 2], [1, 2])` | [0.4636476090008061, 1.1071487177940906] | [0.46364760900080604, 1.1071487177940906] ⊃ | [0.4636476090008061, 1.1071487177940906] ✓ | n/a | [0.4636476090008061, 1.1071487177940906] ✓ |  |
| 268 | `atan2([1, 2], [−2, −1])` | [2.0344439357957027, 2.6779450445889874] | [2.0344439357957023, 2.6779450445889874] ⊃ | [2.0344439357957027, 2.6779450445889874] ✓ | n/a | [2.0344439357957027, 2.6779450445889874] ✓ |  |
| 269 | `atan2([−2, −1], [−2, −1])` | [−2.6779450445889874, −2.0344439357957027] | [−2.6779450445889874, −2.0344439357957023] ⊃ | [−2.6779450445889874, −2.0344439357957027] ✓ | n/a | [−2.6779450445889874, −2.0344439357957027] ✓ |  |
| 270 | `atan2([−2, −1], [1, 2])` | [−1.1071487177940906, −0.4636476090008061] | [−1.1071487177940906, −0.46364760900080604] ⊃ | [−1.1071487177940906, −0.4636476090008061] ✓ | n/a | [−1.1071487177940906, −0.4636476090008061] ✓ |  |
| 271 | `atan2([1, 2], [−1, 1])` | [0.7853981633974483, 2.3561944901923453] | [0.7853981633974483, 2.3561944901923453] ✓ | [0.7853981633974483, 2.3561944901923453] ✓ | n/a | [0.7853981633974483, 2.3561944901923453] ✓ |  |
| 272 | `atan2([−1, 1], [1, 2])` | [−0.7853981633974484, 0.7853981633974484] | [−0.7853981633974484, 0.7853981633974484] ✓ | [−0.7853981633974484, 0.7853981633974484] ✓ | n/a | [−0.7853981633974484, 0.7853981633974484] ✓ |  |
| 273 | `atan2([0], [1, 2])` | [0] | [0] ✓ | [−0] ✓ | n/a | [−0] ✓ |  |
| 274 | `atan2([0], [−2, −1])` | [3.141592653589793, 3.1415926535897936] | [3.141592653589793, 3.1415926535897936] ✓ | [3.141592653589793, 3.1415926535897936] ✓ | n/a | [3.141592653589793, 3.1415926535897936] ✓ |  |
| 275 | `atan2([0], [−1, 1])` | [0, 3.1415926535897936] | [0, 3.1415926535897936] ✓ | [−0, 3.1415926535897936] ✓ | n/a | [−3.1415926535897936, 3.1415926535897936] ⊃ |  |
| 276 | `atan2([0], [−1, 0])` | [3.141592653589793, 3.1415926535897936] | [3.141592653589793, 3.1415926535897936] ✓ | [3.141592653589793, 3.1415926535897936] ✓ | n/a | [−3.1415926535897936, 3.1415926535897936] ⊃ |  |
| 277 | `atan2([1, 2], [0])` | [1.5707963267948966, 1.5707963267948968] | [1.5707963267948966, 1.5707963267948968] ✓ | [1.5707963267948966, 1.5707963267948968] ✓ | n/a | [1.5707963267948966, 1.5707963267948968] ✓ |  |
| 278 | `atan2([−2, −1], [0])` | [−1.5707963267948968, −1.5707963267948966] | [−1.5707963267948968, −1.5707963267948966] ✓ | [−1.5707963267948968, −1.5707963267948966] ✓ | n/a | [−1.5707963267948968, −1.5707963267948966] ✓ |  |
| 279 | `atan2([−1, 1], [0])` | [−1.5707963267948968, 1.5707963267948968] | [−1.5707963267948968, 1.5707963267948968] ✓ | [−1.5707963267948968, 1.5707963267948968] ✓ | n/a | [−3.1415926535897936, 3.1415926535897936] ⊃ |  |
| 280 | `atan2([0, 1], [0])` | [1.5707963267948966, 1.5707963267948968] | [1.5707963267948966, 1.5707963267948968] ✓ | [1.5707963267948966, 1.5707963267948968] ✓ | n/a | [−3.1415926535897936, 3.1415926535897936] ⊃ |  |
| 281 | `atan2([0, 1], [−2, −1])` | [2.356194490192345, 3.1415926535897936] | [2.3561944901923444, 3.1415926535897936] ⊃ | [2.356194490192345, 3.1415926535897936] ✓ | n/a | [2.356194490192345, 3.141592653589794] ⊃ |  |
| 282 | `atan2([−1, 0], [−2, −1])` | [−3.1415926535897936, 3.1415926535897936] | [−3.1415926535897936, 3.1415926535897936] ✓ | [−3.1415926535897936, 3.1415926535897936] ✓ | n/a | [−3.141592653589794, −2.356194490192345] ✗ | points on the half-line y = 0, x < 0, where atan2 is π, and points below it, where it is next to −π |
| 283 | `atan2([−1, 1], [−2, −1])` | [−3.1415926535897936, 3.1415926535897936] | [−3.1415926535897936, 3.1415926535897936] ✓ | [−3.1415926535897936, 3.1415926535897936] ✓ | n/a | [2.356194490192345, 3.9269908169872423] ✗ | points on the half-line y = 0, x < 0, where atan2 is π, and points below it, where it is next to −π |
| 284 | `atan2([−1, 0], [1, 2])` | [−0.7853981633974484, 0] | [−0.7853981633974484, 0] ✓ | [−0.7853981633974484, 0] ✓ | n/a | [−0.7853981633974484, 0] ✓ |  |
| 285 | `atan2([−1, 1], [−1, 1])` | [−3.1415926535897936, 3.1415926535897936] | [−3.1415926535897936, 3.1415926535897936] ✓ | [−3.1415926535897936, 3.1415926535897936] ✓ | n/a | [−3.1415926535897936, 3.1415926535897936] ✓ |  |
| 286 | `atan2([−∞, +∞], [−∞, +∞])` | [−3.1415926535897936, 3.1415926535897936] | [−3.1415926535897936, 3.1415926535897936] ✓ | [−3.1415926535897936, 3.1415926535897936] ✓ | n/a | [−3.1415926535897936, 3.1415926535897936] ✓ |  |
| 287 | `atan2([1, +∞], [1, +∞])` | [0, 1.5707963267948968] | [0, 1.5707963267948968] ✓ | [−0, 1.5707963267948968] ✓ | n/a | [−0, 1.5707963267948968] ✓ |  |
| 288 | `atan2([1, +∞], [−∞, −1])` | [1.5707963267948966, 3.1415926535897936] | [1.5707963267948966, 3.1415926535897936] ✓ | [1.5707963267948966, 3.1415926535897936] ✓ | n/a | [1.5707963267948966, 3.1415926535897936] ✓ |  |
| 289 | `atan2([−∞, −1], [−∞, −1])` | [−3.1415926535897936, −1.5707963267948966] | [−3.1415926535897936, −1.5707963267948966] ✓ | [−3.1415926535897936, −1.5707963267948966] ✓ | n/a | [−3.1415926535897936, −1.5707963267948966] ✓ |  |
| 290 | `atan2([−∞, +∞], [1, 2])` | [−1.5707963267948968, 1.5707963267948968] | [−1.5707963267948968, 1.5707963267948968] ✓ | [−1.5707963267948968, 1.5707963267948968] ✓ | n/a | [−1.5707963267948968, 1.5707963267948968] ✓ |  |
| 291 | `atan2([1, 2], [−∞, +∞])` | [0, 3.1415926535897936] | [0, 3.1415926535897936] ✓ | [−0, 3.1415926535897936] ✓ | n/a | [−0, 3.1415926535897936] ✓ |  |

<!-- END GENERATED TABLES -->
```plaintext

```
