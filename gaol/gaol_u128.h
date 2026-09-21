/*-*-C-*-----------------------------------------------------------------------
 * gaol -- NOT Just Another Interval Library
 *------------------------------------------------------------------------------
 * The 128-bit unsigned integer that the accurate phases of CORE-MATH's log,
 * sin, cos, tan, atan2 and pow, and of log2p1, log10p1, atan2pi, hypot, rsqrt
 * and asinpi, compute with, on every compiler GAOL is built with. asinpi
 * computes with a signed 128-bit integer too, which a gaol_u128 holds in two's
 * complement, as the signed type of the compiler holds it:
 * gaol_u128_imul64(), gaol_u128_of_i64() and gaol_u128_sar() are the product,
 * the conversion and the shift of that signed type.
 *
 * GCC and Clang have a 128-bit integer type on 64-bit targets
 * (`unsigned __int128`), and Clang 14 and GCC 14 have `_BitInt(128)`, which
 * Clang provides on 32-bit targets too. Visual C++ has neither, whatever the
 * architecture, and so has GCC for a 32-bit target: there GAOL computes with
 * the structure of two 64-bit halves below, and the sources of CORE-MATH call
 * the functions of this header rather than the operators of the language
 * (`#if GAOL_U128_NATIVE` in their extended-arithmetic functions).
 *
 * The functions are written so that both paths give the same bits: the
 * emulated ones are the textbook algorithms on two halves, and
 * tests/u128.cpp compares them with the native type over random values and
 * over the values at the ends of the ranges.
 *
 * Copyright (c) 2026 ENSTA, France
 *
 * Created 2026-09-20 by Jordan NININ
 *------------------------------------------------------------------------------
 * gaol is a software distributed WITHOUT ANY WARRANTY. Read the associated
 * COPYING file for information.
 *----------------------------------------------------------------------------*/

#ifndef __gaol_u128_h__
#define __gaol_u128_h__

#include <stdint.h>

/* GAOL_U128_FORCE_EMULATION compiles the two halves even where the compiler
   has a 128-bit type, which is how tests/u128.cpp compares the two paths and
   how the continuous integration runs the code of Visual C++ and of the
   32-bit targets on an ordinary machine. */
#if defined(GAOL_U128_FORCE_EMULATION)
#  define GAOL_U128_NATIVE 0
#  if defined(__BYTE_ORDER__) && defined(__ORDER_BIG_ENDIAN__) && __BYTE_ORDER__ == __ORDER_BIG_ENDIAN__
typedef struct { uint64_t h; uint64_t l; } gaol_u128;
#  else
typedef struct { uint64_t l; uint64_t h; } gaol_u128;
#  endif
/* `unsigned __int128` of GCC and Clang, on every 64-bit target. */
#elif defined(__SIZEOF_INT128__)
#  define GAOL_U128_NATIVE 1
typedef unsigned __int128 gaol_u128;
typedef __int128 gaol_s128; /* the signed type, for gaol_u128_imul64() and gaol_u128_sar() */
/* `_BitInt(128)` of C23: Clang 14 has it on 32-bit targets too, GCC 14 only
   where the ABI defines it, which `__BITINT_MAXWIDTH__` tells. */
#elif (defined(__clang__) && __clang_major__ >= 14) \
   || (defined(__GNUC__) && __GNUC__ >= 14 && defined(__BITINT_MAXWIDTH__) && __BITINT_MAXWIDTH__ >= 128)
#  define GAOL_U128_NATIVE 1
typedef unsigned _BitInt(128) gaol_u128;
typedef _BitInt(128) gaol_s128; /* the signed type, for gaol_u128_imul64() and gaol_u128_sar() */
#else
#  define GAOL_U128_NATIVE 0
/* The halves are ordered as the union of CORE-MATH's uint128_t orders them,
   so that a gaol_u128 and that union hold the same bits at the same places. */
#  if defined(__BYTE_ORDER__) && defined(__ORDER_BIG_ENDIAN__) && __BYTE_ORDER__ == __ORDER_BIG_ENDIAN__
typedef struct { uint64_t h; uint64_t l; } gaol_u128;
#  else
typedef struct { uint64_t l; uint64_t h; } gaol_u128;
#  endif
#endif

#if defined(_MSC_VER) && !defined(__clang__)
#  include <intrin.h>
#endif

#if defined(__cplusplus)
#  define GAOL_U128_INLINE inline
#else
#  define GAOL_U128_INLINE static inline
#endif

#if GAOL_U128_NATIVE

/* h*2^64 + l, which the two halves had and the native type had not: the port
   of rsqrt builds a 128-bit integer from its halves (GAOL v5) */
