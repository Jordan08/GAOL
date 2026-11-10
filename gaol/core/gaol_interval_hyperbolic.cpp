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
  \brief  The hyperbolic functions of the intervals: gaol_core

  sinh, cosh, tanh, asinh, acosh and atanh (GAOL v5: a part of
  gaol_interval_core.cpp, see gaol_interval_internal.h). Their reverse
  functions are in gaol_interval_relational.cpp.

  \author Goualard Frederic, NININ Jordan
  \date   2001-09-28
*/

#include "gaol/core/gaol_interval_internal.h"

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

  static inline double asinh_lo(double x) { return (x == 0.0) ? 0.0 : upward::asinh_dn(x); }
  static inline double asinh_hi(double x) { return (x == 0.0) ? 0.0 : upward::asinh_up(x); }
  static inline double acosh_lo(double x) { return (x == 1.0) ? 0.0 : upward::acosh_dn(x); }
  static inline double acosh_hi(double x) { return (x == 1.0) ? 0.0 : upward::acosh_up(x); }
  static inline double atanh_lo(double x) { return (x == 0.0) ? 0.0 : upward::atanh_dn(x); }
  static inline double atanh_hi(double x) { return (x == 0.0) ? 0.0 : upward::atanh_up(x); }

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


  /*
    The bodies of acosh(), asinh() and atanh() after their check, for these
    functions and for acosh_rel(), asinh_rel() and atanh_rel(), which check
    the direction before them (GAOL v5: the reverse functions called acosh(),
    asinh() and atanh(), which checked it a second time)
  */
  interval acosh_upward(const interval& I)
  {
    const interval J = I &  interval::one_plus_infinity();

    if (J.is_empty()) {
      return J;
    }
    const double l = acosh_lo(J.left()), r = acosh_hi(J.right());
    return interval(l,r);
  }

  interval asinh_upward(const interval& I)
  {
    if (I.is_empty()) {
      return I;
    }
    const double l = asinh_lo(I.left()), r = asinh_hi(I.right());
    return interval(l,r);
  }

  // by_division: the infinite bounds at -1 and 1 are those of CORE-MATH, which
  // raises the division-by-zero exception for them; atanh_rel() takes them
  // without a division, its result being a finite interval where I is (GAOL
  // v5, point 79 of TODO.md: atanh_rel([0, 1], [0, 1]) raised it)
  interval atanh_upward(const interval& I, bool by_division)
  {
    const interval J = I & interval::minus_one_plus_one();
    // atanh is defined on (-1, 1) (IEEE 1788-2015, Table 9.1, GAOL v5): an I
    // meeting [-1, 1] at 1 alone, or at -1 alone, as [1] and [-5,-1], holds
    // no point of it, and gives the empty set. The value of CORE-MATH at 1 is
    // +oo, and the lower bound the double below the value: [1] was [MAX,+oo]
    // ([-1] was empty only because interval(-oo,-oo) is). The limits -oo at
    // -1 and +oo at 1 remain the bounds when I holds other points of (-1, 1)
    if (J.is_empty() || J.left() == 1.0 || J.right() == -1.0) {
      return interval::emptyset();
    }
    const double l = (!by_division && J.left() == -1.0) ? -GAOL_INFINITY : atanh_lo(J.left());
    const double r = (!by_division && J.right() == 1.0) ? GAOL_INFINITY : atanh_hi(J.right());
    return interval(l,r);
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
