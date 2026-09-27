/*-*-C++-*------------------------------------------------------------------
 * gaol -- NOT Just Another Interval Library
 *--------------------------------------------------------------------------
 * Examples of GAOL v5: proving the solutions of a nonlinear system with
 * the Krawczyk operator.
 *
 * The example finds all the solutions of two systems of two equations in
 * two unknowns, f(x, y) = 0, and proves that each box it returns holds
 * exactly one of them. It follows the branch and prune solver of IBEX's
 * documentation, 'solver-generic' in examples/doc-solver.cpp: a stack of
 * boxes; a box over which f cannot vanish (0 not in F(X)) is discarded; the
 * others are contracted by the Krawczyk operator
 *     K(X) = m - C f(m) + (I - C J(X)) (X - m),
 * m being the middle of X, J(X) an enclosure of the Jacobian matrix of f
 * over X and C any real matrix, here the inverse of the middle of J(X)
 * computed in doubles; K(X) holds every solution that X holds, and K(X)
 * inside the interior of X proves that X holds exactly one; the boxes where
 * Krawczyk makes too little progress are bisected. The first system is
 * IBEX's pair of circles, x^2 + y^2 = 1 and (x - 1)^2 + y^2 = 1 on
 * [-10, 10]^2, whose solutions are (1/2, +-sqrt(3)/2) (IBEX writes the
 * equations as distances, sqrt(x^2 + y^2) - 1 = 0, the same solutions). The
 * second is lambda + exp(-lambda) = 0 for a complex lambda = x + i y, as two
 * real equations, on [-10, 10] x [0, 20]: the Evans function of Codac's
 * examples/02_centered_form/main_evans.cpp is its square, and its roots are
 * the values W_k(-1) of Lambert's function, three of them in that box.
 * The Jacobian comes from the automatic differentiation of dual.h: the end
 * of the example shows the Jacobian of the second system written by hand
 * with a sign error, as it was in a review of GAOL v5, and the false proof
 * it gives.
 *
 * Copyright (c) 2026 ENSTA, France
 *
 * Created 2026-09-27 by Jordan NININ
 *--------------------------------------------------------------------------
 * gaol is a software distributed WITHOUT ANY WARRANTY. Read the associated
 * COPYING file for information.
 *--------------------------------------------------------------------------*/

#include <array>
#include <cmath>
#include <cstddef>
#include <cstdlib>
#include <iostream>
#include <string>
#include <utility>
#include <vector>

#include <gaol/gaol.h>

#include "box.h"
#include "dual.h"

using namespace gaol;
using examples::Box;
using Dual2 = examples::Dual<2>;

namespace {

  bool all_checks_hold = true;

  // Each claim of the output is checked: a claim that does not hold is
  // reported, and the program fails
  void check(bool ok, const std::string& what)
  {
    if (!ok) {
      std::cout << "FAILED: " << what << "\n";
      all_checks_hold = false;
    }
  }

  // The systems are written once, as templates: with T = interval they give
  // the natural interval extension of f, with T = Dual2 its Jacobian matrix
  // as well. The constants are doubles, 1.0 rather than 1, as in the other
  // examples.
  template <class T>
  std::array<T, 2> circles(const T& x, const T& y)
  {
    // sqr(x) rather than x * x: the product does not know that both of its
    // factors are the same x, and x * x is [-100, 100] over [-10, 10], where
    // sqr(x) is [0, 100]
    return { sqr(x) + sqr(y) - 1.0, sqr(x - 1.0) + sqr(y) - 1.0 };
  }

  // lambda + exp(-lambda) with lambda = x + i y: exp(-lambda) is
  // exp(-x) (cos(y) - i sin(y)). Codac's Evans function, with its delays 1
  // and 2, is lambda^2 + 2 lambda exp(-lambda) + exp(-2 lambda), the square
  // of this one: the same roots, but double ones, where the Jacobian is
  // singular and no Krawczyk test can succeed. They are proved here as the
  // simple roots of lambda + exp(-lambda).
  template <class T>
  std::array<T, 2> lambert(const T& x, const T& y)
  {
    const T e = exp(-x);
    return { x + e * cos(y), y - e * sin(y) };
  }

  using Matrix = std::array<std::array<interval, 2>, 2>;

  // An enclosure of the Jacobian matrix of f over X: f evaluated on the dual
  // numbers (X[0], (1, 0)) and (X[1], (0, 1)); row i holds the enclosures of
  // the partial derivatives of f_i over the whole of X
  template <class F>
  Matrix jacobian(const F& f, const Box& X)
  {
    const std::array<Dual2, 2> r = f(Dual2::variable(X[0], 0), Dual2::variable(X[1], 1));
    return { { { r[0].d[0], r[0].d[1] }, { r[1].d[0], r[1].d[1] } } };
  }

