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
  \file   gaol_interval_pow.cpp
  \brief  The powers and the roots of the intervals: gaol_core

  The integer powers (gaol_pown(), gaol_uipow(), gaol_pown_ll(), whose kernel
  is in gaol_interval_core.cpp), pow, the n-th roots, the cube root, rsqrt,
  hypot and compoundm1 (GAOL v5: a part of gaol_interval_core.cpp, see
  gaol_interval_internal.h).

  \author Goualard Frederic, NININ Jordan
  \date   2001-09-28
*/

#include "gaol/core/gaol_interval_internal.h"
// x * 2^n rounded once, for the n-th roots (compare_scaled(), GAOL v5)
#include "gaol/math/gaol_ldexp.h"
// (1 + x)^y - 1 at a point, with the tightest bounds (GAOL v5)
#include "gaol/math/gaol_compoundm1.h"

namespace gaol_core {

  // x > 0 finite as m 2^e, m an odd integer, read from the bits of x (in
  // pow_is_double() and hypot()): std::frexp(), std::ldexp() and a loop over
  // the trailing zeros cost as much as CORE-MATH's pow. The lowest set bit of
  // the significand is a power of two below 2^53, a double whose exponent is
  // the number of trailing zeros
  static inline void odd_significand(double x, std::uint64_t& m, int& e)
  {
    std::uint64_t bits;
    std::memcpy(&bits, &x, sizeof bits);
    const int field = static_cast<int>(bits >> 52); // no sign bit; 0 for a subnormal
    m = bits & ((static_cast<std::uint64_t>(1) << 52) - 1);
    e = -1074;
    if (field != 0) {
      m |= static_cast<std::uint64_t>(1) << 52;
      e = field - 1075;
    }
    const double lowest = static_cast<double>(m & (~m + 1)); // exact
    std::memcpy(&bits, &lowest, sizeof bits);
    const int zeros = static_cast<int>(bits >> 52) - 1023;
    m >>= zeros;
    e += zeros;
  }

  /*
    Whether x^y is a double, and that double d, for a finite x > 0 other than
    1 and a finite y other than 0 (GAOL v5). pow_lo() takes d as the lower
    bound, where the double below CORE-MATH's value rounded upward is one
    double below it: pow([4], 0.5) was [2 - 2^-52, 2]. Calling CORE-MATH
    downward too would tell, at twice the cost of every power: this proves it
    instead, with integers and with operations on doubles that are exact
    whatever the rounding direction, and leaves at the first tests for nearly
    every y. y is a/2^k, a an odd integer and k >= 1, or an integer a (k = 0),
    and x is m 2^e, m an odd integer.
    - m = 1: x^y = 2^(e y), a double where e y is an integer of [-1074, 1023],
      and irrational elsewhere. As |e| <= 1074 < 2^11, e y is an integer only
      where 2^k divides e, k <= 10: 1024 y is an integer j, and e j a multiple
      of 1024. The product e*y of doubles could not tell: for e = 3 and y the
      double nearest 1/3, 3 y = 1 - 2^-54 rounds to 1.
    - m >= 3: x^y is rational only where x is the 2^k-th power of a rational
      z (a and 2^k being coprime), which is c 2^f with c odd, x being dyadic:
      m = c^(2^k), e = f 2^k, and x^y = z^a = c^a 2^(f a). That is no double
      for a < 0 (c >= 3), and a double for a > 0 where c^a < 2^53,
      f a >= -1074 and c^a 2^(f a) < 2^1024. c >= 3 gives a <= 33
      (3^33 < 2^53 < 3^34) and 2^k <= 33 (3^(2^k) <= m < 2^53): y is a multiple
      of 1/32 in (0, 33]. Then 2^k has to divide e, and m to be a 2^k-th
      power: the square root of a double is exact at a perfect square below
      2^53, and r*r is m for no integer r otherwise. c^a is a product of
      doubles, exact below 2^53, and at least 2^53 above it, 2^53 being a
      double.
  */
  bool pow_is_double(double x, double y, double& d)
  {
    if (!(y >= -1075.0 && y <= 1075.0)) {
      return false; // |e y| > 1074 for |e| >= 1
    }
    const double scaled = 1024.0*y; // exact, unless flush-to-zero makes it 0 (|y| < 2^-1032)
    const int j = static_cast<int>(scaled);
    if (j == 0 || static_cast<double>(j) != scaled) {
      return false; // k > 10, j being 0 for y != 0 only where 1024 y was flushed to 0
    }
    std::uint64_t m;
    int e;
    odd_significand(x, m, e);
    if (m == 1) {
      const int t = e*j; // 1024 e y, below 1.2e9 in magnitude
      if (t % 1024 != 0 || t < -1074*1024 || t > 1023*1024) {
        return false;
      }
      d = std::ldexp(1.0, t/1024);
      return true;
    }
    if (j <= 0 || j > 33*1024 || (j >= 2048 && m >= (static_cast<std::uint64_t>(1) << 27))) {
      return false; // c^a = m^y >= 2^54 for y >= 2 and m >= 2^27, as for most x
    }
    // y = a/2^k: j = a 2^(10-k), whose lowest set bit is 2^(10-k) for k > 0
    const double lowest = static_cast<double>(j & -j); // exact
    std::uint64_t bits;
    std::memcpy(&bits, &lowest, sizeof bits);
    const int zeros = static_cast<int>(bits >> 52) - 1023;
    const int k = (zeros >= 10) ? 0 : 10 - zeros, a = j >> (10 - k);
    if (k > 5 || (static_cast<unsigned int>(e) & ((1u << k) - 1u)) != 0u) {
      return false; // 2^k does not divide e
    }
    for (int i = 0; i < k; ++i) {
      const std::uint64_t r = static_cast<std::uint64_t>(std::sqrt(static_cast<double>(m)));
      if (r*r != m) {
        return false;
      }
      m = r;
    }
    const double c = static_cast<double>(m);
    double p = c; // c^a
    for (int i = 1; i < a; ++i) {
      p *= c;
      if (!(p < 9007199254740992.0)) { // 2^53
        return false;
      }
    }
    std::memcpy(&bits, &p, sizeof bits);
    const int b = static_cast<int>(bits >> 52) - 1022; // 2^(b-1) <= c^a < 2^b
    const int s = e/(1 << k)*a; // f a
    if (s < -1074 || s + b > 1024) {
      return false;
    }
    d = std::ldexp(p, s); // exact
    return true;
  }

  /*
    x^n for an integer n beyond the ints (GAOL v5, point 74 of TODO.md):
    the bounds of c^n at the corners of pown_corners(), from CORE-MATH's pow
    (pow_lo(), pow_hi()), the tightest, where n is a double, |n| <= 2^53 and
    beyond as a double; the sign from the parity of n. An integer n of more
    than 53 bits that is no double is nh + nl, nh being n with the bits of
    |n| below its 53 highest cleared, a double, and nl the rest, below 2^11:
    c^n is c^nh c^nl, rounded outward, within three doubles of the tightest
    bounds.
    gaol::pow(x, n) gave [-oo, +oo] beyond the ints and the unsigned ints,
    whereas x^n is finite and not trivial for x near 1: pow([1 + 2^-30],
    2^33), about e^8, is [2980.9579759367944, 2980.9579759367948]. Beyond
    2^53, only a base within 2^-43 of 1 or -1 has a power that is neither
    below the least subnormal nor above the largest double, the others being
    rounded to [0, 2^-1074] or [DBL_MAX, +oo], with their signs.
  */
  static void pown_point_beyond(double c, double nh, double nl, bool odd_n, double& lo, double& hi)
  {
    const double g = std::fabs(c);
    if (g == 0.0 || g == GAOL_INFINITY) {
      lo = hi = ((g == 0.0) == (nh < 0.0)) ? GAOL_INFINITY : 0.0;
    } else {
      lo = pow_lo(g, nh);
      hi = pow_hi(g, nh);
      if (nl != 0.0) {
        // c^nh and c^nl are on the same side of 1: the product is no 0 oo
        lo = -((-lo)*pow_lo(g, nl));
        hi = hi*pow_hi(g, nl);
      }
    }
    if (std::signbit(c) && odd_n) {
      const double t = lo;
      lo = -hi;
      hi = -t;
    }
  }

