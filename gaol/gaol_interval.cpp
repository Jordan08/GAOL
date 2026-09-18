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



#if USING_SSE2_INSTRUCTIONS
#  include "gaol/gaol_interval_sse.cpp"
#else
#  include "gaol/gaol_interval_fpu.cpp"
#endif // USING_SSE2_INSTRUCTIONS



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
     when s*s is, x being a double, and s is then below sqrt(x) (fork of GAOL:
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
     negated, as the SSE2 intervals store their lower bound (fork of GAOL: GAOL
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
    They are called between GAOL_RND_NEAREST_ENTER() and
    GAOL_RND_NEAREST_LEAVE(), as the functions of gaol::nearest they call.
  */
  static inline double sin_lo(double x) { return (x == 0.0) ? 0.0 : nearest::sin_dn(x); }
  static inline double sin_hi(double x) { return (x == 0.0) ? 0.0 : nearest::sin_up(x); }
  static inline double cos_lo(double x) { return (x == 0.0) ? 1.0 : nearest::cos_dn(x); }
  static inline double cos_hi(double x) { return (x == 0.0) ? 1.0 : nearest::cos_up(x); }
  static inline double tan_lo(double x) { return (x == 0.0) ? 0.0 : nearest::tan_dn(x); }
  static inline double tan_hi(double x) { return (x == 0.0) ? 0.0 : nearest::tan_up(x); }

  /*
    The signs of sin(x) and cos(x), x being finite (fork of GAOL, issue #6).
    Neither is 0 but sin(0): no other double is a multiple of pi/2, and the
    closest one, 6381956970095103 2^797, has a cosine of 4.7e-19, so that the
    value of the mathematical library, moved one double downward, still has
    the sign of the function. Next to 0 the sign is known from x. Rounding to nearest
    (GAOL_RND_NEAREST_ENTER()).
  */
  static inline int sign_of_sin(double x)
  {
    if (std::fabs(x) < 3.0) {
      return (x > 0.0) - (x < 0.0);
    }
    return (nearest::sin_dn(x) > 0.0) ? 1 : -1;
  }

  static inline int sign_of_cos(double x)
  {
    if (std::fabs(x) < 1.5) {
      return 1;
    }
    return (nearest::cos_dn(x) > 0.0) ? 1 : -1;
  }

  static double asin_lo(double x)
  {
    return (x == 0.0) ? 0.0 : ((x == 1.0) ? half_pi_dn : ((x == -1.0) ? -half_pi_up : nearest::asin_dn(x)));
  }

  static double asin_hi(double x)
  {
    return (x == 0.0) ? 0.0 : ((x == 1.0) ? half_pi_up : ((x == -1.0) ? -half_pi_dn : nearest::asin_up(x)));
  }

  static double acos_lo(double x)
  {
    return (x == 1.0) ? 0.0 : ((x == 0.0) ? half_pi_dn : ((x == -1.0) ? pi_dn : nearest::acos_dn(x)));
  }

  static double acos_hi(double x)
  {
    return (x == 1.0) ? 0.0 : ((x == 0.0) ? half_pi_up : ((x == -1.0) ? pi_up : nearest::acos_up(x)));
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
    return nearest::atan_dn(x);
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
    return nearest::atan_up(x);
  }

  /*
    Bounds of atan2(y, x) at a point other than (0, 0) (fork of GAOL): 0, pi/2,
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
    return maximum(nearest::atan2_dn(y,x), -pi_up);
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
    return minimum(nearest::atan2_up(y,x), pi_up);
  }

  // x^y for x > 0: 1 for x = 1 or y = 0, and at least 0
  static inline double pow_lo(double x, double y)
  {
    return (x == 1.0 || y == 0.0) ? 1.0 : maximum(nearest::nthroot_dn(x,y), 0.0);
  }

  static inline double pow_hi(double x, double y)
  {
    return (x == 1.0 || y == 0.0) ? 1.0 : nearest::nthroot_up(x,y);
  }

  static inline double sinh_lo(double x)
  {
    return (x == 0.0) ? 0.0 : ((x >= 711.0) ? std::numeric_limits<double>::max() : nearest::sinh_dn(x));
  }

  static inline double sinh_hi(double x)
  {
    return (x == 0.0) ? 0.0 : ((x <= -711.0) ? -std::numeric_limits<double>::max() : nearest::sinh_up(x));
  }

  // cosh(x), cosh(-x): the lower bound for |x|, the upper bound for 0
  static inline double cosh_lo(double x)
  {
    return (x == 0.0) ? 1.0 : ((std::fabs(x) >= 711.0) ? std::numeric_limits<double>::max() : nearest::cosh_dn(x));
  }

  static inline double cosh_hi(double x)
  {
    return (x == 0.0) ? 1.0 : nearest::cosh_up(x);
  }

  // 1 - 2^-53, the double below 1, exactly
  static inline double tanh_lo(double x)
  {
    return (x == 0.0) ? 0.0 : ((x >= 20.0) ? 1.0 - 0.5*std::numeric_limits<double>::epsilon() : nearest::tanh_dn(x));
  }

  static inline double tanh_hi(double x)
  {
    return (x == 0.0) ? 0.0 : ((x <= -20.0) ? -(1.0 - 0.5*std::numeric_limits<double>::epsilon()) : nearest::tanh_up(x));
  }

  static inline double asinh_lo(double x) { return (x == 0.0) ? 0.0 : nearest::asinh_dn(x); }
  static inline double asinh_hi(double x) { return (x == 0.0) ? 0.0 : nearest::asinh_up(x); }
  static inline double acosh_lo(double x) { return (x == 1.0) ? 0.0 : nearest::acosh_dn(x); }
  static inline double acosh_hi(double x) { return (x == 1.0) ? 0.0 : nearest::acosh_up(x); }
  static inline double atanh_lo(double x) { return (x == 0.0) ? 0.0 : nearest::atanh_dn(x); }
  static inline double atanh_hi(double x) { return (x == 0.0) ? 0.0 : nearest::atanh_up(x); }

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
    case interval_format::hexa: // Display in the form [H L, H L]
      if (I.is_empty()) {
				os << "[empty]";
      } else {
				uintdouble tmp;
				tmp.d = l;
				os.setf(std::ios_base::hex,std::ios_base::basefield);
				os << '[' <<  std::setw(8) << std::setfill('0') << HI_UINTDOUBLE(tmp)
	   			<<  std::setw(8) << std::setfill('0')  << LO_UINTDOUBLE(tmp) << ", ";
				tmp.d = r;
				os << std::setw(8) << std::setfill('0') << HI_UINTDOUBLE(tmp)
	   			<< std::setw(8) << std::setfill('0') << LO_UINTDOUBLE(tmp) << ']';
				os.setf(std::ios_base::dec,std::ios_base::basefield);
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
	  			// each, without the zeros ending it (fork of GAOL: GAOL dropped
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
			  x^-m is 1/x^m, unless x^m overflows: then (1/x)^m (fork of GAOL).
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
      Hybrid semantics (fork of GAOL): a degenerate integer exponent always
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
      x^y is only real for a negative x when y is an integer (fork of GAOL,
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
      For a base above 0 and finite bounds, the pow of the mathematical library
      at the corners of the box (fork of GAOL, issue #8): x^y increases with y
      for x > 1 and decreases for x < 1, increases with x for y > 0 and
      decreases for y < 0, so that its extrema over I x J are at corners, which
      the places of the bounds about 1 and 0 give. mathlib's upow() is
      correctly rounded, and each bound is one double from the tightest one at
      most, where exp(J*log(I)) multiplied the relative width of log(I) by
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
      GAOL_RND_NEAREST_ENTER();
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
      GAOL_RND_NEAREST_LEAVE();
      return interval(l,r);
    }
    return exp(J*log(base));
  }

  /*!
    \brief I^p for a floating-point p (fork of GAOL)

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
      if (odd(n)) {
	if (J.is_empty() || I.is_empty()) {
	  return interval::emptyset();
	}
	interval inv_n = interval::one()/double(n);
	double n_lo = inv_n.left();
	double n_hi = inv_n.right();
	double l, r;
	if (J.left() >= 1.0) {
	  l = nthroot_dn(J.left(),n_lo);
	} else {
	  if (J.left() >= 0.0) {
	    l = nthroot_dn(J.left(),n_hi);
	  } else {
	    if (J.left() >= -1.0) {
	      /*
		current nthroot_up implementation does not work with non-integral
		exponent for negative first argument
	      */
	      l = -nthroot_up(-J.left(),n_lo);
	    } else {
	      l = -nthroot_up(-J.left(),n_hi);
	    }
	  }
	}
	if (J.right() >= 1.0) {
	  r = nthroot_up(J.right(),n_hi);
	} else {
	  if (J.right() >= 0.0) {
	    r = nthroot_up(J.right(),n_lo);
	  } else {
	    if (J.right() >= -1.0) {
	      r = -nthroot_dn(-J.right(),n_hi);
	    } else {
	      r = -nthroot_dn(-J.right(),n_lo);
	    }
	  }
	}
	return (interval(l,r) & I);
      }
      // n is even
      interval Jpos = interval(maximum(0.0,J.left()),J.right());
      if (Jpos.is_empty() || I.is_empty()) {
	return interval::emptyset();
      }
      interval inv_n = interval::one()/double(n);
      double n_lo = inv_n.left();
      double n_hi = inv_n.right();
      double l, r;
      if (Jpos.left() >= 1.0) {
	l = nthroot_dn(Jpos.left(),n_lo);
      } else {
	l = nthroot_dn(Jpos.left(),n_hi);
      }
      
      if (Jpos.right() >= 1.0) {
	r = nthroot_up(Jpos.right(),n_hi);
      } else {
	r = nthroot_up(Jpos.right(),n_lo);
      }
      interval tmp(l,r);
      if (I.certainly_positive()) {
	return (tmp & I);
      }
      if (I.certainly_negative()) {
	return (-tmp & I);
      }
      return ((tmp & I) | ((-tmp) & I));
    }
  }

  /*
    The n-th root of d >= 0 rounded downward and upward, n > 2, 1/n being
    enclosed by [n_lo,n_hi]: d^e grows with e for d >= 1 and decreases for
    d < 1. The roots of 0 and 1 are 0 and 1, which nthroot_dn() and
    nthroot_up() move one double away (fork of GAOL). Rounding to nearest
    (GAOL_RND_NEAREST_ENTER())
  */
  static double root_dn(double d, double n_lo, double n_hi)
  {
    return (d == 0.0 || d == 1.0) ? d : nearest::nthroot_dn(d,(d >= 1.0) ? n_lo : n_hi);
  }

  static double root_up(double d, double n_lo, double n_hi)
  {
    return (d == 0.0 || d == 1.0) ? d : nearest::nthroot_up(d,(d >= 1.0) ? n_hi : n_lo);
  }

  /*
    Code inspired from ia_math by Timothy Hickey

    rootn of IEEE 1788-2015 (Table 10.5, fork of GAOL): defined on R for an odd
    n, the root of x < 0 being -(-x)^(1/n), and on [0,+oo] for an even n. GAOL
    took the roots of the part of I in [0,+oo] for every n: nth_root([-8,27],3)
    was [0,3] rather than [-2,3]. rootn(x,0) is not defined, and gives the
    empty set.
  */
