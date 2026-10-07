// The benchmark with the intervals of Boost.Interval: its default policies,
// with the elementary functions of the C library added by
// rounded_transc_opp (see boost_policies.h and bench_ops.h)
//
// Copyright (c) 2026 ENSTA, France
//
// Created 2026-10-06 by Jordan NININ
#include <cmath>
#include <cstdlib>
#include "boost_policies.h"
#include "bench_ops.h"

struct Boost
{
  typedef boost_interval I;
  static constexpr const char *name = "boost";
  static I make(double lo, double hi) { return I(lo, hi); }
  // Boost.Interval reads no interval from text (io.hpp has operator<< only):
  // the decimal number is read with strtod, rounded to nearest, and widened to
  // the two doubles around it, as for PROFIL/BIAS
  static I decimal(const char *s)
  {
    const double d = std::strtod(s, nullptr);
    return I(std::nextafter(d, -HUGE_VAL), std::nextafter(d, HUGE_VAL));
  }
  static I sqr(const I& x) { return boost::numeric::square(x); }
  static I sqrt(const I& x) { return boost::numeric::sqrt(x); }
  static I exp(const I& x) { return boost::numeric::exp(x); }
  static I log(const I& x) { return boost::numeric::log(x); }
  static I sin(const I& x) { return boost::numeric::sin(x); }
  static I cos(const I& x) { return boost::numeric::cos(x); }
  static I pown(const I& x, int n) { return boost::numeric::pow(x, n); }
  // Boost.Interval has no real power: exp(y log x), as a program writes it
  static I pow(const I& x, const I& y) { return boost::numeric::exp(y * boost::numeric::log(x)); }
  static double mid(const I& x) { return boost::numeric::median(x); }
  static double wid(const I& x) { return boost::numeric::width(x); }
};

int main(int argc, char *argv[])
{
  return bench_main<Boost>(argc, argv);
}
