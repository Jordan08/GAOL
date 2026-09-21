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
 * CVS: $Id: gaol_interval.cpp 281 2015-05-23 12:50:28Z goualard $
 * Last modified: Sun May 10 22:40:50 2009 on almighty
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
#include <cstdint>
#include <sstream>

#if USING_SSE2_INSTRUCTIONS
#  include <pmmintrin.h>
#endif

#include "gaol/gaol_config.h"
#include "gaol/gaol_limits.h"
#include "gaol/gaol_fpu.h"
#include "gaol/gaol_common.h"
#include "gaol/gaol_parser.h"
#include "gaol/gaol_port.h"
#include "gaol/gaol_interval.h"
#include "gaol/gaol_parameters.h"
#include "gaol/gaol_limits.h"
#include "gaol/gaol_exceptions.h"

using std::ostream;
using std::istream;
using std::sqrt;

//debug
using std::cout;
using std::endl;


namespace gaol {

const interval interval::cst_two_pi(2.0*pi_dn,2.0*pi_up); // No rounding when multiplying by 2
const interval interval::cst_pi(pi_dn,pi_up);
const interval interval::cst_half_pi(half_pi_dn,half_pi_up);
const interval interval::cst_one_plus_infinity(1.0,GAOL_INFINITY);
const interval interval::cst_one(1.0);
const interval interval::cst_minus_one_plus_one(-1.0,1.0);



  // I^e for a non-empty I and e > 0, defined below: uipow(), in the files
  // included here, calls it
  static interval uipow_nonempty(const interval& I, unsigned int e);

#if USING_SSE2_INSTRUCTIONS
#  include "gaol/gaol_interval_sse.cpp"
#else
#  include "gaol/gaol_interval_fpu.cpp"
#endif // USING_SSE2_INSTRUCTIONS

  /*
    x^n rounded upward and downward from exact products, x >= 0 and n >= 2, the
    rounding direction being upward (GAOL v5, issue #7). GAOL rounds each
    product of its binary exponentiation outward (uipow_rounded()), and x^n is
    then up to n + 1 doubles from the tightest bound: 2, 3, 5, 6, 8 for n = 3
    to 7.

    The power is kept as h + l, l being small: a product h*y, rounded upward,
    is p, and fma(h, y, -p) is the rest h*y - p <= 0, a double, exactly. The
    upper bound keeps h + l above the power, l <= 0 being rounded upward. The
    lower bound keeps h - nl below it, nl >= 0 being rounded upward, and the
    square of nl, which would raise it, left out. The power is only rounded at the
    end, h + l upward and h - nl downward: the bounds are the tightest, or one
    double beyond where the power is within n 2^-104 of a double, and exact
    where the power is a double. false when a product is not finite, or is
    below 2^-968, its rest being no double then: the rounded products handle
    these powers, 0 included.
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
      nl = std::fma(-h,h,p) + (2.0*h)*nl; // p - h*h, rounded upward in an underflow only
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
    I^e for a non-empty I and e > 0, as pow() and uipow() call it: from exact
    products for e > 2, the square being the tightest already, and from the
    rounded products where a power is not finite or is below 2^-968
  */
  static interval uipow_nonempty(const interval& I, unsigned int e)
  {
    if (e < 3) {
      return uipow_rounded(I,e);
    }
    GAOL_RND_ENTER();
    const double a = I.left(), b = I.right();
    double l = 0.0, r = 0.0, t = 0.0;
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
    GAOL_RND_KEEP(l);
    GAOL_RND_KEEP(r);
    GAOL_RND_LEAVE();
    return finite ? interval(l,r) : uipow_rounded(I,e);
  }



  // Default format for output
  interval_format::format_t interval::output = interval_format::bounds;

  // Number of digits to print.
  std::streamsize interval::output_precision = 16;

  // Declaring some prototypes included by the files below.
  double inv_dn(double);
  double inv_up(double);

  /* Square roots rounded upward and downward, whatever the rounding of ::sqrt.
     IEEE 754 requires a square root to be rounded in the rounding direction in
     effect, but the C library of Visual C++ for 32-bit x86 rounds it to nearest
     in every direction. The result of ::sqrt, correctly rounded in some
     direction and thus within one float of the exact root, is compared with the
     exact root through s*s rounded in the other direction, and moved to the next
     float on the other side when it is not a bound yet; where ::sqrt rounds as
     it should, it is left unchanged. The next float is reached by adding the
     smallest denormal, rather than with nextafter(), which IBEX found to crash
     on ARM64 macOS when not rounding to nearest. To be called with the rounding
     direction set upward, respectively downward.

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

  static double gaol_sqrt_down(double x)
  {
    double s = ::sqrt(x);
    if (-((-s)*s) > x) { // s*s rounded upward > x proves s > sqrt(x)
      s -= std::numeric_limits<double>::denorm_min();
    }
    return s;
  }

  /* The square root of x rounded downward, u being the square root rounded
     upward, to be called with the rounding direction set upward: u when it is
     the exact root, the double below u otherwise. u*u rounded upward is x
     exactly when u is the exact root, u*u being above x otherwise. Returned
     negated, as the SSE2 intervals store their lower bound (GAOL v5: GAOL
     computed x/u rounded downward, one double below the tightest bound for half
     of the doubles). */
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
    They are called between GAOL_RND_ENTER() and
    GAOL_RND_LEAVE(), as the functions of gaol::nearest they call.
  */
  static inline double sin_lo(double x) { return (x == 0.0) ? 0.0 : sin_dn(x); }
  static inline double sin_hi(double x) { return (x == 0.0) ? 0.0 : sin_up(x); }
  static inline double cos_lo(double x) { return (x == 0.0) ? 1.0 : cos_dn(x); }
  static inline double cos_hi(double x) { return (x == 0.0) ? 1.0 : cos_up(x); }
  static inline double tan_lo(double x) { return (x == 0.0) ? 0.0 : tan_dn(x); }
  static inline double tan_hi(double x) { return (x == 0.0) ? 0.0 : tan_up(x); }

