/*-*-C++-*------------------------------------------------------------------
 * gaol -- NOT Just Another Interval Library
 *--------------------------------------------------------------------------
 * Tests of GAOL v5: the other functions on intervals.
 *
 * On random intervals, and for the midpoints on intervals of subnormal bounds,
 * compared exactly with the exact results: midpoints (of gaol::intervalf too,
 * where a developer of GAOL compiles it, see gaol/gaol_config.h),
 * widths, magnitudes and mignitudes, Hausdorff distances (of intervals with
 * infinite bounds too), nb_fp_numbers() across the two zeros, splitting, integer
 * parts, radii; the tools of TODO point P: width_enclosure(), inflate(),
 * interval::midrad(), bisect(), is_bisectable(), hull() and intersect(); the
 * comparisons of IEEE 1788-2015 (Tables 10.3 and 10.4); and
 * the relational functions (sqrt_rel, div_rel...), which have to keep the
 * values they are given and bound them within a few doubles; and the reverse
 * functions of max, min, sign and floor (max_rel, min_rel, sign_rel,
 * floor_rel), which have to be the hulls of their sets, read from their
 * definitions on a grid and by sampling (GAOL v5).
 *
 * Copyright (c) 2026 ENSTA, France
 *
 * Created 2026-09-20 by Jordan NININ
 *--------------------------------------------------------------------------
 * gaol is a software distributed WITHOUT ANY WARRANTY. Read the associated
 * COPYING file for information.
 *--------------------------------------------------------------------------*/

#include "gaol_tests.h"
#ifdef GAOL_FLOAT_INTERVALS
// Not included by gaol/gaol (see gaol/gaol_config.h)
#  include "gaol/gaol_intervalf.h"
#endif

#include <algorithm>
#include <type_traits>
#include <utility>

using namespace gaol;
using namespace gaol_tests;

namespace
{
  const int nb_random_values = 5000;

  // The largest distance from the tightest bounds allowed, in doubles, about
  // twice the 26 found on the platforms tested: the relational functions bound
  // the preimage of an enclosure of the image, as tightly as it allows
  const int limit = 64;
  // The relational functions of the periodic functions: about twice the 4
  // found, which the width of f([x]) magnified by the inverse function gives
  const int periodic_limit = 6;

  // The radius, rad of IEEE 1788-2015 (12.12.8): the smallest double r such
  // that [x] is in [m - r, m + r], m being the midpoint; mid_rad() gives both
  void radius(const interval& X, const std::string& in)
  {
    const double l = X.left(), u = X.right(), m = X.midpoint(), r = X.rad();
    const auto describe = [&] { return hex(X) + ": " + hex(r); };
    const auto covers = [&](double t) {
      return compare(dyadic(m) - dyadic(t), dyadic(l)) <= 0 && compare(dyadic(m) + dyadic(t), dyadic(u)) >= 0;
    };
    if (check("rad() encloses [x] around the midpoint" + in, r >= 0.0 && covers(r), describe)) {
      check("rad() the smallest such radius" + in, r == 0.0 || !covers(previous_double(r)), describe);
    }
    double mm, rr;
    X.mid_rad(mm, rr);
    check("mid_rad() the midpoint and the radius" + in, mm == m && rr == r, describe);
  }

  // The midpoint, rounded to nearest: within the interval, and one of the
  // doubles on each side of the exact midpoint; and mid(), the tightest interval
  // enclosing it
  void midpoints(const interval& X, const std::string& in)
  {
    const double l = X.left(), r = X.right(), m = X.midpoint();
    const Exact middle = quotient(dyadic(l) + dyadic(r), dyadic(2.0));
    check("midpoint() within [x]" + in, l <= m && m <= r, [&] { return hex(X) + ": " + hex(m); });
    check("midpoint() next to the exact midpoint" + in,
          is_tightest_lower_bound(m, middle) || is_tightest_upper_bound(m, middle),
          [&] { return hex(X) + ": " + hex(m); });
    const interval mid = X.mid();
    check("mid() the tightest enclosure of the exact midpoint" + in, is_tightest_enclosure(mid, middle),
          [&] { return hex(X) + ": " + hex(mid); });
    radius(X, in);
  }

  template<class Draw>
  void measures(const std::string& range, Draw draw)
  {
    const std::string in = " (" + range + ")";
    for (int i = 0; i < nb_random_values; ++i) {
      const interval X = hull(draw(), draw()), Y = hull(draw(), draw());
      const double l = X.left(), r = X.right();
      const Dyadic dl = dyadic(l), dr = dyadic(r);
      const auto describe = [&] { return "x=" + hex(X) + " y=" + hex(Y); };

      midpoints(X, in);
      const double m = X.midpoint();

      // The width, rounded upward
      const double w = X.width();
      check("width() the tightest upper bound" + in, is_tightest_upper_bound(w, exact(dr - dl)),
            [&] { return describe() + ": " + hex(w); });

      // Magnitude and mignitudes, exact
      const double ml = std::fabs(l), mr = std::fabs(r);
      check("mag()" + in, X.mag() == std::max(ml, mr), describe);
      const bool has_zero = X.set_contains(0.0);
      check("mig()" + in, X.mig() == (has_zero ? 0.0 : std::min(ml, mr)), describe);
      check("smig()" + in, X.smig() == (has_zero ? 0.0 : (l > 0.0 ? l : r)), describe);

      // The Hausdorff distance, rounded upward: GAOL computed it in the rounding
      // direction in effect, and |a - c| rounded upward below the exact distance
      // when a - c < 0
      const double h = hausdorff(X, Y);
      const Exact dleft = exact(dyadic(l) - dyadic(Y.left())), dright = exact(dyadic(r) - dyadic(Y.right()));
      const std::vector<Exact> distances = { dleft, -dleft, dright, -dright };
      check("hausdorff() the tightest upper bound of the exact distance" + in,
            is_tightest_upper_bound(h, max(distances)),
            [&] { return describe() + ": " + hex(h); });

      // Splitting at the midpoint
      interval left, right;
      X.split(left, right);
      check("split()" + in, left.left() == l && left.right() == m && right.left() == m && right.right() == r,
            [&] { return describe() + ": " + hex(left) + " " + hex(right); });
      check("split_left() and split_right()" + in, X.split_left().set_eq(left) && X.split_right().set_eq(right), describe);

      // The tools of TODO point P (GAOL v5): the enclosure of the width, X
      // widened by a radius and the interval of a midpoint and a radius, each
      // the tightest enclosure of its exact bounds, which GAOL computed
      // rounding both upward; the parts of bisect(), and hull() and
      // intersect(), the operators | and &
      const interval we = X.width_enclosure();
      check("width_enclosure() the tightest enclosure of the width, width() its upper bound" + in,
            is_tightest_enclosure(we, exact(dr - dl)) && we.right() == w,
            [&] { return describe() + ": " + hex(we); });
      const double rad = std::fabs(Y.right());
      const Dyadic drad = dyadic(rad);
      const interval wider = X.inflate(rad);
      check("inflate(r) the tightest enclosure of [l - r, r + r]" + in,
            is_tightest_enclosure(wider, exact(dl - drad), exact(dr + drad)),
            [&] { return describe() + " r=" + hex(rad) + ": " + hex(wider); });
      const interval around = interval::midrad(l, rad);
      check("interval::midrad(m, r) the tightest enclosure of [m - r, m + r]" + in,
            is_tightest_enclosure(around, exact(dl - drad), exact(dl + drad)),
            [&] { return describe() + " r=" + hex(rad) + ": " + hex(around); });
      const double ratios[] = { 0.5, 0.25, 0.75, 1.0/3.0, 1e-9, 1.0 - 1e-9 };
      const double ratio = ratios[i % 6];
      const std::pair<interval, interval> parts = X.bisect(ratio);
      const double c = parts.first.right();
      const bool bisectable = next_double(l) < r;
      const auto describe_parts = [&] { return describe() + " ratio " + hex(ratio) + ": " + hex(parts.first) + " "
                                               + hex(parts.second); };
      check("bisect(ratio): two parts sharing a point of X, which they cover" + in,
            parts.first.left() == l && parts.second.left() == c && parts.second.right() == r && l <= c && c <= r,
            describe_parts);
      check("is_bisectable(): a double strictly between the bounds" + in, X.is_bisectable() == bisectable, describe);
      check("bisect(ratio) of a bisectable interval: two parts smaller than X" + in, !bisectable || (l < c && c < r),
            describe_parts);
      check("bisect(0.5) cuts as split()" + in, X.bisect(0.5).first.set_eq(left) && X.bisect(0.5).second.set_eq(right),
            describe);
      check("hull() and intersect(): X | Y and X & Y" + in, hull(X, Y).set_eq(X | Y) && intersect(X, Y).set_eq(X & Y),
            describe);

      // Integer parts, exact
      check("floor()" + in, floor(X).left() == std::floor(l) && floor(X).right() == std::floor(r), describe);
      check("ceil()" + in, ceil(X).left() == std::ceil(l) && ceil(X).right() == std::ceil(r), describe);
      const interval n = integer(X);
      check("integer()" + in, std::ceil(l) <= std::floor(r)
                              ? (n.left() == std::ceil(l) && n.right() == std::floor(r)) : n.is_empty(), describe);

      check("set_contains(double)" + in, X.set_contains(m) && X.set_contains(l) && X.set_contains(r)
                                         && !X.set_contains(previous_double(l)) && !X.set_contains(next_double(r)), describe);
    }

    const interval unbounded[] = { interval::universe(), interval(-inf, 1.), interval(1., inf) };
    const double midpoints[] = { 0.0, -std::numeric_limits<double>::max(), std::numeric_limits<double>::max() };
    for (int i = 0; i < 3; ++i) {
      check("midpoint() of unbounded intervals", unbounded[i].midpoint() == midpoints[i],
            [&] { return hex(unbounded[i]) + ": " + hex(unbounded[i].midpoint()); });
    }
    check("width() of unbounded intervals", interval(-inf, 1.).width() == inf);
    check("midpoint() of the empty set", std::isnan(interval::emptyset().midpoint()));
    check("rad() of the empty set", std::isnan(interval::emptyset().rad()));
    check("rad() of unbounded intervals", interval::universe().rad() == inf && interval(-inf, 1.).rad() == inf
                                          && interval(1., inf).rad() == inf);
    check("rad() of [-MAX, MAX]", interval(-std::numeric_limits<double>::max(), std::numeric_limits<double>::max()).rad()
                                  == std::numeric_limits<double>::max());
    {
      double m, r;
      interval::emptyset().mid_rad(m, r);
      check("mid_rad() of the empty set", std::isnan(m) && std::isnan(r));
    }
    // NaN, as wid of IEEE 1788-2015 (12.12.8): GAOL returned -1
    check("width() of the empty set", std::isnan(interval::emptyset().width()));
    const double a = 0.1;
    check("is_canonical()", interval(a, next_double(a)).is_canonical() && interval(a).is_canonical()
                            && !interval(a, next_double(next_double(a))).is_canonical());
    check("nb_fp_numbers()", nb_fp_numbers(a, a) == 1 && nb_fp_numbers(a, next_double(a)) == 2
                             && nb_fp_numbers(1.0, 2.0) == 4503599627370497ull);
  }

