/*-*-C++-*------------------------------------------------------------------
 * gaol -- NOT Just Another Interval Library
 *--------------------------------------------------------------------------
 * Tests of this fork of GAOL: numbers and constants.
 *
 * An interval read from a number, interval("0.1"), has to be the tightest
 * interval of doubles enclosing it, and the double itself when the number is
 * one, whatever the C library. Each number is compared exactly with the
 * bounds read. The constants have to be the tightest intervals enclosing pi,
 * 2pi and pi/2, and the hexadecimal output to give the bounds bit for bit.
 *--------------------------------------------------------------------------
 * gaol is a software distributed WITHOUT ANY WARRANTY. Read the associated
 * COPYING file for information.
 *--------------------------------------------------------------------------*/

#include "gaol_tests.h"

#include <cstdlib>

using namespace gaol;
using namespace gaol_tests;

namespace
{
  const int nb_random_values = 2000;

  // The exact value of the number s, written as GAOL reads numbers: decimal
  // digits with an optional point and exponent
  Exact decimal(const std::string& s)
  {
    Natural n;
    long k = 0;
    bool fraction = false;
    std::size_t i = 0;
    for (; i < s.size() && s[i] != 'e' && s[i] != 'E'; ++i) {
      if (s[i] == '.') {
        fraction = true;
        continue;
      }
      n = n*Natural(10) + Natural(static_cast<std::uint64_t>(s[i] - '0'));
      if (fraction) {
        --k;
      }
    }
    if (i < s.size()) {
      k += std::stol(s.substr(i + 1));
    }
    // n*10^k = n*2^k*5^k
    Natural five_k(1);
    for (long j = 0; j < (k < 0 ? -k : k); ++j) {
      five_k = five_k*Natural(5);
    }
    Dyadic scaled, power_of_five;
    scaled.sign = n.is_zero() ? 0 : 1;
    scaled.m = n;
    scaled.e = k;
    power_of_five.sign = 1;
    power_of_five.m = five_k;
    return (k >= 0) ? exact(scaled*power_of_five) : quotient(scaled, power_of_five);
  }

  void expect_number(const std::string& name, const std::string& s)
  {
    const interval r(s.c_str());
    check(name, is_tightest_enclosure(r, decimal(s)), [&] { return "\"" + s + "\": " + hex(r); });
  }

  // s formatted by snprintf()
  std::string format(const char *f, double x)
  {
    std::vector<char> buffer(4096);
    std::snprintf(buffer.data(), buffer.size(), f, x);
    return buffer.data();
  }

  void numbers()
  {
    const char *const numbers[] = {
      "0", "0.0", "1", "2.5", "0.5", "0.25", ".5", "5.", "0.1", "0.3", "1.1", "3.14159", "000123.4500e-2",
      "1e22", "1e23", "1E+23", "9007199254740992", "9007199254740993", "18014398509481985",
      "123456789012345678901234567890", "1e308", "1.7976931348623157e308", "1.7976931348623159e308",
      "1e309", "1e400", "1e100000", "2.2250738585072014e-308", "1e-320", "4.9406564584124654e-324",
      "2.4703282292062327e-324", "2.4703282292062328e-324", "1e-400", "1e-100000",
      "0.1000000000000000055511151231257827021181583404541015625",
      "0.1000000000000000055511151231257827021181583404541015626",
      "0.1000000000000000055511151231257827021181583404541015624",
    };
    for (const char *s : numbers) {
      expect_number("interval(number): the tightest enclosure of the number", s);
    }

    // Doubles written with 17 significant digits, and exactly
    Random random;
    for (int i = 0; i < nb_random_values; ++i) {
      const double x = std::fabs(random.any());
      expect_number("interval(number) for numbers of 17 significant digits", format("%.16e", x));
      expect_number("interval(number) for the decimal expansions of doubles", format("%.1100f", x));
      const double y = random.positive(-30, 30);
      expect_number("interval(number) for numbers of 17 significant digits", format("%.16e", y));
      expect_number("interval(number) for the decimal expansions of doubles", format("%.1100f", y));
    }

    // Numbers in intervals and expressions
    const Exact tenth = decimal("0.1"), three_tenths = decimal("0.3");
    const Exact third = quotient(dyadic(1.0), dyadic(3.0));
    const struct { const char *s; Exact lo, hi; } expressions[] = {
      { "[0.1, 0.3]", tenth, three_tenths },
      { "[0.1]", tenth, tenth },
      { "-0.1", -tenth, -tenth },
      { "[-0.3, -0.1]", -three_tenths, -tenth },
      { "1/3", third, third },
      { "[1/3, 0.3]", third, three_tenths },
    };
    for (const auto& e : expressions) {
      const interval r(e.s);
      check("interval(expression): the tightest enclosure", is_tightest_enclosure(r, e.lo, e.hi),
            [&] { return std::string("\"") + e.s + "\": " + hex(r); });
    }
    const interval two(".1", "0.3");
    check("interval(number, number): the tightest enclosure", is_tightest_enclosure(two, tenth, three_tenths),
          [&] { return hex(two); });
    const interval sum("0.1+0.2");
    check("interval(\"0.1+0.2\") encloses 3/10", is_enclosure(sum, three_tenths), [&] { return hex(sum); });
  }

