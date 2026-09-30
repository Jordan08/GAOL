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

    // Built from constants rather than copied from static intervals, which
    // the dynamic initialization of gaol/gaol_interval.cpp computed after the
    // static objects of a program linked with the static library GAOL, except
    // with MinGW-w64 (GAOL v5)
    INLINE interval interval::zero(void)
    {
        return interval(0.0);
    }

    INLINE interval interval::universe(void)
    {
        return interval(-GAOL_INFINITY, GAOL_INFINITY);
    }

    // Both bounds NaN, as interval(double) sets them for a NaN, but without the
    // comparisons that decide it: they signal the invalid-operation exception
    // on a NaN, and a build without optimization runs them (GAOL v5)
    INLINE interval interval::emptyset(void)
    {
        interval I;
        I.lb_ = I.rb_ = std::numeric_limits<double>::quiet_NaN();
        return I;
    }

    INLINE interval interval::positive(void) // [0, +oo]
    {
        return interval(0.0, GAOL_INFINITY);
    }

    INLINE interval interval::negative(void) // [-oo, 0]
    {
        return interval(-GAOL_INFINITY, 0.0);
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


  // [-right(), -left()]: the stored bounds exchanged, as with the SSE2
  // intervals, rather than given to the constructor, which compares them and
  // raises the invalid-operation exception on the NaN bounds of the empty set
  // (GAOL v5)
  INLINE
  interval interval::operator-(void) const
  {
    interval I;
    I.lb_ = rb_;
    I.rb_ = lb_;
    return I;
  }

 INLINE
  interval& interval::operator&=(const interval& I)
  {
    if (is_empty()) {
      return *this;
    }
    // From now on, "this" is known to be nonempty.
    // The comparisons are quiet ones, which raise no invalid-operation
    // exception on the NaN bounds of an empty I (GAOL v5)
    if (!std::islessequal(I.left(), left())) { // I.left() == NaN => lb_ <- NaN
      lb_ = I.lb_;
    }
    if (!std::isgreaterequal(I.right(), right())) {
      rb_ = I.rb_;
    }
    // Disjoint intervals give the empty set, [NaN, NaN] as interval::emptyset()
    // (GAOL v5): their bounds in the wrong order, [3, 2] for
    // [1, 2] & [3, 4], were empty for is_empty(), but the operations computing
    // on the bounds gave [3, 2] + [0, 1] = [3, 3]
    if (is_empty()) {
      lb_ = rb_ = std::numeric_limits<double>::quiet_NaN();
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
