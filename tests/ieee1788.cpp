/*-*-C++-*----------------------------------------------------------------------
 * gaol -- NOT Just Another Interval Library
 *------------------------------------------------------------------------------
 * Tests of GAOL v5: gaol_ieee1788 as a program uses it, with
 * using namespace gaol_ieee1788 at file scope, without using namespace gaol,
 * and with gaol/gaol_expression.h included before gaol/gaol.
 *
 * A program opens one of the two namespaces. The names of the standard are
 * called unqualified, which compiles only if none of them is ambiguous with a
 * function that argument-dependent lookup finds on an interval, in gaol_core:
 * sin, min and the others brought in by using-declarations are gaol_core's
 * own, and the overloads of gaol::expression are in sight, which made
 * gaol_ieee1788 depend on the order of the includes when it took GAOL's
 * functions by using-declarations. pow, which is in gaol_ieee1788 and in gaol
 * but not in gaol_core, takes an interval, an int or a double as exponent, and
 * has to be the pow of Table 9.1 for each, on intervals and on expressions,
 * where GAOL's pow takes pown for an integer exponent: pow(x, 2) is
 * pow(x, [2]), the integer power being pown(x, 2). The functions of C on
 * numbers have to remain those of C. intervalToExact() has to be
 * exact_string(), and to leave the global output format alone, which another
 * thread writing intervals meanwhile sees.
 *
 * Copyright (c) 2026 ENSTA, France
 *
 * Created 2026-09-21 by Jordan NININ
 *------------------------------------------------------------------------------
 * gaol is a software distributed WITHOUT ANY WARRANTY. Read the associated
 * COPYING file for information.
 *----------------------------------------------------------------------------*/

// gaol/gaol_expression.h first, through the evaluator of the expressions
#include "gaol/gaol_expr_eval.h"
#include "gaol_tests.h"

#include <cstdlib>
#include <sstream>
#include <string>
#include <type_traits>

// std::thread, which libstdc++ has only when built with a thread model
#if !defined(__GLIBCXX__) || defined(_GLIBCXX_HAS_GTHREADS)
#  include <atomic>
#  include <thread>
#  define GAOL_TESTS_THREADS 1
#endif

using namespace gaol_ieee1788;
using gaol_tests::check;
using gaol_tests::hex;

namespace
{
  const double oo = std::numeric_limits<double>::infinity();

  // The functions of C on numbers remain those of C: the templates of
  // gaol_ieee1788 take part only when an argument is an interval
  static_assert(std::is_same<decltype(sqrt(2.0)), double>::value, "sqrt(double) is C's");
  static_assert(std::is_same<decltype(floor(2.5)), double>::value, "floor(double) is C's");
  static_assert(std::is_same<decltype(atan2(1.0, 2.0)), double>::value, "atan2(double, double) is C's");
  static_assert(std::is_same<decltype(abs(-3)), int>::value, "abs(int) is C's");

  // The value of an expression, which the evaluator of GAOL computes
  interval value_of(const gaol::expression& e)
  {
    gaol::expr_eval eval;
    e.get_root()->accept(eval);
    return eval.result();
  }

