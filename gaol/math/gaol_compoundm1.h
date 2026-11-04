/*-*-C++-*---------------------------------------------------------------------
 * gaol -- NOT Just Another Interval Library
 *------------------------------------------------------------------------------
 * compoundm1(x, y) = (1 + x)^y - 1 of IEEE 1788-2015 (Table 10.5) at a point,
 * with bounds that are the tightest ones but in a case out of 10^17 or so: the
 * greatest double at most the value and the least double at least it (GAOL v5).
 *
 * CORE-MATH has no function for it, and exp(y log1p(x)) - 1, computed with
 * doubles, magnifies the relative error of the logarithm by |y log1p(x)|. The
 * value is therefore computed here with integers only, in two phases:
 *  - a fast one with 64-bit fixed point, whose error bound is proved below
 *    (compoundm1_fast()): where the bounds it gives of the value hold no
 *    double, the doubles next to them are the tightest bounds (95 % of the
 *    pairs for |y log(1 + x)| below 0.35, 71 % below 8, in about a seventh of
 *    the time of the other phase);
 *  - the other one, with a 128-bit significand, in the balls below (a
 *    significand, an exponent and a bound of the absolute error), whose every
 *    operation adds the error it makes to the bound: the double the bound
 *    rounds to cannot be wrong, and the width of the ball shows how tight it
 *    is (compoundm1_balls()). It proves the values that are doubles too.
 * Only integers are used, but for the two approximations that cannot make a
 * bound wrong, the log1p() of CORE-MATH and the multiple of log(2) that is
 * nearest (a double, below 2^-9 of an error): the rounding direction in
 * effect, the exceptions and the flush-to-zero mode of the program play no
 * part, and the code is the same on every platform. gaol_u128_mul64() is the
 * one function of gaol_u128.h it needs, which is native or emulated.
 *
 * The algorithm of the balls, for finite x > -1 and y, both nonzero:
 *   1. y0 = log1p(x) from CORE-MATH, a double next to the logarithm L.
 *   2. d = (1 + x) exp(-y0) - 1, which is about L - y0, below 2^-42: a series
 *      of 27 terms for exp(r) - 1 with |r| < 1/2 after the reduction by a
 *      multiple of log(2) (Cody and Waite, in two parts that make the product
 *      with the integer exact), and for a y0 of |y0| < 0.3466 d = x + E + x E
 *      with E = exp(-y0) - 1, as 1 + x would hide a small x.
 *   3. L = y0 + d - d^2/2 + d^3/3, the next term being below d^4.
 *   4. f = exp(y L) - 1 by the same series; y L above 2048 or below -64 is
 *      past the doubles, where f is +oo and -1 (the tightest bounds there).
 *   5. The ball of f is rounded outward to doubles by integer operations. If
 *      it holds a double d, d is the value where (1 + x)^y = 1 + d is
 *      proved exactly (power_equals()), and the bounds are the next doubles
 *      on both sides of d otherwise.
 *
 * The relative width of the ball of f is below 2^-111 for |y L| below 746 (2^-118
 * below 10 and 2^-120 below 1, measured), so that its bounds are one double
 * apart unless f is within 2^-111 relatively of a double, which is a case in
 * about 2^57 for an f nobody chose. Two families of f that are close to a
 * double without being it are told apart: a tiny x, f = y x (1 + (y - 1) x / 2
 * + ...), and the powers 2^m that (1 + x)^y is, f = 2^m - 1 being below the
 * double 2^m. Others are not (the bound is one double wider on a side, as the
 * ball of f holds that double), but y = -1, where f = -x / (1 + x) is the quotient
 * of two integers that reciprocal_m1() divides, with the exact bounds: it was
 * a contrived family there, x = 2^e + 2^(2e-1) (-51 <= e <= -40), for which
 * 1/(1 + x) = 1 - x + x^2 - x^3 + ... loses its cubic term and f is within
 * 2^-155 of a double. Only the cases that the fast phase does not settle come
 * to the balls (5 % of the pairs).
 *
 * Copyright (c) 2026 ENSTA, France
 *
 * Created 2026-11-04 by Jordan NININ
 *------------------------------------------------------------------------------
 * gaol is a software distributed WITHOUT ANY WARRANTY. Read the associated
 * COPYING file for information.
 *----------------------------------------------------------------------------*/

#ifndef GAOL_COMPOUNDM1_H
#define GAOL_COMPOUNDM1_H

#include <cmath>
#include <cstdint>
#include <cstring>
#include <limits>

#include "gaol/math/gaol_core_math.h"
#include "gaol/math/gaol_u128.h"

namespace gaol_cm1
{
  typedef std::uint64_t u64;

  /* ---------------------------------------------------------------------- */
  /* 64-bit words                                                           */
  /* ---------------------------------------------------------------------- */

  // The number of leading zeros of x != 0, by bisection (every compiler)
  inline int clz64_soft(u64 x)
  {
    int n = 0;
    if ((x >> 32) == 0) { n += 32; x <<= 32; }
    if ((x >> 48) == 0) { n += 16; x <<= 16; }
    if ((x >> 56) == 0) { n += 8; x <<= 8; }
    if ((x >> 60) == 0) { n += 4; x <<= 4; }
    if ((x >> 62) == 0) { n += 2; x <<= 2; }
    if ((x >> 63) == 0) { n += 1; }
    return n;
  }

  // The number of leading zeros of x != 0: the instruction where GCC or Clang have a builtin
  inline int clz64(u64 x)
  {
#if (defined(__GNUC__) || defined(__clang__)) && !defined(GAOL_CM1_SOFT_CLZ)
    return __builtin_clzll(x);
#else
    return clz64_soft(x);
#endif
  }

  // The number of trailing zeros of x != 0
  inline int ctz64(u64 x) { return 63 - clz64(x & (~x + 1)); }

  inline void mul64(u64 a, u64 b, u64& hi, u64& lo)
  {
    const gaol_u128 p = gaol_u128_mul64(a, b);
    hi = gaol_u128_hi(p);
    lo = gaol_u128_lo(p);
  }

  // a + b + carry, the carry (0 or 1) going out in carry
  inline u64 adc(u64 a, u64 b, u64& carry)
  {
    const u64 s = a + b;
    const u64 c1 = (s < a) ? 1u : 0u;
    const u64 t = s + carry;
    const u64 c2 = (t < s) ? 1u : 0u;
    carry = c1 | c2;
    return t;
  }

  // a - b - borrow, the borrow (0 or 1) going out in borrow
  inline u64 sbb(u64 a, u64 b, u64& borrow)
  {
    const u64 d = a - b;
    const u64 b1 = (a < b) ? 1u : 0u;
    const u64 t = d - borrow;
    const u64 b2 = (d < borrow) ? 1u : 0u;
    borrow = b1 | b2;
    return t;
  }

  /* ---------------------------------------------------------------------- */
  /* 128-bit and 256-bit unsigned integers                                  */
  /* ---------------------------------------------------------------------- */

  struct U128 { u64 h, l; };
  struct U256 { u64 w[4]; }; // w[0] is the least significant word

  inline U128 mk(u64 h, u64 l) { U128 r; r.h = h; r.l = l; return r; }
  inline bool is_zero(const U128& a) { return (a.h | a.l) == 0; }
  inline bool eq(const U128& a, const U128& b) { return a.h == b.h && a.l == b.l; }
  inline bool lt(const U128& a, const U128& b) { return a.h != b.h ? a.h < b.h : a.l < b.l; }

  inline U128 add(const U128& a, const U128& b, u64& carry)
  {
    carry = 0;
    U128 r;
    r.l = adc(a.l, b.l, carry);
    r.h = adc(a.h, b.h, carry);
    return r;
  }

  inline U128 sub(const U128& a, const U128& b, u64& borrow)
  {
    borrow = 0;
    U128 r;
    r.l = sbb(a.l, b.l, borrow);
    r.h = sbb(a.h, b.h, borrow);
    return r;
  }

  // a << n and a >> n, for 0 <= n < 128
  inline U128 shl(const U128& a, int n)
  {
    if (n == 0) { return a; }
    if (n >= 64) { return mk(a.l << (n - 64), 0); }
    return mk((a.h << n) | (a.l >> (64 - n)), a.l << n);
  }

  inline U128 shr(const U128& a, int n)
  {
    if (n == 0) { return a; }
    if (n >= 64) { return mk(0, a.h >> (n - 64)); }
    return mk(a.h >> n, (a.l >> n) | (a.h << (64 - n)));
  }

  // The number of bits of a: 0 for 0, else 1 + the index of the top bit
  inline int bitlen(const U128& a)
  {
    if (a.h != 0) { return 128 - clz64(a.h); }
    if (a.l != 0) { return 64 - clz64(a.l); }
    return 0;
  }

