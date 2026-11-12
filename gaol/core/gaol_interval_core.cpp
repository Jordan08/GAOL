/*-*-C++-*------------------------------------------------------------------
 * GAOL -- not Just Another Interval Library
 *--------------------------------------------------------------------------
 * Copyright (c) 2001 Swiss Federal Institute of Technology, Switzerland
 * Copyright (c) 2002-2006 Laboratoire d'Informatique de
 *                         Nantes-Atlantique, France
 * Copyright (c) 2026 ENSTA, France
 *
 * By: Frederic Goualard, Jordan NININ
 *--------------------------------------------------------------------------
 * GAOL is a software distributed WITHOUT ANY WARRANTY. Read the associated
 * COPYING.LIB file for information.
 *--------------------------------------------------------------------------*/

/*!
  \file   gaol_interval_core.cpp
  \brief  The operations of the intervals: the members of interval, + - * / and sqrt of each
          kind (the *_impl.h it includes), the kernel of the integer powers, max, min, abs, fma.

  \author Goualard Frederic, NININ Jordan
  \date   2001-09-28
*/

// SIMD (GAOL v5, doc/simd.md): with SSE2 and NEON the two bounds of an interval are the two lanes of one
// vector, and + - * / sqr are vector kernels of gaol_interval_sse_impl.h and gaol_interval_neon_impl.h (sqrt too
// with NEON); the AVX-512 path of gaol_interval_avx512_impl.h carries the rounding direction in the
// instruction, its square root being two scalar roots, and the integer powers have a two-lane FMA kernel
// with NEON, and with SSE2 and AVX-512 where __FMA__ is. What is scalar in the sources of the intervals is
// the elementary functions, which are CORE-MATH calls, one per bound, and the operations made of branches on
// the bounds.
// doc/simd.md lists what could still gain (sqrt with SSE2, mid(), the reduction of the argument of sin(),
// cos() and tan()) and what the other processors would need

#include "gaol/core/gaol_interval_internal.h"
// x * 2^n rounded once, for the integer powers beyond the doubles (GAOL v5)
#include "gaol/math/gaol_ldexp.h"
// The nearest integer, halfway values to the even one, from the bits (GAOL v5)
#include "gaol/math/gaol_roundeven.h"

/*
  The operations of the intervals: the members of interval, +, -, *, / and
  sqrt of each kind of intervals (gaol_interval_sse_impl.h,
  gaol_interval_neon_impl.h or gaol_interval_fpu_impl.h, and
  gaol_interval_avx512_impl.h, included below), the kernel of the integer
  powers, max, min, abs, fma, the cancellative operations, erf, erfc, chi,
  midpoint() and bisect(). The other functions are in the sources of their
  domain (GAOL v5), listed in gaol_interval_internal.h.
*/

namespace gaol_core {

  /*
    x^n for an int n, |n| >= 2 but n = 2 (x^2 is sqr(), which gaol_pown() and
    pown_upward() take before this code, GAOL v5), with the tightest bounds,
    from exact products (GAOL v5). GAOL rounded each
    product of its binary exponentiation outward, and its bounds were up to
    n + 1 doubles beyond the tightest (32 to 39 at n = 30, 1300 to 1500 at
    n = 1000); GAOL v5 then kept each power as a double and a rest, rounded
    downward and upward apart, which cost 41 ns for x^1000 against 9.7 with
    GAOL 4, and took x^-m as 1/x^m, two roundings, one or two doubles beyond
    the tightest bound.

    The bounds come from two corners c0 and c1 (pown_corners()): x^n is c0^n
    at its least and c1^n at its greatest. Both powers are computed at once,
    |c0|^m in lane 0 and |c1|^m in lane 1 of an SSE2 register, or of a NEON one
    (of a pair of doubles in the other builds, the same operations in the same order), as
    h + l, h being the power rounded and l a rest, the sign of c being given
    back to it at the end for an odd m: the products of c and of -c, rounded
    upward, are not opposite, and the powers of c itself would make
    pow([-c], n) other than -pow([c], n) where c^n is within K of a double,
    and other than the bound pown_point() gives from |c|. From the highest
    bit of m = |n| down, a square
      p = h*h, e = h*h - p, t = 2h + l, l' = l*t + e, h' = p,
    and, for each bit set, a product by |c|
      q = h*|c|, f = h*|c| - q, l' = l*|c| + f, h' = q,
    e and f being exactly the rests of the products (fma(), the products
    being normal), and t and l' rounded once each (fma()). The error of
    h + l is bounded beforehand, relatively: |h + l - |c|^m| <= K h with
    K = (8 m L + 16) 2^-104, L being the number of bits of m (below). A
    square, m = 2, is h + l exactly, and comes here for n = -2 only, whose K
    is that of the inverse (below): x^2 is sqr(), which has no error to
    bound. Each bound is then rounded once, upward, the rounding direction being upward
    throughout, v + w being h + l or -(h + l), of the sign of c^m:
      lower bound = -RU(-v + RU(K h - w)), upper bound = RU(v + RU(K h + w)),
    the lane 0 holding the opposite of the lower bound, as the SSE2 intervals
    store it. The bounds are the tightest, and exact where c^m is a double:
    every product is exact then (c^k is a double for each k <= m), each rest
    e and f is +0, and K is taken as 0 where the rests or-ed together are 0
    (without that, pown([2, 3], 3) was [8 - 2^-50, 27 + 2^-48]). They are one
    double beyond where c^m is within 2K of a double, relatively (K is 2^-98
    at m = 3, 2^-88 at m = 1000, 2^-65 at m = 2^31 - 1): 2^-46 of the bounds
    at m = 3 at random, but 37.5 % of them where c is within 8 doubles of 1
    (12 of 32 bounds at m = 3), 28.5 % over m = 3 to 10 and 8.3 % over m = 3
    to 200, c^m being then that close to a double: pow([1 + 2^-52], 3) is
    [1 + 2^-51, 1 + 2^-50], [1 + 3 2^-52, 1 + 2^-50] being the tightest.
    Deciding these bounds with CORE-MATH's pow took 55 to 270 ns, these
    powers being its hard cases: they are left so (decided on 13 October
    2026).

    For n < 0, 1/c^m from h + l: q = 1/h rounded upward, R = 1 - q(h + l) =
    (1 - q h) - q l, 1 - q h being exact, r = R rounded once, and
    1/(h + l) = q (1 + R + R^2 + R^3/(1 - R)), which q + q (r + r^2) gives
    to within the error of r, two roundings and q R^3: K = 2 K_m + 64 2^-104
    + 2 m^3 2^-156 (below). The result is exact where 1/c^m is a double, r
    being +0 too. 1/x is inverse() itself, correctly rounded.

    The bound K, with u = 2^-52 and m <= 2^31. A rounding upward is within
    u|z| of a normal z, and within 2^-1074 of any z, which is u^2/2 of
    2^-969, below which no power c^k goes here. After j steps (squares and
    products by c) reaching c^k = X, k u <= 2^-20 and
      |l| <= b |X|, b = (k - 1) u (1 + 2(k - 1) u),
      |h + l - X| <= a |X|, a = 3 k j u^2.
    The first square gives c^2 = h + l exactly, |l| < u|X|. A square takes
    d = h + l - X to d(2X + d) + l(t - 2h - l) + (l' - l t - e), and a
    product by c to c d + (l' - l c - f), with |h| <= (1 + a + b)|X|,
    |e| <= u h^2 and |f| <= u(1 + u)|h c|: the bounds become
      b' = (1 + u)^2 b (2 + 2a + 3b) + (1 + u) u (1 + a + b)^2 + u^2/2,
      a' = a(2 + a) + u b (2 + 2a + 3b) + u b' + u^2/2
    after a square, and
      b' = (1 + u)(b + u (1 + u)(1 + a + b)) + u^2/2,  a' = a + u b' + u^2/2
    after a product, about 2b + u = (2k - 1) u and 2a + 4 k u^2, b + u and
    a + k u^2: below the bounds for 2k and j + 1, for k + 1 and j + 1, by
    (b + u)^2/4 and 2 k u^2 at least for the terms of second order
    (tests/tools/pow/pown_bound.py checks it with rationals). With
    j <= 2(L - 1), |h + l - c^m| <= 6 m (L - 1) u^2 |X|, which is below K|h|,
    |h| being above (1 - 2^-19)|X|. For n < 0, |R| <= u + |q l| <=
    m u (1 + 2^-17); the roundings of r, q r and q r (1 + r) are within
    3 m u^2 (1 + 2^-16)|q| together, q R^3/(1 - R) within 1.0001 m^3 u^3 |q|,
    and 1/(h + l) within 6 m (L - 1) u^2 (1 + 2^-17)|q| of 1/c^m: below
    2 K_m |q| for the first and the third, 2 m^3 u^3 |q| for the second, and
    64 u^2 |q| for the roundings within 2^-1074. A product scaled by 2^-500 in
    pown_point() rounds its rest l within 2^-1074 too, the power being above
    1 there.

    Each power c^k stays within [2^-969, 2^961] where |c| is within
    [pown_least[L], pown_greatest[L]] (c = 0 too for n > 0): the rests are
    doubles, and q = 1/h is normal, for every m of L bits. Any other corner
    takes pown_point(), one bound at a time, c^m being computed there with c
    scaled to [1, 2) and the power kept below 2^501 by powers of two, scaled
    back at the end, rounded once in the direction of the bound: the same
    bounds, the tightest, for the powers beyond the doubles and below 2^-968
    too, and the limits at 0 and at the infinities. No operation computes
    oo - oo or 0 oo: x^n raises no invalid-operation exception, where the
    exponentiation from an infinite bound would.
  */

