/*-*-C++-*------------------------------------------------------------------
 * gaol -- NOT Just Another Interval Library
 *--------------------------------------------------------------------------
 * Tests of GAOL v5: the bounds of the elementary functions against
 * CORE-MATH itself.
 *
 * GAOL bounds every elementary function with CORE-MATH (gaol/gaol_core_math.h),
 * which is correctly rounded in the rounding direction in effect. The tightest
 * bounds of f at a double x are therefore the values CORE-MATH gives in the
 * downward and the upward rounding, which this test computes by setting the
 * direction itself, independently of GAOL:
 *
 *   tightest(f, x) = [ RD(f(x)), RU(f(x)) ]
 *
 * Computing in the upward direction it keeps, GAOL takes RU(f(x)) as the upper
 * bound, and the double below it as the lower one, which is RD(f(x)) unless
 * f(x) is a double: so GAOL's bounds have to enclose the tightest ones, and to
 * be within one double of them below and equal above. Where the operations of
 * gaol/gaol_interval.cpp give the exact value themselves (log(1) = 0,
 * sin(0) = 0, the bounds of pi/2 at asin(1)...), they are the tightest ones.
 *
 * Each function is tried on the values at the ends of its domain and next to
 * them, on the values GAOL treats apart, on the powers of two and their
 * neighbours, on the subnormals, and on random doubles of every magnitude.
 *
 * Copyright (c) 2026 ENSTA, France
 *
 * Created 2026-09-20 by Jordan NININ
 *--------------------------------------------------------------------------
 * gaol is a software distributed WITHOUT ANY WARRANTY. Read the associated
 * COPYING file for information.
 *--------------------------------------------------------------------------*/

#include "gaol_tests.h"
#include "gaol/gaol_core_math.h"

#include <functional>
#include <random>

using namespace gaol;
using namespace gaol_tests;

namespace
{
  // The number of doubles from a to b, saturated
  int doubles_between(double a, double b)
  {
    if (a == b) {
      return 0;
    }
    int n = 0;
    double x = a;
    while (x < b && n < 1000) {
      x = next_float(x);
      ++n;
    }
    return n;
  }

  std::string show(double x)
  {
    char b[64];
    std::snprintf(b, sizeof b, "%a", x);
    return b;
  }

  // The values every function is tried on
  std::vector<double> arguments()
  {
    std::vector<double> v;
    const double specials[] = {
      0.0, -0.0, 1.0, -1.0, 2.0, -2.0, 0.5, -0.5, 3.0, 10.0, 100.0,
      0x1p-1074, -0x1p-1074, 0x1p-1022, 0x1.fffffffffffffp-1023, // subnormals
      0x1.fffffffffffffp+1023, -0x1.fffffffffffffp+1023,         // MAX
      0x1.921fb54442d18p+0, 0x1.921fb54442d19p+0,                // around pi/2
      0x1.5bf0a8b145769p+1, 0x1.2cd9fc44eb982p+0,                // e, ...
      0x1p+52, 0x1p+53, 0x1p-52, 0x1p-53, 4503599627370495.0,    // 2^52 - 1
      20.0, -20.0, 711.0, -711.0, 710.0, 0x1.62e42fefa39efp+9,
    };
    for (double d : specials) {
      v.push_back(d);
    }
    // the powers of two and their neighbours, over the whole range
    for (int k = -1070; k <= 1020; k += 7) {
      const double p = std::ldexp(1.0, k);
      v.push_back(p);
      v.push_back(next_float(p));
      v.push_back(previous_float(p));
      v.push_back(-p);
    }
    // random doubles of every magnitude
    std::mt19937_64 gen(20260920u);
    for (int i = 0; i < 4000; ++i) {
      const uint64_t u = gen();
      double x;
      std::memcpy(&x, &u, sizeof x);
      if (std::isfinite(x)) {
        v.push_back(x);
      }
    }
    // random doubles of moderate magnitude, where the functions are usually used
    for (int i = 0; i < 4000; ++i) {
      const double x = (double)(int64_t)(gen() % 2000001 - 1000000) / 1000.0;
      v.push_back(x);
    }
    return v;
  }

