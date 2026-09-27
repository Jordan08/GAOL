/*-*-C++-*------------------------------------------------------------------
 * gaol -- NOT Just Another Interval Library
 *--------------------------------------------------------------------------
 * Examples of GAOL v5: set inversion via interval analysis (SIVIA).
 *
 * SIVIA (Jaulin and Walter, 1993) encloses a set S = {x in X : f(x) in Y}
 * between two pavings. A box whose image f(box) lies in Y is inside S, a box
 * whose image misses Y is outside, and the others are cut in two, down to a
 * width eps where they are kept as boundary boxes. The area of S then lies
 * between the area of the inside boxes and that area plus the area of the
 * boundary boxes: a guaranteed bracket, which no sampling of points gives.
 *
 * Part 1 is Codac's examples/03_sivia (sivia() and regular_pave() in
 * src/core/paver/codac2_pave.h): S = {x : x0^2 sin(x0^2 + x1^2) - x1^2 >= 0}
 * in [-5, 5] x [-4, 4], eps = 0.01, boxes cut at 0.49 of their widest side.
 * Codac evaluates f with the natural and the centered forms together; this
 * example uses the natural form alone (04_centered_form.cpp shows the other).
 * Part 2 is IBEX's lab2 and lab3 (doc/lab.rst, examples/lab2.cpp and
 * examples/lab3.cpp): S = {(x, y) : sin(x + y) - 0.1 x y in [0, 2]} in
 * [-10, 10]^2, eps = 0.1, first by evaluation, then with forward-backward
 * contractors built with asin_rel and div_rel (07_contractors.cpp), which
 * remove the parts of a box outside S, then those inside S, before any cut:
 * fewer boxes, a narrower bracket. Part 3 shows a trap of set inversion:
 * {sqrt(x) + sqrt(y) in [0, 1]} in [-1, 1]^2. GAOL follows IEEE 1788-2015
 * without decorations: f is evaluated on the part of a box where it is
 * defined, and sqrt([-1, -0.5]) is the empty set, which is contained in
 * [0, 1] as it is in every set. The naive test then puts boxes where f is not
 * defined inside S; testing the domain explicitly gives back the true area,
 * 1/6.
 *
 * Given a file name, the program also draws the paving of Part 1 there, as
 * an SVG picture.
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
#include <fstream>
#include <iomanip>
#include <iostream>
#include <random>
#include <vector>

#include <gaol/gaol.h>

#include "box.h"

using namespace gaol;
using examples::Box;

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

// What a test tells of a box, as Codac's BoolInterval: all its points are in
// S (inside, TRUE), none is (outside, FALSE), or it cannot tell (UNKNOWN)
enum class Verdict { inside, outside, unknown };

// A paving of the initial box. The outside boxes are only counted; the areas
// are intervals, so that their bounds are bounds on the exact areas, which
// doubles rounded in one direction would not give.
struct Paving {
  std::vector<Box> inside, boundary;
  long processed = 0, outside = 0;
  // interval(0.0), not interval(0): the literal 0 is a null pointer too, and
  // interval has a constructor from a C string, so interval(0) is ambiguous
  interval inside_area = interval(0.0);
  interval boundary_area = interval(0.0);

  // Guaranteed bounds on the area of S: S holds the inside boxes, and lies in
  // the inside and boundary boxes together
  interval area() const { return interval(inside_area.left(), (inside_area + boundary_area).right()); }
};

// SIVIA as Codac's regular_pave() and IBEX's lab2 write it: each box is
// tested, and a box the test cannot decide is cut at ratio of its widest
// side, or kept as a boundary box when it is narrower than eps. The cut is a
// double that the program computes (Box::bisect), rounded upward by default
// and to nearest when GAOL is built with GAOL_PRESERVE_ROUNDING: the numbers
// of boxes differ a little between the two, and the brackets hold in both.
template <class Test>
Paving sivia(const Box& initial, double eps, double ratio, Test test)
{
  Paving p;
  std::vector<Box> stack{ initial };
  while (!stack.empty()) {
    const Box b = stack.back();
    stack.pop_back();
    ++p.processed;
    switch (test(b)) {
      case Verdict::inside:
        p.inside.push_back(b);
        p.inside_area += b.volume();
        break;
      case Verdict::outside:
        ++p.outside;
        break;
      case Verdict::unknown:
        if (b.max_width() > eps) {
          const auto halves = b.bisect(ratio);
          stack.push_back(halves.second);
          stack.push_back(halves.first);
        } else {
          p.boundary.push_back(b);
          p.boundary_area += b.volume();
        }
        break;
    }
  }
  return p;
}

// The test by evaluation, in the order of Codac's sivia(): f(b) in Y proves
// that b is inside S, f(b) disjoint from Y that it is outside. Both use the
// set relations of GAOL, set_contains and set_disjoint.
template <class F>
auto evaluation_test(F f, const interval& Y)
{
  return [f, Y](const Box& b) {
    const interval y = f(b);
    if (Y.set_contains(y)) {
      return Verdict::inside;
    }
    if (y.set_disjoint(Y)) {
      return Verdict::outside;
    }
    return Verdict::unknown;
  };
}

// A point of the box b drawn at random, as a box of point intervals, at which
// f is evaluated with intervals, its rounding errors enclosed. Unless GAOL is
// built with GAOL_PRESERVE_ROUNDING, the program's own doubles are rounded
// upward until gaol::cleanup(): left + t * width could pass the right bound,
// hence the clamp.
Box random_point(const Box& b, std::mt19937& rng)
{
  Box p(b.size());
  for (std::size_t i = 0; i < b.size(); ++i) {
    // t in (0, 1), an exact multiple of 2^-25
    const double t = (static_cast<double>(rng() >> 8) + 0.5) / 16777216.0;
    double x = b[i].left() + t * (b[i].right() - b[i].left());
    if (x > b[i].right()) {
      x = b[i].right();
    }
    p[i] = interval(x);
  }
  return p;
}

// The number of random points, two per inside box, at which f is not
// certainly defined and in Y: none, if the inside boxes are inside S
template <class F>
int points_not_in_S(const Paving& p, F f, const interval& Y, std::mt19937& rng)
{
  int wrong = 0;
  for (const Box& b : p.inside) {
    for (int k = 0; k < 2; ++k) {
      const interval y = f(random_point(b, rng));
      // An empty y is where f is not defined: the point is not in S, though
      // Y.set_contains(y) is true
      if (y.is_empty() || !Y.set_contains(y)) {
        ++wrong;
      }
    }
  }
  return wrong;
}

// ---- Part 1: Codac's examples/03_sivia
interval f_codac(const Box& x)
{
  return sqr(x[0]) * sin(sqr(x[0]) + sqr(x[1])) - sqr(x[1]);
}

// ---- Part 2: IBEX's lab2 and lab3, f(x, y) = sin(x + y) - 0.1 x y
//
// 0.1 is not a double: interval("0.1") encloses 1/10, where interval(0.1)
// would be the double nearest to it, and S another set. A function-local
// static builds it once, at the first call, after GAOL is initialized.
const interval& tenth()
{
  static const interval t("0.1");
  return t;
}

interval f_lab(const Box& b)
{
  return sin(b[0] + b[1]) - tenth() * b[0] * b[1];
}

// The forward-backward contractor of f(x, y) in c (IBEX's CtcFwdBwd): it
// removes from b points where f is not in c, and returns false when it
// removes them all. See 07_contractors.cpp for the reverse operations.
bool contract_lab(Box& b, const interval& c)
{
  interval x = b[0], y = b[1];
  // Forward: every node of the tree of f over the box
  interval a = x + y;
  interval s = sin(a);
  interval t = tenth() * x;
  interval p = t * y;
  interval f = s - p;
  // The root must be in c
  f &= c;
  if (f.is_empty()) {
    return false;
  }
  // Backward, from the root down to x and y
  s &= f + p;                // f = s - p
  p &= s - f;
  a = asin_rel(s, a);        // s = sin(a), over every period of sin
  t = div_rel(p, y, t);      // p = t * y, for t and then for y
  y = div_rel(p, t, y);
  x = div_rel(t, tenth(), x);  // t = 0.1 * x
  x &= a - y;                // a = x + y
  y &= a - x;
  if (x.is_empty() || y.is_empty()) {
    return false;
  }
  b[0] = x;
  b[1] = y;
  return true;
}

// The part of the box b outside its subset r, as boxes: two slabs along each
// side where r is narrower than b (IBEX's IntervalVector::diff)
std::vector<Box> difference(const Box& b, const Box& r)
{
  std::vector<Box> pieces;
  Box rest = b;
  for (std::size_t i = 0; i < b.size(); ++i) {
    if (rest[i].left() < r[i].left()) {
      Box piece = rest;
      piece[i] = interval(rest[i].left(), r[i].left());
      pieces.push_back(piece);
    }
    if (r[i].right() < rest[i].right()) {
      Box piece = rest;
      piece[i] = interval(r[i].right(), rest[i].right());
      pieces.push_back(piece);
    }
    rest[i] = r[i];
  }
  return pieces;
}

// SIVIA with contractors, IBEX's lab3. The outer contractor keeps the points
// where f can be in [0, 2]: what it removes is outside S. The inner one keeps
// the points where f can be outside [0, 2], in [-oo, 0] or in [2, +oo], one
// contraction for each and their hull: what it removes is inside S, and is
// kept as inside boxes, as lab3 draws it. Only what both leave is cut.
Paving sivia_contractors(const Box& initial, double eps, double ratio)
{
  const interval Y(0.0, 2.0), below(-GAOL_INFINITY, 0.0), above(2.0, GAOL_INFINITY);
  Paving p;
  std::vector<Box> stack{ initial };
  while (!stack.empty()) {
    Box b = stack.back();
    stack.pop_back();
    ++p.processed;
    if (!contract_lab(b, Y)) {
      ++p.outside;
      continue;
    }
    Box low = b, high = b;
    const bool has_low = contract_lab(low, below);
    const bool has_high = contract_lab(high, above);
    const Box rest = !has_low ? high : !has_high ? low : (low | high);
    if (!has_low && !has_high) {
      p.inside.push_back(b);
      p.inside_area += b.volume();
      continue;
    }
    for (const Box& piece : difference(b, rest)) {
      p.inside.push_back(piece);
      p.inside_area += piece.volume();
    }
    if (rest.max_width() > eps) {
      const auto halves = rest.bisect(rest.widest(), ratio);
      stack.push_back(halves.second);
      stack.push_back(halves.first);
    } else {
      p.boundary.push_back(rest);
      p.boundary_area += rest.volume();
    }
  }
  return p;
}

// ---- Part 3: the domain of f
interval f_domain(const Box& b)
{
  return sqrt(b[0]) + sqrt(b[1]);
}

// The picture of the paving: the initial box in blue (outside), the inside
// boxes in green, the boundary boxes in yellow. The coordinates are doubles
// computed rounded upward (see random_point), which a picture does not mind.
void write_svg(const char* name, const Box& initial, const Paving& p)
{
  std::ofstream svg(name);
  const double scale = 100.0;
  const double x0 = initial[0].left(), y1 = initial[1].right();
  const auto rect = [&](const Box& b, const char* colour) {
    svg << "<rect x=\"" << (b[0].left() - x0) * scale << "\" y=\"" << (y1 - b[1].right()) * scale
        << "\" width=\"" << b[0].width() * scale << "\" height=\"" << b[1].width() * scale << "\" fill=\""
        << colour << "\"/>\n";
  };
  svg << "<svg xmlns=\"http://www.w3.org/2000/svg\" width=\"" << initial[0].width() * scale << "\" height=\""
      << initial[1].width() * scale << "\">\n";
  rect(initial, "#4a90d9");
  for (const Box& b : p.inside) {
    rect(b, "#3cb44b");
  }
  for (const Box& b : p.boundary) {
    rect(b, "#ffe119");
  }
  svg << "</svg>\n";
}

// One line of the tables of Parts 2 and 3
void print_row(const char* name, const Paving& p)
{
  std::cout << "  " << std::left << std::setw(26) << name << std::right << std::setw(6) << p.processed
            << std::setw(8) << p.inside.size() << std::setw(10) << p.boundary.size() << "   " << p.area()
            << "\n";
}

} // namespace

int main(int argc, char* argv[])
{
  std::mt19937 rng(1788);
  interval::precision(6);

  // ---- Part 1
  const Box codac_box{ interval(-5.0, 5.0), interval(-4.0, 4.0) };
  const interval positive = interval::positive();  // [0, +oo]
  const Paving codac = sivia(codac_box, 0.01, 0.49, evaluation_test(f_codac, positive));
  // The area of S, by two methods in double precision: the exact length of
  // S on 8000 columns x0 (its ends found with brentq), and a grid of
  // 16000 x 16000 points; they agree to 0.001
  const interval codac_reference("[19.562, 19.566]");

  std::cout << "Part 1. Codac's examples/03_sivia: S = {x : x0^2 sin(x0^2 + x1^2) - x1^2 >= 0}\n"
            << "        in [-5, 5] x [-4, 4], boxes cut at 0.49 of their widest side, down to 0.01\n"
            << "  boxes processed:            " << codac.processed << "\n"
            << "  inside, outside, boundary:  " << codac.inside.size() << ", " << codac.outside << ", "
            << codac.boundary.size() << "\n"
            << "  area of the inside boxes:   " << codac.inside_area << "\n"
            << "  area of the boundary boxes: " << codac.boundary_area << "\n"
            << "  area of S:                  " << codac.area() << "  holds 19.564, computed apart\n";
  check(codac.area().set_contains(codac_reference), "the area of S is in the bracket");
  const int codac_wrong = points_not_in_S(codac, f_codac, positive, rng);
  std::cout << "  f >= 0 at " << 2 * codac.inside.size() << " random points of the inside boxes: "
            << (codac_wrong == 0 ? "all" : "NOT all") << "\n";
  check(codac_wrong == 0, "the random points of the inside boxes are in S");
  if (argc > 1) {
    write_svg(argv[1], codac_box, codac);
    std::cout << "  paving drawn in " << argv[1] << "\n";
  }

  // ---- Part 2
  const Box lab_box{ interval(-10.0, 10.0), interval(-10.0, 10.0) };
  const interval zero_two(0.0, 2.0);
  // IBEX cuts boxes in the middle (IntervalVector::bisect(i), ratio 0.5)
  const Paving lab2 = sivia(lab_box, 0.1, 0.5, evaluation_test(f_lab, zero_two));
  const Paving lab3 = sivia_contractors(lab_box, 0.1, 0.5);
  // The area of S, by the same two methods as in Part 1 (their results
  // 100.87360 and 100.87342)
  const interval lab_reference("[100.872, 100.875]");

  std::cout << "\nPart 2. IBEX's lab2 and lab3: S = {(x, y) : sin(x + y) - 0.1 x y in [0, 2]}\n"
            << "        in [-10, 10]^2, boxes cut in the middle, down to 0.1\n"
            << "                             boxes  inside  boundary   area of S\n";
  print_row("by evaluation (lab2)", lab2);
  print_row("with contractors (lab3)", lab3);
  std::cout << "  both hold 100.874, computed apart; the contractors process fewer boxes\n"
            << "  and give a narrower bracket\n";
  check(lab2.area().set_contains(lab_reference) && lab3.area().set_contains(lab_reference),
        "the area of S is in both brackets");
  check(lab3.processed < lab2.processed, "the contractors process fewer boxes");
  check(lab3.area().width() < lab2.area().width(), "the contractors give a narrower bracket");
  const int lab_wrong = points_not_in_S(lab2, f_lab, zero_two, rng) + points_not_in_S(lab3, f_lab, zero_two, rng);
  std::cout << "  f in [0, 2] at " << 2 * (lab2.inside.size() + lab3.inside.size())
            << " random points of the inside boxes: " << (lab_wrong == 0 ? "all" : "NOT all") << "\n";
  check(lab_wrong == 0, "the random points of the inside boxes are in S");

  // ---- Part 3
  const Box domain_box{ interval(-1.0, 1.0), interval(-1.0, 1.0) };
  const interval zero_one(0.0, 1.0);
  const interval undefined = sqrt(interval(-1.0, -0.5));
  std::cout << "\nPart 3. S = {(x, y) : sqrt(x) + sqrt(y) in [0, 1]} in [-1, 1]^2, whose area is 1/6\n"
            << "  sqrt([-1, -0.5]) = " << undefined << ", which [0, 1] contains and which is disjoint from [0, 1]\n";
  check(undefined.is_empty() && zero_one.set_contains(undefined) && undefined.set_disjoint(zero_one),
        "the empty set passes both tests");

  // The naive test puts in S the boxes where f is not defined, as the empty
  // image is contained in [0, 1], and the boxes where f is defined on a part
  // only, as sqrt([-0.5, 0.25]) is sqrt([0, 0.25])
  const Paving naive = sivia(domain_box, 0.01, 0.49, evaluation_test(f_domain, zero_one));

  // The fix: an empty image means that no point of the box is in the domain
  // of f, so none is in S; and f(b) proves b inside S only when the whole box
  // is in the domain. The decoration def of IEEE 1788 would tell it; GAOL has
  // no decorations, so the domain is tested on the box itself
  const auto domain_test = [&zero_one, &positive](const Box& b) {
    const interval y = f_domain(b);
    if (y.is_empty() || y.set_disjoint(zero_one)) {
      return Verdict::outside;
    }
    const bool in_domain = positive.set_contains(b[0]) && positive.set_contains(b[1]);
    if (in_domain && zero_one.set_contains(y)) {
      return Verdict::inside;
    }
    return Verdict::unknown;
  };
  const Paving fixed = sivia(domain_box, 0.01, 0.49, domain_test);

  std::cout << "                             boxes  inside  boundary   area of S\n";
  print_row("naive test", naive);
  print_row("domain tested", fixed);
  const int naive_wrong = points_not_in_S(naive, f_domain, zero_one, rng);
  const int fixed_wrong = points_not_in_S(fixed, f_domain, zero_one, rng);
  std::cout << "  naive: an area above 1, though S lies in [0, 1]^2; " << naive_wrong << " of "
            << 2 * naive.inside.size() << " random points\n"
            << "         of its inside boxes are not in S\n"
            << "  domain tested: the bracket holds 1/6, and all " << 2 * fixed.inside.size()
            << " random points are in S\n";
  check(naive.inside_area.left() > 1.0 && naive_wrong > 0, "the naive test is wrong");
  check(fixed.area().set_contains(interval(1.0) / 6.0), "the bracket holds 1/6");
  check(fixed_wrong == 0, "the random points of the inside boxes are in S");

  gaol::cleanup();

  if (failures != 0) {
    return EXIT_FAILURE;
  }
  std::cout << "All the claims above were checked.\n";
  return 0;
}