  static interval pown_beyond_upward(const interval& I, double nh, double nl, bool odd_n)
  {
    double c0, c1, lo, hi, other;
    if (I.is_empty()) {
      return I;
    }
    if (!pown_corners(I.left(), I.right(), nh < 0.0, odd_n, c0, c1)) {
      return interval::emptyset();
    }
    pown_point_beyond(c0, nh, nl, odd_n, lo, other);
    pown_point_beyond(c1, nh, nl, odd_n, other, hi);
    return interval(lo, hi);
  }

  // A degenerate exponent [d], d an integer beyond the ints: odd only below
  // 2^53, where the doubles are all even
  static interval pown_double_upward(const interval& I, double d)
  {
    const bool odd_d = std::fabs(d) < 9007199254740992.0 && std::fmod(d, 2.0) != 0.0;
    return pown_beyond_upward(I, d, 0.0, odd_d);
  }

  /*
    I^n for an int n. n = 2 is sqr(), which makes its own check of the
    rounding direction, and is taken before any other (GAOL v5): the integer
    power took four times the time of sqr() for the same bounds (doc/simd.md),
    and the sign of a zero bound is now that of sqr(). The other forms of the
    integer power take pown_upward() after their own check, which has
    upward_access::sqr() for n = 2. The rest is a function of its own, which is not
    inlined here: with it, the frame that its kernel needs was set up before
    the test of n = 2, which took 3.6 ns for x^2 against 1.7 (an Apple M2,
    NEON intervals).
  */
  interval gaol_pown(const interval& I, int n)
  {
    if (n == 2) {
      // sqr(I), inline (gaol_interval_internal.h)
      return upward_access::checked_sqr(I);
    }
    return pown_checked(I, n);
  }

  // gaol_pown() within the ints, CORE-MATH's pow beyond them (GAOL v5)
  interval gaol_uipow(const interval& I, unsigned int e)
  {
    if (e <= static_cast<unsigned int>(INT_MAX)) {
      return gaol_pown(I, static_cast<int>(e));
    }
    GAOL_RND_ENTER();
    interval res = pown_beyond_upward(I, static_cast<double>(e), 0.0, (e & 1u) != 0u);
    GAOL_RND_KEEP(res);
    GAOL_RND_LEAVE();
    return res;
  }

  interval gaol_pown_ll(const interval& I, long long n)
  {
    // The empty set returns before the check, as sqr() does (GAOL v5): the
    // powers of an n beyond the unsigned ints come here, those of the others
    // go to gaol_pown() and gaol_uipow()
    if (I.is_empty()) {
      return interval::emptyset();
    }
    GAOL_RND_ENTER();
    interval res;
    if (n >= INT_MIN && n <= INT_MAX) {
      res = pown_upward(I, static_cast<int>(n));
    } else {
      // |n| without a signed negation, which overflows at LLONG_MIN
      const unsigned long long magnitude = (n < 0) ? 0ull - static_cast<unsigned long long>(n) : static_cast<unsigned long long>(n);
      // high: |n| with the bits below its 53 highest cleared, a double, and
      // nl = 0 where n is one, 2^53 + 2 included
      int dropped = 0;
      for (unsigned long long v = magnitude >> 53; v != 0u; v >>= 1) {
        ++dropped;
      }
      const unsigned long long high = (magnitude >> dropped) << dropped;
      const double nh = static_cast<double>(high), nl = static_cast<double>(magnitude - high); // exact
      res = pown_beyond_upward(I, (n < 0) ? -nh : nh, (n < 0) ? -nl : nl, (magnitude & 1u) != 0u);
    }
    GAOL_RND_KEEP(res);
    GAOL_RND_LEAVE();
    return res;
  }