  // The tightest bounds of f at x: CORE-MATH in the two directed roundings,
  // computed apart from GAOL
  void tightest(double (*f)(double), double x, double& lo, double& hi)
  {
    std::fesetround(FE_DOWNWARD);
    lo = f(x);
    std::fesetround(FE_UPWARD);
    hi = f(x);
  }

  /* GAOL's bounds of an interval [x,x] against the tightest ones. A bound that
     is not finite, and an empty result, are left to the tests of
     tests/elementary.cpp: here only the values inside the domain are compared. */
  void compare(const std::string& name, double (*cr)(double),
               const std::function<interval(const interval&)>& gaol_f,
               const std::vector<double>& values)
  {
    for (double x : values) {
      double lo, hi;
      tightest(cr, x, lo, hi);
      std::fesetround(FE_UPWARD);
      if (!std::isfinite(lo) || !std::isfinite(hi)) {
        continue; // outside the domain, or an infinite value
      }
      const interval got = evaluate(name, [&] { return gaol_f(interval(x, x)); },
                                    [&] { return name + "(" + show(x) + ")"; });
      if (got.is_empty()) {
        check(name + ": not empty inside the domain", false,
              [&] { return name + "(" + show(x) + ")"; });
        continue;
      }
      check(name + ": encloses the tightest bounds",
            got.left() <= lo && got.right() >= hi,
            [&] {
              return name + "(" + show(x) + ") = [" + show(got.left()) + ", " + show(got.right())
                   + "] rather than [" + show(lo) + ", " + show(hi) + "]";
            });
      /* The upper bound is the value rounded upward, and the lower one the
         double below the value rounded upward: at most one double below the
         tightest lower bound, and never below more. */
      check_distance(name, doubles_between(got.left(), lo), 1,
                     [&] {
                       return name + "(" + show(x) + ") = [" + show(got.left()) + ", ...] rather than ["
                            + show(lo) + ", ...]";
                     });
      check_distance(name, doubles_between(hi, got.right()), 0,
                     [&] {
                       return name + "(" + show(x) + ") = [..., " + show(got.right()) + "] rather than [..., "
                            + show(hi) + "]";
                     });
    }
  }

  void elementary_functions()
  {
    const std::vector<double> v = arguments();

    compare("exp", gaol_cr_exp, [](const interval& x) { return exp(x); }, v);
    compare("log", gaol_cr_log, [](const interval& x) { return log(x); }, v);
    compare("sin", gaol_cr_sin, [](const interval& x) { return sin(x); }, v);
    compare("cos", gaol_cr_cos, [](const interval& x) { return cos(x); }, v);
    compare("tan", gaol_cr_tan, [](const interval& x) { return tan(x); }, v);
    compare("asin", gaol_cr_asin, [](const interval& x) { return asin(x); }, v);
    compare("acos", gaol_cr_acos, [](const interval& x) { return acos(x); }, v);
    compare("atan", gaol_cr_atan, [](const interval& x) { return atan(x); }, v);
    compare("sinh", gaol_cr_sinh, [](const interval& x) { return sinh(x); }, v);
    compare("cosh", gaol_cr_cosh, [](const interval& x) { return cosh(x); }, v);
    compare("tanh", gaol_cr_tanh, [](const interval& x) { return tanh(x); }, v);
    compare("asinh", gaol_cr_asinh, [](const interval& x) { return asinh(x); }, v);
    compare("acosh", gaol_cr_acosh, [](const interval& x) { return acosh(x); }, v);
    compare("atanh", gaol_cr_atanh, [](const interval& x) { return atanh(x); }, v);
    // The exponentials and the logarithms in base 2 and 10, which IEEE
    // 1788-2015 requires (Table 9.1, GAOL v5)
    compare("exp2", gaol_cr_exp2, [](const interval& x) { return exp2(x); }, v);
    compare("exp10", gaol_cr_exp10, [](const interval& x) { return exp10(x); }, v);
    compare("log2", gaol_cr_log2, [](const interval& x) { return log2(x); }, v);
    compare("log10", gaol_cr_log10, [](const interval& x) { return log10(x); }, v);
  }

