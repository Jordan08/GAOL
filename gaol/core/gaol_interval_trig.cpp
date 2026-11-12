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
  \file   gaol_interval_trig.cpp
  \brief  The trigonometric functions of the intervals: sin, cos, tan, their inverses, atan2,
          the functions of pi x, and modulo_k_pi(), the reduction modulo pi.

  \author Goualard Frederic, NININ Jordan
  \date   2001-09-28
*/

#include "gaol/core/gaol_interval_internal.h"

/*
  sin, cos, tan, asin, acos, atan, atan2 and the functions of pi x (sinpi,
  cospi, tanpi, asinpi, acospi, atanpi, atan2pi), with modulo_k_pi() (GAOL
  v5: a part of gaol_interval_core.cpp, see gaol_interval_internal.h). Their
  reverse functions are in gaol_interval_relational.cpp.
*/

namespace gaol_core {

  // The bounds of sin(x) and cos(x) where they are known (see the bounds of
  // the elementary functions in gaol_interval_internal.h)
  static inline double sin_lo(double x) { return (x == 0.0) ? 0.0 : upward::sin_dn(x); }
  static inline double sin_hi(double x) { return (x == 0.0) ? 0.0 : upward::sin_up(x); }
  static inline double cos_lo(double x) { return (x == 0.0) ? 1.0 : upward::cos_dn(x); }
  static inline double cos_hi(double x) { return (x == 0.0) ? 1.0 : upward::cos_up(x); }

  /*
    The signs of sin(x) and cos(x), x being finite (GAOL v5, issue #6).
    Neither is 0 but sin(0): no other double is a multiple of pi/2, and the
    closest one, 6381956970095103 2^797, has a cosine of 4.7e-19, so that the
    value of the mathematical library, moved one double downward, still has
    the sign of the function. Next to 0 the sign is known from x. Rounding
    upward, after the GAOL_RND_ENTER() of the caller.
  */
  static inline int sign_of_sin(double x)
  {
    if (std::fabs(x) < 3.0) {
      return (x > 0.0) - (x < 0.0);
    }
    return (upward::sin_dn(x) > 0.0) ? 1 : -1;
  }

  static inline int sign_of_cos(double x)
  {
    if (std::fabs(x) < 1.5) {
      return 1;
    }
    return (upward::cos_dn(x) > 0.0) ? 1 : -1;
  }

  /*
    Bounds of atan2(y, x) at a point other than (0, 0) (GAOL v5): 0, pi/2,
    pi, their opposites and pi/4 where the angle is one of them, a bound being
    an infinity included, where atan2 has that limit; the value of the
    mathematical library moved outward elsewhere, within [-pi, pi]. The sign of
    a zero does not count: atan2(0, x) is pi for x < 0.
  */
  static double atan2_lo(double y, double x)
  {
    if (y == 0.0 || x == GAOL_INFINITY) {
      return (x > 0.0) ? 0.0 : gaol_pi_dn;
    }
    if (x == 0.0 || std::fabs(y) == GAOL_INFINITY) {
      return (y > 0.0) ? gaol_half_pi_dn : -gaol_half_pi_up;
    }
    if (x == -GAOL_INFINITY) {
      return (y > 0.0) ? gaol_pi_dn : -gaol_pi_up;
    }
    if (x == std::fabs(y)) {
      return (y > 0.0) ? gaol_half_pi_dn*0.5 : -gaol_half_pi_up*0.5;
    }
    return maximum(upward::atan2_dn(y,x), -gaol_pi_up);
  }

  static double atan2_hi(double y, double x)
  {
    if (y == 0.0 || x == GAOL_INFINITY) {
      return (x > 0.0) ? 0.0 : gaol_pi_up;
    }
    if (x == 0.0 || std::fabs(y) == GAOL_INFINITY) {
      return (y > 0.0) ? gaol_half_pi_up : -gaol_half_pi_dn;
    }
    if (x == -GAOL_INFINITY) {
      return (y > 0.0) ? gaol_pi_up : -gaol_pi_dn;
    }
    if (x == std::fabs(y)) {
      return (y > 0.0) ? gaol_half_pi_up*0.5 : -gaol_half_pi_dn*0.5;
    }
    return minimum(upward::atan2_up(y,x), gaol_pi_up);
  }

  // atan(x)/pi is rational at a rational x only for x in {0, +-1}, by Niven's
  // theorem, where it is 0 and +-1/4; and it is +-1/2 at the infinities
  static inline bool atanpi_is_exact(double x)
  {
    return x == 0.0 || x == 1.0 || x == -1.0 || !is_finite(x);
  }

  // acos(x)/pi is a double at -1, 0 and 1 only, where it is 1, 1/2 and 0: the
  // other rational values, 1/3 and 2/3 at +-1/2, are no doubles
  static inline bool acospi_is_exact(double x)
  {
    return x == 0.0 || x == 1.0 || x == -1.0;
  }