  /* The tools of TODO point P on special intervals and arguments (GAOL v5):
     the empty set, points, zeros, infinite bounds and radii, NaN, the
     largest double, and the cases the random draws seldom give: an inexact
     bound, which GAOL rounded upward on both sides (the lower bound of
     [1, 2] inflated by 1e-20 was 1), the whole line, which bisect() cut into
     two empty sets, and two consecutive doubles. */
  void tools_of_point_p()
  {
    const double max = std::numeric_limits<double>::max();
    const interval empty = interval::emptyset(), one_two(1.0, 2.0);

    // The width: [MAX, +oo] for +oo, as textToInterval() reads inf
    check("width_enclosure() of the empty set", empty.width_enclosure().is_empty());
    check("width_enclosure() of points", interval(3.0).width_enclosure().set_eq(interval(0.0))
                                         && interval(-0.0, 0.0).width_enclosure().set_eq(interval(0.0)));
    const interval w = interval(1e-20, 1.0).width_enclosure();
    check("width_enclosure() of [1e-20, 1]: [1 - u/2, 1]", w.left() == previous_double(1.0) && w.right() == 1.0,
          [&] { return hex(w); });
    const interval unbounded[] = { interval::universe(), interval(-inf, 1.0), interval(1.0, inf) };
    for (const interval& x : unbounded) {
      check("width_enclosure() of an unbounded interval: [MAX, +oo]", x.width_enclosure().set_eq(interval(max, inf)),
            [&] { return hex(x) + ": " + hex(x.width_enclosure()); });
    }

    // Widened by a radius, and built from a midpoint and a radius
    const interval wider = one_two.inflate(1e-20);
    check("[1, 2].inflate(1e-20): [1 - u/2, 2 + 2u]", wider.left() == previous_double(1.0)
                                                       && wider.right() == next_double(2.0),
          [&] { return hex(wider); });
    check("inflate(0) keeps the interval", one_two.inflate(0.0).set_eq(one_two));
    check("inflate(r) for r < 0 or NaN: the empty set",
          one_two.inflate(-1.0).is_empty() && one_two.inflate(std::numeric_limits<double>::quiet_NaN()).is_empty());
    check("inflate(+oo): the whole line", one_two.inflate(inf).is_entire());
    check("inflate() of the empty set", empty.inflate(1.0).is_empty() && empty.inflate(inf).is_empty());
    check("inflate() past the largest double", interval(max).inflate(max).set_eq(interval(0.0, inf))
                                               && interval(1.0, inf).inflate(1.0).set_eq(interval(0.0, inf)));
    const interval around = interval::midrad(1.0, 1e-20);
    check("interval::midrad(1, 1e-20): [1 - u/2, 1 + u]", around.left() == previous_double(1.0)
                                                          && around.right() == next_double(1.0),
          [&] { return hex(around); });
    check("interval::midrad(m, 0): the point m", interval::midrad(0.1, 0.0).set_eq(interval(0.1)));
    const double nan = std::numeric_limits<double>::quiet_NaN();
    check("interval::midrad() for r < 0, or a NaN r or m: the empty set",
          interval::midrad(1.0, -1.0).is_empty() && interval::midrad(1.0, nan).is_empty()
          && interval::midrad(nan, 1.0).is_empty());
    check("interval::midrad() of an infinite m: the empty set, as interval(m)",
          interval::midrad(inf, 1.0).is_empty() && interval::midrad(-inf, 1.0).is_empty());
    check("interval::midrad(m, +oo): the whole line", interval::midrad(1.0, inf).is_entire());
    check("interval::midrad(MAX, MAX): [0, +oo]", interval::midrad(max, max).set_eq(interval(0.0, inf)));

    // The parts of bisect(), unbounded intervals cut where split() cuts them
    struct { interval x; double ratio; interval first, second; } const cuts[] = {
      { interval::universe(), 0.25, interval(-inf, 0.0), interval(0.0, inf) },
      { interval(0.0, inf), 0.5, interval(0.0, max), interval(max, inf) },
      { interval(-inf, 0.0), 0.75, interval(-inf, -max), interval(-max, 0.0) },
      { interval(max, inf), 0.5, interval(max), interval(max, inf) },
      { interval(0.0, 4.0), 0.25, interval(0.0, 1.0), interval(1.0, 4.0) },
      { interval(2.5), 0.25, interval(2.5), interval(2.5) },
      { interval(1.0, next_double(next_double(1.0))), 1e-300, interval(1.0, next_double(1.0)),
        interval(next_double(1.0), next_double(next_double(1.0))) },
    };
    for (const auto& cut : cuts) {
      const std::pair<interval, interval> parts = cut.x.bisect(cut.ratio);
      check("bisect(ratio) of special intervals", parts.first.set_eq(cut.first) && parts.second.set_eq(cut.second),
            [&] { return hex(cut.x) + " ratio " + hex(cut.ratio) + ": " + hex(parts.first) + " " + hex(parts.second); });
    }
    const std::pair<interval, interval> none = empty.bisect(0.5);
    check("bisect() of the empty set: two empty sets", none.first.is_empty() && none.second.is_empty());
    const double c = interval(-max, max).bisect(0.25).first.right();
    check("bisect(0.25) of [-MAX, MAX]: a finite cut, where l + ratio (r - l) overflowed", c > -max && c < 0.0,
          [&] { return hex(c); });

    // Bisectable: a double strictly between the bounds
    const interval bisectable[] = { interval(1.0, next_double(next_double(1.0))), interval::universe(),
                                    interval(0.0, inf), interval(-inf, 0.0), interval(-max, max) };
    for (const interval& x : bisectable) {
      check("is_bisectable()", x.is_bisectable(), [&] { return hex(x); });
    }
    const interval not_bisectable[] = { empty, interval(1.0), interval(-0.0, 0.0), interval(1.0, next_double(1.0)),
                                        interval(max, inf), interval(-inf, -max) };
    for (const interval& x : not_bisectable) {
      check("not is_bisectable()", !x.is_bisectable(), [&] { return hex(x); });
    }

#if GAOL_EXCEPTIONS_ENABLED
    // A ratio outside (0, 1), or NaN: invalid_action_error
    const double wrong[] = { 0.0, 1.0, -0.5, 1.5, nan, inf };
    for (double ratio : wrong) {
      bool thrown = false;
      try {
        (void)one_two.bisect(ratio);
      } catch (const invalid_action_error&) {
        thrown = true;
      }
      check("bisect(ratio) for a ratio outside (0, 1): invalid_action_error", thrown, [&] { return hex(ratio); });
    }
#endif

    // hull() and intersect() with the empty set
    check("hull() and intersect() with the empty set",
          hull(empty, one_two).set_eq(one_two) && hull(one_two, empty).set_eq(one_two)
          && intersect(empty, one_two).is_empty() && intersect(one_two, interval(3.0, 4.0)).is_empty());
  }

  // Midpoints of intervals whose bounds are multiples of the smallest subnormal
  // double, around 0 and below 2^-1021, where halving a bound is not exact, which
  // the random draws almost never give: mid() subtracted the half of the upper
  // bound rounded upward, and did not enclose the midpoint of [0, 2^-1074]
  void subnormal_bounds()
  {
    const long long below = (1LL << 53) - 1; // 2^-1021 - 2^-1074, in units of 2^-1074
    const long long firsts[] = { -64, below - 128, -below };
    for (long long first : firsts) {
      for (long long i = first; i <= first + 128; ++i) {
        for (long long j = i; j <= first + 128; ++j) {
          midpoints(interval(std::ldexp(static_cast<double>(i), -1074), std::ldexp(static_cast<double>(j), -1074)),
                    " (subnormal bounds)");
        }
      }
    }
    const double tiny = std::numeric_limits<double>::denorm_min(), huge = std::numeric_limits<double>::max();
    const interval mixed[] = { interval(-huge, -tiny), interval(tiny, huge), interval(-tiny, huge),
                               interval(huge / 2, huge), interval(huge, huge) };
    for (const interval& X : mixed) {
      midpoints(X, " (subnormal and huge bounds)");
    }
  }

  // The Hausdorff distance of intervals with infinite bounds, and with zeros of
  // both signs, from the exact distances of their bounds: two equal bounds, the
  // infinite ones too (whose difference is a NaN), are at distance 0, and a bound
  // infinite in one interval only is at distance +oo. GAOL 4 gave +oo as soon
  // as a bound was infinite, but for two entire intervals: hausdorff(x, x) for
  // x = [1, +oo], and hausdorff([1, +oo], [2, +oo]), which is 1, so that a
  // fixed-point loop on a box with an unbounded side never stopped
  void hausdorff_of_infinite_bounds()
  {
    const double bounds[] = { -inf, -2., -1., -0., 0., 1., 2., inf };
    std::vector<interval> xs;
    for (double l : bounds) {
      for (double u : bounds) {
        if (l <= u && l < inf && u > -inf) {
          xs.push_back(interval(l, u));
        }
      }
    }
    for (const interval& a : xs) {
      for (const interval& b : xs) {
        const double h = hausdorff(a, b);
        const double ab[] = { a.left(), a.right() }, bb[] = { b.left(), b.right() };
        bool unbounded = false;
        std::vector<Exact> distances = { exact(0.0) };
        for (int i = 0; i < 2; ++i) {
          if (ab[i] == bb[i] && std::isinf(ab[i])) {
            continue;
          }
          if (std::isinf(ab[i]) || std::isinf(bb[i])) {
            unbounded = true;
          } else {
            const Exact d = exact(dyadic(ab[i]) - dyadic(bb[i]));
            distances.push_back(d);
            distances.push_back(-d);
          }
        }
        const auto describe = [&] { return "a=" + hex(a) + " b=" + hex(b) + ": " + hex(h); };
        check("hausdorff() with infinite bounds: +oo for a bound infinite in one interval only, else the tightest upper bound",
              !std::isnan(h) && (unbounded ? h == inf : is_tightest_upper_bound(h, max(distances))), describe);
        check("hausdorff() with infinite bounds: symmetric", hausdorff(b, a) == h, describe);
      }
    }
    check("hausdorff([1, +oo], [1, +oo]) is 0", hausdorff(interval(1., inf), interval(1., inf)) == 0.0);
    check("hausdorff([1, +oo], [2, +oo]) is 1", hausdorff(interval(1., inf), interval(2., inf)) == 1.0);
    check("hausdorff([-oo, 1], [-oo, 3]) is 2", hausdorff(interval(-inf, 1.), interval(-inf, 3.)) == 2.0);
    check("hausdorff([1, +oo], [1, 2]) is +oo", hausdorff(interval(1., inf), interval(1., 2.)) == inf);
    check("hausdorff([1, +oo], [-oo, 1]) is +oo", hausdorff(interval(1., inf), interval(-inf, 1.)) == inf);
    for (const interval& x : { interval::universe(), interval(-inf, 1.), interval(1., inf), interval(-inf, -0.) }) {
      check("hausdorff(x, x) is 0 for an unbounded x", hausdorff(x, x) == 0.0, [&] { return hex(x); });
    }
  }

