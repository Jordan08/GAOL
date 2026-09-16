# Special cases: GAOL, libieeep1788 and Solaris Studio

Part of the [comparison](README.md) of GAOL with libieeep1788 and Solaris
Studio.

The special cases of GAOL's tests (`tests/*.cpp`, and `check/*.cpp` for the
integer powers) are computed by the three libraries: zeros, infinities and NaN
as bounds or operands, empty sets, divisions by intervals containing zero,
arguments at the edges of the domains, the powers `pow` and `pown`, the
operators `+=`, `-=`, `*=`, `/=` with special doubles, the reading of
intervals from text and the numeric functions of intervals. Each result is
compared with the result IEEE 1788-2015 defines for the set-based flavor, the
tightest interval of doubles, computed with mpmath:

- ✓ the result of IEEE 1788;
- ⊃ an interval enclosing it, wider;
- ✗ another result: a set not enclosing it, a set where IEEE 1788 has the empty
  set, or an exception;
- n/a: the library has no such operation;
- no mark: IEEE 1788 defines no result (two cases).

Signed zeros are not told apart. The bounds are written with the shortest
decimal that reads back as the same double, MAX being the largest double and
pred(MAX) the one below it.

| Library | Version | Intervals |
|---|---|---|
| GAOL | this fork | `gaol::interval`, CMake Release build, SSE2 intervals, mathlib 2.1.1 |
| libieeep1788 | last commit, 30 March 2015 | `interval<double, mpfr_bin_ieee754_flavor>`: the set-based flavor of the preliminary IEEE P1788, on doubles, computed with MPFR |
| Solaris Studio | Sun Fortran 95 8.7 (Solaris Studio 12.4), Linux x86-64 | `interval(8)` of `f90 -xia`, computed by `libsunimath` |

Each case is written once, in [code/cases.py](code/cases.py), and translated
for each library: libieeep1788 has no operators of an interval and a double
nor compound assignments, which are computed with `II(d, d)` there; Solaris
Studio's are written `x = x + d`. [code/README.md](code/README.md) says how
to run the comparison again.

## What the cases show

**libieeep1788** gives the result of IEEE 1788 in each of the 217 cases
where IEEE 1788 defines one and libieeep1788 has the operation: it computes
every bound with MPFR, correctly rounded. It has no n-th root (`rootn`), no
operators with doubles and no compound assignments.

**GAOL** gives the result of IEEE 1788, or an interval enclosing it, in 209
cases out of 221, and something else in 12:

- The hybrid `pow(x, y)` of this fork takes the integer power `pown` for a
  degenerate integer exponent, where IEEE 1788's `pow` only takes the part of
  x in [0, +∞] (see [What differs from GAOL](../differences.md)):
  `pow([−2], 2.0)` is [4], `pow([0], [0])` is [1], `pow([−1], [2^31−1])` is
  [−1] and `pow([−2, 0], [−1])` is [−∞, −0.5], where IEEE 1788 has ∅
  (cases 158, 178 to 180, 183); `pow([−2, −1], [1e10])`, an integer beyond the
  ints, is [−∞, +∞] (173). Five more cases enclose IEEE 1788's result for the
  same reason (157, 161, 174 to 176).
- `log([−4, 0])` and `log([0])` are [−∞, −MAX]: `log()` keeps the part of x
  in [0, +∞], 0 included, and takes log(0) as −∞, where IEEE 1788, whose
  `log` is defined on (0, +∞), has ∅ (97, 98).
- `nth_root([−8, 27], 3)` is [−2^-1074, 3.000000000000001]: GAOL takes the
  n-th roots of the part of x in [0, +∞], for odd n too, as GAOL's own checks
  want (`check/non_arithmetic.cpp`), where the `rootn` of IEEE 1788 is
  [−2, 3] (198).
- `interval("[entire]")` and the uncertain form `interval("3.56?1")` throw
  `input_format_error`: GAOL's parser reads `[empty]`, but neither
  `[entire]` nor the uncertain form of IEEE 1788 (28, 29). GAOL reads a bare
  number, `interval("0.1")`, as the interval enclosing it, an extension of the
  literals IEEE 1788 allows (19).
- `width()` of the empty set is −1, where IEEE 1788's `wid` is NaN (212).

Its wider results are a few doubles off (sin, cos, tan, acos, `pow(x, y)`,
which is exp(y log x)), or a subnormal bound where IEEE 1788 has 0:
`log([0, 1])` is [−∞, 2^-1074], `acosh([0, 1])` is [−3·2^-1074, 3·2^-1074].
`pow([1], [−∞, +∞])` is [0, +∞] rather than [1], `pow([0, 1], [1, +∞])`
[0, +∞] rather than [0, 1], and `pow([10], −400)` [0, 5.6e−309] rather than
[0, 2^-1074].

**Solaris Studio** follows the containment sets of Sun's interval arithmetic
(G. W. Walster), whose values include the infinities: 1/0 is {−∞, +∞}, 0 × ∞
is every extended real, and +∞ is a point. So:

- The intervals built from an infinity are not empty: `interval(+∞)` is
  [MAX, +∞], `x + ∞` and `x = +∞` give [MAX, +∞], and `[1, 2] / ∞` gives
  [0, 1.1e−308] (1 to 4, 11, 12, 61 to 64, 79, 80).
- The arguments that are not an interval give [−∞, +∞] rather than ∅:
  `interval(2, 1)`, NaN bounds, `x + NaN` (5 to 10, 67).
- A division by an interval containing zero is [−∞, +∞], a division by [0]
  too (30 to 43, 68 to 70, 83), and so is a product of a zero and an infinite
  bound (50 to 53, 57, 60, 65, 66, 74, 75, 81, 82).
- `pow` includes the limits: `pow([0], [0])` is [0, +∞], `pow([0], [−1])` is
  [+∞] and `pow([4], [+∞])` is [MAX, +∞] (166 to 170, 179 to 181, 183, 187 to
  193), and `x**(−n)` is [−∞, +∞] when x contains 0 (138 to 140). The powers
  are often one or two doubles wider than the tightest, `x**n` included (132,
  134, 136, 137, 141, 146, 150, 152, 157, 159, 161, 163).
- `mid([−∞, 1])` is −∞ and `mid([1, +∞])` is +∞, rather than −MAX and MAX
  (204, 205).