  interval atanpi(const interval& I)
  {
    // within [-1/2, 1/2], the image of atan divided by pi
    return increasing_cr(I, gaol_cr_atanpi, atanpi_is_exact, -0.5, 0.5);
  }

  interval acospi(const interval& I)
  {
    // Defined on [-1, 1], as acos (IEEE 1788-2015, Table 10.5): the part of I
    // outside is left out, and I holding no point of it gives the empty set
    if (I.is_empty()) {
      return interval::emptyset();
    }
    GAOL_RND_ENTER();
    if (I.right() < -1.0 || I.left() > 1.0) {
      GAOL_RND_LEAVE();
      return interval::emptyset();
    }
    const double l = maximum(-1.0, I.left()), r = minimum(1.0, I.right());
    // decreasing: the upper bound at the left bound, the lower one at the right
    const double v = gaol_cr_acospi(l);
    const double w = gaol_cr_acospi(r);
    const double u = acospi_is_exact(r) ? w : previous_float(w);
    interval res(maximum(0.0, u), minimum(1.0, v));
    GAOL_RND_KEEP(res);
    GAOL_RND_LEAVE();
    return res;
  }

  /*
    sinpi, cospi and tanpi: the points where these functions change direction,
    or have a pole, are the multiples of 1/2, which are doubles, so the
    analysis of an interval reduces exactly to the integers t = 2x. Doubling a
    double is exact, and so are its ceiling and its floor; below 2^61 these
    integers are held exactly in 64 bits, and two distinct doubles beyond 2^61
    are at least 2^9 apart, whole periods. sin(pi*x) and cos(pi*x) have their
    extrema at the odd, respectively even, t, and tan(pi*x) its poles at the
    odd t, increasing between them.
  */
  static const double gaol_two_61 = 2305843009213693952.0;

  // Whether an integer congruent to r modulo m lies in [lo, hi]
  static inline bool holds_residue(std::int64_t lo, std::int64_t hi,
                                   std::int64_t r, std::int64_t m)
  {
    if (lo > hi) {
      return false;
    }
    std::int64_t lm = lo % m;
    if (lm < 0) {
      lm += m;
    }
    std::int64_t d = (r - lm) % m;
    if (d < 0) {
      d += m;
    }
    return lo + d <= hi;
  }

  // Whether k*x is an integer: sin(pi*x) and cos(pi*x) are doubles exactly at
  // the multiples of 1/2 (k = 2), and tan(pi*x) at those of 1/4 (k = 4). By
  // Niven's theorem sin and cos take at a rational x the rational values 0,
  // +-1/2 and +-1 only, +-1/2 at points that are not doubles, and tan the
  // values 0 and +-1 only. Beyond 2^61 every double is an even integer.
  static inline bool is_multiple_of_inverse(double x, double k)
  {
    if (std::fabs(x) >= gaol_two_61) {
      return true;
    }
    const double t = k * x;
    return t == std::floor(t);
  }

  // sin(pi*x) or cos(pi*x) over I: the maximum 1 where a t congruent to up
  // modulo 4 lies within, the minimum -1 where one congruent to down does, and
  // the values at the bounds otherwise. The rounding direction is upward, after
  // the check of sin_or_cos_pi(), made before the bounds are compared: 2 l,
  // whose ceiling gives ka, is 0 for a subnormal l with the modes that flush
  // them to zero (see gaol/gaol_fpu.h)
  static interval sin_or_cos_pi_upward(const interval& I, double (*f)(double),
                                       std::int64_t up, std::int64_t down)
  {
    if (I.is_empty()) {
      return interval::emptyset();
    }
    const double l = I.left(), r = I.right();
    if (!is_finite(l) || !is_finite(r)) {
      return interval(-1.0, 1.0);
    }
    const double far = maximum(std::fabs(l), std::fabs(r));
    if (l != r && far >= gaol_two_61) {
      return interval(-1.0, 1.0);
    }
    bool has_max = false, has_min = false;
    if (far < gaol_two_61) {
      const std::int64_t ka = static_cast<std::int64_t>(std::ceil(2.0 * l));
      const std::int64_t kb = static_cast<std::int64_t>(std::floor(2.0 * r));
      if (kb - ka >= 3) {
        return interval(-1.0, 1.0);  // every residue modulo 4
      }
      has_max = holds_residue(ka, kb, up, 4);
      has_min = holds_residue(ka, kb, down, 4);
    }
    const double fl = f(l), fr = f(r);
    const double dl = is_multiple_of_inverse(l, 2.0) ? fl : previous_float(fl);
    const double dr = is_multiple_of_inverse(r, 2.0) ? fr : previous_float(fr);
    const double lower = has_min ? -1.0 : maximum(-1.0, minimum(dl, dr));
    const double upper = has_max ? 1.0 : minimum(1.0, maximum(fl, fr));
    return interval(lower, upper);
  }

