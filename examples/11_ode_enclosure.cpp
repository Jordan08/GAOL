/*-*-C++-*------------------------------------------------------------------
 * gaol -- NOT Just Another Interval Library
 *--------------------------------------------------------------------------
 * Examples of GAOL v5: enclosing the solutions of an ordinary differential
 * equation, and an integral.
 *
 * The program encloses, at a final time, every solution of x' = f(x) that
 * starts in a box X0, as validated ODE solvers do. Each step of size h has
 * two stages. The first finds an a priori enclosure of x over the whole
 * step: if X + [0, h] f(B) is a subset of B, Picard's theorem says that
 * every solution starting in X exists over [0, h] and stays in B; B is found
 * by epsilon-inflation. The second encloses x(t + h) by the Taylor
 * polynomial of the solution in h, plus Lagrange's remainder bounded over B.
 * Part 1 does it for x' = -x^2 from [0.9, 1.1] on [0, 2], with polynomials
 * of order 1 and 2, then with [0.9, 1.1] cut into 10 pieces, and checks the
 * results against the image of [0.9, 1.1] by the exact solution
 * x0 / (1 + x0 t). Part 2 is Codac's examples/14_lohner (x0' = -x0,
 * x1' = -sin(x1) from [0.9, 1.1]^2 on [0, 10], steps 0.2 and 0.01).
 * Evaluated naturally, the step x + h f(x) takes x and f(x) as independent,
 * and the width of the enclosure grows as (1 + h)^n where the true one
 * shrinks as (1 - h)^n: thousands instead of 1e-5. The mean-value form of
 * the step, phi(m) + J(X) (X - m) with the Jacobian J computed by dual.h,
 * keeps it near the true width; it is the core of Lohner's method (Codac's
 * CtcLohner, CAPD, VNODE-LP). The exact solution, x0(0) e^-t and
 * 2 atan(tan(x1(0) / 2) e^-t), is checked at the corners and the center of
 * the initial box. Part 3 encloses the integral of exp(-x^2) over [0, 1] by
 * interval Riemann sums, as a tube of Codac encloses its integral
 * (src/core/domains/tube/codac2_SlicedTube_integral_impl.h). The references
 * were computed with mpmath, from the closed forms and checked by its
 * odefun and quad. The method follows R. J. Lohner, "Enclosing the
 * solutions of ordinary initial and boundary value problems" (in Kaucher,
 * Kulisch and Ullrich, "Computer Arithmetic", Teubner, 1987), the survey of
 * N. S. Nedialkov, K. R. Jackson and G. F. Corliss, "Validated solutions of
 * initial value problems for ordinary differential equations" (Applied
 * Mathematics and Computation 105, 1999), and chapters 9 and 10 of Moore,
 * Kearfott and Cloud, "Introduction to Interval Analysis" (SIAM, 2009).
 *
 * Copyright (c) 2026 ENSTA, France
 *
 * Created 2026-09-27 by Jordan NININ
 *--------------------------------------------------------------------------
 * gaol is a software distributed WITHOUT ANY WARRANTY. Read the associated
 * COPYING file for information.
 *--------------------------------------------------------------------------*/

#include <algorithm>
#include <array>
#include <cfenv>
#include <cstddef>
#include <cstdio>
#include <cstdlib>
#include <iostream>
#include <sstream>
#include <string>
#include <vector>

#include <gaol/gaol.h>

#include "box.h"
#include "dual.h"

// The namespace gaol gives interval and GAOL's pow(x, n), which the Taylor
// remainders use: argument-dependent lookup finds sqr, sin and exp on
// intervals, but not pow (see 14_generic_programming.cpp)
using namespace gaol;
using examples::Box;
using examples::Dual;
using Dual2 = examples::Dual<2>;

namespace {

  bool all_checks_hold = true;

  // Every claim of the output is checked here: a claim that does not hold is
  // reported, and makes the program fail, which is how ctest runs it
  void check(bool ok, const std::string& what)
  {
    if (!ok) {
      std::cout << "FAILED: " << what << "\n";
      all_checks_hold = false;
    }
  }

