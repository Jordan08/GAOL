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
results may miss. It is run under its own names (`gaol`) and under those of
IEEE 1788-2015 (`gaol_ieee1788`), which compute with the same functions, x³
being `pow(x, 3)` in the first and `pown(x, 3)` in the second.

| Library | Version | Elementary functions |
|---|---|---|
| GAOL, `gaol` and `gaol_ieee1788` | v5, this branch, CMake Release build by Clang 21 | CORE-MATH, correctly rounded in each direction |
| Boost.Interval, `rounded_transc_opp` | Boost 1.92.0 (August 2026), compiled by Clang 21 with the flags of interval arithmetic | The C library's, glibc 2.43 (Ubuntu 26.04.1), the policy of the benchmark and of the special cases |
| Boost.Interval, `rounded_transc_std` | The same | The same, through the other policy |
| C library, downward and upward | glibc 2.43 | Each function called with the rounding direction downward, then upward, what both policies expect to be an enclosure; `pow(x, 3.0)` for x³ |

The results depend on the C library: glibc 2.43 here, Linux x86-64 (glibc
2.31, under Ubuntu 20.04, missed in 41 % and 27 % of the results, asinh,
atanh and acosh included). Other
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

**GAOL v5** encloses f(x) in every one of the 1 600 000 results, under both
namespaces, and gives the tightest interval of doubles around it in all of
them (GAOL v5: 1 232 results of x³, where the cube is below 2^−968, came from
products rounded outward and were one or two doubles wider, until point B of
`TODO.md` computed these corners apart, scaled; see [accuracy](../accuracy.md)).

**Boost.Interval misses f(x) in 34 % of the results with `rounded_transc_opp`
and in 20 % with `rounded_transc_std`.** Its square root and its x³, which it
computes itself with the rounding of the processor, never miss, nor do its
asinh and atanh; every other elementary function misses, log once, the others
in 0.05 % of the results or more, and asin, atan, sinh, tanh and acosh in
100 % with `rounded_transc_opp`. The causes are the C library, whose column
shows what glibc 2.43 gives when it is called downward and upward, and the bug
above:

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
  downward; cosh does so for up to 7 %, acos misses in 0.09 to 5 %, log once
  in the 100 000. Both policies give these values as they are.
- **asin, sinh and tanh round the wrong way for many arguments.** For most
  negative arguments, and for many positive ones of tanh, the value glibc
  computes downward is above the one it computes upward, as if the function
  were computed for |x| and its sign changed afterwards. With
  `rounded_transc_std` the result is then inverted, its lower bound above its
  upper one: 44 to 75 % of the results of these three functions miss f(x), a
  large part of them inverted, the others single doubles. The default
  checking, `checking_strict`, does not see an inverted interval as empty,
  since it holds that no interval is: the program goes on with it. glibc 2.31
  did the same with asinh and atanh, and missed in 0.08 to 0.7 % of the
  results of acosh; glibc 2.43 encloses their values.

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

