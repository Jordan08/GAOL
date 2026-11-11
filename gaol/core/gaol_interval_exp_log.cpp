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
  \file   gaol_interval_exp_log.cpp
  \brief  The exponentials and the logarithms of the intervals: exp and log, in bases e, 2
          and 10, and their forms near 0 (expm1, exp2m1, exp10m1, log1p, log2p1, log10p1).

  \author Goualard Frederic, NININ Jordan
  \date   2001-09-28
*/

#include "gaol/core/gaol_interval_internal.h"

/*
  exp, log, exp2, exp10, log2, log10, expm1, exp2m1, exp10m1, log1p, log2p1
  and log10p1 (GAOL v5: a part of gaol_interval_core.cpp, see
  gaol_interval_internal.h).
*/

namespace gaol_core {

  /*
    The exponentials and the logarithms in base 2 and 10 (GAOL v5)

    IEEE 1788-2015 requires them among the forward elementary functions
    (Table 9.1: "exp, exp2, exp10(x) = b^x" on R with range (0, +oo), and
    "log, log2, log10(x) = log_b(x)" on (0, +oo), note d saying b = e, 2 or
    10), and GAOL did not provide them. CORE-MATH computes them correctly
    rounded in the rounding direction in effect, so, in the upward rounding
    GAOL keeps, the value at the right bound is the upper bound and the double
    below the value at the left bound the lower one -- unless that value is
    itself a double, and the tests below tell when it is. The bounds are then
    the tightest ones, as they are for exp and log.
  */

  // 2^x is a double exactly when x is an integer of [-1074, 1023]: 2^-1074 is
  // the smallest subnormal and 2^1024 overflows
  static inline bool exp2_is_exact(double x)
  {
    return x == std::floor(x) && x >= -1074.0 && x <= 1023.0;
  }

  // 10^x is a double exactly when x is an integer of [0, 22]: 10^23 is not a
  // double, and no negative power of ten is (0.1 is not one)
  static inline bool exp10_is_exact(double x)
  {
    return x == std::floor(x) && x >= 0.0 && x <= 22.0;
  }

  // log2(x) is a double exactly when x is a power of two, which frexp tells:
  // it writes x as m*2^e with m in [1/2, 1), so m is 1/2 for a power of two
  // and nothing else (and 0 for zero, inf for an infinity)
  static inline bool log2_is_exact(double x)
  {
    int e;
    return std::frexp(x, &e) == 0.5;
  }

  // log10(x) is a double exactly when x is a power of ten with an exponent in
  // [0, 22], the powers of ten that are doubles. The value rounded upward is
  // an integer of that range for those x, and might be for others, which
  // raising ten to it settles: 10^n is exact there.
  static inline bool log10_is_exact(double x)
  {
    if (!(x > 0.0) || !is_finite(x)) {
      return false;
    }
    const double n = gaol_cr_log10(x);
    if (n != std::floor(n) || n < 0.0 || n > 22.0) {
      return false;
    }
    return gaol_cr_exp10(n) == x;
  }

  interval exp2(const interval& I)
  {
    if (I.is_empty()) {
      return interval::emptyset();
    }
    // The check before the bounds are read, and the maximum taken before
    // GAOL_RND_LEAVE(), as exp() does (see gaol/gaol_fpu.h)
    GAOL_RND_ENTER();
    const double l = I.left(), r = I.right();
    const double w = gaol_cr_exp2(l);
    const double u = exp2_is_exact(l) ? w : previous_float(w);
    const double v = gaol_cr_exp2(r);
    // Within [0, +oo], as exp: 2^x is positive
    interval res(maximum(0.0, u), v);
    GAOL_RND_KEEP(res);
    GAOL_RND_LEAVE();
    return res;
  }

  interval exp10(const interval& I)
  {
    if (I.is_empty()) {
      return interval::emptyset();
    }
    GAOL_RND_ENTER();
    const double l = I.left(), r = I.right();
    const double w = gaol_cr_exp10(l);
    const double u = exp10_is_exact(l) ? w : previous_float(w);
    const double v = gaol_cr_exp10(r);
    interval res(maximum(0.0, u), v);
    GAOL_RND_KEEP(res);
    GAOL_RND_LEAVE();
    return res;
  }

  interval log2(const interval& I)
  {
    // Defined on (0, +oo), as log: I holding no positive number gives the
    // empty set (IEEE 1788-2015, Table 9.1), the bounds being compared after
    // the check, as in log()
    if (I.is_empty()) {
      return interval::emptyset();
    }
    GAOL_RND_ENTER();
    if (!(I.right() > 0.0)) {
      GAOL_RND_LEAVE();
      return interval::emptyset();
    }
    const double l = maximum(0.0, I.left()), r = I.right();
    const double w = gaol_cr_log2(l);
    const double u = log2_is_exact(l) ? w : previous_float(w);
    const double v = gaol_cr_log2(r);
    GAOL_RND_LEAVE();
    return interval(u, v);
  }

