/*-*-C++-*------------------------------------------------------------------
 * gaol -- NOT Just Another Interval Library
 *--------------------------------------------------------------------------
 * Tests of GAOL v5: the arithmetic operations.
 *
 * On random doubles and intervals, the bounds are compared exactly with the
 * exact results: sums, differences, products, quotients, squares and inverses
 * have to be enclosed as tightly as possible, which GAOL does computing with
 * the rounding direction set upward, and so do square roots, which IEEE
 * 1788-2015 requires (12.10.2), and integer powers, of either sign, whose
 * bounds GAOL v5 computes from exact products (pown_products() of
 * gaol/gaol_interval.cpp); n-th roots, which GAOL computes in several
 * rounded operations, to be enclosed within a few doubles. Doubles of every
 * magnitude are drawn, subnormal ones and results beyond the largest double
 * included. Integer powers for n up to 2^32 - 1 are compared with bounds
 * computed apart with mpmath, and the powers that are doubles have to be
 * exact. The operators of an interval with a
 * double are also compared with the operators with the degenerate interval of
 * the double, on bounds and doubles of special values: zeros of both signs,
 * infinities and NaN. Products of intervals with zero and infinite bounds have
 * to be the hull of the products of the bounds, a zero bound times an infinite
 * one counting as 0, the pairs of bounds that are not an interval being the
 * empty set. The n-th roots are the rootn of IEEE 1788-2015: for an odd n,
 * the root of a negative number is the opposite of the root of its magnitude.
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

  // The largest distances from the tightest bounds allowed, in doubles, about
  // twice those found on the platforms tested. Integer powers are the
  // tightest bounds, of either sign, below 2^-968 and beyond the largest
  // double too (GAOL v5: computed by repeated rounded products there, n + 1
  // doubles away at most, as GAOL computed every power, and x^-n as the
  // inverse of x^n, 8 doubles away). The n-th roots are powers with a rounded
  // exponent, whose error grows with |log x|, up to 8 doubles between 2^-30
  // and 2^30 and 234 over all the doubles. Square roots are the tightest
  // (GAOL's lower bound was one double below it for half of the doubles)
  const int power_limit = 0;
  const int square_root_limit = 0;
  const int root_limit = 2;

  std::string operands(const interval& x, const interval& y)
  {
    return "x=" + hex(x) + " y=" + hex(y);
  }

  // Checks that r is the tightest interval of doubles enclosing [lo, hi]
  void expect_tightest(const std::string& name, const interval& r, const Exact& lo, const Exact& hi,
                       const interval& x, const interval& y)
  {
    check(name, is_tightest_enclosure(r, lo, hi), [&] { return operands(x, y) + ": " + hex(r); });
  }

  void expect_tightest(const std::string& name, const interval& r, const Exact& v,
                       const interval& x, const interval& y)
  {
    expect_tightest(name, r, v, v, x, y);
  }

  // Checks that r encloses [lo, hi], with bounds no more than limit doubles
  // away from the tightest ones
  void expect_close(const std::string& name, const interval& r, const Exact& lo, const Exact& hi, int limit,
                    const interval& x, const interval& y)
  {
    const auto describe = [&] { return operands(x, y) + ": " + hex(r); };
    if (check(name + ": encloses", is_enclosure(r, lo, hi), describe)) {
      const int distance = std::max(doubles_below_tightest(r.left(), lo, limit),
                                    doubles_above_tightest(r.right(), hi, limit));
      check_distance(name, distance, limit, describe);
    }
  }

  // x^n exactly, x being finite and n >= 0
  Dyadic dyadic_power(double x, int n)
  {
    Dyadic p = dyadic(1.0);
    const Dyadic d = dyadic(x);
    for (int i = 0; i < n; ++i) {
      p = p*d;
    }
    return p;
  }

  // Whether l <= x^(1/n), respectively u >= x^(1/n), x being >= 0
  bool is_root_lower_bound(double l, double x, int n)
  {
    return l <= 0.0 || (l != inf && compare(power(l, n), exact(x)) <= 0);
  }

  bool is_root_upper_bound(double u, double x, int n)
  {
    return u == inf || (u >= 0.0 && compare(power(u, n), exact(x)) >= 0);
  }

  // Checks that r encloses x^(1/n), x being >= 0, with bounds no more than
  // limit doubles away from the tightest ones
  void expect_root(const std::string& name, const interval& r, double x, int n, int limit)
  {
    const auto describe = [&] { return "x=" + hex(x) + " n=" + std::to_string(n) + ": " + hex(r); };
    if (check(name + ": encloses", !r.is_empty() && is_root_lower_bound(r.left(), x, n)
                                   && is_root_upper_bound(r.right(), x, n), describe)) {
      int below = 0, above = 0;
      for (double t = next_double(r.left()); below <= limit && is_root_lower_bound(t, x, n); t = next_double(t)) {
        ++below;
      }
      for (double t = previous_double(r.right()); above <= limit && is_root_upper_bound(t, x, n); t = previous_double(t)) {
        ++above;
      }
      check_distance(name, std::max(below, above), limit, describe);
    }
  }

  // Operations on doubles, drawn by draw, root_limit being the largest distance
  // from the tightest bounds allowed for n-th roots
  template<class Draw>
  void operations_on_doubles(const std::string& range, Draw draw, int root_limit)
  {
    const std::string in = " (" + range + ")";
    for (int i = 0; i < nb_random_values; ++i) {
      const double a = draw(), b = draw();
      const interval A(a), B(b);
      const Dyadic da = dyadic(a), db = dyadic(b);
      const Exact sum = exact(da + db), difference = exact(da - db), product = exact(da*db);

      expect_tightest("[a]+[b]" + in, A + B, sum, A, B);
      expect_tightest("[a]+b" + in, A + b, sum, A, B);
      expect_tightest("a+[b]" + in, a + B, sum, A, B);
      { interval r(A); r += B; expect_tightest("[a]+=[b]" + in, r, sum, A, B); }
      { interval r(A); r += b; expect_tightest("[a]+=b" + in, r, sum, A, B); }

      expect_tightest("[a]-[b]" + in, A - B, difference, A, B);
      expect_tightest("[a]-b" + in, A - b, difference, A, B);
      expect_tightest("a-[b]" + in, a - B, difference, A, B);
      { interval r(A); r -= B; expect_tightest("[a]-=[b]" + in, r, difference, A, B); }
      { interval r(A); r -= b; expect_tightest("[a]-=b" + in, r, difference, A, B); }
      expect_tightest("-[a]" + in, -A, exact(-da), A, B);

      expect_tightest("[a]*[b]" + in, A * B, product, A, B);
      expect_tightest("[a]*b" + in, A * b, product, A, B);
      expect_tightest("a*[b]" + in, a * B, product, A, B);
      { interval r(A); r *= B; expect_tightest("[a]*=[b]" + in, r, product, A, B); }
      { interval r(A); r *= b; expect_tightest("[a]*=b" + in, r, product, A, B); }
      expect_tightest("sqr([a])" + in, sqr(A), exact(da*da), A, B);

      if (b != 0.0) {
        const Exact q = quotient(da, db);
        expect_tightest("[a]/[b]" + in, A / B, q, A, B);
        expect_tightest("[a]/b" + in, A / b, q, A, B);
        expect_tightest("a/[b]" + in, a / B, q, A, B);
        { interval r(A); r /= B; expect_tightest("[a]/=[b]" + in, r, q, A, B); }
        { interval r(A); r /= b; expect_tightest("[a]/=b" + in, r, q, A, B); }
        // The relational division: the x such that a = x*b
        expect_tightest("[a]%[b]" + in, A % B, q, A, B);
        expect_tightest("[a]%b" + in, A % b, q, A, B);
        expect_tightest("a%[b]" + in, a % B, q, A, B);
        { interval r(A); r %= B; expect_tightest("[a]%=[b]" + in, r, q, A, B); }
        { interval r(A); r %= b; expect_tightest("[a]%=b" + in, r, q, A, B); }
      }
      if (a != 0.0) {
        const Exact inv = quotient(dyadic(1.0), da);
        expect_tightest("inverse([a])" + in, inverse(A), inv, A, B);
        expect_tightest("[a].inverse()" + in, A.inverse(), inv, A, B);
      }

      // The exponents of the powers unrolled (up to 32), and above
      for (int n : { 2, 3, 4, 5, 6, 7, 8, 9, 16, 31, 32, 33 }) {
        const Exact p = power(a, n);
        const interval N(n);
        expect_close("pow([a],n) for n=" + std::to_string(n) + in, pow(A, n), p, p, power_limit, A, N);
      }
      if (a != 0.0) {
        for (int n : { 1, 2, 3, 4, 7, 16, 33 }) {
          const Dyadic an = dyadic_power(a, n);
          const Exact p = quotient(dyadic(1.0), an);
          const std::string name = "pow([a],-n) for n=" + std::to_string(n) + in;
          // (1/a)^n where a^n is beyond the largest double: GAOL computed
          // 1/a^n, [0, 1/max], wide at that scale
          expect_close(name, pow(A, -n), p, p, power_limit, A, interval(-n));
        }
      }

      const double m = std::fabs(a);
      expect_root("sqrt([|a|])" + in, sqrt(interval(m)), m, 2, square_root_limit);
      for (int n = 2; n <= 7; ++n) {
        expect_root("nth_root([|a|],n) for n=" + std::to_string(n) + in, nth_root(interval(m), static_cast<unsigned int>(n)), m, n, root_limit);
      }
      // The odd roots of -|a|, the opposites of those of |a|, and the even
      // roots of -|a|, which are empty unless a is 0
      for (unsigned int n = 3; n <= 7; ++n) {
        const interval pos = nth_root(interval(m), n), neg = nth_root(interval(-m), n);
        const std::string name = "nth_root([-|a|],n) for n=" + std::to_string(n) + in;
        const auto describe = [&] { return "x=" + hex(-m) + ": " + hex(neg); };
        if (odd(n)) {
          check(name + ": -nth_root([|a|],n)", !neg.is_empty() && neg.left() == -pos.right() && neg.right() == -pos.left(), describe);
        } else {
          check(name + ": empty unless a = 0", m == 0.0 ? (neg.left() == 0.0 && neg.right() == 0.0) : neg.is_empty(), describe);
        }
      }
    }
  }

  // Operations on intervals, whose bounds are drawn by draw
  template<class Draw>
  void operations_on_intervals(const std::string& range, Draw draw)
  {
    const std::string in = " (" + range + ")";
    const Exact zero = exact(0.0);
    for (int i = 0; i < nb_random_values; ++i) {
      const interval X = hull(draw(), draw()), Y = hull(draw(), draw());
      const Dyadic xl = dyadic(X.left()), xh = dyadic(X.right()), yl = dyadic(Y.left()), yh = dyadic(Y.right());
      const bool x_has_zero = X.set_contains(0.0), y_has_zero = Y.set_contains(0.0);

      expect_tightest("[x]+[y]" + in, X + Y, exact(xl + yl), exact(xh + yh), X, Y);
      expect_tightest("[x]-[y]" + in, X - Y, exact(xl - yh), exact(xh - yl), X, Y);
      { interval r(X); r += Y; expect_tightest("[x]+=[y]" + in, r, exact(xl + yl), exact(xh + yh), X, Y); }
      { interval r(X); r -= Y; expect_tightest("[x]-=[y]" + in, r, exact(xl - yh), exact(xh - yl), X, Y); }
      expect_tightest("-[x]" + in, -X, exact(-xh), exact(-xl), X, Y);

      // x op= x, the operand being *this: the FPU intervals wrote a bound of
      // *this before reading the one of the operand, and x -= x was [-2, 1]
      // for x = [-3, -1], x /= x and x %= x [0.5, 1] for x = [0.25, 0.5]
      // (GAOL v5)
      { interval r(X); r += r; expect_tightest("[x]+=[x], the same interval" + in, r, exact(xl + xl), exact(xh + xh), X, X); }
      { interval r(X); r -= r; expect_tightest("[x]-=[x], the same interval" + in, r, exact(xl - xh), exact(xh - xl), X, X); }
      {
        const std::vector<Exact> self_products = { exact(xl*xl), exact(xl*xh), exact(xh*xh) };
        interval r(X);
        r *= r;
        expect_tightest("[x]*=[x], the same interval" + in, r, min(self_products), max(self_products), X, X);
      }
      if (!x_has_zero) {
        const std::vector<Exact> self_quotients = { quotient(xl, xh), quotient(xh, xl), quotient(xl, xl) };
        { interval r(X); r /= r; expect_tightest("[x]/=[x], the same interval" + in, r, min(self_quotients), max(self_quotients), X, X); }
        { interval r(X); r %= r; expect_tightest("[x]%=[x], the same interval" + in, r, min(self_quotients), max(self_quotients), X, X); }
      }

      const std::vector<Exact> products = { exact(xl*yl), exact(xl*yh), exact(xh*yl), exact(xh*yh) };
      expect_tightest("[x]*[y]" + in, X * Y, min(products), max(products), X, Y);
      { interval r(X); r *= Y; expect_tightest("[x]*=[y]" + in, r, min(products), max(products), X, Y); }

      if (!y_has_zero) {
        const std::vector<Exact> quotients = { quotient(xl, yl), quotient(xl, yh), quotient(xh, yl), quotient(xh, yh) };
        expect_tightest("[x]/[y]" + in, X / Y, min(quotients), max(quotients), X, Y);
        { interval r(X); r /= Y; expect_tightest("[x]/=[y]" + in, r, min(quotients), max(quotients), X, Y); }
        expect_tightest("[x]%[y]" + in, X % Y, min(quotients), max(quotients), X, Y);
        expect_tightest("inverse([y])" + in, inverse(Y), quotient(dyadic(1.0), yh), quotient(dyadic(1.0), yl), X, Y);
      }

      // With a double of either sign, a bound of [y]
      const double d = (i % 2 == 0) ? Y.left() : Y.right();
      const interval D(d);
      const Dyadic dd = dyadic(d);
      { interval r(X); r += d; expect_tightest("[x]+=d" + in, r, exact(xl + dd), exact(xh + dd), X, D); }
      { interval r(X); r -= d; expect_tightest("[x]-=d" + in, r, exact(xl - dd), exact(xh - dd), X, D); }
      const std::vector<Exact> products_by_d = { exact(xl*dd), exact(xh*dd) };
      { interval r(X); r *= d; expect_tightest("[x]*=d" + in, r, min(products_by_d), max(products_by_d), X, D); }
      if (d != 0.0) {
        const std::vector<Exact> quotients_by_d = { quotient(xl, dd), quotient(xh, dd) };
        { interval r(X); r /= d; expect_tightest("[x]/=d" + in, r, min(quotients_by_d), max(quotients_by_d), X, D); }
        { interval r(X); r %= d; expect_tightest("[x]%=d" + in, r, min(quotients_by_d), max(quotients_by_d), X, D); }
      }

      const std::vector<Exact> squares = { exact(xl*xl), exact(xh*xh) };
      expect_tightest("sqr([x])" + in, sqr(X), x_has_zero ? zero : min(squares), max(squares), X, Y);
      for (int n : { 2, 3, 4, 5, 12, 31 }) {
        const std::vector<Exact> powers = { power(X.left(), n), power(X.right(), n) };
        const Exact lo = (n % 2 == 1) ? powers[0] : (x_has_zero ? zero : min(powers));
        const Exact hi = (n % 2 == 1) ? powers[1] : max(powers);
        expect_close("pow([x],n) for n=" + std::to_string(n) + in, pow(X, n), lo, hi, power_limit, X, interval(n));
      }
      // x^-n: 1/x^n at the bounds where x does not hold 0, and the limit
      // +oo or -oo at 0 where it does (pown of IEEE 1788-2015)
      for (int n : { 1, 2, 3, 6, 17 }) {
        const interval r = pow(X, -n);
        const std::string name = "pow([x],-n) for n=" + std::to_string(n) + in;
        const auto describe = [&] { return "x=" + hex(X) + ": " + hex(r); };
        const auto inverse_power = [n](double b) { return quotient(dyadic(1.0), dyadic_power(b, n)); };
        const double l = X.left(), u = X.right();
        if (!x_has_zero) {
          const std::vector<Exact> inverses = { inverse_power(l), inverse_power(u) };
          expect_close(name, r, min(inverses), max(inverses), power_limit, X, interval(-n));
        } else if (l == 0.0 && u == 0.0) {
          check(name + ": empty at [0]", r.is_empty(), describe);
        } else if (n % 2 == 0) {
          check(name + ": [mag(x)^-n, +oo]", !r.is_empty() && r.right() == inf
                && is_tightest_lower_bound(r.left(), inverse_power(std::max(-l, u))), describe);
        } else if (l < 0.0 && u > 0.0) {
          check(name + ": [-oo, +oo] for 0 inside", !r.is_empty() && r.left() == -inf && r.right() == inf, describe);
        } else if (l == 0.0) {
          check(name + ": [b^-n, +oo] for [0, b]", !r.is_empty() && r.right() == inf
                && is_tightest_lower_bound(r.left(), inverse_power(u)), describe);
        } else {
          check(name + ": [-oo, a^-n] for [a, 0]", !r.is_empty() && r.left() == -inf
                && is_tightest_upper_bound(r.right(), inverse_power(l)), describe);
        }
      }

      // Square roots: the tightest roots of the bounds of [x] & [0, +oo]
      {
        const interval r = sqrt(X);
        const auto describe = [&] { return "x=" + hex(X) + ": " + hex(r); };
        if (X.right() < 0.0) {
          check("sqrt([x])" + in + ": empty", r.is_empty(), describe);
        } else {
          const double lo = std::max(0.0, X.left());
          check("sqrt([x])" + in + ": the roots of the bounds",
                !r.is_empty() && r.left() == sqrt(interval(lo)).left() && r.right() == sqrt(interval(X.right())).right(),
                describe);
          expect_root("sqrt([x]) lower bound" + in, interval(r.left(), sqrt(interval(lo)).right()), lo, 2, 0);
        }
      }

      // n-th roots: increasing, from the roots of the bounds, on the whole of
      // [x] for an odd n and on its part in [0, +oo] for an even n
      for (unsigned int n = 3; n <= 6; ++n) {
        const interval r = nth_root(X, n);
        const std::string name = "nth_root([x],n) for n=" + std::to_string(n) + in;
        const auto describe = [&] { return "x=" + hex(X) + ": " + hex(r); };
        if (odd(n) || X.right() >= 0.0) {
          const double lo = odd(n) ? X.left() : std::max(0.0, X.left());
          check(name + ": the roots of the bounds",
                !r.is_empty() && r.left() == nth_root(interval(lo), n).left() && r.right() == nth_root(interval(X.right()), n).right(),
                describe);
        } else {
          check(name + ": empty", r.is_empty(), describe);
        }
      }

      const std::vector<Exact> magnitudes = { exact(std::fabs(X.left())), exact(std::fabs(X.right())) };
      expect_tightest("abs([x])" + in, abs(X), x_has_zero ? zero : min(magnitudes), max(magnitudes), X, Y);

      const double lo = std::max(X.left(), Y.left()), hi = std::min(X.right(), Y.right());
      const interval meet = X & Y;
      if (lo <= hi) {
        expect_tightest("[x]&[y]" + in, meet, exact(lo), exact(hi), X, Y);
      } else {
        check("[x]&[y]" + in, meet.is_empty(), [&] { return operands(X, Y) + ": " + hex(meet); });
      }
      expect_tightest("[x]|[y]" + in, X | Y, exact(std::min(X.left(), Y.left())), exact(std::max(X.right(), Y.right())), X, Y);
      expect_tightest("min([x],[y])" + in, min(X, Y), exact(std::min(X.left(), Y.left())), exact(std::min(X.right(), Y.right())), X, Y);
      expect_tightest("max([x],[y])" + in, max(X, Y), exact(std::max(X.left(), Y.left())), exact(std::max(X.right(), Y.right())), X, Y);
    }
  }

  // Divisions by intervals containing zero: the enclosure of the quotients
  // a/b for a in [x] and b in [y] other than zero (x/y), and of the x such
  // that some a in [x] is x*b for some b in [y] (x%y)
  void divisions_by_zero()
  {
    struct Case
    {
      interval x, y;
      double lo, hi;
      bool empty;
    };
    const Case divisions[] = {
      { interval(1., 2.), interval(0., 1.), 1., inf, false },
      { interval(1., 2.), interval(-1., 0.), -inf, -1., false },
      { interval(-2., -1.), interval(0., 1.), -inf, -1., false },
      { interval(-2., -1.), interval(-1., 0.), 1., inf, false },
      { interval(1., 2.), interval(-1., 1.), -inf, inf, false },
      { interval(-1., 2.), interval(0., 1.), -inf, inf, false },
      { interval(0., 2.), interval(0., 1.), 0., inf, false },
      { interval(-2., 0.), interval(0., 1.), -inf, 0., false },
      { interval(0., 0.), interval(-1., 1.), 0., 0., false },
      { interval(1., 2.), interval(0., 0.), 0., 0., true },
    };
    for (const auto& c : divisions) {
      const interval r = c.x / c.y;
      const auto describe = [&] { return operands(c.x, c.y) + ": " + hex(r); };
      if (c.empty) {
        check("[x]/[y] for [y] containing 0: empty", r.is_empty(), describe);
      } else {
        check("[x]/[y] for [y] containing 0: encloses", !r.is_empty() && r.left() <= c.lo && r.right() >= c.hi, describe);
        check("[x]/[y] for [y] containing 0: tightest", r.left() == c.lo && r.right() == c.hi, describe);
      }
    }
    const Case relational_divisions[] = {
      { interval(1., 2.), interval(0., 1.), 1., inf, false },
      { interval(1., 2.), interval(-1., 0.), -inf, -1., false },
      { interval(1., 2.), interval(-1., 1.), -inf, inf, false },
      { interval(0., 0.), interval(-1., 1.), -inf, inf, false },
      { interval(-1., 2.), interval(0., 0.), -inf, inf, false },
      { interval(1., 2.), interval(0., 0.), 0., 0., true },
    };
    for (const auto& c : relational_divisions) {
      const interval r = c.x % c.y;
      const auto describe = [&] { return operands(c.x, c.y) + ": " + hex(r); };
      if (c.empty) {
        check("[x]%[y] for [y] containing 0: empty", r.is_empty(), describe);
      } else {
        check("[x]%[y] for [y] containing 0: encloses", !r.is_empty() && r.left() <= c.lo && r.right() >= c.hi, describe);
        check("[x]%[y] for [y] containing 0: tightest", r.left() == c.lo && r.right() == c.hi, describe);
      }
    }
  }

  // The operators of an interval with a double d, on bounds and doubles of
  // special values, which have no exact result to compare with: the same set
  // as the operators with interval(d), which the operations above and
  // divisions_by_zero() check
  void operations_with_special_doubles()
  {
    const double subnormal = std::numeric_limits<double>::denorm_min(), largest = std::numeric_limits<double>::max();
    const double bounds[] = { -inf, -largest, -2., -1., -subnormal, -0., 0., subnormal, 1., 3., largest, inf };
    const double doubles[] = { -inf, -largest, -3., -1., -0.1, -subnormal, -0., 0., subnormal, 0.1, 1., 3., largest, inf,
                               std::numeric_limits<double>::quiet_NaN() };
    std::vector<interval> xs = { interval::emptyset() };
    for (double l : bounds) {
      for (double u : bounds) {
        if (l <= u) {
          xs.push_back(interval(l, u));
        }
      }
    }
    for (const interval& x : xs) {
      for (double d : doubles) {
        const interval D(d);
        const auto expect_same = [&](const std::string& op, const interval& r, const interval& expected) {
          check("[x]" + op + "d for special values: as [x]" + op + "interval(d)",
                r.is_empty() ? expected.is_empty()
                             : !expected.is_empty() && r.left() == expected.left() && r.right() == expected.right(),
                [&] { return operands(x, D) + ": " + hex(r) + " rather than " + hex(expected); });
        };
        { interval r(x), e(x); r += d; e += D; expect_same("+=", r, e); }
        { interval r(x), e(x); r -= d; e -= D; expect_same("-=", r, e); }
        { interval r(x), e(x); r *= d; e *= D; expect_same("*=", r, e); }
        { interval r(x), e(x); r /= d; e /= D; expect_same("/=", r, e); }
        { interval r(x), e(x); r %= d; e %= D; expect_same("%=", r, e); }
      }
    }
  }

  // Products of intervals whose bounds are zeros, infinities or small integers:
  // the hull of the products of the bounds, a zero bound times an infinite
  // bound counting as 0, so that [0, 1]*[1, +oo] is [0, +oo] and [0, 0]*[1, +oo]
  // is [0, 0]. The pairs of bounds that are not an interval, [+oo, +oo] among
  // them, are the empty set: the constructors give it there
  void products_with_infinite_bounds()
  {
    const double bounds[] = { -inf, -2., -1., -0., 0., 1., 3., inf };
    std::vector<interval> xs = { interval::emptyset() };
    for (double l : bounds) {
      for (double u : bounds) {
        if (l <= u) {
          xs.push_back(interval(l, u));
        }
      }
    }
    const auto product = [](double a, double b) {
      return ((a == 0.0 && std::isinf(b)) || (b == 0.0 && std::isinf(a))) ? 0.0 : a*b;
    };
    for (const interval& x : xs) {
      for (const interval& y : xs) {
        const bool empty = x.is_empty() || y.is_empty();
        double lo = 0.0, hi = 0.0;
        if (!empty) {
          const double p[] = { product(x.left(), y.left()), product(x.left(), y.right()),
                               product(x.right(), y.left()), product(x.right(), y.right()) };
          lo = *std::min_element(p, p + 4);
          hi = *std::max_element(p, p + 4);
        }
        const auto expect = [&](const std::string& name, const interval& r) {
          check(name + " with zero and infinite bounds",
                empty ? r.is_empty() : !r.is_empty() && r.left() == lo && r.right() == hi,
                [&] { return operands(x, y) + ": " + hex(r); });
        };
        expect("[x]*[y]", x * y);
        { interval r(x); r *= y; expect("[x]*=[y]", r); }
        if (!y.is_empty() && y.left() == y.right()) {
          const double d = y.left();
          expect("[x]*d", x * d);
          expect("d*[x]", d * x);
          { interval r(x); r *= d; expect("[x]*=d", r); }
        }
      }
    }
  }

  // gaol::pow(x, n) for an unsigned n, gaol_uipow(), the pown of IEEE 1788
  // for an unsigned exponent, as for an int n: GAOL declared it public, as
  // uipow(), but defined it GAOL_INLINE with the SSE2 intervals, and a program
  // calling it did not link (GAOL v5)
  void unsigned_powers()
  {
    const double bounds[] = { -inf, -3., -1., -0.5, -0., 0., 0.5, 2., inf };
    std::vector<interval> xs = { interval::emptyset() };
    for (double l : bounds) {
      for (double u : bounds) {
        if (l <= u) {
          xs.push_back(interval(l, u));
        }
      }
    }
    Random random;
    for (int i = 0; i < 200; ++i) {
      const double a = random(-30, 30), b = random(-30, 30);
      xs.push_back(interval(std::min(a, b), std::max(a, b)));
    }
    for (const interval& x : xs) {
      for (unsigned int n = 0; n <= 9; ++n) {
        const interval r = gaol::pow(x, n), p = pow(x, static_cast<int>(n));
        check("gaol::pow([x],n) for an unsigned n as for an int n",
              r.is_empty() ? p.is_empty() : (!p.is_empty() && r.left() == p.left() && r.right() == p.right()),
              [&] { return hex(x) + ", n=" + std::to_string(n) + ": " + hex(r) + " and " + hex(p); });
      }
    }
    const interval one = gaol::pow(interval(-2., 3.), 0u), none = gaol::pow(interval::emptyset(), 0u);
    check("gaol::pow([-2,3],0u) = [1]", !one.is_empty() && one.left() == 1.0 && one.right() == 1.0, [&] { return hex(one); });
    check("gaol::pow(empty,0u): empty", none.is_empty(), [&] { return hex(none); });
  }

  // Negative integer powers whose x^n overflows: x^-n is (1/x)^n there. GAOL
  // computed 1/x^n, and pow([10], -400) was [0, 5.6e-309]
  void negative_powers_at_special_values()
  {
    const double tiny = std::numeric_limits<double>::denorm_min(), largest = std::numeric_limits<double>::max();
    const struct { const char *name; interval x; int n; double lo, hi; } cases[] = {
      { "pow([10],-400)", interval(10.), -400, 0., tiny },
      { "pow([2],-1050)", interval(2.), -1050, 0x1p-1050, 0x1p-1050 },
      { "pow([-2],-1051)", interval(-2.), -1051, -0x1p-1051, -0x1p-1051 },
      { "pow([0.5],-1050)", interval(0.5), -1050, largest, inf },
      { "pow([2,4],-520)", interval(2., 4.), -520, 0x1p-1040, 0x1p-520 },
      { "pow([-2,2],-1050)", interval(-2., 2.), -1050, 0x1p-1050, inf },
      { "pow([-2,3],-1051)", interval(-2., 3.), -1051, -inf, inf },
      { "pow([-10,10],-400)", interval(-10., 10.), -400, 0., inf },
      { "pow([1,+oo],-3)", interval(1., inf), -3, 0., 1. },
      { "pow([-oo,-1],-3)", interval(-inf, -1.), -3, -1., 0. },
      { "pow([-oo,+oo],-2)", interval::universe(), -2, 0., inf },
      { "pow([-3,2],-2)", interval(-3., 2.), -2, 0x1.c71c71c71c71cp-4, inf },
    };
    for (const auto& c : cases) {
      const interval r = pow(c.x, c.n);
      check(std::string(c.name) + ": exact", !r.is_empty() && r.left() == c.lo && r.right() == c.hi,
            [&] { return hex(r); });
    }
  }

  // Integer powers at special values: a bound 0 or infinite, a power below
  // 2^-968 or beyond the largest double, negative powers of intervals with a
  // bound 0, the corners pown_upward() computes one at a time (GAOL v5: they
  // are computed after the one check of the rounding direction of pow(x, n))
  void powers_at_special_values()
  {
    const double tiny = std::numeric_limits<double>::denorm_min(), largest = std::numeric_limits<double>::max();
    const struct { const char *name; interval x; int n; bool empty; double lo, hi; } cases[] = {
      { "pow([0,2],3)", interval(0., 2.), 3, false, 0., 8. },
      { "pow([-0,1],4)", interval(-0., 1.), 4, false, 0., 1. },
      { "pow([-2,0],3)", interval(-2., 0.), 3, false, -8., 0. },
      { "pow([1,+oo],3)", interval(1., inf), 3, false, 1., inf },
      { "pow([-oo,-1],3)", interval(-inf, -1.), 3, false, -inf, -1. },
      { "pow([-oo,2],4)", interval(-inf, 2.), 4, false, 0., inf },
      { "pow([1e200],3)", interval(1e200), 3, false, largest, inf },
      { "pow([2^-400],3)", interval(0x1p-400), 3, false, 0., tiny },
      { "pow([-2^-400],3)", interval(-0x1p-400), 3, false, -tiny, 0. },
      { "pow([2^-330],3)", interval(0x1p-330), 3, false, 0x1p-990, 0x1p-990 },
      { "pow([1.5 2^-330],3)", interval(0x1.8p-330), 3, false, 0x1.bp-989, 0x1.bp-989 },
      { "pow([3 2^-358],3), a subnormal", interval(0x1.8p-357), 3, false, 0x1.bp-1070, 0x1.bp-1070 },
      { "pow([3 2^-360],3), below 2^-1074", interval(0x1.8p-359), 3, false, 0., tiny },
      { "pow([0,+oo],-3)", interval(0., inf), -3, false, 0., inf },
      { "pow([-oo,0],-3)", interval(-inf, 0.), -3, false, -inf, 0. },
      { "pow([0,4],-1)", interval(0., 4.), -1, false, 0.25, inf },
      { "pow([0,4],-2)", interval(0., 4.), -2, false, 0.0625, inf },
      { "pow([-2,0],-3)", interval(-2., 0.), -3, false, -inf, -0.125 },
      { "pow([-0,+0],-1): empty", interval(-0., 0.), -1, true, 0., 0. },
      { "pow([-0,+0],-2): empty", interval(-0., 0.), -2, true, 0., 0. },
      { "pow([-2^-1074],-1)", interval(-tiny), -1, false, -inf, -largest },
      { "pow([2^-600],-2)", interval(0x1p-600), -2, false, largest, inf },
      { "pow([2^600],-2)", interval(0x1p600), -2, false, 0., tiny },
      { "pow([2^500],-2)", interval(0x1p500), -2, false, 0x1p-1000, 0x1p-1000 },
      // Corners beyond the range of the exponentiation at once, whose powers
      // are scaled by powers of two as they are computed, near the ends of
      // the doubles: the tightest bounds (mpmath, 800 bits)
      { "pow([1.0001],7000000)", interval(1.0001), 7000000, false, 0x1.c8fef87df0888p+1009, 0x1.c8fef87df0889p+1009 },
      { "pow([1.0001],-7000000)", interval(1.0001), -7000000, false, 0x1.1ecfe92881a5dp-1010, 0x1.1ecfe92881a5ep-1010 },
      { "pow([0.9999],7000000)", interval(0.9999), 7000000, false, 0x1.0b6c002853dd9p-1010, 0x1.0b6c002853ddap-1010 },
      { "pow([-1.0001],7000001)", interval(-1.0001), 7000001, false, -0x1.c90aab75f708cp+1009, -0x1.c90aab75f708bp+1009 },
      { "pow([1.5],1700)", interval(1.5), 1700, false, 0x1.5a63b3b6d05e3p+994, 0x1.5a63b3b6d05e4p+994 },
      { "pow([1.0001],7100000)", interval(1.0001), 7100000, false, largest, inf },
    };
    for (const auto& c : cases) {
      const interval r = pow(c.x, c.n);
      check(std::string(c.name) + ": exact",
            c.empty ? r.is_empty() : (!r.is_empty() && r.left() == c.lo && r.right() == c.hi),
            [&] { return hex(r); });
    }
  }

  /*
    Integer powers near the ends of the doubles, those of
    powers_at_special_values() at random: a bound 0 or infinite, or whose
    power is above 2^1100, below 2^-1000, or from about 2^-1000 to 2^1000,
    where the corners are computed one at a time, scaled (pown_point() of
    gaol/gaol_interval.cpp). The bounds have to be the tightest, as
    elsewhere: GAOL v5 took the products rounded outward there, up to n + 1
    doubles beyond, and their bounds differed between the SSE2 and the FPU
    intervals for about half of these intervals, which multiplied them from
    either end of n.
  */
  void powers_near_the_ends()
  {
    Random random;
    for (int i = 0; i < 4000; ++i) {
      const unsigned int n = static_cast<unsigned int>(random.integer(2, 60));
      const int m = static_cast<int>(n);
      const int big = std::min(1023, 1100/m + 1), small = -(1000/m) - 2;
      const auto huge = [&] { return random.integer(0, 4) == 0 ? inf : random.positive(big, 1023); };
      // 0, a subnormal (from its bits), or a normal double whose power is
      // below 2^-1000
      const auto tiny = [&] {
        const int kind = random.integer(0, 5);
        if (kind == 0) {
          return 0.0;
        }
        if (kind == 1) {
          const std::uint64_t bits = (random.bits() & 0x000FFFFFFFFFFFFFull) | 1u;
          double d;
          std::memcpy(&d, &bits, sizeof d);
          return d;
        }
        return random.positive(-1022, small);
      };
      const auto any = [&] { return random.positive(-(1000/m), 1000/m); };
      double a = 0.0, b = 0.0;
      switch (random.integer(0, 3)) {
      case 0: // [tiny, x]
        a = tiny(); b = std::max(a, any());
        break;
      case 1: // [-x, huge] or [x, huge]
        b = huge(); a = random.integer(0, 1) == 0 ? -any() : std::min(b, any());
        break;
      case 2: // [x, -tiny] or [-huge, x], x <= 0
        if (random.integer(0, 1) == 0) {
          b = -tiny(); a = std::min(b, -any());
        } else {
          a = -huge(); b = random.integer(0, 1) == 0 ? any() : std::max(a, -any());
        }
        break;
      default: // [-tiny, x]
        a = -tiny(); b = any();
        break;
      }
      const interval X(a, b), r = gaol::pow(X, n);
      // The power of each bound, or its limit, the infinite bounds having
      // infinite powers
      const auto bound_power = [&](double v) { return power(v, m); };
      const bool odd_n = n % 2u == 1u, has_zero = a <= 0.0 && b >= 0.0;
      bool ok = !r.is_empty();
      if (ok && odd_n) {
        ok = (a == -inf ? r.left() == -inf : is_tightest_lower_bound(r.left(), bound_power(a)))
             && (b == inf ? r.right() == inf : is_tightest_upper_bound(r.right(), bound_power(b)));
      } else if (ok) {
        const double mig = has_zero ? 0.0 : std::min(std::fabs(a), std::fabs(b)), mag = std::max(std::fabs(a), std::fabs(b));
        ok = is_tightest_lower_bound(r.left(), bound_power(mig))
             && (mag == inf ? r.right() == inf : is_tightest_upper_bound(r.right(), bound_power(mag)));
      }
      check("pow(x, n) near the ends of the doubles, the tightest", ok,
            [&] { return "pow(" + hex(X) + ", " + std::to_string(n) + ") = " + hex(r); });
    }
  }

  /*
    The powers that are doubles are exact, both bounds (GAOL v5): the rests
    of the exact products are all 0 then, and the error bound is left out
    (pown_products() of gaol/gaol_interval.cpp). Without that, pow([2, 3], 3)
    was [8 - 2^-50, 27 + 2^-48]. x^n of an integer x is a double while it is
    below 2^53, and x^-n is one for a power of two x only.
  */
  void exact_powers()
  {
    const auto expect = [](const interval& x, int n, double lo, double hi) {
      const interval r = pow(x, n);
      check("pow(x, n) exact where x^n is a double", !r.is_empty() && r.left() == lo && r.right() == hi,
            [&] { return "pow(" + hex(x) + ", " + std::to_string(n) + ") = " + hex(r) + " rather than ["
                         + hex(lo) + ", " + hex(hi) + "]"; });
    };
    // p^n computed exactly in doubles, rounded to nearest: false where it is
    // no double
    const auto integer_power = [](double p, int n, double& v) {
      RoundingToNearest nearest;
      v = 1.0;
      for (int i = 0; i < n; ++i) {
        v *= p;
        if (std::fabs(v) >= 9007199254740992.0) { // 2^53
          return false;
        }
      }
      return true;
    };
    for (int p = -40; p <= 40; ++p) {
      for (int k = -4; k <= 4; k += 4) {
        for (int n = 2; n <= 40; ++n) {
          double v = 0.0, w = 0.0;
          if (!integer_power(p, n, v) || !integer_power(p + 1, n, w)) {
            continue;
          }
          const double s = std::ldexp(1.0, k), t = std::ldexp(1.0, k*n);
          expect(interval(p*s), n, v*t, v*t);
          // [p, p + 1] 2^k: x^n is monotone there unless p < 0 < p + 1
          const double vl = v*t, wl = w*t;
          if (n % 2 == 1 || p >= 0) {
            expect(interval(p*s, (p + 1)*s), n, vl, wl);
          } else if (p + 1 <= 0) {
            expect(interval(p*s, (p + 1)*s), n, wl, vl);
          }
        }
      }
    }
    // x^-n of a power of two, and of intervals of them
    for (int k = -60; k <= 60; k += 3) {
      for (int n = 1; n <= 17; ++n) {
        const double x = std::ldexp(1.0, k), v = std::ldexp(1.0, -k*n);
        expect(interval(x), -n, v, v);
        expect(interval(-x), -n, (n % 2 == 1) ? -v : v, (n % 2 == 1) ? -v : v);
        expect(interval(x, 2*x), -n, std::ldexp(v, -n), v);
      }
    }
  }

  /*
    Integer powers for large n: the tightest bounds, from exact products
    within the ints, each of these powers being farther from a double than
    twice the bound K of the error of their products (pown_products() of
    gaol_interval.cpp), and from CORE-MATH's pow beyond them, for an unsigned
    n. The square of the rest carried along was left out of the lower bound
    (GAOL v5, review #7 of examples/examples.md), and the lower bound of x^n was 8
    doubles below the tightest for x = 1.0000001 and n = 2^28 - 1, 557 for
    2^31 - 1, 1962 for 2^32 - 1, more than one double below for most n above
    2^28. The bounds are computed apart, with mpmath:

      mpmath.mp.prec = 2000
      v = mpmath.mpf(x) ** n
      lo, hi = (float(mpmath.mpf(v, prec=53, rounding=r)) for r in 'du')

    and, giving the same doubles, with integers: the n-th power of the
    integer significand of x bounded from below and from above on 300 bits
    after each product. The powers of -x and of intervals of these x are
    checked with them, for an unsigned n and an int n.
  */
  void large_powers()
  {
    struct Case
    {
      double x;
      unsigned int n;
      double lo, hi;
    };
    const Case cases[] = {
      { 1.0000001, 268435455u, 0x1.a7be622958a9dp+38, 0x1.a7be622958a9ep+38 },    // 2^28 - 1
      { 1.0000001, 2147483647u, 0x1.c2d024509add0p+309, 0x1.c2d024509add1p+309 }, // 2^31 - 1
      { 1.0000001, 4294967295u, 0x1.8cf0370d27fd7p+619, 0x1.8cf0370d27fd8p+619 }, // 2^32 - 1
      { 1.0000001, 2147483648u, 0x1.c2d02744f1988p+309, 0x1.c2d02744f1989p+309 }, // 2^31: squares only
      { 1.0000001, 4294967294u, 0x1.8cf034733449bp+619, 0x1.8cf034733449cp+619 },
      { 1.0000001, 3000000019u, 0x1.c05a8eaa7c66dp+432, 0x1.c05a8eaa7c66ep+432 },
      { 1.0000001, 1000000007u, 0x1.349453ea21054p+144, 0x1.349453ea21055p+144 },
      { 1.0000001, 16777217u, 0x1.569d34bfcb832p+2, 0x1.569d34bfcb833p+2 },       // 2^24 + 1
      { 0.9999999, 2147483647u, 0x1.22bd57ce7f5c3p-310, 0x1.22bd57ce7f5c4p-310 },
      { 0.9999999, 4294967295u, 0x1.4a3184d0a48f7p-620, 0x1.4a3184d0a48f8p-620 },
      { 0x1.000001p+0, 4294967295u, 0x1.41c7065bd948cp+369, 0x1.41c7065bd948dp+369 },
      // The power of 2^(1023.5/n) is next to the largest double, the one of
      // 2^(-967/n) next to 2^-968, below which the products are rounded
      { 0x1.000002c56faabp+0, 4294967295u, 0x1.6a09ea8bc37f0p+1023, 0x1.6a09ea8bc37f1p+1023 },
      { 0x1.fffffac37415cp-1, 4294967295u, 0x1.fffffc7d4b348p-968, 0x1.fffffc7d4b349p-968 },
    };
    const auto expect = [](const std::string& name, const interval& x, unsigned int n, const interval& r,
                           double lo, double hi) {
      check("pow(x, n) for large n: " + name, !r.is_empty() && r.left() == lo && r.right() == hi,
            [&] {
              return "pow(" + hex(x) + ", " + std::to_string(n) + ") = " + hex(r) + " rather than ["
                   + hex(lo) + ", " + hex(hi) + "]";
            });
    };
    for (const Case& c : cases) {
      const interval x(c.x), minus_x(-c.x);
      expect("[x], the tightest", x, c.n, gaol::pow(x, c.n), c.lo, c.hi);
      if (c.n <= static_cast<unsigned int>(std::numeric_limits<int>::max())) {
        expect("[x] for an int n, the tightest", x, c.n, pow(x, static_cast<int>(c.n)), c.lo, c.hi);
      }
      if (c.n % 2u == 1u) {
        expect("[-x], an odd n: -[x]^n", minus_x, c.n, gaol::pow(minus_x, c.n), -c.hi, -c.lo);
      } else {
        expect("[-x], an even n: [x]^n", minus_x, c.n, gaol::pow(minus_x, c.n), c.lo, c.hi);
      }
    }
    // Intervals of 0.9999999 and 1.0000001: their bounds are the powers of
    // cases[0..2] and cases[8..9]
    const struct { interval x; unsigned int n; double lo, hi; } intervals[] = {
      { interval(0.9999999, 1.0000001), 2147483647u, cases[8].lo, cases[1].hi },
      { interval(0.9999999, 1.0000001), 4294967295u, cases[9].lo, cases[2].hi },
      { interval(-1.0000001, -0.9999999), 4294967295u, -cases[2].hi, -cases[9].lo },
      { interval(-0.9999999, 1.0000001), 2147483647u, -cases[8].hi, cases[1].hi },
      { interval(-1.0000001, 0.9999999), 4294967294u, 0.0, cases[4].hi },
    };
    for (const auto& c : intervals) {
      expect("intervals, the tightest", c.x, c.n, gaol::pow(c.x, c.n), c.lo, c.hi);
    }
    // x^-n for large n, 1/x^n from h + l corrected to the second order
    // (GAOL v5): its rest r is about n 2^-52, and the terms of r^2 left out
    // made the bounds of pow([1.0000001], -2^31 + 1) 718 doubles away
    const struct { double x; int n; double lo, hi; } negative[] = {
      { 1.0000001, -2147483647, 0x1.22beec72a0774p-310, 0x1.22beec72a0775p-310 },
      { 1.0000001, -2147483647 - 1, 0x1.22beea8ad61c3p-310, 0x1.22beea8ad61c4p-310 },
      { 1.0000001, -1000000007, 0x1.a8c267610691cp-145, 0x1.a8c267610691dp-145 },
      { 0.9999999, -2147483647, 0x1.c2d297bd940f4p+309, 0x1.c2d297bd940f5p+309 },
      { 1.0000001, -16777217, 0x1.7e9085a4ba4adp-3, 0x1.7e9085a4ba4aep-3 },
      { 1.0000001, -268435455, 0x1.3551ae28dc2adp-39, 0x1.3551ae28dc2aep-39 },
    };
    for (const auto& c : negative) {
      const interval x(c.x), r = pow(x, c.n);
      check("pow(x, n) for large negative n: [x], the tightest", !r.is_empty() && r.left() == c.lo && r.right() == c.hi,
            [&] { return "pow(" + hex(x) + ", " + std::to_string(c.n) + ") = " + hex(r); });
    }
  }

  // The n-th roots at special values: the rootn of IEEE 1788-2015 (Table
  // 10.5), defined on R for an odd n and on [0, +oo] for an even n, the root
  // of 0 being 0. GAOL took the roots of the part of [x] in [0, +oo] for every
  // n, and moved the root of 0 one double away
  void roots_at_special_values()
  {
    const interval cube_root_8 = nth_root(interval(8.), 3), cube_root_27 = nth_root(interval(27.), 3);
    const interval r = nth_root(interval(-8., 27.), 3);
    check("nth_root([-8,27],3): [-2,3], from the roots of 8 and 27",
          !r.is_empty() && r.left() <= -2. && r.right() >= 3. && r.left() == -cube_root_8.right()
          && r.right() == cube_root_27.right(),
          [&] { return hex(r); });

    struct Case
    {
      const char *name;
      interval x;
      unsigned int n;
      double lo, hi;
      bool empty, exact;
    };
    const Case cases[] = {
      { "nth_root([0],3)", interval(0.), 3, 0., 0., false, true },
      { "nth_root([1],3)", interval(1.), 3, 1., 1., false, true },
      { "nth_root([-1],5)", interval(-1.), 5, -1., -1., false, true },
      { "nth_root([1],6)", interval(1.), 6, 1., 1., false, true },
      { "nth_root([-0,0],4)", interval(-0., 0.), 4, 0., 0., false, true },
      { "nth_root([-oo,+oo],3)", interval::universe(), 3, -inf, inf, false, true },
      { "nth_root([-oo,+oo],4)", interval::universe(), 4, 0., inf, false, true },
      { "nth_root([0,+oo],5)", interval(0., inf), 5, 0., inf, false, true },
      { "nth_root([-oo,-1],3)", interval(-inf, -1.), 3, -inf, -1., false, false },
      { "nth_root([-4,9],4)", interval(-4., 9.), 4, 0., std::sqrt(3.), false, false },
      { "nth_root([-8,-1],3)", interval(-8., -1.), 3, -2., -1., false, false },
      { "nth_root([-4,-1],4)", interval(-4., -1.), 4, 0., 0., true, false },
      { "nth_root(empty,3)", interval::emptyset(), 3, 0., 0., true, false },
      { "nth_root([-8,27],0)", interval(-8., 27.), 0, 0., 0., true, false },
    };
    // The root of a double that is an n-th power is that double: the power of
    // the root is x exactly, which proves it to be both bounds (few powers:
    // the exact arithmetic of the tests is slow in the Debug builds of Visual
    // C++, where the whole test has an hour)
    for (unsigned int n = 3; n <= 12; ++n) {
      for (const double root : { 2., 3., 7., 10., 1.5, 0.75, 0x1.8p-40, 0x1.4p+25 }) {
        const double x = std::pow(root, static_cast<double>(n));
        if (x > 0. && x < inf && compare(x, power(root, static_cast<int>(n))) == 0) {
          const interval rn = nth_root(interval(x), n), minus_rn = nth_root(interval(-x), n);
          const auto describe = [&] { return "x=" + hex(x) + " n=" + std::to_string(n) + ": " + hex(rn); };
          check("nth_root([a^n],n): [a]", !rn.is_empty() && rn.left() == root && rn.right() == root, describe);
          if (n % 2 == 1) {
            check("nth_root([-a^n],n) for an odd n: [-a]", !minus_rn.is_empty() && minus_rn.left() == -root && minus_rn.right() == -root,
                  describe);
          }
        }
      }
    }
    // The extreme doubles, and large n, where the power with a rounded
    // exponent was the furthest from the root
    for (const double x : { std::numeric_limits<double>::max(), std::numeric_limits<double>::min(),
                            std::numeric_limits<double>::denorm_min(), 3*std::numeric_limits<double>::denorm_min(),
                            0x1.fffffffffffffp-1, 0x1.0000000000001p+0, 10., 1e300, 1e-300 }) {
      for (const unsigned int n : { 3u, 4u, 17u }) {
        expect_root("nth_root([x],n) at extreme doubles and large n", nth_root(interval(x), n), x, static_cast<int>(n), root_limit);
      }
    }

    for (const Case& c : cases) {
      const interval x = nth_root(c.x, c.n);
      const auto describe = [&] { return hex(x); };
      if (c.empty) {
        check(std::string(c.name) + ": empty", x.is_empty(), describe);
      } else if (c.exact) {
        check(std::string(c.name) + ": exact", !x.is_empty() && x.left() == c.lo && x.right() == c.hi, describe);
      } else {
        // An enclosure, within 4 doubles at the scale of these values
        const double slack = 4 * std::ldexp(1.0, -52);
        check(std::string(c.name) + ": encloses, within 4 doubles",
              !x.is_empty() && x.left() <= c.lo && x.right() >= c.hi
              && x.left() >= c.lo - slack * std::max(1.0, std::fabs(c.lo))
              && x.right() <= c.hi + slack * std::max(1.0, std::fabs(c.hi)), describe);
      }
    }
  }

  /*
    fma, cancel_minus and cancel_plus of IEEE 1788-2015 (GAOL v5), which are
    required to be the tightest.

    The bounds of fma are compared with std::fma called in the downward and
    the upward rounding directly, apart from the negations GAOL rounds with:
    GCC folded -fma(-a, b, -c) into fma(a, b, c), a single instruction rounded
    upward, and the lower bound came one double above the exact one. cancel
    is checked on intervals of the same width, which rounding the differences
    of the bounds cannot tell from others, and on intervals one double wider.
  */
  void fused_and_cancellative()
  {
    Random random;
    const auto corner = [](double a, double b, double c, int direction) {
      if (a == 0.0 || b == 0.0) {
        return c;  // 0 times an infinite bound counts as 0
      }
      // rnd_keep() before the rounding direction changes back: GCC moved
      // std::fma, a pure function to it, after the fesetround() that follows,
      // and the lower bounds were rounded upward (the continuous integration,
      // armhf and s390x)
      std::fesetround(direction);
      const double v = gaol::rnd_keep(std::fma(a, b, c));
      std::fesetround(FE_UPWARD);
      return v;
    };
    for (int i = 0; i < 100000; ++i) {
      const interval X = hull(random.any(), random.any());
      const interval Y = hull(random.any(), random.any());
      const interval Z = hull(random.any(), random.any());
      const interval got = fma(X, Y, Z);
      double lo = -GAOL_INFINITY, hi = GAOL_INFINITY;
      if (Z.left() != -GAOL_INFINITY) {
        lo = GAOL_INFINITY;
        for (double a : {X.left(), X.right()})
          for (double b : {Y.left(), Y.right()})
            lo = std::min(lo, corner(a, b, Z.left(), FE_DOWNWARD));
      }
      if (Z.right() != GAOL_INFINITY) {
        hi = -GAOL_INFINITY;
        for (double a : {X.left(), X.right()})
          for (double b : {Y.left(), Y.right()})
            hi = std::max(hi, corner(a, b, Z.right(), FE_UPWARD));
      }
      check("fma: the tightest bounds, each rounded once", got.left() == lo && got.right() == hi,
            [&] {
              return "fma(" + hex(X) + ", " + hex(Y) + ", " + hex(Z) + ") = " + hex(got)
                   + " rather than [" + hex(lo) + ", " + hex(hi) + "]";
            });
    }
    check("fma with an empty argument: empty",
          fma(interval::emptyset(), interval(1.0), interval(1.0)).is_empty(), [] { return std::string(); });
    check("fma([0, 1], [0, +oo], [2]) = [2, +oo]: 0 times +oo counts as 0",
          fma(interval(0.0, 1.0), interval(0.0, GAOL_INFINITY), interval(2.0)).left() == 2.0,
          [] { return std::string(); });

    for (int i = 0; i < 100000; ++i) {
      const interval X = hull(random.any(), random.any());
      const double w = X.right() - X.left();   // the width rounded upward
      const double b = random.any();
      // a Y of the width of X rounded, then one double narrower and one wider
      for (int shift = -1; shift <= 1; ++shift) {
        double u = b + w;
        if (shift < 0) u = std::nextafter(u, -GAOL_INFINITY);
        if (shift > 0) u = std::nextafter(u, GAOL_INFINITY);
        if (!(b <= u) || !std::isfinite(u) || !X.is_finite()) {
          continue;
        }
        const interval Y(b, u);
        const interval got = cancel_minus(X, Y);
        // defined when X is at least as wide as Y: in exact arithmetic
        // X.right() - X.left() >= u - b, which Y + result must then contain
        if (got.is_entire()) {
          continue;  // no value at Level 1, which the next check does not see
        }
        const interval sum = Y + got;
        check("cancel_minus: Y + result encloses X", sum.set_contains(X),
              [&] { return "cancel_minus(" + hex(X) + ", " + hex(Y) + ") = " + hex(got); });
        // the tightest bounds: the differences of the bounds rounded in the
        // two directed roundings directly, the lower one through rnd_keep()
        // before the rounding direction changes back: Clang 14 moved the
        // subtraction after the fesetround() that follows, and rounded it
        // upward (the continuous integration, Ubuntu 22.04)
        std::fesetround(FE_DOWNWARD);
        const double lo = gaol::rnd_keep(X.left() - Y.left());
        std::fesetround(FE_UPWARD);
        const double hi = X.right() - Y.right();
        check("cancel_minus: the tightest bounds", got.left() == lo && got.right() == hi,
              [&] {
                return "cancel_minus(" + hex(X) + ", " + hex(Y) + ") = " + hex(got)
                     + " rather than [" + hex(lo) + ", " + hex(hi) + "]";
              });
      }
    }
    check("cancel_minus(empty, [1, 2]) = empty",
          cancel_minus(interval::emptyset(), interval(1.0, 2.0)).is_empty(), [] { return std::string(); });
    check("cancel_minus([1, 2], empty) = entire",
          cancel_minus(interval(1.0, 2.0), interval::emptyset()).is_entire(), [] { return std::string(); });
    check("cancel_minus([1, +oo], [1, 2]) = entire",
          cancel_minus(interval(1.0, GAOL_INFINITY), interval(1.0, 2.0)).is_entire(), [] { return std::string(); });
    check("cancel_minus([1, 2], [0, 5]) = entire: X narrower than Y",
          cancel_minus(interval(1.0, 2.0), interval(0.0, 5.0)).is_entire(), [] { return std::string(); });
    check("cancel_minus([1, 5], [0, 2]) = [1, 3]",
          cancel_minus(interval(1.0, 5.0), interval(0.0, 2.0)).set_eq(interval(1.0, 3.0)), [] { return std::string(); });
    check("cancel_plus([1, 5], [0, 2]) = cancel_minus([1, 5], [-2, 0]) = [3, 5]",
          cancel_plus(interval(1.0, 5.0), interval(0.0, 2.0)).set_eq(interval(3.0, 5.0)), [] { return std::string(); });

    /* The differences of the bounds that tell whether X is at least as wide as
       Y are computed by TwoSum in the rounding to nearest, their terms kept
       before the direction changes back: GCC computed them after it, upward,
       where TwoSum is no longer exact, and [0.5] looked as wide as
       [4.9e-324, 1e-300], whose rounded differences of the bounds are the
       same; with a bound of DBL_MAX, the first sum read +oo and the terms of
       the error inf - inf, which raised FE_INVALID (GAOL v5, point 48, the
       flag checked in tests/rounding_direction.cpp) */
    check("cancel_minus([0.5], [4.9e-324, 1e-300]) = entire: X narrower than Y, which the rounded differences do not show",
          cancel_minus(interval(0.5), interval(4.9e-324, 1e-300)).is_entire(),
          [&] { return hex(cancel_minus(interval(0.5), interval(4.9e-324, 1e-300))); });
    const double largest = (std::numeric_limits<double>::max)();
    check("cancel_minus([DBL_MAX], [0.5]) = [DBL_MAX one double below, DBL_MAX]",
          cancel_minus(interval(largest), interval(0.5)).set_eq(interval(std::nextafter(largest, 0.0), largest)),
          [&] { return hex(cancel_minus(interval(largest), interval(0.5))); });
    check("cancel_plus([DBL_MAX], [-0.5]) is the same hull",
          cancel_plus(interval(largest), interval(-0.5)).set_eq(interval(std::nextafter(largest, 0.0), largest)),
          [&] { return hex(cancel_plus(interval(largest), interval(-0.5))); });
  }
}

int main()
{
  gaol::init();
  Random random;
  operations_on_doubles("exponents from -30 to 30", [&] { return random(-30, 30); }, 16);
  operations_on_doubles("any doubles", [&] { return random.any(); }, 512);
  operations_on_intervals("exponents from -30 to 30", [&] { return random(-30, 30); });
  operations_on_intervals("any doubles", [&] { return random.any(); });
  divisions_by_zero();
  operations_with_special_doubles();
  products_with_infinite_bounds();
  unsigned_powers();
  negative_powers_at_special_values();
  powers_at_special_values();
  powers_near_the_ends();
  exact_powers();
  large_powers();
  roots_at_special_values();
  fused_and_cancellative();
  const int status = summary();
  gaol::cleanup();
  return status;
}