  /* CORE-MATH gives the same value whatever the rounding direction it is called
     in, when that value is exact: the directed roundings of an exact result are
     that result. This checks the values GAOL relies on being exact. */
  void exact_values()
  {
    struct Case { const char* name; double (*f)(double); double x; double value; };
    const Case cases[] = {
      {"exp(0)", gaol_cr_exp, 0.0, 1.0},
      {"log(1)", gaol_cr_log, 1.0, 0.0},
      {"sin(0)", gaol_cr_sin, 0.0, 0.0},
      {"cos(0)", gaol_cr_cos, 0.0, 1.0},
      {"tan(0)", gaol_cr_tan, 0.0, 0.0},
      {"asin(0)", gaol_cr_asin, 0.0, 0.0},
      {"acos(1)", gaol_cr_acos, 1.0, 0.0},
      {"atan(0)", gaol_cr_atan, 0.0, 0.0},
      {"sinh(0)", gaol_cr_sinh, 0.0, 0.0},
      {"cosh(0)", gaol_cr_cosh, 0.0, 1.0},
      {"tanh(0)", gaol_cr_tanh, 0.0, 0.0},
      {"asinh(0)", gaol_cr_asinh, 0.0, 0.0},
      {"acosh(1)", gaol_cr_acosh, 1.0, 0.0},
      {"atanh(0)", gaol_cr_atanh, 0.0, 0.0},
      {"exp2(0)", gaol_cr_exp2, 0.0, 1.0},
      {"exp2(3)", gaol_cr_exp2, 3.0, 8.0},
      {"exp2(-1)", gaol_cr_exp2, -1.0, 0.5},
      {"exp2(1023)", gaol_cr_exp2, 1023.0, 0x1p1023},
      {"exp10(0)", gaol_cr_exp10, 0.0, 1.0},
      {"exp10(2)", gaol_cr_exp10, 2.0, 100.0},
      {"exp10(22)", gaol_cr_exp10, 22.0, 1e22},
      {"log2(1)", gaol_cr_log2, 1.0, 0.0},
      {"log2(8)", gaol_cr_log2, 8.0, 3.0},
      {"log2(0.25)", gaol_cr_log2, 0.25, -2.0},
      {"log10(1)", gaol_cr_log10, 1.0, 0.0},
      {"log10(100)", gaol_cr_log10, 100.0, 2.0},
      {"log10(1e22)", gaol_cr_log10, 1e22, 22.0},
    };
    for (const Case& c : cases) {
      double lo, hi;
      tightest(c.f, c.x, lo, hi);
      std::fesetround(FE_UPWARD);
      check(std::string("CORE-MATH is exact at ") + c.name,
            lo == c.value && hi == c.value,
            [&] { return std::string(c.name) + " = [" + show(lo) + ", " + show(hi) + "]"; });
    }
  }

