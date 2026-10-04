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
#ifndef GAOL_INTERVAL_H
#  error "File gaol_interval_fpu.h shall only be included directly by gaol_interval.h"
#endif


#ifndef GAOL_INTERVAL_FPU_H
#define GAOL_INTERVAL_FPU_H

    // Built from constants rather than copied from static intervals, which
    // the dynamic initialization of gaol/gaol_interval.cpp computed after the
    // static objects of a program linked with the static library GAOL, except
    // with MinGW-w64 (GAOL v5)
    GAOL_INLINE interval interval::zero(void)
    {
        return interval(0.0);
    }

    GAOL_INLINE interval interval::universe(void)
    {
        return interval(-GAOL_INFINITY, GAOL_INFINITY);
    }

    // Both bounds NaN, as interval(double) sets them for a NaN, but without the
    // comparisons that decide it: they signal the invalid-operation exception
    // on a NaN, and a build without optimization runs them (GAOL v5)
    GAOL_INLINE interval interval::emptyset(void)
    {
        interval I;
        I.lb_ = I.rb_ = std::numeric_limits<double>::quiet_NaN();
        return I;
    }

    GAOL_INLINE interval interval::positive(void) // [0, +oo]
    {
        return interval(0.0, GAOL_INFINITY);
    }

    GAOL_INLINE interval interval::negative(void) // [-oo, 0]
    {
        return interval(-GAOL_INFINITY, 0.0);
    }



  GAOL_INLINE
  interval::interval(void)
  {
    lb_ = GAOL_INFINITY;
    rb_ = GAOL_INFINITY;
  }

  // An infinite a gives the empty set, as in IBEX: IEEE 1788-2015 has no
  // interval [+oo, +oo] nor [-oo, -oo] (10.5.8)
  GAOL_INLINE
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
  GAOL_INLINE
  interval::interval(double a, double b)
  {
    if (a <= b && a < GAOL_INFINITY && b > -GAOL_INFINITY) {
      lb_ = -a;
      rb_ = b;
    } else {
      lb_ = rb_ = std::numeric_limits<double>::quiet_NaN();
    }
  }

    GAOL_INLINE
    interval::interval(const interval& I)
    {
        lb_ = I.lb_;
        rb_ = I.rb_;
    }


  // [-right(), -left()]: the stored bounds exchanged, as with the SSE2
  // intervals, rather than given to the constructor, which compares them and
  // raises the invalid-operation exception on the NaN bounds of the empty set
  // (GAOL v5)
  GAOL_INLINE
  interval interval::operator-(void) const
  {
    interval I;
    I.lb_ = rb_;
    I.rb_ = lb_;
    return I;
  }

 GAOL_INLINE
  interval& interval::operator&=(const interval& I)
  {
#if defined(__arm__) && !defined(__aarch64__)
    // GCC for 32-bit ARM processors (12 to 14) may compile the quiet
    // comparisons of the code after #else into signaling ones: where it turns
    // the choice of a bound into a conditional move, it reverses the
    // comparison, "I.right() < right() or unordered" becoming "I.right() >=
    // right()", and compiles the reversed one with vcmpe, which raises the
    // invalid-operation exception on a NaN, rather than vcmp (GCC bug 52258 is
    // of this kind): x & y, intersection() and the reverse functions
    // intersecting with an empty y raised it on armhf. Both operands are
    // therefore told empty before their bounds are compared, so that these
    // comparisons never meet a NaN. "this" is told with is_empty(), as after
    // #else: when it is false, both bounds of "this" are ordered, and it is
    // the test a program makes of the result, which GCC then knows to be true
    // on that path (told with isunordered(), an empty "this" made GCC compare
    // the NaN bounds again, with vcmpe, in the program's
    // (empty & x).is_empty() ? a : b). I is told with isunordered(), which
    // stays a vcmp when GCC reverses it into "ordered" (GAOL v5)
    if (is_empty()) {
      return *this;
    }
    if (std::isunordered(I.lb_, I.rb_)) {
      lb_ = rb_ = std::numeric_limits<double>::quiet_NaN();
      return *this;
    }
    if (I.left() > left()) {
      lb_ = I.lb_;
    }
    if (I.right() < right()) {
      rb_ = I.rb_;
    }
#else
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
#endif
    // Disjoint intervals give the empty set, [NaN, NaN] as interval::emptyset()
    // (GAOL v5): their bounds in the wrong order, [3, 2] for
    // [1, 2] & [3, 4], were empty for is_empty(), but the operations computing
    // on the bounds gave [3, 2] + [0, 1] = [3, 3]
    if (is_empty()) {
      lb_ = rb_ = std::numeric_limits<double>::quiet_NaN();
    }
    return *this;
  }

  GAOL_INLINE
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


GAOL_INLINE double
interval::left_internal() const
{
    return lb_;
}

GAOL_INLINE double
interval::right_internal() const
{
    return rb_;
}

#endif // GAOL_INTERVAL_FPU_H
