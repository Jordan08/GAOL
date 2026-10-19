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
* Copyright (c) 2026 ENSTA, France
*--------------------------------------------------------------------------
* gaol is a software distributed WITHOUT ANY WARRANTY. Read the associated
* COPYING file for information.
*--------------------------------------------------------------------------
* By: Frederic Goualard <Frederic.Goualard@univ-nantes.fr>, Jordan NININ
*--------------------------------------------------------------------------*/

/*
This file shall be included by gaol_interval_core.h only. It must not be included directly
by any other file.
This file contains all declarations related to the 'interval' class that consider
the bounds to be a float64x2_t variable, on 64-bit ARM processors (GAOL v5).
It is the counterpart of gaol_interval_sse.h: the same stored form, <-left,
right> in the two lanes, so that one addition rounded upward gives both bounds.
**/

#ifndef GAOL_INTERVAL_CORE_H
#  error "File gaol_interval_neon.h shall only be included directly by gaol_interval_core.h"
#endif

#ifndef GAOL_INTERVAL_NEON_H
#define GAOL_INTERVAL_NEON_H

// Built from the lanes, not held in static intervals (GAOL v5): the
// constants of the SSE2 intervals are masks that the library initializes
// when it is compiled, the lanes here are written in the code that uses them
GAOL_INLINE interval interval::universe(void)
{
  return interval(vdupq_n_f64(GAOL_INFINITY));
}

// <+0, +0>, as the SSE2 intervals: [0, 0] with -0 as its left bound
GAOL_INLINE interval interval::zero(void)
{
  return interval(vdupq_n_f64(0.0));
}

// Both bounds NaN, as interval(double) sets them for a NaN, but without the
// comparisons that decide it: they signal the invalid-operation exception on
// a NaN, and a build without optimization runs them (GAOL v5)
GAOL_INLINE interval interval::emptyset(void)
{
  return interval(vdupq_n_f64(std::numeric_limits<double>::quiet_NaN()));
}

GAOL_INLINE interval::interval(const float64x2_t& v)
{
  neonbounds = v;
}

GAOL_INLINE interval interval::positive(void)
{
  return interval(0.0,std::numeric_limits<double>::infinity());
}

GAOL_INLINE interval interval::negative(void)
{
  return interval(-std::numeric_limits<double>::infinity(),0.0);
}

// An infinite v gives the empty set, as in IBEX: IEEE 1788-2015 has no
// interval [+oo, +oo] nor [-oo, -oo] (10.5.8). A NaN gives it too, told by
// a quiet first comparison (see interval(double, double)). The bounds go
// through GAOL_FPU_SCALAR() as with the FPU intervals (gaol_interval_core.h):
// GCC for 64-bit ARM vectorizes loops of constructions and compares the
// bounds of every element, an empty one included, with signaling comparisons
GAOL_INLINE interval::interval(double v)
{
  GAOL_FPU_SCALAR(v);
  if (gaol_detail::quiet_less(-GAOL_INFINITY, v) && v < GAOL_INFINITY) { // false for a NaN
    neonbounds = vcombine_f64(vdup_n_f64(-v), vdup_n_f64(v));
  } else {
    neonbounds = vdupq_n_f64(std::numeric_limits<double>::quiet_NaN());
  }
}

// The empty set for a lower bound of +oo, an upper bound of -oo, bounds in
// the wrong order and NaN bounds, as in IBEX; the first comparison is a quiet
// one, as in is_empty() and the FPU and SSE2 constructors (GAOL v5, decided
// for point D.24 of TODO.md), and the bounds are compared as bounds
// (gaol_detail::bound_less_equal(), gaol_port.h), which the modes that flush
// the subnormals to zero do not change (point Q of TODO.md)
GAOL_INLINE interval::interval(double l, double r)
{
  GAOL_FPU_SCALAR(l);
  GAOL_FPU_SCALAR(r);
  if (gaol_detail::bound_less_equal(l, r) && l < GAOL_INFINITY && r > -GAOL_INFINITY) {
    neonbounds = vcombine_f64(vdup_n_f64(-l), vdup_n_f64(r));
  } else {
    neonbounds = vdupq_n_f64(std::numeric_limits<double>::quiet_NaN());
  }
}

GAOL_INLINE interval::interval()
{
  neonbounds = vdupq_n_f64(std::numeric_limits<double>::infinity());
}

GAOL_INLINE interval::interval(const interval& I)
{
  neonbounds = I.neonbounds;
}

GAOL_INLINE double interval::left_internal() const
{
  return vgetq_lane_f64(neonbounds, 0);
}

GAOL_INLINE double interval::right_internal() const
{
  return vgetq_lane_f64(neonbounds, 1);
}

GAOL_INLINE const float64x2_t& interval::get_neoninterval(void) const
{
  return neonbounds;
}

GAOL_INLINE float64x2_t& interval::get_neoninterval(void)
{
  return neonbounds;
}

