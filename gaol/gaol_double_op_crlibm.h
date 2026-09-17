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
 * CVS: $Id: gaol_double_op_crlibm.h 191 2012-03-06 17:08:58Z goualard $
 * Last modified:
 * By:
 *--------------------------------------------------------------------------*/

/*!
  \file   gaol_double_op_crlibm.h
  \brief  Operations correctly rounded down and up.

  Fast version using standard arithmetic operations and the Correctly
	Rounded Mathematical Library (crlibm)

  \author Frederic Goualard
  \date   2009-02-25
*/


#ifndef __gaol_double_op_crlibm_h__
#define __gaol_double_op_crlibm_h__

#include <cmath>
#include <limits>
#include "gaol/gaol_config.h"
#include "gaol/gaol_port.h"
#include "gaol/gaol_fpu.h"
#include "gaol/gaol_assert.h"
#include "gaol/gaol_common.h"

#include <crlibm.h>

namespace gaol {

  /*
    Computes d^n rounded upward with a binary exponentiation algorithm
    \warning "d" and "n" should be positive
   */

  INLINE double ipow_up(double d, unsigned int n)
  {
    GAOL_RND_ENTER();
    GAOL_ASSERT(d >= 0.0);

    double y = 1;
    double z = d;

    for (;;) {
      if (odd(n)) {
	n >>= 1;
	y *= z;
	if (n == 0) {
	  GAOL_RND_LEAVE();
	  return double(y);
	}
      } else {
	n>>=1;
      }
      z *= z;
    }
  }

  /*
    Computes d^n rounded downward with a binary exponentiation algorithm
    \warning "d" and "n" should be positive
   */
	INLINE double ipow_dn(double d, unsigned int n)
  	{
        GAOL_RND_ENTER();
        GAOL_ASSERT(d >= 0.0);

        double y = 1;
        double z = d;

    	for (;;) {
      	if (odd(n)) {
			n >>= 1;
			y = gaol_opposite(gaol_opposite(y)*z);
			if (n == 0) {
	  			GAOL_RND_LEAVE();
	  			return y;
			}
      	} else {
			n>>=1;
      	}
      	z = gaol_opposite(gaol_opposite(z)*z);
    	}
	}

  /*!
    \brief pow() correctly rounded down

  */
  INLINE double pow_dn(double d, unsigned int e)
    {
      if (d >= 0) {
	return ipow_dn(d,e);
      } else { // d < 0
	if (even(e)) {
	  return ipow_dn(gaol_opposite(d),e);
	} else { // odd(e)
	  return gaol_opposite(ipow_up(gaol_opposite(d),e));
	}
      }
    }

  /*!
    \brief pow() correctly rounded up

  */
   INLINE double pow_up(double d, unsigned int e)
    {
      if (d >= 0) {
	return ipow_up(d,e);
      } else { // d < 0
	if (even(e)) {
	  return ipow_up(gaol_opposite(d),e);
	} else { // odd(e)
	  return gaol_opposite(ipow_dn(gaol_opposite(d),e));
	}
      }
    }

  /*
    tanh, acosh, asinh and atanh come from the libm of the system, rounded to
    nearest, crlibm having none of tanh, acosh, asinh and atanh. GAOL moved their values one float outward,
    which only encloses the exact values when the libm is within one float of
    them, and it is not always (fork of GAOL): on 2000 random arguments of each
    function, the libms of glibc 2.31, musl and MinGW-w64 returned doubles
    beyond the two around the exact value for sinh, cosh, tanh, acosh and atanh
    (86 times for tanh), and GAOL's asinh() did not enclose
    asinh(-0x1.ee84df02a8766p-4), nor its acosh() acosh(0x1.01fd62fff333fp+0).
    The values are moved three floats outward instead, which encloses the exact
    values as long as the libm is within two floats of them: none of the libms
    tested was further than one float beyond the two around the exact value,
    and tests/elementary.cpp checks them on each platform of the continuous
    integration.
  */
  INLINE double gaol_libm_dn(double f)
  {
    return previous_float(previous_float(previous_float(f)));
  }

  INLINE double gaol_libm_up(double f)
  {
    return next_float(next_float(next_float(f)));
  }

