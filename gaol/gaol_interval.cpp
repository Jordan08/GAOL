/*-*-C++-*------------------------------------------------------------------
 * gaol -- NOT Just Another Interval Library
 *--------------------------------------------------------------------------
 * This file is part of the gaol distribution. Gaol was primarily
 * developed at the Swiss Federal Institute of Technology, Lausanne,
 * Switzerland, and is now developed at the Laboratoire d'Informatique de
 * Nantes-Atlantique, France.
 *
 * Copyright (c) 2001 Swiss Federal Institute of Technology, Switzerland
 * Copyright (c) 2002-2006 Laboratoire d'Informatique de
 *                         Nantes-Atlantique, France
 *--------------------------------------------------------------------------
 * gaol is a software distributed WITHOUT ANY WARRANTY. Read the associated
 * COPYING file for information.
 *--------------------------------------------------------------------------
 * By: Frederic Goualard <Frederic.Goualard@univ-nantes.fr>
 *--------------------------------------------------------------------------*/

/*!
  \file   gaol_interval.cpp
  \brief

  <long description>

  \author Goualard Frederic
  \date   2001-09-28
*/

// TODO: Rewrite as many functions as possible to use SIMD instructions
// FIXME: Use of f_negate deprecates -ffloat-store?

#include <iostream>
#include <iomanip>
#include <ios>
#include <sstream>
#include <cmath>
#include <string>
#include <cerrno>
#include <locale>
#include <cstdlib>
#include <cstdio>
#include <climits>
#include <cstdint>
#include <cstring>
#include <sstream>

#if GAOL_USING_SSE2_INSTRUCTIONS
#  include <pmmintrin.h>
#endif

#include "gaol/gaol_config.h"
#include "gaol/gaol_limits.h"
#include "gaol/gaol_fpu.h"
// The bounds at doubles, *_dn() and *_up(): a header of the sources, not
// installed (GAOL v5)
#include "gaol/gaol_double_op.h"
#include "gaol/gaol_common.h"
#include "gaol/gaol_parser.h"
#include "gaol/gaol_port.h"
#include "gaol/gaol_interval.h"
#include "gaol/gaol_ieee1788.h"
#include "gaol/gaol_limits.h"
#include "gaol/gaol_exceptions.h"

using std::ostream;
using std::istream;
using std::sqrt;

//debug
using std::cout;
using std::endl;


namespace gaol {
  // Defined in gaol_interval_lexer.lpp, in the namespace of the parser: the
  // sign of v - x, v being the number s writes, with no sign, compared exactly
  // with the double x
  int compare_number_with_double(const char *s, double x);
}

namespace gaol_core {

  // The bounds of pi and pi/2 (gaol/gaol_port.h), and 2^52, from which on the
  // doubles are integers, which gaol_port.h declared for the code using GAOL
  // too (GAOL v5)
  using namespace gaol_detail;
  const double two_power_52 = 4503599627370496.0;

  // I^e for a non-empty I and e > 0, defined below: gaol_uipow(), in the
  // files included here, calls it
  static interval uipow_nonempty(const interval& I, unsigned int e);

#if defined(__x86_64__) && GAOL_USING_SSE2_INSTRUCTIONS && GAOL_HAVE_AVX512_TARGET
  // Set by init() (gaol/gaol_common.cpp), from the GAOL_PREFER_AVX512 of the
  // build and the instructions the processor has: +, -, *, / and sqrt take
  // the AVX-512 path of gaol/gaol_interval_avx512.cpp then, which sets
  // neither the rounding direction nor the flush-to-zero modes (GAOL v5)
  extern bool avx512_arithmetic;

#  include <immintrin.h>   // the AVX-512 intrinsics of the path below
#  include "gaol/gaol_interval_avx512.cpp"
#endif
#if GAOL_USING_SSE2_INSTRUCTIONS
#  include "gaol/gaol_interval_sse.cpp"
#else
#  include "gaol/gaol_interval_fpu.cpp"
#endif // GAOL_USING_SSE2_INSTRUCTIONS

  /*
    x^n rounded upward and downward from exact products, x >= 0 and n >= 2, the
    rounding direction being upward (GAOL v5, issue #7). GAOL rounds each
    product of its binary exponentiation outward (uipow_rounded()), and x^n is
    then up to n + 1 doubles from the tightest bound: 2, 3, 5, 6, 8 for n = 3
    to 7.

    The power is kept as h + l, l being small: a product h*y, rounded upward,
    is p, and fma(h, y, -p) is the rest h*y - p <= 0, a double, exactly. The
    upper bound keeps h + l above the power, l <= 0 being rounded upward:
    (h + l)^2 = p + (h*h - p) + l*(2h + l). The lower bound keeps h - nl below
    it, nl >= 0 being rounded upward: (h - nl)^2 = p - ((p - h*h) +
    nl*(2h - nl)). The square of nl raises the lower bound: left out (GAOL
    v5, review #7 of examples/examples.md), the lower bound of x^n was 8
    doubles below the tightest for x = 1.0000001 and n = 2^28 - 1, 557 for
    2^31 - 1, 1962 for 2^32 - 1, more than one double below for most n above
    2^28.

    The power is only rounded at the end, h + l upward and h - nl downward.
    The rest of x^k is below (k - 1) 2^-52 x^k; squaring x^k rounds three
    terms below twice that, the rounding of the rest of p adding 2^-104
    x^2k at most, and a product by x rounds two, relatively: h + l and h - nl
    are within (3s + 2m) n 2^-104 of x^n, relatively, s and m being the
    numbers of squarings and of products by x, 5 log2(n) n 2^-104 at most
    (about 1.8 log2(n) n 2^-104 found, n up to 2^32 - 1). The bounds
    are thus the tightest, or one double beyond where the power is within
    5 log2(n) n 2^-104 of a double, relatively, and exact where the power is
    a double. false when a product is not finite, or is below 2^-968, its
    rest being no double then: the rounded products handle these powers, 0
    included.
  */
  // Whether the rest of the product rounded to p is a double for sure: a
  // multiple of 2^-105 p, it is one from p = 2^-968 on. Not for an infinite p
  static inline bool is_normal_product(double p)
  {
    return p >= std::numeric_limits<double>::min()*18014398509481984.0 /* 2^54 */ && p <= std::numeric_limits<double>::max();
  }

  static bool ipow_exact_up(double x, unsigned int n, double& bound)
  {
    unsigned int bit = 1u;
    while ((n >> 1) >= bit) { // The highest bit of n
      bit <<= 1;
    }
    double h = x, l = 0.0;
    for (bit >>= 1; bit != 0u; bit >>= 1) {
      double p = h*h;
      if (!is_normal_product(p)) {
	return false;
      }
      l = std::fma(h,h,-p) + l*(-(-2.0*h - l)); // (h + l)^2 - p: l <= 0 times 2h + l rounded downward
      h = p;
      if (n & bit) {
	p = h*x;
	if (!is_normal_product(p)) {
	  return false;
	}
	l = std::fma(h,x,-p) + l*x;
	h = p;
      }
    }
    bound = h + l;
    return true;
  }

  static bool ipow_exact_dn(double x, unsigned int n, double& bound)
  {
    unsigned int bit = 1u;
    while ((n >> 1) >= bit) {
      bit <<= 1;
    }
    double h = x, nl = 0.0;
    for (bit >>= 1; bit != 0u; bit >>= 1) {
      double p = h*h;
      if (!is_normal_product(p)) {
	return false;
      }
      nl = std::fma(-h,h,p) + nl*(2.0*h - nl); // p - (h - nl)^2 rounded upward, as are 2h - nl and nl*(2h - nl), nl >= 0
      h = p;
      if (n & bit) {
	p = h*x;
	if (!is_normal_product(p)) {
	  return false;
	}
	nl = std::fma(-h,x,p) + nl*x;
	h = p;
      }
    }
    bound = maximum(-(-h + nl), 0.0); // h - nl rounded downward
    return true;
  }

  /*
    I^e for a non-empty I and e > 0, as gaol_pown() and gaol_uipow() call
    it: from exact products for e > 2, the square being the tightest already,
    and from the rounded products where a bound is 0 or infinite, or a power
    is not finite or is below 2^-968. uipow_nonempty_upward() takes the
    rounding direction to be upward already, as it is in gaol_pown() after its
    check; uipow_nonempty() checks it, once: the rounded products of the
    fallback no longer check it again (GAOL v5).
  */
  /* The bounds of I^e from exact products, e > 2, rounding upward; false
     where the rounded products are to take (see above). Inlined by force with
     GCC and Clang in its two callers: called by Clang 18, it made
     pow([0, b], 3) and pow(x, -3) 1 ns slower (8 % and 5 %) than when
     uipow_nonempty() held it itself (GAOL v5). */
#if defined(__GNUC__) || defined(__clang__)
  __attribute__((always_inline))
#endif
  static inline bool uipow_exact(const interval& I, unsigned int e, double& l, double& r)
  {
    const double a = I.left(), b = I.right();
    double t = 0.0;
    bool finite;
    if (a >= 0.0) {
      finite = ipow_exact_dn(a,e,l) && ipow_exact_up(b,e,r);
    } else if (b <= 0.0) {
      if (odd(e)) { // [-|a|^e, -|b|^e]
	finite = ipow_exact_up(-a,e,t) && ipow_exact_dn(-b,e,r);
	l = -t;
	r = -r;
      } else {
	finite = ipow_exact_dn(-b,e,l) && ipow_exact_up(-a,e,r);
      }
    } else if (odd(e)) { // I straddles 0: [-|a|^e, b^e]
      finite = ipow_exact_up(-a,e,t) && ipow_exact_up(b,e,r);
      l = -t;
    } else { // [0, mag(I)^e]
      finite = ipow_exact_up(maximum(-a,b),e,r);
    }
    return finite;
  }

  static interval uipow_nonempty_upward(const interval& I, unsigned int e)
  {
    if (e < 3) {
      return uipow_rounded_upward(I,e);
    }
    double l = 0.0, r = 0.0;
    return uipow_exact(I,e,l,r) ? interval(l,r) : uipow_rounded_upward(I,e);
  }

  static interval uipow_nonempty(const interval& I, unsigned int e)
  {
    // The powers of e < 3 are checked by uipow_rounded(), with
    // GAOL_RND_ENTER_SSE() in the SSE2 build, which saves and restores the
    // direction of the SSE instructions only with GAOL_PRESERVE_ROUNDING
    if (e < 3) {
      return uipow_rounded(I,e);
    }
    GAOL_RND_ENTER();
    double l = 0.0, r = 0.0;
    if (uipow_exact(I,e,l,r)) {
      GAOL_RND_KEEP(l);
      GAOL_RND_KEEP(r);
      GAOL_RND_LEAVE();
      return interval(l,r);
    }
    interval res = uipow_rounded_upward(I,e);
    GAOL_RND_KEEP(res);
    GAOL_RND_LEAVE();
    return res;
  }



  // Default format for output
  interval_format::format_t interval::output = interval_format::bounds;

  // Number of digits to print.
  std::streamsize interval::output_precision = 16;

  /* The square root rounded upward, whatever the rounding of ::sqrt. IEEE 754
     requires a square root to be rounded in the rounding direction in effect,
     but the C library of Visual C++ for 32-bit x86 rounds it to nearest in
     every direction. The result of ::sqrt, correctly rounded in some direction
     and thus within one float of the exact root, is compared with the exact
     root through s*s rounded downward, and moved to the next float above when
     it is not a bound yet; where ::sqrt rounds as it should, it is left
     unchanged. The next float is reached by adding the smallest denormal,
     rather than with nextafter(), which IBEX found to crash on ARM64 macOS when
     not rounding to nearest. To be called with the rounding direction set
     upward; the root rounded downward is gaol_minus_sqrt_down() below.

     s*s rounded downward, -((-s)*s) when rounding upward, is below x exactly
     when s*s is, x being a double, and s is then below sqrt(x) (GAOL v5:
     GAOL compared s with x/s, a division, several times slower than a
     product). */
  static double gaol_sqrt_up(double x)
  {
    double s = ::sqrt(x);
    if (-((-s)*s) < x) { // s*s rounded downward < x proves s < sqrt(x)
      s += std::numeric_limits<double>::denorm_min();
    }
    return s;
  }

  /* The square root of x rounded downward, u being the square root rounded
     upward, to be called with the rounding direction set upward: u when it is
     the exact root, the double below u otherwise. u*u rounded upward is x
     exactly when u is the exact root, u*u being above x otherwise. Returned
     negated, as the SSE2 intervals store their lower bound (GAOL v5: GAOL
     computed x/u rounded downward, one double below the tightest bound for half
     of the doubles). sqrt() and sqrt_rel() take their lower bounds from it,
     without setting the rounding direction downward. */
  static double gaol_minus_sqrt_down(double x, double u)
  {
    return (u*u == x) ? -u : (-u + std::numeric_limits<double>::denorm_min());
  }

  /*
    The bounds of the elementary functions at the arguments where their value
    is a double, or one of the constants whose tightest bounds GAOL knows (fork
    of GAOL): the value of the mathematical library moved outward is not the
    tightest bound there. sin(0) = tan(0) = asin(0) = atan(0) = 0, cos(0) = 1,
    acos(1) = 0, acos(0) = asin(1) = atan(+oo) = pi/2, acos(-1) = pi,
    atan(1) = pi/4, and for the hyperbolic functions, from the libm moved three
    doubles outward: sinh(0) = tanh(0) = asinh(0) = atanh(0) = 0, cosh(0) = 1,
    acosh(1) = 0, cosh(x) and sinh(x) beyond the largest double for x >= 711,
    and tanh(x) above the double below 1 for x >= 20, 2 exp(-2x) being below
    2^-54 there.
    They are called between GAOL_RND_ENTER() and GAOL_RND_LEAVE(), the
    rounding direction being upward, and call the functions of namespace
    upward, which do not check it again (GAOL v5).
  */
  static inline double sin_lo(double x) { return (x == 0.0) ? 0.0 : upward::sin_dn(x); }
  static inline double sin_hi(double x) { return (x == 0.0) ? 0.0 : upward::sin_up(x); }
  static inline double cos_lo(double x) { return (x == 0.0) ? 1.0 : upward::cos_dn(x); }
  static inline double cos_hi(double x) { return (x == 0.0) ? 1.0 : upward::cos_up(x); }
  static inline double tan_lo(double x) { return (x == 0.0) ? 0.0 : upward::tan_dn(x); }
  static inline double tan_hi(double x) { return (x == 0.0) ? 0.0 : upward::tan_up(x); }

  /*
    The signs of sin(x) and cos(x), x being finite (GAOL v5, issue #6).
    Neither is 0 but sin(0): no other double is a multiple of pi/2, and the
    closest one, 6381956970095103 2^797, has a cosine of 4.7e-19, so that the
    value of the mathematical library, moved one double downward, still has
    the sign of the function. Next to 0 the sign is known from x. Rounding
    upward, after the GAOL_RND_ENTER() of the caller.
  */
  static inline int sign_of_sin(double x)
  {
    if (std::fabs(x) < 3.0) {
      return (x > 0.0) - (x < 0.0);
    }
    return (upward::sin_dn(x) > 0.0) ? 1 : -1;
  }

  static inline int sign_of_cos(double x)
  {
    if (std::fabs(x) < 1.5) {
      return 1;
    }
    return (upward::cos_dn(x) > 0.0) ? 1 : -1;
  }

  static double asin_lo(double x)
  {
    return (x == 0.0) ? 0.0 : ((x == 1.0) ? half_pi_dn : ((x == -1.0) ? -half_pi_up : upward::asin_dn(x)));
  }

  static double asin_hi(double x)
  {
    return (x == 0.0) ? 0.0 : ((x == 1.0) ? half_pi_up : ((x == -1.0) ? -half_pi_dn : upward::asin_up(x)));
  }

  static double acos_lo(double x)
  {
    return (x == 1.0) ? 0.0 : ((x == 0.0) ? half_pi_dn : ((x == -1.0) ? pi_dn : upward::acos_dn(x)));
  }

  static double acos_hi(double x)
  {
    return (x == 1.0) ? 0.0 : ((x == 0.0) ? half_pi_up : ((x == -1.0) ? pi_up : upward::acos_up(x)));
  }

  // pi/4 is half of pi/2, exactly
  static double atan_lo(double x)
  {
    if (x == 0.0) {
      return 0.0;
    }
    const double a = std::fabs(x);
    if (a == 1.0 || a == GAOL_INFINITY) {
      const double b = (a == 1.0) ? 0.5 : 1.0;
      return (x > 0.0) ? half_pi_dn*b : -half_pi_up*b;
    }
    return upward::atan_dn(x);
  }

  static double atan_hi(double x)
  {
    if (x == 0.0) {
      return 0.0;
    }
    const double a = std::fabs(x);
    if (a == 1.0 || a == GAOL_INFINITY) {
      const double b = (a == 1.0) ? 0.5 : 1.0;
      return (x > 0.0) ? half_pi_up*b : -half_pi_dn*b;
    }
    return upward::atan_up(x);
  }

  /*
    Bounds of atan2(y, x) at a point other than (0, 0) (GAOL v5): 0, pi/2,
    pi, their opposites and pi/4 where the angle is one of them, a bound being
    an infinity included, where atan2 has that limit; the value of the
    mathematical library moved outward elsewhere, within [-pi, pi]. The sign of
    a zero does not count: atan2(0, x) is pi for x < 0.
  */
  static double atan2_lo(double y, double x)
  {
    if (y == 0.0 || x == GAOL_INFINITY) {
      return (x > 0.0) ? 0.0 : pi_dn;
    }
    if (x == 0.0 || std::fabs(y) == GAOL_INFINITY) {
      return (y > 0.0) ? half_pi_dn : -half_pi_up;
    }
    if (x == -GAOL_INFINITY) {
      return (y > 0.0) ? pi_dn : -pi_up;
    }
    if (x == std::fabs(y)) {
      return (y > 0.0) ? half_pi_dn*0.5 : -half_pi_up*0.5;
    }
    return maximum(upward::atan2_dn(y,x), -pi_up);
  }

