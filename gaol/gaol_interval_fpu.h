/*-*-C++-*------------------------------------------------------------------
 * gaol -- NOT Just Another Interval Library
 *--------------------------------------------------------------------------
 * This file is part of the gaol distribution. Gaol was primarily
 * developed at the Swiss Federal Institute of Technology, Lausanne,
 * Switzerland, and is now developed at the Laboratoire d'Informatique de
 * Nantes-Atlantique, France.
 *
 * Copyright (c) 2001 Swiss Federal Institute of Technology, Switzerland
 * Copyright (c) 2002-2009 Laboratoire d'Informatique de
 *                         Nantes-Atlantique, France
 *--------------------------------------------------------------------------
 * gaol is a software distributed WITHOUT ANY WARRANTY. Read the associated
 * COPYING file for information.
 *--------------------------------------------------------------------------
 * SVN: $Id: gaol_interval_fpu.h 191 2012-03-06 17:08:58Z goualard $
 * Last modified:
 * By: Frederic Goualard <Frederic.Goualard@univ-nantes.fr>
 *--------------------------------------------------------------------------*/

/*
    This file shall be included by gaol_interval.h only. It must not be included directly
        by any other file.

    This file contains all declarations related to the 'interval' class that consider
    the bounds to be a pair of 'double' variables.
*/
#ifndef __gaol_interval_h__
#  error "File gaol_interval_fpu.h shall only be included directly by gaol_interval.h"
#endif


#ifndef __gaol_interval_fpu_h__
#define __gaol_interval_fpu_h__

    INLINE interval interval::zero(void)
    {
        return interval::cst_zero;
    }

    INLINE interval interval::universe(void)
    {
        return interval::cst_universe;
    }

    INLINE interval interval::emptyset(void)
    {
        return interval::cst_emptyset;
    }

    INLINE interval interval::positive(void) // [0, +oo]
    {
        return interval::cst_positive;
    }

    INLINE interval interval::negative(void) // [-oo, 0]
    {
        return interval::cst_negative;
    }



  INLINE
  interval::interval(void)
  {
    lb_ = GAOL_INFINITY;
    rb_ = GAOL_INFINITY;
  }

  // An infinite a gives the empty set, as in IBEX: IEEE 1788-2015 has no
  // interval [+oo, +oo] nor [-oo, -oo] (10.5.8)
  INLINE
  interval::interval(double a)
  {
    // A branch rather than two conditional moves, which would make the bounds
    // depend on the comparison and lengthen the loops accumulating intervals
    if (-GAOL_INFINITY < a && a < GAOL_INFINITY) { // false for a NaN
      lb_ = -a;
      rb_ = a;
    } else {
      lb_ = rb_ = std::numeric_limits<double>::quiet_NaN();
    }
  }

  // The empty set for a lower bound of +oo, an upper bound of -oo, bounds in
  // the wrong order and NaN bounds, as in IBEX
  INLINE
  interval::interval(double a, double b)
  {
    if (a <= b && a < GAOL_INFINITY && b > -GAOL_INFINITY) {
      lb_ = -a;
      rb_ = b;
    } else {
      lb_ = rb_ = std::numeric_limits<double>::quiet_NaN();
    }
  }

    INLINE
    interval::interval(const interval& I)
    {
        lb_ = I.lb_;
        rb_ = I.rb_;
    }


  INLINE
  interval interval::operator-(void) const
  {
    return interval(-right(),-left());
  }

 INLINE
  interval& interval::operator&=(const interval& I)
  {
    if (is_empty()) {
      return *this;
    }
    // From now on, "this" is known to be nonempty.
    if (!(I.left() <= left())) { // I.left() == NaN => lb_ <- NaN
      lb_ = I.lb_;
    }
    if (!(I.right() >= right())) {
      rb_ = I.rb_;
    }
    return *this;
  }

  INLINE
  interval& interval::operator|=(const interval& I)
  {
    if (is_empty()) {
      *this = I;
      return *this;
    }
    if (I.is_empty()) {
      return *this;
    }

    if (I.left() < left()) {
      lb_ = I.lb_;
    }
    if (I.right() > right()) {
      rb_ = I.rb_;
    }
    return *this;
  }


INLINE double
interval::left_internal() const
{
    return lb_;
}

INLINE double
interval::right_internal() const
{
    return rb_;
}

#endif // __gaol_interval_fpu_h__
