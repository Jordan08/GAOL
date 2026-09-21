/*-*-C++-*----------------------------------------------------------------------
 * gaol -- NOT Just Another Interval Library
 *------------------------------------------------------------------------------
 * The operations of IEEE 1788-2015 under their own names: gaol_ieee1788.
 *
 * GAOL names its operations its own way: the reverse function coshRev(c, x) of
 * the standard is acosh_rel(c, x) here, mulRev(b, c, x) is div_rel(c, b, x),
 * with its arguments in another order, and roundTiesToEven(x) is
 * round_ties_to_even(x). This namespace gives each operation of the standard
 * that GAOL provides the name the standard gives it, with its arguments in the
 * order the standard puts them, so that a reader of the standard finds it
 * without looking for its translation. Each one forwards to the operation of
 * GAOL, which the comment above it names.
 *
 * The namespace is not in gaol, and it holds GAOL's type interval as well: a
 * program that adds
 *
 *   using namespace gaol_ieee1788;
 *
 * uses GAOL under the names of the standard, without naming gaol: a program
 * opens one of the two namespaces, not both. The type interval and GAOL's
 * functions are in gaol_core (gaol/gaol_interval.h), the only namespace where
 * argument-dependent lookup looks for a call sin(x) on an interval, so that
 * it finds neither gaol's functions nor these. The functions of GAOL that
 * have the name and the meaning the standard gives them (sin, exp, sqrt,
 * min...) are brought in by using-declarations of those of gaol_core, which
 * name the functions argument-dependent lookup finds too; gaol/gaol_expression.h
 * is included first, so that they take its overloads whatever the order of
 * the includes.
 *
 * Where the standard and GAOL differ, these functions follow the standard:
 *   - pow(x, y) and pow(x, p) are the pow of Table 9.1, defined for x > 0, and
 *     for x = 0 when y > 0, [p] being the exponent for a number p, an int
 *     included: the power with an integer exponent is pown(x, p) in the
 *     standard. gaol::pow takes that integer power for an integer exponent,
 *     which is defined for x < 0 too, and gives [-oo, +oo] for an integer
 *     beyond the ints. Neither pow is in gaol_core: each namespace has its own.
 *     pown(e, n) and pow(e1, e2) build expressions (gaol/gaol_expression.h)
 *     computed with the pown and the pow of the standard;
 *   - inf(x) and sup(x) are +oo and -oo for the empty set (Table 10.2), where
 *     GAOL's bounds are NaN, and inf returns -0 for a lower bound 0 (12.12.8);
 *   - isMember(m, x) is false for an infinite m (10.6.3);
 *   - textToInterval and exactToInterval return the empty set for a string
 *     that is no interval literal (12.1.3), where GAOL's constructor throws.
 *
 * Only bare intervals are provided: GAOL has no decorations (Clause 11). The
 * operations of the standard GAOL does not provide are listed at the end of
 * this file.
 *
 * Copyright (c) 2026 ENSTA, France
 *
 * Created 2026-09-21 by Jordan NININ
 *------------------------------------------------------------------------------
 * gaol is a software distributed WITHOUT ANY WARRANTY. Read the associated
 * COPYING file for information.
 *----------------------------------------------------------------------------*/

#ifndef __gaol_ieee1788_h__
#define __gaol_ieee1788_h__

#include <cmath>
#include <exception>
#include <sstream>
#include <stdexcept>
#include <string>

#include "gaol/gaol_interval.h"
#include "gaol/gaol_expression.h"

namespace gaol_ieee1788 {

  //! The interval of GAOL, the type of the operations below, but for the expressions of pown and pow
  using ::gaol_core::interval;

  /* Each function below calls the operation of GAOL by its full name,
     ::gaol_core::f: inside this namespace an unqualified f would be the
     function of the same name defined here, and call itself. */

  // ----------------------------------------------------------------------
  // Interval constants (10.5.2) and constructors (10.5.8, 12.12.7)
  // ----------------------------------------------------------------------

  //! empty(): interval::emptyset()
  inline interval empty() { return interval::emptyset(); }
  //! entire(): interval::universe()
  inline interval entire() { return interval::universe(); }

  //! numsToInterval(l, u): interval(l, u), the empty set for l > u, l = +oo, u = -oo or a NaN
  inline interval numsToInterval(double l, double u) { return interval(l, u); }

  //! textToInterval(s): interval(s), the empty set for a string that is no interval literal
  inline interval textToInterval(const std::string& s)
  {
    try {
      return interval(s.c_str());
    } catch (const std::exception&) {
      return interval::emptyset();
    }
  }

