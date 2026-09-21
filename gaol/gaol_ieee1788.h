/*-*-C++-*----------------------------------------------------------------------
 * gaol -- NOT Just Another Interval Library
 *------------------------------------------------------------------------------
 * The operations of IEEE 1788-2015 under their own names: gaol::ieee1788.
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
 * Where the standard and GAOL differ, these functions follow the standard:
 *   - pow(x, y) is the pow of Table 9.1, defined for x > 0, and for x = 0 when
 *     y > 0; GAOL's pow takes the integer power pown for a degenerate integer
 *     exponent, which is defined for x < 0 too;
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

namespace gaol {
namespace ieee1788 {

  /* Each function below calls the operation of GAOL by its full name,
     ::gaol::f: inside this namespace an unqualified f would be the function
     of the same name defined here, and call itself. */

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
  inline interval recip(const interval& x) { return ::gaol::inverse(x); }
  //! sqr(x): sqr(x)
  inline interval sqr(const interval& x) { return ::gaol::sqr(x); }
  //! sqrt(x): sqrt(x)
  inline interval sqrt(const interval& x) { return ::gaol::sqrt(x); }
  //! fma(x, y, z): fma(x, y, z)
  inline interval fma(const interval& x, const interval& y, const interval& z)
  {
    return ::gaol::fma(x, y, z);
  }

  //! pown(x, p): pow(x, p) with an int p
  inline interval pown(const interval& x, int p) { return ::gaol::pow(x, p); }

  /*!
    pow(x, y): the pow of IEEE 1788-2015, on the part of x in [0, +oo], 0^y
    having a value only for y > 0. GAOL's pow(x, y) is that pow there, but
    for 0^y with y <= 0 and a degenerate integer y, where it takes pown.
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
    return ::gaol::pow(xp, y);
  }

  //! exp(x), exp2(x), exp10(x), log(x), log2(x), log10(x): the functions of the same names
  inline interval exp(const interval& x) { return ::gaol::exp(x); }
  inline interval exp2(const interval& x) { return ::gaol::exp2(x); }
  inline interval exp10(const interval& x) { return ::gaol::exp10(x); }
  inline interval log(const interval& x) { return ::gaol::log(x); }
  inline interval log2(const interval& x) { return ::gaol::log2(x); }
  inline interval log10(const interval& x) { return ::gaol::log10(x); }

  //! The trigonometric and hyperbolic functions: the functions of the same names
  inline interval sin(const interval& x) { return ::gaol::sin(x); }
  inline interval cos(const interval& x) { return ::gaol::cos(x); }
  inline interval tan(const interval& x) { return ::gaol::tan(x); }
  inline interval asin(const interval& x) { return ::gaol::asin(x); }
  inline interval acos(const interval& x) { return ::gaol::acos(x); }
  inline interval atan(const interval& x) { return ::gaol::atan(x); }
  inline interval atan2(const interval& y, const interval& x) { return ::gaol::atan2(y, x); }
  inline interval sinh(const interval& x) { return ::gaol::sinh(x); }
  inline interval cosh(const interval& x) { return ::gaol::cosh(x); }
  inline interval tanh(const interval& x) { return ::gaol::tanh(x); }
  inline interval asinh(const interval& x) { return ::gaol::asinh(x); }
  inline interval acosh(const interval& x) { return ::gaol::acosh(x); }
  inline interval atanh(const interval& x) { return ::gaol::atanh(x); }

  //! The integer functions
  inline interval sign(const interval& x) { return ::gaol::sign(x); }
  inline interval ceil(const interval& x) { return ::gaol::ceil(x); }
  inline interval floor(const interval& x) { return ::gaol::floor(x); }
  inline interval trunc(const interval& x) { return ::gaol::trunc(x); }
  //! roundTiesToEven(x): round_ties_to_even(x)
  inline interval roundTiesToEven(const interval& x) { return ::gaol::round_ties_to_even(x); }
  //! roundTiesToAway(x): round_ties_to_away(x)
  inline interval roundTiesToAway(const interval& x) { return ::gaol::round_ties_to_away(x); }

  //! The absmax functions; min and max in parentheses, which a macro min or max cannot take
  inline interval abs(const interval& x) { return ::gaol::abs(x); }
  inline interval (min)(const interval& x, const interval& y) { return (::gaol::min)(x, y); }
  inline interval (max)(const interval& x, const interval& y) { return (::gaol::max)(x, y); }

  // ----------------------------------------------------------------------
  // Recommended forward functions (Table 10.5)
  // ----------------------------------------------------------------------

  //! rootn(x, q): nth_root(x, q), q may be negative
  inline interval rootn(const interval& x, int q) { return ::gaol::nth_root(x, q); }
  inline interval expm1(const interval& x) { return ::gaol::expm1(x); }
  inline interval exp2m1(const interval& x) { return ::gaol::exp2m1(x); }
  inline interval exp10m1(const interval& x) { return ::gaol::exp10m1(x); }
  //! sinPi(x), cosPi(x), tanPi(x), atanPi(x), acosPi(x): sinpi(x), cospi(x), tanpi(x), atanpi(x), acospi(x)
  inline interval sinPi(const interval& x) { return ::gaol::sinpi(x); }
  inline interval cosPi(const interval& x) { return ::gaol::cospi(x); }
  inline interval tanPi(const interval& x) { return ::gaol::tanpi(x); }
  inline interval atanPi(const interval& x) { return ::gaol::atanpi(x); }
  inline interval acosPi(const interval& x) { return ::gaol::acospi(x); }

  // ----------------------------------------------------------------------
  // Reverse functions (Table 10.1): the last argument x is optional and
  // [-oo, +oo] when absent (10.5.4)
  // ----------------------------------------------------------------------

  //! sqrRev(c, x): sqrt_rel(c, x)
  inline interval sqrRev(const interval& c, const interval& x) { return ::gaol::sqrt_rel(c, x); }
  inline interval sqrRev(const interval& c) { return ::gaol::sqrt_rel(c, interval::universe()); }
  //! absRev(c, x): invabs_rel(c, x)
  inline interval absRev(const interval& c, const interval& x) { return ::gaol::invabs_rel(c, x); }
  inline interval absRev(const interval& c) { return ::gaol::invabs_rel(c, interval::universe()); }

  /*!
    pownRev(c, x, p): nth_root_rel(c, p, x), for p >= 1. GAOL has no reverse
    of pown for p <= 0, which throws std::invalid_argument rather than give an
    interval it has not computed.
  */
  inline interval pownRev(const interval& c, const interval& x, int p)
  {
    if (p < 1) {
      throw std::invalid_argument("gaol::ieee1788::pownRev: p <= 0 is not provided by GAOL v5");
    }
    return ::gaol::nth_root_rel(c, static_cast<unsigned int>(p), x);
  }
  inline interval pownRev(const interval& c, int p) { return pownRev(c, interval::universe(), p); }

  //! sinRev(c, x), cosRev(c, x), tanRev(c, x): asin_rel(c, x), acos_rel(c, x), atan_rel(c, x)
  inline interval sinRev(const interval& c, const interval& x) { return ::gaol::asin_rel(c, x); }
  inline interval sinRev(const interval& c) { return ::gaol::asin_rel(c, interval::universe()); }
  inline interval cosRev(const interval& c, const interval& x) { return ::gaol::acos_rel(c, x); }
  inline interval cosRev(const interval& c) { return ::gaol::acos_rel(c, interval::universe()); }
  inline interval tanRev(const interval& c, const interval& x) { return ::gaol::atan_rel(c, x); }
  inline interval tanRev(const interval& c) { return ::gaol::atan_rel(c, interval::universe()); }
  //! coshRev(c, x): acosh_rel(c, x)
  inline interval coshRev(const interval& c, const interval& x) { return ::gaol::acosh_rel(c, x); }
  inline interval coshRev(const interval& c) { return ::gaol::acosh_rel(c, interval::universe()); }
  //! mulRev(b, c, x): div_rel(c, b, x), the arguments in another order
  inline interval mulRev(const interval& b, const interval& c, const interval& x)
  {
    return ::gaol::div_rel(c, b, x);
  }
  inline interval mulRev(const interval& b, const interval& c)
  {
    return ::gaol::div_rel(c, b, interval::universe());
  }

  // ----------------------------------------------------------------------
  // Cancellative addition and subtraction (10.5.6), set operations (10.5.7)
  // ----------------------------------------------------------------------

  //! cancelMinus(x, y), cancelPlus(x, y): cancel_minus(x, y), cancel_plus(x, y)
  inline interval cancelMinus(const interval& x, const interval& y) { return ::gaol::cancel_minus(x, y); }
  inline interval cancelPlus(const interval& x, const interval& y) { return ::gaol::cancel_plus(x, y); }
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
    const interval_format::format_t saved = interval::format();
    interval::format(interval_format::hexa);
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
      - of Table 10.5: logp1, log2p1, log10p1, hypot, rSqrt, asinPi and
        atan2Pi, whose CORE-MATH sources need the 128-bit integer of
        gaol/gaol_u128.h first, and compoundm1, which CORE-MATH has not;
      - the slope functions (Table 10.6) and overlap (10.6.4);
      - the reduction operations sum, dot, sumSquare and sumAbs (12.12.12),
        and the exact ones of 12.13.5.
  */

} // namespace ieee1788
} // namespace gaol

#endif /* __gaol_ieee1788_h__ */
