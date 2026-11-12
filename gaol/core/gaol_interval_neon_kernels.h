/*-*-C++-*------------------------------------------------------------------
 * GAOL -- not Just Another Interval Library
 *--------------------------------------------------------------------------
 * Copyright (c) 2026 ENSTA, France
 *
 * By: Jordan NININ (created 2026-10-19)
 *--------------------------------------------------------------------------
 * GAOL is a software distributed WITHOUT ANY WARRANTY. Read the associated
 * COPYING.LIB file for information.
 *--------------------------------------------------------------------------*/

/*!
  \file   gaol_interval_neon_kernels.h
  \brief  The NEON kernels that several sources take inline: the inverse, the square and the
          square root, before and with the check. Included by gaol_interval_internal.h.
*/

#ifndef GAOL_INTERVAL_NEON_KERNELS_H
#define GAOL_INTERVAL_NEON_KERNELS_H

/*
  A header of the sources, not installed, that gaol_interval_internal.h
  includes for the NEON intervals: the bodies of the kernels of
  gaol_interval_neon_impl.h that the sources other than gaol_interval_core.cpp
  call, and the helpers they take. They are inline here, so that the integer
  powers and the roots of gaol_interval_pow.cpp take them inline, as they did
  when every definition was in gaol_interval_core.cpp, and the functions of
  gaol_interval_neon_impl.h are calls of them. upward_access::inverse() and
  upward_access::sqr() are the inverse and the square with the rounding
  direction upward already (inverse_upward() and sqr_upward(), private
  functions of interval before), neon_detail::sqr_bounds_inline() the body
  of gaol_detail::sqr_bounds(), the square with its check,
  upward_access::checked_inverse() the body of inverse(), and
  neon_detail::sqrt_bounds() the vector square root that sqrt_checked() of
  gaol_interval_internal.h takes (see the kernels there). The helpers of the
  division come along with inverse_bounds(), the kernel of the inverse.
*/

namespace gaol_core {

  /*
    The helpers of the kernels below. They work on the stored bounds <-l, r>,
    with the rounding direction upward, as the kernels of the SSE2 intervals do:
    since -RD(p) = RU(-p), one vector operation rounded upward gives the lower
    bound (negated, in the first lane) and the upper one.

    The classification of the bounds is made on their bits, with integer
    instructions: they depend neither on the rounding direction nor on the
    modes that flush the subnormals to zero, which the check of the direction
    clears afterwards, and they raise nothing on a NaN. The comparisons of
    64-bit ARM on doubles, fcmgt and fcmge, signal the invalid-operation
    exception on a quiet NaN, which an empty interval holds in both bounds
    (tests/rounding_direction.cpp), and the compilers turn them into scalar
    comparisons under -frounding-math all the same.
  */
  namespace neon_detail {

  // The bounds read again after the check of the rounding direction: the empty
  // asm statement keeps its place after the check, and gives x back in the
  // register it is in, at no cost, so that no compiler computes with it, or
  // compares it, before the check has set the direction and cleared the
  // modes that flush the subnormals to zero (see rnd_reread() of
  // gaol/gaol_fpu.h, for a double). Visual C++ has no asm statement on ARM64,
  // and keeps the operations where the source puts them with respect to the
  // writes of FPCR (see GAOL_RND_BARRIER() of gaol/gaol_fpu.h): x as it is.
  // It also serves before the check, in the branch of a test on bits that
  // Clang would compute before the branch: an asm statement is not moved above
  // it (the test of NaN of mul_bounds(), GAOL v5)
  GAOL_INLINE float64x2_t reread_bounds(float64x2_t x)
  {
#  if !GAOL_NEON_MSVC
    __asm__ __volatile__ ("" : "+w" (x));
#  endif
    return x;
  }

  // <y1, y0>
  GAOL_INLINE float64x2_t swap_lanes(float64x2_t x) { return vextq_f64(x, x, 1); }
  // <-y0, y1>: the stored bounds with the left one in its natural sign
  GAOL_INLINE float64x2_t negate_first(float64x2_t y)
  {
    return vcombine_f64(vneg_f64(vget_low_f64(y)), vget_high_f64(y));
  }
  GAOL_INLINE float64x2_t nan_pair() { return vdupq_n_f64(std::numeric_limits<double>::quiet_NaN()); }
  GAOL_INLINE float64x2_t stored(double minus_left, double right)
  {
    return vcombine_f64(vdup_n_f64(minus_left), vdup_n_f64(right));
  }