  // The 256-bit product a b, as hi 2^128 + lo
  inline void mul_full(const U128& a, const U128& b, U128& hi, U128& lo)
  {
    u64 h00, l00, h01, l01, h10, l10, h11, l11;
    mul64(a.l, b.l, h00, l00);
    mul64(a.l, b.h, h01, l01);
    mul64(a.h, b.l, h10, l10);
    mul64(a.h, b.h, h11, l11);
    u64 carry = 0;
    u64 w1 = adc(h00, l01, carry);
    u64 c1 = carry;
    carry = 0;
    w1 = adc(w1, l10, carry);
    c1 += carry; // 0 to 2
    u64 w2 = h01;
    carry = 0;
    w2 = adc(w2, h10, carry);
    u64 c2 = carry;
    carry = 0;
    w2 = adc(w2, l11, carry);
    c2 += carry;
    carry = 0;
    w2 = adc(w2, c1, carry);
    c2 += carry; // 0 to 3
    const u64 w3 = h11 + c2;
    lo = mk(w1, l00);
    hi = mk(w3, w2);
  }

  // The high 128 bits of the product a b, rounded down
  inline U128 mul_hi(const U128& a, const U128& b)
  {
    U128 hi, lo;
    mul_full(a, b, hi, lo);
    return hi;
  }

  inline U256 u256_of(const U128& a)
  {
    U256 r;
    r.w[0] = a.l; r.w[1] = a.h; r.w[2] = 0; r.w[3] = 0;
    return r;
  }

  inline bool is_zero(const U256& a) { return (a.w[0] | a.w[1] | a.w[2] | a.w[3]) == 0; }

  // a << n, for 0 <= n < 256, the bits shifted out of the top being lost
  inline U256 shl(const U256& a, int n)
  {
    U256 r;
    const int ws = n >> 6, bs = n & 63;
    for (int i = 3; i >= 0; --i) {
      u64 v = 0;
      if (i - ws >= 0) {
        v = a.w[i - ws] << bs;
        if (bs != 0 && i - ws - 1 >= 0) {
          v |= a.w[i - ws - 1] >> (64 - bs);
        }
      }
      r.w[i] = v;
    }
    return r;
  }

  // a >> n, for n >= 0; lost is set if a bit that is 1 is shifted out
  inline U256 shr(const U256& a, int n, bool& lost)
  {
    U256 r;
    if (n >= 256) {
      r.w[0] = r.w[1] = r.w[2] = r.w[3] = 0;
      lost = lost || !is_zero(a);
      return r;
    }
    const int ws = n >> 6, bs = n & 63;
    for (int i = 0; i < ws; ++i) {
      if (a.w[i] != 0) { lost = true; }
    }
    if (bs != 0 && (a.w[ws] & ((static_cast<u64>(1) << bs) - 1)) != 0) {
      lost = true;
    }
    for (int i = 0; i < 4; ++i) {
      u64 v = 0;
      if (i + ws < 4) {
        v = a.w[i + ws] >> bs;
        if (bs != 0 && i + ws + 1 < 4) {
          v |= a.w[i + ws + 1] << (64 - bs);
        }
      }
      r.w[i] = v;
    }
    return r;
  }

  inline int bitlen(const U256& a)
  {
    for (int i = 3; i >= 0; --i) {
      if (a.w[i] != 0) { return 64 * i + 64 - clz64(a.w[i]); }
    }
    return 0;
  }

  inline int cmp(const U256& a, const U256& b)
  {
    for (int i = 3; i >= 0; --i) {
      if (a.w[i] != b.w[i]) { return a.w[i] < b.w[i] ? -1 : 1; }
    }
    return 0;
  }

  // a + b, which has to fit 256 bits
  inline U256 add(const U256& a, const U256& b)
  {
    U256 r;
    u64 carry = 0;
    for (int i = 0; i < 4; ++i) { r.w[i] = adc(a.w[i], b.w[i], carry); }
    return r;
  }

  // a - b, for a >= b
  inline U256 sub(const U256& a, const U256& b)
  {
    U256 r;
    u64 borrow = 0;
    for (int i = 0; i < 4; ++i) { r.w[i] = sbb(a.w[i], b.w[i], borrow); }
    return r;
  }

  /* ---------------------------------------------------------------------- */
  /* Magnitudes: upper bounds of errors                                     */
  /* ---------------------------------------------------------------------- */

  /*
    m 2^e, with 2^63 <= m < 2^64, or 0 (m = 0). Every operation rounds up, so
    that a magnitude is always at least the value it stands for. e >= inf_exp
    is infinity, which absorbs the sums.
  */
  struct Mag { u64 m; int e; };

  static const int inf_exp = 1 << 28;

  inline Mag mag_zero() { Mag r; r.m = 0; r.e = 0; return r; }
  inline Mag mag_inf() { Mag r; r.m = static_cast<u64>(1) << 63; r.e = inf_exp; return r; }
  inline Mag mag_pow2(int k) { Mag r; r.m = static_cast<u64>(1) << 63; r.e = k - 63; return r; }
  inline bool mag_is_inf(const Mag& a) { return a.m != 0 && a.e >= inf_exp; }

  // m 2^e for m <= 2^64 (m = 2^64 being the carry of a rounding up), normalized
  inline Mag mag_make(u64 m, bool carry, int e)
  {
    Mag r;
    if (carry) { r.m = static_cast<u64>(1) << 63; r.e = e + 1; }
    else { r.m = m; r.e = e; }
    return r;
  }

  inline Mag mag_add(Mag a, Mag b)
  {
    if (a.m == 0) { return b; }
    if (b.m == 0) { return a; }
    if (a.e < b.e) { const Mag t = a; a = b; b = t; }
    if (mag_is_inf(a)) { return a; }
    const int d = a.e - b.e;
    u64 bm;
    if (d == 0) { bm = b.m; }
    else if (d >= 64) { bm = 1; }
    else { bm = (b.m >> d) + (((b.m & ((static_cast<u64>(1) << d) - 1)) != 0) ? 1u : 0u); }
    const u64 s = a.m + bm;
    if (s < a.m) {
      // 2^64 + s, halved and rounded up
      const u64 half = (s >> 1) + (s & 1);
      Mag r;
      r.m = (static_cast<u64>(1) << 63) + half;
      r.e = a.e + 1;
      if (r.m < half) { // 2^64 again
        r.m = static_cast<u64>(1) << 63;
        r.e += 1;
      }
      return r;
    }
    return mag_make(s, false, a.e);
  }

  inline Mag mag_mul(const Mag& a, const Mag& b)
  {
    if (a.m == 0 || b.m == 0) { return mag_zero(); }
    if (mag_is_inf(a) || mag_is_inf(b)) { return mag_inf(); }
    u64 hi, lo;
    mul64(a.m, b.m, hi, lo);
    int e = a.e + b.e + 64;
    u64 m;
    bool inexact;
    if ((hi >> 63) == 0) {
      m = (hi << 1) | (lo >> 63);
      inexact = (lo << 1) != 0;
      e -= 1;
    } else {
      m = hi;
      inexact = lo != 0;
    }
    bool carry = false;
    if (inexact) {
      ++m;
      carry = (m == 0);
    }
    return mag_make(m, carry, e);
  }

  // Whether a <= b
  inline bool mag_le(const Mag& a, const Mag& b)
  {
    if (a.m == 0) { return true; }
    if (b.m == 0) { return false; }
    return a.e != b.e ? a.e < b.e : a.m <= b.m;
  }

  // a 2^k
  inline Mag mag_scale(const Mag& a, int k)
  {
    if (a.m == 0 || mag_is_inf(a)) { return a; }
    Mag r = a;
    r.e += k;
    return r;
  }

  /* ---------------------------------------------------------------------- */
  /* Balls                                                                  */
  /* ---------------------------------------------------------------------- */

  /*
    The value (-1)^neg M 2^(e-128), M = m of 128 bits with its top bit set
    (so that 2^(e-1) <= |value| < 2^e), or 0 (M = 0), within r of the real
    number it stands for.
  */
  struct Num
  {
    bool neg;
    U128 m;
    int e;
    Mag r;
  };

  inline bool num_is_zero(const Num& a) { return is_zero(a.m); }

  inline Num num_zero(const Mag& r)
  {
    Num z;
    z.neg = false; z.m = mk(0, 0); z.e = 0; z.r = r;
    return z;
  }

  // The number (-1)^neg v 2^p for an integer v, exact
  inline Num num_of_u128(bool neg, const U128& v, int p)
  {
    if (is_zero(v)) { return num_zero(mag_zero()); }
    const int bl = bitlen(v);
    Num r;
    r.neg = neg;
    r.m = shl(v, 128 - bl);
    r.e = bl + p; // v 2^p = (v 2^(128-bl)) 2^(bl+p-128)
    r.r = mag_zero();
    return r;
  }

  inline Num num_one() { return num_of_u128(false, mk(0, 1), 0); }

  // A finite double, exactly
  inline Num num_of_double(double d)
  {
    u64 bits;
    std::memcpy(&bits, &d, sizeof bits);
    const bool neg = (bits >> 63) != 0;
    const int field = static_cast<int>((bits >> 52) & 0x7ff);
    u64 m = bits & ((static_cast<u64>(1) << 52) - 1);
    int ex = -1074;
    if (field != 0) {
      m |= static_cast<u64>(1) << 52;
      ex = field - 1075;
    }
    return num_of_u128(neg, mk(0, m), ex);
  }

