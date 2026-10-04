/*-*-C++-*----------------------------------------------------------------------
 * gaol -- NOT Just Another Interval Library
 *------------------------------------------------------------------------------
 * Tests of GAOL v5: gaol_ieee1788 as a program uses it, with
 * using namespace gaol_ieee1788 at file scope and without using namespace
 * gaol.
 *
 * A program opens one of the two namespaces. The names of the standard are
 * called unqualified, which compiles only if none of them is ambiguous with a
 * function that argument-dependent lookup finds on an interval, in gaol_core:
 * sin, min and the others brought in by using-declarations are gaol_core's
 * own. pow, which is in gaol_ieee1788 and in gaol but not in gaol_core, takes
 * an interval, an int or a double as exponent, and has to be the pow of
 * Table 9.1 for each, on intervals and on expressions, where GAOL's pow takes
 * pown for an integer exponent: pow(x, 2) is pow(x, [2]), the integer power
 * being pown(x, 2). Their bounds on boxes that reach each branch of the
 * function they share are checked bit for bit. The functions of C on numbers
 * have to remain those of C. intervalToExact() has to be exact_string(), and
 * to leave the global output format alone. The check of it by a second thread
 * writing intervals meanwhile is commented out: the tests run no thread.
 *
 * Copyright (c) 2026 ENSTA, France
 *
 * Created 2026-09-21 by Jordan NININ
 *------------------------------------------------------------------------------
 * gaol is a software distributed WITHOUT ANY WARRANTY. Read the associated
 * COPYING file for information.
 *----------------------------------------------------------------------------*/

#include "gaol_tests.h"
#include "gaol/gaol_expr_eval.h"

#include <cctype>
#include <clocale>
#include <cstdlib>
#include <locale>
#include <map>
#include <sstream>
#include <stdexcept>
#include <string>
#include <type_traits>
#include <utility>
#include <vector>