  // K is interior to X, component by component
  bool interior(const Box& K, const Box& X)
  {
    return X[0].set_strictly_contains(K[0]) && X[1].set_strictly_contains(K[1]);
  }

  // The Krawczyk operator K(X) = m - C f(m) + (I - C J(X)) (X - m), J(X)
  // being given. Returns false, leaving K alone, when the middle of J(X) has
  // no inverse.
  //
  // A solution s of f in X is a fixed point of s - C f(s), and the
  // mean-value theorem gives f(s) = f(m) + A (s - m) for a matrix A whose
  // entries lie in those of J(X): s = m - C f(m) + (I - C A) (s - m) is in
  // K(X). Hence X & K(X) holds every solution that X holds, and K(X) empty
  // proves that X holds none. If K(X) is inside the interior of X, then
  // x - C f(x) maps X into itself and has a fixed point there (Brouwer's
  // theorem); C and every matrix of J(X) are then regular, so that the
  // fixed point is a solution, and the only one in X: R. Krawczyk,
  // Computing 4, 1969; R. E. Moore, SIAM J. Numer. Anal. 14, 1977.
  template <class F>
  bool krawczyk(const F& f, const Matrix& J, const Box& X, Box& K)
  {
    // m is a box of intervals, the tightest ones around the middle of X,
    // and f(m) is computed with intervals, so that it encloses the true
    // value of f at the middle
    const Box m = X.mid();
    const std::array<interval, 2> fm = f(m[0], m[1]);
    // C is the inverse of the middle of J(X), computed in doubles, and
    // rounded upward as every double of the program is until
    // gaol::cleanup() (unless GAOL is built with GAOL_PRESERVE_ROUNDING).
    // It does not matter: the operator encloses the solutions whatever C
    // is; a C close to the inverse only makes K(X) small, and the proof
    // possible.
    const double a = J[0][0].midpoint(), b = J[0][1].midpoint();
    const double c = J[1][0].midpoint(), d = J[1][1].midpoint();
    const double det = a * d - b * c;
    const double C[2][2] = { { d / det, -b / det }, { -c / det, a / det } };
    for (const auto& row : C) {
      for (double x : row) {
        if (!std::isfinite(x)) {
          return false;
        }
      }
    }
    // Every product below mixes a double of C with an interval: GAOL rounds
    // it outward, and K encloses the exact operator for this C
    K = Box(2);
    for (std::size_t i = 0; i < 2; ++i) {
      interval k = m[i] - (C[i][0] * fm[0] + C[i][1] * fm[1]);
      for (std::size_t j = 0; j < 2; ++j) {
        const interval identity(i == j ? 1.0 : 0.0);
        k += (identity - (C[i][0] * J[0][j] + C[i][1] * J[1][j])) * (X[j] - m[j]);
      }
      K[i] = k;
    }
    return true;
  }

  // A box the solver returns, with its status
  struct Solution {
    Box X;
    bool unique;   // X holds exactly one solution: proved
  };

  // Branch and prune: all the solutions of f(x, y) = 0 in X0, the boxes
  // being cut at the fraction ratio of their widest side. Returns the boxes
  // proved to hold one solution, and the boxes narrower than tolerance that
  // no test decided.
  template <class F>
  std::vector<Solution> solve(const F& f, const Box& X0, double ratio, double tolerance, long& examined)
  {
    std::vector<Box> work{ X0 };
    std::vector<Solution> solutions;
    examined = 0;
    while (!work.empty()) {
      Box X = work.back();
      work.pop_back();
      ++examined;
      bool unique = false, none = false;
      for (int step = 0; step < 50; ++step) {
        // Exclusion: F(X) encloses the range of each f_i over X; without 0
        // in one of them, X holds no solution
        const std::array<interval, 2> FX = f(X[0], X[1]);
        if (!FX[0].set_contains(0.0) || !FX[1].set_contains(0.0)) {
          none = true;
          break;
        }
        Box K;
        if (!krawczyk(f, jacobian(f, X), X, K)) {
          break;
        }
        unique = unique || interior(K, X);
        const Box Y = X & K;   // every solution in X is in K(X)
        if (Y.is_empty()) {
          none = true;
          break;
        }
        // Once a solution is proved, the operator converges quadratically:
        // go on until X no longer changes (set_eq(), GAOL having no == on
        // intervals). Otherwise stop when it makes too little progress.
        const bool fixpoint = Y[0].set_eq(X[0]) && Y[1].set_eq(X[1]);
        const bool progress = Y.max_width() < 0.9 * X.max_width();
        X = Y;
        if (fixpoint || (!unique && !progress)) {
          break;
        }
      }
      if (none) {
        continue;
      }
      if (unique || X.max_width() <= tolerance) {
        solutions.push_back({ X, unique });
        continue;
      }
      // Bisection of the widest side (see cut_in_the_middle() for the ratio)
      const std::pair<Box, Box> halves = X.bisect(ratio);
      work.push_back(halves.second);
      work.push_back(halves.first);
    }
    return solutions;
  }