- Reading `0.1` gives [0, 0.2]: a single number read as an interval carries an
  uncertainty of one unit of its last digit, as the uncertain form `0.1?1` of
  IEEE 1788 would. Bounds in the wrong order are a read error rather than ∅,
  and neither a rational bound (`1/3`), `[entire]` nor `3.56?1` are read (19,
  22, 23, 28, 29).
- It has no `asinh`, `acosh`, `atanh`, n-th root, relational division
  (`mul_rev`), nor `sqr`, computed as `x**2`.

Its elementary functions are the tightest intervals, or one double wider, and
give IEEE 1788's results at the edges of their domains, `log([−4, 0])` and
`log([0])` aside, which are [−∞, −MAX] as in GAOL (97, 98).

## The cases

<!-- BEGIN GENERATED TABLES (doc/compare/code/cases.py) -->

223 cases. Each result is marked against the result IEEE 1788-2015 defines (the tightest interval of doubles, computed with mpmath):

| | GAOL | libieeep1788 | Solaris Studio |
|---|---|---|---|
| ✓ the result of IEEE 1788 | 180 | 217 | 117 |
| ⊃ encloses it, wider | 29 | 0 | 36 |
| ✗ differs | 12 | 0 | 52 |
| n/a no such operation | 0 | 5 | 17 |

### 1. Constructors and assignment

From tests/numbers.cpp (constructors).

| # | Operation | IEEE 1788 | GAOL | libieeep1788 | Solaris Studio | Notes |
|---|---|---|---|---|---|---|
| 001 | `interval(+∞)` | ∅ | ∅ ✓ | ∅ ✓ | [MAX, +∞] ✗ | numsToInterval(+∞, +∞) fails (12.12.7) |
| 002 | `interval(−∞)` | ∅ | ∅ ✓ | ∅ ✓ | [−∞, −MAX] ✗ |  |
| 003 | `interval(+∞, +∞)` | ∅ | ∅ ✓ | ∅ ✓ | [MAX, +∞] ✗ |  |
| 004 | `interval(−∞, −∞)` | ∅ | ∅ ✓ | ∅ ✓ | [−∞, −MAX] ✗ |  |
| 005 | `interval(+∞, 1)` | ∅ | ∅ ✓ | ∅ ✓ | [−∞, +∞] ✗ |  |
| 006 | `interval(1, −∞)` | ∅ | ∅ ✓ | ∅ ✓ | [−∞, +∞] ✗ |  |
| 007 | `interval(2, 1)` | ∅ | ∅ ✓ | ∅ ✓ | [−∞, +∞] ✗ | l > u: the constructor fails |
| 008 | `interval(NaN)` | ∅ | ∅ ✓ | ∅ ✓ | [−∞, +∞] ✗ |  |
| 009 | `interval(NaN, 1)` | ∅ | ∅ ✓ | ∅ ✓ | [−∞, +∞] ✗ |  |
| 010 | `interval(1, NaN)` | ∅ | ∅ ✓ | ∅ ✓ | [−∞, +∞] ✗ |  |
| 011 | `x = [1, 2]; x = +∞` | ∅ | ∅ ✓ | ∅ ✓ | [MAX, +∞] ✗ | libieeep1788 has no assignment of a double: II(d, d) |
| 012 | `x = [1, 2]; x = −∞` | ∅ | ∅ ✓ | ∅ ✓ | [−∞, −MAX] ✗ |  |
| 013 | `[−∞, 1]` | [−∞, 1] | [−∞, 1] ✓ | [−∞, 1] ✓ | [−∞, 1] ✓ |  |
| 014 | `[1, +∞]` | [1, +∞] | [1, +∞] ✓ | [1, +∞] ✓ | [1, +∞] ✓ |  |
| 015 | `[−∞, +∞]` | [−∞, +∞] | [−∞, +∞] ✓ | [−∞, +∞] ✓ | [−∞, +∞] ✓ |  |
| 016 | `[−0]` | [0] | [−0] ✓ | [−0] ✓ | [−0] ✓ |  |
| 017 | `[−MAX, MAX]` | [−MAX, MAX] | [−MAX, MAX] ✓ | [−MAX, MAX] ✓ | [−MAX, MAX] ✓ |  |
| 018 | `empty set` | ∅ | ∅ ✓ | ∅ ✓ | ∅ ✓ | Solaris Studio has no empty constant: [1, 2] .ix. [3, 4] |

### 2. Reading intervals from text

From tests/numbers.cpp (numbers).

| # | Operation | IEEE 1788 | GAOL | libieeep1788 | Solaris Studio | Notes |
|---|---|---|---|---|---|---|
| 019 | `interval("0.1")` | — | [0.09999999999999999, 0.1] | ∅ | [0, 0.2] | not an interval literal (9.7.4): ∅, unless the implementation extends the literals (9.7.1), as GAOL does; Solaris Studio reads 0.1 ± 0.1 |
| 020 | `interval("[0.1]")` | [0.09999999999999999, 0.1] | [0.09999999999999999, 0.1] ✓ | [0.09999999999999999, 0.1] ✓ | [0.09999999999999999, 0.1] ✓ |  |
| 021 | `interval("[0.1, 0.3]")` | [0.09999999999999999, 0.30000000000000004] | [0.09999999999999999, 0.30000000000000004] ✓ | [0.09999999999999999, 0.30000000000000004] ✓ | [0.09999999999999999, 0.30000000000000004] ✓ |  |
| 022 | `interval("[1/3, 0.3]")` | ∅ | ∅ ✓ | ∅ ✓ | read error, iostat 1210 ✗ | rational literal, l > u |
| 023 | `interval("[0.3, 0.1]")` | ∅ | ∅ ✓ | ∅ ✓ | read error, iostat 1211 ✗ | l > u |
| 024 | `interval("[1e309]")` | [MAX, +∞] | [MAX, +∞] ✓ | [MAX, +∞] ✓ | [MAX, +∞] ✓ | the real number 1e309, beyond the doubles |
| 025 | `interval("[1e-400]")` | [0, 2^-1074] | [0, 2^-1074] ✓ | [−0, 2^-1074] ✓ | [0, 2^-1074] ✓ |  |
| 026 | `interval("[1, inf]")` | [1, +∞] | [1, +∞] ✓ | [1, +∞] ✓ | [1, +∞] ✓ | a literal of the set-based flavor (10.5.1) |
| 027 | `interval("[empty]")` | ∅ | ∅ ✓ | ∅ ✓ | ∅ ✓ | a literal of the set-based flavor (10.5.1) |
| 028 | `interval("[entire]")` | [−∞, +∞] | exception input_format_error ✗ | [−∞, +∞] ✓ | read error, iostat 1210 ✗ |  |
| 029 | `interval("3.56?1")` | [3.55, 3.5700000000000003] | exception input_format_error ✗ | [3.55, 3.5700000000000003] ✓ | read error, iostat 1210 ✗ | uncertain form (9.7.4) |