  interval log10(const interval& I)
  {
    if (I.is_empty()) {
      return interval::emptyset();
    }
    GAOL_RND_ENTER();
    if (!(I.right() > 0.0)) {
      GAOL_RND_LEAVE();
      return interval::emptyset();
    }
    const double l = maximum(0.0, I.left()), r = I.right();
    const double w = gaol_cr_log10(l);
    const double u = log10_is_exact(l) ? w : previous_float(w);
    const double v = gaol_cr_log10(r);
    GAOL_RND_LEAVE();
    return interval(u, v);
  }

  /*
    The forward functions IEEE 1788-2015 recommends (Table 10.5) (GAOL v5)

    expm1, exp2m1 and exp10m1 compute b^x - 1 without the cancellation of
    subtracting 1 from b^x near 0, and atanpi, acospi, sinpi, cospi and tanpi a
    function of pi*x, or one divided by pi, without the loss of accuracy of pi
    being irrational (notes c and e of the table). CORE-MATH computes each of
    them correctly rounded in the rounding direction in effect, as it does
    exp2, so the bounds are got the same way: in the upward rounding GAOL keeps,
    the value at a bound rounded upward, and the double below it for a lower
    bound unless the value is itself a double, which the tests below tell.
  */

  // b^x - 1 is a double exactly at 0, and for b = 2 at the integers of
  // [-53, 53], for b = 10 at those of [0, 15]: 2^n - 1 has n significant bits,
  // 1 - 2^-n as many, and 10^n - 1 is below 2^53 up to n = 15
  static inline bool expm1_is_exact(double x)
  {
    return x == 0.0;
  }

  static inline bool exp2m1_is_exact(double x)
  {
    return x == std::floor(x) && x >= -53.0 && x <= 53.0;
  }

  static inline bool exp10m1_is_exact(double x)
  {
    return x == std::floor(x) && x >= 0.0 && x <= 15.0;
  }

  // Within [-1, +oo]: b^x - 1 exceeds -1, which it nears as x goes to -oo.
  // +oo is read after the check: GAOL_INFINITY was the HUGE_VAL of the UCRT,
  // which clang-cl computes when the program runs, and it was FLT_MAX in the
  // caller's downward rounding, which made expm1([1e10]) empty (GAOL v5)
  static interval increasing_cr_from_minus_one(const interval& I, double (*f)(double),
                                               bool (*exact)(double))
  {
    GAOL_RND_ENTER();
    interval res = increasing_cr_upward(I, f, exact, -1.0, GAOL_INFINITY);
    GAOL_RND_KEEP(res);
    GAOL_RND_LEAVE();
    return res;
  }

  interval expm1(const interval& I)
  {
    return increasing_cr_from_minus_one(I, gaol_cr_expm1, expm1_is_exact);
  }

  interval exp2m1(const interval& I)
  {
    return increasing_cr_from_minus_one(I, gaol_cr_exp2m1, exp2m1_is_exact);
  }

  interval exp10m1(const interval& I)
  {
    return increasing_cr_from_minus_one(I, gaol_cr_exp10m1, exp10m1_is_exact);
  }

  /*
    log1p, log2p1 and log10p1, the logp1, log2p1 and log10p1 of IEEE 1788-2015
    (Table 10.5): log_b(1 + x) on (-1, +oo), without the cancellation of
    adding 1 to x near 0. They are increasing and bounded as expm1 is; the
    part of I at or below -1 is left out, and the value at -1 is the limit -oo.
  */

  // log(1 + x) is rational at a rational x only for x = 0, where it is 0: the
  // logarithm of a rational other than 1 is transcendental (Lindemann)
  static inline bool log1p_is_exact(double x)
  {
    return x == 0.0 || x == -1.0 || x == GAOL_INFINITY;
  }

  // log2(1 + x) is a double exactly where 1 + x is a power of two 2^k: then
  // x = 2^k - 1, a double for k in [0, 53], the integers below 2^53 whose
  // successor is a power of two, and for k in [-53, -1], in (-1, -1/2] where
  // 1 + x is computed exactly (Sterbenz). A logarithm of a rational that is
  // not a power of two is irrational.
  static inline bool log2p1_is_exact(double x)
  {
    if (x == -1.0 || x == GAOL_INFINITY) {
      return true;
    }
    if (x >= 0.0 && x < 9007199254740992.0 && x == std::floor(x)) { // 2^53
      const std::uint64_t n = static_cast<std::uint64_t>(x);
      return (n & (n + 1)) == 0;
    }
    if (x > -1.0 && x <= -0.5) {
      int e;
      return std::frexp(1.0 + x, &e) == 0.5;
    }
    return false;
  }

