/*-*-C++-*----------------------------------------------------------------------
 * gaol -- NOT Just Another Interval Library
 *------------------------------------------------------------------------------
 * Tools of GAOL v5: the differential test of pow (tests/tools/pow).
 *
 * Prints, one line per box (x, y), the bounds of
 *
 *   std  = gaol_ieee1788::pow(x, y)        (pow_standard())
 *   hyb  = gaol::pow(x, y)                 (gaol_pow_hybrid())
 *   real = gaol::pow(x, p) | gaol_ieee1788::pow(x, p), for a degenerate y = [p]
 *          (gaol_pow_real(), and the standard's pow with a double exponent)
 *
 * in hexadecimal, the signs of the zeros included, "empty" for the empty set.
 * The same program linked with two builds of GAOL (two versions, or the SSE2
 * and the FPU intervals) prints the same lines where the two compute the same
 * bounds: diffpow.py builds it, runs it and compares the outputs.
 *
 * The boxes are a grid (about 100 bases: zeros of both signs, negative,
 * straddling 0, from 0, positive, with infinite, subnormal and DBL_MAX bounds,
 * and the empty set; about 105 exponents: degenerate integers within and
 * beyond the ints and at their limits, degenerate non-integers, intervals with
 * finite and infinite bounds, and the empty set), lines "G", each computed
 * with the rounding direction upward, then to nearest, downward and toward
 * zero at the call: "same" where the result is the one computed upward,
 * "DIFF{...}" with the results otherwise. Then random boxes, lines "R",
 * computed upward only, drawn from a fixed seed. The boxes are made with the
 * rounding direction to nearest, so that they are the same in every build.
 *
 *   diffpow [number of random boxes, 60000 by default] > output.txt
 *
 * Copyright (c) 2026 ENSTA, France
 *
 * Created 2026-09-29 by Jordan NININ
 *------------------------------------------------------------------------------
 * gaol is a software distributed WITHOUT ANY WARRANTY. Read the associated
 * COPYING file for information.
 *----------------------------------------------------------------------------*/

#include "gaol/gaol.h"
#include "gaol/gaol_ieee1788.h"

#include <cfenv>
#include <cmath>
#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <limits>
#include <string>
#include <utility>
#include <vector>

using gaol::interval;

namespace
{
  const double oo = std::numeric_limits<double>::infinity();
  const double DMAX = std::numeric_limits<double>::max();
  const double DMIN = std::numeric_limits<double>::denorm_min();
  const double NMIN = std::numeric_limits<double>::min();

  std::string fmt(const interval& x)
  {
    if (x.is_empty()) {
      return "empty";
    }
    char b[80];
    std::snprintf(b, sizeof b, "%a %a", x.left(), x.right());
    return b;
  }

  // xorshift64*, the same sequence on every platform
  struct Rng
  {
    std::uint64_t s;
    explicit Rng(std::uint64_t seed) : s(seed) {}
    std::uint64_t next()
    {
      s ^= s >> 12;
      s ^= s << 25;
      s ^= s >> 27;
      return s * 2685821657736338717ULL;
    }
    double unit() { return static_cast<double>(next() >> 11) * (1.0 / 9007199254740992.0); }
    int below(int n) { return static_cast<int>(next() % static_cast<std::uint64_t>(n)); }
  };

  // A random double of a class chosen at random
  double rnd_double(Rng& r)
  {
    switch (r.below(14)) {
    case 0: return 0.0;
    case 1: return -0.0;
    case 2: return oo;
    case 3: return -oo;
    case 4: return (r.below(2) ? 1.0 : -1.0);
    case 5: return r.below(21) - 10.0; // small integers
    case 6: return (r.unit() - 0.5) * 8.0; // [-4, 4]
    case 7: return std::ldexp(r.unit() + 0.5, r.below(60) - 30) * (r.below(2) ? 1.0 : -1.0);
    case 8: return std::ldexp(r.unit() + 0.5, r.below(2098) - 1074) * (r.below(2) ? 1.0 : -1.0);
    case 9: return std::nextafter(1.0, r.below(2) ? 0.0 : 2.0);
    case 10: return (r.below(2) ? DMAX : -DMAX);
    case 11: return (r.below(2) ? DMIN : -DMIN);
    case 12: return std::ldexp(1.0, r.below(2000) - 1000) * (r.below(2) ? 1.0 : -1.0);
    default: return std::floor((r.unit() - 0.5) * 100.0) / 4.0; // multiples of 0.25
    }
  }

  interval rnd_base(Rng& r)
  {
    if (r.below(20) == 0) {
      return interval::emptyset();
    }
    double a = rnd_double(r), b = rnd_double(r);
    if (r.below(6) == 0) {
      b = a;
    }
    if (a > b) {
      std::swap(a, b);
    }
    return interval(a, b);
  }