interval nth_root(const interval& I, unsigned int n)
{
	switch (n) {
	case 0:
		return interval::emptyset();
	case 1:
		return I;
	case 2:
		return sqrt(I);
	default:
		break;
	}
	const interval J = odd(n) ? I : (I & interval::positive());
	if (J.is_empty()) {
		return interval::emptyset();
	}
	const interval inv_n = interval(1.0)/double(n);
	const double n_lo = inv_n.left();
	const double n_hi = inv_n.right();
	GAOL_RND_NEAREST_ENTER();
	const double l = (J.left() >= 0.0) ? root_dn(J.left(),n_lo,n_hi) : -root_up(-J.left(),n_lo,n_hi);
	const double r = (J.right() >= 0.0) ? root_up(J.right(),n_lo,n_hi) : -root_dn(-J.right(),n_lo,n_hi);
	GAOL_RND_NEAREST_LEAVE();
	return interval(l,r);
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

  interval exp(const interval& I)
  {
	/* We intersect the result with [0, +oo] to ensure that the result is strictly positive
 		Otherwise, we might have: exp([-oo, -MAX] = [-v, +v] with v very small.
	*/
    // exp(0) = 1 exactly, where the value of the mathematical library moved
    // outward gave exp([0]) a width, and pow([1], [-oo, +oo]) = [0, +oo]
    // (fork of GAOL)
    const double l = I.left(), r = I.right();
    GAOL_RND_NEAREST_ENTER();
    const double u = (l == 0.0) ? 1.0 : nearest::exp_dn(l);
    const double v = (r == 0.0) ? 1.0 : nearest::exp_up(r);
    GAOL_RND_NEAREST_LEAVE();
    return interval(maximum(0.0,u), v);
  }

  interval log(const interval& I)
  {
    // log is defined on (0,+oo) (IEEE 1788-2015, Table 9.1, fork of GAOL): I
    // holding no positive number, as [-4,0] and [0], gives the empty set, where
    // GAOL kept its part in [0,+oo] and gave [-oo,-MAX]
    if (I.is_empty() || !(I.right() > 0.0)) {
      return interval::emptyset();
    }

    // log(1) = 0 exactly: log([1]) was [-2^-1074, 2^-1074] (fork of GAOL)
    const double l = maximum(0.0,I.left()), r = I.right();
    GAOL_RND_NEAREST_ENTER();
    const double u = (l == 1.0) ? 0.0 : nearest::log_dn(l);
    const double v = (r == 1.0) ? 0.0 : nearest::log_up(r);
    GAOL_RND_NEAREST_LEAVE();
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
      2^52, the signs of the cosine at the bounds do (fork of GAOL, issue #6,
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
    GAOL_RND_NEAREST_ENTER();
    if (!told && narrower_than_pi) {
      no_pole = (sign_of_cos(l) == sign_of_cos(r));
    }
    double u = -GAOL_INFINITY, v = GAOL_INFINITY;
    if (no_pole) {
      u = tan_lo(l);
      v = tan_hi(r);
    }
    GAOL_RND_NEAREST_LEAVE();
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

    GAOL_RND_NEAREST_ENTER();
    const double l = acos_lo(J.right()), r = acos_hi(J.left());
    GAOL_RND_NEAREST_LEAVE();
    return interval(l,r);
  }

  interval asin(const interval& I)
  {
    interval J= I & interval::minus_one_plus_one();
    // J <- I \cap [-1,1]

    if (J.is_empty()) {
      return interval::emptyset();
    }
    GAOL_RND_NEAREST_ENTER();
    const double l = asin_lo(J.left()), r = asin_hi(J.right());
    GAOL_RND_NEAREST_LEAVE();
    return interval(l,r);
  }

  interval atan(const interval& I)
  {
	if (I.is_empty()) {
	  return I;
	}
    GAOL_RND_NEAREST_ENTER();
    const double l = atan_lo(I.left()), r = atan_hi(I.right());
    GAOL_RND_NEAREST_LEAVE();
    return interval(l,r);
  }

  /*
    atan2 of IEEE 1788-2015 (Table 9.1), defined on the plane but (0, 0), with
    values in (-pi, pi] (fork of GAOL; GAOL raised unavailable_feature_error).
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
    GAOL_RND_NEAREST_ENTER();
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
    GAOL_RND_NEAREST_LEAVE();
    return interval(l,r);
  }

  interval cosh(const interval& I)
  {
	if (I.is_empty()) {
		return I;
	}
    double l, r;
    GAOL_RND_NEAREST_ENTER();
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
    GAOL_RND_NEAREST_LEAVE();
    return interval(l,r);
  }

  interval sinh(const interval& I)
  {
	if (I.is_empty()) {
		return I;
	}
	GAOL_RND_NEAREST_ENTER();
	const double l = sinh_lo(I.left()), r = sinh_hi(I.right());
	GAOL_RND_NEAREST_LEAVE();
	return interval(l,r);
  }

  interval tanh(const interval& I)
  {
	if (I.is_empty()) {
		return I;
	}
	GAOL_RND_NEAREST_ENTER();
	const double l = tanh_lo(I.left()), r = tanh_hi(I.right());
	GAOL_RND_NEAREST_LEAVE();
	return interval(l,r) & interval::minus_one_plus_one();
  }


  interval acosh(const interval& I)
  {
    interval J = I &  interval::one_plus_infinity();

    if (J.is_empty()) {
      return J;
    }

  	GAOL_RND_NEAREST_ENTER();
  	const double l = acosh_lo(J.left()), r = acosh_hi(J.right());
  	GAOL_RND_NEAREST_LEAVE();
  	return interval(l,r);
  }



  interval asinh(const interval& I)
  {
    if (I.is_empty()) {
      return I;
    }

    GAOL_RND_NEAREST_ENTER();
    const double l = asinh_lo(I.left()), r = asinh_hi(I.right());
    GAOL_RND_NEAREST_LEAVE();
    return interval(l,r);

  }

  interval atanh(const interval& I)
  {
	  interval J = I & interval::minus_one_plus_one();
    if (J.is_empty()) {
      return interval::emptyset();
    }
    GAOL_RND_NEAREST_ENTER();
    const double l = atanh_lo(J.left()), r = atanh_hi(J.right());
    GAOL_RND_NEAREST_LEAVE();
    return interval(l,r);
  }

  interval acos_k(double i, const interval &Jacos)
  {

    if (!feven(i)) {
      return ((i+1)*interval::pi() - Jacos);
    } else { // even
      return (i*interval::pi() + Jacos);
    }
  }


  interval acos_i(double i, const interval &J, const interval &I)
  {
    double l, r;

    if (J.right() > 1.0) {
      l = 0.0;
    } else {
      l = acos_dn(J.right());
    }

    if (J.left() < -1.0) {
      r = pi_up;
    } else {
      r = acos_up(J.left());
    }

    if (!feven(i)) {
      return ((i+1)*interval::pi()-interval(l,r)) & I;
    } else { // even
      return (i*interval::pi()+interval(l,r)) & I;
    }
  }

  interval acos_rel(const interval& J, const interval &I)
  {
    if (J.is_empty() || I.is_empty() || J.left() > 1.0 || J.right() < -1.0) {
      return interval::emptyset();
    }
    if (J.set_contains(interval::minus_one_plus_one())) {
      return I;
    }
    GAOL_RND_ENTER();

    double kl = 0.0, kr = 0.0; // Meaningless definitions to keep the compiler happy
    interval Jacos = acos(J);

    interval Ileft;
    // Checking whether the left bound is too large to perform
    // a reliable range reduction (i.e., kl would be off by more than 1)
    if (I.left() < -two_power_52 || I.left() > two_power_52) {
      Ileft = I;
    } else {
      round_downward();
      if (I.left() < 0) {
	kl = std::floor(I.left()/interval::pi().left());
      } else {
	if (I.left() > 0) {
	  kl = std::floor(I.left()/interval::pi().right());
	} else {
	  kl = 0;
	}
      }
      // Computed rounding downward: kept before the direction changes (see gaol_fpu.h)
      kl = gaol::rnd_keep(kl);
      round_upward();
      // From here, kl is at most off by 1 less than the true value
      Ileft = acos_k(kl,Jacos) & I;
      if (Ileft.is_empty()) {
	Ileft = acos_k(kl+1,Jacos) & I;
      }
    }
    interval Iright;
    // Checking whether the right bound is too large to perform
    // a reliable range reduction (i.e. kr would be off by more than 1)
    if (I.right() < -two_power_52 || I.right() > two_power_52) {
      Iright = I;
    } else {
      if (I.right() < 0) {
	kr = std::floor(I.right()/interval::pi().right());
      } else {
	if (I.right() > 0) {
	  kr = std::floor(I.right()/interval::pi().left());
	} else {
	  kr = 0;
	}
      }
      // From here, kr is at most off by 1 more than the true value
      if (kr == kl) {
	Iright = Ileft;
      } else {
	Iright = acos_k(kr,Jacos) & I;
	if (Iright.is_empty()) {
	  Iright = acos_k(kr-1,Jacos) & I;
	}
      }
    }
    GAOL_RND_KEEP(Ileft); GAOL_RND_KEEP(Iright);
    GAOL_RND_LEAVE();
    return interval(Ileft.left(),Iright.right());
  }




  interval asin_rel(const interval& J, const interval &I)
  {
    return I & (interval::half_pi()+acos_rel(J,I-interval::half_pi()));
  }

  /*
    Returns the multiple of $\pi$ on which to project tan(x)
  */
  INLINE double tan_period(double x)
  {
    return std::floor((std::floor(x)+1.0)/2.0);
  }

  interval atan_rel(const interval& J, const interval& I)
  {
    if (I.is_empty() || J.is_empty()) {
      return interval::emptyset();
    }
    GAOL_RND_ENTER();
    // kl is not computed when I.left() is too large for a reliable range
    // reduction, but compared with kr below: Iright is then Ileft, which is I
    double kl = 0.0, kr = 0.0;
    interval atanJ = atan(J);

    interval Ileft;
    // Checking whether the left bound is too large to perform
    // a reliable range reduction (i.e., kl would be off by more than 1)
    if (I.left() < - two_power_51 || I.left() > two_power_51) {
      Ileft = I;
    } else {
      round_downward();
      if (I.left() < 0) {
	kl = tan_period(I.left()/interval::half_pi().left());
      } else {
	if (I.left() > 0) {
	  kl = tan_period(I.left()/interval::half_pi().right());
	} else {
	  kl = 0;
	}
      }
      // Computed rounding downward: kept before the direction changes (see gaol_fpu.h)
      kl = gaol::rnd_keep(kl);
      round_upward();
      // From here, kl is at most off by 1 less than the true value
      interval tmp = atanJ + kl*interval::pi();
      Ileft = tmp & I;
      if (Ileft.is_empty()) {
	Ileft = (tmp + interval::pi()) & I;
      }
    }
    interval Iright;
    // Checking whether the right bound is too large to perform
    // a reliable range reduction (i.e. kr would be off by more than 1)
    if (I.right() < -two_power_51 || I.right() > two_power_51) {
      Iright = I;
    } else {
      if (I.right() < 0) {
	kr = tan_period(I.right()/interval::half_pi().right());
      } else {
	if (I.right() > 0) {
	  kr = tan_period(I.right()/interval::half_pi().left());
	} else {
	  kr = 0;
	}
      }
      // From here, kr is at most off by 1 more than the true value
      if (kr == kl) {
	Iright = Ileft;
      } else {
	interval tmp = atanJ + kr*interval::pi();
	Iright = tmp & I;
	if (Iright.is_empty()) {
	  Iright = (tmp - interval::pi()) & I;
	}
      }
    }
    GAOL_RND_KEEP(Ileft); GAOL_RND_KEEP(Iright);
    GAOL_RND_LEAVE();
    return interval(Ileft.left(),Iright.right());
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
   * invabs_rel
   */
  interval invabs_rel(const interval &J, const interval &I)
  {
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
    sine of the mathematical library (fork of GAOL): GAOL computed
    cos(I - [pi/2]), whose subtraction widened I by about 2^-52 max(2, |x|), and
    sin([1e-10]) was 4.4e-16 wide.

    Otherwise (fork of GAOL, issue #6), I is on several pieces for sure when
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
      GAOL_RND_NEAREST_ENTER();
      if (even_m) { // Decreasing, as cos on [m pi, (m+1) pi]; use of cos(x)=cos(-x)
	u = sine ? sin_lo(Iright) : cos_lo(Iright);
	v = sine ? sin_hi(Ileft) : cos_hi(Ileft);
      } else { // Increasing
	u = sine ? sin_lo(Ileft) : cos_lo(Ileft);
	v = sine ? sin_hi(Iright) : cos_hi(Iright);
      }
      GAOL_RND_NEAREST_LEAVE();
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

      GAOL_RND_NEAREST_ENTER();
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
      GAOL_RND_NEAREST_LEAVE();
    }
    // The values of the mathematical library moved outward may leave [-1,1]
    // (fork of GAOL)
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
