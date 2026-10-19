/*-*-C++-*----------------------------------------------------------------------
 * gaol -- NOT Just Another Interval Library
 *------------------------------------------------------------------------------
 * Tools of GAOL v5: the differential test of the arithmetic (tests/tools/arith).
 *
 * Prints, one line per operation, the bounds of the result in hexadecimal (the
 * signs of the zeros included), "E" for the empty set, and the exception flags
 * it raised (f0: none, f1: invalid operation, f2: division by zero, f3: both):
 *
 *   x + y, x - y, x * y, x / y, x % y, x &= y and x |= y on every pair of boxes,
 *   hausdorff(x, y), div_rel(x, y, [-oo, +oo]) and div_rel(x, y, [-3, 5]) on
 *   one pair in 11;
 *   sqr(x), sqrt(x), inverse(x), -x, abs(x), x.mid(), x.midpoint(), exp(x);
 *   x += d, x -= d, x *= d, x /= d, x %= d, for d in the grid of bounds below
 *   and for a NaN d.
 *
 * The boxes are the intervals [a, b] with a <= b of a grid of 23 bounds (the
 * infinities, +-DBL_MAX, +-1e300, -2.5, +-1, +-DBL_MIN, +-the smallest
 * subnormal, +-0, 0.1, 3 and six random numbers), the empty set, [-oo, +oo]
 * and interval::zero(). Each line is computed with the rounding direction of
 * the call set to nearest, upward, downward, then toward zero (the key of a
 * line starts with the direction, 0 to 3), the boxes being built once with the
 * direction to nearest, so that they are the same in every build.
 *
 * The same program linked with two builds of GAOL (the NEON and the FPU
 * intervals, say) prints the same lines where the two compute the same bounds:
 * diffarith.py builds it, runs it and compares the outputs.
 *
 *   diffarith [direction...] > output.txt     (all four by default)
 *------------------------------------------------------------------------------
 * gaol is a software distributed WITHOUT ANY WARRANTY. Read the associated
 * COPYING file for information.
 *------------------------------------------------------------------------------
 * Copyright (c) 2026 ENSTA, France
 *
 * Created 2026-10-19 by Jordan NININ
 *----------------------------------------------------------------------------*/

#include "gaol/gaol.h"

#include <cfenv>
#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <limits>
#include <random>
#include <vector>

using gaol::interval;

namespace
{
  int direction = 0;

  int flags()
  {
    const int raised = std::fetestexcept(FE_INVALID | FE_DIVBYZERO);
    return ((raised & FE_INVALID) ? 1 : 0) | ((raised & FE_DIVBYZERO) ? 2 : 0);
  }

  std::uint64_t bits(double x)
  {
    std::uint64_t b;
    std::memcpy(&b, &x, sizeof b);
    return b;
  }

  void print(const char *op, std::size_t i, std::size_t j, const interval &r)
  {
    if (r.is_empty()) {
      std::printf("%d %s %zu %zu E f%d\n", direction, op, i, j, flags());
    } else {
      std::printf("%d %s %zu %zu %016llx %016llx f%d\n", direction, op, i, j,
                  static_cast<unsigned long long>(bits(r.left())),
                  static_cast<unsigned long long>(bits(r.right())), flags());
    }
  }

  void print(const char *op, std::size_t i, std::size_t j, double r)
  {
    std::printf("%d %s %zu %zu %016llx f%d\n", direction, op, i, j, static_cast<unsigned long long>(bits(r)), flags());
  }

  void clear() { std::feclearexcept(FE_ALL_EXCEPT); }
}

