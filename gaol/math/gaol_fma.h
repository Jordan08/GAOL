/*-*-C-*--------------------------------------------------------------------
 * GAOL -- not Just Another Interval Library
 *--------------------------------------------------------------------------
 * fma(a, b, c) = a*b + c rounded once, in the rounding direction in effect,
 * computed with integers: the fma() of GAOL where the processor has no
 * fused multiply-add instruction and the C library's fma() is wrong (GAOL v5).
 *
 * CORE-MATH computes with __builtin_fma(), which is one instruction with
 * -mfma (GAOL_FMA) and a call of the fma() of the C library without, and the
 * exact products of GAOL (gaol/core/gaol_interval_core.cpp) call it as well:
 * the functions and the bounds are right only if that fma() is correctly
 * rounded, in the four rounding directions. The fma() of mingw-w64's own math
 * library (math/fma.c, the same code from version 9 to 13), which every
 * mingw-w64 before version 12 links and the later ones linked with msvcrt.dll
 * rather than the UCRT (GAOL_MINGW_OWN_MATH, gaol/gaol_config.h), adds the
 * four products of the halves of a and b to c with four roundings, and the
 * fma() of Cygwin is not correct either: 10.7 % of the error-free products
 * fma(a, b, -a*b) were wrong with mingw-w64 11 for x86-64, and the bounds of
 * tan, asin, atan and others did not enclose the exact values. With the
 * instruction (-mfma, which GAOL_FMA gives GCC and Clang for Windows on a
 * processor that has it) the library's function is never called; this one
 * takes its place where there is no instruction (GAOL_SOFT_FMA of
 * gaol/gaol_config.h is 1): GAOL_FMA=OFF, a processor without the
 * instructions (before Intel Haswell and AMD Piledriver), or a build for
 * such a processor. Every other platform keeps the fma() of its C library or
 * its instruction.
 *
 * The product of the two 53-bit significands is exact on 106 bits, c is
 * aligned to it with what falls below the 126 bits kept replaced by a single
 * bit that says "something is left", the sum or the difference is made on 128
 * bits, and the result is rounded once to 53 bits, or fewer in the subnormal
 * range, in the direction asked. The bits that go below the kept ones can
 * change the rounding only when the operands are more than 20 bits apart, and
 * the leading bit of the result is then at most one below that of the
 * larger operand: the 70 or so bits that stay below the 53 of the result are
 * far more than a rounding needs.
 * Only integer operations are made, and the result is built from its bits,
 * so that the function does not depend on the unit that computes the doubles
 * (the x87 unit of 32-bit x86 included), and raises no exception flag of its
 * own: where a result is inexact or overflows, the flags the instruction would
 * raise are not raised, which GAOL does not read. The NaN and the infinities
 * are given by double operations, which round nothing, and the zeros by their
 * bits.
 *
 * The direction in effect is read from MXCSR on x86, where GAOL's doubles
 * are rounded (see gaol/math/gaol_core_math_port.h), and by fegetround()
 * elsewhere; gaol_fma_directed() takes it as an argument. Both are exact
 * whatever the direction, which tests/core_math.cpp checks on every platform
 * against exact values, and, where the processor has the instruction,
 * against it.
 *
 * This header is C, and is included both by GAOL's C++ sources and by
 * gaol/math/gaol_core_math_port.h, which is why it has C comments only, and
 * the macros below for what C and C++ do not write the same way. It is not
 * installed.
 *--------------------------------------------------------------------------
 * Copyright (c) 2026 ENSTA, France
 *
 * By: Jordan NININ (created 2026-11-18)
 *--------------------------------------------------------------------------
 * GAOL is a software distributed WITHOUT ANY WARRANTY. Read the associated
 * COPYING.LIB file for information.
 *--------------------------------------------------------------------------*/

#ifndef GAOL_FMA_H
#define GAOL_FMA_H

#include <stdint.h>
#include <string.h>

#if defined(_MSC_VER) && !defined(__clang__) && defined(_M_X64)
#  include <intrin.h>
#endif

#if defined(__x86_64__) || defined(_M_X64) || defined(__i386__) || defined(_M_IX86)
#  include <xmmintrin.h>
#else
#  include <fenv.h>
#endif

#if defined(_MSC_VER) && !defined(__clang__)
#  define GAOL_FMA_INLINE static __forceinline
#else
#  define GAOL_FMA_INLINE static inline
#endif

