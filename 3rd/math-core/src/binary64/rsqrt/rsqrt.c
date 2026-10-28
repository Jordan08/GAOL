/* Correctly-rounded reciprocal square root of binary64 value.

Copyright (c) 2022-2026 Alexei Sibidanov <sibid@uvic.ca>.

This file is part of the CORE-MATH project
(https://core-math.gitlabpages.inria.fr/).

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
*/

#ifdef CORE_MATH_SUPPORT_ERRNO
#include <errno.h>
#endif
#include <fenv.h> // for FE_TONEAREST, FE_UPWARD, feraiseexcept, FE_INVALID
#include <stdint.h>
#ifdef __x86_64__
#include <x86intrin.h>
#endif

// Warning: clang also defines __GNUC__
#if defined(__GNUC__) && !defined(__clang__)
#pragma GCC diagnostic ignored "-Wunknown-pragmas"
#endif

#pragma STDC FENV_ACCESS ON

// This code emulates the _mm_getcsr SSE intrinsic by reading the FPCR register.
// fegetexceptflag accesses the FPSR register, which seems to be much slower
// than accessing FPCR, so it should be avoided if possible.
// Adapted from sse2neon: https://github.com/DLTcollab/sse2neon
#if defined(__arm64__) || defined(_M_ARM64) || defined(__aarch64__)
#if defined(_MSC_VER)
#include <arm64intr.h>
#endif

typedef struct
{
  uint16_t res0;
  uint8_t  res1  : 6;
  uint8_t  bit22 : 1;
  uint8_t  bit23 : 1;
  uint8_t  bit24 : 1;
  uint8_t  res2  : 7;
  uint32_t res3;
} fpcr_bitfield;

inline static unsigned int get_arm_rounding_mode(void)
{
  union
  {
    fpcr_bitfield field;
    uint64_t value;
  } r;

#if defined(_MSC_VER) && !defined(__clang__)
  r.value = _ReadStatusReg(ARM64_FPCR);
#else
  __asm__ __volatile__("mrs %0, FPCR" : "=r"(r.value));
#endif
  static const unsigned int lut[2][2] = {{FE_TONEAREST, FE_UPWARD}, {FE_DOWNWARD, FE_TOWARDZERO}};
  return lut[r.field.bit23][r.field.bit22];
}
#endif  // defined(__arm64__) || defined(_M_ARM64) || defined(__aarch64__)

static inline int get_rounding_mode (void)
{
#if defined(__x86_64__)
  /* GAOL: the rounding field of MXCSR is 0 to nearest, 1 downward, 2 upward
     and 3 toward zero, whatever values the C library gives FE_TONEAREST, ...
     (0x400 for FE_DOWNWARD with GNU libc and mingw-w64 before 13, 0x100 with
     the UCRT of Windows and later mingw-w64, 1 with newlib; clang-cl and
     Cygwin define __x86_64__ and not __WIN32__): look them up. Patch 6 of
     3rd/README.md. */
  static const unsigned lut[4] = {FE_TONEAREST, FE_DOWNWARD, FE_UPWARD, FE_TOWARDZERO}; /* GAOL */
  return lut[_MM_GET_ROUNDING_MODE()>>13]; /* GAOL */
#elif defined(__arm64__) || defined(_M_ARM64) || defined(__aarch64__)
  return get_arm_rounding_mode();
#else
  return fegetround ();
#endif
}

/* GAOL: u128 is the 128-bit unsigned integer of gaol/gaol_u128.h: the type of
   the compiler where it has one (unsigned __int128, or _BitInt(128) of C23),
   and the structure of two 64-bit halves with Visual C++, which has none on
   any architecture, and on the 32-bit targets of GCC. Where it is that
   structure, the code below calls gaol_u128_*() rather than the operators of
   the language. */
typedef gaol_u128 u128; /* GAOL */

typedef uint64_t u64;
typedef int64_t i64;
typedef union {double f; uint64_t u;} b64u64_u;

