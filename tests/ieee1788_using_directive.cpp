/*-*-C++-*------------------------------------------------------------------
 * gaol -- NOT Just Another Interval Library
 *--------------------------------------------------------------------------
 * Tests of GAOL v5: gaol_ieee1788 as a program uses it, with
 * using namespace gaol_ieee1788 and without using namespace gaol.
 *
 * A program opens one of the two namespaces. The unqualified calls below
 * compile only if none of them is ambiguous with a function that
 * argument-dependent lookup finds on an interval, in gaol_core: sin, min and
 * the others brought in by using-declarations are gaol_core's own, and pow,
 * which is in gaol_ieee1788 and in gaol but not in gaol_core, is the
 * standard's, for an interval or a number as exponent: pow(x, 2) is
 * pow(x, [2]), the integer power being pown(x, 2).
 *
 * Copyright (c) 2026 ENSTA, France
 *
 * Created 2026-09-21 by Jordan NININ
 *--------------------------------------------------------------------------
 * gaol is a software distributed WITHOUT ANY WARRANTY. Read the associated
 * COPYING file for information.
 *--------------------------------------------------------------------------*/

#include "gaol_tests.h"
#include "gaol/gaol_expr_eval.h"

using namespace gaol_ieee1788;
using gaol_tests::check;

namespace
{
  // The value of an expression, which the evaluator of GAOL computes
  interval value_of(const gaol::expression& e)
  {
    gaol::expr_eval eval;
    e.get_root()->accept(eval);
    return eval.result();
  }

  void ieee1788_using_directive()
  {
    const interval x = textToInterval("[0.25, 0.5]"), y = numsToInterval(-1.0, 2.0);
    const interval forward[] = {
      neg(x), add(x, y), sub(x, y), mul(x, y), div(x, y), recip(x), sqr(y), sqrt(x), fma(x, y, x),
      pown(y, 3), pow(x, y), pow(x, 3), pow(x, 0.5), exp(x), exp2(x), exp10(x), log(x), log2(x),
      log10(x), sin(x), cos(x), tan(x), asin(x), acos(x), atan(x), atan2(y, x), sinh(x), cosh(x),
      tanh(x), asinh(x), acosh(y), atanh(x), sign(y), ceil(y), floor(y), trunc(y),
      roundTiesToEven(y), roundTiesToAway(y), abs(y), min(x, y), max(x, y), rootn(x, -3), expm1(x),
      exp2m1(x), exp10m1(x), logp1(x), log2p1(x), log10p1(x), hypot(x, y), rSqrt(x), sinPi(x),
      cosPi(x), tanPi(x), asinPi(x), acosPi(x), atanPi(x), atan2Pi(y, x),
    };
    const interval reverse[] = {
      sqrRev(x), sqrRev(x, y), absRev(x), pownRev(x, 3), sinRev(x), cosRev(x), tanRev(x),
      coshRev(y), mulRev(x, y), cancelMinus(y, x), cancelPlus(y, x), intersection(x, y),
      convexHull(x, y), empty(), entire(),
    };
    double m, r;
    midRad(x, m, r);
    const bool b = isEmpty(empty()) && isEntire(entire()) && equal(x, x) && subset(x, y)
      && less(x, x) && !strictLess(x, x) && !precedes(y, x) && !strictPrecedes(x, y)
      && interior(x, y) && !disjoint(x, y) && isCommonInterval(x) && !isSingleton(x)
      && isMember(0.3, x);
    check("using namespace gaol_ieee1788: sin, min, hypot... are GAOL's",
          sin(x).set_eq(gaol::sin(x)) && min(x, y).set_eq(gaol::min(x, y))
          && hypot(x, y).set_eq(gaol::hypot(x, y)) && sinPi(x).set_eq(gaol::sinpi(x)),
          [] { return std::string(); });
    const interval neg_base(-4.0, -1.0), two(2.0);
    check("using namespace gaol_ieee1788: pow(x, y) is the pow of Table 9.1, not gaol::pow",
          pow(neg_base, two).is_empty() && gaol::pow(neg_base, two).set_eq(interval(1.0, 16.0))
          && pow(x, y).set_eq(gaol_ieee1788::pow(x, y)), [] { return std::string(); });
    // With a number as exponent, the pow of gaol was found here too, and gave
    // [1, 16] for pow([-4, -1], 2) and pow([-4, -1], 2.0), and [1] for pow([0], 0)
    check("using namespace gaol_ieee1788: pow(x, 2), pow(x, 2.0) and pow([0], 0) are the pow of Table 9.1",
          pow(neg_base, 2).is_empty() && pow(neg_base, 2.0).is_empty() && pow(interval(0.0), 0).is_empty()
          && gaol::pow(neg_base, 2).set_eq(interval(1.0, 16.0)) && gaol::pow(neg_base, 2.0).set_eq(interval(1.0, 16.0)),
          [&] { return gaol_tests::hex(pow(neg_base, 2)) + " " + gaol_tests::hex(pow(interval(0.0), 0)); });
    check("using namespace gaol_ieee1788: pown(x, 2) is the integer power, [1, 16] for x = [-4, -1]",
          pown(neg_base, 2).set_eq(interval(1.0, 16.0)) && pown(interval(0.0), 0).set_eq(interval(1.0)),
          [&] { return gaol_tests::hex(pown(neg_base, 2)); });
    // The expressions of pow and pown, evaluated: pow(e1, e2) is the pow of
    // the standard, where the node of gaol::pow(e1, e2) computes GAOL's
    {
      const gaol::expression base(neg_base), exponent(two);
      const interval std_pow = value_of(pow(base, exponent)), std_pown = value_of(pown(base, 2));
      const interval gaol_pow = value_of(gaol::pow(base, exponent));
      check("using namespace gaol_ieee1788: pow(e1, e2) and pown(e, n) on expressions are those of the standard",
            std_pow.is_empty() && std_pown.set_eq(interval(1.0, 16.0)) && gaol_pow.set_eq(interval(1.0, 16.0)),
            [&] { return gaol_tests::hex(std_pow) + " " + gaol_tests::hex(std_pown) + " " + gaol_tests::hex(gaol_pow); });
    }
    check("using namespace gaol_ieee1788: the functions of the standard's names",
          b && inf(x) == 0.25 && sup(x) == 0.5 && m == mid(x) && r == rad(x) && wid(x) == 0.25
          && mag(y) == 2.0 && mig(y) == 0.0 && forward[0].set_eq(-x) && reverse[8].set_eq(y / x)
          && exactToInterval(intervalToExact(x)).set_eq(x) && !intervalToText(x).empty(),
          [] { return std::string(); });
  }
}

int main()
{
  gaol::init();
  ieee1788_using_directive();
  const int status = gaol_tests::summary();
  gaol::cleanup();
  return status;
}