// [-right(), -left()]: the two lanes exchanged, with no comparison (the
// constructor would raise the invalid-operation exception on the NaN bounds
// of the empty set)
GAOL_INLINE interval interval::operator-(void) const
{
  return interval(vextq_f64(neonbounds, neonbounds, 1));
}

/*
  The intersection of the SSE2 intervals, on the lanes (GAOL v5): one quiet
  comparison, std::isunordered() of the two stored lower bounds, tells whether
  either operand is empty, both bounds of the empty set being NaN. No vector
  comparison: those of 64-bit ARM, fcmgt and fcmge, signal the
  invalid-operation exception on a NaN, and a build under -frounding-math
  compiles them as scalar comparisons all the same. The bounds are compared
  as bounds, with the quiet comparisons of gaol_port.h, which the modes that
  flush the subnormals to zero do not change (point Q of TODO.md). They go
  through GAOL_FPU_SCALAR(), as in the FPU intervals.
*/
GAOL_INLINE interval& interval::operator&=(const interval& I)
{
  double bd0 = vgetq_lane_f64(neonbounds, 0), bd1 = vgetq_lane_f64(neonbounds, 1);
  const double Ibd0 = vgetq_lane_f64(I.neonbounds, 0), Ibd1 = vgetq_lane_f64(I.neonbounds, 1);
  if (gaol_detail::quiet_unordered(bd0, Ibd0)) {
    neonbounds = vdupq_n_f64(std::numeric_limits<double>::quiet_NaN());
    return *this;
  }
  if (!gaol_detail::bound_greater_equal(Ibd0, bd0)) { // Left bounds negated
    bd0 = Ibd0;
  }
  if (!gaol_detail::bound_greater_equal(Ibd1, bd1)) {
    bd1 = Ibd1;
  }
  GAOL_FPU_SCALAR(bd0);
  GAOL_FPU_SCALAR(bd1);
  // Disjoint intervals give the empty set, [NaN, NaN] as interval::emptyset()
  if (!gaol_detail::bound_less_equal(-bd0, bd1)) {
    neonbounds = vdupq_n_f64(std::numeric_limits<double>::quiet_NaN());
  } else {
    neonbounds = vcombine_f64(vdup_n_f64(bd0), vdup_n_f64(bd1));
  }
  return *this;
}

// The bounds compared as in operator&=() (GAOL v5)
GAOL_INLINE interval& interval::operator|=(const interval& I)
{
  if (is_empty()) {
    *this = I;
    return *this;
  }
  if (I.is_empty()) {
    return *this;
  }
  double bd0 = vgetq_lane_f64(neonbounds, 0), bd1 = vgetq_lane_f64(neonbounds, 1);
  const double Ibd0 = vgetq_lane_f64(I.neonbounds, 0), Ibd1 = vgetq_lane_f64(I.neonbounds, 1);
  if (gaol_detail::bound_greater(Ibd0, bd0)) { // Left bounds negated
    bd0 = Ibd0;
  }
  if (gaol_detail::bound_greater(Ibd1, bd1)) {
    bd1 = Ibd1;
  }
  neonbounds = vcombine_f64(vdup_n_f64(bd0), vdup_n_f64(bd1));
  return *this;
}

/*
  The sum, the difference, the product, the quotient and the square of
  intervals, on their stored bounds (the left one negated), computed by the
  library, as for the SSE2 intervals (see gaol_interval_sse.h): the operators
  call these functions with the two lanes in a register, which they give back
  in one. They are not part of the interface. The check of the rounding
  direction, and the code that depends on it, are in the library, compiled
  with its flags, not in the code of the program that calls them (GAOL v5).
*/
GAOL_NODISCARD extern GAOL_PUBLIC float64x2_t add_bounds(float64x2_t x, float64x2_t y);
GAOL_NODISCARD extern GAOL_PUBLIC float64x2_t sub_bounds(float64x2_t x, float64x2_t y);
GAOL_NODISCARD extern GAOL_PUBLIC float64x2_t mul_bounds(float64x2_t x, float64x2_t y);
GAOL_NODISCARD extern GAOL_PUBLIC float64x2_t div_bounds(float64x2_t x, float64x2_t y);
GAOL_NODISCARD extern GAOL_PUBLIC float64x2_t sqr_bounds(float64x2_t x);

GAOL_INLINE interval& interval::operator+=(const interval& I)
{
  neonbounds = add_bounds(neonbounds, I.neonbounds);
  return *this;
}

GAOL_INLINE interval& interval::operator-=(const interval& I)
{
  neonbounds = sub_bounds(neonbounds, I.neonbounds);
  return *this;
}

GAOL_INLINE interval& interval::operator*=(const interval& I)
{
  neonbounds = mul_bounds(neonbounds, I.neonbounds);
  return *this;
}

GAOL_INLINE interval& interval::operator/=(const interval& I)
{
  neonbounds = div_bounds(neonbounds, I.neonbounds);
  return *this;
}

//! I^2
GAOL_NODISCARD GAOL_INLINE interval sqr(const interval& I)
{
  return interval(sqr_bounds(I.get_neoninterval()));
}

#endif // GAOL_INTERVAL_NEON_H