  void constants()
  {
    const double pi_below = 0x1.921fb54442d18p+1, pi_above = 0x1.921fb54442d19p+1;
    const struct { const char *name; interval r; double lo, hi; } constants[] = {
      { "interval::pi()", interval::pi(), pi_below, pi_above },
      { "interval::two_pi()", interval::two_pi(), pi_below*2.0, pi_above*2.0 },
      { "interval::half_pi()", interval::half_pi(), pi_below/2.0, pi_above/2.0 },
      { "interval(\"pi\")", interval("pi"), pi_below, pi_above },
      { "interval::zero()", interval::zero(), 0.0, 0.0 },
      { "interval::one()", interval::one(), 1.0, 1.0 },
      { "interval::minus_one_plus_one()", interval::minus_one_plus_one(), -1.0, 1.0 },
      { "interval::one_plus_infinity()", interval::one_plus_infinity(), 1.0, inf },
      { "interval::universe()", interval::universe(), -inf, inf },
      { "interval::positive()", interval::positive(), 0.0, inf },
      { "interval::negative()", interval::negative(), -inf, 0.0 },
      { "interval(\"[dmax, inf]\")", interval("[dmax, inf]"), std::numeric_limits<double>::max(), inf },
    };
    for (const auto& c : constants) {
      check(std::string(c.name) + ": the tightest enclosure", c.r.left() == c.lo && c.r.right() == c.hi,
            [&] { return hex(c.r); });
    }
    check("interval::emptyset(): empty", interval::emptyset().is_empty());
  }

  // The hexadecimal output gives the bits of the bounds
  void hexadecimal_output()
  {
    const interval_format::format_t saved = interval::format();
    interval::format(interval_format::hexa);
    Random random;
    for (int i = 0; i < nb_random_values; ++i) {
      const interval x = hull(random.any(), random.any());
      std::ostringstream s;
      s << x;
      std::uint64_t l = 0, r = 0;
      const bool read = std::sscanf(s.str().c_str(), "[%16llx, %16llx]",
                                    static_cast<unsigned long long*>(static_cast<void*>(&l)),
                                    static_cast<unsigned long long*>(static_cast<void*>(&r))) == 2;
      std::uint64_t lx, rx;
      const double left = x.left(), right = x.right();
      std::memcpy(&lx, &left, sizeof lx);
      std::memcpy(&rx, &right, sizeof rx);
      check("operator<< in hexadecimal: the bits of the bounds", read && l == lx && r == rx,
            [&] { return hex(x) + " written " + s.str(); });
    }
    interval::format(saved);
  }
}

int main()
{
  gaol::init();
  numbers();
  constants();
  hexadecimal_output();
  const int status = summary();
  gaol::cleanup();
  return status;
}
