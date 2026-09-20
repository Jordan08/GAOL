/*-*-C-*-----------------------------------------------------------------------
 * gaol -- NOT Just Another Interval Library
 *------------------------------------------------------------------------------
 * x rounded to the nearest integer, halfway values to the even one: the
 * roundTiesToEven of IEEE 1788-2015 (Table 9.1) and the roundeven() of C23.
 *
 * Neither round() nor nearbyint() does it: round() sends halfway values away
 * from zero, and nearbyint() rounds in the direction in effect, which GAOL
 * leaves upward. The bits are read instead, so that the result depends on
 * neither, and the function is exact: it returns a double that is an integer,
 * with no rounding of its own.
 *
 * This header is C, and is included both by GAOL's C++ sources and by
 * gaol/core_math_port.h, which the three builds give to the C sources of
 * CORE-MATH: the math library of Windows has no roundeven(), which those
 * sources call, and this is what they are given there.
 *
 * Copyright (c) 2026 ENSTA, France
 *
 * Created 2026-09-20 by Jordan NININ
 *------------------------------------------------------------------------------
 * gaol is a software distributed WITHOUT ANY WARRANTY. Read the associated
 * COPYING file for information.
 *----------------------------------------------------------------------------*/

#ifndef __gaol_roundeven_h__
#define __gaol_roundeven_h__

#include <math.h>
#include <stdint.h>
#include <string.h>

#if defined(_MSC_VER) && !defined(__clang__)
#  define GAOL_ROUNDEVEN_INLINE static __forceinline
#else
#  define GAOL_ROUNDEVEN_INLINE static inline
#endif

GAOL_ROUNDEVEN_INLINE double gaol_roundeven(double x)
{
  uint64_t u;
  int e;
  memcpy(&u, &x, sizeof u);
  e = (int)((u >> 52) & 0x7ff) - 1023;
  if (e >= 52) {
    return x; /* an integer already, or an infinity or a NaN */
  }
  if (e < -1) {
    return copysign(0.0, x); /* |x| < 1/2 */
  }
  if (e == -1) {
    /* 1/2 <= |x| < 1: 1/2 goes to 0, which is even */
    const uint64_t frac = u & 0x000fffffffffffffull;
    return frac == 0 ? copysign(0.0, x) : copysign(1.0, x);
  }
  {
    const int shift = 52 - e;
    const uint64_t half = (uint64_t)1 << (shift - 1);
    const uint64_t frac = u & (((uint64_t)1 << shift) - 1);
    uint64_t r = u - frac;
    if (frac > half || (frac == half && (u & ((uint64_t)1 << shift)) != 0)) {
      r += (uint64_t)1 << shift;
    }
    memcpy(&x, &r, sizeof x);
    return x;
  }
}

#endif /* __gaol_roundeven_h__ */
