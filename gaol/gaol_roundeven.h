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
 * Two places call it:
 *
 * - round_ties_to_even(I) of intervals (gaol/gaol_interval_core.h), the
 *   roundTiesToEven of IEEE 1788-2015 (gaol_ieee1788::roundTiesToEven): the
 *   rounding to the nearest integer being non-decreasing, the interval it
 *   returns has for bounds the two bounds of I rounded, each a double that is
 *   an integer. The function has to be exact whatever the rounding direction.
 * - the sources of CORE-MATH, which call __builtin_roundeven() to reduce their
 *   arguments (exp, exp2, expm1, log1p, sin, erfc, pow...: the integer nearest
 *   to x/ln 2, to x/pi...). GCC and Clang turn it into an instruction where
 *   the processor has one, and into a call to roundeven() of the math library
 *   otherwise, which that of Windows has not: gaol/core_math_port.h, which the
 *   three builds give to those C sources, replaces the builtin with this
 *   function there, and for the old compilers that have no such builtin.
 *
 * This header is C, and is included both by GAOL's C++ sources and by
 * gaol/core_math_port.h, which is why it has C comments only, and the macros
 * below for what C and C++ do not write the same way. tests/elementary.cpp
 * checks round_ties_to_even() in the four rounding directions, on halfway
 * values, whole numbers and the doubles next to them.
 *
 * Copyright (c) 2026 ENSTA, France
 * Modified by Jordan NININ (GAOL v5)
 *
 * Created 2026-09-20 by Jordan NININ
 *------------------------------------------------------------------------------
 * gaol is a software distributed WITHOUT ANY WARRANTY. Read the associated
 * COPYING file for information.
 *----------------------------------------------------------------------------*/

#ifndef GAOL_ROUNDEVEN_H
#define GAOL_ROUNDEVEN_H

#include <math.h>
#include <stdint.h>
#include <string.h>

#if defined(_MSC_VER) && !defined(__clang__)
#  define GAOL_ROUNDEVEN_INLINE static __forceinline
#else
#  define GAOL_ROUNDEVEN_INLINE static inline
#endif

/* A conversion: static_cast in C++, where the casts of C are warned of
   (-Wold-style-cast, which .github/scripts/headers.sh gives), and the cast of
   C for the sources of CORE-MATH (GAOL v5) */
#ifdef __cplusplus
#  define GAOL_ROUNDEVEN_CAST(type, x) static_cast<type>(x)
#else
#  define GAOL_ROUNDEVEN_CAST(type, x) ((type)(x))
#endif

/* x rounded to the nearest integer, a halfway value to the even one:
   0.5 is 0, 1.5 and 2.5 are 2, 3.5 is 4, -2.5 is -2 and -0.3 is -0.

   A double is a sign, an exponent e and 52 bits after the leading 1 of its
   significand: the last 52 - e of those bits are its fractional part, so that
   the integer part of x is x with them cleared. Only integer operations on
   the bits are needed, which is why no floating-point operation is made, and
   none can be rounded in the direction in effect or raise an exception. The
   result is always a double: an integer of the magnitude of x or one more. */
GAOL_ROUNDEVEN_INLINE double gaol_roundeven(double x)
{
  uint64_t u;
  int e;
  /* The bits of x, which memcpy() reads without undefined behaviour and which
     the compilers turn into a move between registers */
  memcpy(&u, &x, sizeof u);
  /* The exponent of x, 2^e <= |x| < 2^(e+1): -1023 for 0 and the subnormal
     numbers, 1024 for the infinities and the NaN */
  e = GAOL_ROUNDEVEN_CAST(int, (u >> 52) & 0x7ff) - 1023;
  if (e >= 52) {
    return x; /* an integer already, or an infinity or a NaN */
  }
  if (e < -1) {
    return copysign(0.0, x); /* |x| < 1/2, whose sign it keeps */
  }
  if (e == -1) {
    /* 1/2 <= |x| < 1: 1/2 is halfway between 0 and 1 and goes to 0, which is
       even; anything above it goes to 1 */
    const uint64_t frac = u & 0x000fffffffffffffull;
    return frac == 0 ? copysign(0.0, x) : copysign(1.0, x);
  }
  {
    /* 1 <= |x| < 2^52: the last `shift` bits of u are the fractional part.
       The bit of weight 1 is the one at the position `shift`, the units bit,
       and `half` is 1/2 */
    const int shift = 52 - e;
    const uint64_t one = 1;
    const uint64_t half = one << (shift - 1);
    const uint64_t frac = u & ((one << shift) - 1);
    /* x toward zero: the sign and the exponent are in the high bits of u,
       which clearing the fractional part leaves alone */
    uint64_t r = u - frac;
    /* One more unit if the fractional part is above 1/2, or at 1/2 with an odd
       integer part (the units bit is 1), the even one being the next. The
       carry out of the significand goes into the exponent, as it should:
       1.9 gives the bits of 2 */
    if (frac > half || (frac == half && (u & (one << shift)) != 0)) {
      r += one << shift;
    }
    memcpy(&x, &r, sizeof x);
    return x;
  }
}

#endif /* GAOL_ROUNDEVEN_H */