  /* The minimum and the maximum taken before GAOL_RND_LEAVE(): with
     GAOL_PRESERVE_ROUNDING, the denormals-are-zero mode it restores made two
     subnormal values equal, and sinpi([100, 1000] 2^-1074) was empty (GAOL v5)
     */
  static interval sin_or_cos_pi(const interval& I, double (*f)(double),
                                std::int64_t up, std::int64_t down)
  {
    GAOL_RND_ENTER();
    interval res = sin_or_cos_pi_upward(I, f, up, down);
    GAOL_RND_KEEP(res);
    GAOL_RND_LEAVE();
    return res;
  }

  interval sinpi(const interval& I)
  {
    // sin(pi*x) = 1 at x = 1/2 modulo 2, t = 1 modulo 4; -1 at t = 3
    return sin_or_cos_pi(I, gaol_cr_sinpi, 1, 3);
  }

  interval cospi(const interval& I)
  {
    // cos(pi*x) = 1 at the even x, t = 0 modulo 4; -1 at the odd x, t = 2
    return sin_or_cos_pi(I, gaol_cr_cospi, 0, 2);
  }

  // tanpi() after its check, made before the bounds are compared, as in
  // sin_or_cos_pi()
  static interval tanpi_upward(const interval& I)
  {
    if (I.is_empty()) {
      return interval::emptyset();
    }
    const double l = I.left(), r = I.right();
    if (!is_finite(l) || !is_finite(r)) {
      return interval::universe();
    }
    const double far = maximum(std::fabs(l), std::fabs(r));
    if (l != r && far >= gaol_two_61) {
      return interval::universe();
    }
    bool pole_inside = false, pole_at_l = false, pole_at_r = false;
    if (far < gaol_two_61) {
      const double tl = 2.0 * l, tr = 2.0 * r;
      // the integers strictly between tl and tr, and the poles among them
      const std::int64_t lo = static_cast<std::int64_t>(std::floor(tl)) + 1;
      const std::int64_t hi = static_cast<std::int64_t>(std::ceil(tr)) - 1;
      pole_inside = holds_residue(lo, hi, 1, 2);
      pole_at_l = tl == std::floor(tl) && std::fmod(tl, 2.0) != 0.0;
      pole_at_r = tr == std::floor(tr) && std::fmod(tr, 2.0) != 0.0;
    }
    // tan(pi*x) has no value at a pole (IEEE 1788-2015, Table 10.5), and is
    // increasing between two poles, from -oo to +oo
    if (l == r && pole_at_l) {
      return interval::emptyset();
    }
    if (pole_inside || (pole_at_l && pole_at_r)) {
      return interval::universe();
    }
    // a pole at a bound is the limit -oo from its right, +oo from its left
    double lower = -GAOL_INFINITY, upper = GAOL_INFINITY;
    if (!pole_at_l) {
      const double fl = gaol_cr_tanpi(l);
      lower = is_multiple_of_inverse(l, 4.0) ? fl : previous_float(fl);
    }
    if (!pole_at_r) {
      upper = gaol_cr_tanpi(r);
    }
    return interval(lower, upper);
  }

  interval tanpi(const interval& I)
  {
    GAOL_RND_ENTER();
    interval res = tanpi_upward(I);
    GAOL_RND_KEEP(res);
    GAOL_RND_LEAVE();
    return res;
  }

  /*
    asinpi, the asinPi of IEEE 1788-2015 (Table 10.5): asin(x)/pi on [-1, 1],
    increasing, within [-1/2, 1/2].

    asin(x)/pi is rational at a rational x only for x in {0, +-1/2, +-1}, by
    Niven's theorem, where it is 0, +-1/6 and +-1/2: a double at 0 and +-1.
  */
  static inline bool asinpi_is_exact(double x)
  {
    return x == 0.0 || x == 1.0 || x == -1.0;
  }

  interval asinpi(const interval& I)
  {
    // the part of I outside [-1, 1] is left out, as with acospi, after the
    // check: GCC 13 compared the bounds of the intersection, held in
    // registers, with 0 before it, and asinpi([100 2^-1074]) was
    // [32 2^-1074] with the denormals-are-zero mode set (GAOL v5)
    GAOL_RND_ENTER();
    interval res = increasing_cr_upward(I & interval::minus_one_plus_one(), gaol_cr_asinpi,
                                        asinpi_is_exact, -0.5, 0.5);
    GAOL_RND_KEEP(res);
    GAOL_RND_LEAVE();
    return res;
  }