  /*
    A double with four significant digits, for widths and ratios. GAOL
    rounds the doubles of the whole program upward until gaol::cleanup(), and
    with glibc their conversion to text as well: 1.0000001 would be written
    1.001. The conversion is made rounding to nearest, which GAOL allows: each
    of its operations sets the direction upward again when it is not.
  */
  std::string approx(double d)
  {
    const int direction = std::fegetround();
    std::fesetround(FE_TONEAREST);
    char text[32];
    std::snprintf(text, sizeof text, "%.4g", d);
    std::fesetround(direction);
    return text;
  }

  // The text of an interval or of a box, as operator<< writes it
  template <class T>
  std::string text(const T& x)
  {
    std::ostringstream os;
    os << x;
    return os.str();
  }

  // s followed by spaces up to n characters: the columns of the tables
  std::string column(const std::string& s, std::size_t n)
  {
    return s.size() < n ? s + std::string(n - s.size(), ' ') : s + ' ';
  }

  // The exact width of Y, enclosed: Y.width() is a double rounded upward, an
  // upper bound only, from which no ratio can be bounded below
  interval width_of(const interval& Y) { return interval(Y.right()) - interval(Y.left()); }

  // The ratio of the width of an enclosure to the width of the exact set: an
  // interval, of which the midpoint is printed
  interval ratio(const interval& Y, const interval& exact) { return width_of(Y) / width_of(exact); }

  // Epsilon-inflation (S. M. Rump): P widened by a tenth of its width on each
  // side. It is a guess, which can be computed any way: only the test that
  // accepts it has to be rigorous.
  interval inflate(const interval& P)
  {
    const double r = 0.1 * P.width();
    return P + interval(-r, r);
  }

  /*
    X cut into n slices that cover it with no gap. The bounds are computed in
    doubles, hence rounded; each is the right bound of a slice and the left
    bound of the next one, so that whatever the rounding every point of X
    lies in some slice. 03_dependency_problem.cpp cuts boxes the same way.
  */
  std::vector<interval> slices(const interval& X, int n)
  {
    std::vector<interval> s;
    double a = X.left();
    for (int i = 1; i <= n; ++i) {
      const double b = (i == n) ? X.right() : std::min(X.left() + i * (X.right() - X.left()) / n, X.right());
      s.push_back(interval(a, b));
      a = b;
    }
    return s;
  }

  // ===========================================================================
  // Part 1: x' = -x^2
  // ===========================================================================

  // f is written once, as a generic lambda: interval in, interval out
  const auto f1 = [](const auto& x) { return -sqr(x); };

  /*
    The a priori enclosure of the solutions over one step, from every x(t) in
    X. Picard's theorem: if B is an interval such that

      P = X + [0, h] f(B)   is a subset of B,

    then every solution starting in X exists over [t, t + h] and stays in B.
    It then stays in P as well, x(t + s) being x(t) + the integral of f(x)
    over [t, t + s], which lies in X + s f(B): P, tighter, is returned. B is
    searched for as Codac's LohnerAlgorithm::globalEnclosure searches for it:
    start from X, and while the test fails, take P inflated. Returns false
    when no enclosure is found: the step h is then too large for f.
  */
  bool apriori(const interval& X, const interval& h, interval& enclosure, int& inflations)
  {
    // [0, h], as the hull of 0 and h: h is an interval, see step_size().
    const interval tau = interval(0.0) | h;
    interval B = X;
    for (inflations = 0; inflations < 30; ++inflations) {
      const interval P = X + tau * f1(B);
      if (B.set_contains(P)) {
        enclosure = P;
        return true;
      }
      B = inflate(P);
    }
    return false;
  }