  static double atan2_hi(double y, double x)
  {
    if (y == 0.0 || x == GAOL_INFINITY) {
      return (x > 0.0) ? 0.0 : pi_up;
    }
    if (x == 0.0 || std::fabs(y) == GAOL_INFINITY) {
      return (y > 0.0) ? half_pi_up : -half_pi_dn;
    }
    if (x == -GAOL_INFINITY) {
      return (y > 0.0) ? pi_up : -pi_dn;
    }
    if (x == std::fabs(y)) {
      return (y > 0.0) ? half_pi_up*0.5 : -half_pi_dn*0.5;
    }
    return minimum(upward::atan2_up(y,x), pi_up);
  }

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
  static bool pow_is_double(double x, double y, double& d)
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
    x^y rounded upward for x >= 0 and a finite y, the rounding direction
    being upward: CORE-MATH's pow, which is correctly rounded in that
    direction, but for an exponent of extreme magnitude, |y| < 2^-969 or
    |y| >= 2^1014, and a finite x > 0 other than 1, whose power is computed
    here (GAOL v5). For those, the first phase of CORE-MATH's pow makes its
    approximation of log(x) a NaN on purpose (the exponent field of y below
    0x36 or from 0x7f5 on), so that its rounding test fails, and compares the
    NaN product with 2^-1022 with <, which raises the invalid-operation
    exception: pow([1, 2], [1e-300]) and pow([1, 2], [1e300, 1e308]) raised
    it, and killed a program that enabled it. The value is known there,
    |log(x)| being between 2^-54 and 745 for a positive double x other than 1:
    - |y| < 2^-969: 0 < |y log(x)| < 2^-959, and x^y is within 2^-958 of 1,
      above 1 where y log(x) > 0: x^y rounded upward is the double above 1
      there, and 1 otherwise;
    - |y| >= 2^1014: |y log(x)| > 2^960, and x^y is above DBL_MAX where
      y log(x) > 0, rounded upward to +oo, and below 2^-1074 otherwise,
      rounded upward to 2^-1074.
    These are the values CORE-MATH gives, and the double below each the lower
    bound it gives (pow_rounded_dn()). 1^y and x^0 are 1; CORE-MATH takes 0
    and +oo apart before its first phase.
  */
  static inline double pow_rounded_up(double x, double y)
  {
    // 2^-969 and 2^1014, written in decimal with the digits that give them
    // exactly: C++11 has no hexadecimal floating literal
    const double magnitude = std::fabs(y);
    if (magnitude >= 2.0041683600089728e-292 && magnitude < 1.7555597020139804e+305) {
      return upward::nthroot_up(x, y);
    }
    if (x == 1.0 || y == 0.0) {
      return 1.0;
    }
    if (x == 0.0 || x == GAOL_INFINITY) {
      return upward::nthroot_up(x, y);
    }
    const bool above_one = (x > 1.0) == (y > 0.0);
    if (magnitude < 1.0) {
      return above_one ? 1.0 + std::numeric_limits<double>::epsilon() : 1.0;
    }
    return above_one ? GAOL_INFINITY : std::numeric_limits<double>::denorm_min();
  }

  static inline double pow_rounded_dn(double x, double y)
  {
    return previous_float(pow_rounded_up(x, y));
  }

  // x^y for x > 0: 1 for x = 1 or y = 0, x^y where it is a double
  // (pow_is_double()), the double below CORE-MATH's value rounded upward
  // otherwise, and at least 0
  static inline double pow_lo(double x, double y)
  {
    double d;
    if (x == 1.0 || y == 0.0) {
      return 1.0;
    }
    return pow_is_double(x, y, d) ? d : maximum(pow_rounded_dn(x,y), 0.0);
  }

  static inline double pow_hi(double x, double y)
  {
    return (x == 1.0 || y == 0.0) ? 1.0 : pow_rounded_up(x,y);
  }

  static inline double sinh_lo(double x)
  {
    return (x == 0.0) ? 0.0 : ((x >= 711.0) ? std::numeric_limits<double>::max() : upward::sinh_dn(x));
  }

  static inline double sinh_hi(double x)
  {
    return (x == 0.0) ? 0.0 : ((x <= -711.0) ? -std::numeric_limits<double>::max() : upward::sinh_up(x));
  }

  // cosh(x), cosh(-x): the lower bound for |x|, the upper bound for 0
  static inline double cosh_lo(double x)
  {
    return (x == 0.0) ? 1.0 : ((std::fabs(x) >= 711.0) ? std::numeric_limits<double>::max() : upward::cosh_dn(x));
  }

  static inline double cosh_hi(double x)
  {
    return (x == 0.0) ? 1.0 : upward::cosh_up(x);
  }

  // 1 - 2^-53, the double below 1, exactly
  static inline double tanh_lo(double x)
  {
    return (x == 0.0) ? 0.0 : ((x >= 20.0) ? 1.0 - 0.5*std::numeric_limits<double>::epsilon() : upward::tanh_dn(x));
  }

  static inline double tanh_hi(double x)
  {
    return (x == 0.0) ? 0.0 : ((x <= -20.0) ? -(1.0 - 0.5*std::numeric_limits<double>::epsilon()) : upward::tanh_up(x));
  }

  static inline double asinh_lo(double x) { return (x == 0.0) ? 0.0 : upward::asinh_dn(x); }
  static inline double asinh_hi(double x) { return (x == 0.0) ? 0.0 : upward::asinh_up(x); }
  static inline double acosh_lo(double x) { return (x == 1.0) ? 0.0 : upward::acosh_dn(x); }
  static inline double acosh_hi(double x) { return (x == 1.0) ? 0.0 : upward::acosh_up(x); }
  static inline double atanh_lo(double x) { return (x == 0.0) ? 0.0 : upward::atanh_dn(x); }
  static inline double atanh_hi(double x) { return (x == 0.0) ? 0.0 : upward::atanh_up(x); }

  /*
    \brief test for evenness
    \warning d should not be +/-oo
  */
  bool feven(double d)
  {
    // A subnormal is no integer, which its bits tell whatever the modes that
    // flush the subnormals to zero: under denormals-are-zero, 0.5*d and d
    // were 0 for the comparison, and feven(2^-1074) was true (GAOL v5,
    // point Q of TODO.md)
    if (gaol_detail::bound_is_subnormal(d)) {
      return false;
    }
    return (std::floor(0.5*d)*2.0 == d);
  }


  // The bounds compared as the relations compare them, whatever the modes
  // that flush the subnormals to zero: under denormals-are-zero,
  // [2^-1074, 3*2^-1074] was canonical (GAOL v5, point Q of TODO.md)
  bool interval::is_canonical(void) const
  {
#if defined (_MSC_VER)
    return !is_empty() && gaol_detail::bound_greater_equal(next_float(left()), right());
#else
    // emptyset handled thanks to unorderedness of NaNs, with a quiet
    // comparison, which raises no invalid-operation exception on them (GAOL v5)
    return gaol_detail::bound_greater_equal(next_float(left()),right());
#endif
  }


  void interval::format(interval_format::format_t f)
  {
    output = f;
  }

  interval_format::format_t interval::format(void)
  {
    return output;
  }


  // Sets the failbit of is, without the std::ios_base::failure setstate()
  // throws when the exceptions of is include failbit: the error of a line
  // that is no interval is GAOL's exception. The bit stays set.
  static void set_failbit(istream& is)
  {
    try {
      is.setstate(std::ios_base::failbit);
    } catch (const std::ios_base::failure&) {
      // failbit is set
    }
  }

  /*
    Reads an interval from a line, after the blanks that precede it, line ends
    included, which std::ws skips as the reading of a double does, whatever
    std::noskipws says (GAOL v5). GAOL read the line where the previous value
    stopped: over "1.5\n[1, 2]\n", "in >> d >> x" read the empty rest of the
    first line, and threw input_format_error, as while (in >> x) did over a
    file ending with an empty line. A blank line is now no line to read, and
    the intervals around it are read, as two numbers with a blank line
    between them are.

    Where no line is left, std::getline() fails, which sets failbit: I is left
    as it was and nothing is thrown, as for a double, so that while (is >> x)
    ends at the end of the input (GAOL v5). GAOL read the empty text then,
    threw input_format_error and emptied I, so that such a loop always ended
    with an exception. A line that is no interval sets failbit too, I becomes
    the empty set, and the exception of the reader is thrown:
    input_format_error, or invalid_action_error for a function called with an
    argument it does not take. A program reading on calls is.clear() first.

    std::ws is no extraction: it constructs no sentry, so that it neither
    flushes the stream tied to is nor looks at the state of is, and libstdc++
    reads the buffer whatever that state is. Used alone, it would crash on an
    istream without buffer, consume the blanks of a stream that has failed,
    and leave a prompt written on cout unflushed until the user had typed the
    interval. The sentry of any extraction is therefore constructed first,
    without skipping (std::getline() does the same): it flushes the tied
    stream, and sets failbit where is is not good.
  */
  istream& operator >>(istream& is, interval& I)
  {
    std::string buffer;

    if (!istream::sentry(is,true)) {
      return is;
    }
    if (!std::getline(is >> std::ws,buffer)) {
      return is;
    }

    bool read;
    try {
      read = gaol::parse_interval(buffer.c_str(),I);
    } catch (...) { // An error the reader raised as it read the line
      I = interval::emptyset();
      set_failbit(is);
      throw;
    }
    if (!read) {
      std::string err_msg("Syntax error in expression of interval: ");
      err_msg += buffer;
      I = interval::emptyset();
      set_failbit(is);
      gaol_ERROR(input_format_error,err_msg.c_str());
    }
    return is;
  }

  /*
    Moves the last digit of text by one unit, away from zero or toward it,
    until the number it writes is on the side of the double `magnitude` asked
    for: at least it if `away`, at most it otherwise. text is a positive
    number written to nearest in the fixed format, or in the scientific one if
    `scientific`: one digit, the others after a point, and an exponent with
    its sign.
  */
  static void move_to_side(std::string& text, double magnitude, bool away, bool scientific)
  {
    for (int moves = 0; moves < 4; ++moves) {
      const std::size_t last = scientific ? text.find_first_of("eE") : text.size(); // One past the mantissa
      const int sign = gaol::compare_number_with_double(text.c_str(), magnitude);
      if (sign == 0 || (sign > 0) == away) {
        return;
      }
      std::size_t i = last;
      bool carry = true;
      while (carry && i > 0) {
        --i;
        if (text[i] == '.') {
          continue;
        }
        if (away) {
          carry = (text[i] == '9');
          text[i] = carry ? '0' : static_cast<char>(text[i] + 1);
        } else {
          carry = (text[i] == '0');
          text[i] = carry ? '9' : static_cast<char>(text[i] - 1);
        }
      }
      int exponent_move = 0;
      if (away && carry) {
        // 9.99 has become 0.00: 1.00 with the next exponent, or 10.00
        if (scientific) {
          text[0] = '1';
          exponent_move = 1;
        } else {
          text.insert(0, 1, '1');
        }
      } else if (!away && scientific && text[0] == '0') {
        // 1.00 has become 0.99: 9.99 with the previous exponent, which is
        // below the magnitude as well, 1.00 being it rounded to nearest
        text[0] = '9';
        exponent_move = -1;
      } else if (!away && text[0] == '0' && text.size() > 1 && text[1] != '.') {
        text.erase(0, 1); // 0999.99
      }
      if (exponent_move != 0) {
        const long e = std::strtol(text.c_str() + last + 1, NULL, 10) + exponent_move;
        std::ostringstream exponent;
        exponent << ((e < 0) ? '-' : '+') << std::setw(2) << std::setfill('0') << ((e < 0) ? -e : e);
        text.replace(last + 1, std::string::npos, exponent.str());
      }
    }
  }

  /*
    What GAOL takes from a stream to write a number: its flags, its precision
    and its locale, whose facet numpunct gives the decimal point and, if
    `grouped`, the grouping of the digits before it (GAOL v5). The text of an
    interval is made from these in a character string, rather than in
    std::ostringstreams, one for each interval and one for each bound: each
    built a locale and its cached facets, and destroyed them, a third of the
    work of writing an interval.
  */
  namespace {
  struct text_format {
    std::ios_base::fmtflags flags;
    std::streamsize precision;
    std::locale loc;
    const std::numpunct<char>& punct; // The facet of loc, which the copy keeps
    bool grouped;

    text_format(std::ios_base::fmtflags f, std::streamsize p, const std::locale& l, bool g = false)
      : flags(f), precision(p), loc(l), punct(std::use_facet<std::numpunct<char> >(loc)), grouped(g)
    {
    }
  };

  // How the text of a number is rounded: to nearest, as a stream writes a
  // double, or downward or upward for the bound of an interval
  enum text_rounding { text_nearest, text_downward, text_upward };
  } // namespace

  /*
    x written by a stream that has the flags, the precision and the locale of
    fmt, under the rounding direction asked: what number_to_text() leaves to
    the C library and the facets of the locale, whatever they are, as it
    cannot write it as the stream does. That is a NaN, the hexadecimal
    floating-point format, and a conversion of the C library that fails.
  */
  static std::string stream_text(double x, text_rounding rounding, const text_format& fmt)
  {
    std::ostringstream out;
    out.flags(fmt.flags);
    out.precision(fmt.precision);
    out.imbue(fmt.loc);
    // Zero has no digit to round, and the C runtime of Windows, asked to write
    // it in the upward direction, writes 0.1 (the point interval [0, 0] came
    // out as <0.0, 0.1>): it is written to nearest. The other cases keep the
    // direction of the bound, the hexadecimal format being rounded by the C
    // library when the stream limits its digits.
    if (x == 0.0 || rounding == text_nearest) {
      round_nearest();
    } else if (rounding == text_downward) {
      round_downward();
    }
    out << x;
    round_upward();
    return out.str();
  }

  /*
    The magnitude of a double written by the C library with `prec` digits after
    the point, in the fixed format, or in the scientific one, rounded to
    nearest: the same digits as the stream writes. printf follows the decimal
    point of the C locale, a comma under some, where a stream of C++ follows
    its own locale: the point is written as a '.' whatever the locale, from
    its position among the digits, which the precision gives.
  */
  static bool magnitude_text(std::string& text, double magnitude, bool fixed, int prec)
  {
    char buffer[512];
    const char *const format = fixed ? "%.*f" : "%.*e";
    round_nearest();
    int n = std::snprintf(buffer, sizeof buffer, format, prec, magnitude);
    if (n >= 0 && static_cast<std::size_t>(n) < sizeof buffer) {
      text.assign(buffer, static_cast<std::size_t>(n));
    } else if (n >= 0) {
      text.assign(static_cast<std::size_t>(n) + 1, '\0');
      n = std::snprintf(&text[0], text.size(), format, prec, magnitude);
      text.resize((n >= 0) ? static_cast<std::size_t>(n) : 0);
    }
    round_upward();
    if (n < 0 || text.empty()) {
      return false;
    }
    if (prec > 0) {
      // 3.14e+00 or 3.14: the point is where the digits before it end, and it
      // ends where the digits after it, of the precision, begin
      const std::size_t point = fixed ? text.find_first_not_of("0123456789") : 1;
      const std::size_t digits_end = fixed ? text.size() : text.find('e');
      if (point == std::string::npos || digits_end == std::string::npos
          || digits_end < point + static_cast<std::size_t>(prec) + 1) {
        return false;
      }
      const std::size_t length = digits_end - static_cast<std::size_t>(prec) - point;
      if (length != 1 || text[point] != '.') {
        text.replace(point, length, 1, '.');
      }
    }
    return true;
  }

  /*
    The digits text starts with, those before its decimal point, grouped as
    std::num_put groups those of a double: from the point leftward, each group
    as long as its character of grouping() says, the last one repeated, and
    no group past a size of 0, a negative one or CHAR_MAX, which the standard
    takes as unlimited (22.4.3.1.2). libstdc++ differs there for a facet a
    program defines: it repeats the size before a 0 ("\3\0" groups 1234567 as
    1,234,567, where libc++ and GAOL write 1234,567), and takes the sizes 128
    to 254 for negative ones where char is unsigned (ARM, ppc64le, s390x).
  */
  static void group_digits(std::string& text, const std::numpunct<char>& punct)
  {
    const std::string grouping = punct.grouping();
    std::size_t pos = text.find_first_not_of("0123456789");
    if (pos == std::string::npos) {
      pos = text.size();
    }
    std::size_t g = 0;
    while (g < grouping.size()) {
      const char size = grouping[g];
      if (size <= 0 || size == CHAR_MAX || pos <= static_cast<std::size_t>(size)) {
        break;
      }
      pos -= static_cast<std::size_t>(size);
      text.insert(pos, 1, punct.thousands_sep());
      if (g + 1 < grouping.size()) {
        ++g;
      }
    }
  }

