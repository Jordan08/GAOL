/*-*-C-*--------------------------------------------------------------------
 * GAOL -- not Just Another Interval Library
 *--------------------------------------------------------------------------
 * x rounded to the nearest integer, halfway values away from zero: the
 * roundTiesToAway of IEEE 1788-2015 (Table 9.1) and the round() of C, for the
 * mingw-w64 whose round() is wrong (GAOL v5).
 *
 * The round() of mingw-w64's own math library (math/round.c, the same code
 * from version 9 to 13), which every mingw-w64 before version 12 links, and
 * the later ones linked with msvcrt.dll rather than the UCRT
 * (GAOL_MINGW_OWN_MATH, gaol/gaol_config.h), is ceil(x), less 1 when that is
 * more than 1/2 above x, the difference being computed in doubles and so
 * rounded in the direction in effect:
 *
 * - round(0x1.fffffffffffffp-2), the double below 1/2, is 1 rather than 0 in
 *   every direction but upward (1 - x is 1/2 + 2^-54, which rounds to 1/2),
 *   and the round of its opposite is -1 rather than -0. A bound given by it
 *   is not the one of the exact rounding, and an interval of which it is the
 *   lower bound does not hold 0;
 * - round(0.3) is -0 rather than 0 rounding downward (1 - 1 is -0 there), and
 *   round(-0.3) is 0 rather than -0.
 *
 * The standard defines round() as rounding "regardless of the current
 * rounding direction", which this function does: it reads the bits, makes no
 * operation on doubles, and is exact, with the sign of a zero it returns.
 *
 * Two places call it, only where GAOL_MINGW_OWN_MATH is 1, the other
 * toolchains keeping the round() of their C library:
 *
 * - round_ties_to_away(I) of intervals (gaol/core/gaol_interval_core.cpp),
 *   where the defect above shows;
 * - the sources of CORE-MATH, which call __builtin_round() in the
 *   roundeven_finite() of fifteen of them (exp.c, sin.c...), their fallback
 *   where the processor has neither AVX nor SSE4.1 nor ARMv8 (with GAOL_FMA
 *   off, for instance), where the defect above shows, and in pow.c, to take
 *   the integer nearest to the square root of an odd integer, that is at
 *   least 1, where the round() of mingw-w64 is exact:
 *   gaol/math/gaol_core_math_port.h, which the three builds give to those C
 *   sources, replaces the builtin with this function, so that GAOL calls no
 *   round() of that library.
 *
 * This header is C, and is included both by GAOL's C++ sources and by
 * gaol/math/gaol_core_math_port.h, which is why it has C comments only, and
 * the macros below for what C and C++ do not write the same way. It is not
 * installed. tests/core_math.cpp checks the function in the four rounding
 * directions, on every platform, against a reference that calls no round().
 *--------------------------------------------------------------------------
 * Copyright (c) 2026 ENSTA, France
 *
 * By: Jordan NININ (created 2026-11-18)
 *--------------------------------------------------------------------------
 * GAOL is a software distributed WITHOUT ANY WARRANTY. Read the associated
 * COPYING.LIB file for information.
 *--------------------------------------------------------------------------*/

#ifndef GAOL_ROUND_H
#define GAOL_ROUND_H

#include <stdint.h>
#include <string.h>

#if defined(_MSC_VER) && !defined(__clang__)
#  define GAOL_ROUND_INLINE static __forceinline
#else
#  define GAOL_ROUND_INLINE static inline
#endif

/* A conversion: static_cast in C++, where the casts of C are warned of
   (-Wold-style-cast, which .github/scripts/headers.sh gives), and the cast of
   C for the sources of CORE-MATH */
#ifdef __cplusplus
#  define GAOL_ROUND_CAST(type, x) static_cast<type>(x)
#else
#  define GAOL_ROUND_CAST(type, x) ((type)(x))
#endif

/* x rounded to the nearest integer, a halfway value to the one of larger
   magnitude: 0.5 is 1, 1.5 is 2, 2.5 is 3, -2.5 is -3, -0.3 is -0 and
   0x1.fffffffffffffp-2 is 0.

   A double is a sign, an exponent e and 52 bits after the leading 1 of its
   significand: the last 52 - e of those bits are its fractional part, so that
   the integer part of x is x with them cleared. Only integer operations on
   the bits are made, so that no floating-point operation is rounded in the
   direction in effect or raises an exception. The result is always a double:
   an integer of the magnitude of x or one more. The bits are those of the
   magnitude and of the sign apart, so that adding one unit to the magnitude
   goes away from zero for both signs. */
GAOL_ROUND_INLINE double gaol_round(double x)
{
  const uint64_t sign_mask = 0x8000000000000000ull;
  uint64_t u;
  int e;
  /* The bits of x, which memcpy() reads without undefined behaviour and which
     the compilers turn into a move between registers */
  memcpy(&u, &x, sizeof u);
  /* The exponent of x, 2^e <= |x| < 2^(e+1): -1023 for 0 and the subnormal
     numbers, 1024 for the infinities and the NaN */
  e = GAOL_ROUND_CAST(int, (u >> 52) & 0x7ff) - 1023;
  if (e >= 52) {
    return x; /* an integer already, or an infinity or a NaN */
  }
  if (e < -1) {
    /* |x| < 1/2: 0, with the sign of x */
    u &= sign_mask;
    memcpy(&x, &u, sizeof x);
    return x;
  }
  if (e == -1) {
    /* 1/2 <= |x| < 1: 1/2 is halfway between 0 and 1 and goes to 1, and so
       does anything above it. 1 is 0x3ff0000000000000 */
    u = (u & sign_mask) | 0x3ff0000000000000ull;
    memcpy(&x, &u, sizeof x);
    return x;
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
    /* One more unit if the fractional part is 1/2 or more. The carry out of
       the significand goes into the exponent, as it should: 1.9 gives the
       bits of 2 */
    if (frac >= half) {
      r += one << shift;
    }
    memcpy(&x, &r, sizeof x);
    return x;
  }
}

#endif /* GAOL_ROUND_H */