/* A conversion: static_cast in C++, where the casts of C are warned of
   (-Wold-style-cast, which .github/scripts/headers.sh gives), and the cast of
   C for the sources of CORE-MATH */
#ifdef __cplusplus
#  define GAOL_FMA_CAST(type, x) static_cast<type>(x)
#else
#  define GAOL_FMA_CAST(type, x) ((type)(x))
#endif

/* The rounding directions of gaol_fma_directed(), whatever the values of the
   FE_* of the C library */
#define GAOL_FMA_TO_NEAREST 0
#define GAOL_FMA_UPWARD 1
#define GAOL_FMA_DOWNWARD 2
#define GAOL_FMA_TOWARD_ZERO 3

/* The number of zero bits above the leading bit of x, for x not 0: the
   instruction where the compiler has a builtin for it (GCC and Clang, whatever
   the target), the search of the bit by halves elsewhere */
GAOL_FMA_INLINE int gaol_fma_clz64(uint64_t x)
{
#if defined(__GNUC__)
  return __builtin_clzll(x);
#else
  int n = 0;
  if ((x >> 32) == 0) { n += 32; x <<= 32; }
  if ((x >> 48) == 0) { n += 16; x <<= 16; }
  if ((x >> 56) == 0) { n += 8; x <<= 8; }
  if ((x >> 60) == 0) { n += 4; x <<= 4; }
  if ((x >> 62) == 0) { n += 2; x <<= 2; }
  if ((x >> 63) == 0) { n += 1; }
  return n;
#endif
}

/* A 128-bit unsigned integer, as two 64-bit halves: the arithmetic below is
   written on these, and not on gaol_u128 of gaol/math/gaol_u128.h, which is
   the `_BitInt(128)` of GCC 14 and Clang for the C sources, on 32-bit targets
   too, and two halves for the C++ ones. The GCC 14.2 for 32-bit x86 (the i386
   image of the continuous integration, Debian 13) miscompiles, from -O1, the
   choice between two `_BitInt(128)` values that this function makes to find the
   larger operand: with
       if (swap) { larger = y; smaller = x; } else { larger = x; smaller = y; }
   `larger` and `smaller` are not what the branch gives, whatever follows. The
   pass is -ftree-coalesce-vars, and -fno-tree-coalesce-vars cures it. Here it
   gave a wrong result for each triple whose c is the larger operand (a triple
   in ten), in C only: as C++, or with GAOL_U128_FORCE_EMULATION, the type is
   two halves. The accurate phases of CORE-MATH, which compute with that type,
   passed their tests with this compiler, which is no proof that none of them
   takes that choice. gaol_u128.h takes the two halves for 32-bit x86 with GCC
   since (tests/u128_select.c). These helpers are the textbook ones that
   gaol_u128.h has for Visual C++ and the 32-bit targets, which tests/u128.cpp
   checks against the native type. */
typedef struct { uint64_t hi; uint64_t lo; } gaol_fma_u128;

GAOL_FMA_INLINE gaol_fma_u128 gaol_fma_make(uint64_t hi, uint64_t lo)
{
  gaol_fma_u128 r;
  r.hi = hi;
  r.lo = lo;
  return r;
}

/* The 128-bit product of two 64-bit values: the multiplication of the
   processor where the compiler has a type for it (`unsigned __int128` of GCC
   and Clang on 64-bit targets, _umul128 of Visual C++ for x64), four products
   of 32 bits elsewhere. Only that product is computed with the type of the
   compiler: it is one multiplication, and no choice between two 128-bit values
   (see above) */
GAOL_FMA_INLINE gaol_fma_u128 gaol_fma_mul64(uint64_t a, uint64_t b)
{
#if defined(__SIZEOF_INT128__) && !defined(GAOL_U128_FORCE_EMULATION)
  const unsigned __int128 p = GAOL_FMA_CAST(unsigned __int128, a) * b;
  return gaol_fma_make(GAOL_FMA_CAST(uint64_t, p >> 64), GAOL_FMA_CAST(uint64_t, p));
#elif defined(_MSC_VER) && !defined(__clang__) && defined(_M_X64) && !defined(GAOL_U128_FORCE_EMULATION)
  uint64_t hi;
  const uint64_t lo = _umul128(a, b, &hi);
  return gaol_fma_make(hi, lo);
#else
  const uint64_t a0 = a & 0xffffffffull, a1 = a >> 32;
  const uint64_t b0 = b & 0xffffffffull, b1 = b >> 32;
  const uint64_t p00 = a0 * b0, p01 = a0 * b1, p10 = a1 * b0, p11 = a1 * b1;
  /* the two middle products are added on 65 bits, the carry going to p11 */
  const uint64_t mid = (p00 >> 32) + (p01 & 0xffffffffull) + (p10 & 0xffffffffull);
  return gaol_fma_make(p11 + (p01 >> 32) + (p10 >> 32) + (mid >> 32),
                       (mid << 32) | (p00 & 0xffffffffull));
#endif
}

