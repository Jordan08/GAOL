/*-*-C++-*------------------------------------------------------------------
 * gaol -- NOT Just Another Interval Library
 *--------------------------------------------------------------------------
 * Tests of GAOL v5: numbers, constants and constructors.
 *
 * An interval read from a number, textToInterval("0.1"), has to be the
 * tightest interval of doubles enclosing it, and the double itself when the
 * number is one, whatever the C library, and whether the processor flushes the
 * subnormals to zero or not. Each number is compared exactly with the bounds
 * read. The constants have to be the tightest intervals enclosing
 * pi, 2pi and pi/2, and the hexadecimal output to be read back bit for bit.
 * The constructors have to give the empty set where their arguments are not
 * an interval: an infinite lower bound of +oo, an upper bound of -oo, bounds
 * in the wrong order, and NaN bounds. The interval literals of IEEE 1788-2015
 * (9.7, 12.11) have to be read, whatever the case of their letters: [ ],
 * [empty], [entire], bounds left out or infinite, hexadecimal numbers and the
 * uncertain form 3.56?1.
 *
 * Copyright (c) 2026 ENSTA, France
 *
 * Created 2026-09-20 by Jordan NININ
 *--------------------------------------------------------------------------
 * gaol is a software distributed WITHOUT ANY WARRANTY. Read the associated
 * COPYING file for information.
 *--------------------------------------------------------------------------*/

#include "gaol_tests.h"

#include <chrono>
#include <clocale>
#include <cstdlib>
#include <iomanip>
#include <type_traits>

// The control register of the SSE instructions, where the flush-to-zero and
// denormals-are-zero modes are set
#if defined(__SSE2__) || defined(_M_X64) || (defined(_M_IX86_FP) && _M_IX86_FP >= 2)
#  include <xmmintrin.h>
#  define GAOL_TESTS_HAVE_MXCSR 1
#endif

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
    const interval r = textToInterval(s);
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
      expect_number("textToInterval(number): the tightest enclosure of the number", s);
    }

    // Doubles written with 17 significant digits, and exactly, with up to 1100
    // digits after the point (fewer of those, which take longer)
    Random random;
    for (int i = 0; i < nb_random_values; ++i) {
      const double x = std::fabs(random.any()), y = random.positive(-30, 30);
      expect_number("textToInterval(number) for numbers of 17 significant digits", format("%.16e", x));
      expect_number("textToInterval(number) for numbers of 17 significant digits", format("%.16e", y));
      if (i % 10 == 0) {
        expect_number("textToInterval(number) for the decimal expansions of doubles", format("%.1100f", x));
        expect_number("textToInterval(number) for the decimal expansions of doubles", format("%.1100f", y));
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
      const interval r = textToInterval(e.s);
      check("textToInterval(expression): the tightest enclosure", is_tightest_enclosure(r, e.lo, e.hi),
            [&] { return std::string("\"") + e.s + "\": " + hex(r); });
    }
    // Bounds in the wrong order: the empty set, as interval(2, 1) is
    check("textToInterval(\"[1/3, 0.3]\"): empty", textToInterval("[1/3, 0.3]").is_empty(),
          [&] { return hex(textToInterval("[1/3, 0.3]")); });
    const interval two = textToInterval(".1", "0.3");
    check("textToInterval(number, number): the tightest enclosure", is_tightest_enclosure(two, tenth, three_tenths),
          [&] { return hex(two); });
    const interval sum = textToInterval("0.1+0.2");
    check("textToInterval(\"0.1+0.2\") encloses 3/10", is_enclosure(sum, three_tenths), [&] { return hex(sum); });
  }

