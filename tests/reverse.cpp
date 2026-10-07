/*-*-C++-*------------------------------------------------------------------
 * gaol -- NOT Just Another Interval Library
 *--------------------------------------------------------------------------
 * Tests of GAOL v5: the relational functions, against the reverse
 * functions of IEEE 1788-2015 (10.5.4, Table 10.1).
 *
 * IEEE 1788 defines fRev(c, x) = hull{ x in x | f(x) is defined and in c } and
 * mulRev(b, c, x) = hull{ x in x | x*b is in c for some b in b }, x being the
 * whole line when absent. GAOL computes them as sqrt_rel(c, x) (sqrRev),
 * invabs_rel(c, x) (absRev), nth_root_rel(c, p, x) (pownRev, for p > 0),
 * asin_rel, acos_rel and atan_rel (sinRev, cosRev, tanRev), acosh_rel
 * (coshRev), and div_rel(c, b, x), c % b and c %= b (mulRev), c % d and
 * c %= d for b = [d].
 *
 * In each case of reverse_values.h (the minimal tests of libieeep1788 and
 * random ones, see reverse_values.py), GAOL's result has to be, as IEEE 1788
 * asks of the reverse functions (12.10.2):
 *  - empty when an argument is;
 *  - valid: enclose the hull, whose tightest enclosure the table gives;
 *  - accurate, which the standard recommends for inf-sup types: be within
 *    nextOut(tightest(nextOut(c), nextOut(x))) (12.10.1), the tightest result
 *    for arguments widened by one double, widened by one double.
 * The bounds of asin_rel, acos_rel and atan_rel are allowed one double
 * further, their reduction modulo pi rounding twice, and the bound of x
 * beyond 2^52, which they keep, as doc/accuracy.md documents.
 * The summary also gives how far from the tightest bounds the results were,
 * where both are nonempty.
 *
 * atan2_rel_y and atan2_rel_x (atan2Rev1 and atan2Rev2, GAOL v5) have their
 * own section below, their own table (atan2_rel_values.h), and checks of
 * soundness on points of the boxes.
 *
 * Copyright (c) 2026 ENSTA, France
 *
 * Created 2026-09-20 by Jordan NININ
 *--------------------------------------------------------------------------
 * gaol is a software distributed WITHOUT ANY WARRANTY. Read the associated
 * COPYING file for information.
 *--------------------------------------------------------------------------*/

#include "gaol_tests.h"
#include "reverse_values.h"
#include "pow_rel_values.h"
#include "atan2_rel_values.h"

#include <algorithm>
#include <cfenv>
#include <cmath>
#include <functional>
#include <string>
#include <vector>

using namespace gaol;
using namespace gaol_tests;

namespace
{
  interval make(const reverse_values::I& a)
  {
    return a.empty ? interval::emptyset() : interval(a.l, a.u);
  }

  // The number of doubles from a up to b, stopping beyond limit
  int doubles_from(double a, double b, int limit)
  {
    int n = 0;
    for (double t = a; t < b && n <= limit; t = next_double(t)) {
      ++n;
    }
    return n;
  }

  double previous_double(double x) { return -next_double(-x); }

  // 2^52, beyond which asin_rel, acos_rel and atan_rel keep the bound of x
  const double two_power_52 = 4503599627370496.0;

  // Checks the result r of the function named name in the case k, periodic
  // telling asin_rel, acos_rel and atan_rel from the others
  template<class Describe>
  void expect(const std::string& name, const interval& r, const reverse_values::Case& k, bool periodic,
              const Describe& describe)
  {
    if (k.c.empty || k.b.empty || k.x.empty) {
      check(name + ": empty for an empty argument", r.is_empty(), describe);
      return;
    }
    const reverse_values::I& t = k.tightest;
    const reverse_values::I& a = k.accurate;
    check(name + ": valid, encloses the hull",
          t.empty || (!r.is_empty() && r.left() <= t.l && t.u <= r.right()), describe);
    // The accurate bounds, one double further for the periodic functions, or
    // the bound of x where they keep it (see the head of this file)
    double low = a.l, high = a.u;
    if (periodic && !a.empty) {
      low = std::fabs(k.x.l) > two_power_52 ? std::min(low, k.x.l) : previous_double(low);
      high = std::fabs(k.x.u) > two_power_52 ? std::max(high, k.x.u) : next_double(high);
    }
    check(name + ": no more than the accurate bounds, nextOut(tightest(nextOut(arguments)))",
          r.is_empty() || (!a.empty && low <= r.left() && r.right() <= high), describe);
    // How far from the tightest bounds, recorded without being checked
    if (!t.empty && !r.is_empty() && r.left() <= t.l && t.u <= r.right()) {
      int& largest = largest_distances()[name];
      largest = std::max(largest, std::max(doubles_from(r.left(), t.l, 1000), doubles_from(t.u, r.right(), 1000)));
    }
  }