### 3. Division by an interval containing zero

From tests/arithmetic.cpp (divisions_by_zero).

| # | Operation | IEEE 1788 | GAOL | libieeep1788 | Solaris Studio | Notes |
|---|---|---|---|---|---|---|
| 030 | `[1, 2] / [0, 1]` | [1, +∞] | [1, +∞] ✓ | [1, +∞] ✓ | [−∞, +∞] ⊃ |  |
| 031 | `[1, 2] / [−1, 0]` | [−∞, −1] | [−∞, −1] ✓ | [−∞, −1] ✓ | [−∞, +∞] ⊃ |  |
| 032 | `[−2, −1] / [0, 1]` | [−∞, −1] | [−∞, −1] ✓ | [−∞, −1] ✓ | [−∞, +∞] ⊃ |  |
| 033 | `[−2, −1] / [−1, 0]` | [1, +∞] | [1, +∞] ✓ | [1, +∞] ✓ | [−∞, +∞] ⊃ |  |
| 034 | `[1, 2] / [−1, 1]` | [−∞, +∞] | [−∞, +∞] ✓ | [−∞, +∞] ✓ | [−∞, +∞] ✓ |  |
| 035 | `[−1, 2] / [0, 1]` | [−∞, +∞] | [−∞, +∞] ✓ | [−∞, +∞] ✓ | [−∞, +∞] ✓ |  |
| 036 | `[0, 2] / [0, 1]` | [0, +∞] | [0, +∞] ✓ | [−0, +∞] ✓ | [−∞, +∞] ⊃ |  |
| 037 | `[−2, 0] / [0, 1]` | [−∞, 0] | [−∞, 0] ✓ | [−∞, 0] ✓ | [−∞, +∞] ⊃ |  |
| 038 | `[0] / [−1, 1]` | [0] | [−0] ✓ | [−0] ✓ | [−∞, +∞] ⊃ |  |
| 039 | `[1, 2] / [0]` | ∅ | ∅ ✓ | ∅ ✓ | [−∞, +∞] ✗ |  |
| 040 | `[0] / [0]` | ∅ | ∅ ✓ | ∅ ✓ | [−∞, +∞] ✗ |  |
| 041 | `inverse([0, 1])` | [1, +∞] | [1, +∞] ✓ | [1, +∞] ✓ | [−∞, +∞] ⊃ | recip in IEEE 1788, 1/x in Solaris Studio |
| 042 | `inverse([0])` | ∅ | ∅ ✓ | ∅ ✓ | [−∞, +∞] ✗ |  |
| 043 | `inverse([−1, 1])` | [−∞, +∞] | [−∞, +∞] ✓ | [−∞, +∞] ✓ | [−∞, +∞] ✓ |  |

### 4. Relational division (GAOL's %, mulRev of IEEE 1788)

From tests/arithmetic.cpp (divisions_by_zero).

| # | Operation | IEEE 1788 | GAOL | libieeep1788 | Solaris Studio | Notes |
|---|---|---|---|---|---|---|
| 044 | `[1, 2] % [0, 1]` | [1, +∞] | [1, +∞] ✓ | [1, +∞] ✓ | n/a | mul_rev(y, x) in libieeep1788 |
| 045 | `[1, 2] % [−1, 0]` | [−∞, −1] | [−∞, −1] ✓ | [−∞, −1] ✓ | n/a |  |
| 046 | `[1, 2] % [−1, 1]` | [−∞, +∞] | [−∞, +∞] ✓ | [−∞, +∞] ✓ | n/a |  |
| 047 | `[0] % [−1, 1]` | [−∞, +∞] | [−∞, +∞] ✓ | [−∞, +∞] ✓ | n/a |  |
| 048 | `[−1, 2] % [0]` | [−∞, +∞] | [−∞, +∞] ✓ | [−∞, +∞] ✓ | n/a |  |
| 049 | `[1, 2] % [0]` | ∅ | ∅ ✓ | ∅ ✓ | n/a |  |

### 5. Products with zero and infinite bounds

From tests/arithmetic.cpp (products_with_infinite_bounds).

| # | Operation | IEEE 1788 | GAOL | libieeep1788 | Solaris Studio | Notes |
|---|---|---|---|---|---|---|
| 050 | `[0, 1] * [1, +∞]` | [0, +∞] | [0, +∞] ✓ | [−0, +∞] ✓ | [−∞, +∞] ⊃ |  |
| 051 | `[−1, 0] * [1, +∞]` | [−∞, 0] | [−∞, 0] ✓ | [−∞, 0] ✓ | [−∞, +∞] ⊃ |  |
| 052 | `[0] * [1, +∞]` | [0] | [0] ✓ | [−0] ✓ | [−∞, +∞] ⊃ |  |
| 053 | `[0] * [−∞, +∞]` | [0] | [−0] ✓ | [−0] ✓ | [−∞, +∞] ⊃ |  |
| 054 | `[0, 1] * [−∞, +∞]` | [−∞, +∞] | [−∞, +∞] ✓ | [−∞, +∞] ✓ | [−∞, +∞] ✓ |  |
| 055 | `[1, +∞] * [1, +∞]` | [1, +∞] | [1, +∞] ✓ | [1, +∞] ✓ | [1, +∞] ✓ |  |
| 056 | `[−∞, −1] * [1, +∞]` | [−∞, −1] | [−∞, −1] ✓ | [−∞, −1] ✓ | [−∞, −1] ✓ |  |
| 057 | `[−∞, 0] * [−∞, 0]` | [0, +∞] | [−0, +∞] ✓ | [−0, +∞] ✓ | [−∞, +∞] ⊃ |  |
| 058 | `[−∞, +∞] * [−∞, +∞]` | [−∞, +∞] | [−∞, +∞] ✓ | [−∞, +∞] ✓ | [−∞, +∞] ✓ |  |
| 059 | `[1, 2] * ∅` | ∅ | ∅ ✓ | ∅ ✓ | ∅ ✓ |  |
| 060 | `[0, 1] * interval(+∞)` | ∅ | ∅ ✓ | ∅ ✓ | [−∞, +∞] ✗ | interval(+∞) is empty |