  // ----------------------------------------------------------------------
  // Forward elementary functions (Table 9.1)
  // ----------------------------------------------------------------------

  //! neg(x): -x
  inline interval neg(const interval& x) { return -x; }
  //! add(x, y): x + y
  inline interval add(const interval& x, const interval& y) { return x + y; }
  //! sub(x, y): x - y
  inline interval sub(const interval& x, const interval& y) { return x - y; }
  //! mul(x, y): x * y
  inline interval mul(const interval& x, const interval& y) { return x * y; }
  //! div(x, y): x / y
  inline interval div(const interval& x, const interval& y) { return x / y; }
  //! recip(x): inverse(x)
  inline interval recip(const interval& x) { return ::gaol_core::inverse(x); }
  //! sqr(x), sqrt(x), fma(x, y, z): the functions of GAOL
  using ::gaol_core::sqr;
  using ::gaol_core::sqrt;
  using ::gaol_core::fma;

  /*!
    pown(x, p): gaol_pown(x, p), x^p for an int p, defined for x < 0 too. The
    standard's power with an integer exponent: its p is an integer, not an
    interval (Table 9.1, footnote b), and pow has no such exponent.
  */
  inline interval pown(const interval& x, int p) { return ::gaol_core::gaol_pown(x, p); }

  /*!
    pow(x, y): the pow of IEEE 1788-2015 (Table 9.1), on the part of x in
    [0, +oo], 0^y having a value only for y > 0. gaol_pow_hybrid(x, y) is that
    pow there, but for a degenerate integer exponent [n], for which it takes
    pown: the same on x >= 0 for an n within the ints, and [-oo, +oo] beyond
    them, where pown cannot be called. x^n is monotone in x >= 0, so that its
    bounds there are CORE-MATH's pow at the bounds of x, each one double at
    most from the tightest one: pow([2, 3], [1e10]) is [DBL_MAX, +oo],
    pow([0.5, 0.9], [1e10]) is [0, 2^-1074], and pow([1], [-1e12]) is [1].
  */
  inline interval pow(const interval& x, const interval& y)
  {
    if (x.is_empty() || y.is_empty()) {
      return interval::emptyset();
    }
    const interval xp = x & interval(0.0, GAOL_INFINITY);
    if (xp.is_empty()) {
      return interval::emptyset();
    }
    if (xp.left() == 0.0 && xp.right() == 0.0) {
      // x = {0}: 0^y = 0 for y > 0, no value otherwise
      return (y.right() > 0.0) ? interval(0.0) : interval::emptyset();
    }
    const double n = y.left();
    if (n == y.right() && std::floor(n) == n && !y.is_an_int()) {
      /* |n| > 2^31: x^n increases with x for n > 0, 0^n being 0, and
         decreases for n < 0, +oo being its limit at 0; 1^n is 1. A lower
         bound 0 is taken as +0, CORE-MATH's pow(-0, n) being -oo for an odd
         n < 0. */
      const double xl = (xp.left() == 0.0) ? 0.0 : xp.left(), xu = xp.right();
      const double at_lower = (n > 0.0) ? xl : xu, at_upper = (n > 0.0) ? xu : xl;
      double l, r;
      GAOL_RND_ENTER();
      l = (at_lower == 1.0) ? 1.0 : ::gaol_core::nthroot_dn(at_lower, n);
      r = (at_upper == 1.0) ? 1.0 : ::gaol_core::nthroot_up(at_upper, n);
      GAOL_RND_LEAVE();
      return interval((l > 0.0) ? l : 0.0, r);
    }
    return ::gaol_core::gaol_pow_hybrid(xp, y);
  }
  /*!
    pow(x, p): pow(x, [p]), the pow of the standard for a double as
    exponent. An int exponent comes here too, pow(x, 2) being pow(x, [2]),
    not the integer power pown(x, 2): pow([-4, -1], 2) is the empty set here,
    and [1, 16] in gaol. Without it, pow(x, 0) would not compile, 0 converting
    to an interval through interval(double) and interval(const char*) alike.
  */
  inline interval pow(const interval& x, double p) { return pow(x, interval(p)); }

