/*-*-C++-*------------------------------------------------------------------
 * GAOL -- not Just Another Interval Library
 *--------------------------------------------------------------------------
 * Tests of GAOL v5: the relational functions, against the reverse
 * functions of IEEE 1788-2015 (10.5.4, Table 10.1).
 *
 * IEEE 1788 defines fRev(c, x) = hull{ x in x | f(x) is defined and in c } and
 * mulRev(b, c, x) = hull{ x in x | x*b is in c for some b in b }, x being the
 * whole line when absent. GAOL computes them as sqrt_rel(c, x) (sqrRev),
 * invabs_rel(c, x) (absRev), nth_root_rel(c, p, x) (pownRev, for every integer
 * p: for p > 0 with an unsigned and with an int p, which have to agree, and
 * for p <= 0, GAOL v5), gaol_ieee1788::pownRev(c, x, p),
 * asin_rel, acos_rel and atan_rel (sinRev, cosRev, tanRev), acosh_rel
 * (coshRev), and div_rel(c, b, x), c % b and c %= b (mulRev), c % d and
 * c %= d for b = [d].
 *
 * In each case of reverse_values.h (the minimal tests of libieeep1788, random
 * ones, and, for sinRev, cosRev and tanRev, a bound of x one or two doubles
 * beside a point where the function reaches -1 or 1 or has a pole, the
 * solutions next to it being a period away, see reverse_values.py), GAOL's
 * result has to be, as IEEE 1788 asks of the reverse functions (12.10.2):
 *  - empty when an argument is;
 *  - valid: enclose the hull, whose tightest enclosure the table gives;
 *  - accurate, which the standard recommends for inf-sup types: be within
 *    nextOut(tightest(nextOut(c), nextOut(x))) (12.10.1), the tightest result
 *    for arguments widened by one double, widened by one double.
 * The bounds of asin_rel, acos_rel and atan_rel are allowed one double
 * further, their reduction modulo pi rounding twice, and the bound of x
 * beyond 2^52, which they keep, as doc/accuracy.md documents. Those of
 * nth_root_rel for a negative p (pownRev, GAOL v5), the root of each part of c
 * and then its inverse, are allowed two doubles further: of the 18 052 random
 * cases with p < 0 of nonpositive_pown_rev_cases(20000) in reverse_values.py,
 * 12 495 have a nonempty result with nonempty accurate bounds, of which 521
 * were one double beyond them, 25 two, none more.
 * The summary also gives how far from the tightest bounds the results were,
 * where both are nonempty.
 *
 * atan2_rel_y and atan2_rel_x (atan2Rev1 and atan2Rev2, GAOL v5) have their
 * own section below, their own table (atan2_rel_values.h), and checks of
 * soundness on points of the boxes.
 *--------------------------------------------------------------------------
 * Copyright (c) 2026 ENSTA, France
 *
 * By: Jordan NININ (created 2026-09-20)
 *--------------------------------------------------------------------------
 * GAOL is a software distributed WITHOUT ANY WARRANTY. Read the associated
 * COPYING.LIB file for information.
 *--------------------------------------------------------------------------*/

#include "gaol_tests.h"
#include "reverse_values.h"
#include "pow_rel_values.h"
#include "atan2_rel_values.h"

#if defined(GAOL_USING_NEON_INSTRUCTIONS) && GAOL_USING_NEON_INSTRUCTIONS
#  include <arm_neon.h>
#endif

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
    double t = a;
    while (t < b && n <= limit) {
      ++n;
      t = next_double(t);
    }
    return n;
  }

  double previous_double(double x) { return -next_double(-x); }

  // 2^52, beyond which asin_rel, acos_rel and atan_rel keep the bound of x
  const double two_power_52 = 4503599627370496.0;

  // Checks the result r of the function named name in the case k. The accurate
  // bounds are allowed slack doubles further: 1 for asin_rel, acos_rel and
  // atan_rel, periodic telling them from the others, and 2 for nth_root_rel of
  // a negative p, the root then its inverse, each of which rounds
  template<class Describe>
  void expect(const std::string& name, const interval& r, const reverse_values::Case& k, int slack, bool periodic,
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
    // The cases written out (pownRev for p <= 0, GAOL v5) with an empty hull:
    // nextOut widens the arguments to a solution (the point 0 is no solution,
    // and the gap between the parts of c holds none), which the accurate bounds
    // then hold, so that they cannot tell [0] or the part of x in the gap from
    // the empty set: the result has to be empty
    if (std::string(k.from) == "written" && t.empty) {
      check(name + ": empty where the hull is, in the cases written out", r.is_empty(), describe);
    }
    // The accurate bounds, slack doubles further, or the bound of x where the
    // periodic functions keep it (see the head of this file)
    double low = a.l, high = a.u;
    if (!a.empty) {
      for (int i = 0; i < slack; ++i) {
        low = previous_double(low);
        high = next_double(high);
      }
      if (periodic) {
        low = std::fabs(k.x.l) > two_power_52 ? std::min(a.l, k.x.l) : low;
        high = std::fabs(k.x.u) > two_power_52 ? std::max(a.u, k.x.u) : high;
      }
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
        const std::string cp = "(" + hex(c) + ", " + std::to_string(k.p) + ", " + hex(x) + ")";
        if (k.p >= 1) {
          const unsigned int p = static_cast<unsigned int>(k.p);
          call("nth_root_rel(c, unsigned p, x)", "nth_root_rel" + cp, [&] { return nth_root_rel(c, p, x); });
        }
        // The int p, for every p (GAOL v5), and the standard's name, with and without x
        call("nth_root_rel(c, int p, x)", "nth_root_rel" + cp, [&] { return nth_root_rel(c, k.p, x); });
        call("pownRev(c, x, p)", "pownRev" + cp,
             [&] { return k.x_given ? gaol_ieee1788::pownRev(c, x, k.p) : gaol_ieee1788::pownRev(c, k.p); });
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
      if (f == "pownRev" && k.p >= 1) {
        const interval& u = calls[0].r;
        const interval& n = calls[1].r;
        check("nth_root_rel(c, int p, x) is nth_root_rel(c, unsigned p, x) for p >= 1",
              (u.is_empty() && n.is_empty()) || (!u.is_empty() && !n.is_empty() && u.left() == n.left() && u.right() == n.right()),
              [&] { return hex(u) + " " + hex(n); });
      }
      const bool periodic = (f == "sinRev" || f == "cosRev" || f == "tanRev");
      const int slack = periodic ? 1 : (f == "pownRev" && k.p < 0 ? 2 : 0);
      for (const Call& g : calls) {
        expect(g.name, g.r, k, slack, periodic, [&] {
          return g.text + " (" + f + " of IEEE 1788, case of " + k.from + "): " + hex(g.r) + ", tightest "
            + hex(make(k.tightest)) + ", accurate within " + hex(make(k.accurate));
        });
      }
    }
  }
}


