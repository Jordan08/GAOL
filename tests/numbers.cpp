/*-*-C++-*------------------------------------------------------------------
 * gaol -- NOT Just Another Interval Library
 *--------------------------------------------------------------------------
 * Tests of this fork of GAOL: numbers, constants and constructors.
 *
 * An interval read from a number, interval("0.1"), has to be the tightest
 * interval of doubles enclosing it, and the double itself when the number is
 * one, whatever the C library. Each number is compared exactly with the
 * bounds read. The constants have to be the tightest intervals enclosing pi,
 * 2pi and pi/2, and the hexadecimal output to give the bounds bit for bit.
 * The constructors have to give the empty set where their arguments are not
 * an interval: an infinite lower bound of +oo, an upper bound of -oo, bounds
 * in the wrong order, and NaN bounds. The interval literals of IEEE 1788-2015
 * (9.7, 12.11) have to be read, whatever the case of their letters: [ ],
 * [empty], [entire], bounds left out or infinite, hexadecimal numbers and the
 * uncertain form 3.56?1.
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

    // Doubles written with 17 significant digits, and exactly, with up to 1100
    // digits after the point (fewer of those, which take longer)
    Random random;
    for (int i = 0; i < nb_random_values; ++i) {
      const double x = std::fabs(random.any()), y = random.positive(-30, 30);
      expect_number("interval(number) for numbers of 17 significant digits", format("%.16e", x));
      expect_number("interval(number) for numbers of 17 significant digits", format("%.16e", y));
      if (i % 10 == 0) {
        expect_number("interval(number) for the decimal expansions of doubles", format("%.1100f", x));
        expect_number("interval(number) for the decimal expansions of doubles", format("%.1100f", y));
      }
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
      { "[0.3, 1/3]", three_tenths, third },
    };
    for (const auto& e : expressions) {
      const interval r(e.s);
      check("interval(expression): the tightest enclosure", is_tightest_enclosure(r, e.lo, e.hi),
            [&] { return std::string("\"") + e.s + "\": " + hex(r); });
    }
    // Bounds in the wrong order: the empty set, as interval(2, 1) is
    check("interval(\"[1/3, 0.3]\"): empty", interval("[1/3, 0.3]").is_empty(),
          [&] { return hex(interval("[1/3, 0.3]")); });
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

  // The constructors: the empty set where their arguments are not an interval,
  // as in IBEX, IEEE 1788-2015 having no interval [+oo, +oo] nor [-oo, -oo]
  void constructors()
  {
    const double nan = std::numeric_limits<double>::quiet_NaN();
    const double largest = std::numeric_limits<double>::max();
    const struct { const char *name; interval r; } empty[] = {
      { "interval(+oo)", interval(inf) },
      { "interval(-oo)", interval(-inf) },
      { "interval(+oo, +oo)", interval(inf, inf) },
      { "interval(-oo, -oo)", interval(-inf, -inf) },
      { "interval(+oo, 1)", interval(inf, 1.) },
      { "interval(1, -oo)", interval(1., -inf) },
      { "interval(2, 1)", interval(2., 1.) },
      { "interval(NaN)", interval(nan) },
      { "interval(NaN, 1)", interval(nan, 1.) },
      { "interval(1, NaN)", interval(1., nan) },
    };
    for (const auto& c : empty) {
      check(std::string(c.name) + ": empty", c.r.is_empty(), [&] { return hex(c.r); });
    }
    interval to_infinity(1., 2.);
    to_infinity = inf;
    check("[x] = +oo: empty", to_infinity.is_empty(), [&] { return hex(to_infinity); });
    interval to_minus_infinity(1., 2.);
    to_minus_infinity = -inf;
    check("[x] = -oo: empty", to_minus_infinity.is_empty(), [&] { return hex(to_minus_infinity); });

    // The intervals with one infinite bound are unchanged
    const struct { const char *name; interval r; double lo, hi; } kept[] = {
      { "interval(-oo, 1)", interval(-inf, 1.), -inf, 1. },
      { "interval(1, +oo)", interval(1., inf), 1., inf },
      { "interval(-oo, +oo)", interval(-inf, inf), -inf, inf },
      { "interval(1)", interval(1.), 1., 1. },
      { "interval(-0., 0.)", interval(-0., 0.), -0., 0. },
      { "interval(-dmax, dmax)", interval(-largest, largest), -largest, largest },
    };
    for (const auto& c : kept) {
      check(std::string(c.name) + ": kept", !c.r.is_empty() && c.r.left() == c.lo && c.r.right() == c.hi,
            [&] { return hex(c.r); });
    }
    check("midpoint([1, +oo])", interval(1., inf).midpoint() == largest);
    check("midpoint([-oo, 1])", interval(-inf, 1.).midpoint() == -largest);
  }

  // The interval literals of IEEE 1788-2015 (9.7, 12.11): GAOL read neither
  // [entire], [ ], the bounds left out, infinity, the hexadecimal numbers nor
  // the uncertain form, and the case of the letters mattered
  void ieee_literals()
  {
    const Exact third = quotient(dyadic(1.0), dyadic(3.0));
    // The examples of Tables 9.4 and 12.1, and others
    const struct { const char *s; Exact lo, hi; } tightest[] = {
      { "[1.e-3, 1.1e-3]", decimal("1.e-3"), decimal("1.1e-3") },
      { "[-0x1.3p-1, 2/3]", exact(-0x1.3p-1), quotient(dyadic(2.0), dyadic(3.0)) },
      { "[3.56]", decimal("3.56"), decimal("3.56") },
      { "3.56?1", decimal("3.55"), decimal("3.57") },
      { "3.56?1e2", decimal("355"), decimal("357") },
      { "3.560?2", decimal("3.558"), decimal("3.562") },
      { "3.56?", decimal("3.555"), decimal("3.565") },
      { "3.560?2u", decimal("3.560"), decimal("3.562") },
      { "-10?", -decimal("10.5"), -decimal("9.5") },
      { "-10?u", -decimal("10.0"), -decimal("9.5") },
      { "-10?12", -decimal("22"), decimal("2") },
      { "0.1?1", decimal("0"), decimal("0.2") },
      { "+.5?D", decimal(".45"), decimal(".5") },
      { "1.?3E-400", -decimal("2e-400"), decimal("4e-400") },
      { "1?e400", decimal("0.5e400"), decimal("1.5e400") },
      { "[0x1.00000000000001p0]", exact(dyadic(1.0) + dyadic(0x1p-56)), exact(dyadic(1.0) + dyadic(0x1p-56)) },
      { "[0X.8P1, 0xAp-1]", exact(1.0), exact(5.0) },
      { "[0x.2P0, 1/3]", exact(0.125), third },
    };
    for (const auto& c : tightest) {
      const interval r = evaluate(std::string("interval(\"") + c.s + "\")", [&] { return interval(c.s); },
                                  [] { return std::string(); });
      check("IEEE 1788 literals: the tightest enclosure", !r.is_empty() && is_tightest_enclosure(r, c.lo, c.hi),
            [&] { return std::string("\"") + c.s + "\": " + hex(r); });
    }

    const double largest = std::numeric_limits<double>::max();
    const struct { const char *s; double lo, hi; bool empty; } exactly[] = {
      { "[ ]", 0., 0., true },
      { "[]", 0., 0., true },
      { "[empty]", 0., 0., true },
      { "[ Empty ]", 0., 0., true },
      { "[EMPTY]", 0., 0., true },
      { "[entire]", -inf, inf, false },
      { "[ Entire ]", -inf, inf, false },
      { "[,]", -inf, inf, false },
      { "[1,]", 1., inf, false },
      { "[, 2]", -inf, 2., false },
      { "[0x1.3p-1,]", 0x1.3p-1, inf, false },
      { "[1, inf]", 1., inf, false },
      { "[1,Inf]", 1., inf, false },
      { "[-INFINITY, +infinity]", -inf, inf, false },
      { "[-Inf, 2]", -inf, 2., false },
      { "[1e309, inf]", largest, inf, false },
      { "-10??u", -10., inf, false },
      { "-10??d", -inf, -10., false },
      { "-10??", -inf, inf, false },
      { "[0x1p-1074]", 0x1p-1074, 0x1p-1074, false },
      { "[0x1.fffffffffffffp1023]", largest, largest, false },
      { "[0x1.fffffffffffff8p1023]", largest, inf, false },
      // numsToInterval has no value for a lower bound +oo nor an upper bound
      // -oo (10.5.8): these are the empty set
      { "[inf]", 0., 0., true },
      { "[-inf]", 0., 0., true },
      { "[inf, inf]", 0., 0., true },
      { "[-inf, -inf]", 0., 0., true },
      { "[+inf,]", 0., 0., true },
      { "[, -Infinity]", 0., 0., true },
      { "[2, 1]", 0., 0., true },
      { "[0x2p0, 1/1]", 0., 0., true },
    };
    for (const auto& c : exactly) {
      const interval r = evaluate(std::string("interval(\"") + c.s + "\")", [&] { return interval(c.s); },
                                  [] { return std::string(); });
      check("IEEE 1788 literals: exact", c.empty ? r.is_empty() : (!r.is_empty() && r.left() == c.lo && r.right() == c.hi),
            [&] { return std::string("\"") + c.s + "\": " + hex(r); });
    }

    // Not literals of IEEE 1788 (12.11.3): input_format_error
    const char *const invalid[] = { "[5?1]", "[1 000 000]", "[ganz]", "[entire!comment]", "5???u", "[1,2,3]", "3.56?1?" };
    for (const char *s : invalid) {
      bool threw = false;
      try {
        interval x(s);
      } catch (input_format_error&) {
        threw = true;
      } catch (...) {
      }
      check("IEEE 1788 literals: input_format_error for what is not one", threw, [&] { return std::string(s); });
    }

    // Doubles written in hexadecimal, with 13 digits after the point, are read
    // exactly; with a 14th digit 8, half a unit above them, as the tightest
    // interval
    Random random;
    for (int i = 0; i < nb_random_values; ++i) {
      const double x = random(-1000, 1000);
      const std::string s = format("%.13a", x);
      const interval r(("[" + s + "]").c_str());
      check("interval(\"[hexadecimal double]\"): the double", r.left() == x && r.right() == x,
            [&] { return s + ": " + hex(r); });
      std::string t = s;
      t.insert(t.find_first_of("pP"), "8");
      const Dyadic half_unit = dyadic(std::ldexp(x < 0.0 ? -1.0 : 1.0, std::ilogb(x) - 53));
      const interval u(("[" + t + "]").c_str());
      check("interval(\"[hexadecimal number]\"): the tightest enclosure",
            is_tightest_enclosure(u, exact(dyadic(x) + half_unit)), [&] { return t + ": " + hex(u); });
    }
  }

  // The hexadecimal output gives the bits of the bounds
  // Expressions read again and again, and expressions GAOL cannot compute or
  // read: GAOL's parser did not free their nodes, which LeakSanitizer reports
  // in the tests built with it, and the null node it gives nth_root() and
  // pow() for an exponent it cannot use was deleted with the expression
  void expressions()
  {
    const char *const valid[] = { "sin(1)+cos(2)*2", "[-(1+2), exp(1)/3]", "(pi+1)*(pi-1)", "-[1,2]/tan(1)",
                                  "pow(2, 3)+pow(2, -2)+pow(2, 0)+pow(2, 0.5)", "nth_root(8, 3)+sqrt(4)",
                                  "[cosh(1), sinh(2)+tanh(1)]", "<1+1, 2>", "[1,]+[,2]", "[-inf, log(2)]" };
    for (const char *s : valid) {
      const interval first(s);
      bool same = true;
      for (int i = 0; i < 100; ++i) {
        const interval r(s);
        same = same && r.left() == first.left() && r.right() == first.right();
      }
      check("interval(expression) read again: the same interval", same, [&] { return std::string(s); });
    }
    const char *const invalid[] = { "nth_root(8, 1.5)", "nth_root(8, 1.5)+1", "[nth_root(8, 1.5), 2]",
                                    "sin(1)+", "[sin(1), cos(", "(1+2", "[1, 2*(3+4]", "pow(2, 1)+*3",
                                    "<1, 2>", "exp(1) exp(2)", "nth_root([1,2], 1.5)",
                                    "1+pow(2, atan2(1,1))", "(1+2)*pow(1+2, atan2(1,1)+1)-3",
                                    "[1+nth_root(8, atan2(1,1)), 2]" };
    for (int i = 0; i < 10; ++i) {
      for (const char *s : invalid) {
        bool threw = false;
        try {
          interval x(s);
        } catch (...) {
          threw = true;
        }
        check("interval(expression) not computed: an exception", threw, [&] { return std::string(s); });
      }
    }
    // The exception thrown in the exponent of pow() goes through the parser
    bool unavailable = false;
    try {
      interval x("1+pow(2, atan2(1,1))");
    } catch (unavailable_feature_error&) {
      unavailable = true;
    } catch (...) {
    }
    check("interval(\"1+pow(2, atan2(1,1))\"): unavailable_feature_error", unavailable);
    const interval r("1+2");
    check("interval(expression) after expressions not computed", r.left() == 3.0 && r.right() == 3.0,
          [&] { return hex(r); });
  }

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

  // The exact value of the bound s that operator<< wrote, with its sign
  Exact written(const std::string& s)
  {
    return (s[0] == '-') ? -decimal(s.substr(1)) : decimal(s[0] == '+' ? s.substr(1) : s);
  }

  // The interval [l, r] written by operator<< with the flags and the precision
  // given has to enclose [l, r], whatever the C library does of the rounding
  // direction in its decimal conversions (issue #3), and each bound has to be
  // less than one unit of its last digit away: 10^-precision in the fixed
  // format, 10^-precision times the power of ten written in the scientific
  // one, and at most 10^(1 - precision) times the bound in the general one
  void expect_output(const std::string& name, double l, double r, std::ios_base::fmtflags flags, int precision)
  {
    std::ostringstream os;
    os.flags(flags);
    interval::precision(precision);
    os << interval(l, r);
    const std::string s = os.str();
    const std::size_t comma = s.find(", ");
    const auto describe = [&] { return hex(interval(l, r)) + " with the precision " + std::to_string(precision) + " written " + s; };
    if (!check(name + ": two bounds", comma != std::string::npos && s.size() >= comma + 4, describe)) {
      return;
    }
    const std::string sl = s.substr(1, comma - 1), sr = s.substr(comma + 2, s.size() - comma - 3);
    const Exact vl = written(sl), vr = written(sr);
    check(name + ": encloses the interval", compare(l, vl) >= 0 && compare(r, vr) <= 0, describe);

    const std::ios_base::fmtflags floatfield = flags & std::ios_base::floatfield;
    const std::string bounds[2] = { sl, sr };
    const Exact values[2] = { vl, vr };
    const double doubles[2] = { l, r };
    for (int i = 0; i < 2; ++i) {
      Exact unit = decimal("1e-" + std::to_string(precision));
      if (floatfield == std::ios_base::scientific) {
        unit = unit*decimal("1" + bounds[i].substr(bounds[i].find('e')));
      } else if (floatfield != std::ios_base::fixed) {
        const Exact magnitude = (compare(doubles[i], exact(0.0)) < 0) ? -exact(doubles[i]) : exact(doubles[i]);
        unit = magnitude*decimal("1e" + std::to_string(1 - precision));
      }
      // vl + unit > l, and vr - unit < r
      const bool tight = (doubles[i] == 0.0) ? compare(0.0, values[i]) == 0
                         : ((i == 0) ? compare(l, vl + unit) < 0 : compare(r, vr + (-unit)) > 0);
      check(name + ": less than one unit of the last digit away", tight, describe);
    }
  }

  void decimal_output()
  {
    const interval_format::format_t saved_format = interval::format();
    const std::streamsize saved_precision = interval::precision();
    interval::format(interval_format::bounds);
    const std::ios_base::fmtflags general = std::ios_base::fmtflags(), scientific = std::ios_base::scientific,
      fixed = std::ios_base::fixed, showpoint = std::ios_base::showpoint;
    const int precisions[] = { 1, 2, 4, 6, 15, 16, 17, 20 };

    // Bounds a unit of the last digit from a power of ten, where a digit moved
    // outward changes the number of digits or the exponent, and those of the
    // issue
    const double special[] = {
      2.0/3.0, 123456.789, 0.1, 0.3, 1.0, 10.0, 1e6, 1e-5, 0.5, 9.5, 0.95, 99.5, 999999.5, 999999.7, 1000000.3,
      9.9999995, 0.99999996, 0.000999999997, 0.0001, 0.00010000001, 99999.97, 1e22, 1e23, 9.999999999999999e22,
      0x1.fffffffffffffp-1, 0x1.0000000000001p+0, 0x1.fffffffffffffp+1023, 0x1p-1022, 0x0.0000000000001p-1022,
      123456789012345678.0, 0.001, 0.00099999999999999, 5e-324, 1.5, 2.5, 1e15, 1e16, 1e17, 9999999999999998.0,
    };
    for (double x : special) {
      for (int p : precisions) {
        for (std::ios_base::fmtflags flags : { general, general | showpoint, scientific, fixed }) {
          if (flags == fixed && (std::fabs(x) > 1e30 || std::fabs(x) < 1e-30)) {
            continue; // Hundreds of digits
          }
          expect_output("operator<< in decimal, near powers of ten", x, x, flags, p);
          expect_output("operator<< in decimal, near powers of ten", -x, -x, flags, p);
          expect_output("operator<< in decimal, near powers of ten", -x, x, flags, p);
        }
      }
    }

    Random random;
    for (int i = 0; i < nb_random_values; ++i) {
      const interval x = hull(random.any(), random.any());
      const double a = random.uniform(-1000.0, 1000.0), b = random.uniform(-1.0, 1.0);
      const interval y = hull(a, b);
      const int p = precisions[i % 8];
      expect_output("operator<< in decimal, general format", x.left(), x.right(), (i % 2 == 0) ? general : general | showpoint, p);
      expect_output("operator<< in decimal, general format", y.left(), y.right(), general, p);
      expect_output("operator<< in decimal, scientific format", x.left(), x.right(), scientific, p);
      expect_output("operator<< in decimal, scientific format", y.left(), y.right(), scientific, p);
      expect_output("operator<< in decimal, fixed format", y.left(), y.right(), fixed, p);
      // What is written is read back as an interval enclosing the one written
      std::ostringstream os;
      interval::precision(p);
      os << y;
      const interval back(os.str().c_str());
      check("interval(text written by operator<<): encloses the interval", back.set_contains(y),
            [&] { return hex(y) + " written " + os.str() + " read " + hex(back); });
    }
    interval::precision(saved_precision);
    interval::format(saved_format);
  }
}

int main()
{
  gaol::init();
  numbers();
  constants();
  constructors();
  ieee_literals();
  expressions();
  hexadecimal_output();
  decimal_output();
  const int status = summary();
  gaol::cleanup();
  return status;
}
