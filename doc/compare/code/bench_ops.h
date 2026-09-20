// Copyright (c) 2026 ENSTA, France
//
// Created 2026-09-20 by Jordan NININ
//
// The operations timed by the benchmark, written once for every C++ library.
// T gives the library: its interval type T::I, and
//   T::make(lo, hi)     the interval [lo, hi]
//   T::decimal(s)       the tightest interval enclosing the decimal number s
//   T::sqr, T::sqrt, T::exp, T::log, T::sin, T::cos
//   T::pown(x, n)       x^n for an integer n
//   T::pow(x, y)        x^y for an interval y
//   T::mid, T::wid      the midpoint and the width, for the sums printed
// The same operations are in bench_sun.f90.
#ifndef BENCH_OPS_H
#define BENCH_OPS_H

#include "bench_common.h"

template<class T>
typename T::I shekel5(const typename T::I& x1, const typename T::I& x2, const typename T::I& x3,
                      const typename T::I& x4, const typename T::I a[5][4], const typename T::I c[5],
                      const typename T::I& zero, const typename T::I& one)
{
  typename T::I f = zero;
  for (int i = 0; i < 5; ++i) {
    f = f - one / (T::sqr(x1 - a[i][0]) + T::sqr(x2 - a[i][1]) + T::sqr(x3 - a[i][2]) + T::sqr(x4 - a[i][3]) + c[i]);
  }
  return f;
}

template<class T>
void run_all(const Data& d, int repeats, const std::string& only)
{
  typedef typename T::I I;
  const std::size_t n = static_cast<std::size_t>(d.n);
  std::vector<I> x[8];
  for (int k = 0; k < 8; ++k) {
    x[k].reserve(n);
    for (std::size_t i = 0; i < n; ++i) {
      x[k].push_back(T::make(d.lo[k][i], d.hi[k][i]));
    }
  }
  const std::vector<I>& a = x[DATA_A], & b = x[DATA_B], & p = x[DATA_P], & e = x[DATA_E];
  std::vector<I> r(n, T::make(0.0, 0.0));

  // Shekel 5: -sum_{i=1}^{5} 1/(sum_{j=1}^{4} (x_j - a_ij)^2 + c_i), on [0, 10]^4
  const double sa[5][4] = { { 4, 4, 4, 4 }, { 1, 1, 1, 1 }, { 8, 8, 8, 8 }, { 6, 6, 6, 6 }, { 3, 7, 3, 7 } };
  const char *const sc[5] = { "0.1", "0.2", "0.2", "0.4", "0.4" };
  I sha[5][4], shc[5];
  for (int i = 0; i < 5; ++i) {
    for (int j = 0; j < 4; ++j) {
      sha[i][j] = T::make(sa[i][j], sa[i][j]);
    }
    shc[i] = T::decimal(sc[i]);
  }
  const I zero = T::make(0.0, 0.0), one = T::make(1.0, 1.0);

  // OP(name, expression of a[i], b[i], p[i], e[i])
#define OP(NAME, EXPR)                                                          \
  if (selected(only, NAME)) {                                                   \
    bench(T::name, NAME, repeats, [&]() {                                       \
      for (std::size_t i = 0; i < n; ++i) {                                     \
        r[i] = EXPR;                                                            \
      }                                                                         \
    }, r, [](const I& v) { return T::mid(v); }, [](const I& v) { return T::wid(v); }); \
  }

  OP("add", a[i] + b[i])
  OP("sub", a[i] - b[i])
  OP("mul", a[i] * b[i])
  OP("div", a[i] / p[i])
  OP("sqr", T::sqr(a[i]))
  OP("sqrt", T::sqrt(p[i]))
  OP("exp", T::exp(a[i]))
  OP("log", T::log(p[i]))
  OP("sin", T::sin(a[i]))
  OP("cos", T::cos(a[i]))
  OP("pow_int", T::pown(a[i], 3))
  OP("pow_real", T::pow(p[i], e[i]))
  // Combinations of operations on one line
  OP("line_arith", (a[i] + b[i]) * (a[i] - b[i]) / p[i])
  OP("line_trig", T::sin(a[i]) * T::cos(b[i]) + T::sqr(a[i]))
  OP("line_pow", T::sqrt(p[i]) * T::pown(a[i], 3) - T::exp(b[i] / p[i]))
  OP("shekel5", shekel5<T>(x[DATA_S1][i], x[DATA_S2][i], x[DATA_S3][i], x[DATA_S4][i], sha, shc, zero, one))
  // Five lines of computations
  OP("block5", [&]() {
      const I t1 = a[i] * b[i] + p[i];
      const I t2 = T::sin(t1) * T::cos(b[i]);
      const I t3 = T::sqr(a[i]) + t2 / p[i];
      const I t4 = T::exp(t2) - T::pown(b[i], 3);
      return t3 * t4 + T::sqrt(p[i]);
    }())
#undef OP
}

// The arguments of the programs: the data file, the number of repeats, and
// the operations to run (all of them when absent)
template<class T>
int bench_main(int argc, char *argv[])
{
  if (argc < 3) {
    std::fprintf(stderr, "usage: %s DATA REPEATS [OPERATIONS]\n", argv[0]);
    return 1;
  }
  const Data d = load(argv[1]);
  run_all<T>(d, std::atoi(argv[2]), argc > 3 ? argv[3] : "");
  return 0;
}

#endif // BENCH_OPS_H