  /*
    atan2pi, the atan2Pi of IEEE 1788-2015 (Table 10.5): atan2(y, x)/pi on the
    plane but (0, 0), with values in (-1, 1]. The analysis of the box is that
    of atan2 above, the angles divided by pi. The angle over pi is a double
    where it is a multiple of 1/4, which the tests below find: 0, +-1/4, +-1/2,
    +-3/4 and 1, and -1 as a limit. Elsewhere it is irrational: tan(pi r) is
    rational for a rational r only where it is 0 or +-1 (Niven), and y/x is
    rational.
  */
  static inline bool atan2pi_special(double y, double x, double& v)
  {
    if (y == 0.0 || x == GAOL_INFINITY) {
      v = (x > 0.0) ? 0.0 : 1.0;
      return true;
    }
    if (x == 0.0 || std::fabs(y) == GAOL_INFINITY) {
      v = (y > 0.0) ? 0.5 : -0.5;
      return true;
    }
    if (x == -GAOL_INFINITY) {
      v = (y > 0.0) ? 1.0 : -1.0;
      return true;
    }
    if (std::fabs(x) == std::fabs(y)) {
      const double q = (x > 0.0) ? 0.25 : 0.75;
      v = (y > 0.0) ? q : -q;
      return true;
    }
    return false;
  }

  static double atan2pi_lo(double y, double x)
  {
    double v;
    return atan2pi_special(y, x, v) ? v : maximum(previous_float(gaol_cr_atan2pi(y, x)), -1.0);
  }

  static double atan2pi_hi(double y, double x)
  {
    double v;
    return atan2pi_special(y, x, v) ? v : minimum(gaol_cr_atan2pi(y, x), 1.0);
  }

  // atan2pi() after its check, made before the bounds are compared, as in
  // atan2()
  static interval atan2pi_upward(const interval& Y, const interval& X)
  {
    if (Y.is_empty() || X.is_empty()) {
      return interval::emptyset();
    }
    const double yl = Y.left(), yu = Y.right(), xl = X.left(), xu = X.right();
    if (yl == 0.0 && yu == 0.0 && xl == 0.0 && xu == 0.0) {
      return interval::emptyset();
    }
    if (yl < 0.0 && yu >= 0.0 && xl < 0.0) {
      return interval(-1.0, 1.0);
    }
    double l, r;
    if (yl >= 0.0) { // Upper half-plane: the angle decreases with x
      l = atan2pi_lo((xu > 0.0) ? yl : yu, xu);
      r = (xl == 0.0 && yu == 0.0) ? 0.0 : atan2pi_hi((xl >= 0.0) ? yu : yl, xl);
    } else if (yu < 0.0) { // Lower half-plane, y = 0 left out: the angle increases with x
      l = atan2pi_lo((xl >= 0.0) ? yl : yu, xl);
      r = atan2pi_hi((xu > 0.0) ? yu : yl, xu);
    } else { // yl < 0 <= yu, in the right half-plane
      l = atan2pi_lo(yl, xl);
      r = (yu == 0.0) ? ((xu == 0.0) ? -0.5 : 0.0) : atan2pi_hi(yu, xl);
    }
    return interval(l, r);
  }

  interval atan2pi(const interval& Y, const interval& X)
  {
    GAOL_RND_ENTER();
    interval res = atan2pi_upward(Y, X);
    GAOL_RND_KEEP(res);
    GAOL_RND_LEAVE();
    return res;
  }


  /*
    The bounds of x/[pi] for a finite x, [pi] being [gaol_pi_dn, gaol_pi_up], rounded
    upward, for modulo_k_pi(): computed on doubles, as tan() computes those of
    [x + pi/2]/[pi] (GAOL v5). They are the doubles interval(x) /
    interval::pi() gave, the stored bounds -x and x being divided by gaol_pi_dn
    above 0 and by gaol_pi_up below. A bound 0 of I gives the quotient 0 with its
    own sign, where interval(x), then the division, gave one each build chose:
    floor() keeps it, and the comparisons of modulo_k_pi() do not tell -0
    from +0.
  */
  static inline double lower_of_x_over_pi(double x)
  {
    const double nl = -x;
    return -(nl / ((nl > 0.0) ? gaol_pi_dn : gaol_pi_up));
  }

  static inline double upper_of_x_over_pi(double x)
  {
    return x / ((x > 0.0) ? gaol_pi_dn : gaol_pi_up);
  }

