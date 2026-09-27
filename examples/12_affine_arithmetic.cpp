/*-*-C++-*------------------------------------------------------------------
 * gaol -- NOT Just Another Interval Library
 *--------------------------------------------------------------------------
 * Examples of GAOL v5: affine arithmetic, against interval arithmetic.
 *
 * The program evaluates the same functions with GAOL's intervals and with
 * the affine forms of affine.h, a minimal affine arithmetic written on
 * GAOL's public interval API only: x - x on [-2, 3]; x (1 - x) on [0, 1],
 * [0.2, 0.4] and [0.49, 0.51]; (x - 1)^5 expanded and evaluated by Horner's
 * scheme on [0.99, 1.01]; x^2 + y^2 - 2 x y on [1, 2]^2; exp(x) exp(-x) on
 * [-0.1, 0.1], with the min-range and the Chebyshev linearizations, and
 * x - sqrt(x) on [1, 4]; the wrapping effect, 16 rotations by pi/4 of a
 * box; and the recipe of IBEX's AffineEval and of INTLAB's affari, which
 * intersect the affine enclosure with the interval one, because affine
 * forms can lose to intervals on wide boxes of strongly nonlinear functions
 * (x / (1 + x) on [0, 10], where neither enclosure holds the other, and
 * 1 / x on [1, 100]). Affine forms keep the linear correlations between
 * quantities, which cures the dependency problem at first order and the
 * wrapping of linear maps; what is not linear becomes error terms, which do
 * not cancel. Each function is written once, as a template, for both
 * arithmetics. The cases follow the documentation of ibex-affine (J. Ninin,
 * examples/doc-affine.cpp and doc-affine.txt: the snippets af-build,
 * af-dependency, af-mode, af-vector and af-eval, and
 * examples/ex_affineform.cpp), the affine arithmetic of YalAA (S. Kiel) and
 * INTLAB's affari (S. M. Rump), and J. Stolfi and L. H. de Figueiredo,
 * "Self-validated numerical methods and applications" (1997). The true ranges are closed forms, the
 * functions being monotonic, constant or with one extremum on each box, and
 * were checked with mpmath at 50 digits; the true images of the rotated box
 * are the box turned by a multiple of 90 degrees.
 *
 * Copyright (c) 2026 ENSTA, France
 *
 * Created 2026-09-27 by Jordan NININ
 *--------------------------------------------------------------------------
 * gaol is a software distributed WITHOUT ANY WARRANTY. Read the associated
 * COPYING file for information.
 *--------------------------------------------------------------------------*/

#include <cmath>
#include <cstdlib>
#include <iomanip>
#include <iostream>
#include <sstream>
#include <string>
#include <vector>

#include <gaol/gaol.h>

#include "affine.h"
#include "box.h"

using namespace gaol;
using examples::Affine;
using examples::Box;
using examples::Linearization;
using examples::decimal;

namespace {

  bool all_checks_passed = true;

  // Every claim of the output is checked here: a claim that does not hold is
  // reported, and makes the program fail, which is how ctest runs it
  void check(bool holds, const std::string& claim)
  {
    if (!holds) {
      std::cout << "FAILED: " << claim << "\n";
      all_checks_passed = false;
    }
  }

  // The text of what operator<< writes, to fill the columns of a table
  template <class T>
  std::string text(const T& x)
  {
    std::ostringstream os;
    os << x;
    return os.str();
  }

  // A line of a table: a label, then columns of the given width
  void row(const std::string& label, const std::vector<std::string>& cells, int width = 24)
  {
    std::cout << "   " << std::left << std::setw(22) << label;
    for (std::size_t i = 0; i < cells.size(); ++i) {
      std::cout << std::setw(i + 1 < cells.size() ? width : 0) << cells[i];
    }
    std::cout << "\n";
  }

  // How many times wider a is than b, with three digits up to 1000, two above
  std::string times(const interval& a, const interval& b)
  {
    const double r = a.width() / b.width();
    return decimal(r, r < 1000.0 ? 3 : 2);
  }

  /*
    The functions, written once for both arithmetics. With T = interval, each
    occurrence of x ranges over its interval on its own; with T = Affine, all
    the occurrences share the noise symbol of x. The constants are doubles,
    exact in both: 1.0 - x is Affine(1.0) - x.
  */
  template <class T>
  T logistic(const T& x)
  {
    return x * (1.0 - x);
  }

  // (x - 1)^5 = x^5 - 5 x^4 + 10 x^3 - 10 x^2 + 5 x - 1, by Horner's scheme
  template <class T>
  T quintic(const T& x)
  {
    return ((((x - 5.0) * x + 10.0) * x - 10.0) * x + 5.0) * x - 1.0;
  }

