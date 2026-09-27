/*-*-C++-*------------------------------------------------------------------
 * gaol -- NOT Just Another Interval Library
 *--------------------------------------------------------------------------
 * Examples of GAOL v5: global optimization by branch and bound.
 *
 * Computes a guaranteed enclosure [lower, upper] of the global minimum f* of
 * two classic test functions, the Goldstein-Price function on [-2, 2]^2
 * (f* = 3 at (0, -1)) and the six-hump camel on [-3, 3] x [-2, 2]
 * (f* = -1.0316284534898774 at (0.0898, -0.7126) and its opposite), by the
 * Moore-Skelboe algorithm: the boxes wait in a priority queue ordered by the
 * lower bound of f over them; the box with the smallest lower bound is
 * bisected; f evaluated at a point gives an upper bound of f*, which
 * discards every box whose lower bound is above it. The smallest lower
 * bound in the queue is a lower bound of f* over the whole domain, so that
 * the enclosure of f* is proved, not estimated.
 *
 * The example compares three ways of bounding f over a box: the natural
 * extension (f evaluated with intervals), whose overestimation, the
 * dependency problem, only decreases linearly with the width of the box, so
 * that the algorithm has to bisect a cloud of tiny boxes around the
 * minimizer (the cluster effect); the mean-value form f(m) + grad f(X).(X-m)
 * intersected with the natural extension, whose overestimation is quadratic
 * in the width; and the same with the monotonicity test, which discards a
 * box over which a component of the gradient does not vanish. The gradient
 * comes from the forward automatic differentiation of dual.h, and
 * 04_centered_form.cpp compares the two forms on single boxes.
 *
 * It follows the textbook algorithm of E. Hansen and G. W. Walster, "Global
 * Optimization Using Interval Analysis", 2nd ed., Marcel Dekker, 2004, and
 * of H. Ratschek and J. Rokne, "New Computer Methods for Global
 * Optimization", Ellis Horwood, 1988, which IBEX's global optimizer
 * (examples/doc-optim.cpp, DefaultOptimizer) refines with contractors and
 * local searches.
 *
 * Copyright (c) 2026 ENSTA, France
 *
 * Created 2026-09-27 by Jordan NININ
 *--------------------------------------------------------------------------
 * gaol is a software distributed WITHOUT ANY WARRANTY. Read the associated
 * COPYING file for information.
 *--------------------------------------------------------------------------*/

#include <cstddef>
#include <cstdlib>
#include <iomanip>
#include <iostream>
#include <queue>
#include <vector>

#include <gaol/gaol.h>

#include "box.h"
#include "dual.h"

using namespace gaol;
using examples::Box;
using Dual2 = examples::Dual<2>;

namespace {

  // The functions are written once, as templates: with T = interval they
  // give the natural interval extension, with T = Dual2 the enclosures of
  // the gradient as well. The constants are doubles, 19.0 rather than 19:
  // the operators would take the int, but an interval cannot be built from
  // the literal 0 (interval(0) is ambiguous, 0 being also a null pointer,
  // the text of an interval), and doubles everywhere avoid the question.
  template <class T>
  T goldstein_price(const T& x, const T& y)
  {
    const T a = 1.0 + sqr(x + y + 1.0) *
                        (19.0 - 14.0 * x + 3.0 * sqr(x) - 14.0 * y + 6.0 * x * y + 3.0 * sqr(y));
    const T b = 30.0 + sqr(2.0 * x - 3.0 * y) *
                         (18.0 - 32.0 * x + 12.0 * sqr(x) + 48.0 * y - 36.0 * x * y + 27.0 * sqr(y));
    return a * b;
  }