  /*!
    \brief Angle modulo pi (slower version with integer output)

    Computes k_left and k_right for I=[a,b] such that
    * a = k_left*\pi + a'  with |a'|<\pi
    * b = k_right*\pi + b' with |b'|<\pi
    \return 0 if neither l_left nor l_right could be computed accurately,
    1 if l_right could be computed, 2 if l_left could be computed and
    3 if both could be computed.

    \note There might be more than one integer in
    I.left()/interval::pi() (resp. I.right()/interval::pi()) when one of the bounds of I
    is close to a multiple of \pi
  */
  unsigned short int modulo_k_pi(const interval &I, double &k_left, double &k_right)
  {
    // The empty set, whose bounds are NaN, gives NaN and 0, tested first
    // (GAOL v5): the floor of its NaN bounds divided by pi would be the empty
    // set too, but k_left its lower bound, a NaN of the other sign
    if (I.is_empty()) {
      k_left = k_right = GAOL_NAN;
      return 0;
    }
    // The only check of the rounding direction (GAOL v5): the quotients are
    // computed on doubles, where I.left()/interval::pi() and
    // I.right()/interval::pi() checked it once each. An infinite bound gives
    // NaN and no flag, as interval(+-oo), which is empty, gave through them
    GAOL_RND_ENTER();
    const double a = I.left(), b = I.right();
    const double dmax = (std::numeric_limits<double>::max)();
    const bool finite_a = std::fabs(a) <= dmax, finite_b = std::fabs(b) <= dmax;
    double a_lo = finite_a ? bound_floor(lower_of_x_over_pi(a)) : GAOL_NAN;
    double a_hi = finite_a ? bound_floor(upper_of_x_over_pi(a)) : GAOL_NAN;
    double b_lo = finite_b ? bound_floor(lower_of_x_over_pi(b)) : GAOL_NAN;
    double b_hi = finite_b ? bound_floor(upper_of_x_over_pi(b)) : GAOL_NAN;
    GAOL_RND_KEEP(a_lo); GAOL_RND_KEEP(a_hi); GAOL_RND_KEEP(b_lo); GAOL_RND_KEEP(b_hi);
    GAOL_RND_LEAVE();
    k_left  = a_lo;
    k_right = b_hi;
    return static_cast<unsigned short int>(((finite_a && a_lo == a_hi) ? 2 : 0) + ((finite_b && b_lo == b_hi) ? 1 : 0));
  }


  /*
    The bounds of [x + pi/2]/[pi] for a finite x, [pi] being [gaol_pi_dn, gaol_pi_up],
    rounded upward, as tan() needs them: computed on doubles rather than with
    the operations of intervals, which checked the rounding direction once more
    each, after the check of tan() (GAOL v5). They are the doubles
    interval(x) + interval::half_pi() and operator/=(interval) give, in the two
    builds: the stored bounds of the sum are nl = (-x) + (-gaol_half_pi_dn) and
    nr = x + gaol_half_pi_up, and the division by an interval above 0 divides a
    stored bound above 0 by gaol_pi_dn and one below 0 by gaol_pi_up. A stored bound 0
    stays 0, with a sign that may differ from the one of the operations of
    intervals (+0 or -0 for the lower bound, whose stored value the builds set
    each their own way); tan() only takes floor() and ceil() of these bounds
    and compares them, for which -0 and +0 are the same.
  */
  static inline double lower_of_x_plus_half_pi_over_pi(double x)
  {
    const double nl = (-x) + (-gaol_half_pi_dn);
    return -(nl / ((nl > 0.0) ? gaol_pi_dn : gaol_pi_up));
  }

  static inline double upper_of_x_plus_half_pi_over_pi(double x)
  {
    // x + gaol_half_pi_up rounded upward is next to x or above it, +oo for the
    // largest double only, which is not computed: the sum raised the overflow
    // exception for a finite tangent (GAOL v5:
    // tan([DBL_MAX]))
    const double nr = (x == (std::numeric_limits<double>::max)()) ? GAOL_INFINITY : read_in_branch(x) + gaol_half_pi_up;
    return nr / ((nr > 0.0) ? gaol_pi_dn : gaol_pi_up);
  }