  /*
    The pow of IEEE 1788-2015 (Table 9.1) for an interval exponent, the
    rounding direction being upward (GAOL v5): x^y for the x of x in [0, +oo]
    and the y of y, 0^y being defined for y > 0 only. The base is cut to
    [0, +oo]; [0] gives [0] or the empty set; a degenerate integer exponent
    within the ints takes the integer power, x^n on x >= 0, where pow and
    pown agree; any other box takes the powers at its corners.
    gaol_ieee1788::pow(x, y) is this function after the check of
    pow_standard(), and gaol_pow_hybrid() calls it for every exponent but a
    degenerate integer, for which it takes pown on the whole of x.
  */
  static interval pow_standard_upward(const interval& x, const interval& y)
  {
    // An exponent [+oo] or [-oo] contains no real number, and gives the empty
    // set, as the empty one, as in pow_hybrid_upward() (GAOL v5:
    // gaol_ieee1788::pow([0.5], [+oo]) was [0])
    if (x.is_empty() || y.is_empty() || holds_no_real(y)) {
      return interval::emptyset();
    }

    /*
      x^y is only real for a negative x when y is an integer (GAOL v5,
      ported from the fix of Codac, commit 74086ccb, Jordan Ninin). GAOL
      computed the powers of the negative part of I on its magnitude, and
      pow([-4,-1],[0.5,0.5]) returned [-1,2] where sqrt([-4,-1]) is empty.
    */
    const interval base = x & interval::positive();
    if (base.is_empty()) {
      return interval::emptyset();
    }
    // pow(0,y) is 0 for y > 0, and has no value for y <= 0 (Table 9.1, footnote
    // c), which the limits at the corners do not give. interval(0.0) rather
    // than interval::zero(), whose lower bound is -0 with SSE2 intervals
    if (base.right() == 0.0) {
      return y.right() > 0.0 ? interval(0.0) : interval::emptyset();
    }

    /*
      A degenerate integer exponent [n] within the ints: x^n on x >= 0, where
      pow and pown agree, pown giving the powers that are doubles exactly, as
      pow_is_double() does at the corners below. Beyond the ints, n >= 2^31 or
      n < -2^31, [n] takes the corners below, as pown does there, one
      for each bound, x^n being monotone in x (GAOL v5: a block of its own
      took them, with the bounds the corners give since they take the
      limits): 1 at x = 1, the limit 0 or +oo at x = 0 (+0 or -0) and +oo,
      with no call of CORE-MATH, whose pow(+0, n) raised the divide-by-zero
      exception for n < 0 and pow(-0, n) is -oo for an odd n < 0. x^n is a
      double at 0 and 1 only: for another x it is beyond the doubles (2^(k n),
      for a power of two 2^k, k != 0) or not dyadic (1/m^|n| for n < 0) or
      above 2^53 (m^n, m >= 3 odd), so that the double below CORE-MATH's
      value is the tightest lower bound (pow_is_double() tells no such power
      a double). gaol_pow_hybrid() takes [n] before it comes here: pown on the
      whole of x (pown_double_upward() beyond the ints).
    */
    if (y.is_an_int()) {
      return pown_upward(base, static_cast<int>(y.left()));
    }

    /*
      The corners of the box (GAOL v5, issue #8): x^y increases with y for
      x > 1 and decreases for x < 1, increases with x for y > 0 and decreases
      for y < 0, so that its extrema over I x J are at corners, which the
      places of the bounds about 1 and 0 give: one corner for each bound, two
      where I holds 1 inside and J holds 0 inside. At a finite corner with
      x > 0, CORE-MATH's pow is correctly rounded in the upward rounding GAOL
      computes in, which gives the upper bound, and the double below it the
      lower one, unless the power is a double, which pow_is_double() proves
      and which is then the lower bound itself: the bounds are the tightest
      ones, where exp(J*log(I)) multiplied the relative width of log(I) by
      |y log(x)|: pow([2], [1023.5]) was 1425 doubles below and 748 above,
      and pow([4], 0.5) [2 - 2^-52, 2]. At a corner where x is 0 or +oo or y infinite, the
      bound is the limit of x^y there, 0 or +oo, or 1 where x = 1 or y = 0
      (pow_lo(), pow_hi()):
      pow([4, +oo], 0.5) is [2, +oo] and pow([0, 2^-1000], [-oo, -1])
      [2^1000, +oo], where exp(J*log(I)), which these boxes took, gave
      [2 - 2^-52, +oo] and a lower bound 617 doubles below 2^1000. The limits
      at x = 0 are those of the x > 0 of the domain where y <= 0, and 0^y = 0
      itself where y > 0.
    */
    const double xl = base.left(), xu = base.right(), yl = y.left(), yu = y.right();
    double l, r;
    if (yl == yu) {
      // A degenerate exponent, which [0] and the other integers within the
      // ints are not here: x^y increases with x for y > 0 and decreases for
      // y < 0, one corner for each bound whatever the base (GAOL v5:
      // gaol::pow(x, 0.75) took 109 ns rather than 102, and
      // gaol_ieee1788::pow(x, [1e10]) 72 rather than 68, the place of the
      // base about 1 being tested, with Clang 18 on an i7-1185G7)
      l = pow_lo((yl > 0.0) ? xl : xu, yl);
      r = pow_hi((yl > 0.0) ? xu : xl, yl);
    } else if (xl == 0.0 && yl > 0.0) {
      // A base from 0 and exponents above 0: 0^y = 0 is the least power, and
      // the greatest is at xu, with yu for xu >= 1 and yl below, one call of
      // CORE-MATH (GAOL v5: through the corners of a base holding 1, 48 ns
      // rather than 39 for gaol::pow([0, b], J), b from 1 to 10 and J within
      // [0.5, 2.5], with Clang 18 on an i7-1185G7)
      l = 0.0;
      r = pow_hi(xu, (xu >= 1.0) ? yu : yl);
    } else if (xl >= 1.0) {
      l = pow_lo((yl >= 0.0) ? xl : xu, yl);
      r = pow_hi((yu >= 0.0) ? xu : xl, yu);
    } else if (xu <= 1.0) {
      l = pow_lo((yu >= 0.0) ? xl : xu, yu);
      r = pow_hi((yl >= 0.0) ? xu : xl, yl);
    } else {
      // I holds 1 inside, where x^y is 1: x^y is at most 1 at (xl, y >= 0)
      // and (xu, y <= 0), the least at (xl, yu) or (xu, yl), and at least 1
      // at (xl, y <= 0) and (xu, y >= 0), the greatest at (xl, yl) or
      // (xu, yu). An exponent of the other sign is taken as 0, for which
      // pow_lo() and pow_hi() give 1 with no call of CORE-MATH: one call for
      // each bound where J has one sign, as for the other bases, two where J
      // holds 0 inside (GAOL v5: four; gaol::pow([1/b, b], J), b from 1 to 10
      // and J within [0.5, 2.5], took 134 ns, 137 with GAOL 4, and takes 84,
      // with Clang 18 on an i7-1185G7). Where J holds 0 inside and one of the
      // two corners is a limit, the other is not computed either: the limit
      // is 0 at (xl, yu) and (xu, yl), where x^y is below 1, which bounds the
      // box below, and +oo at (xl, yl) and (xu, yu), which bounds it above,
      // and pow([0, 2], [-1, 1]) takes no call (tested where J holds 0 inside
      // only: 90 ns rather than 84 for the boxes above, 81 without the test)
      const double yu_or_0 = (yu > 0.0) ? yu : 0.0, yl_or_0 = (yl < 0.0) ? yl : 0.0;
      const bool around_0 = yl < 0.0 && yu > 0.0;
      l = (around_0 && (pow_is_limit(xl, yu) || pow_is_limit(xu, yl)))
          ? 0.0 : minimum(pow_lo(xl, yu_or_0), pow_lo(xu, yl_or_0));
      r = (around_0 && (pow_is_limit(xl, yl) || pow_is_limit(xu, yu)))
          ? GAOL_INFINITY : maximum(pow_hi(xl, yl_or_0), pow_hi(xu, yu_or_0));
    }
    return interval(l,r);
  }

  /*
    pow_standard_upward() after one check of the rounding direction, made
    before the bounds are compared: with denormals-are-zero, the lower bound
    -5 2^-1074 of x stayed in x & [0, +oo], and pow([-5 2^-1074, 1], [0.5])
    was empty once the check of the corners had cleared the mode (GAOL v5, see
    gaol/gaol_fpu.h). Its integer powers and its corners do not check the
    direction again (pown_upward(), pow_lo(), pow_hi()).
  */
  interval pow_standard(const interval& x, const interval& y)
  {
    // An empty operand returns before the check, as sqr() does (GAOL v5)
    if (x.is_empty() || y.is_empty()) {
      return interval::emptyset();
    }
    GAOL_RND_ENTER();
    interval res = pow_standard_upward(x, y);
    GAOL_RND_KEEP(res);
    GAOL_RND_LEAVE();
    return res;
  }

  // gaol_pow_hybrid() after its check, made before the bounds of J are
  // compared: with denormals-are-zero, [2^-1074, 2^-1073] was the degenerate
  // integer exponent [0] (GAOL v5, see gaol/gaol_fpu.h)
  static interval pow_hybrid_upward(const interval &I, const interval &J)
  {
    if (I.is_empty() || J.is_empty()) {
      return interval::emptyset();
    }
    // [+oo] and [-oo] contain no real number: numsToInterval(l,u) of IEEE 1788
    // has no value for l = +oo or u = -oo (10.5.8). The constructors and the
    // operations make none, but get_xmminterval() of the SSE2 intervals and the
    // constructor of a register without a check do, as in tan() (GAOL v5: kept,
    // TODO.md B.1 having taken the test for one that is never true)
    if (J.left() == J.right() && !(std::fabs(J.left()) <= (std::numeric_limits<double>::max)())) {
      return interval::emptyset();
    }

    /*
      Hybrid semantics (GAOL v5): a degenerate integer exponent always
      takes the integer power, pown of IEEE 1788, which is defined for a
      negative base too, is 1 at p = 0 for any x, 0 included, and has no value
      at x = 0 for p < 0 (Table 9.1, footnote b), and any other exponent the
      pow of IEEE 1788, defined for x > 0, and for x = 0 when y > 0.
    */
    if (J.left() == J.right() && std::floor(J.left()) == J.left()) {
      if (J.is_an_int()) {
        return pown_upward(I,int(J.left()));
      }
      // An integer beyond the ints: the pown of CORE-MATH's pow (GAOL v5,
      // point 74 of TODO.md: [-oo, +oo] before)
      return pown_double_upward(I, J.left());
    }
    return pow_standard_upward(I, J);
  }

  interval gaol_pow_hybrid(const interval &I, const interval &J)
  {
    // An empty operand returns before the check, as sqr() does (GAOL v5)
    if (I.is_empty() || J.is_empty()) {
      return interval::emptyset();
    }
    GAOL_RND_ENTER();
    interval res = pow_hybrid_upward(I, J);
    GAOL_RND_KEEP(res);
    GAOL_RND_LEAVE();
    return res;
  }