  /*
    The bounds of the functions below, computed in the rounding direction to
    nearest, which the mathematical library needs, set beforehand
    (GAOL_RND_NEAREST_ENTER()): an operation of GAOL sets the direction once
    for both of its bounds, rather than twice for each (fork of GAOL). The
    functions of the same names in gaol:: set it themselves.
  */
  namespace nearest {
    INLINE double nthroot_dn(double d, double e) { return previous_float(pow_rn(d,e)); }
    INLINE double nthroot_up(double d, double e) { return next_float(pow_rn(d,e)); }
    INLINE double exp_dn(double d) { return exp_rd(d); }
    INLINE double exp_up(double d) { return exp_ru(d); }
    INLINE double cos_dn(double d) { return cos_rd(d); }
    INLINE double cos_up(double d) { return cos_ru(d); }
    INLINE double sin_dn(double d) { return sin_rd(d); }
    INLINE double sin_up(double d) { return sin_ru(d); }
    INLINE double tan_dn(double d) { return tan_rd(d); }
    INLINE double tan_up(double d) { return tan_ru(d); }
    INLINE double log_dn(double d) { return log_rd(d); }
    // Checks 0 to avoid log([0,0]) == <-inf, -inf>
    INLINE double log_up(double d) { return (d == 0.0) ? -std::numeric_limits<double>::max() : log_ru(d); }
    INLINE double acos_dn(double d) { return acos_rd(d); }
    INLINE double acos_up(double d) { return acos_ru(d); }
    INLINE double asin_dn(double d) { return asin_rd(d); }
    INLINE double asin_up(double d) { return asin_ru(d); }
    INLINE double atan_dn(double d) { return atan_rd(d); }
    INLINE double atan_up(double d) { return atan_ru(d); }
    INLINE double atan2_dn(double y, double x) { return gaol_libm_dn(atan2(y,x)); } // From libm, not crlibm
    INLINE double atan2_up(double y, double x) { return gaol_libm_up(atan2(y,x)); } // From libm, not crlibm
    INLINE double cosh_dn(double x) { return cosh_rd(x); }
    INLINE double cosh_up(double x) { return cosh_ru(x); }
    INLINE double sinh_dn(double x) { return sinh_rd(x); }
    INLINE double sinh_up(double x) { return sinh_ru(x); }
    INLINE double tanh_dn(double x) { return gaol_libm_dn(tanh(x)); } // From libm, not crlibm
    INLINE double tanh_up(double x) { return gaol_libm_up(tanh(x)); } // From libm, not crlibm
    INLINE double acosh_dn(double x) { return gaol_libm_dn(acosh(x)); } // From libm, not crlibm
    INLINE double acosh_up(double x) { return gaol_libm_up(acosh(x)); } // From libm, not crlibm
    INLINE double asinh_dn(double x) { return gaol_libm_dn(asinh(x)); } // From libm, not crlibm
    INLINE double asinh_up(double x) { return gaol_libm_up(asinh(x)); } // From libm, not crlibm
    INLINE double atanh_dn(double x) { return gaol_libm_dn(atanh(x)); } // From libm, not crlibm
    INLINE double atanh_up(double x) { return gaol_libm_up(atanh(x)); } // From libm, not crlibm
  } // namespace nearest

  /*!
    \brief nthroot() correctly rounded down

    Assumes a correctly rounded pow(double) when rounding direction
    is to nearest (such as ensured by the IBM math library).
    \caution The current rounding direction must be to nearest.
  */
  INLINE double nthroot_dn(double d, double e)
    {
      GAOL_RND_PRESERVE();
      round_nearest();
      double f=nearest::nthroot_dn(d,e);
      GAOL_RND_RESTORE();
      return f;
    }

  /*!
    \brief nthroot() correctly rounded up

    Assumes a correctly rounded pow(double) when rounding direction
    is to nearest (such as ensured by the IBM math library).
    \caution The current rounding direction must be to nearest.
  */
   INLINE double nthroot_up(double d, double e)
    {
      GAOL_RND_PRESERVE();
      round_nearest();
      double f=nearest::nthroot_up(d,e);
      GAOL_RND_RESTORE();
      return f;
    }

  /*!
    \brief Exponential correctly rounded downward

    Assumes a correctly rounded function exp(double) when rounding
    direction is to nearest (such as ensured by the IBM math library).
    \caution The current rounding direction must be to nearest.
  */
  INLINE double exp_dn(double d)
  {
    GAOL_RND_PRESERVE();
    round_nearest();
    double f=nearest::exp_dn(d);
    GAOL_RND_RESTORE();
    return f;
  }

  /*!
    \brief Exponential correctly rounded upward

    Assumes a correctly rounded function exp(double) when rounding
    direction is to nearest (such as ensured by the IBM math library).
    \caution The current rounding direction must be to nearest.
  */
  INLINE double exp_up(double d)
  {
    GAOL_RND_PRESERVE();
    round_nearest();
    double f=nearest::exp_up(d);
    GAOL_RND_RESTORE();
    return f;
  }

  INLINE double cos_dn(double d)
  {
    GAOL_RND_PRESERVE();
    round_nearest();
    double f=nearest::cos_dn(d);
    GAOL_RND_RESTORE();
    return f;
  }

  INLINE double cos_up(double d)
  {
    GAOL_RND_PRESERVE();
    round_nearest();
    double f=nearest::cos_up(d);
    GAOL_RND_RESTORE();
    return f;
  }

  // sin, as cos (fork of GAOL: GAOL computed sin(x) as cos(x - pi/2))
  INLINE double sin_dn(double d)
  {
    GAOL_RND_PRESERVE();
    round_nearest();
    double f=nearest::sin_dn(d);
    GAOL_RND_RESTORE();
    return f;
  }