  // 2^(-968/2^L) rounded upward and 2^(960/2^L) rounded downward, for the
  // m of L bits, 2 <= L <= 32 (2^(L-1) <= m < 2^L): |c|^m is then within
  // [2^-968, 2^960] for |c| within [pown_least[L], pown_greatest[L]]
  static const double pown_least[33] = {
    0.0, 0.0, 1.4149498560666738e-73, 3.76158192263132e-37, 6.133173666733497e-19,
    7.83145814439016e-10, 2.7984742529439433e-05, 0.005290060730222238,
    0.0727328036735986, 0.26969019943927997, 0.5193170509806895,
    0.7206365595643129, 0.848903150874299, 0.9213594037476901,
    0.9598746812723472, 0.979731943580665, 0.9898140954647318,
    0.9948940121765392, 0.9974437388527432, 0.9987210515718307,
    0.9993603211914263, 0.999680109430725, 0.9998400419220692,
    0.9999200177624554, 0.9999600080815509, 0.9999800038408523,
    0.9999900018704448, 0.9999950009227271, 0.9999975004582398,
    0.999998750228339, 0.9999993751139743, 0.9999996875569384,
    0.999999843778457
  };
  static const double pown_greatest[33] = {
    0.0, 0.0, 1.7668470647783843e+72, 1.329227995784916e+36, 1.152921504606847e+18,
    1073741824.0, 32768.0, 181.01933598375615, 13.454342644059432,
    3.668016172818685, 1.9152065613971472, 1.3839098819638318,
    1.1763969916502812, 1.0846183622133092, 1.041450124688316,
    1.020514637174948, 1.0102052450739643, 1.0050896701657839,
    1.0025416052043843, 1.0012699961570726, 1.0006347965951776,
    1.0003173479427303, 1.0001586613846476, 1.000079327545894,
    1.0000396629863706, 1.0000198312965451, 1.000009915599113,
    1.0000049577872665, 1.0000024788905608, 1.0000012394445121,
    1.000000619722064, 1.000000309860984, 1.00000015493048
  };

  // The number of bits of m >= 1
  static GAOL_POWN_INLINE unsigned int pown_bit_length(unsigned int m)
  {
#if defined(__GNUC__) || defined(__clang__)
    return 32u - static_cast<unsigned int>(__builtin_clz(m));
#else
    unsigned int length = 1u;
    while (length < 32u && (m >> length) != 0u) {
      ++length;
    }
    return length;
#endif
  }

  // K for c^m and for c^-m (see above), in the upward rounding: 2^-104 is
  // 4.930380657631324e-32 and 2^-155 2.1895288505075267e-47. pown_bound() is
  // for m >= 3: x^2 is sqr(), which gaol_pown() and pown_upward() take before
  // the kernel (c^2 is h + l exactly, and its K was 0, GAOL v5); c^-2 comes
  // to pown_bound_inverse(), whose K covers the roundings of the inverse
  static GAOL_POWN_INLINE double pown_bound(unsigned int m, unsigned int L)
  {
    return (8.0*m*L + 16.0)*4.930380657631324e-32;
  }

  static GAOL_POWN_INLINE double pown_bound_inverse(unsigned int m, unsigned int L)
  {
    const double dm = m;
    return 2.0*(8.0*m*L + 16.0)*4.930380657631324e-32 + 64.0*4.930380657631324e-32
      + dm*dm*dm*2.1895288505075267e-47;
  }

  static GAOL_POWN_INLINE std::uint64_t pown_bits_of(double x)
  {
    std::uint64_t bits;
    std::memcpy(&bits, &x, sizeof bits);
    return bits;
  }

  /*
    c^m as h + l in each lane, m >= 2, and the rests or-ed in s (see above),
    the operations on the two lanes being those of the class Ops (see below).
    Inlined by force, so that each constant m of pown_powers() unrolls it:
    the loop and its tests cost as much as the products for a small m
    (x^3 took 6.1 ns rather than 4.1 with Clang 18 on an i7-1185G7, in a
    prototype).
  */
  template <class Ops>
  static GAOL_POWN_INLINE_T void pown_products(const typename Ops::lanes& c, unsigned int m, typename Ops::lanes& h,
                                               typename Ops::lanes& l, typename Ops::rests& s)
  {
    typedef typename Ops::lanes Lanes;
    unsigned int bit = (1u << (pown_bit_length(m) - 1u)) >> 1;
    h = Ops::mul(c, c);
    l = Ops::fms(c, c, h); // c^2 = h + l, exactly
    s = Ops::first_rest(l);
    for (;;) {
      if ((m & bit) != 0u) {
        const Lanes q = Ops::mul(h, c);
        const Lanes f = Ops::fms(h, c, q);
        Ops::add_rest(s, f);
        l = Ops::fma(l, c, f);
        h = q;
      }
      bit >>= 1;
      if (bit == 0u) {
        break;
      }
      const Lanes p = Ops::mul(h, h);
      const Lanes e = Ops::fms(h, h, p);
      Ops::add_rest(s, e);
      l = Ops::fma(l, Ops::twice_plus(h, l), e); // 2h + l rounded once, 2h being exact
      h = p;
    }
  }

#define GAOL_POWN_CASE(k) case k: pown_products<Ops>(c, k##u, h, l, s); break;
  template <class Ops>
  static GAOL_POWN_INLINE_T void pown_powers(const typename Ops::lanes& c, unsigned int m, typename Ops::lanes& h,
                                             typename Ops::lanes& l, typename Ops::rests& s)
  {
    switch (m) {
      GAOL_POWN_CASE(2) GAOL_POWN_CASE(3) GAOL_POWN_CASE(4) GAOL_POWN_CASE(5)
      GAOL_POWN_CASE(6) GAOL_POWN_CASE(7) GAOL_POWN_CASE(8) GAOL_POWN_CASE(9)
      GAOL_POWN_CASE(10) GAOL_POWN_CASE(11) GAOL_POWN_CASE(12) GAOL_POWN_CASE(13)
      GAOL_POWN_CASE(14) GAOL_POWN_CASE(15) GAOL_POWN_CASE(16) GAOL_POWN_CASE(17)
      GAOL_POWN_CASE(18) GAOL_POWN_CASE(19) GAOL_POWN_CASE(20) GAOL_POWN_CASE(21)
      GAOL_POWN_CASE(22) GAOL_POWN_CASE(23) GAOL_POWN_CASE(24) GAOL_POWN_CASE(25)
      GAOL_POWN_CASE(26) GAOL_POWN_CASE(27) GAOL_POWN_CASE(28) GAOL_POWN_CASE(29)
      GAOL_POWN_CASE(30) GAOL_POWN_CASE(31) GAOL_POWN_CASE(32)
    default:
      pown_products<Ops>(c, m, h, l, s);
      break;
    }
  }
#undef GAOL_POWN_CASE

  /*
    The bounds of c^n for a double c and an int n, |n| >= 2, the rounding
    direction being upward: the corners pown_large_upward() does not take
    at once (see above). 0^n is 0 for n > 0, and +oo for n < 0, -oo at -0
    for an odd n, the limit of x^n at 0 on its side; +-oo^n is +-oo for n > 0
    and 0 for n < 0. Any other c is |c| = y 2^(e - 1), y in [1, 2), whose
    power is (h + l) 2^z, the product being scaled by 2^-500 above 2^500:
    exactly, but for a rest l below 2^-522, rounded within 2^-1074 of it,
    which the bound K holds. The bounds of y^n, which are within
    [2^-502, 2^502], are then scaled by 2^z, rounded once in their direction
    (gaol_ldexp()).
  */
  static void pown_point(double c, int n, double& lo, double& hi)
  {
    const unsigned int m = (n > 0) ? static_cast<unsigned int>(n) : 0u - static_cast<unsigned int>(n);
    const double g = std::fabs(c);
    if (g == 0.0 || g == GAOL_INFINITY) {
      lo = hi = ((g == 0.0) == (n < 0)) ? GAOL_INFINITY : 0.0;
    } else {
      // 2^500 and 2^-500
      const double big = 3.273390607896142e+150, small = 3.054936363499605e-151;
      const unsigned int L = pown_bit_length(m);
      int ey;
      const double y = 2.0*std::frexp(g, &ey); // exact
      --ey;
      unsigned int bit = (1u << (L - 1u)) >> 1;
      double h = y*y;
      double l = fma_once(y, y, -h);
      std::uint64_t s = pown_bits_of(l);
      long long z = 2LL*ey;
      for (;;) {
        if ((m & bit) != 0u) {
          const double q = h*y;
          const double f = fma_once(h, y, -q);
          s |= pown_bits_of(f);
          l = fma_once(l, y, f);
          h = q;
          z += ey;
        }
        if (h > big) {
          h *= small;
          l *= small;
          z += 500;
        }
        bit >>= 1;
        if (bit == 0u) {
          break;
        }
        const double p = h*h;
        const double e = fma_once(h, h, -p);
        s |= pown_bits_of(e);
        l = fma_once(l, fma_once(2.0, h, l), e);
        h = p;
        z *= 2;
        if (h > big) {
          h *= small;
          l *= small;
          z += 500;
        }
      }
      double K;
      if (n < 0) {
        const double q = 1.0/h;
        const double r = fma_once(-q, l, fma_once(-q, h, 1.0));
        const double qr = q*r;
        l = fma_once(qr, r, qr);
        h = q;
        s |= pown_bits_of(r);
        z = -z;
        K = pown_bound_inverse(m, L);
      } else {
        K = pown_bound(m, L);
      }
      K = (s != 0u) ? K : 0.0;
      // h > 0: the bound above, and the opposite of the bound below, of y^n,
      // then scaled
      const double up = h + fma_once(K, h, l);
      const double down = -h + fma_once(K, h, -l);
      const int w = (z > 2200) ? 2200 : ((z < -2200) ? -2200 : static_cast<int>(z));
      hi = gaol_ldexp(up, w);
      lo = -gaol_ldexp(down, w);
    }
    if (std::signbit(c) && (m & 1u) != 0u) {
      const double t = lo;
      lo = -hi;
      hi = -t;
    }
  }