GAOL_FMA_INLINE gaol_fma_u128 gaol_fma_add(gaol_fma_u128 a, gaol_fma_u128 b)
{
  const uint64_t lo = a.lo + b.lo;
  return gaol_fma_make(a.hi + b.hi + (lo < a.lo ? 1 : 0), lo);
}

GAOL_FMA_INLINE gaol_fma_u128 gaol_fma_sub(gaol_fma_u128 a, gaol_fma_u128 b)
{
  return gaol_fma_make(a.hi - b.hi - (a.lo < b.lo ? 1 : 0), a.lo - b.lo);
}

GAOL_FMA_INLINE gaol_fma_u128 gaol_fma_or(gaol_fma_u128 a, gaol_fma_u128 b)
{
  return gaol_fma_make(a.hi | b.hi, a.lo | b.lo);
}

/* Shifts by n of 0 to 127 */
GAOL_FMA_INLINE gaol_fma_u128 gaol_fma_shl(gaol_fma_u128 a, int n)
{
  if (n == 0) {
    return a;
  }
  if (n >= 64) {
    return gaol_fma_make(a.lo << (n - 64), 0);
  }
  return gaol_fma_make((a.hi << n) | (a.lo >> (64 - n)), a.lo << n);
}

GAOL_FMA_INLINE gaol_fma_u128 gaol_fma_shr(gaol_fma_u128 a, int n)
{
  if (n == 0) {
    return a;
  }
  if (n >= 64) {
    return gaol_fma_make(0, a.hi >> (n - 64));
  }
  return gaol_fma_make(a.hi >> n, (a.lo >> n) | (a.hi << (64 - n)));
}

/* 2^n, for n of 0 to 127 */
GAOL_FMA_INLINE gaol_fma_u128 gaol_fma_bit(int n)
{
  const uint64_t one = 1;
  return n >= 64 ? gaol_fma_make(one << (n - 64), 0) : gaol_fma_make(0, one << n);
}

GAOL_FMA_INLINE int gaol_fma_lt(gaol_fma_u128 a, gaol_fma_u128 b)
{
  return a.hi != b.hi ? a.hi < b.hi : a.lo < b.lo;
}

GAOL_FMA_INLINE int gaol_fma_eq(gaol_fma_u128 a, gaol_fma_u128 b)
{
  return a.hi == b.hi && a.lo == b.lo;
}

GAOL_FMA_INLINE uint64_t gaol_fma_bits(double x)
{
  uint64_t u;
  memcpy(&u, &x, sizeof u);
  return u;
}

GAOL_FMA_INLINE double gaol_fma_double(uint64_t u)
{
  double x;
  memcpy(&x, &u, sizeof x);
  return x;
}

/* The finite nonzero double of bits u as |x| = m 2^e, m having its leading bit
   at 52 whatever the number: a subnormal number is shifted up, and its
   exponent comes down by as much */
GAOL_FMA_INLINE void gaol_fma_split(uint64_t u, uint64_t* m, int* e)
{
  const int field = GAOL_FMA_CAST(int, (u >> 52) & 0x7ff);
  const uint64_t fraction = u & 0x000fffffffffffffull;
  if (field == 0) {
    const int shift = gaol_fma_clz64(fraction) - 11;
    *m = fraction << shift;
    *e = -1074 - shift;
  } else {
    *m = fraction | 0x0010000000000000ull;
    *e = field - 1075;
  }
}

/* fma(a, b, c) rounded once in `direction`, one of GAOL_FMA_TO_NEAREST,
   GAOL_FMA_UPWARD, GAOL_FMA_DOWNWARD and GAOL_FMA_TOWARD_ZERO */
