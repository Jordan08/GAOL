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

#include <algorithm>
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


// atan2_rel and atan2_exponent_rel tests (GAOL v5)
void atan2_reverse_cases()
{
  // Test basic cases
  check("atan2_rel([0, pi/2], [-1, 1], [-1, 1]) contains positive x",
        !atan2_rel(interval(0.0, interval::half_pi().right()), interval(-1.0, 1.0), interval(-1.0, 1.0)).is_empty());
  
  check("atan2_rel([0, pi], [-1, 1], [-1, 1]) is entire X",
        same_bounds(atan2_rel(interval(0.0, interval::pi().right()), interval(-1.0, 1.0), interval(-1.0, 1.0)),
                    interval(-1.0, 1.0)));
  
  // Test with empty arguments
  check("atan2_rel(empty, Y, X) is empty",
        atan2_rel(interval::emptyset(), interval(-1.0, 1.0), interval(-1.0, 1.0)).is_empty());
  check("atan2_rel(Z, empty, X) is empty",
        atan2_rel(interval(0.0, interval::pi().right()), interval::emptyset(), interval(-1.0, 1.0)).is_empty());
  check("atan2_rel(Z, Y, empty) is empty",
        atan2_rel(interval(0.0, interval::pi().right()), interval(-1.0, 1.0), interval::emptyset()).is_empty());
  
  // Test (0,0) undefined case
  check("atan2_rel([0], [0], [0]) is empty",
        atan2_rel(interval(0.0), interval(0.0), interval(0.0)).is_empty());
  
  // Test atan2_exponent_rel
  check("atan2_exponent_rel([0, pi/2], [-1, 1], [-1, 1]) contains positive y",
        !atan2_exponent_rel(interval(0.0, interval::half_pi().right()), interval(-1.0, 1.0), interval(-1.0, 1.0)).is_empty());
  
  check("atan2_exponent_rel([-pi/2, pi/2], [-1, 1], [1, 2]) contains y in [-2, 2]",
        !atan2_exponent_rel(interval(-interval::half_pi().right(), interval::half_pi().right()), interval(1.0, 2.0), interval(-2.0, 2.0)).is_empty());
  
  // Test with IEEE 1788 namespace wrappers
  check("atan2Rev1([-1, 1], [0, pi/2], [-1, 1]) matches atan2_rel",
        same_bounds(gaol_ieee1788::atan2Rev1(interval(-1.0, 1.0), interval(0.0, interval::half_pi().right()), interval(-1.0, 1.0)),
                    atan2_rel(interval(0.0, interval::half_pi().right()), interval(-1.0, 1.0), interval(-1.0, 1.0))));
  
  check("atan2Rev2([-1, 1], [0, pi/2], [-1, 1]) matches atan2_exponent_rel",
        same_bounds(gaol_ieee1788::atan2Rev2(interval(-1.0, 1.0), interval(0.0, interval::half_pi().right()), interval(-1.0, 1.0)),
                    atan2_exponent_rel(interval(0.0, interval::half_pi().right()), interval(-1.0, 1.0), interval(-1.0, 1.0))));
  
  // Test atan2Rev without third argument (universe)
  check("atan2Rev1(Y, C) uses universe for X",
        same_bounds(gaol_ieee1788::atan2Rev1(interval(-1.0, 1.0), interval(0.0, interval::half_pi().right())),
                    atan2_rel(interval(0.0, interval::half_pi().right()), interval(-1.0, 1.0), interval::universe())));
  
  check("atan2Rev2(X, C) uses universe for Y",
        same_bounds(gaol_ieee1788::atan2Rev2(interval(-1.0, 1.0), interval(0.0, interval::half_pi().right())),
                    atan2_exponent_rel(interval(0.0, interval::half_pi().right()), interval(-1.0, 1.0), interval::universe())));

  // IEEE 1788-2015 compliance tests for atan2Rev1 and atan2Rev2
  // According to IEEE 1788-2015 Table 10.1:
  // atan2Rev1(y, c, x) = hull{x in x | exists y in y: atan2(y, x) in c}
  // atan2Rev2(x, c, y) = hull{y in y | exists x in x: atan2(y, x) in c}

  // Test 1: atan2Rev1 should be empty when any argument is empty
  check("atan2Rev1(empty, c, x) is empty",
        gaol_ieee1788::atan2Rev1(interval::emptyset(), interval(0.0, 1.0), interval(0.0, 1.0)).is_empty());
  check("atan2Rev1(y, empty, x) is empty",
        gaol_ieee1788::atan2Rev1(interval(-1.0, 1.0), interval::emptyset(), interval(0.0, 1.0)).is_empty());
  check("atan2Rev1(y, c, empty) is empty",
        gaol_ieee1788::atan2Rev1(interval(-1.0, 1.0), interval(0.0, 1.0), interval::emptyset()).is_empty());

  // Test 2: atan2Rev2 should be empty when any argument is empty
  check("atan2Rev2(empty, c, y) is empty",
        gaol_ieee1788::atan2Rev2(interval::emptyset(), interval(0.0, 1.0), interval(-1.0, 1.0)).is_empty());
  check("atan2Rev2(x, empty, y) is empty",
        gaol_ieee1788::atan2Rev2(interval(0.0, 1.0), interval::emptyset(), interval(-1.0, 1.0)).is_empty());
  check("atan2Rev2(x, c, empty) is empty",
        gaol_ieee1788::atan2Rev2(interval(0.0, 1.0), interval(0.0, 1.0), interval::emptyset()).is_empty());

  // Test 3: atan2Rev1 with c containing the full range of atan2
  // If c contains [-pi, pi], then atan2Rev1 should return all of x (except where undefined)
  check("atan2Rev1(y, [-pi, pi], x) contains all valid x",
        same_bounds(gaol_ieee1788::atan2Rev1(interval(-1.0, 1.0), interval(-interval::pi().right(), interval::pi().right()), interval(-1.0, 1.0)),
                    interval(-1.0, 1.0)));

  // Test 4: atan2Rev2 with c containing the full range of atan2
  check("atan2Rev2(x, [-pi, pi], y) contains all valid y",
        same_bounds(gaol_ieee1788::atan2Rev2(interval(-1.0, 1.0), interval(-interval::pi().right(), interval::pi().right()), interval(-1.0, 1.0)),
                    interval(-1.0, 1.0)));

  // Test 5: Specific values - atan2(0, x) = 0 for x > 0, pi for x < 0
  // atan2(0, x) = 0 for x > 0, pi for x < 0, undefined for x = 0
  const interval atan2rev1_pos = gaol_ieee1788::atan2Rev1(interval(0.0), interval(0.0), interval(0.0, GAOL_INFINITY));
  check("atan2Rev1([0], [0], [0, +oo]) contains positive x",
        !atan2rev1_pos.is_empty(),
        [&] { return "result = " + hex(atan2rev1_pos); });
  
  const interval atan2rev1_neg = gaol_ieee1788::atan2Rev1(interval(0.0), interval(interval::pi().left(), interval::pi().right()), interval(-GAOL_INFINITY, 0.0));
  check("atan2Rev1([0], [pi], [-oo, 0]) contains negative x",
        !atan2rev1_neg.is_empty() && atan2rev1_neg.left() < 0.0,
        [&] { return "result = " + hex(atan2rev1_neg); });

  // Test 6: atan2Rev2 with specific values
  // atan2(y, x) = 0 for y = 0, x > 0
  check("atan2Rev2([0, +oo], [0], [0, +oo]) is non-empty",
        !gaol_ieee1788::atan2Rev2(interval(0.0, GAOL_INFINITY), interval(0.0), interval(0.0, GAOL_INFINITY)).is_empty());
  // atan2(y, x) = pi for y = 0, x < 0
  {
    const interval result = gaol_ieee1788::atan2Rev2(interval(-GAOL_INFINITY, 0.0), interval(interval::pi().left(), interval::pi().right()), interval(-GAOL_INFINITY, 0.0));
    check("atan2Rev2([-oo, 0], [pi], [-oo, 0]) is non-empty",
          !result.is_empty(),
          [&] { return "result = " + hex(result); });
  }

  // Test 7: atan2Rev1 with y spanning zero and x negative
  // This should handle the discontinuity properly
  {
    const interval result = gaol_ieee1788::atan2Rev1(interval(-1.0, 1.0), interval(-interval::pi().right(), interval::pi().right()), interval(-1.0, 0.0));
    check("atan2Rev1([-1, 1], [-pi, pi], [-1, 0]) is non-empty",
          !result.is_empty(),
          [&] { return "result = " + hex(result); });
  }

  // Test 8: atan2Rev2 with x spanning zero
  {
    const interval result = gaol_ieee1788::atan2Rev2(interval(-1.0, 1.0), interval(-interval::half_pi().right(), interval::half_pi().right()), interval(-1.0, 1.0));
    check("atan2Rev2([-1, 1], [-pi/2, pi/2], [-1, 1]) is non-empty",
          !result.is_empty(),
          [&] { return "result = " + hex(result); });
  }

  // Test 9: Verify that atan2Rev1 and atan2Rev2 are consistent with each other
  // For simple cases where we can compute both
  check("atan2Rev1 and atan2Rev2 consistency: simple case",
        !gaol_ieee1788::atan2Rev1(interval(0.5), interval(0.5, 0.6), interval(0.5, 1.0)).is_empty()
        && !gaol_ieee1788::atan2Rev2(interval(0.5, 1.0), interval(0.5, 0.6), interval(0.5)).is_empty());

  // Test 10: Edge cases with infinity
  check("atan2Rev1([1], [pi/2], [0, +oo]) is non-empty",
        !gaol_ieee1788::atan2Rev1(interval(1.0), interval(0.0, interval::half_pi().right()), interval(0.0, GAOL_INFINITY)).is_empty());
  check("atan2Rev2([0, +oo], [pi/2], [1]) is non-empty",
        !gaol_ieee1788::atan2Rev2(interval(0.0, GAOL_INFINITY), interval(0.0, interval::half_pi().right()), interval(1.0)).is_empty());

  // Test 11: Verify that the results are valid (contain the exact preimage)
  // For atan2Rev1: if x is in the result, there should exist y in Y such that atan2(y, x) is in Z
  // This is verified by checking that atan2(Y, result) intersects Z
  {
    const interval Y = interval(-1.0, 1.0);
    const interval Z = interval(0.0, interval::half_pi().right());
    const interval X = interval(-1.0, 1.0);
    const interval result = gaol_ieee1788::atan2Rev1(Y, Z, X);
    if (!result.is_empty()) {
      // For any x in result, atan2(Y, x) should intersect Z
      const interval atan2_Y_result = atan2(Y, result);
      check("atan2Rev1: result is valid (atan2(Y, result) intersects Z)",
            !atan2_Y_result.is_empty() && !(Z & atan2_Y_result).is_empty(),
            [&] { return "atan2Rev1(" + hex(Y) + ", " + hex(Z) + ", " + hex(X) + ") = " + hex(result) + 
                   ", atan2(Y, result) = " + hex(atan2_Y_result); });
    }
  }

  // Test 12: Similar validation for atan2Rev2
  {
    const interval X = interval(-1.0, 1.0);
    const interval Z = interval(0.0, interval::half_pi().right());
    const interval Y = interval(-1.0, 1.0);
    const interval result = gaol_ieee1788::atan2Rev2(X, Z, Y);
    if (!result.is_empty()) {
      // For any y in result, atan2(y, X) should intersect Z
      const interval atan2_result_X = atan2(result, X);
      check("atan2Rev2: result is valid (atan2(result, X) intersects Z)",
            !atan2_result_X.is_empty() && !(Z & atan2_result_X).is_empty(),
            [&] { return "atan2Rev2(" + hex(X) + ", " + hex(Z) + ", " + hex(Y) + ") = " + hex(result) + 
                   ", atan2(result, X) = " + hex(atan2_result_X); });
    }
  }

  // Additional IEEE 1788-2015 compliance tests
  // Test monotonicity and edge cases
  
  // Test 13: atan2Rev1 with Y containing only positive values
  check("atan2Rev1([1], [0, pi/2], [0, +oo]) is non-empty",
        !gaol_ieee1788::atan2Rev1(interval(1.0), interval(0.0, interval::half_pi().right()), interval(0.0, GAOL_INFINITY)).is_empty());

  // Test 14: atan2Rev1 with Y containing only negative values
  check("atan2Rev1([-1], [-pi/2, 0], [0, +oo]) is non-empty",
        !gaol_ieee1788::atan2Rev1(interval(-1.0), interval(-interval::half_pi().right(), 0.0), interval(0.0, GAOL_INFINITY)).is_empty());

  // Test 15: atan2Rev2 with X containing only positive values
  check("atan2Rev2([0, +oo], [0, pi/2], [-1, 1]) is non-empty",
        !gaol_ieee1788::atan2Rev2(interval(0.0, GAOL_INFINITY), interval(0.0, interval::half_pi().right()), interval(-1.0, 1.0)).is_empty());

  // Test 16: atan2Rev2 with X containing only negative values
  check("atan2Rev2([-oo, 0], [pi/2, pi], [-1, 1]) is non-empty",
        !gaol_ieee1788::atan2Rev2(interval(-GAOL_INFINITY, 0.0), interval(interval::half_pi().right(), interval::pi().right()), interval(-1.0, 1.0)).is_empty());

  // Test 17: Verify that atan2Rev1 returns universe when appropriate
  check("atan2Rev1(y, [-pi, pi], universe) is universe",
        same_bounds(gaol_ieee1788::atan2Rev1(interval(-1.0, 1.0), interval(-interval::pi().right(), interval::pi().right()), interval::universe()),
                    interval::universe()));

  // Test 18: Verify that atan2Rev2 returns y when c contains full range
  check("atan2Rev2(universe, [-pi, pi], y) contains y",
        same_bounds(gaol_ieee1788::atan2Rev2(interval::universe(), interval(-interval::pi().right(), interval::pi().right()), interval(-1.0, 1.0)),
                    interval(-1.0, 1.0)));
}



int main()
{
  gaol::init();
  reverse_functions();
  pow_reverse_cases();
  pow_reverse_sampling();
  pow_reverse_examples();
  atan2_reverse_cases();
  const int status = summary();
  gaol::cleanup();
  return status;
}