  template <class T>
  T six_hump_camel(const T& x, const T& y)
  {
    // 2.1 has no double: the double 2.1 is another number, and would make f
    // another function. Read from its text, the interval encloses 21/10.
    static const interval c21("2.1");
    // pow(x, 4) is gaol::pow for intervals, found through "using namespace
    // gaol" (argument-dependent lookup does not find it), and Dual's own pow
    // for Dual2. sqr(x) is tighter than x * x: [-1, 1] * [-1, 1] = [-1, 1],
    // the product not knowing that both factors are the same x.
    return (4.0 - c21 * sqr(x) + pow(x, 4) / 3.0) * sqr(x) + x * y + (-4.0 + 4.0 * sqr(y)) * sqr(y);
  }

  enum class Form { natural, mean_value, monotonicity };

  // A box of the search, with the enclosure of f over it and the enclosure
  // of f at its midpoint
  struct Node {
    Box box;
    interval range;
    interval at_mid;
  };

  // The order of the priority queue. It must be a strict weak ordering, and
  // the intervals' own a.range > b.range is not one: it is the relation
  // "certainly greater" (every point of a above every point of b), under
  // which two overlapping ranges are "equivalent" without this equivalence
  // being transitive. The boxes are ordered by the lower bound of f over
  // them, a double; std::priority_queue puts on top the greatest element
  // for its comparator, hence the ">" to have the smallest lower bound on
  // top.
  struct LargerLowerBound {
    bool operator()(const Node& a, const Node& b) const { return a.range.left() > b.range.left(); }
  };

  // Encloses f over n.box in the chosen form. With the monotonicity test, it
  // returns false when the box holds no global minimizer, and may reduce the
  // box to one of its faces.
  template <class F>
  bool bound(const F& f, Form form, const Box& domain, Node& n)
  {
    Box& X = n.box;
    // X.mid() encloses the midpoint of X: each side's mid() is the interval
    // around the real (a+b)/2, a point, or two consecutive doubles when
    // (a+b)/2 needs one bit more than a double has (midpoint() is a double
    // near it). f is evaluated over it with intervals: f computed with
    // doubles would be rounded, and could fall below the true value.
    Box m = X.mid();
    if (form == Form::natural) {
      n.range = f(X[0], X[1]);
      n.at_mid = f(m[0], m[1]);
      return true;
    }
    // One evaluation over Dual2 gives both the natural extension, r.v, and
    // enclosures r.d[i] of the partial derivatives over the whole box.
    Dual2 r = f(Dual2::variable(X[0], 0), Dual2::variable(X[1], 1));
    if (form == Form::monotonicity) {
      // If df/dx_i > 0 all over X, f grows with x_i: its minimum over X lies
      // on the face x_i = left. If that face is inside the domain, a step
      // left of any of its points, still in the domain, lowers f: X holds
      // no global minimizer. Only a face on the border of the domain can
      // hold one (and symmetrically for a negative derivative). r.d[i] is
      // never empty here, f being a polynomial; for a function not defined
      // everywhere, test is_empty() first: the empty set passes every
      // certainly-test, having no point that fails it.
      bool reduced = true;
      while (reduced) {
        reduced = false;
        for (std::size_t i = 0; i < 2; ++i) {
          if (X[i].is_a_double()) {
            continue;
          }
          if (r.d[i].certainly_strictly_positive()) {
            if (X[i].left() > domain[i].left()) {
              return false;
            }
            X[i] = interval(X[i].left());
            reduced = true;
          } else if (r.d[i].certainly_strictly_negative()) {
            if (X[i].right() < domain[i].right()) {
              return false;
            }
            X[i] = interval(X[i].right());
            reduced = true;
          }
        }
        if (reduced) {
          r = f(Dual2::variable(X[0], 0), Dual2::variable(X[1], 1));
          m = X.mid();
        }
      }
    }
    // The mean-value theorem: for x in X, f(x) = f(m) + grad f(xi).(x - m)
    // for some xi between m and x, hence in X, and grad f(xi) in r.d.
    n.at_mid = f(m[0], m[1]);
    interval mean_value = n.at_mid;
    for (std::size_t i = 0; i < 2; ++i) {
      mean_value += r.d[i] * (X[i] - m[i]);
    }
    // Both forms enclose the range: so does their intersection. The natural
    // one is the tighter on large boxes, the mean-value one on small boxes.
    n.range = r.v & mean_value;
    return true;
  }

