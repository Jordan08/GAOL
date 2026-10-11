/*-*-C-*-----------------------------------------------------------------------
 * gaol -- NOT Just Another Interval Library
 *------------------------------------------------------------------------------
 * x * 2^n rounded once, in the rounding direction in effect: the ldexp() of C
 * as the C standard and IEEE 754 (scaleB) define it (GAOL v5).
 *
 * CORE-MATH's erfc() rounds its subnormal results with __builtin_ldexp(), and
 * is correctly rounded only if that ldexp() rounds in the direction in effect.
 * The ldexp() of the UCRT of Windows, which Visual C++ and clang-cl call for
 * it, does not: rounding upward, erfc(0x1.a8b12fc6e4892p+4) was the double
 * below its value, and GAOL's interval erfc() did not enclose it (clang-cl of
 * Visual Studio 2022 and 2026, for x64, x86 and ARM64). An ldexp() rounding
 * to nearest or toward zero whatever the direction, given to erfc.c on
 * Linux, gave the same wrong bounds. The ldexp() of macOS is wrong too, on
 * fewer results: 2^-1127 rounded upward was 0. This function is what
 * gaol/core_math_port.h gives CORE-MATH for __builtin_ldexp(), with every
 * compiler, so that erfc() depends on no C library.
 *
 * It multiplies x by powers of two, as the scalbn() of musl does: the
 * products before the last one are exact unless their result is subnormal,
 * and n is then left at -54 or below, so that the last product, the only
 * rounding that counts, is below half the least subnormal and rounds as the
 * exact x * 2^n does. A product before the last one that overflows leaves the
 * last one beyond the largest double too, where it rounds as x * 2^n does.
 * The powers are made from their bits, which no rounding direction changes.
 *
 * This header is C, and is included both by gaol/core_math_port.h, for the
 * sources of CORE-MATH, and by tests/core_math.cpp, which checks it in the
 * four rounding directions on every platform. It is not installed.
 *
 * Copyright (c) 2026 ENSTA, France
 *
 * Created 2026-10-11 by Jordan NININ
 *------------------------------------------------------------------------------
 * gaol is a software distributed WITHOUT ANY WARRANTY. Read the associated
 * COPYING file for information.
 *----------------------------------------------------------------------------*/

#ifndef GAOL_LDEXP_H
#define GAOL_LDEXP_H

#include <stdint.h>
#include <string.h>

#if defined(_MSC_VER) && !defined(__clang__)
#  define GAOL_LDEXP_INLINE static __forceinline
#else
#  define GAOL_LDEXP_INLINE static inline
#endif

/* A conversion: static_cast in C++, where the casts of C are warned of, and
   the cast of C for the sources of CORE-MATH */
#ifdef __cplusplus
#  define GAOL_LDEXP_CAST(type, x) static_cast<type>(x)
#else
#  define GAOL_LDEXP_CAST(type, x) ((type)(x))
#endif

/* 2^k, for -1022 <= k <= 1023, from its bits */
GAOL_LDEXP_INLINE double gaol_ldexp_power(int k)
{
  const uint64_t u = GAOL_LDEXP_CAST(uint64_t, 1023 + k) << 52;
  double d;
  memcpy(&d, &u, sizeof d);
  return d;
}

GAOL_LDEXP_INLINE double gaol_ldexp(double x, int n)
{
  double y = x;
  if (n > 1023) {
    y *= gaol_ldexp_power(1023);
    n -= 1023;
    if (n > 1023) {
      y *= gaol_ldexp_power(1023);
      n -= 1023;
      if (n > 1023) {
        n = 1023;
      }
    }
  } else if (n < -1022) {
    /* 2^-1022 * 2^53: n ends at -54 or below where a product is inexact */
    y *= gaol_ldexp_power(-969);
    n += 969;
    if (n < -1022) {
      y *= gaol_ldexp_power(-969);
      n += 969;
      if (n < -1022) {
        n = -1022;
      }
    }
  }
  return y * gaol_ldexp_power(n);
}

#endif /* GAOL_LDEXP_H */
