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
  \file   gaol_interval_hyperbolic.cpp
  \brief  The hyperbolic functions of the intervals: sinh, cosh, tanh, asinh, acosh and atanh.
          Their reverse functions are in gaol_interval_relational.cpp.

  \author Goualard Frederic, NININ Jordan
  \date   2001-09-28
*/

#include "gaol/core/gaol_interval_internal.h"

/*
  sinh, cosh, tanh, asinh, acosh and atanh (GAOL v5: a part of
  gaol_interval_core.cpp, see gaol_interval_internal.h). Their reverse
  functions are in gaol_interval_relational.cpp.
*/

namespace gaol_core {

  // The bounds of the hyperbolic functions where they are known (see the
  // bounds of the elementary functions in gaol_interval_internal.h)
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
    interval res = acosh_upward(I);
    GAOL_RND_KEEP(res);
    GAOL_RND_LEAVE();
    return res;
  }



  interval asinh(const interval& I)
  {
    if (I.is_empty()) {
      return I;
    }

    GAOL_RND_ENTER();
    interval res = asinh_upward(I);
    GAOL_RND_KEEP(res);
    GAOL_RND_LEAVE();
    return res;

  }

  interval atanh(const interval& I)
  {
    // The check before the intersection, as in acos()
    GAOL_RND_ENTER();
    interval res = atanh_upward(I);
    GAOL_RND_KEEP(res);
    GAOL_RND_LEAVE();
    return res;
  }

} // namespace gaol_core