  // A tolerance on the width of the enclosure of f*, with its text: GAOL
  // leaves the rounding upward until gaol::cleanup(), which also rounds the
  // decimal output of a double upward (std::cout << 1e-1 prints 0.100001,
  // the double 1e-1 being slightly above 1/10).
  struct Tolerance {
    double value;
    const char* text;
  };
  const std::vector<Tolerance> tolerances = {
    { 1.0, "1" }, { 1e-1, "1e-1" }, { 1e-2, "1e-2" }, { 1e-3, "1e-3" }, { 1e-6, "1e-6" }, { 1e-9, "1e-9" }
  };
  const long max_boxes = 120000;

  struct Result {
    std::vector<long> boxes;  // boxes bisected to reach each tolerance, -1 if not reached
    interval fstar;           // the enclosure of f* at the end
    Box best;                 // the point where f was found smallest
  };

  // The algorithm follows one path whatever the tolerance: a single run
  // gives the number of boxes bisected until the enclosure of f* is
  // narrower than each tolerance, up to the smallest one or max_boxes.
  template <class F>
  Result minimize(const F& f, const Box& domain, Form form)
  {
    Result result{ std::vector<long>(tolerances.size(), -1), interval::emptyset(), Box() };
    std::priority_queue<Node, std::vector<Node>, LargerLowerBound> queue;
    // f* <= upper, the right bound of f over a point
    double upper = interval::universe().right();
    const auto consider = [&](const Box& X) {
      Node n{ X, interval(), interval() };
      if (!bound(f, form, domain, n)) {
        return;
      }
      // at_mid encloses the true value of f at the midpoint: its right bound
      // is above it, hence above f*.
      if (n.at_mid.right() < upper) {
        upper = n.at_mid.right();
        result.best = n.box.mid();
      }
      // Cut-off: f > upper >= f* all over a box whose lower bound is above
      // upper, so that no global minimizer lies in it.
      if (n.range.left() <= upper) {
        queue.push(n);
      }
    };

    consider(domain);
    double lower = interval::universe().left();
    std::size_t reached = 0;
    for (long boxes = 0;; ++boxes) {
      // Never happens: the box holding a global minimizer is never cut off
      // (were it to happen, the empty enclosure would fail the checks)
      if (queue.empty()) {
        result.fstar = interval::emptyset();
        return result;
      }
      // The boxes cut off or discarded hold no global minimizer: f* is in
      // the queue, and at least the smallest lower bound there, the one on
      // top. The best lower bound met so far holds as well: the mean-value
      // form may bound a half of a box below the box itself.
      const Node& top = queue.top();
      if (top.range.left() > lower) {
        lower = top.range.left();
      }
      result.fstar = interval(lower, upper);
      // width() is rounded upward: no tolerance is claimed before it holds
      while (reached < tolerances.size() && result.fstar.width() <= tolerances[reached].value) {
        result.boxes[reached++] = boxes;
      }
      // A point, a corner of the domain, cannot be bisected any further
      if (reached == tolerances.size() || boxes == max_boxes || top.box.max_width() == 0.0) {
        return result;
      }
      // Bisects the widest side, at 45% of its width as IBEX does
      const auto halves = top.box.bisect();
      queue.pop();
      consider(halves.first);
      consider(halves.second);
    }
  }