  /*
    The tests on the bounds of the divisor of 1 / y (GAOL v5) are made on the bits of
    its two bounds, in general registers, with integer instructions only: nothing is compared as a double,
    which would raise an exception on a NaN
  */
  // The 64 bits of the lane 0 and of the lane 1 of x in a general register, by umov in an asm statement: Clang
  // makes vector operations (a vector comparison, then dup and fmov) of the integer tests that use
  // vgetq_lane_u64(). Visual C++ has no asm statement
  GAOL_INLINE std::uint64_t low_bits(float64x2_t x)
  {
#  if GAOL_NEON_MSVC
    return vgetq_lane_u64(bits_of(x), 0);
#  else
    std::uint64_t bits;
    __asm__ ("umov %0, %1.d[0]" : "=r" (bits) : "w" (x));
    return bits;
#  endif
  }
  GAOL_INLINE std::uint64_t high_bits(float64x2_t x)
  {
#  if GAOL_NEON_MSVC
    return vgetq_lane_u64(bits_of(x), 1);
#  else
    std::uint64_t bits;
    __asm__ ("umov %0, %1.d[1]" : "=r" (bits) : "w" (x));
    return bits;
#  endif
  }

  // x in the register it is in: the compiler cannot split a test on x into branches on its parts, whose
  // outcome is random in a program (the signs of the bounds of a divisor), nor make vector operations of it
  GAOL_INLINE void keep_in_register(std::uint64_t& x)
  {
#  if !GAOL_NEON_MSVC
    __asm__ ("" : "+r" (x));
#  else
    (void)x;
#  endif
  }

  // 1 if the divisor y = <-c, d> has no zero, 0 if it has one: c > 0 (-c is negative and no zero) or d < 0.
  // A negative double that is no zero has bits above those of -0, the sign bit alone, as an unsigned
  // integer. The only branch of a quotient on y is on the result: none on the sign of each bound
  GAOL_INLINE std::uint64_t divisor_without_zero(std::uint64_t y_low_bits, std::uint64_t y_high_bits)
  {
    const std::uint64_t minus_zero_bits = 0x8000000000000000ull;
    std::uint64_t no_zero = static_cast<std::uint64_t>((y_low_bits > minus_zero_bits) | (y_high_bits > minus_zero_bits));
    keep_in_register(no_zero);
    return no_zero;
  }

  } // namespace neon_detail

} // namespace gaol_core

namespace gaol_core {

  namespace neon_detail {

  /*
    The two products and the maximum of sqr_bounds_inline() below on the stored
    bounds x of a non-empty interval, the rounding direction being upward and
    the modes that flush the subnormals to zero clear: the body of
    sqr_bounds_inline() after its check, which upward_access::sqr() takes for
    x^2 (GAOL v5)
  */
  GAOL_INLINE float64x2_t sqr_products(float64x2_t x)
  {
    const float64x2_t q = vmulq_f64(x, vnegq_f64(x));
    const float64x2_t p = vmulq_f64(x, x);
    float64x2_t res = vpmaxq_f64(q, p);
    // The stored bounds that are negative: -l < 0 is l > 0, r < 0 is r < 0.
    // Neither or only the first: the interval straddles zero or is positive
    const uint64x2_t sign = vcltzq_s64(vreinterpretq_s64_f64(x));
    const uint64x2_t any_negative = vorrq_u64(sign, vextq_u64(sign, sign, 1));
    res = from_bits(vandq_u64(bits_of(res), vcombine_u64(vget_low_u64(any_negative), vdup_n_u64(~0ull))));
    return res;
  }

  } // namespace neon_detail

} // namespace gaol_core

namespace gaol_core {

  namespace neon_detail {

