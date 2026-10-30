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

/*
  The helpers of the intersection and the hull, and of the kernels of
  gaol/core/gaol_interval_neon.cpp (GAOL v5). They work on the bits of the bounds, with
  integer instructions: these depend neither on the rounding direction nor on the
  modes that flush the subnormals to zero, which the operations that only compare
  or copy bounds do not check (point Q of TODO.md), and raise nothing on a NaN,
  which the empty set holds in both bounds, where the comparisons of 64-bit ARM on
  doubles signal the invalid-operation exception on a quiet NaN.
*/
namespace neon_detail {

  const std::uint64_t abs_mask = 0x7fffffffffffffffull;
  const std::uint64_t infinity_bits = 0x7ff0000000000000ull;

  GAOL_INLINE uint64x2_t bits_of(float64x2_t x) { return vreinterpretq_u64_f64(x); }
  GAOL_INLINE float64x2_t from_bits(uint64x2_t x) { return vreinterpretq_f64_u64(x); }
  GAOL_INLINE uint64x2_t abs_bits(float64x2_t x) { return vandq_u64(bits_of(x), vdupq_n_u64(abs_mask)); }
  // All ones in the lanes that hold a NaN
  GAOL_INLINE uint64x2_t nan_lanes(float64x2_t x) { return vcgtq_u64(abs_bits(x), vdupq_n_u64(infinity_bits)); }
  GAOL_INLINE bool any_lane(uint64x2_t m) { return vmaxvq_u32(vreinterpretq_u32_u64(m)) != 0; }

  /*
    The order of the doubles that are no NaN, as signed integers, in both lanes
    (bound_key() of gaol/gaol_port.h): -0 and +0 are 0. The stored left bound
    -l has the key of -l, and the key of its opposite l is the opposite of it
  */
  GAOL_INLINE int64x2_t bound_keys(float64x2_t x)
  {
    const int64x2_t magnitude = vreinterpretq_s64_u64(abs_bits(x));
    const uint64x2_t negative = vcltzq_s64(vreinterpretq_s64_f64(x));
    return vbslq_s64(negative, vnegq_s64(magnitude), magnitude);
  }

  /*
    The keys of the stored bounds with the smallest integer for a NaN, so that the
    empty set, which holds NaN in both lanes, loses any choice of the greater of
    two bounds: the hull of the empty set and of x is x, without a test
  */
  GAOL_INLINE int64x2_t bound_keys_nan_lowest(float64x2_t x)
  {
    return vbslq_s64(nan_lanes(x), vdupq_n_s64(std::numeric_limits<std::int64_t>::min()), bound_keys(x));
  }

} // namespace neon_detail

// Built from the lanes, not held in static intervals (GAOL v5): the
// constants of the SSE2 intervals are masks that the library initializes
// when it is compiled, the lanes here are written in the code that uses them
GAOL_INLINE interval interval::universe(void)
{
  return interval(vdupq_n_f64(GAOL_INFINITY), gaol_detail::unchecked_bounds);
}

// <+0, +0>, as the SSE2 intervals: [0, 0] with -0 as its left bound
GAOL_INLINE interval interval::zero(void)
{
  return interval(vdupq_n_f64(0.0), gaol_detail::unchecked_bounds);
}

// Both bounds NaN, as interval(double) sets them for a NaN, but without the
// comparisons that decide it: they signal the invalid-operation exception on
// a NaN, and a build without optimization runs them (GAOL v5)
GAOL_INLINE interval interval::emptyset(void)
{
  return interval(vdupq_n_f64(std::numeric_limits<double>::quiet_NaN()), gaol_detail::unchecked_bounds);
}

GAOL_INLINE interval::interval(const float64x2_t& v, gaol_detail::unchecked_bounds_t)
{
  neonbounds = v;
}

// The stored bounds <-l, r>: -oo in a lane is l = +oo or r = -oo, which no
// interval has (IEEE 1788-2015, 10.5.8). Told by the bits, with integer
// instructions: nothing is compared as a double
GAOL_INLINE interval::interval(const float64x2_t& v)
{
  using namespace neon_detail;
  if (any_lane(vceqq_u64(bits_of(v), vdupq_n_u64(infinity_bits | 0x8000000000000000ull)))) {
    neonbounds = vdupq_n_f64(std::numeric_limits<double>::quiet_NaN());
  } else {
    neonbounds = v;
  }
}

GAOL_INLINE interval interval::positive(void)
{
  return interval(0.0,std::numeric_limits<double>::infinity());
}

GAOL_INLINE interval interval::negative(void)
{
  return interval(-std::numeric_limits<double>::infinity(),0.0);
}

namespace neon_detail {

  /*
    Whether d is neither a NaN nor an infinity, for which interval(d) is the empty set (GAOL v5, point
    PG of TODO.md): |d| < +oo, one quiet comparison in place of the two with -oo and +oo of the old test
    (-oo < d && d < +oo), which were 4 vector instructions (two constants and two comparisons) in place
    of the 3 of the new one (fabs, the move of the result and the comparison). Counted under qemu, the
    path of x + d is 20 instructions with GCC 12 in place of 23, and 21 with Clang 18 and 21 in place
    of 26. The same flags as before: the quiet comparison raises the invalid-operation exception for a
    signaling NaN, and the flag of a subnormal d read as zero under the modes that flush them
    (FPSR.IDC), as the first of the old comparisons did, and fabs() changes neither (it raises nothing
    and does not quiet a signaling NaN; with FPCR.AH clear it clears the sign of a NaN, which the
    comparison does not read)
  */
  GAOL_INLINE bool finite_double(double d) { return gaol_detail::quiet_less(std::fabs(d), GAOL_INFINITY); }