  inline Num num_neg(Num a) { a.neg = !a.neg; return a; }

  // a 2^k, exactly
  inline Num num_scale(Num a, int k)
  {
    if (num_is_zero(a)) { a.r = mag_scale(a.r, k); return a; }
    a.e += k;
    a.r = mag_scale(a.r, k);
    return a;
  }

  // An upper bound of |a|, which the error of a product with it uses
  inline Mag num_mag(const Num& a)
  {
    if (num_is_zero(a)) { return mag_zero(); }
    // |a| < (m.h + 1) 2^(e-64)
    return mag_make(a.m.h + 1, a.m.h + 1 == 0, a.e - 64);
  }

  inline Num num_add(const Num& a0, const Num& b0)
  {
    Mag rad = mag_add(a0.r, b0.r);
    if (num_is_zero(a0)) { Num z = b0; z.r = rad; return z; }
    if (num_is_zero(b0)) { Num z = a0; z.r = rad; return z; }
    const Num* pa = &a0;
    const Num* pb = &b0;
    if (pa->e < pb->e) { pa = &b0; pb = &a0; }
    const int d = pa->e - pb->e;
    // On the grid of 2^(pa->e - 255): A = a.m 2^127 and B = b.m 2^127 / 2^d
    const U256 A = shl(u256_of(pa->m), 127);
    bool lost = false;
    const U256 B = shr(shl(u256_of(pb->m), 127), d, lost);
    U256 S;
    bool neg;
    if (pa->neg == pb->neg) {
      S = add(A, B);
      neg = pa->neg;
    } else if (cmp(A, B) >= 0) {
      S = sub(A, B);
      neg = pa->neg;
    } else {
      S = sub(B, A);
      neg = pb->neg;
    }
    if (lost) { rad = mag_add(rad, mag_pow2(pa->e - 255)); }
    const int bl = bitlen(S);
    if (bl == 0) { return num_zero(rad); }
    Num r;
    r.neg = neg;
    r.e = pa->e - 255 + bl;
    if (bl > 128) {
      bool lost2 = false;
      const U256 t = shr(S, bl - 128, lost2);
      r.m = mk(t.w[1], t.w[0]);
      if (lost2) { rad = mag_add(rad, mag_pow2(r.e - 128)); }
    } else {
      const U256 t = shl(S, 128 - bl);
      r.m = mk(t.w[1], t.w[0]);
    }
    r.r = rad;
    return r;
  }

  inline Num num_sub(const Num& a, const Num& b) { return num_add(a, num_neg(b)); }

  inline Num num_mul(const Num& a, const Num& b)
  {
    const Mag rad = mag_add(mag_add(mag_mul(num_mag(a), b.r), mag_mul(num_mag(b), a.r)),
                            mag_mul(a.r, b.r));
    if (num_is_zero(a) || num_is_zero(b)) { return num_zero(rad); }
    U128 hi, lo;
    mul_full(a.m, b.m, hi, lo); // in [2^254, 2^256)
    Num r;
    r.neg = a.neg != b.neg;
    bool inexact;
    if ((hi.h >> 63) != 0) {
      r.m = hi;
      r.e = a.e + b.e;
      inexact = !is_zero(lo);
    } else {
      r.m = mk((hi.h << 1) | (hi.l >> 63), (hi.l << 1) | (lo.h >> 63));
      r.e = a.e + b.e - 1;
      inexact = (lo.h << 1) != 0 || lo.l != 0;
    }
    r.r = inexact ? mag_add(rad, mag_pow2(r.e - 128)) : rad;
    return r;
  }

  /* ---------------------------------------------------------------------- */
  /* exp(r) - 1 for |r| < 1/2                                               */
  /* ---------------------------------------------------------------------- */

  /*
    The fixed-point values 1/j! on 127 fractional bits, for j = 1 to 27,
    rounded down: G = sum of r^j / (j+1)! for j = 0 to 26 is (exp(r) - 1) / r.
    Horner's scheme with the 26 products rounded down: each of the 27
    constants and 26 products is off by less than one unit (2^-127), and the
    error of a step is that of the step after it times |r| < 1/2 plus two
    units, so that G is off by less than 4 units; the tail of the series,
    below |r|^27 / 28!, is below 2^-125 (2^-133 for |r| <= 0.4), and the
    truncation of |r| to 128 bits moves G by less than one unit: 2^-123
    holds them all.
  */
  static const int nb_terms = 26;

  // The constant 1 / (j + 1)! on 127 fractional bits, rounded down, for 0 <= j <= nb_terms
  inline U128 inv_fact(int j)
  {
    static const u64 C[nb_terms + 1][2] = {
      { 0x8000000000000000ULL, 0x0000000000000000ULL },
      { 0x4000000000000000ULL, 0x0000000000000000ULL },
      { 0x1555555555555555ULL, 0x5555555555555555ULL },
      { 0x0555555555555555ULL, 0x5555555555555555ULL },
      { 0x0111111111111111ULL, 0x1111111111111111ULL },
      { 0x002d82d82d82d82dULL, 0x82d82d82d82d82d8ULL },
      { 0x0006806806806806ULL, 0x8068068068068068ULL },
      { 0x0000d00d00d00d00ULL, 0xd00d00d00d00d00dULL },
      { 0x0000171de3a556c7ULL, 0x338faac1c88e5001ULL },
      { 0x0000024fc9f6ef13ULL, 0xeb8e5de02da7d4ccULL },
      { 0x00000035cc8acfeaULL, 0x89c71fce8fc9706fULL },
      { 0x000000047bb63bfeULL, 0x3625ed5136a61eb3ULL },
      { 0x000000005849184eULL, 0xa1b425f28e0cc748ULL },
      { 0x00000000064e5d2aULL, 0x301f27482eb7c517ULL },
      { 0x00000000006b9fcfULL, 0x9ccee07c476195acULL },
      { 0x000000000006b9fcULL, 0xf9ccee07c476195aULL },
      { 0x000000000000654bULL, 0x1dc0c2b529ac9814ULL },
      { 0x00000000000005a0ULL, 0x9e18ee5f65deec01ULL },
      { 0x000000000000004bULL, 0xd26d1a05055c9328ULL },
      { 0x0000000000000003ULL, 0xca8574804044a0f5ULL },
      { 0x0000000000000000ULL, 0x2e371dedb9eae317ULL },
      { 0x0000000000000000ULL, 0x0219c72db6ff0a52ULL },
      { 0x0000000000000000ULL, 0x001761b413163819ULL },
      { 0x0000000000000000ULL, 0x0000f96780cb97abULL },
      { 0x0000000000000000ULL, 0x000009f9e66e8b2fULL },
      { 0x0000000000000000ULL, 0x000000623a17f1a9ULL },
      { 0x0000000000000000ULL, 0x00000003a356385bULL }
    };
    return mk(C[j][0], C[j][1]);
  }

  inline U128 g_fixed(const U128& R, bool neg)
  {
    U128 P = inv_fact(nb_terms);
    for (int j = nb_terms - 1; j >= 0; --j) {
      const U128 T = mul_hi(R, P);
      const U128 Cj = inv_fact(j);
      u64 c;
      P = neg ? sub(Cj, T, c) : add(Cj, T, c);
    }
    return P;
  }

  // A ball that holds every real number, which a precondition that fails gives
  inline Num num_unknown()
  {
    Num z = num_zero(mag_inf());
    return z;
  }

  /*
    exp(r) - 1 = r (exp(r) - 1) / r, for |r| < 1/2. The error of r moves the
    value by exp(r) times it, 1.65 at most (1.42 for the |r| <= 0.347 of
    reduce()), and the product of the ball of r by that of G accounts for
    G times it, 1.30 at most: half of it is added.
  */
  inline Num expm1_small(const Num& r)
  {
    if (num_is_zero(r)) {
      // 0 +- rho: exp(t) - 1 is within rho + rho^2 of 0 for |t| <= rho <= 1
      if (mag_is_inf(r.r) || (r.r.m != 0 && r.r.e + 64 > -10)) { return num_unknown(); }
      Num z = r;
      z.r = mag_add(r.r, mag_mul(r.r, r.r));
      return z;
    }
    // |r| >= 1/2, or a ball so wide that exp(r) - 1 moves by more than its error
    if (r.e > -1 || mag_is_inf(r.r) || (r.r.m != 0 && r.r.e + 64 > -10)) { return num_unknown(); }
    // R = floor(|r| 2^128) = m >> (-e)
    U128 R = mk(0, 0);
    if (-r.e < 128) { R = shr(r.m, -r.e); }
    const U128 G = g_fixed(R, r.neg);
    // G / 2^127
    Num g = num_of_u128(false, G, -127);
    g.r = mag_pow2(-123);
    Num E = num_mul(r, g);
    E.r = mag_add(E.r, mag_scale(r.r, -1));
    return E;
  }