  // x^n on [a, b] for |n| >= 2, one bound at a time (pown_point())
  static interval pown_by_corners(double a, double b, int n)
  {
    double c0, c1, lo, hi, other;
    if (!pown_corners(a, b, n < 0, (n & 1) != 0, c0, c1)) {
      return interval::emptyset();
    }
    pown_point(c0, n, lo, other);
    pown_point(c1, n, other, hi);
    return interval(lo, hi);
  }

  /*
    The operations on the two lanes are those of a class for each kind of
    intervals (GAOL v5): a pair of doubles here, for the
    FPU intervals and for the SSE2 intervals without the FMA instructions; an
    SSE2 register with FMA in gaol/core/gaol_interval_sse_impl.h, the embedded rounding
    of AVX-512 in gaol/core/gaol_interval_avx512_impl.h and a NEON register in
    gaol/core/gaol_interval_neon_impl.h. Each has the types lanes and rests (the rests
    or-ed together) and the operations mul, fms (a * b - c), fma, twice_plus
    (2 a + b), first_rest and add_rest; the file of the intervals defines the
    function pown_large_inline() that makes the two corners and the bounds from
    the powers pown_powers<Ops>() gives (the others are generic: the powers
    pown_products() and pown_powers(), the corners and the corner alone,
    pown_point()). The class of the pair of doubles also has the function of
    the corners and the bounds in plain doubles, which the intervals without
    a kernel of their own call, pown_scalar_ops::large().
  */
  struct pown_scalar_ops {
    struct lanes { double l0, l1; };
    struct rests { std::uint64_t l0, l1; };
    static GAOL_POWN_INLINE lanes mul(lanes a, lanes b)
    {
      const lanes r = { a.l0*b.l0, a.l1*b.l1 };
      return r;
    }
    static GAOL_POWN_INLINE lanes fms(lanes a, lanes b, lanes c)
    {
      const lanes r = { fma_once(a.l0, b.l0, -c.l0), fma_once(a.l1, b.l1, -c.l1) };
      return r;
    }
    static GAOL_POWN_INLINE lanes fma(lanes a, lanes b, lanes c)
    {
      const lanes r = { fma_once(a.l0, b.l0, c.l0), fma_once(a.l1, b.l1, c.l1) };
      return r;
    }
    static GAOL_POWN_INLINE lanes twice_plus(lanes a, lanes b)
    {
      const lanes r = { fma_once(2.0, a.l0, b.l0), fma_once(2.0, a.l1, b.l1) };
      return r;
    }
    static GAOL_POWN_INLINE rests first_rest(lanes e)
    {
      const rests s = { pown_bits_of(e.l0), pown_bits_of(e.l1) };
      return s;
    }
    static GAOL_POWN_INLINE void add_rest(rests& s, lanes e)
    {
      s.l0 |= pown_bits_of(e.l0);
      s.l1 |= pown_bits_of(e.l1);
    }

    /*
      I^n for |n| >= 2 and a nonempty I, the rounding direction being upward,
      with the pair of doubles: the corners and the bounds as pown_large_inline()
      of the other intervals makes them (see pown_large_inline() below)
    */
    static GAOL_POWN_INLINE_T interval large(const interval& I, int n)
    {
      const bool negative = n < 0;
      const unsigned int m = negative ? 0u - static_cast<unsigned int>(n) : static_cast<unsigned int>(n);
      const unsigned int L = pown_bit_length(m);
      const double a = I.left(), b = I.right();
      if (!negative || a > 0.0 || b < 0.0) {
        double c0, c1;
        (void)pown_corners(a, b, negative, (m & 1u) != 0u, c0, c1);
        const double least = pown_least[L], greatest = pown_greatest[L];
        const double g0 = std::fabs(c0), g1 = std::fabs(c1);
        if (((g0 >= least && g0 <= greatest) || (!negative && g0 == 0.0))
            && ((g1 >= least && g1 <= greatest) || (!negative && g1 == 0.0))) {
          const lanes c = { g0, g1 };
          lanes h, l;
          rests s;
          pown_powers<pown_scalar_ops>(c, m, h, l, s);
          double K;
          if (negative) {
            const double q0 = 1.0/h.l0, q1 = 1.0/h.l1;
            const double r0 = fma_once(-q0, l.l0, fma_once(-q0, h.l0, 1.0));
            const double r1 = fma_once(-q1, l.l1, fma_once(-q1, h.l1, 1.0));
            const double qr0 = q0*r0, qr1 = q1*r1;
            l.l0 = fma_once(qr0, r0, qr0);
            l.l1 = fma_once(qr1, r1, qr1);
            h.l0 = q0;
            h.l1 = q1;
            s.l0 |= pown_bits_of(r0);
            s.l1 |= pown_bits_of(r1);
            K = pown_bound_inverse(m, L);
          } else {
            K = pown_bound(m, L);
          }
          const double K0 = (s.l0 != 0u) ? K : 0.0, K1 = (s.l1 != 0u) ? K : 0.0;
          // the signs of c0^n and c1^n, those of c0 and c1 for an odd m
          const double t0 = ((m & 1u) != 0u && std::signbit(c0)) ? -1.0 : 1.0;
          const double t1 = ((m & 1u) != 0u && std::signbit(c1)) ? -1.0 : 1.0;
          const double down = -t0*h.l0 + fma_once(K0, h.l0, -t0*l.l0);
          const double up = t1*h.l1 + fma_once(K1, h.l1, t1*l.l1);
          return interval(-down, up);
        }
      }
      return pown_by_corners(a, b, n);
    }
  };

  /*
    pown_large_inline(I, n), I^n for |n| >= 2 and a nonempty I, the rounding
    direction being upward, is defined by the file of each kind of intervals,
    included below (gaol_interval_sse_impl.h, _neon_impl.h and _fpu_impl.h): both
    corners at once, in the lanes of the kind, where they are within the range
    of pown_least[] and pown_greatest[], and pown_by_corners() otherwise (see
    pown_products()). It is inlined in pown_checked() alone, the body of
    gaol_pown() for n != 2: its switch over the small exponents is long, and
    the callers that take pown_upward() after their own check call
    pown_large_upward(). pown_large_er() of gaol_interval_avx512_impl.h is its
    twin rounded by the embedded rounding.
  */

#if (defined(__x86_64__) || defined(_M_X64)) && GAOL_USING_SSE2_INSTRUCTIONS && GAOL_AVX512_ARITHMETIC
  // GAOL_AVX512_ARITHMETIC, decided when GAOL is configured: +, -, *, /, sqr,
  // sqrt and, with FMA, the integer powers beyond 1 and -1 take the AVX-512
  // path of gaol/core/gaol_interval_avx512_impl.h, which sets no rounding direction,
  // and clears the flush-to-zero modes as the other operations do
  // (flush_guard there; GAOL v5)
#  include <immintrin.h>   // the AVX-512 intrinsics of the path below
#  include "gaol/core/gaol_interval_avx512_impl.h"
#endif
#if GAOL_USING_SSE2_INSTRUCTIONS
#  include "gaol/core/gaol_interval_sse_impl.h"
#elif GAOL_USING_NEON_INSTRUCTIONS
#  include "gaol/core/gaol_interval_neon_impl.h"
#else
#  include "gaol/core/gaol_interval_fpu_impl.h"
#endif // GAOL_USING_SSE2_INSTRUCTIONS

// The file included above closes gaol_core, which this file opened above:
// it is opened again for what follows (GAOL v5)
namespace gaol_core {

  /*
    \brief test for evenness
    \warning d should not be +/-oo
  */
  bool feven(double d)
  {
    // A subnormal is no integer, which its bits tell whatever the modes that
    // flush the subnormals to zero: under denormals-are-zero, 0.5*d and d
    // were 0 for the comparison, and feven(2^-1074) was true (GAOL v5)
    if (gaol_detail::bound_is_subnormal(d)) {
      return false;
    }
    return integer_is_even(d);
  }