  /*!
    \brief I^p for a floating-point p, gaol::pow(I, p) (GAOL v5)

    Without it, pow(I,2.5) called pow(const interval&, int), converting a
    double to an int being a standard conversion and converting it to an
    interval a user-defined one: the exponent was truncated, and pow([4],0.5)
    returned [1]. An integer p within the ints is computed by gaol_pown(),
    which is defined for negative bases too, any other p by
    gaol_pow_hybrid(), and an infinite or NaN p gives the empty set. Ported
    from the fix of Codac (commit 74086ccb, Jordan Ninin).
  */
  interval  gaol_pow_real(const interval& I, double p)
  {
    // Infinite or NaN, told by a quiet comparison: <= raised the
    // invalid-operation exception for a NaN p (GAOL v5). The empty set
    // returns before the check too, as sqr() does (GAOL v5)
    if (I.is_empty() || !gaol_detail::quiet_less_equal(std::fabs(p), (std::numeric_limits<double>::max)())) {
      return interval::emptyset();
    }
    // p compared after the check: with denormals-are-zero, the floor of a
    // subnormal p was p, and pow(I, p) the power [1] (GAOL v5, see
    // gaol/gaol_fpu.h)
    GAOL_RND_ENTER();
    p = rnd_reread(p);
    interval res;
    if (std::floor(p) == p && p >= (std::numeric_limits<int>::min)() && p <= (std::numeric_limits<int>::max)()) {
      res = pown_upward(I, static_cast<int>(p));
    } else {
      res = pow_hybrid_upward(I, interval(p));
    }
    GAOL_RND_KEEP(res);
    GAOL_RND_LEAVE();
    return res;
  }

  /*
    x^n rounded upward and downward, x >= 0 and n > 0, by binary exponentiation,
    the rounding direction being upward. x^n is a multiple of each square
    computed, so that ipow_hi() only overflows where x^n does; ipow_lo() is
    rounded downward through -((-a)*b) rounded upward, its product being kept
    negated.
  */
  static double ipow_hi(double x, unsigned int n)
  {
    double y = 1.0, z = x;
    for (;;) {
      if (odd(n)) {
	y *= z;
      }
      n >>= 1;
      if (n == 0) {
	return y;
      }
      z *= z;
    }
  }

  static double ipow_lo(double x, unsigned int n)
  {
    double y = -1.0, z = x;
    for (;;) {
      if (odd(n)) {
	y *= z;
      }
      n >>= 1;
      if (n == 0) {
	return -y;
      }
      z = -((-z)*z);
    }
  }

  /*
    ipow_hi() and ipow_lo() for the proofs of the roots of an x below
    tiny_root_argument (GAOL v5, point AC of TODO.md). Next to the root of
    x < 1, the powers l^k of the binary exponentiation (k <= n) are no smaller
    than x, which is below 2^-1022 for a subnormal x, and a subnormal has
    fewer bits: l^32 is about 2^-1041.5 for x = 2^-1074 and n = 33, which has
    33. Each rounded product there moved the proof away from the root by up to
    2^-32 relatively, and the root by that over n: nth_root([2^-1074], 33) was
    1983 doubles below its tightest lower bound and 28785 above its upper one,
    and n = 65 gave 3.5e8 and 6.9e8. The bounds still enclosed the root, each
    product being rounded in the direction of its proof. From
    tiny_root_argument, 4 times the smallest normal number, the powers next to
    the root (where l^n is within a relative 2^-17 of x) stay above 2^-1022,
    and ipow_hi() and ipow_lo() are used as they were.

    Here l = c 2^e with c in [1, 2) (exact), and the products are those of
    ipow_hi() and ipow_lo() on c, in the same order: l^n = h 2^s, s being an
    integer and h in [1, 2^500], scaled by 2^-500 above 2^500 (s is a long
    long: |e| n is below 2^43). Scaling by a power of two is exact and does
    not change what a product rounds to, so that h is the double of ipow_hi()
    or ipow_lo() wherever that one stays normal, and no product is rounded as
    a subnormal. Rounding upward, l > 0, n > 0
  */
  const double tiny_root_argument = 4.0*std::numeric_limits<double>::min();

  static double ipow_hi_scaled(double l, unsigned int n, long long& s)
  {
    const double big = 3.273390607896142e+150, small = 3.054936363499605e-151; // 2^500, 2^-500
    int e;
    double y = 1.0, z = 2.0*std::frexp(l, &e);
    long long ys = 0, zs = e - 1;
    for (;;) {
      if (odd(n)) {
	y *= z;
	ys += zs;
	if (y > big) {
	  y *= small;
	  ys += 500;
	}
      }
      n >>= 1;
      if (n == 0) {
	s = ys;
	return y;
      }
      z *= z;
      zs *= 2;
      if (z > big) {
	z *= small;
	zs += 500;
      }
    }
  }

  static double ipow_lo_scaled(double l, unsigned int n, long long& s)
  {
    const double big = 3.273390607896142e+150, small = 3.054936363499605e-151; // 2^500, 2^-500
    int e;
    double y = -1.0, z = 2.0*std::frexp(l, &e);
    long long ys = 0, zs = e - 1;
    for (;;) {
      if (odd(n)) {
	y *= z;
	ys += zs;
	if (y < -big) {
	  y *= small;
	  ys += 500;
	}
      }
      n >>= 1;
      if (n == 0) {
	s = ys;
	return -y;
      }
      z = -((-z)*z);
      zs *= 2;
      if (z > big) {
	z *= small;
	zs += 500;
      }
    }
  }

  // The sign of h 2^s - x, for h in [1, 2^500], s an integer and x > 0 finite:
  // h 2^s - x = (h 2^d - m) 2^(f - 1) with x = m 2^(f - 1), m in [1, 2)
  static int compare_scaled(double h, long long s, double x)
  {
    int f;
    const double m = 2.0*std::frexp(x, &f);
    const long long d = s - (f - 1);
    if (d > 0) { // h 2^d >= 2
      return 1;
    }
    if (d < -510) { // h 2^d <= 2^-11
      return -1;
    }
    const double t = gaol_ldexp(h, static_cast<int>(d)); // Exact: in [2^-510, 2^500]
    return (t < m) ? -1 : ((t > m) ? 1 : 0);
  }