  // nb_fp_numbers() where a bound is a zero: -0 is +0, one number. GAOL 4 numbered
  // the doubles by the bits of a and b, sign bit included, and the difference of
  // the bits of -0 and of 1 wrapped around: nb_fp_numbers(-0.0, 1.0) was
  // 13830554455654793217, as nb_fp_numbers(-1.0, 0.0) was, and so was the number
  // of doubles of [1, 2] - 1, whose lower bound is -0
  void nb_fp_numbers_across_zero()
  {
    const double tiny = std::numeric_limits<double>::denorm_min(), huge = std::numeric_limits<double>::max();
    // The multiples of the smallest subnormal are consecutive doubles, the zeros
    // being the one multiple 0: [i, j] holds j - i + 1 doubles. With the zeros
    // of both signs at i = 0 and at j = 0
    const auto multiples = [&](int i) {
      return (i == 0) ? std::vector<double>{ 0.0, -0.0 } : std::vector<double>{ static_cast<double>(i)*tiny };
    };
    for (int i = -6; i <= 6; ++i) {
      for (int j = i; j <= 6; ++j) {
        for (double a : multiples(i)) {
          for (double b : multiples(j)) {
            const unsigned long long n = nb_fp_numbers(a, b);
            check("nb_fp_numbers() of [i, j] times the smallest double", n == static_cast<unsigned long long>(j - i + 1),
                  [&] { return "[" + hex(a) + ", " + hex(b) + "]: " + std::to_string(n); });
          }
        }
      }
    }
    // The doubles from 0 to x are numbered by the bits of x, from 0, and there
    // are these plus the one of 0, or the two counts of that for [-x, x], 0 once
    const struct { double a, b; unsigned long long n; } counts[] = {
      { 0.0, 1.0, 0x3FF0000000000001ull }, { -0.0, 1.0, 0x3FF0000000000001ull },
      { -1.0, 0.0, 0x3FF0000000000001ull }, { -1.0, -0.0, 0x3FF0000000000001ull },
      { -1.0, 1.0, 2*0x3FF0000000000000ull + 1 },
      { 0.0, huge, 0x7FEFFFFFFFFFFFFFull + 1 }, { -0.0, huge, 0x7FEFFFFFFFFFFFFFull + 1 },
      { -huge, 0.0, 0x7FEFFFFFFFFFFFFFull + 1 }, { -huge, -0.0, 0x7FEFFFFFFFFFFFFFull + 1 },
      { -huge, huge, 2*0x7FEFFFFFFFFFFFFFull + 1 },
      { -0.0, -0.0, 1 }, { 0.0, -0.0, 1 }, { -0.0, 0.0, 1 }, { 0.0, 0.0, 1 },
    };
    for (const auto& c : counts) {
      const unsigned long long n = nb_fp_numbers(c.a, c.b);
      check("nb_fp_numbers() across zero", n == c.n,
            [&] { return "[" + hex(c.a) + ", " + hex(c.b) + "]: " + std::to_string(n) + " rather than " + std::to_string(c.n); });
    }
    const interval above = interval(1., 2.) - 1., below = interval(1.) - interval(1., 2.);
    check("nb_fp_numbers() of [1, 2] - 1, whatever the sign of its lower bound zero",
          above.set_eq(interval(0., 1.)) && nb_fp_numbers(above.left(), above.right()) == 0x3FF0000000000001ull,
          [&] { return hex(above); });
    check("nb_fp_numbers() of 1 - [1, 2], whatever the sign of its upper bound zero",
          below.set_eq(interval(-1., 0.)) && nb_fp_numbers(below.left(), below.right()) == 0x3FF0000000000001ull,
          [&] { return hex(below); });
  }

#ifdef GAOL_FLOAT_INTERVALS
  // |x - v|, exactly
  Exact distance(double x, const Exact& v)
  {
    const Exact d = exact(x) + -v;
    return (compare(d, exact(0.0)) < 0) ? -d : d;
  }

  // The midpoint of a gaol::intervalf, rounded to the nearest float, ties to
  // even: it was rounded upward
  void float_midpoint(const intervalf& X, const std::string& in)
  {
    const float finf = std::numeric_limits<float>::infinity();
    const float l = X.left(), r = X.right(), m = X.midpoint();
    const Exact middle = quotient(dyadic(l) + dyadic(r), dyadic(2.0));
    const Exact d = distance(m, middle);
    const int to_below = compare(d, distance(std::nextafter(m, -finf), middle));
    const int to_above = compare(d, distance(std::nextafter(m, finf), middle));
    std::uint32_t bits;
    std::memcpy(&bits, &m, sizeof bits);
    const auto describe = [&] { return "[" + hex(l) + ", " + hex(r) + "]: " + hex(m); };
    check("intervalf::midpoint() within [x]" + in, l <= m && m <= r, describe);
    check("intervalf::midpoint() the float nearest the exact midpoint, ties to even" + in,
          to_below <= 0 && to_above <= 0 && ((to_below < 0 && to_above < 0) || (bits & 1u) == 0), describe);
  }

  template<class Draw>
  void float_midpoints(const std::string& range, Draw draw)
  {
    for (int i = 0; i < nb_random_values; ++i) {
      const float a = draw(), b = draw();
      float_midpoint(intervalf(std::min(a, b), std::max(a, b)), " (" + range + ")");
    }
  }

  void float_midpoints()
  {
    const float finf = std::numeric_limits<float>::infinity(), fmax = std::numeric_limits<float>::max();
    const float one = 1.0f, after_one = std::nextafter(one, finf), tiny = std::numeric_limits<float>::denorm_min();
    for (const intervalf& X : { intervalf(one, after_one), intervalf(after_one, std::nextafter(after_one, finf)),
                                intervalf(0.0f, tiny), intervalf(tiny, 2.0f*tiny), intervalf(fmax / 2.0f, fmax),
                                intervalf(-fmax, fmax / 4.0f) }) {
      float_midpoint(X, " (ties, subnormal and huge bounds)");
    }
    check("intervalf::midpoint() of unbounded intervals",
          intervalf(-finf, finf).midpoint() == 0.0f && intervalf(-finf, one).midpoint() == -fmax
          && intervalf(one, finf).midpoint() == fmax);
    check("intervalf::midpoint() of the empty set", std::isnan(intervalf::emptyset().midpoint()));
  }
#endif // GAOL_FLOAT_INTERVALS

  // The comparisons of IEEE 1788-2015 (Tables 10.3 and 10.4), from the bounds:
  // x <0 y is x < y, or x = y infinite
  bool less0(double x, double y)
  {
    return x < y || (x == y && std::isinf(x));
  }

  bool ieee_precedes(const interval& a, const interval& b)
  {
    return a.is_empty() || b.is_empty() || a.right() <= b.left();
  }

  bool ieee_strictly_precedes(const interval& a, const interval& b)
  {
    return a.is_empty() || b.is_empty() || a.right() < b.left();
  }

  bool ieee_interior(const interval& a, const interval& b)
  {
    return a.is_empty() || (!b.is_empty() && less0(b.left(), a.left()) && less0(a.right(), b.right()));
  }

  bool ieee_subset(const interval& a, const interval& b)
  {
    return a.is_empty() || (!b.is_empty() && b.left() <= a.left() && a.right() <= b.right());
  }

  bool ieee_equal(const interval& a, const interval& b)
  {
    return (a.is_empty() && b.is_empty())
      || (!a.is_empty() && !b.is_empty() && a.left() == b.left() && a.right() == b.right());
  }

  bool ieee_disjoint(const interval& a, const interval& b)
  {
    return a.is_empty() || b.is_empty() || a.right() < b.left() || b.right() < a.left();
  }

  /* == and != are not defined on intervals (GAOL v5): with the certainly
     relations, != was !certainly_eq(), true for [3,4] and [3,4], and IEEE
     1788-2015 has equal and disjoint, set_eq() and set_disjoint() */
  template <typename T, typename = void> struct has_equal : std::false_type {};
  template <typename T>
  struct has_equal<T, decltype(void(std::declval<const T&>() == std::declval<const T&>()))> : std::true_type {};
  template <typename T, typename = void> struct has_not_equal : std::false_type {};
  template <typename T>
  struct has_not_equal<T, decltype(void(std::declval<const T&>() != std::declval<const T&>()))> : std::true_type {};
  static_assert(!has_equal<interval>::value, "== is not defined on intervals");
  static_assert(!has_not_equal<interval>::value, "!= is not defined on intervals");
  static_assert(has_equal<double>::value, "the detection of == works");

  /* interval(double) is explicit (GAOL v5): a double does not convert to an
     interval, and gaol::sin(0.5), the sin of [0.5] while it converted, does
     not compile; gaol::sin(interval(0.5)) is that sin */
  template <typename T, typename = void> struct has_gaol_sin : std::false_type {};
  template <typename T>
  struct has_gaol_sin<T, decltype(void(gaol::sin(std::declval<const T&>())))> : std::true_type {};
  template <typename T, typename = void> struct has_ieee1788_sin : std::false_type {};
  template <typename T>
  struct has_ieee1788_sin<T, decltype(void(gaol_ieee1788::sin(std::declval<const T&>())))> : std::true_type {};
  static_assert(!std::is_convertible<double, interval>::value, "a double does not convert implicitly to an interval");
  static_assert(std::is_constructible<interval, double>::value, "interval(d) is [d, d]");
  static_assert(!has_gaol_sin<double>::value, "gaol::sin(0.5) does not compile");
  static_assert(!has_ieee1788_sin<double>::value, "gaol_ieee1788::sin(0.5) does not compile");
  static_assert(has_gaol_sin<interval>::value && has_ieee1788_sin<interval>::value, "the detection of sin works");