  /* pow and atan2 take two arguments: the same comparison, on pairs. */
  void two_arguments()
  {
    std::mt19937_64 gen(20260921u);
    const double bases[] = {0.5, 1.5, 2.0, 3.0, 10.0, 0.125, 7.25, 1e10, 1e-10};
    const double exponents[] = {0.0, 1.0, 2.0, 3.0, -1.0, -2.0, 0.5, -0.5, 1.0 / 3.0, 12.0, -7.5};
    for (double b : bases) {
      for (double e : exponents) {
        std::fesetround(FE_DOWNWARD);
        const double lo = gaol_cr_pow(b, e);
        std::fesetround(FE_UPWARD);
        const double hi = gaol_cr_pow(b, e);
        if (!std::isfinite(lo) || !std::isfinite(hi)) {
          continue;
        }
        const interval got = evaluate("pow", [&] { return pow(interval(b, b), interval(e, e)); },
                                      [&] { return "pow(" + show(b) + ", " + show(e) + ")"; });
        check("pow: encloses the tightest bounds",
              !got.is_empty() && got.left() <= lo && got.right() >= hi,
              [&] {
                return "pow(" + show(b) + ", " + show(e) + ") = [" + show(got.left()) + ", "
                     + show(got.right()) + "] rather than [" + show(lo) + ", " + show(hi) + "]";
              });
      }
    }
    for (int i = 0; i < 2000; ++i) {
      const double y = (double)(int64_t)(gen() % 200001 - 100000) / 100.0;
      const double x = (double)(int64_t)(gen() % 200001 - 100000) / 100.0;
      if (x == 0.0 && y == 0.0) {
        continue;
      }
      std::fesetround(FE_DOWNWARD);
      const double lo = gaol_cr_atan2(y, x);
      std::fesetround(FE_UPWARD);
      const double hi = gaol_cr_atan2(y, x);
      const interval got = evaluate("atan2", [&] { return atan2(interval(y, y), interval(x, x)); },
                                    [&] { return "atan2(" + show(y) + ", " + show(x) + ")"; });
      check("atan2: encloses the tightest bounds",
            !got.is_empty() && got.left() <= lo && got.right() >= hi,
            [&] {
              return "atan2(" + show(y) + ", " + show(x) + ") = [" + show(got.left()) + ", "
                   + show(got.right()) + "] rather than [" + show(lo) + ", " + show(hi) + "]";
            });
    }
  }

  /* The bounds GAOL gives of the exponentials and the logarithms in base 2 and
     10 where the value is a double: they are that double, not the one below it
     (GAOL v5). Where the value is not a double, the bounds are checked
     against the tightest ones by compare() above. */
  void base_two_and_ten_exact()
  {
    struct Case { const char* name; std::function<interval(double)> f; double x; double value; };
    const Case cases[] = {
      {"exp2", [](double x) { return exp2(interval(x, x)); }, 0.0, 1.0},
      {"exp2", [](double x) { return exp2(interval(x, x)); }, 3.0, 8.0},
      {"exp2", [](double x) { return exp2(interval(x, x)); }, -1.0, 0.5},
      {"exp2", [](double x) { return exp2(interval(x, x)); }, -1074.0, 0x1p-1074},
      {"exp2", [](double x) { return exp2(interval(x, x)); }, 1023.0, 0x1p1023},
      {"exp10", [](double x) { return exp10(interval(x, x)); }, 0.0, 1.0},
      {"exp10", [](double x) { return exp10(interval(x, x)); }, 2.0, 100.0},
      {"exp10", [](double x) { return exp10(interval(x, x)); }, 22.0, 1e22},
      {"log2", [](double x) { return log2(interval(x, x)); }, 1.0, 0.0},
      {"log2", [](double x) { return log2(interval(x, x)); }, 8.0, 3.0},
      {"log2", [](double x) { return log2(interval(x, x)); }, 0.25, -2.0},
      {"log2", [](double x) { return log2(interval(x, x)); }, 0x1p-1074, -1074.0},
      {"log10", [](double x) { return log10(interval(x, x)); }, 1.0, 0.0},
      {"log10", [](double x) { return log10(interval(x, x)); }, 100.0, 2.0},
      {"log10", [](double x) { return log10(interval(x, x)); }, 1e22, 22.0},
    };
    for (const Case& c : cases) {
      const std::string name = std::string(c.name) + ": the value that is a double is a bound";
      const interval got = c.f(c.x);
      check(name, !got.is_empty() && got.left() == c.value && got.right() == c.value,
            [&] {
              return std::string(c.name) + "(" + show(c.x) + ") = ["
                   + (got.is_empty() ? std::string("empty")
                                     : show(got.left()) + ", " + show(got.right()))
                   + "] rather than [" + show(c.value) + "]";
            });
    }
    // The domains of Table 9.1: the logarithms are defined on (0, +oo) only
    check("log2: empty where no number is positive", log2(interval(-4.0, 0.0)).is_empty(),
          [] { return std::string("log2([-4, 0])"); });
    check("log10: empty where no number is positive", log10(interval(-4.0, -1.0)).is_empty(),
          [] { return std::string("log10([-4, -1])"); });
    check("exp2: within [0, +oo]", exp2(interval(-GAOL_INFINITY, 0.0)).left() == 0.0,
          [] { return std::string("exp2([-oo, 0])"); });
    check("exp10: within [0, +oo]", exp10(interval(-GAOL_INFINITY, 0.0)).left() == 0.0,
          [] { return std::string("exp10([-oo, 0])"); });
  }

