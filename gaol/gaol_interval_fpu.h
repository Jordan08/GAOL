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
  // interval [+oo, +oo] nor [-oo, -oo] (10.5.8). A NaN gives it too, told by
  // a quiet first comparison (see interval(double, double))
  GAOL_INLINE
  interval::interval(double a)
  {
#if (defined(__arm__) && !defined(__aarch64__)) || defined(_ARCH_PWR9)
    // std::isunordered() first, as in is_empty() and interval(double, double)
    double b = a;
    if (!detail::quiet_unordered(a, b)) {
      detail::keep_ordered(a, b);
      if (-GAOL_INFINITY < a && b < GAOL_INFINITY) {
        lb_ = -a;
        rb_ = b;
        return;
      }
    }
    lb_ = rb_ = std::numeric_limits<double>::quiet_NaN();
#else
    GAOL_FPU_SCALAR(a);
    // A branch rather than two conditional moves, which would make the bounds
    // depend on the comparison and lengthen the loops accumulating intervals
    if (detail::quiet_less(-GAOL_INFINITY, a) && a < GAOL_INFINITY) { // false for a NaN
      lb_ = -a;
      rb_ = a;
    } else {
      lb_ = rb_ = std::numeric_limits<double>::quiet_NaN();
    }
#endif
  }

  // The empty set for a lower bound of +oo, an upper bound of -oo, bounds in
  // the wrong order and NaN bounds, as in IBEX. The first comparison is a
  // quiet one, as in is_empty(), ucomisd for comisd on x86, at the same
  // cost: a NaN bound, which <= compared, raised the invalid-operation
  // exception, and so did interval(NAN), x += NAN and the functions giving
  // the constructor the NaN bounds of an empty operand (floor(), max()...),
  // which had to tell the empty set first (GAOL v5, decided for point D.24 of
  // TODO.md). The other two compare bounds that are no NaN once it is true:
  // made quiet too, they cost GCC a conditional move through the integer
  // registers where it if-converts them, and 30% more in a loop of
  // constructions (GCC 9.4, FPU intervals). GCC for 64-bit ARM computed them
  // for every element of a loop of constructions it vectorized, an empty one
  // included: the bounds go through GAOL_FPU_SCALAR() (gaol_interval.h),
  // which keeps GCC from vectorizing that loop. On 32-bit ARM and on POWER9,
  // the quiet first comparison has the form GCC reversed into a signaling one
  // in is_empty(), and the constructor tells NaN bounds as is_empty() does,
  // with std::isunordered() first, then compares the bounds, which are no NaN
  // after it, through the empty asm statement of detail::keep_ordered() (GAOL
  // v5): floor(), max(), min()... give it the NaN bounds of an empty operand
  // without testing it first
  GAOL_INLINE
  interval::interval(double a, double b)
  {
#if (defined(__arm__) && !defined(__aarch64__)) || defined(_ARCH_PWR9)
    if (!detail::quiet_unordered(a, b)) {
      detail::keep_ordered(a, b);
      if (a <= b && a < GAOL_INFINITY && b > -GAOL_INFINITY) {
        lb_ = -a;
        rb_ = b;
        return;
      }
    }
    lb_ = rb_ = std::numeric_limits<double>::quiet_NaN();
#else
    GAOL_FPU_SCALAR(a);
    GAOL_FPU_SCALAR(b);
    if (detail::quiet_less_equal(a, b) && a < GAOL_INFINITY && b > -GAOL_INFINITY) {
      lb_ = -a;
      rb_ = b;
    } else {
      lb_ = rb_ = std::numeric_limits<double>::quiet_NaN();
    }
#endif
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

  /*
    One quiet comparison, std::isunordered() of the two stored lower bounds,
    tells whether either operand is empty, both bounds of the empty set being
    NaN, and the intersection is then [NaN, NaN], interval::emptyset() (GAOL
    v5, decided for point D.24 of TODO.md). Before it, *this was told empty
    with is_empty() and returned as it was, with the NaN it held, and I with
    the quiet comparisons of its bounds below, or on 32-bit ARM with
    std::isunordered() of its bounds (#60): GCC for 32-bit ARM (12 to 14),
    where it turns the choice of a bound into a conditional move, reverses
    the comparison, "I.right() < right() or unordered" becoming "I.right() >=
    right()", and compiles the reversed one with vcmpe, which raises the
    invalid-operation exception on a NaN, rather than vcmp (GCC bug 52258 is
    of this kind), and GCC 13 for POWER9 with xscmpgedp: x & y,
    intersection() and the reverse functions intersecting with an empty y
    raised it on armhf, and x &= y for an empty y on POWER9.
    std::isunordered() stays quiet when GCC reverses it ("ordered" stays
    vcmp), and no NaN reaches the comparisons after it. (An empty *this told
    so made GCC compare its NaN bounds again with vcmpe in the program's
    (empty & x).is_empty() ? a : b, where is_empty() was one quiet
    comparison: is_empty() tells them with std::isunordered() first on 32-bit
    ARM now, see gaol_interval.h.) The comparisons after it are quiet ones
    all the same: GCC 9.4 at -O3 computed them before that test, for an empty
    I, in a loop of intersections where it turned the function into
    straight-line code, and compiled plain comparisons with comisd, which
    raised the invalid-operation exception (tests/rounding_direction.cpp).
  */
  GAOL_INLINE
  interval& interval::operator&=(const interval& I)
  {
    if (detail::quiet_unordered(lb_, I.lb_)) {
      lb_ = rb_ = std::numeric_limits<double>::quiet_NaN();
      return *this;
    }
    if (!detail::quiet_less_equal(I.left(), left())) {
      lb_ = I.lb_;
    }
    if (!detail::quiet_greater_equal(I.right(), right())) {
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
