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
 *
 * Copyright (c) 2026 ENSTA, France
 *
 * Created 2026-09-20 by Jordan NININ
 *--------------------------------------------------------------------------
 * gaol is a software distributed WITHOUT ANY WARRANTY. Read the associated
 * COPYING file for information.
 *--------------------------------------------------------------------------*/

/*!
  \file   gaol_double_op.h
  \brief  The elementary functions rounded downward and upward, with CORE-MATH.

  Every elementary function of GAOL is bounded with CORE-MATH
  (3rd/math-core, see gaol/gaol_core_math.h), on every architecture and with
  every compiler (GAOL v5). CORE-MATH's functions are correctly rounded
  in the rounding direction in effect, so in the upward rounding GAOL computes
  in:

  - the value at a bound IS the upper bound, with nothing to add;
  - the lower bound is the double below it, which is the tightest one unless
    the exact value is a double, and the operations of gaol/gaol_interval.cpp
    give those exactly (log(1) = 0, sin(0) = 0, asin(1) = the bounds of pi/2...).

  \author Frederic Goualard, then GAOL v5
*/

#ifndef __gaol_double_op_h__
#define __gaol_double_op_h__

#include <cmath>
#include "gaol/gaol_config.h"
#include "gaol/gaol_port.h"
#include "gaol/gaol_fpu.h"
#include "gaol/gaol_assert.h"
#include "gaol/gaol_common.h"
#include "gaol/gaol_core_math.h"