  /*
    The double above x, from its bits: the next one away from -oo, -0 above
    the least negative subnormal, the least positive subnormal above a zero,
    +oo above DBL_MAX; a NaN and +oo stay as they are. nextafter() of the C
    library computes with doubles, which the modes that flush the subnormals
    to zero change: under denormals-are-zero, the UCRT of Windows and the
    libm of macOS (arm64 and x86_64) did not give the subnormal above 100
    times 2^-1074 (GAOL v5, continuous integration)
  */
  static double next_double_above(double x)
  {
    if (!(x == x) || x == GAOL_INFINITY) {
      return x;
    }
    std::uint64_t bits = gaol_detail::bound_bits(x);
    if ((bits << 1) == 0) {
      return double_denorm_min;
    }
    if ((bits >> 63) != 0) {
      --bits; // a smaller magnitude
    } else {
      ++bits;
    }
    double above;
    std::memcpy(&above, &bits, sizeof above);
    return above;
  }

  // The bounds compared as the relations compare them, and the double above
  // the left one taken from its bits, whatever the modes that flush the
  // subnormals to zero: under denormals-are-zero, [2^-1074, 3*2^-1074] was
  // canonical (GAOL v5)
  bool interval::is_canonical(void) const
  {
#if defined (_MSC_VER)
    return !is_empty() && gaol_detail::bound_greater_equal(next_double_above(left()), right());
#else
    // emptyset handled thanks to unorderedness of NaNs, with a quiet
    // comparison, which raises no invalid-operation exception on them (GAOL v5)
    return gaol_detail::bound_greater_equal(next_double_above(left()),right());
#endif
  }

  GAOL_POWN_TARGET interval pown_large_upward(const interval& I, int n)
  {
    return pown_large_inline(I, n);
  }

  /*
    One check of the rounding direction for each call (GAOL v5). The body of
    pown_large_upward() is inlined here, the function of gaol::pow(x, n) and
    of gaol_ieee1788::pown(x, n) for n != 2 (see gaol_pown() in
    gaol_interval_pow.cpp): its call
    took 0.4 to 0.6 ns more for x^3 to x^1000 (5.9 ns rather than 5.5 for x^3,
    with Clang 21 on an i7-1185G7).
  */
  GAOL_POWN_TARGET GAOL_POWN_NOINLINE interval pown_checked(const interval& I, int n)
  {
#if GAOL_POWN_ER
    // The embedded rounding sets no direction: the modes that flush the
    // subnormals to zero are cleared for the kernel (flush_guard), the bounds
    // read after, and the result made before the modes are set back
    if ((n < -1 || n > 1) && !I.is_empty()) {
      const flush_guard flush;
      __m128d stored;
      if (pown_large_er(flush_guard::in(I.left()), flush_guard::in(I.right()), n, stored)) {
        return interval(flush_guard::out(stored), gaol_detail::unchecked_bounds);
      }
    }
#endif
    GAOL_RND_ENTER();
    interval res = (I.is_empty() || (n >= -1 && n <= 1)) ? pown_upward(I, n) : pown_large_inline(I, n);
    GAOL_RND_KEEP(res);
    GAOL_RND_LEAVE();
    return res;
  }

  unsigned long long nb_fp_numbers(double a, double b)
  {
    // a and b compared as bounds, whatever the modes that flush the
    // subnormals to zero (gaol_port.h): under denormals-are-zero,
    // nb_fp_numbers(2^-1074, 2*2^-1074) was 1 (GAOL v5)
    if (!is_finite(a) || !is_finite(b) || gaol_detail::bound_greater(a, b)) {
      // Either a or b is a NaN or +/-oo, or [a,b] is empty? gaol_ERROR
      // throws, or aborts where the exceptions are disabled: there is no
      // value to return, whose line Visual C++ found unreachable (C4702)
      gaol_ERROR(std::domain_error,"invalid argument(s) in call to nb_fp_numbers()");
    }

    if (gaol_detail::bound_equal(a, b)) {
      return 1;
    }

    /*
      The doubles are numbered by the bits of their absolute values, which
      grow with the doubles, -0 and +0 being one number. With the bits of a
      and b themselves, the sign bit of a lower bound -0 (which [1, 2] - 1 has,
      and -0 >= 0), or of a negative a with b = +0, made the difference wrap
      around: GAOL 4 returned 13830554455654793217 for nb_fp_numbers(-0.0, 1.0).
    */
    ullidouble ai, bi;
    ai.d = std::fabs(a);
    bi.d = std::fabs(b);
    if (gaol_detail::bound_greater_equal(a, 0.0)) {
      return (bi.i-ai.i)+1;
    }
    if (gaol_detail::bound_less_equal(b, 0.0)) {
      return (ai.i-bi.i)+1;
    }
    // a < 0 < b: the doubles from a to -0 and from +0 to b, zero being counted once
    return bi.i+ai.i+1;
  }

  /*
    fma(X, Y, Z) of IEEE 1788-2015 (Table 9.1), whose tightness is required
    (12.10.2) (GAOL v5)

    x*y ranges over the hull of the products of the bounds of X and Y, x*y
    being bilinear, so x*y + z ranges from the least of those products plus
    inf Z to the greatest plus sup Z. Each corner is rounded once, by std::fma,
    upward for the upper bound and downward for the lower one as
    -fma(-a, b, -c): the bounds are the tightest. A bound 0 times an infinite
    one counts as 0, as in the product of intervals, where std::fma would give
    NaN; and an infinite bound of Z gives the bound of the result it is on.
  */
  static inline double fma_up(double a, double b, double c)
  {
    return (a == 0.0 || b == 0.0) ? c : fma_once(a, b, c);
  }

  /*
    The result of std::fma goes through rnd_keep() before it is negated: GCC
    folds -fma(-a, b, -c) into fma(a, b, c), a single vfmadd rounded upward
    whatever -frounding-math says, which gave the lower bound rounded the wrong
    way -- a bound one double above the exact one, the interval no longer
    enclosing it. The negations inside may be folded: -a*b - c is rounded
    upward all the same.
  */
  static inline double fma_down(double a, double b, double c)
  {
    return (a == 0.0 || b == 0.0) ? c : -gaol_core::rnd_keep(fma_once(-a, b, -c));
  }

  interval fma(const interval& X, const interval& Y, const interval& Z)
  {
    if (X.is_empty() || Y.is_empty() || Z.is_empty()) {
      return interval::emptyset();
    }
    // The check before the bounds are read, fma_up() and fma_down() comparing
    // them with 0 (see gaol/gaol_fpu.h)
    GAOL_RND_ENTER();
    const double xl = X.left(), xr = X.right(), yl = Y.left(), yr = Y.right();
    const double zl = Z.left(), zr = Z.right();
    double lower = -GAOL_INFINITY, upper = GAOL_INFINITY;
    if (zl != -GAOL_INFINITY) {
      lower = minimum(minimum(fma_down(xl, yl, zl), fma_down(xl, yr, zl)),
                      minimum(fma_down(xr, yl, zl), fma_down(xr, yr, zl)));
    }
    if (zr != GAOL_INFINITY) {
      upper = maximum(maximum(fma_up(xl, yl, zr), fma_up(xl, yr, zr)),
                      maximum(fma_up(xr, yl, zr), fma_up(xr, yr, zr)));
    }
    GAOL_RND_KEEP(lower); GAOL_RND_KEEP(upper);
    GAOL_RND_LEAVE();
    return interval(lower, upper);
  }

  /*
    cancelMinus and cancelPlus of IEEE 1788-2015 (10.5.6, 12.12.5) (GAOL v5)

    Whether the result exists depends on X being at least as wide as Y, that
    is sup X - sup Y >= inf X - inf Y, which rounding the two differences
    cannot decide when they are within a double of each other (note of
    12.12.5). Each difference is therefore computed exactly, as the sum of its
    value rounded to nearest and of its error, by the Fast2Sum of Dekker with
    the operand of the larger magnitude first, which is exact in the rounding
    to nearest: the rounded values compare as the exact ones unless they are
    equal, and the errors decide then.

    The bounds are finite, so an infinite sum is an overflow, and its error is
    not read: difference_at_least() decides by s alone, or halves the bounds.
    The TwoSum of Knuth, ap = s - b, bp = s - ap, e = (a - ap) + (b - bp),
    was not used: for an infinite s, ap is infinite and bp is inf - inf, and
    for a finite s too, where a = +-DBL_MAX and b of the other sign make a sum
    halfway between two doubles that rounds away from the exact one, by 2^970:
    s - b is then DBL_MAX + 2^970, which is infinite to nearest. The invalid-
    operation exception was raised (it stops a program that enabled it), and e
    was NaN, so that cancel_minus([DBL_MAX - 2^971, DBL_MAX], [2^970, 3 2^970]),
    whose bounds have the same width, read X as narrower than Y. s - a is
    exact, and so cannot overflow, when |b| <= |a| (GAOL v5)
  */
  static inline void two_sum(double a, double b, double& s, double& e)
  {
    s = a + b;
    if (std::isinf(s)) {
      e = 0.0;
      return;
    }
    if (std::fabs(a) < std::fabs(b)) {
      const double t = a;
      a = b;
      b = t;
    }
    const double z = s - a;
    e = b - z;
  }