  INLINE double sin_up(double d)
  {
    GAOL_RND_PRESERVE();
    round_nearest();
    double f=nearest::sin_up(d);
    GAOL_RND_RESTORE();
    return f;
  }

  INLINE double tan_dn(double d)
  {
    GAOL_RND_PRESERVE();
    round_nearest();
    double f=nearest::tan_dn(d);
    GAOL_RND_RESTORE();
    return f;
  }

  INLINE double tan_up(double d)
  {
    GAOL_RND_PRESERVE();
    round_nearest();
    double f=nearest::tan_up(d);
    GAOL_RND_RESTORE();
    return f;
  }

   INLINE double log_dn(double d)
  {
    GAOL_RND_PRESERVE();
    round_nearest();
    double f=nearest::log_dn(d);
    GAOL_RND_RESTORE();
    return f;
  }

  INLINE double log_up(double d)
  {
    GAOL_RND_PRESERVE();
    round_nearest();
    double f=nearest::log_up(d);
    GAOL_RND_RESTORE();
    return f;
  }


  INLINE double acos_dn(double d)
  {
    GAOL_RND_PRESERVE();
    round_nearest();
    double f=nearest::acos_dn(d);
    GAOL_RND_RESTORE();
    return f;
  }

  INLINE double acos_up(double d)
  {
    GAOL_RND_PRESERVE();
    round_nearest();
    double f=nearest::acos_up(d);
    GAOL_RND_RESTORE();
    return f;
  }

  INLINE double asin_dn(double d)
  {
    GAOL_RND_PRESERVE();
    round_nearest();
    double f=nearest::asin_dn(d);
    GAOL_RND_RESTORE();
    return f;
  }

  INLINE double asin_up(double d)
  {
    GAOL_RND_PRESERVE();
    round_nearest();
    double f=nearest::asin_up(d);
    GAOL_RND_RESTORE();
    return f;
  }

  INLINE double atan_dn(double d)
  {
    GAOL_RND_PRESERVE();
    round_nearest();
    double f=nearest::atan_dn(d);
    GAOL_RND_RESTORE();
    return f;
  }

  INLINE double atan_up(double d)
  {
    GAOL_RND_PRESERVE();
    round_nearest();
    double f=nearest::atan_up(d);
    GAOL_RND_RESTORE();
    return f;
  }

  INLINE double cosh_dn(double x)
  {
	GAOL_RND_PRESERVE();
	round_nearest();
	double f=nearest::cosh_dn(x);
	GAOL_RND_RESTORE();
	return f;
  }

  INLINE double cosh_up(double x)
  {
	GAOL_RND_PRESERVE();
	round_nearest();
	double f=nearest::cosh_up(x);
	GAOL_RND_RESTORE();
	return f;
  }

  INLINE double sinh_dn(double x)
  {
	GAOL_RND_PRESERVE();
	round_nearest();
	double f=nearest::sinh_dn(x);
	GAOL_RND_RESTORE();
	return f;
  }

  INLINE double sinh_up(double x)
  {
	GAOL_RND_PRESERVE();
	round_nearest();
	double f=nearest::sinh_up(x);
	GAOL_RND_RESTORE();
	return f;
  }

  INLINE double tanh_dn(double x)
  {
	  GAOL_RND_PRESERVE();
	  round_nearest();
	  double f=nearest::tanh_dn(x);
	  GAOL_RND_RESTORE();
	  return f;
  }

  INLINE double tanh_up(double x)
  {
	  GAOL_RND_PRESERVE();
	  round_nearest();
	  double f=nearest::tanh_up(x);
	  GAOL_RND_RESTORE();
	  return f;
  }


  INLINE double acosh_dn(double x)
  {
	  GAOL_RND_PRESERVE();
	  round_nearest();
	  double f=nearest::acosh_dn(x);
	  GAOL_RND_RESTORE();
	  return f;
  }

  INLINE double acosh_up(double x)
  {
	  GAOL_RND_PRESERVE();
	  round_nearest();
	  double f=nearest::acosh_up(x);
	  GAOL_RND_RESTORE();
	  return f;
  }

  INLINE double asinh_dn(double x)
  {
	  GAOL_RND_PRESERVE();
	  round_nearest();
	  double f=nearest::asinh_dn(x);
	  GAOL_RND_RESTORE();
	  return f;
  }

  INLINE double asinh_up(double x)
  {
	  GAOL_RND_PRESERVE();
	  round_nearest();
	  double f=nearest::asinh_up(x);
	  GAOL_RND_RESTORE();
	  return f;
  }

  INLINE double atanh_dn(double x)
  {
	  GAOL_RND_PRESERVE();
	  round_nearest();
	  double f=nearest::atanh_dn(x);
	  GAOL_RND_RESTORE();
	  return f;
  }

  INLINE double atanh_up(double x)
  {
	  GAOL_RND_PRESERVE();
	  round_nearest();
	  double f=nearest::atanh_up(x);
	  GAOL_RND_RESTORE();
	  return f;
  }



} // namespace gaol

#endif /* __gaol_double_op_crlibm_h__ */
