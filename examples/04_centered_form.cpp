/*-*-C++-*------------------------------------------------------------------
 * gaol -- NOT Just Another Interval Library
 *--------------------------------------------------------------------------
 * Examples of GAOL v5: the natural interval extension of a function against
 * its mean-value (centered) form.
 *
 * The program encloses the range of f(x) = x cos(x) over [0, 1] in two ways.
 * The natural extension replaces each operation of f by its interval
 * counterpart. The centered form f(m) + f'(X)(X - m) takes an enclosure of
 * the derivative over the whole box, computed by the forward-mode automatic
 * differentiation of dual.h. Both enclose the range, and so does their
 * intersection, which Codac's AnalyticFunction::eval returns by default. The
 * natural form overestimates the width of the range by a term linear in the
 * width of the box, the centered form by a quadratic one: the centered form
 * loses on wide boxes and wins on small ones, which the program measures on
 * boxes around 0.5. The Chebyshev polynomial T5 then shows that the way f is
 * written matters as much as the form: expanded, in Horner form or as
 * cos(5 acos(y)), it gives three enclosures, the last one exact. Last, when
 * f'(X) does not hold 0, f is monotone on X and its range is f at the two
 * bounds, up to rounding. Every enclosure is checked against the true range,
 * computed with mpmath, and against f at 1001 points of the box.
 *
 * It follows Codac's manual (doc/manual/manual/functions/analytic/src.cpp,
 * block [6]: EvalMode::NATURAL gives [0, 1], CENTERED [-0.0612088, 0.938792]
 * and both [0, 0.938792]), Codac's examples/02_centered_form/main_rump.cpp
 * (T5 in INTLAB's verifynlssparam demo, dglobal) and chapter 6 of Moore,
 * Kearfott and Cloud, "Introduction to Interval Analysis" (SIAM, 2009): the
 * mean value form and the monotonicity test.
 *
 * Copyright (c) 2026 ENSTA, France
 *
 * Created 2026-09-27 by Jordan NININ
 *--------------------------------------------------------------------------
 * gaol is a software distributed WITHOUT ANY WARRANTY. Read the associated
 * COPYING file for information.
 *--------------------------------------------------------------------------*/

#include <algorithm>
#include <cstdio>
#include <cstdlib>
#include <iomanip>
#include <iostream>
#include <string>

#include <gaol/gaol.h>

#include "dual.h"

// The namespace gaol gives interval and GAOL's pow, which the expanded
// Chebyshev polynomial needs: argument-dependent lookup finds cos, acos and
// sqr on intervals, but not pow (see 14_generic_programming.cpp)
using namespace gaol;
using examples::Dual;

namespace {

  // Each function is written once, as a generic lambda: called on an interval
  // it computes the natural extension, on a Dual<1> the derivative as well
  const auto f = [](const auto& x) { return x * cos(x); };

  // The Chebyshev polynomial T5 written three ways. The coefficients are
  // written as doubles (16.0), as in the other examples; an int works too.
  const auto t5_expanded = [](const auto& y) { return 16.0 * pow(y, 5) - 20.0 * pow(y, 3) + 5.0 * y; };
  const auto t5_horner = [](const auto& y) {
    // sqr(y), not y * y, which takes the two factors as independent:
    // [-1, 1] * [-1, 1] is [-1, 1], sqr([-1, 1]) is [0, 1]
    const auto y2 = sqr(y);
    return y * (5.0 + y2 * (-20.0 + 16.0 * y2));
  };
  // cos(5 acos(y)) = T5(y) on [-1, 1]: y occurs once, so that the natural
  // extension is the exact range, up to rounding (no dependency to lose width
  // to). dual.h has no acos, whose derivative is unbounded at -1 and 1 anyway.
  const auto t5_trigo = [](const interval& y) { return cos(5.0 * acos(y)); };

  // The mean-value form. For x and m in X, f(x) = f(m) + f'(xi)(x - m) with xi
  // between m and x, so that f(x) lies in f(m) + f'(X)(X - m) as soon as f'(X)
  // encloses f' over X. m is X.mid(), the tightest interval holding the
  // midpoint of X, a point interval unless the midpoint is no double; the
  // form holds for each point of m. f(m) is computed with intervals, so that
  // its rounding errors are enclosed: X.midpoint() is the same point as a
  // double, on which f would give a double with no error bound.
  template <class F>
  interval centered_form(const F& g, const interval& X)
  {
    const interval m = X.mid();
    const interval dg = g(Dual<1>::variable(X, 0)).d[0];  // f'(X), by automatic differentiation
    return g(m) + dg * (X - m);
  }

