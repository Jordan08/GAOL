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
 * n > 0; any other base takes the corners (pown_by_corners()). The tests of
 * the repository do not tell a comparison at an edge of the range from its
 * strict form: the mutants neon-range-least-strict and
 * neon-range-greatest-strict of mutate.py survive them. This program calls
 * gaol::pow on the edges, which are the bounds of a box or a point, and on
 * their three neighbours on each side (nextafter), with both signs, for the
 * exponents from 2 to 140 and from -140 to -2 and 17 larger ones, as points
 * and as the boxes [x, next(x)], [prev(x), x], [-|x|, |x|], [0, |x|] and
 * [-|x|, 0], and prints the number of calls and the FNV-1a hash of the bits of
 * the bounds, the empty set being a mark of its own. It reads the two tables
 * from the source file given (gaol/core/gaol_interval_core.cpp by default), so
 * that they are the library's, and exits with 1 if it does not find them.
 *
 * Two builds that give the same hash give the same bounds, the sign of a zero
 * included: run before and after a change of the kernel. The boxes are made of
 * exact operations (nextafter), whatever the rounding direction of the program.
 * It is compiled with build.py, like the other programs of the directory.
 *
 * Copyright (c) 2026 ENSTA, France
 *
 * Created 2026-10-29 by Jordan NININ (from the program of the session MACarm4
 * that found the cause of the weak gain of the NEON kernel of pown)
 *------------------------------------------------------------------------------
 * gaol is a software distributed WITHOUT ANY WARRANTY. Read the associated
 * COPYING file for information.
 *----------------------------------------------------------------------------*/

#include "gaol/gaol.h"

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

  // The 33 numbers of "static const double NAME[33] = { ... };" in text
  bool table(const std::string& text, const std::string& name, std::vector<double>& values)
  {
    const std::size_t at = text.find("static const double " + name + "[33] = {");
    if (at == std::string::npos) {
      return false;
    }
    const char *p = text.c_str() + text.find('{', at) + 1;
    values.clear();
    while (values.size() < 33) {
      char *end;
      const double d = std::strtod(p, &end);
      if (end == p) {
        return false;
      }
      values.push_back(d);
      p = end;
      while (*p == ',' || *p == ' ' || *p == '\n' || *p == '\t' || *p == '\r') {
        ++p;
      }
    }
    return *p == '}';
  }
}

int main(int argc, char** argv)
{
  const std::string path = argc > 1 ? argv[1] : "gaol/core/gaol_interval_core.cpp";
  std::ifstream in(path);
  const std::string text((std::istreambuf_iterator<char>(in)), std::istreambuf_iterator<char>());
  std::vector<double> least, greatest;
  if (!table(text, "pown_least", least) || !table(text, "pown_greatest", greatest)) {
    std::fprintf(stderr, "pownedge: no table pown_least[33] and pown_greatest[33] in %s\n", path.c_str());
    return 1;
  }
  gaol::init();
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
  for (int n : { 255, 256, 257, 1023, 1024, 4097, 65535, 65536, 1 << 20, (1 << 30) + 1, 2147483647,
                 -255, -256, -4097, -65536, -(1 << 30), -2147483647 }) {
    exponents.push_back(n);
  }
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