  /*
    The quotient of x by a y that contains zero, c <= 0 <= d, neither empty (GAOL
    v5): the cases of the signs of x and of the bounds of y that are zero, as
    the quotient of the FPU intervals (gaol_interval_fpu_impl.h), each with the one
    division it needs, of two numbers that are no zero, so that none raises
    the division-by-zero or the invalid-operation exception (the divisor
    goes through read_in_branch(): Clang for 64-bit ARM computes the
    divisions of both sides of a test and chooses afterwards). Called with the
    rounding direction upward, on bounds read after its check.
    modulo is the "extended" quotient of operator%=(): where [0] / [0] is
    empty and x / y is empty for a y of zeros, x % y is the whole line
    wherever x contains zero.

    With x = [l, r] stored <a, b> = <-l, r> and y = [c, d] stored <p, q> =
    <-c, d>:
                  y = [0]   N0: c < 0 = d   P0: c = 0 < d   M: c < 0 < d
      x N1 r < 0   empty    [r/c, +oo]       [-oo, r/d]      universe
      x N0 l<0=r   empty    [0, +oo]         [-oo, 0]        universe
      x Z          empty    [0]              [0]             [0]
      x M          empty    universe         universe        universe
      x P0 l=0<r   empty    [-oo, 0]         [0, +oo]        universe
      x P1 l > 0   empty    [-oo, l/c]       [l/d, +oo]      universe
    where x % y is empty for [0] only when x is N1 or P1, and the whole line
    in the rows N0, Z, M and P0 of every column
  */
  GAOL_INLINE float64x2_t divide_by_zero_containing(float64x2_t x, float64x2_t y, bool modulo)
  {
    const double a = vgetq_lane_f64(x, 0), b = vgetq_lane_f64(x, 1); // -l, r
    const double p = vgetq_lane_f64(y, 0), q = vgetq_lane_f64(y, 1); // -c, d
    const double inf = std::numeric_limits<double>::infinity();
    const float64x2_t empty = nan_pair();
    const float64x2_t universe = vdupq_n_f64(inf);
    // x: N1 (r < 0), N0 (l < 0 = r), Z, M (l < 0 < r), P0 (l = 0 < r), P1 (l > 0)
    enum { N1, N0, Z, M, P0, P1 } xs;
    if (b < 0.0) {
      xs = N1;
    } else if (b == 0.0) {
      xs = (a == 0.0) ? Z : N0;
    } else if (a > 0.0) {
      xs = M;
    } else {
      xs = (a == 0.0) ? P0 : P1;
    }
    const bool c_zero = (p == 0.0), d_zero = (q == 0.0);
    if (c_zero && d_zero) { // y = [0]
      if (!modulo || xs == N1 || xs == P1) {
        return empty;
      }
      return universe;
    }
    if (xs == Z) {
      return modulo ? universe : vdupq_n_f64(0.0);
    }
    if (xs == M || (modulo && (xs == N0 || xs == P0)) || (!c_zero && !d_zero)) { // x M, or y M
      return universe;
    }
    if (d_zero) { // y = [c, 0] with c < 0
      switch (xs) {
      case N1: return stored(b / read_in_branch(p), inf);
      case P1: return stored(inf, a / read_in_branch(p));
      case N0: return stored(-0.0, inf); // [0, +oo]
      default: return stored(inf, 0.0);  // P0: [-oo, 0]
      }
    }
    // y = [0, d] with d > 0
    switch (xs) {
    case N1: return stored(inf, b / read_in_branch(q));
    case P1: return stored(a / read_in_branch(q), inf);
    case N0: return stored(inf, 0.0);  // [-oo, 0]
    default: return stored(0.0, inf);  // P0: [0, +oo]
    }
  }

  /*
    The quotient of the stored bounds of two intervals that are no empty set,
    with the rounding direction upward, on bounds read after its check:
    without a zero in y, one division of the bounds of x, chosen by
    their signs, by the bounds of y (x / y = (-x) / (-y) when y is negative),
    where the divisions of the SSE2 intervals and of the former code
    branched on the nine cases of the signs. With a zero in y, the cases
    above.

    With y = <-c, d> without a zero, c > 0 or d < 0; negative d: x and y are
    exchanged (swap and negation of both bounds) to have y = <-c', d'> with
    0 < c' <= d'. The lower bound of the quotient is -(l / c') for l < 0 and -(l /
    d') for l >= 0, the upper one r / c' for r >= 0 and r / d' for r < 0:
    the lane of the denominator is chosen by the sign of the stored bound of
    x, and the quotient rounded upward is the stored bound in both lanes.
    The bounds of x that are infinite are divided by the finite c' or d',
    never by an infinity, and a zero bound by a nonzero denominator: no
    0 / 0, no oo / oo.
  */
  GAOL_INLINE float64x2_t divide_upward(float64x2_t x, float64x2_t y, bool modulo)
  {
    // The signs of the stored bounds of y, told on the integer side (the bits of the two lanes in
    // general registers), where two reductions and the broadcast of a lane cost vector operations:
    // x / y took 5.35 ns, and now 3.96, with Clang 18 on a Neoverse N2 (GAOL v5)
    const std::uint64_t y_low_bits = vgetq_lane_u64(bits_of(y), 0), y_high_bits = vgetq_lane_u64(bits_of(y), 1);
    // lane 0: c > 0 (-c < 0, not a zero), lane 1: d < 0; with bitwise operators, so that the only
    // branch is on whether y holds a zero, none on the sign of each bound (random in a program)
    const std::uint64_t c_positive = (y_low_bits >> 63) & static_cast<std::uint64_t>((y_low_bits << 1) != 0);
    const std::uint64_t d_below_zero = (y_high_bits >> 63) & static_cast<std::uint64_t>((y_high_bits << 1) != 0);
    if ((c_positive | d_below_zero) == 0) {
      return divide_by_zero_containing(x, y, modulo);
    }
    const uint64x2_t d_negative = vdupq_n_u64(0ull - (y_high_bits >> 63)); // all ones if the sign of d is set
    const float64x2_t xp = vbslq_f64(d_negative, swap_lanes(x), x);
    const float64x2_t yp = vbslq_f64(d_negative, swap_lanes(y), y); // <-c', d'>
    const float64x2_t dd = vdupq_laneq_f64(yp, 1);                   // <d', d'>
    const float64x2_t cc = vnegq_f64(vdupq_laneq_f64(yp, 0));        // <c', c'>
    // The denominator of each lane: the larger bound d' where the stored bound of
    // x is negative (its left bound l >= 0), c' where it is not
    const uint64x2_t x_negative = vcltzq_s64(vreinterpretq_s64_f64(xp));
    return vdivq_f64(xp, vbslq_f64(x_negative, dd, cc));
  }