#if GAOL_TESTS_HAVE_MXCSR
  // The bits of MXCSR that flush the subnormals: flush-to-zero (bit 15), which
  // flushes the subnormal results of the operations to 0, and
  // denormals-are-zero (bit 6), which makes the operations and the
  // comparisons read a subnormal as 0
  const unsigned int flush_to_zero = 0x8000u, denormals_are_zero = 0x0040u;

  // Sets the bits of MXCSR it is given for its lifetime, and restores them as
  // it found them afterwards, the rounding direction and the flags being left
  // alone
  class Flushing
  {
    public:

      explicit Flushing(unsigned int bits)
        : bits_(bits), saved_(_mm_getcsr() & bits)
      {
        _mm_setcsr(_mm_getcsr() | bits_);
      }

      ~Flushing()
      {
        _mm_setcsr((_mm_getcsr() & ~bits_) | saved_);
      }

    private:

      const unsigned int bits_, saved_;
  };

  // Whether the processor reads a subnormal as 0 with denormals-are-zero: the
  // least positive double is then not above 0. The result of the comparison is
  // written to volatile memory before the mode is restored: GCC and Clang do
  // not model MXCSR, and made the comparison after the destructor of Flushing
  // otherwise, where the mode is not set anymore.
  bool honours_denormals_are_zero()
  {
    volatile double least = std::numeric_limits<double>::denorm_min();
    volatile bool not_above_zero;
    {
      Flushing flushing(denormals_are_zero);
      not_above_zero = !(least > 0.0);
    }
    return not_above_zero;
  }
