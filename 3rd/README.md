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
library it uses: mathlib (the IBM Accurate Portable Mathematical Library),
CRlibm and the math library of the system, which GAOL could be built with, are
gone (see [What differs from GAOL](../doc/differences.md)).

| | |
| --- | --- |
| Upstream | <https://gitlab.inria.fr/core-math/core-math> |
| Commit | `671f2c7355d76c670f59d714d41a99eb1cf620b6` (19 September 2026) |
| Taken | the whole tree, without the `.wc` files |
| Built | `math-core/src/binary64/<f>/<f>.c` for the twenty-one functions below, compiled into libgaol itself |

The `.wc` files, which hold the hardest-to-round arguments CORE-MATH checks
itself against, are 542 of the 555 MB of the upstream tree and are not needed
to build: they are left out, and the commit above is what to clone to get them.

The twenty-one functions GAOL builds are `exp`, `log`, `pow`, `sin`, `cos`,
`tan`, `asin`, `acos`, `atan`, `atan2`, `sinh`, `cosh`, `tanh`, `asinh`,
`acosh` and `atanh`, then `cbrt`, which `nth_root(x, 3)` takes, and `exp2`,
`exp10`, `log2` and `log10`, which IEEE 1788-2015 requires among the forward
elementary functions (Table 9.1). The other formats and functions of the tree
are kept as they are, so that importing a newer CORE-MATH is a plain copy, but
nothing compiles them.

### How GAOL builds them

The three builds compile the twenty-one sources into `libgaol` and include
`gaol/core_math_port.h` first in each of them, **by the compiler rather than by
the source** (`-include` with GCC and Clang, `/FI` with Visual C++): the sources
never name that header, so that importing a newer CORE-MATH stays a copy. That
header gives them the names `gaol_cr_<f>()`, so that GAOL does not clash with a
program or a C library holding CORE-MATH's functions too, the 128-bit integer
of `gaol/gaol_u128.h`, and what Visual C++ has not of GCC (`__builtin_clzll`,
`__builtin_roundeven`, `__attribute__`...).

They are compiled with the flags of interval arithmetic, as CORE-MATH asks
(`-frounding-math -ffp-contract=off`, `/fp:strict` for Visual C++), and with the
GNU extensions of C (`atan2` uses inline assembly), not with `-std=c99`.

### What differs from upstream

The vendored sources are those of the commit above but for the changes below,
which are marked `/* GAOL */` in the code. They are made in the tree rather than
kept as a patch to reapply.

1. **The 128-bit integer** of the accurate phases of `log`, `sin`, `cos`, `tan`,
   `atan2` and `pow`. Upstream writes it `unsigned __int128`, or
   `unsigned _BitInt(128)` with Clang 14 and GCC 14: Visual C++ has neither, on
   no architecture, and neither has GCC for a 32-bit target. In
   `log/dint.h`, `log10/dint.h`, `pow/dint.h`, `pow/qint.h`, `atan2/tint.h`,
   `sin/sin.c`, `cos/cos.c` and `tan/tan.c`, the five lines that choose the type are one
   line naming `gaol_u128`, which is the type of the compiler where it has one
   and a structure of two 64-bit halves where it has none
   (`gaol/gaol_u128.h`), and the extended-arithmetic functions of those files
   call `gaol_u128_*()` rather than the operators of the language.

   There is no way round this: C has no operator overloading, and the files
   cannot be compiled as C++ because their tables initialise anonymous unions
   in a way C++ rejects.

2. **`~0ul` written `~0ull`** in `sinh/sinh.c`, `cosh/cosh.c` and `tanh/tanh.c`
   (nine places). `unsigned long` has 32 bits on Windows, where `~0ul >> 12` is
   not the mask of the 52 low bits of a double. Upstream writes `~0ull` in its
   other sources: this is a fix to propose to CORE-MATH.

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
- **against the native type.** `tests/u128.cpp` compares every operation of the
  two halves with the same operation on `unsigned __int128`, over 6.5 million
  values and every shift count, and the whole of GAOL is built and tested with
  the halves forced (`-DGAOL_U128_EMULATION=ON`, a job of the continuous
  integration) so that the code Visual C++ and the 32-bit targets take is run
  at each change;
- **against the tightest bounds.** `tests/core_math.cpp` compares the bounds
  GAOL computes with what CORE-MATH gives in the downward and the upward
  rounding, which are the tightest bounds there are.

### To update CORE-MATH

Copy the upstream tree again without the `.wc` files, then make the two changes
above. `git diff` against the previous version shows them: they are marked
`/* GAOL */`, and no other line differs.
