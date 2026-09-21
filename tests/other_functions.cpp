/*-*-C++-*------------------------------------------------------------------
 * gaol -- NOT Just Another Interval Library
 *--------------------------------------------------------------------------
 * Tests of GAOL v5: the other functions on intervals.
 *
 * On random intervals, and for the midpoints on intervals of subnormal bounds,
 * compared exactly with the exact results: midpoints (of gaol::intervalf too),
 * widths, magnitudes and mignitudes, Hausdorff distances, splitting, integer
 * parts, radii; the comparisons of IEEE 1788-2015 (Tables 10.3 and 10.4); and
 * the relational functions (sqrt_rel, div_rel...), which have to keep the
 * values they are given and bound them within a few doubles.
 *
 * Copyright (c) 2026 ENSTA, France
 *
 * Created 2026-09-20 by Jordan NININ
 *--------------------------------------------------------------------------
 * gaol is a software distributed WITHOUT ANY WARRANTY. Read the associated
 * COPYING file for information.
 *--------------------------------------------------------------------------*/

#include "gaol_tests.h"

#include <algorithm>

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

#if GAOL_FLOAT_INTERVALS
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

  // For all x in a and y in b, x = y: both are the same double, or both are
  // empty, as equal() for the empty set
  bool certainly_equal(const interval& a, const interval& b)
  {
    return (a.is_empty() && b.is_empty())
      || (!a.is_empty() && !b.is_empty() && a.left() == a.right() && b.left() == b.right() && a.left() == b.left());
  }

  // GAOL's relations, on the intervals whose bounds are zeros, infinities or
  // small integers, and the empty set: certainly_le() and certainly_leq() are
  // strictPrecedes and precedes, set_strictly_contains() and set_le() interior,
  // set_contains() and set_leq() subset, set_eq() equal, set_disjoint()
  // disjoint. GAOL did not give precedes(a, Empty), interior(Empty, Empty) nor
  // interior(Entire, Entire), and [2] was certainly equal to [1, 2]
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
        check("a.certainly_eq(b): the same double, or both empty", a.certainly_eq(b) == certainly_equal(a, b), describe);
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
    const double inf = GAOL_INFINITY;
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
    gaol::ieee1788, the operations of IEEE 1788-2015 under their own names
    (GAOL v5). A wrong translation compiles all the same -- mulRev(b, c, x) is
    div_rel(c, b, x), its arguments in another order -- so each name is checked
    against the standard itself, computed here apart from GAOL: the eight
    comparisons against the bounds of Table 10.3 and the empty cases of Table
    10.4, pow against the pow of Table 9.1 (x > 0, and x = 0 for y > 0), the
    numeric functions against Table 10.2 and 12.12.8.
  */
  void ieee1788_names()
  {
    namespace std1788 = gaol::ieee1788;
    const double inf = GAOL_INFINITY;
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
        ref[6] = au < bl;
        ref[7] = au < bl || bu < al;
      }
      const bool got[8] = {std1788::equal(a, b), std1788::subset(a, b), std1788::less(a, b),
                           std1788::precedes(a, b), std1788::interior(a, b), std1788::strictLess(a, b),
                           std1788::strictPrecedes(a, b), std1788::disjoint(a, b)};
      const char *names[8] = {"equal", "subset", "less", "precedes", "interior", "strictLess",
                              "strictPrecedes", "disjoint"};
      for (int k = 0; k < 8; ++k) {
        check(std::string("ieee1788::") + names[k] + ": Tables 10.3 and 10.4", got[k] == ref[k],
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
    };
    for (const PowCase& c : pows) {
      const interval got = std1788::pow(c.x, c.y);
      check(std::string("ieee1788::pow: ") + c.what,
            c.empty ? got.is_empty() : (!got.is_empty() && got.set_eq(c.expected)),
            [&] { return hex(got); });
    }
    // pow([4], [0.5]) encloses 2 within one double: GAOL's pow does not tell
    // that 4^0.5 is a double, and gives the double below as its lower bound
    {
      const interval got = std1788::pow(interval(4.0), interval(0.5));
      check("ieee1788::pow([4], [0.5]) encloses 2, within one double",
            got.set_contains(2.0) && got.right() == 2.0 && got.left() >= std::nextafter(2.0, 0.0),
            [&] { return hex(got); });
    }
    // and GAOL's pow wherever x > 0, whatever y
    for (int i = 0; i < 2000; ++i) {
      const interval x = hull(random.uniform(0.1, 4.0), random.uniform(0.1, 4.0));
      const interval y = hull(random.uniform(-3.0, 3.0), random.uniform(-3.0, 3.0));
      check("ieee1788::pow: GAOL's pow for x > 0", std1788::pow(x, y).set_eq(gaol::pow(x, y)),
            [&] { return hex(x) + " " + hex(y); });
    }

    // numeric functions: Table 10.2 and 12.12.8
    const interval empty = interval::emptyset();
    check("ieee1788::inf(empty) = +oo", std1788::inf(empty) == inf, [] { return std::string(); });
    check("ieee1788::sup(empty) = -oo", std1788::sup(empty) == -inf, [] { return std::string(); });
    check("ieee1788::inf([0, 1]) = -0", std1788::inf(interval(0.0, 1.0)) == 0.0
          && std::signbit(std1788::inf(interval(0.0, 1.0))), [] { return std::string(); });
    check("ieee1788::sup([-1, 0]) = +0", std1788::sup(interval(-1.0, 0.0)) == 0.0
          && !std::signbit(std1788::sup(interval(-1.0, 0.0))), [] { return std::string(); });
    check("ieee1788::mid(empty), wid, rad, mag, mig are NaN",
          std::isnan(std1788::mid(empty)) && std::isnan(std1788::wid(empty)) && std::isnan(std1788::rad(empty))
          && std::isnan(std1788::mag(empty)) && std::isnan(std1788::mig(empty)), [] { return std::string(); });
    check("ieee1788::mid(entire) = 0", std1788::mid(interval::universe()) == 0.0, [] { return std::string(); });
    check("ieee1788::isMember(+oo, [1, +oo]) is false: m has to be finite",
          !std1788::isMember(inf, interval(1.0, inf)) && std1788::isMember(2.0, interval(1.0, inf)),
          [] { return std::string(); });

    // reverse functions, their arguments in the order of the standard
    check("ieee1788::mulRev([2], [4, 6]) = [2, 3]: div_rel(c, b, x)",
          std1788::mulRev(interval(2.0), interval(4.0, 6.0)).set_eq(interval(2.0, 3.0)), [] { return std::string(); });
    check("ieee1788::sqrRev([1, 4], [0, 1.5]) = [1, 1.5]",
          std1788::sqrRev(interval(1.0, 4.0), interval(0.0, 1.5)).set_eq(interval(1.0, 1.5)),
          [] { return std::string(); });
    check("ieee1788::pownRev([8], [0, +oo], 3) = [2]",
          std1788::pownRev(interval(8.0), interval(0.0, inf), 3).set_eq(interval(2.0)), [] { return std::string(); });
    bool threw = false;
    try { std1788::pownRev(interval(8.0), 0); } catch (const std::invalid_argument&) { threw = true; }
    check("ieee1788::pownRev with p <= 0 throws, GAOL not providing it", threw, [] { return std::string(); });

    // the forward names reach the functions they name
    const interval x(0.3, 0.7);
    check("ieee1788 forward names", std1788::sinPi(x).set_eq(sinpi(x)) && std1788::rootn(x, -3).set_eq(nth_root(x, -3))
          && std1788::roundTiesToEven(interval(2.5)).set_eq(interval(2.0)) && std1788::recip(interval(4.0)).set_eq(interval(0.25))
          && std1788::cancelMinus(interval(1.0, 5.0), interval(0.0, 2.0)).set_eq(interval(1.0, 3.0))
          && std1788::convexHull(interval(1.0), interval(3.0)).set_eq(interval(1.0, 3.0)),
          [] { return std::string(); });

    // text: an empty set for what is no literal, and the exact form read back
    check("ieee1788::textToInterval of no literal: empty", std1788::textToInterval("[1, 2").is_empty(),
          [] { return std::string(); });
    const interval_format::format_t saved = interval::format();
    const interval third(1.0 / 3.0, 2.0 / 3.0);
    const std::string exact = std1788::intervalToExact(third);
    check("ieee1788::exactToInterval(intervalToExact(x)) = x, bit for bit",
          std1788::exactToInterval(exact).set_eq(third), [&] { return exact; });
    check("ieee1788::intervalToExact sets the format back", interval::format() == saved, [] { return std::string(); });
  }
}

int main()
{
  gaol::init();
  Random random;
  measures("exponents from -30 to 30", [&] { return random(-30, 30); });
  measures("any doubles", [&] { return random.any(); });
  subnormal_bounds();
  comparisons();
  intersections();
#if GAOL_FLOAT_INTERVALS
  float_midpoints("floats of exponents from -30 to 30", [&] { return static_cast<float>(random(-30, 30)); });
  float_midpoints("any floats", [&] { return static_cast<float>(random(-149, 126)); });
  float_midpoints();
#endif // GAOL_FLOAT_INTERVALS
  relations();
  periodic_relations_at_every_magnitude();
  ieee1788_order();
  ieee1788_names();
  const int status = summary();
  gaol::cleanup();
  return status;
}
