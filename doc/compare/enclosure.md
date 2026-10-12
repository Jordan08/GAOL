<!-- Copyright (c) 2026 ENSTA, France
     Created 2026-10-06 by Jordan NININ -->
# Enclosure of the elementary functions: GAOL and Boost.Interval

Part of the [comparison](README.md) of GAOL with libieeep1788, filib++,
PROFIL/BIAS, Solaris Studio and Boost.Interval.

An interval library promises one thing: the interval it returns contains the
exact result. The [special cases](special_cases.md) check the promise at the
edges of the domains; this test checks it in the middle of them. Each of 14
elementary functions, the square root and x³ is computed on the point
intervals [x, x] of 100 000 random arguments x, half of them ordinary, half at
the edges of the doubles or of the domain, and each result is compared with
f(x), computed by mpmath with 2000 bits (8000 where f(x) is that close to a
double, as tanh(709) is to 1). A result [l, u] misses f(x) when l > f(x) or
u < f(x).

[Boost.Interval](https://www.boost.org/libs/numeric/interval) is the
interval library C++ programmers find first, in Boost. Its arithmetic is
computed by the processor in the rounding direction each bound needs, but its
elementary functions are those of the C library (`std::exp`, `std::sin`...),
called with the rounding direction set downward for the lower bound and upward
for the upper one: they enclose f(x) only if the C library honours that
direction. Its documentation warns that "unfortunately, the latter is rarely
the case" (`doc/rounding.htm`). GAOL v5, whose elementary functions are
CORE-MATH's, correctly rounded in each direction, is the control: none of its
results may miss.

| Library | Version | Elementary functions |
|---|---|---|
| GAOL | v5, this branch, CMake Release build by Clang 18 | CORE-MATH, correctly rounded in each direction |
| Boost.Interval, `rounded_transc_opp` | Boost 1.92.0 (August 2026), compiled by Clang 18 with the flags of interval arithmetic | The C library's, glibc 2.31 (Ubuntu 20.04), the policy of the benchmark and of the special cases |
| Boost.Interval, `rounded_transc_std` | The same | The same, through the other policy |
| C library, downward and upward | glibc 2.31 | Each function called with the rounding direction downward, then upward, what both policies expect to be an enclosure; `pow(x, 3.0)` for x³ |

The results depend on the C library: glibc 2.31 here, Linux x86-64. Other
versions of glibc, and other systems, give other numbers: the issue
[#25](https://github.com/boostorg/interval/issues/25) of Boost.Interval
reports an inverted cos on macOS.

## The policies of Boost.Interval

`boost::numeric::interval<double>`, the type with the default policies, has no
elementary function: its rounding policy, `rounded_math<double>`, which is
`save_state<rounded_arith_opp<double> >` for doubles, has no `exp_down()` nor
`sin_up()`, and `exp(x)` does not compile (issue
[#13](https://github.com/boostorg/interval/issues/13)). Its documentation says
why: a policy that takes the functions of the C library "will most likely
lead to a violation of the inclusion property", and the user who chooses one
"knows what he stands against". Two policies give them:

- `rounded_transc_std<double>` sets the rounding direction downward, calls
  the function for the lower bound, sets it upward and calls it for the upper
  one;
- `rounded_transc_opp<double>`, built on `rounded_arith_opp<double>`, the
  arithmetic of the default policies, keeps the direction upward and sets it
  downward only around the call for the lower bound. It is the policy of the
  example of Boost.Interval that takes the elementary functions of the C
  library (`examples/findroot_demo.cpp`).

The benchmark and the special cases use
`interval<double, policies<save_state<rounded_transc_opp<double> >, checking_strict<double> > >`:
everything but the elementary functions is then the code of the default
`interval<double>` (`code/boost_policies.h`). This test measures both
policies.

`rounded_transc_opp` has a bug of its own. For sin, tan, asin, atan, sinh,
tanh and acosh it computes the lower bound as `-force_rounding(-f(x))`, the
opposite of the opposite of f(x) computed upward, that is f(x) computed
upward again: the lower bound is the upper one. The trick of the opposite,
which works for the arithmetic, would need f_down(x) = −f_up(−x), and only for
the odd functions, which acosh is not. Its results of asin, sinh, tanh and
acosh are therefore single doubles, which miss f(x) whenever it is not a
double. The bug is in `boost/numeric/interval/rounded_transc.hpp` of Boost
1.92.0, and none of the issues of the repository of Boost.Interval reports it
(October 2026). Its sin is not affected, Boost.Interval computing sin(x) as
cos(x − π/2), and its tan and atan would be the same without it, glibc
computing them the same in every direction (below).

## What the results show

**GAOL v5** encloses f(x) in every one of the 1 600 000 results, and gives
the tightest interval of doubles around it in all but 1 232: its x³, where the
cube is below 2^−968 and its exact products cannot be taken, rounds each
product outward and is one or two doubles wider, as [accuracy](../accuracy.md)
says.

**Boost.Interval misses f(x) in 41 % of the results with `rounded_transc_opp`
and in 27 % with `rounded_transc_std`.** Its square root and its x³, which it
computes itself with the rounding of the processor, never miss; every
elementary function misses, log once, the others in 0.05 % of the results or
more, and asin, atan, sinh, tanh and acosh in 100 % with `rounded_transc_opp`.
The causes are the C library, whose column shows what glibc 2.31 gives when it
is called downward and upward, and the bug above:

- **sin, cos, tan and atan ignore the rounding direction.** Called downward,
  to nearest and upward, glibc's functions give the same double for every
  argument, which misses f(x) unless f(x) is that double. Boost.Interval's atan misses in 100 % of the results with
  both policies, and `atan([−∞, +∞])` is [−1.5707963267948966,
  1.5707963267948966], below π/2 (case 117 of the [special
  cases](special_cases.md)). Its sin, cos and tan miss less often, 0.3 to
  28 %: they reduce the argument by subtracting a multiple of an interval
  enclosing π or 2π, which widens most results by tens to hundreds of
  doubles; the ones that miss are, for the most part, those that stay a
  single double.
- **exp, cosh, acos and log are computed in the direction asked, but not
  always on the right side of f(x).** Called upward, glibc's exp gives a
  double below f(x) for 2 to 3 % of the arguments, most often the one it gives
  downward; cosh does so for up to 7 %, acos misses in 0.2 to 5 %, log once
  in the 100 000. Both policies give these values as they are.
- **asin, sinh, tanh, asinh and atanh round the wrong way for many
  arguments.** For most negative arguments, and for many positive ones of
  tanh, the value glibc computes downward is above the one it computes upward,
  as if the function were computed for |x| and its sign changed afterwards.
  With `rounded_transc_std` the result is then inverted, its lower bound above
  its upper one: 44 to 75 % of the results of these five functions miss f(x),
  a large part of them inverted, the others single doubles. The default
  checking, `checking_strict`, does not see an inverted interval as empty,
  since it holds that no interval is: the program goes on with it. acosh
  misses in 0.08 to 0.7 % of the results.

On the benchmark, whose intervals are not points, the sums of the widths of
Boost.Interval's results of exp and log equal libieeep1788's
([performance](performance.md)): upper bounds one double too low in 2 to 3 %
of the results, as here, cannot show in a sum of a million widths. Only a
check of each bound against the exact value finds them.

## The results

<!-- BEGIN GENERATED TABLES (doc/compare/code/enclosure.py) -->

100 000 arguments for each function (random seed 1788), half of them ordinary, half at the edges of the doubles or of the domain; each value computed by mpmath with 2000 bits.

#### Results that do not enclose the value

The number of arguments x for which f([x, x]) misses f(x), on its lower bound, its upper bound or both, and how many of these results are inverted, their lower bound above their upper one, or points, a single double that is not f(x):

| Function | Arguments | GAOL v5 | Boost.Interval, `rounded_transc_opp` | Boost.Interval, `rounded_transc_std` | C library, downward and upward |
|---|---|---|---|---|---|
| `exp` | [−20, 20] | 0 | 779 (2 %): upper 779; 740 points | 779 (2 %): upper 779; 740 points | 779 (2 %): upper 779; 740 points |
|  | ±2^u, u in [−60, log2 709] | 0 | 1 632 (3 %): upper 1632; 1625 points | 1 632 (3 %): upper 1632; 1625 points | 1 632 (3 %): upper 1632; 1625 points |
| `log` | [0, 10] | 0 | 0 | 0 | 0 |
|  | 2^u, u in [−1074, 1024] | 0 | 1 (0.002 %): lower 1 | 1 (0.002 %): lower 1 | 1 (0.002 %): lower 1 |
| `sin` | [−10, 10] | 0 | 2 668 (5 %): lower 1722, upper 946; 1764 points | 2 668 (5 %): lower 1722, upper 946; 1764 points | 50 000 (100 %): lower 24958, upper 25042; 50000 points |
|  | ±2^u, u in [−30, 70] | 0 | 155 (0.3 %): lower 102, upper 53; 115 points | 155 (0.3 %): lower 102, upper 53; 115 points | 50 000 (100 %): lower 24958, upper 25042; 50000 points |
| `cos` | [−10, 10] | 0 | 9 901 (20 %): lower 4798, upper 5103; 9504 points | 9 901 (20 %): lower 4798, upper 5103; 9504 points | 50 000 (100 %): lower 25060, upper 24940; 50000 points |
|  | ±2^u, u in [−30, 70] | 0 | 14 004 (28 %): lower 7730, upper 6274; 13972 points | 14 004 (28 %): lower 7730, upper 6274; 13972 points | 50 000 (100 %): lower 25849, upper 24151; 50000 points |
| `tan` | [−10, 10] | 0 | 3 939 (8 %): lower 1985, upper 1954; 3939 points | 3 939 (8 %): lower 1985, upper 1954; 3939 points | 50 000 (100 %): lower 24914, upper 25086; 50000 points |
|  | ±2^u, u in [−30, 70] | 0 | 7 599 (15 %): lower 3390, upper 4209; 7599 points | 7 599 (15 %): lower 3390, upper 4209; 7599 points | 50 000 (100 %): lower 25120, upper 24880; 50000 points |
| `asin` | [−1, 1] | 0 | 50 000 (100 %): lower 28134, upper 21866; 50000 points | 21 914 (44 %): lower 21873, upper 21866; 21825 inverted; 89 points | 21 914 (44 %): lower 21873, upper 21866; 21825 inverted; 89 points |
|  | ±2^u, u in [−60, −1], and ±(1 − 2^u), u in [−53, −1] | 0 | 50 000 (100 %): lower 29971, upper 20029; 50000 points | 27 264 (55 %): lower 20120, upper 20029; 12885 inverted; 14379 points | 27 264 (55 %): lower 20120, upper 20029; 12885 inverted; 14379 points |
| `acos` | [−1, 1] | 0 | 78 (0.2 %): lower 50, upper 28; 78 points | 78 (0.2 %): lower 50, upper 28; 78 points | 78 (0.2 %): lower 50, upper 28; 78 points |
|  | ±2^u, u in [−60, −1], and ±(1 − 2^u), u in [−53, −1] | 0 | 2 339 (5 %): lower 3, upper 2336; 2339 points | 2 339 (5 %): lower 3, upper 2336; 2339 points | 2 339 (5 %): lower 3, upper 2336; 2339 points |
| `atan` | [−10, 10] | 0 | 50 000 (100 %): lower 24898, upper 25102; 50000 points | 50 000 (100 %): lower 24898, upper 25102; 50000 points | 50 000 (100 %): lower 24898, upper 25102; 50000 points |
|  | ±2^u, u in [−60, 1000] | 0 | 50 000 (100 %): lower 25091, upper 24909; 50000 points | 50 000 (100 %): lower 25091, upper 24909; 50000 points | 50 000 (100 %): lower 25091, upper 24909; 50000 points |
| `sinh` | [−10, 10] | 0 | 50 000 (100 %): lower 25182, upper 24818; 50000 points | 26 400 (53 %): lower 24842, upper 24818; 24412 inverted; 823 points | 26 400 (53 %): lower 24842, upper 24818; 24412 inverted; 823 points |
|  | ±2^u, u in [−60, log2 709] | 0 | 50 000 (100 %): lower 25079, upper 24921; 50000 points | 36 899 (74 %): lower 25125, upper 24921; 13276 inverted; 23476 points | 36 899 (74 %): lower 25125, upper 24921; 13276 inverted; 23476 points |
| `cosh` | [−10, 10] | 0 | 27 (0.05 %): upper 27 | 27 (0.05 %): upper 27 | 27 (0.05 %): upper 27 |
|  | ±2^u, u in [−60, log2 709] | 0 | 3 718 (7 %): lower 1, upper 3717; 3711 points | 3 718 (7 %): lower 1, upper 3717; 3711 points | 3 718 (7 %): lower 1, upper 3717; 3711 points |
| `tanh` | [−10, 10] | 0 | 50 000 (100 %): lower 24911, upper 25089; 50000 points | 26 765 (54 %): lower 25128, upper 25089; 24028 inverted; 2140 points | 26 765 (54 %): lower 25128, upper 25089; 24028 inverted; 2140 points |
|  | ±2^u, u in [−60, log2 709] | 0 | 50 000 (100 %): lower 24975, upper 25025; 50000 points | 37 343 (75 %): lower 24991, upper 25025; 17689 inverted; 12701 points | 37 343 (75 %): lower 24991, upper 25025; 17689 inverted; 12701 points |
| `asinh` | [−10, 10] | 0 | 25 763 (52 %): lower 25150, upper 25160; 24665 inverted; 958 points | 25 763 (52 %): lower 25150, upper 25160; 24665 inverted; 958 points | 25 763 (52 %): lower 25150, upper 25160; 24665 inverted; 958 points |
|  | ±2^u, u in [−60, 1000] | 0 | 25 665 (51 %): lower 24896, upper 24876; 24110 inverted; 1552 points | 25 665 (51 %): lower 24896, upper 24876; 24110 inverted; 1552 points | 25 665 (51 %): lower 24896, upper 24876; 24110 inverted; 1552 points |
| `acosh` | [1, 10] | 0 | 50 000 (100 %): lower 49799, upper 201; 50000 points | 342 (0.7 %): lower 173, upper 201; 32 inverted; 231 points | 342 (0.7 %): lower 173, upper 201; 32 inverted; 231 points |
|  | 1 + 2^u, u in [−52, 1000] | 0 | 50 000 (100 %): lower 49977, upper 23; 50000 points | 38 (0.08 %): lower 19, upper 23; 4 inverted; 26 points | 38 (0.08 %): lower 19, upper 23; 4 inverted; 26 points |
| `atanh` | [−1, 1] | 0 | 26 354 (53 %): lower 24969, upper 24915; 24126 inverted; 1705 points | 26 354 (53 %): lower 24969, upper 24915; 24126 inverted; 1705 points | 26 354 (53 %): lower 24969, upper 24915; 24126 inverted; 1705 points |
|  | ±2^u, u in [−60, −1], and ±(1 − 2^u), u in [−53, −1] | 0 | 31 654 (63 %): lower 24799, upper 24783; 18023 inverted; 13566 points | 31 654 (63 %): lower 24799, upper 24783; 18023 inverted; 13566 points | 31 654 (63 %): lower 24799, upper 24783; 18023 inverted; 13566 points |
| `sqrt` | [0, 10] | 0 | 0 | 0 | 0 |
|  | 2^u, u in [−1074, 1024] | 0 | 0 | 0 | 0 |
| x³ (`pow(x, 3)`) | [−10, 10] | 0 | 0 | 0 | 1 094 (2 %): lower 676, upper 418; 1067 points |
|  | ±2^u, u in [−340, 340] | 0 | 0 | 0 | 1 062 (2 %): lower 643, upper 419; 1032 points |
| **All** | | **0** | **656 276 (41 %)** | **433 241 (27 %)** | **697 131 (44 %)** |

#### Tightness

The results that are the tightest interval of doubles around f(x), out of all the arguments, and the median and largest width, in doubles, of the results that enclose f(x) (finite bounds):

| Function | GAOL v5 | Boost.Interval, `rounded_transc_opp` | Boost.Interval, `rounded_transc_std` | C library, downward and upward |
|---|---|---|---|---|
| `exp` | 100 000 (100 %); 1 / 1 | 92 406 (92 %); 1 / 2 | 92 406 (92 %); 1 / 2 | 92 406 (92 %); 1 / 2 |
| `log` | 100 000 (100 %); 1 / 1 | 99 923 (100 %); 1 / 2 | 99 923 (100 %); 1 / 2 | 99 923 (100 %); 1 / 2 |
| `sin` | 100 000 (100 %); 1 / 1 | 3 230 (3 %); 141 / 9.2e18 | 3 230 (3 %); 141 / 9.2e18 | 0 |
| `cos` | 100 000 (100 %); 1 / 1 | 3 869 (4 %); 35 / 9.2e18 | 3 869 (4 %); 35 / 9.2e18 | 0 |
| `tan` | 100 000 (100 %); 1 / 1 | 0; 51 / 9.2e18 | 0; 51 / 9.2e18 | 0 |
| `asin` | 100 000 (100 %); 1 / 1 | 0 | 50 822 (51 %); 1 / 1 | 50 822 (51 %); 1 / 1 |
| `acos` | 100 000 (100 %); 1 / 1 | 97 583 (98 %); 1 / 1 | 97 583 (98 %); 1 / 1 | 97 583 (98 %); 1 / 1 |
| `atan` | 100 000 (100 %); 1 / 1 | 0 | 0 | 0 |
| `sinh` | 100 000 (100 %); 1 / 1 | 0 | 11 419 (11 %); 2 / 3 | 11 419 (11 %); 2 / 3 |
| `cosh` | 100 000 (100 %); 1 / 1 | 46 793 (47 %); 2 / 3 | 46 793 (47 %); 2 / 3 | 46 793 (47 %); 2 / 3 |
| `tanh` | 100 000 (100 %); 1 / 1 | 0 | 35 410 (35 %); 1 / 2 | 35 410 (35 %); 1 / 2 |
| `asinh` | 100 000 (100 %); 1 / 1 | 14 807 (15 %); 2 / 3 | 14 807 (15 %); 2 / 3 | 14 807 (15 %); 2 / 3 |
| `acosh` | 100 000 (100 %); 1 / 1 | 0 | 28 439 (28 %); 2 / 5 | 28 439 (28 %); 2 / 5 |
| `atanh` | 100 000 (100 %); 1 / 1 | 26 168 (26 %); 1 / 5 | 26 168 (26 %); 1 / 5 | 26 168 (26 %); 1 / 5 |
| `sqrt` | 100 000 (100 %); 1 / 1 | 100 000 (100 %); 1 / 1 | 100 000 (100 %); 1 / 1 | 100 000 (100 %); 1 / 1 |
| x³ (`pow(x, 3)`) | 98 768 (99 %); 1 / 3 | 8 371 (8 %); 2 / 3 | 8 371 (8 %); 2 / 3 | 95 711 (96 %); 1 / 2 |

#### Examples

The first argument whose result misses the value, for each function and library, the ordinary arguments first:

| Function | Library | x | f(x) | Result | Missed by |
|---|---|---|---|---|---|
| `exp` | Boost.Interval, `rounded_transc_opp` | 6.043903551312859 | between 421.5353121657026 and 421.53531216570263 | [421.5353121657026, 421.5353121657026] | upper |
| `exp` | Boost.Interval, `rounded_transc_std` | 6.043903551312859 | between 421.5353121657026 and 421.53531216570263 | [421.5353121657026, 421.5353121657026] | upper |
| `exp` | C library, downward and upward | 6.043903551312859 | between 421.5353121657026 and 421.53531216570263 | [421.5353121657026, 421.5353121657026] | upper |
| `log` | Boost.Interval, `rounded_transc_opp` | 1.0932685527420436 | between 0.08917188143174624 and 0.08917188143174626 | [0.08917188143174626, 0.08917188143174627] | lower |
| `log` | Boost.Interval, `rounded_transc_std` | 1.0932685527420436 | between 0.08917188143174624 and 0.08917188143174626 | [0.08917188143174626, 0.08917188143174627] | lower |
| `log` | C library, downward and upward | 1.0932685527420436 | between 0.08917188143174624 and 0.08917188143174626 | [0.08917188143174626, 0.08917188143174627] | lower |
| `sin` | Boost.Interval, `rounded_transc_opp` | 4.848967763736285 | between −0.9906876074118126 and −0.9906876074118125 | [−0.9906876074118127, −0.9906876074118126] | upper |
| `sin` | Boost.Interval, `rounded_transc_std` | 4.848967763736285 | between −0.9906876074118126 and −0.9906876074118125 | [−0.9906876074118127, −0.9906876074118126] | upper |
| `sin` | C library, downward and upward | 2.4874797010453555 | between 0.6084555326154772 and 0.6084555326154774 | [0.6084555326154772, 0.6084555326154772] | upper |
| `cos` | Boost.Interval, `rounded_transc_opp` | 2.006201181442524 | between −0.4217775173719202 and −0.42177751737192015 | [−0.42177751737192015, −0.42177751737192015] | lower |
| `cos` | Boost.Interval, `rounded_transc_std` | 2.006201181442524 | between −0.4217775173719202 and −0.42177751737192015 | [−0.42177751737192015, −0.42177751737192015] | lower |
| `cos` | C library, downward and upward | 5.015835724275885 | between 0.2988112449547942 and 0.2988112449547943 | [0.2988112449547942, 0.2988112449547942] | upper |
| `tan` | Boost.Interval, `rounded_transc_opp` | 0.8933225096676445 | between 1.2430208321751248 and 1.243020832175125 | [1.243020832175125, 1.243020832175125] | lower |
| `tan` | Boost.Interval, `rounded_transc_std` | 0.8933225096676445 | between 1.2430208321751248 and 1.243020832175125 | [1.243020832175125, 1.243020832175125] | lower |
| `tan` | C library, downward and upward | 7.208320416990791 | between 1.3273502556254857 and 1.327350255625486 | [1.327350255625486, 1.327350255625486] | lower |
| `asin` | Boost.Interval, `rounded_transc_opp` | −0.4664245603471817 | between −0.4852444003015382 and −0.48524440030153815 | [−0.4852444003015382, −0.4852444003015382] | upper |
| `asin` | Boost.Interval, `rounded_transc_std` | −0.4664245603471817 | between −0.4852444003015382 and −0.48524440030153815 | [−0.48524440030153815, −0.4852444003015382] | lower and upper |
| `asin` | C library, downward and upward | −0.4664245603471817 | between −0.4852444003015382 and −0.48524440030153815 | [−0.48524440030153815, −0.4852444003015382] | lower and upper |
| `acos` | Boost.Interval, `rounded_transc_opp` | 0.9179662797034012 | between 0.4078737813501995 and 0.4078737813501996 | [0.4078737813501995, 0.4078737813501995] | upper |
| `acos` | Boost.Interval, `rounded_transc_std` | 0.9179662797034012 | between 0.4078737813501995 and 0.4078737813501996 | [0.4078737813501995, 0.4078737813501995] | upper |
| `acos` | C library, downward and upward | 0.9179662797034012 | between 0.4078737813501995 and 0.4078737813501996 | [0.4078737813501995, 0.4078737813501995] | upper |
| `atan` | Boost.Interval, `rounded_transc_opp` | 5.036061608373467 | between 1.3747781986203427 and 1.374778198620343 | [1.374778198620343, 1.374778198620343] | lower |
| `atan` | Boost.Interval, `rounded_transc_std` | 5.036061608373467 | between 1.3747781986203427 and 1.374778198620343 | [1.374778198620343, 1.374778198620343] | lower |
| `atan` | C library, downward and upward | 5.036061608373467 | between 1.3747781986203427 and 1.374778198620343 | [1.374778198620343, 1.374778198620343] | lower |
| `sinh` | Boost.Interval, `rounded_transc_opp` | −1.366763172863143 | between −1.8338512231580473 and −1.833851223158047 | [−1.8338512231580475, −1.8338512231580475] | upper |
| `sinh` | Boost.Interval, `rounded_transc_std` | −1.366763172863143 | between −1.8338512231580473 and −1.833851223158047 | [−1.833851223158047, −1.8338512231580475] | lower and upper |
| `sinh` | C library, downward and upward | −1.366763172863143 | between −1.8338512231580473 and −1.833851223158047 | [−1.833851223158047, −1.8338512231580475] | lower and upper |
| `cosh` | Boost.Interval, `rounded_transc_opp` | 5.556189307814055 | between 129.41923952006852 and 129.41923952006854 | [129.4192395200685, 129.41923952006852] | upper |
| `cosh` | Boost.Interval, `rounded_transc_std` | 5.556189307814055 | between 129.41923952006852 and 129.41923952006854 | [129.4192395200685, 129.41923952006852] | upper |
| `cosh` | C library, downward and upward | 5.556189307814055 | between 129.41923952006852 and 129.41923952006854 | [129.4192395200685, 129.41923952006852] | upper |
| `tanh` | Boost.Interval, `rounded_transc_opp` | 5.730192834795806 | between 0.9999789213320901 and 0.9999789213320902 | [0.9999789213320902, 0.9999789213320902] | lower |
| `tanh` | Boost.Interval, `rounded_transc_std` | −1.7810547934485133 | between −0.9448084865396353 and −0.9448084865396352 | [−0.9448084865396352, −0.9448084865396353] | lower and upper |
| `tanh` | C library, downward and upward | −1.7810547934485133 | between −0.9448084865396353 and −0.9448084865396352 | [−0.9448084865396352, −0.9448084865396353] | lower and upper |
| `asinh` | Boost.Interval, `rounded_transc_opp` | 1.2123911099215015 | between 1.0238816229912833 and 1.0238816229912835 | [1.0238816229912835, 1.0238816229912835] | lower |
| `asinh` | Boost.Interval, `rounded_transc_std` | 1.2123911099215015 | between 1.0238816229912833 and 1.0238816229912835 | [1.0238816229912835, 1.0238816229912835] | lower |
| `asinh` | C library, downward and upward | 1.2123911099215015 | between 1.0238816229912833 and 1.0238816229912835 | [1.0238816229912835, 1.0238816229912835] | lower |
| `acosh` | Boost.Interval, `rounded_transc_opp` | 7.75198450663563 | between 2.7369096528969394 and 2.73690965289694 | [2.7369096528969403, 2.7369096528969403] | lower |
| `acosh` | Boost.Interval, `rounded_transc_std` | 1.0691195005282204 | between 0.36969611242313416 and 0.3696961124231342 | [0.3696961124231342, 0.3696961124231342] | lower |
| `acosh` | C library, downward and upward | 1.0691195005282204 | between 0.36969611242313416 and 0.3696961124231342 | [0.3696961124231342, 0.3696961124231342] | lower |
| `atanh` | Boost.Interval, `rounded_transc_opp` | 0.17622425454097468 | between 0.1780832318080506 and 0.17808323180805063 | [0.17808323180805063, 0.1780832318080506] | lower and upper |
| `atanh` | Boost.Interval, `rounded_transc_std` | 0.17622425454097468 | between 0.1780832318080506 and 0.17808323180805063 | [0.17808323180805063, 0.1780832318080506] | lower and upper |
| `atanh` | C library, downward and upward | 0.17622425454097468 | between 0.1780832318080506 and 0.17808323180805063 | [0.17808323180805063, 0.1780832318080506] | lower and upper |
| x³ (`pow(x, 3)`) | C library, downward and upward | −4.738776853212496 | between −106.41400175425356 and −106.41400175425355 | [−106.41400175425355, −106.41400175425355] | lower |

<!-- END GENERATED TABLES -->
## Running it again

```bash
cd doc/compare/code
./setup.sh              # once (see code/README.md)
./run_enclosure.sh      # -> enclosure.md
```

`run_enclosure.sh` draws the arguments (`enclosure.py data`), compiles and
runs `enclosure_gaol.cpp` and `enclosure_boost.cpp`, which write the bounds of
their results, and checks them against mpmath (`enclosure.py report`), which
writes the tables above. mpmath takes about 5 minutes for the 1 600 000
values, once: they are kept in the work directory. `ENCLOSURE_N` sets the
number of arguments of each function (100 000). The functions and their
arguments are in `enclosure.py` and `enclosure.h`; another library is added
with a program like `enclosure_gaol.cpp` and a line in `LIBRARIES` of
`enclosure.py`.
