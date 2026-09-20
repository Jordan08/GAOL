<!-- Copyright (c) 2026 ENSTA, France
     Created 2026-09-20 by Jordan NININ -->
# The math libraries: libm, mathlib, CRlibm and CORE-MATH

*Not listed in the documentation index of the [README](../README.md#documentation):
a note kept for reference.*

GAOL bounds its elementary functions with one of three math libraries —
mathlib by default, CRlibm, or the math library of the system (see
[Building GAOL](building.md)) — and carries the sources of a fourth,
CORE-MATH, in `gaol/core_math_*.c`. The four come from four moments of the
same history and were written with different goals, which is why they give
bounds of different tightness. This page says where each of them comes from
and what separates them.

## What they all answer: the table maker's dilemma

A math library evaluates exp, log, sin... in a format, binary64 here, in which
the exact result is not representable: it has to round a value it only knows
approximately. For some arguments the exact result falls extraordinarily close
to the middle of two consecutive doubles, and deciding which way to round then
asks for far more than 53 bits of intermediate precision. There is no bound on
that distance that follows from the definition of the functions: this is the
table maker's dilemma, named by W. Kahan.

Two ambitions follow from it, and they are the axis along which these four
libraries differ:

- **faithful rounding**, or "nearly correct": an error below one unit in the
  last place, or about half of one in practice, which is enough for most
  numerical computation;
- **correct rounding**: returning exactly the double the rounding direction in
  effect gives of the exact result. It makes a program reproducible from one
  machine and one library to another, and, for interval arithmetic, it gives
  the tightest bound a format can hold — a value correctly rounded upward *is*
  the upper bound, with nothing to add.

IEEE 754-2008 lists the correctly rounded elementary functions among its
recommended operations, which is what the last two libraries below set out to
provide.

## libm, the math library of the system

libm is not a library but an interface: the mathematical part of the C
standard library, with one implementation per system.

The line starts with the libm of Unix V7 in the 1970s, then the libm of 4.3BSD
in the middle of the 1980s, written around W. Kahan at Berkeley (Coonen, K. C.
Ng, Thomas, Tang) — where the culture of testing the accuracy of elementary
functions was formed. In 1993 Sun published **fdlibm**, the Freely
Distributable libm of K. C. Ng: portable C, no assembly, aiming at less than
one unit in the last place. fdlibm is the common ancestor of nearly everything
that followed — `msun` of FreeBSD, the libm of musl, openlibm of Julia, and
part of that of the glibc.

Its design puts speed first and accuracy second. One evaluation phase, an
argument reduction and a polynomial or rational approximation chosen by hand
for each function; the error bound announced is empirical, established by
tests rather than proved; the mathematical properties (monotonicity, symmetry,
sin² + cos² ≤ 1) are not guaranteed. Rounding to nearest is assumed
throughout: nothing is promised in the directed rounding directions, which is
precisely what interval arithmetic needs. Beside these portable
implementations live the fast and non-portable ones: Intel SVML, AMD libm,
ARM's `optimized-routines`, and the vector variants.

This is why configure and meson warn when GAOL is built with `--with-mathlib=m`:
the bounds are no longer certified, and hold only where the libm of the system
stays within about one double of the exact value (see
[Accuracy of the operations](accuracy.md#with-crlibm-or-the-math-library-of-the-system)).

## mathlib, the IBM Accurate Portable Mathematical Library (libultim)

Written at IBM Haifa at the end of the 1990s (Abraham Ziv, Moshe Olshansky,
Ealan Henis, Anna Reitman). It is the industrial form of the idea Ziv
published in 1991 in *ACM TOMS*: the onion peeling strategy.

The function is evaluated with increasing precision — a fast phase accurate to
about half a unit in the last place plus a little, then a second one, then a
software multiprecision arithmetic — and the computation stops as soon as a
rounding test proves the rounding can be decided. Since the hard cases are
very rare, the average cost stays close to that of an ordinary libm, while the
result is correctly rounded in practice.

Its two limits are the ones that decided its fate. Nothing bounds the number
of iterations: the strategy has no proof and no bound on the worst case, which
can cost thousands of cycles. And it rounds to nearest only, so a bound drawn
from it has to be moved one double outward, which costs one unit in the last
place of tightness — this is what GAOL does for exp, pow, the trigonometric
and the inverse trigonometric functions ([Accuracy of the
operations](accuracy.md)).

mathlib was contributed to the glibc around 2001 and provided its exp, log,
sin, cos, atan and pow for some fifteen years. In 2018 the glibc 2.28 removed
these multiprecision slow paths: the pathological execution times were judged
worse than the accuracy was worth. The glibc thereby gave up correct rounding
and went back to about half to one unit in the last place with a bounded worst
case. The version GAOL builds is mathlib 2.1.1, under the GNU LGPL, as
Frédéric Goualard distributes it (see [3rd/mathlib](../3rd/README.md#mathlib)).

## CRlibm, the proofs

A project of the LIP at the ENS de Lyon and then of the INRIA Arénaire team
(later AriC), from 2000 to about 2011: Jean-Michel Muller, Florent de
Dinechin, Catherine Daramy-Loirat, David Defour, Christoph Lauter, with
Guillaume Melquiond for the proof tools. Its last release, 1.0beta4, dates
from 2010; it is no longer maintained.

CRlibm keeps Ziv's two phases but changes what is guaranteed:

- **the second phase is bounded.** Lefèvre and Muller had computed, by an
  exhaustive search made tractable by their algorithms, the hardest-to-round
  cases in double precision: about 120 bits are known to be enough. No
  unbounded multiprecision is needed any more — a double-double or
  triple-double arithmetic suffices, and the worst-case execution time is
  bounded, typically a few times the average.
- **the error bounds are proved**, and in part checked by machine with Gappa.
  The documentation of CRlibm is as much a proof document as a manual, and
  this is its main scientific contribution.
- **the four rounding directions are provided explicitly**: `cos_rn`, `cos_rd`,
  `cos_ru`, `cos_rz`. This is what makes it natural for interval arithmetic,
  and why GAOL takes its functions rounded downward and upward directly,
  without moving them outward: built with CRlibm, GAOL's exp, log, sin, cos,
  tan, asin, acos, atan, sinh and cosh are the tightest bounds of the value at
  the bound ([Accuracy of the operations](accuracy.md#with-crlibm-or-the-math-library-of-the-system)).

Its price was the work of a specialist for each function, a narrow coverage (a
dozen functions, binary64 only — neither tanh nor asinh, acosh, atanh), and a
LGPL licence that kept it out of the libm of the systems. Its direct
descendant is Metalibm (de Dinechin, Kupriianova, Lauter): generating the code
and its proof rather than writing them.

## CORE-MATH, correct rounding made adoptable

Started in 2021 at INRIA (Caramba team, Nancy) by Paul Zimmermann — also
behind MPFR — with Alexei Sibidanov, Tom Hubrecht, Sélène Corbineau and
others. It draws the lessons of the three libraries above, and its design is
as much a strategy of adoption as a technique.

- **The stated goal is correct rounding at least as fast as the current
  libms.** It answers the reason mathlib was removed from the glibc: if
  correct rounding costs nothing, the argument from latency falls.
- **The licence is MIT**, deliberately permissive, so that the code can be
  taken up by the glibc, the libm of LLVM or musl. It answers the reason
  CRlibm was not adopted, and it works: routines of CORE-MATH have been
  integrated upstream. It is also what lets GAOL v5 simply copy the
  sources it needs into `gaol/core_math_*.c`.
- **The coverage is wide**: the functions of C23, in binary32, binary64 and
  binary128, the recent functions included.
- **The validation is graded**: in binary32 it is exhaustive — the 2³²
  arguments can be enumerated and compared with MPFR; in binary64 and beyond,
  a proved fast phase and an accurate phase, with a search for the hard cases.
- **The four rounding directions** are honoured, the one in effect included.
  This is what GAOL uses for log: correctly rounded in the upward rounding
  GAOL computes in, CORE-MATH's log gives the tightest bounds without
  switching the rounding direction, twice as fast as mathlib's log moved
  outward (see `gaol/gaol_core_math.h`).

A proposal to add `cr_*` functions to the C standard comes from the same team,
so that correct rounding can be asked for by the program rather than imposed
on everyone.

Beside it, and of an orthogonal design, is RLIBM (Rutgers, Jay Lim and Santosh
Nagarakatte, about 2019-2022): instead of approximating the real function and
then rounding, it looks by linear programming for a polynomial that directly
yields the correct rounding for every input. Elegant, but practicable mainly
for the short formats (binary32, bfloat16, posits).

## What separates them

| | libm (fdlibm, glibc) | mathlib (IBM) | CRlibm | CORE-MATH |
|---|---|---|---|---|
| Time | 1980s, 1993 – | about 1997-2001 | 2000-2011 | 2021 – |
| Where from | Berkeley, Sun | IBM Haifa | ENS de Lyon, INRIA | INRIA, CERN |
| Goal | speed, below 1 ulp | correct rounding in practice | correct rounding **proved** | correct rounding **and speed** |
| Method | one phase | unbounded onion peeling | two phases, the second bounded by the hard cases | proved phases, exhaustive in binary32 |
| Evidence | tests | tests | Gappa, published proofs | proofs and exhaustive checks |
| Worst case | bounded | **unbounded** | bounded | bounded |
| Rounding | to nearest | to nearest | the four, explicit | the four, the one in effect |
| Formats | all | binary64 | binary64 | binary32, 64, 128 |
| Licence | permissive | LGPL | LGPL | **MIT** |
| State | alive | frozen | abandoned | active, being adopted |

The line of the story is plain: **libm** established that a math library had
to be portable and fast; **mathlib** showed that correct rounding was
reachable on average, and was rejected on its worst case; **CRlibm** showed
that it could be proved and bounded, and was not adopted for want of a
licence, of coverage and of maintenance; **CORE-MATH** takes all of it up and
treats industrial adoption — performance, MIT licence, C23 coverage,
standardisation — as a design constraint of the same rank as accuracy.

## What it means for GAOL's bounds

The hierarchy is read directly in the tightness of the bounds, which
[Accuracy of the operations](accuracy.md) states function by function:

- the math library of the system: bounds that are **not certified**, the value
  rounded to nearest moved outward by 2·2<sup>−52</sup> relatively and by the
  smallest normal double, several doubles beyond the tightest one;
- mathlib: correctly rounded to nearest, hence **one double outward**, the
  bounds accurate within one double;
- CRlibm and CORE-MATH: **correctly rounded in the direction wanted**, hence
  the tightest bound of the value at the bound, with nothing to add.

This is also why GAOL v5 takes CORE-MATH's sinh, cosh, tanh, asinh, acosh
and atanh (neither mathlib nor CRlibm has them all, and the libm of the system
is not accurate enough everywhere for its values moved outward to be bounds),
and its log where the compiler has a 128-bit integer type.

## Further reading

- A. Ziv, *Fast evaluation of elementary mathematical functions with correctly
  rounded last bit*, ACM TOMS 17(3), 1991: the onion peeling strategy.
- J.-M. Muller, *Elementary Functions: Algorithms and Implementation*, and the
  *Handbook of Floating-Point Arithmetic*: the table maker's dilemma, the hard
  cases and the two-phase evaluation.
- The documentation distributed with the sources of CRlibm, which holds the
  proofs of its error bounds.
- [CORE-MATH](https://core-math.gitlabpages.inria.fr/), and
  [3rd/README.md](../3rd/README.md) for what GAOL v5 takes from it.
- The NEWS of the glibc 2.28, for the removal of the multiprecision slow paths
  inherited from mathlib.
