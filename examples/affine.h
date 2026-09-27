/*-*-C++-*------------------------------------------------------------------
 * gaol -- NOT Just Another Interval Library
 *--------------------------------------------------------------------------
 * Examples of GAOL v5: a minimal affine arithmetic built on GAOL's
 * intervals.
 *
 * An affine form represents a real quantity x as
 *
 *     x = c + a1 e1 + ... + an en + e [-1, 1]
 *
 * The noise symbols e1, ..., en are unknown reals of [-1, 1], shared by all
 * the forms that depend on the same source of uncertainty; the last term,
 * of radius e >= 0, holds errors that no other form shares. Where interval
 * arithmetic forgets that two occurrences of x are the same number, affine
 * arithmetic keeps the linear correlations between quantities, and x - x
 * cancels (J. Stolfi and L. H. de Figueiredo, "Self-validated numerical
 * methods and applications", 1997). The class follows ibex-affine (J.
 * Ninin), whose Affine3 model also keeps sparse terms, numbers the noise
 * symbols with a global counter and computes each coefficient as an
 * interval (src/arithmetic/ibex_Affine3_fAFFullI.h), YalAA (S. Kiel) and
 * the affari class of INTLAB (S. M. Rump), with far less: variables and
 * constants from intervals, +, -, *, /, sqr, exp, sqrt, and to_interval().
 *
 * Its soundness rests on GAOL alone. Each coefficient is computed as a GAOL
 * interval enclosing its exact value; the form keeps the midpoint of that
 * interval and adds its radius to e, mid_rad() giving the smallest radius r
 * such that the interval lies within [m - r, m + r]. The code thus does not
 * depend on the rounding direction. The error-free transformations (TwoSum,
 * Fast2Sum) with which ibex-affine's default model accounts for rounding
 * errors exactly assume rounding to nearest: under the upward rounding that
 * GAOL leaves for the whole program, TwoSum returns an error of 0 for some
 * nonzero errors, and a form built on it misses exact values.
 *
 * A nonlinear function f is replaced by a line alpha x + beta, plus a new
 * noise symbol whose coefficient bounds the residual f(t) - alpha t - beta
 * over the range of x. That band is enclosed by interval evaluations of f at
 * a few points, so that any double alpha gives a sound result: the choice of
 * alpha, min-range or Chebyshev, only makes it tighter or looser.
 *
 * Copyright (c) 2026 ENSTA, France
 *
 * Created 2026-09-27 by Jordan NININ
 *--------------------------------------------------------------------------
 * gaol is a software distributed WITHOUT ANY WARRANTY. Read the associated
 * COPYING file for information.
 *--------------------------------------------------------------------------*/

#ifndef GAOL_EXAMPLES_AFFINE_H
#define GAOL_EXAMPLES_AFFINE_H

#include <cfenv>
#include <cmath>
#include <cstddef>
#include <cstdio>
#include <limits>
#include <ostream>
#include <string>
#include <utility>
#include <vector>

#include <gaol/gaol.h>

namespace examples {

using gaol::interval;

/*
  A double written with the given number of significant digits, rounded to
  nearest. In the default build of GAOL, the program's own doubles are
  rounded upward until gaol::cleanup(), and so is their conversion to text
  by glibc: 0.30000000000000004 would be written 0.300001. The direction is
  set to nearest for the conversion, then restored, which GAOL allows: each
  of its operations sets the direction upward again when it is not.
*/
inline std::string decimal(double d, int digits = 6)
{
  const int direction = std::fegetround();
  std::fesetround(FE_TONEAREST);
  char text[32];
  std::snprintf(text, sizeof text, "%.*g", digits, d);
  std::fesetround(direction);
  return text;
}

// The two ways of choosing the slope of the line that replaces a nonlinear
// function, ibex-affine's AF_Lin_MinRange and AF_Lin_Chebyshev
enum class Linearization { min_range, chebyshev };

class Affine {
public:
  // The whole line, as interval() and ibex-affine's Affine2(): generic code
  // writes T(0.0) for zero
  Affine() { set_unbounded(); }

  // The exact constant d. Not explicit, so that 1.0 - x reads as it does on
  // intervals. A decimal constant such as 0.1 is no double: it is
  // Affine::constant(gaol::textToInterval("0.1")).
  Affine(double d) : c_(d)
  {
    if (!std::isfinite(d)) {
      set_unbounded();
    }
  }

  // A new input known to lie in X, as a component of ibex-affine's
  // Affine2Variables: a new noise symbol, x = m + r e_k, with [m - r, m + r]
  // enclosing X. A form holds neither the empty set nor an unbounded
  // interval: both become the whole line, which is sound.
  static Affine variable(const interval& X)
  {
    Affine x(0.0);
    if (!X.is_common_interval()) {
      x.set_unbounded();
      return x;
    }
    double r;
    X.mid_rad(x.c_, r);
    x.push(new_symbol(), r);
    return x;
  }

