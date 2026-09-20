// The benchmark with the intervals of libieeep1788, set-based flavor on
// doubles, computed with MPFR (see bench_ops.h)
//
// Copyright (c) 2026 ENSTA, France
//
// Created 2026-09-20 by Jordan NININ
#include <p1788/p1788.hpp>
#include "bench_ops.h"

struct P1788
{
  typedef p1788::infsup::interval<double, p1788::flavor::infsup::setbased::mpfr_bin_ieee754_flavor> I;
  static constexpr const char *name = "libieeep1788";
  static I make(double lo, double hi) { return I(lo, hi); }
  static I decimal(const char *s) { return I(std::string("[") + s + "]"); }
  static I sqr(const I& x) { return p1788::infsup::sqr(x); }
  static I sqrt(const I& x) { return p1788::infsup::sqrt(x); }
  static I exp(const I& x) { return p1788::infsup::exp(x); }
  static I log(const I& x) { return p1788::infsup::log(x); }
  static I sin(const I& x) { return p1788::infsup::sin(x); }
  static I cos(const I& x) { return p1788::infsup::cos(x); }
  static I pown(const I& x, int n) { return p1788::infsup::pown(x, n); }
  static I pow(const I& x, const I& y) { return p1788::infsup::pow(x, y); }
  static double mid(const I& x) { return p1788::infsup::mid(x); }
  static double wid(const I& x) { return p1788::infsup::wid(x); }
};

int main(int argc, char *argv[])
{
  return bench_main<P1788>(argc, argv);
}