  int count_proved(const std::vector<Solution>& found)
  {
    int proved = 0;
    for (const Solution& s : found) {
      proved += s.unique ? 1 : 0;
    }
    return proved;
  }

  // The number of boxes of found that hold the point z, a box of intervals
  // enclosing one number each
  int holding(const std::vector<Solution>& found, const Box& z)
  {
    int n = 0;
    for (const Solution& s : found) {
      n += s.X.contains(z) ? 1 : 0;
    }
    return n;
  }

  // Solves f = 0 on X0, prints the boxes found, and checks that each
  // reference solution (x, y) lies in exactly one of them, that each box
  // proved unique holds exactly one reference solution, and that all the
  // reference solutions are proved
  template <class F>
  void run(const std::string& title, const std::string& equations, const F& f, const Box& X0,
           const std::vector<Box>& reference, const std::string& reference_text)
  {
    long examined = 0;
    const std::vector<Solution> found = solve(f, X0, 0.45, 1e-10, examined);
    const int proved = count_proved(found);
    // The boxes the solver does not return were discarded by a test that
    // proves they hold no solution: with no undecided box, the solutions
    // proved are all those of the domain
    std::cout << title << ": " << equations << " on " << X0 << "\n"
              << "  " << examined << " boxes examined, " << proved << " solutions proved unique"
              << (static_cast<int>(found.size()) == proved ? ", none elsewhere:\n" : ":\n");
    for (const Solution& s : found) {
      std::cout << "  " << s.X << (s.unique ? "  unique" : "  undecided") << "\n";
    }
    std::cout << "  " << reference_text << " (mpmath): one in each box\n";

    for (const Box& z : reference) {
      int meeting = 0;
      for (const Solution& s : found) {
        meeting += (!s.X[0].set_disjoint(z[0]) && !s.X[1].set_disjoint(z[1])) ? 1 : 0;
      }
      check(meeting == 1 && holding(found, z) == 1,
            title + ": a solution of mpmath is not in exactly one box");
    }
    for (const Solution& s : found) {
      int held = 0;
      for (const Box& z : reference) {
        held += s.X.contains(z) ? 1 : 0;
      }
      check(!s.unique || held == 1, title + ": a box proved unique does not hold one solution of mpmath");
    }
    check(proved == static_cast<int>(reference.size()) && found.size() == reference.size(),
          title + ": not every solution is proved, or a box is undecided");
  }

  // Why the solver cuts a box at 45% of its width, as IBEX does by default
  // (Codac at 49%), rather than in the middle. On x, the first circle lies
  // in [-1, 1] and the second in [0, 2], and both lie in [-1, 1] on y: a
  // contractor, such as the forward-backward one of IBEX's doc-solver,
  // restricts [-10, 10]^2 to [0, 1] x [-1, 1] at once. Cut in the middle,
  // that box gives [0, 1] x [0, 1], then the cut x = 1/2, the x of both
  // solutions: each of them lies on the border of two boxes, in neither
  // interior, and the Krawczyk test, which asks for K(X) inside the
  // interior of X, cannot prove either. Each solution comes out as two
  // undecided boxes.
  void cut_in_the_middle(const std::vector<Box>& reference)
  {
    const auto f = [](const auto& x, const auto& y) { return circles(x, y); };
    const Box X0{ interval(0.0, 1.0), interval(-1.0, 1.0) };
    long examined = 0;
    const std::vector<Solution> middle = solve(f, X0, 0.5, 1e-10, examined);
    const std::vector<Solution> ibex = solve(f, X0, 0.45, 1e-10, examined);
    std::cout << "  from " << X0 << ", the box IBEX's contractor would give at once:\n"
              << "    cut at 50%: " << middle.size() << " boxes, " << count_proved(middle)
              << " proved, the solutions lie on the cut x = 1/2\n"
              << "    cut at 45%: " << ibex.size() << " boxes, " << count_proved(ibex) << " proved\n";
    check(count_proved(middle) == 0, "a solution on a cut was proved");
    for (const Box& z : reference) {
      check(holding(middle, z) == 2, "a solution on the cut x = 1/2 is not in two boxes");
      check(holding(ibex, z) == 1, "a solution is not in one box when cutting at 45%");
    }
    check(count_proved(ibex) == 2, "cutting at 45% does not prove both solutions");
  }