  // The k-th of n + 1 evenly spaced points of X. GAOL leaves the rounding
  // direction upward until gaol::cleanup(), for the program's doubles too
  // (doc/using.md), so that a + k (b - a) / n can exceed b by one double:
  // std::min keeps the point in X.
  double sample(const interval& X, int k, int n)
  {
    const double a = X.left(), b = X.right();
    return std::min(a + k * ((b - a) / n), b);
  }

  // True when Y holds g([x]) at n + 1 points x of X. g([x]), g evaluated on
  // the point interval [x], encloses the real g(x), which every enclosure of
  // the range of g over X must hold. g is the tightest writing of the
  // function, so that g([x]) is a few doubles wide.
  template <class G>
  bool holds_samples(const G& g, const interval& X, const interval& Y, int n = 1000)
  {
    for (int k = 0; k <= n; ++k) {
      if (!Y.set_contains(g(interval(sample(X, k, n))))) {
        return false;
      }
    }
    return true;
  }

  // The exact width of Y, enclosed: Y.width() is a double rounded upward, an
  // upper bound only, from which no difference can be bounded below
  interval width_of(const interval& Y) { return interval(Y.right()) - interval(Y.left()); }

  // A table cell: the text of the interval, through its conversion to
  // std::string, padded to a fixed width
  std::string cell(const interval& Y, int w)
  {
    std::string s = Y;
    return s + std::string(s.size() < std::size_t(w) ? w - s.size() : 1, ' ');
  }

  bool all_passed = true;

  void check(bool passed, const char* what)
  {
    if (!passed) {
      std::cout << "FAILED: " << what << std::endl;
      all_passed = false;
    }
  }

} // namespace