  // An empty asm statement in the branch of the empty set. Without it Clang 18 to 21 make a select of the
  // two results in interval(d), a mask and a bit select that are 3 vector instructions and 2 scalar ones
  // more than the branch, and in x op= d they move the constant NaN above the test, onto the path that
  // runs. GCC 12 to 15 keep the branch either way
  GAOL_INLINE void keep_branch()
  {
#  if !GAOL_NEON_MSVC
    __asm__ __volatile__ ("");
#  endif
  }

} // namespace neon_detail

// An infinite v gives the empty set, as in IBEX: IEEE 1788-2015 has no
// interval [+oo, +oo] nor [-oo, -oo] (10.5.8). A NaN gives it too, told by
// a quiet comparison of |v| with +oo (neon_detail::finite_double()). The bounds go
// through GAOL_FPU_SCALAR() as with the FPU intervals (gaol_interval_core.h):
// GCC for 64-bit ARM vectorizes loops of constructions and compares the
// bounds of every element, an empty one included, with signaling comparisons
GAOL_INLINE interval::interval(double v)
{
  GAOL_FPU_SCALAR(v);
  if (GAOL_UNLIKELY(!neon_detail::finite_double(v))) { // a NaN and an infinity
    neon_detail::keep_branch();
    neonbounds = vdupq_n_f64(std::numeric_limits<double>::quiet_NaN());
  } else {
    neonbounds = vcombine_f64(vdup_n_f64(-v), vdup_n_f64(v));
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
  return interval(vextq_f64(neonbounds, neonbounds, 1), gaol_detail::unchecked_bounds);
}

/*
  The intersection and the hull, on the stored bounds <-l, r> (GAOL v5): each
  stored bound is the lesser of the two operands' (the greater left bound, the
  lesser right bound) for the intersection, the greater for the hull, chosen by
  one comparison of the keys of the bounds, the integers of
  neon_detail::bound_keys(), which order the doubles whatever the modes that
  flush the subnormals to zero (point Q of TODO.md), and which tell -0 and +0
  equal, the bound of *this being kept: no comparison of doubles, those of
  64-bit ARM signaling the invalid-operation exception on the NaN bounds of
  the empty set. The empty set is told by its NaN, which is never an interval
  of the interface with bounds that cross.
*/
GAOL_INLINE interval& interval::operator&=(const interval& I)
{
  using namespace neon_detail;
  if (any_lane(vorrq_u64(nan_lanes(neonbounds), nan_lanes(I.neonbounds)))) {
    neonbounds = vdupq_n_f64(std::numeric_limits<double>::quiet_NaN());
    return *this;
  }
  const int64x2_t kx = bound_keys(neonbounds), kI = bound_keys(I.neonbounds);
  const uint64x2_t take = vcltq_s64(kI, kx); // I has the lesser stored bound
  const int64x2_t k = vbslq_s64(take, kI, kx);
  // Disjoint intervals give the empty set, [NaN, NaN] as interval::emptyset(),
  // not the bounds in the wrong order: -l > r
  if (-vgetq_lane_s64(k, 0) > vgetq_lane_s64(k, 1)) {
    neonbounds = vdupq_n_f64(std::numeric_limits<double>::quiet_NaN());
  } else {
    neonbounds = vbslq_f64(take, I.neonbounds, neonbounds);
  }
  return *this;
}

// The hull; the empty operand has the lowest keys, and is left out
GAOL_INLINE interval& interval::operator|=(const interval& I)
{
  using namespace neon_detail;
  neonbounds = vbslq_f64(vcltq_s64(bound_keys_nan_lowest(neonbounds), bound_keys_nan_lowest(I.neonbounds)),
                         I.neonbounds, neonbounds);
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
  They are in gaol_detail, with the other helpers of the library, and not in
  gaol_core, which holds the interface.
*/
} // namespace gaol_core

namespace gaol_detail {
GAOL_NODISCARD extern GAOL_PUBLIC float64x2_t add_bounds(float64x2_t x, float64x2_t y);
GAOL_NODISCARD extern GAOL_PUBLIC float64x2_t sub_bounds(float64x2_t x, float64x2_t y);
GAOL_NODISCARD extern GAOL_PUBLIC float64x2_t mul_bounds(float64x2_t x, float64x2_t y);
GAOL_NODISCARD extern GAOL_PUBLIC float64x2_t div_bounds(float64x2_t x, float64x2_t y);
GAOL_NODISCARD extern GAOL_PUBLIC float64x2_t sqr_bounds(float64x2_t x);
} // namespace gaol_detail

namespace gaol_core {

GAOL_INLINE interval& interval::operator+=(const interval& I)
{
  neonbounds = gaol_detail::add_bounds(neonbounds, I.neonbounds);
  return *this;
}

GAOL_INLINE interval& interval::operator-=(const interval& I)
{
  neonbounds = gaol_detail::sub_bounds(neonbounds, I.neonbounds);
  return *this;
}

GAOL_INLINE interval& interval::operator*=(const interval& I)
{
  neonbounds = gaol_detail::mul_bounds(neonbounds, I.neonbounds);
  return *this;
}

GAOL_INLINE interval& interval::operator/=(const interval& I)
{
  neonbounds = gaol_detail::div_bounds(neonbounds, I.neonbounds);
  return *this;
}

//! I^2
GAOL_NODISCARD GAOL_INLINE interval sqr(const interval& I)
{
  return interval(gaol_detail::sqr_bounds(I.get_neoninterval()), gaol_detail::unchecked_bounds);
}

#endif // GAOL_INTERVAL_NEON_H
