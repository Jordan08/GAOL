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
 * - the 128-bit integer their accurate phases compute with
 *   (gaol/gaol_u128.h), so that log, sin, cos, tan, atan2, pow, log2p1,
 *   log10p1, atan2pi, hypot, rsqrt and asinpi are built with Visual C++ and on
 *   32-bit targets too, where the compiler has no 128-bit type of its own;
 * - the names gaol_cr_<f>() rather than cr_<f>(), so that GAOL does not clash
 *   with a program or a C library holding CORE-MATH's functions too;
 * - what Visual C++ has not of GCC: the builtins the sources call, and
 *   __attribute__;
 * - silence for the warnings on conversions GAOL's library is compiled with.
 *
 * Copyright (c) 2026 ENSTA, France
 *
 * Created 2026-09-20 by Jordan NININ
 *--------------------------------------------------------------------------
 * gaol is a software distributed WITHOUT ANY WARRANTY. Read the associated
 * COPYING file for information.
 *--------------------------------------------------------------------------*/

#ifndef __gaol_core_math_port_h__
#define __gaol_core_math_port_h__

/* GCC 14 for a 32-bit x86 target stopped on an internal compiler error in
   asinpi_acc() of asinpi.c ("in extract_bit_field_1, at expmed.cc:1838", at
   -O2 and -O3): its SLP vectorizer puts the two 64-bit halves of the 128-bit
   integer, a structure there, in a vector it cannot take them out of again
   (the continuous integration, MinGW-w64 14.2 x86; reproduced with the GCC
   14.2 of Ubuntu and -m32). GCC 12, 13 and 15 compile it. The SLP vectorizer
   is turned off for the sources of CORE-MATH with that compiler only, which
   compute on scalars. */
#if defined(__GNUC__) && !defined(__clang__) && __GNUC__ == 14 \
    && defined(__i386__) && !defined(__x86_64__)
#pragma GCC optimize ("no-tree-slp-vectorize")
#endif

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
#define cr_cbrt gaol_cr_cbrt
#define cr_exp2 gaol_cr_exp2
#define cr_exp10 gaol_cr_exp10
#define cr_log2 gaol_cr_log2
#define cr_log10 gaol_cr_log10
#define cr_expm1 gaol_cr_expm1
#define cr_exp2m1 gaol_cr_exp2m1
#define cr_exp10m1 gaol_cr_exp10m1
#define cr_sinpi gaol_cr_sinpi
#define cr_cospi gaol_cr_cospi
#define cr_tanpi gaol_cr_tanpi
#define cr_acospi gaol_cr_acospi
#define cr_atanpi gaol_cr_atanpi
#define cr_log1p gaol_cr_log1p
#define cr_log2p1 gaol_cr_log2p1
#define cr_log10p1 gaol_cr_log10p1
#define cr_hypot gaol_cr_hypot
#define cr_rsqrt gaol_cr_rsqrt
#define cr_asinpi gaol_cr_asinpi
#define cr_atan2pi gaol_cr_atan2pi

#include "gaol/gaol_core_math.h"

/*---------------------------------------------------------------------------
  The 128-bit unsigned integer

  The accurate phases of log, sin, cos, tan, atan2, pow, log2p1, log10p1,
  atan2pi, hypot, rsqrt and asinpi compute with a 128-bit unsigned integer,
  which their sources name u128, and asinpi with a signed one too, i128, held
  in a u128 in two's complement. GCC and Clang have
  one on 64-bit targets, and Clang has _BitInt(128) on 32-bit targets too;
  Visual C++ has none, on no architecture, and neither has GCC for a 32-bit
  target. There the sources take the structure of two 64-bit halves of
  gaol/gaol_u128.h, which tests/u128.cpp checks against the native type, and
  call its functions rather than the operators of the language.

  Each of these sources keeps its own definitions of uint128_t, addu_128,
  subu_128, cmp and cmpu, whose signatures are not the same in all of them:
  only the line naming the type is GAOL's.
 --------------------------------------------------------------------------*/