static double __attribute__((noinline)) as_rsqrt_refine(double rf, double a){
  b64u64_u ir = {.f = rf}, ia = {.f = a};
  if(ia.u < 1ll<<52){
    i64 nz = __builtin_clzll(ia.u);
    ia.u <<= nz - 11;
    ia.u &= ~0ull>>12;
    i64 e = nz - 12;
    ia.u |= e<<52;
  }
  if(ia.u<<11 == 1ull<<63){
  } else {
    unsigned mode = get_rounding_mode ();
    int e = (ia.u>>52)&1;
    u64 rm, am;
    rm = (ir.u<<11|1ull<<63)>>11;
    am = ((ia.u&(~0ull>>12))|1ull<<52)<<(5-e);
    u128 rt = gaol_u128_mul64(rm, am); /* GAOL */
    u64 rth = gaol_u128_hi(rt), rtl = gaol_u128_lo(rt); /* GAOL */
    u128 rrt = gaol_u128_mul64(rtl, rm); /* GAOL */
    u64 t0 = gaol_u128_lo(rrt), t1 = gaol_u128_hi(rrt) + rth*rm; /* GAOL */
    rrt = gaol_u128_make(t1, t0); /* GAOL */
    i64 s = gaol_u128_hi(rrt)>>63, dd = 1 - 2*s; /* GAOL */
    /* GAOL: ((rt<<1)^(-s)) + s, -s being 0 or all ones, negates rt<<1 when s is 1 */
    u128 rts = s ? gaol_u128_neg(gaol_u128_shl(rt, 1)) : gaol_u128_shl(rt, 1); /* GAOL */
    u128 prrt;
    u64 am2 = am<<1, am20 = -am;
    do {
      ir.u -= dd;
      prrt = rrt;
      am20 += am2;
      u128 tt = gaol_u128_sub(rts, gaol_u128_of(am20)); /* GAOL */
      rrt = gaol_u128_sub(rrt, tt); /* GAOL */
    } while(__builtin_expect(!((gaol_u128_hi(prrt)^gaol_u128_hi(rrt))>>63), 0)); /* GAOL */
    ir.u += (gaol_u128_hi(rrt)>>63)?0:dd; /* GAOL */
    rrt = (gaol_u128_hi(rrt)>>63)?rrt:prrt; /* GAOL */
    if(__builtin_expect(mode==FE_TONEAREST, 1)){
      rm = (ir.u<<11|1ull<<63)>>11;
      rt = gaol_u128_mul64(rm, am); /* GAOL */
      rrt = gaol_u128_add64(rrt, am>>2); /* GAOL */
      rrt = gaol_u128_add(rrt, rt); /* GAOL */
      u64 inc = gaol_u128_hi(rrt)>>63; /* GAOL */
      ir.u += inc;
    } else {
      ir.u += mode==FE_UPWARD;
    }
    rf = ir.f;
  }
  return rf;
}

double cr_rsqrt(double x){
  b64u64_u ix = {.f = x};
  double r;
  int e = ix.u>>52;
  if(__builtin_expect(!e, 0)) {
    if (ix.u) // x <> +0
      r = __builtin_sqrt(x)/x;
    else {
#ifdef CORE_MATH_SUPPORT_ERRNO
      errno = ERANGE; // pole error
#endif
      return 1.0 / 0.0; // case x = +0
    }
  } else if(__builtin_expect(e<0x7fd, 1)){
    // common case: 2^-1022 <= x < 2^1022
    r = (1.0 / x) * __builtin_sqrt(x);
  } else if(__builtin_expect(e<0x7ff, 1)){
      r = (4.0 / x) * (0.25 * __builtin_sqrt(x));
  } else {
    u64 ax = ix.u<<1;
    if (!ax) {
#ifdef CORE_MATH_SUPPORT_ERRNO
      errno = ERANGE; // pole error
#endif
      return 1.0 / -0.0; // x=-0
    }
    if (ax > 0x7ffull<<53 ) return x + x; // NaN
    if (e >> 11){ // x < 0
#ifdef CORE_MATH_SUPPORT_ERRNO
      errno = EDOM;
#endif
      feraiseexcept (FE_INVALID);
      return __builtin_nan("<0");
    }
    return 0.0; // +Inf
  }
  // use Newton iteration r' = r - r/2*(r^2*x-1)
  double rx = r*x, drx = __builtin_fma (r, x, -rx);
  // rx + drx approximates r*x with about 106 bits
  double h = __builtin_fma (r, rx, -1.0) + r * drx, dr = (r * 0.5) * h;
  // h approximates r^2*x-1
  // dr approximates the correction term r/2*(r^2*x-1)
  double rf = r - dr;
  b64u64_u idr = {.f = dr}, ir = {.f = r};
  int de = (ir.u>>52)-((idr.u>>52)&0x7ff);
  u64 d = (idr.u + 18)&~0ull>>13;
  if(__builtin_expect(d<=23||de>100, 0))
    rf = as_rsqrt_refine(rf, x);
  return rf;
}

#ifdef __INTEL_CLANG_COMPILER // rsqrt is called invsqrt with icx
extern double invsqrt (double);
double rsqrt(double x){
  return invsqrt(x);
}
#endif
