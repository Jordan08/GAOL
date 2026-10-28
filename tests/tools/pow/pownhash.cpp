/*-*-C++-*----------------------------------------------------------------------
 * gaol -- NOT Just Another Interval Library
 *------------------------------------------------------------------------------
 * Tools of GAOL v5: a hash of the bounds of about four million gaol::pow(x, n)
 * for an integer n, to tell at once whether a change of the integer power
 * changed a bound (point PG.7 of TODO.md; tests/tools/pow).
 *
 *   pownhash [d] [calls]
 *
 * The boxes are the pairs of special values (zeros of both signs, +-1, 0.5, 2,
 * the least and the greatest numbers, the infinities...), then boxes drawn
 * from a generator of its own (scales from 2^-30 to 2^30, positive, negative
 * and across 0, close to 1) until `calls` results (4 000 000 by default); the
 * exponents are the integers from -70 to 70 but -1, 0 and 1, and larger ones
 * up to 2^31 - 1. The program prints the number of calls and the FNV-1a hash
 * of the bits of the bounds, the empty set being a mark of its own. With `d`
 * it prints each box and its bounds in hexadecimal as well (the empty set as
 * `empty`), one line each, to compare two builds with diff when their hashes
 * differ (the sign of a zero bound shows there; diffpow.py tells it from the
 * rest, on its own boxes).
 *
 * Two builds that give the same hash give the same bounds, the sign of a zero
 * included. The hash depends on the rounding direction the program computes
 * its boxes in, the draws being doubles computed in it (GAOL_PRESERVE_ROUNDING
 * leaves the direction of the program, to nearest, and the others leave
 * upward): compare hashes of builds with the same option. It is compiled with
 * build.py, like the other programs of the directory, in C++17 (hexadecimal
 * floating literals).
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

#include <cmath>
#include <cstddef>
#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <limits>
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
}

int main(int argc, char** argv)
{
  gaol::init();
  bool dump = false;
  long calls_wanted = 4000000;
  for (int i = 1; i < argc; ++i) {
    if (argv[i][0] == 'd') {
      dump = true;
    } else {
      calls_wanted = std::atol(argv[i]);
    }
  }
  // xorshift64: the draws are the program's, whatever the library
  std::uint64_t s = 0x9E3779B97F4A7C15ULL;
  const auto rnd = [&s] {
    s ^= s << 13;
    s ^= s >> 7;
    s ^= s << 17;
    return static_cast<double>(s >> 11)/9007199254740992.0;
  };
  const double oo = std::numeric_limits<double>::infinity();
  const std::vector<double> special = {
    0.0, -0.0, 1.0, -1.0, 0.5, -0.5, 2.0, -2.0, 1e-300, -1e-300, 5e-324, -5e-324, 1e300, -1e300,
    0x1p500, -0x1p500, 0x1p-500, -0x1p-500, 1.0000000000000002, 0.9999999999999999,
    1.7976931348623157e308, -1.7976931348623157e308, oo, -oo };
  std::vector<int> exponents;
  for (int n = 2; n <= 70; ++n) {
    exponents.push_back(n);
    exponents.push_back(-n);
  }
  for (int n : { 100, 127, 128, 255, 1000, 4097, 100000, 1 << 20, 1 << 30, 2147483647,
                 -100, -127, -128, -1000, -4097, -100000, -(1 << 20), -(1 << 30), -2147483647 }) {
    exponents.push_back(n);
  }
  long calls = 0;
  const auto power = [&](const interval& x, int n) {
    const interval z = gaol::pow(x, n);
    ++calls;
    if (z.is_empty()) {
      mix(1.0);
      mix(0.0);
      hash ^= 0x55;
      if (dump) {
        std::printf("%d %a %a empty\n", n, x.left(), x.right());
      }
      return;
    }
    mix(z.left());
    mix(z.right());
    if (dump) {
      std::printf("%d %a %a %a %a\n", n, x.left(), x.right(), z.left(), z.right());
    }
  };
  for (double a : special) {
    for (double b : special) {
      if (a > b) {
        continue;
      }
      for (int n : exponents) {
        power(interval(a, b), n);
      }
    }
  }
  power(interval::emptyset(), 3);
  power(interval::universe(), 3);
  power(interval::universe(), 4);
  power(interval::universe(), -3);
  for (long i = 0; calls < calls_wanted; ++i) {
    const double scale = std::ldexp(1.0, static_cast<int>(rnd()*60.0) - 30);
    double a, b;
    switch (i % 6) {
    case 0: a = scale*(0.1 + 4.0*rnd()); b = a + scale*2.0*rnd(); break;
    case 1: b = -scale*(0.1 + 4.0*rnd()); a = b - scale*2.0*rnd(); break;
    case 2: a = scale*(-2.0 + 4.0*rnd()); b = a + scale*3.0*rnd(); break;
    case 3: a = 1.0 + (rnd() - 0.5)*1e-3; b = a + rnd()*1e-3; break;
    case 4: a = b = scale*(rnd() - 0.5)*4.0; break;
    default: a = 1.0 + (rnd() - 0.5)*1e-9; b = a + rnd()*1e-9; break;
    }
    const int n = exponents[static_cast<std::size_t>(rnd()*static_cast<double>(exponents.size()))];
    power(interval(a, b), n);
  }
  std::printf("calls %ld hash %016llx\n", calls, static_cast<unsigned long long>(hash));
  gaol::cleanup();
  return 0;
}
