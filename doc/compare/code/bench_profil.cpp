// The benchmark with the intervals of PROFIL/BIAS (see bench_ops.h)
#include <Interval.h>
#include <Functions.h>
#include <cstdlib>
#include "bench_ops.h"

struct Profil
{
  typedef INTERVAL I;
  static constexpr const char *name = "profil";
  static I make(double lo, double hi) { return I(lo, hi); }
  // PROFIL/BIAS reads numbers with the >> of doubles, rounded to nearest, and
  // has no interval literal: the decimal number is read with strtod, rounded
  // to nearest, and widened to the two doubles around it
  static I decimal(const char *s)
  {
    const double d = std::strtod(s, NULL);
    return I(Inf(Pred(I(d))), Sup(Succ(I(d))));
  }
  static I sqr(const I& x) { return Sqr(x); }
  static I sqrt(const I& x) { return Sqrt(x); }
  static I exp(const I& x) { return Exp(x); }
  static I log(const I& x) { return Log(x); }
  static I sin(const I& x) { return Sin(x); }
  static I cos(const I& x) { return Cos(x); }
  static I pown(const I& x, int n) { return Power(x, n); }
  static I pow(const I& x, const I& y) { return Power(x, y); }
  static double mid(const I& x) { return Mid(x); }
  static double wid(const I& x) { return Diam(x); }
};

int main(int argc, char *argv[])
{
  return bench_main<Profil>(argc, argv);
}
