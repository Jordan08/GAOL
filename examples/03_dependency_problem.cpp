/*-*-C++-*------------------------------------------------------------------
 * gaol -- NOT Just Another Interval Library
 *--------------------------------------------------------------------------
 * Examples of GAOL v5: the dependency problem, and subdivision.
 *
 * Evaluating a formula with intervals encloses the range of the function it
 * writes (the fundamental theorem of interval arithmetic), but usually
 * overestimates it: every occurrence of a variable ranges over its interval
 * on its own, as if the others did not exist. The program shows it on
 * x - x, on three forms of x (1 - x) (R. E. Moore, R. B. Kearfott and
 * M. J. Cloud, "Introduction to Interval Analysis", SIAM, 2009: an
 * expression where each variable occurs once gives the exact range), on
 * x * x against sqr(x), and on (x - 1)^5 evaluated by Horner's scheme, as
 * filib++'s examples/horner.cc evaluates its polynomials, against
 * pow(x - 1, 5). It then cuts the domain into n pieces of the same width,
 * as IBEX's examples/lab1.cpp does to draw the image of a box, and shows
 * the hull of the pieces converging to the true range, linearly in the
 * width of the pieces: on x (1 - x), and on the Goldstein-Price function on
 * [-2, 2]^2, whose true range is [3, 1015690.2717980589...]. Its minimum is
 * f(0, -1) = 3; its maximum lies on the edge y = 2, at
 * x = -1.7373725377583070..., found apart by solving df(x, 2)/dx = 0
 * exactly (sympy), and confirmed by a grid of 4001 x 4001 points (numpy)
 * and by an interval branch and bound. It ends on the wrapping effect
 * (R. E. Moore, 1965): rotating the box [-1, 1]^2 eight times by 45 degrees
 * gives a box 16 times wider, where the true image is the box itself. The
 * other true ranges follow from the monotony of the functions on the pieces
 * of their domains, and were checked with mpmath.
 *
 * Copyright (c) 2026 ENSTA, France
 *
 * Created 2026-09-27 by Jordan NININ
 *--------------------------------------------------------------------------
 * gaol is a software distributed WITHOUT ANY WARRANTY. Read the associated
 * COPYING file for information.
 *--------------------------------------------------------------------------*/

#include <algorithm>
#include <cfenv>
#include <cstdio>
#include <cstdlib>
#include <iomanip>
#include <iostream>
#include <string>

#include <gaol/gaol.h>

#include "box.h"

using namespace gaol;
using examples::Box;

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

  /*
    A double with two significant digits, for widths and ratios. GAOL rounds
    the doubles of the whole program upward, and with glibc their conversion
    to text as well: 20.000000000000004 would be written 21. The conversion is
    made rounding to nearest, which GAOL allows: each of its operations sets
    the direction upward again when it is not.
  */
  std::string approx(double d)
  {
    const int direction = std::fegetround();
    std::fesetround(FE_TONEAREST);
    char text[32];
    std::snprintf(text, sizeof text, "%.2g", d);
    std::fesetround(direction);
    return text;
  }

  // A line of a table: a label, an interval, a note. std::string(x) writes x
  // as operator<< does, into one string, which std::setw pads to a column.
  void row(const std::string& label, const interval& x, const std::string& note)
  {
    std::cout << "    " << std::left << std::setw(22) << label << std::setw(48) << std::string(x) << note << "\n";
  }

  /*
    The k-th of the n + 1 points that cut x into n pieces of the same width.
    The two pieces on each side of a point get it from the same expression,
    and the last point is the bound of x itself, so that the pieces cover x
    whatever the rounding of this computation with doubles; std::min keeps a
    point within x, which rounding upward could otherwise pass.
  */
  double cut(const interval& x, int k, int n)
  {
    return k == n ? x.right() : std::min(x.left() + k * (x.right() - x.left()) / n, x.right());
  }

  interval piece(const interval& x, int k, int n)
  {
    return interval(cut(x, k, n), cut(x, k + 1, n));
  }

  // The hull of f over the n pieces of x: each piece is enclosed and the
  // pieces cover x, so that the hull encloses the range of f over x
  template <class F>
  interval mince(const F& f, const interval& x, int n)
  {
    interval hull = interval::emptyset();
    for (int k = 0; k < n; ++k) {
      hull |= f(piece(x, k, n));
    }
    return hull;
  }

  /*
    The Goldstein-Price function, a classic test of global optimization.
    examples/Goldstein_Price.cpp and the manual of GAOL drop the + 1 of
    (x + y + 1)^2, and compute another function.
  */
  interval goldstein_price(const interval& x, const interval& y)
  {
    return (1.0 + sqr(x + y + 1.0)
                  * (19.0 - 14.0 * x + 3.0 * sqr(x) - 14.0 * y + 6.0 * x * y + 3.0 * sqr(y)))
         * (30.0 + sqr(2.0 * x - 3.0 * y)
                   * (18.0 - 32.0 * x + 12.0 * sqr(x) + 48.0 * y - 36.0 * x * y + 27.0 * sqr(y)));
  }

} // namespace