  /*
    t = k log(2) + r, with |r| <= 0.347 (0.4 at most): k is the integer nearest
    to t / log(2) up to 10^-9. log(2) = H + L + e, with H = Mh 2^-116 for a
    116-bit integer Mh, so that k H is exact for |k| < 2^12, L with 128 bits
    and |e| < 2^-244: r = (t - k H) - k L is within |k| 2^-244 of t - k log(2)
    (the ball of L counts it), and t - k H is exact for the t of the 2^-128
    grid of a significand with |t| < 2^11.
  */
  inline bool reduce(const Num& t, int& k, Num& r)
  {
    if (num_is_zero(t) || t.e > 11) {
      k = 0;
      r = t;
      return num_is_zero(t);
    }
    if (t.e < -1) { // |t| < 1/4: no multiple of log(2) is nearer, and no operation on doubles
      k = 0;
      r = t;
      return true;
    }
    const double td = std::ldexp(static_cast<double>(t.m.h), t.e - 64) * (t.neg ? -1.0 : 1.0);
    const double kd = td * 1.4426950408889634; // 1 / log(2)
    k = static_cast<int>(kd >= 0.0 ? kd + 0.5 : kd - 0.5);
    if (k == 0) {
      r = t;
      return true;
    }
    const bool kneg = k < 0;
    const u64 ak = static_cast<u64>(kneg ? -k : k);
    // k H = k Mh 2^-116: Mh = 0x000b17217f7d1cf7 9abc9e3b39803f2f
    u64 h1, l1;
    mul64(ak, 0x9abc9e3b39803f2fULL, h1, l1);
    const U128 prod = mk(0x000b17217f7d1cf7ULL * ak + h1, l1);
    const Num KH = num_of_u128(kneg, prod, -116);
    // L = 0x6af40f343267298b 62d8a0d175b8baaf 2^-244 (116 + 128)
    Num Lo = num_of_u128(false, mk(0x6af40f343267298bULL, 0x62d8a0d175b8baafULL), -244);
    Lo.r = mag_pow2(-244); // the 128 bits are the integer part of L 2^244
    const Num KL = num_mul(num_of_u128(kneg, mk(0, ak), 0), Lo);
    r = num_sub(num_sub(t, KH), KL);
    return true;
  }

  // exp(t) - 1 = 2^k exp(r) - 1 for a ball t with |t| < 2^11
  inline Num expm1_ball(const Num& t)
  {
    if (num_is_zero(t)) { return expm1_small(t); }
    if (t.e > 11) { return num_unknown(); }
    int k;
    Num r;
    reduce(t, k, r);
    const Num E = expm1_small(r);
    if (k == 0) { return E; }
    const Num U = num_add(num_one(), E);
    return num_sub(num_scale(U, k), num_one());
  }

  /* ---------------------------------------------------------------------- */
  /* log(1 + x)                                                             */
  /* ---------------------------------------------------------------------- */

  /*
    log(1 + x) for a finite x > -1 other than 0, from y0, a double next to it
    (within 2^-5 relatively and 2^-5 absolutely at least, the correctly
    rounded log1p() of CORE-MATH being within 2^-52 relatively).
    log(1 + x) = y0 + log(1 + d) for d = (1 + x) exp(-y0) - 1, whose terms
    d - d^2/2 + d^3/3 (the next ones are below d^4) are computed on a ball.
    For |y0| < 0.34 (k = 0 below) 1 + x is not formed, as it would hide a x
    far below 2^-128: d = x + E + x E, with E = exp(-y0) - 1, is computed
    with relative errors, as the cancellation between x and E leaves the 2^-52
    of the error of y0 and no more. Elsewhere |x| is not small, and
    (1 + x) 2^-k U is about 1.
  */
  inline Num log1p_ball(double x, double y0)
  {
    const Num X = num_of_double(x);
    const Num Y0 = num_of_double(y0);
    int k;
    Num r;
    if (!reduce(num_neg(Y0), k, r)) { return num_unknown(); }
    const Num E = expm1_small(r);
    Num d;
    if (k == 0) {
      d = num_add(num_add(X, E), num_mul(X, E));
    } else {
      const Num U = num_add(num_one(), E);
      d = num_sub(num_scale(num_mul(num_add(num_one(), X), U), k), num_one());
    }
    // |d| < 2^-8 is far above what y0 gives; the series needs |d| < 1/2
    const Mag bound = mag_add(num_mag(d), d.r);
    if (bound.m != 0 && bound.e + 64 > -8) { return num_unknown(); }
    const Num d2 = num_mul(d, d);
    const Num d3 = num_mul(d2, d);
    // 1/3 = 0xaaaa...aaaa 2^-129, rounded down
    Num third = num_of_u128(false, mk(0xaaaaaaaaaaaaaaaaULL, 0xaaaaaaaaaaaaaaaaULL), -129);
    third.r = mag_pow2(-129);
    const Num corr = num_add(num_sub(d, num_scale(d2, -1)), num_mul(d3, third));
    Num L = num_add(Y0, corr);
    const Mag b2 = mag_mul(bound, bound);
    L.r = mag_add(L.r, mag_mul(b2, b2)); // the terms from d^4 on, below d^4
    return L;
  }

  /* ---------------------------------------------------------------------- */
  /* From a ball to doubles                                                 */
  /* ---------------------------------------------------------------------- */

  // +0 for a zero, whatever the sign asked
  inline double dbl_of_bits(bool neg, u64 bits)
  {
    if (bits != 0) { bits |= static_cast<u64>(neg ? 1 : 0) << 63; }
    double d;
    std::memcpy(&d, &bits, sizeof d);
    return d;
  }

  inline double dbl_max() { return std::numeric_limits<double>::max(); }
  inline double dbl_inf() { return std::numeric_limits<double>::infinity(); }

  // The least double above d, for a finite d
  inline double next_up(double d)
  {
    u64 bits;
    std::memcpy(&bits, &d, sizeof bits);
    if ((bits >> 63) == 0) {
      bits += 1;
    } else if ((bits << 1) == 0) { // -0
      bits = 1;
    } else {
      bits -= 1;
    }
    std::memcpy(&d, &bits, sizeof d);
    return (d == 0.0) ? 0.0 : d; // +0 for the -0 that the double below 2^-1074 is
  }

  inline double next_down(double d)
  {
    const double below = -next_up(-d);
    return (below == 0.0) ? 0.0 : below;
  }

  /*
    The double nearest to (-1)^neg mag 2^q for an integer mag of up to 256
    bits, in the direction of larger magnitudes (up) or smaller ones. exact
    is set where the number is a double. An overflow gives the infinity for up
    and the largest double otherwise; a zero is +0.
  */
  inline double dbl_round(bool neg, const U256& mag, int q, bool up, bool& exact)
  {
    exact = true;
    const int bl = bitlen(mag);
    if (bl == 0) { return 0.0; }
    const int E = bl - 1 + q; // 2^E <= value < 2^(E+1)
    if (E > 1023) {
      exact = false;
      return up ? (neg ? -dbl_inf() : dbl_inf()) : (neg ? -dbl_max() : dbl_max());
    }
    u64 frac;
    bool inexact = false;
    if (E >= -1022) {
      // 53 bits: mag >> (bl - 53)
      int Eb = E;
      if (bl <= 53) {
        frac = shl(mag, 53 - bl).w[0];
      } else {
        bool lost = false;
        frac = shr(mag, bl - 53, lost).w[0];
        inexact = lost;
      }
      if (up && inexact) {
        ++frac;
        if (frac == (static_cast<u64>(1) << 53)) {
          frac >>= 1;
          ++Eb;
        }
      }
      exact = !inexact;
      if (Eb > 1023) { return neg ? -dbl_inf() : dbl_inf(); }
      return dbl_of_bits(neg, (static_cast<u64>(Eb + 1023) << 52)
                              | (frac & ((static_cast<u64>(1) << 52) - 1)));
    }
    // A subnormal number: mag 2^q in units of 2^-1074
    const int s = -1074 - q; // the bits to drop
    u64 units;
    if (s <= 0) {
      units = shl(mag, -s).w[0]; // below 2^52: exact
    } else {
      bool lost = false;
      units = shr(mag, s, lost).w[0];
      inexact = lost;
      if (up && inexact) { ++units; }
    }
    exact = !inexact;
    return dbl_of_bits(neg, units);
  }

  // The value c +- r, c = (-1)^cneg M 2^q, r = R 2^q with R < 2^127 <= M, as a sign and a magnitude in units of 2^q
  inline void endpoint(bool cneg, const U128& M, const U128& R, bool plus, bool& neg, U256& mag)
  {
    neg = cneg;
    if (cneg != plus) { // c + r for c >= 0, c - r for c < 0: the magnitudes add
      u64 carry;
      const U128 s = add(M, R, carry);
      mag.w[0] = s.l; mag.w[1] = s.h; mag.w[2] = carry; mag.w[3] = 0;
    } else { // c - r for c >= 0, c + r for c < 0: they subtract, and the sign of c stays
      u64 borrow;
      mag = u256_of(sub(M, R, borrow));
    }
  }

