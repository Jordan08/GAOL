/*-*-C++-*------------------------------------------------------------------
 * gaol -- NOT Just Another Interval Library
 *--------------------------------------------------------------------------
 * Examples of GAOL v5: decimal numbers, cancellation, and Rump's example.
 *
 * The first lessons of every course on interval arithmetic (R. E. Moore,
 * R. B. Kearfott and M. J. Cloud, "Introduction to Interval Analysis",
 * SIAM, 2009; W. Tucker, "Validated Numerics", Princeton, 2011), which the
 * demo demos/dintval.m of INTLAB repeats for 0.1 and pi: the program adds
 * 0.1 ten times with doubles and with intervals, encloses sqrt(2) and pi
 * (with interval::pi() and Machin's formula), and evaluates the polynomial
 * of S. M. Rump ("Algorithms for verified inclusions: theory and practice",
 * in R. E. Moore (ed.), "Reliability in Computing", Academic Press, 1988)
 *   f(a, b) = 333.75 b^6 + a^2 (11 a^2 b^2 - b^6 - 121 b^4 - 2) + 5.5 b^8
 *             + a / (2 b)
 * at (77617, 33096), in the form of E. Loh and G. W. Walster ("Rump's
 * example revisited", Reliable Computing 8, 2002). It shows that a decimal
 * constant has to reach GAOL as text, interval("0.1") or the literal 0.1_iv
 * defined below, and not as a double; that sqrt(2.0) is the C library's
 * function of a double; and that intervals do not make a computation exact,
 * but say how far from exact it may be: doubles give Rump's f with a wrong
 * value and no warning, intervals give an enclosure of the true value that
 * is 10^22 wide, the sign that the computation needs more precision (MPFI,
 * Arb). The references were computed apart with mpmath (50 digits), and
 * Rump's true value, -54767/66192, with the rational numbers of Python.
 *
 * Copyright (c) 2026 ENSTA, France
 *
 * Created 2026-09-27 by Jordan NININ
 *--------------------------------------------------------------------------
 * gaol is a software distributed WITHOUT ANY WARRANTY. Read the associated
 * COPYING file for information.
 *--------------------------------------------------------------------------*/

#include <cfenv>
#include <cmath>
#include <cstdio>
#include <cstdlib>
#include <iostream>
#include <string>

#include <gaol/gaol.h>

// M_PI is POSIX, not ISO C++: Visual C++ defines it only after
// _USE_MATH_DEFINES
#ifndef M_PI
#  define M_PI 3.14159265358979323846
#endif

using namespace gaol;

namespace {

  /*
    A decimal literal for intervals, which GAOL does not provide: a raw
    literal operator receives the characters of the literal as the source
    writes them, "0.1" for 0.1_iv, before any conversion to double, and
    interval(const char*) encloses the number they write. 0.1_iv reads like a
    number and holds 1/10.
  */
  interval operator""_iv(const char* digits)
  {
    return interval(digits);
  }

  bool all_checks_passed = true;

  // Every claim of the output is checked here: a claim that does not hold is
  // reported, and makes the program fail, which is how ctest runs it
  void check(bool holds, const char* claim)
  {
    if (!holds) {
      std::cout << "FAILED: " << claim << "\n";
      all_checks_passed = false;
    }
  }

  // What a result shows, then the result on a line of its own
  void show(const char* what, const interval& x)
  {
    std::cout << "  " << what << "\n      " << x << "\n";
  }

  /*
    A double with two significant digits, for an order of magnitude. GAOL
    rounds the doubles of the whole program upward, and with glibc their
    conversion to text as well: 20.000000000000004 would be written 21. The
    conversion is made rounding to nearest, which GAOL allows: each of its
    operations sets the direction upward again when it is not.
  */
  std::string approx(double d)
  {
    const int direction = std::fegetround();
    std::fesetround(FE_TONEAREST);
    char text[32];
    std::snprintf(text, sizeof text, "%.2g", d);
    std::fesetround(direction);
    return text;
  }