GAOL_U128_INLINE gaol_u128 gaol_u128_make(uint64_t h, uint64_t l) { return ((gaol_u128)h << 64) | l; }
GAOL_U128_INLINE gaol_u128 gaol_u128_of(uint64_t a) { return (gaol_u128)a; }
GAOL_U128_INLINE uint64_t gaol_u128_lo(gaol_u128 a) { return (uint64_t)a; }
GAOL_U128_INLINE uint64_t gaol_u128_hi(gaol_u128 a) { return (uint64_t)(a >> 64); }
GAOL_U128_INLINE gaol_u128 gaol_u128_mul64(uint64_t a, uint64_t b) { return (gaol_u128)a * (gaol_u128)b; }
GAOL_U128_INLINE gaol_u128 gaol_u128_mul(gaol_u128 a, gaol_u128 b) { return a * b; }
GAOL_U128_INLINE gaol_u128 gaol_u128_add(gaol_u128 a, gaol_u128 b) { return a + b; }
GAOL_U128_INLINE gaol_u128 gaol_u128_add64(gaol_u128 a, uint64_t b) { return a + b; }
GAOL_U128_INLINE gaol_u128 gaol_u128_sub(gaol_u128 a, gaol_u128 b) { return a - b; }
GAOL_U128_INLINE gaol_u128 gaol_u128_neg(gaol_u128 a) { return -a; }
GAOL_U128_INLINE gaol_u128 gaol_u128_shl(gaol_u128 a, int n) { return a << n; }
GAOL_U128_INLINE gaol_u128 gaol_u128_shr(gaol_u128 a, int n) { return a >> n; }
GAOL_U128_INLINE gaol_u128 gaol_u128_or(gaol_u128 a, gaol_u128 b) { return a | b; }
GAOL_U128_INLINE gaol_u128 gaol_u128_and64(gaol_u128 a, uint64_t b) { return a & b; }
GAOL_U128_INLINE gaol_u128 gaol_u128_bit(int n) { return (gaol_u128)1 << n; }
GAOL_U128_INLINE int gaol_u128_lt(gaol_u128 a, gaol_u128 b) { return a < b; }
GAOL_U128_INLINE int gaol_u128_gt(gaol_u128 a, gaol_u128 b) { return a > b; }
GAOL_U128_INLINE int gaol_u128_eq(gaol_u128 a, gaol_u128 b) { return a == b; }
GAOL_U128_INLINE gaol_u128 gaol_u128_and(gaol_u128 a, gaol_u128 b) { return a & b; }
/* The signed values, in two's complement (the port of asinpi, GAOL v5): the
   conversion of an int64_t, which extends its sign, the product of two
   int64_t, and the shift right that copies the sign bit, which GCC and Clang
   make of >> on a negative value of the signed type (the C standard leaves
   it to the compiler, and so does the conversion to the signed type of a
   value above its maximum, which they make modulo 2^128). */
GAOL_U128_INLINE gaol_u128 gaol_u128_of_i64(int64_t a) { return (gaol_u128)a; }
GAOL_U128_INLINE gaol_u128 gaol_u128_imul64(int64_t a, int64_t b) { return (gaol_u128)((gaol_s128)a * b); }
GAOL_U128_INLINE gaol_u128 gaol_u128_sar(gaol_u128 a, int n) { return (gaol_u128)((gaol_s128)a >> n); }

#else /* the two halves */

GAOL_U128_INLINE gaol_u128 gaol_u128_make(uint64_t h, uint64_t l) {
  gaol_u128 r; r.h = h; r.l = l; return r;
}
GAOL_U128_INLINE gaol_u128 gaol_u128_of(uint64_t a) { return gaol_u128_make(0, a); }
GAOL_U128_INLINE uint64_t gaol_u128_lo(gaol_u128 a) { return a.l; }
GAOL_U128_INLINE uint64_t gaol_u128_hi(gaol_u128 a) { return a.h; }

/* The 128-bit product of two 64-bit values: one instruction where the
   processor has it, four 32-bit products elsewhere. */
GAOL_U128_INLINE gaol_u128 gaol_u128_mul64(uint64_t a, uint64_t b) {
#if defined(_MSC_VER) && !defined(__clang__) && defined(_M_X64)
  /* One instruction on x64; _umul128 is for x64 only */
  uint64_t h;
  uint64_t l = _umul128(a, b, &h);
  return gaol_u128_make(h, l);
#elif defined(_MSC_VER) && !defined(__clang__) && (defined(_M_ARM64) || defined(_M_ARM64EC))
  /* On ARM64, __umulh gives the high half and the ordinary product the low one
     (Visual C++ has no _umul128 there) */
  return gaol_u128_make(__umulh(a, b), a * b);
#else
  uint64_t a0 = (uint32_t)a, a1 = a >> 32;
  uint64_t b0 = (uint32_t)b, b1 = b >> 32;
  uint64_t p00 = a0 * b0, p01 = a0 * b1, p10 = a1 * b0, p11 = a1 * b1;
  /* The two middle products are added on 65 bits, the carry going to p11. */
  uint64_t mid = (p00 >> 32) + (uint32_t)p01 + (uint32_t)p10;
  uint64_t h = p11 + (p01 >> 32) + (p10 >> 32) + (mid >> 32);
  uint64_t l = (mid << 32) | (uint32_t)p00;
  return gaol_u128_make(h, l);
#endif
}

