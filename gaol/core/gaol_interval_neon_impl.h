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
  \file   gaol_interval_neon_impl.h
  \brief  The operations of the NEON intervals: + - * / and their vector kernels on float64x2_t,
          out of line. Included by gaol_interval_core.cpp only.
*/

/*
  The intervals of 64-bit ARM processors, whose two stored bounds <-left,
  right> are the two lanes of a float64x2_t (GAOL v5): the counterpart of
  gaol_interval_sse_impl.h, with the NEON instructions.
*/

} // namespace gaol_core

namespace gaol_detail {

  /*
    With Visual C++ for ARM64 (cl.exe, not clang-cl) add_bounds() and sub_bounds() are each in a code
    section of their own (GAOL v5, point PG of TODO.md). The linker puts the functions of one object
    file one after the other in .text$mn, 4 bytes aligned, so that the address of these two, and the
    time of x + y, x - y and the lines of sums and differences, changed with the size of whatever
    comes before them in the link: any change of the library, one that no operation of these rows
    runs included, moved them by 16 to 48 bytes (modulo 64) and the rows by 8 to 17 % (x + y 1.54 or
    1.43 ns, line of 15 + 15.3 or 13.0 ns on the runners of GitHub). A section of a name of its own is
    aligned to the section alignment of the image, 4096: the two functions then start at a multiple
    of 4096 whatever the link holds before them, and the rows keep the faster of the two times
  */

  /*
    The sum, on the stored bounds: <-l1, r1> + <-l2, r2> = <-(l1 + l2), r1 +
    r2>. An empty operand has NaN in both lanes, which the addition carries
    without raising anything. The infinite bounds are all +oo once stored (a
    stored -oo would be [+oo, ..] or [.., -oo], which the constructors refuse),
    so that inf - inf is never formed.
  */
#  if defined(_MSC_VER) && !defined(__clang__) && defined(_M_ARM64)
#    pragma code_seg(push, ".gadd")
#  endif
  float64x2_t add_bounds(float64x2_t x, float64x2_t y)
  {
    using namespace gaol_core;
    using namespace neon_detail;
    GAOL_RND_ENTER();
    const float64x2_t xx = reread_bounds(x), yy = reread_bounds(y);
    float64x2_t res = vaddq_f64(xx, yy);
    GAOL_RND_KEEP(res);
    GAOL_RND_LEAVE();
    return res;
  }

#  if defined(_MSC_VER) && !defined(__clang__) && defined(_M_ARM64)
#    pragma code_seg(pop)
#    pragma code_seg(push, ".gsub")
#  endif
  // x - y is x + [-r, -l], the bounds of y exchanged
  float64x2_t sub_bounds(float64x2_t x, float64x2_t y)
  {
    using namespace gaol_core;
    using namespace neon_detail;
    GAOL_RND_ENTER();
    const float64x2_t xx = reread_bounds(x), yy = reread_bounds(y);
    float64x2_t res = vaddq_f64(xx, swap_lanes(yy));
    GAOL_RND_KEEP(res);
    GAOL_RND_LEAVE();
    return res;
  }
#  if defined(_MSC_VER) && !defined(__clang__) && defined(_M_ARM64)
#    pragma code_seg(pop)
#  endif

} // namespace gaol_detail

namespace gaol_detail {

  // The body is neon_detail::sqr_bounds_inline() of gaol_interval_neon_kernels.h,
  // which the powers of gaol_interval_pow.cpp take inline too (GAOL v5)
  float64x2_t sqr_bounds(float64x2_t x)
  {
    return gaol_core::neon_detail::sqr_bounds_inline(x);
  }

} // namespace gaol_detail

namespace gaol_core {

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

  /*
    Whether a bound of x or of y is a NaN or an infinity, the exponent field of
    its bits being all ones (GAOL v5, point PG of TODO.md). That is told by the
    bits 62 to 31 of each lane, which vshrn_n_u64() narrows to 32 bits, the sign
    bit dropped: they are at least 0xffe00000 for a NaN and an infinity, and
    below for a finite number, whatever the low bits. The four narrowed values
    go into one vector (shrn and shrn2), where one reduction gives their maximum:
    4 vector instructions in place of 7 or 8, and no constant in a vector
    register (the two absolute values, the two comparisons of 64-bit lanes
    with the bits of +oo and their or, which were the test of each operand
    before, then the same reduction). The same predicate on the bits, for
    every pattern
  */
  GAOL_INLINE bool any_nonfinite(float64x2_t x, float64x2_t y)
  {
    const uint32x4_t exponents = vshrn_high_n_u64(vshrn_n_u64(bits_of(x), 31), bits_of(y), 31);
    return vmaxvq_u32(exponents) >= 0xffe00000u;
  }

  } // namespace neon_detail

} // namespace gaol_core