  // An exponent: a degenerate integer (small, beyond the ints, or at their
  // limits), a degenerate non-integer, or an interval
  interval rnd_exponent(Rng& r)
  {
    switch (r.below(10)) {
    case 0: case 1: { // a degenerate integer within the ints
      static const double v[] = { 0.0, -0.0, 1, -1, 2, -2, 3, -3, 4, 5, 7, 10, -10, 100, -100, 1000, -1000, 65537,
                                  2147483647.0, -2147483648.0, 2147483646.0, -2147483647.0 };
      return interval(v[r.below(int(sizeof v / sizeof v[0]))]);
    }
    case 2: { // a degenerate integer beyond the ints
      static const double v[] = { 2147483648.0, 2147483649.0, -2147483649.0, -2147483650.0, 1e10, -1e10, 1e12, -1e12,
                                  1e15, -1e15, 9007199254740992.0, -9007199254740992.0, 1e100, -1e100, 1e300, -1e300,
                                  DMAX, -DMAX, 4294967296.0, -4294967296.0 };
      return interval(v[r.below(int(sizeof v / sizeof v[0]))]);
    }
    case 3: case 4: { // a degenerate non-integer
      const double p = (r.below(3) == 0) ? std::ldexp(r.unit() + 0.5, r.below(80) - 40) * (r.below(2) ? 1.0 : -1.0)
                                         : (std::floor((r.unit() - 0.5) * 80.0) + 0.5) / 2.0;
      return interval(p);
    }
    default: { // an interval, with integer bounds or not
      double a, b;
      if (r.below(3) == 0) {
        a = r.below(13) - 6.0;
        b = a + r.below(6);
      } else {
        a = rnd_double(r);
        b = rnd_double(r);
        if (r.below(3) == 0) { // next to the limits of the ints
          a = 2147483647.0 - r.below(3);
          b = 2147483648.0 + r.below(3);
        }
      }
      if (a > b) {
        std::swap(a, b);
      }
      return interval(a, b);
    }
    }
  }

  const int modes[4] = { FE_UPWARD, FE_TONEAREST, FE_DOWNWARD, FE_TOWARDZERO };

  // The three results of (x, y), the rounding direction being mode at each call
  void results(const interval& x, const interval& y, int mode, std::string out[3])
  {
    std::fesetround(mode);
    out[0] = fmt(gaol_ieee1788::pow(x, y));
    std::fesetround(mode);
    out[1] = fmt(gaol::pow(x, y));
    if (y.is_empty() || y.left() != y.right()) {
      out[2] = "-";
    } else {
      const double p = y.left();
      std::fesetround(mode);
      const interval h = gaol::pow(x, p);
      std::fesetround(mode);
      const interval s = gaol_ieee1788::pow(x, p);
      out[2] = fmt(h) + " | " + fmt(s);
    }
  }

  void line(const char *tag, long idx, const interval& x, const interval& y, bool all_modes)
  {
    std::string r0[3];
    results(x, y, modes[0], r0);
    std::printf("%s %ld x=%s y=%s std=%s hyb=%s real=%s", tag, idx, fmt(x).c_str(), fmt(y).c_str(), r0[0].c_str(),
                r0[1].c_str(), r0[2].c_str());
    if (all_modes) {
      for (int m = 1; m < 4; ++m) {
        std::string r[3];
        results(x, y, modes[m], r);
        if (r[0] == r0[0] && r[1] == r0[1] && r[2] == r0[2]) {
          std::printf(" same");
        } else {
          std::printf(" DIFF{%s ; %s ; %s}", r[0].c_str(), r[1].c_str(), r[2].c_str());
        }
      }
    }
    std::printf("\n");
  }
}

