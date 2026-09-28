/*-*-C++-*------------------------------------------------------------------
 * gaol -- NOT Just Another Interval Library
 *--------------------------------------------------------------------------
 * Examples of GAOL v5: contractors, forward-backward propagation with
 * GAOL's relational functions.
 *
 * A contractor shrinks a box without losing any solution of a constraint.
 * The forward-backward contractor (HC4Revise, the CtcHC4 of IBEX and the
 * CtcInverse of Codac) evaluates the expression tree of the constraint over
 * the box (forward), intersects the value at the root with the right-hand
 * side, then carries that value back down the tree to the variables
 * (backward), through the reverse of each operation. GAOL's relational
 * functions are these reverse operations: sqrt_rel for sqr, div_rel for the
 * product, asin_rel for sin, and so on; IBEX's wrapper of GAOL calls them for
 * its bwd_sqr, bwd_mul, bwd_sin, bwd_cos, bwd_tan and bwd_abs. Unlike the
 * inverse functions, they keep every branch: asin(-1) is -pi/2 alone, while
 * the reverse of sin finds 3pi/2 in [4, 6].
 *
 * Part 1 is the backward arithmetic of IBEX (examples 4 and 5 of
 * examples/doc-arithmetic.cpp, outputs in examples/doc-arithmetic.txt):
 * x = [1, 2], y = [3, 4], z = x + y; sin(z) = -1 contracts z to 3pi/2, then x
 * and y, and sin(z) = 1 empties z, which proves that the constraint has no
 * solution in the box. Part 2 solves x^2 + y^2 = 1, y = exp(x) - 1.5 by
 * branch and prune, as IBEX's default solver does: a fixpoint of the two
 * contractors (IBEX's CtcFixPoint of CtcHC4), then a bisection of the widest
 * side at 0.45 of its width, until the boxes are narrower than 1e-8. Every
 * solution lies in one of the boxes printed, the contractors never removing
 * one; proving that a box holds a solution, and only one, is the work of
 * 10_krawczyk.cpp.
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
#include <iostream>
#include <vector>

#include <gaol/gaol.h>

#include "box.h"

using namespace gaol;
using examples::Box;

/*
 * The reverse operations under their three names: GAOL v5, IEEE 1788-2015
 * (namespace gaol_ieee1788, Table 10.1) and IBEX (ibex_Interval.h). For a
 * node c = f(x) of the tree, each one returns the hull of the points of x
 * whose image by f is in c: a subset of x, which needs no intersection.
 *
 *   GAOL v5                     gaol_ieee1788               IBEX
 *   x = sqrt_rel(c, x)          x = sqrRev(c, x)            bwd_sqr(c, x)
 *   x = nth_root_rel(c, n, x)   x = pownRev(c, x, n)        bwd_pow(c, n, x)
 *   x = div_rel(c, y, x)        x = mulRev(y, c, x)         bwd_mul(c, x, y)
 *   y = div_rel(c, x, y)        y = mulRev(x, c, y)           (both at once)
 *   x = invabs_rel(c, x)        x = absRev(c, x)            bwd_abs(c, x)
 *   x = asin_rel(c, x)          x = sinRev(c, x)            bwd_sin(c, x)
 *   x = acos_rel(c, x)          x = cosRev(c, x)            bwd_cos(c, x)
 *   x = atan_rel(c, x)          x = tanRev(c, x)            bwd_tan(c, x)
 *   x = acosh_rel(c, x)         x = coshRev(c, x)           bwd_cosh(c, x)
 *   x = asinh_rel(c, x)         x = sinhRev(c, x) (*)       bwd_sinh(c, x)
 *   x &= c - y; y &= c - x      (by subtraction)            bwd_add(c, x, y)
 *   x &= log(c)                 (exp is one-to-one)         bwd_exp(c, x)
 *
 * (*) not in the standard, which needs no reverse for a one-to-one function;
 * gaol_ieee1788 names it after coshRev. Mind the order of the arguments for
 * the product: div_rel(c, y, x) divides c by y within x, while mulRev(y, c, x)
 * gives the factor y first. nth_root_rel and pownRev take an exponent n >= 1.
 */

