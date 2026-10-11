/*-*-C++-*------------------------------------------------------------------
 * gaol -- NOT Just Another Interval Library
 *--------------------------------------------------------------------------
 * Performance of GAOL v5, which the continuous integration prints.
 *
 * The time per operation of GAOL's arithmetic and elementary functions, of
 * the constructor, the intersection and some relations, and of the same
 * operations on doubles, in nanoseconds: the median of 5 measures, each made
 * over 1024 operands, as many times as it takes to last 20 ms. The table is
 * written in Markdown. This is not a test: it always succeeds.
 *
 * The constructor, x &= y and the relations compare bounds with the quiet
 * comparisons of <cmath> (std::islessequal(), std::isunordered()...), one
 * instruction each with GCC and Clang on x86 (GAOL v5): their rows show what
 * these comparisons cost with the other compilers of the continuous
 * integration, Visual C++ among them. They compare the bounds as bounds,
 * as integers where two compare equal (gaol_port.h), which the modes that
 * flush the subnormals to zero do not change (GAOL v5, point Q of TODO.md).
 *
 * After the table, the time of the check each operation makes, 1 + (2^-1060
 * + 0) == 1, which shows the rounding direction and the modes that flush the
 * subnormals to zero (gaol/gaol_fpu.h), and of the same check made of 2^-60,
 * a normal double: a processor that takes a microcode assist for a subnormal
 * operand or result shows it there, each operation of GAOL paying it (GAOL
 * v5, point Q of TODO.md).
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

  // The doubles of the two checks, read from volatile memory, as GAOL reads
  // its own: 2^-1060, the subnormal of gaol/gaol_fpu.h, and 2^-60
  const volatile double check_subnormal = 8.0947715414629834e-320;
  const volatile double check_normal = 8.67361737988403547205962240695953369140625e-19;

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
  // The intervals overlapping X drawn apart, so that X and Y are the operands
  // the table always had
  std::mt19937_64 overlap_generator(43);
  std::uniform_real_distribution<double> uniform(0.5, 4.0);
  std::vector<double> a(n), b(n), c(n);
  std::vector<interval> X(n), Y(n), W(n);
  for (std::size_t i = 0; i < n; ++i) {
    a[i] = uniform(generator);
    b[i] = uniform(generator);
    c[i] = a[i] + 1e-3*uniform(generator);
    X[i] = interval(a[i], c[i]);
    Y[i] = interval(b[i], b[i] + 1e-3*uniform(generator));
    // An interval overlapping X[i], for a nonempty intersection
    W[i] = interval(a[i] - 1e-3*uniform(overlap_generator), a[i] + 5e-4*uniform(overlap_generator));
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
    { "interval(a, b)", nanoseconds_per_operation([&](std::size_t i) { const interval z(a[i], c[i]); sink = z.left(); sink = z.right(); }, n),
               nanoseconds_per_operation([&](std::size_t i) { sink = a[i]; sink = c[i]; }, n) },
    { "floor(x)", nanoseconds_per_operation([&](std::size_t i) { const interval z = floor(X[i]); sink = z.left(); sink = z.right(); }, n),
               nanoseconds_per_operation([&](std::size_t i) { sink = std::floor(a[i]); }, n) },
    { "x &= y", nanoseconds_per_operation([&](std::size_t i) { interval z = X[i]; z &= W[i]; sink = z.left(); sink = z.right(); }, n),
               nanoseconds_per_operation([&](std::size_t i) { sink = std::max(a[i], b[i]); }, n) },
    { "x | y", nanoseconds_per_operation([&](std::size_t i) { const interval z = X[i] | W[i]; sink = z.left(); sink = z.right(); }, n),
               nanoseconds_per_operation([&](std::size_t i) { sink = std::max(a[i], b[i]); }, n) },
    { "max(x, y)", nanoseconds_per_operation([&](std::size_t i) { const interval z = max(X[i], Y[i]); sink = z.left(); sink = z.right(); }, n),
               nanoseconds_per_operation([&](std::size_t i) { sink = std::max(a[i], b[i]); }, n) },
    { "x.midpoint()", nanoseconds_per_operation([&](std::size_t i) { sink = X[i].midpoint(); }, n),
               nanoseconds_per_operation([&](std::size_t i) { sink = 0.5*(a[i] + c[i]); }, n) },
    { "x <= y", nanoseconds_per_operation([&](std::size_t i) { sink = (X[i] <= Y[i]) ? 1.0 : 0.0; }, n),
               nanoseconds_per_operation([&](std::size_t i) { sink = (a[i] <= b[i]) ? 1.0 : 0.0; }, n) },
    { "x.set_contains(y)", nanoseconds_per_operation([&](std::size_t i) { sink = W[i].set_contains(X[i]) ? 1.0 : 0.0; }, n),
               nanoseconds_per_operation([&](std::size_t i) { sink = (a[i] <= b[i]) ? 1.0 : 0.0; }, n) },
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

  const double check_subnormal_ns = nanoseconds_per_operation([&](std::size_t) {
      sink = (1.0 + (check_subnormal + 0.0) == 1.0) ? 1.0 : 0.0; }, n);
  const double check_normal_ns = nanoseconds_per_operation([&](std::size_t) {
      sink = (1.0 + (check_normal + 0.0) == 1.0) ? 1.0 : 0.0; }, n);
  std::printf("\nThe check of each operation, 1 + (2^-1060 + 0) == 1: %.2f ns; the same with 2^-60: %.2f ns\n",
              check_subnormal_ns, check_normal_ns);

  gaol::cleanup();
  return 0;
}