### 6. Operators of an interval and a double (+=, -=, *=, /=)

From tests/arithmetic.cpp (operations_with_special_doubles).

| # | Operation | IEEE 1788 | GAOL | libieeep1788 | Solaris Studio | Notes |
|---|---|---|---|---|---|---|
| 061 | `x = [1, 2]; x += +∞` | ∅ | ∅ ✓ | ∅ ✓ | [MAX, +∞] ✗ | libieeep1788 has neither mixed operators nor op=: x op II(d, d); Solaris Studio: x = x op d |
| 062 | `x = [1, 2]; x -= +∞` | ∅ | ∅ ✓ | ∅ ✓ | [−∞, −pred(MAX)] ✗ |  |
| 063 | `x = [1, 2]; x *= +∞` | ∅ | ∅ ✓ | ∅ ✓ | [MAX, +∞] ✗ |  |
| 064 | `x = [1, 2]; x /= +∞` | ∅ | ∅ ✓ | ∅ ✓ | [0, 1.112536929253601e−308] ✗ |  |
| 065 | `x = [0]; x *= +∞` | ∅ | ∅ ✓ | ∅ ✓ | [−∞, +∞] ✗ |  |
| 066 | `x = [0, 1]; x *= −∞` | ∅ | ∅ ✓ | ∅ ✓ | [−∞, +∞] ✗ |  |
| 067 | `x = [1, 2]; x += NaN` | ∅ | ∅ ✓ | ∅ ✓ | [−∞, +∞] ✗ |  |
| 068 | `x = [1, 2]; x /= 0` | ∅ | ∅ ✓ | ∅ ✓ | [−∞, +∞] ✗ |  |
| 069 | `x = [1, 2]; x /= −0` | ∅ | ∅ ✓ | ∅ ✓ | [−∞, +∞] ✗ |  |
| 070 | `x = [−1, 2]; x /= 0` | ∅ | ∅ ✓ | ∅ ✓ | [−∞, +∞] ✗ |  |
| 071 | `x = [1, 2]; x %= 0` | ∅ | ∅ ✓ | ∅ ✓ | n/a | GAOL's %=, mul_rev in libieeep1788 |
| 072 | `x = [1, 2]; x += −0` | [1, 2] | [1, 2] ✓ | [1, 2] ✓ | [1, 2] ✓ |  |
| 073 | `x = [1, 2]; x *= −0` | [0] | [−0] ✓ | [−0] ✓ | [−0] ✓ |  |
| 074 | `x = [1, +∞]; x *= 0` | [0] | [−0] ✓ | [−0] ✓ | [−∞, +∞] ⊃ |  |
| 075 | `x = [−∞, +∞]; x *= 0` | [0] | [−0] ✓ | [−0] ✓ | [−∞, +∞] ⊃ |  |
| 076 | `x = [1, +∞]; x -= MAX` | [−MAX, +∞] | [−MAX, +∞] ✓ | [−MAX, +∞] ✓ | [−MAX, +∞] ✓ |  |
| 077 | `x = [MAX]; x += MAX` | [MAX, +∞] | [MAX, +∞] ✓ | [MAX, +∞] ✓ | [MAX, +∞] ✓ |  |
| 078 | `x = ∅; x += 1` | ∅ | ∅ ✓ | ∅ ✓ | ∅ ✓ |  |
| 079 | `x = [1, 2]; x += interval(+∞)` | ∅ | ∅ ✓ | ∅ ✓ | [MAX, +∞] ✗ |  |
| 080 | `[1, 2] + +∞` | ∅ | ∅ ✓ | ∅ ✓ | [MAX, +∞] ✗ |  |
| 081 | `[0, 1] * +∞` | ∅ | ∅ ✓ | ∅ ✓ | [−∞, +∞] ✗ |  |
| 082 | `0 * [1, +∞]` | [0] | [0] ✓ | [−0] ✓ | [−∞, +∞] ⊃ |  |
| 083 | `[1, 2] / 0` | ∅ | ∅ ✓ | ∅ ✓ | [−∞, +∞] ✗ |  |

### 7. Elementary functions at the edges of their domains

From tests/elementary.cpp (at_known_intervals), check/.