namespace gaol_detail {

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
    test is any_nonfinite(): the exponent bits of the four bounds, narrowed
    and reduced to one maximum, and one comparison of it. The maximum of
    64-bit ARM ranks +0 above -0 and x86 maxpd returns its second operand:
    where the two lanes of a pair of zeros differ in sign, the sign of a zero
    bound may differ from the one of the SSE2 intervals (GAOL v5, like the
    AVX-512 path; the values are equal). With FPCR.AH set, which GAOL does
    not support, FMAX returns its second operand for zeros of different
    signs, and the sign of a zero bound may differ from what it is with AH
    clear, the order of the operands of the maxima having changed.
  */
  float64x2_t mul_bounds(float64x2_t x, float64x2_t y)
  {
    using namespace gaol_core;
    using namespace neon_detail;
    const bool special = any_nonfinite(x, y);
    // The test of NaN is made in this branch only. Clang 18 to 21 computed it
    // before the test of special, the comparisons being cheap and without
    // effect, and the product of finite bounds, the case that runs, paid a
    // second vector reduction (umaxv, fmov): x * y 5.29 to 4.11 ns on a
    // Neoverse N2 with Clang 18, 2.16 to 1.83 on an Apple M2 with Apple clang
    // 21. The empty asm statements of reread_bounds() are not moved above the
    // test of special, and hold the comparisons back with them (the test works
    // on bits, which the check of the rounding direction does not change).
    // GCC and Visual C++ are unchanged; a call in this branch would give them a
    // frame on the path that runs (GAOL v5, point PG of TODO.md)
    if (special && any_lane(vorrq_u64(nan_lanes(reread_bounds(x)),
                                      nan_lanes(reread_bounds(y))))) { // an empty operand
      return nan_pair();
    }
    GAOL_RND_ENTER();
    x = reread_bounds(x);
    y = reread_bounds(y);
    float64x2_t res;
    if (special) {
      const float64x2_t yc = negate_first(y);
      const float64x2_t yd = swap_lanes(yc);
      const float64x2_t xo = vnegq_f64(swap_lanes(x));
      res = vmaxq_f64(vmaxq_f64(guarded_products(x, yc), guarded_products(x, yd)),
                      vmaxq_f64(guarded_products(xo, yc), guarded_products(xo, yd)));
    } else {
      // The same eight products as above, grouped by the bound of y they are made with, so that
      // two vectors of x and y make them all, by the multiplication by a lane: with x = <a, b>
      // and y = <c, d>, a * (-c) and b * (-c), a * d and b * d, b * c and a * c (the swapped x),
      // -b * d and -a * d. Each is the same double as the product of the same two numbers with the
      // sign moved to the other factor, whichever the rounding direction (the rounding depends on
      // the value and the sign of a product, not on the factor that holds the sign), so that
      // the maxima are those of the four vectors above, in lane 0 as in lane 1. 2 vector
      // instructions to prepare them in place of 6
      const float64x2_t ny = vnegq_f64(y);
      const float64x2_t sx = swap_lanes(x);
      res = vmaxq_f64(vmaxq_f64(vmulq_laneq_f64(x, ny, 0), vmulq_laneq_f64(x, y, 1)),
                      vmaxq_f64(vmulq_laneq_f64(sx, y, 0), vmulq_laneq_f64(sx, ny, 1)));
    }
    GAOL_RND_KEEP(res);
    GAOL_RND_LEAVE();
    return res;
  }

} // namespace gaol_detail

namespace gaol_detail {

  float64x2_t div_bounds(float64x2_t x, float64x2_t y)
  {
    using namespace gaol_core;
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

} // namespace gaol_detail

namespace gaol_core {

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

  // The body is upward_access::checked_inverse() of gaol_interval_neon_kernels.h,
  // which nth_root(I, q) for q = -2 takes inline too (GAOL v5)
  interval interval::inverse() const
  {
    return upward_access::checked_inverse(*this);
  }