  /*
    The doubles lo <= f <= hi around a ball: the greatest double at most the
    lower end c - r and the least double at least the upper end, r rounded up
    to the grid of c. lo_exact is set where c - r is a double, hi_exact where
    c + r is. A ball unknown, or wider than half its center (R < 2^127 on the
    grid of the center, whose significand is 2^127 or more), gives (-oo, +oo):
    the balls of compoundm1 are 2^-111 wide, and never come to it. A center 0
    gives -r and r, rounded outward.
  */
  inline void round_ball(const Num& F, double& lo, double& hi, bool& lo_exact, bool& hi_exact)
  {
    lo_exact = hi_exact = false;
    lo = -dbl_inf();
    hi = dbl_inf();
    if (mag_is_inf(F.r)) { return; }
    if (num_is_zero(F)) {
      // 0 +- r: the double above r, and its opposite
      lo_exact = hi_exact = (F.r.m == 0);
      if (F.r.m == 0) {
        lo = hi = 0.0;
      } else {
        U256 mag;
        mag.w[0] = F.r.m; mag.w[1] = mag.w[2] = mag.w[3] = 0;
        bool ignored;
        hi = dbl_round(false, mag, F.r.e, true, ignored);
        lo = -hi;
      }
      return;
    }
    const int q = F.e - 128;
    // R = ceil(r 2^-q) = ceil(rm 2^(re - q))
    const Mag& r = F.r;
    U128 R = mk(0, 0);
    if (r.m != 0) {
      const int sh = r.e - q;
      if (sh >= 0) {
        if (sh > 63) { return; } // 2^127 or more: too wide
        R = shl(mk(0, r.m), sh);
      } else if (-sh >= 64) {
        R = mk(0, 1);
      } else {
        R = mk(0, (r.m >> -sh) + (((r.m & ((static_cast<u64>(1) << -sh) - 1)) != 0) ? 1u : 0u));
      }
    }
    bool neg;
    U256 mag;
    endpoint(F.neg, F.m, R, false, neg, mag);
    lo = dbl_round(neg, mag, q, neg, lo_exact);
    endpoint(F.neg, F.m, R, true, neg, mag);
    hi = dbl_round(neg, mag, q, !neg, hi_exact);
  }

  /* ---------------------------------------------------------------------- */
  /* Exact values                                                           */
  /* ---------------------------------------------------------------------- */

  // odd 2^sh, for an odd integer odd
  struct Dyadic { U128 odd; int sh; };

  // A finite double v != 0 as (-1)^neg m 2^e, m an odd integer
  inline void split_double(double v, bool& neg, u64& m, int& e)
  {
    u64 bits;
    std::memcpy(&bits, &v, sizeof bits);
    neg = (bits >> 63) != 0;
    const int field = static_cast<int>((bits >> 52) & 0x7ff);
    m = bits & ((static_cast<u64>(1) << 52) - 1);
    e = -1074;
    if (field != 0) {
      m |= static_cast<u64>(1) << 52;
      e = field - 1075;
    }
    const int z = ctz64(m);
    m >>= z;
    e += z;
  }

  // The integer k with 2^k <= |v| < 2^(k+1), for a finite v != 0
  inline int expo(double v)
  {
    bool neg;
    u64 m;
    int e;
    split_double(v, neg, m, e);
    return e + 63 - clz64(m);
  }

  // 1 + v exactly as a dyadic number, for a finite v > -1, if it takes 127 bits
  inline bool one_plus(double v, Dyadic& out)
  {
    if (v == 0.0) {
      out.odd = mk(0, 1);
      out.sh = 0;
      return true;
    }
    bool neg;
    u64 m;
    int e;
    split_double(v, neg, m, e);
    const int lowexp = (e < 0) ? e : 0;
    const int one_sh = -lowexp, v_sh = e - lowexp;
    const int mbits = 64 - clz64(m);
    if (one_sh > 126 || v_sh + mbits > 126) { return false; }
    const U128 one = shl(mk(0, 1), one_sh);
    const U128 vv = shl(mk(0, m), v_sh);
    u64 c;
    U128 S;
    if (neg) {
      if (!lt(vv, one)) { return false; }
      S = sub(one, vv, c);
    } else {
      S = add(one, vv, c);
    }
    const int tz = (S.l != 0) ? ctz64(S.l) : 64 + ctz64(S.h);
    out.odd = shr(S, tz);
    out.sh = lowexp + tz;
    return true;
  }

  // base^p for p >= 1, false if it takes more than 128 bits
  inline bool pow_u128(const U128& base, unsigned p, U128& out)
  {
    U128 result = mk(0, 1), b = base;
    bool first = true;
    for (;;) {
      if ((p & 1u) != 0) {
        U128 hi, lo;
        mul_full(result, b, hi, lo);
        if (!is_zero(hi)) { return false; }
        result = lo;
        first = false;
      }
      p >>= 1;
      if (p == 0) { break; }
      U128 hi, lo;
      mul_full(b, b, hi, lo);
      if (!is_zero(hi)) { return false; }
      b = lo;
    }
    out = result;
    return !first;
  }

  // d as a dyadic number, for a finite d > 0
  inline Dyadic dyadic_of(double d)
  {
    bool neg;
    u64 m;
    int e;
    split_double(d, neg, m, e);
    Dyadic w;
    w.odd = mk(0, m);
    w.sh = e;
    return w;
  }

  // The integer square root of a: r with r^2 <= a < (r + 1)^2
  inline u64 isqrt(const U128& a)
  {
    u64 r = 0;
    for (int bit = 63; bit >= 0; --bit) {
      const u64 t = r | (static_cast<u64>(1) << bit);
      u64 hi, lo;
      mul64(t, t, hi, lo);
      if (hi < a.h || (hi == a.h && lo <= a.l)) { r = t; }
    }
    return r;
  }

  /*
    Whether (1 + x)^y is w exactly, for finite x > -1 other than 0, y other
    than 0, and a dyadic number w (1 + d for a double d, or a double). s =
    1 + x = A 2^a and w = B 2^b, with A and B odd, and y = +-P / 2^k with P
    odd (k >= 0) or an integer.
    - A = 1: s^y = 2^(a y), a power of two, which is w for B = 1 and
      a y = b (an integer: a power of two with a y not an integer is
      irrational).
    - A >= 3: s^y is a power of two or irrational for A = 1 only, and
      s^y = w has an odd part for B >= 3, the odd part of s^y being A^y,
      which is rational for y < 0 never (its denominator has the odd part
      A^|y|), and for y = P / 2^k when A^P = B^(2^k), that is when A is the
      2^k-th power of an integer R, and a a multiple of 2^k, with
      R^P = B and (a / 2^k) P = b. R, found by k square roots, and R^P are
      computed exactly, or the answer is false when R^P takes more than 128
      bits: false means that the equality is not proved, which the bounds do
      not need.
  */
  inline bool power_equals(double x, double y, const Dyadic& w)
  {
    Dyadic s;
    if (!one_plus(x, s)) { return false; }
    bool yneg;
    u64 M;
    int ey;
    split_double(y, yneg, M, ey); // |y| = M 2^ey, M odd
    const bool s_is_pow2 = s.odd.h == 0 && s.odd.l == 1;
    const bool w_is_pow2 = w.odd.h == 0 && w.odd.l == 1;
    if (s_is_pow2) {
      if (!w_is_pow2) { return false; }
      // a y = b
      const int a = s.sh;
      const u64 aa = static_cast<u64>(a < 0 ? -a : a);
      const u64 prod = aa * M; // below 2^11 2^53
      long long val;
      if (ey >= 0) {
        if (ey > 11 || prod > (static_cast<u64>(2048) >> ey)) { return false; }
        val = static_cast<long long>(prod << ey);
      } else {
        const int sft = -ey;
        if (sft >= 64 || (prod & ((static_cast<u64>(1) << sft) - 1)) != 0) { return false; }
        val = static_cast<long long>(prod >> sft);
      }
      if ((a < 0) != yneg) { val = -val; }
      return val == w.sh;
    }
    if (w_is_pow2 || yneg) { return false; }
    u64 P;
    int k;
    if (ey >= 0) {
      if (ey > 7 || M > (static_cast<u64>(127) >> ey)) { return false; }
      P = M << ey;
      k = 0;
    } else {
      if (M > 127 || -ey > 6) { return false; }
      P = M;
      k = -ey;
    }
    // s = (R 2^(a / 2^k))^(2^k) for an odd integer R
    U128 R = s.odd;
    for (int i = 0; i < k; ++i) {
      const u64 r = isqrt(R);
      u64 hi, lo;
      mul64(r, r, hi, lo);
      if (!(hi == R.h && lo == R.l)) { return false; } // no integer square root
      R = mk(0, r);
    }
    if (s.sh % (1 << k) != 0) { return false; }
    U128 RP;
    if (!pow_u128(R, static_cast<unsigned>(P), RP)) { return false; }
    return eq(RP, w.odd)
        && static_cast<long long>(s.sh / (1 << k)) * static_cast<long long>(P) == static_cast<long long>(w.sh);
  }