int main(int argc, char **argv)
{
  gaol::init();
  const int modes[4] = { FE_TONEAREST, FE_UPWARD, FE_DOWNWARD, FE_TOWARDZERO };
  std::vector<int> directions;
  for (int a = 1; a < argc; ++a) {
    directions.push_back(std::atoi(argv[a]) & 3);
  }
  if (directions.empty()) {
    for (int d = 0; d < 4; ++d) {
      directions.push_back(d);
    }
  }

  const double inf = std::numeric_limits<double>::infinity();
  std::vector<double> S = { -inf, -std::numeric_limits<double>::max(), -1e300, -2.5, -1.0, -std::numeric_limits<double>::min(),
                            -std::numeric_limits<double>::denorm_min(), -0.0, 0.0, std::numeric_limits<double>::denorm_min(),
                            std::numeric_limits<double>::min(), 0.1, 1.0, 3.0, 1e300, std::numeric_limits<double>::max(), inf };
  std::mt19937_64 generator(12345);
  std::uniform_real_distribution<double> uniform(-10.0, 10.0);
  for (int i = 0; i < 6; ++i) {
    S.push_back(uniform(generator));
  }

  std::fesetround(FE_TONEAREST);
  std::vector<interval> V;
  for (std::size_t i = 0; i < S.size(); ++i) {
    for (std::size_t j = 0; j < S.size(); ++j) {
      if (S[i] <= S[j] && S[i] != inf && S[j] != -inf) {
        V.push_back(interval(S[i], S[j]));
      }
    }
  }
  V.push_back(interval::emptyset());
  V.push_back(interval::universe());
  V.push_back(interval::zero());
  const double nan = std::numeric_limits<double>::quiet_NaN();
  std::printf("# diffarith: %zu boxes, %zu bounds\n", V.size(), S.size());

  for (int dir : directions) {
    direction = dir;
    std::fesetround(modes[dir]);
    for (std::size_t i = 0; i < V.size(); ++i) {
      clear(); print("sqr", i, 0, gaol::sqr(V[i]));
      clear(); print("sqrt", i, 0, gaol::sqrt(V[i]));
      clear(); print("inverse", i, 0, gaol::inverse(V[i]));
      clear(); print("neg", i, 0, -V[i]);
      clear(); print("abs", i, 0, gaol::abs(V[i]));
      clear(); print("mid", i, 0, V[i].mid());
      clear(); print("midpoint", i, 0, V[i].midpoint());
      clear(); print("exp", i, 0, gaol::exp(V[i]));
      for (std::size_t k = 0; k <= S.size(); ++k) {
        const double d = (k < S.size()) ? S[k] : nan;
        clear(); { interval x = V[i]; x += d; print("+=d", i, k, x); }
        clear(); { interval x = V[i]; x -= d; print("-=d", i, k, x); }
        clear(); { interval x = V[i]; x *= d; print("*=d", i, k, x); }
        clear(); { interval x = V[i]; x /= d; print("/=d", i, k, x); }
        clear(); { interval x = V[i]; x %= d; print("%=d", i, k, x); }
      }
      // Every pair of boxes for the six operations of the table; one pair in
      // eleven for hausdorff() and div_rel(), which are rarer
      for (std::size_t j = 0; j < V.size(); ++j) {
        clear(); print("+", i, j, V[i] + V[j]);
        clear(); print("-", i, j, V[i] - V[j]);
        clear(); print("*", i, j, V[i] * V[j]);
        clear(); print("/", i, j, V[i] / V[j]);
        clear(); print("%", i, j, V[i] % V[j]);
        clear(); { interval x = V[i]; x &= V[j]; print("&=", i, j, x); }
        clear(); { interval x = V[i]; x |= V[j]; print("|=", i, j, x); }
        if ((i * 31 + j) % 11 == 0) {
          clear(); print("hausdorff", i, j, gaol::hausdorff(V[i], V[j]));
          clear(); print("div_rel", i, j, gaol::div_rel(V[i], V[j], interval::universe()));
          clear(); print("div_rel2", i, j, gaol::div_rel(V[i], V[j], interval(-3.0, 5.0)));
        }
      }
    }
  }
  std::fesetround(FE_TONEAREST);
  gaol::cleanup();
  return 0;
}
