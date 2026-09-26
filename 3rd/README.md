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
bounds of an empty interval trap.

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
   not the mask of the 52 low bits of a double. Upstream writes `~0ull` in its
   other sources: this is a fix to propose to CORE-MATH.

3. **Three signed shifts made unsigned** in `cospi/cospi.c` (the test of an
   integer or half-integer argument). `m` is a signed 64-bit integer holding
   the significand with its hidden bit, 2^52, and shifting it left by 11 to 13
   places overflows: undefined behaviour in C, which UBSan reported in the tests
   run with the sanitizers. `sinpi.c` and `tanpi.c`, which do the same, convert
   `m` to `uint64_t` before shifting it; `cospi.c` now does too, and gives the
   same values bit for bit. This is a fix to propose to CORE-MATH.

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
   `tests/core_math.cpp`. This is a fix to propose to CORE-MATH.

5. **A 64-bit test made a comparison** in `rsqrt/rsqrt.c` (`cr_rsqrt`, the
   subnormal x). `__builtin_expect(ix.u, 1)` passed the 64 bits of x where
   `__builtin_expect` takes a `long`, which has 32 bits on the 32-bit targets
   and on Windows: the high half was dropped, and a subnormal whose low 32
   bits are 0, 2^-1040 to 2^-1024 among the powers of 4, was taken for +0,
   its rsqrt being +oo rather than 2^520 to 2^512 (the continuous integration,
   Debian i386 and armhf, MinGW-w64 and MSYS2; Visual C++, where
   `__builtin_expect(x, y)` is `(x)`, was right). It is now
   `__builtin_expect(ix.u != 0, 1)`. The other `__builtin_expect` of the
   sources GAOL compiles take a comparison, or an integer of 32 bits or less.
   This is a fix to propose to CORE-MATH.

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
  rounding, which are the tightest bounds there are.

### To update CORE-MATH

Copy the upstream tree again without the `.wc` files, then make the five changes
above. `git diff` against the previous version shows them: they are marked
`/* GAOL */`, and no other line differs.