  void reverse_functions()
  {
    for (const reverse_values::Case& k : reverse_values::cases) {
      const interval c = make(k.c), b = make(k.b), x = make(k.x);
      const std::string f = k.f;
      const std::string cx = hex(c) + ", " + hex(x);
      struct Call { std::string name, text; interval r; };
      std::vector<Call> calls;
      const auto call = [&](const std::string& name, const std::string& text, const std::function<interval()>& g) {
        calls.push_back({ name, text, evaluate(name, g, [&] { return text; }) });
      };
      if (f == "sqrRev") {
        call("sqrt_rel(c, x)", "sqrt_rel(" + cx + ")", [&] { return sqrt_rel(c, x); });
      } else if (f == "absRev") {
        call("invabs_rel(c, x)", "invabs_rel(" + cx + ")", [&] { return invabs_rel(c, x); });
      } else if (f == "pownRev") {
        const unsigned int p = static_cast<unsigned int>(k.p);
        call("nth_root_rel(c, p, x)", "nth_root_rel(" + hex(c) + ", " + std::to_string(p) + ", " + hex(x) + ")",
             [&] { return nth_root_rel(c, p, x); });
      } else if (f == "sinRev") {
        call("asin_rel(c, x)", "asin_rel(" + cx + ")", [&] { return asin_rel(c, x); });
      } else if (f == "cosRev") {
        call("acos_rel(c, x)", "acos_rel(" + cx + ")", [&] { return acos_rel(c, x); });
      } else if (f == "tanRev") {
        call("atan_rel(c, x)", "atan_rel(" + cx + ")", [&] { return atan_rel(c, x); });
      } else if (f == "coshRev") {
        call("acosh_rel(c, x)", "acosh_rel(" + cx + ")", [&] { return acosh_rel(c, x); });
      } else if (f == "mulRev") {
        call("div_rel(c, b, x)", "div_rel(" + hex(c) + ", " + hex(b) + ", " + hex(x) + ")",
             [&] { return div_rel(c, b, x); });
        if (!k.x_given) {
          // The relational division: c % b is mulRev(b, c)
          const std::string cb = hex(c) + " % " + hex(b);
          call("c % b", cb, [&] { return c % b; });
          call("c %= b", cb, [&] { interval r(c); r %= b; return r; });
          if (!b.is_empty() && b.left() == b.right()) {
            const double d = b.left();
            call("c % d", hex(c) + " % " + hex(d), [&] { return c % d; });
            call("c %= d", hex(c) + " %= " + hex(d), [&] { interval r(c); r %= d; return r; });
          }
        }
      }
      const bool periodic = (f == "sinRev" || f == "cosRev" || f == "tanRev");
      for (const Call& g : calls) {
        expect(g.name, g.r, k, periodic, [&] {
          return g.text + " (" + f + " of IEEE 1788, case of " + k.from + "): " + hex(g.r) + ", tightest "
            + hex(make(k.tightest)) + ", accurate within " + hex(make(k.accurate));
        });
      }
    }
  }
}


//---------------------------------------------------------------------------
// The reverse functions of pow (GAOL v5): pow_rel(c, b, x), powRev1(b, c, x),
// the x of x with an x^y in c for a y of b, and pow_exponent_rel(c, a, x),
// powRev2(a, c, x), the y of x with an a^y in c for an a of a, x^y being the
// pow of IEEE 1788-2015 (Table 9.1), that of gaol_ieee1788::pow.
//
// On the cases of pow_rel_values.h (those of pow_rev.itl of ITF1788, random
// ones, ones whose solutions are doubles, and ones where a bound of x is next
// to an end of the solutions, see pow_rel_values.py), each result has to be
// empty when an argument is, to enclose the tightest enclosure of the hull,
// and to be within the tightest enclosure of the solutions in x widened by one
// double: the tightest result, or that one with a bound of x next to the
// solutions, which pow_rel() keeps where it is not proved to miss them (see
// gaol/gaol_interval.h); the standard's names, with and without x, have to
// give the same bounds. Then, by sampling: the points (x, y) of random boxes
// whose power, the pow of the standard on [x] and [y], is within c, which
// proves x^y in c, have to be in the results, c being made of the powers of
// some of these points, so that the solutions reach its bounds.
//---------------------------------------------------------------------------
namespace
{
  interval make(const pow_rel_values::I& a)
  {
    return a.empty ? interval::emptyset() : interval(a.l, a.u);
  }