  // A quantity known to lie in X that shares nothing with other forms: no
  // noise symbol, its radius goes to the error term, which never cancels
  static Affine constant(const interval& X)
  {
    Affine x(0.0);
    if (!X.is_common_interval()) {
      x.set_unbounded();
      return x;
    }
    X.mid_rad(x.c_, x.e_);
    return x;
  }

  // The range of the form, c + (|a1| + ... + |an| + e) [-1, 1], enclosed
  interval to_interval() const
  {
    ErrorSum radius;
    for (const Term& t : a_) {
      radius.add(std::fabs(t.second));
    }
    radius.add(e_);
    return interval(c_) + interval(-radius.upper(), radius.upper());
  }

  double center() const { return c_; }
  double error() const { return e_; }
  // The number of noise symbols of the form, those of coefficient 0 aside
  std::size_t size() const { return a_.size(); }
  bool unbounded() const { return !std::isfinite(e_); }

  // ---- linear operations ---------------------------------------------------

  // Exact: the opposite of a double is a double
  friend Affine operator-(const Affine& x)
  {
    Affine r(x);
    r.c_ = -r.c_;
    for (Term& t : r.a_) {
      t.second = -t.second;
    }
    return r;
  }

  // The coefficients add up, symbol by symbol; each sum is enclosed by an
  // interval, then settled, and the errors of x and y add up too
  friend Affine operator+(const Affine& x, const Affine& y)
  {
    Affine r(0.0);
    ErrorSum err;
    err.add(x.e_);
    err.add(y.e_);
    r.c_ = settle(interval(x.c_) + y.c_, err);
    merge(x, y, [&](std::size_t k, double ax, double ay) { r.push(k, settle(interval(ax) + ay, err)); });
    r.set_error(err);
    return r;
  }

  friend Affine operator-(const Affine& x, const Affine& y) { return x + (-y); }

  // k x, for a real k known to lie in K: K = interval(2.0) for the double 2,
  // or an enclosure of an irrational such as cos(pi/4). Each k a_i is
  // enclosed by K a_i, which holds it whichever k of K is the true one.
  friend Affine operator*(const interval& K, const Affine& x)
  {
    Affine r(0.0);
    if (!K.is_common_interval() || x.unbounded()) {
      r.set_unbounded();
      return r;
    }
    ErrorSum err;
    err.add(K.mag() * interval(x.e_));
    r.c_ = settle(K * x.c_, err);
    for (const Term& t : x.a_) {
      r.push(t.first, settle(K * t.second, err));
    }
    r.set_error(err);
    return r;
  }

  friend Affine operator*(double k, const Affine& x) { return interval(k) * x; }
  friend Affine operator*(const Affine& x, double k) { return interval(k) * x; }

  // ---- products --------------------------------------------------------------

  /*
    x y = cx cy + sum (cx ay_i + cy ax_i) e_i + Q, where, with Lx = sum ax_i e_i
    and Ex the error term of x, Q = (Lx + Ex)(Ly + Ey) + cx Ey + cy Ex. In
    Lx Ly, the terms ax_i ay_i e_i^2 lie between 0 and ax_i ay_i: their sum is
    P/2 +- |P|/2, with P = sum ax_i ay_i and |P| = sum |ax_i ay_i|; the other
    terms are within +-(Sx Sy - |P|), with Sx = sum |ax_i|. So Q lies in
    P/2 +- (|cx| ey + |cy| ex + (Sx + ex)(Sy + ey) - |P|/2): the formula of
    X.-H. Vu, D. Sam-Haroud and B. Faltings (2004), which ibex-affine uses.
  */
  friend Affine operator*(const Affine& x, const Affine& y)
  {
    Affine r(0.0);
    if (x.unbounded() || y.unbounded()) {
      r.set_unbounded();
      return r;
    }
    ErrorSum err;
    interval Sx(0.0), Sy(0.0), P(0.0), Pabs(0.0);
    merge(x, y, [&](std::size_t k, double ax, double ay) {
      r.push(k, settle(interval(x.c_) * ay + interval(y.c_) * ax, err));
      Sx += std::fabs(ax);
      Sy += std::fabs(ay);
      const interval p = interval(ax) * ay;
      P += p;
      Pabs += abs(p);
    });
    r.c_ = settle(interval(x.c_) * y.c_ + 0.5 * P, err);
    // The bound of Q - P/2 is computed as an interval too: its right bound is
    // above the exact bound, even after the subtraction of |P|/2
    err.add(std::fabs(x.c_) * interval(y.e_) + std::fabs(y.c_) * interval(x.e_)
            + (Sx + x.e_) * (Sy + y.e_) - 0.5 * Pabs);
    r.set_error(err);
    return r;
  }