namespace gaol_core {



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
    The hyperbolic functions below are those of CORE-MATH (gaol_core_math.h),
    correctly rounded, as the other elementary functions (GAOL v5, issue #1).
  */

  /*
    The bounds of the functions below, computed in the rounding direction in
    effect. CORE-MATH is correctly rounded in that direction, so the value at
    a bound rounded upward is the upper bound itself, and the double below it
    the lower bound: nothing is added, and the direction is never switched
    (GAOL v5).

    The functions of namespace nearest are those the code setting the
    direction to nearest itself calls: there the value is rounded to nearest,
    and moved one double outward to enclose the exact one.
  */
  namespace nearest {
    INLINE double nthroot_dn(double d, double e) { return previous_float(gaol_cr_pow(d, e)); }
    INLINE double nthroot_up(double d, double e) { return next_float(gaol_cr_pow(d, e)); }
    INLINE double atan2_dn(double y, double x) { return previous_float(gaol_cr_atan2(y, x)); }
    INLINE double atan2_up(double y, double x) { return next_float(gaol_cr_atan2(y, x)); }
    INLINE double exp_dn(double d) { return previous_float(gaol_cr_exp(d)); }
    INLINE double exp_up(double d) { return next_float(gaol_cr_exp(d)); }
    INLINE double log_dn(double d) { return previous_float(gaol_cr_log(d)); }
    INLINE double log_up(double d) { return next_float(gaol_cr_log(d)); }
    INLINE double sin_dn(double d) { return previous_float(gaol_cr_sin(d)); }
    INLINE double sin_up(double d) { return next_float(gaol_cr_sin(d)); }
    INLINE double cos_dn(double d) { return previous_float(gaol_cr_cos(d)); }
    INLINE double cos_up(double d) { return next_float(gaol_cr_cos(d)); }
    INLINE double tan_dn(double d) { return previous_float(gaol_cr_tan(d)); }
    INLINE double tan_up(double d) { return next_float(gaol_cr_tan(d)); }
    INLINE double asin_dn(double d) { return previous_float(gaol_cr_asin(d)); }
    INLINE double asin_up(double d) { return next_float(gaol_cr_asin(d)); }
    INLINE double acos_dn(double d) { return previous_float(gaol_cr_acos(d)); }
    INLINE double acos_up(double d) { return next_float(gaol_cr_acos(d)); }
    INLINE double atan_dn(double d) { return previous_float(gaol_cr_atan(d)); }
    INLINE double atan_up(double d) { return next_float(gaol_cr_atan(d)); }
    INLINE double sinh_dn(double d) { return previous_float(gaol_cr_sinh(d)); }
    INLINE double sinh_up(double d) { return next_float(gaol_cr_sinh(d)); }
    INLINE double cosh_dn(double d) { return previous_float(gaol_cr_cosh(d)); }
    INLINE double cosh_up(double d) { return next_float(gaol_cr_cosh(d)); }
    INLINE double tanh_dn(double d) { return previous_float(gaol_cr_tanh(d)); }
    INLINE double tanh_up(double d) { return next_float(gaol_cr_tanh(d)); }
    INLINE double asinh_dn(double d) { return previous_float(gaol_cr_asinh(d)); }
    INLINE double asinh_up(double d) { return next_float(gaol_cr_asinh(d)); }
    INLINE double acosh_dn(double d) { return previous_float(gaol_cr_acosh(d)); }
    INLINE double acosh_up(double d) { return next_float(gaol_cr_acosh(d)); }
    INLINE double atanh_dn(double d) { return previous_float(gaol_cr_atanh(d)); }
    INLINE double atanh_up(double d) { return next_float(gaol_cr_atanh(d)); }
  } // namespace nearest

  /*
    Computed in the upward rounding GAOL keeps: the tightest bounds, without
    switching the rounding direction. Those of namespace upward take the
    direction to be upward already, as it is in the functions of intervals
    after GAOL_RND_ENTER(): the ones below it check it again, and a function
    of intervals calling one for each bound checked it three times (GAOL v5).
  */
  namespace upward {
    INLINE double nthroot_dn(double d, double e) { return previous_float(gaol_cr_pow(d, e)); }
    INLINE double nthroot_up(double d, double e) { return gaol_cr_pow(d, e); }
    INLINE double atan2_dn(double y, double x) { return previous_float(gaol_cr_atan2(y, x)); }
    INLINE double atan2_up(double y, double x) { return gaol_cr_atan2(y, x); }
    INLINE double exp_dn(double d) { return previous_float(gaol_cr_exp(d)); }
    INLINE double exp_up(double d) { return gaol_cr_exp(d); }
    INLINE double log_dn(double d) { return previous_float(gaol_cr_log(d)); }
    INLINE double log_up(double d) { return gaol_cr_log(d); }
    INLINE double sin_dn(double d) { return previous_float(gaol_cr_sin(d)); }
    INLINE double sin_up(double d) { return gaol_cr_sin(d); }
    INLINE double cos_dn(double d) { return previous_float(gaol_cr_cos(d)); }
    INLINE double cos_up(double d) { return gaol_cr_cos(d); }
    INLINE double tan_dn(double d) { return previous_float(gaol_cr_tan(d)); }
    INLINE double tan_up(double d) { return gaol_cr_tan(d); }
    INLINE double asin_dn(double d) { return previous_float(gaol_cr_asin(d)); }
    INLINE double asin_up(double d) { return gaol_cr_asin(d); }
    INLINE double acos_dn(double d) { return previous_float(gaol_cr_acos(d)); }
    INLINE double acos_up(double d) { return gaol_cr_acos(d); }
    INLINE double atan_dn(double d) { return previous_float(gaol_cr_atan(d)); }
    INLINE double atan_up(double d) { return gaol_cr_atan(d); }
    INLINE double sinh_dn(double d) { return previous_float(gaol_cr_sinh(d)); }
    INLINE double sinh_up(double d) { return gaol_cr_sinh(d); }
    INLINE double cosh_dn(double d) { return previous_float(gaol_cr_cosh(d)); }
    INLINE double cosh_up(double d) { return gaol_cr_cosh(d); }
    INLINE double tanh_dn(double d) { return previous_float(gaol_cr_tanh(d)); }
    INLINE double tanh_up(double d) { return gaol_cr_tanh(d); }
    INLINE double asinh_dn(double d) { return previous_float(gaol_cr_asinh(d)); }
    INLINE double asinh_up(double d) { return gaol_cr_asinh(d); }
    INLINE double acosh_dn(double d) { return previous_float(gaol_cr_acosh(d)); }
    INLINE double acosh_up(double d) { return gaol_cr_acosh(d); }
    INLINE double atanh_dn(double d) { return previous_float(gaol_cr_atanh(d)); }
    INLINE double atanh_up(double d) { return gaol_cr_atanh(d); }
  } // namespace upward

  INLINE double nthroot_dn(double d, double e) { GAOL_RND_ENTER(); return upward::nthroot_dn(d, e); }
  INLINE double nthroot_up(double d, double e) { GAOL_RND_ENTER(); return upward::nthroot_up(d, e); }
  INLINE double atan2_dn(double y, double x) { GAOL_RND_ENTER(); return upward::atan2_dn(y, x); }
  INLINE double atan2_up(double y, double x) { GAOL_RND_ENTER(); return upward::atan2_up(y, x); }
  INLINE double exp_dn(double d) { GAOL_RND_ENTER(); return upward::exp_dn(d); }
  INLINE double exp_up(double d) { GAOL_RND_ENTER(); return upward::exp_up(d); }
  INLINE double log_dn(double d) { GAOL_RND_ENTER(); return upward::log_dn(d); }
  INLINE double log_up(double d) { GAOL_RND_ENTER(); return upward::log_up(d); }
  INLINE double sin_dn(double d) { GAOL_RND_ENTER(); return upward::sin_dn(d); }
  INLINE double sin_up(double d) { GAOL_RND_ENTER(); return upward::sin_up(d); }
  INLINE double cos_dn(double d) { GAOL_RND_ENTER(); return upward::cos_dn(d); }
  INLINE double cos_up(double d) { GAOL_RND_ENTER(); return upward::cos_up(d); }
  INLINE double tan_dn(double d) { GAOL_RND_ENTER(); return upward::tan_dn(d); }
  INLINE double tan_up(double d) { GAOL_RND_ENTER(); return upward::tan_up(d); }
  INLINE double asin_dn(double d) { GAOL_RND_ENTER(); return upward::asin_dn(d); }
  INLINE double asin_up(double d) { GAOL_RND_ENTER(); return upward::asin_up(d); }
  INLINE double acos_dn(double d) { GAOL_RND_ENTER(); return upward::acos_dn(d); }
  INLINE double acos_up(double d) { GAOL_RND_ENTER(); return upward::acos_up(d); }
  INLINE double atan_dn(double d) { GAOL_RND_ENTER(); return upward::atan_dn(d); }
  INLINE double atan_up(double d) { GAOL_RND_ENTER(); return upward::atan_up(d); }
  INLINE double sinh_dn(double d) { GAOL_RND_ENTER(); return upward::sinh_dn(d); }
  INLINE double sinh_up(double d) { GAOL_RND_ENTER(); return upward::sinh_up(d); }
  INLINE double cosh_dn(double d) { GAOL_RND_ENTER(); return upward::cosh_dn(d); }
  INLINE double cosh_up(double d) { GAOL_RND_ENTER(); return upward::cosh_up(d); }
  INLINE double tanh_dn(double d) { GAOL_RND_ENTER(); return upward::tanh_dn(d); }
  INLINE double tanh_up(double d) { GAOL_RND_ENTER(); return upward::tanh_up(d); }
  INLINE double asinh_dn(double d) { GAOL_RND_ENTER(); return upward::asinh_dn(d); }
  INLINE double asinh_up(double d) { GAOL_RND_ENTER(); return upward::asinh_up(d); }
  INLINE double acosh_dn(double d) { GAOL_RND_ENTER(); return upward::acosh_dn(d); }
  INLINE double acosh_up(double d) { GAOL_RND_ENTER(); return upward::acosh_up(d); }
  INLINE double atanh_dn(double d) { GAOL_RND_ENTER(); return upward::atanh_dn(d); }
  INLINE double atanh_up(double d) { GAOL_RND_ENTER(); return upward::atanh_up(d); }

} // namespace gaol_core

#endif /* __gaol_double_op_h__ */