#endif

  /*
    Numbers read with flush-to-zero and denormals-are-zero set (GAOL v5), as a
    program linked with -Ofast has them (crtfastmath.o), or which loads code
    that has them set. The reader compared each number with the doubles around
    it, taken as doubles, and with denormals-are-zero a subnormal is 0 in the
    comparisons and for std::frexp(): 1e-310 was above every subnormal, and
    read as [0x0.fffffffffffffp-1022, 0x1p-1022], the interval from the
    greatest subnormal to the least normal double, which does not enclose it,
    and 0 was equal to every subnormal, and read as the greatest one. The mode
    is set for the reading only, the exact arithmetic of the checks reading
    doubles as doubles too. x86 only, where MXCSR is: nothing is checked
    elsewhere, nor where the processor does not honour denormals-are-zero.
  */
  void subnormal_numbers()
  {
#if GAOL_TESTS_HAVE_MXCSR
    if (!honours_denormals_are_zero()) {
      std::printf("Denormals-are-zero is not honoured: the reading of numbers with it is not checked\n");
      return;
    }
    const struct { const char *name; unsigned int bits; } modes[] = {
      { "flush-to-zero", flush_to_zero },
      { "denormals-are-zero", denormals_are_zero },
      { "flush-to-zero and denormals-are-zero", flush_to_zero | denormals_are_zero },
    };
    // The exact value of the number read, or the exact bounds of the interval
    struct Text { std::string s; Exact lo, hi; };
    const auto number = [](const std::string& s) { return Text{ s, decimal(s), decimal(s) }; };
    // 2^-1075, half the least positive double, which is no double
    Dyadic half_least;
    half_least.sign = 1;
    half_least.m = Natural(1);
    half_least.e = -1075;
    std::vector<Text> texts = {
      number("0"),
      number("0.0"),
      number("1e-400"),                   // Below the least subnormal
      number("2.5e-324"),                 // Between 0 and the least subnormal, 4.94e-324
      number("5e-324"),                   // Between the two least subnormals
      number("1e-320"),
      number("1e-310"),
      number("1e-309"),
      number("2.2250738585072008e-308"),  // Below the greatest subnormal
      number("2.2250738585072011e-308"),  // Between it and the least normal double
      number("2.2250738585072014e-308"),  // Just above the least normal double
      number("1e-300"),
      { "-1e-310", -decimal("1e-310"), -decimal("1e-310") },
      { "[1e-310, 1e-309]", decimal("1e-310"), decimal("1e-309") },
      { "1?e-310", decimal("0.5e-310"), decimal("1.5e-310") },
      // Hexadecimal: the least subnormal, half the least normal double and the
      // greatest subnormal, which are doubles, and a number between the
      // greatest subnormal and the least normal double
      { "0x1p-1074", exact(0x1p-1074), exact(0x1p-1074) },
      { "0x0.8p-1022", exact(0x0.8p-1022), exact(0x0.8p-1022) },
      { "0x0.fffffffffffffp-1022", exact(0x0.fffffffffffffp-1022), exact(0x0.fffffffffffffp-1022) },
      { "0x1.fffffffffffffp-1023", exact(dyadic(0x1p-1022) - half_least), exact(dyadic(0x1p-1022) - half_least) },
    };
    // Subnormals and the least normal doubles, written with 17 digits, and
    // their exact decimal expansions, whose reading gives them back
    Random random;
    for (int i = 0; i < 100; ++i) {
      const double x = random.positive(-1023, -1023), y = random.positive(-1023, -1021);
      for (double d : { x, y }) {
        texts.push_back(number(format("%.16e", d)));
        texts.push_back(number(format("%.1100f", d)));
      }
    }
    for (const auto& mode : modes) {
      const std::string name = std::string("textToInterval(number) with ") + mode.name + ": the tightest enclosure";
      for (const auto& t : texts) {
        const interval r = evaluate(name, [&] { Flushing flushing(mode.bits); return textToInterval(t.s); },
                                    [&] { return t.s.substr(0, 40); });
        check(name, !r.is_empty() && is_tightest_enclosure(r, t.lo, t.hi),
              [&] { return "\"" + t.s.substr(0, 40) + "\": " + hex(r); });
      }
    }
#else
    std::printf("No control register of the SSE instructions: the reading of numbers with flush-to-zero and denormals-are-zero is not checked\n");
#endif
  }

  void constants()
  {
    const double pi_below = 0x1.921fb54442d18p+1, pi_above = 0x1.921fb54442d19p+1;
    const struct { const char *name; interval r; double lo, hi; } constants[] = {
      { "interval::pi()", interval::pi(), pi_below, pi_above },
      { "interval::two_pi()", interval::two_pi(), pi_below*2.0, pi_above*2.0 },
      { "interval::half_pi()", interval::half_pi(), pi_below/2.0, pi_above/2.0 },
      { "textToInterval(\"pi\")", textToInterval("pi"), pi_below, pi_above },
      { "interval::zero()", interval::zero(), 0.0, 0.0 },
      { "interval::one()", interval::one(), 1.0, 1.0 },
      { "interval::minus_one_plus_one()", interval::minus_one_plus_one(), -1.0, 1.0 },
      { "interval::one_plus_infinity()", interval::one_plus_infinity(), 1.0, inf },
      { "interval::universe()", interval::universe(), -inf, inf },
      { "interval::positive()", interval::positive(), 0.0, inf },
      { "interval::negative()", interval::negative(), -inf, 0.0 },
      { "textToInterval(\"[dmax, inf]\")", textToInterval("[dmax, inf]"), std::numeric_limits<double>::max(), inf },
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

    /* No constructor from one string (GAOL v5): any const char*, nullptr
       included, converted to an interval, and interval(0) was ambiguous */
    static_assert(!std::is_constructible<interval, const char*>::value, "no interval(const char*)");
    static_assert(!std::is_convertible<const char*, interval>::value, "no conversion from const char*");
    static_assert(!std::is_constructible<interval, std::nullptr_t>::value, "no interval(nullptr)");
    static_assert(!std::is_convertible<std::nullptr_t, interval>::value, "no conversion from nullptr");
    const interval zero(0);
    check("interval(0) with an int: [0, 0]", !zero.is_empty() && zero.left() == 0. && zero.right() == 0.,
          [&] { return hex(zero); });

    /* No constructor from two strings either (GAOL v5): textToInterval(sl,
       sr) reads them, and interval(0, 0) is [0] */
    static_assert(!std::is_constructible<interval, const char*, const char*>::value, "no interval(const char*, const char*)");
    const interval zero_zero(0, 0);
    check("interval(0, 0) with ints: [0, 0]", !zero_zero.is_empty() && zero_zero.left() == 0. && zero_zero.right() == 0.,
          [&] { return hex(zero_zero); });
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
      const interval r = evaluate(std::string("textToInterval(\"") + c.s + "\")", [&] { return textToInterval(c.s); },
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
      const interval r = evaluate(std::string("textToInterval(\"") + c.s + "\")", [&] { return textToInterval(c.s); },
                                  [] { return std::string(); });
      check("IEEE 1788 literals: exact", c.empty ? r.is_empty() : (!r.is_empty() && r.left() == c.lo && r.right() == c.hi),
            [&] { return std::string("\"") + c.s + "\": " + hex(r); });
    }

    // Not literals of IEEE 1788 (12.11.3): input_format_error
    /* An uncertain number is an interval literal of its own, not a bound:
       [5?1], and the uncertain numbers as bounds in the other forms, which
       the grammar reading every string as an expression accepted (GAOL v5) */
    const char *const invalid[] = { "[5?1]", "[1 000 000]", "[ganz]", "[entire!comment]", "5???u", "[1,2,3]", "3.56?1?",
                                    "[5?1, 6]", "[1, 3.56?1]", "[3.56?1,]", "[,3.56?1]", "[- 3.56?1]" };
    for (const char *s : invalid) {
      bool threw = false;
      try {
        const interval x = textToInterval(s);
        (void)x;
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
      const interval r = textToInterval("[" + s + "]");
      check("textToInterval(\"[hexadecimal double]\"): the double", r.left() == x && r.right() == x,
            [&] { return s + ": " + hex(r); });
      std::string t = s;
      t.insert(t.find_first_of("pP"), "8");
      const Dyadic half_unit = dyadic(std::ldexp(x < 0.0 ? -1.0 : 1.0, std::ilogb(x) - 53));
      const interval u = textToInterval("[" + t + "]");
      check("textToInterval(\"[hexadecimal number]\"): the tightest enclosure",
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
                                  "[cosh(1), sinh(2)+tanh(1)]", "<1+1, 2>", "[1,]+[,2]", "[-inf, log(2)]",
                                  "1+pow(2, atan2(1,1))", "(1+2)*pow(1+2, atan2(1,1)+1)-3" };
    for (const char *s : valid) {
      const interval first = textToInterval(s);
      bool same = true;
      for (int i = 0; i < 100; ++i) {
        const interval r = textToInterval(s);
        same = same && r.left() == first.left() && r.right() == first.right();
      }
      check("textToInterval(expression) read again: the same interval", same, [&] { return std::string(s); });
    }
    const char *const invalid[] = { "nth_root(8, 1.5)", "nth_root(8, 1.5)+1", "[nth_root(8, 1.5), 2]",
                                    "sin(1)+", "[sin(1), cos(", "(1+2", "[1, 2*(3+4]", "pow(2, 1)+*3",
                                    "<1, 2>", "exp(1) exp(2)", "nth_root([1,2], 1.5)",
                                    "[1+nth_root(8, atan2(1,1)), 2]", "atan2(1)", "atan2(1, 2, 3)" };
    for (int i = 0; i < 10; ++i) {
      for (const char *s : invalid) {
        bool threw = false;
        try {
          const interval x = textToInterval(s);
          (void)x;
        } catch (...) {
          threw = true;
        }
        check("textToInterval(expression) not computed: an exception", threw, [&] { return std::string(s); });
      }
    }
    // atan2(y, x) in expressions: 4 atan2(1, 1) is the tightest enclosure of
    // pi, and atan2(0, 0), which has no value, is empty
    const interval four_angles = textToInterval("4*atan2(1, 1)"), none = textToInterval("atan2(0, 0)"),
      cut = textToInterval("atan2([-1, 1], -1)");
    check("textToInterval(\"4*atan2(1, 1)\"): the tightest enclosure of pi", four_angles.set_eq(interval::pi()),
          [&] { return hex(four_angles); });
    check("textToInterval(\"atan2(0, 0)\"): empty", none.is_empty(), [&] { return hex(none); });
    check("textToInterval(\"atan2([-1, 1], -1)\"): [-pi, pi]", cut.left() == -interval::pi().right()
          && cut.right() == interval::pi().right(), [&] { return hex(cut); });
    const interval r = textToInterval("1+2");
    check("textToInterval(expression) after expressions not computed", r.left() == 3.0 && r.right() == 3.0,
          [&] { return hex(r); });
  }

  /*
    The exact text representation of IEEE 1788-2015 (13.4): written in
    hexadecimal and read again, an interval has to give the same bounds, bit
    for bit, which is the recovery requirement of that subclause. GAOL wrote
    the sixteen digits of each double instead, which is no interval literal and
    which the parser refused (GAOL v5).
  */
  void hexadecimal_output()
  {
    const interval_format::format_t saved = interval::format();
    interval::format(interval_format::hexa);
    const auto round_trip = [&](const interval& x, const char *what) {
      std::ostringstream s;
      s << x;
      bool same = false;
      try {
        const interval y = textToInterval(s.str());
        same = x.is_empty() ? y.is_empty()
                            : (!y.is_empty() && y.left() == x.left() && y.right() == x.right());
      } catch (...) {
        same = false;
      }
      check("operator<< in hexadecimal, read again: the same bounds", same,
            [&] { return std::string(what) + " " + hex(x) + " written " + s.str(); });
    };
    Random random;
    for (int i = 0; i < nb_random_values; ++i) {
      round_trip(hull(random.any(), random.any()), "random");
    }
    /* The values the form has to write apart, or whose digits it has to keep
       all of: the empty set, the infinite bounds, the signed zeros, the
       subnormals and the largest doubles */
    const double max_double = std::numeric_limits<double>::max();
    const double smallest = std::numeric_limits<double>::denorm_min();
    round_trip(interval::emptyset(), "empty");
    round_trip(interval::universe(), "entire");
    round_trip(interval(1.0, GAOL_INFINITY), "[1, +oo]");
    round_trip(interval(-GAOL_INFINITY, -1.0), "[-oo, -1]");
    round_trip(interval(-0.0, 0.0), "[-0, 0]");
    round_trip(interval(smallest, 4.0 * smallest), "subnormal");
    round_trip(interval(-max_double, max_double), "[-MAX, MAX]");
    round_trip(interval::pi(), "pi");
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
      const interval back = textToInterval(os.str());
      check("textToInterval(text written by operator<<): encloses the interval", back.set_contains(y),
            [&] { return hex(y) + " written " + os.str() + " read " + hex(back); });
    }

    // The format writing once the digits both bounds start with, 1.25~[0, 67]
    // being [1.250, 1.2567]: the two numbers it stands for have to enclose
    // the interval too, the right bound keeping its digits when the left one
    // ends with zeros
    interval::format(interval_format::agreeing);
    const auto expect_agreeing = [&](double l, double r, int p) {
      std::ostringstream os;
      interval::precision(p);
      os << interval(l, r);
      const std::string s = os.str();
      const auto describe = [&] { return hex(interval(l, r)) + " with the precision " + std::to_string(p) + " written " + s; };
      const std::size_t tilde = s.find("~[");
      std::string sl = s, sr = s;
      if (s[0] == '[' || s[0] == '<') { // As the format of the bounds, tested above
        return;
      }
      if (tilde != std::string::npos) {
        const std::size_t comma = s.find(", ", tilde);
        if (!check("operator<< with agreeing digits: two bounds", comma != std::string::npos && s[s.size() - 1] == ']'
                   && comma > tilde + 2 && s.size() > comma + 3, describe)) {
          return;
        }
        sl = s.substr(0, tilde) + s.substr(tilde + 2, comma - tilde - 2);
        sr = s.substr(0, tilde) + s.substr(comma + 2, s.size() - comma - 3);
      }
      check("operator<< with agreeing digits: encloses the interval", compare(l, written(sl)) >= 0
            && compare(r, written(sr)) <= 0, describe);
    };
    expect_agreeing(1.25, 1.2567, 5);
    expect_agreeing(100.0, 100.47, 5);
    expect_agreeing(1.2340, 1.2399, 5);
    expect_agreeing(1.5, 1.5078125, 16);
    expect_agreeing(0.1, 0.1, 16);
    expect_agreeing(1.5, 1.5, 6);
    expect_agreeing(150000000.0, 150000010.0, 4);
    expect_agreeing(1e-5, 2e-5, 5);
    for (int i = 0; i < nb_random_values; ++i) {
      const double a = random.positive(-30, 30), w = std::ldexp(random.uniform(0.0, 1.0), -(i % 40));
      expect_agreeing(a, a*(1.0 + w), precisions[i % 8]);
    }
    interval::precision(saved_precision);
    interval::format(saved_format);
  }

  /*
    operator<< leaves the stream as it found it, the text written apart, and
    std::setw and the adjustment of the stream apply to the whole interval
    (GAOL v5): it set the precision of the stream to interval::precision() for
    good, the doubles written afterwards getting 16 digits, and wrote the '['
    apart, which std::setw padded alone.
  */
  void stream_output()
  {
    const interval_format::format_t saved_format = interval::format();
    const std::streamsize saved_precision = interval::precision();
    interval::format(interval_format::bounds);
    interval::precision(16);
    {
      std::ostringstream os;
      os.precision(3);
      os << interval(1.0, 2.0);
      check("operator<< leaves the precision of the stream", os.precision() == 3,
            [&] { return "the precision is " + std::to_string(os.precision()); });
    }
    const auto expect_text = [](const std::string& name, const std::string& got, const std::string& expected) {
      check(name, got == expected, [&] { return "\"" + got + "\" rather than \"" + expected + "\""; });
    };
    {
      std::ostringstream os;
      os << '|' << std::setw(8) << interval(1.0, 2.0) << '|' << interval(3.0, 4.0) << '|';
      expect_text("operator<< pads the whole interval to std::setw, once", os.str(), "|  [1, 2]|[3, 4]|");
    }
    {
      std::ostringstream os;
      os << '|' << std::left << std::setfill('.') << std::setw(8) << interval(1.0, 2.0) << '|';
      expect_text("operator<< pads the whole interval to std::setw, adjusted to the left", os.str(), "|[1, 2]..|");
    }
    interval::precision(saved_precision);
    interval::format(saved_format);
  }

  /*
    operator>> reads an interval from a line. At the end of the input it sets
    failbit, leaves the interval as it was and throws nothing, as for a double,
    so that while (in >> x) ends there (GAOL v5): it threw input_format_error
    and emptied the interval, and such a loop always ended with an exception.
    A line that is no interval sets failbit, then throws input_format_error.
  */
  void stream_input()
  {
    {
      std::istringstream in("[1, 2]\n[3, 4]\n");
      interval x;
      int n = 0;
      bool threw = false;
      try {
        while (in >> x) {
          ++n;
        }
      } catch (...) {
        threw = true;
      }
      check("operator>>: while (in >> x) reads each line, and stops at the end of the input",
            !threw && n == 2 && x.left() == 3.0 && x.right() == 4.0,
            [&] { return std::to_string(n) + " read" + (threw ? ", then an exception, " : ", ") + hex(x); });
    }
    {
      std::istringstream in("");
      interval x(1.0, 2.0);
      bool threw = false;
      try {
        in >> x;
      } catch (...) {
        threw = true;
      }
      check("operator>> at the end of the input: failbit, the interval unchanged, nothing thrown",
            !threw && in.fail() && x.left() == 1.0 && x.right() == 2.0,
            [&] { return std::string(threw ? "threw, " : "") + (in.fail() ? "failbit, " : "no failbit, ") + hex(x); });
    }
    {
      std::istringstream in("[1, 2\n");
      interval x(1.0, 2.0);
      bool threw = false;
      try {
        in >> x;
      } catch (const input_format_error&) {
        threw = true;
      }
      check("operator>> of a line that is no interval: failbit, then input_format_error",
            threw && in.fail() && x.is_empty(),
            [&] { return std::string(threw ? "threw, " : "nothing thrown, ") + (in.fail() ? "failbit, " : "no failbit, ") + hex(x); });
    }
    // The same where the reader throws as it reads, rather than refusing the
    // line at its end: the degenerate interval of two numbers that differ
    {
      std::istringstream in("<3, 4>\n");
      interval x(5.0, 6.0);
      bool threw = false;
      try {
        in >> x;
      } catch (const input_format_error&) {
        threw = true;
      }
      check("operator>> of a line the reader refuses as it reads it: failbit, then input_format_error",
            threw && in.fail() && x.is_empty(),
            [&] { return std::string(threw ? "threw, " : "nothing thrown, ") + (in.fail() ? "failbit, " : "no failbit, ") + hex(x); });
    }
    // A stream whose exceptions include failbit gets input_format_error, the
    // error of the line, rather than std::ios_base::failure
    {
      std::istringstream in("[1, 2\n");
      in.exceptions(std::ios_base::failbit);
      interval x(1.0, 2.0);
      bool format_error = false;
      try {
        in >> x;
      } catch (const input_format_error&) {
        format_error = true;
      } catch (...) {
      }
      check("operator>> of a line that is no interval, on a stream throwing on failbit: input_format_error",
            format_error && in.fail(), [&] { return std::string(format_error ? "input_format_error" : "another exception"); });
    }
  }

  /*
    Numbers whose exponent has 7 digits or more and whose significand has a
    million zeros after its point, so that they are doubles (GAOL v5): the
    exponent was kept below 100000, and 1 written so was compared as
    10^-900001, which the reader, bracketing the number from the double
    strtod() gives, turned into [0, 2^-1074]. Exactly 1 each, and [0.5, 1.5]
    in the uncertain form.
  */
  void long_exponents()
  {
    const std::string zeros(1000000, '0');
    const auto expect_one = [](const std::string& name, const std::string& s) {
      const interval x = textToInterval(s);
      check(name, x.left() == 1.0 && x.right() == 1.0, [&] { return hex(x); });
    };
    expect_one("textToInterval(0.<a million zeros>1e1000001) is 1", "0." + zeros + "1e1000001");
    expect_one("textToInterval(0x0.<300000 zeros>1p1200004) is 1", "0x0." + zeros.substr(0, 300000) + "1p1200004");
    const interval u = textToInterval("0." + zeros + "1?e1000001");
    check("textToInterval(0.<a million zeros>1?e1000001) is [0.5, 1.5]", u.left() == 0.5 && u.right() == 1.5,
          [&] { return hex(u); });
  }

  // A number of some thousands of characters, and the tightest interval of
  // doubles enclosing it, which is known without reading it: 1.5 followed by
  // zeros is 1.5, and followed by zeros and a 1, is between 1.5 and the next
  // double
  struct Long_number
  {
    std::string text;
    double left, right;
  };

  std::vector<Long_number> long_numbers(std::size_t n)
  {
    const std::string zeros(n, '0'), nines(n, '9'), digits = std::to_string(n);
    const double dmax = std::numeric_limits<double>::max(), dmin = std::numeric_limits<double>::denorm_min();
    return {
      { "1.5" + zeros, 1.5, 1.5 },
      { "1.5" + zeros + "1", 1.5, next_double(1.5) },
      { "1." + nines, previous_double(2.0), 2.0 },
      { "1" + zeros + "e-" + digits, 1.0, 1.0 },
      { "15" + zeros + "e-" + std::to_string(n + 1), 1.5, 1.5 },
      { "0." + zeros + "1e" + digits, previous_double(0.1), 0.1 }, // 0.1 is above 1/10
      { "0x1." + zeros + "8p0", 1.0, next_double(1.0) },
      { "0x1.8" + zeros + "p1", 3.0, 3.0 },
      { "1" + zeros, dmax, inf },
      { "0." + zeros + "1", 0.0, dmin },
      // The uncertain form: half a unit of the last place, then one unit
      { "1.5" + zeros + "?", previous_double(1.5), next_double(1.5) },
      { "1.5" + zeros + "?1", previous_double(1.5), next_double(1.5) },
    };
  }

  // The long number that took the longest to read under a locale writing a
  // decimal comma (20000 characters)
  Long_number slowest_long_number()
  {
    return { "1.5" + std::string(20000, '0') + "1", 1.5, next_double(1.5) };
  }

  void check_long_number(const std::string& where, const Long_number& l)
  {
    const interval x = textToInterval(l.text);
    check("textToInterval(long number) " + where + ": the tightest enclosure", x.left() == l.left && x.right() == l.right,
          [&] { return "\"" + l.text.substr(0, 12) + "...\" of " + std::to_string(l.text.size()) + " characters: " + hex(x); });
  }

  void check_long_numbers(const std::string& where)
  {
    for (const Long_number& l : long_numbers(5000)) {
      check_long_number(where, l);
    }
    check_long_number(where, slowest_long_number());
  }

  // The least time, in seconds, that three readings of the text s take
  double reading_time(const std::string& s)
  {
    double least = inf;
    for (int i = 0; i < 3; ++i) {
      const auto start = std::chrono::steady_clock::now();
      static_cast<void>(textToInterval(s));
      const std::chrono::duration<double> elapsed = std::chrono::steady_clock::now() - start;
      if (elapsed.count() < least) {
        least = elapsed.count();
      }
    }
    return least;
  }

  /*
    Long numbers, under the C locale: the reading takes the digits apart once,
    for the comparisons with the doubles around the number (GAOL v5).
  */
  void long_numbers_c_locale()
  {
    check_long_numbers("under the C locale");
  }

  /*
    Numbers read, and exact texts written, under a locale writing a decimal
    comma, which a program sets with setlocale(LC_ALL, "") (GAOL v5): strtod()
    stops there at the '.' of "0.1" and gives 0, from which the reader moved
    one double at a time, about 4.6e18 of them, and never returned; and
    exact_string() wrote the '.' of printf("%a") as a comma,
    "[0x1,8p+0, 0x1,4p+1]", which cannot be read back. The reader then
    compared the number with about 125 doubles, and read its text again for
    each of them, in a time quadratic in the length of the text: seconds for
    20000 characters, where the C locale, which needs two comparisons, took a
    twentieth of a second. The long numbers have to be read as under the C
    locale, and in about the same time: the reading under the C locale is
    timed on the same machine, and the one under the comma locale has to take
    at most 10 times as long, and 50 ms more, a margin that a slow or a busy
    machine does not exhaust (the fault made it 50 times as long). Nothing is
    checked where no such locale is installed.
  */
  void comma_locale()
  {
    const char *const current = std::setlocale(LC_NUMERIC, nullptr);
    const std::string saved = (current != nullptr) ? current : "C";
    const char *const names[] = { "fr_FR.UTF-8", "fr_FR.utf8", "de_DE.UTF-8", "de_DE.utf8", "French_France.1252", "fr_FR" };
    const char *comma = nullptr;
    for (const char *name : names) {
      if (std::setlocale(LC_NUMERIC, name) != nullptr && std::localeconv()->decimal_point[0] == ',') {
        comma = name;
        break;
      }
    }
    if (comma == nullptr) {
      std::setlocale(LC_NUMERIC, saved.c_str());
      std::printf("No locale writing a decimal comma: the reading and the writing under such a locale are not checked\n");
      return;
    }
    const std::string text = exact_string(interval(1.5, 2.5));
    check("exact_string() under a locale writing a decimal comma", text == "[0x1.8p+0, 0x1.4p+1]",
          [&] { return std::string(comma) + ": " + text; });
    for (const char *s : { "0.1", "1.5", "2.5e-3", "123.456", "0.1000000000000000055511151231257827021181583404541015625" }) {
      expect_number("textToInterval(number) under a locale writing a decimal comma", s);
    }
    const interval three = textToInterval("0x1.8p1");
    check("textToInterval(\"0x1.8p1\") under a locale writing a decimal comma", three.left() == 3.0 && three.right() == 3.0,
          [&] { return hex(three); });
    for (double x : { 0.1, std::numeric_limits<double>::denorm_min(), -std::numeric_limits<double>::max() }) {
      const interval y = textToInterval(exact_string(interval(x)));
      check("exact_string() read back under a locale writing a decimal comma", y.left() == x && y.right() == x,
            [&] { return exact_string(interval(x)) + " read " + hex(y); });
    }
    check_long_numbers("under a locale writing a decimal comma");
    const std::string slowest = slowest_long_number().text;
    std::setlocale(LC_NUMERIC, "C");
    const double in_c = reading_time(slowest);
    std::setlocale(LC_NUMERIC, comma);
    const double in_comma = reading_time(slowest);
    check("textToInterval(long number) under a locale writing a decimal comma: the time of the C locale", in_comma <= 10*in_c + 0.05,
          [&] { return std::string(comma) + ": " + std::to_string(in_comma) + " s, against " + std::to_string(in_c) + " s under the C locale"; });
    std::setlocale(LC_NUMERIC, saved.c_str());
  }
}

int main()
{
  gaol::init();
  numbers();
  subnormal_numbers();
  constants();
  constructors();
  ieee_literals();
  expressions();
  hexadecimal_output();
  decimal_output();
  stream_output();
  stream_input();
  long_exponents();
  long_numbers_c_locale();
  comma_locale();
  const int status = summary();
  gaol::cleanup();
  return status;
}
