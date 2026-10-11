/*-*-C++-*----------------------------------------------------------------------
 * gaol -- NOT Just Another Interval Library
 *------------------------------------------------------------------------------
 * Tools of GAOL v5: the time per call of the pow functions, on each path of
 * pow_standard() and of what gaol::pow adds to it (tests/tools/pow).
 *
 * For each row, the function is called on n random bases (200000 by
 * default), seven times, and the best time per call is printed, in
 * nanoseconds. The rows are the paths that the points B.1 and B.2 of TODO.md
 * change: the corners of a box with finite bounds (gaol::pow with a
 * non-integer exponent goes through two checks more than
 * gaol_ieee1788::pow), the integer powers, the integer exponents beyond the
 * ints, and exp(y log x), which the boxes with an infinite bound and the
 * bases from 0 with an exponent that is not above 0 take today.
 *
 *   bench [n]
 *
 * It is compiled in C++14 or later (build.py takes the C++17 of the tests).
 * The same program linked with two builds of GAOL (build.py) compares them:
 * run the two in turn several times on an idle, pinned processor, and keep
 * the best time of each row (README.md). The times depend on the machine and
 * the compiler: only the ratios between two builds measured together mean
 * something.
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

#include <chrono>
#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <limits>
#include <vector>

using gaol::interval;

namespace
{
  double sink;

  template <class F>
  double ns_per_call(const std::vector<interval>& X, const F& f, int repeat)
  {
    double best = 1e30;
    for (int r = 0; r < repeat; ++r) {
      const auto t0 = std::chrono::steady_clock::now();
      for (std::size_t i = 0; i < X.size(); ++i) {
        const interval z = f(X[i]);
        sink += z.left() + z.right();
      }
      const auto t1 = std::chrono::steady_clock::now();
      const double ns = std::chrono::duration<double, std::nano>(t1 - t0).count() / static_cast<double>(X.size());
      if (ns < best) {
        best = ns;
      }
    }
    return best;
  }
}

int main(int argc, char **argv)
{
  gaol::init();
  const int n = (argc > 1) ? std::atoi(argv[1]) : 200000;
  const double oo = std::numeric_limits<double>::infinity();
  std::uint64_t s = 88172645463325252ULL;
  const auto rnd = [&] {
    s ^= s << 13;
    s ^= s >> 7;
    s ^= s << 17;
    return static_cast<double>(s >> 11) / 9007199254740992.0;
  };
  // The bases: above 0, across 0, unbounded above, and from 0
  std::vector<interval> pos, mixed, unbounded, from0;
  for (int i = 0; i < n; ++i) {
    const double a = 0.1 + 4.0 * rnd(), b = a + 2.0 * rnd();
    pos.push_back(interval(a, b));
    const double c = -2.0 + 4.0 * rnd(), d = c + 3.0 * rnd();
    mixed.push_back(interval(c, d));
    unbounded.push_back(interval(a, oo));
    from0.push_back(interval(0.0, b));
  }
  struct Row
  {
    const char *name;
    double ns;
  };
  std::vector<Row> rows;
  const int R = 7;
  const interval y(0.5, 1.5), ynn(-1.5, -0.5), q(3.0), big(1e10), yinf(1.0, oo);
  const auto add = [&](const char *name, const std::vector<interval>& X, const auto& f) {
    rows.push_back({name, ns_per_call(X, f, R)});
  };
  // The corners of a box with finite bounds
  add("gaol::pow(x, [0.5,1.5]), x > 0", pos, [&](const interval& x) { return gaol::pow(x, y); });
  add("gaol::pow(x, [-1.5,-0.5]), x > 0", pos, [&](const interval& x) { return gaol::pow(x, ynn); });
  add("gaol::pow(x, 0.75), x > 0", pos, [&](const interval& x) { return gaol::pow(x, 0.75); });
  add("gaol::pow(x, [0.5,1.5]), x across 0", mixed, [&](const interval& x) { return gaol::pow(x, y); });
  add("gaol_ieee1788::pow(x, [0.5,1.5]), x > 0", pos,
      [&](const interval& x) { return gaol_ieee1788::pow(x, y); });
  add("gaol_ieee1788::pow(x, [0.5,1.5]), x across 0", mixed,
      [&](const interval& x) { return gaol_ieee1788::pow(x, y); });
  // The integer powers
  add("gaol_ieee1788::pow(x, [3]), x > 0", pos, [&](const interval& x) { return gaol_ieee1788::pow(x, q); });
  add("gaol_ieee1788::pow(x, [3]), x across 0", mixed,
      [&](const interval& x) { return gaol_ieee1788::pow(x, q); });
  add("gaol::pow(x, [3]), x > 0", pos, [&](const interval& x) { return gaol::pow(x, q); });
  add("gaol::pow(x, 3), x > 0 (pown alone)", pos, [&](const interval& x) { return gaol::pow(x, 3); });
  // An integer exponent beyond the ints
  add("gaol_ieee1788::pow(x, [1e10]), x > 0", pos, [&](const interval& x) { return gaol_ieee1788::pow(x, big); });
  // exp(y log x), which B.2 replaces by the corners
  add("gaol_ieee1788::pow(x, [0.5,1.5]), x = [a, +oo]", unbounded,
      [&](const interval& x) { return gaol_ieee1788::pow(x, y); });
  add("gaol_ieee1788::pow(x, [-1.5,-0.5]), x = [0, b]", from0,
      [&](const interval& x) { return gaol_ieee1788::pow(x, ynn); });
  add("gaol_ieee1788::pow(x, [1,+oo]), x > 0", pos,
      [&](const interval& x) { return gaol_ieee1788::pow(x, yinf); });
  for (const Row& r : rows) {
    std::printf("%-48s %8.2f ns\n", r.name, r.ns);
  }
  if (sink == 12345.678) {
    std::printf("%g\n", sink);
  }
  gaol::cleanup();
  return 0;
}