  /*
    The text of a number x written as a stream with the flags, the precision
    and the locale of fmt would write it, and, for the bound of an interval,
    rounded downward, or upward, whatever the C library does. GAOL set the
    rounding direction and let the C library write x, but not every C library
    rounds its decimal conversions in the rounding direction: the C runtime of
    Windows rounds the magnitude, so that -2/3 rounded downward was written
    -0.6666, and musl on 64-bit ARM processors rounds to nearest whatever the
    direction (issue #3). The magnitude is now written rounding to nearest, in
    the fixed format or in the scientific one, and compared exactly with x:
    when it is on the wrong side of x, its last digit is moved by one. The
    general format is made from the scientific one, by the rules of printf's
    %g, rather than asked of the C library: glibc 2.31 writes 999999.5 with six
    digits and the zeros kept "1.e+06", whose last digit is not the sixth.
    The text of a zero, or of an infinity, is made the same way, without
    the comparison: "inf" is "INF" with the flag uppercase, but in the fixed
    format, whose conversion the standard has as %f (a stream of libc++, which
    uses %F, wrote "INF" there). With fmt.grouped, the
    digits before the point are grouped as the locale has them, as a stream
    groups those of a double: the midpoint and the radius of the width and
    center formats, both alike (GAOL v5: a stream wrote the midpoint, grouped,
    and GAOL the radius, which was not). The other texts are never grouped.
    Left to a stream, with the rounding direction set (stream_text()): what
    the text cannot be made of.
  */
  static std::string number_to_text(double x, text_rounding rounding, const text_format& fmt)
  {
    const std::ios_base::fmtflags floatfield = fmt.flags & std::ios_base::floatfield;
    if (!(x == x) || floatfield == (std::ios_base::fixed | std::ios_base::scientific)) {
      return stream_text(x, rounding, fmt);
    }
    // A subnormal under denormals-are-zero, which a program may set, compares
    // equal to 0: it is written by a stream, to nearest, as GAOL wrote it
    // before and as tests/numbers.cpp checks against a stream, where the
    // snprintf called below wrote 0 for 5e-324 with MSYS2 CLANG64 and the
    // stream did not. operator<< and intervalToText() clear that mode before
    // they write (GAOL v5, point Q of TODO.md), where GAOL can (x86, and ARM
    // with GCC and Clang): only where it cannot does a subnormal come here
    std::uint64_t bits;
    std::memcpy(&bits, &x, sizeof bits);
    if (x == 0.0 && (bits << 1) != 0) {
      return stream_text(x, text_nearest, fmt);
    }

    const bool fixed = (floatfield == std::ios_base::fixed);
    const bool general = (floatfield == 0);
    const bool showpoint = ((fmt.flags & std::ios_base::showpoint) != 0);
    std::string text;
    if (x == GAOL_INFINITY || x == -GAOL_INFINITY) {
      text = ((fmt.flags & std::ios_base::uppercase) && !fixed) ? "INF" : "inf";
    } else {
      // A stream writes 6 digits for a negative precision: so are the numbers
      // written to nearest and the zeros. A bound that is not 0 takes it as 0
      // in the general format, 1 digit, and as 6 in the fixed and scientific
      // ones, as the stream that wrote it did
      const bool as_stream = (rounding == text_nearest || x == 0.0);
      const std::streamsize precision = (as_stream && fmt.precision < 0) ? 6 : fmt.precision;
      // %g takes a precision of 0 as 1, and writes that many significant digits
      const std::streamsize digits = (general && precision <= 0) ? 1 : precision;
      std::streamsize prec = general ? digits - 1 : precision;
      if (prec < 0) {
        prec = 6;
      } else if (prec > std::numeric_limits<int>::max() / 4) {
        prec = std::numeric_limits<int>::max() / 4;
      }
      const double magnitude = std::fabs(x);
      if (!magnitude_text(text, magnitude, fixed, static_cast<int>(prec))) {
        // snprintf failed (a precision its buffer cannot hold), or its point
        // was not found: the digits of a stream of the C locale, to nearest,
        // as GAOL wrote them before, then moved outward as above. Never those
        // of the C library under the directed rounding, which the C runtime of
        // Windows and musl do not honour (issue #3)
        std::ostringstream out;
        out.imbue(std::locale::classic());
        out.setf(fixed ? std::ios_base::fixed : std::ios_base::scientific, std::ios_base::floatfield);
        out.precision(prec);
        round_nearest();
        out << magnitude;
        round_upward();
        text = out.str();
      }
      // The magnitude written has to be at least that of x when rounding away
      // from zero, and at most that of x otherwise
      if (rounding != text_nearest && x != 0.0) {
        move_to_side(text, magnitude, (rounding == text_upward) == (x > 0.0), !fixed);
      }

      if (general) {
        // d.ddde+XX as %g writes it: without an exponent when XX is from -4 to
        // the precision, and without the zeros ending its fractional part,
        // unless showpoint
        const std::size_t e = text.find('e');
        const long exponent = std::strtol(text.c_str() + e + 1, NULL, 10);
        std::string mantissa = text.substr(0, e);
        mantissa.erase(1, (mantissa.size() > 1) ? 1 : 0); // The digits
        std::string tail;
        if (exponent < -4 || exponent >= digits) {
          tail = text.substr(e);
          mantissa.insert(1, 1, '.');
        } else if (exponent >= 0) {
          mantissa.insert(static_cast<std::size_t>(exponent) + 1, 1, '.');
        } else {
          mantissa.insert(0, "0." + std::string(static_cast<std::size_t>(-exponent - 1), '0'));
        }
        if (!showpoint) {
          mantissa.erase(mantissa.find_last_not_of('0') + 1);
          if (mantissa[mantissa.size() - 1] == '.') {
            mantissa.erase(mantissa.size() - 1);
          }
        }
        text = mantissa + tail;
      } else if (precision == 0 && showpoint) {
        text.insert(fixed ? text.size() : text.find('e'), 1, '.');
      }

      const char point = fmt.punct.decimal_point();
      for (std::size_t i = 0; i < text.size(); ++i) {
        if (text[i] == '.') {
          text[i] = point;
        } else if (text[i] == 'e' && (fmt.flags & std::ios_base::uppercase)) {
          text[i] = 'E';
        }
      }
      // After the point is in place: the separator of the groups may be '.'
      if (fmt.grouped) {
        group_digits(text, fmt.punct);
      }
    }
    if (std::signbit(x)) { // -0 is written -0, as a stream does
      text.insert(0, 1, '-');
    } else if (fmt.flags & std::ios_base::showpos) {
      text.insert(0, 1, '+');
    }
    return text;
  }

  static std::string bound_to_text(double x, bool upward, const text_format& fmt)
  {
    return number_to_text(x, upward ? text_upward : text_downward, fmt);
  }

  /*
    The interval [l, r] as two bounds rounded outward, [l, r], or as [a], the
    literal of IEEE 1788-2015 for a point (12.11), for a point interval whose
    double the text writes exactly (GAOL v5). The text of the lower bound being
    at most it, and that of the upper bound at least it, two equal texts are
    the double itself, which the reader reads back as the point; the other
    point intervals are written as any other interval, [0.1, 0.1000000000000001]
    for interval(0.1), which is read back as an interval enclosing it. A zero
    is written exactly, and a point interval of zero, [-0, +0], [+0, +0] or
    [-0, -0], is written [0], the text of +0: the three are the set {0}, which
    [0] and [-0] are both read back as, and the text of interval::zero() is
    then the same whatever the build ([-0, +0] with the SSE2 intervals,
    [+0, +0] with the FPU ones); the hexadecimal format writes the signs of the
    bounds. The bounds are compared by their bits: under denormals-are-zero,
    which a program may set, a subnormal compares equal to 0, and l == r and
    l == 0.0 would have [0, 5e-324] written [0]. (Under that mode,
    bound_to_text() takes a subnormal bound for 0 as well, and writes it
    rounded to nearest rather than outward, so that two equal texts need not
    be the double: with 1 digit, [22u] (u = 5e-324) was written [1e-322],
    which is read as [20u, 21u]. operator<< and intervalToText() clear that
    mode before they write, where GAOL can: GAOL v5, point Q of TODO.md.)
    fmt has the C locale, whose decimal point is '.', the reader's: operator<<
    writes the bounds format with it whatever the locale of the stream (GAOL
    v5). Under a locale writing a decimal comma, the reader took the comma of
    [0,5, 2,5] for the one between two bounds and refused the text, and would
    have read [-2,5] for interval(-2.5) as the interval [-2, 5], [12,5] as the
    empty set and [0,] (a zero in the fixed format with the showpoint flag and
    no digit) as [0, +oo]; GAOL wrote that point [-2,5, -2,5], which it
    refused. A point is still written alone between the brackets only when
    the decimal point of fmt is '.'.
    GAOL wrote every point interval <a, b>, the text of its double rounded
    downward and upward, which the reader takes for two numbers that are the
    same double only: <0.1, 0.1000000000000001> for interval(0.1) was refused,
    and textToInterval(intervalToText(interval(0.1))) was the empty set. The
    reader still takes the angles, which are no literal of the standard.
  */
  static void display_bounds(double l, double r, std::string& out, const text_format& fmt)
  {
    // A quiet comparison, which raises no invalid-operation exception on the
    // NaN bounds of the empty set (GAOL v5)
    if (!gaol_detail::quiet_less_equal(l, r)) {
      out += "[empty]";
    } else {
      std::uint64_t lbits, rbits;
      std::memcpy(&lbits, &l, sizeof lbits);
      std::memcpy(&rbits, &r, sizeof rbits);
      const bool zero = ((lbits << 1) == 0 && (rbits << 1) == 0); // +0 or -0, both
      const std::string left = bound_to_text(zero ? 0.0 : l, false, fmt);
      const std::string right = bound_to_text(zero ? 0.0 : r, true, fmt);
      out += '[';
      out += left;
      if (!((zero || (lbits == rbits && left == right)) && fmt.punct.decimal_point() == '.')) {
        out += ", ";
        out += right;
      }
      out += ']';
    }
  }


  /*
    A bound written in the hexadecimal-significand form of IEEE 1788-2015
    (13.4.1), which textToInterval() reads back exactly (GAOL v5).

    The recovery requirement of 13.4 asks that writing an interval and reading
    it again give the same bounds. GAOL wrote the sixteen hexadecimal digits of
    each double instead, which is no interval literal at all: the parser
    refused them, and the note of 13.4.1 gives that very form as the one that
    fails the readability test. The form is the one printf("%a") of the glibc
    writes: as many digits as the value needs, "0x1.999999999999ap-4",
    "0x0.0000000000001p-1022" for a subnormal, "0x0p+0" for zero, and the
    lexer reads it.
    It is written from the bits of the double rather than by printf("%a"),
    which follows the decimal point of the C locale: under a locale writing a
    decimal comma, it wrote "0x1,999999999999ap-4", which cannot be read
    back (GAOL v5).
  */
  static void write_hexa_bound(double x, char *buf, std::size_t n)
  {
    if (x == GAOL_INFINITY) {
      std::snprintf(buf, n, "inf");
      return;
    }
    if (x == -GAOL_INFINITY) {
      std::snprintf(buf, n, "-inf");
      return;
    }
    std::uint64_t bits;
    std::memcpy(&bits, &x, sizeof bits);
    const int biased = static_cast<int>((bits >> 52) & 0x7ff);
    std::uint64_t fraction = bits & ((static_cast<std::uint64_t>(1) << 52) - 1);
    // 1.f 2^(e-1023) for a normal double, 0.f 2^-1022 for a subnormal one, 0
    const int leading = (biased == 0) ? 0 : 1;
    const int exponent = (biased == 0) ? ((fraction == 0) ? 0 : -1022) : biased - 1023;
    // The 13 hexadecimal digits of the fraction, without the zeros ending them
    char digits[14];
    for (int i = 12; i >= 0; --i, fraction >>= 4) {
      digits[i] = "0123456789abcdef"[fraction & 0xf];
    }
    int nb_digits = 13;
    while (nb_digits > 0 && digits[nb_digits - 1] == '0') {
      --nb_digits;
    }
    digits[nb_digits] = '\0';
    std::snprintf(buf, n, "%s0x%d%s%sp%+d", (bits >> 63) ? "-" : "", leading,
                  (nb_digits > 0) ? "." : "", digits, exponent);
  }

  /*
    A point interval is written [a], as in the decimal format, and a point
    interval of zero [0x0p+0], whatever the signs of its bounds, the three
    being the set {0} (GAOL v5: it was written with its two bounds,
    [0x1p+2, 0x1p+2], and [-0x0p+0, 0x0p+0] for interval::zero() with the
    SSE2 intervals). The bounds are compared by their bits, which
    denormals-are-zero does not change.
  */
  std::string exact_string(const interval& I)
  {
    if (I.is_empty()) {
      return "[empty]";
    }
    const double l = I.left(), r = I.right();
    std::uint64_t lbits, rbits;
    std::memcpy(&lbits, &l, sizeof lbits);
    std::memcpy(&rbits, &r, sizeof rbits);
    char lo[64], hi[64];
    if ((lbits << 1) == 0 && (rbits << 1) == 0) {
      return "[0x0p+0]";
    }
    write_hexa_bound(l, lo, sizeof lo);
    if (lbits == rbits) {
      return std::string("[") + lo + "]";
    }
    write_hexa_bound(r, hi, sizeof hi);
    return std::string("[") + lo + ", " + hi + "]";
  }

  /*
    text into os as one item, padded to the width of os with its fill, to the
    left, to the right, or inside (std::internal). os pads a string to the left
    or to the right only: for std::internal, the fill is put where a number of
    os has it (GAOL v5).
  */
  static void write_text(ostream& os, std::string& text)
  {
    // std::internal puts the fill after the sign of a number and its 0x
    // prefix, before its digits. The text of the width and center formats
    // starts with the sign of the midpoint, if it has one, and that of the
    // agreeing digits with the sign of the bounds, and the fill goes after
    // it, as std::internal padded the midpoint alone before the interval was
    // written as a whole. The other texts start with '[', before which the
    // fill goes, as with std::right, and std::complex writes its parentheses.
    const std::streamsize width = os.width();
    if ((os.flags() & std::ios_base::adjustfield) == std::ios_base::internal
        && width > static_cast<std::streamsize>(text.size())) {
      std::size_t at = (!text.empty() && (text[0] == '-' || text[0] == '+')) ? 1 : 0;
      if (text.size() > at + 1 && text[at] == '0' && (text[at + 1] == 'x' || text[at + 1] == 'X')) {
        at += 2;
      }
      if (at > 0) {
        text.insert(at, static_cast<std::size_t>(width) - text.size(), os.fill());
      }
    }
    os << text;
  }

