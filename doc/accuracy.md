# Accuracy of the operations

Part of the documentation of [this fork of GAOL](../README.md#documentation).

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

A bound computed as the correctly rounded value moved one double outward is
at most one double beyond the tightest bound, since the tightest bound is
that value or the double next to it: the result is within
nextOut(f<sub>tightest</sub>(x)), hence accurate. Below, "within k doubles"
means that each bound is at most k doubles beyond the tightest one.

## Conditions

The statements hold for GAOL as the three builds make it: with the mathlib
2.1.1 of `3rd/mathlib`, fixed and compiled as they do it (see
[3rd/README.md](../3rd/README.md)), and the code including GAOL's
headers compiled with the flags of interval arithmetic (see
[Using GAOL](using.md)); for the SSE2 intervals and the FPU intervals alike;
whatever the rounding direction the calling code left
(`tests/rounding_direction.cpp`). The last section gives the differences with CRlibm and with the math library
of the system. The float intervals `gaol::intervalf` and `gaol::interval2f`,
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
| `fma` | — | Not provided | | |

## Integer and absmax functions: tightest required

| IEEE 1788 | GAOL | Algorithm | Tightness | Tests |
|---|---|---|---|---|
| `floor`, `ceil` | `floor(x)`, `ceil(x)` | Of each bound | tightest (exact) | exact |
| — | `integer(x)` | The integers in x: [ceil(lower), floor(upper)] | tightest (exact) | exact |
| `sign`, `trunc`, `roundTiesToEven`, `roundTiesToAway` | — | Not provided | | |
| `abs` | `abs(x)` | Magnitudes of the bounds, 0 when x contains 0 | tightest (exact) | tightest |
| `min`, `max` | `min(x, y)`, `max(x, y)` | Of the bounds | tightest (exact) | tightest |

## Power, exponential and logarithm functions: valid required, accurate recommended

| IEEE 1788 | GAOL | Algorithm | Tightness | Tests |
|---|---|---|---|---|
| `pown(x, n)` | `pow(x, n)` for an int n, `uipow(x, n)` for an unsigned n, and `pow(x, y)` for a degenerate integer y | Binary exponentiation of the bounds, each product rounded outward; x<sup>−n</sup> is 1/x<sup>n</sup>, or (1/x)<sup>n</sup> where x<sup>n</sup> is beyond the largest double, and [\|x\|<sup>−n</sup>, +∞] or [−∞, +∞] where x contains 0 inside. n − 1 rounded products move a bound at most (n − 1)·2<sup>−52</sup> relatively | n = 0, 1, 2: tightest. n ≥ 3: valid, within about 2(n − 1) doubles; accurate for n = 3. Negative n: valid, one rounded division more; tightest where the exact bounds are doubles | n: within n doubles; −n: within 8 doubles for n ≤ 3, overflows included; tightest on the special cases |
| `pow(x, y)` | `pow(x, y)`, `pow(x, d)` for a non-integer y | On x ∩ [0, +∞], with finite bounds: mathlib's pow, correctly rounded to nearest, at the corners of the box x × y where x<sup>y</sup> is the least and the greatest, which the places of the bounds about 1 and 0 give, moved one double outward; 1 where a corner has the base 1 or the exponent 0; [0, ·] for a base from 0 and exponents above 0. With an infinite bound, or a base from 0 and an exponent that is not above 0: exp(y·log(x)), which gives the limits; 0<sup>y</sup> = 0 for y > 0, ∅ for y ≤ 0 | accurate with finite bounds: within one double; valid otherwise | within 1 double at points and over 385 boxes; 4 doubles on the special cases |
| `exp` | `exp(x)` | mathlib's exp, correctly rounded to nearest, at the bounds, moved one double outward; the lower bound at least 0; exp(0) = 1 exactly | accurate: within one double | within 1 double |
| `log` | `log(x)` | mathlib's log at the bounds of x ∩ [0, +∞], moved one double outward; ∅ when x holds no positive number; log(0) = −∞, log(1) = 0 exactly | accurate: within one double | within 1 double |
| `exp2`, `exp10`, `log2`, `log10` | — | Not provided | | |

## Trigonometric and hyperbolic functions: valid required, accurate recommended

| IEEE 1788 | GAOL | Algorithm | Tightness | Tests |
|---|---|---|---|---|
| `cos` | `cos(x)` | The bounds divided by an enclosure of π, rounded outward, tell whether x lies on one piece where cos is monotonic; rounded inward, whether an extremum is within x for sure. Where they cannot tell, a bound being within about \|x\|·2<sup>−52</sup> of a multiple of π, or beyond 2<sup>52</sup>, the signs of mathlib's sin at the bounds do, exactly, x being narrower than 2π (and the sign at the middle of x from π on): signs that differ show one extremum, −1 or 1 according to their order. Then mathlib's cos at the bounds, correctly rounded to nearest, moved one double outward, within [−1, 1]; cos(0) = 1 exactly | accurate, at every magnitude: within one double, −1 and 1 exactly where x holds a multiple of π. [−1, 1] from the width 2·π̲ on, π̲ being the double below π: tightest but for the widths between 2·π̲ and 2π | within 1 double, for all doubles, and over intervals next to the extrema and of width about π and 2π |
| `sin` | `sin(x)` | As cos, the bounds divided by an enclosure of π, minus 1/2, and the signs of mathlib's cos where the quotients cannot tell; mathlib's sin at the bounds, moved one double outward, within [−1, 1]; sin(0) = 0 exactly | accurate, at every magnitude, as cos | as cos |
| `tan` | `tan(x)` | [−∞, +∞] when x is wider than π or holds a pole: (x + [π/2])/[π] at the bounds, rounded outward, tells that no pole is within x when the integer parts agree, and rounded inward that one is for sure; where they cannot tell, the signs of mathlib's cos at the bounds do, exactly, a pole being within x when they differ. Otherwise mathlib's tan at the bounds, moved one double outward; tan(0) = 0 exactly | accurate, at every magnitude: within one double between two poles, [−∞, +∞] exactly when a pole is within x, but for the widths between π̲ and π | within 1 double, for all doubles, and over intervals next to the poles |
| `asin`, `acos` | `asin(x)`, `acos(x)` | mathlib's asin and acos at the bounds of x ∩ [−1, 1], moved one double outward; at −1, 0 and 1, 0 exactly or the tightest bounds of ±π/2 and π | accurate: within one double; tightest at −1, 0 and 1 | within 1 double; tightest at −1, 0 and 1 |
| `atan` | `atan(x)` | mathlib's atan at the bounds, moved one double outward; at 0, ±1 and ±∞, 0 exactly or the tightest bounds of ±π/4 and ±π/2 | accurate: within one double; tightest at 0, ±1 and ±∞ | within 1 double; tightest at 0, ±1 and ±∞ |
| `atan2` | `atan2(y, x)` | mathlib's atan2, correctly rounded to nearest, at the corners of the box y × x where the angle is the least and the greatest, which the signs of the bounds give, moved one double outward; 0, ±π/4, ±π/2 and π where a corner is on an axis, on a diagonal of the right half-plane or at an infinity; [−π, π] when the box has points on the half-line y = 0, x < 0, where the angle is π, and points below it; ∅ for the box {(0, 0)} | accurate: within one double; tightest where a bound is 0, ±π/4, ±π/2 or ±π | within 1 double; tightest on the special cases |
| `sinh`, `tanh`, `asinh` | `sinh(x)`, `tanh(x)`, `asinh(x)` | The libm of the system at the bounds (mathlib has none), rounded to nearest, moved three doubles outward; tanh within [−1, 1]. 0 at 0 exactly; sinh(x) beyond ±MAX for \|x\| ≥ 711, and tanh(x) beyond ±(1 − 2<sup>−53</sup>) for \|x\| ≥ 20, where 1 − \|tanh(x)\| < 2<sup>−54</sup> | valid as long as the libm is within two doubles of the exact value; bounds within 3 + e doubles, e being the error of the libm in doubles; tightest at 0 and at the bounds given | within 4 doubles; tightest at 0, and beyond 711 and 20 |
| `cosh` | `cosh(x)` | As sinh, at the bound of largest magnitude; 1 when x contains 0, and at 0; beyond MAX for \|x\| ≥ 711 | valid as above; tightest at 0 and beyond 711 | within 3 doubles; tightest at 0 and beyond 711 |
| `acosh`, `atanh` | `acosh(x)`, `atanh(x)` | As sinh, on x ∩ [1, +∞], respectively x ∩ [−1, 1]; acosh(1) = 0 and atanh(0) = 0 exactly | valid as above; tightest at 1, respectively 0 | within 4 and 3 doubles; tightest at 1 and 0 |

The libms of glibc, musl, MinGW-w64, macOS and Visual C++ tested by the
continuous integration were at most one double beyond the doubles around the
exact values; `tests/elementary.cpp` checks the enclosures on each platform.

## Recommended functions (Table 10.5)

| IEEE 1788 | GAOL | Algorithm | Tightness | Tests |
|---|---|---|---|---|
| `rootn(x, n)`, n > 0 | `nth_root(x, n)` | n = 1: x; n = 2: `sqrt`. Otherwise mathlib's pow at the bounds with an exponent rounded from 1/n, moved one double outward, on x (odd n, the root of a negative number being the opposite of the root of its magnitude) or x ∩ [0, +∞] (even n); the roots of 0 and 1 are 0 and 1 | n = 1, 2: tightest; tightest at 0, 1 and −1. n a power of 2: accurate, within one double. Otherwise valid: the rounded exponent moves the root by about \|log x\|·2<sup>−53</sup>/n relatively, besides the double | within 8 doubles for x in [2<sup>−30</sup>, 2<sup>30</sup>], 233 for all doubles |
| others | — | Not provided | | |

## Reverse functions (Table 10.1): valid required

GAOL's relational functions compute `f_rel(J, I)`, the hull of the x of I
whose image is in J, the reverse functions of IEEE 1788 with the arguments
in another order. Each computes the preimage of J with the functions above,
rounded outward, and intersects it with I: **valid**. The tests check that
they keep the values they are given, within the number of doubles below.

| IEEE 1788 | GAOL | Tests |
|---|---|---|
| `sqrRev(c, x)` | `sqrt_rel(c, x)` | within 1 double |
| `absRev(c, x)` | `invabs_rel(c, x)` | tightest |
| `pownRev(c, x, p)`, p > 0 | `nth_root_rel(c, p, x)` | within 23 doubles |
| `sinRev`, `cosRev`, `tanRev` | `asin_rel`, `acos_rel`, `atan_rel` | within 2<sup>−49</sup> |
| `coshRev(c, x)` | `acosh_rel(c, x)` | within 16 doubles |
| — | `asinh_rel`, `atanh_rel` | within 10 and 26 doubles |
| `mulRev(b, c, x)` | `div_rel(c, b, x)`, and `c % b` for x = [−∞, +∞] | `%`: tightest; `div_rel`: within 2 doubles |
| `powRev1`, `powRev2`, `atan2Rev1`, `atan2Rev2`, `pownRev` for p < 0 | — | |

`mulRevToPair` (10.5.5), `cancelMinus` and `cancelPlus` (10.5.6) are not
provided.

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
| `isEntire`, `isCommonInterval`, `less`, `strictLess` | — | Not provided | | |
| `intervalToText` | `operator<<` | Hexadecimal format: the bits of the bounds. Decimal formats: each bound written to nearest by the C library with the digits asked for, compared exactly with the bound, and its last digit moved outward when it is on the wrong side | hexadecimal: exact; decimal: valid, and the tightest with the digits asked for where the C library rounds to nearest as it should | hexadecimal: exact; decimal: less than one unit of the last digit |

## With CRlibm or the math library of the system

Built with CRlibm (`--with-mathlib=crlibm`, `-Dwith-mathlib=crlibm`), `exp`,
`log`, `cos`, `tan`, `asin`, `acos`, `atan`, `sinh` and `cosh` take CRlibm's
functions rounded downward and upward, and `sin` too: their bounds are the
tightest where the algorithms above evaluate the functions at the bounds. `pow`, and so `nth_root`, take CRlibm's
pow rounded to nearest, moved one double outward, as with mathlib; `tanh`,
`asinh`, `acosh` and `atanh` the libm, as above.

Built with the math library of the system (`--with-mathlib=m`,
`-Dwith-mathlib=default`), every elementary function takes the libm's value
rounded to nearest and moves it outward by 2·2<sup>−52</sup> relatively and
by the smallest normal double: the bounds are valid only where the libm is
within about one double of the exact value, which is not certified, and each
bound is several doubles beyond the tightest one, and one smallest normal
double away from it near 0. configure and meson warn about it.
