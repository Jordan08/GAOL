/*-*-C++-*------------------------------------------------------------------
 * gaol -- NOT Just Another Interval Library
 *--------------------------------------------------------------------------
 * Tests of this fork of GAOL: the relational functions, against the reverse
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

int main()
{
  gaol::init();
  reverse_functions();
  const int status = summary();
  gaol::cleanup();
  return status;
}
