<!-- Copyright (c) 2026 ENSTA, France
     Created 2026-09-20 by Jordan NININ -->
# Accuracy of the operations

Part of the documentation of [GAOL v5](../README.md#documentation).

IEEE 1788-2015 requires an implementation to document the tightness of each
of its interval operations, dividing the possible inputs into ranges and
stating the tightness achieved in each (12.10.3). This page does so for
`gaol::interval`, the inf-sup type of GAOL on doubles, from the algorithm of
each operation; the tests of `tests/` check each statement on random and
special arguments, and the last column gives the largest distance they
found.

## Accuracy modes

IEEE 1788-2015 defines three accuracy modes (12.10.1), for an operation f,
its exact value f\*, and an input box x:

- **tightest**: f(x) is the hull, in doubles, of f\*(x): each bound is the
  nearest double on its side of the exact bound;
- **accurate**: f(x) encloses f\*(x), and is within
  nextOut(f<sub>tightest</sub>(nextOut(x))), nextOut moving each bound one
  double outward: roughly, f(x) is no wider than the tightest result for an
  input one double wider, plus one double on each side;
- **valid**: f(x) encloses f\*(x).

It requires the basic operations (`add`, `sub`, `mul`, `div`, `recip`,
`sqr`, `sqrt`, `fma`), the integer functions and `abs`, `min` and `max` to be
tightest; the other elementary functions, the reverse functions and the
recommended functions to be valid, and recommends them to be accurate
(12.10.2).

GAOL bounds its elementary functions with CORE-MATH, which is correctly
rounded in the rounding direction in effect. Computing in the upward rounding
it keeps, GAOL takes the value at a bound as the upper bound, with nothing to
add, and the double below it as the lower one: those are the tightest bounds,
the second one unless the exact value is a double, which the operations below
give exactly where it happens (log(1) = 0, sin(0) = 0, asin(1) = the bounds of
π/2...). Where an algorithm evaluates the function at the bounds of the
interval, the bounds of the interval are therefore the tightest ones.

Before GAOL v5, the value was correctly rounded to nearest and moved one
double outward, which is at most one double beyond the tightest bound, hence
accurate and not tightest. Below, "within k doubles" means that each bound is
at most k doubles beyond the tightest one.

## Conditions

The statements hold for GAOL as the three builds make it: with the CORE-MATH
of `3rd/math-core`, compiled as they do it (see
[3rd/README.md](../3rd/README.md)), and the code including GAOL's
headers compiled with the flags of interval arithmetic (see
[Using GAOL](using.md)); for the SSE2 intervals and the FPU intervals alike;
whatever the rounding direction the calling code left
(`tests/rounding_direction.cpp`). They hold on every architecture and with
every compiler alike, CORE-MATH giving the same bits everywhere: there is no
other mathematical library to build GAOL with (see
[What differs from GAOL](differences.md)). The float intervals `gaol::intervalf` and `gaol::interval2f`,
off by default and unfinished, are not covered.

## Basic operations (Table 9.1): tightest required

| IEEE 1788 | GAOL | Algorithm | Tightness | Tests |
|---|---|---|---|---|
| `neg` | `-x` | Exchange of the bounds | tightest (exact) | exact |
| `add`, `sub` | `x + y`, `x - y`, `+=`, `-=`, and with a double | Sums of the bounds rounded upward, the lower bound being stored negated | tightest | tightest |
| `mul` | `x * y`, `*=`, and with a double | Products of the bounds chosen by their signs, rounded upward; a zero bound times an infinite one counts as 0 | tightest | tightest |
| `div` | `x / y`, `/=`, and with a double | Quotients of the bounds chosen by their signs, rounded upward; the hull of the quotients when y contains 0, the empty set when y is [0] | tightest | tightest |
| `recip` | `inverse(x)`, `x.inverse()` | 1 divided by the bounds, rounded upward | tightest | tightest |
| `sqr` | `sqr(x)` | Squares of the bounds rounded upward, 0 when x contains 0 | tightest | tightest |
| `sqrt` | `sqrt(x)` | On x ∩ [0, +∞]: the square root u rounded upward, from the square root of the C library checked with a product rounded the other way and moved one double when needed; the lower bound is u when u·u = x exactly, the double below u otherwise | tightest | tightest |
| `fma` | `fma(x, y, z)` | x·y ranges over the hull of the products of the bounds, being bilinear, so x·y + z ranges from the least product plus inf z to the greatest plus sup z; each corner is rounded once by `std::fma`, upward for the upper bound and downward for the lower one as −fma(−a, b, −c), the result of `std::fma` going through `rnd_keep()` before it is negated: GCC folded −fma(−a, b, −c) into fma(a, b, c), a single instruction rounded upward, and the lower bound came one double above the exact one. A bound 0 times an infinite one counts as 0 (GAOL v5) | tightest | tightest, over 200 000 boxes against exact rational arithmetic |

## Integer and absmax functions: tightest required

| IEEE 1788 | GAOL | Algorithm | Tightness | Tests |
|---|---|---|---|---|
| `floor`, `ceil` | `floor(x)`, `ceil(x)` | Of each bound | tightest (exact) | exact |
| — | `integer(x)` | The integers in x: [ceil(lower), floor(upper)] | tightest (exact) | exact |
| `sign`, `trunc`, `roundTiesToEven`, `roundTiesToAway` | `sign(x)`, `trunc(x)`, `round_ties_to_even(x)`, `round_ties_to_away(x)` | The value at each bound, each function being non-decreasing, as `ceil` and `floor` above; ∅ for ∅, which the four test: GAOL holds the empty interval as the two bounds NaN, which `trunc` and the two roundings send to themselves, where `sign` would give [0, 0], a NaN comparing false both to 0 and above it. None of them rounds, each returning a double that is an integer: `trunc` and `round_ties_to_away` are `std::trunc` and `std::round`, which the C++ standard defines without reading the rounding direction, and `round_ties_to_even` reads the bits (`gaol/gaol_roundeven.h`), where `std::nearbyint` and `std::rint` would round in the direction in effect, upward in GAOL (GAOL v5) | tightest, and exact | tightest; the same result in the four rounding directions, over 22 407 doubles (halfway values, whole numbers and the doubles on either side of them) |
| `abs` | `abs(x)` | Magnitudes of the bounds, 0 when x contains 0 | tightest (exact) | tightest |
| `min`, `max` | `min(x, y)`, `max(x, y)` | Of the bounds | tightest (exact) | tightest |

## Power, exponential and logarithm functions: valid required, accurate recommended

| IEEE 1788 | GAOL | Algorithm | Tightness | Tests |
|---|---|---|---|---|
| `pown(x, n)` | `pow(x, n)` for an int or an unsigned n, and `pow(x, y)` for a degenerate integer y within the ints (`gaol_pown()`, `gaol_uipow()`) | Binary exponentiation of the bounds from exact products: each product is rounded, and the rest, which `fma` gives exactly, is carried along and bounded, so that the power is rounded outward once at the end (n ≥ 3, and from 2<sup>−968</sup> up to the overflow; elsewhere, and for n = 2, each product rounded outward, as GAOL did for every power). x<sup>−n</sup> is 1/x<sup>n</sup>, or (1/x)<sup>n</sup> where x<sup>n</sup> is beyond the largest double, and [\|x\|<sup>−n</sup>, +∞] or [−∞, +∞] where x contains 0 inside | n = 0, 1, 2: tightest. n ≥ 3: the tightest bounds, or one double beyond where the power is within n·2<sup>−104</sup> of a double; exact where the power is a double; the rounded products, within about 2(n − 1) doubles, below 2<sup>−968</sup> and at the overflow. Negative n: valid, one rounded division more, within 2 doubles where x<sup>n</sup> is normal; tightest where the exact bounds are doubles | tightest for x between 2<sup>−30</sup> and 2<sup>30</sup>; within n doubles for all doubles; −n: within 8 doubles for n ≤ 3, overflows included; tightest on the special cases |
| `pow(x, y)` | `pow(x, y)`, `pow(x, d)` for a non-integer y (`gaol_pow_hybrid()`, `gaol_pow_real()`); `gaol_ieee1788::pow(x, y)` for any y | On x ∩ [0, +∞], with finite bounds: CORE-MATH's pow at the corners of the box x × y where x<sup>y</sup> is the least and the greatest, which the places of the bounds about 1 and 0 give, correctly rounded upward; 1 where a corner has the base 1 or the exponent 0; [0, ·] for a base from 0 and exponents above 0. With an infinite bound, or a base from 0 and an exponent that is not above 0: exp(y·log(x)), which gives the limits; 0<sup>y</sup> = 0 for y > 0, ∅ for y ≤ 0. An integer y beyond the ints, in `gaol_ieee1788::pow`: CORE-MATH's pow at the bounds of x, x<sup>y</sup> being monotone in x, where the `pow` of `gaol` gives [−∞, +∞] | tightest at the corners with finite bounds, and at the bounds for an integer y beyond the ints; valid otherwise | within 1 double at points and over 385 boxes; 4 doubles on the special cases; tightest at 5 integer exponents beyond the ints |
| `exp` | `exp(x)` | CORE-MATH's exp at the bounds, correctly rounded upward: the value at the right bound, and the double below the value at the left bound; the lower bound at least 0; exp(0) = 1 exactly | tightest where the function is evaluated at the bounds | tightest, for all doubles |
| `log` | `log(x)` | CORE-MATH's log at the bounds of x ∩ [0, +∞], correctly rounded upward: the value at the right bound, and the double below the value at the left bound; ∅ when x holds no positive number; log(0) = −∞, log(1) = 0 exactly | tightest | tightest at the 75 doubles of the tests and the million intervals of doc/compare |
| `exp2`, `exp10` | `exp2(x)`, `exp10(x)` | CORE-MATH's exp2 and exp10 at the bounds, correctly rounded upward: the value at the right bound, and the double below the value at the left bound unless that value is a double, which 2<sup>x</sup> is for an integer x of [−1074, 1023] and 10<sup>x</sup> for an integer x of [0, 22] (10<sup>23</sup> is no double, and no negative power of ten is); the lower bound at least 0 (GAOL v5) | tightest | tightest, over 6 483 doubles each, and at the values that are doubles |
| `log2`, `log10` | `log2(x)`, `log10(x)` | CORE-MATH's log2 and log10 at the bounds of x ∩ [0, +∞], correctly rounded upward, the double below the value at the left bound unless that value is a double, which log<sub>2</sub>x is for a power of two and log<sub>10</sub>x for a power of ten of [10<sup>0</sup>, 10<sup>22</sup>]; ∅ when x holds no positive number (GAOL v5) | tightest | tightest, over 6 483 doubles each, and at the values that are doubles |

## Trigonometric and hyperbolic functions: valid required, accurate recommended

| IEEE 1788 | GAOL | Algorithm | Tightness | Tests |
|---|---|---|---|---|
| `cos` | `cos(x)` | The bounds divided by an enclosure of π, rounded outward, tell whether x lies on one piece where cos is monotonic; rounded inward, whether an extremum is within x for sure. Where they cannot tell, a bound being within about \|x\|·2<sup>−52</sup> of a multiple of π, or beyond 2<sup>52</sup>, the signs of CORE-MATH's sin at the bounds do, exactly, x being narrower than 2π (and the sign at the middle of x from π on): signs that differ show one extremum, −1 or 1 according to their order. Then CORE-MATH's cos at the bounds, correctly rounded upward (the double below the value at the left bound), within [−1, 1]; cos(0) = 1 exactly | tightest where the function is evaluated at the bounds, at every magnitude: −1 and 1 exactly where x holds a multiple of π. [−1, 1] from the width 2·π̲ on, π̲ being the double below π: tightest but for the widths between 2·π̲ and 2π | tightest, for all doubles, and over intervals next to the extrema and of width about π and 2π |
| `sin` | `sin(x)` | As cos, the bounds divided by an enclosure of π, minus 1/2, and the signs of CORE-MATH's cos where the quotients cannot tell; CORE-MATH's sin at the bounds, correctly rounded upward, within [−1, 1]; sin(0) = 0 exactly | accurate, at every magnitude, as cos | as cos |
| `tan` | `tan(x)` | [−∞, +∞] when x is wider than π or holds a pole: (x + [π/2])/[π] at the bounds, rounded outward, tells that no pole is within x when the integer parts agree, and rounded inward that one is for sure; where they cannot tell, the signs of CORE-MATH's cos at the bounds do, exactly, a pole being within x when they differ. Otherwise CORE-MATH's tan at the bounds, correctly rounded upward; tan(0) = 0 exactly | tightest between two poles, at every magnitude, [−∞, +∞] exactly when a pole is within x, but for the widths between π̲ and π | tightest, for all doubles, and over intervals next to the poles |
| `asin`, `acos` | `asin(x)`, `acos(x)` | CORE-MATH's asin and acos at the bounds of x ∩ [−1, 1], correctly rounded upward; at −1, 0 and 1, 0 exactly or the tightest bounds of ±π/2 and π | tightest; and exactly 0 or the bounds of ±π/2 and π at −1, 0 and 1 | within 1 double; tightest at −1, 0 and 1 |
| `atan` | `atan(x)` | CORE-MATH's atan at the bounds, correctly rounded upward; at 0, ±1 and ±∞, 0 exactly or the tightest bounds of ±π/4 and ±π/2 | tightest; and exactly 0 or the bounds of ±π/4 and ±π/2 at 0, ±1 and ±∞ | within 1 double; tightest at 0, ±1 and ±∞ |
| `atan2` | `atan2(y, x)` | CORE-MATH's atan2 at the corners of the box y × x where the angle is the least and the greatest, which the signs of the bounds give, correctly rounded upward; 0, ±π/4, ±π/2 and π where a corner is on an axis, on a diagonal of the right half-plane or at an infinity; [−π, π] when the box has points on the half-line y = 0, x < 0, where the angle is π, and points below it; ∅ for the box {(0, 0)} | tightest at the corners; and exactly 0, ±π/4, ±π/2 or ±π where a bound is one of them | within 1 double; tightest on the special cases |
| `sinh`, `tanh`, `asinh` | `sinh(x)`, `tanh(x)`, `asinh(x)` | CORE-MATH's functions at the bounds, correctly rounded upward; tanh within [−1, 1]. 0 at 0 exactly; sinh(x) beyond ±MAX for \|x\| ≥ 711, and tanh(x) beyond ±(1 − 2<sup>−53</sup>) for \|x\| ≥ 20, where 1 − \|tanh(x)\| < 2<sup>−54</sup> | tightest; and exact at 0 and at the bounds given | within 1 double; tightest at 0, and beyond 711 and 20 |
| `cosh` | `cosh(x)` | As sinh, at the bound of largest magnitude; 1 when x contains 0, and at 0; beyond MAX for \|x\| ≥ 711 | tightest; and exact at 0 and beyond 711 | within 1 double; tightest at 0 and beyond 711 |
| `acosh`, `atanh` | `acosh(x)`, `atanh(x)` | As sinh, on x ∩ [1, +∞], respectively x ∩ [−1, 1]; acosh(1) = 0 and atanh(0) = 0 exactly | tightest; and exact at 1, respectively 0 | within 1 double; tightest at 1 and 0 |

Every elementary function above is that of
[CORE-MATH](https://core-math.gitlabpages.inria.fr/) (`3rd/math-core`, see
[3rd/README.md](../3rd/README.md)). Before v5, GAOL took the hyperbolic
functions from the libm of the system, whose values are sometimes further than
one double from the exact ones, and millions of doubles away for the `acosh` of
some ([issue #1](https://github.com/Jordan08/GAOL/issues/1)), and the others
from a library correctly rounded to nearest only, whose values it moved one
double outward.

## Recommended functions (Table 10.5)

| IEEE 1788 | GAOL | Algorithm | Tightness | Tests |
|---|---|---|---|---|
| `rootn(x, n)`, n > 0 | `nth_root(x, n)` | n = 1: x; n = 2: `sqrt`; **n = 3: CORE-MATH's cbrt at the bounds, correctly rounded upward on their magnitudes, the root of a negative number being the opposite of the root of its magnitude: the tightest bounds, and exact where the cube root is a double, which cubing the value tells (GAOL v5; 50 ns rather than 189 with the search below, Intel i7-1185G7, Clang 18.1)**. Otherwise, on x (odd n, the root of a negative number being the opposite of the root of its magnitude) or x ∩ [0, +∞] (even n): the lower bound is the largest double l with l<sup>n</sup> rounded upward at most the bound of x, the upper bound the smallest double u with u<sup>n</sup> rounded downward at least it, which proves them; they are looked for from CORE-MATH's pow with the exponent 1/n rounded, after a step of Newton's method, by steps that double then by bisection, two powers where the start is next to the root; the roots of 0 and 1 are 0 and 1 | n = 1, 2, 3: tightest; tightest where the bound of x is an n-th power of a double. Otherwise accurate: the n − 1 rounded products of l<sup>n</sup> move the root by less than 2<sup>−53</sup> relatively, the bounds being the tightest or one double beyond, whatever the magnitude of x | within 2 doubles, for all doubles (1 found) |
| `rootn(x, q)`, q < 0 | `nth_root(x, q)` | 1/x<sup>1/\|q\|</sup>, the inverse of the root above: the domain is then ℝ∖{0} for an odd q and (0, +∞) for an even one, which taking the inverse gives, an interval holding 0 having for inverse the hull of the values away from it. `nth_root(x, 0)` is ∅ (GAOL v5) | accurate: the root above, then the tightest division | encloses, over 1.5 million values against a reference in long double; the inverse of the positive root, over 8 000 values |
| `expm1`, `exp2m1`, `exp10m1` | `expm1(x)`, `exp2m1(x)`, `exp10m1(x)` | CORE-MATH's functions at the bounds, correctly rounded upward, the double below the value at the left bound unless it is a double, which b<sup>x</sup> − 1 is at 0, and at the integers of [−53, 53] for b = 2 and of [0, 15] for b = 10; within [−1, +∞] (GAOL v5) | tightest | tightest, over 400 000 points and intervals each |
| `sinPi`, `cosPi` | `sinpi(x)`, `cospi(x)` | The extrema are at the multiples of 1/2, which are doubles: doubling the bounds, which is exact, and taking their ceiling and floor give exactly the integers t = 2x within, and sin(πx) has its maximum 1 at the t congruent to 1 modulo 4 and its minimum −1 at those congruent to 3 (cos at 0 and 2); otherwise CORE-MATH's functions at the bounds, exact at the multiples of 1/2. Beyond 2<sup>61</sup> two distinct doubles are whole periods apart, [−1, 1] (GAOL v5) | tightest, at every magnitude: sinPi(10<sup>17</sup>) is 0, where sin(π·x) gave [−1, 1] | tightest, over 400 000 points and intervals each |
| `tanPi` | `tanpi(x)` | As sinPi, its poles at the odd t: [−∞, +∞] with a pole within, −∞ or +∞ for a pole at a bound, and ∅ for a pole alone, where tanPi has no value (GAOL v5) | tightest | tightest, over 400 000 points and intervals |
| `logp1`, `log2p1`, `log10p1` | `log1p(x)`, `log2p1(x)`, `log10p1(x)` | CORE-MATH's functions at the bounds of x ∩ [−1, +∞], the value at −1 being the limit −∞, and x holding no number above −1 giving ∅; the double below the value at the left bound unless it is a double, which log<sub>b</sub>(1 + x) is at 0, at 2<sup>k</sup> − 1 for b = 2 (k in [−53, 53]) and at 10<sup>k</sup> − 1 for b = 10 (k in [0, 15]) only (GAOL v5) | tightest | tightest, over 40 000 intervals each, their bounds drawn among those points and their neighbours |
| `hypot` | `hypot(x, y)` | CORE-MATH's hypot at the point of the box nearest to the origin, (mig x, mig y), and at the farthest, (mag x, mag y). The lower bound is the value itself where it is a double: writing a, b and h = √(a² + b²) as odd integers times powers of two, a² + b² = h² can only hold where h and the smaller of a, b have the same power, and h² − b² = a²·4<sup>d</sup> is then compared exactly on 107 bits (GAOL v5) | tightest | tightest, over 200 000 boxes, among them Pythagorean triples scaled by powers of two and the doubles next to them |
| `rSqrt` | `rsqrt(x)` | CORE-MATH's rsqrt at the bounds of x ∩ [0, +∞], decreasing, the value at 0 being the limit +∞, and x holding no positive number giving ∅; exact at the powers of 4 (GAOL v5) | tightest | tightest, over 40 000 intervals |
| `atanPi`, `asinPi`, `acosPi` | `atanpi(x)`, `asinpi(x)`, `acospi(x)` | CORE-MATH's functions at the bounds, asinPi and acosPi on x ∩ [−1, 1], acosPi decreasing; exact at 0 and ±1 (and ±∞ for atanPi), the only doubles where the values are rational (GAOL v5) | tightest | tightest, over 400 000 points and intervals each (40 000 intervals for asinPi, and the arguments next to ±1 where CORE-MATH's asinpi shifted by 65 bits, see [3rd/README.md](../3rd/README.md)) |
| `atan2Pi` | `atan2pi(y, x)` | The analysis of the box of `atan2`, the angles divided by π: [−1, 1] for a box crossing the half-line y = 0, x < 0, the corners elsewhere. The values at the multiples of 1/4 (0, ±1/4, ±1/2, ±3/4, 1, and −1 as a limit) are exact, CORE-MATH's atan2pi giving the others, which are irrational (GAOL v5) | tightest | tightest, over 200 000 boxes |
| `compoundm1` | — | Not provided: CORE-MATH has no binary64 version | | |

## Reverse functions (Table 10.1): accurate

GAOL's relational functions compute `f_rel(J, I)`, the hull of the x of I
whose image is in J, the reverse functions of IEEE 1788 with the arguments
in another order. Each computes the preimage of J with the functions above,
rounded outward, and intersects it with I: **valid**, which the standard
requires. `tests/reverse.cpp` checks on the cases of the minimal tests of
libieeep1788 and on random ones that they are also **accurate**, which the
standard recommends for inf-sup types, the hulls being computed from the
definition of each function, apart from GAOL (see
[the tests](tests.md)); `tests/other_functions.cpp` checks that they keep
the values they are given, within the number of doubles below.

| IEEE 1788 | GAOL | Tightness | Tests |
|---|---|---|---|
| `sqrRev(c, x)` | `sqrt_rel(c, x)` | accurate | accurate; within 1 double of the value it keeps |
| `absRev(c, x)` | `invabs_rel(c, x)` | tightest: the bounds of J, of −J and of I | tightest |
| `pownRev(c, x, p)`, p > 0 | `nth_root_rel(c, p, x)` | accurate (the roots of `nth_root()`) | accurate, within 1 double of the tightest; within 2 doubles of the value it keeps |
| `sinRev`, `cosRev`, `tanRev` | `asin_rel`, `acos_rel`, `atan_rel` | accurate, or one double beyond: the pieces of the preimage are k·π, enclosed within about one double from π in double-double, plus or minus the inverse function of J, which is added before the sum is rounded; beyond 2<sup>52</sup> a bound of x is kept, and an x of a single double is decided by the image of the function | accurate but for one double, with the bounds of x beyond 2<sup>52</sup>; within 6 doubles (4, 5 and 3 found) of the value they keep from 1 to 2<sup>50</sup>, away from the points where the inverse function magnifies the width of the image |
| `coshRev(c, x)` | `acosh_rel(c, x)` | accurate | accurate; within 16 doubles of the value it keeps |
| `sinhRev`, `tanhRev` (not in Table 10.1, named after `coshRev`) | `asinh_rel`, `atanh_rel` | accurate | within 10 and 26 doubles of the value they keep |
| `mulRev(b, c, x)` | `div_rel(c, b, x)`, and `c % b` for x = [−∞, +∞] | accurate; tightest where the bounds of the quotients are doubles | accurate, the tightest bounds over the cases of `%`; within 2 doubles of the value `div_rel` keeps |
| `powRev1`, `powRev2`, `atan2Rev1`, `atan2Rev2`, `pownRev` for p < 0 | — | | |

Where a bound of J is one double from the image of a bound of the preimage,
these functions keep a point of I whose image is just outside J:
`sqrt_rel([1 + 2^-52, +oo], [-1, 1 + 2^-52])` is `[-1, 1 + 2^-52]`, whose
tightest enclosure is `[1, 1 + 2^-52]`. They are accurate all the same, the
widened J of nextOut holding that image.

`mulRevToPair` (10.5.5) is not provided. `cancelMinus(x, y)` and
`cancelPlus(x, y)` (10.5.6) are `cancel_minus(x, y)` and `cancel_plus(x, y)`,
the tightest, which 12.12.5 requires: whether the result exists depends on x
being at least as wide as y, which rounding the differences of the bounds
cannot decide when they are within a double of each other; each difference
is computed exactly, as the sum of its value rounded to nearest and of its
error by the TwoSum of Knuth. [−∞, +∞] where there is no value, and ∅ for an
empty x and a bounded y. Checked over 200 000 pairs against exact rational
arithmetic, 11 524 of them of exactly equal widths (GAOL v5).

## Set operations, constructors, numeric and boolean functions

| IEEE 1788 | GAOL | Algorithm | Tightness | Tests |
|---|---|---|---|---|
| `intersection`, `convexHull` | `x & y`, `x \| y` | Of the bounds | tightest (exact) | exact |
| `numsToInterval(l, u)` | `interval(l, u)`, `interval(d)` | The bounds; the empty set where l > u, l = +∞, u = −∞ or a bound is NaN | tightest (exact) | exact |
| `textToInterval(s)` | `interval(s)`, `interval(sl, su)`, `operator>>` | Numbers, decimal or hexadecimal, compared exactly with the doubles around them; rational numbers p/q divided as intervals; the uncertain form read as its exact decimal bounds; the literals `[ ]`, `[empty]`, `[entire]`, infinite and missing bounds; any case of letters. GAOL also reads expressions (`"sin(1)+0.1"`), computed with the operations above | tightest for the literals of IEEE 1788, p and q being doubles; valid for expressions | tightest |
| `inf`, `sup` | `left()`, `right()` | The stored bounds; NaN for the empty set, where IEEE 1788 has +∞ and −∞ | exact | exact |
| `mid` | `midpoint()` | (l + u)/2 rounded to nearest, ties to even, or l/2 + u/2 when l + u overflows; 0, −MAX and MAX for unbounded intervals; NaN for the empty set | as 12.12.8 requires | exact |
| — | `mid()` | The same midpoint, rounded outward | tightest | tightest |
| `wid` | `width()` | u − l rounded upward; NaN for the empty set | as 12.12.8 requires | exact |
| `rad`, `midRad` | `rad()`, `mid_rad(m, r)` | The greater of m − l and u − m rounded upward, m being `midpoint()`; +∞ for unbounded intervals, NaN for the empty set | as 12.12.8 requires | exact |
| `mag`, `mig` | `mag()`, `mig()` | Of the bounds | exact | exact |
| — | `smig()`, `hausdorff(x, y)` | Of the bounds; the distance rounded upward | exact; tightest | exact; tightest |
| `isEmpty`, `isSingleton`, `isMember` | `is_empty()`, `is_a_double()`, `set_contains(d)` | Of the bounds | exact | exact |
| `equal`, `subset`, `interior`, `disjoint` | `set_eq`, `set_contains` and `set_leq`, `set_strictly_contains` and `set_le`, `set_disjoint` | Of the bounds, as Tables 10.3 and 10.4 | exact | exact |
| `precedes`, `strictPrecedes` | `certainly_leq`, `certainly_le` (and `certainly_geq`, `certainly_ge`) | Of the bounds, true when an interval is empty | exact | exact |
| `isEntire`, `isCommonInterval` | `is_entire()`, `is_common_interval()` | Of the bounds; a common interval is nonempty and bounded, the empty set's NaN bounds being no infinities (GAOL v5) | exact | exact |
| `less`, `strictLess` | `less(y)`, `strictly_less(y)` | Of the bounds, as Table 10.3, strictLess counting an infinite bound as below itself; the empty cases of Table 10.4 (GAOL v5) | exact | exact, and against Tables 10.3 and 10.4 over 20 000 pairs |
| `intervalToText` | `operator<<` | Hexadecimal format: the bounds in the hexadecimal-significand form of 13.4.1. Decimal formats: each bound written to nearest by the C library with the digits asked for, compared exactly with the bound, and its last digit moved outward when it is on the wrong side | hexadecimal: exact; decimal: valid, and the tightest with the digits asked for where the C library rounds to nearest as it should | hexadecimal: exact; decimal: less than one unit of the last digit |
| `intervalToExact`, `exactToInterval` | `operator<<` with `interval_format::hexa`, `interval(const char*)` | The recovery requirement of 13.4: an interval written in hexadecimal and read again gives the same bounds, bit for bit, the empty set, the infinite bounds, the signed zeros and the subnormals included (GAOL v5) | exact | exact, over 2 000 random intervals and the values written apart |

## The mathematical library is no longer a choice

GAOL is built with CORE-MATH only, whose sources are compiled into the
library: the bounds above are the bounds on every machine.

The bounds of this page are therefore those of every build of GAOL, and
`tests/core_math.cpp` checks them against CORE-MATH called in the downward and
the upward rounding, which are the tightest bounds there are.