  // Whether a1 - b1 >= a2 - b2, exactly
  static bool difference_at_least(double a1, double b1, double a2, double b2)
  {
    double s1, e1, s2, e2;
    GAOL_RND_NEAREST_ENTER();
    two_sum(a1, -b1, s1, e1);
    two_sum(a2, -b2, s2, e2);
    /* The four doubles go through rnd_keep() before the direction changes
       back: GCC computed them after GAOL_RND_NEAREST_LEAVE(), upward, where
       TwoSum is no longer exact, and cancel_minus([0.5], [4.9e-324, 1e-300]),
       whose exact differences are within a double of each other, took X to be
       wider than Y; the terms of the error of cancel_minus([DBL_MAX], [0.5])
       then read +oo and inf - inf, and raised FE_INVALID
       (GAOL v5) */
    s1 = gaol_core::rnd_keep(s1);
    e1 = gaol_core::rnd_keep(e1);
    s2 = gaol_core::rnd_keep(s2);
    e2 = gaol_core::rnd_keep(e2);
    GAOL_RND_NEAREST_LEAVE();
    const bool inf1 = std::isinf(s1), inf2 = std::isinf(s2);
    if (inf1 || inf2) {
      // A difference beyond the largest double decides the comparison unless
      // both are, with the same sign; the four doubles are then 2^970 or more
      // in magnitude (a difference beyond DBL_MAX of two doubles of at most
      // DBL_MAX leaves each of them 2^970 or more), where halving them is exact
      if (inf1 && inf2 && (s1 > 0.0) == (s2 > 0.0)) {
        return difference_at_least(0.5 * a1, 0.5 * b1, 0.5 * a2, 0.5 * b2);
      }
      return s1 >= s2;
    }
    if (s1 != s2) {
      return s1 > s2;
    }
    return e1 >= e2;
  }

  // cancel_minus() after its check, made before the differences of the bounds
  // are compared (see gaol/gaol_fpu.h)
  static interval cancel_minus_upward(const interval& X, const interval& Y)
  {
    const bool x_empty = X.is_empty(), y_empty = Y.is_empty();
    // no value at Level 1, which 12.12.5 has return [-oo, +oo]: an unbounded X
    // or Y, a nonempty X with an empty Y, or X narrower than Y
    if ((!x_empty && !X.is_finite()) || (!y_empty && !Y.is_finite())) {
      return interval::universe();
    }
    if (x_empty) {
      return interval::emptyset();
    }
    if (y_empty) {
      return interval::universe();
    }
    const double xl = X.left(), xr = X.right(), yl = Y.left(), yr = Y.right();
    if (!difference_at_least(xr, yr, xl, yl)) {
      return interval::universe();
    }
    // [xl - yl, xr - yr] rounded outward: xl - yl downward as -(yl - xl)
    const double lower = -(yl - xl);
    const double upper = xr - yr;
    return interval(lower, upper);
  }

  interval cancel_minus(const interval& X, const interval& Y)
  {
    GAOL_RND_ENTER();
    interval res = cancel_minus_upward(X, Y);
    GAOL_RND_KEEP(res);
    GAOL_RND_LEAVE();
    return res;
  }

  interval cancel_plus(const interval& X, const interval& Y)
  {
    return cancel_minus(X, -Y);
  }

  /*
    The error function and its complement (GAOL v5)

    erf(x) = 2/sqrt(pi) int_0^x exp(-t^2) dt and erfc(x) = 1 - erf(x), the
    erf and erfc of C, which IEEE 1788-2015 does not name, defined on R: erf
    is increasing, from its limit -1 at -oo to its limit 1 at +oo, and erfc
    decreasing, from 2 to 0. CORE-MATH computes both correctly rounded in the
    rounding direction in effect, so they are bounded as exp2 is: in the
    upward rounding GAOL keeps, the value at one bound is a bound, and the
    double below the value at the other bound the other one, unless that
    value is itself a double. erf(0) = 0 and erfc(0) = 1 are: without the
    test below, the lower bound of erf([0, 1]) would be -2^-1074, and that of
    erfc([-1, 0]) the double below 1. So are the limits at an infinite bound,
    erf(-oo) = -1, erf(+oo) = 1, erfc(-oo) = 2 and erfc(+oo) = 0, which
    CORE-MATH returns exactly, and which the range of each function would
    bring the lower bound back to as well. No other double is known where erf
    or erfc takes a value that is a double, and CORE-MATH's sources treat no
    such case: the bounds are the tightest ones.
  */
  // The points where erf and erfc take a value that is a double
  static inline bool erf_is_exact(double x)
  {
    return x == 0.0 || !is_finite(x);
  }

  interval erf(const interval& I)
  {
    // Within [-1, 1]: |erf(x)| < 1 at every finite x, and above 1 - 2^-53,
    // the double below 1, from |x| = 5.87 on, where CORE-MATH rounds erf(x)
    // upward to 1 and the lower bound is 1 - 2^-53
    return increasing_cr(I, gaol_cr_erf, erf_is_exact, -1.0, 1.0);
  }

  interval erfc(const interval& I)
  {
    if (I.is_empty()) {
      return interval::emptyset();
    }
    // The check before the bounds are read, and the result made before
    // GAOL_RND_LEAVE(), as acospi() does (see gaol/gaol_fpu.h): erfc(x) is
    // subnormal from x = 26.55 on, and with GAOL_PRESERVE_ROUNDING the
    // denormals-are-zero mode it restores would compare such bounds as 0
    GAOL_RND_ENTER();
    const double l = I.left(), r = I.right();
    // Decreasing: the upper bound at the left bound, the lower one at the
    // right, within [0, 2]. erfc(x) is below 2^-1074 from x = 27.22 on, where
    // CORE-MATH rounds it upward to 2^-1074 and the lower bound is 0
    const double v = gaol_cr_erfc(l);
    const double w = gaol_cr_erfc(r);
    const double u = erf_is_exact(r) ? w : previous_float(w);
    interval res(maximum(0.0, u), minimum(2.0, v));
    GAOL_RND_KEEP(res);
    GAOL_RND_LEAVE();
    return res;
  }

  /*
   * abs
   */
  interval abs(const interval& I)
  {
    if (I.is_empty()) {
      return I;
    }

    if (I.certainly_positive()) {
      return I;
    }
    if (I.certainly_negative()) {
      return interval(-I.right(),-I.left());
    }
    return interval(0.0,maximum(-I.left(),I.right()));
  }


  double chi(const interval &I)
  {
    // NaN for the empty set, as width() and mig() give (GAOL v5): the
    // quotient of its NaN bounds had the sign the generated code left it,
    // -nan with GCC on x86 and nan with Clang
    if (I.is_empty()) {
      return GAOL_NAN;
    }
    if (I.is_zero()) {
      return -1.0;
    } else {
      if (!I.is_finite()) {
        if (I.set_eq(interval::universe())) {
          return 1.0;
        } else {
          return 0.0;
        }
      } else {
	double res;
	// The check first, which clears the modes that flush the subnormals to
	// zero, as in midpoint(): under denormals-are-zero, the quotient of two
	// subnormal bounds is 0/0 (GAOL v5). Before,
	// is_zero() took them for zeros, and chi([2, 4]*2^-1074) was -1
	GAOL_RND_PRESERVE();
	round_upward_if_needed();
	round_nearest();
	// A quiet comparison: the empty set, whose bounds are NaN, gives NaN
	// without the invalid-operation exception (GAOL v5)
	if (gaol_detail::quiet_less_equal(std::fabs(I.left()), std::fabs(I.right()))) {
	  res = I.left() / I.right();
	} else {
	  res = I.right() / I.left();
	}
	// Computed rounding to nearest: kept before the direction changes (see gaol_fpu.h)
	res = gaol_core::rnd_keep(res);
	GAOL_RND_RESTORE();
	return res;
      }
    }
  }

  bool interval::is_finite(void) const
  {
    return !std::isinf(left()) && !std::isinf(right());
  }