  /*
    x^2 = cx^2 + sum 2 cx a_i e_i + 2 cx Ex + (Lx + Ex)^2, and (Lx + Ex)^2
    lies in [0, (S + e)^2], S = sum |a_i|: a center cx^2 + (S + e)^2 / 2 and
    an error of at most 2 |cx| e + (S + e)^2 / 2. Tighter than x * x when x
    has several symbols: the quadratic part of x * x can go below 0.
  */
  friend Affine sqr(const Affine& x)
  {
    Affine r(0.0);
    if (x.unbounded()) {
      r.set_unbounded();
      return r;
    }
    ErrorSum err;
    interval S(x.e_);
    for (const Term& t : x.a_) {
      S += std::fabs(t.second);
      r.push(t.first, settle(2.0 * interval(x.c_) * t.second, err));
    }
    // sqr of an interval, found in GAOL by argument-dependent lookup
    const interval half = 0.5 * sqr(S);
    r.c_ = settle(sqr(interval(x.c_)) + half, err);
    err.add(2.0 * std::fabs(x.c_) * interval(x.e_) + half);
    r.set_error(err);
    return r;
  }

  // Written as ibex-affine writes its forms, 0.5 + 2.5 e1 - 0.1 e4 + 1.4e-17 [-1, 1],
  // with 6 digits, and 2 for the error term, whose size is what matters
  friend std::ostream& operator<<(std::ostream& os, const Affine& x)
  {
    if (x.unbounded()) {
      return os << "(the whole line)";
    }
    os << decimal(x.c_);
    for (const Term& t : x.a_) {
      os << (t.second < 0.0 ? " - " : " + ") << decimal(std::fabs(t.second)) << " e" << t.first;
    }
    if (x.e_ != 0.0) {
      os << " + " << decimal(x.e_, 2) << " [-1, 1]";
    }
    return os;
  }

private:
  using Term = std::pair<std::size_t, double>;  // a noise symbol and its coefficient

  double c_ = 0.0;
  std::vector<Term> a_;  // sorted by symbol, no zero coefficient
  double e_ = 0.0;

  /*
    An upper bound of a sum of nonnegative errors, accumulated in an interval
    so that it is rounded upward whatever the rounding direction. Each term
    is added as [0, r] rather than as interval(r), which is the empty set for
    r = +oo (IEEE 1788 has no interval [+oo, +oo]): an overflow must give "any
    error", not "no error". NaN, the radius of the empty set, counts as +oo.
  */
  class ErrorSum {
  public:
    void add(double r)
    {
      if (std::isnan(r)) {
        r = std::numeric_limits<double>::infinity();
      }
      if (r > 0.0) {
        s_ += interval(0.0, r);
      }
    }
    // An error known by an enclosure: its right bound is above it
    void add(const interval& r) { add(r.is_empty() ? std::numeric_limits<double>::infinity() : r.right()); }
    double upper() const { return s_.right(); }

  private:
    interval s_ = interval(0.0);
  };

  /*
    The coefficient kept for an exact value that v encloses: the midpoint of
    v, the distance to the exact value, at most the radius of v, going to the
    error. This is where rounding errors are accounted for, whatever the
    rounding direction in effect.
  */
  static double settle(const interval& v, ErrorSum& err)
  {
    double m, r;
    v.mid_rad(m, r);
    err.add(r);
    return m;
  }

  // The noise symbols are numbered 1, 2, ... for the whole program
  static std::size_t new_symbol()
  {
    static std::size_t count = 0;
    return ++count;
  }

  void push(std::size_t k, double a)
  {
    if (a != 0.0) {
      a_.emplace_back(k, a);
    }
  }

  // Makes e the error term; an infinite one (an overflow) makes the form the
  // whole line
  void set_error(const ErrorSum& err)
  {
    e_ = err.upper();
    if (!std::isfinite(e_) || !std::isfinite(c_)) {
      set_unbounded();
    }
  }

  void set_unbounded()
  {
    c_ = 0.0;
    a_.clear();
    e_ = std::numeric_limits<double>::infinity();
  }