  /*
    Rump's polynomial, written once for doubles and for intervals. Only + - *
    / appear: with using namespace gaol, sqr(b) or pow(b, 6) on a double would
    be GAOL's functions and return an interval. For the points a and b, a * a
    is as tight as sqr(a): the dependency problem needs a variable that varies.
    333.75 = 1335/4 and 5.5 = 11/2 are doubles exactly, so that the double
    literals are the numbers of the formula, which 0.1 would not be.
  */
  template <class T>
  T rump(const T& a, const T& b)
  {
    const T a2 = a * a, b2 = b * b, b4 = b2 * b2, b6 = b4 * b2, b8 = b4 * b4;
    return 333.75 * b6 + a2 * (11.0 * a2 * b2 - b6 - 121.0 * b4 - 2.0) + 5.5 * b8 + a / (2.0 * b);
  }

} // namespace

int main()
{
  // GAOL writes 16 significant digits by default; 20 show where the doubles
  // lie next to the decimal numbers. The setting is GAOL's, for every interval
  // written: std::setprecision does not apply to intervals.
  interval::precision(20);

  // ------------------------------------------------------------------------
  std::cout << "Decimal numbers are not doubles\n";

  // The compiler turns 0.1 into the double nearest 1/10 before GAOL sees it:
  // interval(0.1) holds that one double, 0.1000000000000000055..., and cannot
  // hold 1/10, which is no double (its denominator is not a power of 2). GAOL
  // writes a point as <a, b>: one double, between the decimal numbers a and b.
  const interval double_tenth(0.1);
  show("interval(0.1): a point, the double nearest 1/10, which is not 1/10", double_tenth);
  check(double_tenth.left() == double_tenth.right(), "interval(0.1) is a point");

  // As text, the number reaches the parser of GAOL, which rounds it down for
  // the lower bound and up for the upper bound
  const interval tenth("0.1");
  show("interval(\"0.1\"): the two doubles around 1/10, which contain it", tenth);
  // 1 / 10 computed by GAOL's division, a way of its own to enclose 1/10
  check(tenth.set_contains(interval(1.0) / 10.0), "interval(\"0.1\") contains 1/10");
  check(tenth.left() < tenth.right(), "interval(\"0.1\") is not a point");
  show("0.1_iv: the same interval, written as a number", 0.1_iv);
  // The parentheses are needed: 0.1_iv.set_eq would be read as one number.
  // set_eq() is the equality of sets; GAOL has no operator==.
  check((0.1_iv).set_eq(tenth), "0.1_iv is interval(\"0.1\")");

  // ------------------------------------------------------------------------
  std::cout << "\nAdding 0.1 ten times (with doubles: at the end)\n";

  // interval s(0.0), not s(0): the literal 0 is also the null pointer, and
  // interval(0) hesitates between interval(double) and interval(const char*)
  interval s_double(0.0), s_tenth(0.0);
  for (int i = 0; i < 10; ++i) {
    s_double += 0.1;  // adds the double 0.1, exactly as interval(0.1) would
    s_tenth += 0.1_iv;
  }

  // The ten doubles 0.1 add up to 1.000000000000000055..., not to 1. The
  // interval encloses that sum, and contains 1 only because rounding its lower
  // bound down went below 1: an enclosure, but of another problem.
  const interval ten_doubles("1.000000000000000055511151231257827021181583404541015625");
  show("with s += 0.1 on an interval: encloses 10 x 0.1000000000000000055, and 1 by chance", s_double);
  check(s_double.set_contains(ten_doubles), "the sum of interval(0.1) encloses 10 times the double 0.1");
  check(s_double.set_contains(1.0), "the sum of interval(0.1) contains 1");
  show("with s += 0.1_iv: contains 1, as it must", s_tenth);
  check(s_tenth.set_contains(1.0), "the sum of 0.1_iv contains 1");

  // Squaring shows the difference: the square of the double 0.1 lies above
  // 1/100, and so does the whole interval
  const interval hundredth("0.01");
  const interval double_square = sqr(double_tenth);
  show("sqr(interval(0.1)): lies above 1/100", double_square);
  // x >= y is a certainly-relation: every element of x is at least every
  // element of y. The upper bound of hundredth is at least 1/100, and not
  // equal to it, 1/100 being no double: double_square misses 1/100. The empty
  // set, which has no element, satisfies every certainly-relation: hence
  // is_empty() first.
  check(!double_square.is_empty() && double_square >= hundredth, "sqr(interval(0.1)) misses 1/100");
  show("sqr(0.1_iv): contains 1/100", sqr(0.1_iv));
  check(sqr(0.1_iv).set_contains(hundredth), "sqr(0.1_iv) contains 1/100");

  // ------------------------------------------------------------------------
  std::cout << "\nOn a number, sqrt is the C library's\n";

  // sqrt(2.0) calls sqrt(double) of <cmath>, which fits a double better than
  // GAOL's sqrt(const interval&): it returns sqrt(2) rounded to a double,
  // which converts silently to an interval, a point. sqrt(2) is irrational,
  // and no double.
  const interval sqrt2("1.4142135623730950488016887242096980785696718753769");
  const interval root_point = sqrt(2.0);
  show("interval r = sqrt(2.0): a point, the rounded double, which misses sqrt(2)", root_point);
  check(root_point.left() == root_point.right(), "interval r = sqrt(2.0) is a point");
  show("sqrt(interval(2.0)): contains sqrt(2)", sqrt(interval(2.0)));
  check(sqrt(interval(2.0)).set_contains(sqrt2), "sqrt(interval(2.0)) contains sqrt(2)");
  // The parser computes the expressions it reads with intervals
  show("interval(\"sqrt(2)\"): contains sqrt(2)", interval("sqrt(2)"));
  check(interval("sqrt(2)").set_contains(sqrt2), "interval(\"sqrt(2)\") contains sqrt(2)");

  // ------------------------------------------------------------------------
  std::cout << "\nPi\n";

  const interval pi("3.1415926535897932384626433832795028841971693993751");
  show("interval::pi(): contains pi", interval::pi());
  check(interval::pi().set_contains(pi), "interval::pi() contains pi");
  // Machin's formula (1706). 1/5 and 1/239 are no doubles either:
  // interval(1.0) / 5.0 encloses 1/5, where atan(0.2) would take the
  // arctangent of a double near it
  const interval machin = 16.0 * atan(interval(1.0) / 5.0) - 4.0 * atan(interval(1.0) / 239.0);
  show("16 atan(1/5) - 4 atan(1/239) (Machin): contains pi", machin);
  check(machin.set_contains(pi), "Machin's formula contains pi");
  show("sin(interval::pi()): contains 0", sin(interval::pi()));
  check(sin(interval::pi()).set_contains(0.0), "sin(interval::pi()) contains 0");
  // M_PI is the double nearest pi, below it: its sine is positive, and the
  // enclosure proves it
  const interval sin_m_pi = sin(interval(M_PI));
  show("sin(interval(M_PI)): positive, which proves that M_PI is not pi", sin_m_pi);
  check(!sin_m_pi.is_empty() && 0.0 < sin_m_pi, "sin(interval(M_PI)) is positive");

  // ------------------------------------------------------------------------
  std::cout << "\nRump's f(77617, 33096), whose true value is -0.827396059946821368...\n";

  // The terms of f reach 8e36, and cancel down to -0.83: their rounding
  // errors, up to 2^70 = 1.2e21 each, are all that is left of them
  const interval truth("-0.82739605994682136814116509547981629199903311578438");
  const interval r = rump(interval(77617.0), interval(33096.0));
  show(("with intervals: contains the true value, and 0; its width, " + approx(r.width()) +
        ", is the alarm").c_str(), r);
  check(r.set_contains(truth), "the interval f contains the true value");
  check(r.set_contains(0.0) && r.width() > 1e21, "the interval f contains 0 and is wider than 1e21");
  std::cout << "      the computation needs more precision (MPFI, Arb), not more trust\n";

  // The last use of GAOL: cleanup() sets back the rounding direction the
  // program started with, to nearest, which GAOL had set upward for the whole
  // program (unless GAOL was built with GAOL_PRESERVE_ROUNDING)
  gaol::cleanup();

  // ------------------------------------------------------------------------
  // The same with doubles, computed and printed only now, rounded to nearest
  // as without GAOL. Computing them before cleanup() as well would not do:
  // -frounding-math does not stop GCC from reusing after the call of
  // cleanup() a double computed before it, rounded upward.
  double s = 0.0;
  for (int i = 0; i < 10; ++i) {
    s += 0.1;
  }
  const double r_double = rump(77617.0, 33096.0);
  std::printf("\nThe same with doubles, after gaol::cleanup() set the rounding to nearest again\n");
  std::printf("  0.1 added ten times: %.17g, not 1\n", s);
  std::printf("  Rump's f: %.17g, wrong, without any warning\n", r_double);
  check(s != 1.0, "the sum of doubles is not 1");
  check(std::fabs(r_double - (-0.8273960599468214)) > 1.0, "the double f is wrong");

  return all_checks_passed ? 0 : EXIT_FAILURE;
}