| # | Operation | IEEE 1788 | GAOL | libieeep1788 | Solaris Studio | Notes |
|---|---|---|---|---|---|---|
| 084 | `sin([1, 2])` | [0.8414709848078965, 1] | [0.8414709848078963, 1] ⊃ | [0.8414709848078965, 1] ✓ | [0.8414709848078964, 1] ⊃ |  |
| 085 | `sin([4, 5])` | [−1, −0.7568024953079282] | [−1, −0.7568024953079279] ⊃ | [−1, −0.7568024953079282] ✓ | [−1, −0.7568024953079282] ✓ |  |
| 086 | `sin([0, 7])` | [−1, 1] | [−1, 1] ✓ | [−1, 1] ✓ | [−1, 1] ✓ |  |
| 087 | `sin([−∞, +∞])` | [−1, 1] | [−1, 1] ✓ | [−1, 1] ✓ | [−1, 1] ✓ |  |
| 088 | `cos([3, 4])` | [−1, −0.6536436208636118] | [−1, −0.6536436208636118] ✓ | [−1, −0.6536436208636118] ✓ | [−1, −0.6536436208636118] ✓ |  |
| 089 | `cos([−1, 1])` | [0.5403023058681397, 1] | [0.5403023058681397, 1] ✓ | [0.5403023058681397, 1] ✓ | [0.5403023058681397, 1] ✓ |  |
| 090 | `cos([0, 7])` | [−1, 1] | [−1, 1] ✓ | [−1, 1] ✓ | [−1, 1] ✓ |  |
| 091 | `cos([2^52−1])` | [0.473292885954309, 0.4732928859543091] | [0.473292885954309, 0.47329288595430913] ⊃ | [0.473292885954309, 0.4732928859543091] ✓ | [0.473292885954309, 0.4732928859543091] ✓ | a wrong mathlib gives −0.4855 (see manual) |
| 092 | `tan([1, 2])` | [−∞, +∞] | [−∞, +∞] ✓ | [−∞, +∞] ✓ | [−∞, +∞] ✓ |  |
| 093 | `tan([−1, 1])` | [−1.5574077246549023, 1.5574077246549023] | [−1.5574077246549025, 1.5574077246549025] ⊃ | [−1.5574077246549023, 1.5574077246549023] ✓ | [−1.5574077246549023, 1.5574077246549023] ✓ |  |
| 094 | `tan([−∞, +∞])` | [−∞, +∞] | [−∞, +∞] ✓ | [−∞, +∞] ✓ | [−∞, +∞] ✓ |  |
| 095 | `log([−1, 1])` | [−∞, 0] | [−∞, 2^-1074] ⊃ | [−∞, 0] ✓ | [−∞, 0] ✓ |  |
| 096 | `log([0, 1])` | [−∞, 0] | [−∞, 2^-1074] ⊃ | [−∞, 0] ✓ | [−∞, 0] ✓ |  |
| 097 | `log([−4, 0])` | ∅ | [−∞, −MAX] ✗ | ∅ ✓ | [−∞, −MAX] ✗ | no x > 0 in [−4, 0]: GAOL and Solaris Studio take log(0) = −∞ |
| 098 | `log([0])` | ∅ | [−∞, −MAX] ✗ | ∅ ✓ | [−∞, −MAX] ✗ |  |
| 099 | `log([−2, −1])` | ∅ | ∅ ✓ | ∅ ✓ | ∅ ✓ |  |
| 100 | `log([1, +∞])` | [0, +∞] | [−2^-1074, +∞] ⊃ | [−0, +∞] ✓ | [−0, +∞] ✓ |  |
| 101 | `exp([−∞, 0])` | [0, 1] | [0, 1.0000000000000002] ⊃ | [−0, 1] ✓ | [0, 1] ✓ |  |
| 102 | `exp([740])` | [MAX, +∞] | [MAX, +∞] ✓ | [MAX, +∞] ✓ | [MAX, +∞] ✓ |  |
| 103 | `exp([−800])` | [0, 2^-1074] | [0, 2^-1074] ✓ | [−0, 2^-1074] ✓ | [0, 2^-1074] ✓ |  |
| 104 | `exp([−∞, +∞])` | [0, +∞] | [0, +∞] ✓ | [−0, +∞] ✓ | [0, +∞] ✓ |  |
| 105 | `exp(∅)` | ∅ | ∅ ✓ | ∅ ✓ | ∅ ✓ |  |
| 106 | `sqrt([−4, 4])` | [0, 2] | [0, 2] ✓ | [−0, 2] ✓ | [−0, 2] ✓ |  |
| 107 | `sqrt([−4, −1])` | ∅ | ∅ ✓ | ∅ ✓ | ∅ ✓ |  |
| 108 | `sqrt([−∞, +∞])` | [0, +∞] | [0, +∞] ✓ | [−0, +∞] ✓ | [−0, +∞] ✓ |  |
| 109 | `sqr([−∞, +∞])` | [0, +∞] | [−0, +∞] ✓ | [−0, +∞] ✓ | [−0, +∞] ✓ | x**2 in Solaris Studio |
| 110 | `sqr([−∞, −MAX])` | [MAX, +∞] | [MAX, +∞] ✓ | [MAX, +∞] ✓ | [MAX, +∞] ✓ |  |
| 111 | `asin([−2, 2])` | [−1.5707963267948968, 1.5707963267948968] | [−1.5707963267948968, 1.5707963267948968] ✓ | [−1.5707963267948968, 1.5707963267948968] ✓ | [−1.5707963267948968, 1.5707963267948968] ✓ |  |
| 112 | `asin([2, 3])` | ∅ | ∅ ✓ | ∅ ✓ | ∅ ✓ |  |
| 113 | `acos([−2, 2])` | [0, 3.1415926535897936] | [−2^-1074, 3.1415926535897936] ⊃ | [−0, 3.1415926535897936] ✓ | [−0, 3.1415926535897936] ✓ |  |
| 114 | `acos([1, 3])` | [0] | [−2^-1074, 2^-1074] ⊃ | [−0] ✓ | [−0] ✓ |  |
| 115 | `acos([−3, −1])` | [3.141592653589793, 3.1415926535897936] | [3.1415926535897927, 3.1415926535897936] ⊃ | [3.141592653589793, 3.1415926535897936] ✓ | [3.141592653589793, 3.1415926535897936] ✓ |  |
| 116 | `acos([−3, −2])` | ∅ | ∅ ✓ | ∅ ✓ | ∅ ✓ |  |
| 117 | `atan([−∞, +∞])` | [−1.5707963267948968, 1.5707963267948968] | [−1.5707963267948968, 1.5707963267948968] ✓ | [−1.5707963267948968, 1.5707963267948968] ✓ | [−1.5707963267948968, 1.5707963267948968] ✓ |  |
| 118 | `sinh([−∞, +∞])` | [−∞, +∞] | [−∞, +∞] ✓ | [−∞, +∞] ✓ | [−∞, +∞] ✓ |  |
| 119 | `cosh([−∞, +∞])` | [1, +∞] | [1, +∞] ✓ | [1, +∞] ✓ | [1, +∞] ✓ |  |
| 120 | `tanh([−∞, +∞])` | [−1, 1] | [−1, 1] ✓ | [−1, 1] ✓ | [−1, 1] ✓ |  |
| 121 | `asinh([−∞, +∞])` | [−∞, +∞] | [−∞, +∞] ✓ | [−∞, +∞] ✓ | n/a | Solaris Studio has no asinh, acosh, atanh |
| 122 | `acosh([0, 1])` | [0] | [−3·2^-1074, 3·2^-1074] ⊃ | [−0] ✓ | n/a |  |
| 123 | `acosh([−1, 0.5])` | ∅ | ∅ ✓ | ∅ ✓ | n/a |  |
| 124 | `atanh([−1, 1])` | [−∞, +∞] | [−∞, +∞] ✓ | [−∞, +∞] ✓ | n/a |  |
| 125 | `atanh([2, 3])` | ∅ | ∅ ✓ | ∅ ✓ | n/a |  |
| 126 | `abs([−∞, −1])` | [1, +∞] | [1, +∞] ✓ | [1, +∞] ✓ | [1, +∞] ✓ |  |
| 127 | `abs([−∞, 1])` | [0, +∞] | [0, +∞] ✓ | [−0, +∞] ✓ | [−0, +∞] ✓ |  |