  /*
    1 / y of the stored bounds <-c, d> of a y that is no empty set, with the rounding direction upward, on
    bounds read after its check (GAOL v5). For a y without a zero, the quotient of [1], stored
    <-1, 1>, by y of divide_upward() is -1 / d and 1 / -(-c) for the sign of d clear, 1 / -d and -1 / (-c)
    for a negative d: the numerator being a constant, each of the two bounds is the quotient of -1 by one of
    the stored bounds of y, -1 / d for the lower bound and -1 / (-c) for the upper one, whichever the sign of d, and the
    same quotient however it is written (a division gives the correctly rounded value, the exceptions and the
    sign of its exact quotient: 1 / -v is -1 / v, the sign of a zero included). So one division
    of -1 by the stored bounds exchanged, <d, -c>, in both lanes, where divide_upward() selects the
    lanes of x and of y and the denominator of each lane. A y with a zero, told as in divide_upward(),
    takes the cases of divide_by_zero_containing()
  */
  GAOL_INLINE float64x2_t inverse_bounds(float64x2_t y)
  {
    if (divisor_without_zero(low_bits(y), high_bits(y)) == 0) {
      return divide_by_zero_containing(stored(-1.0, 1.0), y, false);
    }
    return vdivq_f64(vdupq_n_f64(-1.0), swap_lanes(y));
  }

  } // namespace neon_detail

} // namespace gaol_core

namespace gaol_core {

  namespace neon_detail {

  /*
    The square from two products and a maximum, no branch but the one of the
    empty set (GAOL v5), as sqr_bounds() of the SSE2 intervals: with x = <a,
    b> = <-l, r>, q = <a*(-a), b*(-b)> and p = <a*a, b*b>, the pairwise
    maxima <max(q0, q1), max(p0, p1)> are <-l*l or -r*r, the larger of l*l and
    r*r>, whatever the signs: <-RD(l^2), RU(r^2)> for l >= 0, <-RD(r^2),
    RU(l^2)> for r <= 0 and, for an interval straddling zero, the lower bound
    -min(l^2, r^2) is wrong, and set to +0 by a mask taken from the signs of
    the stored bounds. All the zero products of q are -0 and those of p are
    +0, so that the pairwise maximum, which ranks +0 above -0, is the
    same as the maximum of the SSE2 kernel, which takes its second operand
    for equal zeros, bit for bit. Only l*l and r*r are formed: 0 * oo never
    occurs, and [-oo, 3] gives [0, +oo]. The body of gaol_detail::sqr_bounds()
    (gaol_interval_neon_impl.h), which calls it, and the square that x^2 of
    gaol_pown() takes (upward_access::checked_sqr())
  */
  GAOL_INLINE float64x2_t sqr_bounds_inline(float64x2_t x)
  {
    if (any_lane(nan_lanes(x))) { // the empty set, as it is
      return x;
    }
    GAOL_RND_ENTER();
    x = reread_bounds(x);
    float64x2_t res = sqr_products(x);
    GAOL_RND_KEEP(res);
    GAOL_RND_LEAVE();
    return res;
  }

  } // namespace neon_detail

  namespace neon_detail {

