/*-*-C++-*----------------------------------------------------------------------
 * gaol -- NOT Just Another Interval Library
 *------------------------------------------------------------------------------
 * Tools of GAOL v5: the edges of the range of the packed kernel of the integer
 * power (point PG.7 of TODO.md; tests/tools/pow).
 *
 *   pownedge [gaol/core/gaol_interval_core.cpp]
 *
 * The packed kernel of gaol::pow(x, n) (pown_large_inline() of the NEON
 * intervals, the SSE2 ones with FMA...) takes a base c = x or 1/x within
 * [pown_least[L], pown_greatest[L]] in absolute value, L being the number of
 * bits of |n| (the tables of gaol/core/gaol_interval_core.cpp), and c = 0 for
 * n > 0; any other base takes the corners (pown_by_corners()). This program
 * calls gaol::pow on the edges of every L from 2 to 32, with the exponents
 * +-2^(L-1) and +-(2^L - 1) (-2^31 for L = 32) and the integers from 2 to 140
 * and from -140 to -2, on the edges and on their three neighbours on each side
 * (nextafter), with both signs, as points and as the boxes [x, next(x)],
 * [prev(x), x], [-|x|, |x|], [0, |x|] and [-|x|, 0], and prints the number of
 * calls and the FNV-1a hash of the bits of the bounds, the empty set being a
 * mark of its own. It reads the two tables from the source file given
 * (gaol/core/gaol_interval_core.cpp by default), in the rounding direction to
 * nearest whatever the program's, so that they are the library's, and exits
 * with 1 if it does not find them.
 *
 * Two builds that give the same hash give the same bounds, the sign of a zero
 * included: run before and after a change of the kernel. A base at an edge
 * that takes the corners instead of the kernel gives the same bounds but for a
 * zero lower bound, which is -0 with the kernel and +0 with the corners: the
 * hash sees it, the tests and diffpow.py do not compare the sign of a zero
 * (the mutants neon-range-least-strict and neon-range-greatest-strict of
 * mutate.py). The boxes are made of exact operations (nextafter), whatever the
 * rounding direction of the program, so that the hash is the same with and
 * without GAOL_PRESERVE_ROUNDING. It is compiled with build.py, like the other
 * programs of the directory, in C++14 or later.
 *
 * Copyright (c) 2026 ENSTA, France
 *
 * Created 2026-10-28 by Jordan NININ (from a program of the study of the weak
 * gain of the NEON kernel of pown, point PG.7)
 *------------------------------------------------------------------------------
 * gaol is a software distributed WITHOUT ANY WARRANTY. Read the associated
 * COPYING file for information.
 *----------------------------------------------------------------------------*/

#include "gaol/gaol.h"

#include <cfenv>
#include <climits>
#include <cmath>
#include <cstddef>
#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <fstream>
#include <iterator>
#include <limits>
#include <string>
#include <vector>

using gaol::interval;

namespace
{
  std::uint64_t hash = 1469598103934665603ULL;

  void mix(double d)
  {
    std::uint64_t bits;
    std::memcpy(&bits, &d, 8);
    for (int i = 0; i < 8; ++i) {
      hash ^= (bits >> (8 * i)) & 0xff;
      hash *= 1099511628211ULL;
    }
  }

  // The 33 numbers of "static const double NAME[33] = { ... };" in text, read
  // to nearest (gaol::init() leaves the direction upward, where strtod() reads
  // the decimal numbers of the tables as the double above the compiler's)
  bool table(const std::string& text, const std::string& name, std::vector<double>& values)
  {
    const std::size_t at = text.find("static const double " + name + "[33] = {");
    if (at == std::string::npos) {
      return false;
    }
    const char *p = text.c_str() + text.find('{', at) + 1;
    values.clear();
    const int direction = std::fegetround();
    std::fesetround(FE_TONEAREST);
    bool ok = true;
    while (ok && values.size() < 33) {
      char *end;
      const double d = std::strtod(p, &end);
      if (end == p) {
        ok = false;
        break;
      }
      values.push_back(d);
      p = end;
      while (*p == ',' || *p == ' ' || *p == '\n' || *p == '\t' || *p == '\r') {
        ++p;
      }
    }
    std::fesetround(direction);
    return ok && *p == '}';
  }
}

int main(int argc, char** argv)
{
  const std::string path = argc > 1 ? argv[1] : "gaol/core/gaol_interval_core.cpp";
  std::ifstream in(path);
  const std::string text((std::istreambuf_iterator<char>(in)), std::istreambuf_iterator<char>());
  gaol::init();
  std::vector<double> least, greatest;
  if (!table(text, "pown_least", least) || !table(text, "pown_greatest", greatest)) {
    std::fprintf(stderr, "pownedge: no table pown_least[33] and pown_greatest[33] in %s\n", path.c_str());
    return 1;
  }
  const double oo = std::numeric_limits<double>::infinity();
  std::vector<double> edges = { 0.0, -0.0, 5e-324, 1e-300 };
  for (int L = 2; L <= 32; ++L) {
    for (double e : { least[static_cast<std::size_t>(L)], greatest[static_cast<std::size_t>(L)] }) {
      double up = e, down = e;
      for (int k = 0; k < 4; ++k) {
        edges.push_back(up);
        edges.push_back(down);
        up = std::nextafter(up, oo);
        down = std::nextafter(down, -oo);
      }
    }
  }
  std::vector<int> exponents;
  for (int n = 2; n <= 140; ++n) {
    exponents.push_back(n);
    exponents.push_back(-n);
  }
  // Every length L: 2^(L-1) and 2^L - 1, and the same negative; 2^31 is -INT_MIN
  for (int L = 2; L <= 31; ++L) {
    const int low = 1 << (L - 1), high = static_cast<int>((1u << L) - 1u);
    for (int n : { low, high, -low, -high }) {
      exponents.push_back(n);
    }
  }
  exponents.push_back(INT_MIN);
  long calls = 0;
  const auto power = [&calls](const interval& x, int n) {
    const interval z = gaol::pow(x, n);
    ++calls;
    if (z.is_empty()) {
      mix(1.0);
      hash ^= 0x55;
      return;
    }
    mix(z.left());
    mix(z.right());
  };
  for (double a : edges) {
    for (double sign : { 1.0, -1.0 }) {
      const double x = sign*a;
      const double magnitude = std::fabs(x);
      for (int n : exponents) {
        power(interval(x, x), n);
        power(interval(x, std::nextafter(x, oo)), n);
        power(interval(std::nextafter(x, -oo), x), n);
        power(interval(-magnitude, magnitude), n);
        power(interval(0.0, magnitude), n);
        power(interval(-magnitude, 0.0), n);
      }
    }
  }
  std::printf("calls %ld hash %016llx\n", calls, static_cast<unsigned long long>(hash));
  gaol::cleanup();
  return 0;
}