  interval tan(const interval& I)
  {
    // Empty set?
    if (I.is_empty()) {
      return interval::emptyset();
    }

    // The only check of the rounding direction (GAOL v5): the width and the
    // quotients below are computed on doubles, where tan() called width(), +
    // and / on intervals, which checked it five times more
    GAOL_RND_ENTER();
    const double l = I.left(), r = I.right();
    /*
      The width of I, rounded upward as width() does: the exact width is at
      most w, and w <= gaol_pi_dn, the double below pi, proves it below pi, so that
      I holds at most one pole (GAOL v5, review #8 of examples/examples.md: the
      test was !(w < gaol_pi_up) in GAOL 4, and [-M_PI_2, M_PI_2], of width gaol_pi_dn, gave
      [-oo, +oo], as did the intervals whose exact width lies between the
      double below gaol_pi_dn and gaol_pi_dn, which round up to it).
      Above gaol_pi_dn, I may hold two poles, the cosine having the same sign
      at both bounds: [-oo, +oo], the tightest bound but for the intervals
      holding no pole whose exact width is below pi. Also for a NaN width, from
      [+oo, +oo] or [-oo, -oo] built from SSE2 registers, which the
      constructors refuse: GAOL gave [-oo, +oo] for them too
    */
    const double w = r - l;
    if (!(w <= gaol_pi_dn)) {
      GAOL_RND_LEAVE();
      return interval::universe();
    }

    /*
      The poles of tan are the x for which (x + pi/2)/pi is an integer, which
      no double is. A and B enclose that quotient at the bounds of I: there is
      no pole within I when the lower bound of A and the upper bound of B have
      the same integer part, and there is one when an integer lies between the
      upper bound of A and the lower bound of B. When the quotients cannot
      tell, a bound of I being within about |x| 2^-52 of a pole, or beyond
      2^52, the signs of the cosine at the bounds do (GAOL v5, issue #6,
      see cos_or_sin()): I being narrower than pi, as w <= gaol_pi_dn shows, there
      is a pole within I exactly when they differ. GAOL gave [-oo, +oo] then,
      as for the double below pi/2, whose tangent is 0x1.9153d9443ed0bp+51, and
      for every interval beyond 2^52.
    */
    // l and r are finite, w being at most gaol_pi_dn
    const double A_left = lower_of_x_plus_half_pi_over_pi(l), A_right = upper_of_x_plus_half_pi_over_pi(l),
      B_left = lower_of_x_plus_half_pi_over_pi(r), B_right = upper_of_x_plus_half_pi_over_pi(r);
    bool no_pole = (std::floor(A_left) == std::floor(B_right));
    const bool told = no_pole || (std::ceil(A_right) <= std::floor(B_left));
    if (told && !no_pole) { // A pole within I for sure
      GAOL_RND_LEAVE();
      return interval::universe();
    }
    // The rounding direction is upward already, set at the top of tan()
    if (!told) {
      no_pole = (sign_of_cos(l) == sign_of_cos(r));
    }
    double u = -GAOL_INFINITY, v = GAOL_INFINITY;
    if (no_pole) {
      u = tan_lo(l);
      v = tan_hi(r);
    }
    GAOL_RND_KEEP(u); GAOL_RND_KEEP(v);
    GAOL_RND_LEAVE();
    return interval(u,v);
  }


  interval acos(const interval& I)
  {
    // The check before the intersection, whose bounds, held in registers,
    // acos_lo() and acos_hi() compare (see asinpi())
    GAOL_RND_ENTER();
    interval J = I & interval::minus_one_plus_one();
    // J <- I \cap [-1,1]

    if (J.is_empty()) {
      GAOL_RND_LEAVE();
      return interval::emptyset();
    }

    double l = acos_lo(J.right()), r = acos_hi(J.left());
    GAOL_RND_KEEP(l); GAOL_RND_KEEP(r);
    GAOL_RND_LEAVE();
    return interval(l,r);
  }

  interval asin(const interval& I)
  {
    GAOL_RND_ENTER();
    interval J= I & interval::minus_one_plus_one();
    // J <- I \cap [-1,1]

    if (J.is_empty()) {
      GAOL_RND_LEAVE();
      return interval::emptyset();
    }
    double l = asin_lo(J.left()), r = asin_hi(J.right());
    GAOL_RND_KEEP(l); GAOL_RND_KEEP(r);
    GAOL_RND_LEAVE();
    return interval(l,r);
  }

  interval atan(const interval& I)
  {
	if (I.is_empty()) {
	  return I;
	}
    GAOL_RND_ENTER();
    const double l = atan_lo(I.left()), r = atan_hi(I.right());
    GAOL_RND_LEAVE();
    return interval(l,r);
  }