  bool same_bounds(const interval& a, const interval& b)
  {
    return (a.is_empty() && b.is_empty())
      || (!a.is_empty() && !b.is_empty() && a.left() == b.left() && a.right() == b.right());
  }

  bool within(const interval& a, const interval& b)
  {
    return a.is_empty() || (!b.is_empty() && b.left() <= a.left() && a.right() <= b.right());
  }

  void pow_reverse_cases()
  {
    long tightest = 0;
    for (const pow_rel_values::Case& k : pow_rel_values::cases) {
      const interval b = make(k.b), c = make(k.c), x = make(k.x), t = make(k.tightest), n = make(k.near);
      const bool base = (k.f == 1);
      const std::string name = base ? "pow_rel(c, b, x)" : "pow_exponent_rel(c, a, x)";
      const std::string text = (base ? "pow_rel(" : "pow_exponent_rel(") + hex(c) + ", " + hex(b) + ", " + hex(x) + ")";
      const interval r = evaluate(name, [&] { return base ? pow_rel(c, b, x) : pow_exponent_rel(c, b, x); },
                                  [&] { return text; });
      const auto describe = [&] {
        return text + " (powRev" + std::to_string(k.f) + " of IEEE 1788, case of " + k.from + "): " + hex(r)
          + ", tightest " + hex(t) + ", within " + hex(n);
      };
      if (k.b.empty || k.c.empty || k.x.empty) {
        check(name + ": empty for an empty argument", r.is_empty(), describe);
      }
      check(name + ": valid, encloses the hull", within(t, r), describe);
      check(name + ": within the tightest result for x one double wider", within(r, n), describe);
      if (same_bounds(r, t)) {
        ++tightest;
      } else if (!t.is_empty() && !r.is_empty()) {
        int& largest = largest_distances()[name];
        largest = std::max(largest, std::max(doubles_from(r.left(), t.left(), 1000), doubles_from(t.right(), r.right(), 1000)));
      }
      // The names of IEEE 1788-2015, x left out where the case does
      const interval s = base ? (k.x_given ? gaol_ieee1788::powRev1(b, c, x) : gaol_ieee1788::powRev1(b, c))
        : (k.x_given ? gaol_ieee1788::powRev2(b, c, x) : gaol_ieee1788::powRev2(b, c));
      check(std::string(base ? "powRev1" : "powRev2") + ": the bounds of " + name, same_bounds(s, r), describe);
    }
    std::printf("pow_rel and pow_exponent_rel: the tightest result in %ld cases of %ld\n", tightest,
                static_cast<long>(sizeof(pow_rel_values::cases)/sizeof(pow_rel_values::cases[0])));
  }

  // A double of [lo, hi], lo <= hi, hi > 0 for a positive lo: the bounds now
  // and then, otherwise spread over the magnitudes where the interval spans
  // several
  double sample(Random& random, double lo, double hi)
  {
    const int pick = random.integer(0, 9);
    if (pick == 0 && lo > -GAOL_INFINITY) {
      return lo;
    }
    if (pick == 1 && hi < GAOL_INFINITY) {
      return hi;
    }
    const double l = std::max(lo, -1e300), h = std::min(hi, 1e300);
    double d;
    if (l >= 0.0 && h > 2.0*l) {
      // From l, or from 2^-80 h for l = 0
      d = std::exp2(random.uniform((l > 0.0) ? std::log2(l) : std::log2(h) - 80.0, std::log2(h)));
    } else if (h <= 0.0 && l < 2.0*h) {
      d = -std::exp2(random.uniform((h < 0.0) ? std::log2(-h) : std::log2(-l) - 80.0, std::log2(-l)));
    } else if (l < 0.0 && h > 0.0 && pick < 6) {
      d = (pick < 4 ? 1.0 : -1.0)*std::exp2(random.uniform(-60.0, std::log2(pick < 4 ? h : -l)));
    } else {
      d = random.uniform(l, h);
    }
    return std::min(std::max(d, l), h);
  }