  // The Jacobian of lambert() as the review of GAOL v5 first wrote it by
  // hand: d(x + exp(-x) cos(y))/dy is -exp(-x) sin(y), written here with
  // the wrong sign. It no longer encloses the true Jacobian, and the
  // Krawczyk operator built on it no longer encloses the solutions.
  Matrix lambert_jacobian_by_hand(const Box& X)
  {
    const interval e = exp(-X[0]), c = cos(X[1]), s = sin(X[1]);
    return { { { 1.0 - e * c, e * s }, { e * s, 1.0 - e * c } } };
  }

  void wrong_jacobian(const Box& root)
  {
    const auto f = [](const auto& x, const auto& y) { return lambert(x, y); };
    // A small box around W_0(-1), read from text: X is the tightest box of
    // doubles around these decimal bounds, none of which is a double, and
    // printed with 16 digits it would show -0.3181320000000001; the text is
    // printed instead
    const std::string text = "[-0.318132, -0.318131] x [1.337235, 1.337236]";
    const Box X{ textToInterval("[-0.318132, -0.318131]"), textToInterval("[1.337235, 1.337236]") };
    Box K_hand, K_ad;
    const bool hand = krawczyk(f, lambert_jacobian_by_hand(X), X, K_hand);
    const bool ad = krawczyk(f, jacobian(f, X), X, K_ad);
    std::cout << "A Jacobian written by hand with a sign error, on X = " << text << ":\n"
              << "  K(X) = " << K_hand << "\n"
              << "    inside X: a false proof, W_0(-1) is not in K(X)\n"
              << "  with the Jacobian of Dual<2>:\n"
              << "  K(X) = " << K_ad << "\n"
              << "    inside X, and W_0(-1) is in K(X)\n";
    check(hand && interior(K_hand, X), "the wrong Jacobian does not give its false proof");
    check(K_hand[0].set_disjoint(root[0]) || K_hand[1].set_disjoint(root[1]),
          "the false proof of the wrong Jacobian holds W_0(-1)");
    check(ad && interior(K_ad, X) && K_ad.contains(root), "the Jacobian of Dual<2> does not prove W_0(-1)");
  }

} // namespace

int main()
{
  // 16 significant digits. GAOL writes the bounds rounded outward: the
  // printed box encloses the computed one.
  interval::precision(16);

  // IBEX's two circles. The references are exact but for sqrt(3)/2, which
  // mpmath gives with 40 digits and textToInterval("...") encloses.
  const interval x_star(0.5), y_star = textToInterval("0.8660254037844386467637231707529361834714");
  const std::vector<Box> intersections{ Box{ x_star, -y_star }, Box{ x_star, y_star } };
  run("IBEX's doc-solver.cpp", "x^2 + y^2 = 1 and (x - 1)^2 + y^2 = 1",
      [](const auto& x, const auto& y) { return circles(x, y); },
      Box{ interval(-10.0, 10.0), interval(-10.0, 10.0) }, intersections,
      "(1/2, -sqrt(3)/2) and (1/2, sqrt(3)/2)");
  cut_in_the_middle(intersections);

  // Codac's Evans function. W_k(-1) by mpmath.lambertw(-1, k) with 40
  // digits; W_3(-1) = -3.02 + 20.27 i lies beyond y = 20
  const Box w0{ textToInterval("-0.3181315052047641353126542515876645172035"),
                textToInterval("1.337235701430689408901162143193710612540") };
  const Box w1{ textToInterval("-2.062277729598283884978486720008045951284"),
                textToInterval("7.588631178472512622568923954107584383013") };
  const Box w2{ textToInterval("-2.653191974038697286601106643318049074593"),
                textToInterval("13.94920833453321445528891803900272649216") };
  run("Codac's main_evans.cpp", "x + i y + exp(-(x + i y)) = 0",
      [](const auto& x, const auto& y) { return lambert(x, y); },
      Box{ interval(-10.0, 10.0), interval(0.0, 20.0) }, { w0, w1, w2 }, "W_0(-1), W_1(-1) and W_2(-1)");

  wrong_jacobian(w0);

  // GAOL leaves the rounding upward for the whole program until cleanup(),
  // called right after the last use of GAOL
  gaol::cleanup();
  if (!all_checks_hold) {
    return EXIT_FAILURE;
  }
  std::cout << "All the checks hold.\n";
  return 0;
}