  /*
    The signs of sin(x) and cos(x), x being finite (GAOL v5, issue #6).
    Neither is 0 but sin(0): no other double is a multiple of pi/2, and the
    closest one, 6381956970095103 2^797, has a cosine of 4.7e-19, so that the
    value of the mathematical library, moved one double downward, still has
    the sign of the function. Next to 0 the sign is known from x. Rounding to nearest
    (GAOL_RND_ENTER()).
  */
  static inline int sign_of_sin(double x)
  {
    if (std::fabs(x) < 3.0) {
      return (x > 0.0) - (x < 0.0);
    }
    return (sin_dn(x) > 0.0) ? 1 : -1;
  }

  static inline int sign_of_cos(double x)
  {
    if (std::fabs(x) < 1.5) {
      return 1;
    }
    return (cos_dn(x) > 0.0) ? 1 : -1;
  }

  static double asin_lo(double x)
  {
    return (x == 0.0) ? 0.0 : ((x == 1.0) ? half_pi_dn : ((x == -1.0) ? -half_pi_up : asin_dn(x)));
  }

  static double asin_hi(double x)
  {
    return (x == 0.0) ? 0.0 : ((x == 1.0) ? half_pi_up : ((x == -1.0) ? -half_pi_dn : asin_up(x)));
  }

  static double acos_lo(double x)
  {
    return (x == 1.0) ? 0.0 : ((x == 0.0) ? half_pi_dn : ((x == -1.0) ? pi_dn : acos_dn(x)));
  }

  static double acos_hi(double x)
  {
    return (x == 1.0) ? 0.0 : ((x == 0.0) ? half_pi_up : ((x == -1.0) ? pi_up : acos_up(x)));
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
    return atan_dn(x);
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
    return atan_up(x);
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
    return maximum(atan2_dn(y,x), -pi_up);
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
    return minimum(atan2_up(y,x), pi_up);
  }

  // x^y for x > 0: 1 for x = 1 or y = 0, and at least 0
  static inline double pow_lo(double x, double y)
  {
    return (x == 1.0 || y == 0.0) ? 1.0 : maximum(nthroot_dn(x,y), 0.0);
  }

  static inline double pow_hi(double x, double y)
  {
    return (x == 1.0 || y == 0.0) ? 1.0 : nthroot_up(x,y);
  }

  static inline double sinh_lo(double x)
  {
    return (x == 0.0) ? 0.0 : ((x >= 711.0) ? std::numeric_limits<double>::max() : sinh_dn(x));
  }

  static inline double sinh_hi(double x)
  {
    return (x == 0.0) ? 0.0 : ((x <= -711.0) ? -std::numeric_limits<double>::max() : sinh_up(x));
  }

  // cosh(x), cosh(-x): the lower bound for |x|, the upper bound for 0
  static inline double cosh_lo(double x)
  {
    return (x == 0.0) ? 1.0 : ((std::fabs(x) >= 711.0) ? std::numeric_limits<double>::max() : cosh_dn(x));
  }

  static inline double cosh_hi(double x)
  {
    return (x == 0.0) ? 1.0 : cosh_up(x);
  }

  // 1 - 2^-53, the double below 1, exactly
  static inline double tanh_lo(double x)
  {
    return (x == 0.0) ? 0.0 : ((x >= 20.0) ? 1.0 - 0.5*std::numeric_limits<double>::epsilon() : tanh_dn(x));
  }

  static inline double tanh_hi(double x)
  {
    return (x == 0.0) ? 0.0 : ((x <= -20.0) ? -(1.0 - 0.5*std::numeric_limits<double>::epsilon()) : tanh_up(x));
  }

  static inline double asinh_lo(double x) { return (x == 0.0) ? 0.0 : asinh_dn(x); }
  static inline double asinh_hi(double x) { return (x == 0.0) ? 0.0 : asinh_up(x); }
  static inline double acosh_lo(double x) { return (x == 1.0) ? 0.0 : acosh_dn(x); }
  static inline double acosh_hi(double x) { return (x == 1.0) ? 0.0 : acosh_up(x); }
  static inline double atanh_lo(double x) { return (x == 0.0) ? 0.0 : atanh_dn(x); }
  static inline double atanh_hi(double x) { return (x == 0.0) ? 0.0 : atanh_up(x); }

  /*
    \brief test for evenness
    \warning d should not be +/-oo
  */
  bool feven(double d)
  {
    return (std::floor(0.5*d)*2.0 == d);
  }





  /*!
    \brief Computes 1/a rounded upward
  */
  INLINE double inv_up(double a)
  {
    GAOL_RND_ENTER();
    double tmp = 1.0/a;
    GAOL_RND_KEEP(tmp);
    GAOL_RND_LEAVE();
    return tmp;
  }


  /*!
    \brief Computes 1/a rounded downward
  */
  INLINE double inv_dn(double a)
  {
    GAOL_RND_ENTER();
    double res = f_negate(1/(-a));
    GAOL_RND_KEEP(res);
    GAOL_RND_LEAVE();
    return res;
  }


  bool interval::is_canonical(void) const
  {
#if defined (_MSC_VER)
    return !is_empty() && (next_float(left())>=right());
#else
    // emptyset handled thanks to unorderedness of NaNs
    return next_float(left())>=right();
#endif
  }


  interval::interval(const char *const s)
  {
    interval tmp;
    bool ok = parse_interval(s,tmp);
    if (!ok) {
      std::string err_msg("Syntax error in interval initialization: ");
      err_msg += s;
      *this = interval::emptyset();
      GAOL_ERRNO = -1;
      gaol_ERROR(input_format_error,err_msg.c_str());
    } else {
#if USING_SSE2_INSTRUCTIONS
      xmmbounds = tmp.xmmbounds;
#else
      lb_ = tmp.lb_;
      rb_ = tmp.rb_;
#endif
    }
  }