  // Calls f(k, ax, ay) for each symbol k of x or y, in increasing order, with
  // the coefficients of e_k in x and in y, 0.0 in the form that lacks it
  template <class F>
  static void merge(const Affine& x, const Affine& y, F f)
  {
    std::size_t i = 0, j = 0;
    while (i < x.a_.size() || j < y.a_.size()) {
      if (j == y.a_.size() || (i < x.a_.size() && x.a_[i].first < y.a_[j].first)) {
        f(x.a_[i].first, x.a_[i].second, 0.0);
        ++i;
      } else if (i == x.a_.size() || y.a_[j].first < x.a_[i].first) {
        f(y.a_[j].first, 0.0, y.a_[j].second);
        ++j;
      } else {
        f(x.a_[i].first, x.a_[i].second, y.a_[j].second);
        ++i;
        ++j;
      }
    }
  }
};

// ---- nonlinear functions -----------------------------------------------------

/*
  f(x) for a function f convex or concave over the range X of x, as
  alpha x + beta + delta e_new. The residual r(t) = f(t) - alpha t is then
  convex or concave as well: over X, it reaches its extremes at the bounds of
  X and, when it lies in X, at the point where f'(t) = alpha, which U
  encloses. The band of r over X is enclosed by evaluating f with intervals
  at those points, so that [beta - delta, beta + delta] holds it whatever
  double alpha is. The residual depends on x in a way that the form does not
  follow: it becomes a new noise symbol, as with Stolfi and de Figueiredo,
  rather than going to the error term, so that u - u still cancels for
  u = exp(x).
*/
template <class F>
Affine linearize(const Affine& x, F f, double alpha, const interval& U)
{
  const interval X = x.to_interval();
  const interval a(X.left()), b(X.right());
  interval band = (f(a) - alpha * a) | (f(b) - alpha * b);
  const interval inner = U & X;
  if (!inner.is_empty()) {
    band |= f(inner) - alpha * inner;
  }
  double beta, delta;
  band.mid_rad(beta, delta);
  return alpha * x + beta + Affine::variable(interval(-delta, delta));
}

// A double near the slope of the chord of f over X: the Chebyshev slope
template <class F>
double chord_slope(F f, const interval& X)
{
  const interval a(X.left()), b(X.right());
  return ((f(b) - f(a)) / (b - a)).midpoint();
}

inline Affine exp(const Affine& x, Linearization mode = Linearization::chebyshev)
{
  const interval X = x.to_interval();
  if (!X.is_finite() || X.is_a_double()) {
    return Affine::constant(gaol::exp(X));
  }
  const auto f = [](const interval& t) { return gaol::exp(t); };
  // Min-range: a lower bound of the smallest slope of exp over X, exp(lb).
  // The residual is then nondecreasing, and the range of the result is
  // exp(X) itself, hence the name; Chebyshev makes the band narrower.
  double alpha = gaol::exp(interval(X.left())).left();
  if (mode == Linearization::chebyshev) {
    alpha = chord_slope(f, X);
  }
  // exp'(t) = alpha at t = log(alpha); none when alpha <= 0 (log is empty)
  return linearize(x, f, alpha, gaol::log(interval(alpha)));
}

inline Affine sqrt(const Affine& x, Linearization mode = Linearization::chebyshev)
{
  const interval X = x.to_interval();
  // Below 0, GAOL's sqrt keeps the part of X in its domain, as IEEE 1788
  // does; the example does not split X at 0, and returns that enclosure as a
  // constant (the empty set, for X below 0, becoming the whole line)
  if (!X.is_finite() || X.is_a_double() || X.left() < 0.0) {
    return Affine::constant(gaol::sqrt(X));
  }
  const auto f = [](const interval& t) { return gaol::sqrt(t); };
  // Min-range: a lower bound of the smallest slope, 1 / (2 sqrt(ub)): the
  // residual is then nondecreasing
  double alpha = (0.5 / gaol::sqrt(interval(X.right()))).left();
  if (mode == Linearization::chebyshev) {
    alpha = chord_slope(f, X);
  }
  // sqrt'(t) = alpha at t = 1 / (4 alpha^2)
  return linearize(x, f, alpha, 0.25 / sqr(interval(alpha)));
}

// 1 / x, for a range X that does not hold 0; otherwise 1 / X, the whole line
inline Affine inv(const Affine& x, Linearization mode = Linearization::chebyshev)
{
  const interval X = x.to_interval();
  if (!X.is_finite() || X.is_a_double() || X.straddles_zero()) {
    return Affine::constant(1.0 / X);
  }
  const auto f = [](const interval& t) { return 1.0 / t; };
  const bool positive = X.left() > 0.0;
  // Min-range: an upper bound of the largest slope -1/t^2, at the bound of X
  // farthest from 0: the residual is then nonincreasing
  const interval far(positive ? X.right() : X.left());
  double alpha = (-1.0 / sqr(far)).right();
  if (mode == Linearization::chebyshev) {
    alpha = chord_slope(f, X);
  }
  // -1/t^2 = alpha at t = 1/sqrt(-alpha), taken with the sign of X
  const interval root = 1.0 / gaol::sqrt(interval(-alpha));
  return linearize(x, f, alpha, positive ? root : -root);
}

inline Affine operator/(const Affine& x, const Affine& y) { return x * inv(y); }

} // namespace examples

#endif /* GAOL_EXAMPLES_AFFINE_H */
