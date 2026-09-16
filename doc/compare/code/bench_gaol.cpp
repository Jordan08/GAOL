// The benchmark with GAOL's intervals (see bench_ops.h)
#include <gaol/gaol.h>
#include "bench_ops.h"

struct Gaol
{
  typedef gaol::interval I;
  static constexpr const char *name = "gaol";
  static I make(double lo, double hi) { return I(lo, hi); }
  static I decimal(const char *s) { return I(s); }
  static I sqr(const I& x) { return gaol::sqr(x); }
  static I sqrt(const I& x) { return gaol::sqrt(x); }
  static I exp(const I& x) { return gaol::exp(x); }
  static I log(const I& x) { return gaol::log(x); }
  static I sin(const I& x) { return gaol::sin(x); }
  static I cos(const I& x) { return gaol::cos(x); }
  static I pown(const I& x, int n) { return gaol::pow(x, n); }
  static I pow(const I& x, const I& y) { return gaol::pow(x, y); }
  static double mid(const I& x) { return x.midpoint(); }
  static double wid(const I& x) { return x.width(); }
};

int main(int argc, char *argv[])
{
  gaol::init();
  const int status = bench_main<Gaol>(argc, argv);
  gaol::cleanup();
  return status;
}