| Function | Arguments | GAOL v5, `gaol` | GAOL v5, `gaol_ieee1788` | Boost.Interval, `rounded_transc_opp` | Boost.Interval, `rounded_transc_std` | C library, downward and upward |
|---|---|---|---|---|---|---|
| `exp` | [−20, 20] | 0 | 0 | 779 (2 %): upper 779; 740 points | 779 (2 %): upper 779; 740 points | 779 (2 %): upper 779; 740 points |
|  | ±2^u, u in [−60, log2 709] | 0 | 0 | 1 644 (3 %): upper 1644; 1637 points | 1 644 (3 %): upper 1644; 1637 points | 1 644 (3 %): upper 1644; 1637 points |
| `log` | [0, 10] | 0 | 0 | 0 | 0 | 0 |
|  | 2^u, u in [−1074, 1024] | 0 | 0 | 1 (0.002 %): lower 1 | 1 (0.002 %): lower 1 | 1 (0.002 %): lower 1 |
| `sin` | [−10, 10] | 0 | 0 | 2 668 (5 %): lower 1722, upper 946; 1764 points | 2 668 (5 %): lower 1722, upper 946; 1764 points | 50 000 (100 %): lower 24958, upper 25042; 50000 points |
|  | ±2^u, u in [−30, 70] | 0 | 0 | 151 (0.3 %): lower 102, upper 49; 112 points | 151 (0.3 %): lower 102, upper 49; 112 points | 50 000 (100 %): lower 24901, upper 25099; 50000 points |
| `cos` | [−10, 10] | 0 | 0 | 9 901 (20 %): lower 4798, upper 5103; 9504 points | 9 901 (20 %): lower 4798, upper 5103; 9504 points | 50 000 (100 %): lower 25060, upper 24940; 50000 points |
|  | ±2^u, u in [−30, 70] | 0 | 0 | 14 000 (28 %): lower 7711, upper 6289; 13970 points | 14 000 (28 %): lower 7711, upper 6289; 13970 points | 50 000 (100 %): lower 25715, upper 24285; 50000 points |
| `tan` | [−10, 10] | 0 | 0 | 3 939 (8 %): lower 1988, upper 1951; 3939 points | 3 939 (8 %): lower 1988, upper 1951; 3939 points | 50 000 (100 %): lower 24917, upper 25083; 50000 points |
|  | ±2^u, u in [−30, 70] | 0 | 0 | 7 599 (15 %): lower 3381, upper 4218; 7599 points | 7 599 (15 %): lower 3381, upper 4218; 7599 points | 50 000 (100 %): lower 25131, upper 24869; 50000 points |
| `asin` | [−1, 1] | 0 | 0 | 50 000 (100 %): lower 28136, upper 21864; 50000 points | 21 889 (44 %): lower 21869, upper 21864; 21861 inverted; 4 points | 21 889 (44 %): lower 21869, upper 21864; 21861 inverted; 4 points |
|  | ±2^u, u in [−60, −1], and ±(1 − 2^u), u in [−53, −1] | 0 | 0 | 50 000 (100 %): lower 29970, upper 20030; 50000 points | 27 263 (55 %): lower 20118, upper 20030; 12887 inverted; 14372 points | 27 263 (55 %): lower 20118, upper 20030; 12887 inverted; 14372 points |
| `acos` | [−1, 1] | 0 | 0 | 46 (0.09 %): lower 30, upper 16; 39 points | 46 (0.09 %): lower 30, upper 16; 39 points | 46 (0.09 %): lower 30, upper 16; 39 points |
|  | ±2^u, u in [−60, −1], and ±(1 − 2^u), u in [−53, −1] | 0 | 0 | 2 339 (5 %): lower 3, upper 2336; 2335 points | 2 339 (5 %): lower 3, upper 2336; 2335 points | 2 339 (5 %): lower 3, upper 2336; 2335 points |
| `atan` | [−10, 10] | 0 | 0 | 50 000 (100 %): lower 24890, upper 25110; 50000 points | 50 000 (100 %): lower 24890, upper 25110; 50000 points | 50 000 (100 %): lower 24890, upper 25110; 50000 points |
|  | ±2^u, u in [−60, 1000] | 0 | 0 | 50 000 (100 %): lower 25094, upper 24906; 50000 points | 50 000 (100 %): lower 25094, upper 24906; 50000 points | 50 000 (100 %): lower 25094, upper 24906; 50000 points |
| `sinh` | [−10, 10] | 0 | 0 | 50 000 (100 %): lower 25179, upper 24821; 50000 points | 26 392 (53 %): lower 24851, upper 24821; 24419 inverted; 817 points | 26 392 (53 %): lower 24851, upper 24821; 24419 inverted; 817 points |
|  | ±2^u, u in [−60, log2 709] | 0 | 0 | 50 000 (100 %): lower 25080, upper 24920; 50000 points | 36 896 (74 %): lower 25116, upper 24920; 13271 inverted; 23489 points | 36 896 (74 %): lower 25116, upper 24920; 13271 inverted; 23489 points |
| `cosh` | [−10, 10] | 0 | 0 | 27 (0.05 %): upper 27 | 27 (0.05 %): upper 27 | 27 (0.05 %): upper 27 |
|  | ±2^u, u in [−60, log2 709] | 0 | 0 | 3 715 (7 %): lower 1, upper 3714; 3708 points | 3 715 (7 %): lower 1, upper 3714; 3708 points | 3 715 (7 %): lower 1, upper 3714; 3708 points |
| `tanh` | [−10, 10] | 0 | 0 | 50 000 (100 %): lower 24911, upper 25089; 50000 points | 26 764 (54 %): lower 25126, upper 25089; 24026 inverted; 2140 points | 26 764 (54 %): lower 25126, upper 25089; 24026 inverted; 2140 points |
|  | ±2^u, u in [−60, log2 709] | 0 | 0 | 50 000 (100 %): lower 24980, upper 25020; 50000 points | 37 336 (75 %): lower 24966, upper 25020; 17682 inverted; 12684 points | 37 336 (75 %): lower 24966, upper 25020; 17682 inverted; 12684 points |
| `asinh` | [−10, 10] | 0 | 0 | 0 | 0 | 0 |
|  | ±2^u, u in [−60, 1000] | 0 | 0 | 0 | 0 | 0 |
| `acosh` | [1, 10] | 0 | 0 | 50 000 (100 %): lower 50000; 50000 points | 0 | 0 |
|  | 1 + 2^u, u in [−52, 1000] | 0 | 0 | 50 000 (100 %): lower 50000; 50000 points | 0 | 0 |
| `atanh` | [−1, 1] | 0 | 0 | 0 | 0 | 0 |
|  | ±2^u, u in [−60, −1], and ±(1 − 2^u), u in [−53, −1] | 0 | 0 | 0 | 0 | 0 |
| `sqrt` | [0, 10] | 0 | 0 | 0 | 0 | 0 |
|  | 2^u, u in [−1074, 1024] | 0 | 0 | 0 | 0 | 0 |
| x³ (`pow(x, 3)`) | [−10, 10] | 0 | 0 | 0 | 0 | 1 094 (2 %): lower 676, upper 418; 1067 points |
|  | ±2^u, u in [−340, 340] | 0 | 0 | 0 | 0 | 1 060 (2 %): lower 648, upper 412; 1021 points |
| **All** | | **0** | **0** | **546 809 (34 %)** | **323 349 (20 %)** | **587 245 (37 %)** |