  /*
    The n-th root of x rounded downward and upward, n > 2 and 0 < x < +oo, from
    an approximation r of it, the rounding direction being upward (fork of
    GAOL). GAOL took the power x^(1/n) of the mathematical library moved one
    double outward, 1/n being rounded: the root is then |log(x)|/n 2^-53 away
    relatively, up to 234 doubles for a cube root over all the doubles, and 8
    between 2^-30 and 2^30.

    A double l is below the root when l^n rounded upward is at most x, and u
    above it when u^n rounded downward is at least x: both tests grow with
    their argument. r is first brought next to the root by a step of Newton's
    method on r^n = x, r - r (r^n - x)/(n r^n), where r^n is a normal double,
    its rounding errors being relative then. The lower bound is the largest
    double proved to be below the root, and the upper bound the smallest one
    proved to be above it, looked for from r: outward or inward by steps that
    double, until a double that is proved and one that is not are found, then
    by bisection between them. From a double next to the root, that is two
    powers; from any other, as the math library of the system may give, the
    search still ends, and the bounds are proved.

    l^n, of n - 1 rounded products, is within (n - 1) 2^-52 of its value
    relatively (a product rounded in a direction is within 2^-52), which moves
    the root by less than 2^-52 relatively: the bounds are the tightest or up
    to 2 doubles beyond, and the root of a double that is an n-th power is that
    double. (GAOL v5: for an x below tiny_root_argument, the proofs take the
    scaled powers of ipow_hi_scaled() and ipow_lo_scaled(), whose products are
    never subnormal. This function does not: for a subnormal x its power is
    subnormal, its step is left out, and the search below starts from the
    value of CORE-MATH.)
  */
  static double newton_root(double x, double r, unsigned int n)
  {
    if (!(r > 0.0)) { // Negative or NaN
      return 0.0;
    }
    if (r > std::numeric_limits<double>::max()) {
      return std::numeric_limits<double>::max();
    }
    // The power of an r next to the root of an x above huge_root_argument may
    // overflow, which raised the overflow exception (GAOL v5, point 79 of
    // TODO.md): the step is left out, as for a subnormal x, and the search
    // starts from the value of CORE-MATH
    if (x > huge_root_argument) {
      return r;
    }
    const double p = ipow_hi(r,n);
    if (p >= std::numeric_limits<double>::min() && p <= std::numeric_limits<double>::max()) {
      // (p - x)/(n p) as ((p - x)/p)/n: n p overflowed for an n up to 2^32 and
      // a p above 2^992 (GAOL v5, point 79 of TODO.md). The step is left out
      // where it is 1 or more in magnitude, p being then no approximation of x
      const double t = quotient_sat(p - x, p)/double(n);
      if (std::fabs(t) < 1.0) {
	const double s = r - r*t;
	if (s > 0.0 && s <= std::numeric_limits<double>::max()) {
	  return s;
	}
      }
    }
    return r;
  }

  /*
    The two tests of the proofs, by the powers they take: ipow_hi() and
    ipow_lo(), or the scaled ones for an x below tiny_root_argument. The
    searches below are made for one or the other, chosen once (GAOL v5, point
    AC of TODO.md): a test of x inside them cost 3 to 30 ns of the 80 of the
    root of a normal x, according to how the compiler inlined them (n = 33,
    AppleClang 21, arm64, two builds of the same code)
  */
  struct plain_proofs
  {
    static inline bool below(double l, double x, unsigned int n)
    {
      return ipow_hi(l,n) <= x;
    }

    static inline bool above(double u, double x, unsigned int n)
    {
      return ipow_lo(u,n) >= x;
    }
  };

  struct scaled_proofs
  {
    static inline bool below(double l, double x, unsigned int n)
    {
      if (!(l > 0.0 && l < GAOL_INFINITY)) {
	return l == 0.0;
      }
      long long s;
      const double h = ipow_hi_scaled(l,n,s);
      return compare_scaled(h,s,x) <= 0;
    }

    static inline bool above(double u, double x, unsigned int n)
    {
      if (!(u > 0.0 && u < GAOL_INFINITY)) {
	return u == GAOL_INFINITY;
      }
      long long s;
      const double h = ipow_lo_scaled(u,n,s);
      return compare_scaled(h,s,x) >= 0;
    }
  };

  // A step of about one double at r, which doubles
  static inline double first_step(double r)
  {
    return maximum(r*std::numeric_limits<double>::epsilon(), double_denorm_min);
  }

  /*
    A double between l and h, l < h, their middle where it is one: the double
    above l otherwise, which is h when l and h are consecutive. The neighbours
    are reached with nextafter() (next_float(), previous_float()), whatever
    the rounding direction and the treatment of subnormals.
  */
  static inline double double_between(double l, double h)
  {
    const double m = l + 0.5*(h - l);
    return (l < m && m < h) ? m : next_float(l);
  }

  // The steps of the searches below end within a few thousand steps (the
  // doubling ones reach 0 or the largest double within 2100), and are bounded
  // in case a platform would not move: the bound found so far is returned
  const int root_search_limit = 4096;

  template<class Proofs>
  static double proved_root_dn_by(double x, double r, unsigned int n)
  {
    // l is proved to be below the root, and h is not: 0 is, and the largest
    // double is not
    const double largest = std::numeric_limits<double>::max();
    double l = newton_root(x,r,n), h = l, step = first_step(l);
    int steps = 0;
    if (Proofs::below(l,x,n)) {
      for (h = minimum(l + step, largest); Proofs::below(h,x,n) && h < largest; h = minimum(l + step, largest)) {
	l = h;
	step *= 2.0;
	if (++steps > root_search_limit) {
	  return l;
	}
      }
      if (h >= largest && Proofs::below(h,x,n)) { // Not for n > 1 and a finite x
	return h;
      }
    } else {
      for (l = maximum(-(step - h), 0.0); !Proofs::below(l,x,n); l = maximum(-(step - h), 0.0)) {
	h = l;
	step *= 2.0;
	if (++steps > root_search_limit) {
	  return 0.0;
	}
      }
    }
    for (;;) {
      const double m = double_between(l,h);
      if (!(m < h) || ++steps > root_search_limit) {
	return l;
      }
      if (Proofs::below(m,x,n)) {
	l = m;
      } else {
	h = m;
      }
    }
  }

  template<class Proofs>
  static double proved_root_up_by(double x, double r, unsigned int n)
  {
    // u is proved to be above the root, and l is not: the largest double is,
    // and 0 is not
    const double largest = std::numeric_limits<double>::max();
    double u = newton_root(x,r,n), l = u, step = first_step(u);
    int steps = 0;
    if (Proofs::above(u,x,n)) {
      for (l = maximum(-(step - u), 0.0); Proofs::above(l,x,n); l = maximum(-(step - u), 0.0)) {
	u = l;
	step *= 2.0;
	if (++steps > root_search_limit) {
	  return u;
	}
      }
    } else {
      for (u = minimum(l + step, largest); !Proofs::above(u,x,n); u = minimum(l + step, largest)) {
	l = u;
	step *= 2.0;
	if (++steps > root_search_limit) {
	  return GAOL_INFINITY;
	}
      }
    }
    for (;;) {
      const double m = double_between(l,u);
      if (!(m < u) || ++steps > root_search_limit) {
	return u;
      }
      if (Proofs::above(m,x,n)) {
	u = m;
      } else {
	l = m;
      }
    }
  }

  // The scaled proofs also for an x above huge_root_argument, 2^1000: the
  // powers of the doubles next to the root, up to a relative 2^-17 beyond it
  // as far as the search goes, stay below 2^1024 up to there, and the plain
  // ones overflowed above the largest double for a finite root (GAOL v5, point
  // 79 of TODO.md: nth_root([DBL_MAX], 5))
  static double proved_root_dn(double x, double r, unsigned int n)
  {
    return (x < tiny_root_argument || x > huge_root_argument) ? proved_root_dn_by<scaled_proofs>(x,r,n)
                                                              : proved_root_dn_by<plain_proofs>(x,r,n);
  }

  static double proved_root_up(double x, double r, unsigned int n)
  {
    return (x < tiny_root_argument || x > huge_root_argument) ? proved_root_up_by<scaled_proofs>(x,r,n)
                                                              : proved_root_up_by<plain_proofs>(x,r,n);
  }

  /*
    The power d^e of CORE-MATH, e being 1/n rounded, for the approximation the
    n-th root of d starts from. The roots of 0, 1 and +oo are themselves.
    Rounding upward
  */
  static inline bool root_is_itself(double d)
  {
    return d == 0.0 || d == 1.0 || d == GAOL_INFINITY;
  }

  static inline double root_near(double d, double e)
  {
    return root_is_itself(d) ? d : upward::nthroot_up(d,e);
  }

  // Rounding upward
  static inline double root_dn(double d, double near_root, unsigned int n)
  {
    return root_is_itself(d) ? d : proved_root_dn(d,near_root,n);
  }