  /*
    The square root of the part of x in [0, +oo], the empty set for an empty x
    and for an upper bound below 0 (GAOL v5), as sqrt() computes it from the
    bounds at doubles: both roots by one vector square root rounded upward,
    the lower one, in the first lane, being the root rounded downward. This
    one is the root when u * u is exactly the argument v, and else the double
    below the root u rounded upward, whose bits are those of u less one (u is
    a positive normal number, or 0, or +oo, for which u * u is v). The part
    of x in [0, +oo] is taken with the sign bits, not a maximum with +0: a
    bound whose sign bit is set, negative or -0, becomes +0, whose root is +0.
    The maximum of 64-bit ARM ranks +0 above -0, but with FPCR.AH (FEAT_AFP,
    Armv8.7), which GAOL does not clear, FMAX returns its second operand for
    two zeros of different signs, and the operands of vmaxq_f64() may be
    exchanged by a compiler (Clang takes it for commutative): the maximum
    would have given the empty set for [-0, r]. Neither the empty set nor
    the sign of the upper bound is told with a comparison of doubles: the
    first is a NaN in the bits, the second the sign bit of a bound that is
    not a zero.
  */
  GAOL_INLINE float64x2_t sqrt_bounds(float64x2_t x)
  {
    // The check of the rounding direction first, whatever x is: sqrt() of the
    // other intervals starts with it, and an operation clears the modes that
    // flush the subnormals to zero even for an argument that gives the empty set
    // (tests/rounding_direction.cpp)
    GAOL_RND_ENTER();
    x = reread_bounds(x);
    const std::uint64_t right_bits = vgetq_lane_u64(bits_of(x), 1);
    float64x2_t res;
    if (any_lane(nan_lanes(x)) || ((right_bits >> 63) != 0 && (right_bits << 1) != 0)) { // empty, or an upper bound below 0
      res = nan_pair();
    } else {
      // <max(l, 0), max(r, 0)>: the lanes with the sign bit set are cleared. Neither lane
      // is a NaN here (told above), which the mask would turn into +0, and the upper bound
      // is no negative number but -0
      const float64x2_t natural = negate_first(x);
      const uint64x2_t negative = vcltzq_s64(vreinterpretq_s64_f64(natural));
      const float64x2_t v = from_bits(vbicq_u64(bits_of(natural), negative));
      const float64x2_t u = vsqrtq_f64(v);
      const uint64x2_t exact = vceqq_u64(bits_of(vmulq_f64(u, u)), bits_of(v));
      const float64x2_t below = from_bits(vsubq_u64(bits_of(u), vdupq_n_u64(1)));
      const float64x2_t low = vbslq_f64(exact, u, below);
      res = vcombine_f64(vneg_f64(vget_low_f64(low)), vget_high_f64(u));
    }
    GAOL_RND_KEEP(res);
    GAOL_RND_LEAVE();
    return res;
  }

  } // namespace neon_detail

  // 1 / I: the quotient of [1] by I (inverse_bounds()), the empty
  // set for [0] included; the rounding direction is upward, as for the FPU
  // intervals (see gaol_interval_core.h)
  inline interval upward_access::inverse(const interval& I)
  {
    // The callers tell the empty set beforehand; a NaN bound would otherwise
    // be told a divisor with neither a zero nor a sign, and give the universe
    if (neon_detail::any_lane(neon_detail::nan_lanes(I.neonbounds))) {
      return interval::emptyset();
    }
    return interval(neon_detail::inverse_bounds(I.neonbounds), gaol_detail::unchecked_bounds);
  }

  // sqr() of a non-empty I, the rounding direction being upward already (see
  // gaol_interval_core.h): the square that pow() takes for x^2 after its own
  // check (GAOL v5)
  inline interval upward_access::sqr(const interval& I)
  {
    return interval(neon_detail::sqr_products(I.neonbounds), gaol_detail::unchecked_bounds);
  }

  // sqr() of I, the empty set or not, with its check: x^2 of gaol_pown() (GAOL v5)
  inline interval upward_access::checked_sqr(const interval& I)
  {
    return interval(neon_detail::sqr_bounds_inline(I.neonbounds), gaol_detail::unchecked_bounds);
  }

  // inverse() of I, the empty set or not, with its check: its body, and
  // 1/sqrt(x) of nth_root(x, -2) (GAOL v5)
  inline interval upward_access::checked_inverse(const interval& I)
  {
    // is_empty() is true for a NaN in a bound, the comparison it makes being false: no
    // empty set is left below, which a reduction as in upward_access::inverse() would tell again
    if (I.is_empty()) {
      return interval::emptyset();
    }

    GAOL_RND_ENTER();
    interval res(neon_detail::inverse_bounds(neon_detail::reread_bounds(I.neonbounds)), gaol_detail::unchecked_bounds);
    GAOL_RND_KEEP(res);
    GAOL_RND_LEAVE();
    return res;
  }

} // namespace gaol_core

#endif /* GAOL_INTERVAL_NEON_KERNELS_H */