#### Tightness

The results that are the tightest interval of doubles around f(x), out of all the arguments, and the median and largest width, in doubles, of the results that enclose f(x) (finite bounds):

| Function | GAOL v5, `gaol` | GAOL v5, `gaol_ieee1788` | Boost.Interval, `rounded_transc_opp` | Boost.Interval, `rounded_transc_std` | C library, downward and upward |
|---|---|---|---|---|---|
| `exp` | 100 000 (100 %); 1 / 1 | 100 000 (100 %); 1 / 1 | 92 401 (92 %); 1 / 2 | 92 401 (92 %); 1 / 2 | 92 401 (92 %); 1 / 2 |
| `log` | 100 000 (100 %); 1 / 1 | 100 000 (100 %); 1 / 1 | 99 922 (100 %); 1 / 2 | 99 922 (100 %); 1 / 2 | 99 922 (100 %); 1 / 2 |
| `sin` | 100 000 (100 %); 1 / 1 | 100 000 (100 %); 1 / 1 | 3 235 (3 %); 141 / 9.2e18 | 3 235 (3 %); 141 / 9.2e18 | 0 |
| `cos` | 100 000 (100 %); 1 / 1 | 100 000 (100 %); 1 / 1 | 3 870 (4 %); 35 / 9.2e18 | 3 870 (4 %); 35 / 9.2e18 | 0 |
| `tan` | 100 000 (100 %); 1 / 1 | 100 000 (100 %); 1 / 1 | 0; 51 / 9.2e18 | 0; 51 / 9.2e18 | 0 |
| `asin` | 100 000 (100 %); 1 / 1 | 100 000 (100 %); 1 / 1 | 0 | 50 842 (51 %); 1 / 2 | 50 842 (51 %); 1 / 2 |
| `acos` | 100 000 (100 %); 1 / 1 | 100 000 (100 %); 1 / 1 | 97 588 (98 %); 1 / 2 | 97 588 (98 %); 1 / 2 | 97 588 (98 %); 1 / 2 |
| `atan` | 100 000 (100 %); 1 / 1 | 100 000 (100 %); 1 / 1 | 0 | 0 | 0 |
| `sinh` | 100 000 (100 %); 1 / 1 | 100 000 (100 %); 1 / 1 | 0 | 11 380 (11 %); 2 / 3 | 11 380 (11 %); 2 / 3 |
| `cosh` | 100 000 (100 %); 1 / 1 | 100 000 (100 %); 1 / 1 | 46 800 (47 %); 2 / 3 | 46 800 (47 %); 2 / 3 | 46 800 (47 %); 2 / 3 |
| `tanh` | 100 000 (100 %); 1 / 1 | 100 000 (100 %); 1 / 1 | 0 | 35 417 (35 %); 1 / 2 | 35 417 (35 %); 1 / 2 |
| `asinh` | 100 000 (100 %); 1 / 1 | 100 000 (100 %); 1 / 1 | 100 000 (100 %); 1 / 1 | 100 000 (100 %); 1 / 1 | 100 000 (100 %); 1 / 1 |
| `acosh` | 100 000 (100 %); 1 / 1 | 100 000 (100 %); 1 / 1 | 0 | 100 000 (100 %); 1 / 1 | 100 000 (100 %); 1 / 1 |
| `atanh` | 100 000 (100 %); 1 / 1 | 100 000 (100 %); 1 / 1 | 100 000 (100 %); 1 / 1 | 100 000 (100 %); 1 / 1 | 100 000 (100 %); 1 / 1 |
| `sqrt` | 100 000 (100 %); 1 / 1 | 100 000 (100 %); 1 / 1 | 100 000 (100 %); 1 / 1 | 100 000 (100 %); 1 / 1 | 100 000 (100 %); 1 / 1 |
| x³ (`pow(x, 3)`) | 100 000 (100 %); 1 / 1 | 100 000 (100 %); 1 / 1 | 8 391 (8 %); 2 / 3 | 8 391 (8 %); 2 / 3 | 95 765 (96 %); 1 / 2 |

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
| `acosh` | Boost.Interval, `rounded_transc_opp` | 7.75198450663563 | between 2.7369096528969394 and 2.73690965289694 | [2.73690965289694, 2.73690965289694] | lower |
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