  // Whether x y is d exactly, for finite doubles x, y and d, none of them 0
  inline bool product_equals(double x, double y, double d)
  {
    bool xneg, yneg, dneg;
    u64 mx, my, md;
    int ex, ey, ed;
    split_double(x, xneg, mx, ex);
    split_double(y, yneg, my, ey);
    split_double(d, dneg, md, ed);
    u64 hi, lo;
    mul64(mx, my, hi, lo); // odd, as mx and my are
    return hi == 0 && lo == md && ex + ey == ed && (xneg != yneg) == dneg;
  }

  /* ---------------------------------------------------------------------- */
  /* The reciprocal: y = -1                                                 */
  /* ---------------------------------------------------------------------- */

  /*
    (1 + x)^-1 - 1 = -x / (1 + x) for a finite x > -1 other than 0, whose
    bounds are the tightest ones without any ball: with 1 + x = S 2^s for an
    odd integer S of at most 127 bits (one_plus()), |f| = mx 2^ex / (S 2^s) is
    the quotient of two integers, which the schoolbook division gives with 56
    or 57 bits and a remainder; the doubles around the quotient follow from
    them, a remainder that is not 0 being a sticky bit under the 56 bits (the
    doubles are multiples of 8 units at least there, and none is between the
    quotient and the quotient plus the sticky bit). The balls cannot tell the
    doubles apart where 1/(1 + x) = 1 - x + x^2 - x^3 ... is within 2^-155 of a
    double without being one, as it is for the x = 2^e + 2^(2e-1) of
    -51 <= e <= -40. False where 1 + x takes more than 127 bits (a tiny x, or
    a huge one), which the ball handles.
  */
  inline bool reciprocal_m1(double x, double& lo, double& hi)
  {
    Dyadic S;
    if (!one_plus(x, S)) { return false; }
    bool xneg;
    u64 mx;
    int ex;
    split_double(x, xneg, mx, ex);
    // N = mx 2^k has bitlen(S.odd) + 57 bits, so that N / S.odd is in [2^56, 2^58)
    const int k = 57 + bitlen(S.odd) - bitlen(mk(0, mx));
    const U256 N = shl(u256_of(mk(0, mx)), k);
    bool lost = false;
    const U256 Nh = shr(N, 58, lost); // N / 2^58 < S.odd: the remainder to start with
    U128 R = mk(Nh.w[1], Nh.w[0]);
    u64 quo = 0;
    for (int i = 57; i >= 0; --i) {
      const u64 bit = (N.w[0] >> i) & 1u;
      R = mk((R.h << 1) | (R.l >> 63), (R.l << 1) | bit); // below 2 S.odd < 2^128
      quo <<= 1;
      if (!lt(R, S.odd)) {
        u64 borrow;
        R = sub(R, S.odd, borrow);
        quo |= 1u;
      }
    }
    // |f| is between quo and quo + 1 units of 2^(ex - S.sh - k): 2 quo + sticky stands for it, in half units
    U256 mag;
    mag.w[0] = (quo << 1) | (is_zero(R) ? 0u : 1u);
    mag.w[1] = mag.w[2] = mag.w[3] = 0;
    const int q = ex - S.sh - k - 1;
    const bool fneg = !xneg; // -x / (1 + x) has the sign of -x
    bool e1, e2;
    lo = dbl_round(fneg, mag, q, fneg, e1);
    hi = dbl_round(fneg, mag, q, !fneg, e2);
    return true;
  }

  /* ---------------------------------------------------------------------- */
  /* The fast phase: 64-bit fixed point                                     */
  /* ---------------------------------------------------------------------- */

  /*
    Most of the time f = (1 + x)^y - 1 is far from a double, and 60 bits of it
    show which ones are next to it. The fast phase computes f with 64-bit
    integers and the bound of its relative error that the analysis below
    proves, and returns the doubles around f if none is between its bounds. The
    other cases go on to the balls, which prove the values that are doubles
    and tell the doubles apart that are closer than 2^-58: 5 % of the pairs
    for |y log(1 + x)| below 0.35, 8 % below 1, 10 % below 2, 17 % below 4
    and 29 % below 8 (measured on random pairs). It returns false for the ones
    out of its domain too, |log(1 + x)| > 8 (x < -0.99966 or x > 2980),
    |y log(1 + x)| >= 8 and the values below 2^-1022. It takes 80 to 120
    nanoseconds, against 600 to 900 for the balls (i7-1185G7, GCC 15).

    The unit is u = 2^-63. G(r) = (exp(r) - 1) / r = sum of r^j / (j+1)!, in
    Q1.63 (the integer G 2^63), for |r| <= 1/2 and R = floor(|r| 2^64):
    g64(R, neg) is within 5 u of G(R / 2^64) (with the sign neg for r). It is
    Horner's scheme with the 17 constants c_j = floor(2^63 / (j+1)!), the
    products rounded down: with e_j the error of the step j, |e_j| <= 2 + |r|
    |e_(j+1)| for the two roundings, e_16 is below 1 + 2^63 / 18! (the tail of
    the series from j = 17 on) = 1443, which |r|^16 weighs in e_0: |e_0| <= 4
    + 2^-16 1443 < 4.03. Then R moves G by G'(r) 2^-64 <= 0.35 u (G' <= 0.7).
    5 u holds them. exp(r) - 1 = r G(r) is computed as the product, rounded
    down, of R by G: V = 1 + r G(r) is off by 5 |r| + 1 + 0.7 <= 3.5 u for
    |r| <= log(2) / 2 = 0.3466.

    L = log(1 + x) is computed from y0 = log1p(x), a double within one unit in
    the last place of L (|d| <= 2^-51 |y0| for d below), as L = y0 + log(1 + d)
    = y0 + d - d^2/2 ... for d = (1 + x) exp(-y0) - 1, whose terms of d^2 on
    weigh 2^-102 |y0|^2 at most. Two ways to d, for |y0| <= 1/2 (L is within
    base = 14 u of the value relatively) and 1/2 < |y0| <= 8 (base = 20 u):
      A. exp(-y0) - 1 = -y0 G(-y0) gives d = x - y0 (G(-y0) (1 + x) - 1), so
         that L = x - y0 P / 2^63 for P = floor(G W / 2^63) - 2^63, G =
         g64(R of -y0), W = floor((1 + x) 2^63) (0.6065 to 1.65 2^63). P is
         off by 5 u 1.65 (G times W, at most when G is 0.79) or 1 u 1.3 (W
         times G) and 1 u for the product cut: 10.1 u at most, so L, which the
         integers compute exactly from x, y0 and P, is within 10.1 u |y0| +
         2^-104 |y0|^2 of the value: 11 u relatively to |L|, 13 with the 64
         high bits of L used below (1 u) and the product t = y L cut (1 u),
         14 with a margin.
      B. |y0| = kk log(2) + s with |s| <= log(2) / 2 (log(2) to 2^-120, an
         error below 2^-116), exp(-y0) = 2^(-+kk) V for V = exp(r), r = -+s,
         off by 3.5 u: d = (1 + x) 2^(-+kk) V - 1, with 1 + x cut to the 64 bits
         of wm 2^we (1 u relatively, exact if it has 64 bits or less) and the
         product cut to 2^-63 (1 u): d is off by 1 + 1.414 3.5 + 1 + 1 (the
         log(2) and the terms of d^2) <= 8 u at most, |L| >= 1/2: 16 u
         relatively, 18 with the cut and the product (2 u) and 20 with a
         margin.
    Then, with t = y L of |t| < 2^(te+64) <= 8, and phi(t) = t / (1 - exp(-t))
    <= 1 + max(t, 0), which multiplies the relative error of t in that of f
    (the derivative of exp(t) - 1 is exp(t)):
      - |t| < log(2) / 2: f = t G(t) with G(t) within 5 u, 5 / 0.845 = 6 u,
        and the product cut: 1.35 base + 7 u, with 1 more for a margin;
      - else k = round(t / log(2)), r = t - k log(2) and exp(t) - 1 = 2^k V -
        1 for V = exp(r) off by 3.5 u. For k >= 1, 2^k V - 1 >= 2^k 0.707 - 1
        (0.414, 1.83, ...) so the error is 2^k 3.5 / (2^k 0.707 - 1) <= 17 u
        (k = 1), then 7.6, 6...; for k <= -1 it is (3.5 u 2^k + 1 u) / (1 -
        1.414 2^k) <= 9.6 u (the shift of V cut). base (1 + |t|) + 20 holds
        them, and 2 u for the cut of the result.
    The enclosure is m +- (rho m / 2^63 + 2) units of the last place for the
    64-bit significand m of the result: the doubles around f are those next to
    it if none is in the enclosure (m has 11 more bits than a double).
  */
  static const int fast_terms = 16;

