<!-- Copyright (c) 2026 ENSTA, France
     Created 2026-09-20 by Jordan NININ -->
# Third-party sources

Part of the documentation of [GAOL v5](../README.md#documentation).

## CORE-MATH

`math-core/` holds the sources of
[CORE-MATH](https://core-math.gitlabpages.inria.fr/) (Alexei Sibidanov, Paul
Zimmermann and others, Inria), which provides mathematical functions that are
correctly rounded: the double each returns is the rounding of the exact value
in the rounding direction in effect. It is distributed under the MIT licence
(`math-core/LICENSE`).

GAOL bounds **every one of its elementary functions** with them, on every
architecture and with every compiler, and they are the only mathematical
library it uses: the other libraries GAOL could be built with are gone (see
[What differs from GAOL](../doc/differences.md)).

| | |
| --- | --- |
| Upstream | <https://gitlab.inria.fr/core-math/core-math> |
| Commit | `6b84457310ad90b644b3335af2faa8c7301e1c47` (25 September 2026) |
| Taken | the whole tree, without the `.wc` files |
| Built | `math-core/src/binary64/<f>/<f>.c` for the thirty-six functions below, compiled into libgaol itself |

The `.wc` files, which hold the hardest-to-round arguments CORE-MATH checks
itself against, are 542 of the 555 MB of the upstream tree and are not needed
to build: they are left out, and the commit above is what to clone to get them.

The thirty-six functions GAOL builds are `exp`, `log`, `pow`, `sin`, `cos`,
`tan`, `asin`, `acos`, `atan`, `atan2`, `sinh`, `cosh`, `tanh`, `asinh`,
`acosh` and `atanh`, then `cbrt`, which `nth_root(x, 3)` takes, `exp2`,
`exp10`, `log2` and `log10`, which IEEE 1788-2015 requires among the forward
elementary functions (Table 9.1), and the fifteen of its recommended ones
(Table 10.5) CORE-MATH has: `expm1`, `exp2m1`, `exp10m1`, `log1p`, `log2p1`,
`log10p1`, `hypot`, `rsqrt`, `sinpi`, `cospi`, `tanpi`, `asinpi`, `acospi`,
`atanpi` and `atan2pi`. The other formats and functions of the tree are kept
as they are, so that importing a newer CORE-MATH is a plain copy, but nothing
compiles them.

### How GAOL builds them

The three builds compile the thirty-six sources into `libgaol` and include
`gaol/core_math_port.h` first in each of them, **by the compiler rather than by
the source** (`-include` with GCC and Clang, `/FI` with Visual C++): the sources
never name that header, so that importing a newer CORE-MATH stays a copy. That
header gives them the names `gaol_cr_<f>()`, so that GAOL does not clash with a
program or a C library holding CORE-MATH's functions too, the 128-bit integer
of `gaol/gaol_u128.h`, what Visual C++ has not of GCC (`__builtin_clzll`,
`__builtin_roundeven`, `__attribute__`...), the `roundeven()` the math library
of Windows has not (`gaol/gaol_roundeven.h`, which GAOL's own sources use too),
given as well for `__builtin_roundeven()` to the compilers that have no such
builtin, GCC before 10 among them: `sin.c` calls it without the guard of the
other sources since its rewrite (upstream commit `6b84457`), and GCC 9.4 did
not link it,
and, on a 32-bit x86 Windows, `fegetexceptflag()` and `fesetexceptflag()`
written on MXCSR: `cbrt`, `pow` and `atan2` keep the exception flags around
their work with them, and the ones of mingw-w64 clear the mask bits of MXCSR as
well, which unmasks the exceptions and makes the first comparison of the NaN
bounds of an empty interval trap. On a 32-bit x86 Windows, with every
compiler, it gives them too the square root of SSE2 for `__builtin_sqrt()`: the
one of the C library, rounded to nearest in every direction, is what Visual C++
calls for it, and GCC where it does not optimize (`-O0`, the Debug builds) and,
at `-O2` too, in the accurate phases the sources mark cold (`pow(x, 0.5)` was
below the root, rounding upward, at subnormal x, and so was GAOL's upper bound
of `pow([x], [0.5])`). With Visual C++, it gives them `NAN` and `INFINITY` read
from their bits, which the UCRT writes for C as products of `1e+300` that
`/fp:strict` computes when the program runs, in the rounding direction in
effect (`-0` and `FLT_MAX` downward and toward zero: `pow` rounded downward was
`-0` from 2<sup>−1075</sup> to about 2<sup>−947</sup>). `tests/core_math.cpp`
checks both.

On x86-64 it also gives them a `fegetround()` that reads MXCSR, the register
that rounds their doubles, rather than the control word of the x87 unit, which
the `fegetround()` of glibc reads. `pow`, and `cos` and `tan`, whose accurate
phases carry the same code, round a subnormal result themselves, in the
direction `fegetround()` gives: with the x87 unit to nearest and MXCSR upward,
the state the `exactinit()` of the predicates of Shewchuk and of Triangle
leaves, `pow` rounded to nearest and its upper bound was below the exact value
for about half of the arguments with a subnormal result. `<fenv.h>` is included
first and the sources see `fegetround` renamed by a macro, which changes
nothing for the rest of GAOL; `tests/rounding_direction.cpp` checks it. On a
32-bit x86 processor, GAOL reads both units and sets both when one is not
upward, so that they never differ when CORE-MATH runs.

They are compiled with the flags of interval arithmetic, as CORE-MATH asks
(`-frounding-math -ffp-contract=off`, `/fp:strict` for Visual C++), and with the
GNU extensions of C (`atan2` uses inline assembly), not with `-std=c99`.

### What differs from upstream

The vendored sources are those of the commit above but for the changes below,
which are marked `/* GAOL */` in the code. They are made in the tree rather than
kept as a patch to reapply.

1. **The 128-bit integer** of the accurate phases of `log`, `sin`, `cos`, `tan`,
   `atan2` and `pow`, and of `log2p1`, `log10p1`, `atan2pi`, `hypot`, `rsqrt`
   and `asinpi`. Upstream writes it `unsigned __int128`, or
   `unsigned _BitInt(128)` with Clang 14 and GCC 14: Visual C++ has neither, on
   no architecture, and neither has GCC for a 32-bit target. In
   `log/dint.h`, `log10/dint.h`, `log2p1/dint_log2p1.h`, `log10p1/dint.h`,
   `pow/dint.h`, `pow/qint.h`, `atan2/tint.h`, `atan2pi/tint.h`, `sin/sin.c`,
   `cos/cos.c`, `tan/tan.c`, `hypot/hypot.c`, `rsqrt/rsqrt.c` and
   `asinpi/asinpi.c`, the lines that choose the type name `gaol_u128` instead,
   which is the type of the compiler where it has one and a structure of two
   64-bit halves where it has none (`gaol/gaol_u128.h`), and the
   extended-arithmetic functions of those files call `gaol_u128_*()` rather
   than the operators of the language. `asinpi.c` computes with a signed
   128-bit integer too, `i128`, which is then a `gaol_u128` read in two's
   complement: its product, its conversion from a 64-bit integer and its shift
   right are `gaol_u128_imul64()`, `gaol_u128_of_i64()` and `gaol_u128_sar()`.
   `log1p/dint.h` is unchanged: `log1p.c` does not include it.

   `sin.c`, rewritten upstream (commit `6b84457`), no longer has the
   extended-arithmetic functions of `dint.h`: its accurate path computes in
   fixed point on the 128-bit integer throughout, the argument reduction
   (`reduce_large()`, `reduce_large_acc()`), the truncated product `mhUU()`,
   the polynomials (`evalPS()`, `evalPC()`), the combination of the tables in
   `sin_large_accurate()` and the rounding to double (`u128_tod()`). Each
   operation on `u128` there is a `gaol_u128_*()` call, and the tables of
   128-bit constants are written with `U128()`, which gives the two halves as
   designated initialisers where the type is the structure.

   There is no way round this: C has no operator overloading, and the files
   cannot be compiled as C++ because their tables initialise anonymous unions
   in a way C++ rejects.

2. **`~0ul` written `~0ull`** in `sinh/sinh.c`, `cosh/cosh.c` and `tanh/tanh.c`
   (nine places). `unsigned long` has 32 bits on Windows, where `~0ul >> 12` is
   not the mask of the 52 low bits of a double. `sinpi/sinpi.c` and
   `log1p/log1p.c`, which GAOL compiles too, have the same masks and are
   unchanged: no result was found to change with a mask of 20 bits, in these
   files or in the three above. The fix of every one of them is
   [proposed to CORE-MATH](#1-masks-written-with-long).

3. **Three signed shifts made unsigned** in `cospi/cospi.c` (the test of an
   integer or half-integer argument). `m` is a signed 64-bit integer holding
   the significand with its hidden bit, 2^52, and shifting it left by 11 to 13
   places overflows: undefined behaviour in C, which UBSan reported in the tests
   run with the sanitizers. `sinpi.c` and `tanpi.c`, which do the same, convert
   `m` to `uint64_t` before shifting it; `cospi.c` now does too, and gives the
   same values bit for bit. Upstream has made the same fix since
   ([commit `b1a4badf`](#already-fixed-upstream-the-shifts-of-cospic)).

4. **A 128-bit shift written on 128 bits** in `asinpi/asinpi.c`
   (`asinpi_acc`). `D`, the 64-bit integer `dc` times 2<sup>ss</sup>, was made
   of its two halves, `dc << ss` and `dc >> (64 - ss)`, which is right for
   0 < ss < 64 only; next to ±1 (1 − |x| about 2<sup>−40</sup>) ss reaches 65
   to 69, and both shifts are undefined behaviour, which UBSan reported on
   the arguments of the accurate phase. It is now
   `gaol_u128_shl(gaol_u128_of_i64(dc), ss)`, the same value for
   0 < ss < 64 and the intended one beyond. On x86-64 the two versions gave
   the same results over 460 000 arguments next to ±1, the part of `D` lost
   being about 2<sup>−54</sup> of a small correction; the regression test is in
   `tests/core_math.cpp`. The fix is
   [proposed to CORE-MATH](#2-the-shift-of-asinpi_acc).

5. **A 64-bit test made a comparison** in `rsqrt/rsqrt.c` (`cr_rsqrt`, the
   subnormal x). `__builtin_expect(ix.u, 1)` passed the 64 bits of x where
   `__builtin_expect` takes a `long`, which has 32 bits on the 32-bit targets
   and on Windows: the high half was dropped, and a subnormal whose low 32
   bits are 0, 2^-1042 to 2^-1024 among the powers of 4, was taken for +0,
   its rsqrt being +oo rather than 2^521 to 2^512 (the continuous integration,
   Debian i386 and armhf, MinGW-w64 and MSYS2; Visual C++, where
   `__builtin_expect(x, y)` is `(x)`, was right). It is now
   `__builtin_expect(ix.u != 0, 1)`. The other `__builtin_expect` of the
   sources GAOL compiles take a comparison, or an integer of 32 bits or less.
   The fix is [proposed to CORE-MATH](#3-the-__builtin_expect-of-rsqrtc).

6. **The rounding direction of `cbrt` with mingw-w64 on x86-64** in
   `cbrt/cbrt.c` (`get_rounding_mode()`). `cr_cbrt()` takes the direction as 0
   to 3 (to nearest, downward, upward, toward zero): it indexes `off[4]` with
   it, and rounds the seven hardest arguments of `wlist` away from zero when it
   is 2 minus the sign. Where `__x86_64__` is defined (GCC and Clang), the
   function computes the `FE_*` value of the direction from MXCSR, for the
   values of glibc, or for those of Windows where `__WIN32__` is defined
   (`FE_UPWARD` 0x200 or 0x100), then maps it to 0 to 3 by a switch; under
   `__WIN32__` with other values, which are those of mingw-w64 (`FE_UPWARD`
   0x800, as glibc), it returned `fegetround()` itself, past the switch. In the
   directed roundings `off[]` was then read kilobytes beyond its four elements,
   and the seven arguments were rounded toward zero: rounding upward,
   `cr_cbrt(0x1.3a9ccd7f022dbp+0)` was `0x1.1236160ba9b93p+0`, below the cube
   root (`0x1.1236160ba9b930000000000001e7e8fap+0`), and `nth_root(x, 3)` did
   not enclose it, with MinGW-w64 and MSYS2 on x64. That `fegetround()` now
   goes through the switch like the other branches. `tests/core_math.cpp`
   checks `cbrt` and `nth_root(x, 3)` at these arguments, scaled by powers of 8
   and on both signs, against mpmath. `rsqrt.c` and `asinpi.c`, whose
   `get_rounding_mode()` has the same branch, compare its result with the
   `FE_*` values themselves, and are right. The fix is
   [proposed to CORE-MATH](#6-the-rounding-field-of-mxcsr-in-cbrtc-rsqrtc-and-asinpic),
   within a broader one.

7. **The rounding direction of `cbrt`, `rsqrt` and `asinpi` with clang-cl on
   x86-64** in `cbrt/cbrt.c`, `rsqrt/rsqrt.c` and `asinpi/asinpi.c`
   (`get_rounding_mode()`). Where `__x86_64__` is defined, the three functions
   make the `FE_*` value of the direction from the rounding field of MXCSR:
   where `__WIN32__` or `__WIN64__` is defined, shifted by 5 for the values of
   the UCRT of Windows (`FE_UPWARD` 0x200), through a table for those of its
   versions before 14393 (`FE_UPWARD` 0x100), and by `fegetround()` for other
   values, the path of mingw-w64 (item 6); otherwise, shifted by 3 for those
   of glibc (`FE_UPWARD` 0x800). clang-cl defines `__x86_64__` and `_WIN32`,
   but neither `__WIN32__` nor `__WIN64__`, and takes the `fenv.h` of the
   UCRT: the three shifted by 3 and compared 0x400, 0x800 and 0xc00 with the
   UCRT's 0x100, 0x200 and 0x300, so that the downward and the upward
   roundings were taken for toward zero. The line testing the two macros now
   tests `_WIN32` as well, which sends clang-cl to the branch of Windows, and
   changes nothing for the other compilers: Visual C++ defines no
   `__x86_64__`, mingw-w64 defines `__WIN32__` already, and Cygwin defines no
   `_WIN32`. The three sources, compiled by clang-cl 18 with the flags of GAOL
   (`/fp:strict /arch:AVX2`) and the values of the `fenv.h` and `float.h` of
   the UCRT, and run under wine over the arguments of patch 6 below, gave 42
   wrong `cbrt` results downward and 42 upward, 1,338 wrong `rsqrt` results
   upward and 499,838 and 526,673 different `asinpi` results downward and
   upward, all those checked with mpmath being wrong: every wrong upward
   result, at a positive argument, below the exact value, and every wrong
   downward one, at a negative argument, above it; with the change, the
   results of x86-64 Linux. The bound of `nth_root(x, 3)` farther from zero
   (the root of a negative number being the opposite of the upward root of its
   magnitude) and the upper bounds of `rsqrt` and `asinpi` did not enclose the
   exact values there: `tests/core_math.cpp` checks `cbrt` at the arguments of
   item 6, `asinpi` next to ±1 and `rsqrt` at the successors of the powers of
   4, which failed so (the same checks, run on the three objects under wine:
   70 of 70, 22 of 22 and 1,023 of 1,023), and the continuous integration
   builds and tests GAOL with clang-cl on x64. The fix is
   [proposed to CORE-MATH](#6-the-rounding-field-of-mxcsr-in-cbrtc-rsqrtc-and-asinpic),
   within a broader one.

8. **The exact and midpoint powers of `pow` on 32-bit ARM** in `pow/pow.c`
   (`exact_pow()`). Where its accurate phase cannot round, `pow` computes an
   exact or midpoint power, c<sup>a</sup>·2<sup>s</sup>, as an integer of at
   most 54 bits, and converts it with `(double)`, which has to round in the
   direction in effect. On 32-bit ARM, VFP has no conversion of a 64-bit
   integer: GCC calls `__aeabi_l2d` of libgcc, integer code that rounds to
   nearest whatever FPSCR says. A midpoint, c<sup>a</sup> odd of 54 bits, was
   then rounded to nearest in the directed roundings too: rounding upward,
   `cr_pow(0x1.8p-29, 34)`, 3<sup>34</sup>·2<sup>−1020</sup>, was
   `0x1.d9fe779881944p-967`, below the power, and the upper bound of
   `pow([0x1.8p-29], [34, 35])` did not enclose it (the continuous
   integration, Debian armhf). Only powers below about 2<sup>−950</sup> reach
   `exact_pow()`; the first phases round the others. The integer is now
   converted as its two 32-bit halves, whose conversions and product by
   2<sup>32</sup> are exact, and one addition, which rounds in the direction
   in effect on every processor (`i64_to_double()`). Over 13.3 million pairs
   and 3,950 midpoints, against MPFR, the vendored `pow` gave 415 wrong
   results downward and toward zero and 631 upward on armhf (GCC 13.3 under
   qemu-arm), and the changed one none; on x86-64 the two give the same bits
   in the four directions. `tests/core_math.cpp` checks five such midpoints
   against their exact values. The fix is
   [proposed to CORE-MATH](#7-the-exact-and-midpoint-powers-of-pow-on-32-bit-arm).

9. **Shifts of a 64-bit result written on 64 bits** in `cos/cos.c`, `tan/tan.c`
   and `hypot/hypot.c`, which the warnings of Visual C++ at `/W4` (C4334)
   point at, in the job of the continuous integration that compiles GAOL with
   `/W4 /WX`. In `cos.c` and `tan.c`, `1l << (a->ex + 1073)` is written
   `1ll << ...`: the shift goes up to 51 places, undefined where `long` has 32
   bits, as on Windows, though no argument reaches it (see the
   [proposed fix](#1-masks-written-with-long), which writes it so). In
   `hypot.c`, the two `1 << ...` subtracted from or masking a `u64` are
   written `1ull << ...`, with no change of value: their shifts stay below 32.
   The other warnings of `/W4` on these sources, of what they write on
   purpose, are turned off for them alone (`CMakeLists.txt`).

### How the changes are checked

The changes touch the arithmetic of the accurate phases, so they are checked by
comparison rather than by reading:

- **against the upstream sources.** Each of the functions whose files changed, compiled from
  this tree, was compared with the same function compiled from the pristine
  upstream commit, over about 40 million arguments in the four rounding
  directions, including the hard paths (large arguments reduced modulo π/2,
  bases next to 1, exact results): the two give the same bits everywhere. This
  found a real bug the first time, an addition of two 64-bit halves that has to
  be made on 128 bits;
- **on the paths that are seldom taken.** The 128-bit code of `log2p1`,
  `log10p1`, `atan2pi`, `hypot`, `rsqrt` and `asinpi` only runs when their
  fast phase cannot round, which random arguments seldom make happen. They
  were therefore also compared over arguments collected because they reach it
  (6,077 for `log2p1`, 4,503 for `log10p1`, 133,085 for `asinpi`, Pythagorean
  triples for `hypot`, the powers of 4 and their neighbours for `rsqrt`), and
  the functions of `atan2pi/tint.h` and the accurate phase of `asinpi` were
  called directly, from upstream and from this tree, including the branches no
  argument reaches in practice (a subtraction cancelling 64 bits or more,
  |x| < 0.0131875 in `asinpi_acc`). Each comparison was shown able to fail: a
  fault put in the ported code for a moment (a shift count, a dropped carry, a
  signed shift made unsigned) makes it report thousands of differences;
- **the rewritten `sin.c`** (commit `6b84457`), ported with the native type and
  with the two halves forced, against the upstream file compiled unchanged,
  with Clang 18 and GCC 9.4: the same bits in the four rounding directions
  over 102.5 million arguments of `cr_sin()` (random bit patterns, [−π, π],
  2^-26 to 2^31, beyond 2^31, the doubles next to 2.5 million multiples of π),
  over 10 million calls of `sin_large_accurate()` itself, from 2^-16 to the
  largest double, and over 10 million of each argument reduction and
  40 million products `mhUU()`. A fault put in the port (a term of `mhUU()`
  dropped, a shift count, the halves of the tables swapped) makes it report
  from 400 000 to 3 million differences over a hundredth of these arguments. `tests/core_math.cpp` checks 19
  arguments that take the accurate path in the upward rounding against the
  values of the upstream file, in the four directions;
- **against the native type.** `tests/u128.cpp` compares every operation of the
  two halves with the same operation on `unsigned __int128` and `__int128`, over
  8 million values and every shift count, and the whole of GAOL is built and
  tested with the halves forced (`-DGAOL_U128_EMULATION=ON`, a job of the
  continuous integration, which checks that every source of CORE-MATH is
  compiled with them) so that the code Visual C++ and the 32-bit targets take
  is run at each change;
- **against the tightest bounds.** `tests/core_math.cpp` compares the bounds
  GAOL computes with what CORE-MATH gives in the downward and the upward
  rounding, which are the tightest bounds there are, and those of `pow` at the
  corners of a box with the power itself, computed exactly, where it is
  rational, checking CORE-MATH's two values against it too (item 8).

### To update CORE-MATH

Copy the upstream tree again without the `.wc` files, then make the eight
changes above. `git diff` against the previous version shows them: they are
marked `/* GAOL */`, and no other line differs. A change upstream has made in
the meantime is not made again: that of `cospi.c` from commit `b1a4badf` on,
and those of the [patches below](#changes-to-propose-upstream) once upstream
takes them.

## Changes to propose upstream

Five of the [changes above](#what-differs-from-upstream) are fixes rather than
adaptations to GAOL (2 to 5, and 8), and `gaol/core_math_port.h` makes up for
a sixth defect, the `__builtin_roundeven()` of `sin.c` (see
[How GAOL builds them](#how-gaol-builds-them)). They are written below as
patches against the current upstream sources, with three more found while
preparing them or since: the masks of item 2 in three other files, the rounding
direction `pow` reads for its subnormal results, and the one `cbrt.c`,
`rsqrt.c` and `asinpi.c` read with mingw-w64, with clang-cl and on Cygwin,
which the smaller changes 6 and 7 above fix in GAOL's copy for the first two.
Through CORE-MATH they would reach glibc too, which takes functions from it
since version 2.41, so each defect was looked for there as well.

**Nothing has been sent**, neither to CORE-MATH nor to glibc: sending them is
for the maintainer of GAOL v5, as said in each part below.

| Defect | CORE-MATH master, `b1a4badf` | glibc master (`30f988f`), 2.41 to 2.44 |
| --- | --- | --- |
| [masks of `long`](#1-masks-written-with-long) | present | absent: its copies write `MANTISSA_MASK` or `~UINT64_C(0) >> 12` |
| [shift of `asinpi_acc()`](#2-the-shift-of-asinpi_acc) | present | no `asinpi` of doubles from CORE-MATH |
| [`__builtin_expect` of `rsqrt.c`](#3-the-__builtin_expect-of-rsqrtc) | present | no `rsqrt` of CORE-MATH |
| [`__builtin_roundeven` of `sin.c`](#4-the-__builtin_roundeven-of-sinc) | present | no `sin` of CORE-MATH |
| [rounding direction of `pow`](#5-the-rounding-direction-of-pow) | present | no `pow` of CORE-MATH; the x87 unit by design |
| [rounding field of MXCSR in `cbrt.c`, `rsqrt.c`, `asinpi.c`](#6-the-rounding-field-of-mxcsr-in-cbrtc-rsqrtc-and-asinpic) | present | absent: its `cbrt` maps glibc's own `FE_*` values with a switch |
| [exact and midpoint powers of `pow` on 32-bit ARM](#7-the-exact-and-midpoint-powers-of-pow-on-32-bit-arm) | present | no `pow` of CORE-MATH |
| [signed shifts of `cospi.c`](#already-fixed-upstream-the-shifts-of-cospic) | fixed by `b1a4badf` | no `cospi` of doubles from CORE-MATH |

### CORE-MATH

The patches are a series of seven commits on upstream master,
`b1a4badf6765d761873ee31f7419d1cc1cac19f8` (29 September 2026), in this order,
each written in the style of the file it changes, with the subject given; the
diff blocks below are what `git diff` prints for each, but for the blank lines
of context, written without their leading space, and `git apply` or
`patch -p1` take them as they are there (line numbers below are those of
`b1a4badf`). Each block also applies alone to `b1a4badf`, but the `index`
lines of patch 6 are those of the files patches 2 and 3 leave, which
`git am` and `git apply --3way` rely on. To send them: a
merge request on <https://gitlab.inria.fr/core-math/core-math>, or the output
of `git format-patch` mailed to core-math@inria.fr, the address the sources
give for reports, with the defect, the reproduction and the checks of each
part.

They were compiled with GCC 13.3 and Clang 18 on x86-64 Linux, and with
MinGW-w64 11 (GCC 13, and Clang 18 for patches 5 and 6) for Windows x64, whose
`long` has 32 bits, the programs being run under wine and compiled with
`-mfma`: CORE-MATH needs a correctly rounded `fma()`, which the instruction of
the processor is and the `fma()` of the `libmingwex.a` of mingw-w64 11, linked
into the programs, is not. With
MPFR 4.2.1, CORE-MATH's own checks pass on the patched tree:
`./check.sh --worst`, which checks every argument of the `.wc` file in the
four rounding directions, for every function a patch touches, and
`./check.sh --special` with `CORE_MATH_TESTS=2000000` random arguments rather
than the default, for all of them but `sincos` and `lgamma`, whose object code
the patch does not change on x86-64 Linux (the random checks of `sincos` alone
take more than an hour). The one exception is `pow` in the downward direction,
where the worst-case check stops at a spurious underflow exception
(`x = -0x1.10a688680a753p-93`, `y = 11`, a result of −2<sup>−1022</sup>, an
argument added upstream on 22 September) that master raises as well: over the
whole of `pow.wc` in the four directions, the patched `pow` gives the results
and the exception flags of master. Patch 7, written later, was checked on
32-bit ARM and on x86-64, as its part says.

#### 1. Masks written with `long`

**The defect.** `~0ul>>12` is meant as the mask of the 52 low bits of a double,
but `unsigned long` has 32 bits on Windows and on the 32-bit targets, where the
mask is `0xfffff`: in `sinh.c` (lines 113, 353, 413), `cosh.c` (107, 115, 314,
374), `tanh.c` (136, 385), `sinpi.c` (99, 109), `log1p.c` (483) and `lgamma.c`
(753). Elsewhere upstream writes `~0ull`. The tests these masks make become
weaker rather than wrong: where 52 bits decide that the fast result may be hard
to round (its low part a power of 2 or next to one), 20 bits decide it more
often, and the code then takes a path that is right anyway, a look-up in the
table of hard cases or a nudge of the low part by one of its own ulps, which
cannot move the sum across a rounding boundary (at line 99 of `sinpi.c`, the
mask builds a tiny constant whose value changes but not its effect). So no
result changes (see below), but the code does not do what it says. The same
assumption is in the `1l << (a->ex + 1073)` of `cos.c` (401), `tan.c` (530) and
`sincos.c` (392), a shift by up to 51 places, undefined where `long` has 32
bits; no argument reaches it, since their accurate phases never give a
subnormal, and the patch writes it `1ll`, as `pow.h` does. The `~0ul` of
`binary80/atan2/atan2l.c` and `binary128/expm1/expm1q.c` were not examined.
For the authors: at line 753 of `lgamma.c` the masked value goes into an
`unsigned`, so that the test there sees 32 bits with the patch, as master does
where `long` has 64 bits, and never 52; if 52 are meant, `ft` wants the type
`uint64_t`.

The patch, to commit as
`[binary64] do not assume that long has 64 bits`:

```diff
diff --git a/src/binary64/cos/cos.c b/src/binary64/cos/cos.c
index 9a222dab..c4506539 100644
--- a/src/binary64/cos/cos.c
+++ b/src/binary64/cos/cos.c
@@ -398,7 +398,7 @@ static inline double dint_tod(dint64_t *a) {
         e.f = 0x0.0000000000001p-1022;
       }
     } else {
-      e.u = 1l << (a->ex + 1073);
+      e.u = 1ll << (a->ex + 1073);
     }
   }

diff --git a/src/binary64/cosh/cosh.c b/src/binary64/cosh/cosh.c
index b5e31cba..e97f623f 100644
--- a/src/binary64/cosh/cosh.c
+++ b/src/binary64/cosh/cosh.c
@@ -104,7 +104,7 @@ static double __attribute__((noinline)) as_cosh_zero(double x){
   double y0 = fasttwosum(1.0, y1, &y1);
   y1 = fasttwosum(y1, y2, &y2);
   b64u64_u t = {.f = y1};
-  if(__builtin_expect(!(t.u&(~0ul>>12)), 0)){
+  if(__builtin_expect(!(t.u&(~0ull>>12)), 0)){
     b64u64_u w = {.f = y2};
     if((w.u^t.u)>>63)
       t.u--;
@@ -112,7 +112,7 @@ static double __attribute__((noinline)) as_cosh_zero(double x){
       t.u++;
     y1 = t.f;
   }
-  if(__builtin_expect((t.u&(~0ul>>12))==(~0ul>>12), 0)) return as_cosh_database(x, y0 + y1);
+  if(__builtin_expect((t.u&(~0ull>>12))==(~0ull>>12), 0)) return as_cosh_database(x, y0 + y1);
   return y0 + y1;
 }

@@ -311,7 +311,7 @@ double cr_cosh(double x){
       th = as_exp_accurate(ax, t, th, tl, &tl);
       th = fasttwosum(th, tl, &tl);
       b64u64_u uh = {.f = th}, ul = {.f = tl};
-      int64_t eh = (uh.u>>52)&0x7ff, el = (ul.u>>52)&0x7ff, ml = (ul.u + 8)&(~0ul>>12);
+      int64_t eh = (uh.u>>52)&0x7ff, el = (ul.u>>52)&0x7ff, ml = (ul.u + 8)&(~0ull>>12);
       th += tl;
       th *= 2;
       th *= sp.f;
@@ -371,7 +371,7 @@ double cr_cosh(double x){
   }
   rh = fasttwosum(rh, rl, &rl);
   b64u64_u uh = {.f = rh}, ul = {.f = rl};
-  int64_t eh = (uh.u>>52)&0x7ff, el = (ul.u>>52)&0x7ff, ml = (ul.u + 8)&(~0ul>>12);
+  int64_t eh = (uh.u>>52)&0x7ff, el = (ul.u>>52)&0x7ff, ml = (ul.u + 8)&(~0ull>>12);
   rh += rl;
   if(__builtin_expect(ml<=16 || eh-el>103,0)) return as_cosh_database(x, rh);
   return rh;
diff --git a/src/binary64/lgamma/lgamma.c b/src/binary64/lgamma/lgamma.c
index 155ae62f..32bbf3d0 100644
--- a/src/binary64/lgamma/lgamma.c
+++ b/src/binary64/lgamma/lgamma.c
@@ -750,7 +750,7 @@ static __attribute__((noinline)) double as_lgamma_accurate(double x){
   }

   b64u64_u tl = {.f = fl};
-  unsigned ft = (tl.u+2)&(~0ul>>12);
+  unsigned ft = (tl.u+2)&(~0ull>>12);
   if(ft <= 2u) return as_lgamma_database(sx, fh + fl);
   return fh + fl;
 }
diff --git a/src/binary64/log1p/log1p.c b/src/binary64/log1p/log1p.c
index e45d2d1b..e55f1e41 100644
--- a/src/binary64/log1p/log1p.c
+++ b/src/binary64/log1p/log1p.c
@@ -480,7 +480,7 @@ static double __attribute__((noinline)) as_log1p_refine(double x, double a){
   ln21 = fasttwosum(ln21,ln20, &ln20);

   b64u64_u t = {.f = ln21};
-  if(__builtin_expect(!(t.u&(~0ul>>12)), 0)){
+  if(__builtin_expect(!(t.u&(~0ull>>12)), 0)){
     b64u64_u w = {.f = ln20};
     if((w.u^t.u)>>63)
       t.u--;
diff --git a/src/binary64/sincos/sincos.c b/src/binary64/sincos/sincos.c
index 10ba965a..29c425c9 100644
--- a/src/binary64/sincos/sincos.c
+++ b/src/binary64/sincos/sincos.c
@@ -389,7 +389,7 @@ static inline double dint_tod(dint64_t *a) {
         e.f = 0x0.0000000000001p-1022;
       }
     } else {
-      e.u = 1l << (a->ex + 1073);
+      e.u = 1ll << (a->ex + 1073);
     }
   }

diff --git a/src/binary64/sinh/sinh.c b/src/binary64/sinh/sinh.c
index e9b24d45..98526d40 100644
--- a/src/binary64/sinh/sinh.c
+++ b/src/binary64/sinh/sinh.c
@@ -110,7 +110,7 @@ static double __attribute__((noinline)) as_sinh_zero(double x){
   double y0 = fasttwosum(x, y1, &y1);
   y1 = fasttwosum(y1, y2, &y2);
   b64u64_u t = {.f = y1};
-  if(__builtin_expect(!(t.u&(~0ul>>12)), 0)){
+  if(__builtin_expect(!(t.u&(~0ull>>12)), 0)){
     b64u64_u w = {.f = y2};
     if((w.u^t.u)>>63)
       t.u--;
@@ -350,7 +350,7 @@ double cr_sinh(double x){
       th *= __builtin_copysign(1, x);
       tl *= __builtin_copysign(1, x);
       b64u64_u uh = {.f = th}, ul = {.f = tl};
-      int64_t eh = (uh.u>>52)&0x7ff, el = (ul.u>>52)&0x7ff, ml = (ul.u + 8)&(~0ul>>12);
+      int64_t eh = (uh.u>>52)&0x7ff, el = (ul.u>>52)&0x7ff, ml = (ul.u + 8)&(~0ull>>12);
       th += tl;
       th *= 2;
       th *= sp.f;
@@ -410,7 +410,7 @@ double cr_sinh(double x){
   }
   rh = fasttwosum(rh, rl, &rl);
   b64u64_u uh = {.f = rh}, ul = {.f = rl};
-  int64_t eh = (uh.u>>52)&0x7ff, el = (ul.u>>52)&0x7ff, ml = (ul.u + 8)&(~0ul>>12);
+  int64_t eh = (uh.u>>52)&0x7ff, el = (ul.u>>52)&0x7ff, ml = (ul.u + 8)&(~0ull>>12);
   rh *= __builtin_copysign(1, x);
   rl *= __builtin_copysign(1, x);
   rh += rl;
diff --git a/src/binary64/sinpi/sinpi.c b/src/binary64/sinpi/sinpi.c
index bc72ea9f..fe8ab9f5 100644
--- a/src/binary64/sinpi/sinpi.c
+++ b/src/binary64/sinpi/sinpi.c
@@ -96,7 +96,7 @@ static double as_sinpi_zero(double x){
   const double pi0 = 0x1.92p+1, pi1 = 0x1.fb54442d1846ap-11, pi2 = -0x1.d9cceba3f91f2p-65;
   double y0 = pi0*x;
   b64u64_u b = {.f = y0};
-  b.u &= ~0ul>>12;
+  b.u &= ~0ull>>12;
   b.u += (int64_t)85<<51;
   y0 = (y0 + b.f) - b.f;
   double y0l = __builtin_fma(pi0,x,-y0);
@@ -106,7 +106,7 @@ static double as_sinpi_zero(double x){
   y0 = fasttwosum(y0,y1, &y1);
   y1 = fasttwosum(y1,y2, &y2);
   b64u64_u t = {.f = y1};
-  if(__builtin_expect(!(t.u&(~0ul>>12)), 0)){
+  if(__builtin_expect(!(t.u&(~0ull>>12)), 0)){
     b64u64_u w = {.f = y2};
     if((w.u^t.u)>>63)
       t.u--;
diff --git a/src/binary64/tan/tan.c b/src/binary64/tan/tan.c
index fea7d730..041bb8c9 100644
--- a/src/binary64/tan/tan.c
+++ b/src/binary64/tan/tan.c
@@ -527,7 +527,7 @@ static inline double dint_tod(dint64_t *a) {
         e.f = 0x0.0000000000001p-1022;
       }
     } else {
-      e.u = 1l << (a->ex + 1073);
+      e.u = 1ll << (a->ex + 1073);
     }
   }

diff --git a/src/binary64/tanh/tanh.c b/src/binary64/tanh/tanh.c
index 9aea80b6..6b7ae8ee 100644
--- a/src/binary64/tanh/tanh.c
+++ b/src/binary64/tanh/tanh.c
@@ -133,7 +133,7 @@ static double __attribute__((noinline)) as_tanh_zero(double x){ // |x|<0.25
     double y0 = fasttwosum(x, y1, &y1);
     y1 = fasttwosum(y1, y2, &y2);
     b64u64_u t = {.f = y1};
-    if (__builtin_expect(!(t.u & (~0ul >> 12)), 0)) {
+    if (__builtin_expect(!(t.u & (~0ull >> 12)), 0)) {
         b64u64_u w = {.f = y2};
         if ((w.u ^ t.u) >> 63)
             t.u--;
@@ -382,6 +382,6 @@ double cr_tanh(double x){
   rh = fasttwosum(rh, rl, &rl);
   double res = __builtin_copysign(2,x)*rh + __builtin_copysign(2,x)*rl;
   b64u64_u lu = {.f = rl};
-  if(__builtin_expect(((lu.u+32)&(~0ul>>12))<65, 0)) return as_tanh_database(x, res);
+  if(__builtin_expect(((lu.u+32)&(~0ull>>12))<65, 0)) return as_tanh_database(x, res);
   return res;
 }
```

**How it was checked.** GCC 13 makes the same object code of the nine files
before and after on x86-64 Linux, where `long` has 64 bits. With MinGW-w64 for
Windows x64, `sinh`, `cosh`, `tanh`, `sinpi`, `log1p` and `lgamma` compiled
from master and from the patched tree give the results of x86-64 Linux, bit for
bit, in the four directions, over the arguments of their `.wc` files and a
million random ones each. On random arguments, the masks of a 32-bit `long`
made `sinh`, `cosh` and `tanh` look a hard case up 5, 11 and 1 times in
4 million calls, against never with 64 bits.

**Status.** Present in master; GAOL makes the change in `sinh.c`, `cosh.c` and
`tanh.c` only (item 2 above).

#### 2. The shift of `asinpi_acc()`

**The defect.** In `asinpi_acc()`, line 273,
`D = {.bl = (u64)dc << ss, .bh = (u64)(dc>>(64-ss))}` is dc·2<sup>ss</sup> on
128 bits for 0 < ss < 64 only, but ss = 24 + `ixe` − `ce` grows as |x| nears 1,
to 76 for 1 − |x| = 2<sup>−53</sup>, and passes 63 as soon as 1 − |x| is below
about 2<sup>−39</sup>: both shifts are then undefined behaviour. UBSan (GCC 13,
`-fsanitize=undefined`) stops on it for `cr_asinpi(0x1.ffffffffff0c7p-1)` in any
rounding direction (`asinpi.c:273:26: runtime error: shift exponent 65 is too
large for 64-bit type 'long unsigned int'`). On x86-64, which takes shift
counts modulo 64, `D` is then about dc·2<sup>ss−64</sup>, a correction
2<sup>64</sup> times too small, but a small one: on the arguments below, the
results do not change. The patch shifts on 128 bits:
`(u128)dc` extends the sign of `dc`, and the shift is defined up to 127
places.

The patch, to commit as
`[asinpi] shift dc on 128 bits: ss exceeds 63 next to +-1`:

```diff
diff --git a/src/binary64/asinpi/asinpi.c b/src/binary64/asinpi/asinpi.c
index 226a7fd8..ce089f18 100644
--- a/src/binary64/asinpi/asinpi.c
+++ b/src/binary64/asinpi/asinpi.c
@@ -269,8 +269,8 @@ static double asinpi_acc(double x){
     sm2.a += dsm3.a;
     int k = ixe-ce;
     ss = 24 + k;
-    u128_u Cm = {.bl = 0, .bh = cm},
-      D = {.bl = (u64)dc << ss, .bh = (u64)(dc>>(64-ss))};
+    // 25 <= ss <= 76: ss > 63 next to +-1, so shift on 128 bits
+    u128_u Cm = {.bl = 0, .bh = cm}, D = {.a = (u128)dc << ss};
     Cm.a -= D.a;
     h = sm2.a>>14;
     dc = mh(h, ixm);
```

**How it was checked.** `cr_asinpi()` from master and from the patched tree,
with GCC and with Clang, give the same results over a million arguments next to
±1 (1 − |x| from 2<sup>−53</sup> to 2<sup>−30</sup>) in the four directions.
Over 3.9 million more (1 − |x| from 2<sup>−53</sup> to 2<sup>−1</sup>) and the
8,193 doubles nearest to √(1 − 4<sup>−j</sup>) for j = 1 to 27, 19,041 calls
took `asinpi_acc()` in one of the four directions, down to
1 − |x| ≈ 2<sup>−46</sup>, 2,172 of them with ss > 63: master and the patch
give the same results there, all the correct rounding (mpmath, 300 bits).
UBSan reports nothing on the patched file. For the authors:
`asinpi_acc()` called directly, which `cr_asinpi()` does not do there, on
x = ±0x1.fffffffffffffp-1 in the downward and toward-zero directions, finds
c = √(1 − x²) rounded just below 2<sup>−26</sup>, `dc` = −2<sup>51</sup> and
ss = 76, so that `Cm.a - D.a` wraps around 2<sup>128</sup> and the patched
function returns a wrong result where master happens to return the right one;
`cr_asinpi()` rounds these arguments in its fast phase, in every direction, but
the bound on ss the proof relies on may deserve a look.

**Status.** Present in master; GAOL computes the same 128-bit shift with
`gaol_u128_shl(gaol_u128_of_i64(dc), ss)` (item 4 above).

#### 3. The `__builtin_expect` of `rsqrt.c`

**The defect.** In `cr_rsqrt()`, line 172, `__builtin_expect(ix.u, 1)` tests
x ≠ +0 for a subnormal x, but the builtin takes a `long`: where `long` has
32 bits, the test sees the low 32 bits of x only, and the subnormals
m·2<sup>−1042</sup>, 1 ≤ m < 2<sup>20</sup>, are taken for +0: their `rsqrt`
is +∞, with the divide-by-zero exception, rather than 2<sup>521</sup> and
below. `exp2.c` writes `frac != 0` for the same reason (line 362, "on 32-bit
machines, `__builtin_expect(frac,1)` does not work"), and so does the patch.

The patch, to commit as
`[rsqrt] pass __builtin_expect a comparison, not 64 bits`:

```diff
diff --git a/src/binary64/rsqrt/rsqrt.c b/src/binary64/rsqrt/rsqrt.c
index 43ffbd18..6bc8cf64 100644
--- a/src/binary64/rsqrt/rsqrt.c
+++ b/src/binary64/rsqrt/rsqrt.c
@@ -169,7 +169,7 @@ double cr_rsqrt(double x){
   b64u64_u ix = {.f = x};
   double r;
   if(__builtin_expect(ix.u < 1ll<<52, 0)){ // 0 <= x < 0x1p-1022
-    if(__builtin_expect(ix.u, 1)){ // x <> +0
+    if(__builtin_expect(ix.u != 0, 1)){ // x <> +0
       r = __builtin_sqrt(x)/x;
     } else {
 #ifdef CORE_MATH_SUPPORT_ERRNO
```

**How it was checked.** With MinGW-w64 for Windows x64 (a 32-bit `long`, and
`__int128`), master returns +∞ in the four directions for all the 149,797
subnormals tried whose low 32 bits are 0 (every seventh m); the patched
`rsqrt` returns the results of x86-64 Linux for them and for 100,000 random
subnormals. GCC 13 makes the same object code before and after on x86-64
Linux. GAOL's continuous integration found it on Debian i386 and armhf, with
MinGW-w64 and with MSYS2 (item 5 above).

**Status.** Present in master; GAOL makes the same change.

#### 4. The `__builtin_roundeven` of `sin.c`

**The defect.** Since the rewrite of `sin.c` (merged in `6b84457`),
`cr_sin_moderate()`, line 829, calls `__builtin_roundeven()` unconditionally.
GCC has it from version 10 only (<https://gcc.gnu.org/gcc-10/changes.html>),
and GCC 9.4 did not link the call. The other sources that round to an integer
(`exp.c`, `exp2.c`, `expm1.c`, `acos.c`, `erfc.c`, `tgamma.c`...) take the
builtin from GCC 10 and Clang 17 only, and call `roundeven_finite()` otherwise:
the patch copies that block of `exp.c` into `sin.c` and calls it.

The patch, to commit as
`[sin] guard __builtin_roundeven as exp.c does`:

```diff
diff --git a/src/binary64/sin/sin.c b/src/binary64/sin/sin.c
index 8e78fdc7..35fa7c0c 100644
--- a/src/binary64/sin/sin.c
+++ b/src/binary64/sin/sin.c
@@ -38,6 +38,43 @@ SOFTWARE.

 #pragma STDC FENV_ACCESS ON

+/* __builtin_roundeven was introduced in gcc 10:
+   https://gcc.gnu.org/gcc-10/changes.html,
+   and in clang 17 */
+#if ((defined(__GNUC__) && __GNUC__ >= 10) || (defined(__clang__) && __clang_major__ >= 17)) && !defined(_MSC_VER) && (defined(__aarch64__) || defined(__x86_64__) || defined(__i386__))
+# define roundeven_finite(x) __builtin_roundeven (x)
+#else
+/* round x to nearest integer, breaking ties to even */
+static double
+roundeven_finite (double x)
+{
+  double ix;
+# if (defined(__GNUC__) || defined(__clang__)) && (defined(__AVX__) || defined(__SSE4_1__) || (__ARM_ARCH >= 8))
+#  if defined __AVX__
+   __asm__("vroundsd $0x8,%1,%1,%0":"=x"(ix):"x"(x));
+#  elif __ARM_ARCH >= 8
+   __asm__ ("frintn %d0, %d1":"=w"(ix):"w"(x));
+#  else /* __SSE4_1__ */
+   __asm__("roundsd $0x8,%1,%0":"=x"(ix):"x"(x));
+#  endif
+# else
+  ix = __builtin_round (x); /* nearest, away from 0 */
+  if (__builtin_fabs (ix - x) == 0.5)
+  {
+    /* if ix is odd, we should return ix-1 if x>0, and ix+1 if x<0 */
+    union { double f; uint64_t n; } u, v;
+    u.f = ix;
+    v.f = ix - __builtin_copysign (1.0, x);
+    /* Warning: v.n is 0 when x=0.5; while u.n cannot be zero since ix
+       is rounded away from zero. */
+    if (v.n == 0 || __builtin_ctzll (v.n) > __builtin_ctzll (u.n))
+      ix = v.f;
+  }
+# endif
+  return ix;
+}
+#endif
+
 #if (defined(__clang__) && __clang_major__ >= 14) || (defined(__GNUC__) && __GNUC__ >= 14 && __BITINT_MAXWIDTH__ && __BITINT_MAXWIDTH__ >= 128)
 typedef unsigned _BitInt(128) u128;
 #else
@@ -826,7 +863,7 @@ cr_sin_moderate (double x, int sbit)
   double ax = __builtin_fabs(x);
   static const double invpi = 0x1.45f306dc9c883p+12;
   // |invpi/2^14 - 1/pi| < 2^-55.496
-  double k = __builtin_roundeven (invpi * ax);
+  double k = roundeven_finite (invpi * ax);
   // |2^14*(pih + pil) + pi| < 2^-108.041
   double rh = __builtin_fma (k, pih, ax), rl = k * pil; // rh is exact

```

**How it was checked.** GCC 13, which has the builtin, makes the same object
code before and after on x86-64 Linux. With the condition made false, so that
`roundeven_finite()` is the function in its three forms (`vroundsd` with AVX,
`roundsd` with SSE 4.1, and the portable code), `cr_sin()` gives the results of
master over 3 million arguments (any bit pattern, and 2<sup>−26</sup> to
2<sup>31</sup>, where `cr_sin_moderate()` works) in the four directions, and
`roundeven_finite()` gives those of `__builtin_roundeven()` over 16 million
values, half of them halfway between two integers. No compiler without the
builtin was at hand.

**Status.** Present in master; GAOL gives the sources a `__builtin_roundeven()`
where the compiler has none, in `gaol/core_math_port.h`.

#### 5. The rounding direction of `pow`

**The defect.** `dint_tod_subnormal()` and `subnormalize_qint()` of `pow.h`
(lines 236, 261 and 408) round a subnormal result themselves, in the direction
`fegetround()` returns, where every other operation of `pow` rounds in the
direction of MXCSR, x86-64 computing the doubles with SSE. The `fegetround()`
of glibc reads the control word of the x87 unit only
(`sysdeps/x86_64/fpu/fegetround.c`: "We only check the x87 FPU unit. The SSE
unit should be the same"), and so do those of mingw-w64 (`fnstcw` alone, in
the `libmingwex.a` of mingw-w64 11) and of newlib, which Cygwin uses
(`newlib/libm/machine/shared_x86/fenv.c`), so a program that sets MXCSR
without the x87 unit, with `_mm_setcsr()` as interval libraries do, gets its
subnormal powers rounded in the direction of the x87 unit. This is how GAOL's
`pow` was found to miss the exact value of such results with the x87 unit to
nearest and MXCSR upward. `cbrt.c`, `rsqrt.c` and `asinpi.c` read MXCSR
already, with their `get_rounding_mode()`, but with mingw-w64 they call
`fegetround()`, and with clang-cl and on Cygwin they read it wrong
([patch 6](#6-the-rounding-field-of-mxcsr-in-cbrtc-rsqrtc-and-asinpic)): the
patch gives `pow.h` a `get_rounding_mode()` of the form patch 6 gives them,
which on x86-64 reads the rounding field of MXCSR and looks its `FE_*` value up
in a table, whatever values the C library gives them, and calls `fegetround()`
elsewhere. `cos.c`, `tan.c` and `sincos.c` carry the same rounding code
(`subnormalize_dint()`), but their accurate phases never give a subnormal, and
they are left alone.

The patch, to commit as
`[pow] read the rounding mode from MXCSR on x86-64 for the subnormal results`:

```diff
diff --git a/src/binary64/pow/pow.h b/src/binary64/pow/pow.h
index 84513b3e..e55896d4 100644
--- a/src/binary64/pow/pow.h
+++ b/src/binary64/pow/pow.h
@@ -219,6 +219,24 @@ static inline int64_t dint_toi(const dint64_t *a) {
   return a->sgn ? -r : r;
 }

+/* The doubles are rounded by the SSE unit on x86-64: read the rounding mode
+   from MXCSR. The fegetround() of some C libraries (GNU libc, mingw-w64,
+   newlib) reads the control word of the x87 unit, which differs from MXCSR
+   if a program changed one without the other. The rounding field of MXCSR
+   is 0 to nearest, 1 downward, 2 upward and 3 toward zero, whatever values
+   the C library gives FE_TONEAREST, ... (0x400 for FE_DOWNWARD with GNU
+   libc and mingw-w64, 0x100 with the UCRT of Windows, 1 with newlib): look
+   them up. */
+static inline int get_rounding_mode (void)
+{
+#if defined(__x86_64__)
+  static const int lut[4] = {FE_TONEAREST, FE_DOWNWARD, FE_UPWARD, FE_TOWARDZERO};
+  return lut[_MM_GET_ROUNDING_MODE()>>13];
+#else
+  return fegetround ();
+#endif
+}
+
 // round a, assuming a is in the subnormal range
 // exact is non-zero iff x^y is exact
 static inline double dint_tod_subnormal(dint64_t *a, int exact) {
@@ -233,7 +251,7 @@ static inline double dint_tod_subnormal(dint64_t *a, int exact) {
   uint64_t rb, sb;

   if (ex >= 64) { // all bits disappear: |a| < 2^-1074
-    switch (fegetround()) {
+    switch (get_rounding_mode ()) {
     case FE_TONEAREST:
       rb = (a->hi >> 63);        // only used when e=64
       sb = (a->hi << 1) | a->lo; // idem
@@ -258,7 +276,7 @@ static inline double dint_tod_subnormal(dint64_t *a, int exact) {
   rb = (a->hi >> (ex - 1)) & 0x1; // round bit
   sb = (a->hi << (65 - ex)) || a->lo; // sticky bit

-  switch (fegetround()) {
+  switch (get_rounding_mode ()) {
   case FE_TONEAREST:
     // if ex=12 there is no underflow when hi rounds to 2^52 and rb=1
     // and the next bit is 1 too
@@ -405,7 +423,7 @@ static inline void subnormalize_qint(qint64_t *a) {
   uint64_t md = (a->hh >> (ex - 1)) & 0x1;
   uint64_t lo = (a->hh & (~0ull >> ex)) || a->hl || a->lh || a->ll;

-  switch (fegetround()) {
+  switch (get_rounding_mode ()) {
   case FE_TONEAREST:
     hi += lo ? md : hi & md;
     break;
```

**How it was checked.** With the x87 unit to nearest and MXCSR upward,
`pow(0x1.4a1249ba9b6d5p-1, 0x1.97f9e85175cf7p+10)`, whose exact value is
1458662373659.29·2<sup>−1074</sup>, is `0x0.001539f0d791bp-1022` with master
rather than `0x0.001539f0d791cp-1022`, and two more powers of values
567863.26·2<sup>−1074</sup> and 1.30·2<sup>−1074</sup> are rounded down too
(references from mpmath); the patched `pow` rounds the three upward, with GCC
and with Clang. The patched object code reads MXCSR (`stmxcsr`) and no longer
calls `fegetround()`. Over 1.2 million pairs (those of `pow.wc`, and 200,000
whose power is next to 2<sup>−1022</sup> or below), with the x87 unit to
nearest and MXCSR set to each direction, master gives about 97,000 results per
directed rounding other than those it gives with both units set, with GCC and
Clang on Linux, with MinGW-w64 (GCC and Clang) under wine, and with Cygwin,
emulated as for patch 6; the patched `pow` gives none. With clang-cl, emulated
in the same way, the patched `pow` gives the results of Linux, where a copy of
the `get_rounding_mode()` of `rsqrt.c` gives 193,196 wrong results upward and
1,168 downward.

**Status.** Present in master.

#### 6. The rounding field of MXCSR in `cbrt.c`, `rsqrt.c` and `asinpi.c`

**The defect.** On x86-64, the `get_rounding_mode()` of these three files reads
the rounding field of MXCSR and makes of it the `FE_*` value of the C library
by a shift that depends on the library (`cbrt.c` lines 84 to 101, `rsqrt.c` 84
to 101, `asinpi.c` 93 to 110): by 3 for the values of glibc (`FE_DOWNWARD`
0x400, `FE_UPWARD` 0x800); where `__WIN32__` or `__WIN64__` is defined, by 5 or
through a table for the two layouts of the UCRT of Windows (`FE_UPWARD` 0x200,
or 0x100 before Windows 10 14393, as the comment there says), and otherwise
with a `#warning` and `return fegetround();`. Two compilers and Cygwin get it
wrong:

- **mingw-w64** (GCC, and Clang for the CLANG64 environment of MSYS2) defines
  `__WIN32__` and gives `FE_*` the values of glibc (`fenv.h` of mingw-w64 11):
  it takes the last branch. `rsqrt.c` and `asinpi.c` compare the value with
  `FE_*` and stay right as long as the two units agree, the `fegetround()` of
  mingw-w64 reading the x87 unit only. But `cr_cbrt()` wants 0 to 3 (to
  nearest, downward, upward, toward zero), which the switch at the end of its
  `get_rounding_mode()` makes of the `FE_*` value, and the `return` skips the
  switch: rounding upward, `rm` is 0x800, `off[rm]` (lines 204, 205, 215 and
  216) reads 16 KiB past its four elements, and `rm+sign == 2` (line 235)
  never holds, so that hard cases are rounded wrong in the three directed
  roundings. Upward, `cr_cbrt(0x1.3a9ccd7f022dbp+0)` is `0x1.1236160ba9b93p+0`,
  below the cube root, about `0x1.1236160ba9b930000000000001e7e8fap+0`.
  `cbrt.c` took this branch from `rsqrt.c` in `49d77f99` ("Fix handling of
  FE_UPWARD on Windows pre-14393 update", 5 February 2026), where returning the
  `FE_*` value is right.
- **clang-cl** defines `__x86_64__`, `_WIN32` and `_WIN64` but neither
  `__WIN32__` nor `__WIN64__` (clang-cl 18), and uses the `fenv.h` of the UCRT:
  the three files shift by 3 and compare 0x400, 0x800 and 0xc00 with the
  UCRT's 0x100, 0x200 and 0x300, so that the downward and upward roundings are
  taken for toward zero.
- **Cygwin**, where GCC and Clang define `__x86_64__` but neither `__WIN32__`
  nor `__WIN64__`, uses the `fenv.h` of newlib
  (`newlib/libc/machine/shared_x86/sys/fenv.h`), which gives `FE_*` the values
  0 to 3: the three files shift by 3, as with clang-cl, and the downward and
  upward roundings are taken for toward zero in the same way. Upward,
  `cr_cbrt(0x1.3a9ccd7f022dbp+0)` is `0x1.1236160ba9b93p+0` there too.

Visual C++ defines no `__x86_64__` and calls `fegetround()`, and glibc, musl,
macOS and the BSDs, whose `FE_*` are the values of the rounding field in the
x87 control word, are right. The rounding field of MXCSR is 0 to nearest, 1
downward, 2 upward and 3 toward zero whatever the system: the patch reads it so
in the three files, whatever values the C library gives `FE_*`, as the table of
the branch `FE_UPWARD == 0x0100` already does, and as the `binary128` functions
do with `_MM_ROUND_*`. `cbrt.c` returns the field itself, which is its 0 to 3;
`rsqrt.c` and `asinpi.c` look their `FE_*` value up in that table. The
`#warning` and the call to `fegetround()` go, so that `rsqrt` and `asinpi` read
MXCSR with mingw-w64 too, as they mean to (see patch 5). The smallest change,
`mode = fegetround();` for `return fegetround();` in `cbrt.c`, would fix `cbrt`
with mingw-w64 only, while the two units agree. `binary80/pow/powl.c` has the
same branches, returning `FE_*` as `rsqrt.c` does, and was not examined.

The patch, to commit as
`[cbrt,rsqrt,asinpi] read the rounding field of MXCSR whatever the values of FE_*`:

```diff
diff --git a/src/binary64/asinpi/asinpi.c b/src/binary64/asinpi/asinpi.c
index ce089f18..65607be1 100644
--- a/src/binary64/asinpi/asinpi.c
+++ b/src/binary64/asinpi/asinpi.c
@@ -90,24 +90,13 @@ inline static unsigned int get_arm_rounding_mode(void)
 static inline int get_rounding_mode (void)
 {
 #if defined(__x86_64__)
-  #if defined(__WIN32__) || defined(__WIN64__)
-    // Windows 10 14393 swapped FE_UPWARD and FE_DOWNWARD.
-    // Before: FE_UPWARD = 0x0100, FE_DOWNWARD = 0x0200
-    // After:  FE_UPWARD = 0x0200, FE_DOWNWARD = 0x0100
-    // The amount we need to shift changes depending on the value.
-    #if FE_UPWARD == 0x0200
-      return _MM_GET_ROUNDING_MODE()>>5;
-    #elif FE_UPWARD == 0x0100
-      // Lookup table used to eliminate branches.
-      static const unsigned lut[4] = {FE_TONEAREST, FE_DOWNWARD, FE_UPWARD, FE_TOWARDZERO};
-      return lut[_MM_GET_ROUNDING_MODE()>>13];
-    #else
-      #warning The floating point rounding constants have an unknown value. A slower path will be taken.
-      return fegetround();
-    #endif
-  #else
-    return _MM_GET_ROUNDING_MODE()>>3;
-  #endif
+  // The rounding field of MXCSR is 0 to nearest, 1 downward, 2 upward and
+  // 3 toward zero, whatever values the C library gives FE_TONEAREST, ...
+  // (0x400 for FE_DOWNWARD with GNU libc and mingw-w64, 0x100 with the UCRT
+  // of Windows, 1 with newlib; clang-cl and Cygwin define __x86_64__ and
+  // not __WIN32__): look them up.
+  static const unsigned lut[4] = {FE_TONEAREST, FE_DOWNWARD, FE_UPWARD, FE_TOWARDZERO};
+  return lut[_MM_GET_ROUNDING_MODE()>>13];
 #elif defined(__arm64__) || defined(_M_ARM64) || defined(__aarch64__)
   return get_arm_rounding_mode();
 #else
diff --git a/src/binary64/cbrt/cbrt.c b/src/binary64/cbrt/cbrt.c
index b416c6f0..c398e49f 100644
--- a/src/binary64/cbrt/cbrt.c
+++ b/src/binary64/cbrt/cbrt.c
@@ -81,24 +81,12 @@ static inline int get_rounding_mode (fexcept_t *flagp)
   unsigned int mode;
 #if defined(__x86_64__)
   *flagp = _mm_getcsr ();
-  #if defined(__WIN32__) || defined(__WIN64__)
-    // Windows 10 14393 swapped FE_UPWARD and FE_DOWNWARD.
-    // Before: FE_UPWARD = 0x0100, FE_DOWNWARD = 0x0200
-    // After:  FE_UPWARD = 0x0200, FE_DOWNWARD = 0x0100
-    // The amount we need to shift changes depending on the value.
-    #if FE_UPWARD == 0x0200
-      mode = (*flagp & _MM_ROUND_MASK)>>5;
-    #elif FE_UPWARD == 0x0100
-      // Lookup table used to eliminate branches.
-      static const unsigned lut[4] = {FE_TONEAREST, FE_DOWNWARD, FE_UPWARD, FE_TOWARDZERO};
-      mode = lut[(*flagp & _MM_ROUND_MASK)>>13];
-    #else
-      #warning The floating point rounding constants have an unknown value. A slower path will be taken.
-      return fegetround();
-    #endif
-  #else
-    mode = (*flagp & _MM_ROUND_MASK)>>3;
-  #endif
+  // The rounding field of MXCSR is 0 to nearest, 1 downward, 2 upward and
+  // 3 toward zero, the values this function returns, whatever values the C
+  // library gives FE_TONEAREST, ... (0x400 for FE_DOWNWARD with GNU libc and
+  // mingw-w64, 0x100 with the UCRT of Windows, 1 with newlib; clang-cl and
+  // Cygwin define __x86_64__ and not __WIN32__).
+  return (*flagp & _MM_ROUND_MASK)>>13;
 #elif defined(__arm64__) || defined(_M_ARM64) || defined(__aarch64__)
   fegetexceptflag (flagp, FE_ALL_EXCEPT);
   mode = get_arm_rounding_mode();
diff --git a/src/binary64/rsqrt/rsqrt.c b/src/binary64/rsqrt/rsqrt.c
index 6bc8cf64..07579525 100644
--- a/src/binary64/rsqrt/rsqrt.c
+++ b/src/binary64/rsqrt/rsqrt.c
@@ -81,24 +81,13 @@ inline static unsigned int get_arm_rounding_mode(void)
 static inline int get_rounding_mode (void)
 {
 #if defined(__x86_64__)
-  #if defined(__WIN32__) || defined(__WIN64__)
-    // Windows 10 14393 swapped FE_UPWARD and FE_DOWNWARD.
-    // Before: FE_UPWARD = 0x0100, FE_DOWNWARD = 0x0200
-    // After:  FE_UPWARD = 0x0200, FE_DOWNWARD = 0x0100
-    // The amount we need to shift changes depending on the value.
-    #if FE_UPWARD == 0x0200
-      return _MM_GET_ROUNDING_MODE()>>5;
-    #elif FE_UPWARD == 0x0100
-      // Lookup table used to eliminate branches.
-      static const unsigned lut[4] = {FE_TONEAREST, FE_DOWNWARD, FE_UPWARD, FE_TOWARDZERO};
-      return lut[_MM_GET_ROUNDING_MODE()>>13];
-    #else
-      #warning The floating point rounding constants have an unknown value. A slower path will be taken.
-      return fegetround();
-    #endif
-  #else
-    return _MM_GET_ROUNDING_MODE()>>3;
-  #endif
+  // The rounding field of MXCSR is 0 to nearest, 1 downward, 2 upward and
+  // 3 toward zero, whatever values the C library gives FE_TONEAREST, ...
+  // (0x400 for FE_DOWNWARD with GNU libc and mingw-w64, 0x100 with the UCRT
+  // of Windows, 1 with newlib; clang-cl and Cygwin define __x86_64__ and
+  // not __WIN32__): look them up.
+  static const unsigned lut[4] = {FE_TONEAREST, FE_DOWNWARD, FE_UPWARD, FE_TOWARDZERO};
+  return lut[_MM_GET_ROUNDING_MODE()>>13];
 #elif defined(__arm64__) || defined(_M_ARM64) || defined(__aarch64__)
   return get_arm_rounding_mode();
 #else
```

**How it was checked.** With MinGW-w64 11 for Windows x64, GCC 13 and Clang 18
(`--target=x86_64-w64-mingw32`), the programs run under wine 9.0 and the
direction set in both units, as `fesetround()` does, over 1,212,574 arguments
(those of `cbrt.wc` on both signs, the seven of `wlist` times 8<sup>k</sup>
for k = 0, ±1 and ±340 on both signs, and a million random bit patterns),
master's `cbrt` gives 771 to 778 results per directed rounding other than those
of x86-64 Linux, and none to nearest. Checked with mpmath (400 bits), 2,315
(GCC) and 2,321 (Clang) of these 3,140 results are wrong, 60 of the 70 of
`wlist` among them, and those of Linux and of the patched `cbrt` are all
right. The patched `cbrt`, `rsqrt` (1,020,000 arguments: those of `rsqrt.wc`,
the powers of 2 and their neighbours, random ones) and `asinpi` (1,058,405:
those of `asinpi.wc`, random ones in [−1, 1] and next to ±1) give the results
of Linux in the four directions, with both compilers. Master's `rsqrt` and
`asinpi` do too while the two units agree, but with MXCSR set alone (the x87
unit to nearest) they give 187 to 1,225 and about 513,000 wrong results per
directed rounding, and the patched ones none. clang-cl, emulated with Clang 18
on Linux (the `FE_*` values of the UCRT put over those of glibc, `__WIN32__`
undefined as there): master gives 42 wrong `cbrt` results downward and 42
upward, 1,338 wrong `rsqrt` results upward, and 499,838 and 526,673 wrong
`asinpi` results downward and upward; the patched files none. Cygwin,
emulated in the same way with GCC 13 and Clang 18 (the `FE_*` values of
newlib, 0 to 3, put over those of glibc, and a `fegetround()` that reads the
x87 unit, as newlib's does): master gives 42 wrong `cbrt` results downward and
42 upward, 1,338 (Clang) and 1,341 (GCC) wrong `rsqrt` results upward, and
499,838 and 526,673 different `asinpi` results downward and upward (mpmath:
all those of `cbrt` and `rsqrt` and 3,000 sampled of `asinpi` are wrong); the
patched files none, the direction being set in both units or in MXCSR alone.
The patched files compile without a warning with GCC 13 and Clang 18, for Linux
and for MinGW-w64, where master's `#warning` fires, and their object code no
longer calls `fegetround()` on x86-64. With MPFR, `./check.sh --worst`
(212,508, 19,844 and 116,822 arguments per direction) and
`./check.sh --special` with 2,000,000 random arguments pass for the three
functions. On x86-64 Linux the patched `cbrt` is as fast as master (about 12 ns
a call with GCC 13, within the noise of the measure).

**Status.** Present in master; GAOL's copy makes the two smallest changes
instead, `mode = fegetround();` in `cbrt.c` for mingw-w64 (item 6 above) and
`|| defined(_WIN32)` in the three files for clang-cl (item 7), which leave
Cygwin as it is.

#### 7. The exact and midpoint powers of `pow` on 32-bit ARM

**The defect.** Where the second phase of `cr_pow()` cannot round,
`exact_pow()` (`pow.c`, lines 1231 to 1348) computes the exact and midpoint
powers, c<sup>a</sup>·2<sup>s</sup> with c<sup>a</sup> of at most 54 bits, as
a 64-bit integer, which `(double)` converts (lines 1306 and 1337) before
`pow2()` scales it. The conversion of a midpoint, c<sup>a</sup> odd of 54
bits, is the one rounding of the result, and has to be made in the rounding
direction in effect. On 32-bit ARM, VFP has no instruction converting a 64-bit
integer: GCC calls `__aeabi_l2d` of libgcc, integer code that rounds to
nearest whatever the rounding mode of FPSCR is (a probe under qemu-arm:
`(double)` of 3<sup>34</sup> is 16677181699666568 in the four directions, while
the VFP addition 16677181699666568.0 + 1.0 gives 16677181699666570 upward). The
midpoints that reach `exact_pow()`, powers below about 2<sup>−950</sup>, were
so rounded to nearest in the directed roundings too: rounding upward,
`cr_pow(0x1.8p-29, 34)`, 3<sup>34</sup>·2<sup>−1020</sup>, is
`0x1.d9fe779881944p-967` with master, below the power, rather than
`0x1.d9fe779881945p-967`. The patch converts the integer as its two 32-bit
halves, whose conversions and product by 2<sup>32</sup> are exact, and one
addition, the only rounding, which every floating-point unit makes in the
direction in effect.

The patch, to commit as
`[pow] round the exact and midpoint results in the rounding mode on 32-bit ARM`:

```diff
diff --git a/src/binary64/pow/pow.c b/src/binary64/pow/pow.c
index e5af998e..c29245ee 100644
--- a/src/binary64/pow/pow.c
+++ b/src/binary64/pow/pow.c
@@ -1215,6 +1215,21 @@ static void exp_3 (qint64_t *r, qint64_t *x) {
    The only remaining one is the first one, where 2^-F divides E.
 */

+/* Convert k, |k| <= 2^54, to double, rounding in the current rounding mode.
+   (double) k does not do so on every target: on 32-bit ARM, GCC converts a
+   64-bit integer with __aeabi_l2d from libgcc, which always rounds to
+   nearest, so that a midpoint k*2^g (k odd with 54 bits) was rounded to
+   nearest in the directed rounding modes too. The conversions of the two
+   32-bit halves of k are exact, and so is the product by 2^32: the sum is the
+   only rounding. */
+static inline double
+i64_to_double (int64_t k)
+{
+  double hi = (double) (int32_t) (k >> 32) * 0x1p32;
+  double lo = (double) (uint32_t) k;
+  return hi + lo;
+}
+
 /*
   Computes x^y and returns 1 if the result fits into 54 bits, i.e. computes
   exactly x^y for exact and midpoint cases.
@@ -1303,7 +1318,7 @@ exact_pow (double *r, double x, double y, const dint64_t *z,
        to reduce to 2^X*r with odd r. It checks whether k is an odd number
        multiplied by 2^(g-G). */
     if (((k & ~(~1ull << (g - G))) == (1ull << (g - G)))) {
-      *r = (double)((k >> (g - G)) * _s);
+      *r = i64_to_double ((k >> (g - G)) * _s);
       pow2(r, g);
       goto end;
     }
@@ -1334,7 +1349,7 @@ exact_pow (double *r, double x, double y, const dint64_t *z,
   if (k >> 54)
     return 0;

-  *r = (double)(k * _s);
+  *r = i64_to_double (k * _s);
   int64_t G = E * (n << F);
   pow2(r, G);

```

**How it was checked.** Over 13,302,409 pairs (powers of two to the powers
n/e at the ends of the exponents, perfect 2<sup>k</sup>-th powers to the
powers a/2<sup>k</sup>, random pairs, and the neighbours of each, 579,436 of
them with a power that is a double) and 3,950 midpoints c<sup>a</sup>·2<sup>s</sup>
with c<sup>a</sup> odd of 54 bits, s from −1075 to 969, whose roundings
downward and upward were computed with MPFR 4.2.1 and exactly: compiled by
the GCC 13.3 cross compiler for armhf (`-mfpu=neon-vfpv4 -mfloat-abi=hard`)
and run under qemu-arm 8.2.2, master gives 415 wrong results downward and
toward zero and 631 upward, all at midpoints, and the patched `pow` none; on
x86-64 Linux, GCC 13.3, the two give the same bits in the four directions,
all right. With MPFR, `./check.sh --worst` (1,003,179 arguments per direction)
passes to nearest, toward zero and upward; downward it stops at the spurious
underflow exception said above, where master stops as well.
`./check.sh --special` with `CORE_MATH_TESTS=200000`, exact and midpoint values
among its arguments, passes in the four directions.

**Status.** Present in master (`b1a4badf`, and `284b3b0` of 1 October 2026,
whose `pow.c` is the same); GAOL's copy makes the same change (item 8 above).

#### Already fixed upstream: the shifts of `cospi.c`

The three signed shifts of item 3 above (lines 179 to 181 at `6b84457`) were
fixed upstream on 29 September 2026 by `b1a4badf` ("[cospi] make the sanitizer
happy"), which declares `m` as `uint64_t` rather than converting it at each
shift. Over 4 million arguments (integers and half-integers among them) in the
four directions, `cospi` of master gives the results of GAOL's, and UBSan,
which stops on `6b84457`, reports nothing. Nothing to send: GAOL's change goes
at the next import.

### glibc

glibc takes functions from CORE-MATH, each file saying "copied from the
CORE-MATH project": functions of floats from 2.41 (`sysdeps/ieee754/flt-32`),
and functions of doubles from 2.43 (`sysdeps/ieee754/dbl-64`): `acosh`,
`asinh`, `atanh`, `erf`, `erfc`, `lgamma` and `tgamma` in 2.43, `cosh`, `sinh`
and `tanh` in 2.44, and `cbrt` on master (`30f988fbe795`, 29 September 2026).
None of the defects above is in them, in any of these releases:

- the masks: glibc's `cosh`, `sinh` and `tanh` write `MANTISSA_MASK`, which is
  `UINT64_C(0x000fffffffffffff)` (`sysdeps/ieee754/dbl-64/math_config.h`),
  where CORE-MATH writes `~0ul>>12` (on master, `e_cosh.c` lines 97, 106, 222
  and 295, `e_sinh.c` 105, 213 and 286, `s_tanh.c` 104 and 284), and `lgamma`
  writes `~UINT64_C(0) >> 12` (`e_lgamma_r.c` 1003); the `__glibc_likely()`
  and `__glibc_unlikely()` of these files, glibc's `__builtin_expect()`, take
  comparisons only;
- `asinpi`, `rsqrt` and `cospi` of doubles are glibc's own generic code
  (`math/s_asinpi_template.c`, `math/s_rsqrt_template.c`,
  `math/s_cospi_template.c`), and its `sin` and `pow` are other code (`s_sin.c`
  from the IBM Accurate Mathematical Library, `e_pow.c` from 2018);
- the rounding direction: inside glibc it is read with `get_rounding_mode()`
  (`sysdeps/generic/get-rounding-mode.h`), which on x86 reads the control word
  of the x87 unit (`_FPU_GETCW`, `fnstcw`, in `sysdeps/x86/fpu_control.h`), as
  `fegetround()` does, and of the functions of doubles it took from CORE-MATH,
  only `cbrt` asks for it (`s_cbrt.c`, line 37). That the two units agree is
  glibc's model, `fesetround()` setting both: a program that sets MXCSR alone
  is outside it, and there is nothing to propose. Nor has `cbrt` the defect of
  patch 6: glibc's `get_rounding_mode()` maps the rounding field of the
  control word (`_FPU_RC_*`) to glibc's own `FE_*` values and aborts on
  anything else, and `cbrt_rounding_index()` maps these four to 0 to 3 with a
  switch whose default is `__builtin_unreachable()`, so that no raw value
  reaches `off[]`; there is no branch for other C libraries.

So there is nothing to send to glibc. Were one of these defects to reach it
with a later import, the patch would go by `git send-email` to
libc-alpha@sourceware.org, its subject starting with `math:`, with a
`Signed-off-by:` line under the Developer's Certificate of Origin (glibc has
not required a copyright assignment to the FSF since August 2021), and a test
in `math/auto-libm-test-in` for the arguments above.