  /*
    max() and min() of two intervals: the intervals of the larger and of the
    smaller bounds. maximum() and minimum() give NaN bounds for an empty I or
    J, which give the empty set: the constructor tells them with a quiet
    comparison (GAOL v5). They were told first, with one more comparison, when
    the constructor compared them with <=, which raises the invalid-operation
    exception on a NaN; testing I and J first made max() 5 to 10% slower
    (Clang 18).

    The SSE2 intervals do not call maximum() and minimum() for the common
    case, where the stored bounds <-l, r> of the two operands give both bounds
    of the result at once: the larger lower bound is the smaller of the two
    lanes <-l>, the larger upper bound the larger of the two lanes <r>. Two
    calls of maximum() and the constructor took 8.9 ns with GCC 15.2 and 5.5
    to 6.4 with Clang 21, the lanes taken by one comparison 2.3 and 2.0 (an
    i7-1185G7, gaol_performance). The lanes are exchanged by a mask of
    that comparison, not taken by minpd and maxpd, which signal the
    invalid-operation exception for the NaN of an empty operand and, under
    denormals-are-zero, give a zero for a subnormal lane. They are taken only
    where
      - no lane is a NaN: the empty set is given, without an exception. The NaN
        and the equal lanes are told by two comparisons and two movemasks, not
        one comparison of both: GCC merged cmpeq | cmpunord into the single
        comparison of an unordered or equal pair, which signals;
      - the lanes equal as doubles have the same bits, or are zeros of both
        signs: two lanes of the same bits, the lower bounds of [0, 1] and [0, 2]
        or max(x, x), give the same lane whichever is chosen; for +0 and -0 the
        lane is the one maximum() and minimum() give, +0 for the larger bound
        and -0 for the smaller (an OR or an AND of the sign bits). Two
        subnormals, or a subnormal and a zero, that denormals-are-zero makes
        equal are told apart by maximum() and minimum() by the bits
        (gaol_detail::bound_key()) and by the code below;
      - the lower bound l of the result is below its upper bound r, or l and r
        are the same finite double (a point result, max([-1, 0], [0, 0]) for
        ReLU): the constructor of two bounds takes anything else, a pair in
        the wrong order or with an infinite bound that no interval holds, for
        the empty set.
    The code of the other kinds of intervals takes the other cases, out of
    line and cold, and gives the same bounds bit for bit: it returns the
    stored bounds in a register, GCC adding a frame and a stack protector to
    the common case when the function returns an interval through its hidden
    pointer (see GAOL_RND_OUT_OF_LINE of gaol/gaol_fpu.h).
  */
#if GAOL_USING_SSE2_INSTRUCTIONS
  // The other cases, out of line: the stored bounds of the interval that the other kinds of intervals give
  static GAOL_NOINLINE_COLD __m128d max_of_bounds(const interval &I, const interval &J)
  {
    return upward_access::stored_bounds(interval(maximum(I.left(),J.left()), maximum(I.right(),J.right())));
  }

  static GAOL_NOINLINE_COLD __m128d min_of_bounds(const interval &I, const interval &J)
  {
    return upward_access::stored_bounds(interval(minimum(I.left(),J.left()), minimum(I.right(),J.right())));
  }

  //! All ones in the lanes where x and y are equal as doubles and have different bits
  GAOL_INLINE __m128d equal_lanes_of_different_bits(__m128d x, __m128d y)
  {
    const __m128i same_dwords = _mm_cmpeq_epi32(_mm_castpd_si128(x), _mm_castpd_si128(y));
    const __m128i same_bits = _mm_and_si128(same_dwords, _mm_shuffle_epi32(same_dwords, _MM_SHUFFLE(2, 3, 0, 1)));
    return _mm_andnot_pd(_mm_castsi128_pd(same_bits), _mm_cmpeq_pd(x, y));
  }

  //! All ones in the lanes where x and y are both zeros, of either sign
  GAOL_INLINE __m128d zero_lanes(__m128d x, __m128d y)
  {
    const __m128i magnitudes = _mm_castpd_si128(_mm_andnot_pd(_mm_set1_pd(-0.0), _mm_or_pd(x, y)));
    const __m128i zero_dwords = _mm_cmpeq_epi32(magnitudes, _mm_setzero_si128());
    return _mm_castsi128_pd(_mm_and_si128(zero_dwords, _mm_shuffle_epi32(zero_dwords, _MM_SHUFFLE(2, 3, 0, 1))));
  }

  /*
    The lanes of res where x and y are equal with different bits, for the
    stored bounds of max() (maximum_wanted) or of min(): zeros of different
    signs, -0 for the stored lane that is the bound -l of a larger lower bound
    l = +0 (and for the smaller upper bound), +0 for the other; false for
    other pairs (subnormals under denormals-are-zero), which the code below tells
  */
  GAOL_INLINE bool settle_ties(__m128d x, __m128d y, bool maximum_wanted, __m128d& res)
  {
    if (_mm_movemask_pd(_mm_cmpeq_pd(x, y)) == 0) {
      return true;
    }
    const __m128d different = equal_lanes_of_different_bits(x, y);
    if (_mm_movemask_pd(different) == 0) { // the same bits: either lane is the one
      return true;
    }
    if (_mm_movemask_pd(_mm_andnot_pd(zero_lanes(x, y), different)) != 0) {
      return false;
    }
    // max: <the OR of the lower lanes, the AND of the upper ones>, min: the other way round
    const __m128d signs_or = _mm_or_pd(x, y), signs_and = _mm_and_pd(x, y);
    const __m128d fix = maximum_wanted ? _mm_move_sd(signs_and, signs_or) : _mm_move_sd(signs_or, signs_and);
    res = _mm_or_pd(_mm_and_pd(different, fix), _mm_andnot_pd(different, res));
    return true;
  }

  /*
    The stored bounds <-l, r> are those of an interval that the constructor of
    two bounds would keep: l is below r, or l and r are the same finite double.
    False for a NaN, the stored bounds of the empty set
  */
  GAOL_INLINE bool valid_stored_bounds(__m128d stored)
  {
    const double l = -_mm_cvtsd_f64(stored), r = _mm_cvtsd_f64(_mm_unpackhi_pd(stored, stored));
    return gaol_detail::quiet_less(l, r)
           || (gaol_detail::bound_bits(l) == gaol_detail::bound_bits(r)
               && gaol_detail::quiet_less(l, GAOL_INFINITY) && gaol_detail::quiet_less(-GAOL_INFINITY, l));
  }

  // reread_bounds() keeps the comparison behind the test of NaN where a compiler
  // would compute it first (Clang did with the test of NaN of mul_bounds() on NEON);
  // none of the compilers tried moves it here
  interval  max(const interval &I, const interval &J)
  {
    const __m128d x = upward_access::stored_bounds(I), y = upward_access::stored_bounds(J);
    __m128d res = _mm_set1_pd(std::numeric_limits<double>::quiet_NaN()); // the empty set, for an empty operand
    if (_mm_movemask_pd(_mm_cmpunord_pd(x, y)) == 0) {
      const __m128d xx = reread_bounds(x), yy = reread_bounds(y);
      const __m128d sel = _mm_and_pd(_mm_xor_pd(xx, yy), _mm_cmpgt_pd(xx, yy)); // the lanes where x is above y
      res = _mm_move_sd(_mm_xor_pd(yy, sel), _mm_xor_pd(xx, sel)); // <the smaller lane 0, the larger lane 1>
      if (!settle_ties(x, y, true, res)) {
        res = _mm_set1_pd(std::numeric_limits<double>::quiet_NaN()); // the bounds are told by the code below
      }
      if (!valid_stored_bounds(res)) {
        res = max_of_bounds(I, J);
      }
    }
    return interval(res, gaol_detail::unchecked_bounds);
  }

  interval  min(const interval &I, const interval &J)
  {
    const __m128d x = upward_access::stored_bounds(I), y = upward_access::stored_bounds(J);
    __m128d res = _mm_set1_pd(std::numeric_limits<double>::quiet_NaN());
    if (_mm_movemask_pd(_mm_cmpunord_pd(x, y)) == 0) {
      const __m128d xx = reread_bounds(x), yy = reread_bounds(y);
      const __m128d sel = _mm_and_pd(_mm_xor_pd(xx, yy), _mm_cmpgt_pd(xx, yy));
      res = _mm_move_sd(_mm_xor_pd(xx, sel), _mm_xor_pd(yy, sel)); // <the larger lane 0, the smaller lane 1>
      if (!settle_ties(x, y, false, res)) {
        res = _mm_set1_pd(std::numeric_limits<double>::quiet_NaN());
      }
      if (!valid_stored_bounds(res)) {
        res = min_of_bounds(I, J);
      }
    }
    return interval(res, gaol_detail::unchecked_bounds);
  }
#elif GAOL_USING_NEON_INSTRUCTIONS
  /*
    The same with the NEON intervals, told by integer instructions on the
    four stored words <-l, r> of the operands (GAOL v5). The comparisons of
    64-bit ARM on doubles signal the invalid-operation exception on a NaN, and
    the modes that flush the subnormals to zero change them: the bounds are
    compared as signed integers, the keys of order_key(), which order the
    doubles that are no NaN as maximum() and minimum() do, -0 below +0 and the
    subnormals by their values. The lane <-l> of the larger lower bound is
    the lane of the smaller key, so that the key of l is the complement of the
    key of the lane, and the larger lower bound and the larger upper bound
    are chosen the same way.

    The common case is told first, by the words alone: none is a NaN or a
    stored -oo, which no interval holds (told on the words rotated by one bit,
    rotated_word()), and l is below r in each operand by the keys (the same
    double, or -0 below +0, is). The bounds of max() and min() are then the
    lanes of the larger, or of the smaller, keys, one comparison for each, and
    the constructor of two bounds would keep them: l is below r in the result
    as it is in the operands, and its words are among theirs. Nothing else is
    told, and no floating-point instruction runs. Counted under qemu (Clang 18
    to 21, GCC 12 and 14), the common case is 33 to 36 instructions, at most 2
    of them on the vector registers (the move of the two words to a vector
    register, when they are not stored as a pair), where the lanes compared as
    vectors and told by the constructor of two bounds were 41 to 43, 28 or 29
    of them on the vector registers (47 to 50 when the bounds of the result
    are equal, the constructor then comparing their bits). The bounds are
    chosen by conditional selects, not by branches.

    The empty set comes next, a NaN in a single word too. A NaN in the first
    words <-l> of the operands, as the empty set has in both words, is told by
    the first comparison of the common case, before the keys are computed: the
    larger of the two rotated words against the rotated -oo, below which are
    the stored bounds, equal to which is a stored -oo, and above which are the
    NaN. The code of the empty set
    follows that comparison, and the common case jumps over it (the lanes
    tested as vectors did the same, and GAOL_MAXMIN_FIRST() tells the
    compilers to keep this order). An empty operand, whichever of the two it
    is, then takes 15 instructions under qemu (Clang 18 to 21, GCC 12 and 14),
    one at most on the vector registers (the store of the empty set), and no
    taken branch, where the lanes tested as vectors took 16, 9 of them on the
    vector registers, and 19 with GCC. A second comparison tells a NaN from a
    stored -oo, and a NaN in the second words <r> from the other cases. These,
    bounds in the wrong order, a stored -oo, the zeros of different signs as l
    and r ([+0, -0], which the constructor of two bounds takes), are
    max_min_other_cases(), out of line and cold: the lanes compared as vectors
    and the constructor, as before, so that the bits are the same.
  */
#  ifndef GAOL_NOINLINE_COLD
#    if defined(__GNUC__) || defined(__clang__)
#      define GAOL_NOINLINE_COLD __attribute__((noinline, cold))
#    elif defined(_MSC_VER)
#      define GAOL_NOINLINE_COLD __declspec(noinline)
#    else
#      define GAOL_NOINLINE_COLD
#    endif
#  endif

#  if defined(__GNUC__) || defined(__clang__)
#    define GAOL_MAXMIN_FIRST(condition) __builtin_expect(!!(condition), 1)
#  else
#    define GAOL_MAXMIN_FIRST(condition) (condition)
#  endif

#  if defined(__GNUC__) || defined(__clang__)
#    define GAOL_MAXMIN_INLINE __attribute__((always_inline)) inline
#  elif defined(_MSC_VER)
#    define GAOL_MAXMIN_INLINE __forceinline
#  else
#    define GAOL_MAXMIN_INLINE inline
#  endif