  /*
    The operations with a double. The empty interval and a d that is
    infinite or a NaN, for which interval(d) is empty, give the empty set,
    told by a quiet comparison of |d| with +oo (neon_detail::finite_double()). Both lanes are
    computed, <-l, r> + <-d, d> for the sum, and the product or the quotient by |d| of the bounds,
    exchanged for a negative d (GAOL v5, as the SSE2 intervals). The bounds
    are read after the check of the direction (reread_bounds()), as are d
    (rnd_reread()) before it is compared, so that the modes that flush the
    subnormals to zero, which the check clears, cannot read a subnormal d as 0.
  */
  interval& interval::operator+=(double d)
  {
    if (is_empty() || !neon_detail::finite_double(d)) { // interval(d) is empty for an infinite d and for a NaN
      neon_detail::keep_branch();
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
    if (is_empty() || !neon_detail::finite_double(d)) { // interval(d) is empty for an infinite d and for a NaN
      neon_detail::keep_branch();
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
    if (is_empty() || !neon_detail::finite_double(d)) { // interval(d) is empty for an infinite d and for a NaN
      neon_detail::keep_branch();
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
    if (is_empty() || !neon_detail::finite_double(d)) { // interval(d) is empty for an infinite d and for a NaN
      neon_detail::keep_branch();
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
    if (is_empty() || !neon_detail::finite_double(d)) { // interval(d) is empty for an infinite d and for a NaN
      neon_detail::keep_branch();
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
#if GAOL_NEON_MSVC
      // Each half with a negated factor goes through rnd_keep(), whose volatile
      // memory keeps it as written: Visual C++ (/O2 /fp:strict) rewrites
      // (-.5)*x + y into y - .5*x, which rounds the half of x in the wrong
      // direction when it is inexact, and mid([2^-1074, MAX]) was
      // [MAX/2, MAX/2], as with the SSE2 and the FPU intervals (gaol_interval_sse_impl.h)
      mid_left  = -(.5*l + gaol_core::rnd_keep((-.5)*r));
      mid_right = gaol_core::rnd_keep((-.5)*l) + .5*r;
#else
      mid_left  = -(.5*l + (-.5)*r);
      mid_right = (-.5)*l + .5*r;
#endif
    } else {
      double sum = r - l;     // a+b, rounded upward
      double opp_sum = l - r; // -(a+b), rounded upward
#if GAOL_NEON_MSVC
      // The half goes through rnd_keep() before it is negated, for the reason
      // given above (-(.5*x) is not (-.5)*x in the rounding direction upward)
      mid_left  = -gaol_core::rnd_keep(.5*opp_sum);
#else
      mid_left  = -(.5*opp_sum);
#endif
      mid_right = .5*sum;
    }
    GAOL_RND_KEEP(mid_left); GAOL_RND_KEEP(mid_right);
    GAOL_RND_LEAVE();
    return interval(mid_left,mid_right);
  }

  /*
    The integer powers (GAOL v5, point AP of TODO.md): the two lanes of the
    kernel of pown_products() in a NEON register, vmulq_f64() and the fused
    vfmaq_f64(): a * b - c is a * b + (-c), the negation being exact, so that
    the sum is rounded once and the product not at all, as _mm_fmsub_pd()
    does. The operations are those of the pair of doubles of pown_scalar_ops,
    lane by lane, in the direction upward, so that the bounds are the same bit
    for bit.
  */
  struct pown_neon_ops {
    typedef float64x2_t lanes;
    typedef uint64x2_t rests;
    static GAOL_POWN_INLINE lanes mul(lanes a, lanes b) { return vmulq_f64(a, b); }
    static GAOL_POWN_INLINE lanes fms(lanes a, lanes b, lanes c) { return vfmaq_f64(vnegq_f64(c), a, b); }
    static GAOL_POWN_INLINE lanes fma(lanes a, lanes b, lanes c) { return vfmaq_f64(c, a, b); }
    static GAOL_POWN_INLINE lanes twice_plus(lanes a, lanes b) { return vfmaq_f64(b, vdupq_n_f64(2.0), a); }
    static GAOL_POWN_INLINE rests first_rest(lanes e) { return vreinterpretq_u64_f64(e); }
    static GAOL_POWN_INLINE void add_rest(rests& s, lanes e) { s = vorrq_u64(s, vreinterpretq_u64_f64(e)); }
  };

  /*
    I^n for |n| >= 2 and a nonempty I with the NEON intervals, the rounding
    direction being upward: both corners at once, in the NEON register, where
    they are within the range of pown_least[] and pown_greatest[], and
    pown_by_corners() otherwise (see pown_products()). It repeats the block of
    the SSE2 intervals (gaol/core/gaol_interval_sse_impl.h), the same operations in the
    same order: keep them alike, with pown_large_er() (gaol_interval_avx512_impl.h).
    tests/tools/pow/mutate.py has the mutants of the SSE2 block.
  */
  static GAOL_POWN_INLINE_T interval pown_large_inline(const interval& I, int n)
  {
    const bool negative = n < 0;
    const unsigned int m = negative ? 0u - static_cast<unsigned int>(n) : static_cast<unsigned int>(n);
    const unsigned int L = pown_bit_length(m);
    const double a = I.left(), b = I.right();
    if (!negative || a > 0.0 || b < 0.0) {
      // The bounds (a, b) and (-b, -a) are made
      // from the stored register <-a, b> without going through memory: two
      // stores of a double and a load of the pair do not forward on many cores
      const float64x2_t zero = vdupq_n_f64(0.0);
      const float64x2_t ab = neon_detail::negate_first(I.get_neoninterval());
      const float64x2_t nba = vnegq_f64(vextq_f64(ab, ab, 1));
      float64x2_t c = ab;
      if ((m & 1u) == 0u) {
        const float64x2_t greater = vmaxq_f64(ab, nba);
        c = vcombine_f64(vget_low_f64(vmaxq_f64(greater, zero)), vget_high_f64(greater));
      }
      if (negative) {
        c = vextq_f64(c, c, 1);
#if !GAOL_NEON_MSVC
        // Clang makes a select of this swap, with a move from a general register
        // to the vector one on the way to the kernel (GAOL v5, point PG.7)
        __asm__ ("" : "+w" (c));
#endif
      }
      const float64x2_t g = vabsq_f64(c);
      // The absolute comparisons, g being |c| and the limits positive: with
      // -frounding-math Clang scalarizes the plain vcgeq_f64(), vcleq_f64() and
      // vceqq_f64() (a fcmpe or fcmp, a csetm and a move for each lane: 18
      // instructions more in pow(x, 3) for the same masks), and vcageq_f64() and
      // vcaleq_f64() stay on the vector unit; c == 0 is |c| <= 0 (GAOL v5, point PG.7)
      uint64x2_t ok = vandq_u64(vcageq_f64(g, vdupq_n_f64(pown_least[L])), vcaleq_f64(g, vdupq_n_f64(pown_greatest[L])));
      if (!negative) {
        ok = vorrq_u64(ok, vcaleq_f64(c, zero));
      }
      if ((vgetq_lane_u64(ok, 0) & vgetq_lane_u64(ok, 1)) != 0u) {
        float64x2_t h, l, K;
        uint64x2_t s;
        pown_powers<pown_neon_ops>(g, m, h, l, s);
        if (negative) {
          const float64x2_t one = vdupq_n_f64(1.0);
          const float64x2_t q = vdivq_f64(one, h);
          const float64x2_t r = vfmsq_f64(vfmsq_f64(one, q, h), q, l); // (1 - q h) - q l
          const float64x2_t qr = vmulq_f64(q, r);
          l = vfmaq_f64(qr, qr, r); // q (r + r^2)
          h = q;
          s = vorrq_u64(s, vreinterpretq_u64_f64(r));
          K = vdupq_n_f64(pown_bound_inverse(m, L));
        } else {
          K = vdupq_n_f64(pown_bound(m, L));
        }
        // The rests compared as integers (see gaol_interval_sse_impl.h): K is 0 where they are
        K = vreinterpretq_f64_u64(vbicq_u64(vreinterpretq_u64_f64(K), vceqzq_u64(s)));
        // (-v0 + RU(K h - w0), v1 + RU(K h + w1)), v + w being +-(h + l), of
        // the sign of c for an odd m, the lower bound negated
        const std::uint64_t sign_bit = 0x8000000000000000ull;
        const uint64x2_t odd = ((m & 1u) != 0u) ? vandq_u64(vreinterpretq_u64_f64(c), vdupq_n_u64(sign_bit)) : vdupq_n_u64(0u);
        const uint64x2_t sign = veorq_u64(vcombine_u64(vcreate_u64(sign_bit), vcreate_u64(0u)), odd);
        const float64x2_t v = vreinterpretq_f64_u64(veorq_u64(vreinterpretq_u64_f64(h), sign));
        const float64x2_t w = vreinterpretq_f64_u64(veorq_u64(vreinterpretq_u64_f64(l), sign));
        return interval(vaddq_f64(v, vfmaq_f64(w, K, h)), gaol_detail::unchecked_bounds);
      }
    }
    return pown_by_corners(a, b, n);
  }

} // namespace gaol_core
