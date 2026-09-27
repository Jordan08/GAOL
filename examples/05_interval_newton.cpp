/*-*-C++-*------------------------------------------------------------------
 * gaol -- NOT Just Another Interval Library
 *--------------------------------------------------------------------------
 * Examples of GAOL v5: interval Newton, enclosing and proving the roots of
 * a function of one variable.
 *
 * The interval Newton operator N(X) = m - f(m) / F'(X), m being the middle
 * of X and F'(X) an enclosure of f' over X, holds every root of f that X
 * holds (the mean-value theorem): X & N(X) is a smaller interval with the
 * same roots, and N(X) inside the interior of X proves that X holds exactly
 * one root. Floating-point Newton finds a root; interval Newton encloses it
 * between two doubles and proves that it exists and is unique.
 *
 * The example computes (a) the root of sqrt(x) + (x + 1) cos(x) in [2, 3],
 * a port of C-XSC's examples/inewton.cpp (C-XSC 2.5.4, whose listing ends
 * with the enclosure [2.059045253415142, 2.059045253415145]); (b) all the
 * roots of sin(x) - x/10 in [-10, 10], seven of them, and (c) both roots of
 * x^2 - 2 in [-3, 3], with a worklist of intervals, bisection, and the
 * extended Newton operator where F'(X) holds 0, as C-XSC's
 * examples/allzeros.cpp (the AllZeros of its C++ Toolbox), filib++'s
 * xinewton examples and IBEX's CtcNewton do, following E. Hansen and
 * G. W. Walster, "Global Optimization Using Interval Analysis", 2nd ed.,
 * Marcel Dekker, 2004, chapter 9. It shows why the division of f(m) by an
 * F'(X) holding 0 must be the relational one (div_rel, or %), not the
 * functional /, which gives a false proof there. The derivative comes from
 * the automatic differentiation of dual.h rather than from a formula
 * written by hand, which would have to enclose the true derivative for the
 * proofs to hold.
 *
 * Copyright (c) 2026 ENSTA, France
 *
 * Created 2026-09-27 by Jordan NININ
 *--------------------------------------------------------------------------
 * gaol is a software distributed WITHOUT ANY WARRANTY. Read the associated
 * COPYING file for information.
 *--------------------------------------------------------------------------*/

#include <cstdlib>
#include <iomanip>
#include <iostream>
#include <string>
#include <utility>
#include <vector>

#include <gaol/gaol.h>

#include "dual.h"

