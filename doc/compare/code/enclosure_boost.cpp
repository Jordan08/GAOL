// The test of enclosure (see enclosure.h) with Boost.Interval, under its two
// policies that take the elementary functions of the C library:
// rounded_transc_opp, the one of the benchmark and of the special cases, which
// keeps the arithmetic of the default policies, and rounded_transc_std (see
// boost_policies.h); and with the C library alone, whose functions are called
// with the rounding direction set downward for the lower bound and upward for
// the upper one, what both policies expect them to honour
//
// Copyright (c) 2026 ENSTA, France
//
// Created 2026-10-06 by Jordan NININ
#include <cfenv>
#include <cmath>
#include "boost_policies.h"
#include "enclosure.h"

template<class Interval>
struct BoostFunctions
{
  typedef Interval I;
  static I point(double x) { return I(x); }
  static double lower(const I& x) { return x.lower(); }
  static double upper(const I& x) { return x.upper(); }
  static I exp(const I& x) { return boost::numeric::exp(x); }
  static I log(const I& x) { return boost::numeric::log(x); }
  static I sin(const I& x) { return boost::numeric::sin(x); }
  static I cos(const I& x) { return boost::numeric::cos(x); }
  static I tan(const I& x) { return boost::numeric::tan(x); }
  static I asin(const I& x) { return boost::numeric::asin(x); }
  static I acos(const I& x) { return boost::numeric::acos(x); }
  static I atan(const I& x) { return boost::numeric::atan(x); }
  static I sinh(const I& x) { return boost::numeric::sinh(x); }
  static I cosh(const I& x) { return boost::numeric::cosh(x); }
  static I tanh(const I& x) { return boost::numeric::tanh(x); }
  static I asinh(const I& x) { return boost::numeric::asinh(x); }
  static I acosh(const I& x) { return boost::numeric::acosh(x); }
  static I atanh(const I& x) { return boost::numeric::atanh(x); }
  static I sqrt(const I& x) { return boost::numeric::sqrt(x); }
  static I pow3(const I& x) { return boost::numeric::pow(x, 3); }
};

struct BoostOpp : BoostFunctions<boost_interval>
{
  static constexpr const char *name = "boost";
};

struct BoostStd : BoostFunctions<boost_interval_std>
{
  static constexpr const char *name = "boost_std";
};

// The C library: f(x) computed with the rounding direction downward, then
// upward, through a volatile pointer, so that the compiler calls the function
// in each direction rather than once
struct Libm
{
  struct I
  {
    double lo, hi;
  };
  static constexpr const char *name = "libm";
  static I point(double x) { return I{ x, x }; }
  static double lower(const I& x) { return x.lo; }
  static double upper(const I& x) { return x.hi; }
  static I call(double (*f)(double), const I& x)
  {
    double (*volatile g)(double) = f;
    const int direction = std::fegetround();
    std::fesetround(FE_DOWNWARD);
    const double lo = g(x.lo);
    std::fesetround(FE_UPWARD);
    const double hi = g(x.hi);
    std::fesetround(direction);
    return I{ lo, hi };
  }
  static double cube(double x) { return std::pow(x, 3.0); }
  static I exp(const I& x) { return call(std::exp, x); }
  static I log(const I& x) { return call(std::log, x); }
  static I sin(const I& x) { return call(std::sin, x); }
  static I cos(const I& x) { return call(std::cos, x); }
  static I tan(const I& x) { return call(std::tan, x); }
  static I asin(const I& x) { return call(std::asin, x); }
  static I acos(const I& x) { return call(std::acos, x); }
  static I atan(const I& x) { return call(std::atan, x); }
  static I sinh(const I& x) { return call(std::sinh, x); }
  static I cosh(const I& x) { return call(std::cosh, x); }
  static I tanh(const I& x) { return call(std::tanh, x); }
  static I asinh(const I& x) { return call(std::asinh, x); }
  static I acosh(const I& x) { return call(std::acosh, x); }
  static I atanh(const I& x) { return call(std::atanh, x); }
  static I sqrt(const I& x) { return call(std::sqrt, x); }
  static I pow3(const I& x) { return call(cube, x); }
};

int main(int argc, char *argv[])
{
  if (argc != 2) {
    std::fprintf(stderr, "usage: %s DIR\n", argv[0]);
    return 1;
  }
  bool ok = enclosure_all<BoostOpp>(argv[1]);
  ok = enclosure_all<BoostStd>(argv[1]) && ok;
  return enclosure_all<Libm>(argv[1]) && ok ? 0 : 1;
}