  /*!
    pown(e, n), pow(e1, e2): the expressions of pown and pow
    (gaol/gaol_expression.h). pown(e, n) is gaol_pown_exp(e, n), whose node
    is computed by gaol_pown(), the pown of the standard. The node of
    pow(e1, e2) keeps the function computing it: pow(x, y) above here, where
    gaol_pow_exp(e1, e2), which is gaol::pow(e1, e2), keeps GAOL's pow,
    [1, 16] for [-4, -1]^[2].
  */
  inline const ::gaol_core::expression pown(const ::gaol_core::expression& e, int n)
  {
    return ::gaol_core::gaol_pown_exp(e, n);
  }
  inline const ::gaol_core::expression pow(const ::gaol_core::expression& e1, const ::gaol_core::expression& e2)
  {
    const ::gaol_core::pow_itv_node::power_function standard_pow = pow;
    return *(new ::gaol_core::pow_itv_node(e1, e2, standard_pow));
  }

  //! exp(x), exp2(x), exp10(x), log(x), log2(x), log10(x): the functions of GAOL
  using ::gaol_core::exp;
  using ::gaol_core::exp2;
  using ::gaol_core::exp10;
  using ::gaol_core::log;
  using ::gaol_core::log2;
  using ::gaol_core::log10;

  //! The trigonometric and hyperbolic functions: the functions of GAOL, atan2(y, x) included
  using ::gaol_core::sin;
  using ::gaol_core::cos;
  using ::gaol_core::tan;
  using ::gaol_core::asin;
  using ::gaol_core::acos;
  using ::gaol_core::atan;
  using ::gaol_core::atan2;
  using ::gaol_core::sinh;
  using ::gaol_core::cosh;
  using ::gaol_core::tanh;
  using ::gaol_core::asinh;
  using ::gaol_core::acosh;
  using ::gaol_core::atanh;

  //! The integer functions: sign, ceil, floor and trunc of GAOL
  using ::gaol_core::sign;
  using ::gaol_core::ceil;
  using ::gaol_core::floor;
  using ::gaol_core::trunc;
  //! roundTiesToEven(x): round_ties_to_even(x)
  inline interval roundTiesToEven(const interval& x) { return ::gaol_core::round_ties_to_even(x); }
  //! roundTiesToAway(x): round_ties_to_away(x)
  inline interval roundTiesToAway(const interval& x) { return ::gaol_core::round_ties_to_away(x); }

  //! The absmax functions: abs, min and max of GAOL
  using ::gaol_core::abs;
  using ::gaol_core::min;
  using ::gaol_core::max;

  // ----------------------------------------------------------------------
  // Recommended forward functions (Table 10.5)
  // ----------------------------------------------------------------------

  //! rootn(x, q): nth_root(x, q), q may be negative
  inline interval rootn(const interval& x, int q) { return ::gaol_core::nth_root(x, q); }
  //! expm1, exp2m1, exp10m1, log2p1, log10p1 and hypot: the functions of GAOL
  using ::gaol_core::expm1;
  using ::gaol_core::exp2m1;
  using ::gaol_core::exp10m1;
  using ::gaol_core::log2p1;
  using ::gaol_core::log10p1;
  using ::gaol_core::hypot;
  //! logp1(x): log1p(x), the name of C
  inline interval logp1(const interval& x) { return ::gaol_core::log1p(x); }
  //! rSqrt(x): rsqrt(x)
  inline interval rSqrt(const interval& x) { return ::gaol_core::rsqrt(x); }
  //! sinPi(x), cosPi(x), tanPi(x), asinPi(x), acosPi(x), atanPi(x), atan2Pi(y, x):
  //! sinpi(x), cospi(x), tanpi(x), asinpi(x), acospi(x), atanpi(x), atan2pi(y, x)
  inline interval sinPi(const interval& x) { return ::gaol_core::sinpi(x); }
  inline interval cosPi(const interval& x) { return ::gaol_core::cospi(x); }
  inline interval tanPi(const interval& x) { return ::gaol_core::tanpi(x); }
  inline interval asinPi(const interval& x) { return ::gaol_core::asinpi(x); }
  inline interval acosPi(const interval& x) { return ::gaol_core::acospi(x); }
  inline interval atanPi(const interval& x) { return ::gaol_core::atanpi(x); }
  inline interval atan2Pi(const interval& y, const interval& x) { return ::gaol_core::atan2pi(y, x); }

  // ----------------------------------------------------------------------
  // Reverse functions (Table 10.1): the last argument x is optional and
  // [-oo, +oo] when absent (10.5.4)
  // ----------------------------------------------------------------------