  /*
    The text is made in a character string, from the flags and the locale of
    os and the precision of interval::precision(), then written into os at once
    (GAOL v5): os keeps its precision, and its width (std::setw) and
    adjustment apply to the whole interval. GAOL set the precision of os to
    interval::precision() and left it so, the doubles written afterwards
    getting 16 digits, and wrote the interval piece by piece, std::setw
    padding its '[' only.

    The text is not written in a std::ostringstream of its own, whose locale
    and cached facets, built and destroyed for each interval and each bound,
    took a third of the work of writing an interval. os is only read, and is
    never given another precision or width for the time of the writing: the
    threads of a program writing to std::cout at once would race on them.
  */
  ostream& operator<<(ostream& os, const interval& I)
  {
    //    double l = ((I.left()==0.0) ? 0.0  : I.left()); // Avoids printing -0
    //    double r = ((I.right()==0.0) ? 0.0 : I.right());  // Avoids printing -0
    std::string text;

    // The guard, rather than GAOL_RND_PRESERVE() and GAOL_RND_RESTORE(): the
    // construction of the text may fail (std::bad_alloc), and a program
    // reading the exception found the direction left as this function had
    // set it, to nearest (GAOL v5). The check of the direction, as in
    // intervalToText(), clears the modes that flush the subnormals to zero,
    // which the guard sets back with GAOL_PRESERVE_ROUNDING: under
    // denormals-are-zero, a subnormal bound was 0 for the comparisons and
    // was written to nearest, [22*2^-1074] as [1e-322] with 1 digit, and the
    // C library writes it 0 (gdtoa of FreeBSD and macOS, the Debug runtime
    // of Visual C++, issue #68; GAOL v5, point Q of TODO.md)
    const rounding_guard rnd;
	round_upward_if_needed();

    double l = I.left(), r = I.right();
    const interval_format::format_t format = interval::format();

    switch (format) {
    case interval_format::bounds: { // Display in the form "[ l, r ]"
      // The bounds format, and the agreeing format where it writes bounds,
      // in the C locale, whose decimal point is the reader's (GAOL v5); the
      // width and center formats, which are for the eye, in the locale of os
      const text_format fmt(os.flags(), interval::precision(), std::locale::classic());
      display_bounds(l,r,text,fmt);
      break;
    }
    case interval_format::hexa: // The exact text representation of 13.4
      text = exact_string(I);
      break;
    case interval_format::width:  // Display in the form "c (+/- w)"
    case interval_format::center: // Display in the form "c"
      if (I.is_empty()) {
        text = "[empty]";
      } else {
        /*
          The midpoint c and the radius w of IEEE 1788-2015 (12.12.8),
          midpoint() and rad() (GAOL v5): [c-w, c+w] contains the interval, w
          being the smallest double that makes it so, and +oo for an
          unbounded interval, whose midpoint is 0 or plus or minus the
          largest double. c is written rounded to nearest, and w upward, so
          that a radius that is not 0 is never written 0. GAOL wrote (l+r)/2
          and (r-l)/2, both rounded to nearest: c and w did not contain the
          interval ([1, 1+2^-52] was "1 (+/- 1.11e-16)"), w was 0 for
          [0, 5e-324], l+r overflowed for [1e308, 1.7e308], and the center of
          a point interval was written rounded upward (0.1 was
          0.1000000000000001).
          A point interval has no radius: it is written as its center, which
          tells it from an interval too narrow for the digits of its center.
          These formats are for the eye: c has the digits of the precision, so
          that c (+/- w) written need not contain the interval, where the
          bounds format does. A radius that made the digits of c enclose it
          would show 0.1 (+/- 5.6e-18) for the point interval 0.1, and a
          radius worth the resolution of the digits of c for any interval
          narrower than that: the formats could no longer show that an
          interval is narrower than its digits (the manual, Output format).
        */
        // The center format writes no radius: midpoint() alone
        double c, w = 0.0;
        if (format == interval_format::width) {
          I.mid_rad(c, w);
        } else {
          c = I.midpoint();
        }
        // The decimal point of the locale of os, and its grouping of the
        // digits, for c and w alike; a midpoint 0 is written 0, as the bounds
        // format writes a zero, where midpoint() gives -0 for
        // [-2^-1073, 2^-1074]: its bits, which denormals-are-zero leaves alone
        const text_format shown(os.flags(), interval::precision(), os.getloc(), true);
        std::uint64_t cbits;
        std::memcpy(&cbits, &c, sizeof cbits);
        if ((cbits << 1) == 0) {
          c = 0.0;
        }
        text = number_to_text(c, text_nearest, shown);
        if (format == interval_format::width && w != 0.0) {
          // The radius is never negative, and has no sign under showpos,
          // which is for the midpoint: "+2 (+/- 1)" (GAOL v5: "+2 (+/- +1)")
          text_format radius(shown);
          radius.flags &= ~std::ios_base::showpos;
          text += " (+/- ";
          text += bound_to_text(w, true, radius);
          text += ')';
        }
      }
      break;
    case interval_format::agreeing:
      if (I.is_empty()) {
        text = "[empty]";
      } else {
        // Each bound is written with the digits of the precision and the flag
        // showpoint, in the locale of the program, as a stream of its own,
        // whose flags are the ones a stream starts with, would write it: the
        // flags and the locale of os do not apply here
        const text_format shown(std::ios_base::skipws | std::ios_base::dec | std::ios_base::showpoint,
                                interval::precision(), std::locale());
        const char point = shown.punct.decimal_point();
        const std::string lb = bound_to_text(l, false, shown), rb = bound_to_text(r, true, shown);
        std::size_t i = 0;
        while (i < lb.length() && i < rb.length() && lb[i] == rb[i]) {
          ++i;
        }
        /*
          The digits both bounds start with are written once, before the rest
          of each, only where they line up and say something: two bounds
          finite, not 0 and of the same sign, written with the same number of
          digits before the point and the same exponent, that share their
          first digit that is not 0. The other intervals are written as in
          the bounds format (GAOL v5). GAOL tested r > 10 l, true for every
          interval with a negative bound or 0, which were all written with
          their bounds, and false for [1, 10], written 1~[., 0.], the 1 of 10
          taken for the 1 of 1, or for [1, 2], written ~[1., 2.] with no
          digit shared, and for zeros, written ~[-0., 0.] for
          interval::zero() with the SSE2 intervals where the bounds format
          writes [0].
        */
        std::uint64_t lbits, rbits;
        std::memcpy(&lbits, &l, sizeof lbits);
        std::memcpy(&rbits, &r, sizeof rbits);
        const std::size_t le = lb.find_first_of("eE"), re = rb.find_first_of("eE");
        const bool line_up = (lbits << 1) != 0 && (rbits << 1) != 0
          && l > -GAOL_INFINITY && r < GAOL_INFINITY && std::signbit(l) == std::signbit(r)
          && lb.find(point) == rb.find(point)
          && ((le == std::string::npos) ? std::string() : lb.substr(le))
             == ((re == std::string::npos) ? std::string() : rb.substr(re))
          && lb.find_first_of("123456789") < i;
        if (!line_up) {
          const text_format fmt(os.flags(), interval::precision(), std::locale::classic());
          display_bounds(l,r,text,fmt);
        } else {
          // The characters both bounds start with, then what is left of each,
          // without the zeros ending it (GAOL v5: GAOL dropped from both
          // bounds the characters after the last one of the left bound that is
          // not a zero, and wrote [1.25, 1.2567] "1.25~[, ]"; zeros ending an
          // exponent are not dropped)
          text = lb.substr(0, i);
          if (i < lb.length() || i < rb.length()) {
            const std::string *bounds[2] = { &lb, &rb };
            text += "~[";
            for (int k = 0; k < 2; ++k) {
              const std::string& b = *bounds[k];
              std::size_t end = b.length();
              if (b.find_first_of("eE") == std::string::npos && b.find(point) != std::string::npos) {
                while (end > i && end > b.find(point) + 1 && b[end - 1] == '0') {
                  --end;
                }
              }
              text += (end > i) ? b.substr(i, end - i) : std::string("0");
              text += (k == 0) ? ", " : "]";
            }
          }
        }
      }
    }
    // No double is computed from here on: the text is written before the
    // guard sets the direction back
    write_text(os, text);
    return os;
  }




	interval gaol_pown(const interval& I, int n)
	{
		if (I.is_empty()) {
			return I;
		}
		if (n < 0) {
			/*
			  x^-m is 1/x^m, unless x^m overflows: then (1/x)^m (GAOL v5).
			  GAOL computed 1/x^m, whose x^m overflowed for some |x| > 1 before
			  the inversion: pow([10],-400) was [0,5.6e-309] rather than
			  [0,2^-1074], and pow([2],-1050) [0,2^-1024] rather than [2^-1050].
			  1/x is an interval where I does not straddle 0; where it does,
			  x^-m is [-oo,+oo] for an odd m, and [mag(I)^-m,+oo] for an even m.

			  The rounding direction is checked once, here (GAOL v5): the powers
			  and the inverses are computed by the bodies of uipow_nonempty() and
			  inverse(), which do not check it again, where GAOL called them and
			  gaol_uipow(), which checked it two to five times. Each interval
			  given to inverse_upward() is non-empty: a finite power, [g, g] for
			  g > 0, I.
			*/
			const unsigned int m = 0u - static_cast<unsigned int>(n);
			const double largest = std::numeric_limits<double>::max();
			GAOL_RND_ENTER();
			interval res = uipow_nonempty_upward(I,m);
			if (res.left() >= -largest && res.right() <= largest) {
				res = interval::inverse_upward(res);
			} else if (I.left() < 0.0 && I.right() > 0.0) {
				if (odd(m)) {
					res = interval::universe();
				} else {
					const double g = I.mag();
					if (g == GAOL_INFINITY) {
						res = interval::positive();
					} else {
						res = interval(uipow_nonempty_upward(interval::inverse_upward(interval(g)),m).left(),GAOL_INFINITY);
					}
				}
			} else {
				// gaol_uipow() gave the empty set back as it is
				const interval J = interval::inverse_upward(I);
				res = J.is_empty() ? J : uipow_nonempty_upward(J,m);
			}
			GAOL_RND_KEEP(res);
			GAOL_RND_LEAVE();
			return res;
		} else {
			if (n > 0) {
				return uipow_nonempty(I,static_cast<unsigned int>(n));
			} else {
				return interval(1.0);
			}
		}
	}

