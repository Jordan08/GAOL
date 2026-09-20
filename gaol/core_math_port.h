/*-*-C-*---------------------------------------------------------------------
 * gaol -- NOT Just Another Interval Library
 *--------------------------------------------------------------------------
 * What the sources of CORE-MATH (3rd/math-core) need to compile as a part of
 * GAOL, with every compiler GAOL is built with and on every architecture.
 *
 * The three builds include this header of themselves, before anything else,
 * in each source of CORE-MATH they compile (`-include` with GCC and Clang,
 * `/FI` with Visual C++): the sources themselves do not name it, so that
 * importing a newer CORE-MATH is a plain copy.
 *
 * It gives them:
 *
 * - the 128-bit unsigned integer their accurate phases compute with
 *   (gaol/gaol_u128.h), so that log, sin, cos, tan, atan2 and pow are built
 *   with Visual C++ and on 32-bit targets too, where the compiler has no
 *   128-bit type of its own;
 * - the names gaol_cr_<f>() rather than cr_<f>(), so that GAOL does not clash
 *   with a program or a C library holding CORE-MATH's functions too;
 * - what Visual C++ has not of GCC: the builtins the sources call, and
 *   __attribute__;
 * - silence for the warnings on conversions GAOL's library is compiled with.
 *--------------------------------------------------------------------------
 * gaol is a software distributed WITHOUT ANY WARRANTY. Read the associated
 * COPYING file for information.
 *--------------------------------------------------------------------------*/

#ifndef __gaol_core_math_port_h__
#define __gaol_core_math_port_h__

#include <stdint.h>

/*---------------------------------------------------------------------------
  The names of the functions
 --------------------------------------------------------------------------*/

#define cr_exp gaol_cr_exp
#define cr_log gaol_cr_log
#define cr_pow gaol_cr_pow
#define cr_sin gaol_cr_sin
#define cr_cos gaol_cr_cos
#define cr_tan gaol_cr_tan
#define cr_asin gaol_cr_asin
#define cr_acos gaol_cr_acos
#define cr_atan gaol_cr_atan
#define cr_atan2 gaol_cr_atan2
#define cr_sinh gaol_cr_sinh
#define cr_cosh gaol_cr_cosh
#define cr_tanh gaol_cr_tanh
#define cr_asinh gaol_cr_asinh
#define cr_acosh gaol_cr_acosh
#define cr_atanh gaol_cr_atanh

#include "gaol/gaol_core_math.h"

/*---------------------------------------------------------------------------
  The 128-bit unsigned integer

  The accurate phases of log, sin, cos, tan, atan2 and pow compute with a
  128-bit unsigned integer, which their sources name u128. GCC and Clang have
  one on 64-bit targets, and Clang has _BitInt(128) on 32-bit targets too;
  Visual C++ has none, on no architecture, and neither has GCC for a 32-bit
  target. There the sources take the structure of two 64-bit halves of
  gaol/gaol_u128.h, which tests/u128.cpp checks against the native type, and
  call its functions rather than the operators of the language.

  Each of the seven sources keeps its own definitions of uint128_t, addu_128,
  subu_128, cmp and cmpu, whose signatures are not the same in all of them:
  only the line naming the type is GAOL's.
 --------------------------------------------------------------------------*/

#include "gaol/gaol_u128.h"

/*---------------------------------------------------------------------------
  What Visual C++ has not of GCC and Clang
 --------------------------------------------------------------------------*/

#if defined(_MSC_VER) && !defined(__clang__)

#include <math.h>
#include <string.h>
#include <intrin.h>

#pragma warning(disable: 4244 4267 4146 4305 4723)

#define __builtin_expect(x, y) (x)
#define __builtin_fma(x, y, z) fma(x, y, z)
#define __builtin_fabs(x) fabs(x)
#define __builtin_copysign(x, y) copysign(x, y)
#define __builtin_sqrt(x) sqrt(x)
#define __builtin_floor(x) floor(x)
#define __builtin_round(x) round(x)
#define __builtin_trunc(x) trunc(x)
#define __builtin_isnan(x) isnan(x)
#define __builtin_isinf(x) isinf(x)
#define __builtin_fmin(x, y) fmin(x, y)
#define __builtin_fmax(x, y) fmax(x, y)
#define __builtin_nan(s) nan(s)
#define __builtin_inf() ((double)INFINITY)
#define __attribute__(x)

/* The number of leading and of trailing zero bits, undefined at 0 as the
   builtins of GCC are. _BitScanReverse64 and _BitScanForward64 are for the
   64-bit architectures only: on 32-bit Windows the halves are scanned. */
static __forceinline int gaol_clzll(uint64_t x)
{
#if defined(_M_X64) || defined(_M_ARM64) || defined(_M_ARM64EC)
  unsigned long i;
  _BitScanReverse64(&i, x);
  return 63 - (int)i;
#else
  unsigned long i;
  if (_BitScanReverse(&i, (unsigned long)(x >> 32))) return 31 - (int)i;
  _BitScanReverse(&i, (unsigned long)x);
  return 63 - (int)i;
#endif
}

static __forceinline int gaol_ctzll(uint64_t x)
{
#if defined(_M_X64) || defined(_M_ARM64) || defined(_M_ARM64EC)
  unsigned long i;
  _BitScanForward64(&i, x);
  return (int)i;
#else
  unsigned long i;
  if (_BitScanForward(&i, (unsigned long)x)) return (int)i;
  _BitScanForward(&i, (unsigned long)(x >> 32));
  return 32 + (int)i;
#endif
}

#define __builtin_clzll(x) gaol_clzll(x)
#define __builtin_ctzll(x) gaol_ctzll(x)
#define __builtin_clzl(x) gaol_clzll((uint64_t)(x))
#define __builtin_ctzl(x) gaol_ctzll((uint64_t)(x))

/* x rounded to the nearest integer, halfway values to the even one, which is
   neither round() (halfway away from zero) nor nearbyint() (the rounding
   direction in effect, upward in GAOL). The bits are read, so that the result
   does not depend on that direction. */
static __forceinline double gaol_roundeven(double x)
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

#define __builtin_roundeven(x) gaol_roundeven(x)

/* The sources use __builtin_mul_overflow on 64-bit operands only. */
static __forceinline int gaol_mul_overflow_u64(uint64_t a, uint64_t b, uint64_t *r)
{
  const gaol_u128 p = gaol_u128_mul64(a, b);
  *r = gaol_u128_lo(p);
  return gaol_u128_hi(p) != 0;
}

#define __builtin_mul_overflow(a, b, r) gaol_mul_overflow_u64((uint64_t)(a), (uint64_t)(b), (uint64_t *)(r))

#endif /* _MSC_VER */

/*---------------------------------------------------------------------------
  The warnings GAOL's library is compiled with, which the sources of
  CORE-MATH are not written for
 --------------------------------------------------------------------------*/

#if defined(__GNUC__) || defined(__clang__)
#  pragma GCC diagnostic ignored "-Wconversion"
#  pragma GCC diagnostic ignored "-Wsign-conversion"
#  pragma GCC diagnostic ignored "-Wfloat-conversion"
#  pragma GCC diagnostic ignored "-Wunused-function"
#  pragma GCC diagnostic ignored "-Wunused-but-set-variable"
#endif

#endif /* __gaol_core_math_port_h__ */
