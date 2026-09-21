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
    // The forward functions IEEE 1788-2015 recommends (Table 10.5, GAOL v5)
    compare("expm1", gaol_cr_expm1, [](const interval& x) { return expm1(x); }, v);
    compare("exp2m1", gaol_cr_exp2m1, [](const interval& x) { return exp2m1(x); }, v);
    compare("exp10m1", gaol_cr_exp10m1, [](const interval& x) { return exp10m1(x); }, v);
    compare("sinpi", gaol_cr_sinpi, [](const interval& x) { return sinpi(x); }, v);
    compare("cospi", gaol_cr_cospi, [](const interval& x) { return cospi(x); }, v);
    compare("tanpi", gaol_cr_tanpi, [](const interval& x) { return tanpi(x); }, v);
    compare("atanpi", gaol_cr_atanpi, [](const interval& x) { return atanpi(x); }, v);
    compare("acospi", gaol_cr_acospi, [](const interval& x) { return acospi(x); }, v);
    compare("log1p", gaol_cr_log1p, [](const interval& x) { return log1p(x); }, v);
    compare("log2p1", gaol_cr_log2p1, [](const interval& x) { return log2p1(x); }, v);
    compare("log10p1", gaol_cr_log10p1, [](const interval& x) { return log10p1(x); }, v);
    compare("rsqrt", gaol_cr_rsqrt, [](const interval& x) { return rsqrt(x); }, v);
    compare("asinpi", gaol_cr_asinpi, [](const interval& x) { return asinpi(x); }, v);
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
      {"log1p(0)", gaol_cr_log1p, 0.0, 0.0},
      {"log2p1(1)", gaol_cr_log2p1, 1.0, 1.0},
      {"log2p1(-1/2)", gaol_cr_log2p1, -0.5, -1.0},
      {"log2p1(2^53 - 1)", gaol_cr_log2p1, 9007199254740991.0, 53.0},
      {"log10p1(9)", gaol_cr_log10p1, 9.0, 1.0},
      {"log10p1(10^15 - 1)", gaol_cr_log10p1, 999999999999999.0, 15.0},
      {"rsqrt(4)", gaol_cr_rsqrt, 4.0, 0.5},
      {"rsqrt(2^-1074)", gaol_cr_rsqrt, 0x1p-1074, 0x1p537},
      {"asinpi(1)", gaol_cr_asinpi, 1.0, 0.5},
      {"asinpi(-1)", gaol_cr_asinpi, -1.0, -0.5},
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

  /*
    The forward functions IEEE 1788-2015 recommends (Table 10.5) over
    intervals (GAOL v5). They are claimed the tightest, so each result has to
    equal the hull of the image computed here apart from GAOL: for the
    monotonic ones the values at the bounds, rounded outward by CORE-MATH in
    the two directed roundings; for sinpi, cospi and tanpi the values at the
    bounds and at every multiple of 1/2 within, enumerated one by one, where
    the extrema and the poles are.
  */
  void recommended_intervals()
  {
    std::uint64_t state = 0x9e3779b97f4a7c15ULL;
    const auto next = [&] {
      state ^= state << 13; state ^= state >> 7; state ^= state << 17;
      return state;
    };
    const auto uniform = [&](double lo, double hi) {
      return lo + (hi - lo) * (static_cast<double>(next() >> 11) * 0x1p-53);
    };
    const auto rd = [](double (*f)(double), double x) {
      std::fesetround(FE_DOWNWARD);
      const double v = f(x);
      std::fesetround(FE_UPWARD);
      return v;
    };
    const auto ru = [](double (*f)(double), double x) {
      std::fesetround(FE_UPWARD);
      return f(x);
    };
    // a pole of tan(pi*x): 2x an odd integer
    const auto is_pole = [](double x) {
      return 2.0 * x == std::floor(2.0 * x) && std::fmod(std::fabs(2.0 * x), 2.0) == 1.0;
    };

    struct Periodic { const char *name; double (*cr)(double);
                      interval (*g)(const interval&); bool tan; };
    const Periodic periodic[] = {
      {"sinpi", gaol_cr_sinpi, sinpi, false},
      {"cospi", gaol_cr_cospi, cospi, false},
      {"tanpi", gaol_cr_tanpi, tanpi, true},
    };
    for (const Periodic& p : periodic) {
      for (int i = 0; i < 20000; ++i) {
        const double a = uniform(-6.0, 6.0);
        // every eighth interval starts on a multiple of 1/2
        const double l = (i % 8 == 0) ? std::floor(2.0 * a) / 2.0 : a;
        const double r = l + uniform(0.0, 2.6);
        std::fesetround(FE_UPWARD);
        const interval got = p.g(interval(l, r));
        double lo = GAOL_INFINITY, hi = -GAOL_INFINITY;
        bool pole_in = false;
        for (double h = std::ceil(2.0 * l) / 2.0; h <= r; h += 0.5) {
          if (p.tan) {
            pole_in = pole_in || (h > l && h < r && is_pole(h));
          } else {
            lo = std::min(lo, rd(p.cr, h));
            hi = std::max(hi, ru(p.cr, h));
          }
        }
        if (p.tan) {
          const bool pl = is_pole(l), pr = is_pole(r);
          if (pole_in || (pl && pr)) {
            lo = -GAOL_INFINITY;
            hi = GAOL_INFINITY;
          } else {
            lo = pl ? -GAOL_INFINITY : rd(p.cr, l);
            hi = pr ? GAOL_INFINITY : ru(p.cr, r);
          }
        } else {
          lo = std::min(lo, std::min(rd(p.cr, l), rd(p.cr, r)));
          hi = std::max(hi, std::max(ru(p.cr, l), ru(p.cr, r)));
        }
        std::fesetround(FE_UPWARD);
        check(std::string(p.name) + " over an interval: the tightest bounds",
              got.left() == lo && got.right() == hi,
              [&] {
                return std::string(p.name) + "([" + show(l) + ", " + show(r) + "]) = ["
                     + show(got.left()) + ", " + show(got.right()) + "] rather than ["
                     + show(lo) + ", " + show(hi) + "]";
              });
      }
    }

    // The values given apart: the exact ones, the extrema and the poles at a
    // bound, the infinite bounds, the domain of acospi, and beyond 2^61
    const double inf = GAOL_INFINITY;
    struct Case { const char *what; interval got; double lo, hi; bool empty; };
    const Case cases[] = {
      {"sinpi([1e17]) = 0, where sin(pi*x) gave [-1, 1]", sinpi(interval(1e17)), 0.0, 0.0, false},
      {"sinpi([1/2]) = 1", sinpi(interval(0.5)), 1.0, 1.0, false},
      {"cospi([1]) = -1", cospi(interval(1.0)), -1.0, -1.0, false},
      {"tanpi([1/4]) = 1", tanpi(interval(0.25)), 1.0, 1.0, false},
      {"atanpi([1]) = 1/4", atanpi(interval(1.0)), 0.25, 0.25, false},
      {"acospi([0]) = 1/2", acospi(interval(0.0)), 0.5, 0.5, false},
      {"exp2m1([10]) = 1023", exp2m1(interval(10.0)), 1023.0, 1023.0, false},
      {"exp10m1([3]) = 999", exp10m1(interval(3.0)), 999.0, 999.0, false},
      {"expm1([0]) = 0", expm1(interval(0.0)), 0.0, 0.0, false},
      {"sinpi([0, 1]) = [0, 1]", sinpi(interval(0.0, 1.0)), 0.0, 1.0, false},
      {"tanpi([0.4, 0.6]): a pole within", tanpi(interval(0.4, 0.6)), -inf, inf, false},
      {"tanpi([1/2]): a pole alone, no value", tanpi(interval(0.5)), 0.0, 0.0, true},
      {"expm1([-oo, 0]) = [-1, 0]", expm1(interval(-inf, 0.0)), -1.0, 0.0, false},
      {"atanpi(entire) = [-1/2, 1/2]", atanpi(interval::universe()), -0.5, 0.5, false},
      {"sinpi(entire) = [-1, 1]", sinpi(interval::universe()), -1.0, 1.0, false},
      {"acospi([2, 3]): outside the domain", acospi(interval(2.0, 3.0)), 0.0, 0.0, true},
      {"acospi([-5, 5]) = [0, 1]", acospi(interval(-5.0, 5.0)), 0.0, 1.0, false},
      {"sinpi(empty)", sinpi(interval::emptyset()), 0.0, 0.0, true},
      {"cospi([2^62]) = 1", cospi(interval(4611686018427387904.0)), 1.0, 1.0, false},
      {"sinpi([2^62, 2^62 + 1024]) = [-1, 1]",
       sinpi(interval(4611686018427387904.0, 4611686018427388928.0)), -1.0, 1.0, false},
    };
    for (const Case& c : cases) {
      if (c.empty) {
        check(std::string(c.what), c.got.is_empty(), [&] { return hex(c.got); });
      } else {
        check(std::string(c.what), !c.got.is_empty() && c.got.left() == c.lo && c.got.right() == c.hi,
              [&] { return hex(c.got); });
      }
    }
  }
  /*
    The forward functions of Table 10.5 claimed the tightest (GAOL v5), over
    intervals: each result has to be the hull of the image, computed here
    apart from GAOL with CORE-MATH in the two directed roundings -- for the
    monotonic ones the values at the bounds of the part of the interval
    inside the domain, for hypot the values at the points of the box nearest
    to the origin and farthest from it, and for atan2pi the values at the
    corners of the box. The bounds are drawn among the points where the value
    is a double (2^k - 1 for log2p1, 10^k - 1 for log10p1, 4^k for rsqrt,
    Pythagorean triples for hypot, the diagonals for atan2pi...) and their
    neighbours, where a lower bound taken as the value itself is right only if
    the value is exact: an exactness wrongly claimed gives a bound above the
    tightest one, which then no longer encloses the image, and one missed
    gives a bound a double below it.
  */
  void recommended_tightest()
  {
    std::mt19937_64 gen(20260921u);
    const double inf = GAOL_INFINITY;
    const auto rd = [](double (*f)(double), double x) {
      std::fesetround(FE_DOWNWARD);
      const double v = f(x);
      std::fesetround(FE_UPWARD);
      return v;
    };
    const auto ru = [](double (*f)(double), double x) {
      std::fesetround(FE_UPWARD);
      return f(x);
    };
    const auto same = [](const interval& got, double lo, double hi) {
      return !got.is_empty() && got.left() == lo && got.right() == hi;
    };

    // The bounds the intervals are drawn from
    std::vector<double> points = {0.0, 1.0, -1.0, 0.5, -0.5, 2.0, 3.0, 0.25, 4.0, -0.75,
                                  inf, -inf, 1e300, -1e300, 0x1p-1074, 0x1p-1022};
    const auto with_neighbours = [&](double x) {
      points.push_back(x);
      points.push_back(next_float(x));
      points.push_back(previous_float(x));
    };
    for (int k = -53; k <= 60; ++k) {
      with_neighbours(std::ldexp(1.0, k) - 1.0); // 2^k - 1: log2p1
    }
    double p10 = 1.0;
    for (int k = 0; k <= 17; ++k, p10 *= 10.0) {
      with_neighbours(p10 - 1.0);                // 10^k - 1: log10p1
    }
    for (int k = -537; k <= 511; k += 3) {
      with_neighbours(std::ldexp(1.0, 2 * k));   // 4^k: rsqrt
    }
    for (int i = 0; i < 300; ++i) {
      points.push_back(std::ldexp(std::uniform_real_distribution<double>(-1.0, 1.0)(gen),
                                  (int)(gen() % 40) - 20));
      points.push_back(std::uniform_real_distribution<double>(-1.0, 1.0)(gen));
    }

    struct Monotonic { const char *name; double (*cr)(double); interval (*g)(const interval&);
                       double lo, hi; bool lo_open, hi_open, decreasing; };
    const Monotonic monotonic[] = {
      {"expm1", gaol_cr_expm1, expm1, -inf, inf, false, false, false},
      {"exp2m1", gaol_cr_exp2m1, exp2m1, -inf, inf, false, false, false},
      {"exp10m1", gaol_cr_exp10m1, exp10m1, -inf, inf, false, false, false},
      {"atanpi", gaol_cr_atanpi, atanpi, -inf, inf, false, false, false},
      {"acospi", gaol_cr_acospi, acospi, -1.0, 1.0, false, false, true},
      {"asinpi", gaol_cr_asinpi, asinpi, -1.0, 1.0, false, false, false},
      {"log1p", gaol_cr_log1p, log1p, -1.0, inf, true, false, false},
      {"log2p1", gaol_cr_log2p1, log2p1, -1.0, inf, true, false, false},
      {"log10p1", gaol_cr_log10p1, log10p1, -1.0, inf, true, false, false},
      {"rsqrt", gaol_cr_rsqrt, rsqrt, 0.0, inf, true, false, true},
    };
    for (const Monotonic& m : monotonic) {
      for (int i = 0; i < 40000; ++i) {
        double l = points[gen() % points.size()], r = points[gen() % points.size()];
        if (i % 4 == 0) {
          r = l; // a single point
        }
        if (l > r) {
          std::swap(l, r);
        }
        if (l == inf || r == -inf) {
          continue;
        }
        std::fesetround(FE_UPWARD);
        const interval got = m.g(interval(l, r));
        // the part of [l, r] in the domain, the open end of which is a limit
        const bool none = r < m.lo || l > m.hi || (m.lo_open && r == m.lo);
        std::string what = std::string(m.name) + "([" + show(l) + ", " + show(r) + "])";
        if (none) {
          check(std::string(m.name) + ": empty outside the domain", got.is_empty(),
                [&] { return what + " = " + hex(got); });
          continue;
        }
        const double a = (l > m.lo) ? l : (m.lo == 0.0 ? 0.0 : m.lo), b = (r < m.hi) ? r : m.hi;
        const double lo = m.decreasing ? rd(m.cr, b) : rd(m.cr, a);
        const double hi = m.decreasing ? ru(m.cr, a) : ru(m.cr, b);
        check(std::string(m.name) + " over an interval: the tightest bounds", same(got, lo, hi),
              [&] { return what + " = " + hex(got) + " rather than [" + show(lo) + ", " + show(hi) + "]"; });
      }
    }

    /* hypot and atan2pi, over boxes whose bounds are drawn among zeros,
       infinities, the legs of Pythagorean triples scaled by powers of two,
       and their neighbours */
    std::vector<double> coords = {0.0, 1.0, -1.0, 3.0, 4.0, -3.0, -4.0, 5.0, 12.0, 0.5, inf, -inf, 1e300};
    for (int i = 0; i < 400; ++i) {
      const std::uint64_t n = 1 + gen() % 3000, m = n + 1 + gen() % 3000000;
      const int e = (int)(gen() % 2100) - 1060;
      const double a = std::ldexp((double)(m * m - n * n), e), b = std::ldexp((double)(2 * m * n), e);
      const double sa = (gen() & 1) ? 1.0 : -1.0, sb = (gen() & 1) ? 1.0 : -1.0;
      coords.push_back(sa * a);
      coords.push_back(sb * b);
      coords.push_back(sa * next_float(a));
      coords.push_back(sb * std::ldexp(b, 1)); // the triple broken
      coords.push_back(std::ldexp(std::uniform_real_distribution<double>(-1.0, 1.0)(gen),
                                  (int)(gen() % 60) - 30));
    }
    const auto hypot_rd = [](double x, double y) {
      std::fesetround(FE_DOWNWARD);
      const double v = gaol_cr_hypot(x, y);
      std::fesetround(FE_UPWARD);
      return v;
    };
    const auto hypot_ru = [](double x, double y) {
      std::fesetround(FE_UPWARD);
      return gaol_cr_hypot(x, y);
    };
    const auto atan2pi_rd = [](double y, double x) {
      std::fesetround(FE_DOWNWARD);
      const double v = gaol_cr_atan2pi(y, x);
      std::fesetround(FE_UPWARD);
      return v;
    };
    const auto atan2pi_ru = [](double y, double x) {
      std::fesetround(FE_UPWARD);
      return gaol_cr_atan2pi(y, x);
    };
    for (int i = 0; i < 200000; ++i) {
      std::size_t j = gen() % coords.size();
      double x1 = coords[j], y1 = coords[(i % 3 == 0) ? (j ^ 1) : gen() % coords.size()];
      double x2 = (i % 2 == 0) ? x1 : coords[gen() % coords.size()];
      double y2 = (i % 5 < 2) ? y1 : coords[gen() % coords.size()];
      if (i % 7 == 0) {
        std::swap(x1, y1); // the Pythagorean pairs both ways
        std::swap(x2, y2);
      }
      const double xl = std::min(x1, x2), xu = std::max(x1, x2);
      const double yl = std::min(y1, y2), yu = std::max(y1, y2);
      if (xl == inf || xu == -inf || yl == inf || yu == -inf) {
        continue;
      }
      std::fesetround(FE_UPWARD);
      const interval X(xl, xu), Y(yl, yu);
      const std::string box = "([" + show(yl) + ", " + show(yu) + "], [" + show(xl) + ", " + show(xu) + "])";

      // hypot: least at the point nearest to the origin, greatest at the farthest
      const double nx = (xl <= 0.0 && xu >= 0.0) ? 0.0 : std::min(std::fabs(xl), std::fabs(xu));
      const double ny = (yl <= 0.0 && yu >= 0.0) ? 0.0 : std::min(std::fabs(yl), std::fabs(yu));
      const double fx = std::max(std::fabs(xl), std::fabs(xu)), fy = std::max(std::fabs(yl), std::fabs(yu));
      const double hlo = hypot_rd(nx, ny), hhi = hypot_ru(fx, fy);
      const interval h = hypot(X, Y);
      check("hypot over a box: the tightest bounds", same(h, hlo, hhi),
            [&] { return "hypot" + box + " = " + hex(h) + " rather than [" + show(hlo) + ", " + show(hhi) + "]"; });

      // atan2pi: the hull of the corners, but where the box crosses the
      // half-line y = 0, x < 0, where the angle jumps from 1 to -1
      const interval t = atan2pi(Y, X);
      if (xl == 0.0 && xu == 0.0 && yl == 0.0 && yu == 0.0) {
        check("atan2pi: empty at the origin alone", t.is_empty(), [&] { return "atan2pi" + box; });
        continue;
      }
      double tlo, thi;
      if (yl < 0.0 && yu >= 0.0 && xl < 0.0) {
        tlo = -1.0;
        thi = 1.0;
      } else {
        tlo = inf;
        thi = -inf;
        const double cy[2] = {yl, yu}, cx[2] = {xl, xu};
        for (double y : cy) {
          for (double x : cx) {
            if (x == 0.0 && y == 0.0) {
              continue; // no angle at the origin
            }
            const double yy = (y == 0.0) ? 0.0 : y; // +0: the angle on y = 0, x < 0 is 1
            tlo = std::min(tlo, atan2pi_rd(yy, x));
            thi = std::max(thi, atan2pi_ru(yy, x));
          }
        }
      }
      check("atan2pi over a box: the tightest bounds", same(t, tlo, thi),
            [&] { return "atan2pi" + box + " = " + hex(t) + " rather than [" + show(tlo) + ", " + show(thi) + "]"; });
    }

    /* asinpi next to +-1 (1 - |x| about 2^-40), where CORE-MATH's accurate
       phase shifted a 64-bit integer by 65 to 69 bits, undefined behaviour
       that UBSan found (3rd/README.md): each of these arguments reaches that
       shift in some rounding direction, the four with (u) in the upward one
       GAOL computes in. The jobs of the continuous integration with the
       sanitizers stop on it, and the bounds have to be the tightest. */
    const double near_one[] = {
      0x1.ffffffffff03bp-1, 0x1.ffffffffff0c7p-1 /* (u) */, 0x1.ffffffffff23p-1 /* (u) */,
      0x1.ffffffffff28bp-1 /* (u) */, 0x1.ffffffffff358p-1, 0x1.ffffffffff7a3p-1,
      0x1.ffffffffff83bp-1, 0x1.ffffffffffaf7p-1, 0x1.ffffffffffc8cp-1 /* (u) */,
      0x1.ffffffffffe8cp-1, 0x1.fffffffffff23p-1,
    };
    const int directions[] = {FE_TONEAREST, FE_UPWARD, FE_DOWNWARD, FE_TOWARDZERO};
    for (double x0 : near_one) {
      for (double x : {x0, -x0}) {
        double v[4];
        for (int d = 0; d < 4; ++d) {
          std::fesetround(directions[d]);
          v[d] = gaol_cr_asinpi(x);
        }
        std::fesetround(FE_UPWARD);
        const interval got = asinpi(interval(x));
        check("asinpi next to +-1: the tightest bounds, the four roundings consistent",
              same(got, v[2], v[1]) && v[2] <= v[0] && v[0] <= v[1] && (v[3] == v[1] || v[3] == v[2])
              && next_float(v[2]) == v[1],
              [&] { return "asinpi(" + show(x) + ") = " + hex(got); });
      }
    }

    // The values given apart
    struct Case { const char *what; interval got; double lo, hi; bool empty; };
    const Case cases[] = {
      {"log1p([-1, 0]) = [-oo, 0]", log1p(interval(-1.0, 0.0)), -inf, 0.0, false},
      {"log1p([-1]): no value", log1p(interval(-1.0)), 0.0, 0.0, true},
      {"log2p1([-5, 1]) = [-oo, 1]", log2p1(interval(-5.0, 1.0)), -inf, 1.0, false},
      {"log2p1([2^53 - 1]) = 53", log2p1(interval(9007199254740991.0)), 53.0, 53.0, false},
      {"log10p1([99]) = 2", log10p1(interval(99.0)), 2.0, 2.0, false},
      {"rsqrt([0, 4]) = [1/2, +oo]", rsqrt(interval(0.0, 4.0)), 0.5, inf, false},
      {"rsqrt([-0, 4]) = [1/2, +oo]", rsqrt(interval(-0.0, 4.0)), 0.5, inf, false},
      {"rsqrt([-4, 0]): no value", rsqrt(interval(-4.0, 0.0)), 0.0, 0.0, true},
      {"rsqrt([4, +oo]) = [0, 1/2]", rsqrt(interval(4.0, inf)), 0.0, 0.5, false},
      {"asinpi([-2, 2]) = [-1/2, 1/2]", asinpi(interval(-2.0, 2.0)), -0.5, 0.5, false},
      {"asinpi([2, 3]): outside the domain", asinpi(interval(2.0, 3.0)), 0.0, 0.0, true},
      {"hypot([3], [4]) = 5", hypot(interval(3.0), interval(4.0)), 5.0, 5.0, false},
      {"hypot([-4, 3], [-1, 12]) = [0, 12.64...]", hypot(interval(-4.0, 3.0), interval(-1.0, 12.0)),
       0.0, gaol_cr_hypot(4.0, 12.0), false},
      {"hypot(entire, [1]) = [1, +oo]", hypot(interval::universe(), interval(1.0)), 1.0, inf, false},
      {"atan2pi([1], [-1]) = 3/4", atan2pi(interval(1.0), interval(-1.0)), 0.75, 0.75, false},
      {"atan2pi([0], [-1]) = 1", atan2pi(interval(0.0), interval(-1.0)), 1.0, 1.0, false},
      {"atan2pi([-1, 1], [-1]) = [-1, 1]", atan2pi(interval(-1.0, 1.0), interval(-1.0)), -1.0, 1.0, false},
      {"atan2pi([0], [0]): no value", atan2pi(interval(0.0), interval(0.0)), 0.0, 0.0, true},
    };
    for (const Case& c : cases) {
      if (c.empty) {
        check(std::string(c.what), c.got.is_empty(), [&] { return hex(c.got); });
      } else {
        check(std::string(c.what), same(c.got, c.lo, c.hi), [&] { return hex(c.got); });
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
  recommended_intervals();
  recommended_tightest();
  std::fesetround(FE_UPWARD);
  const int status = summary();
  gaol::cleanup();
  return status;
}