  // GAOL's relations, on the intervals whose bounds are zeros, infinities or
  // small integers, and the empty set: certainly_le() and certainly_leq() are
  // strictPrecedes and precedes, set_strictly_contains() and set_le() interior,
  // set_contains() and set_leq() subset, set_eq() equal, set_disjoint()
  // disjoint. GAOL did not give precedes(a, Empty), interior(Empty, Empty) nor
  // interior(Entire, Entire)
  void comparisons()
  {
    const double bounds[] = { -inf, -2., -1., 0., 1., 2., inf };
    std::vector<interval> xs = { interval::emptyset() };
    for (double l : bounds) {
      for (double u : bounds) {
        if (l <= u && l < inf && u > -inf) {
          xs.push_back(interval(l, u));
        }
      }
    }
    for (const interval& a : xs) {
      for (const interval& b : xs) {
        const auto describe = [&] { return "a=" + hex(a) + " b=" + hex(b); };
        const bool p = ieee_precedes(a, b), sp = ieee_strictly_precedes(a, b), in = ieee_interior(a, b),
          sub = ieee_subset(a, b);
        check("a.certainly_leq(b) and b.certainly_geq(a): precedes(a, b)",
              a.certainly_leq(b) == p && b.certainly_geq(a) == p, describe);
        check("a.certainly_le(b) and b.certainly_ge(a): strictPrecedes(a, b)",
              a.certainly_le(b) == sp && b.certainly_ge(a) == sp, describe);
        check("b.set_strictly_contains(a), a.set_le(b), b.set_ge(a): interior(a, b)",
              b.set_strictly_contains(a) == in && a.set_le(b) == in && b.set_ge(a) == in, describe);
        check("b.set_contains(a), a.set_leq(b), b.set_geq(a): subset(a, b)",
              b.set_contains(a) == sub && a.set_leq(b) == sub && b.set_geq(a) == sub, describe);
        check("a.set_eq(b): equal(a, b)", a.set_eq(b) == ieee_equal(a, b), describe);
        check("a.set_disjoint(b): disjoint(a, b)", a.set_disjoint(b) == ieee_disjoint(a, b), describe);
      }
    }
  }

  // The intersection of the intervals of comparisons(): [max of the left bounds,
  // min of the right bounds], and the empty set, [NaN, NaN], for disjoint
  // intervals, which the operations computing on the bounds keep empty. GAOL
  // gave [1, 2] & [3, 4] = [3, 2], and then [3, 2] + [0, 1] = [3, 3]
  void intersections()
  {
    const double bounds[] = { -inf, -2., -1., 0., 1., 2., inf };
    std::vector<interval> xs = { interval::emptyset(), interval(-inf, -0.), interval(0., inf) };
    for (double l : bounds) {
      for (double u : bounds) {
        if (l <= u && l < inf && u > -inf) {
          xs.push_back(interval(l, u));
        }
      }
    }
    for (const interval& a : xs) {
      for (const interval& b : xs) {
        const interval z = a & b;
        interval w(a);
        w &= b;
        const auto describe = [&] { return "a=" + hex(a) + " b=" + hex(b) + ": " + hex(z); };
        const bool empty = a.is_empty() || b.is_empty() || std::max(a.left(), b.left()) > std::min(a.right(), b.right());
        if (empty) {
          check("a & b, a &= b: the empty set [NaN, NaN] for disjoint intervals",
                std::isnan(z.left()) && std::isnan(z.right()) && std::isnan(w.left()) && std::isnan(w.right()), describe);
          w += interval(10., 20.);
          check("(a & b) + [0, 1], (a &= b) += [10, 20]: empty", (z + interval(0., 1.)).is_empty() && w.is_empty(),
                describe);
        } else {
          check("a & b, a &= b: [max of the left bounds, min of the right bounds]",
                z.left() == std::max(a.left(), b.left()) && z.right() == std::min(a.right(), b.right()) && z.set_eq(w),
                describe);
        }
      }
    }
    // div_rel(K, J, I) intersects I with the quotients: [5, 6] / [1, 2] = [2.5, 6]
    // and [10, 20], disjoint, gave [10, 6]
    const interval d[] = { div_rel(interval(5., 6.), interval(1., 2.), interval(10., 20.)),
                           div_rel(interval(-6., -5.), interval(1., 2.), interval(1., 2.)),
                           div_rel(interval(1., 2.), interval(0., 1.), interval(-6., -5.)) };
    for (const interval& z : d) {
      check("div_rel(K, J, I): the empty set [NaN, NaN] where I holds no quotient",
            std::isnan(z.left()) && std::isnan(z.right()) && (z + interval(0., 1.)).is_empty(), [&] { return hex(z); });
    }
  }

  // Checks that r contains v, and is no more than limit doubles away from it,
  // or no more than slack when slack is given
  void expect_kept(const std::string& name, const interval& r, double v, const std::string& operands, double slack = 0.0,
                   int max_doubles = limit)
  {
    const auto describe = [&] { return operands + ": " + hex(r) + ", which has to contain " + hex(v); };
    const Exact e = exact(v);
    if (check(name + ": keeps the value", !r.is_empty() && r.left() <= v && v <= r.right(), describe)) {
      if (slack > 0.0) {
        RoundingToNearest nearest;
        check(name + ": no more than " + hex(slack) + " from the value", v - r.left() <= slack && r.right() - v <= slack, describe);
      } else {
        check_distance(name, std::max(doubles_below_tightest(r.left(), e, max_doubles), doubles_above_tightest(r.right(), e, max_doubles)),
                       max_doubles, describe);
      }
    }
  }

  // The relational functions: f_rel(J, I) is the hull of the x in I related to
  // some y in J, the relation being y = x^2 for sqrt_rel, z = x*y for div_rel,
  // y = sinh(x) for asinh_rel, y = cos(x) for acos_rel, and so on. Given J
  // enclosing the image of a, and I containing a but no other value with the
  // same image, the result has to contain a, as tightly as J allows.
  void relations()
  {
    Random random;
    for (int i = 0; i < nb_random_values; ++i) {
      const double a = random(-30, 30), b = random(-30, 30);
      const interval A(a), B(b), around = hull(a*0.5, a*1.5);
      const std::string as = "a=" + hex(a);
      expect_kept("sqrt_rel(sqr([a]), [a/2,3a/2])", sqrt_rel(sqr(A), around), a, as);
      expect_kept("div_rel([a]*[b], [b], [a/2,3a/2])", div_rel(A*B, B, around), a, as + " b=" + hex(b));
      for (unsigned int n = 2; n <= 5; ++n) {
        expect_kept("nth_root_rel(pow([a],n), n, [a/2,3a/2]) for n=" + std::to_string(n),
                    nth_root_rel(pow(A, static_cast<int>(n)), n, around), a, as);
      }
      expect_kept("invabs_rel(abs([a]), [a/2,3a/2])", invabs_rel(abs(A), around), a, as);

      // Where sinh, cosh and tanh are finite and not flat
      const double h = random(-30, 8), t = random(-30, 0), c = random.positive(-1, 8);
      const interval H(h), T(t), C(c);
      expect_kept("asinh_rel(sinh([h]), [h/2,3h/2])", asinh_rel(sinh(H), hull(h*0.5, h*1.5)), h, "h=" + hex(h));
      expect_kept("atanh_rel(tanh([t]), [t/2,3t/2])", atanh_rel(tanh(T), hull(t*0.5, t*1.5)), t, "t=" + hex(t));
      expect_kept("acosh_rel(cosh([c]), [c/2,3c/2])", acosh_rel(cosh(C), interval(c*0.5, c*1.5)), c, "c=" + hex(c));

      // Periodic functions, within less than a half period of the argument:
      // within 4 doubles (issue #6: GAOL found the periods with an interval
      // enclosing pi, whose width, 2^-51, the bounds took on, and they were
      // checked to within 2^-49)
      const double u = random.uniform(0.5, 2.6), v = random.uniform(-1.3, 1.3), w = random.uniform(-1.4, 1.4);
      expect_kept("acos_rel(cos([u]), [u-1/4,u+1/4])", acos_rel(cos(interval(u)), interval(u - 0.25, u + 0.25)), u, "u=" + hex(u), 0.0,
                  periodic_limit);
      expect_kept("asin_rel(sin([v]), [v-1/10,v+1/10])", asin_rel(sin(interval(v)), interval(v - 0.1, v + 0.1)), v, "v=" + hex(v), 0.0,
                  periodic_limit);
      expect_kept("atan_rel(tan([w]), [w-1,w+1])", atan_rel(tan(interval(w)), interval(w - 1.0, w + 1.0)), w, "w=" + hex(w), 0.0,
                  periodic_limit);
    }
  }

  // The relational functions of the periodic functions at every magnitude
  // (issue #6): f_rel(f([x]), [x - d, x + d]) has to keep x within a few
  // doubles from 1 up to 2^50, away from the points where the inverse
  // function magnifies the width of f([x]) (|sin x| >= 1/4 for acos_rel,
  // |cos x| >= 1/4 for the others; below 1 that width is several doubles of x
  // already); beyond 2^53, [x - d, x + d] is [x], and f_rel([x]) is [x] when
  // f([x]) meets J, empty otherwise
  void periodic_relations_at_every_magnitude()
  {
    Random random;
    for (int i = 0; i < nb_random_values; ++i) {
      const double x = random(0, 50);
      const interval X(x), around = interval(x - 0.25, x + 0.25), wide = interval(x - 1.0, x + 1.0);
      const std::string xs = "x=" + hex(x);
      RoundingToNearest nearest;
      const double s = std::sin(x), c = std::cos(x);
      if (std::fabs(s) >= 0.25) {
        expect_kept("acos_rel(cos([x]), [x-1/4,x+1/4]) from 1 to 2^50", acos_rel(cos(X), around), x, xs, 0.0, periodic_limit);
      }
      if (std::fabs(c) >= 0.25) {
        expect_kept("asin_rel(sin([x]), [x-1/4,x+1/4]) from 1 to 2^50", asin_rel(sin(X), around), x, xs, 0.0, periodic_limit);
        expect_kept("atan_rel(tan([x]), [x-1,x+1]) from 1 to 2^50", atan_rel(tan(X), wide), x, xs, 0.0, periodic_limit);
      }
    }
    for (int i = 0; i < 1000; ++i) {
      const double x = random(53, 1023);
      const interval X(x);
      const std::string xs = "x=" + hex(x);
      // An image that f([x]) does not meet: shifted by 1/2 within [-1, 1], or
      // beyond 1 for the tangent
      const interval C = cos(X), S = sin(X), T = tan(X);
      const interval notC = (C.right() < 0.5) ? C + 0.5 : C - 0.5, notS = (S.right() < 0.5) ? S + 0.5 : S - 0.5;
      const interval notT = (T.right() < 0.0) ? T + 1.0 : T - 1.0;
      struct Case { const char *name; interval r; bool kept; };
      const Case cases[] = {
        { "acos_rel(cos([x]), [x]) beyond 2^53: [x]", acos_rel(C, X), true },
        { "acos_rel(cos([x]) shifted, [x]) beyond 2^53: empty", acos_rel(notC, X), false },
        { "asin_rel(sin([x]), [x]) beyond 2^53: [x]", asin_rel(S, X), true },
        { "asin_rel(sin([x]) shifted, [x]) beyond 2^53: empty", asin_rel(notS, X), false },
        { "atan_rel(tan([x]), [x]) beyond 2^53: [x]", atan_rel(T, X), true },
        { "atan_rel(tan([x]) shifted, [x]) beyond 2^53: empty", atan_rel(notT, X), false },
      };
      for (const Case& k : cases) {
        check(k.name, k.kept ? (!k.r.is_empty() && k.r.left() == x && k.r.right() == x) : k.r.is_empty(),
              [&] { return xs + ": " + hex(k.r); });
      }
    }
  }

