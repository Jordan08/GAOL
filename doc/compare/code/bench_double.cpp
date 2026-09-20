// Copyright (c) 2026 ENSTA, France
//
// Created 2026-09-20 by Jordan NININ
//
// The benchmark on doubles, for reference: the same operations on the
// midpoints of the intervals, without any rounding (see bench_ops.h)
#include <cmath>
#include "bench_ops.h"

struct Double
{
  typedef double I;
  static constexpr const char *name = "double";
  static I make(double lo, double hi) { return lo == hi ? lo : lo + (hi - lo) / 2; }
  static I decimal(const char *s) { return std::strtod(s, nullptr); }
  static I sqr(I x) { return x * x; }
  static I sqrt(I x) { return std::sqrt(x); }
  static I exp(I x) { return std::exp(x); }
  static I log(I x) { return std::log(x); }
  static I sin(I x) { return std::sin(x); }
  static I cos(I x) { return std::cos(x); }
  static I pown(I x, int n) { return std::pow(x, n); }
  static I pow(I x, I y) { return std::pow(x, y); }
  static double mid(I x) { return x; }
  static double wid(I) { return 0.0; }
};

int main(int argc, char *argv[])
{
  return bench_main<Double>(argc, argv);
}
