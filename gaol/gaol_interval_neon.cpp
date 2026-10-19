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
    This file is not meant to be compiled. It is only to be included
        by gaol/gaol_interval_core.cpp.
    The intervals of 64-bit ARM processors, whose two stored bounds <-left,
    right> are the two lanes of a float64x2_t (GAOL v5): the counterpart of
    gaol_interval_sse.cpp, with the NEON instructions.
*/

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

  const std::uint64_t abs_mask = 0x7fffffffffffffffull;
  const std::uint64_t infinity_bits = 0x7ff0000000000000ull;

  // The bounds read again after the check of the rounding direction: the empty
  // asm statement keeps its place after the check, and gives x back in the
  // register it is in, at no cost, so that no compiler computes with it, or
  // compares it, before the check has set the direction and cleared the
  // modes that flush the subnormals to zero (see rnd_reread() of
  // gaol/gaol_fpu.h, for a double)
  GAOL_INLINE float64x2_t reread_bounds(float64x2_t x)
  {
    __asm__ __volatile__ ("" : "+w" (x));
    return x;
  }

  GAOL_INLINE uint64x2_t bits_of(float64x2_t x) { return vreinterpretq_u64_f64(x); }
  GAOL_INLINE float64x2_t from_bits(uint64x2_t x) { return vreinterpretq_f64_u64(x); }
  // <y1, y0>
  GAOL_INLINE float64x2_t swap_lanes(float64x2_t x) { return vextq_f64(x, x, 1); }
  // <-y0, y1>: the stored bounds with the left one in its natural sign
  GAOL_INLINE float64x2_t negate_first(float64x2_t y)
  {
    return vcombine_f64(vneg_f64(vget_low_f64(y)), vget_high_f64(y));
  }
  GAOL_INLINE uint64x2_t abs_bits(float64x2_t x) { return vandq_u64(bits_of(x), vdupq_n_u64(abs_mask)); }
  // All ones in the lanes that hold a NaN or an infinity
  GAOL_INLINE uint64x2_t nonfinite_lanes(float64x2_t x) { return vcgeq_u64(abs_bits(x), vdupq_n_u64(infinity_bits)); }
  // All ones in the lanes that hold a NaN
  GAOL_INLINE uint64x2_t nan_lanes(float64x2_t x) { return vcgtq_u64(abs_bits(x), vdupq_n_u64(infinity_bits)); }
  GAOL_INLINE bool any_lane(uint64x2_t m) { return vmaxvq_u32(vreinterpretq_u32_u64(m)) != 0; }
  GAOL_INLINE float64x2_t nan_pair() { return vdupq_n_f64(std::numeric_limits<double>::quiet_NaN()); }
  GAOL_INLINE float64x2_t stored(double minus_left, double right)
  {
    return vcombine_f64(vdup_n_f64(minus_left), vdup_n_f64(right));
  }

  } // namespace neon_detail

  /*
    The sum, on the stored bounds: <-l1, r1> + <-l2, r2> = <-(l1 + l2), r1 +
    r2>. An empty operand has NaN in both lanes, which the addition carries
    without raising anything. The infinite bounds are all +oo once stored (a
    stored -oo would be [+oo, ..] or [.., -oo], which the constructors refuse),
    so that inf - inf is never formed.
  */
  float64x2_t add_bounds(float64x2_t x, float64x2_t y)
  {
    using namespace neon_detail;
    GAOL_RND_ENTER();
    const float64x2_t xx = reread_bounds(x), yy = reread_bounds(y);
    float64x2_t res = vaddq_f64(xx, yy);
    GAOL_RND_KEEP(res);
    GAOL_RND_LEAVE();
    return res;
  }

  // x - y is x + [-r, -l], the bounds of y exchanged
  float64x2_t sub_bounds(float64x2_t x, float64x2_t y)
  {
    using namespace neon_detail;
    GAOL_RND_ENTER();
    const float64x2_t xx = reread_bounds(x), yy = reread_bounds(y);
    float64x2_t res = vaddq_f64(xx, swap_lanes(yy));
    GAOL_RND_KEEP(res);
    GAOL_RND_LEAVE();
    return res;
  }

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
    occurs, and [-oo, 3] gives [0, +oo].
  */
  float64x2_t sqr_bounds(float64x2_t x)
  {
    using namespace neon_detail;
    if (any_lane(nan_lanes(x))) { // the empty set, as it is
      return x;
    }
    GAOL_RND_ENTER();
    x = reread_bounds(x);
    const float64x2_t q = vmulq_f64(x, vnegq_f64(x));
    const float64x2_t p = vmulq_f64(x, x);
    float64x2_t res = vpmaxq_f64(q, p);
    // The stored bounds that are negative: -l < 0 is l > 0, r < 0 is r < 0.
    // Neither or only the first: the interval straddles zero or is positive
    const uint64x2_t sign = vcltzq_s64(vreinterpretq_s64_f64(x));
    const uint64x2_t any_negative = vorrq_u64(sign, vextq_u64(sign, sign, 1));
    res = from_bits(vandq_u64(bits_of(res), vcombine_u64(vget_low_u64(any_negative), vdup_n_u64(~0ull))));
    GAOL_RND_KEEP(res);
    GAOL_RND_LEAVE();
    return res;
  }

  namespace neon_detail {

  /*
    u * v for the four products of the product of two intervals, where a
    factor is 0 and the other infinite: both are set to +0, so that the product
    is +0 and raises no invalid-operation exception (0 * oo is NaN, and
    [0, 1] * [+oo, +oo] is [0, +oo]), as products_with_infinities() of the SSE2
    intervals does. Integer instructions only until the product
  */
  GAOL_INLINE float64x2_t guarded_products(float64x2_t u, float64x2_t v)
  {
    const uint64x2_t au = abs_bits(u), av = abs_bits(v);
    const uint64x2_t u_zero = vceqzq_u64(au), v_zero = vceqzq_u64(av);
    const uint64x2_t u_inf = vceqq_u64(au, vdupq_n_u64(infinity_bits));
    const uint64x2_t v_inf = vceqq_u64(av, vdupq_n_u64(infinity_bits));
    const uint64x2_t mask = vorrq_u64(vandq_u64(u_zero, v_inf), vandq_u64(v_zero, u_inf));
    return vmulq_f64(from_bits(vbicq_u64(bits_of(u), mask)), from_bits(vbicq_u64(bits_of(v), mask)));
  }

  } // namespace neon_detail

  /*
    The product: the four products of the corners, taken lane by lane, and
    their maximum (GAOL v5). With x = <-l1, r1> and y = <-l2, r2>, yc = <l2,
    r2>, yd = <r2, l2> and xo = <-r1, l1>:
      x * yc  = <-l1*l2, r1*r2>
      x * yd  = <-l1*r2, r1*l2>
      xo * yc = <-r1*l2, l1*r2>
      xo * yd = <-r1*r2, l1*l2>
    The first lane of the maximum is the negated minimum of the four corner
    products, the second one the maximum: the lower and upper bounds of the
    product, whatever the signs. The maxima commute with the rounding
    upward, which is monotone, so each product is rounded once, upward, and
    the negated lower bound is rounded downward as it should. The nine cases
    of the signs, and the branches that predicted them badly, are gone.

    A bound that is NaN, or infinite, takes the slower path of
    guarded_products(): the empty set, which has NaN in both lanes, makes
    the empty set; a zero times an infinity is zero, whatever the signs. The
    test is one unsigned comparison of the bits. The maximum of 64-bit ARM
    ranks +0 above -0 and x86 maxpd returns its second operand: where the
    two lanes of a pair of zeros differ in sign, the sign of a zero bound
    may differ from the one of the SSE2 intervals (GAOL v5, like the AVX-512
    path; the values are equal).
  */
  float64x2_t mul_bounds(float64x2_t x, float64x2_t y)
  {
    using namespace neon_detail;
    const uint64x2_t nonfinite = vorrq_u64(nonfinite_lanes(x), nonfinite_lanes(y));
    const bool special = any_lane(nonfinite);
    if (special && any_lane(vorrq_u64(nan_lanes(x), nan_lanes(y)))) { // an empty operand
      return nan_pair();
    }
    GAOL_RND_ENTER();
    x = reread_bounds(x);
    y = reread_bounds(y);
    const float64x2_t yc = negate_first(y);
    const float64x2_t yd = swap_lanes(yc);
    const float64x2_t xo = vnegq_f64(swap_lanes(x));
    float64x2_t res;
    if (special) {
      res = vmaxq_f64(vmaxq_f64(guarded_products(x, yc), guarded_products(x, yd)),
                      vmaxq_f64(guarded_products(xo, yc), guarded_products(xo, yd)));
    } else {
      res = vmaxq_f64(vmaxq_f64(vmulq_f64(x, yc), vmulq_f64(x, yd)),
                      vmaxq_f64(vmulq_f64(xo, yc), vmulq_f64(xo, yd)));
    }
    GAOL_RND_KEEP(res);
    GAOL_RND_LEAVE();
    return res;
  }

  namespace neon_detail {

  /*
    The quotient of x by a y that contains zero, c <= 0 <= d, neither empty (GAOL
    v5): the cases of the signs of x and of the bounds of y that are zero, as
    the quotient of the FPU intervals (gaol_interval_fpu.cpp), each with the one
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
    const uint64x2_t y_negative = vcltzq_s64(vreinterpretq_s64_f64(y));
    const uint64x2_t y_nonzero = vtstq_u64(bits_of(y), vdupq_n_u64(abs_mask));
    // lane 0: c > 0 (-c < 0, not a zero), lane 1: d < 0
    if (!any_lane(vandq_u64(y_negative, y_nonzero))) {
      return divide_by_zero_containing(x, y, modulo);
    }
    const uint64x2_t d_negative = vdupq_laneq_u64(y_negative, 1);
    const float64x2_t xp = vbslq_f64(d_negative, swap_lanes(x), x);
    const float64x2_t yp = vbslq_f64(d_negative, swap_lanes(y), y); // <-c', d'>
    const float64x2_t dd = vdupq_laneq_f64(yp, 1);                   // <d', d'>
    const float64x2_t cc = vnegq_f64(vdupq_laneq_f64(yp, 0));        // <c', c'>
    // The denominator of each lane: the larger bound d' where the stored bound of
    // x is not negative, c' where it is
    const uint64x2_t x_negative = vcltzq_s64(vreinterpretq_s64_f64(xp));
    return vdivq_f64(xp, vbslq_f64(x_negative, dd, cc));
  }

  } // namespace neon_detail

  float64x2_t div_bounds(float64x2_t x, float64x2_t y)
  {
    using namespace neon_detail;
    if (any_lane(vorrq_u64(nan_lanes(x), nan_lanes(y)))) { // an empty operand
      return nan_pair();
    }
    GAOL_RND_ENTER();
    x = reread_bounds(x);
    y = reread_bounds(y);
    float64x2_t res = divide_upward(x, y, false);
    GAOL_RND_KEEP(res);
    GAOL_RND_LEAVE();
    return res;
  }

  namespace neon_detail {

  /*
    The square root of the part of x in [0, +oo], the empty set for an empty x
    and for an upper bound below 0 (GAOL v5), as sqrt() computes it from the
    bounds at doubles: both roots by one vector square root rounded upward,
    the lower one, in the first lane, being the root rounded downward. This
    one is the root when u * u is exactly the argument v, and else the double
    below the root u rounded upward, whose bits are those of u less one (u is
    a positive normal number, or 0, or +oo, for which u * u is v). An argument
    -0, which the maximum with +0 makes +0 (it ranks +0 above -0), is a root +0.
    Neither the empty set nor the sign of the upper bound is told with a
    comparison of doubles: the first is a NaN in the bits, the second the
    sign bit of a bound that is not a zero.
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
      const float64x2_t v = vmaxq_f64(negate_first(x), vdupq_n_f64(0.0)); // <max(l, 0), max(r, 0)>
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

  interval& interval::operator%=(const interval& I)
  {
    using namespace neon_detail;
    if (any_lane(vorrq_u64(nan_lanes(neonbounds), nan_lanes(I.neonbounds)))) { // an empty operand
      neonbounds = nan_pair();
      return *this;
    }
    GAOL_RND_ENTER();
    const float64x2_t x = reread_bounds(neonbounds), y = reread_bounds(I.neonbounds);
    float64x2_t res = divide_upward(x, y, true);
    GAOL_RND_KEEP(res);
    GAOL_RND_LEAVE();
    neonbounds = res;
    return *this;
  }

  // 1 / I: the quotient of [1] by I, which the cases above give, the empty
  // set for [0] included; the rounding direction is upward, as for the FPU
  // intervals (see gaol_interval_core.h)
  interval interval::inverse_upward(const interval& I)
  {
    // The callers tell the empty set beforehand; a NaN bound would otherwise
    // be told a divisor with neither a zero nor a sign, and give the universe
    if (neon_detail::any_lane(neon_detail::nan_lanes(I.neonbounds))) {
      return interval::emptyset();
    }
    return interval(neon_detail::divide_upward(neon_detail::stored(-1.0, 1.0), I.neonbounds, false));
  }

  interval interval::inverse() const
  {
    if (is_empty()) {
      return interval::emptyset();
    }

    GAOL_RND_ENTER();
    interval res = inverse_upward(interval(neon_detail::reread_bounds(neonbounds)));
    GAOL_RND_KEEP(res);
    GAOL_RND_LEAVE();
    return res;
  }

  /*
    The operations with a double. The empty interval and a d that is
    infinite or a NaN, for which interval(d) is empty, give the empty set,
    told with quiet comparisons. Both lanes are computed, <-l, r> + <-d, d>
    for the sum, and the product or the quotient by |d| of the bounds,
    exchanged for a negative d (GAOL v5, as the SSE2 intervals). The bounds
    are read after the check of the direction (reread_bounds()), as are d
    (rnd_reread()) before it is compared, so that the modes that flush the
    subnormals to zero, which the check clears, cannot read a subnormal d as 0.
  */
  interval& interval::operator+=(double d)
  {
    if (is_empty() || !(gaol_detail::quiet_less(-GAOL_INFINITY, d) && gaol_detail::quiet_less(d, GAOL_INFINITY))) { // interval(d) is empty for an infinite d and for a NaN
      *this = interval::emptyset();
      return *this;
    }
    GAOL_RND_ENTER();
    const float64x2_t x = neon_detail::reread_bounds(neonbounds);
    neonbounds = vaddq_f64(x, neon_detail::stored(-d, d)); // <-l, r> + <-d, d>
    GAOL_RND_LEAVE();
    return *this;
  }

  interval& interval::operator-=(double d)
  {
    if (is_empty() || !(gaol_detail::quiet_less(-GAOL_INFINITY, d) && gaol_detail::quiet_less(d, GAOL_INFINITY))) { // interval(d) is empty for an infinite d and for a NaN
      *this = interval::emptyset();
      return *this;
    }
    GAOL_RND_ENTER();
    const float64x2_t x = neon_detail::reread_bounds(neonbounds);
    neonbounds = vaddq_f64(x, neon_detail::stored(d, -d)); // <-l, r> + <d, -d>
    GAOL_RND_LEAVE();
    return *this;
  }

  interval& interval::operator*=(double d)
  {
    if (is_empty() || !(gaol_detail::quiet_less(-GAOL_INFINITY, d) && gaol_detail::quiet_less(d, GAOL_INFINITY))) { // interval(d) is empty for an infinite d and for a NaN
      *this = interval::emptyset();
      return *this;
    }
    GAOL_RND_ENTER();
    d = rnd_reread(d);
    if (d == 0.0) {
      neonbounds = vdupq_n_f64(0.0);
      GAOL_RND_LEAVE();
      return *this;
    }
    const float64x2_t x = neon_detail::reread_bounds(neonbounds);
    if (d > 0.0) {
      neonbounds = vmulq_n_f64(x, d); // <-l*d, r*d>
    } else { // d < 0.0
      neonbounds = vmulq_n_f64(neon_detail::swap_lanes(x), -d); // <r*(-d), -l*(-d)>
    }
    GAOL_RND_LEAVE();
    return *this;
  }

  interval& interval::operator/=(double d)
  {
    if (is_empty() || !(gaol_detail::quiet_less(-GAOL_INFINITY, d) && gaol_detail::quiet_less(d, GAOL_INFINITY))) { // interval(d) is empty for an infinite d and for a NaN
      *this = interval::emptyset();
      return *this;
    }
    GAOL_RND_ENTER();
    d = rnd_reread(d);
    if (d == 0.0) {
      *this = interval::emptyset();
      GAOL_RND_LEAVE();
      return *this;
    }
    const float64x2_t x = neon_detail::reread_bounds(neonbounds);
    if (d > 0.0) {
      neonbounds = vdivq_f64(x, vdupq_n_f64(d)); // <-l/d, r/d>
    } else { // d < 0.0
      neonbounds = vdivq_f64(neon_detail::swap_lanes(x), vdupq_n_f64(-d)); // <r/(-d), -l/(-d)>
    }
    GAOL_RND_LEAVE();
    return *this;
  }

  interval& interval::operator%=(double d)
  {
    if (is_empty() || !(gaol_detail::quiet_less(-GAOL_INFINITY, d) && gaol_detail::quiet_less(d, GAOL_INFINITY))) { // interval(d) is empty for an infinite d and for a NaN
      *this = interval::emptyset();
      return *this;
    }
    GAOL_RND_ENTER();
    d = rnd_reread(d);
    if (d == 0.0) { // x*0 is in [this] for any x when [this] contains 0
      *this = straddles_zero() ? interval::universe() : interval::emptyset();
      GAOL_RND_LEAVE();
      return *this;
    }
    const float64x2_t x = neon_detail::reread_bounds(neonbounds);
    if (d > 0.0) {
      neonbounds = vdivq_f64(x, vdupq_n_f64(d)); // <-l/d, r/d>
    } else { // d < 0.0
      neonbounds = vdivq_f64(neon_detail::swap_lanes(x), vdupq_n_f64(-d)); // <r/(-d), -l/(-d)>
    }
    GAOL_RND_LEAVE();
    return *this;
  }


double hausdorff(const interval &I1, const interval &I2)
{
	if (I1.is_empty() || I2.is_empty()) {
	return GAOL_NAN;
	}
	// A bound infinite in one interval only puts them at distance +oo, returned
	// before the check of the rounding direction (GAOL v5, point R.50 of
	// TODO.md; GAOL 4 returned +oo there for every infinite bound): the check
	// and the differences made it two to four times as long (Intel i7-1185G7,
	// GCC 9.4 and Clang 18). Whether a bound is infinite depends neither on the
	// rounding direction nor on the flush-to-zero modes, which read a subnormal
	// bound as a zero, no infinity either. The bounds compared after the check
	// are read after it (GAOL_RND_BARRIER() of gaol/gaol_fpu.h, and /fp:strict
	// with Visual C++)
	if ((std::fabs(I1.left()) == GAOL_INFINITY) != (std::fabs(I2.left()) == GAOL_INFINITY)
		|| (std::fabs(I1.right()) == GAOL_INFINITY) != (std::fabs(I2.right()) == GAOL_INFINITY)) {
	return GAOL_INFINITY;
	}
	// The tightest upper bound of the distance, whatever the rounding direction
	// of the caller: each difference is rounded upward both ways.
	// Equal bounds, infinite ones included, are at distance 0 (inf - inf is a
	// NaN): the distance of [1, +oo] to [1, +oo] is 0 and the one to [2, +oo] is
	// 1, where GAOL 4 returned +oo.
	GAOL_RND_ENTER();
	const double a = I1.left(), b = I1.right(), c = I2.left(), e = I2.right();
	double d = maximum((a == c) ? 0.0 : maximum(a - c, c - a),
					(b == e) ? 0.0 : maximum(b - e, e - b));
	GAOL_RND_KEEP(d);
	GAOL_RND_LEAVE();
	return d;
}


  interval interval::mid() const
  {
    if (is_empty()) {
      return interval::emptyset();
    }
    // The check before the bounds are compared (see gaol/gaol_fpu.h)
    GAOL_RND_ENTER();
    if (is_symmetric()) { // symmetric case handles [-oo, +oo]
      GAOL_RND_LEAVE();
      return interval(0.0);
    }
    if (left() == -GAOL_INFINITY) {
      GAOL_RND_LEAVE();
      return interval(-std::numeric_limits<double>::max());
    }
    if (right() == GAOL_INFINITY) {
      GAOL_RND_LEAVE();
      return interval(std::numeric_limits<double>::max());
    }
    double l = left_internal();
    double r = right_internal();
    // As midpoint(), rounded outward: the half of the sum of the bounds, or the
    // sum of their halves where it may overflow, each half rounded outward too
    // (halving a bound below 2^-1021 is not exact). The sum may overflow only
    // where a bound is 2^1023 or more in magnitude, which is tested first, so
    // that mid([DBL_MAX]) raises no overflow exception (GAOL v5: the sum was
    // computed first, and its overflow told the other case); the sum of the
    // halves gives the same bounds as the half of the sum there
    double mid_left, mid_right;
    const double big = 8.9884656743115795e+307; // 2^1023 (C++11 has no hexadecimal floating literal)
    if (std::fabs(l) >= big || std::fabs(r) >= big) {
      mid_left  = -(.5*l + (-.5)*r);
      mid_right = (-.5)*l + .5*r;
    } else {
      double sum = r - l;     // a+b, rounded upward
      double opp_sum = l - r; // -(a+b), rounded upward
      mid_left  = -(.5*opp_sum);
      mid_right = .5*sum;
    }
    GAOL_RND_KEEP(mid_left); GAOL_RND_KEEP(mid_right);
    GAOL_RND_LEAVE();
    return interval(mid_left,mid_right);
  }

  interval div_rel(const interval &K, const interval &J, const interval &I)
  {
    // One check, before the bounds are compared, as in the SSE2 build (GAOL
    // v5, see gaol/gaol_fpu.h)
    GAOL_RND_ENTER();
    interval res = [&]() -> interval {
    if (K.is_empty() || J.is_empty() || I.is_empty()) {
      return interval::emptyset();
    }
    
    if (K.right() < 0.0) { // [this] N1
      if (J.right() < 0.0) { // [J] N1
	// I & [K.r/J.l, K.l/J.r]
	interval tmp(-(K.right_internal()/J.left_internal()),K.left_internal()/(-J.right_internal()));
	return I & tmp; // N1 N1
      } else { // [J] P or Z
	if (J.right() == 0.0) { // [J] N0 or Z
	  if (J.left() == 0.0) { // [J] Z
	    return interval::emptyset(); // N1 Z
	  }
	  // I & [K.r/J.l,+oo]
	  interval tmp(-(K.right_internal()/J.left_internal()),GAOL_INFINITY);
	  return I & tmp; // N1 N0
	} else { // [I] P or M
	  if (J.left() < 0.0) { // [J] M
	    interval tmp(-GAOL_INFINITY,K.right_internal()/J.right_internal());
	    // The lower bound rounded downward, the rounding direction being
	    // upward (GAOL v5: K.right_internal()/(-J.left_internal()) was rounded upward, and the
	    // result left out the doubles below it that belong to the hull)
	    interval tmp2(-(K.right_internal()/J.left_internal()),GAOL_INFINITY);
	    return (I&tmp) | (I&tmp2); // N1 M
	  } else { // [J] P0 or P1
	    if (J.left() == 0.0) { // [J] P0
	      interval tmp2 = I & interval(-GAOL_INFINITY,K.right()/J.right()); // N1 P0
	      return tmp2; // N1 P0
	    } else { // [J] P1
	      interval tmp(-(K.left_internal()/(-J.left_internal())),K.right_internal()/J.right_internal());
	      return I & tmp; // N1 P1
	    }
	  }
	}
      }
    } else { // [this] N0, Z, M, or P
      if (K.right() == 0.0) { // [this] N0 or Z
	if (K.left() == 0.0) { // [this] Z
	  if (J.straddles_zero()) {
	    return I; // Z (N0, M, P0)
	  } else {
	    return I & interval::zero(); // Z (N1, P1)
	  }
	} else { // [this] N0
	  if (J.right() < 0.0) { // [J] N1
	    interval tmp2 = I & interval(0.0,K.left()/J.right()); // N0 N1
	    return tmp2;
	  } else { // [J] P or Z
	    if (J.right() == 0.0) { // [J] N0 or Z
	      if (J.left() == 0.0) { // [J] Z
		return I; // N0 Z
	      }
	      return I; // N0 N0
	    } else { // [J] P or M
	      if (J.left() < 0.0) { // [J] M
		return I; // N0 M
	      } else { // [J] P0 or P1
		if (J.left() == 0.0) { // [J] P0
		  return I; // N0 P0
		} else { // [J] P1
		  interval tmp(-(K.left_internal()/(-J.left_internal())),0.0);
		  return I & tmp; // N0 P1
		}
	      }
	    }
	  }
	}
      } else { // [this] M, P0, or P1
	if (K.left() < 0.0) { // [this] M
	  if (J.right() < 0.0) { // [J] N1
	    interval tmp(-(K.right_internal()/(-J.right_internal())),K.left_internal()/(-J.right_internal()));
	    return I & tmp; // M N1
	  } else { // [J] P or Z
	    if (J.right() == 0.0) { // [J] N0 or Z
	      return I; // M N0
	    } else { // [J] P or M
	      if (J.left() < 0.0) { // [J] M
		return I; // M M
	      } else { // [J] P0 or P1
		if (J.left() == 0.0) { // [J] P0
		  return I; // M P0
		} else { // [J] P1
		  interval tmp(-(K.left_internal()/(-J.left_internal())),K.right_internal()/(-J.left_internal()));
		  return I & tmp; // M P1
		}
	      }
	    }
	  }
	} else { // [this] P0 or P1
	  if (K.left() == 0.0) { // [this] P0
	    if (J.right() < 0.0) { // [J] N1
	      interval tmp(-(K.right_internal()/(-J.right_internal())),0.0);
	      return I & tmp; // P0 N1
	    } else { // [J] P or Z
	      if (J.right() == 0.0) { // [J] N0 or Z
		return I; // P0 (Z or N0)
	      } else { // [J] P or M
		if (J.left() < 0.0) { // [J] M
		  return I; // P0 M
		} else { // [J] P0 or P1
		  if (J.left() == 0.0) { // [J] P0
		    return I; // P0 P0
		  } else { // [J] P1
		    interval tmp2 = I & interval(0.0,K.right()/J.left()); // P0 P1
		    return tmp2;
		  }
		}
	      }
	    }
	  } else { // [this] P1
	    if (J.right() < 0.0) { // [J] N1
	      interval tmp(-(K.right_internal()/(-J.right_internal())),K.left_internal()/J.left_internal());
	      return I & tmp; // P1 N1
	    } else { // [J] P or Z
	      if (J.right() == 0.0) { // [J] N0 or Z
		if (J.left() == 0.0) { // [J] Z
		  return interval::emptyset(); // P1 Z
		}
		interval tmp2 = I & interval(-GAOL_INFINITY,K.left_internal()/J.left_internal()); // P1 N0
		return tmp2;
	      } else { // [J] P or M
		if (J.left() < 0.0) { // [J] M
		  interval tmp(-GAOL_INFINITY,K.left_internal()/J.left_internal());
		  interval tmp2 = interval(-(K.left_internal()/J.right_internal()),GAOL_INFINITY);
		  return (I&tmp) | (I&tmp2); // P1 M
		} else { // [J] P0 or P1
		  if (J.left() == 0.0) { // [J] P0
		    interval tmp(-(K.left_internal()/J.right_internal()),GAOL_INFINITY);
		    return I & tmp; // P1 P0
		  } else { // [J] P1
		    interval tmp(-(K.left_internal()/J.right_internal()),K.right_internal()/(-J.left_internal()));
		    return I & tmp; // P1 P1
		  }
		}
	      }
	    }
	  }
	}
      }
    }
    }();
    GAOL_RND_KEEP(res);
    GAOL_RND_LEAVE();
    return res;
  }