  /*
    less, strictLess, isEntire and isCommonInterval of IEEE 1788-2015
    (Tables 10.3 and 10.4, 10.5.10, 10.6.3), GAOL v5. Each case is written with
    the value the standard gives it: the two empty sets are in both orders, an
    empty set and a nonempty interval in neither; strictLess counts an
    infinite bound as below itself (<' of Table 10.3), so that [-oo, 1] is
    strictly less than [-oo, 2] while [1, 2] is not strictly less than [1, 3].
  */
  void ieee1788_order()
  {
    const interval empty = interval::emptyset();
    struct Case { const char *what; bool got, expected; };
    const Case cases[] = {
      {"less(empty, empty)", empty.less(empty), true},
      {"less(empty, [1, 2])", empty.less(interval(1.0, 2.0)), false},
      {"less([1, 2], empty)", interval(1.0, 2.0).less(empty), false},
      {"less([1, 2], [1, 3])", interval(1.0, 2.0).less(interval(1.0, 3.0)), true},
      {"less([1, 2], [1, 2])", interval(1.0, 2.0).less(interval(1.0, 2.0)), true},
      {"less([1, 4], [2, 3])", interval(1.0, 4.0).less(interval(2.0, 3.0)), false},
      {"less([-oo, 1], [-oo, +oo])", interval(-inf, 1.0).less(interval(-inf, inf)), true},
      {"strictly_less(empty, empty)", empty.strictly_less(empty), true},
      {"strictly_less(empty, [1, 2])", empty.strictly_less(interval(1.0, 2.0)), false},
      {"strictly_less([1, 2], [3, 4])", interval(1.0, 2.0).strictly_less(interval(3.0, 4.0)), true},
      {"strictly_less([1, 2], [1, 3]): equal lower bounds", interval(1.0, 2.0).strictly_less(interval(1.0, 3.0)), false},
      {"strictly_less([-oo, 1], [-oo, 2]): -oo <' -oo", interval(-inf, 1.0).strictly_less(interval(-inf, 2.0)), true},
      {"strictly_less([1, +oo], [2, +oo]): +oo <' +oo", interval(1.0, inf).strictly_less(interval(2.0, inf)), true},
      {"strictly_less(entire, entire)", interval::universe().strictly_less(interval::universe()), true},
      {"is_entire([-oo, +oo])", interval::universe().is_entire(), true},
      {"is_entire([-oo, 1])", interval(-inf, 1.0).is_entire(), false},
      {"is_entire(empty)", empty.is_entire(), false},
      {"is_common_interval([1, 2])", interval(1.0, 2.0).is_common_interval(), true},
      {"is_common_interval([1, 1])", interval(1.0).is_common_interval(), true},
      {"is_common_interval([1, +oo])", interval(1.0, inf).is_common_interval(), false},
      {"is_common_interval(empty): its NaN bounds are no infinities", empty.is_common_interval(), false},
    };
    for (const Case& c : cases) {
      check(std::string("IEEE 1788 order: ") + c.what, c.got == c.expected,
            [&] { return std::string(c.what) + " is " + (c.got ? "true" : "false"); });
    }
  }

  /*
    gaol_ieee1788, the operations of IEEE 1788-2015 under their own names
    (GAOL v5). A wrong translation compiles all the same -- mulRev(b, c, x) is
    div_rel(c, b, x), its arguments in another order -- so each name is checked
    against the standard itself, computed here apart from GAOL: the eight
    comparisons against the bounds of Table 10.3 and the empty cases of Table
    10.4, pow against the pow of Table 9.1 (x > 0, and x = 0 for y > 0), the
    numeric functions against Table 10.2 and 12.12.8.
  */
  void ieee1788_names()
  {
    namespace std1788 = ::gaol_ieee1788;
    Random random;

    // <' of Table 10.3: < but for -oo <' -oo and +oo <' +oo
    const auto lt1 = [](double a, double b) { return a < b || (a == b && std::isinf(a)); };
    for (int i = 0; i < 20000; ++i) {
      // bounds among a few values, so that equal bounds, infinite bounds and
      // the empty set all occur
      const double pool[] = {-inf, -2.0, -1.0, 0.0, 1.0, 2.0, inf};
      const auto pick = [&] { return pool[random.integer(0, 6)]; };
      const auto make = [&] {
        if (random.integer(0, 7) == 0) return interval::emptyset();
        double l = pick(), u = pick();
        if (l > u) std::swap(l, u);
        return interval(l, u);   // the empty set too, for [+oo, +oo] or [-oo, -oo]
      };
      const interval a = make(), b = make();
      const bool ea = a.is_empty(), eb = b.is_empty();
      bool ref[8];
      if (ea || eb) {
        // Table 10.4: a = 0, b != 0 | a != 0, b = 0 | both empty
        const int col = (ea && eb) ? 2 : (ea ? 0 : 1);
        const bool table[8][3] = {
          {false, false, true},  // equal
          {true,  false, true},  // subset
          {false, false, true},  // less
          {true,  true,  true},  // precedes
          {true,  false, true},  // interior
          {false, false, true},  // strictLess
          {true,  true,  true},  // strictPrecedes
          {true,  true,  true},  // disjoint
        };
        for (int k = 0; k < 8; ++k) ref[k] = table[k][col];
      } else {
        const double al = a.left(), au = a.right(), bl = b.left(), bu = b.right();
        ref[0] = al == bl && au == bu;
        ref[1] = bl <= al && au <= bu;
        ref[2] = al <= bl && au <= bu;
        ref[3] = au <= bl;
        ref[4] = lt1(bl, al) && lt1(au, bu);
        ref[5] = lt1(al, bl) && lt1(au, bu);
        ref[6] = lt1(au, bl);
        ref[7] = au < bl || bu < al;
      }
      const bool got[8] = {std1788::equal(a, b), std1788::subset(a, b), std1788::less(a, b),
                           std1788::precedes(a, b), std1788::interior(a, b), std1788::strictLess(a, b),
                           std1788::strictPrecedes(a, b), std1788::disjoint(a, b)};
      const char *names[8] = {"equal", "subset", "less", "precedes", "interior", "strictLess",
                              "strictPrecedes", "disjoint"};
      for (int k = 0; k < 8; ++k) {
        check(std::string("gaol_ieee1788::") + names[k] + ": Tables 10.3 and 10.4", got[k] == ref[k],
              [&] { return std::string(names[k]) + "(" + hex(a) + ", " + hex(b) + ") is "
                         + (got[k] ? "true" : "false"); });
      }
    }

    // pow of Table 9.1: the cases GAOL's own pow gives otherwise
    struct PowCase { const char *what; interval x, y, expected; bool empty; };
    const PowCase pows[] = {
      {"pow([-2, 3], [3]) = [0, 27]", interval(-2.0, 3.0), interval(3.0), interval(0.0, 27.0), false},
      {"pow([-2], [2]) = empty: x < 0", interval(-2.0), interval(2.0), interval(), true},
      {"pow([-1], [2^31 - 1]) = empty", interval(-1.0), interval(2147483647.0), interval(), true},
      {"pow([-2, 0], [-1]) = empty: 0^-1 has no value", interval(-2.0, 0.0), interval(-1.0), interval(), true},
      {"pow([0], [0]) = empty: 0^0 has no value", interval(0.0), interval(0.0), interval(), true},
      {"pow([0], [1, 2]) = [0]", interval(0.0), interval(1.0, 2.0), interval(0.0), false},
      {"pow([0, 4], [0]) = [1]: x^0 = 1 for x > 0", interval(0.0, 4.0), interval(0.0), interval(1.0), false},
      // An integer exponent beyond the ints, which pown cannot take: [-oo, +oo]
      // was the result, negative values included
      {"pow([2, 3], [1e10]) = [DBL_MAX, +oo]", interval(2.0, 3.0), interval(1e10),
       interval(std::numeric_limits<double>::max(), inf), false},
      {"pow([0.5, 0.9], [1e10]) = [0, 2^-1074]", interval(0.5, 0.9), interval(1e10),
       interval(0.0, std::numeric_limits<double>::denorm_min()), false},
      {"pow([1], [-1e12]) = [1]", interval(1.0), interval(-1e12), interval(1.0), false},
      {"pow([-1, 1], [2^31 + 1]) = [0, 1]", interval(-1.0, 1.0), interval(2147483649.0), interval(0.0, 1.0), false},
      // 0^n for an odd n < 0: +oo, the limit at 0 of x^n for x > 0
      {"pow([-1, 0.5], [-2^31 - 1]) = [DBL_MAX, +oo]", interval(-1.0, 0.5), interval(-2147483649.0),
       interval(std::numeric_limits<double>::max(), inf), false},
    };
    for (const PowCase& c : pows) {
      const interval got = std1788::pow(c.x, c.y);
      check(std::string("gaol_ieee1788::pow: ") + c.what,
            c.empty ? got.is_empty() : (!got.is_empty() && got.set_eq(c.expected)),
            [&] { return hex(got); });
    }
    // pow([4], [0.5]) is [2]: 4^0.5 is a double, which GAOL's pow tells, where
    // it gave the double below as its lower bound
    {
      const interval got = std1788::pow(interval(4.0), interval(0.5));
      check("gaol_ieee1788::pow([4], [0.5]) is [2]",
            got.left() == 2.0 && got.right() == 2.0, [&] { return hex(got); });
    }
    // and GAOL's pow wherever x > 0, whatever y
    for (int i = 0; i < 2000; ++i) {
      const interval x = hull(random.uniform(0.1, 4.0), random.uniform(0.1, 4.0));
      const interval y = hull(random.uniform(-3.0, 3.0), random.uniform(-3.0, 3.0));
      check("gaol_ieee1788::pow: GAOL's pow for x > 0", std1788::pow(x, y).set_eq(gaol::pow(x, y)),
            [&] { return hex(x) + " " + hex(y); });
    }

    // numeric functions: Table 10.2 and 12.12.8
    const interval empty = interval::emptyset();
    check("gaol_ieee1788::inf(empty) = +oo", std1788::inf(empty) == inf, [] { return std::string(); });
    check("gaol_ieee1788::sup(empty) = -oo", std1788::sup(empty) == -inf, [] { return std::string(); });
    check("gaol_ieee1788::inf([0, 1]) = -0", std1788::inf(interval(0.0, 1.0)) == 0.0
          && std::signbit(std1788::inf(interval(0.0, 1.0))), [] { return std::string(); });
    check("gaol_ieee1788::sup([-1, 0]) = +0", std1788::sup(interval(-1.0, 0.0)) == 0.0
          && !std::signbit(std1788::sup(interval(-1.0, 0.0))), [] { return std::string(); });
    check("gaol_ieee1788::mid(empty), wid, rad, mag, mig are NaN",
          std::isnan(std1788::mid(empty)) && std::isnan(std1788::wid(empty)) && std::isnan(std1788::rad(empty))
          && std::isnan(std1788::mag(empty)) && std::isnan(std1788::mig(empty)), [] { return std::string(); });
    check("gaol_ieee1788::mid(entire) = 0", std1788::mid(interval::universe()) == 0.0, [] { return std::string(); });
    check("gaol_ieee1788::isMember(+oo, [1, +oo]) is false: m has to be finite",
          !std1788::isMember(inf, interval(1.0, inf)) && std1788::isMember(2.0, interval(1.0, inf)),
          [] { return std::string(); });

    // reverse functions, their arguments in the order of the standard
    check("gaol_ieee1788::mulRev([2], [4, 6]) = [2, 3]: div_rel(c, b, x)",
          std1788::mulRev(interval(2.0), interval(4.0, 6.0)).set_eq(interval(2.0, 3.0)), [] { return std::string(); });
    check("gaol_ieee1788::sqrRev([1, 4], [0, 1.5]) = [1, 1.5]",
          std1788::sqrRev(interval(1.0, 4.0), interval(0.0, 1.5)).set_eq(interval(1.0, 1.5)),
          [] { return std::string(); });
    check("gaol_ieee1788::sinhRev([-oo, 0], [-1, 1]) = [-1, 0], tanhRev([0, 1], [-2, 2]) = [0, 2]",
          std1788::sinhRev(interval(-inf, 0.0), interval(-1.0, 1.0)).set_eq(interval(-1.0, 0.0))
          && std1788::tanhRev(interval(0.0, 1.0), interval(-2.0, 2.0)).set_eq(interval(0.0, 2.0))
          && std1788::sinhRev(interval(0.0)).set_eq(interval(0.0)) && std1788::tanhRev(interval(0.0)).set_eq(interval(0.0)),
          [] { return std::string(); });
    check("gaol_ieee1788::pownRev([8], [0, +oo], 3) = [2]",
          std1788::pownRev(interval(8.0), interval(0.0, inf), 3).set_eq(interval(2.0)), [] { return std::string(); });
    bool threw = false;
    try { (void)std1788::pownRev(interval(8.0), 0); } catch (const std::invalid_argument&) { threw = true; }
    check("gaol_ieee1788::pownRev with p <= 0 throws, GAOL not providing it", threw, [] { return std::string(); });

    // the forward names reach the functions they name
    const interval x(0.3, 0.7);
    check("gaol_ieee1788 forward names", std1788::sinPi(x).set_eq(sinpi(x)) && std1788::rootn(x, -3).set_eq(nth_root(x, -3))
          && std1788::roundTiesToEven(interval(2.5)).set_eq(interval(2.0)) && std1788::recip(interval(4.0)).set_eq(interval(0.25))
          && std1788::cancelMinus(interval(1.0, 5.0), interval(0.0, 2.0)).set_eq(interval(1.0, 3.0))
          && std1788::convexHull(interval(1.0), interval(3.0)).set_eq(interval(1.0, 3.0)),
          [] { return std::string(); });
    const interval y(-2.0, 0.5);
    check("gaol_ieee1788 names of Table 10.5 whose GAOL names differ, and argument orders",
          std1788::logp1(x).set_eq(log1p(x)) && std1788::log2p1(x).set_eq(log2p1(x))
          && std1788::log10p1(x).set_eq(log10p1(x)) && std1788::rSqrt(x).set_eq(rsqrt(x))
          && std1788::asinPi(x).set_eq(asinpi(x)) && std1788::hypot(x, y).set_eq(hypot(x, y))
          && std1788::atan2Pi(y, x).set_eq(atan2pi(y, x)) && std1788::atan2Pi(interval(1.0), interval(-1.0)).set_eq(interval(0.75)),
          [] { return std::string(); });

    // text: an empty set for what is no literal, and the exact form read back
    check("gaol_ieee1788::textToInterval of no literal: empty", std1788::textToInterval("[1, 2").is_empty(),
          [] { return std::string(); });
    const interval_format::format_t saved = interval::format();
    const interval third(1.0 / 3.0, 2.0 / 3.0);
    const std::string exact = std1788::intervalToExact(third);
    check("gaol_ieee1788::exactToInterval(intervalToExact(x)) = x, bit for bit",
          std1788::exactToInterval(exact).set_eq(third), [&] { return exact; });
    check("gaol_ieee1788::intervalToExact sets the format back", interval::format() == saved, [] { return std::string(); });
  }