  /*
    The pow of IEEE 1788-2015 (Table 9.1) for an interval exponent, written
    once (GAOL v5): gaol_ieee1788::pow(x, y) is this function, after the
    check of pow_standard(), and gaol_pow_hybrid() calls it for every exponent
    but a degenerate integer, for which it takes pown. This was the second half
    of gaol_pow_hybrid(), which gaol_ieee1788::pow went through after making
    its checks again: x cut to [0,+oo], the empty sets, x = {0}.
  */
  static interval pow_standard_upward(const interval& x, const interval& y)
  {
    if (x.is_empty() || y.is_empty()) {
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
    // c), which exp(J*log([0])) does not give, log([0]) being empty. interval(0.0)
    // rather than interval::zero(), whose lower bound is -0 with SSE2 intervals
    if (base.right() == 0.0) {
      return y.right() > 0.0 ? interval(0.0) : interval::emptyset();
    }

    /*
      A degenerate integer exponent [n]: x^n on x >= 0, where pow and pown
      agree. Within the ints, pown gives the powers that are doubles exactly,
      as pow_is_double() does at the corners below. Beyond the ints, which pown
      cannot take, |n| > 2^31: x^n increases with x for n > 0, 0^n being 0, and
      decreases for n < 0, +oo being its limit at 0; 1^n is 1. A lower bound 0
      is taken as +0, CORE-MATH's pow(-0, n) being -oo for an odd n < 0. x^n is
      a double at 0 and 1 only, taken apart here: for another x it is beyond the
      doubles (2^(k n), for a power of two 2^k, k != 0) or not dyadic (1/m^|n|
      for n < 0) or above 2^53 (m^n, m >= 3 odd), so that the double below
      CORE-MATH's value is the tightest lower bound (see pow_is_double()).
      gaol_pow_hybrid() takes [n] before it comes here: its powers are those of
      the whole of x, and [-oo,+oo] beyond the ints.
    */
    const double n = y.left();
    if (n == y.right() && std::floor(n) == n) {
      if (y.is_an_int()) {
        return gaol_pown(base, static_cast<int>(n));
      }
      const double xl = (base.left() == 0.0) ? 0.0 : base.left(), xu = base.right();
      const double at_lower = (n > 0.0) ? xl : xu, at_upper = (n > 0.0) ? xu : xl;
      // The bounds of namespace upward, which do not check the rounding
      // direction again, through pow_rounded_dn() and pow_rounded_up(),
      // which compute the powers for an n from 2^1014 on themselves (GAOL v5)
      const double l = (at_lower == 1.0) ? 1.0 : pow_rounded_dn(at_lower, n);
      const double r = (at_upper == 1.0) ? 1.0 : pow_rounded_up(at_upper, n);
      return interval((l > 0.0) ? l : 0.0, r);
    }

    /*
      For a base above 0 and finite bounds, CORE-MATH's pow at the corners of
      the box (GAOL v5, issue #8): x^y increases with y for x > 1 and decreases
      for x < 1, increases with x for y > 0 and decreases for y < 0, so that
      its extrema over I x J are at corners, which the places of the bounds
      about 1 and 0 give. CORE-MATH's pow is correctly rounded in the upward
      rounding GAOL computes in, which gives the upper bound, and the double
      below it the lower one, unless the power is a double, which
      pow_is_double() proves and which is then the lower bound itself: the
      bounds are the tightest ones, where exp(J*log(I)) multiplied the relative
      width of log(I) by |y log(x)|: pow([2], [1023.5]) was 1425 doubles below
      and 748 above. The lower bound was the double below CORE-MATH's value
      even where that value is exact: pow([4], 0.5) was [2 - 2^-52, 2].
      A base from 0, whose powers are from 0 for exponents above 0, takes its
      upper bound so. The other boxes (a base from 0 with an exponent that is
      not above 0, an infinite bound) keep exp(J*log(I)), which gives their
      limits.
    */
    const double xl = base.left(), xu = base.right(), yl = y.left(), yu = y.right();
    const double dmax = (std::numeric_limits<double>::max)();
    if (xu <= dmax && yl >= -dmax && yu <= dmax && (xl > 0.0 || yl > 0.0)) {
      double l, r;
      if (xl == 0.0) {
        l = 0.0;
        r = pow_hi(xu, (xu >= 1.0) ? yu : yl);
      } else if (xl >= 1.0) {
        l = pow_lo((yl >= 0.0) ? xl : xu, yl);
        r = pow_hi((yu >= 0.0) ? xu : xl, yu);
      } else if (xu <= 1.0) {
        l = pow_lo((yu >= 0.0) ? xl : xu, yu);
        r = pow_hi((yl >= 0.0) ? xu : xl, yl);
      } else {
        l = minimum(pow_lo(xl, yu), pow_lo(xu, yl));
        r = maximum(pow_hi(xl, yl), pow_hi(xu, yu));
      }
      return interval(l,r);
    }
    return exp(y*log(base));
  }

  /*
    pow_standard_upward() after one check of the rounding direction, made
    before the bounds are compared: with denormals-are-zero, the lower bound
    -5 2^-1074 of x stayed in x & [0, +oo], and pow([-5 2^-1074, 1], [0.5])
    was empty once the check of the corners had cleared the mode (GAOL v5, see
    gaol/gaol_fpu.h). Its powers and its exp(y*log(base)) check the
    direction once more each.
  */
  static interval pow_standard(const interval& x, const interval& y)
  {
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
    // has no value for l = +oo or u = -oo (10.5.8)
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
        return gaol_pown(I,int(J.left()));
      }
      // An integer beyond the ints, which gaol_pown() cannot take
      return interval::universe();
    }
    return pow_standard_upward(I, J);
  }

  interval gaol_pow_hybrid(const interval &I, const interval &J)
  {
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
    // invalid-operation exception for a NaN p (GAOL v5)
    if (!gaol_detail::quiet_less_equal(std::fabs(p), (std::numeric_limits<double>::max)())) {
      return interval::emptyset();
    }
    // p compared after the check: with denormals-are-zero, the floor of a
    // subnormal p was p, and pow(I, p) the power [1] (GAOL v5, see
    // gaol/gaol_fpu.h)
    GAOL_RND_ENTER();
    p = rnd_reread(p);
    interval res;
    if (std::floor(p) == p && p >= (std::numeric_limits<int>::min)() && p <= (std::numeric_limits<int>::max)()) {
      res = gaol_pown(I, static_cast<int>(p));
    } else {
      res = pow_hybrid_upward(I, interval(p));
    }
    GAOL_RND_KEEP(res);
    GAOL_RND_LEAVE();
    return res;
  }

  /*
    Code inspired by ia_math code by Timothy Hickey. The rounding direction
    is upward, after the check of nth_root_rel(), whose intersections and
    hulls are taken before the modes are restored with
    GAOL_PRESERVE_ROUNDING (GAOL v5, see gaol/gaol_fpu.h)
  */
  static interval nth_root_rel_upward(const interval& J, unsigned int n, const interval& I)
  {
    switch (n) {
    case 0:
      if (J.set_contains(1.0)) {
	return I;
      } else {
	return interval::emptyset();
      }
    case 1:
      return J & I;
    case 2:
      return sqrt_rel(J,I);
    default:
      break;
    }
    // The roots of nth_root(), proved to be bounds (GAOL v5: GAOL took the
    // powers of the mathematical library with the exponent 1/n here too)
    if (I.is_empty()) {
      return interval::emptyset();
    }
    const interval tmp = nth_root(J,n);
    if (odd(n) || tmp.is_empty()) {
      return (tmp & I);
    }
    // n is even: the roots of either sign
    if (I.certainly_positive()) {
      return (tmp & I);
    }
    if (I.certainly_negative()) {
      return (-tmp & I);
    }
    return ((tmp & I) | ((-tmp) & I));
  }

  interval nth_root_rel(const interval& J, unsigned int n, const interval& I)
  {
    GAOL_RND_ENTER();
    interval res = nth_root_rel_upward(J, n, I);
    GAOL_RND_KEEP(res);
    GAOL_RND_LEAVE();
    return res;
  }

  /*
    x^n rounded upward and downward, x >= 0 and n > 0, by binary exponentiation,
    the rounding direction being upward: the products of ipow_up() and
    ipow_dn() (gaol_double_op.h). x^n is a multiple of each square computed, so
    that ipow_hi() only overflows where x^n does; ipow_lo() is rounded downward
    through -((-a)*b) rounded upward, its product being kept negated.
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

    l^n, of n - 1 rounded products, is within (n - 1) 2^-53 of its value
    relatively, which moves the root by less than 2^-53 relatively: the bounds
    are the tightest or one double beyond, and the root of a double that is an
    n-th power is that double.
  */
  static double newton_root(double x, double r, unsigned int n)
  {
    if (!(r > 0.0)) { // Negative or NaN
      return 0.0;
    }
    if (r > std::numeric_limits<double>::max()) {
      return std::numeric_limits<double>::max();
    }
    const double p = ipow_hi(r,n);
    if (p >= std::numeric_limits<double>::min() && p <= std::numeric_limits<double>::max()) {
      const double s = r - r*((p - x)/(double(n)*p));
      if (s > 0.0 && s <= std::numeric_limits<double>::max()) {
	return s;
      }
    }
    return r;
  }

  static inline bool is_proved_below_root(double l, double x, unsigned int n)
  {
    return ipow_hi(l,n) <= x;
  }

  static inline bool is_proved_above_root(double u, double x, unsigned int n)
  {
    return ipow_lo(u,n) >= x;
  }

  // A step of about one double at r, which doubles
  static inline double first_step(double r)
  {
    return maximum(r*std::numeric_limits<double>::epsilon(), std::numeric_limits<double>::denorm_min());
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

  static double proved_root_dn(double x, double r, unsigned int n)
  {
    // l is proved to be below the root, and h is not: 0 is, and the largest
    // double is not
    const double largest = std::numeric_limits<double>::max();
    double l = newton_root(x,r,n), h = l, step = first_step(l);
    int steps = 0;
    if (is_proved_below_root(l,x,n)) {
      for (h = minimum(l + step, largest); is_proved_below_root(h,x,n) && h < largest; h = minimum(l + step, largest)) {
	l = h;
	step *= 2.0;
	if (++steps > root_search_limit) {
	  return l;
	}
      }
      if (h >= largest && is_proved_below_root(h,x,n)) { // Not for n > 1 and a finite x
	return h;
      }
    } else {
      for (l = maximum(-(step - h), 0.0); !is_proved_below_root(l,x,n); l = maximum(-(step - h), 0.0)) {
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
      if (is_proved_below_root(m,x,n)) {
	l = m;
      } else {
	h = m;
      }
    }
  }

  static double proved_root_up(double x, double r, unsigned int n)
  {
    // u is proved to be above the root, and l is not: the largest double is,
    // and 0 is not
    const double largest = std::numeric_limits<double>::max();
    double u = newton_root(x,r,n), l = u, step = first_step(u);
    int steps = 0;
    if (is_proved_above_root(u,x,n)) {
      for (l = maximum(-(step - u), 0.0); is_proved_above_root(l,x,n); l = maximum(-(step - u), 0.0)) {
	u = l;
	step *= 2.0;
	if (++steps > root_search_limit) {
	  return u;
	}
      }
    } else {
      for (u = minimum(l + step, largest); !is_proved_above_root(u,x,n); u = minimum(l + step, largest)) {
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
      if (is_proved_above_root(m,x,n)) {
	u = m;
      } else {
	l = m;
      }
    }
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
    upward, as nth_root(), its only caller, set it: the direction is not
    checked again here (GAOL v5)
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
  method then by bisection, which took 179 ns for a cube root against 67 ns
  here, and gave bounds up to 2 doubles from the tightest ones.
*/
static inline double cube_root_up(double x) // x >= 0
{
  return gaol_cr_cbrt(x);
}

static inline double cube_root_dn(double x) // x >= 0
{
  const double u = gaol_cr_cbrt(x);
  return (u * u * u == x) ? u : previous_float(u);
}

static interval cube_root(const interval& I)
{
  if (I.is_empty()) {
    return interval::emptyset();
  }
  GAOL_RND_ENTER();
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
  GAOL_RND_KEEP(lo);
  GAOL_RND_KEEP(hi);
  GAOL_RND_LEAVE();
  return interval(lo, hi);
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
	const interval J = odd(n) ? I : (I & interval::positive());
	if (J.is_empty()) {
		GAOL_RND_LEAVE();
		return interval::emptyset();
	}
	// The roots of the magnitudes of the bounds, the root of x < 0 being
	// -(-x)^(1/n)
	const double a = std::fabs(J.left()), b = std::fabs(J.right());
	double near_a, near_b;
	near_roots(a,b,n,near_a,near_b);
	double l = (J.left() >= 0.0) ? root_dn(a,near_a,n) : -root_up(a,near_a,n);
	double r = (J.right() >= 0.0) ? root_up(b,near_b,n) : -root_dn(b,near_b,n);
	GAOL_RND_KEEP(l);
	GAOL_RND_KEEP(r);
	GAOL_RND_LEAVE();
	return interval(l,r);
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
	// -q on a long, INT_MIN having no opposite on an int
	const unsigned int n = static_cast<unsigned int>(-static_cast<long>(q));
	return inverse(nth_root(I,n));
}

  unsigned long long nb_fp_numbers(double a, double b)
  {
    // a and b compared as bounds, whatever the modes that flush the
    // subnormals to zero (gaol_port.h): under denormals-are-zero,
    // nb_fp_numbers(2^-1074, 2*2^-1074) was 1 (GAOL v5, point Q of TODO.md)
    if (!is_finite(a) || !is_finite(b) || gaol_detail::bound_greater(a, b)) {
      // Either a or b is a NaN or +/-oo, or [a,b] is empty? gaol_ERROR
      // throws, or aborts where the exceptions are disabled: there is no
      // value to return, whose line Visual C++ found unreachable (C4702)
      gaol_ERROR(invalid_action_error,"invalid argument(s) in call to nb_fp_numbers()");
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
    The exponentials and the logarithms in base 2 and 10 (GAOL v5)

    IEEE 1788-2015 requires them among the forward elementary functions
    (Table 9.1: "exp, exp2, exp10(x) = b^x" on R with range (0, +oo), and
    "log, log2, log10(x) = log_b(x)" on (0, +oo), note d saying b = e, 2 or
    10), and GAOL did not provide them. CORE-MATH computes them correctly
    rounded in the rounding direction in effect, so, in the upward rounding
    GAOL keeps, the value at the right bound is the upper bound and the double
    below the value at the left bound the lower one -- unless that value is
    itself a double, and the tests below tell when it is. The bounds are then
    the tightest ones, as they are for exp and log.
  */

  // 2^x is a double exactly when x is an integer of [-1074, 1023]: 2^-1074 is
  // the smallest subnormal and 2^1024 overflows
  static inline bool exp2_is_exact(double x)
  {
    return x == std::floor(x) && x >= -1074.0 && x <= 1023.0;
  }

  // 10^x is a double exactly when x is an integer of [0, 22]: 10^23 is not a
  // double, and no negative power of ten is (0.1 is not one)
  static inline bool exp10_is_exact(double x)
  {
    return x == std::floor(x) && x >= 0.0 && x <= 22.0;
  }

  // log2(x) is a double exactly when x is a power of two, which frexp tells:
  // it writes x as m*2^e with m in [1/2, 1), so m is 1/2 for a power of two
  // and nothing else (and 0 for zero, inf for an infinity)
  static inline bool log2_is_exact(double x)
  {
    int e;
    return std::frexp(x, &e) == 0.5;
  }

  // log10(x) is a double exactly when x is a power of ten with an exponent in
  // [0, 22], the powers of ten that are doubles. The value rounded upward is
  // an integer of that range for those x, and might be for others, which
  // raising ten to it settles: 10^n is exact there.
  static inline bool log10_is_exact(double x)
  {
    if (!(x > 0.0) || !is_finite(x)) {
      return false;
    }
    const double n = gaol_cr_log10(x);
    if (n != std::floor(n) || n < 0.0 || n > 22.0) {
      return false;
    }
    return gaol_cr_exp10(n) == x;
  }

  interval exp2(const interval& I)
  {
    if (I.is_empty()) {
      return interval::emptyset();
    }
    // The check before the bounds are read, and the maximum taken before
    // GAOL_RND_LEAVE(), as exp() does (see gaol/gaol_fpu.h)
    GAOL_RND_ENTER();
    const double l = I.left(), r = I.right();
    const double w = gaol_cr_exp2(l);
    const double u = exp2_is_exact(l) ? w : previous_float(w);
    const double v = gaol_cr_exp2(r);
    // Within [0, +oo], as exp: 2^x is positive
    interval res(maximum(0.0, u), v);
    GAOL_RND_KEEP(res);
    GAOL_RND_LEAVE();
    return res;
  }

  interval exp10(const interval& I)
  {
    if (I.is_empty()) {
      return interval::emptyset();
    }
    GAOL_RND_ENTER();
    const double l = I.left(), r = I.right();
    const double w = gaol_cr_exp10(l);
    const double u = exp10_is_exact(l) ? w : previous_float(w);
    const double v = gaol_cr_exp10(r);
    interval res(maximum(0.0, u), v);
    GAOL_RND_KEEP(res);
    GAOL_RND_LEAVE();
    return res;
  }

  interval log2(const interval& I)
  {
    // Defined on (0, +oo), as log: I holding no positive number gives the
    // empty set (IEEE 1788-2015, Table 9.1), the bounds being compared after
    // the check, as in log()
    if (I.is_empty()) {
      return interval::emptyset();
    }
    GAOL_RND_ENTER();
    if (!(I.right() > 0.0)) {
      GAOL_RND_LEAVE();
      return interval::emptyset();
    }
    const double l = maximum(0.0, I.left()), r = I.right();
    const double w = gaol_cr_log2(l);
    const double u = log2_is_exact(l) ? w : previous_float(w);
    const double v = gaol_cr_log2(r);
    GAOL_RND_LEAVE();
    return interval(u, v);
  }

  interval log10(const interval& I)
  {
    if (I.is_empty()) {
      return interval::emptyset();
    }
    GAOL_RND_ENTER();
    if (!(I.right() > 0.0)) {
      GAOL_RND_LEAVE();
      return interval::emptyset();
    }
    const double l = maximum(0.0, I.left()), r = I.right();
    const double w = gaol_cr_log10(l);
    const double u = log10_is_exact(l) ? w : previous_float(w);
    const double v = gaol_cr_log10(r);
    GAOL_RND_LEAVE();
    return interval(u, v);
  }

  /*
    The forward functions IEEE 1788-2015 recommends (Table 10.5) (GAOL v5)

    expm1, exp2m1 and exp10m1 compute b^x - 1 without the cancellation of
    subtracting 1 from b^x near 0, and atanpi, acospi, sinpi, cospi and tanpi a
    function of pi*x, or one divided by pi, without the loss of accuracy of pi
    being irrational (notes c and e of the table). CORE-MATH computes each of
    them correctly rounded in the rounding direction in effect, as it does
    exp2, so the bounds are got the same way: in the upward rounding GAOL keeps,
    the value at a bound rounded upward, and the double below it for a lower
    bound unless the value is itself a double, which the tests below tell.
  */

  // b^x - 1 is a double exactly at 0, and for b = 2 at the integers of
  // [-53, 53], for b = 10 at those of [0, 15]: 2^n - 1 has n significant bits,
  // 1 - 2^-n as many, and 10^n - 1 is below 2^53 up to n = 15
  static inline bool expm1_is_exact(double x)
  {
    return x == 0.0;
  }

  static inline bool exp2m1_is_exact(double x)
  {
    return x == std::floor(x) && x >= -53.0 && x <= 53.0;
  }

  static inline bool exp10m1_is_exact(double x)
  {
    return x == std::floor(x) && x >= 0.0 && x <= 15.0;
  }

  // atan(x)/pi is rational at a rational x only for x in {0, +-1}, by Niven's
  // theorem, where it is 0 and +-1/4; and it is +-1/2 at the infinities
  static inline bool atanpi_is_exact(double x)
  {
    return x == 0.0 || x == 1.0 || x == -1.0 || !is_finite(x);
  }

  // acos(x)/pi is a double at -1, 0 and 1 only, where it is 1, 1/2 and 0: the
  // other rational values, 1/3 and 2/3 at +-1/2, are no doubles
  static inline bool acospi_is_exact(double x)
  {
    return x == 0.0 || x == 1.0 || x == -1.0;
  }

  // An increasing function, whose range the bounds are brought back into: the
  // value at the right bound, the double below the value at the left bound
  // unless it is a double. The rounding direction is upward, after the check
  // of increasing_cr() or of the caller, made before I is computed (see
  // gaol/gaol_fpu.h)
  static interval increasing_cr_upward(const interval& I, double (*f)(double),
                                       bool (*exact)(double), double lowest, double highest)
  {
    if (I.is_empty()) {
      return interval::emptyset();
    }
    const double l = I.left(), r = I.right();
    const double w = f(l);
    const double u = exact(l) ? w : previous_float(w);
    const double v = f(r);
    return interval(maximum(lowest, u), minimum(highest, v));
  }

  static interval increasing_cr(const interval& I, double (*f)(double),
                                bool (*exact)(double), double lowest, double highest)
  {
    GAOL_RND_ENTER();
    interval res = increasing_cr_upward(I, f, exact, lowest, highest);
    GAOL_RND_KEEP(res);
    GAOL_RND_LEAVE();
    return res;
  }

  // Within [-1, +oo]: b^x - 1 exceeds -1, which it nears as x goes to -oo.
  // +oo is read after the check: GAOL_INFINITY was the HUGE_VAL of the UCRT,
  // which clang-cl computes when the program runs, and it was FLT_MAX in the
  // caller's downward rounding, which made expm1([1e10]) empty (GAOL v5)
  static interval increasing_cr_from_minus_one(const interval& I, double (*f)(double),
                                               bool (*exact)(double))
  {
    GAOL_RND_ENTER();
    interval res = increasing_cr_upward(I, f, exact, -1.0, GAOL_INFINITY);
    GAOL_RND_KEEP(res);
    GAOL_RND_LEAVE();
    return res;
  }

  interval expm1(const interval& I)
  {
    return increasing_cr_from_minus_one(I, gaol_cr_expm1, expm1_is_exact);
  }

  interval exp2m1(const interval& I)
  {
    return increasing_cr_from_minus_one(I, gaol_cr_exp2m1, exp2m1_is_exact);
  }

  interval exp10m1(const interval& I)
  {
    return increasing_cr_from_minus_one(I, gaol_cr_exp10m1, exp10m1_is_exact);
  }

  interval atanpi(const interval& I)
  {
    // within [-1/2, 1/2], the image of atan divided by pi
    return increasing_cr(I, gaol_cr_atanpi, atanpi_is_exact, -0.5, 0.5);
  }

  interval acospi(const interval& I)
  {
    // Defined on [-1, 1], as acos (IEEE 1788-2015, Table 10.5): the part of I
    // outside is left out, and I holding no point of it gives the empty set
    if (I.is_empty()) {
      return interval::emptyset();
    }
    GAOL_RND_ENTER();
    if (I.right() < -1.0 || I.left() > 1.0) {
      GAOL_RND_LEAVE();
      return interval::emptyset();
    }
    const double l = maximum(-1.0, I.left()), r = minimum(1.0, I.right());
    // decreasing: the upper bound at the left bound, the lower one at the right
    const double v = gaol_cr_acospi(l);
    const double w = gaol_cr_acospi(r);
    const double u = acospi_is_exact(r) ? w : previous_float(w);
    interval res(maximum(0.0, u), minimum(1.0, v));
    GAOL_RND_KEEP(res);
    GAOL_RND_LEAVE();
    return res;
  }

  /*
    sinpi, cospi and tanpi: the points where these functions change direction,
    or have a pole, are the multiples of 1/2, which are doubles, so the
    analysis of an interval reduces exactly to the integers t = 2x. Doubling a
    double is exact, and so are its ceiling and its floor; below 2^61 these
    integers are held exactly in 64 bits, and two distinct doubles beyond 2^61
    are at least 2^9 apart, whole periods. sin(pi*x) and cos(pi*x) have their
    extrema at the odd, respectively even, t, and tan(pi*x) its poles at the
    odd t, increasing between them.
  */
  static const double gaol_two_61 = 2305843009213693952.0;

  // Whether an integer congruent to r modulo m lies in [lo, hi]
  static inline bool holds_residue(std::int64_t lo, std::int64_t hi,
                                   std::int64_t r, std::int64_t m)
  {
    if (lo > hi) {
      return false;
    }
    std::int64_t lm = lo % m;
    if (lm < 0) {
      lm += m;
    }
    std::int64_t d = (r - lm) % m;
    if (d < 0) {
      d += m;
    }
    return lo + d <= hi;
  }

  // Whether k*x is an integer: sin(pi*x) and cos(pi*x) are doubles exactly at
  // the multiples of 1/2 (k = 2), and tan(pi*x) at those of 1/4 (k = 4). By
  // Niven's theorem sin and cos take at a rational x the rational values 0,
  // +-1/2 and +-1 only, +-1/2 at points that are not doubles, and tan the
  // values 0 and +-1 only. Beyond 2^61 every double is an even integer.
  static inline bool is_multiple_of_inverse(double x, double k)
  {
    if (std::fabs(x) >= gaol_two_61) {
      return true;
    }
    const double t = k * x;
    return t == std::floor(t);
  }

  // sin(pi*x) or cos(pi*x) over I: the maximum 1 where a t congruent to up
  // modulo 4 lies within, the minimum -1 where one congruent to down does, and
  // the values at the bounds otherwise. The rounding direction is upward, after
  // the check of sin_or_cos_pi(), made before the bounds are compared: 2 l,
  // whose ceiling gives ka, is 0 for a subnormal l with the modes that flush
  // them to zero (see gaol/gaol_fpu.h)
  static interval sin_or_cos_pi_upward(const interval& I, double (*f)(double),
                                       std::int64_t up, std::int64_t down)
  {
    if (I.is_empty()) {
      return interval::emptyset();
    }
    const double l = I.left(), r = I.right();
    if (!is_finite(l) || !is_finite(r)) {
      return interval(-1.0, 1.0);
    }
    const double far = maximum(std::fabs(l), std::fabs(r));
    if (l != r && far >= gaol_two_61) {
      return interval(-1.0, 1.0);
    }
    bool has_max = false, has_min = false;
    if (far < gaol_two_61) {
      const std::int64_t ka = static_cast<std::int64_t>(std::ceil(2.0 * l));
      const std::int64_t kb = static_cast<std::int64_t>(std::floor(2.0 * r));
      if (kb - ka >= 3) {
        return interval(-1.0, 1.0);  // every residue modulo 4
      }
      has_max = holds_residue(ka, kb, up, 4);
      has_min = holds_residue(ka, kb, down, 4);
    }
    const double fl = f(l), fr = f(r);
    const double dl = is_multiple_of_inverse(l, 2.0) ? fl : previous_float(fl);
    const double dr = is_multiple_of_inverse(r, 2.0) ? fr : previous_float(fr);
    const double lower = has_min ? -1.0 : maximum(-1.0, minimum(dl, dr));
    const double upper = has_max ? 1.0 : minimum(1.0, maximum(fl, fr));
    return interval(lower, upper);
  }

  /* The minimum and the maximum taken before GAOL_RND_LEAVE(): with
     GAOL_PRESERVE_ROUNDING, the denormals-are-zero mode it restores made two
     subnormal values equal, and sinpi([100, 1000] 2^-1074) was empty (GAOL v5,
     review of point 4) */
  static interval sin_or_cos_pi(const interval& I, double (*f)(double),
                                std::int64_t up, std::int64_t down)
  {
    GAOL_RND_ENTER();
    interval res = sin_or_cos_pi_upward(I, f, up, down);
    GAOL_RND_KEEP(res);
    GAOL_RND_LEAVE();
    return res;
  }

  interval sinpi(const interval& I)
  {
    // sin(pi*x) = 1 at x = 1/2 modulo 2, t = 1 modulo 4; -1 at t = 3
    return sin_or_cos_pi(I, gaol_cr_sinpi, 1, 3);
  }

  interval cospi(const interval& I)
  {
    // cos(pi*x) = 1 at the even x, t = 0 modulo 4; -1 at the odd x, t = 2
    return sin_or_cos_pi(I, gaol_cr_cospi, 0, 2);
  }

  // tanpi() after its check, made before the bounds are compared, as in
  // sin_or_cos_pi()
  static interval tanpi_upward(const interval& I)
  {
    if (I.is_empty()) {
      return interval::emptyset();
    }
    const double l = I.left(), r = I.right();
    if (!is_finite(l) || !is_finite(r)) {
      return interval::universe();
    }
    const double far = maximum(std::fabs(l), std::fabs(r));
    if (l != r && far >= gaol_two_61) {
      return interval::universe();
    }
    bool pole_inside = false, pole_at_l = false, pole_at_r = false;
    if (far < gaol_two_61) {
      const double tl = 2.0 * l, tr = 2.0 * r;
      // the integers strictly between tl and tr, and the poles among them
      const std::int64_t lo = static_cast<std::int64_t>(std::floor(tl)) + 1;
      const std::int64_t hi = static_cast<std::int64_t>(std::ceil(tr)) - 1;
      pole_inside = holds_residue(lo, hi, 1, 2);
      pole_at_l = tl == std::floor(tl) && std::fmod(tl, 2.0) != 0.0;
      pole_at_r = tr == std::floor(tr) && std::fmod(tr, 2.0) != 0.0;
    }
    // tan(pi*x) has no value at a pole (IEEE 1788-2015, Table 10.5), and is
    // increasing between two poles, from -oo to +oo
    if (l == r && pole_at_l) {
      return interval::emptyset();
    }
    if (pole_inside || (pole_at_l && pole_at_r)) {
      return interval::universe();
    }
    // a pole at a bound is the limit -oo from its right, +oo from its left
    double lower = -GAOL_INFINITY, upper = GAOL_INFINITY;
    if (!pole_at_l) {
      const double fl = gaol_cr_tanpi(l);
      lower = is_multiple_of_inverse(l, 4.0) ? fl : previous_float(fl);
    }
    if (!pole_at_r) {
      upper = gaol_cr_tanpi(r);
    }
    return interval(lower, upper);
  }

  interval tanpi(const interval& I)
  {
    GAOL_RND_ENTER();
    interval res = tanpi_upward(I);
    GAOL_RND_KEEP(res);
    GAOL_RND_LEAVE();
    return res;
  }

  /*
    log1p, log2p1 and log10p1, the logp1, log2p1 and log10p1 of IEEE 1788-2015
    (Table 10.5): log_b(1 + x) on (-1, +oo), without the cancellation of
    adding 1 to x near 0. They are increasing and bounded as expm1 is; the
    part of I at or below -1 is left out, and the value at -1 is the limit -oo.
  */

  // log(1 + x) is rational at a rational x only for x = 0, where it is 0: the
  // logarithm of a rational other than 1 is transcendental (Lindemann)
  static inline bool log1p_is_exact(double x)
  {
    return x == 0.0 || x == -1.0 || x == GAOL_INFINITY;
  }

  // log2(1 + x) is a double exactly where 1 + x is a power of two 2^k: then
  // x = 2^k - 1, a double for k in [0, 53], the integers below 2^53 whose
  // successor is a power of two, and for k in [-53, -1], in (-1, -1/2] where
  // 1 + x is computed exactly (Sterbenz). A logarithm of a rational that is
  // not a power of two is irrational.
  static inline bool log2p1_is_exact(double x)
  {
    if (x == -1.0 || x == GAOL_INFINITY) {
      return true;
    }
    if (x >= 0.0 && x < 9007199254740992.0 && x == std::floor(x)) { // 2^53
      const std::uint64_t n = static_cast<std::uint64_t>(x);
      return (n & (n + 1)) == 0;
    }
    if (x > -1.0 && x <= -0.5) {
      int e;
      return std::frexp(1.0 + x, &e) == 0.5;
    }
    return false;
  }

  // log10(1 + x) is a double exactly where 1 + x is a power of ten 10^k with
  // k >= 0, the others being no doubles: x = 10^k - 1 for k in [0, 15],
  // 10^16 - 1 being above 2^53
  static inline bool log10p1_is_exact(double x)
  {
    if (x == -1.0 || x == GAOL_INFINITY) {
      return true;
    }
    if (!(x >= 0.0 && x < 1e15 && x == std::floor(x))) {
      return false;
    }
    double p = 1.0; // 10^k, exact up to 10^15
    for (int k = 0; k <= 15; ++k, p *= 10.0) {
      if (x == p - 1.0) {
        return true;
      }
    }
    return false;
  }

  // The check before the interval cut at -1 is made (see increasing_cr())
  static interval log_p1(const interval& I, double (*f)(double), bool (*exact)(double))
  {
    if (I.is_empty()) {
      return interval::emptyset();
    }
    GAOL_RND_ENTER();
    interval res = !(I.right() > -1.0) ? interval::emptyset()
      : increasing_cr_upward(interval(maximum(-1.0, I.left()), I.right()), f, exact,
                             -GAOL_INFINITY, GAOL_INFINITY);
    GAOL_RND_KEEP(res);
    GAOL_RND_LEAVE();
    return res;
  }

  interval log1p(const interval& I)
  {
    return log_p1(I, gaol_cr_log1p, log1p_is_exact);
  }

  interval log2p1(const interval& I)
  {
    return log_p1(I, gaol_cr_log2p1, log2p1_is_exact);
  }

  interval log10p1(const interval& I)
  {
    return log_p1(I, gaol_cr_log10p1, log10p1_is_exact);
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
    asinpi, the asinPi of IEEE 1788-2015 (Table 10.5): asin(x)/pi on [-1, 1],
    increasing, within [-1/2, 1/2].

    asin(x)/pi is rational at a rational x only for x in {0, +-1/2, +-1}, by
    Niven's theorem, where it is 0, +-1/6 and +-1/2: a double at 0 and +-1.
  */
  static inline bool asinpi_is_exact(double x)
  {
    return x == 0.0 || x == 1.0 || x == -1.0;
  }

  interval asinpi(const interval& I)
  {
    // the part of I outside [-1, 1] is left out, as with acospi, after the
    // check: GCC 13 compared the bounds of the intersection, held in
    // registers, with 0 before it, and asinpi([100 2^-1074]) was
    // [32 2^-1074] with the denormals-are-zero mode set (GAOL v5)
    GAOL_RND_ENTER();
    interval res = increasing_cr_upward(I & interval::minus_one_plus_one(), gaol_cr_asinpi,
                                        asinpi_is_exact, -0.5, 0.5);
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
        && std::fma(s, t, -p) == std::ldexp(std::fma(m, m, -q), 2 * d);
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
    const double al = X.mig(), bl = Y.mig(), ar = X.mag(), br = Y.mag();
    const double w = gaol_cr_hypot(al, bl);
    const double u = hypot_is_exact(al, bl, w) ? w : previous_float(w);
    const double v = gaol_cr_hypot(ar, br);
    interval res(maximum(0.0, u), v);
    GAOL_RND_KEEP(res);
    GAOL_RND_LEAVE();
    return res;
  }

  /*
    atan2pi, the atan2Pi of IEEE 1788-2015 (Table 10.5): atan2(y, x)/pi on the
    plane but (0, 0), with values in (-1, 1]. The analysis of the box is that
    of atan2 above, the angles divided by pi. The angle over pi is a double
    where it is a multiple of 1/4, which the tests below find: 0, +-1/4, +-1/2,
    +-3/4 and 1, and -1 as a limit. Elsewhere it is irrational: tan(pi r) is
    rational for a rational r only where it is 0 or +-1 (Niven), and y/x is
    rational.
  */
  static inline bool atan2pi_special(double y, double x, double& v)
  {
    if (y == 0.0 || x == GAOL_INFINITY) {
      v = (x > 0.0) ? 0.0 : 1.0;
      return true;
    }
    if (x == 0.0 || std::fabs(y) == GAOL_INFINITY) {
      v = (y > 0.0) ? 0.5 : -0.5;
      return true;
    }
    if (x == -GAOL_INFINITY) {
      v = (y > 0.0) ? 1.0 : -1.0;
      return true;
    }
    if (std::fabs(x) == std::fabs(y)) {
      const double q = (x > 0.0) ? 0.25 : 0.75;
      v = (y > 0.0) ? q : -q;
      return true;
    }
    return false;
  }

  static double atan2pi_lo(double y, double x)
  {
    double v;
    return atan2pi_special(y, x, v) ? v : maximum(previous_float(gaol_cr_atan2pi(y, x)), -1.0);
  }

  static double atan2pi_hi(double y, double x)
  {
    double v;
    return atan2pi_special(y, x, v) ? v : minimum(gaol_cr_atan2pi(y, x), 1.0);
  }

  // atan2pi() after its check, made before the bounds are compared, as in
  // atan2()
  static interval atan2pi_upward(const interval& Y, const interval& X)
  {
    if (Y.is_empty() || X.is_empty()) {
      return interval::emptyset();
    }
    const double yl = Y.left(), yu = Y.right(), xl = X.left(), xu = X.right();
    if (yl == 0.0 && yu == 0.0 && xl == 0.0 && xu == 0.0) {
      return interval::emptyset();
    }
    if (yl < 0.0 && yu >= 0.0 && xl < 0.0) {
      return interval(-1.0, 1.0);
    }
    double l, r;
    if (yl >= 0.0) { // Upper half-plane: the angle decreases with x
      l = atan2pi_lo((xu > 0.0) ? yl : yu, xu);
      r = (xl == 0.0 && yu == 0.0) ? 0.0 : atan2pi_hi((xl >= 0.0) ? yu : yl, xl);
    } else if (yu < 0.0) { // Lower half-plane, y = 0 left out: the angle increases with x
      l = atan2pi_lo((xl >= 0.0) ? yl : yu, xl);
      r = atan2pi_hi((xu > 0.0) ? yu : yl, xu);
    } else { // yl < 0 <= yu, in the right half-plane
      l = atan2pi_lo(yl, xl);
      r = (yu == 0.0) ? ((xu == 0.0) ? -0.5 : 0.0) : atan2pi_hi(yu, xl);
    }
    return interval(l, r);
  }

  interval atan2pi(const interval& Y, const interval& X)
  {
    GAOL_RND_ENTER();
    interval res = atan2pi_upward(Y, X);
    GAOL_RND_KEEP(res);
    GAOL_RND_LEAVE();
    return res;
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
    return (a == 0.0 || b == 0.0) ? c : std::fma(a, b, c);
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
    return (a == 0.0 || b == 0.0) ? c : -gaol_core::rnd_keep(std::fma(-a, b, -c));
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
    value rounded to nearest and of its error, by the TwoSum of Knuth, which is
    exact in the rounding to nearest: the rounded values compare as the exact
    ones unless they are equal, and the errors decide then.
  */
  static inline void two_sum(double a, double b, double& s, double& e)
  {
    s = a + b;
    const double ap = s - b;
    const double bp = s - ap;
    e = (a - ap) + (b - bp);
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
      // both are, with the same sign; the four doubles are then beyond 2^1022,
      // where halving them is exact
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

  interval exp(const interval& I)
  {
    // The empty set tested first: its NaN bounds would give the empty set
    // too, through the quiet comparison of the constructor, but after the
    // check of the rounding direction and two calls of CORE-MATH (GAOL v5)
    if (I.is_empty()) {
      return interval::emptyset();
    }
	/* We intersect the result with [0, +oo] to ensure that the result is strictly positive
 		Otherwise, we might have: exp([-oo, -MAX] = [-v, +v] with v very small.
	*/
    // exp(0) = 1 exactly, where the value of the mathematical library moved
    // outward gave exp([0]) a width, and pow([1], [-oo, +oo]) = [0, +oo]
    // (GAOL v5). The bounds read after the check (see gaol/gaol_fpu.h)
    GAOL_RND_ENTER();
    const double l = I.left(), r = I.right();
    // The maximum taken before GAOL_RND_LEAVE(): with GAOL_PRESERVE_ROUNDING,
    // the denormals-are-zero mode it restores made the subnormal lower bound of
    // exp([-740]) equal to 0, and 0 the bound (GAOL v5)
    double u = (l == 0.0) ? 1.0 : maximum(0.0,upward::exp_dn(l));
    double v = (r == 0.0) ? 1.0 : upward::exp_up(r);
    GAOL_RND_KEEP(u); GAOL_RND_KEEP(v);
    GAOL_RND_LEAVE();
    return interval(u, v);
  }

  interval log(const interval& I)
  {
    // log is defined on (0,+oo) (IEEE 1788-2015, Table 9.1, GAOL v5): I
    // holding no positive number, as [-4,0] and [0], gives the empty set, where
    // GAOL kept its part in [0,+oo] and gave [-oo,-MAX]. The bounds compared
    // after the check: with denormals-are-zero, [1e-310, 1e-309] held no
    // positive number (GAOL v5)
    if (I.is_empty()) {
      return interval::emptyset();
    }
    GAOL_RND_ENTER();
    if (!(I.right() > 0.0)) {
      GAOL_RND_LEAVE();
      return interval::emptyset();
    }

    // log(1) = 0 exactly: log([1]) was [-2^-1074, 2^-1074] (GAOL v5)
    const double l = maximum(0.0,I.left()), r = I.right();
    // CORE-MATH's log, correctly rounded in the rounding direction in effect
    // (GAOL v5): in the upward rounding GAOL computes in, RU(log r) is the
    // right bound, and RD(log l) = pred(RU(log l)) the left one, log(l) being
    // no double for l other than 1. The tightest bounds, without switching the
    // rounding direction.
    const double u = (l == 1.0) ? 0.0 : previous_float(gaol_cr_log(l));
    const double v = (r == 1.0) ? 0.0 : gaol_cr_log(r);
    GAOL_RND_LEAVE();
    return interval(u, v);
  }


  /*!
    \brief Angle modulo pi (fast, boolean version)

    Computes k_left and k_right for I=[a,b] such that
    * a = k_left*\pi + a'  with |a'|<\pi
    * b = k_right*\pi + b' with |b'|<\pi
    \return true whevener k_left and k_right could be found and false
    otherwise.
    \note if I.left() (resp. I.right()) is very large, there might be
    more than one integer in I.left()/interval::pi() (resp.
    I.right()/interval::pi()).
  */
  bool fast_modulo_k_pi(const interval &I, double &k_left, double &k_right)
  {
    interval kl = floor(I.left()/interval::pi());
    if (kl.left() != kl.right()) {
      return false;
    }

    interval kr = floor(I.right()/interval::pi());
    if (kr.left() != kr.right()) {
      return false;
    }
    k_left  = kl.left();
    k_right = kr.right();
    return true;
  }


  /*!
    \brief Angle modulo pi (slower version with integer output)

    Computes k_left and k_right for I=[a,b] such that
    * a = k_left*\pi + a'  with |a'|<\pi
    * b = k_right*\pi + b' with |b'|<\pi
    \return 0 if neither l_left nor l_right could be computed accurately,
    1 if l_right could be computed, 2 if l_left could be computed and
    3 if both could be computed.

    \note There might be more than one integer in
    I.left()/interval::pi() (resp. I.right()/interval::pi()) when one of the bounds of I
    is close to a multiple of \pi
  */
  unsigned short int modulo_k_pi(const interval &I, double &k_left, double &k_right)
  {
    // The empty set, whose bounds are NaN, gives NaN and 0, tested first
    // (GAOL v5): the floor of its NaN bounds divided by pi would be the empty
    // set too, but k_left its lower bound, a NaN of the other sign
    if (I.is_empty()) {
      k_left = k_right = GAOL_NAN;
      return 0;
    }
    interval kl = floor(I.left()/interval::pi());
    interval kr = floor(I.right()/interval::pi());
    k_left  = kl.left();
    k_right = kr.right();
    return static_cast<unsigned short int>((kl.is_a_double() ? 2 : 0) + (kr.is_a_double() ? 1 : 0));
  }


  /*
    The bounds of [x + pi/2]/[pi] for a finite x, [pi] being [pi_dn, pi_up],
    rounded upward, as tan() needs them: computed on doubles rather than with
    the operations of intervals, which checked the rounding direction once more
    each, after the check of tan() (GAOL v5). They are the doubles
    interval(x) + interval::half_pi() and operator/=(interval) give, in the two
    builds: the stored bounds of the sum are nl = (-x) + (-half_pi_dn) and
    nr = x + half_pi_up, and the division by an interval above 0 divides a
    stored bound above 0 by pi_dn and one below 0 by pi_up. A stored bound 0
    stays 0, with a sign that may differ from the one of the operations of
    intervals (+0 or -0 for the lower bound, whose stored value the builds set
    each their own way); tan() only takes floor() and ceil() of these bounds
    and compares them, for which -0 and +0 are the same.
  */
  static inline double lower_of_x_plus_half_pi_over_pi(double x)
  {
    const double nl = (-x) + (-half_pi_dn);
    return -(nl / ((nl > 0.0) ? pi_dn : pi_up));
  }

  static inline double upper_of_x_plus_half_pi_over_pi(double x)
  {
    const double nr = x + half_pi_up;
    return nr / ((nr > 0.0) ? pi_dn : pi_up);
  }

  interval tan(const interval& I)
  {
    // Empty set?
    if (I.is_empty()) {
      return interval::emptyset();
    }

    // The only check of the rounding direction (GAOL v5): the width and the
    // quotients below are computed on doubles, where tan() called width(), +
    // and / on intervals, which checked it five times more
    GAOL_RND_ENTER();
    const double l = I.left(), r = I.right();
    /*
      The width of I, rounded upward as width() does: the exact width is at
      most w, and w <= pi_dn, the double below pi, proves it below pi, so that
      I holds at most one pole (GAOL v5, review #8 of examples/examples.md: the
      test was !(w < pi_up) in GAOL 4, and [-M_PI_2, M_PI_2], of width pi_dn, gave
      [-oo, +oo], as did the intervals whose exact width lies between the
      double below pi_dn and pi_dn, which round up to it; that the signs
      of cos at the bounds give the tightest bounds was measured on 29 400 intervals).
      Above pi_dn, I may hold two poles, the cosine having the same sign
      at both bounds: [-oo, +oo], the tightest bound but for the intervals
      holding no pole whose exact width is below pi. Also for a NaN width, from
      [+oo, +oo] or [-oo, -oo] built from SSE2 registers, which the
      constructors refuse: GAOL gave [-oo, +oo] for them too
    */
    const double w = r - l;
    if (!(w <= pi_dn)) {
      GAOL_RND_LEAVE();
      return interval::universe();
    }

    /*
      The poles of tan are the x for which (x + pi/2)/pi is an integer, which
      no double is. A and B enclose that quotient at the bounds of I: there is
      no pole within I when the lower bound of A and the upper bound of B have
      the same integer part, and there is one when an integer lies between the
      upper bound of A and the lower bound of B. When the quotients cannot
      tell, a bound of I being within about |x| 2^-52 of a pole, or beyond
      2^52, the signs of the cosine at the bounds do (GAOL v5, issue #6,
      see cos_or_sin()): I being narrower than pi, as w <= pi_dn shows, there
      is a pole within I exactly when they differ. GAOL gave [-oo, +oo] then,
      as for the double below pi/2, whose tangent is 0x1.9153d9443ed0bp+51, and
      for every interval beyond 2^52.
    */
    // l and r are finite, w being at most pi_dn
    const double A_left = lower_of_x_plus_half_pi_over_pi(l), A_right = upper_of_x_plus_half_pi_over_pi(l),
      B_left = lower_of_x_plus_half_pi_over_pi(r), B_right = upper_of_x_plus_half_pi_over_pi(r);
    bool no_pole = (std::floor(A_left) == std::floor(B_right));
    const bool told = no_pole || (std::ceil(A_right) <= std::floor(B_left));
    if (told && !no_pole) { // A pole within I for sure
      GAOL_RND_LEAVE();
      return interval::universe();
    }
    // The rounding direction is upward already, set at the top of tan()
    if (!told) {
      no_pole = (sign_of_cos(l) == sign_of_cos(r));
    }
    double u = -GAOL_INFINITY, v = GAOL_INFINITY;
    if (no_pole) {
      u = tan_lo(l);
      v = tan_hi(r);
    }
    GAOL_RND_KEEP(u); GAOL_RND_KEEP(v);
    GAOL_RND_LEAVE();
    return interval(u,v);
  }


  interval acos(const interval& I)
  {
    // The check before the intersection, whose bounds, held in registers,
    // acos_lo() and acos_hi() compare (see asinpi())
    GAOL_RND_ENTER();
    interval J = I & interval::minus_one_plus_one();
    // J <- I \cap [-1,1]

    if (J.is_empty()) {
      GAOL_RND_LEAVE();
      return interval::emptyset();
    }

    double l = acos_lo(J.right()), r = acos_hi(J.left());
    GAOL_RND_KEEP(l); GAOL_RND_KEEP(r);
    GAOL_RND_LEAVE();
    return interval(l,r);
  }

  interval asin(const interval& I)
  {
    GAOL_RND_ENTER();
    interval J= I & interval::minus_one_plus_one();
    // J <- I \cap [-1,1]

    if (J.is_empty()) {
      GAOL_RND_LEAVE();
      return interval::emptyset();
    }
    double l = asin_lo(J.left()), r = asin_hi(J.right());
    GAOL_RND_KEEP(l); GAOL_RND_KEEP(r);
    GAOL_RND_LEAVE();
    return interval(l,r);
  }

  interval atan(const interval& I)
  {
	if (I.is_empty()) {
	  return I;
	}
    GAOL_RND_ENTER();
    const double l = atan_lo(I.left()), r = atan_hi(I.right());
    GAOL_RND_LEAVE();
    return interval(l,r);
  }

  /*
    atan2 of IEEE 1788-2015 (Table 9.1), defined on the plane but (0, 0), with
    values in (-pi, pi] (GAOL v5; GAOL raised unavailable_feature_error).
    The angle of a point of the box Y x X is monotonic in y and in x in each
    quadrant, and its extrema are at corners of the box, which the signs of the
    bounds give. It jumps from pi to -pi across the half-line y = 0, x < 0: a
    box with points on that half-line and points below it has angles next to
    -pi and the angle pi, and [-pi, pi] is the hull of its angles.
  */
  // atan2() after its check, made before the bounds are compared: with
  // denormals-are-zero, atan2([100 2^-1074], [100 2^-1074]) was the empty
  // set, the point taken for (0, 0) (GAOL v5, see gaol/gaol_fpu.h)
  static interval atan2_upward(const interval& Y, const interval& X)
  {
    if (Y.is_empty() || X.is_empty()) {
      return interval::emptyset();
    }
    const double yl = Y.left(), yu = Y.right(), xl = X.left(), xu = X.right();
    if (yl == 0.0 && yu == 0.0 && xl == 0.0 && xu == 0.0) {
      return interval::emptyset();
    }
    if (yl < 0.0 && yu >= 0.0 && xl < 0.0) {
      return interval(-pi_up, pi_up);
    }
    double l, r;
    if (yl >= 0.0) { // Upper half-plane: the angle decreases with x
      // The least angle is at the right of the box, at the bottom if x > 0
      // there and at the top otherwise, and the greatest at the left. The box
      // {0} x [xl, 0] has no other corner there than (0, 0): its angle is pi,
      // and that of {0} x [0, xu] is 0
      l = atan2_lo((xu > 0.0) ? yl : yu, xu);
      r = (xl == 0.0 && yu == 0.0) ? 0.0 : atan2_hi((xl >= 0.0) ? yu : yl, xl);
    } else if (yu < 0.0) { // Lower half-plane, y = 0 left out: the angle increases with x
      l = atan2_lo((xl >= 0.0) ? yl : yu, xl);
      r = atan2_hi((xu > 0.0) ? yu : yl, xu);
    } else { // yl < 0 <= yu, in the right half-plane
      l = atan2_lo(yl, xl);
      r = (yu == 0.0) ? ((xu == 0.0) ? -half_pi_dn : 0.0) : atan2_hi(yu, xl);
    }
    return interval(l,r);
  }

  interval atan2(const interval& Y, const interval& X)
  {
    GAOL_RND_ENTER();
    interval res = atan2_upward(Y, X);
    GAOL_RND_KEEP(res);
    GAOL_RND_LEAVE();
    return res;
  }

  interval cosh(const interval& I)
  {
	if (I.is_empty()) {
		return I;
	}
    double l, r;
    GAOL_RND_ENTER();
    if (I.right() < 0) {
      l = cosh_lo(I.right());
      r = cosh_hi(I.left());
    } else if (I.left() > 0) {
      l = cosh_lo(I.left());
      r = cosh_hi(I.right());
    } else { // 0 \in I
      const double abs_l = -I.left();
      l = 1.0;
      r = cosh_hi((abs_l >= I.right()) ? abs_l : I.right());
    }
    GAOL_RND_LEAVE();
    return interval(l,r);
  }

  interval sinh(const interval& I)
  {
	if (I.is_empty()) {
		return I;
	}
	GAOL_RND_ENTER();
	const double l = sinh_lo(I.left()), r = sinh_hi(I.right());
	GAOL_RND_LEAVE();
	return interval(l,r);
  }

  interval tanh(const interval& I)
  {
	if (I.is_empty()) {
		return I;
	}
	GAOL_RND_ENTER();
	const double l = tanh_lo(I.left()), r = tanh_hi(I.right());
	GAOL_RND_LEAVE();
	return interval(l,r) & interval::minus_one_plus_one();
  }


  interval acosh(const interval& I)
  {
    // The check before the intersection, as in acos()
    GAOL_RND_ENTER();
    interval J = I &  interval::one_plus_infinity();

    if (J.is_empty()) {
      GAOL_RND_LEAVE();
      return J;
    }

    double l = acosh_lo(J.left()), r = acosh_hi(J.right());
    GAOL_RND_KEEP(l); GAOL_RND_KEEP(r);
  	GAOL_RND_LEAVE();
  	return interval(l,r);
  }



  interval asinh(const interval& I)
  {
    if (I.is_empty()) {
      return I;
    }

    GAOL_RND_ENTER();
    const double l = asinh_lo(I.left()), r = asinh_hi(I.right());
    GAOL_RND_LEAVE();
    return interval(l,r);

  }

  interval atanh(const interval& I)
  {
    // The check before the intersection, as in acos()
    GAOL_RND_ENTER();
	  interval J = I & interval::minus_one_plus_one();
    // atanh is defined on (-1, 1) (IEEE 1788-2015, Table 9.1, GAOL v5): an I
    // meeting [-1, 1] at 1 alone, or at -1 alone, as [1] and [-5,-1], holds
    // no point of it, and gives the empty set. The value of CORE-MATH at 1 is
    // +oo, and the lower bound the double below the value: [1] was [MAX,+oo]
    // ([-1] was empty only because interval(-oo,-oo) is). The limits -oo at
    // -1 and +oo at 1 remain the bounds when I holds other points of (-1, 1)
    if (J.is_empty() || J.left() == 1.0 || J.right() == -1.0) {
      GAOL_RND_LEAVE();
      return interval::emptyset();
    }
    double l = atanh_lo(J.left()), r = atanh_hi(J.right());
    GAOL_RND_KEEP(l); GAOL_RND_KEEP(r);
    GAOL_RND_LEAVE();
    return interval(l,r);
  }

  /*
    k pi + X, k being an integer double and X a bounded interval, enclosed
    within about one double (GAOL v5, issue #6): pi = pi_hi + pi_lo,
    pi_hi being the double below pi, and pi_lo lying between two consecutive
    doubles; k pi_hi is p + e exactly, p being the product rounded and e its
    rest, from fma(); k pi_lo is bounded by the products with the two
    doubles, rounded outward. The bounds of X are added to e + k pi_lo, and
    the sums to p, rounded once at the magnitude of the result: adding X to
    k pi rounded would round twice, and the bounds could be two doubles from
    the tightest. GAOL computed k [pi_dn, pi_up] + X, whose width, k 2^-51,
    the relational functions took on: their bounds were up to
    2^-49 max(1, |x|) from the values they had to keep. The rounding
    direction is upward.
  */
  static interval k_pi_plus(double k, const interval& X)
  {
    // 0x1.1a62633145c06p-53 and 0x1.1a62633145c07p-53, the doubles around
    // pi - pi_dn = 1.2246467991473531772e-16
    static const double pi_lo_dn = std::ldexp(4967757600021510.0,-105);
    static const double pi_lo_up = std::ldexp(4967757600021511.0,-105);
    const double p = k*pi_dn;
    const double e = std::fma(k,pi_dn,-p);
    // The products of k with the doubles around pi_lo, rounded outward
    const double below = (k >= 0.0) ? pi_lo_dn : pi_lo_up, above = (k >= 0.0) ? pi_lo_up : pi_lo_dn;
    const double lo_lo = -((-k)*below), lo_hi = k*above;
    const double hi = p + ((e + lo_hi) + X.right());
    const double lo = -((-p) + (((-e) - lo_lo) - X.left()));
    return interval(lo,hi);
  }

  /*
    The bounds of interval(x)/interval::pi() + shift, for a finite x of
    magnitude at most 2^52, rounded upward: the doubles operator/=(interval)
    and operator+=(double) give, in the two builds, computed on doubles. The
    division of interval(x), whose stored bounds are -x and x, by
    [pi_dn, pi_up] divides a stored bound above 0 by pi_dn and one below 0 by
    pi_up, and gives interval::zero() for x = 0. The stored bounds of
    interval::zero() are -0 and +0 in the FPU build, +0 and +0 in the SSE2
    build, and the sign of the zero reaches the bounds of acos_rel() through
    k_pi_plus(): they are taken from interval::zero(), -left() being the
    stored lower bound, rather than written 0.
  */
  static inline double lower_of_x_over_pi_plus(double x, double shift)
  {
    const double nl = (x == 0.0) ? -interval::zero().left() : (-x) / ((x < 0.0) ? pi_dn : pi_up);
    return -(nl - shift);
  }

  static inline double upper_of_x_over_pi_plus(double x, double shift)
  {
    const double nr = (x == 0.0) ? interval::zero().right() : x / ((x > 0.0) ? pi_dn : pi_up);
    return nr + shift;
  }

  /*
    The hull of the x of I whose image by a periodic function is in J (the
    relational acos_rel(), asin_rel() and atan_rel()), the preimage of J
    being the union of the pieces piece(i), the piece i lying on
    [(i - shift) pi, (i + 1 - shift) pi], shift being 0 for the cosine and
    1/2 for the sine and the tangent (GAOL v5, issue #6: GAOL had three
    copies of this, with k [pi]).

    A bound of I lies on the piece floor(x/pi + shift), or on the next one
    when the quotient, rounded outward, is off by one: the leftmost point of
    the preimage within I is on the piece of the left bound or the next one,
    the union of the pieces being symmetric about every multiple of pi (plus
    shift pi), and the rightmost point on the piece of the right bound or the
    previous one. Beyond 2^52 the quotient may be off by more, and I is kept
    as it is on that side; an I of a single double is decided by the image of
    the function, at every magnitude.

    The rounding direction is checked once (GAOL v5): the inverse image of J
    (inverse(), with the bounds of acos_lo(), asin_lo()... and no check of
    their own) and the quotients x/pi + shift are computed after the check
    here, where the relational functions called acos(J), asin(J) or atan(J),
    and four operations of intervals, which checked it once more each.

    For acos_rel() and asin_rel(), whose function has its image in [-1, 1]
    (bounded), a J outside [-1, 1] has no preimage, and a J containing it has
    the whole line: these are decided after the check too, though the modes
    flushing the subnormals to zero do not change how J compares with -1 and
    1, so that the check, which clears the modes, is made on every path of a
    non-empty J and I (GAOL v5, second review of point 4).
  */
  template<class Inverse, class Piece, class Image>
  static interval periodic_rel(const interval& J, const interval& I, double shift, bool bounded, Inverse inverse,
                               Piece piece, Image image)
  {
    if (J.is_empty() || I.is_empty()) {
      return interval::emptyset();
    }
    // The check before the bounds are compared (see gaol/gaol_fpu.h)
    GAOL_RND_ENTER();
    if (bounded && (J.left() > 1.0 || J.right() < -1.0)) {
      GAOL_RND_LEAVE();
      return interval::emptyset();
    }
    if (bounded && J.set_contains(interval::minus_one_plus_one())) {
      GAOL_RND_LEAVE();
      return I;
    }
    if (I.left() == I.right()) {
      interval K = image(I) & J;
      GAOL_RND_KEEP(K);
      GAOL_RND_LEAVE();
      return K.is_empty() ? interval::emptyset() : I;
    }
    const interval Jinv = inverse(J);
    interval Ileft, Iright;
    if (std::fabs(I.left()) > two_power_52) {
      Ileft = I;
    } else {
      const double kl = std::floor(lower_of_x_over_pi_plus(I.left(), shift));
      Ileft = piece(kl, Jinv) & I;
      if (Ileft.is_empty()) {
	Ileft = piece(kl + 1.0, Jinv) & I;
      }
    }
    if (std::fabs(I.right()) > two_power_52) {
      Iright = I;
    } else {
      const double kr = std::floor(upper_of_x_over_pi_plus(I.right(), shift));
      Iright = piece(kr, Jinv) & I;
      if (Iright.is_empty()) {
	Iright = piece(kr - 1.0, Jinv) & I;
      }
    }
    GAOL_RND_KEEP(Ileft); GAOL_RND_KEEP(Iright);
    GAOL_RND_LEAVE();
    if (Ileft.is_empty() || Iright.is_empty()) {
      return interval::emptyset();
    }
    return interval(Ileft.left(),Iright.right());
  }

  interval acos_rel(const interval& J, const interval &I)
  {
    // The preimage of J: i pi + acos(J) for an even i, (i + 1) pi - acos(J)
    // for an odd i. acos(J) as acos() computes it, J & [-1, 1] being non-empty
    return periodic_rel(J, I, 0.0, true,
			[](const interval& X) {
			  const interval K = X & interval::minus_one_plus_one();
			  return interval(acos_lo(K.right()), acos_hi(K.left()));
			},
			[](double i, const interval& Jacos) { return feven(i) ? k_pi_plus(i, Jacos) : k_pi_plus(i + 1.0, -Jacos); },
			[](const interval& X) { return cos(X); });
  }

  interval asin_rel(const interval& J, const interval &I)
  {
    // The preimage of J: i pi + asin(J) for an even i, i pi - asin(J) for an
    // odd i (GAOL v5: GAOL computed pi/2 + acos_rel(J, I - pi/2), two
    // additions of an enclosure of pi/2 more). asin(J) as asin() computes it,
    // J & [-1, 1] being non-empty
    return periodic_rel(J, I, 0.5, true,
			[](const interval& X) {
			  const interval K = X & interval::minus_one_plus_one();
			  return interval(asin_lo(K.left()), asin_hi(K.right()));
			},
			[](double i, const interval& Jasin) { return k_pi_plus(i, feven(i) ? Jasin : -Jasin); },
			[](const interval& X) { return sin(X); });
  }

  interval atan_rel(const interval& J, const interval& I)
  {
    if (I.is_empty() || J.is_empty()) {
      return interval::emptyset();
    }
    // The preimage of J: i pi + atan(J), atan(J) as atan() computes it
    return periodic_rel(J, I, 0.5, false,
			[](const interval& X) { return interval(atan_lo(X.left()), atan_hi(X.right())); },
			[](double i, const interval& Jatan) { return k_pi_plus(i, Jatan); },
			[](const interval& X) { return tan(X); });
  }

  /*
    acosh_rel(), asinh_rel() and atanh_rel() check the rounding direction
    before they compare the bounds of the inverse image with those of I, which
    acosh(), asinh() and atanh() return with the modes that flush the
    subnormals to zero restored, with GAOL_PRESERVE_ROUNDING (GAOL v5, see
    gaol/gaol_fpu.h)
  */
  static interval acosh_rel_upward(const interval &J, const interval &I)
  {
    if (I.is_empty() || J.is_empty()) {
      return interval::emptyset();
    }

    interval tmp = acosh(J);

    if (I.certainly_positive()) {
      return I & tmp;
    }
    if (I.certainly_negative()) {
      return I & (-tmp);
    }
    return (I & -tmp) | (I & tmp);
  }

  interval acosh_rel(const interval &J, const interval &I)
  {
    GAOL_RND_ENTER();
    interval res = acosh_rel_upward(J, I);
    GAOL_RND_KEEP(res);
    GAOL_RND_LEAVE();
    return res;
  }

  interval asinh_rel(const interval &J, const interval &I)
  {
    GAOL_RND_ENTER();
    interval res = asinh(J) & I;
    GAOL_RND_KEEP(res);
    GAOL_RND_LEAVE();
    return res;
  }

  interval atanh_rel(const interval &J, const interval &I)
  {
    GAOL_RND_ENTER();
    interval res = atanh(J) & I;
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

  /*
   * invabs_rel: the hull of the x of I whose magnitude is in J, absRev of
   * IEEE 1788, the negative part of J holding no magnitude (GAOL v5:
   * GAOL took it as one, invabs_rel([-2, -1], I) being [-2, 2] & I instead of
   * the empty set, and invabs_rel([-2, 0], I) [-2, 2] & I instead of [0])
   */
  interval invabs_rel(const interval &Jall, const interval &I)
  {
    const interval J = Jall & interval::positive();
    if (J.is_empty()) {
      return interval::emptyset();
    }
    if (I.certainly_geq(interval::zero())) {
      return (J & I);
    }

    if (I.certainly_leq(interval::zero())) {
      return ((-J) & I);
    }
    return (J&I) | ((-J)&I);
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
	// subnormal bounds is 0/0 (GAOL v5, point Q of TODO.md). Before,
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
    maximum() and minimum() give NaN bounds for an empty I or J, which give
    the empty set: the constructor tells them with a quiet comparison (GAOL
    v5). They were told first, with one more comparison, when the constructor
    compared them with <=, which raises the invalid-operation exception on a
    NaN; testing I and J first made max() 5 to 10% slower (Clang 18).
  */
  interval  max(const interval &I, const interval &J)
  {
    return interval(maximum(I.left(),J.left()), maximum(I.right(),J.right()));
  }

  interval  min(const interval &I, const interval &J)
  {
    return interval(minimum(I.left(),J.left()), minimum(I.right(),J.right()));
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
      // flush the subnormals to zero (GAOL v5, point Q of TODO.md)
      if (gaol_detail::bound_less(right(), 0.0)) {
	return right();
      } else { // left() > 0.0
	return left();
      }
    }
  }

  double interval::mig(void) const
  {
    if (is_empty()) {
      return GAOL_NAN;
    } else {
      if (set_contains(0)) {
	return 0.0;
      }
      // Compared as in the relations (gaol_port.h): under denormals-are-zero,
      // mig([-3, -2]*2^-1074) was 0 (GAOL v5, point Q of TODO.md)
      if (gaol_detail::bound_less(right(), 0.0)) {
	return -right();
      } else { // left() > 0.0
	return left();
      }
    }
  }

  double interval::mag(void) const
  {
    return maximum(std::fabs(left()),std::fabs(right()));
  }


  interval::operator std::string() const
  {
    // Not named output, the format of the intervals written, which Visual C++
    // warned that it hid (C4458, GAOL v5)
    std::ostringstream text;
    text.precision(interval::precision());
    text << *this;
    return text.str();
  }

  std::streamsize interval::precision(void)
  {
    return output_precision;
  }

  std::streamsize interval::precision(std::streamsize n)
  {
    std::streamsize old = output_precision;
    output_precision = n;
    return old;
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
      half of it (GAOL v5, point Q of TODO.md). The comparisons above are
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


  interval sqrt(const interval& I)
  {
#if defined(__x86_64__) && GAOL_USING_SSE2_INSTRUCTIONS && GAOL_HAVE_AVX512_TARGET
    /* The AVX-512 path: the roots by the embedded rounding, the part of I
       in [0, +oo] and its emptiness by the sign and the magnitude bits,
       which the denormals-are-zero mode does not change (the SSE2 path
       checks the modes for the intersection, a lower bound -1e-310 having
       been compared equal to 0) */
    if (avx512_arithmetic && !I.is_empty()) {
      return interval(fast_sqrt(I.get_xmminterval()));
    }
#endif
    // The part of I in [0, +oo], as nth_root() takes it: the intersection
    // compares the bounds with quiet comparisons, and keeps an empty I as it
    // is, where the constructor, given its NaN bounds, raised the
    // invalid-operation exception (GAOL v5). A lower bound -0 is taken as 0
    // below. The check before the intersection: with denormals-are-zero, a
    // lower bound -1e-310 compared equal to 0 and stayed, and its root, once
    // the check had cleared the mode, made the result empty (GAOL v5, see
    // gaol/gaol_fpu.h)
    GAOL_RND_ENTER();
    const interval Ipos = I & interval::positive();

    if (Ipos.is_empty()) {
      GAOL_RND_LEAVE();
      return interval::emptyset();
    }

    double l = (Ipos.left() == 0.0) ? 0.0 : -gaol_minus_sqrt_down(Ipos.left(), gaol_sqrt_up(Ipos.left()));
    double r = gaol_sqrt_up(Ipos.right());
    GAOL_RND_KEEP(l); GAOL_RND_KEEP(r);
    GAOL_RND_LEAVE();
    return interval(l,r);
  }


  // sqrt_rel() after its check, made before the bounds are compared, as in
  // sqrt()
  static interval sqrt_rel_upward(const interval& J, const interval& I)
  {
    // The part of J in [0, +oo], computed as in sqrt() (GAOL v5)
    const interval Jpos = J & interval::positive();

    if (Jpos.is_empty() || I.is_empty()) {
      return interval::emptyset();
    }

	double l, r;

    // The lower bound as sqrt() computes it, in the upward rounding: GAOL set
    // the direction downward for it, then upward again, two changes of
    // direction per call whichever way GAOL is built (GAOL v5)
    if (Jpos.left() == 0.0) {
      l = 0.0;
      r = gaol_sqrt_up(Jpos.right());
    } else {
      const double x = Jpos.left();
      l = -gaol_minus_sqrt_down(x, gaol_sqrt_up(x));
      r = gaol_sqrt_up(Jpos.right());
    }

	interval Res(l,r);

    if (I.certainly_positive()) {
      return Res & I;
    }
    if (I.certainly_negative()) {
      return (-Res) & I;
    }
    return (I & Res) | (I & (-Res));
  }

  interval sqrt_rel(const interval& J, const interval& I)
  {
    GAOL_RND_ENTER();
    interval res = sqrt_rel_upward(J, I);
    GAOL_RND_KEEP(res);
    GAOL_RND_LEAVE();
    return res;
  }


  /*
    cos(I), and sin(I) as cos(I - pi/2) (template argument sine).

    The bounds of I divided by an enclosure of pi, minus 1/2 for the sine,
    rounded outward, give the pieces [m pi, n pi] on which I lies: on one
    piece (n - m < 2), the function is monotonic, and its bounds are the values
    of the mathematical library at the bounds of I. sin is computed from the
    sine of the mathematical library (GAOL v5): GAOL computed
    cos(I - [pi/2]), whose subtraction widened I by about 2^-52 max(2, |x|), and
    sin([1e-10]) was 4.4e-16 wide.

    Otherwise (GAOL v5, issue #6), I is on several pieces for sure when
    the same divisions rounded inward give the same m and n: an extremum is
    within I when n - m is 2, and both extrema are beyond. When they do not,
    a bound of I being within about |x| 2^-52 of an extremum, or beyond 2^52,
    where the quotients are integers, the divisions cannot tell, and GAOL took
    the extremum as reached: cos([2^60]) was [-1, 1]. The signs of the
    derivative at the bounds of I then tell, exactly, the mathematical library
    being correctly rounded: for I narrower than 2 pi, the derivative has at
    most two zeros within I; signs that differ at the bounds show one, a
    minimum or a maximum according to their order; the same signs show none
    for I narrower than pi, and none or two beyond, which the sign at the
    middle of I tells, each half being narrower than pi. The bounds are then
    within one double of the tightest ones at every magnitude.
  */
  template<bool sine>
  static inline interval cos_or_sin(double Il, double Ir)
  {
    // I is not empty, Il being the opposite of its left bound, as the SSE2
    // intervals store it, and Ir its right bound
    // The only check of the rounding direction: nothing below changes it, and
    // the bounds are computed with the functions of namespace upward (GAOL v5).
    // Il and Ir, read before it, are compared after it (see gaol/gaol_fpu.h)
    GAOL_RND_ENTER();
    Il = rnd_reread(Il);
    Ir = rnd_reread(Ir);
    double a,b;
    double Ileft = -Il;
    double Iright = Ir;

    // a <= Ileft/pi, b >= Iright/pi
    if (Ileft >= 0) {
      a = -(Il/pi_up);
      b = Ir/pi_dn;
    } else {
      a = -(Il/pi_dn);
      if (Iright > 0) {
	b = Ir/pi_dn;
      } else {
	b = Ir/pi_up;
      }
    }
    if (sine) {
      // a <= (Ileft - pi/2)/pi, b >= (Iright - pi/2)/pi, rounded upward
      a = -((-a) + 0.5);
      b = b - 0.5;
    }

    double m=std::floor(a),
      n=std::ceil(b),
      u = -1.0, v = 1.0;

    double nm = (n-m); // Must be rounded towards +oo

    if (nm < 2.0) {
      // even(m)? No conversion to int in order to avoid overflow
      const bool even_m = feven(m);
      if (even_m) { // Decreasing, as cos on [m pi, (m+1) pi]; use of cos(x)=cos(-x)
	u = sine ? sin_lo(Iright) : cos_lo(Iright);
	v = sine ? sin_hi(Ileft) : cos_hi(Ileft);
      } else { // Increasing
	u = sine ? sin_lo(Ileft) : cos_lo(Ileft);
	v = sine ? sin_hi(Iright) : cos_hi(Iright);
      }
    } else {
      // The width of I, rounded upward: both extrema from 2 pi on, or for
      // infinite bounds
      const double w = Ir + Il;
      if (!(w < 2.0*pi_dn)) {
	GAOL_RND_LEAVE();
	return interval::minus_one_plus_one();
      }
      // a_in >= Ileft/pi and b_in <= Iright/pi, minus 1/2 for the sine
      double a_in, b_in;
      if (Ileft >= 0) {
	a_in = Ileft/pi_dn;
	b_in = -((-Ir)/pi_up);
      } else {
	a_in = Ileft/pi_up;
	b_in = (Iright > 0) ? -((-Ir)/pi_up) : -((-Ir)/pi_dn);
      }
      if (sine) {
	a_in = a_in - 0.5;
	b_in = -((-b_in) + 0.5);
      }
      // The halves of I, for the sign at its middle
      const double middle = 0.5*Ileft + 0.5*Iright;
      const bool narrow_halves = (middle + Il < pi_dn) && (Ir - middle < pi_dn);
      const bool sure = (std::floor(a_in) == m && std::ceil(b_in) == n);
      if (sure && nm != 2.0) { // Both extrema
	GAOL_RND_LEAVE();
	return interval::minus_one_plus_one();
      }
      const bool even_m = feven(m);

      // -1: a minimum within I; 1: a maximum; 0: none; 2: both
      int extremum;
      if (sure) {
	extremum = even_m ? -1 : 1;
      } else {
	// The derivatives are -sin and cos: s is the sign of the slope, taken
	// on the side of I at a bound that is 0
	int sl = sine ? sign_of_cos(Ileft) : -sign_of_sin(Ileft);
	int sr = sine ? sign_of_cos(Iright) : -sign_of_sin(Iright);
	if (sl == 0) {
	  sl = -1; // cos decreases on the right of 0
	}
	if (sr == 0) {
	  sr = 1; // and increases on its left
	}
	if (sl != sr) {
	  extremum = sl; // Increasing then decreasing: a maximum
	} else if (w < pi_dn
		   || (narrow_halves && (sine ? sign_of_cos(middle) : -sign_of_sin(middle)) == sl)) {
	  extremum = 0;
	} else {
	  extremum = 2;
	}
	if (extremum == 0) { // Monotonic
	  if (sl < 0) {
	    u = sine ? sin_lo(Iright) : cos_lo(Iright);
	    v = sine ? sin_hi(Ileft) : cos_hi(Ileft);
	  } else {
	    u = sine ? sin_lo(Ileft) : cos_lo(Ileft);
	    v = sine ? sin_hi(Iright) : cos_hi(Iright);
	  }
	}
      }
      if (extremum == -1) {
	const double
	  u1 = sine ? sin_hi(Ileft) : cos_hi(Ileft),
	  u2 = sine ? sin_hi(Iright) : cos_hi(Iright);
	u = -1.0;
	v = ((u1 > u2) ? u1 : u2);
      } else if (extremum == 1) {
	const double
	  u1 = sine ? sin_lo(Ileft) : cos_lo(Ileft),
	  u2 = sine ? sin_lo(Iright) : cos_lo(Iright);
	u = ((u1 < u2) ? u1 : u2);
	v = 1.0;
      } else if (extremum == 2) {
	u = -1.0;
	v = 1.0;
      }
    }
    // The values of the mathematical library moved outward may leave [-1,1]
    // (GAOL v5)
    if (u < -1.0) {
      u = -1.0;
    }
    if (v > 1.0) {
      v = 1.0;
    }
    GAOL_RND_KEEP(u); GAOL_RND_KEEP(v);
    GAOL_RND_LEAVE();
    return interval(u,v);
  }

  interval cos(const interval& I)
  {
    if (I.is_empty()) {
      return interval::emptyset();
    }
    return cos_or_sin<false>(I.left_internal(),I.right_internal());
  }

  interval sin(const interval& I)
  {
    if (I.is_empty()) {
      return interval::emptyset();
    }
    return cos_or_sin<true>(I.left_internal(),I.right_internal());
  }


} // namespace gaol_core

namespace gaol_ieee1788 {

  /*
    pow(x, y) of IEEE 1788-2015 (gaol/gaol_ieee1788.h), in the library rather
    than inline in its header, for the bounds at doubles of
    gaol/gaol_double_op.h, which no installed header includes (GAOL v5): it is
    pow_standard(), whose body gaol_pow_hybrid() calls too, for every exponent
    but a degenerate integer
  */
  interval pow(const interval& x, const interval& y)
  {
    return ::gaol_core::pow_standard(x, y);
  }

  /*
    intervalToText(x) of IEEE 1788-2015 (gaol/gaol_ieee1788.h, 13.3): the
    bounds of x rounded outward, [l, r], [a] for a point interval whose double
    the digits write exactly, as [4], or [empty], with the C locale, the flags
    a stream starts with and the digits of interval::precision() (16 unless
    the program sets another), so that the text is a portable literal
    (12.11.5) whatever the global output format, the flags of a stream and the
    locale of the program (GAOL v5). It is the text operator<< writes in the
    bounds format under these settings. GAOL wrote what operator<< writes: the
    width format "1.5 (+/- 0.5)", the agreeing digits, a decimal comma under
    the locale of a program that sets one, and <4, 4> for the point interval
    4, none of them a literal, and the text changed with the format another
    thread was setting.
  */
  std::string intervalToText(const interval& x)
  {
    // The flags a new stream starts with, the digits of the intervals and the C locale
    const ::gaol_core::text_format fmt(std::ios_base::skipws | std::ios_base::dec, interval::precision(),
                                       std::locale::classic());
    std::string out;
    // The guard, rather than GAOL_RND_ENTER() and GAOL_RND_LEAVE():
    // display_bounds() allocates, and a failed allocation left the direction
    // upward, which GAOL_PRESERVE_ROUNDING promises to set back (GAOL v5)
    const ::gaol_core::rounding_guard rnd;
    ::gaol_core::round_upward_if_needed();
    ::gaol_core::display_bounds(x.left(), x.right(), out, fmt);
    return out;
  }

} // namespace gaol_ieee1788

namespace gaol {

  /*
    textToInterval(s) of gaol/gaol_interval.h: what the constructor
    interval(const char*) of GAOL 4 did, which GAOL v5 no longer has
  */
  interval textToInterval(const std::string& s)
  {
    interval tmp;
    if (!parse_interval(s.c_str(),tmp)) {
      std::string err_msg("Syntax error in interval initialization: ");
      err_msg += s;
      GAOL_ERRNO = -1;
      gaol_ERROR(input_format_error,err_msg.c_str());
    }
    return tmp;
  }

  // The constructor interval(const char*, const char*) of GAOL 4, as a
  // function (GAOL v5): the left bound of the interval sl writes and the
  // right bound of the one sr writes
  interval textToInterval(const std::string& sl, const std::string& sr)
  {
    interval tmpl, tmpr;
    if (!parse_interval(sl.c_str(),tmpl)) {
      std::string err_msg("Syntax error in left bound initialization: ");
      err_msg += sl;
      GAOL_ERRNO = -1;
      gaol_ERROR(input_format_error,err_msg.c_str());
      return interval::emptyset();
    }
    if (!parse_interval(sr.c_str(),tmpr)) {
      std::string err_msg("Syntax error in right bound initialization: ");
      err_msg += sr;
      GAOL_ERRNO = -1;
      gaol_ERROR(input_format_error,err_msg.c_str());
      return interval::emptyset();
    }
    // An empty one gives the empty set, from its NaN bounds, which the
    // constructor tells with a quiet comparison (GAOL v5: it was told first,
    // when the constructor compared them with <=, which raises the
    // invalid-operation exception on a NaN)
    return interval(tmpl.left(), tmpr.right());
  }

} // namespace gaol

/*
  The intervals of floats, gaol::intervalf and gaol::interval2f: unfinished,
  and compiled only where a developer of GAOL defines GAOL_FLOAT_INTERVALS
  (see gaol/gaol_config.h); none of the three builds has an option for them
  (GAOL v5)
*/
#ifdef GAOL_FLOAT_INTERVALS
#  include "gaol/gaol_intervalf.cpp"
#  if GAOL_USING_SSE3_INSTRUCTIONS
#    include "gaol/gaol_interval2f.cpp"
#  endif
#endif // GAOL_FLOAT_INTERVALS