  // Runs the three forms on one function and checks what they print
  template <class F>
  bool compare(const char* title, const F& f, const Box& domain, const interval& fstar_ref,
               const interval& range_ref)
  {
    bool ok = true;
    const interval natural = f(domain[0], domain[1]);
    std::cout << title << " on " << domain << "\n"
              << "  natural extension over the domain  " << natural << "\n"
              << "  encloses the range of f            " << range_ref << "\n";
    if (!natural.set_contains(range_ref)) {
      std::cout << "FAILED: the natural extension misses a part of the range\n";
      ok = false;
    }

    const char* const names[] = { "natural extension", "natural & mean-value", "  + monotonicity" };
    Result r[3];
    std::cout << "  boxes bisected until the enclosure [lower, upper] of f* is at most as wide as\n"
              << std::setw(24) << "";
    for (const Tolerance& t : tolerances) {
      std::cout << std::setw(9) << t.text;
    }
    std::cout << "\n";
    for (int k = 0; k < 3; ++k) {
      r[k] = minimize(f, domain, Form(k));
      std::cout << "  " << std::left << std::setw(22) << names[k] << std::right;
      for (long b : r[k].boxes) {
        if (b >= 0) {
          std::cout << std::setw(9) << b;
        } else {
          std::cout << std::setw(9) << "-";
        }
      }
      std::cout << "\n";
    }
    std::cout << "  (-: not within " << max_boxes << " boxes, the cluster effect of the natural extension)\n"
              << "  enclosures of f* at the end\n";
    for (int k = 0; k < 3; ++k) {
      std::cout << "  " << std::left << std::setw(22) << names[k] << std::right << r[k].fstar << "\n";
      if (!r[k].fstar.set_contains(fstar_ref)) {
        std::cout << "FAILED: the enclosure of f* misses the value computed apart\n";
        ok = false;
      }
    }
    // A point interval is written <a, b>: the double it holds lies between
    // the two decimal numbers, the output being rounded outward. The point
    // is written with 7 digits, interval::precision() setting how many
    // digits GAOL writes.
    std::cout << "  each one holds f* = " << fstar_ref << ", computed apart\n";
    const std::streamsize digits = interval::precision(7);
    std::cout << "  upper bound from f at " << r[2].best << "\n";
    interval::precision(digits);
    // The claims of the table: the mean-value form reaches every tolerance,
    // with far fewer boxes than the natural extension, which pays the
    // cluster effect; the monotonicity test saves more boxes still
    const long nat = r[0].boxes.back(), mv = r[1].boxes.back(), mono = r[2].boxes.back();
    if (mv < 0 || mono < 0 || mono > mv || (nat >= 0 && nat < 10 * mv) || mv * 10 > max_boxes) {
      std::cout << "FAILED: the numbers of boxes do not show the cluster effect\n";
      ok = false;
    }
    return ok;
  }

} // namespace

int main()
{
  bool ok = true;

  // The Goldstein-Price function: f(0, -1) = 1 * (30 + 9 * (18 - 48 + 27)) = 3,
  // its global minimum over [-2, 2]^2; its maximum there is at
  // (-1.7373725377583070, 2), computed by mpmath.
  ok &= compare(
    "Goldstein-Price", [](const auto& x, const auto& y) { return goldstein_price(x, y); },
    Box{ interval(-2.0, 2.0), interval(-2.0, 2.0) }, interval(3.0), interval("[3, 1015690.2717980589082989]"));
  std::cout << "\n";

  // The six-hump camel: f* computed by mpmath with 60 digits at the zero of
  // the gradient that findroot finds from (0.0898, -0.7126); interval("...")
  // encloses the decimal number, which a double could not. Its maximum is
  // f(3, 2) = 162.9.
  ok &= compare(
    "Six-hump camel", [](const auto& x, const auto& y) { return six_hump_camel(x, y); },
    Box{ interval(-3.0, 3.0), interval(-2.0, 2.0) },
    interval("-1.03162845348987735041636543714940299235123243853811645053101"),
    interval("[-1.03162845348987735041636543714940299235123243853811645053101, 162.9]"));

  gaol::cleanup();
  return ok ? 0 : EXIT_FAILURE;
}