  static inline double root_up(double d, double near_root, unsigned int n)
  {
    return root_is_itself(d) ? d : proved_root_up(d,near_root,n);
  }

  /*
    The approximations the n-th roots of a >= 0 and b >= 0 start from. Rounding
    upward, as nth_root_upward(), its only caller, takes it after the check of
    its callers: the direction is not checked again here (GAOL v5)
  */
  static void near_roots(double a, double b, unsigned int n, double& near_a, double& near_b)
  {
    const double e = 1.0/double(n);
    near_a = root_near(a,e);
    near_b = (b == a) ? near_a : root_near(b,e);
  }

  /*
    Code inspired from ia_math by Timothy Hickey

    rootn of IEEE 1788-2015 (Table 10.5, GAOL v5): defined on R for an odd
    n, the root of x < 0 being -(-x)^(1/n), and on [0,+oo] for an even n. GAOL
    took the roots of the part of I in [0,+oo] for every n: nth_root([-8,27],3)
    was [0,3] rather than [-2,3]. rootn(x,0) is not defined, and gives the
    empty set.
  */
/*
  The cube root, with CORE-MATH's cbrt (GAOL v5)

  cbrt is correctly rounded in the rounding direction in effect and increasing
  on the whole line. Computed in the upward rounding GAOL keeps, cbrt(x) of a
  positive x is therefore the tightest double at or above the cube root, and
  the double below it the tightest one at or below, unless the cube root is a
  double: u being the value rounded upward, u^3 >= x, and the product of three
  positive numbers rounded upward is at least u^3, so it equals x only when
  u^3 = x exactly. That test is only made on positive numbers, where the
  roundings compose; the root of a negative number is the opposite of the root
  of its magnitude, as the general nth_root() takes it.

  The general nth_root() looked for the bounds from pow(x, 1/n) by Newton's
  method then by bisection, which took 89 ns for a cube root against 41 ns
  here (the medians of three runs, i7-1185G7, Clang 18; 77 against 24 ns on an
  Apple M2), and gave bounds up to 2 doubles from the tightest ones.
*/
/*
  u*u*u rounded upward == x, for u the cube root of x rounded upward. The
  products reach 2^1024 for an x next to the largest double, which raised the
  overflow exception for a finite root (GAOL v5, point 79 of TODO.md): from
  huge_root_argument, the root u >= 2^333 is scaled by 2^-342 and x by 2^-1026,
  exactly (x >= 2^-26 is then normal), as the square root is (see
  gaol_sqrt_up()), and the products scaled with them are rounded to the same
  bits
*/
static const double two_power_minus_342 = 1.1161986242990967e-103, two_power_minus_513 = 3.7291703656001034e-155;

static inline bool cube_up_equals(double u, double x)
{
  if (x > huge_root_argument) {
    const double t = read_in_branch(u)*two_power_minus_342;
    return t*t*t == (read_in_branch(x)*two_power_minus_513)*two_power_minus_513;
  }
  return u*u*u == x;
}

// Inlined by force with GCC and Clang, as cube_root_upward() and cube_root()
// below
#if defined(__GNUC__) || defined(__clang__)
__attribute__((always_inline))
#endif
static inline double cube_root_up(double x) // x >= 0
{
  return gaol_cr_cbrt(x);
}

#if defined(__GNUC__) || defined(__clang__)
__attribute__((always_inline))
#endif
static inline double cube_root_dn(double x) // x >= 0
{
  const double u = gaol_cr_cbrt(x);
  return cube_up_equals(u, x) ? u : previous_float(u);
}

// The rounding direction being upward already, after the check of cube_root()
// or of the callers of nth_root_upward() (GAOL v5). Inlined by force with GCC
// and Clang, as cube_root(), cube_root_up() and cube_root_dn(): Clang 18
// called cube_root_dn() from nth_root() otherwise, and the cube root of an
// interval took 40 ns rather than 34 (Intel i7-1185G7)
#if defined(__GNUC__) || defined(__clang__)
__attribute__((always_inline))
#endif
static inline interval cube_root_upward(const interval& I)
{
  if (I.is_empty()) {
    return interval::emptyset();
  }
  const double a = I.left(), b = I.right();
  double lo, hi;
  if (a == -GAOL_INFINITY) {
    lo = -GAOL_INFINITY;
  } else {
    lo = (a >= 0.0) ? cube_root_dn(a) : -cube_root_up(-a);
  }
  if (b == GAOL_INFINITY) {
    hi = GAOL_INFINITY;
  } else {
    hi = (b >= 0.0) ? cube_root_up(b) : -cube_root_dn(-b);
  }
  return interval(lo, hi);
}

#if defined(__GNUC__) || defined(__clang__)
__attribute__((always_inline))
#endif
static inline interval cube_root(const interval& I)
{
  if (I.is_empty()) {
    return interval::emptyset();
  }
  GAOL_RND_ENTER();
  interval res = cube_root_upward(I);
  GAOL_RND_KEEP(res);
  GAOL_RND_LEAVE();
  return res;
}

/*
  The n-th root of I, the rounding direction being upward already: the body
  of nth_root(), which nth_root(I, q) for q < 0 and nth_root_rel() call after
  their own check (GAOL v5: they called nth_root(), sqrt() or inverse(), which
  checked the direction again)
*/
interval nth_root_upward(const interval& I, unsigned int n)
{
	switch (n) {
	case 0:
		return interval::emptyset();
	case 1:
		return I;
	case 2:
		return sqrt_upward(I);
	case 3:
		return cube_root_upward(I);
	default:
		break;
	}
	const interval J = odd(n) ? I : (I & interval::positive());
	if (J.is_empty()) {
		return interval::emptyset();
	}
	// The roots of the magnitudes of the bounds, the root of x < 0 being
	// -(-x)^(1/n)
	const double a = std::fabs(J.left()), b = std::fabs(J.right());
	double near_a, near_b;
	near_roots(a,b,n,near_a,near_b);
	const double l = (J.left() >= 0.0) ? root_dn(a,near_a,n) : -root_up(a,near_a,n);
	const double r = (J.right() >= 0.0) ? root_up(b,near_b,n) : -root_dn(b,near_b,n);
	return interval(l,r);
}

interval nth_root(const interval& I, unsigned int n)
{
	switch (n) {
	case 0:
		return interval::emptyset();
	case 1:
		return I;
	case 2:
		return sqrt(I);
	case 3:
		return cube_root(I);
	default:
		break;
	}
	// The check before the intersection, as in sqrt()
	GAOL_RND_ENTER();
	interval res = nth_root_upward(I, n);
	GAOL_RND_KEEP(res);
	GAOL_RND_LEAVE();
	return res;
}

/*
  rootn(x, q) with an integer q, which may be negative (GAOL v5)

  IEEE 1788-2015 recommends rootn(x, q) for every q of Z\{0} (Table 10.5),
  where GAOL only took a positive one. For q < 0, x^(1/q) = 1/x^(1/|q|), whose
  domain is the one the table gives: R\{0} for an odd q, (0, +oo) for an even
  one. Taking the inverse gives exactly that, the inverse of an interval
  holding 0 being the hull of the values away from it, so the result is the
  natural interval extension: rootn([-1, 1], -3) is the hull of
  (-oo, -1] u [1, +oo), that is the whole line, and rootn([0], -3) is empty.
*/
interval nth_root(const interval& I, int q)
{
	if (q > 0) {
		return nth_root(I, static_cast<unsigned int>(q));
	}
	if (q == 0) {
		// As rootn(x, 0), which the table leaves out: no value
		return interval::emptyset();
	}
	// |q| without a signed negation: -q overflows an int for INT_MIN, and so
	// did -static_cast<long>(q) where a long has 32 bits too, with Visual C++,
	// MinGW-w64 and on 32-bit Linux (GAOL v5)
	const unsigned int n = 0u - static_cast<unsigned int>(q);
#if ((defined(__x86_64__) || defined(_M_X64)) && GAOL_USING_SSE2_INSTRUCTIONS && GAOL_AVX512_ARITHMETIC) \
    || (GAOL_USING_NEON_INSTRUCTIONS && !GAOL_PRESERVE_ROUNDING)
	// The square root has a vector path of its own, by the embedded rounding of
	// AVX-512 and by one vector instruction with NEON, which sqrt_upward()
	// (gaol_interval_internal.h) does not have: the one check before the
	// scalar root took nth_root(x, -2) from 17.8 to 22.2 ns with
	// GAOL_PRESERVE_ROUNDING (an Intel i7-1185G7, Clang 21),
	// where the cube root, which has only the scalar body, went from 53.0 to 45.9.
	// Not with NEON where the direction is preserved: sqrt() changes the direction
	// at its entry and sets it back at its exit there, and inverse() does it again,
	// which the one check of the code below pays once (an Apple M2, Clang 21:
	// 46.4 ns against 67.6 ns with this path, 8.4 against 5.5 without the
	// preservation) (GAOL v5, point PG.2 of TODO.md)
	if (n == 2u) {
		return inverse(sqrt(I));
	}
#endif
	// One check, before the root and its inverse, which do not check the
	// direction again (GAOL v5: the root and inverse() checked it each)
	GAOL_RND_ENTER();
	const interval root = nth_root_upward(I, n);
	interval res = root.is_empty() ? root : upward_access::inverse(root);
	GAOL_RND_KEEP(res);
	GAOL_RND_LEAVE();
	return res;
}