  interval::interval(const char *const sl, const char *const sr)
  {
    interval tmpl, tmpr;
    if (!parse_interval(sl,tmpl)) {
      std::string err_msg("Syntax error in left bound initialization: ");
      err_msg += sl;
      *this = interval::emptyset();
      GAOL_ERRNO = -1;
      gaol_ERROR(input_format_error,err_msg.c_str());
    }
    if (!parse_interval(sr,tmpr)) {
	  std::string err_msg("Syntax error in right bound initialization: ");
      err_msg += sr;
      *this = interval::emptyset();
      GAOL_ERRNO = -1;
      gaol_ERROR(input_format_error,err_msg.c_str());
    }
#if USING_SSE2_INSTRUCTIONS
    xmm2d tmp = {tmpl.left_internal(), tmpr.right_internal()};
    xmmbounds = _mm_load_pd(tmp);
#else
    lb_ = tmpl.lb_;
    rb_ = tmpr.rb_;
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


  istream& operator >>(istream& is, interval& I)
  {
    std::string buffer;

    getline(is,buffer);

    if (!parse_interval(buffer.c_str(),I)) {
      std::string err_msg("Syntax error in expression of interval: ");
      err_msg += buffer;
      I = interval::emptyset();
      gaol_ERROR(input_format_error,err_msg.c_str());
    }
    return is;
  }

  // Defined in gaol_interval_lexer.lpp: the sign of v - x, v being the number s
  // writes, with no sign, compared exactly with the double x
  int compare_number_with_double(const char *s, double x);

  /*
    Moves the last digit of text by one unit, away from zero or toward it,
    until the number it writes is on the side of the double `magnitude` asked
    for: at least it if `away`, at most it otherwise. text is a positive
    number written to nearest in the fixed format, or in the scientific one if
    `scientific`: one digit, the others after a point, and an exponent with
    its sign.
  */
  void move_to_side(std::string& text, double magnitude, bool away, bool scientific)
  {
    for (int moves = 0; moves < 4; ++moves) {
      const std::size_t last = scientific ? text.find_first_of("eE") : text.size(); // One past the mantissa
      const int sign = compare_number_with_double(text.c_str(), magnitude);
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
    The text of the bound x of an interval, written as os would write it (its
    precision, its flags), and rounded downward, or upward if `upward`,
    whatever the C library does. GAOL set the rounding direction and let the C
    library write x, but not every C library rounds its decimal conversions in
    the rounding direction: the C runtime of Windows rounds the magnitude, so
    that -2/3 rounded downward was written -0.6666, and musl on 64-bit ARM
    processors rounds to nearest whatever the direction (issue #3). The
    magnitude is now written rounding to nearest, in the fixed format or in
    the scientific one, and compared exactly with x: when it is on the wrong
    side of x, its last digit is moved by one. The general format is made from
    the scientific one, by the rules of printf's %g, rather than asked of the
    C library: glibc 2.31 writes 999999.5 with six digits and the zeros kept
    "1.e+06", whose last digit is not the sixth.
    Left to the C library, with the rounding direction set: what is not a
    finite nonzero number, and the hexadecimal floating-point format.
  */
  std::string bound_to_text(double x, bool upward, const ostream& os)
  {
    const std::ios_base::fmtflags floatfield = os.flags() & std::ios_base::floatfield;
    if (x == 0.0 || !is_finite(x) || floatfield == (std::ios_base::fixed | std::ios_base::scientific)) {
      std::ostringstream as_it_was;
      as_it_was.copyfmt(os);
      as_it_was.width(0);
      if (!upward) {
        round_downward();
      }
      as_it_was << x;
      round_upward();
      return as_it_was.str();
    }

    const bool fixed = (floatfield == std::ios_base::fixed);
    const bool general = (floatfield == 0);
    // %g takes a precision of 0 as 1, and writes that many significant digits
    const std::streamsize digits = (general && os.precision() <= 0) ? 1 : os.precision();
    std::ostringstream out;
    out.imbue(std::locale::classic());
    out.setf(fixed ? std::ios_base::fixed : std::ios_base::scientific, std::ios_base::floatfield);
    out.precision(general ? digits - 1 : digits);
    const double magnitude = std::fabs(x);
    round_nearest();
    out << magnitude;
    round_upward();
    std::string text = out.str();
    // The magnitude written has to be at least that of x when rounding away
    // from zero, and at most that of x otherwise
    move_to_side(text, magnitude, upward == (x > 0.0), !fixed);

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
      if (!(os.flags() & std::ios_base::showpoint)) {
        mantissa.erase(mantissa.find_last_not_of('0') + 1);
        if (mantissa[mantissa.size() - 1] == '.') {
          mantissa.erase(mantissa.size() - 1);
        }
      }
      text = mantissa + tail;
    } else if (os.precision() == 0 && (os.flags() & std::ios_base::showpoint)) {
      text.insert(fixed ? text.size() : text.find('e'), 1, '.');
    }

    const char point = std::use_facet<std::numpunct<char> >(os.getloc()).decimal_point();
    for (std::size_t i = 0; i < text.size(); ++i) {
      if (text[i] == '.') {
        text[i] = point;
      } else if (text[i] == 'e' && (os.flags() & std::ios_base::uppercase)) {
        text[i] = 'E';
      }
    }
    if (x < 0.0) {
      text.insert(0, 1, '-');
    } else if (os.flags() & std::ios_base::showpos) {
      text.insert(0, 1, '+');
    }
    return text;
  }

  void display_bounds(double l, double r, ostream& os)
  {
    if (!(l <= r)) {
      os << "[empty]";
    } else {
      const std::string left = bound_to_text(l, false, os);
      const std::string right = bound_to_text(r, true, os);
      if (l == r) {
				os << '<' << left << ", " << right << '>';
      } else {
				os << '[' << left << ", " << right << ']';
      }
    }
  }


  /*
    A bound written in the hexadecimal-significand form of IEEE 1788-2015
    (13.4.1), which interval(const char*) reads back exactly (GAOL v5).

    The recovery requirement of 13.4 asks that writing an interval and reading
    it again give the same bounds. GAOL wrote the sixteen hexadecimal digits of
    each double instead, which is no interval literal at all: the parser
    refused them, and the note of 13.4.1 gives that very form as the one that
    fails the readability test. printf("%a") writes as many digits as the value
    needs, the C standard asking for an exact representation when no precision
    is given, and the lexer already reads that form.
  */
  static void write_hexa_bound(double x, char *buf, std::size_t n)
  {
    if (x == GAOL_INFINITY) {
      std::snprintf(buf, n, "inf");
    } else if (x == -GAOL_INFINITY) {
      std::snprintf(buf, n, "-inf");
    } else {
      std::snprintf(buf, n, "%a", x);
    }
  }

  ostream& operator<<(ostream& os, const interval& I)
  {
    //    double l = ((I.left()==0.0) ? 0.0  : I.left()); // Avoids printing -0
    //    double r = ((I.right()==0.0) ? 0.0 : I.right());  // Avoids printing -0
    GAOL_RND_PRESERVE();
	round_upward();

    double l = I.left(), r = I.right();

    os.precision(interval::precision());

    switch (interval::format()) {
    case interval_format::bounds: // Display in the form "[ l, r ]"
      display_bounds(l,r,os);
      break;
    case interval_format::hexa: // The exact text representation of 13.4
      if (I.is_empty()) {
				os << "[empty]";
      } else {
				char lo[64], hi[64];
				write_hexa_bound(l, lo, sizeof lo);
				write_hexa_bound(r, hi, sizeof hi);
				os << '[' << lo << ", " << hi << ']';
      }
      break;
    case interval_format::width: // Display in the form "c (+/- w)"
      if (I.is_empty()) {
				os << "empty";
      } else {
				if (l == r) {
	  			os << l;
				} else {
	  			round_nearest();
	  			if (l == -GAOL_INFINITY) {
	    			if (r == GAOL_INFINITY) { // [-oo, +oo]
	      			os << 0.0;
	    			} else {                        // [-oo, x]
	      			os << -std::numeric_limits<double>::max();
	    			}
	  			} else {
	    			if (r == GAOL_INFINITY) { // [x, +oo]
	      			os << std::numeric_limits<double>::max();
	    			} else {                        // [x, y]
	      			os << ((l+r)/2.0);
	    			}
	  			}
	  				os << " (+/- " << ((r-l)/2.0) << ")";
				}
      }
      break;
    case interval_format::center: // Display in the form "c"
      if (I.is_empty()) {
				os << "empty";
      } else {
				if (l == r) {
	  			os << l;
				} else {
	  			round_nearest();
	  			if (l == -GAOL_INFINITY) {
	    			if (r == GAOL_INFINITY) { // [-oo, +oo]
	      			os << 0.0;
	    			} else {                        // [-oo, x]
	      			os << -std::numeric_limits<double>::max();
	    			}
	  			} else {
	    			if (r == GAOL_INFINITY) { // [x, +oo]
	      			os << std::numeric_limits<double>::max();
	    			} else {                        // [x, y]
	      			os << ((l+r)/2.0);
	    			}
	  			}
				}
      }
      break;
    case interval_format::agreeing:
      if (I.is_empty()) {
				os << "[empty]";
      } else {
				if (I.right() > 10*I.left()) {
	  			display_bounds(l,r,os);
				} else {
	  			std::ostringstream lbound, rbound;
	  			std::string itv;
	  			lbound.precision(interval::precision());
	  			rbound.precision(interval::precision());

	  			lbound << std::showpoint;
	  			rbound << std::showpoint;
	  			lbound << bound_to_text(I.left(), false, lbound);
	  			rbound << bound_to_text(I.right(), true, rbound);

	  			// The characters both bounds start with, then what is left of
	  			// each, without the zeros ending it (GAOL v5: GAOL dropped
	  			// from both bounds the characters after the last one of the left
	  			// bound that is not a zero, and wrote [1.25, 1.2567] "1.25~[, ]";
	  			// zeros ending an exponent are not dropped)
	  			const std::string lb = lbound.str(), rb = rbound.str();
	  			std::size_t i = 0;
	  			while (i < lb.length() && i < rb.length() && lb[i] == rb[i]) {
	    			itv += lb[i];
	    			++i;
	  			}
	  			if (i < lb.length() || i < rb.length()) {
	    			const std::string *bounds[2] = { &lb, &rb };
	    			itv += "~[";
	    			for (int k = 0; k < 2; ++k) {
	      			const std::string& b = *bounds[k];
	      			std::size_t end = b.length();
	      			if (b.find_first_of("eE") == std::string::npos && b.find('.') != std::string::npos) {
	        			while (end > i && end > b.find('.') + 1 && b[end - 1] == '0') {
	          			--end;
	        			}
	      			}
	      			itv += (end > i) ? b.substr(i, end - i) : std::string("0");
	      			itv += (k == 0) ? ", " : "]";
	    			}
	  			}
	  			os << itv;
				}
      }
    }
    GAOL_RND_RESTORE();
    return os;
  }



	interval pow(const interval& I, int n)
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
			*/
			const unsigned int m = 0u - static_cast<unsigned int>(n);
			const interval p = uipow_nonempty(I,m);
			const double largest = std::numeric_limits<double>::max();
			if (p.left() >= -largest && p.right() <= largest) {
				return inverse(p);
			}
			if (I.left() < 0.0 && I.right() > 0.0) {
				if (odd(m)) {
					return interval::universe();
				}
				const double g = I.mag();
				if (g == GAOL_INFINITY) {
					return interval::positive();
				}
				return interval(uipow(inverse(interval(g)),m).left(),GAOL_INFINITY);
			}
			return uipow(inverse(I),m);
		} else {
			if (n > 0) {
				return uipow_nonempty(I,static_cast<unsigned int>(n));
			} else {
				return interval(1.0);
			}
		}
	}

  interval pow(const interval &I, const interval &J)
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
        return pow(I,int(J.left()));
      }
      // An integer beyond the ints, for which pow(I,int) cannot be called
      return interval::universe();
    }

    /*
      x^y is only real for a negative x when y is an integer (GAOL v5,
      ported from the fix of Codac, commit 74086ccb, Jordan Ninin). GAOL
      computed the powers of the negative part of I on its magnitude, and
      pow([-4,-1],[0.5,0.5]) returned [-1,2] where sqrt([-4,-1]) is empty.
    */
    const interval base = I & interval::positive();
    if (base.is_empty()) {
      return interval::emptyset();
    }
    // pow(0,y) is 0 for y > 0, and has no value for y <= 0 (Table 9.1, footnote
    // c), which exp(J*log([0])) does not give, log([0]) being empty
    if (base.right() == 0.0) {
      return J.right() > 0.0 ? interval::zero() : interval::emptyset();
    }
    /*
      For a base above 0 and finite bounds, CORE-MATH's pow at the corners of
      the box (GAOL v5, issue #8): x^y increases with y for x > 1 and decreases
      for x < 1, increases with x for y > 0 and decreases for y < 0, so that
      its extrema over I x J are at corners, which the places of the bounds
      about 1 and 0 give. CORE-MATH's pow is correctly rounded in the upward
      rounding GAOL computes in, which gives the upper bound, and the double
      below it the lower one: each bound is one double from the tightest one
      at most, where exp(J*log(I)) multiplied the relative width of log(I) by
      |y log(x)|: pow([2], [1023.5]) was 1425 doubles below and 748 above.
      A base from 0, whose powers are from 0 for exponents above 0, takes its
      upper bound so. The other boxes (a base from 0 with an exponent that is
      not above 0, an infinite bound) keep exp(J*log(I)), which gives their
      limits.
    */
    const double xl = base.left(), xu = base.right(), yl = J.left(), yu = J.right();
    const double dmax = (std::numeric_limits<double>::max)();
    if (xu <= dmax && yl >= -dmax && yu <= dmax && (xl > 0.0 || yl > 0.0)) {
      double l, r;
      GAOL_RND_ENTER();
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
      GAOL_RND_LEAVE();
      return interval(l,r);
    }
    return exp(J*log(base));
  }

  /*!
    \brief I^p for a floating-point p (GAOL v5)

    Without it, pow(I,2.5) called pow(const interval&, int), converting a
    double to an int being a standard conversion and converting it to an
    interval a user-defined one: the exponent was truncated, and pow([4],0.5)
    returned [1]. An integer p within the ints is computed by
    pow(const interval&, int), which is defined for negative bases too, any
    other p by pow(const interval&, const interval&), and an infinite or NaN p
    gives the empty set. Ported from the fix of Codac (commit 74086ccb, Jordan
    Ninin).
  */
  interval  pow(const interval& I, double p)
  {
    if (!(std::fabs(p) <= (std::numeric_limits<double>::max)())) { // Infinite or NaN
      return interval::emptyset();
    }
    if (std::floor(p) == p && p >= (std::numeric_limits<int>::min)() && p <= (std::numeric_limits<int>::max)()) {
      return pow(I, static_cast<int>(p));
    }
    return pow(I, interval(p));
  }

  /*
    Code inspired by ia_math code by Timothy Hickey
  */
  interval nth_root_rel(const interval& J, unsigned int n, const interval& I)
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
    The power d^e of the mathematical library, e being 1/n rounded, for the
    approximation the n-th root of d starts from. The roots of 0, 1 and +oo are
    themselves. Rounding to nearest (GAOL_RND_ENTER())
  */
  static inline bool root_is_itself(double d)
  {
    return d == 0.0 || d == 1.0 || d == GAOL_INFINITY;
  }

  static inline double root_near(double d, double e)
  {
    return root_is_itself(d) ? d : nthroot_up(d,e);
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
    The approximations the n-th roots of a >= 0 and b >= 0 start from, the
    rounding direction being set to nearest once for both. Rounding upward
  */
  static void near_roots(double a, double b, unsigned int n, double& near_a, double& near_b)
  {
    GAOL_RND_ENTER();
    const double e = 1.0/double(n);
    near_a = root_near(a,e);
    near_b = (b == a) ? near_a : root_near(b,e);
    GAOL_RND_KEEP(near_a);
    GAOL_RND_KEEP(near_b);
    GAOL_RND_LEAVE();
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
	const interval J = odd(n) ? I : (I & interval::positive());
	if (J.is_empty()) {
		return interval::emptyset();
	}
	// The roots of the magnitudes of the bounds, the root of x < 0 being
	// -(-x)^(1/n)
	GAOL_RND_ENTER();
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

  ULONGLONGINT nb_fp_numbers(double a, double b)
  {
    if (!is_finite(a) || !is_finite(b) || (a > b)) {
      // Either a or b is a NaN or +/-oo, or [a,b] is empty?
      gaol_ERROR(invalid_action_error,"invalid argument(s) in call to nb_fp_numbers()");
      return std::numeric_limits<ULONGLONGINT>::max();
    }

    if (a == b) {
      return 1;
    }

    ullidouble ai, bi;
    ai.d = a;
    bi.d = b;
    if (a >= 0) {
      return (bi.i-ai.i)+1;
    }
    if (b <= 0) {
      return (ai.i-bi.i)+1;
    }
    ullidouble zi;
    zi.d = 0.0;
    ai.d = -ai.d;
    return (bi.i-zi.i)+(ai.i-zi.i)+1;
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
    const double l = I.left(), r = I.right();
    GAOL_RND_ENTER();
    const double w = gaol_cr_exp2(l);
    const double u = exp2_is_exact(l) ? w : previous_float(w);
    const double v = gaol_cr_exp2(r);
    GAOL_RND_LEAVE();
    // Within [0, +oo], as exp: 2^x is positive
    return interval(maximum(0.0, u), v);
  }

  interval exp10(const interval& I)
  {
    if (I.is_empty()) {
      return interval::emptyset();
    }
    const double l = I.left(), r = I.right();
    GAOL_RND_ENTER();
    const double w = gaol_cr_exp10(l);
    const double u = exp10_is_exact(l) ? w : previous_float(w);
    const double v = gaol_cr_exp10(r);
    GAOL_RND_LEAVE();
    return interval(maximum(0.0, u), v);
  }

  interval log2(const interval& I)
  {
    // Defined on (0, +oo), as log: I holding no positive number gives the
    // empty set (IEEE 1788-2015, Table 9.1)
    if (I.is_empty() || !(I.right() > 0.0)) {
      return interval::emptyset();
    }
    const double l = maximum(0.0, I.left()), r = I.right();
    GAOL_RND_ENTER();
    const double w = gaol_cr_log2(l);
    const double u = log2_is_exact(l) ? w : previous_float(w);
    const double v = gaol_cr_log2(r);
    GAOL_RND_LEAVE();
    return interval(u, v);
  }

  interval log10(const interval& I)
  {
    if (I.is_empty() || !(I.right() > 0.0)) {
      return interval::emptyset();
    }
    const double l = maximum(0.0, I.left()), r = I.right();
    GAOL_RND_ENTER();
    const double w = gaol_cr_log10(l);
    const double u = log10_is_exact(l) ? w : previous_float(w);
    const double v = gaol_cr_log10(r);
    GAOL_RND_LEAVE();
    return interval(u, v);
  }

  /*
    The forward functions IEEE 1788-2015 recommends (Table 10.5) that CORE-MATH
    provides without the 128-bit integer (GAOL v5)

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
  // unless it is a double
  static interval increasing_cr(const interval& I, double (*f)(double),
                                bool (*exact)(double), double lowest, double highest)
  {
    if (I.is_empty()) {
      return interval::emptyset();
    }
    const double l = I.left(), r = I.right();
    GAOL_RND_ENTER();
    const double w = f(l);
    const double u = exact(l) ? w : previous_float(w);
    const double v = f(r);
    GAOL_RND_LEAVE();
    return interval(maximum(lowest, u), minimum(highest, v));
  }

  interval expm1(const interval& I)
  {
    // within [-1, +oo]: b^x - 1 exceeds -1, which it nears as x goes to -oo
    return increasing_cr(I, gaol_cr_expm1, expm1_is_exact, -1.0, GAOL_INFINITY);
  }

  interval exp2m1(const interval& I)
  {
    return increasing_cr(I, gaol_cr_exp2m1, exp2m1_is_exact, -1.0, GAOL_INFINITY);
  }

  interval exp10m1(const interval& I)
  {
    return increasing_cr(I, gaol_cr_exp10m1, exp10m1_is_exact, -1.0, GAOL_INFINITY);
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
    if (I.is_empty() || I.right() < -1.0 || I.left() > 1.0) {
      return interval::emptyset();
    }
    const double l = maximum(-1.0, I.left()), r = minimum(1.0, I.right());
    GAOL_RND_ENTER();
    // decreasing: the upper bound at the left bound, the lower one at the right
    const double v = gaol_cr_acospi(l);
    const double w = gaol_cr_acospi(r);
    const double u = acospi_is_exact(r) ? w : previous_float(w);
    GAOL_RND_LEAVE();
    return interval(maximum(0.0, u), minimum(1.0, v));
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
  // the values at the bounds otherwise
  static interval sin_or_cos_pi(const interval& I, double (*f)(double),
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
    GAOL_RND_ENTER();
    const double fl = f(l), fr = f(r);
    const double dl = is_multiple_of_inverse(l, 2.0) ? fl : previous_float(fl);
    const double dr = is_multiple_of_inverse(r, 2.0) ? fr : previous_float(fr);
    GAOL_RND_LEAVE();
    const double lower = has_min ? -1.0 : maximum(-1.0, minimum(dl, dr));
    const double upper = has_max ? 1.0 : minimum(1.0, maximum(fl, fr));
    return interval(lower, upper);
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

  interval tanpi(const interval& I)
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
    GAOL_RND_ENTER();
    if (!pole_at_l) {
      const double fl = gaol_cr_tanpi(l);
      lower = is_multiple_of_inverse(l, 4.0) ? fl : previous_float(fl);
    }
    if (!pole_at_r) {
      upper = gaol_cr_tanpi(r);
    }
    GAOL_RND_LEAVE();
    return interval(lower, upper);
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
    return (a == 0.0 || b == 0.0) ? c : -gaol::rnd_keep(std::fma(-a, b, -c));
  }

  interval fma(const interval& X, const interval& Y, const interval& Z)
  {
    if (X.is_empty() || Y.is_empty() || Z.is_empty()) {
      return interval::emptyset();
    }
    const double xl = X.left(), xr = X.right(), yl = Y.left(), yr = Y.right();
    const double zl = Z.left(), zr = Z.right();
    double lower = -GAOL_INFINITY, upper = GAOL_INFINITY;
    GAOL_RND_ENTER();
    if (zl != -GAOL_INFINITY) {
      lower = minimum(minimum(fma_down(xl, yl, zl), fma_down(xl, yr, zl)),
                      minimum(fma_down(xr, yl, zl), fma_down(xr, yr, zl)));
    }
    if (zr != GAOL_INFINITY) {
      upper = maximum(maximum(fma_up(xl, yl, zr), fma_up(xl, yr, zr)),
                      maximum(fma_up(xr, yl, zr), fma_up(xr, yr, zr)));
    }
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

  interval cancel_minus(const interval& X, const interval& Y)
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
    GAOL_RND_ENTER();
    // [xl - yl, xr - yr] rounded outward: xl - yl downward as -(yl - xl)
    const double lower = -(yl - xl);
    const double upper = xr - yr;
    GAOL_RND_LEAVE();
    return interval(lower, upper);
  }

  interval cancel_plus(const interval& X, const interval& Y)
  {
    return cancel_minus(X, -Y);
  }

  interval exp(const interval& I)
  {
	/* We intersect the result with [0, +oo] to ensure that the result is strictly positive
 		Otherwise, we might have: exp([-oo, -MAX] = [-v, +v] with v very small.
	*/
    // exp(0) = 1 exactly, where the value of the mathematical library moved
    // outward gave exp([0]) a width, and pow([1], [-oo, +oo]) = [0, +oo]
    // (GAOL v5)
    const double l = I.left(), r = I.right();
    GAOL_RND_ENTER();
    const double u = (l == 0.0) ? 1.0 : exp_dn(l);
    const double v = (r == 0.0) ? 1.0 : exp_up(r);
    GAOL_RND_LEAVE();
    return interval(maximum(0.0,u), v);
  }

  interval log(const interval& I)
  {
    // log is defined on (0,+oo) (IEEE 1788-2015, Table 9.1, GAOL v5): I
    // holding no positive number, as [-4,0] and [0], gives the empty set, where
    // GAOL kept its part in [0,+oo] and gave [-oo,-MAX]
    if (I.is_empty() || !(I.right() > 0.0)) {
      return interval::emptyset();
    }

    // log(1) = 0 exactly: log([1]) was [-2^-1074, 2^-1074] (GAOL v5)
    const double l = maximum(0.0,I.left()), r = I.right();
    // CORE-MATH's log, correctly rounded in the rounding direction in effect
    // (GAOL v5): in the upward rounding GAOL computes in, RU(log r) is the
    // right bound, and RD(log l) = pred(RU(log l)) the left one, log(l) being
    // no double for l other than 1. The tightest bounds, without switching the
    // rounding direction.
    GAOL_RND_ENTER();
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
    interval kl = floor(I.left()/interval::pi());
    interval kr = floor(I.right()/interval::pi());
    k_left  = kl.left();
    k_right = kr.right();
    return static_cast<unsigned short int>((kl.is_a_double() ? 2 : 0) + (kr.is_a_double() ? 1 : 0));
  }


  interval tan(const interval& I)
  {
    GAOL_RND_ENTER();
    // Empty set?
    if (I.is_empty()) {
      GAOL_RND_LEAVE();
      return interval::emptyset();
    }

    if (I.width() >= pi_up) {
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
      see cos_or_sin()): I being narrower than pi, there is a pole within I
      exactly when they differ. GAOL gave [-oo, +oo] then, as for the double
      below pi/2, whose tangent is 0x1.9153d9443ed0bp+51, and for every
      interval beyond 2^52.
    */
    const double l = I.left(), r = I.right();
    const interval A = (interval(l) + interval::half_pi())/interval::pi(),
      B = (interval(r) + interval::half_pi())/interval::pi();
    bool no_pole = (std::floor(A.left()) == std::floor(B.right()));
    const bool told = no_pole || (std::ceil(A.right()) <= std::floor(B.left()));
    if (told && !no_pole) { // A pole within I for sure
      GAOL_RND_LEAVE();
      return interval::universe();
    }
    // Rounded upward
    const bool narrower_than_pi = (r - l < pi_dn);
    // The rounding direction is upward already, set at the top of tan()
    if (!told && narrower_than_pi) {
      no_pole = (sign_of_cos(l) == sign_of_cos(r));
    }
    double u = -GAOL_INFINITY, v = GAOL_INFINITY;
    if (no_pole) {
      u = tan_lo(l);
      v = tan_hi(r);
    }
    GAOL_RND_LEAVE();
    GAOL_RND_KEEP(u); GAOL_RND_KEEP(v);
    GAOL_RND_LEAVE();
    return interval(u,v);
  }


  interval acos(const interval& I)
  {
    interval J = I & interval::minus_one_plus_one();
    // J <- I \cap [-1,1]

    if (J.is_empty()) {
      return interval::emptyset();
    }

    GAOL_RND_ENTER();
    const double l = acos_lo(J.right()), r = acos_hi(J.left());
    GAOL_RND_LEAVE();
    return interval(l,r);
  }

  interval asin(const interval& I)
  {
    interval J= I & interval::minus_one_plus_one();
    // J <- I \cap [-1,1]

    if (J.is_empty()) {
      return interval::emptyset();
    }
    GAOL_RND_ENTER();
    const double l = asin_lo(J.left()), r = asin_hi(J.right());
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
  interval atan2(const interval& Y, const interval& X)
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
    GAOL_RND_ENTER();
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
    GAOL_RND_LEAVE();
    return interval(l,r);
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
    interval J = I &  interval::one_plus_infinity();

    if (J.is_empty()) {
      return J;
    }

  	GAOL_RND_ENTER();
  	const double l = acosh_lo(J.left()), r = acosh_hi(J.right());
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
	  interval J = I & interval::minus_one_plus_one();
    if (J.is_empty()) {
      return interval::emptyset();
    }
    GAOL_RND_ENTER();
    const double l = atanh_lo(J.left()), r = atanh_hi(J.right());
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
  */
  template<class Piece, class Image>
  static interval periodic_rel(const interval& J, const interval& I, double shift, Piece piece, Image image)
  {
    if (J.is_empty() || I.is_empty()) {
      return interval::emptyset();
    }
    if (I.left() == I.right()) {
      return (image(I) & J).is_empty() ? interval::emptyset() : I;
    }
    GAOL_RND_ENTER();
    interval Ileft, Iright;
    if (std::fabs(I.left()) > two_power_52) {
      Ileft = I;
    } else {
      const double kl = std::floor((interval(I.left())/interval::pi() + shift).left());
      Ileft = piece(kl) & I;
      if (Ileft.is_empty()) {
	Ileft = piece(kl + 1.0) & I;
      }
    }
    if (std::fabs(I.right()) > two_power_52) {
      Iright = I;
    } else {
      const double kr = std::floor((interval(I.right())/interval::pi() + shift).right());
      Iright = piece(kr) & I;
      if (Iright.is_empty()) {
	Iright = piece(kr - 1.0) & I;
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
    if (J.is_empty() || I.is_empty() || J.left() > 1.0 || J.right() < -1.0) {
      return interval::emptyset();
    }
    if (J.set_contains(interval::minus_one_plus_one())) {
      return I;
    }
    // The preimage of J: i pi + acos(J) for an even i, (i + 1) pi - acos(J)
    // for an odd i
    const interval Jacos = acos(J);
    return periodic_rel(J, I, 0.0,
			[&](double i) { return feven(i) ? k_pi_plus(i, Jacos) : k_pi_plus(i + 1.0, -Jacos); },
			[](const interval& X) { return cos(X); });
  }

  interval asin_rel(const interval& J, const interval &I)
  {
    if (J.is_empty() || I.is_empty() || J.left() > 1.0 || J.right() < -1.0) {
      return interval::emptyset();
    }
    if (J.set_contains(interval::minus_one_plus_one())) {
      return I;
    }
    // The preimage of J: i pi + asin(J) for an even i, i pi - asin(J) for an
    // odd i (GAOL v5: GAOL computed pi/2 + acos_rel(J, I - pi/2), two
    // additions of an enclosure of pi/2 more)
    const interval Jasin = asin(J);
    return periodic_rel(J, I, 0.5,
			[&](double i) { return k_pi_plus(i, feven(i) ? Jasin : -Jasin); },
			[](const interval& X) { return sin(X); });
  }

  interval atan_rel(const interval& J, const interval& I)
  {
    if (I.is_empty() || J.is_empty()) {
      return interval::emptyset();
    }
    // The preimage of J: i pi + atan(J)
    const interval Jatan = atan(J);
    return periodic_rel(J, I, 0.5,
			[&](double i) { return k_pi_plus(i, Jatan); },
			[](const interval& X) { return tan(X); });
  }

  interval acosh_rel(const interval &J, const interval &I)
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

  interval asinh_rel(const interval &J, const interval &I)
  {
    return asinh(J) & I;
  }

  interval atanh_rel(const interval &J, const interval &I)
  {
    return atanh(J) & I;
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
	GAOL_RND_PRESERVE();
	round_nearest();
	if (std::fabs(I.left()) <= std::fabs(I.right())) {
	  res = I.left() / I.right();
	} else {
	  res = I.right() / I.left();
	}
	// Computed rounding to nearest: kept before the direction changes (see gaol_fpu.h)
	res = gaol::rnd_keep(res);
	GAOL_RND_RESTORE();
	return res;
      }
    }
  }

  bool interval::is_finite(void) const
  {
    return !std::isinf(left()) && !std::isinf(right());
  }

  interval  max(const interval &I, const interval &J)
  {
    return interval(maximum(I.left(),J.left()),maximum(I.right(),J.right()));
  }

  interval  min(const interval &I, const interval &J)
  {
    return interval(minimum(I.left(),J.left()),minimum(I.right(),J.right()));
  }


  double interval::smig(void) const
  {
    if (is_empty()) {
      return GAOL_NAN;
    } else {
      if (set_contains(0)) {
	return 0.0;
      }
      if (right() < 0.0) {
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
      if (right() < 0.0) {
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
    std::ostringstream output;
    output.precision(interval::precision());
    output << *this;
    return output.str();
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

    GAOL_RND_PRESERVE();
    round_nearest();
    double middle = 0.5*(left()+right());
	 if (std::isinf(middle)) {
		middle = 0.5*left() + 0.5*right();
	 }
    // Computed rounding to nearest: kept before the direction changes (see gaol_fpu.h)
    middle = gaol::rnd_keep(middle);
    GAOL_RND_RESTORE();
    return middle;
  }


  interval sqrt(const interval& I)
  {
    interval Ipos = interval(maximum(0.0,I.left()),I.right());

    if (Ipos.is_empty()) {
      return interval::emptyset();
    }

    if (Ipos.left() == 0.0) {
			GAOL_RND_ENTER();
			interval tmp = interval(0.0,gaol_sqrt_up(Ipos.right()));
      GAOL_RND_KEEP(tmp);
      GAOL_RND_LEAVE();
      return tmp;
    } else {
			GAOL_RND_ENTER();
			double l = gaol_minus_sqrt_down(Ipos.left(), gaol_sqrt_up(Ipos.left()));
			double r = gaol_sqrt_up(Ipos.right());
      GAOL_RND_KEEP(l); GAOL_RND_KEEP(r);
      GAOL_RND_LEAVE();
      return interval(-l,r);
    }
  }


  interval sqrt_rel(const interval& J, const interval& I)
  { // TODO: Rewrite sqrt_rel (eliminate round_downward()?)
    interval Jpos = interval(maximum(0.0,J.left()),J.right());

    if (Jpos.is_empty() || I.is_empty()) {
      return interval::emptyset();
    }

	double l, r;

    GAOL_RND_ENTER();
    if (Jpos.left() == 0.0) {
      l = 0.0;
      r = gaol_sqrt_up(Jpos.right());
    } else {
      round_downward();
			l = gaol_sqrt_down(Jpos.left());
      // Computed rounding downward: kept before the direction changes (see gaol_fpu.h)
      l = gaol::rnd_keep(l);
      round_upward();
			r = gaol_sqrt_up(Jpos.right());
    }
    GAOL_RND_KEEP(r);
    GAOL_RND_LEAVE();

	interval Res(l,r);

    if (I.certainly_positive()) {
      return Res & I;
    }
    if (I.certainly_negative()) {
      return (-Res) & I;
    }
    return (I & Res) | (I & (-Res));
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
    GAOL_RND_ENTER();
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
      GAOL_RND_ENTER();
      if (even_m) { // Decreasing, as cos on [m pi, (m+1) pi]; use of cos(x)=cos(-x)
	u = sine ? sin_lo(Iright) : cos_lo(Iright);
	v = sine ? sin_hi(Ileft) : cos_hi(Ileft);
      } else { // Increasing
	u = sine ? sin_lo(Ileft) : cos_lo(Ileft);
	v = sine ? sin_hi(Iright) : cos_hi(Iright);
      }
      GAOL_RND_LEAVE();
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

      GAOL_RND_ENTER();
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
      GAOL_RND_LEAVE();
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


} // namespace gaol