  void pow_reverse_sampling()
  {
    Random random;
    // Bounds of the boxes: 0, 1 and the doubles around it, the infinities,
    // small integers and halves, extremes
    const double bases[] = { -1.0, -0.0, 0.0, 0x1p-1074, 1e-300, 0.25, 0.5, 0x1.fffffffffffffp-1, 1.0, 0x1.0000000000001p+0,
                             1.5, 2.0, 3.0, 10.0, 1e300, GAOL_INFINITY };
    const double exponents[] = { -GAOL_INFINITY, -50.0, -2.0, -1.0, -0.5, -0.0, 0.0, 0.5, 1.0, 2.0, 3.0, 50.0,
                                 GAOL_INFINITY };
    const int n_bases = static_cast<int>(sizeof(bases)/sizeof(bases[0]));
    const int n_exponents = static_cast<int>(sizeof(exponents)/sizeof(exponents[0]));
    long solutions = 0;
    for (int box = 0; box < 6000; ++box) {
      double a1 = random.integer(0, 2) == 0 ? random.positive(-40, 40) : bases[random.integer(0, n_bases - 1)];
      double a2 = random.integer(0, 2) == 0 ? random.positive(-40, 40) : bases[random.integer(0, n_bases - 1)];
      double e1 = random.integer(0, 2) == 0 ? random(-6, 6) : exponents[random.integer(0, n_exponents - 1)];
      double e2 = random.integer(0, 2) == 0 ? random(-6, 6) : exponents[random.integer(0, n_exponents - 1)];
      if (a2 < a1) {
        std::swap(a1, a2);
      }
      if (e2 < e1) {
        std::swap(e1, e2);
      }
      const interval a(a1, a2), y(e1, e2);
      if (a.is_empty() || y.is_empty() || a.right() < 0.0) {
        continue;
      }
      // Points of the box, and their powers
      std::vector<double> xs, ys;
      std::vector<interval> powers;
      for (int i = 0; i < 24; ++i) {
        const double xi = sample(random, std::max(a.left(), 0.0), a.right());
        const double yi = sample(random, y.left(), y.right());
        xs.push_back(xi);
        ys.push_back(yi);
        powers.push_back(gaol_ieee1788::pow(interval(xi), interval(yi)));
      }
      // c from the powers of two points, or around one, or of the pools
      interval c;
      const int i = random.integer(0, 23), j = random.integer(0, 23), kind = random.integer(0, 3);
      if (kind == 0 && !powers[i].is_empty() && !powers[j].is_empty()) {
        c = interval(std::min(powers[i].left(), powers[j].left()), std::max(powers[i].right(), powers[j].right()));
      } else if (kind == 1 && !powers[i].is_empty()) {
        c = powers[i];
      } else if (kind == 2 && !powers[i].is_empty()) {
        c = interval(powers[i].left(), random.integer(0, 1) ? GAOL_INFINITY : powers[i].right()*2.0);
      } else {
        const double c1 = bases[random.integer(0, n_bases - 1)], c2 = bases[random.integer(0, n_bases - 1)];
        c = interval(std::min(c1, c2), std::max(c1, c2));
      }
      if (c.is_empty()) {
        continue;
      }
      const interval rx = pow_rel(c, y, a), ry = pow_exponent_rel(c, a, y);
      for (int p = 0; p < 24; ++p) {
        if (powers[p].is_empty() || !c.set_contains(powers[p])) {
          continue; // x^y not proved to be in c
        }
        ++solutions;
        const auto describe = [&] {
          return "x = " + hex(xs[p]) + ", y = " + hex(ys[p]) + ", x^y in " + hex(powers[p]) + " within c = " + hex(c)
            + ": pow_rel(c, " + hex(y) + ", " + hex(a) + ") = " + hex(rx) + ", pow_exponent_rel(c, " + hex(a) + ", "
            + hex(y) + ") = " + hex(ry);
        };
        check("pow_rel(c, b, x): every x of x with x^y in c for a y of b is kept", rx.set_contains(xs[p]), describe);
        check("pow_exponent_rel(c, a, x): every y of x with a^y in c for an a of a is kept", ry.set_contains(ys[p]), describe);
      }
    }
    check("pow_rel and pow_exponent_rel: the sampling found solutions", solutions > 20000,
          [&] { return std::to_string(solutions) + " solutions"; });
  }

