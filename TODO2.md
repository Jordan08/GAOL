# TODO2: what was done with TODO.md

The ten points of [TODO.md](TODO.md), from the review of commit a7544a1
(`gaol_ieee1788`), are dealt with on the branch `todo-review-fixes`, made from
`MATH-CORE` at 0554504. Each section says what changed, in which commit, and
how it was checked; the last ones say what is left and what changed for the
programs that use GAOL.

| Point | Status | Commits |
| --- | --- | --- |
| 1. `pow` with a number as exponent under the using-directive | fixed | be03d16, f6b02e0 |
| 2. binary compatibility of `gaol::pow` | fixed | be03d16 |
| 3. an integer exponent beyond the ints gave [−∞, +∞] | fixed in `gaol_ieee1788::pow` | be03d16, f6b02e0 |
| 4. the logic of `pow` written twice | fixed | be03d16 |
| 5. the overload set depended on the order of the includes | fixed | f6b02e0 |
| 6. `less` clashes with `std::less` | documented (a clash of names, which only the program can avoid) | 5cb574e |
| 7. `intervalToExact` neither exception-safe nor thread-safe | fixed | ca64947, f6b02e0 |
| 8. the test of the using-directive did not cover point 1 | fixed | f6b02e0 |
| 9. the documentation of `pow` | fixed | 5cb574e |
| 10. proposing the CORE-MATH fixes upstream | patches ready; sending them is yours | 76f6f7d |

## pow (points 1 to 4)

`gaol/gaol_interval.h` and `gaol/gaol_interval.cpp` now have four functions
the library compiles:

- `pown(I, n)`, the integer power, which `pow(I, int)` was;
- `pow_real(I, J)`, the pow of IEEE 1788-2015 (Table 9.1), on the part of I
  in [0, +∞], which was the second half of `pow_hybrid()`; for a degenerate
  integer exponent within the ints it takes `pown` on that part, which gives
  the powers that are doubles exactly, and otherwise CORE-MATH's pow at the
  corners of the box, so that an exponent beyond the ints is no longer
  [−∞, +∞]: `pow([2, 3], [1e10])` is [DBL_MAX, +∞], `pow([1], [−1e12])` is
  [1];
- `pow_hybrid(I, J)`, GAOL's pow: `pown` for a degenerate integer exponent,
  `pow_real` otherwise (point 4: the checks are no longer repeated);
- `pow_hybrid(I, p)`, for a double p, which `pow(I, double)` was.

The three `gaol::pow` (int, interval and double exponents) are function
templates whose parameter is never deduced, calling them. `gaol_ieee1788` has
three plain `pow`, which call `pow_real` (with `[p]` for a number p), and
overload resolution prefers a plain function to a template that fits as well:
under `using namespace gaol_ieee1788;`, `pow(x, 2)` and `pow(x, 0.5)` are now
the pow of the standard (empty for x = [−4, −1]), where they silently were
GAOL's pown ([1, 16]). `gaol_ieee1788::pown` is `gaol::pown`.

Point 2: the library still defines the three plain `pow(const interval&, …)`
it had, which the header no longer declares, so that an object compiled
against the former header links. `nm -C libgaol.a` lists them beside `pown`,
`pow_real` and the two `pow_hybrid`.

## The names of GAOL in gaol_ieee1788 (point 5)

The functions of GAOL that have the name and the meaning of the standard
(`sqr`, `sqrt`, `fma`, `exp`, `exp2`, `exp10`, `log`, `log2`, `log10`, the
trigonometric and hyperbolic functions, `sign`, `ceil`, `floor`, `trunc`,
`abs`, `min`, `max`, `expm1`, `exp2m1`, `exp10m1`, `log2p1`, `log10p1`,
`hypot`) are no longer brought in by using-declarations, which took the
overloads of `gaol::expression` too when `gaol/gaol_expression.h` came first,
but forwarded by function templates. A template takes part only when one
argument at least is an interval and every other one converts to an interval;
on intervals, the plain function of `gaol` found by argument-dependent lookup
wins over it, and is the same computation. So the overload set is the same
whatever the order of the includes, `min(x, 1.0)` and `atan2(y, 1.0)` work,
and `sqrt(4)` or `floor(2.5)` on numbers remain the functions of C (a
template accepting any number would have taken `sqrt(4)` from C).

## less and std::less (point 6)

`less(x, y)` next to `using namespace std;` is ambiguous with the class
template `std::less`, and nothing in GAOL can change that: the standard names
the function `less`. `doc/using.md` says to write `gaol_ieee1788::less(x, y)`
there. The other names of the standard were checked against `std` with GCC
and Clang (`sqrt`, `min`, `max`, `abs`, `pow`, `fma`, `hypot`, `atan2`,
`equal`...): they do not clash.

## intervalToExact (point 7)

`gaol::exact_string(I)` returns the exact text of an interval, what
`operator<<` writes in `interval_format::hexa`, without reading or changing
the global output format; `operator<<` uses it, and
`gaol_ieee1788::intervalToExact` is it. Switching the format to hexa and back
is gone, with the output left in hexa if it threw, and the other threads
seeing hexa meanwhile.