  // ---------------------------------------------------------------------------
  // max_rel, min_rel, sign_rel and floor_rel (GAOL v5, point P.25 of TODO.md)
  // ---------------------------------------------------------------------------

  /*
    The reverse functions of max, min, sign and floor: max_rel(Z, Y, X) and
    min_rel(Z, Y, X) are the hull of the x of X such that max(x, y),
    respectively min(x, y), is in Z for some y of Y; sign_rel(Z, X) and
    floor_rel(Z, X) the hull of the x of X whose sign, respectively floor, is
    in Z. Each has to be the tightest enclosure of its set, which is checked
    apart from the reasoning of gaol/gaol_interval.cpp, from the definition of
    the set:
     - on every interval whose bounds are among -oo, -2, -1.5, ..., 2, +oo, -0
       and +0, and the empty set (max_rel and min_rel: among -oo, -1, -0.5,
       -0, +0, 0.5, 1 and +oo, every Z, Y and X): the set is then a union of
       intervals whose ends are multiples of 1/2 within [-3, 3] (floor(2) + 1
       is 3), so that its hull is read from the definition at the multiples of
       1/4 from -4 to 4, a y being looked for among them too;
     - on cases written out: disjoint, touching and nested intervals,
       infinite bounds, -0 and +0, subnormal bounds, a Z holding no sign or no
       integer, and floor_rel beyond 2^53, where every double is an integer
       and the open end floor(sup Z) + 1 of its set is no double;
     - floor_rel at every magnitude up to DBL_MAX, its upper bound against
       floor(sup Z) + 1 computed exactly;
     - on random intervals of every magnitude, by sampling: every double of X
       that the definition puts in the set, the bounds of X, Y and Z, 0, -1,
       1, ceil(inf Z), floor(sup Z) and floor(sup Z) + 1 and the doubles on
       either side of each included, has to be in the result, which has to be
       within X, and its finite bounds in the set, or the limit of the doubles
       of the set where the set is open (sign_rel at 0, floor_rel at
       floor(sup Z) + 1).
  */

  // Whether the real number t, finite, is in X
  bool member(double t, const interval& X)
  {
    return std::isfinite(t) && !X.is_empty() && X.left() <= t && t <= X.right();
  }

  // The sign of t, -0 and +0 included, as sign() defines it
  double sign_of_real(double t)
  {
    return (t < 0.0) ? -1.0 : ((t > 0.0) ? 1.0 : 0.0);
  }

  bool same_set(const interval& a, const interval& b)
  {
    if (a.is_empty() || b.is_empty()) {
      return a.is_empty() && b.is_empty();
    }
    return a.left() == b.left() && a.right() == b.right();
  }

  // The empty set and the intervals [l, u] of the bounds given, l <= u: [-0, 0]
  // and [0, -0] both, and [-oo, -oo] and [+oo, +oo], which are empty
  std::vector<interval> intervals_of(const std::vector<double>& bounds)
  {
    std::vector<interval> xs(1, interval::emptyset());
    for (double l : bounds) {
      for (double u : bounds) {
        if (l <= u) {
          xs.push_back(interval(l, u));
        }
      }
    }
    return xs;
  }

  // The multiples of 1/4 from -4 to 4
  std::vector<double> quarters()
  {
    std::vector<double> ts;
    for (int k = -16; k <= 16; ++k) {
      ts.push_back(k/4.0);
    }
    return ts;
  }

  /*
    The hull of a set S, given by in_S(t) at the multiples t of 1/4 from -4 to
    4, S being a union of intervals whose ends are multiples of 1/2 within
    [-3, 3]: a multiple of 1/2 is in S or not, the reals between two of them
    are all in S or none, which the multiple of 1/4 between them tells, and so
    are the reals below -3 and those above 3, which -4 and 4 tell
  */
  template<class In>
  interval grid_hull(const In& in_S)
  {
    double lo = inf, hi = -inf;
    for (int k = -16; k <= 16; ++k) {
      const double t = k/4.0;
      if (!in_S(t)) {
        continue;
      }
      double l = (k % 2 == 0) ? t : t - 0.25, u = (k % 2 == 0) ? t : t + 0.25;
      if (k == -16) {
        l = -inf;
      }
      if (k == 16) {
        u = inf;
      }
      lo = std::min(lo, l);
      hi = std::max(hi, u);
    }
    return (lo <= hi) ? interval(lo, hi) : interval::emptyset();
  }

  void max_min_sign_floor_rel_on_a_grid()
  {
    const std::vector<double> ts = quarters();
    const std::vector<interval> unary = intervals_of({ -inf, -2.0, -1.5, -1.0, -0.5, -0.0, 0.0, 0.5, 1.0, 1.5, 2.0, inf });
    for (const interval& Z : unary) {
      for (const interval& X : unary) {
        const auto describe = [&](const interval& r, const interval& h) {
          return [&, r, h] { return "Z=" + hex(Z) + " X=" + hex(X) + ": " + hex(r) + " rather than " + hex(h); };
        };
        const interval s = sign_rel(Z, X), hs = grid_hull([&](double t) { return member(t, X) && member(sign_of_real(t), Z); });
        check("sign_rel(Z, X): the hull of the x of X whose sign is in Z, on every Z and X of half-integer bounds",
              same_set(s, hs), describe(s, hs));
        const interval f = floor_rel(Z, X), hf = grid_hull([&](double t) { return member(t, X) && member(std::floor(t), Z); });
        check("floor_rel(Z, X): the hull of the x of X whose floor is in Z, on every Z and X of half-integer bounds",
              same_set(f, hf), describe(f, hf));
      }
    }
    const std::vector<interval> binary = intervals_of({ -inf, -1.0, -0.5, -0.0, 0.0, 0.5, 1.0, inf });
    for (const interval& Y : binary) {
      std::vector<double> ys;
      for (double t : ts) {
        if (member(t, Y)) {
          ys.push_back(t);
        }
      }
      for (const interval& Z : binary) {
        for (const interval& X : binary) {
          const auto with = [&](double t, bool maximum) -> bool {
            if (!member(t, X)) {
              return false;
            }
            for (double y : ys) {
              if (member(maximum ? std::max(t, y) : std::min(t, y), Z)) {
                return true;
              }
            }
            return false;
          };
          const interval m = max_rel(Z, Y, X), hm = grid_hull([&](double t) { return with(t, true); });
          const interval n = min_rel(Z, Y, X), hn = grid_hull([&](double t) { return with(t, false); });
          const auto describe = [&](const interval& r, const interval& h) {
            return [&, r, h] {
              return "Z=" + hex(Z) + " Y=" + hex(Y) + " X=" + hex(X) + ": " + hex(r) + " rather than " + hex(h);
            };
          };
          check("max_rel(Z, Y, X): the hull of the x of X with max(x, y) in Z for a y of Y, "
                "on every Z, Y and X of half-integer bounds", same_set(m, hm), describe(m, hm));
          check("min_rel(Z, Y, X): the hull of the x of X with min(x, y) in Z for a y of Y, "
                "on every Z, Y and X of half-integer bounds", same_set(n, hn), describe(n, hn));
        }
      }
    }
  }