  /*
    The root of order q of I for a q beyond the ints and the unsigned ints,
    after one check of the rounding direction (GAOL v5: the template
    gaol_detail::huge_root() of gaol/core/gaol_interval_core.h computed it, where 1/|q|
    and the two powers checked it once each): order is the tightest interval
    holding |q|, negative and odd tell the sign and the parity of q, and the
    inverse of order has positive bounds. Declared in no header, and not
    exported from a shared library, which is built with hidden visibility.
  */
  interval root_of_large_order(const interval& I, const interval& order, bool negative, bool odd)
  {
    GAOL_RND_ENTER();
    const interval inverse = upward_access::inverse(order);
    const interval exponent = negative ? -inverse : inverse;
    interval res = pow_hybrid_upward(I & interval::positive(), exponent);
    if (odd) {
      res = res | -pow_hybrid_upward(-(I & interval::negative()), exponent);
    }
    GAOL_RND_KEEP(res);
    GAOL_RND_LEAVE();
    return res;
  }

  interval nth_root(const interval& I, long long q)
  {
    if (q >= INT_MIN && q <= INT_MAX) {
      return nth_root(I, static_cast<int>(q));
    }
    if (q > 0 && q <= static_cast<long long>(UINT_MAX)) {
      return nth_root(I, static_cast<unsigned int>(q));
    }
    return root_of_large_order(I, (q < 0) ? -interval(q) : interval(q), q < 0, q % 2 != 0);
  }

  interval nth_root(const interval& I, unsigned long long q)
  {
    if (q <= UINT_MAX) {
      return nth_root(I, static_cast<unsigned int>(q));
    }
    return root_of_large_order(I, interval(q), false, q % 2 != 0);
  }

  /*
    rsqrt, the rSqrt of IEEE 1788-2015 (Table 10.5): 1/sqrt(x) on (0, +oo),
    decreasing, +oo the limit at 0.

    1/sqrt(x) is a double exactly at the powers of two of even exponent,
    2^(2k), where it is 2^-k: were 1/sqrt(x) = m 2^e with m an odd integer
    above 1, x = 2^(-2e)/m^2 would be no double. And it is 0 at +oo.
  */
  static inline bool rsqrt_is_exact(double x)
  {
    if (x == GAOL_INFINITY) {
      return true;
    }
    int e;
    return std::frexp(x, &e) == 0.5 && (e - 1) % 2 == 0;
  }

  interval rsqrt(const interval& I)
  {
    if (I.is_empty()) {
      return interval::emptyset();
    }
    GAOL_RND_ENTER();
    if (!(I.right() > 0.0)) {
      GAOL_RND_LEAVE();
      return interval::emptyset();
    }
    // +0 rather than -0 for a left bound at or below 0: 1/sqrt(-0) is -oo
    const double l = (I.left() > 0.0) ? I.left() : 0.0, r = I.right();
    // decreasing: the upper bound at the left bound, the lower one at the right
    const double v = gaol_cr_rsqrt(l);
    const double w = gaol_cr_rsqrt(r);
    const double u = rsqrt_is_exact(r) ? w : previous_float(w);
    interval res(maximum(0.0, u), v);
    GAOL_RND_KEEP(res);
    GAOL_RND_LEAVE();
    return res;
  }

  /*
    hypot of IEEE 1788-2015 (Table 10.5): sqrt(x^2 + y^2) on the plane. It
    increases with |x| and with |y|: its least value on X x Y is at the point
    nearest to the origin, (mig X, mig Y), and its greatest at the farthest,
    (mag X, mag Y).
  */

  // Whether h^2 - b^2 = a^2 4^d, for odd integers h, b, a below 2^53: both
  // sides are integers below 2^107, which a double with the residual of its
  // rounding holds exactly, std::fma computing the residual. The rounding of
  // an exact value is unique, whatever its direction: the two sides are equal
  // exactly when the roundings and the residuals are.
  static inline bool difference_of_squares(std::uint64_t h, std::uint64_t b,
                                           std::uint64_t a, int d)
  {
    if (h <= b || d > 53) { // a^2 4^d >= 2^(2d) > (h - b)(h + b) for d > 53
      return false;
    }
    const double s = static_cast<double>(h - b), t = static_cast<double>(h + b); // both exact
    const double m = static_cast<double>(a);
    const double p = s * t, q = m * m;
    return p == std::ldexp(q, 2 * d)
        && fma_once(s, t, -p) == std::ldexp(fma_once(m, m, -q), 2 * d);
  }

  /* Whether h, sqrt(a^2 + b^2) rounded upward, is the exact value, for
     a, b >= 0. It is at a = 0 or b = 0, where it is the other one, and at an
     infinity. Otherwise, with a = A 2^ea, b = B 2^eb and h = H 2^eh, A, B and H
     odd integers: dividing A^2 4^ea + B^2 4^eb = H^2 4^eh by the least power of
     4 among them leaves one, two or three odd squares, each 1 modulo 8, the
     other terms being multiples of 4. One odd square alone is not 0 modulo 4,
     A^2 + B^2 is 2 modulo 8 whether H^2 is left or not: the equality holds
     only where eh is the least exponent and equal to eb, H^2 - B^2 being
     A^2 4^(ea - eb), or equal to ea, the same with a and b exchanged. */
  static bool hypot_is_exact(double a, double b, double h)
  {
    if (a == 0.0 || b == 0.0 || !is_finite(a) || !is_finite(b)) {
      return true;
    }
    if (!is_finite(h)) {
      return false; // beyond the largest double
    }
    std::uint64_t A, B, H;
    int ea, eb, eh;
    odd_significand(a, A, ea);
    odd_significand(b, B, eb);
    odd_significand(h, H, eh);
    if (eh == eb && ea > eb) {
      return difference_of_squares(H, B, A, ea - eb);
    }
    if (eh == ea && eb > ea) {
      return difference_of_squares(H, A, B, eb - ea);
    }
    return false;
  }