  // The examples of the documentation (gaol/gaol_interval.h, doc/using.md, the manual)
  void pow_reverse_examples()
  {
    const interval entire = interval::universe();
    check("pow_rel([4, 9], [2], entire) is [2, 3]", same_bounds(pow_rel(interval(4.0, 9.0), interval(2.0), entire), interval(2.0, 3.0)));
    check("pow_rel([2, 3], [1, +oo], [0, 1]) is empty, x^y >= 2 asking x > 1",
          pow_rel(interval(2.0, 3.0), interval(1.0, GAOL_INFINITY), interval(0.0, 1.0)).is_empty());
    check("pow_exponent_rel([8], [2], entire) is [3]", same_bounds(pow_exponent_rel(interval(8.0), interval(2.0), entire), interval(3.0)));
    check("pow_rel([2], [0.5], [0, 10]) is [4], 4^0.5 being 2 exactly",
          same_bounds(pow_rel(interval(2.0), interval(0.5), interval(0.0, 10.0)), interval(4.0)));
    check("pow_rel([0], [1], [-1, 1]) is [0], and pow_rel([0], [-1, 0], [-1, 1]) empty: 0^y has a value for y > 0 only",
          same_bounds(pow_rel(interval(0.0), interval(1.0), interval(-1.0, 1.0)), interval(0.0))
          && pow_rel(interval(0.0), interval(-1.0, 0.0), interval(-1.0, 1.0)).is_empty());
    check("pow_rel([1, 4], [2], [-3, 3]) is [1, 2]: no negative base, where pow(x, 2) of gaol has one",
          same_bounds(pow_rel(interval(1.0, 4.0), interval(2.0), interval(-3.0, 3.0)), interval(1.0, 2.0)));
    // The double below the square root of 2, whose square is more than one
    // double below 2, is proved to miss it; 1 + 2^-51, whose square root is
    // within one double below 1 + 2^-52, is kept, (1 + 2^-52)^2 being
    // 1 + 2^-51 + 2^-104: the tightest result for x one double wider
    const double below_root_2 = 0x1.6a09e667f3bccp+0;
    check("pow_rel([2], [2], [0, the double below the square root of 2]) is empty",
          pow_rel(interval(2.0), interval(2.0), interval(0.0, below_root_2)).is_empty());
    check("pow_rel([2], [2], [0, 2]) is the tightest enclosure of the square root of 2",
          same_bounds(pow_rel(interval(2.0), interval(2.0), interval(0.0, 2.0)), interval(below_root_2, 0x1.6a09e667f3bcdp+0)));
    check("pow_rel([1 + 2^-52], [0.5], [0, 1 + 2^-51]) is [1 + 2^-51]",
          same_bounds(pow_rel(interval(0x1.0000000000001p+0), interval(0.5), interval(0.0, 0x1.0000000000002p+0)),
                      interval(0x1.0000000000002p+0)));
  }
}

//---------------------------------------------------------------------------
// atan2_rel_y and atan2_rel_x: atan2Rev1 and atan2Rev2 (GAOL v5)
//---------------------------------------------------------------------------
/*
  atan2_rel_y(c, x, y) and atan2_rel_x(c, y, x), atan2Rev1(x, c, y) and
  atan2Rev2(y, c, x) of IEEE 1788, are the hulls of the ordinates and of the
  abscissas of the points (x, y) of the box y x x, (0, 0) left out, whose angle
  atan2(y, x), in (-pi, pi], is in c.
   - In each case of atan2_rel_values.h (angles at, next to and around the
     multiples of pi/4, the axes, the half-line y = 0, x < 0, angles outside
     [-pi, pi], boxes in and across the quadrants, with zero, tiny, huge and
     infinite bounds, and random cases: see atan2_rel_values.py), the results
     have to be empty when an argument is, valid (enclose the tightest
     enclosures of the hulls, computed with mpmath) and accurate (within
     nextOut(tightest(nextOut(arguments)))), and the functions of
     gaol_ieee1788 have to give them, with and without their last argument.
   - Soundness, on points: in each case of the table and in random ones, the
     corners of the box, points of its sides and of the axes, random points,
     and the points of the sides next to where the half-lines bounding the
     angles of c cross them, where the bounds of the results are made. Each
     point whose angle is in c has to be in both results. Whether it is, is
     decided exactly from atan2() of the point, whose bounds are the two
     doubles around its angle (the tightest enclosure, CORE-MATH's atan2
     rounded both ways), or the angle itself where it is 0, the only angle of
     a point of doubles that is a double: a bound of c, a double, is never
     strictly between them.
   - The two results are both empty or neither, and the angles of the box
     they make meet c, widened by 4 doubles: a box of (0, 0) alone, which has
     no angle, is never a result, nor is a box away from the cone.
   - No invalid-operation flag for boxes and angles with zero and infinite
     bounds.
*/
namespace
{
  interval make(const atan2_rel_values::I& a)
  {
    return a.empty ? interval::emptyset() : interval(a.l, a.u);
  }