  /*
    The Taylor expansion of the solution. Along a solution, the derivative of
    x^n is n x^(n-1) x' = -n x^(n+1), so that the k-th derivative of x is
    (-1)^k k! x^(k+1), and

      x(t + h) = sum_{k=0..p} (-h)^k x(t)^(k+1)  +  (-h)^(p+1) x(tau)^(p+2)

    for some tau in [t, t + h] (Lagrange's remainder). A general solver
    computes these coefficients by automatic differentiation of f (VNODE-LP,
    CAPD); for -x^2 they are simple enough to be written by hand.

    The polynomial, the step map phi(x), by Horner's scheme,
    x (1 - h x (1 - h x (...))). It is a template: on an interval it gives
    phi(X), on a Dual<1> the derivative phi'(X) as well.
  */
  template <class T>
  T taylor_polynomial(const T& x, const interval& h, int p)
  {
    T s(1.0);
    for (int k = 0; k < p; ++k) {
      s = 1.0 - h * x * s;
    }
    return x * s;
  }

  // The remainder: x(tau) lies in the a priori enclosure B. pow(x, n) is
  // GAOL's integer power, sharper than a product of n factors, each of which
  // would range over B independently of the others.
  interval taylor_remainder(const interval& B, const interval& h, int p)
  {
    return pow(-h, p + 1) * pow(B, p + 2);
  }

  /*
    One step, from the enclosure X of x(t) to that of x(t + h). phi(X) is
    evaluated in its mean-value form,

      phi(x) is in phi(m) + phi'(X) (X - m)   for every x in X,

    m being the middle of X, rather than naturally: part 2 shows why. phi(m)
    is computed on the point interval m = X.mid() (mid() is an interval,
    midpoint() the double in it), so that its rounding errors are enclosed;
    phi'(X) encloses the derivative over the whole of X.

    The mean-value form is not exact either: phi'(X) has a width of about
    2 h w when X has the width w, and multiplied by X - m it adds some h w^2
    to the width at each step. The excess is quadratic in w: cut X0 into k
    pieces, integrate each, and the excess of each piece is divided by k^2;
    only the two end pieces widen the hull of the results.
  */
  bool step(interval& X, const interval& h, int p)
  {
    interval P;
    int inflations = 0;
    if (!apriori(X, h, P, inflations)) {
      return false;
    }
    const interval m = X.mid();
    const interval dphi = taylor_polynomial(Dual<1>::variable(X, 0), h, p).d[0];
    X = taylor_polynomial(m, h, p) + dphi * (X - m) + taylor_remainder(P, h, p);
    return true;
  }

  // The step T / n. 0.1 and 0.01 are no doubles: h is kept as an interval
  // holding T / n, so that n steps end at T exactly, whatever the rounding
  // (a double h = 0.01 is 0.01000000000000000021, and 1000 steps of it would
  // end beyond 10).
  interval step_size(double T, int n) { return interval(T) / double(n); }

  // x(2) from every x(0) in X0, by n steps of order p, X0 being cut into the
  // given number of pieces: the hull of the enclosures of the pieces
  bool integrate(const interval& X0, int n, int p, int pieces, interval& result)
  {
    const interval h = step_size(2.0, n);
    result = interval::emptyset();
    for (const interval& piece : slices(X0, pieces)) {
      interval X = piece;
      for (int k = 0; k < n; ++k) {
        if (!step(X, h, p)) {
          return false;
        }
      }
      result |= X;
    }
    return true;
  }

  // ===========================================================================
  // Part 2: Codac's examples/14_lohner, x0' = -x0, x1' = -sin(x1)
  // ===========================================================================

  template <class T>
  std::array<T, 2> f2(const T& x0, const T& x1)
  {
    return { -x0, -sin(x1) };
  }

  // The step map of order 1, phi(x) = x + h f(x), for intervals and Dual2
  template <class T>
  std::array<T, 2> phi2(const T& x0, const T& x1, const interval& h)
  {
    const std::array<T, 2> fx = f2(x0, x1);
    return { x0 + h * fx[0], x1 + h * fx[1] };
  }

  using Matrix = std::array<std::array<interval, 2>, 2>;