int main()
{
  // ------------------------------------------------------------------------
  std::cout << "x - x: the two occurrences of x vary independently\n";

  // x - x is computed as {a - b : a in x, b in x}: the subtraction does not
  // know that both operands are the same number. This is not rounding, which
  // is exact here, but the price of computing with sets.
  const interval unit(0.0, 1.0);
  row("x - x on [0, 1]", unit - unit, "the true range is [0, 0]");
  check((unit - unit).set_eq(interval(-1.0, 1.0)), "x - x on [0, 1] is [-1, 1]");

  // ------------------------------------------------------------------------
  std::cout << "\nf(x) = x (1 - x) written three ways\n";

  // Each form encloses the range of f; how much wider depends on how often x
  // occurs, each occurrence bringing the whole width of x. With x once, the
  // enclosure is the range itself, rounded outward.
  const auto natural = [](const interval& x) { return x * (1.0 - x); };
  const auto expanded = [](const interval& x) { return x - sqr(x); };
  const auto single_use = [](const interval& x) { return 0.25 - sqr(x - 0.5); };
  struct Domain {
    const char* text;  // the domain and the true range of f over it
    interval x, range;
  };
  // [0.4, 0.6] is read as text, like its range: the interval then holds the
  // real numbers from 2/5 to 3/5, which are no doubles, and the range too
  const Domain domains[] = {
    {"on [0, 1], true range [0, 0.25]", interval(0.0, 1.0), interval(0.0, 0.25)},
    {"on [0.4, 0.6], true range [0.24, 0.25]", textToInterval("[0.4, 0.6]"), textToInterval("[0.24, 0.25]")},
  };
  for (const Domain& d : domains) {
    std::cout << "  " << d.text << "\n";
    const interval fx[] = {natural(d.x), expanded(d.x), single_use(d.x)};
    const char* forms[] = {"x * (1 - x)", "x - sqr(x)", "0.25 - sqr(x - 0.5)"};
    for (int i = 0; i < 3; ++i) {
      const double times = fx[i].width() / d.range.width();
      row(forms[i], fx[i], i == 2 ? "x once: exact" : approx(times) + " times too wide");
      check(fx[i].set_contains(d.range), std::string(forms[i]) + " " + d.text + ": contains the range");
    }
    check(fx[2].width() < 1.000001 * d.range.width(), std::string("0.25 - sqr(x - 0.5) ") + d.text + ": exact");
  }

  // ------------------------------------------------------------------------
  std::cout << "\nx * x or sqr(x), on [-1, 2] (true range [0, 4])\n";

  // x * x multiplies two intervals whose elements are chosen apart: -1 * 2 is
  // in it, though no square is negative. sqr(x) squares one number: its
  // result is the range. GAOL may write a zero bound as -0 (its SSE2 code does
  // here); for an interval, -0 and 0 are the same bound.
  const interval w(-1.0, 2.0);
  row("x * x", w * w, "has negative numbers");
  row("sqr(x)", sqr(w), "exact");
  check((w * w).set_contains(interval(0.0, 4.0)) && (w * w).left() < 0.0,
        "x * x on [-1, 2] contains [0, 4] and a negative number");
  check(sqr(w).set_eq(interval(0.0, 4.0)), "sqr(x) on [-1, 2] is [0, 4]");

  // ------------------------------------------------------------------------
  std::cout << "\n(x - 1)^5 on [0.99, 1.01] (true range [-1e-10, 1e-10])\n";

  // Expanded, (x - 1)^5 is x^5 - 5 x^4 + 10 x^3 - 10 x^2 + 5 x - 1, which
  // Horner's scheme evaluates with x five times. Near 1 the terms, as large as
  // 10, cancel down to 1e-10 for each x; the five independent occurrences of x
  // cannot cancel, and each brings its width 0.02 times the size of what it
  // multiplies.
  const interval x = textToInterval("[0.99, 1.01]");
  const interval horner = ((((x - 5.0) * x + 10.0) * x - 10.0) * x + 5.0) * x - 1.0;
  // pow is not in namespace gaol_core of the type interval, where
  // argument-dependent lookup finds sqr() or sin(), but in namespace gaol:
  // pow(x, 5) needs using namespace gaol, or gaol::pow
  const interval power = pow(x - 1.0, 5);
  const interval range = textToInterval("[-1e-10, 1e-10]");
  row("Horner's scheme", horner, "x five times");
  row("pow(x - 1, 5)", power, "x once");
  std::cout << "    Horner's scheme gives an enclosure " << approx(horner.width() / power.width()) << " times wider\n";
  check(horner.set_contains(range) && power.set_contains(range), "both forms of (x - 1)^5 contain the range");
  check(horner.width() > 1e8 * power.width(), "Horner's scheme is more than 1e8 times wider than pow");

  // ------------------------------------------------------------------------
  std::cout << "\nx * (1 - x) on n pieces of [0, 1] (true range [0, 0.25])\n";

  // On a piece of width 1/n, each occurrence of x brings a width 1/n: the
  // excess over the true range decreases like 1/n. The convergence is linear,
  // which makes subdivision alone an expensive cure (04_centered_form does
  // better).
  double excess_before = 0.0;
  for (int n : {1, 10, 100, 1000}) {
    const interval hull = mince(natural, unit, n);
    const double excess = hull.width() - 0.25;
    row("n = " + std::to_string(n), hull, "width " + approx(excess) + " in excess");
    check(hull.set_contains(interval(0.0, 0.25)), "x * (1 - x) on " + std::to_string(n) + " pieces contains the range");
    check(n == 1 || (excess < excess_before / 5.0 && excess > excess_before / 20.0),
          "10 times more pieces: about 10 times less excess, n = " + std::to_string(n));
    excess_before = excess;
  }
  std::cout << "    10 times more pieces, about 10 times less excess: linear convergence\n";

  // ------------------------------------------------------------------------
  std::cout << "\nGoldstein-Price on n x n boxes of [-2, 2]^2 (true range [3, 1015690.2717980589])\n";

  // In two dimensions, n x n boxes: dividing the excess by 4 costs 16 times
  // as many evaluations
  const interval side(-2.0, 2.0);
  const interval gp_range = interval(3.0) | interval(1015690.2717980589082988423120822331039464707651154);
  // The minimum 3 is f(0, -1): this point evaluation is exact
  check(goldstein_price(interval(0.0), interval(-1.0)).set_eq(interval(3.0)), "f(0, -1) = 3");
  excess_before = 0.0;
  for (int n : {1, 4, 16, 64}) {
    interval hull = interval::emptyset();
    for (int i = 0; i < n; ++i) {
      for (int j = 0; j < n; ++j) {
        hull |= goldstein_price(piece(side, i, n), piece(side, j, n));
      }
    }
    const double excess = hull.width() - gp_range.width();
    row(std::to_string(n * n) + (n == 1 ? " box" : " boxes"), hull, "width " + approx(excess) + " in excess");
    check(hull.set_contains(gp_range), "Goldstein-Price on " + std::to_string(n * n) + " boxes contains the range");
    check(n == 1 || excess < excess_before / 4.0,
          "4 times narrower boxes: at least 4 times less excess, n = " + std::to_string(n));
    excess_before = excess;
  }
  std::cout << "    4 times narrower boxes, at least 4 times less excess, for 16 times as many boxes\n";

  // ------------------------------------------------------------------------
  std::cout << "\nThe box [-1, 1]^2 rotated by 45 degrees 8 times, a whole turn\n";

  // The image of a box by a rotation of 45 degrees is a diamond; the smallest
  // box around it is sqrt(2) times wider, and the next rotation starts from
  // that box, not from the diamond. After a whole turn, the true image is the
  // box itself, and the interval box is sqrt(2)^8 = 16 times wider: the
  // wrapping effect, against which Lohner's QR method, affine arithmetic
  // (12_affine_arithmetic) and zonotopes keep the shape of the set.
  const interval c = sqrt(interval(2.0)) / 2.0;  // cos 45 = sin 45, enclosed
  const Box square{interval(-1.0, 1.0), interval(-1.0, 1.0)};
  Box b = square;
  // 6 digits: GAOL rounds the bounds outward when writing them, so that the
  // text still encloses the box; -16.00000000000003 is written -16.0001
  const std::streamsize digits = interval::precision(6);
  for (int k = 1; k <= 8; ++k) {
    b = Box{c * b[0] - c * b[1], c * b[0] + c * b[1]};
    std::cout << "    " << std::right << std::setw(3) << 45 * k << " degrees  " << b << "\n";
  }
  interval::precision(digits);
  check(b.contains(square), "the rotated box contains the true image");
  check(b.max_width() >= 16.0 * 2.0, "the rotated box is 16 times wider");

  gaol::cleanup();
  return all_checks_passed ? 0 : EXIT_FAILURE;
}