  void pow_of_the_standard()
  {
    // With a number as exponent, the pow of gaol was found here too, and gave
    // [1, 16] for pow([-4, -1], 2) and pow([-4, -1], 2.0), and [1] for pow([0], 0)
    const interval x = numsToInterval(-4.0, -1.0);
    check("pow(x, 2), pow(x, 2.0), pow(x, [2]) are the pow of Table 9.1: empty for x < 0",
          pow(x, 2).is_empty() && pow(x, 2.0).is_empty() && pow(x, interval(2.0)).is_empty()
          && pow(x, 3).is_empty() && pow(x, 0.5).is_empty(),
          [&] { return hex(pow(x, 2)) + " " + hex(pow(x, 2.0)); });
    const interval y = textToInterval("[0.25, 0.5]"), z = numsToInterval(-1.0, 2.0);
    check("pow(x, y) unqualified is gaol_ieee1788::pow", pow(y, z).set_eq(gaol_ieee1788::pow(y, z)),
          [&] { return hex(pow(y, z)); });
    check("pown(x, 2) is the integer power, [1, 16] for x = [-4, -1]",
          pown(x, 2).set_eq(numsToInterval(1.0, 16.0)) && pown(x, 3).set_eq(numsToInterval(-64.0, -1.0))
          && pown(interval(0.0), 0).set_eq(interval(1.0)),
          [&] { return hex(pown(x, 2)); });
    check("gaol::pow(x, 2) remains GAOL's, [1, 16]",
          gaol::pow(x, 2).set_eq(numsToInterval(1.0, 16.0)) && gaol::pow(x, 2.0).set_eq(numsToInterval(1.0, 16.0))
          && gaol::pow(x, interval(2.0)).set_eq(numsToInterval(1.0, 16.0)),
          [&] { return hex(gaol::pow(x, 2)); });
    const interval zero(0.0);
    check("pow([0], 0) and pow([0], [0]) have no value, pow([0], 1) = [0]",
          pow(zero, 0).is_empty() && pow(zero, interval(0.0)).is_empty() && pow(zero, 1).set_eq(zero),
          [&] { return hex(pow(zero, 0)); });
    const interval b(0.0, 4.0);
    check("pow([0, 4], n) = pown([0, 4], n) for x >= 0: [1] for n = 0, [1/4, +oo] for n = -1",
          pow(b, 0).set_eq(interval(1.0)) && pow(b, -1).set_eq(numsToInterval(0.25, oo))
          && pow(b, 2).set_eq(numsToInterval(0.0, 16.0)),
          [&] { return hex(pow(b, 0)) + " " + hex(pow(b, -1)); });
    check("pow(x, +oo) and pow(x, NaN) contain no real exponent: empty",
          pow(b, oo).is_empty() && pow(b, std::numeric_limits<double>::quiet_NaN()).is_empty(),
          [&] { return hex(pow(b, oo)); });

    // An integer exponent beyond the ints: the pow of the standard on x >= 0,
    // where it gave [-oo, +oo] by taking pown
    const double dmax = std::numeric_limits<double>::max();
    const interval big = pow(numsToInterval(2.0, 3.0), interval(1e10));
    check("pow([2, 3], [1e10]) = [DBL_MAX, +oo]", big.left() == dmax && big.right() == oo,
          [&] { return hex(big); });
    const interval small = pow(numsToInterval(0.5, 0.9), interval(1e10));
    check("pow([0.5, 0.9], [1e10]) is in [0, 2^-1074]",
          small.left() == 0.0 && small.right() > 0.0 && small.right() <= std::nextafter(0.0, 1.0),
          [&] { return hex(small); });
    const interval one = pow(interval(1.0), interval(-1e12));
    check("pow([1], [-1e12]) = [1]", one.set_eq(interval(1.0)), [&] { return hex(one); });

    // The expressions of pow and pown, evaluated: pow(e1, e2) is the pow of
    // the standard, where the node of gaol::pow(e1, e2) computes GAOL's
    const gaol::expression base(x), exponent(interval(2.0));
    const interval std_pow = value_of(pow(base, exponent)), std_pown = value_of(pown(base, 2));
    const interval gaol_pow = value_of(gaol::pow(base, exponent));
    check("pow(e1, e2) and pown(e, n) on expressions are those of the standard",
          std_pow.is_empty() && std_pown.set_eq(numsToInterval(1.0, 16.0)) && gaol_pow.set_eq(numsToInterval(1.0, 16.0)),
          [&] { return hex(std_pow) + " " + hex(std_pown) + " " + hex(gaol_pow); });
  }