GAOL_FMA_INLINE double gaol_fma_directed(double a, double b, double c, int direction)
{
  const uint64_t ua = gaol_fma_bits(a);
  const uint64_t ub = gaol_fma_bits(b);
  const uint64_t uc = gaol_fma_bits(c);
  const uint64_t one = 1;
  const int fa = GAOL_FMA_CAST(int, (ua >> 52) & 0x7ff);
  const int fb = GAOL_FMA_CAST(int, (ub >> 52) & 0x7ff);
  const int fc = GAOL_FMA_CAST(int, (uc >> 52) & 0x7ff);
  const int sign_p = GAOL_FMA_CAST(int, (ua ^ ub) >> 63); /* the sign of a*b */
  const int c_is_zero = (uc << 1) == 0;
  uint64_t ma, mb, mc = 0;
  int ea, eb, ec = 0;
  gaol_fma_u128 product, x, t;
  int shift, ex, el, sign_r;
  int m, q, inexact, cmp, up;
  uint64_t mant, bits;

  /* A NaN or an infinity: a*b + c in doubles, which rounds nothing, gives
     the NaN, the infinity or the invalid operation (infinity * 0, infinity
     - infinity) of the standard, but for c alone, an infinity or a NaN
     with a and b finite, whose product may overflow in a double without
     the exact a*b + c being any less c */
  if (fa == 0x7ff || fb == 0x7ff || fc == 0x7ff) {
    if (fc == 0x7ff && fa != 0x7ff && fb != 0x7ff) {
      return c;
    }
    return a * b + c;
  }

  /* A zero factor: the product is a zero, and the sum is c; two zeros add to
     the zero of the sign they share, or to +0 (-0 rounding downward) */
  if ((ua << 1) == 0 || (ub << 1) == 0) {
    if (!c_is_zero) {
      return c;
    }
    {
      const uint64_t sc = uc >> 63;
      const uint64_t sp = GAOL_FMA_CAST(uint64_t, sign_p);
      const uint64_t negative = (sp == sc) ? sp : (direction == GAOL_FMA_DOWNWARD ? one : 0);
      return gaol_fma_double(negative << 63);
    }
  }

  /* a*b = product 2^(ea + eb), exactly: 2^104 <= product < 2^106. x is it
     shifted up so that its leading bit is the 125th, which leaves two bits
     above it for the carry of a sum, and x 2^ex is its value */
  gaol_fma_split(ua, &ma, &ea);
  gaol_fma_split(ub, &mb, &eb);
  product = gaol_fma_mul64(ma, mb);
  shift = (product.hi >> 41) != 0 ? 20 : 21;
  x = gaol_fma_shl(product, shift);
  ex = ea + eb - shift;

  if (c_is_zero) {
    /* Nothing to add: the product, rounded */
    t = x;
    el = ex;
    sign_r = sign_p;
  } else {
    /* c = y 2^ey with the leading bit of y at the 125th too: the operand of
       the larger exponent is the larger, and the one of equal exponents of
       the larger bits */
    const int sign_c = GAOL_FMA_CAST(int, uc >> 63);
    gaol_fma_u128 y, larger, smaller;
    int ey, es, d;
    gaol_fma_split(uc, &mc, &ec);
    y = gaol_fma_shl(gaol_fma_make(0, mc), 73);
    ey = ec - 73;
    if (ex > ey || (ex == ey && !gaol_fma_lt(x, y))) {
      larger = x; el = ex; sign_r = sign_p;
      smaller = y; es = ey;
    } else {
      larger = y; el = ey; sign_r = sign_c;
      smaller = x; es = ex;
    }
    /* The smaller one, brought to the exponent of the larger, with a single
       bit for everything that falls below the bits kept. It is exact up to 20
       bits of difference, the 20 low bits of the product being zero, and no
       cancellation of more than one bit happens beyond. */
    d = el - es;
    if (d >= 128) {
      smaller = gaol_fma_make(0, 1);
    } else if (d > 0) {
      const gaol_fma_u128 shifted = gaol_fma_shr(smaller, d);
      smaller = gaol_fma_eq(gaol_fma_shl(shifted, d), smaller)
              ? shifted : gaol_fma_or(shifted, gaol_fma_make(0, 1));
    }
    if (sign_p == sign_c) {
      t = gaol_fma_add(larger, smaller);
    } else {
      t = gaol_fma_sub(larger, smaller);
      if (gaol_fma_eq(t, gaol_fma_make(0, 0))) {
        /* a*b = -c exactly: +0, and -0 rounding downward */
        return gaol_fma_double(direction == GAOL_FMA_DOWNWARD ? one << 63 : 0);
      }
    }
  }

  /* t 2^el, t not 0 and below 2^127: kept on 53 bits from its leading bit m,
     or on fewer, down to the unit 2^-1074 of the subnormal numbers, which
     makes the number of low bits q that go. mant is what is kept, and q <= 0
     means nothing goes and the number is exact. */
  m = t.hi != 0 ? 127 - gaol_fma_clz64(t.hi)
                           : 63 - gaol_fma_clz64(t.lo);
  q = m - 52;
  if (q < -1074 - el) {
    q = -1074 - el;
  }
  inexact = 0;
  cmp = -1; /* the bits that go, compared with half of the unit kept */
  if (q <= 0) {
    mant = gaol_fma_shl(t, -q).lo;
  } else if (q >= 128) {
    /* Below half the least subnormal number: t is under 2^127 */
    mant = 0;
    inexact = 1;
  } else {
    const gaol_fma_u128 kept = gaol_fma_shr(t, q);
    const gaol_fma_u128 rest = gaol_fma_sub(t, gaol_fma_shl(kept, q));
    const gaol_fma_u128 half = gaol_fma_bit(q - 1);
    mant = kept.lo;
    inexact = !gaol_fma_eq(rest, gaol_fma_make(0, 0));
    cmp = gaol_fma_lt(rest, half) ? -1 : (gaol_fma_eq(rest, half) ? 0 : 1);
  }
  up = 0;
  if (inexact) {
    if (direction == GAOL_FMA_TO_NEAREST) {
      up = cmp > 0 || (cmp == 0 && (mant & 1) != 0);
    } else if (direction == GAOL_FMA_UPWARD) {
      up = !sign_r;
    } else if (direction == GAOL_FMA_DOWNWARD) {
      up = sign_r;
    }
  }
  if (up) {
    mant += 1;
  }

  /* The number is mant 2^(el + q), mant 2^53 when rounding went up from
     2^53 - 1. Its bits are those of mant added to the exponent field of the
     unit: the leading bit of mant, the 52nd, adds the 1 of the field of a
     normal number, a mant below 2^52 is a subnormal number (the field is 0
     then, el + q being -1074), and the carry of 2^53 goes into the exponent.
     The result is 0 when nothing is left of a number that is below the unit. */
  if (el + q + 1074 + GAOL_FMA_CAST(int, mant >> 52) >= 2047) {
    /* Overflow: the infinity, or the largest double in the directions that
       round toward zero for this sign */
    const int to_infinity = direction == GAOL_FMA_TO_NEAREST
                         || (direction == GAOL_FMA_UPWARD && !sign_r)
                         || (direction == GAOL_FMA_DOWNWARD && sign_r);
    bits = to_infinity ? 0x7ff0000000000000ull : 0x7fefffffffffffffull;
  } else {
    bits = (GAOL_FMA_CAST(uint64_t, el + q + 1074) << 52) + mant;
  }
  return gaol_fma_double(bits | (GAOL_FMA_CAST(uint64_t, sign_r) << 63));
}