  // G(r) 2^63 for r = +- R / 2^64, R <= 2^63, within 5
  inline u64 g64(u64 R, bool neg)
  {
    u64 P = inv_fact(fast_terms).h;
    for (int j = fast_terms - 1; j >= 0; --j) {
      u64 hi, lo;
      mul64(R, P, hi, lo);
      const u64 c = inv_fact(j).h;
      P = neg ? c - hi : c + hi;
    }
    return P;
  }

  // A finite d != 0 as (-1)^neg m 2^e, with m of 64 bits, the top one set
  inline void norm64(double d, bool& neg, u64& m, int& e)
  {
    u64 bits;
    std::memcpy(&bits, &d, sizeof bits);
    neg = (bits >> 63) != 0;
    const int field = static_cast<int>((bits >> 52) & 0x7ff);
    m = bits & ((static_cast<u64>(1) << 52) - 1);
    e = -1074;
    if (field != 0) {
      m |= static_cast<u64>(1) << 52;
      e = field - 1075;
    }
    const int z = clz64(m);
    m <<= z;
    e -= z;
  }

  // The 64 high bits of the 128-bit hi 2^64 + lo != 0 in m, and the left shift z that took them
  inline void top64(u64 hi, u64 lo, u64& m, int& z)
  {
    if (hi == 0) {
      z = 64 + clz64(lo);
      m = lo << (z - 64);
    } else {
      z = clz64(hi);
      m = (z == 0) ? hi : ((hi << z) | (lo >> (64 - z)));
    }
  }

  // log(2) 2^120, rounded down
  inline U128 ln2_120() { return mk(0xb17217f7d1cf79ULL, 0xabc9e3b39803f2f6ULL); }

  /*
    For T = |v| 2^120 < 2^123: the integer k nearest to |v| / log(2) (at most
    12), and s = |v| - k log(2), of the sign sneg, with R = floor(|s| 2^64),
    below 2^63.
  */
  inline void fast_reduce(const U128& T, int& k, bool& sneg, u64& R)
  {
    const U128 M = ln2_120();
    const U128 T2 = shl(T, 1), M2 = shl(M, 1);
    U128 thr = M; // (2 k + 1) M, compared with 2 T
    k = 0;
    u64 c;
    while (!lt(T2, thr)) {
      ++k;
      thr = add(thr, M2, c);
    }
    u64 kh, kl;
    mul64(M.l, static_cast<u64>(k), kh, kl);
    const U128 kM = mk(M.h * static_cast<u64>(k) + kh, kl);
    U128 sabs;
    u64 bw;
    if (lt(T, kM)) {
      sabs = sub(kM, T, bw);
      sneg = true;
    } else {
      sabs = sub(T, kM, bw);
      sneg = false;
    }
    R = shr(sabs, 56).l; // |s| < M / 2 < 2^119
  }

  /*
    L = log(1 + x) as LM 2^g with the sign Lneg, within base units (14 or 20)
    relatively, for the double y0 = log1p(x) (norm64() of x and y0 are given).
  */
  inline bool fast_log_small(bool xneg, u64 xm, int xe, bool y0neg, u64 y0m, int y0e,
                             U128& LM, bool& Lneg, int& g)
  {
    const u64 one63 = static_cast<u64>(1) << 63;
    // R = floor(|y0| 2^64) = y0m 2^(y0e + 64), y0e + 64 <= 0 for |y0| <= 1/2
    const int sr = -(y0e + 64);
    if (sr < 0) { return false; }
    const u64 R = (sr >= 64) ? 0 : (y0m >> sr);
    if (R > one63) { return false; }
    const u64 G = g64(R, !y0neg); // r = -y0
    // W = floor((1 + x) 2^63), |x| 2^63 = xm 2^(xe + 63) with xe + 63 < 0 for |x| < 1
    const int sw = -(xe + 63);
    if (sw < 1) { return false; }
    u64 fl = 0, ce = 1;
    if (sw < 64) {
      fl = xm >> sw;
      ce = fl + (((xm & ((static_cast<u64>(1) << sw) - 1)) != 0) ? 1u : 0u);
    }
    const u64 W = xneg ? one63 - ce : one63 + fl;
    // Q = floor(G W / 2^63), P = Q - 2^63
    u64 qh, ql;
    mul64(G, W, qh, ql);
    if ((qh >> 63) != 0) { return false; }
    const u64 Q = (qh << 1) | (ql >> 63);
    const bool Pneg = Q < one63;
    const u64 Pmag = Pneg ? one63 - Q : Q - one63;
    // L = x - y0 P / 2^63 exactly, in units of 2^g, g = y0e - 63: y0 P / 2^63 = y0m Pmag 2^g
    g = y0e - 63;
    const int sx = xe - g;
    if (sx < 0 || sx > 64) { return false; }
    const U128 A = shl(mk(0, xm), sx);
    u64 bh, bl;
    mul64(y0m, Pmag, bh, bl);
    const U128 B = mk(bh, bl);
    const bool Bneg = (y0neg == Pneg); // -y0 P is negative for y0 P > 0
    u64 c;
    if (is_zero(B)) {
      LM = A;
      Lneg = xneg;
    } else if (xneg == Bneg) {
      LM = add(A, B, c);
      if (c != 0) { return false; }
      Lneg = xneg;
    } else if (!lt(A, B)) {
      LM = sub(A, B, c);
      Lneg = xneg;
    } else {
      LM = sub(B, A, c);
      Lneg = Bneg;
    }
    return true;
  }

  inline bool fast_log_large(bool xneg, u64 xm, int xe, bool y0neg, u64 y0m, int y0e,
                             U128& LM, bool& Lneg, int& g)
  {
    const u64 one63 = static_cast<u64>(1) << 63;
    // |y0| 2^120 = y0m 2^(y0e + 120), below 2^123 for |y0| <= 8
    if (y0e + 64 < 0 || y0e + 64 > 3) { return false; }
    int kk;
    bool sneg;
    u64 Rs;
    fast_reduce(shl(mk(0, y0m), y0e + 120), kk, sneg, Rs);
    if (kk < 1) { return false; }
    // exp(-y0) = 2^(-+kk) exp(r) with r = -+s: V = exp(r) 2^63
    const bool rneg = y0neg ? sneg : !sneg;
    const u64 Gs = g64(Rs, rneg);
    u64 eh, el;
    mul64(Rs, Gs, eh, el);
    const u64 Vn = rneg ? one63 - eh : one63 + eh;
    // 1 + x = U 2^xe exactly, U = 2^-xe +- xm, cut to its 64 high bits
    if (xe > -1 || xe < -126) { return false; }
    const U128 one = shl(mk(0, 1), -xe);
    u64 c;
    const U128 U = xneg ? sub(one, mk(0, xm), c) : add(one, mk(0, xm), c);
    if (c != 0 || is_zero(U)) { return false; }
    const int wb = bitlen(U);
    const u64 wm = (wb >= 64) ? shr(U, wb - 64).l : shl(U, 64 - wb).l;
    const int we = xe + wb - 64;
    // (1 + x) exp(-y0) = wm 2^we 2^(-+kk) Vn 2^-63, about 1: Zn = its integer part times 2^63
    const int sh = (y0neg ? -kk : kk) - we;
    if (sh < 60 || sh > 68) { return false; }
    u64 ph, pl;
    mul64(wm, Vn, ph, pl);
    const U128 Zs = shr(mk(ph, pl), sh);
    if (Zs.h != 0) { return false; }
    const bool dneg = Zs.l < one63;
    const u64 dmag = dneg ? one63 - Zs.l : Zs.l - one63;
    if (dmag >= (static_cast<u64>(1) << 20)) { return false; }
    // L = y0 + d, d = +-dmag 2^-63, exactly
    g = (y0e < -63) ? y0e : -63;
    const U128 A = shl(mk(0, y0m), y0e - g);
    const U128 D = shl(mk(0, dmag), -63 - g);
    if (y0neg == dneg) {
      LM = add(A, D, c);
      if (c != 0) { return false; }
      Lneg = y0neg;
    } else if (!lt(A, D)) {
      LM = sub(A, D, c);
      Lneg = y0neg;
    } else {
      LM = sub(D, A, c);
      Lneg = dneg;
    }
    return true;
  }

  // The bound rho of the relative error of f, in units of 2^-63, for |t| < 2^tube and base units for t
  inline u64 fast_rho(int k, int tube, u64 base)
  {
    if (k == 0) { return (base * 27 + 19) / 20 + 8; }
    return base * (1 + (tube > 0 ? (static_cast<u64>(1) << tube) : 1)) + 20;
  }