// Commented out: the tests run no thread (GAOL v5)
// // std::thread, which libstdc++ has only when built with a thread model
// #if !defined(__GLIBCXX__) || defined(_GLIBCXX_HAS_GTHREADS)
// #  include <atomic>
// #  include <thread>
// #  define GAOL_TESTS_THREADS 1
// #endif

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

  /* The functions of gaol_ieee1788 take the overloads of the expressions,
     gaol/gaol_expression.h being included here after gaol/gaol_ieee1788.h,
     and before it in tests/expressions.cpp (GAOL v5) */
  static_assert(std::is_same<decltype(gaol_ieee1788::sin(std::declval<const gaol::expression&>())), const gaol::expression>::value,
                "gaol_ieee1788::sin of an expression, gaol/gaol_expression.h included after gaol/gaol_ieee1788.h");

  /* A number converts neither to an interval nor to an expression (GAOL
     v5): min(y, 1.0) or atan2(y, 1.0), which took [1] while it converted,
     do not compile, and the interval is written interval(1.0) */
  template <typename A, typename B, typename = void> struct has_min : std::false_type {};
  template <typename A, typename B>
  struct has_min<A, B, decltype(void(min(std::declval<const A&>(), std::declval<const B&>())))> : std::true_type {};
  template <typename A, typename B, typename = void> struct has_atan2 : std::false_type {};
  template <typename A, typename B>
  struct has_atan2<A, B, decltype(void(atan2(std::declval<const A&>(), std::declval<const B&>())))> : std::true_type {};
  static_assert(!has_min<interval, double>::value && !has_min<double, interval>::value, "min(y, 1.0) does not compile");
  static_assert(!has_atan2<interval, double>::value && !has_atan2<double, interval>::value, "atan2(y, 1.0) does not compile");
  static_assert(has_min<interval, interval>::value && has_atan2<interval, interval>::value, "the detection works");

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
    const interval y = numsToInterval(0.25, 0.5), z = numsToInterval(-1.0, 2.0);
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

  /*
    The pow of Table 9.1 is written once, in gaol/gaol_interval.cpp:
    gaol_ieee1788::pow(x, y) is that function, and gaol::pow(x, y) calls it for
    every exponent but a degenerate integer, which takes pown on the whole of x,
    and [-oo, +oo] beyond the ints. The two were written apart, the first going
    through the second after checks of its own (x cut to [0, +oo], the empty
    sets, x = {0}): a check lost from either changes the bounds of a few boxes
    only, most having the same. Each box below reaches a branch of the shared
    function, or of what gaol::pow adds to it. The bounds are those the two
    functions gave before the pow of the standard was written once, bit for bit
    (as on 70 500 more boxes, under the four rounding directions, with SSE2 and
    FPU intervals; but for the lower bound of gaol::pow([0], y), y > 0, which
    was -0 with SSE2 intervals, and is +0 as in gaol_ieee1788::pow, and for a
    lower bound at a corner where the power is a double, which is that double
    now and was the double below, as 2^-2 for the box [0.1, 2] x [-2, 0.5]),
    each checked against the exact power computed with 500 bits (mpmath): it
    encloses it, within one double of the tightest bound.
  */
  void pow_on_boxes()
  {
    struct Box
    {
      interval x, y, standard, hybrid;
      Box(const interval& x_, const interval& y_, const interval& s) : x(x_), y(y_), standard(s), hybrid(s) {}
      Box(const interval& x_, const interval& y_, const interval& s, const interval& h)
        : x(x_), y(y_), standard(s), hybrid(h) {}
    };
    const auto I = [](double l, double u) { return interval(l, u); };
    const auto P = [](double p) { return interval(p); };
    const interval none = interval::emptyset(), all = interval::universe();
    // {x, y, gaol_ieee1788::pow(x, y)} where gaol::pow(x, y) is the same, {x, y, gaol_ieee1788::pow(x, y),
    // gaol::pow(x, y)} where it is not
    const Box boxes[] = {
      // an empty base or exponent
      {none, P(2), none},
      {P(2), none, none},
      // x < 0: no value for the pow of the standard, pown for a degenerate integer exponent of gaol::pow
      {I(-4, -1), P(0.5), none},
      {I(-4, -1), I(1.5, 2.5), none},
      {I(-4, -1), P(2), none, I(1, 16)},
      {I(-4, -1), P(3), none, I(-64, -1)},
      {I(-4, -1), P(-2), none, I(0.0625, 1)},
      {I(-oo, -1), P(2), none, I(1, oo)},
      {I(-3, 5), P(0.5), I(0, 0x1.1e3779b97f4a8p+1)},
      {I(-3, 5), I(0.5, 1.5), I(0, 0x1.65c55827df1d2p+3)},
      {I(-3, 5), I(-0.5, 1.5), I(0, oo)},
      // x cut to {0} (x = {0}, or x from below to 0): 0^y is 0 for y > 0, and has no value for y <= 0
      {P(0), I(1, 2), P(0)},
      {P(0), I(-1, 2), P(0)},
      {P(0), P(0.5), P(0)},
      {P(0), P(3), P(0)},
      {P(0), P(-0.5), none},
      {P(0), I(-2, 0), none},
      {P(0), P(0), none, P(1)},
      {P(0), P(-1), none},
      {I(-1, -0.0), P(0.5), P(0)},
      {I(-2, 0), P(2), P(0), I(0, 4)},
      {P(0), P(1e10), P(0), all},
      {P(0), P(-1e10), none, all},
      // a degenerate integer exponent within the ints: pown, on x >= 0 for the standard's pow, on the whole of x for gaol::pow
      {I(2, 3), P(5), I(32, 243)},
      {I(-2, 3), P(3), I(0, 27), I(-8, 27)},
      {I(-2, 3), P(2), I(0, 9)},
      {I(-3, 2), P(2), I(0, 4), I(0, 9)},
      {I(0, 4), P(0), P(1)},
      {I(0, 4), P(-1), I(0.25, oo)},
      {I(0, 4), P(2), I(0, 16)},
      {I(0.5, 2), P(-3), I(0.125, 8)},
      {P(1.1), P(3), I(0x1.54bc6a7ef9db3p+0, 0x1.54bc6a7ef9db4p+0)},
      {P(1), P(7), P(1)},
      {I(-oo, oo), P(2), I(0, oo)},
      {I(-oo, oo), P(3), I(0, oo), all},
      {I(1e-300, 1e300), P(3), I(0, oo)},
      {I(-1, 2), P(2147483647.0), I(0, oo), I(-1, oo)},
      {I(0.5, 1), P(-2147483648.0), I(1, oo)},
      // an integer exponent beyond the ints: CORE-MATH's pow at the bounds of x for the standard's pow, [-oo, +oo] for gaol::pow
      {I(2, 3), P(1e10), I(0x1.fffffffffffffp+1023, oo), all},
      {I(0.5, 0.9), P(1e10), I(0, 0x1p-1074), all},
      {I(2, 3), P(-1e10), I(0, 0x1p-1074), all},
      {P(1), P(-1e12), P(1), all},
      {I(-1, 1), P(2147483649.0), I(0, 1), all},
      {I(-1, 0.5), P(-2147483649.0), I(0x1.fffffffffffffp+1023, oo), all},
      {I(0, 2), P(1e10), I(0, oo), all},
      {I(0, 0.5), P(-1e10), I(0x1.fffffffffffffp+1023, oo), all},
      {I(2, oo), P(1e10), I(0x1.fffffffffffffp+1023, oo), all},
      {I(0, oo), P(1e12), I(0, oo), all},
      {I(1, 2), P(2147483648.0), I(1, oo), all},
      {I(0, 0x1.00000004p+0), P(2147483649.0), I(0, 0x1.d8e64b8d4ddaep+2), all},
      {I(0, 0x1.00000004p+0), P(-2147483649.0), I(0x1.152aaa3bf81cbp-3, oo), all},
      {I(-0.0, 0x1.00000004p+0), P(-2147483649.0), I(0x1.152aaa3bf81cbp-3, oo), all},
      {I(0x1.fffffff8p-1, oo), P(2147483649.0), I(0x1.152aaa334ec76p-3, oo), all},
      {I(0x1.fffffff8p-1, oo), P(-2147483649.0), I(0, 0x1.d8e64b9c150d4p+2), all},
      // CORE-MATH's pow at the corners of the box, from a base from 0, a base above 1, below 1, and around 1
      {I(2, 3), P(0.5), I(0x1.6a09e667f3bccp+0, 0x1.bb67ae8584cabp+0)},
      // where the power at a corner is a double, the lower bound is that double
      {P(4), P(0.5), P(2)},
      {I(4, 16), P(-0.5), I(0.25, 0.5)},
      {I(3, 4), I(2, 3), I(9, 64)},
      {I(0, 2), P(0.5), I(0, 0x1.6a09e667f3bcdp+0)},
      {I(0, 0.5), I(0.5, 1.5), I(0, 0x1.6a09e667f3bcdp-1)},
      {I(1.5, 2.5), I(0.5, 1.5), I(0x1.3988e1409212ep+0, 0x1.f9f6e4990f228p+1)},
      {I(1.5, 2.5), I(-1.5, -0.5), I(0x1.030dc4ea03a72p-2, 0x1.a20bd700c2c3ep-1)},
      {I(1.5, 2.5), I(-1, 2), I(0x1.9999999999999p-2, 6.25)},
      {I(0.3, 0.7), I(0.5, 1.5), I(0x1.50854f2c3d222p-3, 0x1.ac5eb3f7ab2f8p-1)},
      {I(0.3, 0.7), I(-1.5, -0.5), I(0x1.31fa808c55b43p+0, 0x1.857dd943cb7f5p+2)},
      {I(0.3, 0.7), I(-1.5, 2.5), I(0x1.93d32bceafc29p-5, 0x1.857dd943cb7f5p+2)},
      {I(0.5, 2), I(1.5, 2.5), I(0x1.6a09e667f3bccp-3, 0x1.6a09e667f3bcdp+2)},
      {I(0.5, 2), I(-2.5, -1.5), I(0x1.6a09e667f3bccp-3, 0x1.6a09e667f3bcdp+2)},
      {I(0.5, 2), I(-1.5, 2.5), I(0x1.6a09e667f3bccp-3, 0x1.6a09e667f3bcdp+2)},
      {I(0.1, 2), I(-2, 0.5), I(0x1p-2, 100)},
      {I(1, 2), I(-0.5, 1.5), I(0x1.6a09e667f3bccp-1, 0x1.6a09e667f3bcdp+1)},
      {P(1), I(-0.5, 1.5), P(1)},
      {I(0.3, 1), I(0.5, 1.5), I(0x1.50854f2c3d222p-3, 1)},
      {I(0.3, 3), I(-0.5, 1.5), I(0x1.50854f2c3d222p-3, 0x1.4c8dc2e42398p+2)},
      {I(3, 7), I(1.5, 2.5), I(0x1.4c8dc2e42397fp+2, 0x1.03489be058693p+7)},
      {I(1e-5, 1e5), I(-2.5, 3.5), I(0x1.d2ab78e5b8624p-59, 0x1.18ddde9363225p+58)},
      {P(2), P(1023.5), I(0x1.6a09e667f3bccp+1023, 0x1.6a09e667f3bcdp+1023)},
      {I(2, 3), I(2147483647.0, 2147483649.0), I(0x1.fffffffffffffp+1023, oo)},
      {P(1e-300), P(1.5), I(0, 0x1p-1074)},
      {P(1e300), P(1.5), I(0x1.fffffffffffffp+1023, oo)},
      // exp(y log(x)), for an infinite bound and for a base from 0 with an exponent that is not above 0
      {I(2, oo), I(0.5, 1.5), I(0x1.6a09e667f3bccp+0, oo)},
      {I(0, 2), I(-1.5, -0.5), I(0x1.6a09e667f3bcbp-2, oo)},
      {I(0, 1), I(-1, 1), I(0, oo)},
      {I(0, 1), I(0, 2), I(0, 1)},
      {I(0.5, 2), I(0, oo), I(0, oo)},
      {I(1, 3), I(-oo, 2), I(0, 0x1.2000000000001p+3)},
      {I(0.25, 0.5), I(-oo, -1), I(0x1.fffffffffffffp+0, oo)},
      {I(2, 4), I(1, oo), I(0x1.fffffffffffffp+0, oo)},
      {I(-oo, oo), P(0.5), I(0, oo)},
      {I(0, oo), I(1, 2), I(0, oo)},
      {I(4, oo), P(-0.5), I(0, 0x1.0000000000001p-1)},
      {I(0, 3), P(-0.5), I(0x1.279a74590331cp-1, oo)},
    };
    const auto same = [](const interval& a, const interval& b) {
      return a.is_empty() ? b.is_empty() : (!b.is_empty() && a.left() == b.left() && a.right() == b.right());
    };
    const auto text = [](const interval& a) { return a.is_empty() ? std::string("[empty]") : hex(a); };
    for (const Box& box : boxes) {
      const interval s = gaol_ieee1788::pow(box.x, box.y), h = gaol::pow(box.x, box.y);
      const std::string what = "pow(" + text(box.x) + ", " + text(box.y) + ")";
      check("gaol_ieee1788::pow(x, y) on boxes: the pow of Table 9.1", same(s, box.standard),
            [&] { return what + " is " + text(s) + " rather than " + text(box.standard); });
      check("gaol::pow(x, y) on boxes: pown for a degenerate integer exponent, the pow of Table 9.1 otherwise",
            same(h, box.hybrid), [&] { return what + " is " + text(h) + " rather than " + text(box.hybrid); });
      if (!box.y.is_empty() && box.y.left() == box.y.right()) {
        // With the exponent as a double: the same functions, gaol::pow(x, p) taking pown for an integer p
        // within the ints without the call of gaol::pow(x, [p])
        const double p = box.y.left();
        const interval sp = gaol_ieee1788::pow(box.x, p), hp = gaol::pow(box.x, p);
        check("gaol_ieee1788::pow(x, p) on boxes is gaol_ieee1788::pow(x, [p])", same(sp, box.standard),
              [&] { return what + " is " + text(sp) + " rather than " + text(box.standard); });
        check("gaol::pow(x, p) on boxes is gaol::pow(x, [p])", same(hp, box.hybrid),
              [&] { return what + " is " + text(hp) + " rather than " + text(box.hybrid); });
      }
    }
  }

  void gaol_functions()
  {
    const interval x = numsToInterval(0.25, 0.5), y = numsToInterval(-1.0, 2.0);
    check("sin, exp, sqrt, abs... on an interval are GAOL's",
          sin(x).set_eq(gaol::sin(x)) && exp(x).set_eq(gaol::exp(x)) && sqrt(x).set_eq(gaol::sqrt(x))
          && abs(y).set_eq(gaol::abs(y)) && sqr(y).set_eq(gaol::sqr(y)) && sign(y).set_eq(gaol::sign(y))
          && fma(x, y, x).set_eq(gaol::fma(x, y, x)) && expm1(x).set_eq(gaol::expm1(x))
          && min(x, y).set_eq(gaol::min(x, y)) && hypot(x, y).set_eq(gaol::hypot(x, y))
          && sinPi(x).set_eq(gaol::sinpi(x)),
          [&] { return hex(sin(x)); });
    check("the qualified names take intervals",
          gaol_ieee1788::sin(x).set_eq(gaol::sin(x)) && gaol_ieee1788::min(x, y).set_eq(gaol::min(x, y)),
          [&] { return hex(gaol_ieee1788::sin(x)); });
    check("the functions of C on numbers are C's", sqrt(4.0) == 2.0 && floor(2.5) == 2.0 && abs(-3) == 3,
          [] { return std::string(); });
  }

  // Every name of the standard gaol_ieee1788 provides, called unqualified
  void names_of_the_standard()
  {
    const interval x = numsToInterval(0.25, 0.5), y = numsToInterval(-1.0, 2.0);
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

  /* textToInterval reads the names of the standard (GAOL v5): each name of
     Tables 9.1 and 10.5 gives the function of the same name, pow the pow of
     Table 9.1, and a name of GAOL alone is no function, which gives the
     empty set; gaol::textToInterval reads the names of GAOL. textToInterval
     read those of GAOL, and pow([-4,-1],2) was [1, 16], where
     pow([-4,-1], 2) is the empty set. */
  void text_with_the_names_of_the_standard()
  {
    const interval x = numsToInterval(0.25, 0.5), y = numsToInterval(-1.0, 2.0),
      p = numsToInterval(2.0, 5.0);
    struct Case { const char *text; interval value; };
    const Case cases[] = {
      {"pown([2,5],5)", pown(p, 5)}, {"pow([2,5],5)", pow(p, 5.0)},
      {"pow([-4,-1],2)", pow(numsToInterval(-4.0, -1.0), 2.0)},
      {"rootn([-8,27],3)", rootn(numsToInterval(-8.0, 27.0), 3)},
      {"rootn([0.25,0.5],-3)", rootn(x, -3)},
      {"neg([0.25,0.5])", neg(x)}, {"add([0.25,0.5],[-1,2])", add(x, y)},
      {"sub([0.25,0.5],[-1,2])", sub(x, y)}, {"mul([0.25,0.5],[-1,2])", mul(x, y)},
      {"div([0.25,0.5],[-1,2])", div(x, y)}, {"recip([0.25,0.5])", recip(x)},
      {"sqr([-1,2])", sqr(y)}, {"sqrt([0.25,0.5])", sqrt(x)},
      {"fma([0.25,0.5],[-1,2],[0.25,0.5])", fma(x, y, x)},
      {"exp([0.25,0.5])", exp(x)}, {"exp2([0.25,0.5])", exp2(x)}, {"exp10([0.25,0.5])", exp10(x)},
      {"log([0.25,0.5])", log(x)}, {"log2([0.25,0.5])", log2(x)}, {"log10([0.25,0.5])", log10(x)},
      {"sin([0.25,0.5])", sin(x)}, {"cos([0.25,0.5])", cos(x)}, {"tan([0.25,0.5])", tan(x)},
      {"asin([0.25,0.5])", asin(x)}, {"acos([0.25,0.5])", acos(x)}, {"atan([0.25,0.5])", atan(x)},
      {"atan2([-1,2],[0.25,0.5])", atan2(y, x)},
      {"sinh([0.25,0.5])", sinh(x)}, {"cosh([0.25,0.5])", cosh(x)}, {"tanh([0.25,0.5])", tanh(x)},
      {"asinh([0.25,0.5])", asinh(x)}, {"acosh([-1,2])", acosh(y)}, {"atanh([0.25,0.5])", atanh(x)},
      {"sign([-1,2])", sign(y)}, {"ceil([-1,2])", ceil(y)}, {"floor([-1,2])", floor(y)},
      {"trunc([-1,2])", trunc(y)}, {"roundTiesToEven([0.5,2.5])", roundTiesToEven(numsToInterval(0.5, 2.5))},
      {"roundTiesToAway([0.5,2.5])", roundTiesToAway(numsToInterval(0.5, 2.5))},
      {"abs([-1,2])", abs(y)}, {"min([0.25,0.5],[-1,2])", min(x, y)}, {"max([0.25,0.5],[-1,2])", max(x, y)},
      {"expm1([0.25,0.5])", expm1(x)}, {"exp2m1([0.25,0.5])", exp2m1(x)},
      {"exp10m1([0.25,0.5])", exp10m1(x)}, {"logp1([0.25,0.5])", logp1(x)},
      {"log2p1([0.25,0.5])", log2p1(x)}, {"log10p1([0.25,0.5])", log10p1(x)},
      {"hypot([0.25,0.5],[-1,2])", hypot(x, y)}, {"rSqrt([0.25,0.5])", rSqrt(x)},
      {"sinPi([0.25,0.5])", sinPi(x)}, {"cosPi([0.25,0.5])", cosPi(x)}, {"tanPi([0.25,0.5])", tanPi(x)},
      {"asinPi([0.25,0.5])", asinPi(x)}, {"acosPi([0.25,0.5])", acosPi(x)},
      {"atanPi([0.25,0.5])", atanPi(x)}, {"atan2Pi([-1,2],[0.25,0.5])", atan2Pi(y, x)},
      {"SINPI([0.25,0.5])", sinPi(x)}, {"[pown(2,3), rootn(27,3)*3]", numsToInterval(8.0, 9.0)},
    };
    for (const Case& c : cases) {
      const interval got = textToInterval(c.text);
      check("textToInterval reads the names of IEEE 1788-2015", got.set_eq(c.value),
            [&] { return std::string(c.text) + ": " + hex(got) + " rather than " + hex(c.value); });
    }
    // The names of GAOL alone, and the calls that are wrong: the empty set
    const char *const not_the_standard[] = {
      "nth_root(8,3)", "cbrt(8)", "inverse(2)", "integer([1.5,3])", "log1p(0)",
      "round_ties_to_even(1)", "pown([2,5],2.5)", "sin(1,2)", "fma(1,2)",
    };
    for (const char *t : not_the_standard) {
      const interval got = textToInterval(t);
      check("textToInterval gives the empty set for a name of GAOL alone or a wrong call",
            got.is_empty(), [&] { return std::string(t) + ": " + hex(got); });
    }
    check("gaol::textToInterval reads the names of GAOL: pow([-4,-1],2) is [1, 16]",
          gaol::textToInterval("pow([-4,-1],2)").set_eq(numsToInterval(1.0, 16.0)),
          [] { return hex(gaol::textToInterval("pow([-4,-1],2)")); });
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

  /*
    The text of a point interval is read back (GAOL v5): intervalToText(
    interval(0.1)) was <0.1, 0.1000000000000001>, whose two numbers are not the
    same double, which the reader takes for a point only, so that
    textToInterval() gave the empty set, where the recovery requirement of
    IEEE 1788-2015 (13.4) asks for an interval containing the one written. A
    point that the digits write exactly is written [a], the literal of the
    standard for a point, and read back as the point itself.
  */
  void text_of_a_point_interval()
  {
    const gaol::interval_format::format_t saved = interval::format();
    interval::format(gaol::interval_format::bounds);
    const interval tenth(0.1);
    const interval back = textToInterval(intervalToText(tenth));
    check("textToInterval(intervalToText(interval(0.1))) contains 0.1", back.set_contains(0.1),
          [&] { return intervalToText(tenth) + " read " + hex(back); });
    const double points[] = { 0.1, -0.1, 1.0 / 3.0, 3.14159265358979, 1e23, 4.0, 0.5, 0.0, -0.0,
                              std::numeric_limits<double>::denorm_min(), -std::numeric_limits<double>::max() };
    for (double x : points) {
      const std::string text = intervalToText(interval(x));
      const interval y = textToInterval(text);
      check("textToInterval(intervalToText(point)) contains the point", y.set_contains(x),
            [&] { return text + " read " + hex(y); });
      if (text.find(',') == std::string::npos) {
        check("textToInterval(intervalToText(point)) written [a]: the point itself", y.left() == x && y.right() == x,
              [&] { return text + " read " + hex(y); });
      }
    }
    interval::format(saved);
  }

  /*
    Whether s is an interval literal of the standard that intervalToText()
    writes: [ ], [empty], [m] or [l, u], m, l and u being decimal numbers, or
    inf, with a sign, as in Tables 9.5 and 12.2 (12.11.5), whose letters may be
    of either case. The angles <a, a> of GAOL, a width "c (+/- w)", the
    agreeing digits and a decimal comma are none.
  */
  bool is_portable_literal(const std::string& text)
  {
    std::string s;
    for (char c : text) {
      s += static_cast<char>(std::tolower(static_cast<unsigned char>(c)));
    }
    std::size_t i = 0;
    const auto spaces = [&] { while (i < s.size() && (s[i] == ' ' || s[i] == '\t')) { ++i; } };
    const auto digits = [&] { const std::size_t from = i; while (i < s.size() && std::isdigit(static_cast<unsigned char>(s[i]))) { ++i; } return i - from; };
    const auto expect = [&](char c) { if (i < s.size() && s[i] == c) { ++i; return true; } return false; };
    const auto number = [&] {
      if (i < s.size() && (s[i] == '+' || s[i] == '-')) {
        ++i;
      }
      if (s.compare(i, 3, "inf") == 0) {
        i += 3;
        return true;
      }
      const std::size_t before = digits();
      std::size_t after = 0;
      if (expect('.')) {
        after = digits();
      }
      if (before + after == 0) {
        return false;
      }
      if (expect('e')) {
        if (i < s.size() && (s[i] == '+' || s[i] == '-')) {
          ++i;
        }
        return digits() > 0;
      }
      return true;
    };
    if (!expect('[')) {
      return false;
    }
    spaces();
    if (expect(']')) {
      return i == s.size();
    }
    if (s.compare(i, 5, "empty") == 0) {
      i += 5;
      spaces();
      return expect(']') && i == s.size();
    }
    if (!number()) {
      return false;
    }
    spaces();
    if (expect(',')) {
      spaces();
      if (!number()) {
        return false;
      }
      spaces();
    }
    return expect(']') && i == s.size();
  }

  /*
    intervalToText(x) is an interval literal that textToInterval() reads back
    as an interval containing x, [l, r], [a] for a point that the digits write
    exactly, and [empty] for the empty set, whatever the global output format
    and the locale of the program (GAOL v5), as the standard asks of it
    (13.3), with the digits of interval::precision(): it wrote what operator<<
    does, the width "1.5 (+/- 0.5)", the agreeing digits, a decimal comma under
    the locale of a program that sets one, and <4, 4> for the point interval
    4, which is no literal of the standard. The texts written by hand are
    those of 16 digits and of 1 digit; with the other precisions, it has to be
    the text operator<< writes in the bounds format. Under a locale writing a
    decimal comma, operator<< writes the bounds format with a decimal point
    too, and the text of a point has to be read back as the point, or as an
    interval containing it when it has two bounds (GAOL v5: the literal [a]
    made interval(-2.5) [-2,5], read as the interval [-2, 5], and GAOL wrote
    [-2,5, -2,5], which the reader refused). Where no such locale is
    installed, that part is not checked.
  */
  void text_independent_of_the_output_settings()
  {
    const gaol::interval_format::format_t saved_format = interval::format();
    const std::streamsize saved_precision = interval::precision();
    const double smallest = std::numeric_limits<double>::denorm_min(), largest = std::numeric_limits<double>::max();
    // The texts to expect, written by hand, with 16 digits and with 1
    const struct { const char *name; interval x; const char *text; const char *text1; } forms[] = {
      { "[1, 2]", interval(1.0, 2.0), "[1, 2]", "[1, 2]" },
      { "interval(4)", interval(4.0), "[4]", "[4]" },
      { "interval(0.5)", interval(0.5), "[0.5]", "[0.5]" },
      { "interval(-1024)", interval(-1024.0), "[-1024]", "[-2e+03, -1e+03]" },
      { "interval(0.1)", interval(0.1), "[0.1, 0.1000000000000001]", "[0.1, 0.2]" },
      { "[-0, 0]", interval(-0.0, 0.0), "[0]", "[0]" },
      { "interval(-0.0)", interval(-0.0), "[0]", "[0]" },
      { "interval::zero()", interval::zero(), "[0]", "[0]" },
      { "the double nearest 1/3", interval(0x1.5555555555555p-2), "[0.3333333333333333, 0.3333333333333334]",
        "[0.3, 0.4]" },
      { "[1, +oo]", interval(1.0, oo), "[1, inf]", "[1, inf]" },
      { "[-oo, -1.5]", interval(-oo, -1.5), "[-inf, -1.5]", "[-inf, -1]" },
      { "the universe", interval::universe(), "[-inf, inf]", "[-inf, inf]" },
      { "the empty set", interval::emptyset(), "[empty]", "[empty]" },
      { "[5e-324, 1e-323]", interval(smallest, 2.0*smallest), "[4.940656458412465e-324, 9.881312916824931e-324]",
        "[4e-324, 1e-323]" },
      { "[-MAX, MAX]", interval(-largest, largest), "[-1.797693134862316e+308, 1.797693134862316e+308]",
        "[-2e+308, 2e+308]" },
    };
    const interval others[] = { interval(0.1, 0.3), interval::pi(), interval(1.25, 1.2567), interval(-2.0/3.0, 1e23),
                                interval(1e-5, 2e-5) };

    const auto expect_text = [&](const interval& x, const std::string& expected, const std::string& situation) {
      const std::string text = intervalToText(x);
      const auto describe = [&] { return hex(x) + " " + situation + ": \"" + text + "\""; };
      if (!expected.empty()) {
        check("intervalToText: the text of the manual, whatever the format and the locale", text == expected,
              [&] { return describe() + " rather than \"" + expected + "\""; });
      }
      check("intervalToText: an interval literal of IEEE 1788-2015", is_portable_literal(text), describe);
      const interval back = textToInterval(text);
      check("intervalToText: read back as an interval containing x",
            x.is_empty() ? back.is_empty() : back.set_contains(x), [&] { return describe() + " read " + hex(back); });
    };

    const std::streamsize precisions[] = { 1, 3, 8, 16, 17, 30 };
    const std::size_t nb_others = sizeof others / sizeof others[0];
    // The text operator<< writes in the bounds format, in a stream of the C
    // locale and its first flags, with each precision: what intervalToText()
    // has to give with that precision whatever the other settings
    std::map<std::streamsize, std::vector<std::string> > reference;
    interval::format(gaol::interval_format::bounds);
    for (std::streamsize precision : precisions) {
      interval::precision(precision);
      for (std::size_t i = 0; i < nb_others; ++i) {
        std::ostringstream os;
        os.imbue(std::locale::classic());
        os << others[i];
        reference[precision].push_back(os.str());
      }
      for (const auto& form : forms) {
        std::ostringstream os;
        os.imbue(std::locale::classic());
        os << form.x;
        reference[precision].push_back(os.str());
      }
    }

    const struct { const char *name; gaol::interval_format::format_t format; } formats[] = {
      { "bounds", gaol::interval_format::bounds }, { "width", gaol::interval_format::width },
      { "center", gaol::interval_format::center }, { "hexa", gaol::interval_format::hexa },
      { "agreeing", gaol::interval_format::agreeing },
    };
    const auto expect_all = [&](const std::string& where) {
      for (const auto& f : formats) {
        for (std::streamsize precision : precisions) {
          interval::format(f.format);
          interval::precision(precision);
          const std::string situation = "in the " + std::string(f.name) + " format with the precision "
                                        + std::to_string(precision) + where;
          std::size_t k = 0;
          for (std::size_t i = 0; i < nb_others; ++i) {
            expect_text(others[i], reference[precision][k++], situation);
          }
          for (const auto& form : forms) {
            expect_text(form.x, (precision == 16) ? form.text : (precision == 1) ? form.text1 : "", situation);
            expect_text(form.x, reference[precision][k++], situation);
          }
          check("intervalToText leaves the global format as it was", interval::format() == f.format, [&] { return situation; });
          check("intervalToText leaves the global precision as it was", interval::precision() == precision, [&] { return situation; });
        }
      }
    };
    expect_all("");

    // Under a locale writing a decimal comma, which a program sets for all its
    // streams with std::locale::global()
    const std::locale saved_locale;
    const std::string saved_c_locale = std::setlocale(LC_ALL, nullptr);
    bool commas = false;
    for (const char *name : { "fr_FR.UTF-8", "fr_FR.utf8", "de_DE.UTF-8", "de_DE.utf8", "French_France.1252", "fr_FR" }) {
      try {
        std::locale::global(std::locale(name));
      } catch (const std::runtime_error&) {
        continue;
      }
      std::ostringstream probe;
      probe << 1.5;
      if (probe.str() == "1,5") {
        commas = true;
        break;
      }
    }
    if (commas) {
      expect_all(" under a locale writing a decimal comma");
      std::ostringstream os;
      interval::format(gaol::interval_format::bounds);
      interval::precision(16);
      os << interval(0.25, 0.5);
      check("operator<< writes the bounds format with a decimal point under that locale, the text of intervalToText",
            os.str() == "[0.25, 0.5]" && intervalToText(interval(0.25, 0.5)) == "[0.25, 0.5]",
            [&] { return os.str() + " and " + intervalToText(interval(0.25, 0.5)); });
      // A point written by operator<< under that locale: the literal [a] of a
      // number with a decimal comma would be two numbers, which the reader
      // takes, [-2,5] being [-2, 5], [12,5] the empty set and [0,] (no digit,
      // showpoint) [0, +oo]; the text has to be read back as an interval
      // containing the point, and as the point itself when it is one number
      const struct { const char *name; std::ios_base::fmtflags flags; std::streamsize precision; } settings[] = {
        { "with 16 digits", std::ios_base::fmtflags(), 16 },
        { "in the fixed format with the showpoint flag and no digit", std::ios_base::fixed | std::ios_base::showpoint, 0 },
        { "in the fixed format with 1074 digits", std::ios_base::fixed, 1074 },
      };
      for (const auto& setting : settings) {
        interval::precision(setting.precision);
        for (double x : { -2.5, -0.5, 12.5, 0.5, 4.0, 0.0, -smallest }) {
          std::ostringstream point;
          point.setf(setting.flags);
          point << interval(x);
          const std::string text = point.str();
          bool refused = false;
          interval back;
          try {
            back = gaol::textToInterval(text);
          } catch (const gaol::input_format_error&) {
            refused = true;
          }
          const bool one_number = (text.find(", ") == std::string::npos);
          check("operator<< of a point under that locale: read back as an interval containing it, [a] as the point",
                !refused && back.set_contains(x) && (!one_number || (back.left() == x && back.right() == x)),
                [&] { return text.substr(0, 60) + " " + setting.name + ", read " + hex(back); });
        }
      }
      interval::precision(16);
      std::ostringstream four;
      four << interval(4.0);
      check("operator<< of a point without a comma under that locale: [a]", four.str() == "[4]",
            [&] { return four.str(); });
    } else {
      std::printf("No locale writing a decimal comma: intervalToText under such a locale is not checked\n");
    }
    std::locale::global(saved_locale);
    std::setlocale(LC_ALL, saved_c_locale.c_str());
    interval::precision(saved_precision);
    interval::format(saved_format);
  }

// Commented out: the tests run no thread (GAOL v5)
// #if GAOL_TESTS_THREADS
//   /*
//     intervalToExact() in one thread while another writes intervals in
//     interval_format::bounds: the other one has to write them in that format.
//     intervalToExact() switched the global output format to hexa and back,
//     which the other threads saw meanwhile (TODO.md, point 7).
//   */
//   void exact_text_in_another_thread()
//   {
//     const interval third(1.0 / 3.0, 2.0 / 3.0);
//     const gaol::interval_format::format_t saved = interval::format();
//     interval::format(gaol::interval_format::bounds);
//     std::atomic<bool> done(false);
//     std::thread exact_writer([&] {
//       for (int i = 0; i < 20000; ++i) {
//         (void)intervalToExact(third);
//       }
//       done = true;
//     });
//     long written = 0, in_hexa = 0;
//     std::string seen;
//     while (!done) {
//       std::ostringstream s;
//       s << third;
//       ++written;
//       if (s.str().find("0x") != std::string::npos) {
//         ++in_hexa;
//         seen = s.str();
//       }
//     }
//     exact_writer.join();
//     check("intervalToExact in one thread leaves the output format of the others", in_hexa == 0,
//           [&] { return std::to_string(in_hexa) + " of " + std::to_string(written) + " intervals written in hexa, " + seen; });
//     interval::format(saved);
//   }
// #endif
}

int main()
{
  gaol::init();
  pow_of_the_standard();
  pow_on_boxes();
  gaol_functions();
  names_of_the_standard();
  text_with_the_names_of_the_standard();
  exact_text();
  text_of_a_point_interval();
  text_independent_of_the_output_settings();
// Commented out: the tests run no thread (GAOL v5)
// #if GAOL_TESTS_THREADS
//   exact_text_in_another_thread();
// #endif
  const int status = gaol_tests::summary();
  gaol::cleanup();
  return status;
}