  //! sqrRev(c, x): sqrt_rel(c, x)
  inline interval sqrRev(const interval& c, const interval& x) { return ::gaol_core::sqrt_rel(c, x); }
  inline interval sqrRev(const interval& c) { return ::gaol_core::sqrt_rel(c, interval::universe()); }
  //! absRev(c, x): invabs_rel(c, x)
  inline interval absRev(const interval& c, const interval& x) { return ::gaol_core::invabs_rel(c, x); }
  inline interval absRev(const interval& c) { return ::gaol_core::invabs_rel(c, interval::universe()); }

  /*!
    pownRev(c, x, p): nth_root_rel(c, p, x), for p >= 1. GAOL has no reverse
    of pown for p <= 0, which throws std::invalid_argument rather than give an
    interval it has not computed.
  */
  inline interval pownRev(const interval& c, const interval& x, int p)
  {
    if (p < 1) {
      throw std::invalid_argument("gaol_ieee1788::pownRev: p <= 0 is not provided by GAOL v5");
    }
    return ::gaol_core::nth_root_rel(c, static_cast<unsigned int>(p), x);
  }
  inline interval pownRev(const interval& c, int p) { return pownRev(c, interval::universe(), p); }

  //! sinRev(c, x), cosRev(c, x), tanRev(c, x): asin_rel(c, x), acos_rel(c, x), atan_rel(c, x)
  inline interval sinRev(const interval& c, const interval& x) { return ::gaol_core::asin_rel(c, x); }
  inline interval sinRev(const interval& c) { return ::gaol_core::asin_rel(c, interval::universe()); }
  inline interval cosRev(const interval& c, const interval& x) { return ::gaol_core::acos_rel(c, x); }
  inline interval cosRev(const interval& c) { return ::gaol_core::acos_rel(c, interval::universe()); }
  inline interval tanRev(const interval& c, const interval& x) { return ::gaol_core::atan_rel(c, x); }
  inline interval tanRev(const interval& c) { return ::gaol_core::atan_rel(c, interval::universe()); }
  //! coshRev(c, x): acosh_rel(c, x)
  inline interval coshRev(const interval& c, const interval& x) { return ::gaol_core::acosh_rel(c, x); }
  inline interval coshRev(const interval& c) { return ::gaol_core::acosh_rel(c, interval::universe()); }
  //! sinhRev(c, x), tanhRev(c, x): asinh_rel(c, x), atanh_rel(c, x); not in
  //! Table 10.1, sinh and tanh being one-to-one, but named after coshRev
  inline interval sinhRev(const interval& c, const interval& x) { return ::gaol_core::asinh_rel(c, x); }
  inline interval sinhRev(const interval& c) { return ::gaol_core::asinh_rel(c, interval::universe()); }
  inline interval tanhRev(const interval& c, const interval& x) { return ::gaol_core::atanh_rel(c, x); }
  inline interval tanhRev(const interval& c) { return ::gaol_core::atanh_rel(c, interval::universe()); }
  //! mulRev(b, c, x): div_rel(c, b, x), the arguments in another order
  inline interval mulRev(const interval& b, const interval& c, const interval& x)
  {
    return ::gaol_core::div_rel(c, b, x);
  }
  inline interval mulRev(const interval& b, const interval& c)
  {
    return ::gaol_core::div_rel(c, b, interval::universe());
  }

  // ----------------------------------------------------------------------
  // Cancellative addition and subtraction (10.5.6), set operations (10.5.7)
  // ----------------------------------------------------------------------

  //! cancelMinus(x, y), cancelPlus(x, y): cancel_minus(x, y), cancel_plus(x, y)
  inline interval cancelMinus(const interval& x, const interval& y) { return ::gaol_core::cancel_minus(x, y); }
  inline interval cancelPlus(const interval& x, const interval& y) { return ::gaol_core::cancel_plus(x, y); }
  //! intersection(x, y): x & y
  inline interval intersection(const interval& x, const interval& y) { return x & y; }
  //! convexHull(x, y): x | y
  inline interval convexHull(const interval& x, const interval& y) { return x | y; }

  // ----------------------------------------------------------------------
  // Numeric functions of intervals (Table 10.2, 12.12.8)
  // ----------------------------------------------------------------------

