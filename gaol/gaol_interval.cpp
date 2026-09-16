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
  */
  static inline double sin_lo(double x) { return (x == 0.0) ? 0.0 : sin_dn(x); }
  static inline double sin_hi(double x) { return (x == 0.0) ? 0.0 : sin_up(x); }
  static inline double cos_lo(double x) { return (x == 0.0) ? 1.0 : cos_dn(x); }
  static inline double cos_hi(double x) { return (x == 0.0) ? 1.0 : cos_up(x); }
  static inline double tan_lo(double x) { return (x == 0.0) ? 0.0 : tan_dn(x); }
  static inline double tan_hi(double x) { return (x == 0.0) ? 0.0 : tan_up(x); }

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

  void display_bounds(double l, double r, ostream& os)
  {
    if (!(l <= r)) {
      os << "[empty]";
    } else {
      if (l == r) {
				round_downward();
				os << '<';
				os << l; //dtoa_downward(l,os);
				os << ", ";
				round_upward();
				os << r; //dtoa_upward(l,os);
				os << '>';
      } else {
				os << '[';
				round_downward();
				os << l; //dtoa_downward(l,os);
				os << ", ";
				round_upward();
				os << r; //dtoa_upward(r,os);
				os << ']';
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

				  round_downward();
	  			lbound << std::showpoint << I.left();

				  round_upward();
	  			rbound << std::showpoint << I.right();

				  unsigned int i = 0;
	  			while (lbound.str()[i] == rbound.str()[i]) {
	    			itv += lbound.str()[i];
	    			++i;
	  			}
	  			size_t lblen = lbound.str().length();
	  			size_t rblen = rbound.str().length();

	  			// Some digits not in common?
	  			if (i < lblen || i < rblen) {
	    			itv += "~[";
	    			if (i >= lbound.str().length()) {
	      			itv += "0";
	    			} else {
	      			// Removing trailing zeroes
	      			size_t last_not_zero = lbound.str().find_last_not_of("0");
	      			itv += lbound.str().substr(i,last_not_zero-i+1);
	    			}
	    			itv += ", ";
	    			if (i >= rbound.str().length()) {
	      			itv += "0";
	    			} else {
	      			size_t last_not_zero = lbound.str().find_last_not_of("0");
	      			itv += rbound.str().substr(i,last_not_zero-i+1);
	    			}
	    			itv += "]";
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
			return inverse(uipow(I,0u - static_cast<unsigned int>(n))); // 1/uipow(I,0u - static_cast<unsigned int>(n))
		} else {
			if (n > 0) {
				return uipow(I,static_cast<unsigned int>(n));
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
    nthroot_up() move one double away (fork of GAOL)
  */
  static double root_dn(double d, double n_lo, double n_hi)
  {
    return (d == 0.0 || d == 1.0) ? d : nthroot_dn(d,(d >= 1.0) ? n_lo : n_hi);
  }

  static double root_up(double d, double n_lo, double n_hi)
  {
    return (d == 0.0 || d == 1.0) ? d : nthroot_up(d,(d >= 1.0) ? n_hi : n_lo);
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
	const double l = (J.left() >= 0.0) ? root_dn(J.left(),n_lo,n_hi) : -root_up(-J.left(),n_lo,n_hi);
	const double r = (J.right() >= 0.0) ? root_up(J.right(),n_lo,n_hi) : -root_dn(-J.right(),n_lo,n_hi);
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
    return interval((l == 0.0) ? 1.0 : maximum(0.0,exp_dn(l)), (r == 0.0) ? 1.0 : exp_up(r));
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
    return interval((l == 1.0) ? 0.0 : log_dn(l), (r == 1.0) ? 0.0 : log_up(r));
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
      If I is outside [-2^52,2^52], it is useless to compute
      the tangent because the rounding error in modulo_k_pi
      would forbid computing k_left or k_right accurately
    */
    if (I.right() < -two_power_52 || I.left() > two_power_52) {
      GAOL_RND_LEAVE();
      return interval::universe();
    }

    double kl, kr;
    if (!fast_modulo_k_pi(I+interval::half_pi(),kl,kr) || (kl != kr)) {
      GAOL_RND_LEAVE();
      return interval::universe();
    }
    GAOL_RND_LEAVE();
    return interval(tan_lo(I.left()),tan_hi(I.right()));
  }


  interval acos(const interval& I)
  {
    interval J = I & interval::minus_one_plus_one();
    // J <- I \cap [-1,1]

    if (J.is_empty()) {
      return interval::emptyset();
    }

    return interval(acos_lo(J.right()),acos_hi(J.left()));
  }

  interval asin(const interval& I)
  {
    interval J= I & interval::minus_one_plus_one();
    // J <- I \cap [-1,1]

    if (J.is_empty()) {
      return interval::emptyset();
    }
    return interval(asin_lo(J.left()),asin_hi(J.right()));
  }

  interval atan(const interval& I)
  {
	if (I.is_empty()) {
	  return I;
	}
    return interval(atan_lo(I.left()),atan_hi(I.right()));
  }

  interval atan2(const interval& Y, const interval& X)
  {
    gaol_ERROR(unavailable_feature_error,"atan2 not yet implemented");
    return interval::emptyset();
  }

  interval cosh(const interval& I)
  {
	if (I.is_empty()) {
		return I;
	}
    if (I.right() < 0) {
      return interval(cosh_lo(I.right()),cosh_hi(I.left()));
    }
    if (I.left() > 0) {
      return interval(cosh_lo(I.left()),cosh_hi(I.right()));
    }
    // 0 \in I
    double abs_l = -I.left();
    if (abs_l >= I.right()) {
      return interval(1.0,cosh_hi(abs_l));
    } else {
      return interval(1.0,cosh_hi(I.right()));
    }
  }

  interval sinh(const interval& I)
  {
	if (I.is_empty()) {
		return I;
	}
	return interval(sinh_lo(I.left()),sinh_hi(I.right()));
  }

  interval tanh(const interval& I)
  {
	if (I.is_empty()) {
		return I;
	}
  	return interval(tanh_lo(I.left()),tanh_hi(I.right())) & interval::minus_one_plus_one();
  }


  interval acosh(const interval& I)
  {
    interval J = I &  interval::one_plus_infinity();

    if (J.is_empty()) {
      return J;
    }

  	return interval(acosh_lo(J.left()),acosh_hi(J.right()));
  }



  interval asinh(const interval& I)
  {
    if (I.is_empty()) {
      return I;
    }

    return interval(asinh_lo(I.left()),asinh_hi(I.right()));

  }

  interval atanh(const interval& I)
  {
	  interval J = I & interval::minus_one_plus_one();
    if (J.is_empty()) {
      return interval::emptyset();
    }
    return interval(atanh_lo(J.left()),atanh_hi(J.right()));
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


  interval cos(const interval& I)
  {
    if (I.is_empty()) {
      return interval::emptyset();
    }

    GAOL_RND_ENTER();
    double a,b, Il, Ir;
    Il = I.left_internal();
    Ir = I.right_internal();
    double Ileft = -Il;
    double Iright = Ir;
    
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
    
    double m=std::floor(a),
      n=std::ceil(b),
      u, v;

    double nm = (n-m); // Must be rounded towards +oo
    
    if (nm < 2.0) {
      if (feven(m)) { // even(m)? No conversion to int in order
	// to avoid overflow
	u=cos_lo(Iright);// use of cos(x)=cos(-x)
	v=cos_hi(Ileft);
      } else { // odd(m)?
	u=cos_lo(Ileft);
	v=cos_hi(Iright);
      }
    } else {
      if (nm == 2.0) {
	if (feven(m)) {
	  double
	    u1=cos_hi(Ileft),
	    u2=cos_hi(Iright);
	  u= -1.0;
	  v= ((u1 > u2) ? u1 : u2);
	} else {
	  double
	    u1=cos_lo(Ileft),
	    u2=cos_lo(Iright);
	  
	  u= ((u1 < u2) ? u1 : u2);
	  v= 1.0;
	}
      } else {
	GAOL_RND_LEAVE();
	return interval::minus_one_plus_one();
      }
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

  /*
    sin(I), as cos(I): the bounds of I divided by an enclosure of pi, minus 1/2,
    rounded outward, give the pieces of I on which sin is monotonic, sin(x)
    being cos(x - pi/2); sin is computed by the mathematical library at the
    bounds of I (fork of GAOL). GAOL computed cos(I - [pi/2]), whose
    subtraction widened I by about 2^-52 max(2, |x|): sin([1e-10]) was
    4.4e-16 wide.
  */
  interval sin(const interval& I)
  {
    if (I.is_empty()) {
      return interval::emptyset();
    }

    GAOL_RND_ENTER();
    double a,b, Il, Ir;
    Il = I.left_internal();
    Ir = I.right_internal();
    double Ileft = -Il;
    double Iright = Ir;

    // a <= Ileft/pi, b >= Iright/pi, as in cos()
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
    // a <= (Ileft - pi/2)/pi, b >= (Iright - pi/2)/pi, rounded upward
    a = -((-a) + 0.5);
    b = b - 0.5;

    double m=std::floor(a),
      n=std::ceil(b),
      u, v;

    double nm = (n-m); // Rounded upward

    if (nm < 2.0) {
      if (feven(m)) { // sin decreasing, as cos on [m pi, (m+1) pi]
	u=sin_lo(Iright);
	v=sin_hi(Ileft);
      } else { // sin increasing
	u=sin_lo(Ileft);
	v=sin_hi(Iright);
      }
    } else {
      if (nm == 2.0) {
	if (feven(m)) { // A minimum, -1, within I
	  double
	    u1=sin_hi(Ileft),
	    u2=sin_hi(Iright);
	  u= -1.0;
	  v= ((u1 > u2) ? u1 : u2);
	} else { // A maximum, 1, within I
	  double
	    u1=sin_lo(Ileft),
	    u2=sin_lo(Iright);
	  u= ((u1 < u2) ? u1 : u2);
	  v= 1.0;
	}
      } else {
	GAOL_RND_LEAVE();
	return interval::minus_one_plus_one();
      }
    }
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


} // namespace gaol