int main(int argc, char **argv)
{
  const long randoms = (argc > 1) ? std::atol(argv[1]) : 60000;
  gaol::init();

#ifdef GAOL_USING_SSE2_INSTRUCTIONS
  const char *intervals = "SSE2";
#else
  const char *intervals = "FPU";
#endif
#ifdef GAOL_PRESERVE_ROUNDING
  const char *preserve = " GAOL_PRESERVE_ROUNDING";
#else
  const char *preserve = "";
#endif
  std::printf("# diffpow: GAOL %s, %s intervals%s\n", GAOL_VERSION, intervals, preserve);

  // The boxes are made to nearest: GAOL leaves the rounding direction upward
  std::fesetround(FE_TONEAREST);
  std::vector<interval> bases;
  {
    const double v[][2] = {
      // signs and zeros
      {0.0, 0.0}, {-0.0, -0.0}, {-0.0, 0.0}, {0.0, -0.0},
      // negative
      {-4, -1}, {-oo, -1}, {-1, -0.0}, {-2, -0.5}, {-oo, -0.0}, {-oo, 0.0}, {-DMAX, -DMIN}, {-DMIN, -DMIN}, {-1, -1},
      {-2, 0.0}, {-2, -0.0},
      // straddling 0
      {-2, 3}, {-3, 2}, {-1, 1}, {-oo, 5}, {-5, oo}, {-1e-300, 1e-300}, {-oo, oo}, {-DMIN, DMIN}, {-0.5, 4}, {-4, 0.5},
      {-2, 1e300}, {-1e300, 2},
      // from 0
      {0.0, 1}, {0.0, 0.5}, {0.0, 2}, {0.0, 4}, {0.0, oo}, {0.0, 1e300}, {-0.0, 1}, {0.0, DMIN}, {0.0, DMAX},
      {0.0, 0.25}, {0.0, 1.5}, {0.0, 3}, {-0.0, oo}, {0.0, NMIN},
      // positive
      {1, 1}, {2, 2}, {4, 4}, {0.5, 0.5}, {1, 2}, {0.5, 2}, {0.25, 0.5}, {2, 3}, {0.5, 0.9}, {0.1, 10},
      {1e-300, 1e300}, {1.0000001, 1.0000001}, {0.9999999, 1.0000001}, {DMIN, DMIN}, {DMAX, DMAX}, {NMIN, NMIN},
      {1.5, 2.5}, {3, 3}, {10, 10}, {0.1, 0.1}, {0.25, 0.75}, {0.25, 1}, {1, 1.5}, {2, 4}, {0.75, 1.25}, {1e-5, 1e-3},
      {100, 1000}, {0.3, 0.7}, {1.1, 1.1}, {6, 7}, {DMIN, 1}, {1, DMAX}, {DMIN, DMAX}, {5e-324, 2},
      // unbounded above
      {2, oo}, {1, oo}, {0.5, oo}, {DMAX, oo}, {1e-300, oo}, {4, oo}, {0.25, oo}, {DMIN, oo},
    };
    bases.push_back(interval::emptyset());
    for (const auto& p : v) {
      bases.push_back(interval(p[0], p[1]));
    }
  }

  std::vector<interval> exps;
  {
    exps.push_back(interval::emptyset());
    const double d[] = {
      // degenerate integers within the ints
      0.0, -0.0, 1, -1, 2, -2, 3, -3, 4, 5, 6, 7, 8, 10, -10, 17, 100, -100, 1000, -1000, 100000, 1073741824.0,
      2147483646.0, 2147483647.0, -2147483647.0, -2147483648.0,
      // beyond the ints
      2147483648.0, 2147483649.0, -2147483649.0, -2147483650.0, 4294967296.0, 1e10, -1e10, 1e12, -1e12, 1e15, -1e15,
      9007199254740992.0, 1152921504606846976.0, 1e100, -1e100, 1e300, -1e300, DMAX, -DMAX,
      // degenerate non-integers
      0.5, -0.5, 1.5, 2.5, -2.5, 1.0 / 3.0, -1.0 / 3.0, 0.1, 0.25, 0.75, -0.75, 1e-300, -1e-300, DMIN, -DMIN,
      1023.5, -1023.5, 2147483648.5, -2147483649.5, 10000000000.5, 0.9999999999999999, 1.0000000000000002,
      3.9999999999999996, 1e-10, 12.5, -12.5, 7.25,
    };
    for (double p : d) {
      exps.push_back(interval(p));
    }
    const double b[][2] = {
      {0.0, 1}, {-1, 1}, {-1, 2}, {0.5, 1.5}, {1, 2}, {2, 3}, {-2, -1}, {-3, -0.5}, {0.0, 0.5}, {1.5, 2.5}, {0.5, 1},
      {-0.5, 0.5}, {-0.0, 1}, {-1, 0.0}, {-1, -0.0}, {-2, 0.0}, {0.0, 2}, {0.0, 0.0}, {3, 7}, {-3, 3}, {0.1, 0.2},
      {-oo, 0.0}, {0.0, oo}, {1, oo}, {-oo, -1}, {-oo, oo}, {-1, oo}, {0.5, oo}, {-oo, 0.5}, {-oo, 2}, {2, oo},
      {-oo, -0.0}, {-oo, 1}, {-0.5, oo}, {-oo, -0.5},
      {2147483647.0, 2147483649.0}, {1e10, 1e10 + 2048}, {-1e10, 1e10}, {DMAX / 2, DMAX}, {-DMAX, DMAX}, {-DMAX, 0.0},
      {0.0, DMAX}, {2147483648.0, 4294967296.0}, {-4294967296.0, -2147483648.0}, {2147483646.0, 2147483647.0},
      {-2147483649.0, -2147483648.0}, {1, 3}, {2, 5}, {0.9, 1.1}, {-1.5, -0.5}, {-2.5, 1.5}, {1023, 1024},
    };
    for (const auto& p : b) {
      exps.push_back(interval(p[0], p[1]));
    }
  }

  long n = 0;
  for (const interval& x : bases) {
    for (const interval& y : exps) {
      line("G", n++, x, y, true);
    }
  }
  Rng r(0x9e3779b97f4a7c15ULL);
  for (long i = 0; i < randoms; ++i) {
    std::fesetround(FE_TONEAREST);
    const interval x = rnd_base(r);
    const interval y = rnd_exponent(r);
    line("R", i, x, y, false);
  }
  gaol::cleanup();
  return 0;
}
