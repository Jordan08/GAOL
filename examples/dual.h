/*-*-C++-*------------------------------------------------------------------
 * gaol -- NOT Just Another Interval Library
 *--------------------------------------------------------------------------
 * Examples of GAOL v5: forward-mode automatic differentiation over
 * intervals.
 *
 * Dual<N> holds an enclosure of the value of a function of N variables and
 * an enclosure of each partial derivative over the same box. A function
 * written once as a template, template<class T> T f(const T& x), then gives
 * its natural interval extension with T = interval, and its gradient with
 * T = Dual<N>: the enclosures the mean-value form, interval Newton, Krawczyk
 * and the monotonicity tests need. Codac's AnalyticFunction computes the same
 * enclosures, by the same rules, for each of its operators.
 *
 * A derivative written by hand has to enclose the true one, or the proofs
 * built on it are false: a sign error in a Jacobian can make Krawczyk's test
 * "prove" a root in a box that does not hold it. Computing it here removes
 * that risk.
 *
 * Copyright (c) 2026 ENSTA, France
 *
 * Created 2026-09-27 by Jordan NININ
 *--------------------------------------------------------------------------
 * gaol is a software distributed WITHOUT ANY WARRANTY. Read the associated
 * COPYING file for information.
 *--------------------------------------------------------------------------*/

#ifndef GAOL_EXAMPLES_DUAL_H
#define GAOL_EXAMPLES_DUAL_H

#include <array>
#include <cstddef>

#include <gaol/gaol.h>

namespace examples {

using gaol::interval;

template <std::size_t N>
struct Dual {
  interval v;                 // enclosure of the value
  std::array<interval, N> d;  // enclosures of the partial derivatives

  // A constant: its derivatives are zero. Not explicit, so that 2.0 * x and
  // x + interval("0.1") read as they do on intervals.
  Dual(const interval& c = interval(0.0)) : v(c) { d.fill(interval(0.0)); }
  Dual(double c) : Dual(interval(c)) {}

  // The variable number i, over the interval x
  static Dual variable(const interval& x, std::size_t i) {
    Dual r(x);
    r.d[i] = interval(1.0);
    return r;
  }

  friend Dual operator+(const Dual& a, const Dual& b) {
    Dual r(a.v + b.v);
    for (std::size_t i = 0; i < N; ++i) r.d[i] = a.d[i] + b.d[i];
    return r;
  }
  friend Dual operator-(const Dual& a, const Dual& b) {
    Dual r(a.v - b.v);
    for (std::size_t i = 0; i < N; ++i) r.d[i] = a.d[i] - b.d[i];
    return r;
  }
  friend Dual operator-(const Dual& a) {
    Dual r(-a.v);
    for (std::size_t i = 0; i < N; ++i) r.d[i] = -a.d[i];
    return r;
  }
  friend Dual operator*(const Dual& a, const Dual& b) {
    Dual r(a.v * b.v);
    for (std::size_t i = 0; i < N; ++i) r.d[i] = a.d[i] * b.v + a.v * b.d[i];
    return r;
  }
  // (a/b)' = (a' - (a/b) b') / b
  friend Dual operator/(const Dual& a, const Dual& b) {
    Dual r(a.v / b.v);
    for (std::size_t i = 0; i < N; ++i) r.d[i] = (a.d[i] - r.v * b.d[i]) / b.v;
    return r;
  }
  Dual& operator+=(const Dual& b) { return *this = *this + b; }
  Dual& operator-=(const Dual& b) { return *this = *this - b; }
  Dual& operator*=(const Dual& b) { return *this = *this * b; }
  Dual& operator/=(const Dual& b) { return *this = *this / b; }

  // The elementary functions: the value by GAOL, each derivative by the
  // chain rule, both over the whole box. They are found by
  // argument-dependent lookup, as GAOL's are for intervals.
  friend Dual sqr(const Dual& a) {
    Dual r(sqr(a.v));
    for (std::size_t i = 0; i < N; ++i) r.d[i] = 2.0 * a.v * a.d[i];
    return r;
  }
  friend Dual sqrt(const Dual& a) {
    Dual r(sqrt(a.v));
    for (std::size_t i = 0; i < N; ++i) r.d[i] = a.d[i] / (2.0 * r.v);
    return r;
  }
  friend Dual exp(const Dual& a) {
    Dual r(exp(a.v));
    for (std::size_t i = 0; i < N; ++i) r.d[i] = r.v * a.d[i];
    return r;
  }
  friend Dual log(const Dual& a) {
    Dual r(log(a.v));
    for (std::size_t i = 0; i < N; ++i) r.d[i] = a.d[i] / a.v;
    return r;
  }
  friend Dual sin(const Dual& a) {
    Dual r(sin(a.v));
    const interval c = cos(a.v);
    for (std::size_t i = 0; i < N; ++i) r.d[i] = c * a.d[i];
    return r;
  }
  friend Dual cos(const Dual& a) {
    Dual r(cos(a.v));
    const interval s = -sin(a.v);
    for (std::size_t i = 0; i < N; ++i) r.d[i] = s * a.d[i];
    return r;
  }
  friend Dual atan(const Dual& a) {
    Dual r(atan(a.v));
    const interval q = 1.0 + sqr(a.v);
    for (std::size_t i = 0; i < N; ++i) r.d[i] = a.d[i] / q;
    return r;
  }
  // The integer power of GAOL, gaol::pow(x, n), which pow() of namespace
  // gaol is (argument-dependent lookup does not find it: see
  // 14_generic_programming.cpp)
  friend Dual pow(const Dual& a, int n) {
    Dual r(gaol::pow(a.v, n));
    if (n != 0) {
      const interval c = double(n) * gaol::pow(a.v, n - 1);
      for (std::size_t i = 0; i < N; ++i) r.d[i] = c * a.d[i];
    }
    return r;
  }
};

} // namespace examples

#endif /* GAOL_EXAMPLES_DUAL_H */