  // The number of doubles from a up to b, stopping beyond limit
  int atan2_doubles_from(double a, double b, int limit)
  {
    int n = 0;
    for (double t = a; t < b && n <= limit; t = next_double(t)) {
      ++n;
    }
    return n;
  }

  // Checks the result r, whose tightest value is t and whose accurate
  // results are within a, and, where both are nonempty, no more than 2
  // doubles from it, as doc/accuracy.md says: the slopes of the bounds of c
  // are rounded once, and their products with the bounds of the box once
  // more (or exactly, at a subnormal angle, see atan2_rel_piece())
  template<class Describe>
  void atan2_expect(const std::string& name, const interval& r, const atan2_rel_values::I& t,
                    const atan2_rel_values::I& a, bool empty_argument, const Describe& describe)
  {
    if (empty_argument) {
      check(name + ": empty for an empty argument", r.is_empty(), describe);
      return;
    }
    check(name + ": valid, encloses the hull",
          t.empty || (!r.is_empty() && r.left() <= t.l && t.u <= r.right()), describe);
    check(name + ": no more than the accurate bounds, nextOut(tightest(nextOut(arguments)))",
          r.is_empty() || (!a.empty && a.l <= r.left() && r.right() <= a.u), describe);
    if (!t.empty && !r.is_empty() && r.left() <= t.l && t.u <= r.right()) {
      check_distance(name, std::max(atan2_doubles_from(r.left(), t.l, 1000), atan2_doubles_from(t.u, r.right(), 1000)),
                     2, describe);
    }
  }

  bool same(const interval& a, const interval& b)
  {
    return (a.is_empty() && b.is_empty()) || a.set_eq(b);
  }

  /*
    Whether the angle of the point (x, y), not (0, 0), is in c: 1 if it is,
    0 if not, -1 where atan2() of the point is neither the angle 0 nor two
    adjacent doubles, which the decision needs
  */
  int angle_in(double y, double x, const interval& c)
  {
    const interval a = atan2(interval(y), interval(x));
    if (a.is_empty() || !(a.left() == a.right() || next_double(a.left()) == a.right())) {
      return -1;
    }
    return (c.left() <= a.left() && a.right() <= c.right()) ? 1 : 0;
  }

  // The coordinates of the points sampled on one axis of the box: its bounds,
  // or doubles of large magnitudes where they are infinite, 0, +-1 and the
  // least subnormals where they are within, and random ones
  std::vector<double> coordinates(const interval& v, Random& random)
  {
    std::vector<double> out;
    const double l = v.left(), u = v.right(), max = std::numeric_limits<double>::max();
    const double candidates[] = { l, u, 0.0, 1.0, -1.0, 0x1p-1074, -0x1p-1074, max, -max, 1e300, -1e300,
                                  0x1p-1022, -0x1p-1022 };
    for (double d : candidates) {
      if (std::isfinite(d) && l <= d && d <= u) {
        out.push_back(d);
      }
    }
    const double lo = std::isfinite(l) ? l : -1e10, hi = std::isfinite(u) ? u : 1e10;
    for (int i = 0; i < 6; ++i) {
      const double d = (lo < hi) ? random.uniform(lo, hi) : lo;
      if (l <= d && d <= u) {
        out.push_back(d);
      }
    }
    return out;
  }