/* The rounding direction in effect, as GAOL_FMA_*: the one of MXCSR on x86,
   where the doubles of GAOL are computed, and the one fegetround() gives
   elsewhere */
GAOL_FMA_INLINE int gaol_fma_current_direction(void)
{
#if defined(__x86_64__) || defined(_M_X64) || defined(__i386__) || defined(_M_IX86)
  switch (_mm_getcsr() & _MM_ROUND_MASK) {
    case _MM_ROUND_NEAREST: return GAOL_FMA_TO_NEAREST;
    case _MM_ROUND_UP: return GAOL_FMA_UPWARD;
    case _MM_ROUND_DOWN: return GAOL_FMA_DOWNWARD;
    default: return GAOL_FMA_TOWARD_ZERO;
  }
#else
  switch (fegetround()) {
    case FE_UPWARD: return GAOL_FMA_UPWARD;
    case FE_DOWNWARD: return GAOL_FMA_DOWNWARD;
    case FE_TOWARDZERO: return GAOL_FMA_TOWARD_ZERO;
    default: return GAOL_FMA_TO_NEAREST;
  }
#endif
}

/* fma(a, b, c) rounded once in the direction in effect */
GAOL_FMA_INLINE double gaol_fma(double a, double b, double c)
{
  return gaol_fma_directed(a, b, c, gaol_fma_current_direction());
}

#endif /* GAOL_FMA_H */