  /*
    The enclosure [m - err, m + err] 2^fe of |f|, with the sign fneg: the
    doubles around f, or false where a double is in it, or it goes over a power
    of two or past the normal doubles.
  */
  inline bool fast_doubles(bool fneg, u64 m, int fe, u64 rho, double& lo, double& hi)
  {
    u64 eh, el;
    mul64(rho, m, eh, el);
    const u64 err = ((eh << 1) | (el >> 63)) + 2;
    const u64 a = m - err, b = m + err;
    if (a < (static_cast<u64>(1) << 63) || b < m) { return false; }
    if ((a & 0x7ffu) == 0 || (a >> 11) != (b >> 11)) { return false; }
    const int field = fe + 11 + 52 + 1023; // |f| = A 2^(fe + 11), A of 53 bits
    if (field < 1 || field > 2046) { return false; }
    const u64 bits = (static_cast<u64>(field) << 52) | ((a >> 11) - (static_cast<u64>(1) << 52));
    if (fneg) {
      lo = dbl_of_bits(true, bits + 1);
      hi = dbl_of_bits(true, bits);
    } else {
      lo = dbl_of_bits(false, bits);
      hi = dbl_of_bits(false, bits + 1);
    }
    return true;
  }

  /*
    The centre of the enclosure of f: |f| = fm 2^fe with the sign fneg, off by
    rho units of 2^-63 relatively at most, or false for a x, y out of its
    domain.
  */
  inline bool fast_centre(double x, double y, bool& fneg, u64& fm, int& fe, u64& rho)
  {
    const u64 one63 = static_cast<u64>(1) << 63;
    const double y0 = gaol_cr_log1p(x);
    if (!(std::fabs(y0) <= 8.0) || y0 == 0.0) { return false; }
    bool xneg, y0neg, yneg;
    u64 xm, y0m, ym;
    int xe, y0e, ye;
    norm64(x, xneg, xm, xe); // x = xm 2^xe, 2^63 <= xm < 2^64
    norm64(y0, y0neg, y0m, y0e);
    norm64(y, yneg, ym, ye);
    U128 LM;
    bool Lneg;
    int g;
    u64 base;
    if (std::fabs(y0) <= 0.5) {
      if (!fast_log_small(xneg, xm, xe, y0neg, y0m, y0e, LM, Lneg, g)) { return false; }
      base = 14;
    } else {
      if (!fast_log_large(xneg, xm, xe, y0neg, y0m, y0e, LM, Lneg, g)) { return false; }
      base = 20;
    }
    const int lbits = bitlen(LM);
    if (lbits < 64) { return false; }
    const u64 Lt = shr(LM, lbits - 64).l; // the 64 high bits of L, at 2^(g + lbits - 64)
    // t = y L
    u64 th, tl;
    mul64(ym, Lt, th, tl);
    u64 tm;
    int tz;
    top64(th, tl, tm, tz);
    const int te = ye + (g + lbits - 64) + 64 - tz; // |t| = tm 2^te
    const bool tneg = yneg != Lneg;
    if (te + 64 > 3) { return false; } // |t| < 2^(te + 64) <= 8
    int k = 0;
    bool sneg = false;
    u64 Rr = 0;
    if (te + 64 >= -1) { // |t| >= 0.25: the multiple of log(2) nearest to it
      fast_reduce(shl(mk(0, tm), te + 120), k, sneg, Rr);
    }
    if (k == 0) {
      // |t| < log(2) / 2: f = t G(t)
      const int sh = -(te + 64);
      if (sh < 1) { return false; }
      const u64 Rt = (sh >= 64) ? 0 : (tm >> sh);
      const u64 Gt = g64(Rt, tneg);
      u64 fh, fl;
      mul64(tm, Gt, fh, fl);
      u64 fm0;
      int fz;
      top64(fh, fl, fm0, fz);
      fneg = tneg;
      fm = fm0;
      fe = te + 1 - fz;
      rho = fast_rho(0, 0, base);
      return true;
    }
    // |t| = k log(2) + s, |s| <= log(2) / 2: f = 2^k (1 + r G(r)) - 1 for r = s (t > 0) or -s (t < 0)
    const bool rneg = tneg ? !sneg : sneg;
    const u64 Gr = g64(Rr, rneg);
    u64 eh, el;
    mul64(Rr, Gr, eh, el);
    const u64 Vn = rneg ? one63 - eh : one63 + eh;
    U128 F;
    if (!tneg) {
      u64 cb;
      F = sub(shl(mk(0, Vn), k), mk(0, one63), cb); // 2^k V - 1
    } else {
      F = mk(0, one63 - (Vn >> k)); // 1 - 2^-k V
    }
    const int fbits = bitlen(F);
    fneg = tneg;
    fm = (fbits >= 64) ? shr(F, fbits - 64).l : shl(F, 64 - fbits).l;
    fe = fbits - 64 - 63;
    rho = fast_rho(k, te + 64, base);
    return true;
  }

  // The doubles around f if the fast phase settles them, or false
  inline bool compoundm1_fast(double x, double y, double& lo, double& hi)
  {
    bool fneg;
    u64 fm, rho;
    int fe;
    return fast_centre(x, y, fneg, fm, fe, rho) && fast_doubles(fneg, fm, fe, rho, lo, hi);
  }

  /* ---------------------------------------------------------------------- */
  /* The bounds of compoundm1 at a point                                    */
  /* ---------------------------------------------------------------------- */

  /*
    The bounds with the 128-bit balls, for a finite x > -1 other than 0 and a
    finite y other than 0 or 1: the part of compoundm1_point() that every
    value can come to.
  */
  inline void compoundm1_balls(double x, double y, double& lo, double& hi)
  {
    const double y0 = gaol_cr_log1p(x);
    const Num L = log1p_ball(x, y0);
    const Num T = num_mul(num_of_double(y), L);
    if (T.neg && T.e >= 7) {
      // y log(1 + x) <= -64: the value is -1 + e^-64 at most, past the doubles
      lo = -1.0;
      hi = next_up(-1.0);
      return;
    }
    if (!T.neg && T.e >= 12) {
      // y log(1 + x) >= 2048: past the largest double
      lo = dbl_max();
      hi = dbl_inf();
      return;
    }
    const Num F = expm1_ball(T);
    bool lo_exact, hi_exact;
    round_ball(F, lo, hi, lo_exact, hi_exact);
    // The value is above -1
    if (lo < -1.0) { lo = -1.0; }
    if (hi < -1.0) { hi = -1.0; }
    // A double in the ball: the value if (1 + x)^y = 1 + d
    double d = 0.0;
    bool candidate = false;
    if (lo_exact) { d = lo; candidate = true; }
    else if (hi_exact) { d = hi; candidate = true; }
    else if (lo < dbl_inf() && next_up(lo) < hi) { d = next_up(lo); candidate = true; }
    if (!candidate) {
      return;
    }
    Dyadic w;
    if (d > -1.0 && d < dbl_inf() && one_plus(d, w) && power_equals(x, y, w)) {
      lo = hi = d;
      return;
    }
    /*
      A power (1 + x)^y that is the double d, 2^54 or more: f = d - 1 is no
      double, between the double below d and d (the doubles below d are 2 or
      more apart). The ball of f holds d once 1 is below its radius, from m =
      118 on for 2^m - 1 = (1/2)^-m - 1 (its width is 2^-118 relatively near
      m = 113), and the rule applies from 2^54 on, where d - 1 is no double.
    */
    if (d > 0.0 && d < dbl_inf() && expo(d) >= 54 && power_equals(x, y, dyadic_of(d))) {
      lo = next_down(d);
      hi = d;
      return;
    }
    /*
      A tiny x: f = P (1 + (y - 1) x / 2 + ...) for P = y x, which is a double
      d where the product of x and y takes 53 bits. The ball cannot tell the
      side of d where (y - 1) x / 2 is below its radius (2^-120 relatively
      for |y log1p(x)| below 1), and the sign of f - d = P (y - 1) x / 2, that
      of y (y - 1), gives it as long as the first term of the series is the
      one that counts: x (|y| + 2) <= 1/2 makes the ratio of two terms at
      most 1/2, and the first of them 1/6 of the second term at most, so that
      the terms from the third on weigh less than a third of the second.
    */
    if (d != 0.0 && expo(x) + ((expo(y) + 1 > 2) ? expo(y) + 1 : 2) + 2 <= -1
        && product_equals(x, y, d)) {
      if (y > 1.0 || y < 0.0) { // f - d = d (y - 1) x / 2 has the sign of y (y - 1) x^2
        lo = d;
        hi = next_up(d);
      } else {
        lo = next_down(d);
        hi = d;
      }
    }
  }

  /*
    lo <= (1 + x)^y - 1 <= hi for a finite x > -1 other than 0 and a finite y
    other than 0: the doubles next to the value, or the value twice where it
    is a double. lo and hi are the greatest double at most the value and the
    least double at least it, whatever the rounding direction. A bound that
    is 0 is +0, never -0.
  */
  inline void compoundm1_point(double x, double y, double& lo, double& hi)
  {
    if (y == 1.0) {
      lo = hi = x; // (1 + x) - 1 = x, whatever the rounding of 1 + x
      return;
    }
    if (y == -1.0 && reciprocal_m1(x, lo, hi)) { return; }
    if (compoundm1_fast(x, y, lo, hi)) { return; }
    compoundm1_balls(x, y, lo, hi);
  }

} // namespace gaol_cm1

#endif /* GAOL_COMPOUNDM1_H */