  // An enclosure of the Jacobian matrix of g over X: g evaluated on the dual
  // numbers (X[0], (1, 0)) and (X[1], (0, 1)), as in 10_krawczyk.cpp
  template <class G>
  Matrix jacobian(const G& g, const Box& X)
  {
    const std::array<Dual2, 2> r = g(Dual2::variable(X[0], 0), Dual2::variable(X[1], 1));
    return { { { r[0].d[0], r[0].d[1] }, { r[1].d[0], r[1].d[1] } } };
  }

  // The a priori enclosure of part 1, component by component
  bool apriori(const Box& X, const interval& h, Box& enclosure)
  {
    const interval tau = interval(0.0) | h;
    Box B = X;
    for (int k = 0; k < 30; ++k) {
      const std::array<interval, 2> fB = f2(B[0], B[1]);
      const Box P{ X[0] + tau * fB[0], X[1] + tau * fB[1] };
      if (B.contains(P)) {
        enclosure = P;
        return true;
      }
      B = Box{ inflate(P[0]), inflate(P[1]) };
    }
    return false;
  }

  enum class Form { natural, mean_value };

  /*
    One step of order 1. x(t + h) = x(t) + h f(x(t)) + h^2 / 2 x''(tau), and
    x'' = J_f(x) f(x): the remainder lies in h^2 / 2 J_f(B) f(B), B being the
    a priori enclosure, with J_f(B) the Jacobian of f over B.

    Natural form: X + h f(X). Each occurrence of x ranges over X on its own:
    x0 + h (-x0) over [a, b] is [a - h b, b - h a], of width (1 + h)(b - a),
    where the true image [(1 - h) a, (1 - h) b] has width (1 - h)(b - a). The
    width is multiplied by (1 + h) at each step instead of (1 - h): by
    (1 + h)^n / (1 - h)^n, some e^(2 t), after n steps. It is the dependency
    problem of 03_dependency_problem.cpp, and a smaller step does not cure it.

    Mean-value form: phi(m) + J_phi(X) (X - m), J_phi = I + h J_f the
    Jacobian of the step map over X, by Dual2. X - m occurs once: the factor
    (1 - h) now multiplies the width. Here J_phi is diagonal and the box stays
    a box; for a system that turns boxes, enclosing the turned box in a box at
    each step loses width again, the wrapping effect (R. E. Moore, 1965; see
    03_dependency_problem.cpp). Lohner's method keeps the set as m + A r, a
    parallelepiped with a matrix A carried from step to step and kept well
    conditioned by a QR factorization (the matrix B of Codac's
    LohnerAlgorithm); CAPD and VNODE-LP add Taylor polynomials of high order.

    Every rounding error has to be kept, in every part of the step: phi(m) is
    computed with intervals on the point box m, never in doubles. Codac's
    LohnerAlgorithm::integrate (src/core/contractors/codac2_CtcLohner.cpp)
    computes its new center u_hat + h mid(f(u_hat)) + m1 in doubles, and the
    inverse of its matrix B1 in doubles: the rounding errors of these
    operations are enclosed nowhere.
  */
  bool step(Box& X, const interval& h, Form form)
  {
    Box B;
    if (!apriori(X, h, B)) {
      return false;
    }
    const Matrix JB = jacobian([](const auto& x0, const auto& x1) { return f2(x0, x1); }, B);
    const std::array<interval, 2> fB = f2(B[0], B[1]);
    const interval half_h2 = sqr(h) / 2.0;
    const auto phi = [&h](const auto& x0, const auto& x1) { return phi2(x0, x1, h); };

    Box next(2);
    if (form == Form::natural) {
      const std::array<interval, 2> y = phi(X[0], X[1]);
      next = Box{ y[0], y[1] };
    } else {
      const Box m = X.mid();
      const std::array<interval, 2> ym = phi(m[0], m[1]);
      const Matrix J = jacobian(phi, X);
      for (std::size_t i = 0; i < 2; ++i) {
        next[i] = ym[i] + J[i][0] * (X[0] - m[0]) + J[i][1] * (X[1] - m[1]);
      }
    }
    for (std::size_t i = 0; i < 2; ++i) {
      next[i] += half_h2 * (JB[i][0] * fB[0] + JB[i][1] * fB[1]);
    }
    X = next;
    return true;
  }