  /*
    Checks that each sampled point of the box y x x whose angle is in c is in
    ry = atan2_rel_y(c, x, y) and rx = atan2_rel_x(c, y, x): the points of
    coordinates() of y and of x, and, for each bound of c and each of these
    coordinates, the doubles next to the point where the half-line of that
    angle crosses the side through it (e tan(theta) for an abscissa e, and
    e / tan(theta) for an ordinate e), computed rounding to nearest
  */
  void sample(const interval& c, const interval& y, const interval& x, Random& random, long& undecided)
  {
    if (c.is_empty() || y.is_empty() || x.is_empty()) {
      return;
    }
    const interval ry = atan2_rel_y(c, x, y), rx = atan2_rel_x(c, y, x);
    // Both empty or neither, and a box of results holding points whose angle
    // is in c, or next to it where the box is within a rounding of the cone
    // and no point of it is in c; a box of (0, 0) alone, which has no angle,
    // is not one of them
    check("atan2_rel_y and atan2_rel_x: both empty or neither", ry.is_empty() == rx.is_empty(),
          [&] { return "c = " + hex(c) + ", y = " + hex(y) + ", x = " + hex(x) + ": " + hex(ry) + ", " + hex(rx); });
    if (!ry.is_empty() && !rx.is_empty()) {
      interval near = c;
      for (int k = 0; k < 4; ++k) {
        near = interval(previous_double(near.left()), next_double(near.right()));
      }
      check("atan2_rel_y and atan2_rel_x: the angles of the box of the results meet c, widened by 4 doubles",
            !(atan2(ry, rx) & near).is_empty(),
            [&] { return "c = " + hex(c) + ", y = " + hex(y) + ", x = " + hex(x) + ": " + hex(ry) + ", " + hex(rx); });
    }
    const std::vector<double> ys = coordinates(y, random), xs = coordinates(x, random);
    std::vector<std::pair<double, double> > points; // (y, x)
    for (double py : ys) {
      for (double px : xs) {
        points.push_back(std::make_pair(py, px));
      }
    }
    {
      RoundingToNearest nearest;
      for (double theta : { c.left(), c.right() }) {
        if (!std::isfinite(theta)) {
          continue;
        }
        const double t = std::tan(theta);
        for (double e : xs) {
          double py = e*t;
          for (int k = 0; k < 3 && std::isfinite(py); ++k) {
            py = std::nextafter(py, -inf);
          }
          for (int k = 0; k < 7 && std::isfinite(py); ++k, py = std::nextafter(py, inf)) {
            if (y.left() <= py && py <= y.right()) {
              points.push_back(std::make_pair(py, e));
            }
          }
        }
        for (double e : ys) {
          double px = e/t;
          for (int k = 0; k < 3 && std::isfinite(px); ++k) {
            px = std::nextafter(px, -inf);
          }
          for (int k = 0; k < 7 && std::isfinite(px); ++k, px = std::nextafter(px, inf)) {
            if (x.left() <= px && px <= x.right()) {
              points.push_back(std::make_pair(e, px));
            }
          }
        }
      }
    }
    for (const std::pair<double, double>& p : points) {
      if (p.first == 0.0 && p.second == 0.0) {
        continue;
      }
      const int in = angle_in(p.first, p.second, c);
      if (in < 0) {
        ++undecided;
        continue;
      }
      if (in == 1) {
        const auto describe = [&] {
          return "c = " + hex(c) + ", y = " + hex(y) + ", x = " + hex(x) + ": the point (x, y) = (" + hex(p.second)
            + ", " + hex(p.first) + "), atan2_rel_y " + hex(ry) + ", atan2_rel_x " + hex(rx);
        };
        check("atan2_rel_y(c, x, y): every sampled y whose point has its angle in c", ry.set_contains(p.first), describe);
        check("atan2_rel_x(c, y, x): every sampled x whose point has its angle in c", rx.set_contains(p.second), describe);
      }
    }
  }

  // An interval of random bounds, of magnitudes from 2^emin to 2^emax, zero
  // or infinite now and then
  interval random_box_side(Random& random, int emin, int emax)
  {
    double b[2];
    for (double& d : b) {
      const int kind = random.integer(0, 9);
      d = (kind == 0) ? 0.0 : (kind == 1) ? ((random.integer(0, 1) == 0) ? -inf : inf) : random(emin, emax);
    }
    const interval v = gaol_tests::hull(b[0], b[1]);
    return v.is_empty() ? interval(-1.0, 1.0) : v;
  }