  void max_min_sign_floor_rel_cases()
  {
    const interval empty = interval::emptyset(), entire = interval::universe();
    // Made at run time (see the GCC releases of doc/using.md)
    const double tiny = next_double(0.0), two_52 = std::ldexp(1.0, 52), two_53 = std::ldexp(1.0, 53);
    const double big = (std::numeric_limits<double>::max)();
    struct Case { std::string what; interval got, expected; };
    const Case cases[] = {
      // max_rel: Y above Z, meeting it (touching, nested) and below it
      { "max_rel([1, 2], [3, 4], [-5, 5]): Y above Z, empty",
        max_rel(interval(1.0, 2.0), interval(3.0, 4.0), interval(-5.0, 5.0)), empty },
      { "max_rel([1, 2], [0, 1], [-5, 5]): Y touching Z",
        max_rel(interval(1.0, 2.0), interval(0.0, 1.0), interval(-5.0, 5.0)), interval(-5.0, 2.0) },
      { "max_rel([0, 3], [1, 2], [-5, 5]): Y within Z",
        max_rel(interval(0.0, 3.0), interval(1.0, 2.0), interval(-5.0, 5.0)), interval(-5.0, 3.0) },
      { "max_rel([1, 2], [-1, 0], [-5, 5]): Y below Z, X & Z",
        max_rel(interval(1.0, 2.0), interval(-1.0, 0.0), interval(-5.0, 5.0)), interval(1.0, 2.0) },
      { "max_rel([1, 2], [-1, 0], [2, 4]): X touching Z",
        max_rel(interval(1.0, 2.0), interval(-1.0, 0.0), interval(2.0, 4.0)), interval(2.0) },
      { "max_rel([1, 2], [0, 1], [3, 4]): X above Z, empty",
        max_rel(interval(1.0, 2.0), interval(0.0, 1.0), interval(3.0, 4.0)), empty },
      { "max_rel([1, +oo], [-oo, 0], entire)",
        max_rel(interval(1.0, inf), interval(-inf, 0.0), entire), interval(1.0, inf) },
      { "max_rel([-oo, 0], [-1, 5], [-3, 3])",
        max_rel(interval(-inf, 0.0), interval(-1.0, 5.0), interval(-3.0, 3.0)), interval(-3.0, 0.0) },
      { "max_rel(entire, entire, [-3, 3])",
        max_rel(entire, entire, interval(-3.0, 3.0)), interval(-3.0, 3.0) },
      { "max_rel([-0], [+0], [-1, 1]): -0 and +0 are one number",
        max_rel(interval(-0.0), interval(0.0), interval(-1.0, 1.0)), interval(-1.0, 0.0) },
      { "max_rel([-0], [-1, -0.5], [-1, 1])",
        max_rel(interval(-0.0), interval(-1.0, -0.5), interval(-1.0, 1.0)), interval(0.0) },
      { "max_rel([0], [tiny, 1], [-1, 1]): Y above Z by one subnormal",
        max_rel(interval(0.0), interval(tiny, 1.0), interval(-1.0, 1.0)), empty },
      { "max_rel(empty, Y, X)",
        max_rel(empty, entire, entire), empty },
      { "max_rel(Z, empty, X)",
        max_rel(entire, empty, entire), empty },
      { "max_rel(Z, Y, empty)",
        max_rel(entire, entire, empty), empty },
      // min_rel
      { "min_rel([1, 2], [-1, 0], [-5, 5]): Y below Z, empty",
        min_rel(interval(1.0, 2.0), interval(-1.0, 0.0), interval(-5.0, 5.0)), empty },
      { "min_rel([1, 2], [2, 3], [-5, 5]): Y touching Z",
        min_rel(interval(1.0, 2.0), interval(2.0, 3.0), interval(-5.0, 5.0)), interval(1.0, 5.0) },
      { "min_rel([1, 2], [3, 4], [-5, 5]): Y above Z, X & Z",
        min_rel(interval(1.0, 2.0), interval(3.0, 4.0), interval(-5.0, 5.0)), interval(1.0, 2.0) },
      { "min_rel([-oo, 0], [1, 2], entire)",
        min_rel(interval(-inf, 0.0), interval(1.0, 2.0), entire), interval(-inf, 0.0) },
      { "min_rel([-0], [+0], [-1, 1])",
        min_rel(interval(-0.0), interval(0.0), interval(-1.0, 1.0)), interval(0.0, 1.0) },
      { "min_rel(Z, Y, empty)",
        min_rel(entire, entire, empty), empty },
      { "min_rel(empty, Y, X)",
        min_rel(empty, entire, entire), empty },
      { "min_rel(Z, empty, X)",
        min_rel(entire, empty, entire), empty },
      // sign_rel
      { "sign_rel([-1], [-2, 3]): up to 0, the limit of the negative x",
        sign_rel(interval(-1.0), interval(-2.0, 3.0)), interval(-2.0, 0.0) },
      { "sign_rel([-1], [0, 3]): sign(0) = 0, empty",
        sign_rel(interval(-1.0), interval(0.0, 3.0)), empty },
      { "sign_rel([0], [-2, 3])",
        sign_rel(interval(0.0), interval(-2.0, 3.0)), interval(0.0) },
      { "sign_rel([0, 1], [-2, 3])",
        sign_rel(interval(0.0, 1.0), interval(-2.0, 3.0)), interval(0.0, 3.0) },
      { "sign_rel([-1, 1], [-2, 3])",
        sign_rel(interval(-1.0, 1.0), interval(-2.0, 3.0)), interval(-2.0, 3.0) },
      { "sign_rel([0.5, 0.7], [-2, 3]): no sign in Z, empty",
        sign_rel(interval(0.5, 0.7), interval(-2.0, 3.0)), empty },
      { "sign_rel([-5, -2], entire): no sign in Z, empty",
        sign_rel(interval(-5.0, -2.0), entire), empty },
      { "sign_rel([1], [2, 3])",
        sign_rel(interval(1.0), interval(2.0, 3.0)), interval(2.0, 3.0) },
      { "sign_rel([-1], [2, 3]): empty",
        sign_rel(interval(-1.0), interval(2.0, 3.0)), empty },
      { "sign_rel([1], [-0, +0]): empty",
        sign_rel(interval(1.0), interval(-0.0, 0.0)), empty },
      { "sign_rel([-0], [-0, +0])",
        sign_rel(interval(-0.0), interval(-0.0, 0.0)), interval(0.0) },
      { "sign_rel([1], [-tiny, tiny])",
        sign_rel(interval(1.0), interval(-tiny, tiny)), interval(0.0, tiny) },
      { "sign_rel([-1], [-tiny, tiny])",
        sign_rel(interval(-1.0), interval(-tiny, tiny)), interval(-tiny, 0.0) },
      { "sign_rel([1], [tiny, 1])",
        sign_rel(interval(1.0), interval(tiny, 1.0)), interval(tiny, 1.0) },
      { "sign_rel([-oo, -1], entire)",
        sign_rel(interval(-inf, -1.0), entire), interval(-inf, 0.0) },
      { "sign_rel([1, +oo], entire)",
        sign_rel(interval(1.0, inf), entire), interval(0.0, inf) },
      { "sign_rel(empty, X)",
        sign_rel(empty, entire), empty },
      { "sign_rel(Z, empty)",
        sign_rel(entire, empty), empty },
      // floor_rel
      { "floor_rel([0, 2], [-5, 5]): up to 3, the limit of [0, 3)",
        floor_rel(interval(0.0, 2.0), interval(-5.0, 5.0)), interval(0.0, 3.0) },
      { "floor_rel([0], [1, 2]): X starting at the open end, empty",
        floor_rel(interval(0.0), interval(1.0, 2.0)), empty },
      { "floor_rel([0], [0.5, 2])",
        floor_rel(interval(0.0), interval(0.5, 2.0)), interval(0.5, 1.0) },
      { "floor_rel([3], [0, 3]): X touching",
        floor_rel(interval(3.0), interval(0.0, 3.0)), interval(3.0) },
      { "floor_rel([0.5, 0.7], entire): no integer in Z, empty",
        floor_rel(interval(0.5, 0.7), entire), empty },
      { "floor_rel([-0.5, 0.5], entire)",
        floor_rel(interval(-0.5, 0.5), entire), interval(0.0, 1.0) },
      { "floor_rel([-0], entire)",
        floor_rel(interval(-0.0), entire), interval(0.0, 1.0) },
      { "floor_rel([-1], [-0.5, -0])",
        floor_rel(interval(-1.0), interval(-0.5, -0.0)), interval(-0.5, 0.0) },
      { "floor_rel([-oo, 2.5], entire)",
        floor_rel(interval(-inf, 2.5), entire), interval(-inf, 3.0) },
      { "floor_rel([1.5, +oo], entire)",
        floor_rel(interval(1.5, inf), entire), interval(2.0, inf) },
      { "floor_rel(entire, [-3.5, 7.25])",
        floor_rel(entire, interval(-3.5, 7.25)), interval(-3.5, 7.25) },
      { "floor_rel([2^52 - 0.5], entire): no integer in Z, empty",
        floor_rel(interval(two_52 - 0.5), entire), empty },
      { "floor_rel([2^53 - 1], entire)",
        floor_rel(interval(two_53 - 1.0), entire), interval(two_53 - 1.0, two_53) },
      { "floor_rel([2^53], entire): up to 2^53 + 2, the double above 2^53 + 1",
        floor_rel(interval(two_53), entire), interval(two_53, two_53 + 2.0) },
      { "floor_rel([2^53], [0, 2^53])",
        floor_rel(interval(two_53), interval(0.0, two_53)), interval(two_53) },
      { "floor_rel([2^53], [2^53 + 2, 2^54]): empty",
        floor_rel(interval(two_53), interval(two_53 + 2.0, 2.0*two_53)), empty },
      { "floor_rel([2^53 + 2], [2^53, 2^54])",
        floor_rel(interval(two_53 + 2.0), interval(two_53, 2.0*two_53)), interval(two_53 + 2.0, two_53 + 4.0) },
      { "floor_rel([-2^53 - 2], entire)",
        floor_rel(interval(-two_53 - 2.0), entire), interval(-two_53 - 2.0, -two_53) },
      { "floor_rel([-2^53 - 2], [-2^53, 0]): X starting at the open end, the double above -2^53 - 1, empty",
        floor_rel(interval(-two_53 - 2.0), interval(-two_53, 0.0)), empty },
      { "floor_rel([-2^53 - 2], [-2^53 - 2, -2^53])",
        floor_rel(interval(-two_53 - 2.0), interval(-two_53 - 2.0, -two_53)), interval(-two_53 - 2.0, -two_53) },
      { "floor_rel([2^60, 2^61], entire)",
        floor_rel(interval(128.0*two_53, 256.0*two_53), entire), interval(128.0*two_53, 256.0*two_53 + 512.0) },
      { "floor_rel([DBL_MAX], entire): up to +oo, DBL_MAX + 1 being beyond the doubles",
        floor_rel(interval(big), entire), interval(big, inf) },
      { "floor_rel([-DBL_MAX], entire)",
        floor_rel(interval(-big), entire), interval(-big, -previous_double(big)) },
      { "floor_rel(empty, X)",
        floor_rel(empty, entire), empty },
      { "floor_rel(Z, empty)",
        floor_rel(entire, empty), empty },
    };
    for (const Case& c : cases) {
      check(c.what, same_set(c.got, c.expected), [&] { return hex(c.got) + " rather than " + hex(c.expected); });
    }
  }