### 8. Integer powers (GAOL's pow(x, n), pown of IEEE 1788, x**n of Fortran)

From check/non_arithmetic.cpp (test_pow_int), tests/arithmetic.cpp.

| # | Operation | IEEE 1788 | GAOL | libieeep1788 | Solaris Studio | Notes |
|---|---|---|---|---|---|---|
| 128 | `pow([0], 0)` | [1] | [1] ✓ | [1] ✓ | [1] ✓ | pown(x, 0) = 1 for every x (Table 9.1, b) |
| 129 | `pow(∅, 0)` | ∅ | ∅ ✓ | ∅ ✓ | ∅ ✓ |  |
| 130 | `pow([−∞, +∞], 0)` | [1] | [1] ✓ | [1] ✓ | [1] ✓ |  |
| 131 | `pow([−3, 5], 2)` | [0, 25] | [−0, 25] ✓ | [−0, 25] ✓ | [−0, 25] ✓ |  |
| 132 | `pow([−2, 3], 3)` | [−8, 27] | [−8, 27] ✓ | [−8, 27] ✓ | [−8.000000000000002, 27.000000000000004] ⊃ |  |
| 133 | `pow([−3, 2], −1)` | [−∞, +∞] | [−∞, +∞] ✓ | [−∞, +∞] ✓ | [−∞, +∞] ✓ |  |
| 134 | `pow([−3, 2], −2)` | [0.1111111111111111, +∞] | [0.1111111111111111, +∞] ✓ | [0.1111111111111111, +∞] ✓ | [0.11111111111111109, +∞] ⊃ |  |
| 135 | `pow([−3, 2], −3)` | [−∞, +∞] | [−∞, +∞] ✓ | [−∞, +∞] ✓ | [−∞, +∞] ✓ |  |
| 136 | `pow([−3, −2], −3)` | [−0.125, −0.037037037037037035] | [−0.125, −0.037037037037037035] ✓ | [−0.125, −0.037037037037037035] ✓ | [−0.12500000000000003, −0.03703703703703703] ⊃ |  |
| 137 | `pow([2, 3], −4)` | [0.012345679012345678, 0.0625] | [0.012345679012345678, 0.0625] ✓ | [0.012345679012345678, 0.0625] ✓ | [0.012345679012345677, 0.06250000000000001] ⊃ |  |
| 138 | `pow([0], −1)` | ∅ | ∅ ✓ | ∅ ✓ | [−∞, +∞] ✗ | pown(0, p) has no value for p < 0 |
| 139 | `pow([0, 2], −1)` | [0.5, +∞] | [0.5, +∞] ✓ | [0.5, +∞] ✓ | [−∞, +∞] ⊃ |  |
| 140 | `pow([−2, 0], −1)` | [−∞, −0.5] | [−∞, −0.5] ✓ | [−∞, −0.5] ✓ | [−∞, +∞] ⊃ |  |
| 141 | `pow([−2, 0], −2)` | [0.25, +∞] | [0.25, +∞] ✓ | [0.25, +∞] ✓ | [0.24999999999999997, +∞] ⊃ |  |
| 142 | `pow([0, +∞], −2)` | [0, +∞] | [0, +∞] ✓ | [−0, +∞] ✓ | [−0, +∞] ✓ |  |
| 143 | `pow([−∞, −MAX], 3)` | [−∞, −MAX] | [−∞, −MAX] ✓ | [−∞, −MAX] ✓ | [−∞, −MAX] ✓ |  |
| 144 | `pow([−∞, −MAX], 4)` | [MAX, +∞] | [MAX, +∞] ✓ | [MAX, +∞] ✓ | [MAX, +∞] ✓ |  |
| 145 | `pow([MAX, +∞], 3)` | [MAX, +∞] | [MAX, +∞] ✓ | [MAX, +∞] ✓ | [MAX, +∞] ✓ |  |
| 146 | `pow([−15], 17)` | [−(9.852612533569336e+19), −(9.852612533569334e+19)] | [−(9.852612533569338e+19), −(9.852612533569334e+19)] ⊃ | [−(9.852612533569336e+19), −(9.852612533569334e+19)] ✓ | [−(9.852612533569338e+19), −(9.852612533569334e+19)] ⊃ |  |
| 147 | `pow([10], 400)` | [MAX, +∞] | [MAX, +∞] ✓ | [MAX, +∞] ✓ | [MAX, +∞] ✓ |  |
| 148 | `pow([10], −400)` | [0, 2^-1074] | [0, 5.56268464626801e−309] ⊃ | [−0, 2^-1074] ✓ | [−0, 2^-1074] ✓ |  |

### 9. Real powers (GAOL's pow(x, d) and pow(x, y), pow of IEEE 1788, x**y of Fortran)

From tests/elementary.cpp (powers).