namespace {

int failures = 0;

// Every claim printed is checked: a false one prints "FAILED: ..." and makes
// the program exit with a failure
void check(bool ok, const char* what)
{
  if (!ok) {
    std::cout << "FAILED: " << what << "\n";
    ++failures;
  }
}

// The contractor of x^2 + y^2 = 1, whose tree is s = a + c, a = sqr(x),
// c = sqr(y), with s = 1. Returns false when it proves that the box holds no
// solution.
bool revise_circle(Box& b)
{
  // Forward: an enclosure of each node over the box
  interval a = sqr(b[0]);
  interval c = sqr(b[1]);
  interval s = a + c;
  // The root must be 1: when 1 is not in s, no point of the box is on the
  // circle
  s &= interval(1.0);
  if (s.is_empty()) {
    return false;
  }
  // Backward, from the root down: s = a + c gives a = s - c and c = s - a,
  // intersected with what a and c already are
  a &= s - c;
  c &= s - a;
  // a = x^2: the relational square root keeps both -sqrt(a) and +sqrt(a),
  // where sqrt(a) keeps the positive root alone and would lose the solutions
  // with x < 0. Its result is already intersected with x.
  b[0] = sqrt_rel(a, b[0]);
  b[1] = sqrt_rel(c, b[1]);
  return !b.is_empty();
}

// The contractor of y = exp(x) - 1.5, whose tree is t = e - 1.5, e = exp(x),
// with t = y. 1.5 is a double: a constant that is not one, such as 1.1, would
// be written textToInterval("1.1"), since interval(1.1) is not 11/10.
bool revise_curve(Box& b)
{
  interval e = exp(b[0]);  // forward
  interval t = e - 1.5;
  t &= b[1];               // t = y: both lie in their intersection
  if (t.is_empty()) {
    return false;
  }
  b[1] = t;
  e &= t + 1.5;            // backward through t = e - 1.5
  b[0] &= log(e);          // through e = exp(x): exp being one-to-one, its
                           // reverse is its inverse log (IBEX's bwd_exp)
  return !b[0].is_empty();
}

// Applies both contractors in turn until a round shrinks no side by more than
// 10% of its width, as IBEX's CtcFixPoint does with its default ratio 0.1:
// going to the exact fixpoint would spend many rounds on gains of a few
// doubles. The gain is measured on the widths of the sides. Returns false
// when the box holds no solution; rounds tells how many rounds were made.
bool contract(Box& b, int& rounds)
{
  for (rounds = 1;; ++rounds) {
    const Box before = b;
    if (!revise_circle(b) || !revise_curve(b)) {
      return false;
    }
    bool small_gain = true;
    for (std::size_t i = 0; i < b.size(); ++i) {
      if (b[i].width() < 0.9 * before[i].width()) {
        small_gain = false;
      }
    }
    if (small_gain) {
      return true;
    }
  }
}

// What a branch and prune search returns
struct Search {
  std::vector<Box> found;  // the boxes narrower than eps it keeps
  int boxes = 0;           // the boxes it processed
  int rounds = 0;          // the rounds of the two contractors it made
};

// Branch and prune: contracts each box, drops it when it holds no solution,
// keeps it when it is narrower than eps, and otherwise cuts its widest side
// at 0.45 of its width, as IBEX does, rather than in its middle, where a
// solution of a symmetric problem often lies. With to_fixpoint false, each
// box gets one round of the two contractors only.
Search branch_and_prune(const Box& initial, double eps, bool to_fixpoint)
{
  Search search;
  std::vector<Box> stack{ initial };
  while (!stack.empty()) {
    Box b = stack.back();
    stack.pop_back();
    ++search.boxes;
    int rounds = 1;
    const bool kept = to_fixpoint ? contract(b, rounds) : revise_circle(b) && revise_curve(b);
    search.rounds += rounds;
    if (!kept) {
      continue;  // proved: no solution in b
    }
    if (b.max_width() < eps) {
      search.found.push_back(b);
      continue;
    }
    const auto halves = b.bisect();
    stack.push_back(halves.second);
    stack.push_back(halves.first);
  }
  return search;
}

} // namespace