/* The low 128 bits of the product, as the operators of the language give. */
GAOL_U128_INLINE gaol_u128 gaol_u128_mul(gaol_u128 a, gaol_u128 b) {
  gaol_u128 r = gaol_u128_mul64(a.l, b.l);
  r.h += a.l * b.h + a.h * b.l;
  return r;
}

GAOL_U128_INLINE gaol_u128 gaol_u128_add(gaol_u128 a, gaol_u128 b) {
  gaol_u128 r;
  r.l = a.l + b.l;
  r.h = a.h + b.h + (r.l < a.l);
  return r;
}
GAOL_U128_INLINE gaol_u128 gaol_u128_add64(gaol_u128 a, uint64_t b) {
  gaol_u128 r;
  r.l = a.l + b;
  r.h = a.h + (r.l < a.l);
  return r;
}
GAOL_U128_INLINE gaol_u128 gaol_u128_neg(gaol_u128 a) {
  gaol_u128 r;
  r.l = ~a.l + 1;
  r.h = ~a.h + (r.l == 0);
  return r;
}
GAOL_U128_INLINE gaol_u128 gaol_u128_sub(gaol_u128 a, gaol_u128 b) {
  return gaol_u128_add(a, gaol_u128_neg(b));
}

/* Shifting by 128 or more is undefined, as it is for the native type; the
   halves are shifted so that a count of 0 and a count of 64 are right. */
GAOL_U128_INLINE gaol_u128 gaol_u128_shl(gaol_u128 a, int n) {
  gaol_u128 r;
  if (n == 0) return a;
  if (n >= 64) { r.h = a.l << (n - 64); r.l = 0; }
  else { r.h = (a.h << n) | (a.l >> (64 - n)); r.l = a.l << n; }
  return r;
}
GAOL_U128_INLINE gaol_u128 gaol_u128_shr(gaol_u128 a, int n) {
  gaol_u128 r;
  if (n == 0) return a;
  if (n >= 64) { r.l = a.h >> (n - 64); r.h = 0; }
  else { r.l = (a.l >> n) | (a.h << (64 - n)); r.h = a.h >> n; }
  return r;
}
GAOL_U128_INLINE gaol_u128 gaol_u128_or(gaol_u128 a, gaol_u128 b) {
  return gaol_u128_make(a.h | b.h, a.l | b.l);
}
GAOL_U128_INLINE gaol_u128 gaol_u128_and64(gaol_u128 a, uint64_t b) {
  return gaol_u128_make(0, a.l & b);
}
GAOL_U128_INLINE gaol_u128 gaol_u128_bit(int n) {
  return n >= 64 ? gaol_u128_make((uint64_t)1 << (n - 64), 0)
                 : gaol_u128_make(0, (uint64_t)1 << n);
}
GAOL_U128_INLINE int gaol_u128_lt(gaol_u128 a, gaol_u128 b) {
  return a.h != b.h ? a.h < b.h : a.l < b.l;
}
GAOL_U128_INLINE int gaol_u128_gt(gaol_u128 a, gaol_u128 b) {
  return a.h != b.h ? a.h > b.h : a.l > b.l;
}
GAOL_U128_INLINE int gaol_u128_eq(gaol_u128 a, gaol_u128 b) {
  return a.h == b.h && a.l == b.l;
}
GAOL_U128_INLINE gaol_u128 gaol_u128_and(gaol_u128 a, gaol_u128 b) {
  return gaol_u128_make(a.h & b.h, a.l & b.l);
}

/* The signed values, in two's complement (the port of asinpi, GAOL v5). */
/* a, its sign extended to the high half */
GAOL_U128_INLINE gaol_u128 gaol_u128_of_i64(int64_t a) {
  return gaol_u128_make(a < 0 ? UINT64_MAX : 0, (uint64_t)a);
}
/* With A and B the bits of a and b read as unsigned, a = A - 2^64 when a < 0,
   and so a*b = A*B - 2^64*(B when a < 0, plus A when b < 0) modulo 2^128. */
GAOL_U128_INLINE gaol_u128 gaol_u128_imul64(int64_t a, int64_t b) {
  gaol_u128 r = gaol_u128_mul64((uint64_t)a, (uint64_t)b);
  r.h -= (a < 0 ? (uint64_t)b : 0) + (b < 0 ? (uint64_t)a : 0);
  return r;
}
/* Shifting right, the sign bit copied into the bits vacated: shifts of
   unsigned words only, whose result the C standard defines. */
GAOL_U128_INLINE gaol_u128 gaol_u128_sar(gaol_u128 a, int n) {
  const uint64_t s = (a.h >> 63) ? UINT64_MAX : 0;
  gaol_u128 r;
  if (n == 0) return a;
  if (n >= 64) { r.h = s; r.l = n == 64 ? a.h : (a.h >> (n - 64)) | (s << (128 - n)); }
  else { r.l = (a.l >> n) | (a.h << (64 - n)); r.h = (a.h >> n) | (s << (64 - n)); }
  return r;
}

#endif /* GAOL_U128_NATIVE */

#endif /* __gaol_u128_h__ */