| # | Operation | IEEE 1788 | GAOL | libieeep1788 | Solaris Studio | Notes |
|---|---|---|---|---|---|---|
| 149 | `pow([4], 0.5)` | [2] | [1.9999999999999996, 2.0000000000000004] ⊃ | [2] ✓ | [2] ✓ | libieeep1788: pow(x, II(d, d)) |
| 150 | `pow([4], 1.5)` | [8] | [7.999999999999994, 8.000000000000004] ⊃ | [8] ✓ | [7.999999999999999, 8.000000000000002] ⊃ |  |
| 151 | `pow([4, 9], 0.5)` | [2, 3] | [1.9999999999999996, 3.0000000000000013] ⊃ | [2, 3] ✓ | [2, 3] ✓ |  |
| 152 | `pow([4], −0.5)` | [0.5] | [0.4999999999999999, 0.5000000000000002] ⊃ | [0.5] ✓ | [0.49999999999999994, 0.5000000000000001] ⊃ |  |
| 153 | `pow([0, 4], 0.5)` | [0, 2] | [0, 2.0000000000000004] ⊃ | [−0, 2] ✓ | [−0, 2] ✓ |  |
| 154 | `pow([−4, 9], 0.5)` | [0, 3] | [0, 3.0000000000000013] ⊃ | [−0, 3] ✓ | [−0, 3] ✓ |  |
| 155 | `pow([−2, 3], [1, 2])` | [0, 9] | [0, 9.000000000000007] ⊃ | [−0, 9] ✓ | [−0, 9] ✓ |  |
| 156 | `pow([−10, 10], −2)` | [0.009999999999999998, +∞] | [0.009999999999999998, +∞] ✓ | [0.009999999999999998, +∞] ✓ | [0.009999999999999998, +∞] ✓ |  |
| 157 | `pow([−2, 3], 3)` | [0, 27] | [−8, 27] ⊃ | [−0, 27] ✓ | [−0, 27.000000000000004] ⊃ | GAOL: pown for an integer exponent (choice of the fork), IEEE 1788's pow: x > 0 only |
| 158 | `pow([−2], 2)` | ∅ | [4] ✗ | ∅ ✓ | ∅ ✓ | GAOL: pown for an integer exponent (choice of the fork), IEEE 1788's pow: x > 0 only |
| 159 | `pow([2, 3], 4)` | [16, 81] | [16, 81] ✓ | [16, 81] ✓ | [15.999999999999998, 81.00000000000001] ⊃ |  |
| 160 | `pow([4], 0)` | [1] | [1] ✓ | [1] ✓ | [1] ✓ |  |
| 161 | `pow([−2, 3], [3])` | [0, 27] | [−8, 27] ⊃ | [−0, 27] ✓ | [−0, 27.000000000000004] ⊃ | GAOL: pown for an integer exponent (choice of the fork), IEEE 1788's pow: x > 0 only |
| 162 | `pow([−2, 3], [2])` | [0, 9] | [−0, 9] ✓ | [−0, 9] ✓ | [−0, 9] ✓ |  |
| 163 | `pow([3, 4], [2, 3])` | [9, 64] | [8.999999999999996, 64.00000000000004] ⊃ | [9, 64] ✓ | [9, 64.00000000000001] ⊃ |  |
| 164 | `pow([−4, −1], 0.5)` | ∅ | ∅ ✓ | ∅ ✓ | ∅ ✓ |  |
| 165 | `pow([−4, −1], [0.5])` | ∅ | ∅ ✓ | ∅ ✓ | ∅ ✓ |  |
| 166 | `pow([4], +∞)` | ∅ | ∅ ✓ | ∅ ✓ | [MAX, +∞] ✗ | the exponent interval(+∞) is empty |
| 167 | `pow([4], −∞)` | ∅ | ∅ ✓ | ∅ ✓ | [−0, 2^-1074] ✗ |  |
| 168 | `pow([4], NaN)` | ∅ | ∅ ✓ | ∅ ✓ | [−0, +∞] ✗ |  |
| 169 | `pow([4], interval(+∞))` | ∅ | ∅ ✓ | ∅ ✓ | [MAX, +∞] ✗ |  |
| 170 | `pow([4], interval(−∞))` | ∅ | ∅ ✓ | ∅ ✓ | [−0, 2^-1074] ✗ |  |
| 171 | `pow(∅, 1.5)` | ∅ | ∅ ✓ | ∅ ✓ | ∅ ✓ |  |
| 172 | `pow([4], ∅)` | ∅ | ∅ ✓ | ∅ ✓ | ∅ ✓ |  |
| 173 | `pow([−2, −1], [1e10])` | ∅ | [−∞, +∞] ✗ | ∅ ✓ | ∅ ✓ | GAOL: pown, [−∞, +∞] for an integer beyond the ints |
| 174 | `pow([2, 3], [2^31])` | [MAX, +∞] | [−∞, +∞] ⊃ | [MAX, +∞] ✓ | [MAX, +∞] ✓ | GAOL: [−∞, +∞] for an integer beyond the ints |
| 175 | `pow([0.25, 0.5], [−(2^31+1)])` | [MAX, +∞] | [−∞, +∞] ⊃ | [MAX, +∞] ✓ | [MAX, +∞] ✓ |  |
| 176 | `pow([−2, 3], 1e10)` | [0, +∞] | [−∞, +∞] ⊃ | [−0, +∞] ✓ | [−0, +∞] ✓ |  |
| 177 | `pow(∅, [1e10])` | ∅ | ∅ ✓ | ∅ ✓ | ∅ ✓ |  |
| 178 | `pow([−1], [2^31−1])` | ∅ | [−1] ✗ | ∅ ✓ | ∅ ✓ | GAOL: pown for an integer exponent (choice of the fork), IEEE 1788's pow: x > 0 only |
| 179 | `pow([0], [0])` | ∅ | [1] ✗ | ∅ ✓ | [−0, +∞] ✗ | pow(0, y) has no value for y ≤ 0 (Table 9.1, c); GAOL: pown(0, 0) = 1 |
| 180 | `pow([0], 0)` | ∅ | [1] ✗ | ∅ ✓ | [−0, +∞] ✗ |  |
| 181 | `pow([−∞, +∞], [0])` | [1] | [1] ✓ | [1] ✓ | [−0, +∞] ⊃ |  |
| 182 | `pow([0, 2], [−1])` | [0.5, +∞] | [0.5, +∞] ✓ | [0.5, +∞] ✓ | [0.5, +∞] ✓ |  |
| 183 | `pow([−2, 0], [−1])` | ∅ | [−∞, −0.5] ✗ | ∅ ✓ | [+∞] ✗ | GAOL: pown for an integer exponent (choice of the fork), IEEE 1788's pow: x > 0 only |
| 184 | `pow([0], [0.5])` | [0] | [−0] ✓ | [−0] ✓ | [−0] ✓ | pow(0, y) = 0 for y > 0 |
| 185 | `pow([0], 0.5)` | [0] | [−0] ✓ | [−0] ✓ | [−0] ✓ |  |
| 186 | `pow([−2, 0], [2.5])` | [0] | [−0] ✓ | [−0] ✓ | [−0] ✓ |  |
| 187 | `pow([0], [0, 1])` | [0] | [−0] ✓ | [−0] ✓ | [−0, +∞] ⊃ |  |
| 188 | `pow([0], [−1, 1])` | [0] | [−0] ✓ | [−0] ✓ | [−0, +∞] ⊃ |  |
| 189 | `pow([0], [−1])` | ∅ | ∅ ✓ | ∅ ✓ | [+∞] ✗ |  |
| 190 | `pow([0], [−0.5])` | ∅ | ∅ ✓ | ∅ ✓ | [MAX, +∞] ✗ |  |
| 191 | `pow([0], −0.5)` | ∅ | ∅ ✓ | ∅ ✓ | [MAX, +∞] ✗ |  |
| 192 | `pow([−2, 0], [−0.5])` | ∅ | ∅ ✓ | ∅ ✓ | [MAX, +∞] ✗ |  |
| 193 | `pow([0], [−1, 0])` | ∅ | ∅ ✓ | ∅ ✓ | [−0, +∞] ✗ |  |
| 194 | `pow([0.5], [−∞, +∞])` | [0, +∞] | [0, +∞] ✓ | [−0, +∞] ✓ | [−0, +∞] ✓ |  |
| 195 | `pow([1], [−∞, +∞])` | [1] | [0, +∞] ⊃ | [1] ✓ | [−0, +∞] ⊃ |  |
| 196 | `pow([0, 1], [1, +∞])` | [0, 1] | [0, +∞] ⊃ | [−0, 1] ✓ | [−0, +∞] ⊃ |  |
| 197 | `pow([2, +∞], [−1, 1])` | [0, +∞] | [0, +∞] ✓ | [−0, +∞] ✓ | [−0, +∞] ✓ |  |