#include "gaol/gaol_u128.h"

/*---------------------------------------------------------------------------
  roundeven(), which the math library of Windows has not

  The sources call __builtin_roundeven(). GCC and Clang turn it into one
  instruction where the processor has it (roundsd of SSE4.1, frintn on ARM),
  and into a call to roundeven() of C23 otherwise: the math library of glibc
  has that function, those of mingw-w64 and of Visual C++ have not, and a
  program linking GAOL there stopped on "undefined reference to roundeven"
  (the continuous integration, with MinGW-w64 15 and MSYS2, where GAOL is
  compiled without the AVX instructions).

  So on Windows the builtin is replaced by the function below, which reads the
  bits: it is neither round() (halfway values away from zero) nor nearbyint()
  (the rounding direction in effect, upward in GAOL), and it does not depend on
  that direction.
 --------------------------------------------------------------------------*/

#include "gaol/gaol_roundeven.h"

/*---------------------------------------------------------------------------
  fesetexceptflag() on a 32-bit Windows, which unmasks the exceptions

  cbrt, pow and atan2 read the rounding direction and keep the exception flags
  around their work, with _mm_getcsr() and _mm_setcsr() on x86-64 and with
  fegetexceptflag() and fesetexceptflag() everywhere else. The
  fesetexceptflag() of mingw-w64 for a 32-bit target does not write the flags
  alone: it clears the mask bits of MXCSR with them, so that the invalid, the
  divide-by-zero and the overflow exceptions become unmasked (the register went
  from 0x5fb2 to 0x5932 in the continuous integration). An exception then traps
  rather than raise a flag, and GAOL died on the first comparison of the bounds
  of an empty interval, which are NaN and which is_empty() compares with <=, an
  operation that signals invalid.

  So on a 32-bit x86 Windows the two are written here, on MXCSR, where GAOL's
  doubles are computed (gaol/gaol_config.h refuses an x86 target whose doubles
  are not). The six flag bits of MXCSR are the FE_* values of x86 in the same
  order, so no mapping is needed; the mask bits are left exactly as they are.
 --------------------------------------------------------------------------*/

#if defined(_WIN32) && (defined(__i386__) || defined(_M_IX86)) && !defined(__x86_64__)

#include <fenv.h>
#include <xmmintrin.h>

#define GAOL_X86_FLAG_BITS 0x3fu /* IE DE ZE OE UE PE, bits 0 to 5 */

static inline void gaol_fegetexceptflag(fexcept_t *flagp, int excepts)
{
  *flagp = (fexcept_t)(_mm_getcsr() & (unsigned int)excepts & GAOL_X86_FLAG_BITS);
}

static inline void gaol_fesetexceptflag(const fexcept_t *flagp, int excepts)
{
  const unsigned int keep = (unsigned int)excepts & GAOL_X86_FLAG_BITS;
  const unsigned int want = (unsigned int)(*flagp) & keep;
  _mm_setcsr((_mm_getcsr() & ~keep) | want);
}

#define fegetexceptflag(f, e) gaol_fegetexceptflag((f), (e))
#define fesetexceptflag(f, e) gaol_fesetexceptflag((f), (e))

#endif /* a 32-bit x86 Windows */

/* The math library of Windows has no roundeven(), which the sources call
   through __builtin_roundeven(): GCC and Clang turn that builtin into one
   instruction where the processor has it (roundsd of SSE4.1, frintn on ARM)
   and into a call to roundeven() otherwise, and a program linking GAOL there
   stopped on "undefined reference to roundeven" (the continuous integration,
   with MinGW-w64 15 and MSYS2, where GAOL is compiled without the AVX
   instructions). The function of gaol/gaol_roundeven.h is given instead. */
#if defined(_WIN32) || defined(__MINGW32__) || defined(__CYGWIN__)
#define __builtin_roundeven(x) gaol_roundeven(x)
#endif /* Windows */

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
#define __builtin_ldexp(x, n) ldexp(x, n)
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