  /*
    atan2 of IEEE 1788-2015 (Table 9.1), defined on the plane but (0, 0), with
    values in (-pi, pi] (GAOL v5; GAOL 4 threw its unavailable_feature_error).
    The angle of a point of the box Y x X is monotonic in y and in x in each
    quadrant, and its extrema are at corners of the box, which the signs of the
    bounds give. It jumps from pi to -pi across the half-line y = 0, x < 0: a
    box with points on that half-line and points below it has angles next to
    -pi and the angle pi, and [-pi, pi] is the hull of its angles.
  */
  // atan2() after its check, made before the bounds are compared: with
  // denormals-are-zero, atan2([100 2^-1074], [100 2^-1074]) was the empty
  // set, the point taken for (0, 0) (GAOL v5, see gaol/gaol_fpu.h)
  static interval atan2_upward(const interval& Y, const interval& X)
  {
    if (Y.is_empty() || X.is_empty()) {
      return interval::emptyset();
    }
    const double yl = Y.left(), yu = Y.right(), xl = X.left(), xu = X.right();
    if (yl == 0.0 && yu == 0.0 && xl == 0.0 && xu == 0.0) {
      return interval::emptyset();
    }
    if (yl < 0.0 && yu >= 0.0 && xl < 0.0) {
      return interval(-gaol_pi_up, gaol_pi_up);
    }
    double l, r;
    if (yl >= 0.0) { // Upper half-plane: the angle decreases with x
      // The least angle is at the right of the box, at the bottom if x > 0
      // there and at the top otherwise, and the greatest at the left. The box
      // {0} x [xl, 0] has no other corner there than (0, 0): its angle is pi,
      // and that of {0} x [0, xu] is 0
      l = atan2_lo((xu > 0.0) ? yl : yu, xu);
      r = (xl == 0.0 && yu == 0.0) ? 0.0 : atan2_hi((xl >= 0.0) ? yu : yl, xl);
    } else if (yu < 0.0) { // Lower half-plane, y = 0 left out: the angle increases with x
      l = atan2_lo((xl >= 0.0) ? yl : yu, xl);
      r = atan2_hi((xu > 0.0) ? yu : yl, xu);
    } else { // yl < 0 <= yu, in the right half-plane
      l = atan2_lo(yl, xl);
      r = (yu == 0.0) ? ((xu == 0.0) ? -gaol_half_pi_dn : 0.0) : atan2_hi(yu, xl);
    }
    return interval(l,r);
  }

  interval atan2(const interval& Y, const interval& X)
  {
    GAOL_RND_ENTER();
    interval res = atan2_upward(Y, X);
    GAOL_RND_KEEP(res);
    GAOL_RND_LEAVE();
    return res;
  }