  // log10(1 + x) is a double exactly where 1 + x is a power of ten 10^k with
  // k >= 0, the others being no doubles: x = 10^k - 1 for k in [0, 15],
  // 10^16 - 1 being above 2^53
  static inline bool log10p1_is_exact(double x)
  {
    if (x == -1.0 || x == GAOL_INFINITY) {
      return true;
    }
    if (!(x >= 0.0 && x < 1e15 && x == std::floor(x))) {
      return false;
    }
    double p = 1.0; // 10^k, exact up to 10^15
    for (int k = 0; k <= 15; ++k, p *= 10.0) {
      if (x == p - 1.0) {
        return true;
      }
    }
    return false;
  }

  // The check before the interval cut at -1 is made (see increasing_cr())
  static interval log_p1(const interval& I, double (*f)(double), bool (*exact)(double))
  {
    if (I.is_empty()) {
      return interval::emptyset();
    }
    GAOL_RND_ENTER();
    interval res = !(I.right() > -1.0) ? interval::emptyset()
      : increasing_cr_upward(interval(maximum(-1.0, I.left()), I.right()), f, exact,
                             -GAOL_INFINITY, GAOL_INFINITY);
    GAOL_RND_KEEP(res);
    GAOL_RND_LEAVE();
    return res;
  }

  interval log1p(const interval& I)
  {
    return log_p1(I, gaol_cr_log1p, log1p_is_exact);
  }

  interval log2p1(const interval& I)
  {
    return log_p1(I, gaol_cr_log2p1, log2p1_is_exact);
  }

  interval log10p1(const interval& I)
  {
    return log_p1(I, gaol_cr_log10p1, log10p1_is_exact);
  }

  interval exp(const interval& I)
  {
    // The empty set tested first: its NaN bounds would give the empty set
    // too, through the quiet comparison of the constructor, but after the
    // check of the rounding direction and two calls of CORE-MATH (GAOL v5)
    if (I.is_empty()) {
      return interval::emptyset();
    }
	/* We intersect the result with [0, +oo] to ensure that the result is strictly positive
		Otherwise, we might have: exp([-oo, -MAX] = [-v, +v] with v very small.
	*/
    // exp(0) = 1 exactly, where the value of the mathematical library moved
    // outward gave exp([0]) a width, and pow([1], [-oo, +oo]) = [0, +oo]
    // (GAOL v5). The bounds read after the check (see gaol/gaol_fpu.h)
    GAOL_RND_ENTER();
    const double l = I.left(), r = I.right();
    // The maximum taken before GAOL_RND_LEAVE(): with GAOL_PRESERVE_ROUNDING,
    // the denormals-are-zero mode it restores made the subnormal lower bound of
    // exp([-740]) equal to 0, and 0 the bound (GAOL v5)
    double u = (l == 0.0) ? 1.0 : maximum(0.0,upward::exp_dn(l));
    double v = (r == 0.0) ? 1.0 : upward::exp_up(r);
    GAOL_RND_KEEP(u); GAOL_RND_KEEP(v);
    GAOL_RND_LEAVE();
    return interval(u, v);
  }

  interval log(const interval& I)
  {
    // log is defined on (0,+oo) (IEEE 1788-2015, Table 9.1, GAOL v5): I
    // holding no positive number, as [-4,0] and [0], gives the empty set, where
    // GAOL kept its part in [0,+oo] and gave [-oo,-MAX]. The bounds compared
    // after the check: with denormals-are-zero, [1e-310, 1e-309] held no
    // positive number (GAOL v5)
    if (I.is_empty()) {
      return interval::emptyset();
    }
    GAOL_RND_ENTER();
    if (!(I.right() > 0.0)) {
      GAOL_RND_LEAVE();
      return interval::emptyset();
    }

    // log(1) = 0 exactly: log([1]) was [-2^-1074, 2^-1074] (GAOL v5)
    const double l = maximum(0.0,I.left()), r = I.right();
    // CORE-MATH's log, correctly rounded in the rounding direction in effect
    // (GAOL v5): in the upward rounding GAOL computes in, RU(log r) is the
    // right bound, and RD(log l) = pred(RU(log l)) the left one, log(l) being
    // no double for l other than 1. The tightest bounds, without switching the
    // rounding direction.
    const double u = (l == 1.0) ? 0.0 : previous_float(gaol_cr_log(l));
    const double v = (r == 1.0) ? 0.0 : gaol_cr_log(r);
    GAOL_RND_LEAVE();
    return interval(u, v);
  }

} // namespace gaol_core