### 10. n-th roots (GAOL's nth_root, rootn of IEEE 1788)

From check/non_arithmetic.cpp, tests/arithmetic.cpp.

| # | Operation | IEEE 1788 | GAOL | libieeep1788 | Solaris Studio | Notes |
|---|---|---|---|---|---|---|
| 198 | `nth_root([−8, 27], 3)` | [−2, 3] | [−2^-1074, 3.000000000000001] ✗ | n/a | n/a | neither libieeep1788 nor Solaris Studio has rootn; GAOL takes the roots of x ∩ [0, +∞] for every n (check/non_arithmetic.cpp) |
| 199 | `nth_root([−4, 9], 2)` | [0, 3] | [0, 3] ✓ | n/a | n/a |  |
| 200 | `nth_root([−4, −1], 2)` | ∅ | ∅ ✓ | n/a | n/a |  |
| 201 | `nth_root([−∞, +∞], 0)` | — | ∅ | n/a | n/a | rootn(x, q) is for q ≠ 0 only (Table 10.5) |
| 202 | `nth_root([0, +∞], 2)` | [0, +∞] | [0, +∞] ✓ | n/a | n/a |  |

### 11. Numeric and set functions

From tests/other_functions.cpp, tests/numbers.cpp.

| # | Operation | IEEE 1788 | GAOL | libieeep1788 | Solaris Studio | Notes |
|---|---|---|---|---|---|---|
| 203 | `[−∞, +∞].midpoint()` | 0 | 0 ✓ | 0 ✓ | 0 ✓ | mid (12.12.8) |
| 204 | `[−∞, 1].midpoint()` | −MAX | −MAX ✓ | −MAX ✓ | −∞ ✗ |  |
| 205 | `[1, +∞].midpoint()` | MAX | MAX ✓ | MAX ✓ | +∞ ✗ |  |
| 206 | `∅.midpoint()` | NaN | NaN ✓ | NaN ✓ | NaN ✓ |  |
| 207 | `[MAX/2, MAX].midpoint()` | 1.3482698511467367e+308 | 1.3482698511467367e+308 ✓ | 1.3482698511467367e+308 ✓ | 1.3482698511467367e+308 ✓ |  |
| 208 | `[0, 2^-1074].midpoint()` | 0 | 0 ✓ | 0 ✓ | 0 ✓ | the tie rounded to even |
| 209 | `[−2^-1074, 4·2^-1074].midpoint()` | 2·2^-1074 | 2·2^-1074 ✓ | 2·2^-1074 ✓ | 2·2^-1074 ✓ | 1.5·2^-1074, to even |
| 210 | `[−∞, 1].width()` | +∞ | +∞ ✓ | +∞ ✓ | +∞ ✓ | wid (12.12.8) |
| 211 | `[−MAX, MAX].width()` | +∞ | +∞ ✓ | +∞ ✓ | +∞ ✓ |  |
| 212 | `∅.width()` | NaN | −1 ✗ | NaN ✓ | NaN ✓ | GAOL returns −1 for the empty set |
| 213 | `[−3, 2].mag()` | 3 | 3 ✓ | 3 ✓ | 3 ✓ |  |
| 214 | `∅.mag()` | NaN | NaN ✓ | NaN ✓ | NaN ✓ |  |
| 215 | `[−3, 2].mig()` | 0 | 0 ✓ | 0 ✓ | 0 ✓ |  |
| 216 | `[−3, −2].mig()` | 2 | 2 ✓ | 2 ✓ | 2 ✓ |  |
| 217 | `∅.mig()` | NaN | NaN ✓ | NaN ✓ | NaN ✓ |  |
| 218 | `[1, 2] \| ∅` | [1, 2] | [1, 2] ✓ | [1, 2] ✓ | [1, 2] ✓ | convex_hull in libieeep1788, .ih. in Solaris Studio |
| 219 | `[1, 2] & [3, 4]` | ∅ | ∅ ✓ | ∅ ✓ | ∅ ✓ | intersection in libieeep1788, .ix. in Solaris Studio |
| 220 | `[1, 2] & [2, 3]` | [2] | [2] ✓ | [2] ✓ | [2] ✓ |  |
| 221 | `min([−∞, 1], [0, 2])` | [−∞, 1] | [−∞, 1] ✓ | [−∞, 1] ✓ | [−∞, 1] ✓ |  |
| 222 | `max([1, 2], ∅)` | ∅ | ∅ ✓ | ∅ ✓ | ∅ ✓ |  |
| 223 | `−[−∞, 1]` | [−1, +∞] | [−1, +∞] ✓ | [−1, +∞] ✓ | [−1, +∞] ✓ |  |

<!-- END GENERATED TABLES -->