  // (x - y)^2 expanded; sqr() is GAOL's for intervals, affine.h's for forms
  template <class T>
  T square_of_difference(const T& x, const T& y)
  {
    return sqr(x) + sqr(y) - 2.0 * x * y;
  }

  template <class T>
  T ratio(const T& x)
  {
    return x / (1.0 + x);
  }

} // namespace

int main()
{
  // Six digits, rounded outward: the text of an interval encloses it, so that
  // the interval read from "[0.2, 0.4]", which holds the reals from 0.2 to
  // 0.4, is written [0.199999, 0.400001]
  const std::streamsize digits = interval::precision(6);

  // ------------------------------------------------------------------------
  std::cout << "1. x - x with x in [-2, 3] (true range [0, 0]): the dependency problem\n";

  // The interval subtraction takes its two operands apart. An affine form
  // names the source of its uncertainty, the noise symbol e1 of x: x - x is
  // (0.5 - 0.5) + (2.5 - 2.5) e1. y is another variable with the same range,
  // so another symbol; a constant has none, and its error cannot cancel.
  const interval X(-2.0, 3.0);
  const Affine x = Affine::variable(X);
  const Affine y = Affine::variable(X);
  const Affine z = Affine::constant(X);
  row("intervals", {"x - x = " + text(X - X), "each x ranges over [-2, 3] on its own"});
  row("x = " + text(x), {"x - x = " + text(x - x), "the terms in e1 cancel"});
  row("y = " + text(y), {"x - y = " + text((x - y).to_interval()), "right: y is another number"});
  row("z = " + text(z), {"z - z = " + text((z - z).to_interval()), "a constant: its error stays"});
  check((X - X).set_eq(interval(-5.0, 5.0)), "x - x on [-2, 3] is [-5, 5] with intervals");
  check((x - x).size() == 0 && (x - x).to_interval().set_eq(interval(0.0)), "x - x is 0 with affine forms");
  check((x - y).to_interval().set_contains(interval(-5.0, 5.0)), "x - y contains [-5, 5]");
  check((z - z).to_interval().set_contains(interval(-5.0, 5.0)), "z - z is [-5, 5]");

  // ------------------------------------------------------------------------
  std::cout << "\n" << std::left << std::setw(25) << "2. x (1 - x)" << std::setw(24) << "intervals" << std::setw(24)
            << "affine forms" << "true range\n";

  // The product of two forms keeps its linear part exactly; its quadratic
  // part, -a^2 e1^2, is not linear, and becomes an error of radius a^2 / 2
  // around a center moved by -a^2 / 2. On a box centered on the vertex 0.5,
  // the linear part is 0, and the affine enclosure is the range. GAOL may
  // write a zero bound as -0; for an interval, -0 and 0 are the same bound.
  struct Case {
    const char* box;
    const char* range;  // read from text: the real numbers, enclosed
  };
  const Case cases[] = {{"[0, 1]", "[0, 0.25]"}, {"[0.2, 0.4]", "[0.16, 0.24]"}, {"[0.49, 0.51]", "[0.2499, 0.25]"}};
  std::string interval_times, affine_times, form;
  for (const Case& k : cases) {
    const interval X2(k.box), R(k.range);
    const Affine x2 = Affine::variable(X2);
    const interval I = logistic(X2), A = logistic(x2).to_interval();
    row(std::string("x in ") + k.box, {text(I), text(A), k.range});
    interval_times += (interval_times.empty() ? "" : ", ") + times(I, R);
    affine_times += (affine_times.empty() ? "" : ", ") + times(A, R);
    check(I.set_contains(R) && A.set_contains(R), std::string("x (1 - x) on ") + k.box + ": contains the range");
    check(A.width() < I.width(), std::string("x (1 - x) on ") + k.box + ": affine narrower");
    if (std::string(k.box) != "[0.2, 0.4]") {
      check(A.width() < 1.000001 * R.width(), std::string("x (1 - x) on ") + k.box + ": affine exact");
    } else {
      form = "x = " + text(x2) + " and x (1 - x) = " + text(logistic(x2));
    }
  }
  std::cout << "   width over the true width: " << interval_times << " with intervals, " << affine_times
            << " with affine forms\n";
  std::cout << "   on [0.2, 0.4], " << form << "\n";

  // ------------------------------------------------------------------------
  std::cout << "\n3. (x - 1)^5 expanded, by Horner's scheme, x in [0.99, 1.01] (true range [-1e-10, 1e-10])\n";

  // Near 1 the terms of the expanded polynomial, as large as 10, cancel down
  // to 1e-10. Affine forms cancel their first-order parts, not the products
  // of the noise symbol with itself, which Horner's scheme makes at each step.
  const interval X3("[0.99, 1.01]"), R3("[-1e-10, 1e-10]");
  const interval I3 = quintic(X3), A3 = quintic(Affine::variable(X3)).to_interval();
  // pow is in namespace gaol, not in gaol_core with the type interval:
  // pow(x, 5) needs using namespace gaol, or gaol::pow
  const interval P3 = pow(X3 - 1.0, 5);
  row("intervals", {text(I3)});
  row("affine forms", {text(A3), times(I3, A3) + " times narrower, " + times(A3, R3) + " times too wide"}, 30);
  row("pow(x - 1, 5)", {text(P3), "x once: the range, rounded outward"}, 30);
  check(I3.set_contains(R3) && A3.set_contains(R3) && P3.set_contains(R3), "(x - 1)^5: contains the range");
  check(100.0 * A3.width() < I3.width(), "(x - 1)^5: affine more than 100 times narrower");
  check(A3.width() > 1e5 * R3.width(), "(x - 1)^5: affine still far too wide");
  check(P3.width() < 1.000001 * R3.width(), "(x - 1)^5: pow gives the range");

  // ------------------------------------------------------------------------
  std::cout << "\n4. x^2 + y^2 - 2 x y = (x - y)^2 with x, y in [1, 2] (true range [0, 1])\n";

  // x and y are two variables: two symbols. The linear terms in e1 and e2
  // cancel; sqr(x), sqr(y) and x y leave errors that add up.
  const interval X4(1.0, 2.0), R4(0.0, 1.0);
  const Affine f4 = square_of_difference(Affine::variable(X4), Affine::variable(X4));
  const interval I4 = square_of_difference(X4, X4), A4 = f4.to_interval();
  row("intervals", {text(I4)});
  row("affine forms", {text(A4), "= " + text(f4) + ": no symbol left"});
  check(I4.set_contains(R4) && A4.set_contains(R4), "(x - y)^2: contains the range");
  check(A4.width() < I4.width() && f4.size() == 0, "(x - y)^2: affine narrower, no symbol left");

  // ------------------------------------------------------------------------
  std::cout << "\n5. exp(x) exp(-x) = 1 with x in [-0.1, 0.1]: exp(x) becomes a line and a new symbol\n";

  // exp(x) becomes alpha x + beta + delta e_new. Min-range takes for alpha
  // the smallest slope of exp over the box, Chebyshev the slope of its chord,
  // which halves delta; alpha is any double, the band [beta - delta,
  // beta + delta] being enclosed by interval evaluations of exp.
  const interval X5("[-0.1, 0.1]"), R5(1.0);
  const Affine x5 = Affine::variable(X5);
  const interval I5 = exp(X5) * exp(-X5);
  const interval M5 = (exp(x5, Linearization::min_range) * exp(-x5, Linearization::min_range)).to_interval();
  const Affine e5 = exp(x5);
  const interval C5 = (e5 * exp(-x5)).to_interval();
  std::cout << "   x = " << x5 << ", exp(x) = " << e5 << " (Chebyshev)\n";
  row("intervals", {text(I5)});
  row("affine, min-range", {text(M5)});
  row("affine, Chebyshev", {text(C5)});
  check(I5.set_contains(R5) && M5.set_contains(R5) && C5.set_contains(R5), "exp(x) exp(-x): contains 1");
  check(C5.width() < M5.width() && M5.width() < I5.width(), "exp(x) exp(-x): Chebyshev < min-range < intervals");
  check(x5.size() == 1 && e5.size() == 2, "exp(x): the symbol of x and a new one");

  // sqrt is concave: its Chebyshev line lies below it, the band above
  const interval X6(1.0, 4.0), R6(0.0, 2.0);
  const Affine x6 = Affine::variable(X6);
  const interval I6 = X6 - sqrt(X6), A6 = (x6 - sqrt(x6)).to_interval();
  std::cout << "   x - sqrt(x) with x in [1, 4] (true range [0, 2])\n";
  row("intervals", {text(I6)});
  row("affine, Chebyshev", {text(A6)});
  check(I6.set_contains(R6) && A6.set_contains(R6), "x - sqrt(x): contains the range");
  check(A6.width() < I6.width(), "x - sqrt(x): affine narrower");

  // ------------------------------------------------------------------------
  std::cout << "\n6. The wrapping effect: the box ([1, 2] ; [-0.5, 0.5]) turned k times by 45 degrees\n";
  std::cout << "     k  " << std::setw(46) << "intervals" << "affine forms\n";

  // Each rotation maps the box to a diamond; intervals keep the box around
  // it, whose next image is a larger diamond. Affine forms represent the
  // diamond itself (a zonotope, c + A e), and a linear map moves it exactly
  // but for rounding. cos(pi/4) = sin(pi/4) is irrational: c encloses it,
  // and each c * x holds the true product whichever real of c is the true one.
  const interval c = sqrt(interval(2.0)) / 2.0;
  const Box start{interval(1.0, 2.0), interval(-0.5, 0.5)};
  // The true images after 90, 180 and 360 degrees
  const Box quarter{interval(-0.5, 0.5), interval(1.0, 2.0)};
  const Box half{interval(-2.0, -1.0), interval(-0.5, 0.5)};
  Box b = start;
  Affine u = Affine::variable(start[0]), v = Affine::variable(start[1]);
  for (int k = 1; k <= 16; ++k) {
    b = Box{c * b[0] - c * b[1], c * b[0] + c * b[1]};
    const Affine turned = c * u - c * v;
    v = c * u + c * v;
    u = turned;
    if (k == 2 || k == 4 || k == 8 || k == 16) {
      const Box a{u.to_interval(), v.to_interval()};
      std::cout << "    " << std::right << std::setw(2) << k << "  " << std::left << std::setw(46) << text(b) << a
                << "\n";
      const Box& image = k == 2 ? quarter : k == 4 ? half : start;
      const std::string after = " after " + std::to_string(k) + " rotations";
      const double growth = std::pow(2.0, k / 2);
      check(b.contains(image) && a.contains(image), "both boxes contain the true image" + after);
      check(b.max_width() >= growth && b.max_width() < 1.000001 * growth, "the interval box is 2^(k/2) wide" + after);
      check(a.max_width() < 1.000001, "the affine box is the true image, 1 wide" + after);
    }
  }
  std::cout << "   the true images, the box turned by 90, 180 and 360 degrees, are 1 wide, as the affine\n"
               "   boxes; the interval boxes are 2^(k/2) wide: the wrapping effect\n";

  // ------------------------------------------------------------------------
  std::cout << "\n7. The affine enclosure & the interval one, as IBEX's AffineEval and INTLAB's affari do\n";
  row("x / (1 + x)", {"intervals", "affine forms", "intersection", "true range"});

  // On a wide box, the line of 1/t is far from the curve, and the error term
  // is wide: affine forms can do worse than intervals. The intersection of
  // both enclosures holds the range, and is never wider than either.
  // The true ranges are enclosed by GAOL itself: 2/3 and 10/11 are no
  // doubles, and one division encloses each of them tightly
  struct Recipe {
    const char* label;
    interval box, range;
    const char* range_text;
    bool affine_wins;  // else neither enclosure holds the other
  };
  const Recipe recipes[] = {
      {"x in [1, 2]", interval(1.0, 2.0), interval(0.5) | (interval(2.0) / 3.0), "[0.5, 2/3]", true},
      {"x in [0, 10]", interval(0.0, 10.0), interval(0.0) | (interval(10.0) / 11.0), "[0, 10/11]", false},
  };
  for (const Recipe& r : recipes) {
    const interval I = ratio(r.box), A = ratio(Affine::variable(r.box)).to_interval(), both = A & I;
    row(r.label, {text(I), text(A), text(both), r.range_text});
    const std::string on = std::string("x / (1 + x), ") + r.label;
    check(I.set_contains(r.range) && A.set_contains(r.range) && both.set_contains(r.range), on + ": contains the range");
    if (r.affine_wins) {
      check(I.set_contains(A) && A.width() < I.width(), on + ": the affine enclosure is inside the interval one");
    } else {
      check(!I.set_contains(A) && !A.set_contains(I) && both.width() < A.width() && both.width() < I.width(),
            on + ": the intersection is narrower than both");
    }
  }

  // x occurs once in 1 / x: intervals give the range, affine forms can only
  // lose, and the intersection is the interval enclosure
  const interval X7(1.0, 100.0), R7("[0.01, 1]");
  const interval I7 = 1.0 / X7, A7 = inv(Affine::variable(X7)).to_interval();
  row("1 / x, x in [1, 100]", {text(I7), text(A7), text(A7 & I7), "[0.01, 1]"});
  check(I7.set_contains(R7) && A7.set_contains(R7), "1 / x: contains the range");
  check(A7.width() > I7.width() && (A7 & I7).set_eq(I7), "1 / x: affine wider, the intersection is the interval");
  std::cout << "   on [1, 2], affine forms win; on [0, 10], the intersection beats both; with x once, intervals win\n";

  interval::precision(digits);
  gaol::cleanup();
  return all_checks_passed ? 0 : EXIT_FAILURE;
}
