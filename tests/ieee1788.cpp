/*-*-C++-*----------------------------------------------------------------------
 * gaol -- NOT Just Another Interval Library
 *------------------------------------------------------------------------------
 * gaol_ieee1788 as a program uses it: using namespace gaol_ieee1788 at file
 * scope, no using namespace gaol, and gaol/gaol_expression.h included before
 * gaol/gaol (GAOL v5).
 *
 * The names of the standard are called unqualified, which compiles only if
 * none of them is ambiguous with the function of gaol that argument-dependent
 * lookup finds; the overloads of gaol::expression are in sight, which made
 * gaol_ieee1788 depend on the order of the includes when it took GAOL's
 * functions by using-declarations. pow takes an interval, an int or a double
 * as exponent, and has to be the pow of Table 9.1 for each, where GAOL's pow
 * takes pown for an integer exponent. The functions of C on numbers have to
 * remain those of C. intervalToExact() has to be exact_string(), and to leave
 * the global output format alone, which another thread writing intervals
 * meanwhile sees.
 *
 * Copyright (c) 2026 ENSTA, France
 *
 * Created 2026-09-21 by Jordan NININ
 *------------------------------------------------------------------------------
 * gaol is a software distributed WITHOUT ANY WARRANTY. Read the associated
 * COPYING file for information.
 *----------------------------------------------------------------------------*/

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

  void pow_of_the_standard()
  {
    const interval x = numsToInterval(-4.0, -1.0);
    check("pow(x, 2), pow(x, 2.0), pow(x, [2]) are the pow of Table 9.1: empty for x < 0",
          pow(x, 2).is_empty() && pow(x, 2.0).is_empty() && pow(x, interval(2.0)).is_empty()
          && pow(x, 3).is_empty() && pow(x, 0.5).is_empty(),
          [&] { return hex(pow(x, 2)) + " " + hex(pow(x, 2.0)); });
    check("pown(x, 2) is the integer power, [1, 16] for x = [-4, -1]",
          pown(x, 2).set_eq(numsToInterval(1.0, 16.0)) && pown(x, 3).set_eq(numsToInterval(-64.0, -1.0)),
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
  }

  void gaol_functions()
  {
    const interval x = textToInterval("[0.25, 0.5]"), y = numsToInterval(-1.0, 2.0);
    check("sin, exp, sqrt, abs... on an interval are GAOL's",
          sin(x).set_eq(gaol::sin(x)) && exp(x).set_eq(gaol::exp(x)) && sqrt(x).set_eq(gaol::sqrt(x))
          && abs(y).set_eq(gaol::abs(y)) && sqr(y).set_eq(gaol::sqr(y)) && sign(y).set_eq(gaol::sign(y))
          && fma(x, y, x).set_eq(gaol::fma(x, y, x)) && expm1(x).set_eq(gaol::expm1(x)),
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
  exact_text();
#if GAOL_TESTS_THREADS
  exact_text_in_another_thread();
#endif
  const int status = gaol_tests::summary();
  gaol::cleanup();
  return status;
}