  /*
    floor_rel at every magnitude, its upper bound the smallest double at least
    floor(sup Z) + 1, computed exactly, where X goes on beyond it. X is the
    whole line, random, or has its bounds among ceil(inf Z), floor(sup Z), the
    doubles next to them and the two doubles above floor(sup Z), around
    floor(sup Z) + 1, where the bounds of X decide the result
  */
  void floor_rel_at_every_magnitude()
  {
    Random random;
    for (int i = 0; i < 30000; ++i) {
      const int e = random.integer(-2, 1023);
      const double a = random(e, e), b = (random.integer(0, 1) == 0) ? a : random(e, e);
      const interval Z = hull(a, b);
      const double first = std::ceil(Z.left()), last = std::floor(Z.right());
      const double above = next_double(last);
      // Not named near, an empty macro of <windows.h>
      const double next_to_ends[] = { previous_double(first), first, next_double(first), previous_double(last), last,
                                      above, next_double(above) };
      const int kind = random.integer(0, 2);
      const double c = (kind == 2) ? next_to_ends[random.integer(0, 6)] : random(-2, 1023);
      const double d = (kind == 2) ? next_to_ends[random.integer(0, 6)] : random(-2, 1023);
      const interval X = (kind == 0) ? interval::universe() : hull(c, d);
      const interval r = floor_rel(Z, X);
      const Exact end = exact(dyadic(last) + dyadic(1.0));
      const auto describe = [&] { return "Z=" + hex(Z) + " X=" + hex(X) + ": " + hex(r); };
      // X empty where its bounds are +oo, above DBL_MAX
      if (X.is_empty() || first > last || first > X.right() || compare(X.left(), end) >= 0) {
        check("floor_rel(Z, X) at every magnitude: empty where Z holds no integer or X none of [ceil(inf Z), floor(sup Z) + 1)",
              r.is_empty(), describe);
        continue;
      }
      const bool to_end = compare(X.right(), end) >= 0;
      check("floor_rel(Z, X) at every magnitude: [max(inf X, ceil(inf Z)), the double above min(sup X, floor(sup Z) + 1)]",
            !r.is_empty() && r.left() == std::max(X.left(), first)
            && (to_end ? is_tightest_upper_bound(r.right(), end) : r.right() == X.right()), describe);
    }
  }

  // Random intervals of every magnitude, by sampling (see above)
  void max_min_sign_floor_rel_sampled()
  {
    Random random;
    const double big = (std::numeric_limits<double>::max)();
    const auto bound = [&]() -> double {
      switch (random.integer(0, 7)) {
        case 0: { const double s[] = { -inf, inf, 0.0, -0.0, 1.0, -1.0, big, -big }; return s[random.integer(0, 7)]; }
        case 1: {
          // One draw per declarator, in order (see Random)
          const int k = random.integer(-5, 5), h = random.integer(0, 1);
          return k + 0.5*h;
        }
        case 2: return random.any();
        default: return random(-30, 60);
      }
    };
    const auto draw = [&]() -> interval {
      if (random.integer(0, 30) == 0) {
        return interval::emptyset();
      }
      const double a = bound(), b = bound();
      return (a <= b) ? interval(a, b) : interval(b, a);
    };
    for (int i = 0; i < 20000; ++i) {
      const interval Z = draw(), Y = draw(), X = draw();
      // The doubles sampled: the bounds, 0, +-1, the integers next to the
      // bounds of Z and floor(sup Z) + 1, the doubles on either side of each,
      // and random doubles of X
      std::vector<double> ts, ys;
      for (const interval* I : { &Z, &Y, &X }) {
        if (!I->is_empty()) {
          ys.push_back(I->left());
          ys.push_back(I->right());
        }
      }
      std::vector<double> marks(ys);
      marks.push_back(0.0);
      marks.push_back(1.0);
      marks.push_back(-1.0);
      if (!Z.is_empty()) {
        marks.push_back(std::ceil(Z.left()));
        marks.push_back(std::floor(Z.right()));
        marks.push_back(std::floor(Z.right()) + 1.0);
      }
      for (double m : marks) {
        if (std::isfinite(m)) {
          ts.push_back(m);
          ts.push_back(next_double(m));
          ts.push_back(previous_double(m));
        }
      }
      if (!X.is_empty()) {
        ts.push_back(X.midpoint());
        const double width = X.right() - X.left();
        for (int k = 0; k < 8; ++k) {
          const double t = std::isfinite(width) ? X.left() + random.uniform(0.0, 1.0)*width : random.any();
          ts.push_back(member(t, X) ? t : X.midpoint());
        }
      }
      // A y of Y with max(t, y), respectively min(t, y), in Z: the set of them is
      // an interval whose finite ends are bounds of Y and Z, or the whole line
      const auto with = [&](double t, bool maximum) -> bool {
        if (!member(t, X)) {
          return false;
        }
        std::vector<double> candidates(ys);
        candidates.push_back(t);
        for (double y : candidates) {
          if (member(y, Y) && member(maximum ? std::max(t, y) : std::min(t, y), Z)) {
            return true;
          }
        }
        return false;
      };
      const auto in_sign = [&](double t) { return member(t, X) && member(sign_of_real(t), Z); };
      const auto in_floor = [&](double t) { return member(t, X) && member(std::floor(t), Z); };

      const interval m = max_rel(Z, Y, X), n = min_rel(Z, Y, X), s = sign_rel(Z, X), f = floor_rel(Z, X);
      const auto describe = [&](const interval& r, double t) {
        return [&, r, t] { return "Z=" + hex(Z) + " Y=" + hex(Y) + " X=" + hex(X) + ": " + hex(r) + ", t=" + hex(t); };
      };
      for (double t : ts) {
        if (with(t, true)) {
          check("max_rel(Z, Y, X) sampled: keeps every x of the set", member(t, m), describe(m, t));
        }
        if (with(t, false)) {
          check("min_rel(Z, Y, X) sampled: keeps every x of the set", member(t, n), describe(n, t));
        }
        if (in_sign(t)) {
          check("sign_rel(Z, X) sampled: keeps every x of the set", member(t, s), describe(s, t));
        }
        if (in_floor(t)) {
          check("floor_rel(Z, X) sampled: keeps every x of the set", member(t, f), describe(f, t));
        }
      }
      // Within X, and the finite bounds in the set, or the limits of its
      // doubles where it is open
      const auto within = [&](const interval& r) {
        return r.is_empty() || (!X.is_empty() && X.left() <= r.left() && r.right() <= X.right());
      };
      const auto closed = [&](double b, bool keeps) { return !std::isfinite(b) || keeps; };
      check("max_rel(Z, Y, X) sampled: within X, its finite bounds in the set",
            within(m) && (m.is_empty() || (closed(m.left(), with(m.left(), true)) && closed(m.right(), with(m.right(), true)))),
            describe(m, 0.0));
      check("min_rel(Z, Y, X) sampled: within X, its finite bounds in the set",
            within(n) && (n.is_empty() || (closed(n.left(), with(n.left(), false)) && closed(n.right(), with(n.right(), false)))),
            describe(n, 0.0));
      // sign_rel open at 0: the set holds the doubles next to 0, X going on to it
      const bool open_left = s.left() == 0.0 && X.left() <= 0.0 && in_sign(next_double(0.0));
      const bool open_right = s.right() == 0.0 && X.right() >= 0.0 && in_sign(previous_double(0.0));
      check("sign_rel(Z, X) sampled: within X, its finite bounds in the set or 0 where it is open there",
            within(s) && (s.is_empty() || (closed(s.left(), in_sign(s.left()) || open_left)
                                           && closed(s.right(), in_sign(s.right()) || open_right))),
            describe(s, 0.0));
      const auto open_end = [&](double b) {
        const double below = previous_double(b);
        return in_floor(below) && is_tightest_upper_bound(b, exact(dyadic(std::floor(below)) + dyadic(1.0)));
      };
      check("floor_rel(Z, X) sampled: within X, its lower bound in the set, its upper bound too or the double above its open end",
            within(f) && (f.is_empty() || (closed(f.left(), in_floor(f.left()))
                                           && closed(f.right(), in_floor(f.right()) || open_end(f.right())))),
            describe(f, 0.0));
    }
  }

  void max_min_sign_floor_rel()
  {
    max_min_sign_floor_rel_on_a_grid();
    max_min_sign_floor_rel_cases();
    floor_rel_at_every_magnitude();
    max_min_sign_floor_rel_sampled();
  }
  // ---------------------------------------------------------------------------
  // End of max_rel, min_rel, sign_rel and floor_rel
  // ---------------------------------------------------------------------------
}

int main()
{
  gaol::init();
  Random random;
  measures("exponents from -30 to 30", [&] { return random(-30, 30); });
  measures("any doubles", [&] { return random.any(); });
  tools_of_point_p();
  subnormal_bounds();
  hausdorff_of_infinite_bounds();
  nb_fp_numbers_across_zero();
  comparisons();
  intersections();
#ifdef GAOL_FLOAT_INTERVALS
  float_midpoints("floats of exponents from -30 to 30", [&] { return static_cast<float>(random(-30, 30)); });
  float_midpoints("any floats", [&] { return static_cast<float>(random(-149, 126)); });
  float_midpoints();
#endif // GAOL_FLOAT_INTERVALS
  relations();
  periodic_relations_at_every_magnitude();
  ieee1788_order();
  ieee1788_names();
  max_min_sign_floor_rel();
  const int status = summary();
  gaol::cleanup();
  return status;
}