  void atan2_rel_functions()
  {
    Random random;
    long undecided = 0;
    for (const atan2_rel_values::Case& k : atan2_rel_values::cases) {
      const interval c = make(k.c), y = make(k.y), x = make(k.x);
      const bool empty_argument = k.c.empty || k.y.empty || k.x.empty;
      const interval ry = evaluate("atan2_rel_y(c, x, y)", [&] { return atan2_rel_y(c, x, y); }, [] { return std::string(); });
      const interval rx = evaluate("atan2_rel_x(c, y, x)", [&] { return atan2_rel_x(c, y, x); }, [] { return std::string(); });
      const auto describe = [&] {
        return "c = " + hex(c) + ", y = " + hex(y) + ", x = " + hex(x) + " (" + k.from + "): atan2_rel_y " + hex(ry)
          + ", tightest " + hex(make(k.y_tightest)) + ", accurate within " + hex(make(k.y_accurate)) + "; atan2_rel_x "
          + hex(rx) + ", tightest " + hex(make(k.x_tightest)) + ", accurate within " + hex(make(k.x_accurate));
      };
      atan2_expect("atan2_rel_y(c, x, y)", ry, k.y_tightest, k.y_accurate, empty_argument, describe);
      atan2_expect("atan2_rel_x(c, y, x)", rx, k.x_tightest, k.x_accurate, empty_argument, describe);
      // The names of IEEE 1788, the last argument [-oo, +oo] when left out
      check("gaol_ieee1788::atan2Rev1(x, c, y) is atan2_rel_y(c, x, y), atan2Rev2(y, c, x) atan2_rel_x(c, y, x)",
            same(gaol_ieee1788::atan2Rev1(x, c, y), ry) && same(gaol_ieee1788::atan2Rev2(y, c, x), rx), describe);
      check("gaol_ieee1788::atan2Rev1(x, c) and atan2Rev2(y, c): the last argument [-oo, +oo]",
            same(gaol_ieee1788::atan2Rev1(x, c), atan2_rel_y(c, x, interval::universe()))
            && same(gaol_ieee1788::atan2Rev2(y, c), atan2_rel_x(c, y, interval::universe()))
            && (!y.is_entire() || same(gaol_ieee1788::atan2Rev1(x, c), ry))
            && (!x.is_entire() || same(gaol_ieee1788::atan2Rev2(y, c), rx)), describe);
      sample(c, y, x, random, undecided);
    }
    // Random angles and boxes, sampled
    for (int i = 0; i < 3000; ++i) {
      double a = random.uniform(-4.0, 4.0), b = random.uniform(-4.0, 4.0);
      if (random.integer(0, 4) == 0) {
        b = a;
      }
      const interval c = gaol_tests::hull(a, b);
      const bool any = random.integer(0, 2) == 0;
      const interval y = random_box_side(random, any ? -1074 : -20, any ? 1023 : 20);
      const interval x = random_box_side(random, any ? -1074 : -20, any ? 1023 : 20);
      sample(c, y, x, random, undecided);
    }

    // Zero and infinite bounds raise no invalid-operation flag (0 times +oo,
    // +oo / +oo...), and give no NaN bound
    std::vector<interval> sides;
    const double bounds[] = { -inf, -2.0, -0.0, 0.0, 3.0, inf };
    for (double l : bounds) {
      for (double u : bounds) {
        if (!interval(l, u).is_empty()) {
          sides.push_back(interval(l, u));
        }
      }
    }
    const interval angles[] = { interval::universe(), interval(0.0), interval(-0.0, 0.0), interval::pi(), -interval::pi(),
                                interval::half_pi(), -interval::half_pi(), interval(1.0, inf), interval(-inf, -1.0),
                                interval(0.0, inf), interval(-inf, 0.0), interval(0x1p-1074, 1.0) };
    for (const interval& c : angles) {
      for (const interval& y : sides) {
        for (const interval& x : sides) {
          std::feclearexcept(FE_INVALID);
          const interval ry = atan2_rel_y(c, x, y), rx = atan2_rel_x(c, y, x);
          const bool raised = std::fetestexcept(FE_INVALID) != 0;
          check("atan2_rel_y and atan2_rel_x of zero and infinite bounds raise no invalid-operation flag", !raised,
                [&] { return "c = " + hex(c) + ", y = " + hex(y) + ", x = " + hex(x); });
          check("atan2_rel_y and atan2_rel_x: no NaN bound, results within y and x",
                (ry.is_empty() || (!std::isnan(ry.left()) && !std::isnan(ry.right()) && y.set_contains(ry)))
                && (rx.is_empty() || (!std::isnan(rx.left()) && !std::isnan(rx.right()) && x.set_contains(rx))),
                [&] { return "c = " + hex(c) + ", y = " + hex(y) + ", x = " + hex(x) + ": " + hex(ry) + ", " + hex(rx); });
          sample(c, y, x, random, undecided);
        }
      }
    }
    check("atan2_rel: whether the angle of a sampled point is in c is always decided", undecided == 0,
          [&] { return std::to_string(undecided) + " points undecided"; });
  }
}

int main()
{
  gaol::init();
  reverse_functions();
  pow_reverse_cases();
  pow_reverse_sampling();
  pow_reverse_examples();
  atan2_rel_functions();
  const int status = summary();
  gaol::cleanup();
  return status;
}
