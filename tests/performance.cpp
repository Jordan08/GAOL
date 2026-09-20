/*-*-C++-*------------------------------------------------------------------
 * gaol -- NOT Just Another Interval Library
 *--------------------------------------------------------------------------
 * Performance of this fork of GAOL, which the continuous integration prints.
 *
 * The time per operation of GAOL's arithmetic and elementary functions, and
 * of the same operations on doubles, in nanoseconds: the median of 5 measures,
 * each made over 1024 operands, as many times as it takes to last 20 ms. The
 * table is written in Markdown. This is not a test: it always succeeds.
 *
 * Copyright (c) 2026 ENSTA, France
 *
 * Created 2026-09-20 by Jordan NININ
 *--------------------------------------------------------------------------
 * gaol is a software distributed WITHOUT ANY WARRANTY. Read the associated
 * COPYING file for information.
 *--------------------------------------------------------------------------*/

#include "gaol/gaol.h"

#include <algorithm>
#include <chrono>
#include <cmath>
#include <cstddef>
#include <cstdio>
#include <random>
#include <string>
#include <vector>

using namespace gaol;

namespace
{
  volatile double sink;

  template<class F>
  void repeat(const F& operation, std::size_t rounds, std::size_t n)
  {
    for (std::size_t r = 0; r < rounds; ++r) {
      for (std::size_t i = 0; i < n; ++i) {
        operation(i);
      }
    }
  }

  template<class F>
  double nanoseconds_per_operation(const F& operation, std::size_t n)
  {
    typedef std::chrono::steady_clock clock;
    std::size_t rounds = 1;
    for (;;) {
      const clock::time_point start = clock::now();
      repeat(operation, rounds, n);
      const double seconds = std::chrono::duration<double>(clock::now() - start).count();
      if (seconds >= 0.02 || rounds >= (static_cast<std::size_t>(1) << 20)) {
        break;
      }
      rounds *= 2;
    }
    std::vector<double> measures;
    for (int m = 0; m < 5; ++m) {
      const clock::time_point start = clock::now();
      repeat(operation, rounds, n);
      measures.push_back(std::chrono::duration<double, std::nano>(clock::now() - start).count() / static_cast<double>(rounds * n));
    }
    std::sort(measures.begin(), measures.end());
    return measures[2];
  }

  std::string platform()
  {
    std::string p =
#if defined(__x86_64__) || defined(_M_X64)
      "x86-64";
#elif defined(__i386__) || defined(_M_IX86)
      "x86";
#elif defined(__aarch64__) || defined(_M_ARM64)
      "arm64";
#elif defined(__arm__) || defined(_M_ARM)
      "arm";
#elif defined(__s390x__)
      "s390x";
#elif defined(__powerpc64__)
      "ppc64";
#elif defined(__riscv)
      "riscv";
#else
      "unknown processor";
#endif
    p +=
#if defined(_WIN32)
      ", Windows";
#elif defined(__APPLE__)
      ", macOS";
#elif defined(__linux__)
      ", Linux";
#else
      "";
#endif
#if defined(__clang__)
    p += ", Clang " __clang_version__;
#elif defined(__GNUC__)
    p += ", GCC " __VERSION__;
#elif defined(_MSC_VER)
    p += ", Visual C++ " + std::to_string(_MSC_FULL_VER);
#endif
    return p;
  }
}

int main()
{
  gaol::init();

  const std::size_t n = 1024;
  std::mt19937_64 generator(42);
  std::uniform_real_distribution<double> uniform(0.5, 4.0);
  std::vector<double> a(n), b(n);
  std::vector<interval> X(n), Y(n);
  for (std::size_t i = 0; i < n; ++i) {
    a[i] = uniform(generator);
    b[i] = uniform(generator);
    X[i] = interval(a[i], a[i] + 1e-3*uniform(generator));
    Y[i] = interval(b[i], b[i] + 1e-3*uniform(generator));
  }

  struct Row
  {
    const char *operation;
    double interval_ns, double_ns;
  };
  const Row rows[] = {
    { "x + y", nanoseconds_per_operation([&](std::size_t i) { const interval z = X[i] + Y[i]; sink = z.left(); sink = z.right(); }, n),
               nanoseconds_per_operation([&](std::size_t i) { sink = a[i] + b[i]; }, n) },
    { "x - y", nanoseconds_per_operation([&](std::size_t i) { const interval z = X[i] - Y[i]; sink = z.left(); sink = z.right(); }, n),
               nanoseconds_per_operation([&](std::size_t i) { sink = a[i] - b[i]; }, n) },
    { "x * y", nanoseconds_per_operation([&](std::size_t i) { const interval z = X[i] * Y[i]; sink = z.left(); sink = z.right(); }, n),
               nanoseconds_per_operation([&](std::size_t i) { sink = a[i] * b[i]; }, n) },
    { "x / y", nanoseconds_per_operation([&](std::size_t i) { const interval z = X[i] / Y[i]; sink = z.left(); sink = z.right(); }, n),
               nanoseconds_per_operation([&](std::size_t i) { sink = a[i] / b[i]; }, n) },
    { "sqr(x)", nanoseconds_per_operation([&](std::size_t i) { const interval z = sqr(X[i]); sink = z.left(); sink = z.right(); }, n),
               nanoseconds_per_operation([&](std::size_t i) { sink = a[i] * a[i]; }, n) },
    { "pow(x, 3)", nanoseconds_per_operation([&](std::size_t i) { const interval z = pow(X[i], 3); sink = z.left(); sink = z.right(); }, n),
               nanoseconds_per_operation([&](std::size_t i) { sink = a[i] * a[i] * a[i]; }, n) },
    { "sqrt(x)", nanoseconds_per_operation([&](std::size_t i) { const interval z = sqrt(X[i]); sink = z.left(); sink = z.right(); }, n),
               nanoseconds_per_operation([&](std::size_t i) { sink = std::sqrt(a[i]); }, n) },
    { "exp(x)", nanoseconds_per_operation([&](std::size_t i) { const interval z = exp(X[i]); sink = z.left(); sink = z.right(); }, n),
               nanoseconds_per_operation([&](std::size_t i) { sink = std::exp(a[i]); }, n) },
    { "log(x)", nanoseconds_per_operation([&](std::size_t i) { const interval z = log(X[i]); sink = z.left(); sink = z.right(); }, n),
               nanoseconds_per_operation([&](std::size_t i) { sink = std::log(a[i]); }, n) },
    { "sin(x)", nanoseconds_per_operation([&](std::size_t i) { const interval z = sin(X[i]); sink = z.left(); sink = z.right(); }, n),
               nanoseconds_per_operation([&](std::size_t i) { sink = std::sin(a[i]); }, n) },
    { "cos(x)", nanoseconds_per_operation([&](std::size_t i) { const interval z = cos(X[i]); sink = z.left(); sink = z.right(); }, n),
               nanoseconds_per_operation([&](std::size_t i) { sink = std::cos(a[i]); }, n) },
  };

  std::printf("GAOL performance: %s, rounding direction %s\n\n", platform().c_str(),
#if GAOL_PRESERVE_ROUNDING
              "restored after each operation (GAOL_PRESERVE_ROUNDING)"
#else
              "set upward by each operation when it is not (default)"
#endif
  );
  std::printf("| Operation | Interval (ns) | Double (ns) | Interval / double |\n");
  std::printf("|---|---:|---:|---:|\n");
  for (const Row& row : rows) {
    std::printf("| %s | %.2f | %.2f | %.1f |\n", row.operation, row.interval_ns, row.double_ns, row.interval_ns / row.double_ns);
  }

  gaol::cleanup();
  return 0;
}
