# CORE-MATH in GAOL

[CORE-MATH](https://core-math.gitlabpages.inria.fr/) (Alexei Sibidanov, Paul
Zimmermann and others, Inria) provides mathematical functions that are
correctly rounded: the double they return is the rounding of the exact value in
the rounding direction in effect. It is distributed under the MIT licence
([LICENSE](LICENSE)), one source file for each function.

GAOL bounds its hyperbolic functions with six of them, `sinh`, `cosh`, `tanh`,
`asinh`, `acosh` and `atanh` in double precision, mathlib having none and
CRlibm only `sinh` and `cosh`
([issue #1](https://github.com/Jordan08/GAOL/issues/1)): the value rounded to
nearest is moved one double outward, as for the functions of mathlib.

GAOL bounds its logarithm with CORE-MATH's `log` too, with mathlib and CRlibm,
where the compiler has a 128-bit integer type, which its accurate phase
computes with (`__int128` of GCC and Clang for 64-bit targets,
`GAOL_CORE_MATH_LOG` in `gaol/gaol_core_math.h`): computed in the upward
rounding GAOL computes in, it gives the tightest bounds without switching the
rounding direction, twice as fast as mathlib's log moved outward. With Visual
C++ and on 32-bit targets, GAOL keeps mathlib's log.

| | |
|---|---|
| Sources | `src/binary64/<function>/<function>.c` of https://gitlab.inria.fr/core-math/core-math, and `src/binary64/log/dint.h` |
| Commit | `91d2102cefc484aacfccc9acd737c4f4410813af` (17 September 2026) |
| In GAOL | `gaol/core_math_<function>.c`, compiled into GAOL's library by the three builds, under the names `gaol_cr_<function>()` (`gaol/gaol_core_math.h`); `gaol/core_math_log_dint.h`, the `dint.h` of `log`, which is not installed |

They are compiled with the flags of interval arithmetic, as CORE-MATH asks
(`-frounding-math -ffp-contract=off`, `/fp:strict` for Visual C++).

## Changes

The sources are those of CORE-MATH but for:

- `#include "gaol/core_math_port.h"`, added before their first include. That
  header renames the functions, so that GAOL does not clash with a program or a
  C library that has CORE-MATH's functions too; gives Visual C++ the builtins
  of GCC the sources use (`__builtin_fma`, `__builtin_expect`...), from
  `<math.h>`; and turns off the warnings on conversions GAOL's library is
  compiled with.
- `~0ul` written `~(u64)0` in `sinh`, `cosh` and `tanh` (10 places):
  `unsigned long` has 32 bits on Windows, where `~0ul>>12` is not the mask of
  the 52 low bits of a double. `asinh`, `acosh` and `atanh` have it so already.
- In `log`: its code between `#if GAOL_CORE_MATH_LOG` and `#endif`, so that
  without a 128-bit integer type it compiles to nothing, and its `dint.h`
  included as `gaol/core_math_log_dint.h`, a copy of it without change.

To update them: copy the seven files and `dint.h` again, and make the same
changes.