using namespace gaol;
using examples::Dual;

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

  // An enclosure of f' over X: f evaluated on the dual number (X, 1), whose
  // derivative part the chain rule carries through every operation of f
  template <class F>
  interval derivative(const F& f, const interval& X)
  {
    return f(Dual<1>::variable(X, 0)).d[0];
  }

  // An interval the search returns, with its status
  struct Root {
    interval X;
    bool unique;   // X holds exactly one root: proved
  };

  // One step of the extended Newton operator: the parts of X where f can
  // vanish, as zero, one or two intervals in increasing order. unique is
  // set when the step proves that X holds exactly one root.
  template <class F>
  std::vector<interval> newton_step(const F& f, const interval& X, bool& unique)
  {
    // X.mid() is an interval, the tightest one around the middle of X (a
    // point when the middle is a double); X.midpoint() is that double, the
    // other way round from IBEX and Codac, whose mid() is the point. f(m) is
    // thus computed with intervals: evaluated in doubles, it would be
    // rounded, and the Newton step would no longer enclose the roots.
    const interval m = X.mid();
    const interval fm = f(m);
    const interval D = derivative(f, X);
    unique = false;
    if (!D.set_contains(0.0)) {
      // The ordinary operator: D holds no 0, so the functional division is
      // right, and f is monotonic over X, with one root at most. A root x
      // of f in X is x = m - f(m) / f'(xi) for some xi in X, hence in N(X).
      // N(X) in the interior of X proves that the root exists; N(X) & X
      // encloses it.
      const interval N = m - fm / D;
      unique = X.set_strictly_contains(N);
      const interval P = X & N;
      return P.is_empty() ? std::vector<interval>{} : std::vector<interval>{ P };
    }
    // D holds 0, and the functional division f(m) / D is of no use. It is
    // the set of the quotients a / b with b other than 0 (IEEE 1788-2015):
    // [0] when f(m) is [0], which loses every root (see false_proof()
    // below), and otherwise, 0 being inside D, the whole line: the hull of
    // two unbounded pieces, the gap between them lost. What a root x of f
    // in X satisfies is a relation: (x - m) d = -f(m) for some d in D, x - m
    // being in X - m. The relational division div_rel(-f(m), D, X - m)
    // (mulRev of IEEE 1788-2015; -f(m) % D is the same without X - m) keeps
    // the t of X - m such that t d = z for some d in D and some z in -f(m):
    // all of X - m when d and z can both be 0, rightly. It returns the hull
    // of these t, though, and over the whole of D that hull would fill the
    // gap around t = 0 again. Dividing by the negative part and by the
    // positive part of D apart keeps the gap, as the two-piece division of
    // IEEE 1788-2015, mulRevToPair, which GAOL does not provide, would.
    const interval a = (m + div_rel(-fm, D & interval::negative(), X - m)) & X;
    const interval b = (m + div_rel(-fm, D & interval::positive(), X - m)) & X;
    if (a.is_empty() || b.is_empty()) {
      const interval& P = a.is_empty() ? b : a;
      return P.is_empty() ? std::vector<interval>{} : std::vector<interval>{ P };
    }
    if (!a.set_disjoint(b)) {
      return { a | b };
    }
    return a.left() < b.left() ? std::vector<interval>{ a, b } : std::vector<interval>{ b, a };
  }

  // Cuts X at 45% of its width, as IBEX's bisectors and Box::bisect() of
  // box.h do: a cut in the middle of [-10, 10] would fall on the root 0,
  // which would then lie on the border of both halves, where Newton cannot
  // prove it (the proof asks for N(X) inside the interior of X); the search
  // would return two undecided intervals, [-1e-323, 0] and [0, 1e-323].
  // c is a double of the program, rounded upward as all of them are until
  // gaol::cleanup() (unless GAOL is built with GAOL_PRESERVE_ROUNDING): it
  // is a little off 45%, which does not matter, any cut inside X being
  // right.
  std::pair<interval, interval> bisect(const interval& X)
  {
    double c = X.left() + 0.45 * (X.right() - X.left());
    if (!(X.left() < c && c < X.right())) {
      c = X.midpoint();
    }
    return { interval(X.left(), c), interval(c, X.right()) };
  }

  // All the roots of f in X0, with a worklist of intervals: an interval
  // over which f cannot vanish (0 not in F(X)) is discarded; the others are
  // contracted by Newton steps, split in two when the extended operator
  // leaves a gap, and bisected when Newton makes too little progress.
  // Returns the intervals proved to hold one root, and the intervals of
  // width below tolerance that no test decided.
  template <class F>
  std::vector<Root> all_roots(const F& f, const interval& X0, double tolerance, int& examined)
  {
    std::vector<interval> work{ X0 };
    std::vector<Root> roots;
    examined = 0;
    while (!work.empty()) {
      interval X = work.back();
      work.pop_back();
      ++examined;
      bool unique = false, split = false;
      for (int step = 0; step < 100; ++step) {
        // F(X) encloses the range of f over X: without 0, no root in X
        if (!f(X).set_contains(0.0)) {
          X = interval::emptyset();
          break;
        }
        bool proved = false;
        const std::vector<interval> N = newton_step(f, X, proved);
        unique = unique || proved;
        if (N.empty()) {   // no root in X
          X = interval::emptyset();
          break;
        }
        if (N.size() == 2) {   // a gap without root in the middle of X
          work.push_back(N[1]);
          work.push_back(N[0]);
          split = true;
          break;
        }
        // Once a root is proved, Newton converges quadratically: go on
        // until X no longer changes. set_eq() is the equality of two sets;
        // GAOL has no == on intervals.
        const bool fixpoint = N[0].set_eq(X);
        const bool progress = N[0].width() < 0.9 * X.width();
        X = N[0];
        if (fixpoint || (!unique && !progress)) {
          break;
        }
      }
      if (split || X.is_empty()) {
        continue;
      }
      if (unique || X.width() <= tolerance) {
        roots.push_back({ X, unique });
        continue;
      }
      const std::pair<interval, interval> halves = bisect(X);
      work.push_back(halves.second);
      work.push_back(halves.first);
    }
    return roots;
  }

  // Prints the intervals returned, and checks that each reference root lies
  // in exactly one of them and that each interval proved unique holds
  // exactly one reference root
  void report(const std::vector<Root>& found, const std::vector<interval>& reference, const std::string& name)
  {
    // The intervals are aligned as text: std::string(X) writes X as << does
    for (const Root& r : found) {
      std::cout << "    " << std::left << std::setw(42) << std::string(r.X) << std::right
                << (r.unique ? "one root, proved" : "undecided") << "\n";
    }
    for (const interval& z : reference) {
      int holding = 0;
      bool contained = false;
      for (const Root& r : found) {
        if (!r.X.set_disjoint(z)) {
          ++holding;
          contained = r.X.set_contains(z);
        }
      }
      check(holding == 1 && contained, name + ": a root of mpmath is not in exactly one interval");
    }
    for (const Root& r : found) {
      int held = 0;
      for (const interval& z : reference) {
        held += r.X.set_contains(z) ? 1 : 0;
      }
      check(!r.unique || held == 1, name + ": an interval proved unique does not hold one root of mpmath");
    }
  }

  // (a) C-XSC's examples/inewton.cpp
  void cxsc_inewton()
  {
    // f is written once, for intervals and for the dual numbers of its
    // derivative. C-XSC writes deriv(x) by hand.
    const auto f = [](const auto& x) { return sqrt(x) + (x + 1.0) * cos(x); };
    // The root, by mpmath's findroot with 60 digits; interval("...")
    // encloses the decimal number, which no double is
    const interval root("2.059045253415143788680636155343254522623");

    interval X(2.0, 3.0);
    std::cout << "(a) sqrt(x) + (x + 1) cos(x) = 0 on " << X << ", C-XSC's inewton.cpp\n";

    // C-XSC's criterion: f(2) f(3) < 0, a root by the intermediate value
    // theorem, and 0 not in f'(X), one root at most. f is evaluated on the
    // point intervals [2] and [3], so that its values are enclosed. < is
    // "certainly less": every point of the product is negative. The empty
    // set passes every such test, having no point that fails it: hence
    // is_empty() first (sqrt of a negative number would give it). The
    // product is compared with 0.0, not 0: where an interval is expected,
    // the literal 0 is also a null pointer, which interval(const char*)
    // takes, and p < 0 does not compile. C-XSC writes the second test
    // !(0.0 <= deriv(x)), <= meaning "is an element of" there; in GAOL, <=
    // is "certainly less or equal", and membership is set_contains().
    const interval p = f(interval(X.left())) * f(interval(X.right()));
    const interval D = derivative(f, X);
    const bool sign_change = !p.is_empty() && p < 0.0;
    const bool monotonic = !D.set_contains(0.0);
    std::cout << "    f(2) f(3) < 0: X holds a root (C-XSC's test)\n"
              << "    F'(X) = " << D << " does not hold 0: only one\n";
    check(sign_change && monotonic, "C-XSC's criterion does not hold on [2, 3]");

    // C-XSC's iteration, X = (m - f(m) / f'(X)) & X, until X no longer
    // changes. C-XSC's mid(x) is a real, which its f turns into a point
    // interval; GAOL's X.mid() is that interval already. & is the
    // intersection in both libraries.
    interval previous;
    bool proved = false;
    int k = 0;
    do {
      previous = X;
      const interval m = X.mid();
      const interval N = m - f(m) / derivative(f, X);
      const bool interior = X.set_strictly_contains(N);
      std::cout << "    X" << k << " = " << X;
      std::cout << (interior && !proved ? "   N(X) inside X: one root, proved\n" : "\n");
      proved = proved || interior;
      X = N & X;
      ++k;
    } while (!X.set_eq(previous));
    std::cout << "    X no longer changes: the root 2.0590452534151437887 (mpmath) is in X" << k - 1 << "\n";
    check(proved, "N(X) was never inside X in (a)");
    check(X.set_contains(root), "the root of (a) is not in the final interval");
    check(X.width() < 1e-14, "the final interval of (a) is not a few doubles wide");
  }

  // The trap: Newton with the functional division where F'(X) holds 0
  void false_proof()
  {
    const auto f = [](const auto& x) { return sin(x) - x / 10.0; };
    const interval X(-10.0, 10.0);
    const interval m = X.mid(), fm = f(m), D = derivative(f, X);
    // f(0) = 0 exactly, F'(X) = cos(X) - 1/10 = [-1.1, 0.9]. The division /
    // of IEEE 1788 is the set of the a / b with b other than 0: 0 / D is
    // [0], and the "Newton operator" is [0], inside X, which the test reads
    // as a proof that X holds exactly one root. It holds seven. The true
    // relation is (x - m) f'(xi) = -f(m) for some xi: with f'(xi) = 0 and
    // f(m) = 0 every x satisfies it, and the relational division, here the
    // operator %, says so. GAOL prints the sign of a zero bound, and a point
    // interval as <a, b>.
    const interval naive = m - fm / D;
    const interval relational = m - fm % D;
    std::cout << "    X = " << X << ", f(m) = " << fm << ", F'(X) = " << D << " holds 0\n"
              << "    m - f(m) / F'(X) = " << naive << ": inside X, a false proof of a single root\n"
              << "    m - f(m) % F'(X) = " << relational << ": no information, X is bisected\n";
    check(fm.set_contains(0.0) && D.set_contains(0.0), "f(m) or F'(X) does not hold 0 on [-10, 10]");
    check(X.set_strictly_contains(naive), "the functional division does not give its false proof");
    check(relational.is_entire(), "the relational division is not the whole line");
  }

  // (b) All the roots of sin(x) - x/10 in [-10, 10]
  void sine_roots()
  {
    // x / 10.0 divides by 10, which is a double: GAOL rounds the quotient
    // outward, and it encloses x/10. x * 0.1 would multiply by the double
    // nearest 1/10, not by 1/10 (interval(0.1) holds that double only), and
    // f would be another function.
    const auto f = [](const auto& x) { return sin(x) - x / 10.0; };
    std::cout << "(b) sin(x) - x/10 = 0 on [-10, 10]: all the roots\n";
    false_proof();

    int examined = 0;
    const std::vector<Root> found = all_roots(f, interval(-10.0, 10.0), 1e-12, examined);
    int proved = 0;
    for (const Root& r : found) {
      proved += r.unique ? 1 : 0;
    }
    // The intervals the search does not return were discarded by a test
    // that proves they hold no root: with no undecided interval, the roots
    // proved are all the roots of f in [-10, 10]
    std::cout << "    worklist: " << examined << " intervals examined, " << proved << " roots proved"
              << (static_cast<int>(found.size()) == proved ? ", none elsewhere:\n" : ":\n");
    // The roots, by mpmath (a sign change on a grid of 20000 points, then
    // findroot with 60 digits); f is odd, and the negation of an interval
    // is exact
    const interval r1("2.852341894450091648325219940702758448067");
    const interval r2("7.068174358095817395977019713295141723188");
    const interval r3("8.423203932360491733611069596154942218252");
    report(found, { -r3, -r2, -r1, interval(0.0), r1, r2, r3 }, "(b)");
    check(proved == 7 && found.size() == 7, "(b) did not prove its 7 roots");
  }

  // (c) Both roots of x^2 - 2 in [-3, 3]
  void square_roots()
  {
    // sqr(x) rather than x * x: [-3, 3] * [-3, 3] is [-9, 9], the product
    // not knowing that both factors are the same x, and sqr([-3, 3]) is
    // [0, 9]
    const auto f = [](const auto& x) { return sqr(x) - 2.0; };
    const interval X(-3.0, 3.0);
    bool unique = false;
    const std::vector<interval> first = newton_step(f, X, unique);
    std::cout << "(c) x^2 - 2 = 0 on " << X << ": F'(X) = " << derivative(f, X) << " holds 0\n";
    check(first.size() == 2, "the first step of (c) does not leave a gap");
    if (first.size() == 2) {
      std::cout << "    one extended step keeps " << first[0] << " and " << first[1] << "\n";
    }

    int examined = 0;
    const std::vector<Root> found = all_roots(f, X, 1e-12, examined);
    std::cout << "    worklist: " << examined << " intervals examined, both roots proved, none elsewhere:\n";
    const interval sqrt2("1.414213562373095048801688724209698078570");   // mpmath
    report(found, { -sqrt2, sqrt2 }, "(c)");
    check(found.size() == 2 && found[0].unique && found[1].unique, "(c) did not prove its 2 roots");
  }

} // namespace

int main()
{
  // 16 significant digits. GAOL writes the bounds rounded outward: the
  // printed interval encloses the computed one.
  interval::precision(16);

  cxsc_inewton();
  sine_roots();
  square_roots();

  // GAOL leaves the rounding upward for the whole program until cleanup(),
  // called right after the last use of GAOL
  gaol::cleanup();
  if (!all_checks_hold) {
    return EXIT_FAILURE;
  }
  std::cout << "All the checks hold.\n";
  return 0;
}