  /*
    cos(I), and sin(I) as cos(I - pi/2) (template argument sine).

    The bounds of I divided by an enclosure of pi, minus 1/2 for the sine,
    rounded outward, give the pieces [m pi, n pi] on which I lies: on one
    piece (n - m < 2), the function is monotonic, and its bounds are the values
    of the mathematical library at the bounds of I. sin is computed from the
    sine of the mathematical library (GAOL v5): GAOL computed
    cos(I - [pi/2]), whose subtraction widened I by about 2^-52 max(2, |x|), and
    sin([1e-10]) was 4.4e-16 wide.

    Otherwise (GAOL v5, issue #6), I is on several pieces for sure when
    the same divisions rounded inward give the same m and n: an extremum is
    within I when n - m is 2, and both extrema are beyond. When they do not,
    a bound of I being within about |x| 2^-52 of an extremum, or beyond 2^52,
    where the quotients are integers, the divisions cannot tell, and GAOL took
    the extremum as reached: cos([2^60]) was [-1, 1]. The signs of the
    derivative at the bounds of I then tell, exactly, the mathematical library
    being correctly rounded: for I narrower than 2 pi, the derivative has at
    most two zeros within I; signs that differ at the bounds show one, a
    minimum or a maximum according to their order; the same signs show none
    for I narrower than pi, and none or two beyond, which the sign at the
    middle of I tells, each half being narrower than pi. The bounds are then
    within one double of the tightest ones at every magnitude.

    That the test w < gaol_pi_dn below gives the tightest bounds was measured,
    not proved: sin and cos gave the tightest bounds over 29 400 intervals
    of width about pi whose bounds are next to extrema of sin (GAOL v5, decided
    on 3 October).
  */
template<bool sine>
static inline interval cos_or_sin(double Il, double Ir)
{
  // I is not empty, Il being the opposite of its left bound, as the SSE2
  // intervals store it, and Ir its right bound
  // The only check of the rounding direction: nothing below changes it, and
  // the bounds are computed with the functions of namespace upward (GAOL v5).
  // Il and Ir, read before it, are compared after it (see gaol/gaol_fpu.h)
  GAOL_RND_ENTER();
  Il = rnd_reread(Il);
  Ir = rnd_reread(Ir);
  double a,b;
  double Ileft = -Il;
  double Iright = Ir;

  // a <= Ileft/pi, b >= Iright/pi
  if (Ileft >= 0) {
    a = -(Il/gaol_pi_up);
    b = Ir/gaol_pi_dn;
  } else {
    a = -(Il/gaol_pi_dn);
    if (Iright > 0) {
      b = Ir/gaol_pi_dn;
    } else {
      b = Ir/gaol_pi_up;
    }
  }
  if (sine) {
    // a <= (Ileft - pi/2)/pi, b >= (Iright - pi/2)/pi, rounded upward
    a = -((-a) + 0.5);
    b = b - 0.5;
  }

  double m=std::floor(a),
  n=std::ceil(b),
  u = -1.0, v = 1.0;

  double nm = (n-m); // Must be rounded towards +oo

  if (nm < 2.0) {
    // even(m)? No conversion to int in order to avoid overflow
    const bool even_m = integer_is_even(m);
    if (even_m) { // Decreasing, as cos on [m pi, (m+1) pi]; use of cos(x)=cos(-x)
      u = sine ? sin_lo(Iright) : cos_lo(Iright);
      v = sine ? sin_hi(Ileft) : cos_hi(Ileft);
    } else { // Increasing
      u = sine ? sin_lo(Ileft) : cos_lo(Ileft);
      v = sine ? sin_hi(Iright) : cos_hi(Iright);
    }
  } else {
    // The half of the width of I, rounded upward: both extrema from 2 pi on,
    // or for infinite bounds. The width itself, Ir + Il, overflowed for a
    // finite result, [-1, 1], when a bound was next to the largest double
    // (GAOL v5: cos([-1, DBL_MAX])); the halves of the
    // bounds are exact, and their sum is the half of the rounded sum
    const double half_width = 0.5*Ir + 0.5*Il;
    if (!(half_width < gaol_pi_dn)) {
      GAOL_RND_LEAVE();
      return interval::minus_one_plus_one();
    }
    // a_in >= Ileft/pi and b_in <= Iright/pi, minus 1/2 for the sine
    double a_in, b_in;
    if (Ileft >= 0) {
      a_in = Ileft/gaol_pi_dn;
      b_in = -((-Ir)/gaol_pi_up);
    } else {
      a_in = Ileft/gaol_pi_up;
      b_in = (Iright > 0) ? -((-Ir)/gaol_pi_up) : -((-Ir)/gaol_pi_dn);
    }
    if (sine) {
      a_in = a_in - 0.5;
      b_in = -((-b_in) + 0.5);
    }
    // The halves of I, for the sign at its middle
    const double middle = 0.5*Ileft + 0.5*Iright;
    const bool narrow_halves = (middle + Il < gaol_pi_dn) && (Ir - middle < gaol_pi_dn);
    const bool sure = (std::floor(a_in) == m && std::ceil(b_in) == n);
    if (sure && nm != 2.0) { // Both extrema
      GAOL_RND_LEAVE();
      return interval::minus_one_plus_one();
    }
    const bool even_m = integer_is_even(m);

    // -1: a minimum within I; 1: a maximum; 0: none; 2: both
    int extremum;
    if (sure) {
        extremum = even_m ? -1 : 1;
    } else {
        // The derivatives are -sin and cos: s is the sign of the slope, taken
        // on the side of I at a bound that is 0
        int sl = sine ? sign_of_cos(Ileft) : -sign_of_sin(Ileft);
        int sr = sine ? sign_of_cos(Iright) : -sign_of_sin(Iright);
        if (sl == 0) {
          sl = -1; // cos decreases on the right of 0
        }
        if (sr == 0) {
          sr = 1; // and increases on its left
        }
        if (sl != sr) {
            extremum = sl; // Increasing then decreasing: a maximum
        } else if (half_width < 0.5*gaol_pi_dn || (narrow_halves && (sine ? sign_of_cos(middle) : -sign_of_sin(middle)) == sl)) {
            extremum = 0;
        } else {
            extremum = 2;
        }
        if (extremum == 0) { // Monotonic
            if (sl < 0) {
              u = sine ? sin_lo(Iright) : cos_lo(Iright);
              v = sine ? sin_hi(Ileft) : cos_hi(Ileft);
            } else {
              u = sine ? sin_lo(Ileft) : cos_lo(Ileft);
              v = sine ? sin_hi(Iright) : cos_hi(Iright);
            }
          }
      }
      if (extremum == -1) {
          const double
          u1 = sine ? sin_hi(Ileft) : cos_hi(Ileft),
          u2 = sine ? sin_hi(Iright) : cos_hi(Iright);
          u = -1.0;
          v = ((u1 > u2) ? u1 : u2);
      } else if (extremum == 1) {
          const double
          u1 = sine ? sin_lo(Ileft) : cos_lo(Ileft),
          u2 = sine ? sin_lo(Iright) : cos_lo(Iright);
          u = ((u1 < u2) ? u1 : u2);
          v = 1.0;
      } else if (extremum == 2) {
          u = -1.0;
          v = 1.0;
      }
  }
  // The values of the mathematical library moved outward may leave [-1,1]
  // (GAOL v5)
  if (u < -1.0) {
    u = -1.0;
  }
  if (v > 1.0) {
    v = 1.0;
  }
  GAOL_RND_KEEP(u); GAOL_RND_KEEP(v);
  GAOL_RND_LEAVE();
  return interval(u,v);
}

  interval cos(const interval& I)
  {
    if (I.is_empty()) {
      return interval::emptyset();
    }
    return cos_or_sin<false>(-I.left(), I.right());
  }

  interval sin(const interval& I)
  {
    if (I.is_empty()) {
      return interval::emptyset();
    }
    return cos_or_sin<true>(-I.left(), I.right());
  }

} // namespace gaol_core