//---------------------------------------------------------------------------
// The reverse functions of the pow of IEEE 1788-2015 (GAOL v5): powRev1(b, c,
// x), the x of x with an x^y in c for a y of b, and powRev2(a, c, x), the y
// of x with an a^y in c for an a of a, x^y being the pow of the standard
// (Table 9.1), that of gaol_ieee1788::pow, which has no negative base:
// pow_standard_rel(c, b, x) and pow_standard_exponent_rel(c, a, x) are these
// functions. pow_rel(c, b, x) and pow_exponent_rel(c, a, x) of gaol are those
// of gaol::pow (#400), see the second part below.
//
// On the cases of pow_rel_values.h (those of pow_rev.itl of ITF1788, random
// ones, ones whose solutions are doubles, and ones where a bound of x is next
// to an end of the solutions, see pow_rel_values.py), each result has to be
// empty when an argument is, to enclose the tightest enclosure of the hull,
// and to be within the tightest enclosure of the solutions in x widened by one
// double: the tightest result, or that one with a bound of x next to the
// solutions, which powRev1() keeps where it is not proved to miss them (see
// gaol/core/gaol_interval_core.h); pow_rel() and pow_exponent_rel(), which add
// the negative bases, have to enclose the result. Then, by sampling: the
// points (x, y) of random boxes whose power, the pow of the standard on [x]
// and [y], is within c, which proves x^y in c, have to be in the results, c
// being made of the powers of some of these points, so that the solutions
// reach its bounds.
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
      const std::string name = base ? "powRev1(b, c, x)" : "powRev2(a, c, x)";
      const std::string text = (base ? "powRev1(" : "powRev2(") + hex(b) + ", " + hex(c) + ", " + hex(x) + ")";
      // The names of IEEE 1788-2015, x left out where the case does: x is then the whole line
      const interval r = evaluate(name, [&] {
          return base ? (k.x_given ? gaol_ieee1788::powRev1(b, c, x) : gaol_ieee1788::powRev1(b, c))
            : (k.x_given ? gaol_ieee1788::powRev2(b, c, x) : gaol_ieee1788::powRev2(b, c)); },
        [&] { return text; });
      const auto describe = [&] {
        return text + " (case of " + k.from + "): " + hex(r) + ", tightest " + hex(t) + ", within " + hex(n);
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
      // The names of GAOL, pow_rel(c, b, x) and pow_exponent_rel(c, a, x), add the negative bases: their result
      // encloses the result of the standard
      const interval h = base ? pow_rel(c, b, x) : pow_exponent_rel(c, b, x);
      check(std::string(base ? "pow_rel" : "pow_exponent_rel") + ": encloses the result of " + name, within(r, h),
            [&] { return describe() + ", gaol's " + hex(h); });
      if (k.b.empty || k.c.empty || k.x.empty) {
        check(std::string(base ? "pow_rel" : "pow_exponent_rel") + ": empty for an empty argument", h.is_empty(), describe);
      }
    }
    std::printf("powRev1 and powRev2: the tightest result in %ld cases of %ld\n", tightest,
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
      const std::size_t i = static_cast<std::size_t>(random.integer(0, 23)), j = static_cast<std::size_t>(random.integer(0, 23));
      const int kind = random.integer(0, 3);
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
      const interval rx = gaol_ieee1788::powRev1(y, c, a), ry = gaol_ieee1788::powRev2(a, c, y);
      // The reverse functions of gaol::pow enclose them, with the negative bases and 0^0 (#400)
      check("pow_rel(c, b, x) encloses powRev1(b, c, x), and pow_exponent_rel(c, a, x) powRev2(a, c, x)",
            within(rx, pow_rel(c, y, a)) && within(ry, pow_exponent_rel(c, a, y)),
            [&] { return "c = " + hex(c) + ", b = " + hex(y) + ", x = " + hex(a); });
      for (std::size_t p = 0; p < 24; ++p) {
        if (powers[p].is_empty() || !c.set_contains(powers[p])) {
          continue; // x^y not proved to be in c
        }
        ++solutions;
        const auto describe = [&] {
          return "x = " + hex(xs[p]) + ", y = " + hex(ys[p]) + ", x^y in " + hex(powers[p]) + " within c = " + hex(c)
            + ": powRev1(" + hex(y) + ", c, " + hex(a) + ") = " + hex(rx) + ", powRev2(" + hex(a) + ", c, "
            + hex(y) + ") = " + hex(ry);
        };
        check("powRev1(b, c, x): every x of x with x^y in c for a y of b is kept", rx.set_contains(xs[p]), describe);
        check("powRev2(a, c, x): every y of x with a^y in c for an a of a is kept", ry.set_contains(ys[p]), describe);
      }
    }
    check("powRev1 and powRev2: the sampling found solutions", solutions > 20000,
          [&] { return std::to_string(solutions) + " solutions"; });
  }

  // The examples of the documentation (gaol/gaol_interval.h, doc/using.md, the manual), for the reverse
  // functions of the pow of IEEE 1788: powRev1(b, c, x) and powRev2(a, c, x)
  void pow_reverse_examples()
  {
    const interval entire = interval::universe();
    using gaol_ieee1788::powRev1;
    using gaol_ieee1788::powRev2;
    check("powRev1([2], [4, 9], entire) is [2, 3]", same_bounds(powRev1(interval(2.0), interval(4.0, 9.0), entire), interval(2.0, 3.0)));
    check("powRev1([1, +oo], [2, 3], [0, 1]) is empty, x^y >= 2 asking x > 1",
          powRev1(interval(1.0, GAOL_INFINITY), interval(2.0, 3.0), interval(0.0, 1.0)).is_empty());
    check("powRev2([2], [8], entire) is [3]", same_bounds(powRev2(interval(2.0), interval(8.0), entire), interval(3.0)));
    check("powRev1([0.5], [2], [0, 10]) is [4], 4^0.5 being 2 exactly",
          same_bounds(powRev1(interval(0.5), interval(2.0), interval(0.0, 10.0)), interval(4.0)));
    check("powRev1([1], [0], [-1, 1]) is [0], and powRev1([-1, 0], [0], [-1, 1]) empty: 0^y has a value for y > 0 only",
          same_bounds(powRev1(interval(1.0), interval(0.0), interval(-1.0, 1.0)), interval(0.0))
          && powRev1(interval(-1.0, 0.0), interval(0.0), interval(-1.0, 1.0)).is_empty());
    check("powRev1([2], [1, 4], [-3, 3]) is [1, 2]: no negative base, where pow(x, 2) of gaol has one",
          same_bounds(powRev1(interval(2.0), interval(1.0, 4.0), interval(-3.0, 3.0)), interval(1.0, 2.0)));
    // The double below the square root of 2, whose square is more than one
    // double below 2, is proved to miss it; 1 + 2^-51, whose square root is
    // within one double below 1 + 2^-52, is kept, (1 + 2^-52)^2 being
    // 1 + 2^-51 + 2^-104: the tightest result for x one double wider
    const double below_root_2 = 0x1.6a09e667f3bccp+0;
    check("powRev1([2], [2], [0, the double below the square root of 2]) is empty",
          powRev1(interval(2.0), interval(2.0), interval(0.0, below_root_2)).is_empty());
    check("powRev1([2], [2], [0, 2]) is the tightest enclosure of the square root of 2",
          same_bounds(powRev1(interval(2.0), interval(2.0), interval(0.0, 2.0)), interval(below_root_2, 0x1.6a09e667f3bcdp+0)));
    check("powRev1([0.5], [1 + 2^-52], [0, 1 + 2^-51]) is [1 + 2^-51]",
          same_bounds(powRev1(interval(0.5), interval(0x1.0000000000001p+0), interval(0.0, 0x1.0000000000002p+0)),
                      interval(0x1.0000000000002p+0)));
  }

  //-------------------------------------------------------------------------
  // The reverse functions of gaol::pow, pow_rel(c, b, x) and
  // pow_exponent_rel(c, a, x) (#400), and gaol::pow itself. gaol::pow(I, J) is
  // the hull of h over I x J, h(x, y) being pown(x, n) where y is an integer
  // n, and the pow of IEEE 1788-2015 (Table 9.1) elsewhere; pow_rel and
  // pow_exponent_rel are the reverse of h. h differs from the pow of the
  // standard in two places only, the negative bases with an integer exponent,
  // and 0^0 = 1.
  //-------------------------------------------------------------------------

  // The integers of the brute force: the exponents of the boxes below are
  // within the bounds {-oo, -3, ..., 3, +oo} and a few doubles that are no
  // integer, and the extreme integers of a parity that matter are within 60
  const int brute_integers = 60;

  // gaol::pow(I, J) by enumeration: the pow of the standard, [1] for 0^0, and,
  // for each integer n of J, the powers x^n of the negative part of I, which
  // are those of t = -x >= 0, negated for an odd n
  interval brute_pow(const interval& I, const interval& J)
  {
    if (I.is_empty() || J.is_empty()) {
      return interval::emptyset();
    }
    interval r = gaol_ieee1788::pow(I, J);
    if (I.set_contains(0.0) && J.set_contains(0.0)) {
      r |= interval(1.0);
    }
    const interval negative = I & interval::negative();
    if (!negative.is_empty() && negative.left() < 0.0) {
      const interval T = -negative;
      for (int n = -brute_integers; n <= brute_integers; ++n) {
        if (J.set_contains(static_cast<double>(n))) {
          const interval P = gaol_ieee1788::pown(T, n);
          r |= (n % 2 != 0) ? -P : P;
        }
      }
    }
    return r;
  }

  // The kept part of the standard of the reverse, then for each integer n of
  // J: the t = -x with t^n in Kp, K cut to [0, +oo] for an even n, -K for an
  // odd one (pown(-t, n) = (-1)^n t^n), by the reverse of the standard on [n]
  interval brute_pow_rel(const interval& K, const interval& J, const interval& I)
  {
    if (K.is_empty() || J.is_empty() || I.is_empty()) {
      return interval::emptyset();
    }
    interval r = gaol_ieee1788::powRev1(J, K, I);
    if (I.set_contains(0.0) && J.set_contains(0.0) && K.set_contains(1.0)) {
      r |= interval(0.0);
    }
    const interval negative = I & interval::negative();
    if (!negative.is_empty() && negative.left() < 0.0) {
      const interval T = -negative;
      for (int n = -brute_integers; n <= brute_integers; ++n) {
        if (!J.set_contains(static_cast<double>(n))) {
          continue;
        }
        if (n == 0) {
          if (K.set_contains(1.0)) {
            r |= negative;
          }
          continue;
        }
        const interval Kp = ((n % 2 != 0) ? -K : K) & interval::positive();
        r |= -gaol_ieee1788::powRev1(interval(static_cast<double>(n)), Kp, T);
      }
    }
    return r;
  }

  interval brute_pow_exponent_rel(const interval& K, const interval& H, const interval& I)
  {
    if (K.is_empty() || H.is_empty() || I.is_empty()) {
      return interval::emptyset();
    }
    interval r = gaol_ieee1788::powRev2(H, K, I);
    if (H.set_contains(0.0) && I.set_contains(0.0) && K.set_contains(1.0)) {
      r |= interval(0.0);
    }
    const interval negative = H & interval::negative();
    if (!negative.is_empty() && negative.left() < 0.0) {
      const interval T = -negative;
      for (int n = -brute_integers; n <= brute_integers; ++n) {
        if (!I.set_contains(static_cast<double>(n))) {
          continue;
        }
        if (n == 0) {
          if (K.set_contains(1.0)) {
            r |= interval(0.0);
          }
          continue;
        }
        const interval Kp = ((n % 2 != 0) ? -K : K) & interval::positive();
        if (!gaol_ieee1788::powRev1(interval(static_cast<double>(n)), Kp, T).is_empty()) {
          r |= interval(static_cast<double>(n));
        }
      }
    }
    return r;
  }

  // The bounds of a box: the bounds around 0 and 1 and the infinities, then
  // exponents that are no integer
  std::vector<interval> boxes_of(const std::vector<double>& bounds)
  {
    std::vector<interval> boxes;
    for (std::size_t i = 0; i < bounds.size(); ++i) {
      for (std::size_t j = i; j < bounds.size(); ++j) {
        boxes.push_back(interval(bounds[i], bounds[j]));
      }
    }
    return boxes;
  }

  // The reverse functions and the forward function against their enumeration
  // over the integers of the exponent: the same bounds, on every box of the
  // grids below and on random boxes. The forward bounds are those of pown
  // for the integer exponents of the enumeration, and of CORE-MATH's pow at
  // the corners for the ones of the hull, both the tightest ones but for the
  // rounding of pown, so that they are allowed to differ by the two doubles
  // pown is
  void pow_hybrid_enumeration()
  {
    const double oo = GAOL_INFINITY;
    const std::vector<interval> bases = boxes_of({ -oo, -3.0, -2.0, -1.5, -1.0, -0.5, 0.0, 0.5, 1.0, 1.5, 2.0, 3.0, oo });
    const std::vector<interval> powers = boxes_of({ -oo, -3.0, -1.0, -0.5, 0.0, 0.5, 1.0, 2.0, oo });
    const std::vector<interval> exponents = boxes_of({ -oo, -3.0, -2.0, -1.0, -0.5, 0.0, 0.5, 1.0, 1.5, 2.0, 3.0, oo });
    long forward = 0, rel = 0, exponent_rel = 0, nonempty = 0, exact_forward = 0, exact_reverse = 0, exact_random = 0;
    // The enumeration is the hull where the exponents are within its integers: with an exponent box that is not,
    // the integers beyond are limits (x^n for n -> -oo, for x = -0.5 and K = [-0.5], -1), which the enumeration
    // does not reach and the results have to enclose
    const auto within_enumeration = [](const interval& exponents_of_box) {
      return std::fabs(exponents_of_box.left()) <= brute_integers && std::fabs(exponents_of_box.right()) <= brute_integers;
    };
    const auto equal = [&](const interval& a, const interval& b, const interval& exponents_of_box) {
      return within_enumeration(exponents_of_box) ? same_bounds(a, b) : within(b, a);
    };
    for (const interval& I : bases) {
      for (const interval& J : exponents) {
        const interval p = pow(I, J), q = brute_pow(I, J);
        ++forward;
        exact_forward += within_enumeration(J) ? 1 : 0;
        check("pow(x, y): the hull over the integers of y, and the pow of the standard elsewhere",
              equal(p, q, J) || (within(q, p) && within(p, interval(q.left() - std::fabs(q.left())*0x1p-51,
                                                                 q.right() + std::fabs(q.right())*0x1p-51))),
              [&] { return "pow(" + hex(I) + ", " + hex(J) + ") = " + hex(p) + ", enumeration " + hex(q); });
        for (const interval& K : powers) {
          const interval r = pow_rel(K, J, I), s = brute_pow_rel(K, J, I);
          const interval e = pow_exponent_rel(K, I, J), f = brute_pow_exponent_rel(K, I, J);
          ++rel;
          ++exponent_rel;
          exact_reverse += within_enumeration(J) ? 1 : 0;
          nonempty += (!r.is_empty() ? 1 : 0) + (!e.is_empty() ? 1 : 0);
          check("pow_rel(c, b, x): the same bounds as the enumeration over the integers of b", equal(r, s, J),
                [&] { return "pow_rel(" + hex(K) + ", " + hex(J) + ", " + hex(I) + ") = " + hex(r) + ", enumeration " + hex(s); });
          check("pow_exponent_rel(c, a, x): the same bounds as the enumeration over the integers of x", equal(e, f, J),
                [&] { return "pow_exponent_rel(" + hex(K) + ", " + hex(I) + ", " + hex(J) + ") = " + hex(e) + ", enumeration " + hex(f); });
        }
      }
    }
    // Random boxes, the bounds of the grids and doubles of the grids' gaps, and bounds that are no
    // integer, for the exponents
    Random random;
    const double values[] = { -oo, -50.0, -7.5, -3.0, -2.0, -1.5, -1.0, -0.5, -0.0, 0.0, 0.5, 1.0, 1.5, 2.0, 2.5, 3.0, 4.0, 7.0, 20.0, 50.0, oo };
    const int n_values = static_cast<int>(sizeof(values)/sizeof(values[0]));
    const auto box = [&](bool wide) {
      double a = random.integer(0, 2) == 0 ? random(-3, 4) : values[random.integer(0, n_values - 1)];
      double b = random.integer(0, 2) == 0 ? random(-3, 4) : values[random.integer(0, n_values - 1)];
      if (wide && random.integer(0, 3) == 0) {
        a = -oo;
      }
      return interval(std::min(a, b), std::max(a, b));
    };
    for (int i = 0; i < 20000; ++i) {
      const interval I = box(false), J = box(true), K = box(true);
      const interval r = pow_rel(K, J, I), s = brute_pow_rel(K, J, I);
      const interval e = pow_exponent_rel(K, I, J), f = brute_pow_exponent_rel(K, I, J);
      nonempty += (!r.is_empty() ? 1 : 0) + (!e.is_empty() ? 1 : 0);
      exact_random += within_enumeration(J) ? 1 : 0;
      check("pow_rel(c, b, x): the same bounds as the enumeration over the integers of b, on random boxes", equal(r, s, J),
            [&] { return "pow_rel(" + hex(K) + ", " + hex(J) + ", " + hex(I) + ") = " + hex(r) + ", enumeration " + hex(s); });
      check("pow_exponent_rel(c, a, x): the same bounds as the enumeration over the integers of x, on random boxes",
            equal(e, f, J),
            [&] { return "pow_exponent_rel(" + hex(K) + ", " + hex(I) + ", " + hex(J) + ") = " + hex(e) + ", enumeration " + hex(f); });
    }
    check("pow_rel and pow_exponent_rel: the enumeration found solutions", nonempty > 100000,
          [&] { return std::to_string(nonempty) + " nonempty results in " + std::to_string(rel + exponent_rel) + " grid boxes"; });
    std::printf("gaol::pow and its reverse functions against the enumeration: %ld forward boxes, %ld and %ld reverse ones, %ld nonempty; "
                "the same bounds on %ld, %ld and %ld of the grid boxes and on %ld of 20000 random ones\n",
                forward, rel, exponent_rel, nonempty, exact_forward, exact_reverse, exact_reverse, exact_random);
  }

  // The enclosure of h at a point (x, y): gaol::pow on the degenerate boxes,
  // pown for an integer y, the pow of the standard elsewhere, which has no
  // value for x < 0
  interval h_at(double x, double y)
  {
    return gaol::pow(interval(x), interval(y));
  }

  // Soundness on points: for the points (x, y) of random boxes whose power h(x, y) is proved to be within c, x is
  // in pow_rel(c, J, I) and y in pow_exponent_rel(c, I, J), and h(x, y) is in pow(I, J). Part of the points have
  // an integer y, which is where the negative x are
  void pow_hybrid_sampling()
  {
    Random random;
    const double oo = GAOL_INFINITY;
    const double bounds[] = { -oo, -50.0, -3.0, -2.0, -1.5, -1.0, -0.5, -0.0, 0.0, 0.5, 1.0, 1.5, 2.0, 3.0, 50.0, oo };
    const int n_bounds = static_cast<int>(sizeof(bounds)/sizeof(bounds[0]));
    long solutions = 0, negative_solutions = 0;
    for (int box = 0; box < 8000; ++box) {
      double a1 = random.integer(0, 2) == 0 ? random(-3, 5) : bounds[random.integer(0, n_bounds - 1)];
      double a2 = random.integer(0, 2) == 0 ? random(-3, 5) : bounds[random.integer(0, n_bounds - 1)];
      double e1 = random.integer(0, 2) == 0 ? random(-3, 5) : bounds[random.integer(0, n_bounds - 1)];
      double e2 = random.integer(0, 2) == 0 ? random(-3, 5) : bounds[random.integer(0, n_bounds - 1)];
      if (a2 < a1) {
        std::swap(a1, a2);
      }
      if (e2 < e1) {
        std::swap(e1, e2);
      }
      const interval a(a1, a2), y(e1, e2);
      std::vector<double> xs, ys;
      std::vector<interval> powers;
      for (int i = 0; i < 24; ++i) {
        const double xi = sample(random, a.left(), a.right());
        double yi = sample(random, y.left(), y.right());
        // An integer y, for the negative x, where the integers of y are within it
        if (random.integer(0, 1) == 0 && std::fabs(yi) < 1000.0) {
          const double n = std::floor(yi + 0.5);
          yi = y.set_contains(n) ? n : yi;
        }
        xs.push_back(xi);
        ys.push_back(yi);
        powers.push_back(h_at(xi, yi));
      }
      const interval whole = pow(a, y);
      interval c;
      const std::size_t i = static_cast<std::size_t>(random.integer(0, 23)), j = static_cast<std::size_t>(random.integer(0, 23));
      const int kind = random.integer(0, 3);
      if (kind == 0 && !powers[i].is_empty() && !powers[j].is_empty()) {
        c = interval(std::min(powers[i].left(), powers[j].left()), std::max(powers[i].right(), powers[j].right()));
      } else if (kind == 1 && !powers[i].is_empty()) {
        c = powers[i];
      } else if (kind == 2 && !powers[i].is_empty()) {
        c = interval(powers[i].left(), random.integer(0, 1) ? oo : powers[i].right()*2.0);
      } else {
        const double c1 = bounds[random.integer(0, n_bounds - 1)], c2 = bounds[random.integer(0, n_bounds - 1)];
        c = interval(std::min(c1, c2), std::max(c1, c2));
      }
      if (c.is_empty()) {
        continue;
      }
      const interval rx = pow_rel(c, y, a), ry = pow_exponent_rel(c, a, y);
      for (std::size_t p = 0; p < 24; ++p) {
        if (powers[p].is_empty()) {
          continue;
        }
        const auto describe = [&] {
          return "x = " + hex(xs[p]) + ", y = " + hex(ys[p]) + ", h(x, y) in " + hex(powers[p]) + ", c = " + hex(c)
            + ", pow_rel(c, " + hex(y) + ", " + hex(a) + ") = " + hex(rx) + ", pow_exponent_rel(c, " + hex(a) + ", "
            + hex(y) + ") = " + hex(ry) + ", pow(x, y) = " + hex(whole);
        };
        check("pow(x, y): h(x, y) is kept, for the points of x and y", within(powers[p], whole), describe);
        if (!c.set_contains(powers[p])) {
          continue; // h(x, y) not proved to be in c
        }
        ++solutions;
        negative_solutions += (xs[p] < 0.0) ? 1 : 0;
        check("pow_rel(c, b, x): every x of x with h(x, y) in c for a y of b is kept", rx.set_contains(xs[p]), describe);
        check("pow_exponent_rel(c, a, x): every y of x with h(x, y) in c for an x of a is kept", ry.set_contains(ys[p]), describe);
      }
    }
    check("pow_rel and pow_exponent_rel: the sampling found solutions, negative bases too",
          solutions > 20000 && negative_solutions > 5000,
          [&] { return std::to_string(solutions) + " solutions, " + std::to_string(negative_solutions) + " with x < 0"; });
    std::printf("pow_rel and pow_exponent_rel: %ld sampled solutions of h, %ld with x < 0\n", solutions, negative_solutions);
  }

  // gaol::pow is monotone for the inclusion of its arguments: for the boxes of
  // a random box, the result is within the result of the box, up to the two
  // doubles that pown, and CORE-MATH's pow at the corners, differ by. The
  // degenerate integer exponents [n] of a box are the boxes that were not within it
  void pow_hybrid_monotonicity()
  {
    Random random;
    const double oo = GAOL_INFINITY;
    const double bounds[] = { -oo, -50.0, -3.0, -2.0, -1.5, -1.0, -0.5, -0.0, 0.0, 0.5, 1.0, 1.5, 2.0, 3.0, 50.0, oo };
    const int n_bounds = static_cast<int>(sizeof(bounds)/sizeof(bounds[0]));
    long compared = 0, integer_points = 0, outside = 0;
    for (int box = 0; box < 20000; ++box) {
      double a1 = random.integer(0, 2) == 0 ? random(-3, 5) : bounds[random.integer(0, n_bounds - 1)];
      double a2 = random.integer(0, 2) == 0 ? random(-3, 5) : bounds[random.integer(0, n_bounds - 1)];
      double e1 = random.integer(0, 2) == 0 ? random(-3, 5) : bounds[random.integer(0, n_bounds - 1)];
      double e2 = random.integer(0, 2) == 0 ? random(-3, 5) : bounds[random.integer(0, n_bounds - 1)];
      if (a2 < a1) {
        std::swap(a1, a2);
      }
      if (e2 < e1) {
        std::swap(e1, e2);
      }
      const interval I(a1, a2), J(e1, e2);
      if (I.is_empty() || J.is_empty()) {
        continue;
      }
      const interval whole = pow(I, J);
      const interval wider = whole.is_empty() ? whole : interval(whole.left() - std::fabs(whole.left())*0x1p-51,
                                                                 whole.right() + std::fabs(whole.right())*0x1p-51);
      for (int k = 0; k < 6; ++k) {
        // A smaller box: bounds of the box, or points of it; and, for the exponent, an integer of J half of the time
        const double xl = sample(random, I.left(), I.right()), xu = sample(random, I.left(), I.right());
        double yl = sample(random, J.left(), J.right()), yu = sample(random, J.left(), J.right());
        if (random.integer(0, 1) == 0 && std::fabs(yl) < 1000.0) {
          const double n = std::floor(yl + 0.5);
          if (J.set_contains(n)) {
            yl = yu = n;
            ++integer_points;
          }
        }
        const interval I2(std::min(xl, xu), std::max(xl, xu)), J2(std::min(yl, yu), std::max(yl, yu));
        const interval part = pow(I2, J2);
        ++compared;
        outside += within(part, whole) ? 0 : 1;
        check("pow(x, y) is monotone for the inclusion of x and y", within(part, wider),
              [&] { return "pow(" + hex(I2) + ", " + hex(J2) + ") = " + hex(part) + " is not within pow(" + hex(I) + ", "
                  + hex(J) + ") = " + hex(whole); });
      }
    }
    check("pow(x, y) is monotone: the sampling compared boxes, integer exponents too", compared > 100000 && integer_points > 20000,
          [&] { return std::to_string(compared) + " boxes, " + std::to_string(integer_points) + " with an integer exponent"; });
    std::printf("gaol::pow: %ld boxes within a box compared, %ld of them with an integer exponent, %ld outside the result of the box "
                "(within 2^-51 relatively)\n", compared, integer_points, outside);
  }

  // The examples of the documentation (gaol/gaol_interval.h, doc/using.md, the manual), and the cases of #400
  void pow_hybrid_examples()
  {
    const interval entire = interval::universe();
    const double oo = GAOL_INFINITY;
    const auto I = [](double l, double u) { return interval(l, u); };
    const auto P = [](double p) { return interval(p); };
    // gaol::pow: the hull of h
    check("pow([-1], [3]) is [-1], and pow([-2, 2], [0.5, 3]) is [-8, 8], which holds it",
          same_bounds(pow(P(-1.0), P(3.0)), P(-1.0)) && same_bounds(pow(I(-2.0, 2.0), I(0.5, 3.0)), I(-8.0, 8.0)));
    check("pow([-2, -1], [0.5, 3]) is [-8, 4]", same_bounds(pow(I(-2.0, -1.0), I(0.5, 3.0)), I(-8.0, 4.0)));
    check("pow([0], [0]) is [1], and pow([0], [0, 1]) is [0, 1], which holds it",
          same_bounds(pow(P(0.0), P(0.0)), P(1.0)) && same_bounds(pow(P(0.0), I(0.0, 1.0)), I(0.0, 1.0)));
    check("pow([-4, -1], [0.5]) is empty: no integer in [0.5]", pow(I(-4.0, -1.0), P(0.5)).is_empty());
    check("pow([-4, 9], [0.5]) is [0, 3] and pow([4, +oo], [0.5]) is [2, +oo]",
          same_bounds(pow(I(-4.0, 9.0), P(0.5)), I(0.0, 3.0)) && same_bounds(pow(I(4.0, oo), P(0.5)), I(2.0, oo)));
    // pow_rel and pow_exponent_rel: the reverse of h, with the negative bases and 0^0
    check("pow_rel([-1], [0.5, 3], [-2, 2]) is [-1]: x = -1, y = 3",
          same_bounds(pow_rel(P(-1.0), I(0.5, 3.0), I(-2.0, 2.0)), P(-1.0)));
    check("pow_exponent_rel([-1], [-2, 2], [0.5, 3]) is [1, 3]",
          same_bounds(pow_exponent_rel(P(-1.0), I(-2.0, 2.0), I(0.5, 3.0)), I(1.0, 3.0)));
    check("pow_rel([-8], [0.5, 3], [-3, 3]) is [-2]", same_bounds(pow_rel(P(-8.0), I(0.5, 3.0), I(-3.0, 3.0)), P(-2.0)));
    // x^n = 4 for the n of [1.5, 4.5] that are 2, 3 and 4: -2 and -sqrt(2); the double below sqrt(2) ends the result
    const double below_root_2 = 0x1.6a09e667f3bccp+0;
    check("pow_rel([4], [1.5, 4.5], [-3, -1]) is [-2, -sqrt(2)]",
          same_bounds(pow_rel(P(4.0), I(1.5, 4.5), I(-3.0, -1.0)), I(-2.0, -below_root_2)));
    check("pow_exponent_rel([-1000, -10], [-3, -2], [0, +oo]) is [3, 9]",
          same_bounds(pow_exponent_rel(I(-1000.0, -10.0), I(-3.0, -2.0), I(0.0, oo)), I(3.0, 9.0)));
    check("pow_rel([1], [0, 1], [0]) is [0]: 0^0 = 1", same_bounds(pow_rel(P(1.0), I(0.0, 1.0), P(0.0)), P(0.0)));
    check("pow_rel([1, 4], [2], [-3, 3]) is [-2, 2], where the pow of the standard gives [1, 2]",
          same_bounds(pow_rel(I(1.0, 4.0), P(2.0), I(-3.0, 3.0)), I(-2.0, 2.0))
          && same_bounds(gaol_ieee1788::powRev1(P(2.0), I(1.0, 4.0), I(-3.0, 3.0)), I(1.0, 2.0)));
    check("pow_rel([4, 9], [2], entire) is [-3, 3], pow_exponent_rel([8], [2], entire) is [3]",
          same_bounds(pow_rel(I(4.0, 9.0), P(2.0), entire), I(-3.0, 3.0))
          && same_bounds(pow_exponent_rel(P(8.0), P(2.0), entire), P(3.0)));
    // The case where the rounding of the exponents lets an integer in: K = [-oo, -(8 + 2^-49)], the bases
    // [-2, -1.5] and the exponents [0.5, 10]: log(8 + 2^-49)/log(2) is above 3 by less than half a double,
    // so that the exponents start at 3, and the odd integers with a solution at 5
    const interval K(-oo, -(8.0 + 0x1p-49));
    check("pow_exponent_rel([-oo, -(8 + 2^-49)], [-2, -1.5], [0.5, 10]) is [5, 9], not [3, 9]",
          same_bounds(pow_exponent_rel(K, I(-2.0, -1.5), I(0.5, 10.0)), I(5.0, 9.0)));
    // The same at the other end: the exponents with a solution end next to 9, where 2^9 is above the double below
    // 512 by less than half a double of the logarithm, and the odd integers with a solution end at 7
    const interval K_below_512(-(512.0 - 0x1p-44), -1.0);
    check("pow_exponent_rel([-(512 - 2^-44), -1], [-3, -2], [1, 20]) is [1, 7], not [1, 9]",
          same_bounds(pow_exponent_rel(K_below_512, I(-3.0, -2.0), I(1.0, 20.0)), I(1.0, 7.0)));
    // Beyond 2^53, where every double is an even integer: [d] has no odd integer, an interval has
    const double big = 0x1p60;
    check("pow_rel([-1], [big], [-2, -0.5]) is empty, pow_rel([1], [big], [-2, -0.5]) is [-1]: [d] is even",
          pow_rel(P(-1.0), P(big), I(-2.0, -0.5)).is_empty() && same_bounds(pow_rel(P(1.0), P(big), I(-2.0, -0.5)), P(-1.0)));
    check("pow_rel([-1], [2^60, 2^61], [-2, -0.5]) is [-1]: the odd integers between the doubles are in it",
          same_bounds(pow_rel(P(-1.0), I(big, 2.0*big), I(-2.0, -0.5)), P(-1.0)));
    // The integers around 2^53, where the move by 2 inwards of an integer that the rounding let in leaves the
    // doubles: the exponents [2^53 - 4, 2^53 + 4] hold the odd 2^53 + 1, no double, and with x = -(1 + 2^-44) the
    // power is about -e^512, within K (computed with mpmath at 4000 bits: x^y is at 3e-14 relatively from the
    // upper bound of K, and 9e-14 from the lower one). The solution (x, y) has to be kept: with the move to the
    // inside of 2^53 - 1 as the odd 2^53 + 1, rounded upward to 2^53 + 2, both results were empty
    const interval K53(-0x1.9476504b8f317p+738, -0x1.9476504b8efedp+738);
    const interval J53(0x1.ffffffffffffcp+52, 0x1.0000000000002p+53);
    const interval I53(-0x1.00000000001p+0);
    const interval r53 = pow_rel(K53, J53, I53), e53 = pow_exponent_rel(K53, I53, J53);
    check("pow_rel([-0x1.9476504b8f317p+738, -0x1.9476504b8efedp+738], [2^53 - 4, 2^53 + 4], [-(1 + 2^-44)]) keeps -(1 + 2^-44)",
          r53.set_contains(-0x1.00000000001p+0), [&] { return hex(r53); });
    check("pow_exponent_rel(the same K, [-(1 + 2^-44)], [2^53 - 4, 2^53 + 4]) holds the odd integer 2^53 + 1",
          !e53.is_empty() && e53.left() <= 0x1p+53 && e53.right() >= 0x1p+53 + 2.0, [&] { return hex(e53); });
    // The empty set for an empty argument, and the base [-oo] of a register without a check is empty
    check("pow_rel and pow_exponent_rel are empty for an empty argument",
          pow_rel(interval::emptyset(), P(2.0), entire).is_empty() && pow_rel(P(1.0), interval::emptyset(), entire).is_empty()
          && pow_rel(P(1.0), P(2.0), interval::emptyset()).is_empty()
          && pow_exponent_rel(interval::emptyset(), P(2.0), entire).is_empty()
          && pow_exponent_rel(P(1.0), interval::emptyset(), entire).is_empty()
          && pow_exponent_rel(P(1.0), P(2.0), interval::emptyset()).is_empty());
  }

  // [+oo] and [-oo], which no constructor makes but the one of a register
  // without a check (gaol_detail::unchecked_bounds), built from a register of
  // the SSE2 or the NEON intervals (the FPU intervals make none): they hold no
  // real number, so that pow_rel() and pow_exponent_rel() are empty for one
  // as any argument, as gaol_ieee1788::pow(x, y) is for an exponent: as the
  // power, the exponent or the base, which they test, and as the x or y to
  // find, whose result interval(lo, hi) is empty by itself (GAOL v5:
  // powRev1([+oo], [2]) was [1, 1 + 2^-52],
  // powRev2([+oo], [2]) [0, 2^-1074], powRev1([2], [+oo]) [DBL_MAX, +oo] and
  // powRev1([-oo], [0, 1]) [1, +oo], the limits of x^y)
  void pow_reverse_infinite_arguments()
  {
    std::vector<interval> infinite;
#if defined(GAOL_USING_SSE2_INSTRUCTIONS) && GAOL_USING_SSE2_INSTRUCTIONS
    for (const double d : { GAOL_INFINITY, -GAOL_INFINITY }) {
      infinite.push_back(interval(_mm_set_pd(d, -d), gaol_detail::unchecked_bounds));
    }
#elif defined(GAOL_USING_NEON_INSTRUCTIONS) && GAOL_USING_NEON_INSTRUCTIONS
    // The lanes hold the stored bounds <-left, right>
    for (const double d : { GAOL_INFINITY, -GAOL_INFINITY }) {
      const double lanes[2] = { -d, d };
      infinite.push_back(interval(vld1q_f64(lanes), gaol_detail::unchecked_bounds));
    }
#endif
    if (infinite.empty()) {
      std::printf("pow_rel and pow_exponent_rel with [+oo] or [-oo] as an argument: no SSE2 or NEON register to build them, skipped\n");
      return;
    }
    const std::vector<interval> others = { interval(0.5), interval(2.0), interval(0.0), interval(1.0), interval(0.0, 1.0),
                                           interval(0.5, 3.0), interval(-2.0, 2.0), interval::universe() };
    for (const interval& oo : infinite) {
      check("[+oo] and [-oo] are what the register made", oo.left() == oo.right() && std::fabs(oo.left()) == GAOL_INFINITY,
            [&] { return hex(oo); });
      for (const interval& a : others) {
        for (const interval& b : others) {
          // One argument is [+oo] or [-oo], in each of the three places
          const interval args[3][3] = { { oo, a, b }, { a, oo, b }, { a, b, oo } };
          for (int place = 0; place < 3; ++place) {
            const interval& k = args[place][0], &j = args[place][1], &i = args[place][2];
            const interval r = pow_rel(k, j, i), s = pow_exponent_rel(k, j, i);
            const auto describe = [&] { return "(" + hex(k) + ", " + hex(j) + ", " + hex(i) + "): " + hex(r) + ", " + hex(s); };
            check("pow_rel(c, b, x) with [+oo] or [-oo] as an argument is empty", r.is_empty(), describe);
            check("pow_exponent_rel(c, a, x) with [+oo] or [-oo] as an argument is empty", s.is_empty(), describe);
            // The names of IEEE 1788-2015, powRev1(b, c, x) and powRev2(a, c, x)
            check("powRev1(b, c, x) with [+oo] or [-oo] as an argument is empty", gaol_ieee1788::powRev1(j, k, i).is_empty(), describe);
            check("powRev2(a, c, x) with [+oo] or [-oo] as an argument is empty", gaol_ieee1788::powRev2(j, k, i).is_empty(), describe);
          }
        }
      }
      // The base [-oo] has no power, for an exponent that holds integers or not (GAOL v5, #400: its negative part
      // took the powers of [+oo]); the base [+oo] of gaol::pow is left as it was
      if (oo.left() < 0.0) {
        for (const interval& j : { interval(0.5, 3.0), interval(-3.0, -0.5), interval(0.0, 1.0), interval(1.5, 2.5),
                                   interval::universe() }) {
          check("pow([-oo], y) is empty", pow(oo, j).is_empty(), [&] { return hex(oo) + ", " + hex(j) + ": " + hex(pow(oo, j)); });
        }
      }
      // Without x, which is then the whole line
      for (const interval& a : others) {
        check("powRev1(b, c) and powRev2(a, c) with [+oo] or [-oo] as an argument are empty",
              gaol_ieee1788::powRev1(oo, a).is_empty() && gaol_ieee1788::powRev1(a, oo).is_empty()
              && gaol_ieee1788::powRev2(oo, a).is_empty() && gaol_ieee1788::powRev2(a, oo).is_empty(),
              [&] { return hex(oo) + " and " + hex(a); });
      }
    }
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
    double t = a;
    while (t < b && n <= limit) {
      ++n;
      t = next_double(t);
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
    adjacent doubles, which the decision needs. The reference is GAOL's own
    atan2(), so that this checks the soundness of the results against the
    points; the independent values, computed with mpmath, are those of the
    table of atan2_rel_values.h, which atan2_expect() checks
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
          for (int k = 0; k < 7 && std::isfinite(py); ++k) {
            if (y.left() <= py && py <= y.right()) {
              points.push_back(std::make_pair(py, e));
            }
            py = std::nextafter(py, inf);
          }
        }
        for (double e : ys) {
          double px = e/t;
          for (int k = 0; k < 3 && std::isfinite(px); ++k) {
            px = std::nextafter(px, -inf);
          }
          for (int k = 0; k < 7 && std::isfinite(px); ++k) {
            if (x.left() <= px && px <= x.right()) {
              points.push_back(std::make_pair(e, px));
            }
            px = std::nextafter(px, inf);
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
    check("atan2_rel_y and atan2_rel_x: whether the angle of a sampled point is in c is always decided", undecided == 0,
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
  pow_hybrid_examples();
  pow_hybrid_enumeration();
  pow_hybrid_sampling();
  pow_hybrid_monotonicity();
  pow_reverse_infinite_arguments();
  atan2_rel_functions();
  const int status = summary();
  gaol::cleanup();
  return status;
}