## Tests (point 8)

`tests/ieee1788.cpp` is a new test (the eleventh of ctest) written as a
program uses `gaol_ieee1788`: `using namespace gaol_ieee1788;` at file scope,
no `using namespace gaol`, and `gaol/gaol_expression.h` included before
`gaol/gaol`. It checks, in 16 checks and 4 `static_assert`: `pow` with an
interval, an `int` or a `double` exponent on a negative base (empty), `pown`
and `gaol::pow` (the integer power), `pow([0], 0)`, `pow([0, 4], n)`, infinite
and NaN exponents, the integer exponents beyond the ints of point 3, GAOL's
functions on intervals and on an interval and a number, the qualified names,
the functions of C on numbers (their types, by `static_assert`), and
`intervalToExact` against `exact_string` and the output format.
`tests/other_functions.cpp` keeps its checks of the names and of the
using-directive beside `using namespace gaol`.

## Documentation (point 9)

- `gaol/gaol_interval.h`: a Doxygen block for each of `pown`, `pow_real`,
  `pow_hybrid` (both) and the three templates.
- `manual/gaol.tex`: the entry of `pow` lists the three overloads and says
  what each is and why they are templates; a new entry for `pown` and
  `pow_real`; `exact_string` next to the hexa format. The line saying the
  power for interval exponents was "not yet fully tested" is gone. The manual
  compiles with pdflatex.
- `doc/using.md` (the names of IEEE 1788-2015), `doc/differences.md`,
  `doc/tests.md` (the new test) and `doc/accuracy.md` (the rows of `pown` and
  `pow`).

## CORE-MATH (point 10)

`3rd/core-math-patches/` holds four patches against upstream `master`
(03720fea, 21 September 2026), which still has the four defects, written in
upstream's own terms, and a README saying what each fixes and how it was
checked:

1. the masks `~0ul >> 12` of `sinh.c`, `cosh.c` and `tanh.c` (ten, on nine
   lines);
2. the signed shifts of `cospi.c`;
3. the 128-bit shift of `asinpi_acc`, now `(u128)(i128)dc << ss`;
4. `__builtin_expect(ix.u, 1)` of `rsqrt.c`, now `ix.u != 0`.

Each applies with `git apply`. Compiled as they are and as patched, the six
functions give the same bits over 1.6 million arguments in the four rounding
directions on x86-64 (GCC 13) and on i386 (Clang 18), but for `rsqrt` on
i386, where the upstream function returns +∞ for the subnormal powers of 4
(6 029 differences) and the patched one the right power of two. Proposing them
(a merge request on gitlab.inria.fr, or core-math@inria.fr) is to be done by
you: it speaks for the project.

## How it was all checked

Locally, before pushing:

- x86-64, GCC 13, Release: 11 tests of 11;
- x86-64, GCC 13, Debug with ASan, UBSan and `_GLIBCXX_ASSERTIONS`: 11 of 11;
- x86-64, Clang 18, Release: 11 of 11;
- i386, GCC 13 (`-m32`): 11 of 11;
- a program using `gaol_ieee1788` alone, compiled with `-std=c++11 -Wall
  -Wextra -pedantic` by GCC and Clang: no warning from the new code;
- with `using namespace std;` as well: everything compiles but the
  unqualified `less`, as expected.

The continuous integration of the branch, on 76f6f7d: the six workflows green,
106 jobs of 106 — Linux 23, Linux containers 17 (i386, armhf, s390x, ppc64le,
riscv64 and musl among them), macOS 11, Windows 27 (Visual Studio 2022 and 2026
on x86, x64 and arm64, MinGW-w64, MSYS2), Autotools and meson 27, and Manual 1,
the manual being built by both builds.

## What changes for the programs that use GAOL

- `gaol_ieee1788::sin(0.5)`, and the other functions of GAOL's names called
  qualified with numbers only, no longer compile: their templates take an
  interval. `gaol_ieee1788::sin(interval(0.5))` is to be written. They
  compiled before only when `gaol/gaol_expression.h` did not come first.
- `pow(x, 2)` and `pow(x, 0.5)` under `using namespace gaol_ieee1788;` are
  the pow of the standard; `pown(x, 2)` is the integer power.
- `gaol::pow` is a template: `&gaol::pow` with an explicit function type
  still works; the library keeps the former symbols.

## Left as it was

- `pow_hybrid` (GAOL's own `pow`) still gives [−∞, +∞] for a degenerate
  integer exponent beyond the ints, as documented: only the pow of the
  standard was asked to change. The integer power there could use the parity
  of the exponent (even for any double beyond 2^53) on `pow_real` of the
  magnitudes.
- The warnings `-Wdeprecated-copy` of `gaol/gaol_interval.h` (the copy
  assignment of `interval`), which programs compiled with `-Wextra` see, are
  older than this work.