  interval hypot(const interval& X, const interval& Y)
  {
    if (X.is_empty() || Y.is_empty()) {
      return interval::emptyset();
    }
    // mig and mag compare the bounds: after the check (see gaol/gaol_fpu.h)
    GAOL_RND_ENTER();
    const double al = mig_of(X), bl = mig_of(Y), ar = mag_of(X), br = mag_of(Y);
    const double w = gaol_cr_hypot(al, bl);
    const double u = hypot_is_exact(al, bl, w) ? w : previous_float(w);
    const double v = gaol_cr_hypot(ar, br);
    interval res(maximum(0.0, u), v);
    GAOL_RND_KEEP(res);
    GAOL_RND_LEAVE();
    return res;
  }

  /*
    The signs of the bounds, and whether two are equal, by their bits: a mode
    that flushes the subnormals to zero, which the program may have set, makes
    a subnormal bound compare as 0 (see gaol/gaol_fpu.h), and no comparison
    here depends on it (GAOL v5)
  */
  static inline bool cm1_positive(double d) { return !gaol_cm1::dbl_is_zero(d) && !gaol_cm1::dbl_is_negative(d); }
  static inline bool cm1_negative(double d) { return !gaol_cm1::dbl_is_zero(d) && gaol_cm1::dbl_is_negative(d); }
  static inline bool cm1_same(double a, double b)
  {
    return gaol_cm1::dbl_bits(a) == gaol_cm1::dbl_bits(b) || (gaol_cm1::dbl_is_zero(a) && gaol_cm1::dbl_is_zero(b));
  }

  /*
    compoundm1 of IEEE 1788-2015 (Table 10.5): (1 + x)^y - 1 for x > -1, and
    for x = -1 when y > 0, where it is -1 (GAOL v5).

    With u = 1 + x it is u^y - 1, whose analysis is that of pow above: it
    increases with y for x > 0 and decreases for x < 0, increases with x for
    y > 0 and decreases for y < 0, and is 0 on x = 0 and on y = 0, so that its
    extrema over a box are at corners, which the places of the bounds about 0
    give. The corners are those of pow_standard_upward() with the bases moved
    by 1 (the base 0 of pow being x = -1, its base 1 x = 0, and its limit 0 the
    limit -1), and the point function is gaol_cm1::compoundm1_point() of
    gaol/math/gaol_compoundm1.h, which has the tightest bounds: CORE-MATH has
    no compoundm1, and exp(y log1p(x)) - 1 would move the relative width of
    log1p(x) by |y log1p(x)|. At x = -1 and at +oo, and for an infinite y,
    the bound is the limit of the function, -1 or +oo, which bounds it both
    ways; at x = 0 or y = 0 it is 0.
  */
  static inline bool cm1_is_limit(double x, double y)
  {
    return !(x > -1.0 && x < GAOL_INFINITY && std::fabs(y) < GAOL_INFINITY);
  }

  // The bounds of (1 + x)^y - 1 at a corner of a box, x in [-1, +oo] and y in
  // [-oo, +oo]
  static inline void cm1_corner(double x, double y, double& lo, double& hi)
  {
    if (gaol_cm1::dbl_is_zero(x) || gaol_cm1::dbl_is_zero(y)) {
      lo = hi = 0.0;
    } else if (cm1_is_limit(x, y)) {
      // +oo where 1 + x > 1 and y > 0 or 1 + x < 1 and y < 0, -1 otherwise
      lo = hi = (cm1_positive(x) == cm1_positive(y)) ? GAOL_INFINITY : -1.0;
    } else {
      gaol_cm1::compoundm1_point(x, y, lo, hi);
    }
  }

  static inline double cm1_lo(double x, double y)
  {
    double lo, hi;
    cm1_corner(x, y, lo, hi);
    return lo;
  }

  static inline double cm1_hi(double x, double y)
  {
    double lo, hi;
    cm1_corner(x, y, lo, hi);
    return hi;
  }

  static interval compoundm1_upward(const interval& x, const interval& y)
  {
    // [+oo] and [-oo] contain no real number, as for pow (holds_no_real())
    if (x.is_empty() || y.is_empty() || holds_no_real(x) || holds_no_real(y)) {
      return interval::emptyset();
    }
    // x^y - 1 is only real for 1 + x > 0, and for 1 + x = 0 when y > 0
    const interval base = x & interval(-1.0, GAOL_INFINITY);
    if (base.is_empty()) {
      return interval::emptyset();
    }
    if (base.right() == -1.0) {
      return cm1_positive(y.right()) ? interval(-1.0) : interval::emptyset();
    }
    const double xl = base.left(), xu = base.right(), yl = y.left(), yu = y.right();
    double l, r;
    if (cm1_same(xl, xu) && cm1_same(yl, yu)) {
      // A point: one corner gives both bounds
      cm1_corner(xl, yl, l, r);
    } else if (cm1_same(yl, yu)) {
      // A degenerate exponent: increasing with x for y > 0, decreasing for
      // y < 0, one corner for each bound
      l = cm1_lo(cm1_positive(yl) ? xl : xu, yl);
      r = cm1_hi(cm1_positive(yl) ? xu : xl, yl);
    } else if (xl == -1.0 && cm1_positive(yl)) {
      // A base from -1 and exponents above 0: -1 is the least value, and the
      // greatest is at xu, with yu for xu >= 0 and yl below
      l = -1.0;
      r = cm1_hi(xu, !cm1_negative(xu) ? yu : yl);
    } else if (!cm1_negative(xl)) {
      l = cm1_lo(!cm1_negative(yl) ? xl : xu, yl);
      r = cm1_hi(!cm1_negative(yu) ? xu : xl, yu);
    } else if (!cm1_positive(xu)) {
      l = cm1_lo(!cm1_negative(yu) ? xl : xu, yu);
      r = cm1_hi(!cm1_negative(yl) ? xu : xl, yl);
    } else {
      // x holds 0 inside, where the function is 0: it is at most 0 at
      // (xl, y >= 0) and (xu, y <= 0), the least at (xl, yu) or (xu, yl), and
      // at least 0 at (xl, y <= 0) and (xu, y >= 0), the greatest at
      // (xl, yl) or (xu, yu). An exponent of the other sign is taken as 0,
      // for which the corner is 0 with no call of compoundm1_point(). Where y
      // holds 0 inside and one of the two corners is a limit, the other is
      // not computed: the limit is -1 at (xl, yu) and (xu, yl), which bounds
      // the box below, and +oo at (xl, yl) and (xu, yu), which bounds it above
      const double yu_or_0 = cm1_positive(yu) ? yu : 0.0, yl_or_0 = cm1_negative(yl) ? yl : 0.0;
      const bool around_0 = cm1_negative(yl) && cm1_positive(yu);
      l = (around_0 && (cm1_is_limit(xl, yu) || cm1_is_limit(xu, yl)))
          ? -1.0 : minimum(cm1_lo(xl, yu_or_0), cm1_lo(xu, yl_or_0));
      r = (around_0 && (cm1_is_limit(xl, yl) || cm1_is_limit(xu, yu)))
          ? GAOL_INFINITY : maximum(cm1_hi(xl, yl_or_0), cm1_hi(xu, yu_or_0));
    }
    return interval(l, r);
  }

  interval compoundm1(const interval& X, const interval& Y)
  {
    // An empty operand returns before the check, as pow() does (GAOL v5)
    if (X.is_empty() || Y.is_empty()) {
      return interval::emptyset();
    }
    GAOL_RND_ENTER();
    interval res = compoundm1_upward(X, Y);
    GAOL_RND_KEEP(res);
    GAOL_RND_LEAVE();
    return res;
  }

} // namespace gaol_core
