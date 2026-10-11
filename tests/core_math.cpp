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
 * First, the test checks the fma() and round() of the C library, which
 * CORE-MATH and GAOL call, against values computed apart: where they are
 * wrong, so are the bounds, whatever the platform.
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

// The control register of the SSE instructions, where flush-to-zero is set
#if defined(__SSE2__) || defined(_M_X64) || (defined(_M_IX86_FP) && _M_IX86_FP >= 2)
#  include <xmmintrin.h>
#  define GAOL_TESTS_HAVE_MXCSR 1
#endif

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

  /*
    The fma() and round() of the C library, which CORE-MATH and GAOL call.

    CORE-MATH computes with __builtin_fma(), one instruction where the
    compiler has the fused multiply-add instructions (GAOL_FMA) and a call to
    fma() otherwise, as std::fma() is in the exact products of
    gaol/gaol_interval.cpp: the functions and the bounds are right only if that
    fma() is correctly rounded, in the four rounding directions. round() is
    roundTiesToAway (round_ties_to_away() of gaol/gaol_interval.h, and pow of
    CORE-MATH), which does not depend on the rounding direction.

    The fma() of mingw-w64's own math library (see gaol/gaol_config.h) adds
    the products of the halves of x and y to z with four roundings. That of
    mingw-w64 11 for x86-64, under wine, got wrong the three error-free
    products fma(a, b, -a*b) below, on which double-double arithmetic is
    built, the next two triples in every direction, and x + x*2^-54, what
    cr_tan() gives for a tiny x, rounded upward to two doubles above x (the
    MinGW-w64 GCC 11 to 13 of Chocolatey gave wrong bounds so); its round() of
    +-0x1.fffffffffffffp-2 was +-1 in every direction but upward. The fma()
    of mingw-w64 11 for 32-bit x86, computed in extended precision, passes
    these checks, though not correctly rounded everywhere (see
    gaol/gaol_config.h), and so does its round(). The expected values were
    computed with exact rational arithmetic, and checked with mpmath.
  */
  void c_library_fma_and_round()
  {
    const int directions[] = {FE_TONEAREST, FE_UPWARD, FE_DOWNWARD, FE_TOWARDZERO};
    const char* const names[] = {"to nearest", "upward", "downward", "toward zero"};

    struct Triple { double a, b, c; double rounded[4]; }; // in the order of directions
    const Triple triples[] = {
      {0x1.eb7817f86ead9p+10, -0x1.69a347dd2966bp+16, 0x1.5b22e89dbe07ap+27,
       {0x1.f617413fd629ap-27, 0x1.f617413fd629ap-27, 0x1.f617413fd629ap-27, 0x1.f617413fd629ap-27}},
      {-0x1.541e25d0dba6bp+12, 0x1.b7bb54e687499p+0, 0x1.241c4ad579bbbp+13,
       {-0x1.c4af65649cbe6p-41, -0x1.c4af65649cbe6p-41, -0x1.c4af65649cbe6p-41, -0x1.c4af65649cbe6p-41}},
      {-0x1.12a265e560f2bp-14, -0x1.fb5204d248917p+18, -0x1.101fce7f05914p+5,
       {0x1.d89f131cabfbap-49, 0x1.d89f131cabfbap-49, 0x1.d89f131cabfbap-49, 0x1.d89f131cabfbap-49}},
      {-0x1.5db89d6bd603fp+1, -0x1.c75e3f76eee27p+3, 0x1.cb29218b1d2c3p-19,
       {0x1.3709f1eed8b4ep+5, 0x1.3709f1eed8b4ep+5, 0x1.3709f1eed8b4dp+5, 0x1.3709f1eed8b4dp+5}},
      {-0x1.4c7fec7446f29p+0, 0x1.debe6f4813504p-10, -0x1.741bfc46048bep-40,
       {-0x1.36e71980bfe61p-9, -0x1.36e71980bfe61p-9, -0x1.36e71980bfe62p-9, -0x1.36e71980bfe61p-9}},
      {0x1.56e1fc2f8f359p-997, 0x1p-54, 0x1.56e1fc2f8f359p-997,
       {0x1.56e1fc2f8f359p-997, 0x1.56e1fc2f8f35ap-997, 0x1.56e1fc2f8f359p-997, 0x1.56e1fc2f8f359p-997}},
    };
    for (const Triple& t : triples) {
      for (int d = 0; d < 4; ++d) {
        // read at run time, so that the compiler does not compute the fma, and
        // kept by rnd_keep() before the direction changes back (see
        // tests/arithmetic.cpp)
        volatile double a = t.a, b = t.b, c = t.c;
        std::fesetround(directions[d]);
        const double got = gaol::rnd_keep(std::fma(a, b, c));
        std::fesetround(FE_UPWARD);
        check("the fma() of the C library: correctly rounded", got == t.rounded[d],
              [&] {
                return "fma(" + show(t.a) + ", " + show(t.b) + ", " + show(t.c) + ") rounded "
                     + names[d] + " = " + show(got) + " rather than " + show(t.rounded[d]);
              });
      }
    }

    struct Rounded { double x, value; };
    const Rounded values[] = {
      {0x1.fffffffffffffp-2, 0.0}, {-0x1.fffffffffffffp-2, -0.0},
      {0.5, 1.0}, {-0.5, -1.0}, {2.5, 3.0}, {-2.5, -3.0},
      {0x1.fffffffffffffp+51, 0x1p+52}, // 2^52 - 1/2
    };
    for (const Rounded& v : values) {
      for (int d = 0; d < 4; ++d) {
        volatile double x = v.x;
        std::fesetround(directions[d]);
        const double got = gaol::rnd_keep(std::round(x));
        std::fesetround(FE_UPWARD);
        check("the round() of the C library: the same in every rounding direction", got == v.value,
              [&] {
                return "round(" + show(v.x) + ") rounding " + names[d] + " = " + show(got)
                     + " rather than " + show(v.value);
              });
      }
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

  // |x| = m 2^e, m an odd integer, for a finite x other than 0
  void odd_times_power_of_two(double x, std::uint64_t& m, std::int64_t& e)
  {
    int ex;
    const double f = std::frexp(std::fabs(x), &ex); // |x| = f 2^ex, 1/2 <= f < 1
    m = static_cast<std::uint64_t>(std::ldexp(f, 53)); // exact
    e = static_cast<std::int64_t>(ex) - 53;
    while (m % 2 == 0) {
      m /= 2;
      ++e;
    }
  }

  // The integer c whose 2^k-th power is m, or 0 where there is none, for an odd m < 2^53 and k >= 1: the
  // largest c with c^(2^k) <= m, by a search over the integers
  std::uint64_t integer_root(std::uint64_t m, int k)
  {
    const auto at_most_m = [&](std::uint64_t c) { // c^(2^k) <= m
      std::uint64_t p = c;
      for (int i = 0; i < k; ++i) {
        if (p > 0xffffffffu || p*p > m) {
          return false;
        }
        p *= p;
      }
      return p <= m;
    };
    std::uint64_t lo = 1, hi = static_cast<std::uint64_t>(1) << 27; // c^2 <= m < 2^53
    while (lo < hi) {
      const std::uint64_t mid = lo + (hi - lo + 1)/2;
      if (at_most_m(mid)) {
        lo = mid;
      } else {
        hi = mid - 1;
      }
    }
    std::uint64_t p = lo;
    for (int i = 0; i < k; ++i) {
      p *= p;
    }
    return (p == m) ? lo : 0;
  }

  // n 2^e
  Dyadic scaled(const Natural& n, long e)
  {
    Dyadic d;
    d.sign = 1;
    d.m = n;
    d.e = e;
    return d;
  }

  /* What x^y is, for a finite x > 0 other than 1 and a finite y other than 0:
     the reference of pow_exact_at_corners(), computed apart from GAOL, whose
     pow_is_double() decides with square roots and products of doubles, and
     from CORE-MATH, with integers and the exact arithmetic of gaol_tests.h.
     x = m 2^e and y = a 2^q, m and a odd integers.
     - q >= 0, y an integer: x^y = m^y 2^(e y), rational.
     - q < 0, k = -q: a and 2^k are coprime, and x^y is rational where x is the
       2^k-th power of a rational, which is c 2^f with c odd, x being dyadic:
       m = c^(2^k), e = f 2^k, and x^y = c^a 2^(f a); irrational otherwise,
       2^(e a/2^k) being so where 2^k does not divide e. As |e| <= 1074, k <= 10
       for m = 1, and as 3 <= c, 3^(2^k) <= m < 2^53, k <= 5 for m >= 3.
     A rational x^y, c^A 2^s, is a double where A > 0, c^A < 2^53, s >= -1074
     and c^A 2^s < 2^1024, and the midpoint of two doubles where c^A has 54
     bits. Its value is kept where c^|A| < 2^200: beyond, which A > 200 gives,
     x^y is known to be neither a double nor a midpoint. A value beyond 2^1100,
     or below 2^-1100, is taken as 2^1100 or 2^-1100, which round to the same
     doubles.
     An irrational x^y, y = A/2^k, is the 2^k-th root of the rational x^A,
     which is kept where k <= 6, m^|A| < 2^400 and |e A| <= 2^18: a double
     d > 0 is above x^y where d^(2^k), exact, is above x^A, and no double is
     x^y. These are the square roots of pow.c for y = 0.5, and the other
     roots its first phase rounds. */
  struct Power
  {
    enum Kind { irrational, rational, beyond } kind = irrational; // beyond: rational, c^|A| >= 2^200
    bool is_double = false;
    Exact value;
    int root = 0; // irrational, and x^y = radicand^(1/2^root) where root > 0
    Exact radicand;
    std::string text; // c^A 2^s, or (m^A 2^s)^(1/2^k)
  };

  // Where x^y is irrational, y = A/2^k, k = -q, x = m 2^e: x^y = (x^A)^(1/2^k), x^A = m^A 2^(e A)
  void root_of_rational(Power& r, std::uint64_t m, std::int64_t e, std::int64_t q, std::int64_t A)
  {
    const std::int64_t k = -q, magnitude = (A < 0) ? -A : A, limit = static_cast<std::int64_t>(1) << 18;
    if (k > 6 || magnitude > limit || (e < 0 ? -e : e)*magnitude > limit) {
      return;
    }
    const Natural bound = Natural(1u).shifted_left(400);
    Natural p(1u); // m^|A|
    for (std::int64_t i = 0; m != 1 && i < magnitude; ++i) {
      p = p*Natural(m);
      if (compare(p, bound) >= 0) {
        return;
      }
    }
    const std::int64_t s = e*A;
    r.root = static_cast<int>(k);
    r.radicand = (A > 0) ? exact(scaled(p, static_cast<long>(s)))
                         : quotient(scaled(Natural(1u), static_cast<long>(s)), scaled(p, 0));
    r.text = "(" + (m == 1 ? std::string() : std::to_string(m) + "^" + std::to_string(A) + " ") + "2^"
           + std::to_string(s) + ")^(1/" + std::to_string(static_cast<std::int64_t>(1) << k) + ")";
  }

  Power power_of(double x, double y)
  {
    Power r;
    std::uint64_t m, a;
    std::int64_t e, q;
    odd_times_power_of_two(x, m, e);
    odd_times_power_of_two(y, a, q);
    std::uint64_t c = m;
    std::int64_t f = e;
    if (q < 0) {
      const std::int64_t k = -q, signed_a = (y < 0.0) ? -static_cast<std::int64_t>(a) : static_cast<std::int64_t>(a);
      if (k > (m == 1 ? 10 : 5) || e % (static_cast<std::int64_t>(1) << k) != 0) {
        root_of_rational(r, m, e, q, signed_a);
        return r;
      }
      if (m != 1) {
        c = integer_root(m, static_cast<int>(k));
        if (c == 0) {
          root_of_rational(r, m, e, q, signed_a);
          return r;
        }
      }
      f = e/(static_cast<std::int64_t>(1) << k);
    }
    // x^y = c^A 2^(f A), A = a for q < 0 and y for q >= 0
    const double magnitude = (q < 0) ? static_cast<double>(a) : std::fabs(y);
    const int sign = (y < 0.0) ? -1 : 1;
    const double limit = (c == 1) ? 2097152.0 : 200.0; // |f A| >= 2^21 for c = 1, c^|A| >= 3^201 for c >= 3
    if (magnitude > limit) {
      if (c != 1) {
        r.kind = Power::beyond;
        r.text = std::to_string(c) + "^" + (sign < 0 ? "-" : "") + "A, A > 200";
        return r;
      }
      r.kind = Power::rational;
      const bool above = (f > 0) == (sign > 0);
      r.value = exact(scaled(Natural(1u), above ? 1100 : -1100));
      r.text = std::string("2^") + (above ? "" : "-") + "s, s > 2^21";
      return r;
    }
    const std::int64_t A = sign*static_cast<std::int64_t>(magnitude), s = f*A;
    Natural p(1u); // c^|A|
    std::uint64_t small = 1; // c^|A| while it is below 2^53, 0 beyond
    for (std::int64_t i = 0; c != 1 && i < (A < 0 ? -A : A); ++i) {
      p = p*Natural(c);
      small = (small != 0 && small <= ((static_cast<std::uint64_t>(1) << 53) - 1)/c) ? small*c : 0;
    }
    if (compare(p, Natural(1u).shifted_left(200)) >= 0) {
      r.kind = Power::beyond;
      r.text = std::to_string(c) + "^" + std::to_string(A) + " 2^" + std::to_string(s);
      return r;
    }
    r.kind = Power::rational;
    r.text = (c == 1 ? "" : std::to_string(c) + "^" + std::to_string(A) + " ") + "2^" + std::to_string(s);
    if (s > 1300 || s < -1300) { // c^|A| < 2^200: x^y beyond 2^1100, or below 2^-1100
      r.value = exact(scaled(Natural(1u), (s > 0) ? 1100 : -1100));
      return r;
    }
    if (A > 0 || c == 1) {
      int bits = 0;
      while (small >> bits != 0) {
        ++bits;
      }
      r.is_double = small != 0 && s >= -1074 && s + bits <= 1024;
      r.value = exact(scaled(p, static_cast<long>(s)));
    } else {
      r.value = quotient(scaled(Natural(1u), static_cast<long>(s)), scaled(p, 0));
    }
    return r;
  }

  // The sign of d - x^y, d being a double, infinite or not, where power_of() knows x^y: rational, or the
  // 2^k-th root of a rational
  int compare_with(double d, const Power& power)
  {
    if (power.kind == Power::rational) {
      return compare(d, power.value);
    }
    if (!(d > 0.0)) {
      return -1;
    }
    if (d == inf) {
      return 1;
    }
    Dyadic t = dyadic(d); // d^(2^k)
    for (int i = 0; i < power.root; ++i) {
      t = t*t;
    }
    return compare(exact(t), power.radicand);
  }

  /* The bounds of pow(x, y) at the corners of a box, where x^y is a double
     (GAOL v5). GAOL takes CORE-MATH's value in the upward rounding as the upper
     bound and the double below it as the lower one, unless x^y is a double,
     which is then the lower bound: pow([4], 0.5) was [2 - 2^-52, 2]. x^y is a
     double for x = 2^p and y = t/p, t an integer of [-1074, 1023], and for
     x = c^(2^k) 2^(f 2^k), c odd, and y = a/2^k, a > 0, where c^a < 2^53 and
     c^a 2^(f a) is within the doubles; for no other pair, however near. The
     pairs below are those, at the edges of the doubles too (subnormal bases
     and powers, 2^-1074, f a from -1077 to -1074, the largest powers and the
     first beyond, 3^33 and 3^34), the integers from 2 to 100 to the powers
     a/2^k, k <= 3 (the roots of the squares and of the fourth powers, 81^(3/4)
     among them, and 17, 33..., 1 modulo 8 but no squares), midpoints between
     two doubles, c^a odd of 54 bits, whose power is small enough for
     CORE-MATH to compute it exactly (3^34 2^-1020, 459^6 2^-1032, 197^7 2^-1008
     and 2^-1015, 29^11 2^-1045), the neighbours of each (the double above and
     below x, then y), a power of two whose product p*y rounds to an integer
     without being one (8 to the double nearest 1/3, whose 3y is 1 - 2^-54, and
     to the double above), and random pairs.

     The bounds have to be the tightest ones: the lower bound RD(x^y) at every
     pair, which is x^y where it is a double, and the upper one RU(x^y). Where
     x^y is rational, the reference is x^y itself, which power_of() computes
     exactly, apart from GAOL and from CORE-MATH; where it is the 2^k-th root
     of a rational, y = A/2^k with k <= 6, the doubles are compared with it
     exactly too, d^(2^k) with x^A. CORE-MATH's values in the two directed
     roundings have to be its roundings too. Elsewhere, x^y irrational with
     more digits in y, or rational of too many digits, it is neither a double
     nor the midpoint of two, and the reference is CORE-MATH's values downward
     and upward, which have to be two neighbouring doubles. Wherever x^y is no
     double, GAOL's bounds have to differ, which needs no reference. So a
     fault of CORE-MATH is told apart from one of GAOL's bounds: on 32-bit
     ARM, CORE-MATH rounded to nearest in every direction the midpoints it
     computes exactly (see exact_pow() in
     3rd/math-core/src/binary64/pow/pow.c), and GAOL's upper bound, its value
     upward, was below x^y there; compiled by Visual C++, CORE-MATH returned
     -0 rounding downward where x^y is between 2^-1075 and about 2^-947; and
     on a 32-bit x86 Windows, with Visual C++ and with MinGW-w64 at -O0, the
     square root it takes for y = 0.5 was rounded to nearest in every
     direction, and GAOL's upper bound of pow([x], [0.5]) was below the root
     at subnormal x (see gaol/core_math_port.h). A reference taken from
     CORE-MATH alone took the first and the last for faults of the lower
     bound, or checked no bound.

     The box [x] x [y] is degenerate, and pow takes pown for an integer y: the
     box is then [y, next(y)] or [prev(y), y], the one whose lower bound is at
     y, and the other for the upper bound. The pairs are made in the rounding to
     nearest, whatever the direction the tests before left. */
  void pow_exact_at_corners()
  {
    std::vector<std::pair<double, double>> pairs;
    const auto add = [&](double x, double y) {
      pairs.push_back({x, y});
      pairs.push_back({next_float(x), y});
      pairs.push_back({previous_float(x), y});
      pairs.push_back({x, next_float(y)});
      pairs.push_back({x, previous_float(y)});
    };
    std::fesetround(FE_TONEAREST);
    // Powers of two: 2^p to the power t/p, t around the ends of the range of doubles
    for (int p : {-1074, -1073, -1023, -1022, -538, -537, -100, -3, -2, -1, 1, 2, 3, 10, 511, 512, 1000, 1023}) {
      const double x = std::ldexp(1.0, p);
      for (int t : {-1075, -1074, -1073, -1022, -100, -3, -1, 1, 2, 3, 100, 1023, 1024}) {
        add(x, (double)t / (double)p);
      }
      for (double y : {0.5, 0.25, 1.5, 2.0, 3.0, 33.0, -0.5, -1.0, -2.0, 0x1.5555555555555p-2, 0x1.5555555555556p-2}) {
        add(x, y);
      }
    }
    // c^(2^k) 2^(f 2^k) to the powers a/2^k. c^a is below 2^53 up to a = 33 for c = 3
    // (3^33 = 5559060566555523), and above it for a = 34. f = -537 and -358 give subnormal bases and
    // powers, 3 2^-537 = (9 2^-1074)^(1/2) and 27 2^-1074 = (9 2^-716)^(3/2) among them, f = -538, -359 and
    // -215 the exponents f a = -1076, -1077 and -1075 just below the least double, and f = 340 a power
    // beyond the largest double whose exponent f a is not: (9 2^680)^(3/2) = 27 2^1020
    struct Corner
    {
      int c, f, k, a;
    };
    std::vector<Corner> corners;
    for (int k = 0; k <= 6; ++k) {
      for (int c : {3, 5, 7, 9, 15, 255, 257, 65537, 67108865}) {
        for (int f : {-538, -537, -359, -358, -215, -30, -1, 0, 1, 20, 340}) {
          for (int a : {1, 2, 3, 5, 7, 9, 31, 32, 33, 34, -1, -3}) {
            if (k == 0 || a % 2 != 0) {
              corners.push_back({c, f, k, a});
            }
          }
        }
      }
    }
    // Midpoints that CORE-MATH computes exactly, (3 2^-30)^34 among those above
    corners.push_back({459, -172, 0, 6});
    corners.push_back({197, -144, 0, 7});
    corners.push_back({197, -145, 1, 7});
    corners.push_back({29, -95, 2, 11});
    for (const Corner& corner : corners) {
      double m = corner.c;
      for (int i = 0; i < corner.k; ++i) {
        m *= m; // c^(2^k), exact below 2^53
      }
      if (m < 9007199254740992.0) {
        add(std::ldexp(m, corner.f*(1 << corner.k)), (double)corner.a/std::ldexp(1.0, corner.k));
      }
    }
    // The integers from 2 to 100 to the powers a/2^k, k <= 3
    for (int x = 2; x <= 100; ++x) {
      for (int k = 0; k <= 3; ++k) {
        for (int a = -3; a <= 34; ++a) {
          if (a != 0 && (k == 0 || a % 2 != 0)) {
            add((double)x, (double)a/(double)(1 << k));
          }
        }
      }
    }
    // Random pairs, drawn one number at a time, the same whatever the order in which a compiler evaluates
    // the arguments of a function
    std::mt19937_64 gen(20260929u);
    for (int i = 0; i < 4000; ++i) {
      const double significand = 1.0 + (double)(gen() >> 11)/9007199254740992.0;
      const int exponent = (int)(gen() % 200) - 100;
      const double numerator = (double)((int)(gen() % 2049) - 1024);
      const int k = (int)(gen() % 6);
      add(std::ldexp(significand, exponent), numerator/(double)(1 << k));
    }
    int doubles = 0, rationals = 0, roots = 0;
    for (const std::pair<double, double>& pair : pairs) {
      const double x = pair.first, y = pair.second;
      if (!(x > 0.0) || x == 1.0 || y == 0.0 || !std::isfinite(x) || !std::isfinite(y)) {
        continue;
      }
      std::fesetround(FE_TONEAREST);
      const Power power = power_of(x, y);
      std::fesetround(FE_DOWNWARD);
      const double lo = gaol_cr_pow(x, y);
      std::fesetround(FE_UPWARD);
      const double hi = gaol_cr_pow(x, y);
      const auto describe = [&] { return "pow(" + show(x) + ", " + show(y) + ")"; };
      const bool rational = power.kind == Power::rational, known = rational || power.root > 0;
      const auto power_text = [&] {
        return std::string(", x^y being ") + (power.text.empty() ? "irrational" : power.text);
      };
      // Where x^y is known: whether d <= x^y, and whether d >= x^y
      const auto at_most = [&](double d) { return compare_with(d, power) <= 0; };
      const auto at_least = [&](double d) { return compare_with(d, power) >= 0; };
      // CORE-MATH's values: x^y rounded where it is known exactly, two neighbouring doubles elsewhere,
      // x^y being then no double
      bool reference = true;
      if (known) {
        rationals += rational;
        roots += !rational;
        doubles += power.is_double;
        check(rational ? "pow: CORE-MATH's pow is x^y rounded downward and upward where x^y is rational"
                       : "pow: CORE-MATH's pow is x^y rounded downward and upward where x^y is a 2^k-th root, k <= 6",
              at_most(lo) && !at_most(next_double(lo)) && at_least(hi) && !at_least(previous_double(hi)),
              [&] { return describe() + " is " + show(lo) + " downward and " + show(hi) + " upward" + power_text(); });
      } else {
        reference = !std::signbit(lo) && hi == next_double(lo);
        check("pow: CORE-MATH's pow downward and upward are neighbouring doubles where x^y is not known exactly",
              reference,
              [&] { return describe() + " is " + show(lo) + " downward and " + show(hi) + " upward" + power_text(); });
      }
      const bool integer = std::floor(y) == y;
      const interval at_lower = !integer ? interval(y, y) : (x > 1.0 ? interval(y, next_float(y)) : interval(previous_float(y), y));
      const interval at_upper = !integer ? interval(y, y) : (x > 1.0 ? interval(previous_float(y), y) : interval(y, next_float(y)));
      const interval got = evaluate("pow", [&] { return pow(interval(x, x), at_lower); }, describe);
      const interval got_upper = !integer ? got : evaluate("pow", [&] { return pow(interval(x, x), at_upper); }, describe);
      const double l = got.is_empty() ? inf : got.left(), u = got_upper.is_empty() ? -inf : got_upper.right();
      if (!integer && !power.is_double) {
        check("pow: the bounds differ where x^y is not a double", l < u,
              [&] { return describe() + " is [" + show(l) + ", " + show(u) + "]" + power_text(); });
      }
      if (!known && !reference) {
        continue; // no reference
      }
      const auto lower = [&] {
        return describe() + " has the lower bound " + show(l) + (known ? power_text() : " rather than " + show(lo));
      };
      const auto upper = [&] {
        return describe() + " has the upper bound " + show(u) + (known ? power_text() : " rather than " + show(hi));
      };
      const bool lower_sound = known ? at_most(l) : l <= lo;
      const bool upper_sound = known ? at_least(u) : u >= hi;
      check("pow: the lower bound is at most x^y", lower_sound, lower);
      check(power.is_double ? "pow: the lower bound is x^y where it is a double"
                            : "pow: the lower bound is the tightest one where x^y is not a double",
            lower_sound && (known ? !at_most(next_double(l)) : l == lo), lower);
      check("pow: the upper bound is at least x^y", upper_sound, upper);
      check("pow: the upper bound is the tightest one",
            upper_sound && (known ? !at_least(previous_double(u)) : u == hi), upper);
    }
    check("pow: powers that are doubles among the pairs", doubles >= 3000,
          [&] { return std::to_string(doubles) + " of them"; });
    check("pow: powers known exactly among the pairs", rationals >= 8000,
          [&] { return std::to_string(rationals) + " of them"; });
    check("pow: powers known as 2^k-th roots among the pairs", roots >= 1000,
          [&] { return std::to_string(roots) + " of them"; });
  }

  /* pow(x, y) at a tiny exponent with flush-to-zero set (GAOL v5), as a
     program linked with -Ofast has it (crtfastmath.o): 1024 y, of which the
     test of whether x^y is a double makes an integer, is then 0 for
     |y| < 2^-1032, and x^y was taken for x^0 = 1, the lower bound of
     pow([0.5], [2^-1074]), above 0.5^(2^-1074) < 1. x86 only, where MXCSR is,
     and where the processor honours flush-to-zero; the other effects of
     flush-to-zero on GAOL's bounds are not looked at here. */
  void pow_with_flush_to_zero()
  {
#if GAOL_TESTS_HAVE_MXCSR
    const unsigned int flush_to_zero = 0x8000u, saved = _mm_getcsr() & flush_to_zero;
    // Whether the processor flushes 1024 2^-1074 to 0. The product is written to volatile memory before
    // the mode is cleared: GCC and Clang, which do not model MXCSR, might compute it afterwards otherwise
    volatile double least = 0x1p-1074, product;
    _mm_setcsr(_mm_getcsr() | flush_to_zero);
    product = 1024.0*least;
    _mm_setcsr((_mm_getcsr() & ~flush_to_zero) | saved);
    if (product != 0.0) {
      std::printf("Flush-to-zero is not honoured: pow with it is not checked\n");
      return;
    }
    _mm_setcsr(_mm_getcsr() | flush_to_zero);
    const interval point = pow(interval(0.5), interval(0x1p-1074));
    const interval box = pow(interval(0.5, 0.75), interval(0x1p-1074, 0x1p-1070));
    _mm_setcsr((_mm_getcsr() & ~flush_to_zero) | saved);
    check("pow with flush-to-zero: the lower bound is below x^y < 1", !point.is_empty() && point.left() < 1.0,
          [&] { return "pow([0.5], [0x1p-1074]) = " + hex(point); });
    check("pow with flush-to-zero: the lower bound is below x^y < 1", !box.is_empty() && box.left() < 1.0,
          [&] { return "pow([0.5, 0.75], [0x1p-1074, 0x1p-1070]) = " + hex(box); });
#else
    std::printf("No control register of the SSE instructions: pow with flush-to-zero is not checked\n");
#endif
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
  /* sin at arguments that take the accurate path of CORE-MATH's sin.c in the
     upward rounding GAOL computes in, where the rewrite of upstream commit
     6b84457 computes with the 128-bit integer throughout (sin_large_accurate(),
     reduce_large_acc(), mhUU(), u128_tod()), ported to gaol_u128
     (3rd/README.md): from 2^-16 to 1, from 1 to 2^31, beyond 2^31, and next to
     multiples of pi. The values are those of the upstream sources, compiled
     unchanged, in the four roundings: the jobs of the continuous integration
     that compute with the two 64-bit halves (GAOL_U128_EMULATION, Visual C++,
     the 32-bit targets) have to find them too. */
  void sin_accurate_path()
  {
    struct Value { double x, rn, ru, rd, rz; };
    const Value values[] = {
      {-0x1.d14d4430c32b6p-14, -0x1.d14d4420c006dp-14, -0x1.d14d4420c006cp-14, -0x1.d14d4420c006dp-14, -0x1.d14d4420c006cp-14},
      {-0x1.336ec93771f1dp-13, -0x1.336ec924f89e7p-13, -0x1.336ec924f89e7p-13, -0x1.336ec924f89e8p-13, -0x1.336ec924f89e7p-13},
      {0x1.10c145535098fp-13, 0x1.10c1454669ea5p-13, 0x1.10c1454669ea5p-13, 0x1.10c1454669ea4p-13, 0x1.10c1454669ea4p-13},
      {0x1.c4fbc1a4b3a65p-15, 0x1.c4fbc1a1021e6p-15, 0x1.c4fbc1a1021e7p-15, 0x1.c4fbc1a1021e6p-15, 0x1.c4fbc1a1021e6p-15},
      {-0x1.a170e03c7f1d6p+11, 0x1.d73f6254f437fp-7, 0x1.d73f6254f438p-7, 0x1.d73f6254f437fp-7, 0x1.d73f6254f437fp-7},
      {0x1.9ebef1d961064p+17, -0x1.b95c6468b2531p-3, -0x1.b95c6468b253p-3, -0x1.b95c6468b2531p-3, -0x1.b95c6468b253p-3},
      {-0x1.90a5f260af55dp+4, 0x1.7939da374bb67p-4, 0x1.7939da374bb68p-4, 0x1.7939da374bb67p-4, 0x1.7939da374bb67p-4},
      {0x1.4fe39e39fcc5p+20, -0x1.c2c850d185ec3p-1, -0x1.c2c850d185ec2p-1, -0x1.c2c850d185ec3p-1, -0x1.c2c850d185ec2p-1},
      {0x1.9a43dc47106fep+22, 0x1.845645bb8a558p-7, 0x1.845645bb8a559p-7, 0x1.845645bb8a558p-7, 0x1.845645bb8a558p-7},
      {-0x1.3c4c1127c58c2p+759, 0x1.4f9030064a64cp-12, 0x1.4f9030064a64dp-12, 0x1.4f9030064a64cp-12, 0x1.4f9030064a64cp-12},
      {-0x1.a099d37b49741p+931, 0x1.618390d7c05e5p-10, 0x1.618390d7c05e6p-10, 0x1.618390d7c05e5p-10, 0x1.618390d7c05e5p-10},
      {-0x1.40fde94b6f696p+889, 0x1.73d23c7e5ee47p-8, 0x1.73d23c7e5ee48p-8, 0x1.73d23c7e5ee47p-8, 0x1.73d23c7e5ee47p-8},
      {-0x1.e0e52ffbecd8ap+141, 0x1.6a2a2880dbf25p-5, 0x1.6a2a2880dbf25p-5, 0x1.6a2a2880dbf24p-5, 0x1.6a2a2880dbf24p-5},
      {0x1.62a76058c6aadp+764, 0x1.25ae24b86d4fcp-3, 0x1.25ae24b86d4fcp-3, 0x1.25ae24b86d4fbp-3, 0x1.25ae24b86d4fbp-3},
      {-0x1.0f92923c41b34p+627, -0x1.e1a4679a65e86p-1, -0x1.e1a4679a65e86p-1, -0x1.e1a4679a65e87p-1, -0x1.e1a4679a65e86p-1},
      {0x1.f6150c5c955eep+538, 0x1.3269779299746p-6, 0x1.3269779299746p-6, 0x1.3269779299745p-6, 0x1.3269779299745p-6},
      // next to multiples of pi
      {0x1.f00e7f249eb5ap+33, 0x1.c029885e3124ap-21, 0x1.c029885e3124bp-21, 0x1.c029885e3124ap-21, 0x1.c029885e3124ap-21},
      {0x1.51481f216a9b2p+42, 0x1.3bb9e1309d4eep-13, 0x1.3bb9e1309d4efp-13, 0x1.3bb9e1309d4eep-13, 0x1.3bb9e1309d4eep-13},
      {0x1.17ba50afa4cfp+35, 0x1.0ed85ce5ce86cp-20, 0x1.0ed85ce5ce86dp-20, 0x1.0ed85ce5ce86cp-20, 0x1.0ed85ce5ce86cp-20},
    };
    const int directions[] = {FE_TONEAREST, FE_UPWARD, FE_DOWNWARD, FE_TOWARDZERO};
    for (const Value& e : values) {
      const double want[4] = {e.rn, e.ru, e.rd, e.rz};
      double v[4];
      for (int d = 0; d < 4; ++d) {
        std::fesetround(directions[d]);
        v[d] = gaol_cr_sin(e.x);
      }
      std::fesetround(FE_UPWARD);
      check("sin on its accurate path: the values of the upstream sources in the four roundings",
            v[0] == want[0] && v[1] == want[1] && v[2] == want[2] && v[3] == want[3],
            [&] {
              return "sin(" + show(e.x) + ") = " + show(v[0]) + ", " + show(v[1]) + ", " + show(v[2]) + ", "
                   + show(v[3]) + " rather than " + show(want[0]) + ", " + show(want[1]) + ", "
                   + show(want[2]) + ", " + show(want[3]);
            });
      const interval got = sin(interval(e.x));
      check("sin on its accurate path: the tightest bounds",
            !got.is_empty() && got.left() == e.rd && got.right() == e.ru,
            [&] { return "sin([" + show(e.x) + "]) = " + hex(got); });
    }
  }

  /* cbrt at the seven arguments of wlist in CORE-MATH's cbrt.c, whose cube
     roots are less than 10^-15 ulp above a double: cr_cbrt() rounds them
     apart, away from zero when get_rounding_mode() gives it 2, the upward
     rounding, for a positive x (1, downward, for a negative one). With
     mingw-w64 on x86-64 (MinGW-w64, MSYS2), get_rounding_mode() returned
     FE_UPWARD itself, 0x800, where 0 to 3 are expected, and the upper bound
     of nth_root(x, 3) was below the cube root (3rd/README.md); with clang-cl
     on x86-64 it took the upward and the downward roundings for toward zero
     (see rsqrt_hard_cases()). The roundings are computed apart, with mpmath
     at 2000 bits: below and above are the doubles on each side of the cube
     root of x, the nearest being below. The cube root of x 8^k is that of x
     times 2^k, exactly, and so are its roundings. */
  void cbrt_hard_cases()
  {
    struct Value { double x, below, above; };
    const Value values[] = {
      {0x1.3a9ccd7f022dbp+0, 0x1.1236160ba9b93p+0, 0x1.1236160ba9b94p+0},
      {0x1.7845d2faac6fep+0, 0x1.23115e657e49cp+0, 0x1.23115e657e49dp+0},
      {0x1.d1ef81cbbbe71p+0, 0x1.388fb44cdcf5ap+0, 0x1.388fb44cdcf5bp+0},
      {0x1.0a2014f62987cp+1, 0x1.46bcbf47dc1e8p+0, 0x1.46bcbf47dc1e9p+0},
      {0x1.fe18a044a5501p+1, 0x1.95decfec9c904p+0, 0x1.95decfec9c905p+0},
      {0x1.a6bb8c803147bp+2, 0x1.e05335a6401dep+0, 0x1.e05335a6401dfp+0},
      {0x1.ac8538a031cbdp+2, 0x1.e281d87098de8p+0, 0x1.e281d87098de9p+0},
    };
    const int directions[] = {FE_TONEAREST, FE_UPWARD, FE_DOWNWARD, FE_TOWARDZERO};
    for (const Value& e : values) {
      for (int k : {0, 1, -1, 340, -340}) {
        for (double s : {1.0, -1.0}) {
          const double x = s * std::ldexp(e.x, 3 * k);
          // the roundings to nearest and toward zero are the double nearer to 0
          const double to_zero = s * std::ldexp(e.below, k), away = s * std::ldexp(e.above, k);
          const double lo = (s > 0.0) ? to_zero : away, hi = (s > 0.0) ? away : to_zero;
          const double want[4] = {to_zero, hi, lo, to_zero};
          double v[4];
          for (int d = 0; d < 4; ++d) {
            std::fesetround(directions[d]);
            v[d] = gaol_cr_cbrt(x);
          }
          std::fesetround(FE_UPWARD);
          check("cbrt at the hard cases of cbrt.c: the roundings of the cube root",
                v[0] == want[0] && v[1] == want[1] && v[2] == want[2] && v[3] == want[3],
                [&] {
                  return "cbrt(" + show(x) + ") = " + show(v[0]) + ", " + show(v[1]) + ", " + show(v[2]) + ", "
                       + show(v[3]) + " rather than " + show(want[0]) + ", " + show(want[1]) + ", "
                       + show(want[2]) + ", " + show(want[3]);
                });
          const interval got = nth_root(interval(x), 3u);
          check("nth_root(x, 3) at the hard cases of cbrt.c: the tightest bounds",
                !got.is_empty() && got.left() == lo && got.right() == hi,
                [&] { return "nth_root([" + show(x) + "], 3) = " + hex(got); });
          const interval std_got = gaol_ieee1788::rootn(interval(x), 3);
          check("rootn(x, 3) of gaol_ieee1788 at the hard cases of cbrt.c: the tightest bounds",
                !std_got.is_empty() && std_got.left() == lo && std_got.right() == hi,
                [&] { return "rootn([" + show(x) + "], 3) = " + hex(std_got); });
        }
      }
    }
  }

  /* rsqrt at the successors of the powers of 4, x = 4^k (1 + 2^-52), for
     every k where x and 2^-k are normal: 1/sqrt(x) is 2^-k (1 + 2^-52)^(-1/2),
     strictly between 2^-k (1 - 2^-53), the double below 2^-k, and 2^-k, and
     about 1.5 x 2^-53 of an ulp above the first, so that it rounds downward,
     to nearest and toward zero to 2^-k (1 - 2^-53) and upward to 2^-k. The
     fast phase of CORE-MATH's rsqrt cannot round them, and as_rsqrt_refine()
     adds the last ulp when get_rounding_mode() says upward. With clang-cl on
     x86-64, which defines __x86_64__ and _WIN32 but not __WIN32__, that
     function compared the FE_UPWARD of glibc, 0x800, with that of the UCRT,
     0x200, and the upper bound of rsqrt(x) was the double below 1/sqrt(x) at
     every one of them (clang-cl 18, the program run under wine; see
     3rd/README.md). */
  void rsqrt_hard_cases()
  {
    const int directions[] = {FE_TONEAREST, FE_UPWARD, FE_DOWNWARD, FE_TOWARDZERO};
    for (int k = -511; k <= 511; ++k) {
      const double x = std::ldexp(1.0 + 0x1p-52, 2 * k);
      const double below = std::ldexp(1.0 - 0x1p-53, -k), above = std::ldexp(1.0, -k);
      const double want[4] = {below, above, below, below};
      double v[4];
      for (int d = 0; d < 4; ++d) {
        std::fesetround(directions[d]);
        v[d] = gaol_cr_rsqrt(x);
      }
      std::fesetround(FE_UPWARD);
      check("rsqrt at the successors of the powers of 4: the roundings of 1/sqrt(x)",
            v[0] == want[0] && v[1] == want[1] && v[2] == want[2] && v[3] == want[3],
            [&] {
              return "rsqrt(" + show(x) + ") = " + show(v[0]) + ", " + show(v[1]) + ", " + show(v[2]) + ", "
                   + show(v[3]) + " rather than " + show(want[0]) + ", " + show(want[1]) + ", "
                   + show(want[2]) + ", " + show(want[3]);
            });
      const interval got = rsqrt(interval(x));
      check("rsqrt at the successors of the powers of 4: the tightest bounds",
            !got.is_empty() && got.left() == below && got.right() == above,
            [&] { return "rsqrt([" + show(x) + "]) = " + hex(got); });
    }
  }

  void recommended_tightest()
  {
    std::mt19937_64 gen(20260921u);
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
    // five values per triple from there: its legs a and b, then three neighbours
    const std::size_t first_triple = coords.size(), triples = 400;
    for (std::size_t i = 0; i < triples; ++i) {
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
      double x1 = coords[j], y1;
      if (i % 3 == 0) {
        // the two legs of a triple, whose hypot is exact; j ^ 1 read past the
        // end of coords for its last index, and paired no legs, the triples
        // starting at an odd index
        j = first_triple + 5 * (j % triples);
        x1 = coords[j];
        y1 = coords[j + 1];
      } else {
        y1 = coords[gen() % coords.size()];
      }
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
  c_library_fma_and_round();
  elementary_functions();
  exact_values();
  two_arguments();
  pow_exact_at_corners();
  pow_with_flush_to_zero();
  base_two_and_ten_exact();
  negative_roots();
  recommended_intervals();
  recommended_tightest();
  sin_accurate_path();
  cbrt_hard_cases();
  rsqrt_hard_cases();
  std::fesetround(FE_UPWARD);
  const int status = summary();
  gaol::cleanup();
  return status;
}