  namespace neon_detail {

  // The word rotated left by one bit, the sign last. For the stored bound of an interval, no NaN and no
  // -oo, it is at most the +oo rotated (0xffe0000000000000), whatever the sign; the stored -oo rotated is
  // 0xffe0000000000001, and a NaN of either sign is above it
  GAOL_INLINE std::uint64_t rotated_word(std::uint64_t w)
  {
    return (w << 1) | (w >> 63);
  }

  // The larger of the rotated words
  GAOL_INLINE std::uint64_t larger_rotated(std::uint64_t u, std::uint64_t v)
  {
    const std::uint64_t ru = rotated_word(u), rv = rotated_word(v);
    return ru > rv ? ru : rv;
  }

  // The word in the register it is in, at no cost: an empty asm statement that the compilers do not
  // move a computation above (as reread_bounds() of gaol_interval_neon_kernels.h; Visual C++ has none on ARM64)
  GAOL_INLINE std::uint64_t keep_word(std::uint64_t w)
  {
#  if !GAOL_NEON_MSVC
    __asm__ __volatile__ ("" : "+r" (w));
#  endif
    return w;
  }

  // The order of the doubles that are no NaN as signed integers, -0 below +0
  GAOL_INLINE std::int64_t order_key(std::uint64_t w)
  {
    return static_cast<std::int64_t>(w ^ (static_cast<std::uint64_t>(static_cast<std::int64_t>(w) >> 63) >> 1));
  }

  // The order of the doubles that are no NaN as signed integers in both lanes, -0 below +0
  GAOL_INLINE int64x2_t zero_apart_keys(float64x2_t x)
  {
    const int64x2_t bits = vreinterpretq_s64_f64(x);
    return veorq_s64(bits, vreinterpretq_s64_u64(vshrq_n_u64(vreinterpretq_u64_s64(vshrq_n_s64(bits, 63)), 1)));
  }

  // The stored bounds of max(), or of min(), for the stored bounds x and y of
  // two intervals that are not empty
  GAOL_INLINE float64x2_t max_min_stored(float64x2_t x, float64x2_t y, bool maximum_wanted)
  {
    const uint64x2_t above = vcgtq_s64(zero_apart_keys(x), zero_apart_keys(y)); // x above y, lane by lane
    const float64x2_t where_above = vcombine_f64(vget_low_f64(y), vget_high_f64(x));  // <y0, x1>
    const float64x2_t where_not = vcombine_f64(vget_low_f64(x), vget_high_f64(y));    // <x0, y1>
    return maximum_wanted ? vbslq_f64(above, where_above, where_not) : vbslq_f64(above, where_not, where_above);
  }

  // The stored bounds as two words
  GAOL_INLINE void stored_words(const interval &I, std::uint64_t (&w)[2])
  {
    std::memcpy(w, &I.get_neoninterval(), sizeof w);
  }

  } // namespace neon_detail

  // The cases that the words do not tell, out of line: the stored bounds that the lanes compared as
  // vectors and the constructor of two bounds give (the code that max() and min() were, GAOL v5)
  static GAOL_NOINLINE_COLD float64x2_t max_min_other_cases(float64x2_t x, float64x2_t y, bool maximum_wanted)
  {
    using namespace neon_detail;
    if (any_lane(vorrq_u64(nan_lanes(x), nan_lanes(y)))) { // an empty operand
      return nan_pair();
    }
    const float64x2_t res = max_min_stored(x, y, maximum_wanted);
    return upward_access::stored_bounds(interval(-vgetq_lane_f64(res, 0), vgetq_lane_f64(res, 1)));
  }

  template<bool maximum_wanted> GAOL_MAXMIN_INLINE interval max_min_words(const interval &I, const interval &J)
  {
    using namespace neon_detail;
    std::uint64_t a[2], b[2];
    stored_words(I, a);
    stored_words(J, b);
    // The rotated stored -oo: the rotated words of the stored bounds are below it, those of the NaN above it. It is
    // kept in its register: the compilers would compare with the rotated +oo instead, a second constant
    const std::uint64_t minus_infinity = keep_word(0xffe0000000000001ull);
    const std::uint64_t larger0 = larger_rotated(a[0], b[0]);
    // GAOL_MAXMIN_FIRST() is where the compilers put the code, not what the inputs are: the code that follows this
    // test, the empty set, stays before the common case, so that the empty operand takes no branch
    if (!GAOL_MAXMIN_FIRST(larger0 >= minus_infinity)) { // the first words, <-l> of the operands, are stored bounds
      // Lane 0 holds -l: the key of l is the complement of the key of the lane. It is kept as such: the compilers
      // turn the comparisons of the complements into those of the keys of the lanes, and compute them a second time
      const std::int64_t la = static_cast<std::int64_t>(keep_word(static_cast<std::uint64_t>(~order_key(a[0])))),
                         lb = static_cast<std::int64_t>(keep_word(static_cast<std::uint64_t>(~order_key(b[0])))),
                         ra = order_key(a[1]), rb = order_key(b[1]);
      if (!GAOL_UNLIKELY(larger_rotated(a[1], b[1]) >= minus_infinity || la > ra || lb > rb)) {
        const std::uint64_t first = (maximum_wanted ? la > lb : la < lb) ? a[0] : b[0];
        const std::uint64_t second = (maximum_wanted ? ra > rb : ra < rb) ? a[1] : b[1];
        // The two words are written into the result, not moved to the lanes of a vector register that is stored
        interval res = interval::emptyset();
        const std::uint64_t words[2] = { first, second };
        std::memcpy(&res.get_neoninterval(), words, sizeof words);
        return res;
      }
    }
    // Not the common case. The empty set, a NaN in a single word too, comes first: a NaN in the first words was
    // told by the test of the common case, and a second one, on the second words, tells the others
    if (GAOL_UNLIKELY(larger0 <= minus_infinity)) {
      if (GAOL_UNLIKELY(larger_rotated(a[1], b[1]) <= minus_infinity)) { // no NaN: the other cases
        return interval(max_min_other_cases(upward_access::stored_bounds(I), upward_access::stored_bounds(J), maximum_wanted),
                        gaol_detail::unchecked_bounds);
      }
    }
    return interval::emptyset();
  }

  interval  max(const interval &I, const interval &J)
  {
    return max_min_words<true>(I, J);
  }

  interval  min(const interval &I, const interval &J)
  {
    return max_min_words<false>(I, J);
  }
#else
  interval  max(const interval &I, const interval &J)
  {
    return interval(maximum(I.left(),J.left()), maximum(I.right(),J.right()));
  }

  interval  min(const interval &I, const interval &J)
  {
    return interval(minimum(I.left(),J.left()), minimum(I.right(),J.right()));
  }
#endif

  /*
    gaol_roundeven() reads the bits of its argument: it is exact whatever the
    rounding direction, and this function changes none, as the inline
    round_ties_to_away() beside it in gaol_interval_core.h. The rounding to
    the nearest integer being non-decreasing, the bounds of the result are
    those of I rounded.
  */
  interval round_ties_to_even(const interval& I)
  {
    if (I.is_empty()) {
      return interval::emptyset();
    }
    return interval(gaol_roundeven(I.left()), gaol_roundeven(I.right()));
  }