int main()
{
  // ---- Part 1: IBEX's backward arithmetic
  std::cout << "Part 1. Backward arithmetic (IBEX examples/doc-arithmetic.cpp, #4 and #5)\n";

  // 3pi/2, from mpmath with 50 digits: interval(d) holds one of the two
  // doubles around it, which an enclosure of 3pi/2 contains
  const interval three_pi_2 = interval(4.7123889803846898576939650749192543262957540990627);
  {
    interval x(1.0, 2.0), y(3.0, 4.0);
    interval z = x + y;
    std::cout << "  x = " << x << ", y = " << y << ", z = x + y = " << z << "\n";
    // sin(z) = -1: z becomes the part of z whose sine is -1 (IBEX's
    // bwd_sin(-1.0, z)), which asin_rel finds over all the periods of sin
    z = asin_rel(interval(-1.0), z);
    // z = x + y: x = z - y and y = z - x (IBEX's bwd_add(z, x, y)). IBEX's
    // doc-arithmetic.txt, made with GAOL 4, prints z one double wider on each
    // side, [4.712388980384688, 4.712388980384692]
    x &= z - y;
    y &= z - x;
    std::cout << "  sin(z) = -1:  z = " << z << "  encloses 3pi/2\n"
              << "                x = " << x << "  = [1, 3pi/2 - 3]\n"
              << "                y = " << y << "  = [3, 3pi/2 - 1]\n";
    check(z.set_contains(three_pi_2) && z.width() < 1e-14, "z is a tight enclosure of 3pi/2");
    check(x.set_contains(interval(1.0) | (three_pi_2 - 3.0)) && x.right() < 1.7123889803847,
          "x is [1, 3pi/2 - 3]");
    check(y.set_contains(interval(3.0) | (three_pi_2 - 1.0)) && y.right() < 3.7123889803847,
          "y is [3, 3pi/2 - 1]");

    // The inverse function is not the reverse one: asin(-1) is -pi/2 only,
    // the preimage in [-pi/2, pi/2], and intersecting [4, 6] with it would
    // "prove" that sin(z) = -1 has no solution there
    const interval wrong = interval(4.0, 6.0) & asin(interval(-1.0));
    std::cout << "  z = [4, 6] & asin(-1) would be " << wrong << ": asin has one branch, a wrong proof\n";
    check(wrong.is_empty(), "asin(-1) misses 3pi/2");
  }
  {
    interval x(1.0, 2.0), y(3.0, 4.0);
    interval z = x + y;
    // The solutions of sin(z) = 1 are pi/2 + 2k pi: 1.57..., 7.85..., none
    // in [4, 6]. The empty z empties x and y.
    z = asin_rel(interval(1.0), z);
    x &= z - y;
    y &= z - x;
    std::cout << "  sin(z) = 1:   z = " << z << ", x = " << x << ", y = " << y
              << ": no solution in the box\n";
    check(z.is_empty() && x.is_empty() && y.is_empty(), "sin(x + y) = 1 has no solution");
  }
  {
    // The product x * y = c: dividing c by y gives nothing when y holds 0,
    // c / y being [-oo, +oo]; the relational division keeps the x of x for
    // which some y of y gives a product in c, here |x| >= 1
    const interval c(1.0, 2.0), y(-1.0, 1.0), x(0.5, 10.0);
    const interval by_division = x & (c / y);
    const interval by_relation = div_rel(c, y, x);
    std::cout << "  x * y = c = [1, 2], y = [-1, 1], x = [0.5, 10]:\n"
              << "      x & (c / y) = " << by_division << ", div_rel(c, y, x) = " << by_relation << "\n";
    check(by_division.set_eq(x), "dividing by an interval holding 0 does not contract");
    check(by_relation.set_eq(interval(1.0, 10.0)), "div_rel gives [1, 10]");
    check(by_relation.set_eq(gaol_ieee1788::mulRev(y, c, x)), "div_rel(c, y, x) is mulRev(y, c, x)");

    // The same with sqr: sqrt keeps the positive root, sqrt_rel both
    const interval x2(-3.0, 3.0);
    std::cout << "  x^2 = c = [1, 4], x = [-3, 3]:\n"
              << "      x & sqrt(c) = " << (x2 & sqrt(interval(1.0, 4.0)))
              << ", sqrt_rel(c, x) = " << sqrt_rel(interval(1.0, 4.0), x2) << "\n";
    check(sqrt_rel(interval(1.0, 4.0), x2).set_eq(interval(-2.0, 2.0)), "sqrt_rel gives [-2, 2]");
  }

  // ---- Part 2: branch and prune
  // The two solutions, from mpmath with 50 digits (findroot on
  // x^2 + (exp(x) - 1.5)^2 = 1, checked by findroot on the system)
  const Box solution[2] = {
    { interval(-0.47660954471908055134256347216127561813625353271843),
      interval(-0.87911509023714907230029912788341393476004746249643) },
    { interval(0.76351037484796563769106834834693518671171708347844),
      interval(0.64579556169078694043427738736715192326909564742362) } };

  const Box initial{ interval(-10.0, 10.0), interval(-10.0, 10.0) };
  interval::precision(10);
  std::cout << "\nPart 2. Branch and prune: x^2 + y^2 = 1, y = exp(x) - 1.5, x and y in [-10, 10]\n";
  {
    Box once = initial;
    const bool kept = revise_circle(once) && revise_curve(once);
    Box fixpoint = initial;
    int rounds = 0;
    const bool kept_fixpoint = contract(fixpoint, rounds);
    std::cout << "  one round of the two contractors: " << once << "\n"
              << "  fixpoint, after " << rounds << " rounds:         " << fixpoint << "\n"
              << "  it holds both solutions: the contractors cannot split them, bisection does\n";
    check(kept && kept_fixpoint && initial.contains(once) && once.contains(fixpoint),
          "each contraction is a subset of the box before it");
    check(fixpoint.max_width() < 0.5 * initial.max_width(), "the fixpoint is much smaller");
    check(fixpoint.contains(solution[0]) && fixpoint.contains(solution[1]),
          "the fixpoint holds both solutions");
  }

  const double eps = 1e-8;
  const Search once = branch_and_prune(initial, eps, false);
  const Search fixpoint = branch_and_prune(initial, eps, true);
  std::cout << "  one round per box: " << once.boxes << " boxes processed, " << once.rounds << " rounds, "
            << once.found.size() << " boxes narrower than 1e-8 left\n"
            << "  fixpoint per box:   " << fixpoint.boxes << " boxes processed, " << fixpoint.rounds
            << " rounds, " << fixpoint.found.size() << " boxes narrower than 1e-8 left:\n";
  check(fixpoint.boxes < once.boxes, "the fixpoint needs fewer bisections");
  for (const Search* search : { &once, &fixpoint }) {
    // Each solution lies in a box kept, and each box kept holds a solution
    for (const Box& s : solution) {
      bool in_one = false;
      for (const Box& b : search->found) {
        in_one = in_one || b.contains(s);
      }
      check(in_one, "each solution of mpmath lies in a box kept");
    }
    for (const Box& b : search->found) {
      check(b.max_width() < eps && (b.contains(solution[0]) || b.contains(solution[1])),
            "each box kept is narrower than 1e-8 and holds a solution of mpmath");
    }
  }
  for (const Box& b : fixpoint.found) {
    std::cout << "    " << b << "  holds a solution of mpmath\n";
  }
  // Near a solution, a round of HC4 removes about the same fraction of the
  // box each time, which is linear convergence; interval Newton converges
  // quadratically there (05_interval_newton.cpp, 10_krawczyk.cpp)
  std::cout << "  the fixpoint saves bisections, but near a solution each round removes the same\n"
            << "  fraction of the box: HC4 converges linearly, interval Newton quadratically\n"
            << "  every solution in [-10, 10]^2 lies in these boxes: the contractors remove none\n";
  check(fixpoint.rounds > once.rounds, "the fixpoint makes more rounds");

  gaol::cleanup();

  if (failures != 0) {
    return EXIT_FAILURE;
  }
  std::cout << "All the claims above were checked.\n";
  return 0;
}