  void gaol_functions()
  {
    const interval x = textToInterval("[0.25, 0.5]"), y = numsToInterval(-1.0, 2.0);
    check("sin, exp, sqrt, abs... on an interval are GAOL's",
          sin(x).set_eq(gaol::sin(x)) && exp(x).set_eq(gaol::exp(x)) && sqrt(x).set_eq(gaol::sqrt(x))
          && abs(y).set_eq(gaol::abs(y)) && sqr(y).set_eq(gaol::sqr(y)) && sign(y).set_eq(gaol::sign(y))
          && fma(x, y, x).set_eq(gaol::fma(x, y, x)) && expm1(x).set_eq(gaol::expm1(x))
          && min(x, y).set_eq(gaol::min(x, y)) && hypot(x, y).set_eq(gaol::hypot(x, y))
          && sinPi(x).set_eq(gaol::sinpi(x)),
          [&] { return hex(sin(x)); });
    check("min, max, atan2 and hypot take an interval and a number",
          min(y, 1.0).set_eq(gaol::min(y, interval(1.0))) && max(0.0, y).set_eq(gaol::max(interval(0.0), y))
          && atan2(y, 1.0).set_eq(gaol::atan2(y, interval(1.0))) && hypot(3.0, x).set_eq(gaol::hypot(interval(3.0), x)),
          [&] { return hex(min(y, 1.0)); });
    check("the qualified names take intervals",
          gaol_ieee1788::sin(x).set_eq(gaol::sin(x)) && gaol_ieee1788::min(x, y).set_eq(gaol::min(x, y)),
          [&] { return hex(gaol_ieee1788::sin(x)); });
    check("the functions of C on numbers are C's", sqrt(4.0) == 2.0 && floor(2.5) == 2.0 && abs(-3) == 3,
          [] { return std::string(); });
  }

  // Every name of the standard gaol_ieee1788 provides, called unqualified
  void names_of_the_standard()
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
    check("the functions of the standard's names",
          b && inf(x) == 0.25 && sup(x) == 0.5 && m == mid(x) && r == rad(x) && wid(x) == 0.25
          && mag(y) == 2.0 && mig(y) == 0.0 && forward[0].set_eq(-x) && reverse[8].set_eq(y / x)
          && exactToInterval(intervalToExact(x)).set_eq(x) && !intervalToText(x).empty(),
          [] { return std::string(); });
  }

  void exact_text()
  {
    const interval third(1.0 / 3.0, 2.0 / 3.0);
    const gaol::interval_format::format_t saved = interval::format();
    interval::format(gaol::interval_format::bounds);
    const std::string exact = intervalToExact(third);
    check("intervalToExact does not change the output format",
          interval::format() == gaol::interval_format::bounds, [] { return std::string(); });
    check("intervalToExact(x) = exact_string(x), read back bit for bit",
          exact == gaol::exact_string(third) && exactToInterval(exact).set_eq(third), [&] { return exact; });
    interval::format(gaol::interval_format::hexa);
    std::ostringstream s;
    s << third << ' ' << interval::emptyset();
    check("operator<< in interval_format::hexa writes exact_string",
          s.str() == gaol::exact_string(third) + " [empty]", [&] { return s.str(); });
    interval::format(saved);
  }

#if GAOL_TESTS_THREADS
  /*
    intervalToExact() in one thread while another writes intervals in
    interval_format::bounds: the other one has to write them in that format.
    intervalToExact() switched the global output format to hexa and back,
    which the other threads saw meanwhile (TODO.md, point 7).
  */
  void exact_text_in_another_thread()
  {
    const interval third(1.0 / 3.0, 2.0 / 3.0);
    const gaol::interval_format::format_t saved = interval::format();
    interval::format(gaol::interval_format::bounds);
    std::atomic<bool> done(false);
    std::thread exact_writer([&] {
      for (int i = 0; i < 20000; ++i) {
        (void)intervalToExact(third);
      }
      done = true;
    });
    long written = 0, in_hexa = 0;
    std::string seen;
    while (!done) {
      std::ostringstream s;
      s << third;
      ++written;
      if (s.str().find("0x") != std::string::npos) {
        ++in_hexa;
        seen = s.str();
      }
    }
    exact_writer.join();
    check("intervalToExact in one thread leaves the output format of the others", in_hexa == 0,
          [&] { return std::to_string(in_hexa) + " of " + std::to_string(written) + " intervals written in hexa, " + seen; });
    interval::format(saved);
  }
#endif
}

int main()
{
  gaol::init();
  pow_of_the_standard();
  gaol_functions();
  names_of_the_standard();
  exact_text();
#if GAOL_TESTS_THREADS
  exact_text_in_another_thread();
#endif
  const int status = gaol_tests::summary();
  gaol::cleanup();
  return status;
}