  double interval::smig(void) const
  {
    if (is_empty()) {
      return GAOL_NAN;
    } else {
      if (set_contains(0)) {
	return 0.0;
      }
      // Compared as in the relations (gaol_port.h), whatever the modes that
      // flush the subnormals to zero (GAOL v5)
      if (gaol_detail::bound_less(right(), 0.0)) {
	return right();
      } else { // left() > 0.0
	return left();
      }
    }
  }

  // The bodies are mig_of() and mag_of() of gaol_interval_internal.h, which
  // hypot() takes inline too (GAOL v5)
  double interval::mig(void) const
  {
    return mig_of(*this);
  }

  double interval::mag(void) const
  {
    return mag_of(*this);
  }




  double interval::midpoint() const
  {
    if (is_empty()) {
      return NAN;
    }
	 if (is_symmetric()) {
		return 0.0;
	 }
    if (left() == -GAOL_INFINITY) {
      return -std::numeric_limits<double>::max();
    }
    if (right() == GAOL_INFINITY) {
      return std::numeric_limits<double>::max();
    }

    /*
      The half of the sum of the bounds, rounded to nearest, or the sum of
      their halves where the sum may overflow, that is where a bound is 2^1023
      or more in magnitude, which is tested first (GAOL v5): the sum was
      computed first, and its overflow told the other case, so that
      midpoint([DBL_MAX]) raised the overflow exception, though its midpoint
      is DBL_MAX. The sum of the halves gives the same midpoint there: a half
      is exact, unless the bound is below 2^-1021, where its rounding changes
      no sum with a bound of 2^1023 or more, and a sum of halves is the half
      of the sum, rounded, where no half is rounded.
      The check of the rounding direction first, which clears the modes that
      flush the subnormals to zero, and which GAOL_RND_RESTORE() sets back
      with GAOL_PRESERVE_ROUNDING: under denormals-are-zero, the midpoint of
      [2, 4]*2^-1074 was 0, outside the interval, and split() made an empty
      half of it (GAOL v5). The comparisons above are
      those of the relations, which these modes do not change.
    */
    GAOL_RND_PRESERVE();
    round_upward_if_needed();
    round_nearest();
    const double l = left(), r = right();
    double middle;
    const double big = 8.9884656743115795e+307; // 2^1023 (C++11 has no hexadecimal floating literal)
    if (std::fabs(l) >= big || std::fabs(r) >= big) {
      middle = 0.5*l + 0.5*r;
    } else {
      middle = 0.5*(l + r);
    }
    // Computed rounding to nearest: kept before the direction changes (see gaol_fpu.h)
    middle = gaol_core::rnd_keep(middle);
    GAOL_RND_RESTORE();
    return middle;
  }

  /*
    The cut point c of bisect(ratio) (GAOL v5). Any double of [l, r] gives two
    parts that cover [l, r] exactly, sharing c: c is computed in GAOL's
    rounding direction and put back within [l, r], where the two rounded
    products may have taken it; l (1 - ratio) + r ratio cannot overflow where
    l + ratio (r - l) does, as for [-MAX, MAX], and for a bound of 2^1023 or
    more the cut point is that of the halves of the bounds, doubled, so that
    no overflow exception is raised for two finite bounds either. GAOL_RND_ENTER() clears the
    flush-to-zero and denormals-are-zero modes, so that the cut between two
    subnormal bounds is not read as 0. When a double lies strictly between l
    and r, c is moved to the nearest one where it fell on a bound, so that
    both parts are smaller than [l, r]. ratio = 0.5 and the unbounded
    intervals are cut at midpoint(), as split() cuts them, midpoint() being
    strictly between the bounds of a bisectable interval: rounded to nearest,
    it would be l only for an exact midpoint at most half the distance from
    l to the next double. The ratio is compared quietly, a NaN raising no
    invalid-operation exception before it is refused, by the comparisons of
    gaol_detail, ucomisd in line with Visual C++, where std::isgreater() and
    std::isless() are calls of _dpcomp() (GAOL v5), and by
    the bits where the quiet comparison finds two doubles equal: a subnormal
    ratio is no 0 for the modes that flush the subnormals to zero, that the
    program may have set. The cut point is put within [l, r] before the
    direction is restored, by the bits as well (GAOL v5).
  */
  // The double next to x on the side of y, x and y finite and different, from the bits of x:
  // nextafter() of the C library may compare doubles, which the modes that flush the
  // subnormals to zero change (GAOL v5)
  static double bisect_step_toward(double x, double y)
  {
    std::uint64_t bits = gaol_detail::bound_bits(x);
    const bool up = gaol_detail::bound_key(y) > gaol_detail::bound_key(x);
    if ((bits << 1) == 0u) { // 0, of either sign: the least subnormal on the side of y
      bits = up ? 1u : 0x8000000000000001ull;
    } else if (up != ((bits >> 63) != 0u)) { // away from zero
      ++bits;
    } else {
      --bits;
    }
    double next;
    std::memcpy(&next, &bits, sizeof next);
    return next;
  }

  std::pair<interval, interval> interval::bisect(double ratio) const
  {
    if (!(gaol_detail::bound_greater(ratio, 0.0) && gaol_detail::bound_less(ratio, 1.0))) {
      gaol_ERROR(std::domain_error, "bisect(): the ratio is not in (0, 1)");
    }
    if (is_empty()) {
      return std::make_pair(*this, *this);
    }
    const double l = left(), r = right();
    double c;
    if (ratio == 0.5 || std::isinf(l) || std::isinf(r)) {
      c = midpoint();
    } else {
      GAOL_RND_ENTER();
      // l, r and ratio are registers: they go through rnd_reread() after the check,
      // or GCC computes c before it writes the control register, in the
      // program's direction (GAOL v5)
      const double big = 8.9884656743115795e+307; // 2^1023 (C++11 has no hexadecimal floating literal)
      if (std::fabs(l) < big && std::fabs(r) < big) {
        c = rnd_reread(l)*(1.0 - rnd_reread(ratio)) + rnd_reread(r)*rnd_reread(ratio);
      } else {
        // For a bound of 2^1023 or more, the sum of the two products, rounded upward,
        // can exceed the largest double by an ulp, as for [prev(MAX), MAX]: the
        // overflow exception would be raised for two finite bounds. The cut point is
        // that of the halves of the bounds, put between them and doubled, which is
        // exact there (GAOL v5)
        const double lh = 0.5*rnd_reread(l), rh = 0.5*rnd_reread(r);
        double h = lh*(1.0 - rnd_reread(ratio)) + rh*rnd_reread(ratio);
        if (gaol_detail::bound_less(h, lh)) {
          h = lh;
        } else if (gaol_detail::bound_greater(h, rh)) {
          h = rh;
        }
        c = h + h;
      }
      GAOL_RND_KEEP(c);
      // Within [l, r], and strictly inside when the interval is bisectable, before the
      // direction is restored: with GAOL_PRESERVE_ROUNDING the modes that flush the
      // subnormals to zero come back with it, and c < l and c == l read two subnormal
      // bounds as zeros. The comparisons and the step to the next double are those of the
      // bits, whatever the modes
      if (gaol_detail::bound_less(c, l)) {
        c = l;
      } else if (gaol_detail::bound_greater(c, r)) {
        c = r;
      }
      if (is_bisectable()) {
        if (gaol_detail::bound_equal(c, l)) {
          c = bisect_step_toward(l, r);
        } else if (gaol_detail::bound_equal(c, r)) {
          c = bisect_step_toward(r, l);
        }
      }
      // The parts are made in the same way, before the direction is restored. Made after
      // it, with denormals-are-zero back, [c, r] of two subnormals came out as the empty
      // set with GCC 15 in a first version of this function; probably the comparison
      // c <= r of the constructor of [c, r] met a comparison of c and r made before the
      // modes changed (bound_equal(c, r) above), which the compiler does not know can
      // differ (not verified)
      interval first(l, c), second(c, r);
      GAOL_RND_KEEP(first);
      GAOL_RND_KEEP(second);
      GAOL_RND_LEAVE();
      return std::make_pair(first, second);
    }
    return std::make_pair(interval(l, c), interval(c, r));
  }

  // The body is sqrt_checked() of gaol_interval_internal.h, which nth_root(I, q)
  // for q = -2 takes inline too (GAOL v5)
  interval sqrt(const interval& I)
  {
    return sqrt_checked(I);
  }

} // namespace gaol_core

/*
  The intervals of floats, gaol::intervalf and gaol::interval2f: unfinished,
  and compiled only where a developer of GAOL defines GAOL_FLOAT_INTERVALS
  (see gaol/gaol_config.h); none of the three builds has an option for them
  (GAOL v5)
*/
#ifdef GAOL_FLOAT_INTERVALS
#  include "gaol/float/gaol_intervalf_impl.h"
#  if GAOL_USING_SSE3_INSTRUCTIONS
#    include "gaol/float/gaol_interval2f_impl.h"
#  endif
#endif // GAOL_FLOAT_INTERVALS