  // ===========================================================================
  // Part 3: the integral of exp(-x^2) over [0, 1]
  // ===========================================================================

  /*
    [0, 1] cut into n slices X_i = [a_i, b_i]: the integral lies in the sum of
    the (b_i - a_i) F(X_i), F(X_i) enclosing the range of the integrand over
    X_i. It is how a Codac tube, a list of time slices with an interval of
    values each, encloses its integral. The width of a slice is enclosed as
    an interval: width() is a double rounded upward, too large for the lower
    bound of the sum.
  */
  interval riemann_sum(int n)
  {
    const auto F = [](const interval& x) { return exp(-sqr(x)); };
    interval sum(0.0);
    for (const interval& X : slices(interval(0.0, 1.0), n)) {
      sum += width_of(X) * F(X);
    }
    return sum;
  }

} // namespace

int main()
{
  interval::precision(10);

  // -------------------------------------------------------------------------
  std::cout << "1. x' = -x^2, x(0) in X0 = [0.9, 1.1], t in [0, 2]; exact solution x(0) / (1 + x(0) t)\n";
  // textToInterval() encloses the decimals 0.9 and 1.1, which no double
  // equals; interval(0.9, 1.1) would hold the doubles nearest to them only
  const interval X0 = textToInterval("[0.9, 1.1]");
  {
    const interval h = step_size(2.0, 20);
    interval P;
    int inflations = 0;
    const bool found = apriori(X0, h, P, inflations);
    const interval tau = interval(0.0) | h;
    std::cout << "   first step, h = 0.1, from X0 = " << X0 << " (the decimals, enclosed)\n"
              << "     P = X0 + [0, h] f(B) = " << P << " is a subset of B, found after " << inflations
              << " inflation" << (inflations == 1 ? "" : "s") << "\n"
              << "     so every x(t), t in [0, h], lies in P; the true range is [0.9 / 1.09, 1.1]\n";
    check(found && P.set_contains(X0) && P.set_contains(X0 + tau * f1(P)), "Picard's test holds on the first step");
    // The true solutions over [0, h] from [0.9, 1.1]: the lowest is
    // 0.9 / (1 + 0.09) = 0.825688..., the highest 1.1 at t = 0
    check(P.set_contains(textToInterval("0.8256880733944954128440366972477064220183") | X0),
          "the a priori enclosure holds the solutions over [0, 0.1]");
  }

  // The image of [0.9, 1.1] by the exact solution at t = 2: x0 / (1 + 2 x0)
  // increases with x0, and the image is [0.9 / 2.8, 1.1 / 3.2] = [9/28, 11/32]
  const interval exact1 = textToInterval("0.3214285714285714285714285714285714285714") | interval(0.34375);
  std::cout << "   x(2) by Taylor steps in the mean-value form, remainder bounded over P:\n"
            << "     h      order  pieces  x(2) in                         width / exact width\n";
  struct Run {
    int n;
    const char* h;
    int p;
    int pieces;
    interval X;
  };
  Run runs[5] = { { 20, "0.1", 1, 1, {} },
                  { 20, "0.1", 2, 1, {} },
                  { 200, "0.01", 1, 1, {} },
                  { 200, "0.01", 2, 1, {} },
                  { 200, "0.01", 2, 10, {} } };
  for (Run& r : runs) {
    check(integrate(X0, r.n, r.p, r.pieces, r.X), "an a priori enclosure is found at every step of part 1");
    std::cout << "     " << column(r.h, 7) << column(std::to_string(r.p), 7) << column(std::to_string(r.pieces), 8)
              << column(text(r.X), 32) << approx(ratio(r.X, exact1).midpoint()) << "\n";
    check(r.X.set_contains(exact1), "the enclosure of x(2) holds the exact image of [0.9, 1.1]");
  }
  std::cout << "     exact image of X0: [9/28, 11/32] = " << exact1 << "\n";
  // < on intervals is "certainly less": every element of the left interval
  // is below every element of the right one. The widths being enclosed,
  // these tests are proofs, not comparisons of rounded doubles.
  // Order p leaves a remainder in h^(p+1) at each step, h^p over the 2 / h
  // steps: it dominates at h = 0.1, where order 2 gains
  check(width_of(runs[1].X) < width_of(runs[0].X), "at h = 0.1, order 2 is tighter than order 1");
  check(width_of(runs[2].X) < width_of(runs[0].X) && width_of(runs[3].X) < width_of(runs[1].X),
        "h = 0.01 is tighter than h = 0.1");
  // At h = 0.01 the remainders are negligible, and what is left is the excess
  // of the mean-value form, in the square of the width of X (see step())
  check(width_of(runs[2].X) < 1.01 * width_of(runs[3].X) && width_of(runs[3].X) < 1.01 * width_of(runs[2].X),
        "at h = 0.01, orders 1 and 2 are within 1% of each other");
  check(width_of(runs[4].X) < width_of(runs[3].X) && ratio(runs[4].X, exact1) < 1.01,
        "X0 cut into 10 pieces gives a width within 1% of the exact one");
  std::cout << "   each holds the exact image; order 2 gains at h = 0.1, where the remainder\n"
            << "   dominates; at h = 0.01 the excess comes from the width of X: 10 pieces remove it\n";

  // -------------------------------------------------------------------------
  std::cout << "2. Codac's examples/14_lohner: x0' = -x0, x1' = -sin(x1), x(0) in [0.9, 1.1]^2, t in [0, 10]\n";
  // The exact solutions at t = 10 from the corners and the center of the
  // initial box, x0(0) e^-10 and 2 atan(tan(x1(0) / 2) e^-10), by mpmath.
  // Both increase with the initial value: the exact image of the box is the
  // box of the images of 0.9 and 1.1.
  const interval x0_10[3] = { textToInterval("0.00004085993678623636638203236400449554921413"),
                              textToInterval("0.00004539992976248485153559151556055061023792"),
                              textToInterval("0.00004993992273873333668915066711660567126171") };
  const interval x1_10[3] = { textToInterval("0.00004386133209377857577275105566580286115045"),
                              textToInterval("0.00004960418932578612835305135825821192810679"),
                              textToInterval("0.00005566986722621193905245902864968715822707") };
  const Box exact2{ x0_10[0] | x0_10[2], x1_10[0] | x1_10[2] };
  std::cout << "   h     form        width of x0(10)  width of x1(10)  width / exact width\n";
  const int steps2[2] = { 50, 1000 };
  const char* step2_text[2] = { "0.2", "0.01" };
  Box final_box[2][2];
  for (int i = 0; i < 2; ++i) {
    const interval h = step_size(10.0, steps2[i]);
    for (Form form : { Form::natural, Form::mean_value }) {
      Box X{ X0, X0 };
      bool ok = true;
      for (int k = 0; k < steps2[i] && ok; ++k) {
        ok = step(X, h, form);
      }
      check(ok, "an a priori enclosure is found at every step of part 2");
      const int j = form == Form::natural ? 0 : 1;
      final_box[i][j] = X;
      std::cout << "   " << column(step2_text[i], 6) << column(j == 0 ? "natural" : "mean-value", 12)
                << column(approx(width_of(X[0]).midpoint()), 17) << column(approx(width_of(X[1]).midpoint()), 17)
                << approx(ratio(X[0], exact2[0]).midpoint()) << ", " << approx(ratio(X[1], exact2[1]).midpoint())
                << "\n";
      // Every corner and the center: (0.9, 0.9), (0.9, 1.1), ..., (1, 1)
      for (int a = 0; a < 3; ++a) {
        for (int b = 0; b < 3; ++b) {
          if ((a == 1) != (b == 1)) {
            continue;
          }
          check(X[0].set_contains(x0_10[a]) && X[1].set_contains(x1_10[b]),
                "the enclosure of x(10) holds the exact solution from a corner or the center");
        }
      }
    }
  }
  std::cout << "   exact             " << column(approx(width_of(exact2[0]).midpoint()), 17)
            << approx(width_of(exact2[1]).midpoint()) << "\n";
  interval::precision(4);
  std::cout << "   h = 0.01, mean-value form: x(10) in " << final_box[1][1] << "\n"
            << "   exact image of the initial box:     " << exact2 << "\n";
  for (int i = 0; i < 2; ++i) {
    check(ratio(final_box[i][0][0], exact2[0]) > 1e6 && ratio(final_box[i][0][1], exact2[1]) > 1e3,
          "the natural form explodes, whatever the step");
  }
  check(ratio(final_box[1][1][0], exact2[0]) < 1.2 && ratio(final_box[1][1][1], exact2[1]) < 1.2,
        "the mean-value form with h = 0.01 is within 20% of the exact widths");
  check(ratio(final_box[0][1][0], exact2[0]) < 5.0 && ratio(final_box[0][1][1], exact2[1]) < 5.0,
        "the mean-value form with h = 0.2 is within 5 times the exact widths");
  // The cause, on the first step with h = 0.01 and on x0 alone: the width w0
  // of X0 is multiplied by 1 + h in the natural form and by 1 - h in the
  // mean-value form, up to the width of the remainder, below h^2 w0
  {
    const interval h = step_size(10.0, 1000);
    const interval w0 = width_of(X0);
    Box natural{ X0, X0 }, mean_value{ X0, X0 };
    check(step(natural, h, Form::natural) && step(mean_value, h, Form::mean_value),
          "an a priori enclosure is found on the first step");
    const interval wn = width_of(natural[0]), wm = width_of(mean_value[0]);
    check((1.0 + h) * w0 < wn && wn < (1.0 + h + sqr(h)) * w0, "a natural step multiplies the width of x0 by 1 + h");
    check((1.0 - h) * w0 < wm && wm < (1.0 - h + sqr(h)) * w0, "a mean-value step multiplies the width of x0 by 1 - h");
  }
  std::cout << "   all hold the exact solutions from the 4 corners and the center; a step\n"
            << "   multiplies the width of x0 by 1 + h naturally, by 1 - h in the mean-value form\n";

  // -------------------------------------------------------------------------
  std::cout << "3. The integral of exp(-x^2) over [0, 1] by interval Riemann sums\n";
  interval::precision(10);
  // sqrt(pi) / 2 erf(1), by mpmath (quad gives the same digits)
  const interval integral = textToInterval("0.746824132812427025399467436131853005354499687");
  std::cout << "   the integral, sqrt(pi) / 2 erf(1), is in " << integral << "\n";
  interval previous;
  for (int n : { 10, 100, 1000 }) {
    const interval S = riemann_sum(n);
    std::cout << "   n = " << column(std::to_string(n), 7) << column(text(S), 30) << "width "
              << approx(width_of(S).midpoint()) << "\n";
    check(S.set_contains(integral), "the Riemann sum holds the integral");
    // exp(-x^2) decreases: the sum is [lower sum, upper sum], whose width
    // telescopes to (F(0) - F(1)) / n = (1 - 1/e) / n
    const interval expected = (1.0 - exp(interval(-1.0))) / double(n);
    check(0.99 * expected < width_of(S) && width_of(S) < 1.01 * expected, "the width of the sum is (1 - 1/e) / n");
    if (n > 10) {
      check(width_of(S) < width_of(previous) / 5.0, "the Riemann sums shrink as 1 / n");
    }
    previous = S;
  }
  std::cout << "   each holds the integral; their width, (1 - 1/e) / n, shrinks as 1 / n\n";

  // The last use of GAOL: cleanup() sets back the rounding direction the
  // program started with
  gaol::cleanup();
  if (!all_checks_hold) {
    return EXIT_FAILURE;
  }
  std::cout << "All the checks hold.\n";
  return 0;
}
