// The benchmark with the intervals of filib++, as IBEX uses them (see
// bench_ops.h)
#include <interval/interval.hpp>
#include "bench_ops.h"

struct Filib
{
  typedef filib::interval<double, filib::native_switched, filib::i_mode_extended_flag> I;
  static constexpr const char *name = "filib";
  static I make(double lo, double hi) { return I(lo, hi); }
  // filib++ reads the decimal number as bounds rounded outward, one double
  // wider than the tightest enclosure
  static I decimal(const char *s) { return I(s, s); }
  static I sqr(const I& x) { return filib::sqr(x); }
  static I sqrt(const I& x) { return filib::sqrt(x); }
  static I exp(const I& x) { return filib::exp(x); }
  static I log(const I& x) { return filib::log(x); }
  static I sin(const I& x) { return filib::sin(x); }
  static I cos(const I& x) { return filib::cos(x); }
  // filib++ declares pow(x, int) and power(x, y), but defines them the other
  // way round
  static I pown(const I& x, int n) { return filib::power(x, n); }
  static I pow(const I& x, const I& y) { return filib::pow(x, y); }
  static double mid(const I& x) { return x.mid(); }
  static double wid(const I& x) { return x.diam(); }
};

int main(int argc, char *argv[])
{
  filib::fp_traits<double, filib::native_switched>::setup();
  return bench_main<Filib>(argc, argv);
}