  //! inf(x): left(); +oo for the empty set, and -0 for a lower bound 0
  inline double inf(const interval& x)
  {
    if (x.is_empty()) {
      return GAOL_INFINITY;
    }
    return (x.left() == 0.0) ? -0.0 : x.left();
  }
  //! sup(x): right(); -oo for the empty set, and +0 for an upper bound 0
  inline double sup(const interval& x)
  {
    if (x.is_empty()) {
      return -GAOL_INFINITY;
    }
    return (x.right() == 0.0) ? 0.0 : x.right();
  }
  //! mid(x): midpoint(); NaN for the empty set, 0 for [-oo, +oo]
  inline double mid(const interval& x) { return x.midpoint(); }
  //! wid(x): width(); NaN for the empty set
  inline double wid(const interval& x) { return x.width(); }
  //! rad(x): rad(); NaN for the empty set, +oo for an unbounded interval
  inline double rad(const interval& x) { return x.rad(); }
  //! mag(x), mig(x): mag(), mig(); NaN for the empty set
  inline double mag(const interval& x) { return x.mag(); }
  inline double mig(const interval& x) { return x.mig(); }
  //! midRad(x, m, r): mid_rad(m, r), mid and rad at once (10.5.9)
  inline void midRad(const interval& x, double& m, double& r) { x.mid_rad(m, r); }

  // ----------------------------------------------------------------------
  // Boolean functions of intervals (10.5.10, Tables 10.3 and 10.4; 10.6.3)
  // ----------------------------------------------------------------------

  inline bool isEmpty(const interval& x) { return x.is_empty(); }
  inline bool isEntire(const interval& x) { return x.is_entire(); }
  //! equal(a, b): a.set_eq(b)
  inline bool equal(const interval& a, const interval& b) { return a.set_eq(b); }
  //! subset(a, b): a.set_leq(b), a in b
  inline bool subset(const interval& a, const interval& b) { return a.set_leq(b); }
  //! less(a, b), strictLess(a, b): a.less(b), a.strictly_less(b)
  inline bool less(const interval& a, const interval& b) { return a.less(b); }
  inline bool strictLess(const interval& a, const interval& b) { return a.strictly_less(b); }
  //! precedes(a, b), strictPrecedes(a, b): a.certainly_leq(b), a.certainly_le(b)
  inline bool precedes(const interval& a, const interval& b) { return a.certainly_leq(b); }
  inline bool strictPrecedes(const interval& a, const interval& b) { return a.certainly_le(b); }
  //! interior(a, b): a.set_le(b), a interior to b
  inline bool interior(const interval& a, const interval& b) { return a.set_le(b); }
  //! disjoint(a, b): a.set_disjoint(b)
  inline bool disjoint(const interval& a, const interval& b) { return a.set_disjoint(b); }
  //! isCommonInterval(x): is_common_interval(), nonempty and bounded
  inline bool isCommonInterval(const interval& x) { return x.is_common_interval(); }
  //! isSingleton(x): is_a_double()
  inline bool isSingleton(const interval& x) { return x.is_a_double(); }
  //! isMember(m, x): set_contains(m), false for an infinite or NaN m
  inline bool isMember(double m, const interval& x)
  {
    return std::isfinite(m) && x.set_contains(m);
  }

  // ----------------------------------------------------------------------
  // Input and output (Clause 13)
  // ----------------------------------------------------------------------

  //! intervalToText(x): operator<< in the current format of GAOL
  inline std::string intervalToText(const interval& x)
  {
    std::ostringstream s;
    s << x;
    return s.str();
  }

  /*!
    intervalToExact(x): operator<< in interval_format::hexa, whose bounds in
    the hexadecimal-significand form exactToInterval() reads back bit for bit
    (13.4). The format GAOL was in is set back.
  */
  inline std::string intervalToExact(const interval& x)
  {
    const ::gaol_core::interval_format::format_t saved = interval::format();
    interval::format(::gaol_core::interval_format::hexa);
    std::ostringstream s;
    s << x;
    interval::format(saved);
    return s.str();
  }

  //! exactToInterval(s): textToInterval(s)
  inline interval exactToInterval(const std::string& s) { return textToInterval(s); }

  /*
    Not provided, GAOL having no such operation:
      - the decorations and every decorated operation (Clause 11, 12.12.11);
      - mulRevToPair (10.5.5), the two-output division;
      - powRev1, powRev2, atan2Rev1, atan2Rev2 (Table 10.1), and pownRev for
        p <= 0;
      - compoundm1 of Table 10.5, which CORE-MATH has not;
      - the slope functions (Table 10.6) and overlap (10.6.4);
      - the reduction operations sum, dot, sumSquare and sumAbs (12.12.12),
        and the exact ones of 12.13.5.
  */

} // namespace gaol_ieee1788

#endif /* __gaol_ieee1788_h__ */