  /* rootn(x, q) with a negative q, which IEEE 1788-2015 recommends
     (Table 10.5): x^(1/q) = 1/x^(1/|q|), defined on R\{0} for an odd q and on
     (0, +oo) for an even one (GAOL v5). */
  void negative_roots()
  {
    struct Case { const char* what; interval got; interval expected; bool empty; };
    const Case cases[] = {
      {"nth_root([8], -3)", nth_root(interval(8.0, 8.0), -3), interval(0.5, 0.5), false},
      {"nth_root([-8], -3)", nth_root(interval(-8.0, -8.0), -3), interval(-0.5, -0.5), false},
      {"nth_root([16], -4)", nth_root(interval(16.0, 16.0), -4), interval(0.5, 0.5), false},
      {"nth_root([1, 8], -3)", nth_root(interval(1.0, 8.0), -3), interval(0.5, 1.0), false},
      // 0 is outside the domain, and an even root needs a positive number
      {"nth_root([0], -3)", nth_root(interval(0.0, 0.0), -3), interval::emptyset(), true},
      {"nth_root([-4, -1], -4)", nth_root(interval(-4.0, -1.0), -4), interval::emptyset(), true},
      {"nth_root([8], 0)", nth_root(interval(8.0, 8.0), 0), interval::emptyset(), true},
      // an interval holding 0 with an odd q: the hull of the two half-lines
      {"nth_root([-1, 1], -3)", nth_root(interval(-1.0, 1.0), -3), interval::universe(), false},
    };
    for (const Case& c : cases) {
      if (c.empty) {
        check(std::string(c.what) + ": empty", c.got.is_empty(),
              [&] { return std::string(c.what); });
      } else {
        check(std::string(c.what) + ": the expected interval",
              !c.got.is_empty() && c.got.left() == c.expected.left()
                && c.got.right() == c.expected.right(),
              [&] {
                return std::string(c.what) + " = ["
                     + (c.got.is_empty() ? std::string("empty")
                                         : show(c.got.left()) + ", " + show(c.got.right()))
                     + "] rather than [" + show(c.expected.left()) + ", "
                     + show(c.expected.right()) + "]";
              });
      }
    }
    /* A positive q gives what nth_root(x, unsigned) gives, and a negative one
       its inverse: checked over the doubles of a range. */
    for (int k = 1; k <= 2000; ++k) {
      const double x = (double)k / 8.0;
      for (int q : {2, 3, 5, 7}) {
        const interval up = nth_root(interval(x, x), q);
        const interval dn = nth_root(interval(x, x), -q);
        const interval inv = inverse(up);
        check("nth_root: a negative exponent inverts the positive one",
              dn.left() == inv.left() && dn.right() == inv.right(),
              [&] {
                return "nth_root(" + show(x) + ", " + std::to_string(-q) + ")";
              });
        check("nth_root: an int exponent agrees with the unsigned one",
              up.left() == nth_root(interval(x, x), (unsigned int)q).left()
                && up.right() == nth_root(interval(x, x), (unsigned int)q).right(),
              [&] {
                return "nth_root(" + show(x) + ", " + std::to_string(q) + ")";
              });
      }
    }
  }
}

int main()
{
  gaol::init();
  elementary_functions();
  exact_values();
  two_arguments();
  base_two_and_ten_exact();
  negative_roots();
  std::fesetround(FE_UPWARD);
  const int status = summary();
  gaol::cleanup();
  return status;
}