int main()
{
  interval::precision(10);

  // -------------------------------------------------------------------------
  std::cout << "1. f(x) = x cos(x) on X = [0, 1], as in Codac's manual\n";
  // 0.0 rather than 0: interval(0, 0) does not compile, 0 being a null
  // const char* too. [0, 1] has bounds that are doubles, so that the
  // constructor from two doubles holds it exactly.
  const interval X(0.0, 1.0);
  const interval natural = f(X);
  const interval dfX = f(Dual<1>::variable(X, 0)).d[0];
  const interval centered = centered_form(f, X);
  const interval both = natural & centered;  // two enclosures: so is their intersection
  // The range of f on X, from mpmath: f(0) = 0 and the maximum, at x = 0.8603...
  const interval range = interval(0.0) | textToInterval("0.56109633819104506754040375316122670886");
  std::cout << "   natural   f(X)                  = " << natural << "\n"
            << "   f'(X), computed by dual.h       = " << dfX << "\n"
            << "   centered  f(m) + f'(X) (X - m)  = " << centered << "\n"
            << "   both      natural & centered    = " << both << "\n"
            << "   range of f over X (mpmath)      = " << range << "\n";
  check(natural.set_contains(range) && centered.set_contains(range) && both.set_contains(range),
        "the three enclosures hold the range of x cos(x) over [0, 1]");
  check(holds_samples(f, X, both), "f at 1001 points of [0, 1] lies in natural & centered");
  // In exact arithmetic, f'(X) = [cos 1 - sin 1, 1] and the centered form is
  // 0.5 cos 0.5 + [-0.5, 0.5]: GAOL's result holds that interval, and lies in
  // the one Codac's manual prints with 6 digits
  check(centered.set_contains(textToInterval("[-0.06120871905481364194185920869808517400418,"
                                             " 0.9387912809451863580581407913019148259958]")) &&
          textToInterval("[-0.0612088, 0.938792]").set_contains(centered),
        "the centered form is Codac's [-0.0612088, 0.938792]");
  check(dfX.set_contains(textToInterval("-0.30116867893975678925")), "f'(X) holds f'(1) = cos 1 - sin 1");
  std::cout << "   each holds the range and f at 1001 points of X; Codac prints [-0.0612088, 0.938792]\n";

  // -------------------------------------------------------------------------
  std::cout << "2. Monotonicity: when 0 is not in f'(X), the range is f at the bounds of X\n";
  // textToInterval() encloses the decimal 0.8, which no double equals
  const interval X2 = textToInterval("[0, 0.8]");
  const interval dfX2 = f(Dual<1>::variable(X2, 0)).d[0];
  // > is "certainly greater": every element of f'(X) is above 0, f increases
  // on X. The empty set passes every certainly-test, hence the test apart: an
  // f' undefined somewhere on X would give it. The bounds are evaluated as
  // point intervals, so that their rounding errors are enclosed; the hull of
  // the two would cover a decreasing f (f'(X) < 0.0) as well.
  const bool increasing = !dfX2.is_empty() && dfX2 > 0.0;
  const interval at_bounds = f(interval(X2.left())) | f(interval(X2.right()));
  const interval f08 = textToInterval("0.55736536747773233673659998531385994088");  // f(0.8), mpmath
  std::cout << "   X = [0, 0.8]: f'(X) = " << dfX2 << ", above 0: f increases on X\n"
            << "     natural            " << f(X2) << "\n"
            << "     centered           " << centered_form(f, X2) << "\n"
            << "     f([0]) | f([0.8])  " << at_bounds << "  the range, to rounding (f(0.8) = 0.5573653675)\n";
  check(increasing && at_bounds.set_contains(interval(0.0) | f08), "f'([0, 0.8]) > 0 and f([0]) | f([0.8]) holds the range");
  check(at_bounds.right() - f08.left() < 1e-12, "f([0]) | f([0.8]) is the range up to rounding");
  check(holds_samples(f, X2, at_bounds), "f at 1001 points of [0, 0.8] lies in f([0]) | f([0.8])");
  check(width_of(at_bounds) < width_of(f(X2) & centered_form(f, X2)), "the monotonicity test beats both forms");
  // On [0, 1], f'(X) holds 0: f has its maximum inside X, at 0.8603, and the
  // test concludes nothing. A solver would bisect X.
  check(dfX.set_contains(0.0), "f'([0, 1]) holds 0");
  std::cout << "   X = [0, 1]:   f'(X) holds 0: no conclusion (f has its maximum at 0.8603, inside X)\n";

  // -------------------------------------------------------------------------
  std::cout << "3. Chebyshev T5(y) = 16y^5 - 20y^3 + 5y: the way f is written matters\n";
  interval::precision(4);
  const interval Y1(-1.0, 1.0), Y2 = textToInterval("[0.2, 0.3]");
  // The true ranges: T5 = cos(5 acos(y)) takes every value of [-1, 1] on
  // [-1, 1]; on [0.2, 0.3] it increases, from T5(0.2) = 0.84512 to T5(0.3) =
  // 0.99888 (exact decimals: T5 has integer coefficients)
  const interval range1(-1.0, 1.0), range2 = textToInterval("[0.84512, 0.99888]");
  struct Writing {
    const char* name;
    interval on_y1, on_y2;
  };
  const Writing writings[] = {
    {"expanded, with pow", t5_expanded(Y1), t5_expanded(Y2)},
    {"Horner in y^2", t5_horner(Y1), t5_horner(Y2)},
    {"centered, Horner", centered_form(t5_horner, Y1), centered_form(t5_horner, Y2)},
    {"cos(5 acos(y))", t5_trigo(Y1), t5_trigo(Y2)},
  };
  std::cout << "                          Y = [-1, 1]       Y = [0.2, 0.3]\n";
  for (const Writing& w : writings) {
    std::cout << "   " << std::left << std::setw(23) << w.name << cell(w.on_y1, 18) << std::string(w.on_y2) << "\n";
    check(w.on_y1.set_contains(range1) && w.on_y2.set_contains(range2), "every writing of T5 holds its range");
    check(holds_samples(t5_trigo, Y1, w.on_y1) && holds_samples(t5_trigo, Y2, w.on_y2),
          "T5 at 1001 points lies in every enclosure");
  }
  std::cout << "   " << std::setw(23) << "range (exact)" << cell(range1, 18) << std::string(range2) << "\n";
  const auto wider = [](const interval& a, const interval& b) { return width_of(a) > width_of(b); };
  check(wider(writings[0].on_y1, writings[1].on_y1) && wider(writings[1].on_y1, writings[3].on_y1),
        "on [-1, 1], expanded is wider than Horner, wider than cos(5 acos(y))");
  check(wider(writings[2].on_y1, writings[1].on_y1) && wider(writings[1].on_y2, writings[2].on_y2) &&
          wider(writings[0].on_y2, writings[2].on_y2),
        "the centered form loses to Horner on [-1, 1] and beats both polynomial writings on [0.2, 0.3]");
  check(writings[3].on_y1.set_eq(range1) && width_of(writings[3].on_y2).right() < width_of(range2).left() + 1e-12,
        "cos(5 acos(y)) gives the range, up to rounding");
  std::cout << "   y occurs once in cos(5 acos(y)): no dependency, the exact range;\n"
            << "   the centered form is the worst on the wide box, beats both polynomials on the small one\n";

  // -------------------------------------------------------------------------
  // 4. Boxes [0.5 - r, 0.5 + r]: how much wider than the range each form is.
  // f increases there, and its range is [f(0.5 - r), f(0.5 + r)], from mpmath.
  // The boxes enclose the decimals, each bound one double at most beyond: the
  // excess measured includes that, some 1e-16, far below what is measured.
  struct Shrinking {
    const char* r;
    const char* box;
    const char* f_left;
    const char* f_right;
  };
  const Shrinking boxes[] = {
    {"0.1", "[0.4, 0.6]", "0.36842439760115403311941069282072064561", "0.49520136894580697834457149937322562333"},
    {"0.01", "[0.49, 0.51]", "0.43234310071895953289567939797696237279", "0.44509969889933314418396232152350865765"},
    {"0.001", "[0.499, 0.501]", "0.43815271273035631542768756763315184827", "0.43942845151785462153997295334993011129"},
  };
  double natural_excess[3] = {}, centered_excess[3] = {}, natural_ratio[3] = {}, centered_ratio[3] = {};
  interval previous_natural, previous_centered;
  for (int i = 0; i < 3; ++i) {
    const interval Xr = textToInterval(boxes[i].box);
    const interval lo = textToInterval(boxes[i].f_left), hi = textToInterval(boxes[i].f_right);
    const interval nat = f(Xr), cen = centered_form(f, Xr);
    check(nat.set_contains(lo | hi) && cen.set_contains(lo | hi), "both forms hold the range on [0.5 - r, 0.5 + r]");
    // Enclosures of the excesses, then their midpoints to print
    const interval en = width_of(nat) - (hi - lo), ec = width_of(cen) - (hi - lo);
    check(ec < en, "the centered form is narrower than the natural one on [0.5 - r, 0.5 + r]");
    natural_excess[i] = en.midpoint();
    centered_excess[i] = ec.midpoint();
    if (i > 0) {
      // Dividing r by 10 divides a linear excess by 10, a quadratic one by 100
      const interval qn = previous_natural / en, qc = previous_centered / ec;
      check(5.0 < qn && qn < 20.0, "the natural excess is linear in r");
      check(50.0 < qc && qc < 200.0, "the centered excess is quadratic in r");
      natural_ratio[i] = qn.midpoint();
      centered_ratio[i] = qc.midpoint();
    }
    previous_natural = en;
    previous_centered = ec;
  }

  // GAOL is not used below. gaol::cleanup() sets the rounding direction back
  // to the one the program started with, to nearest, so that the doubles of
  // the table are printed rounded to nearest rather than upward.
  gaol::cleanup();

  std::printf("4. Boxes [0.5 - r, 0.5 + r]: width of each form minus the width of the range\n");
  std::printf("       r      natural   ratio      centered   ratio\n");
  for (int i = 0; i < 3; ++i) {
    // The ratios to the line above, from the second line on
    if (i == 0) {
      std::printf("   %5s   %10.3g           %11.3g\n", boxes[i].r, natural_excess[i], centered_excess[i]);
    } else {
      std::printf("   %5s   %10.3g   %5.1f   %11.3g   %5.1f\n", boxes[i].r, natural_excess[i], natural_ratio[i],
                  centered_excess[i], centered_ratio[i]);
    }
  }
  std::printf("   r / 10: the natural excess / 10 (linear), the centered one / 100 (quadratic)\n");

  if (!all_passed) {
    return EXIT_FAILURE;
  }
  std::printf("All checks passed.\n");
  return 0;
}
