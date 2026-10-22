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
 * gaol/gaol_interval_core.cpp); n-th roots, which GAOL computes in several
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
 * Those of numbers below 2^-1020, subnormal ones included, are within 2 doubles
 * of the tightest bounds, and exact where x is an n-th power, for n up to
 * 2^32 - 1 (GAOL v5, point AC).
 * All the intervals made of 15 special bounds (zeros of both signs, infinities,
 * the smallest subnormal and the smallest normal numbers, the largest double...)
 * are operated on with +, -, *, /, %, sqr(), sqrt(), inverse(), the compound
 * forms and the forms with a double (GAOL v5, special_bounds_grid()): the
 * bounds are those of a reference of plain doubles rounded downward and
 * upward by the hardware, whatever the rounding direction of the calling code.
 * The same operations on literals the compiler sees, which no compiler may
 * fold in the rounding to nearest, are the tightest, and lines of 8 to 16
 * operations, with operands used twice, have the bounds of the reference
 * evaluating them one operation after the other.
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
  // that some a in [x] is x*b for some b in [y] (x%y). The tables are plain
  // doubles, which special_bounds_grid() compares its reference with too
  // (GAOL v5)
  struct ZeroDivisionCase
  {
    double xl, xu, yl, yu;
    double lo, hi;
    bool empty;
  };

  const ZeroDivisionCase zero_divisions[] = {
    { 1., 2., 0., 1., 1., inf, false },
    { 1., 2., -1., 0., -inf, -1., false },
    { -2., -1., 0., 1., -inf, -1., false },
    { -2., -1., -1., 0., 1., inf, false },
    { 1., 2., -1., 1., -inf, inf, false },
    { -1., 2., 0., 1., -inf, inf, false },
    { 0., 2., 0., 1., 0., inf, false },
    { -2., 0., 0., 1., -inf, 0., false },
    { 0., 0., -1., 1., 0., 0., false },
    { 1., 2., 0., 0., 0., 0., true },
  };

  const ZeroDivisionCase zero_relational_divisions[] = {
    { 1., 2., 0., 1., 1., inf, false },
    { 1., 2., -1., 0., -inf, -1., false },
    { 1., 2., -1., 1., -inf, inf, false },
    { 0., 0., -1., 1., -inf, inf, false },
    { -1., 2., 0., 0., -inf, inf, false },
    { 1., 2., 0., 0., 0., 0., true },
  };

  void divisions_by_zero()
  {
    for (const auto& c : zero_divisions) {
      const interval x(c.xl, c.xu), y(c.yl, c.yu);
      const interval r = x / y;
      const auto describe = [&] { return operands(x, y) + ": " + hex(r); };
      if (c.empty) {
        check("[x]/[y] for [y] containing 0: empty", r.is_empty(), describe);
      } else {
        check("[x]/[y] for [y] containing 0: encloses", !r.is_empty() && r.left() <= c.lo && r.right() >= c.hi, describe);
        check("[x]/[y] for [y] containing 0: tightest", r.left() == c.lo && r.right() == c.hi, describe);
      }
    }
    for (const auto& c : zero_relational_divisions) {
      const interval x(c.xl, c.xu), y(c.yl, c.yu);
      const interval r = x % y;
      const auto describe = [&] { return operands(x, y) + ": " + hex(r); };
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

  // Quotients of intervals whose bounds are zeros of both signs, infinities,
  // small integers, subnormal and large doubles, by intervals with no zero:
  // the tightest enclosure of the quotients of the bounds, which the division
  // of the SSE2 intervals takes from the signs of both operands without a
  // branch (GAOL v5, point ZZ of TODO.md). An infinite bound over a finite
  // one is an infinite bound, a finite one over an infinite one is 0, and
  // the quotient of two infinite bounds, which the limits of the quotients
  // leave anywhere between the others, counts for none (the divisors with a
  // zero are the cases of divisions_by_zero())
  void quotients_at_special_values()
  {
    const double subnormal = std::numeric_limits<double>::denorm_min(), largest = std::numeric_limits<double>::max();
    const double dividends[] = { -inf, -largest, -2., -1., -subnormal, -0., 0., subnormal, 1., 3., largest, inf };
    const double divisors[] = { -inf, -largest, -3., -1., -subnormal, subnormal, 1., 2., largest, inf };
    for (double xl : dividends) {
      for (double xh : dividends) {
        if (xl > xh || (xl == xh && std::isinf(xl))) {
          continue;
        }
        for (double yl : divisors) {
          for (double yh : divisors) {
            if (yl > yh || (yl < 0.0) != (yh < 0.0) || (yl == yh && std::isinf(yl))) {
              continue;
            }
            const interval X(xl, xh), Y(yl, yh);
            std::vector<Exact> finite;
            bool minus_inf = false, plus_inf = false;
            for (double a : { xl, xh }) {
              for (double b : { yl, yh }) {
                if (std::isinf(a) && std::isinf(b)) {
                  continue;
                }
                if (std::isinf(a)) {
                  ((a < 0.0) == (b < 0.0) ? plus_inf : minus_inf) = true;
                } else if (std::isinf(b)) {
                  finite.push_back(exact(0.0));
                } else {
                  finite.push_back(quotient(dyadic(a), dyadic(b)));
                }
              }
            }
            const auto check_quotient = [&](const std::string& name, const interval& r) {
              const auto describe = [&] { return operands(X, Y) + ": " + hex(r); };
              check(name + " at special values: the lower bound",
                    !r.is_empty() && (minus_inf ? r.left() == -inf : is_tightest_lower_bound(r.left(), min(finite))), describe);
              check(name + " at special values: the upper bound",
                    !r.is_empty() && (plus_inf ? r.right() == inf : is_tightest_upper_bound(r.right(), max(finite))), describe);
            };
            check_quotient("[x]/[y]", X / Y);
            { interval r(X); r /= Y; check_quotient("[x]/=[y]", r); }
            check_quotient("[x]%[y]", X % Y);
          }
        }
      }
    }
  }

  // The rational operations that the lines below are written with
  Exact operator-(const Exact& a, const Exact& b)
  {
    return a + (-b);
  }

  // b is not 0
  Exact operator/(const Exact& a, const Exact& b)
  {
    return quotient(a.num*b.den, a.den*b.num);
  }

  /*
    Lines of + - * / alone between eight operands, written once for the
    intervals and for the rational numbers: a, b, s1 to s4 of either sign, p
    and e positive, the divisors (GAOL v5, point ZZ of TODO.md: the same lines
    as the benchmark of doc/compare)
  */
  template<class T> T sum16(const T& a, const T& b, const T& p, const T& e, const T& s1, const T& s2, const T& s3, const T& s4)
  {
    return a + b + p + e + s1 + s2 + s3 + s4 + a + b + p + e + s1 + s2 + s3 + s4;
  }
  template<class T> T diff16(const T& a, const T& b, const T& p, const T& e, const T& s1, const T& s2, const T& s3, const T& s4)
  {
    return a - b + p - e + s1 - s2 + s3 - s4 - a + b - p + e - s1 + s2 - s3 + s4;
  }
  template<class T> T mul8(const T& a, const T& b, const T& p, const T& e, const T& s1, const T& s2, const T& s3, const T& s4)
  {
    return a * b * p * e * s1 * s2 * s3 * s4;
  }
  template<class T> T mul8_tree(const T& a, const T& b, const T& p, const T& e, const T& s1, const T& s2, const T& s3, const T& s4)
  {
    return ((a * b) * (p * e)) * ((s1 * s2) * (s3 * s4));
  }
  template<class T> T quot8(const T& a, const T& b, const T& p, const T& e, const T& s1, const T& s2, const T& s3, const T& s4)
  {
    (void)b; (void)s1; (void)s2; (void)s3; (void)s4;
    return a / p / e / p / e / p / e / p;
  }
  template<class T> T mix13(const T& a, const T& b, const T& p, const T& e, const T& s1, const T& s2, const T& s3, const T& s4)
  {
    return ((((((a + b) * p - e) * s1 + s2) * p - s3) / e + a) * b - s4) / p + e;
  }
  template<class T> T mix_tree(const T& a, const T& b, const T& p, const T& e, const T& s1, const T& s2, const T& s3, const T& s4)
  {
    return ((a + b) * (p - e) + (s1 - s2) * (s3 + s4)) / (p + e)
           + ((a - b) * (s1 + s2) - (p * e) / (p + e)) / (e + p);
  }

  // The lines of operators on random intervals enclose the line of the
  // rational numbers computed at a bound of each operand, and the operators
  // and the compound assignments give the same bounds, line by line
  void long_lines(Random& random)
  {
    for (int i = 0; i < 300; ++i) {
      interval operands[8];
      double points[8];
      for (int k = 0; k < 8; ++k) {
        const bool positive = (k == 2 || k == 3);
        const double u = positive ? random.positive(-8, 8) : random(-8, 8);
        const double v = positive ? random.positive(-8, 8) : random(-8, 8);
        operands[k] = interval(std::min(u, v), std::max(u, v));
        points[k] = (random.bits() & 1u) ? operands[k].left() : operands[k].right();
      }
      Exact exact_points[8];
      for (int k = 0; k < 8; ++k) {
        exact_points[k] = exact(points[k]);
      }
      const auto describe = [&] {
        std::string text;
        for (int k = 0; k < 8; ++k) {
          text += (k ? " " : "") + hex(operands[k]);
        }
        return text;
      };
#define LONG_LINE(NAME)                                                                                  \
      {                                                                                                  \
        const interval r = NAME<interval>(operands[0], operands[1], operands[2], operands[3],            \
                                          operands[4], operands[5], operands[6], operands[7]);           \
        const Exact v = NAME<Exact>(exact_points[0], exact_points[1], exact_points[2], exact_points[3],  \
                                    exact_points[4], exact_points[5], exact_points[6], exact_points[7]); \
        check(#NAME ": encloses the line at a bound of each operand",                                    \
              !r.is_empty() && compare(r.left(), v) <= 0 && compare(r.right(), v) >= 0, describe);       \
      }
      LONG_LINE(sum16)
      LONG_LINE(diff16)
      LONG_LINE(mul8)
      LONG_LINE(mul8_tree)
      LONG_LINE(quot8)
      LONG_LINE(mix13)
      LONG_LINE(mix_tree)
#undef LONG_LINE
      // The same additions and products with += and *=, the quotients with /=
      {
        const interval *o = operands;
        interval s(o[0]), d(o[0]), m(o[0]), q(o[0]);
        for (int k : { 1, 2, 3, 4, 5, 6, 7, 0, 1, 2, 3, 4, 5, 6, 7 }) {
          s += o[k];
        }
        d -= o[1]; d += o[2]; d -= o[3]; d += o[4]; d -= o[5]; d += o[6]; d -= o[7];
        d -= o[0]; d += o[1]; d -= o[2]; d += o[3]; d -= o[4]; d += o[5]; d -= o[6]; d += o[7];
        for (int k : { 1, 2, 3, 4, 5, 6, 7 }) {
          m *= o[k];
        }
        q /= o[2]; q /= o[3]; q /= o[2]; q /= o[3]; q /= o[2]; q /= o[3]; q /= o[2];
        const auto same = [](const interval& x, const interval& y) {
          return !x.is_empty() && !y.is_empty() && x.left() == y.left() && x.right() == y.right();
        };
        check("a line of + with +=: the bounds of the operators", same(s, sum16<interval>(o[0], o[1], o[2], o[3], o[4], o[5], o[6], o[7])), describe);
        check("a line of + and - with += and -=: the bounds of the operators", same(d, diff16<interval>(o[0], o[1], o[2], o[3], o[4], o[5], o[6], o[7])), describe);
        check("a line of * with *=: the bounds of the operators", same(m, mul8<interval>(o[0], o[1], o[2], o[3], o[4], o[5], o[6], o[7])), describe);
        check("a line of / with /=: the bounds of the operators", same(q, quot8<interval>(o[0], o[1], o[2], o[3], o[4], o[5], o[6], o[7])), describe);
      }
    }
  }

  /*
    The operations at special bounds, against a reference of plain doubles
    (GAOL v5). The reference is the definition of each operation computed on
    the bounds with the hardware, in the two directed roundings: the lower
    bound is the least of the corners rounded downward, the upper one the
    greatest rounded upward, and no case of the signs of the bounds is told
    apart, as the kernels of the SSE2 and NEON intervals do. A product counts
    a zero bound times an infinite one for 0. A quotient by an interval that
    holds 0 is the one by the parts of [y] on either side of 0, where the
    bound 0 of a part is the limit 0- or 0+, a/0+ being an infinity of the
    sign of a, and the quotient of two infinite bounds counts for none (the
    conventions of quotients_at_special_values()). x%y is that quotient, and
    the whole line where both x and y hold 0. The tables of
    divisions_by_zero() are checked against this reference first, so that a
    wrong reference cannot pass for a right result.
  */
  struct Range
  {
    bool empty;
    double lo, hi;
  };

  const Range no_range = { true, 0.0, 0.0 };

  // Not hex(), which a declaration here would hide from the code below
  std::string text(const Range& r)
  {
    return r.empty ? std::string("empty") : "[" + hex(r.lo) + ", " + hex(r.hi) + "]";
  }

  enum Operation { addition, subtraction, multiplication, division, square_root };

  // a op b, in plain doubles rounded in the direction given (b is not used by
  // the square root), the direction upward set again afterwards. The operands
  // are read from volatile memory once the direction is set, and the result is
  // written to it before the direction changes back: no compiler can fold the
  // operation in the rounding to nearest, or compute it elsewhere (GCC moved
  // operations across the changes of direction, see fused_and_cancellative())
  double rounded(Operation op, double a, double b, int direction)
  {
    volatile double operand_a = a, operand_b = b, result = 0.0;
    std::fesetround(direction);
    const double x = operand_a, y = operand_b;
    switch (op) {
    case addition:       result = x + y; break;
    case subtraction:    result = x - y; break;
    case multiplication: result = x * y; break;
    case division:       result = x / y; break;
    default:             result = std::sqrt(x); break;
    }
    const double kept = result;
    std::fesetround(FE_UPWARD);
    return kept;
  }

  // The least and the greatest of the corners, each rounded in its own direction
  struct Corners
  {
    double lo = inf, hi = -inf;
    bool any = false, invalid = false;

    void value(double v)
    {
      invalid = invalid || std::isnan(v);
      lo = std::min(lo, v);
      hi = std::max(hi, v);
      any = true;
    }

    void operation(Operation op, double a, double b)
    {
      const double down = rounded(op, a, b, FE_DOWNWARD), up = rounded(op, a, b, FE_UPWARD);
      invalid = invalid || std::isnan(down) || std::isnan(up);
      lo = std::min(lo, down);
      hi = std::max(hi, up);
      any = true;
    }

    // A NaN bound, which no check passes, where a corner was one
    Range range() const
    {
      const double nan = std::numeric_limits<double>::quiet_NaN();
      return invalid ? Range{ false, nan, nan } : any ? Range{ false, lo, hi } : no_range;
    }
  };

  Range sum_range(const Range& x, const Range& y)
  {
    if (x.empty || y.empty) {
      return no_range;
    }
    return { false, rounded(addition, x.lo, y.lo, FE_DOWNWARD), rounded(addition, x.hi, y.hi, FE_UPWARD) };
  }

  Range difference_range(const Range& x, const Range& y)
  {
    if (x.empty || y.empty) {
      return no_range;
    }
    return { false, rounded(subtraction, x.lo, y.hi, FE_DOWNWARD), rounded(subtraction, x.hi, y.lo, FE_UPWARD) };
  }

  Range product_range(const Range& x, const Range& y)
  {
    if (x.empty || y.empty) {
      return no_range;
    }
    Corners c;
    for (double a : { x.lo, x.hi }) {
      for (double b : { y.lo, y.hi }) {
        if (a == 0.0 || b == 0.0) {
          c.value(0.0); // 0 times an infinite bound counts as 0
        } else {
          c.operation(multiplication, a, b);
        }
      }
    }
    return c.range();
  }

  Range quotient_range(const Range& x, const Range& y)
  {
    if (x.empty || y.empty || (y.lo == 0.0 && y.hi == 0.0)) {
      return no_range;
    }
    Corners c;
    // The quotients of the bounds of x by those of a part of [y] with no 0
    const auto part = [&](double lo, double hi) {
      for (double a : { x.lo, x.hi }) {
        for (double b : { lo, hi }) {
          if (std::isinf(a) && std::isinf(b)) {
            continue;
          }
          if (a == 0.0) {
            c.value(0.0);
          } else {
            c.operation(division, a, b);
          }
        }
      }
    };
    if (y.lo > 0.0 || y.hi < 0.0) {
      part(y.lo, y.hi);
    } else {
      if (y.lo < 0.0) {
        part(y.lo, -0.0);
      }
      if (y.hi > 0.0) {
        part(0.0, y.hi);
      }
    }
    return c.range();
  }

  Range relational_quotient_range(const Range& x, const Range& y)
  {
    const bool x_holds_zero = !x.empty && x.lo <= 0.0 && x.hi >= 0.0, y_holds_zero = !y.empty && y.lo <= 0.0 && y.hi >= 0.0;
    return (x_holds_zero && y_holds_zero) ? Range{ false, -inf, inf } : quotient_range(x, y);
  }

  Range inverse_range(const Range& x)
  {
    return quotient_range(Range{ false, 1.0, 1.0 }, x);
  }

  Range square_range(const Range& x)
  {
    if (x.empty) {
      return no_range;
    }
    Corners c;
    c.operation(multiplication, x.lo, x.lo);
    c.operation(multiplication, x.hi, x.hi);
    const Range r = c.range();
    return { false, (x.lo <= 0.0 && x.hi >= 0.0) ? 0.0 : r.lo, r.hi };
  }

  /*
    The square root of x >= 0 rounded downward and upward, from the root
    rounded to nearest, which every sqrt() gives right, and the sign of its
    square less x, computed exactly (Veltkamp's splitting and Dekker's product,
    in the rounding to nearest, no fused multiply-add: that of mingw-w64 is
    wrong): the sqrt() of the processors that compute in the direction set,
    called in it, gave the root rounded to nearest on 32-bit x86 with Visual
    C++ and MinGW-w64, which the continuous integration showed. A tiny x is
    scaled by an even power of two, and so is a huge one, so that the square of
    the root and its error neither underflow nor overflow (GAOL v5)
  */
  void root_bounds(double x, double& down, double& up)
  {
    std::fesetround(FE_TONEAREST);
    volatile double operand = x;
    int scale = 0;
    double v = operand;
    if (v < std::ldexp(1.0, -900) && v > 0.0) {
      v = std::ldexp(v, 1000);
      scale = -500;
    } else if (v > std::ldexp(1.0, 900) && !std::isinf(v)) { // the square of the root would overflow
      v = std::ldexp(v, -1000);
      scale = 500;
    }
    volatile double root_kept = std::sqrt(v);
    const double r = root_kept;
    if (std::isinf(r) || r == 0.0) {
      down = up = r;
    } else {
      volatile double product = r*r, c = 134217729.0*r;
      const double rh = c - (c - r), rl = r - rh;
      volatile double e1 = ((rh*rh - product) + 2.0*rh*rl) + rl*rl; // r*r = product + e1 exactly
      volatile double d1 = (product - v) + e1;                     // the sign of r*r - v
      const double d = d1;
      if (d == 0.0) {
        down = up = r;
      } else if (d > 0.0) {
        up = r;
        down = std::nextafter(r, 0.0);
      } else {
        down = r;
        up = std::nextafter(r, inf);
      }
    }
    if (scale != 0) {
      down = std::ldexp(down, scale);
      up = std::ldexp(up, scale);
    }
    std::fesetround(FE_UPWARD);
  }

  Range square_root_range(const Range& x)
  {
    if (x.empty || x.hi < 0.0) {
      return no_range;
    }
    double lo_down, lo_up, hi_down, hi_up;
    root_bounds(std::max(x.lo, 0.0), lo_down, lo_up);
    root_bounds(x.hi, hi_down, hi_up);
    return { false, lo_down, hi_up };
  }

  // A range of the reference with the operators of the lines above, so that
  // the lines written for the intervals are evaluated by the reference too,
  // one operation after the other
  struct Reference
  {
    Range range;
  };

  Reference operator+(const Reference& a, const Reference& b) { return { sum_range(a.range, b.range) }; }
  Reference operator-(const Reference& a, const Reference& b) { return { difference_range(a.range, b.range) }; }
  Reference operator*(const Reference& a, const Reference& b) { return { product_range(a.range, b.range) }; }
  Reference operator/(const Reference& a, const Reference& b) { return { quotient_range(a.range, b.range) }; }
  Reference operator%(const Reference& a, const Reference& b) { return { relational_quotient_range(a.range, b.range) }; }
  Reference sqr(const Reference& a) { return { square_range(a.range) }; }
  Reference& operator+=(Reference& a, const Reference& b) { a = a + b; return a; }
  Reference& operator-=(Reference& a, const Reference& b) { a = a - b; return a; }
  Reference& operator*=(Reference& a, const Reference& b) { a = a * b; return a; }
  Reference& operator/=(Reference& a, const Reference& b) { a = a / b; return a; }
  Reference& operator%=(Reference& a, const Reference& b) { a = a % b; return a; }

  // Whether the interval is the range, the empty set for the empty range
  bool is_range(const interval& r, const Range& e)
  {
    return e.empty ? r.is_empty() : (!r.is_empty() && r.left() == e.lo && r.right() == e.hi);
  }

  // An interval and its bounds, kept apart from it: the operands of the
  // reference are never read from the interval under test
  struct Box
  {
    Range range;
    interval x;
  };

  Box box(double l, double u)
  {
    return { { false, l, u }, interval(l, u) };
  }

  // A degenerate interval of a finite double d, which the operators with a double take
  bool is_point(const Range& r)
  {
    return !r.empty && r.lo == r.hi && std::isfinite(r.lo);
  }

  // The direction of the calling code, set before each operation under test
  struct CallerDirection
  {
    const char *name;
    int fenv;
  };

  const CallerDirection caller_directions[] = {
    { "upward", FE_UPWARD }, { "to nearest", FE_TONEAREST }, { "downward", FE_DOWNWARD }, { "toward zero", FE_TOWARDZERO }
  };
  const std::size_t nb_caller_directions = sizeof(caller_directions)/sizeof(caller_directions[0]);

  // The rounding direction found, set again at the end of a test of this section
  struct RestoreDirection
  {
    const int saved = std::fegetround();
    ~RestoreDirection() { std::fesetround(saved); }
  };

  // f(), the direction of the calling code being set just before
  template<class F>
  interval under(int direction, const F& f)
  {
    std::fesetround(direction);
    return f();
  }

  // The names of the checks of a form of an operation, made once
  struct Form
  {
    std::string bounds, not_empty;
  };

  Form form(const std::string& name)
  {
    return { "special bounds: " + name, "special bounds: " + name + ": no unexpected empty set" };
  }

  // The result of the form is the range, and not the empty set where the range is not
  template<class Context>
  void expect_range(const Form& f, const interval& r, const Range& e, const Context& context)
  {
    const auto describe = [&] { return std::string(context()) + ": " + (r.is_empty() ? std::string("empty") : hex(r)) + " rather than " + text(e); };
    check(f.not_empty, e.empty || !r.is_empty(), describe);
    check(f.bounds, is_range(r, e), describe);
  }

  // The intervals [a, b] with a <= b of the special bounds that interval(a, b) accepts, the
  // empty set and the universe
  std::vector<Box> special_boxes()
  {
    const double subnormal = std::numeric_limits<double>::denorm_min(), smallest_normal = std::numeric_limits<double>::min();
    const double largest = std::numeric_limits<double>::max();
    // Read from volatile memory: no compiler knows the values of the grid
    const volatile double special[] = { -inf, -largest, -2.5, -1., -smallest_normal, -subnormal, -0., 0., subnormal,
                                        smallest_normal, 0.1, 1., 3., largest, inf };
    std::vector<double> bounds;
    for (const volatile double& b : special) {
      const double v = b;
      bounds.push_back(v);
    }
    std::vector<Box> boxes = { { no_range, interval::emptyset() }, { { false, -inf, inf }, interval::universe() } };
    for (double l : bounds) {
      for (double u : bounds) {
        if (l <= u) {
          const Box b = box(l, u);
          // [+oo, +oo] and [-oo, -oo] are no intervals
          const bool accepted = l < inf && u > -inf;
          check("special bounds: interval(a, b) is empty for a = +oo or b = -oo only", b.x.is_empty() == !accepted,
                [&] { return "a=" + hex(l) + " b=" + hex(u) + ": " + hex(b.x); });
          if (accepted) {
            boxes.push_back(b);
          }
        }
      }
    }
    return boxes;
  }

  struct Plus
  {
    static std::string symbol() { return "+"; }
    static Range reference(const Range& x, const Range& y) { return sum_range(x, y); }
    static interval apply(const interval& x, const interval& y) { return x + y; }
    static interval apply(const interval& x, double d) { return x + d; }
    static interval apply(double d, const interval& y) { return d + y; }
    static void assign(interval& x, const interval& y) { x += y; }
    static void assign(interval& x, double d) { x += d; }
  };

  struct Minus
  {
    static std::string symbol() { return "-"; }
    static Range reference(const Range& x, const Range& y) { return difference_range(x, y); }
    static interval apply(const interval& x, const interval& y) { return x - y; }
    static interval apply(const interval& x, double d) { return x - d; }
    static interval apply(double d, const interval& y) { return d - y; }
    static void assign(interval& x, const interval& y) { x -= y; }
    static void assign(interval& x, double d) { x -= d; }
  };

  struct Times
  {
    static std::string symbol() { return "*"; }
    static Range reference(const Range& x, const Range& y) { return product_range(x, y); }
    static interval apply(const interval& x, const interval& y) { return x * y; }
    static interval apply(const interval& x, double d) { return x * d; }
    static interval apply(double d, const interval& y) { return d * y; }
    static void assign(interval& x, const interval& y) { x *= y; }
    static void assign(interval& x, double d) { x *= d; }
  };

  struct Over
  {
    static std::string symbol() { return "/"; }
    static Range reference(const Range& x, const Range& y) { return quotient_range(x, y); }
    static interval apply(const interval& x, const interval& y) { return x / y; }
    static interval apply(const interval& x, double d) { return x / d; }
    static interval apply(double d, const interval& y) { return d / y; }
    static void assign(interval& x, const interval& y) { x /= y; }
    static void assign(interval& x, double d) { x /= d; }
  };

  struct Relational
  {
    static std::string symbol() { return "%"; }
    static Range reference(const Range& x, const Range& y) { return relational_quotient_range(x, y); }
    static interval apply(const interval& x, const interval& y) { return x % y; }
    static interval apply(const interval& x, double d) { return x % d; }
    static interval apply(double d, const interval& y) { return d % y; }
    static void assign(interval& x, const interval& y) { x %= y; }
    static void assign(interval& x, double d) { x %= d; }
  };

  // [x] op [y], [x] op= [y], [x] op= [x] (the operand being *this), and the
  // forms with a double of the degenerate intervals of the grid, for every
  // pair of the grid and every direction of the calling code
  template<class Op>
  void binary_grid(const std::vector<Box>& boxes)
  {
    const std::string s = Op::symbol();
    const Form binary = form("[x]" + s + "[y]"), assigned = form("[x]" + s + "=[y]"), self = form("[x]" + s + "=[x], the same interval");
    const Form with_double = form("[x]" + s + "d"), double_with = form("d" + s + "[y]"), assigned_double = form("[x]" + s + "=d");
    for (const Box& X : boxes) {
      for (const Box& Y : boxes) {
        const Range e = Op::reference(X.range, Y.range);
        for (const CallerDirection& c : caller_directions) {
          const auto context = [&] { return "x=" + text(X.range) + " y=" + text(Y.range) + " (the caller's direction " + c.name + ")"; };
          expect_range(binary, under(c.fenv, [&] { return Op::apply(X.x, Y.x); }), e, context);
          expect_range(assigned, under(c.fenv, [&] { interval r(X.x); Op::assign(r, Y.x); return r; }), e, context);
          if (&X == &Y) {
            expect_range(self, under(c.fenv, [&] { interval r(X.x); Op::assign(r, r); return r; }), e, context);
          }
          if (is_point(Y.range)) {
            const double d = Y.range.lo;
            expect_range(with_double, under(c.fenv, [&] { return Op::apply(X.x, d); }), e, context);
            expect_range(assigned_double, under(c.fenv, [&] { interval r(X.x); Op::assign(r, d); return r; }), e, context);
          }
          if (is_point(X.range)) {
            const double d = X.range.lo;
            expect_range(double_with, under(c.fenv, [&] { return Op::apply(d, Y.x); }), e, context);
          }
        }
      }
    }
    // A double that is infinite or a NaN gives the empty set, as interval(d) does
    const Form infinite = form("[x]" + s + "d, d" + s + "[x] and [x]" + s + "=d for an infinite or NaN d");
    for (double d : { -inf, inf, std::numeric_limits<double>::quiet_NaN() }) {
      for (const Box& X : boxes) {
        for (const CallerDirection& c : caller_directions) {
          const auto context = [&] { return "x=" + text(X.range) + " d=" + hex(d) + " (the caller's direction " + c.name + ")"; };
          expect_range(infinite, under(c.fenv, [&] { return Op::apply(X.x, d); }), no_range, context);
          expect_range(infinite, under(c.fenv, [&] { return Op::apply(d, X.x); }), no_range, context);
          expect_range(infinite, under(c.fenv, [&] { interval r(X.x); Op::assign(r, d); return r; }), no_range, context);
        }
      }
    }
  }

  void unary_grid(const std::vector<Box>& boxes)
  {
    const Form square = form("sqr([x])"), root = form("sqrt([x])"), inverse_function = form("inverse([x])"), inverse_method = form("[x].inverse()");
    for (const Box& X : boxes) {
      for (const CallerDirection& c : caller_directions) {
        const auto context = [&] { return "x=" + text(X.range) + " (the caller's direction " + c.name + ")"; };
        expect_range(square, under(c.fenv, [&] { return sqr(X.x); }), square_range(X.range), context);
        expect_range(root, under(c.fenv, [&] { return sqrt(X.x); }), square_root_range(X.range), context);
        expect_range(inverse_function, under(c.fenv, [&] { return inverse(X.x); }), inverse_range(X.range), context);
        expect_range(inverse_method, under(c.fenv, [&] { return X.x.inverse(); }), inverse_range(X.range), context);
      }
    }
  }

  void special_bounds_grid()
  {
    const RestoreDirection restore;
    // The reference first, against the tables of divisions_by_zero()
    for (const auto& c : zero_divisions) {
      const Range q = quotient_range({ false, c.xl, c.xu }, { false, c.yl, c.yu });
      check("special bounds: the reference quotient is the one of the table", c.empty ? q.empty : (!q.empty && q.lo == c.lo && q.hi == c.hi),
            [&] { return "x=" + text({ false, c.xl, c.xu }) + " y=" + text({ false, c.yl, c.yu }) + ": " + text(q); });
    }
    for (const auto& c : zero_relational_divisions) {
      const Range q = relational_quotient_range({ false, c.xl, c.xu }, { false, c.yl, c.yu });
      check("special bounds: the reference relational quotient is the one of the table", c.empty ? q.empty : (!q.empty && q.lo == c.lo && q.hi == c.hi),
            [&] { return "x=" + text({ false, c.xl, c.xu }) + " y=" + text({ false, c.yl, c.yu }) + ": " + text(q); });
    }
    const std::vector<Box> boxes = special_boxes();
    binary_grid<Plus>(boxes);
    binary_grid<Minus>(boxes);
    binary_grid<Times>(boxes);
    binary_grid<Over>(boxes);
    binary_grid<Relational>(boxes);
    unary_grid(boxes);
  }

  /*
    Operations whose operands are literals the compiler sees (GAOL v5), in
    functions it can inline whole: the library computes with the rounding
    direction upward, a direction no compiler models, so a compiler that
    folded an addition of vectors of NEON at compile time, as it folds one
    of doubles, would compute it in the rounding to nearest (it does so once
    the library is inlined too, with -flto). The kernels pass their operands
    through an empty asm statement to prevent it (reread_bounds() of
    gaol/gaol_interval_neon.cpp). Every operation below is inexact, so that
    an interval folded to one double is not the tightest enclosure of its
    result (the exact value is no double: 0.1 + 0.2, 1 - 0.1, 0.1*0.3, 1/3,
    0.1^2 and the root of 2), and has to be the tightest, whatever the
    direction of the calling code. What these checks catch: an operation that
    computes in the rounding to nearest, and a folding at compile time, which
    only happens in a build GAOL refuses to support, a program and a library
    both without -frounding-math (compiled with -flto): with it, which
    gaol::gaol gives to the code using GAOL, the compilers do not fold these
    operations, and the checks cannot fail for this reason.
  */
  interval literal_sum() { return interval(0.1) + interval(0.2); }
  interval literal_difference() { return interval(1.0) - interval(0.1); }
  interval literal_product() { return interval(0.1) * interval(0.3); }
  interval literal_quotient() { return interval(1.0) / interval(3.0); }
  interval literal_relational_quotient() { return interval(1.0) % interval(3.0); }
  interval literal_inverse() { return inverse(interval(3.0)); }
  interval literal_square() { return sqr(interval(0.1)); }
  interval literal_square_root() { return sqrt(interval(2.0)); }
  interval literal_sum_with_double() { return interval(0.1) + 0.2; }
  interval literal_double_difference() { return 1.0 - interval(0.1); }
  interval literal_product_with_double() { return 0.1 * interval(0.3); }
  interval literal_quotient_by_double() { return interval(1.0) / 3.0; }
  // The five compound forms with a double in a row, and a line of operators
  interval literal_compound_line()
  {
    interval r(0.1);
    r += 0.2;
    r *= 0.3;
    r -= 0.1;
    r /= 3.0;
    r %= 0.7;
    return r;
  }
  interval literal_line()
  {
    return ((interval(0.1) + interval(0.2)) * interval(0.3) - interval(1.0) / interval(3.0)) / interval(0.7) + sqr(interval(0.1));
  }
  Reference reference_compound_line()
  {
    Reference r = { { false, 0.1, 0.1 } };
    const Reference a = { { false, 0.2, 0.2 } }, b = { { false, 0.3, 0.3 } }, c = { { false, 0.1, 0.1 } }, d = { { false, 3.0, 3.0 } },
                    e = { { false, 0.7, 0.7 } };
    r += a;
    r *= b;
    r -= c;
    r /= d;
    r %= e;
    return r;
  }
  Reference reference_line()
  {
    const Reference tenth = { { false, 0.1, 0.1 } }, fifth = { { false, 0.2, 0.2 } }, three_tenths = { { false, 0.3, 0.3 } },
                    one = { { false, 1.0, 1.0 } }, three = { { false, 3.0, 3.0 } }, seven_tenths = { { false, 0.7, 0.7 } };
    return ((tenth + fifth) * three_tenths - one / three) / seven_tenths + sqr(tenth);
  }

  void literal_operands()
  {
    const RestoreDirection restore;
    const double tenth = 0.1, fifth = 0.2, three_tenths = 0.3;
    const Dyadic a = dyadic(tenth), b = dyadic(fifth), c = dyadic(three_tenths), one = dyadic(1.0), three = dyadic(3.0);
    const struct { const char *name; interval (*literal)(); Exact exact_value; } cases[] = {
      { "[0.1]+[0.2]", literal_sum, exact(a + b) },
      { "[1]-[0.1]", literal_difference, exact(one - a) },
      { "[0.1]*[0.3]", literal_product, exact(a * c) },
      { "[1]/[3]", literal_quotient, quotient(one, three) },
      { "[1]%[3]", literal_relational_quotient, quotient(one, three) },
      { "inverse([3])", literal_inverse, quotient(one, three) },
      { "sqr([0.1])", literal_square, exact(a * a) },
      { "[0.1]+0.2", literal_sum_with_double, exact(a + b) },
      { "1-[0.1]", literal_double_difference, exact(one - a) },
      { "0.1*[0.3]", literal_product_with_double, exact(a * c) },
      { "[1]/3", literal_quotient_by_double, quotient(one, three) },
    };
    for (const CallerDirection& direction : caller_directions) {
      for (const auto& k : cases) {
        std::fesetround(direction.fenv);
        const interval r = k.literal();
        check(std::string("literal operands: ") + k.name + " is the tightest enclosure", is_tightest_enclosure(r, k.exact_value),
              [&] { return std::string("the caller's direction ") + direction.name + ": " + hex(r); });
      }
      std::fesetround(direction.fenv);
      const interval root = literal_square_root();
      expect_root("literal operands: sqrt([2])", root, 2.0, 2, 0);
      // The lines, against the reference computed step by step at run time
      std::fesetround(direction.fenv);
      const interval compound = literal_compound_line();
      std::fesetround(direction.fenv);
      const interval line = literal_line();
      const Range expected_compound = reference_compound_line().range, expected_line = reference_line().range;
      check("literal operands: a line of the five compound forms with a double", is_range(compound, expected_compound),
            [&] { return std::string("the caller's direction ") + direction.name + ": " + hex(compound) + " rather than " + text(expected_compound); });
      check("literal operands: a line of operators", is_range(line, expected_line),
            [&] { return std::string("the caller's direction ") + direction.name + ": " + hex(line) + " rather than " + text(expected_line); });
    }
  }

  /*
    Lines whose operands appear several times, written once for the
    intervals and for the reference (GAOL v5): an operation reads twice the
    interval it has just computed, or the one it assigns to, as the kernels
    of the NEON intervals do with the registers their results come back in.
    As in long_lines(), p and e are positive, the divisors; r -= r and r *= r
    are the aliasing of the compound forms, the divisions by an interval of
    the grid that holds 0 being those of binary_grid()
  */
  template<class T> T repeated_operands(const T& a, const T& b, const T& p, const T& e, const T& s1, const T& s2, const T& s3, const T& s4)
  {
    T r = a - b;
    r *= r;
    r += r;
    r = r - r + p;
    r *= s1 + s2;
    r /= e;
    r -= s3 * s4;
    r %= p + e;
    return r + r;
  }
  template<class T> T repeated_quotients(const T& a, const T& b, const T& p, const T& e, const T& s1, const T& s2, const T& s3, const T& s4)
  {
    (void)s3; (void)s4;
    return ((p / p) + (e % e)) * (a - b) - (p + e) / (p + e) * s1 + (sqr(s2) + p) / (sqr(s2) + p);
  }
  template<class T> T compound_line(const T& a, const T& b, const T& p, const T& e, const T& s1, const T& s2, const T& s3, const T& s4)
  {
    T r = a;
    r += b; r -= s1; r *= s2; r /= p; r %= e; r -= r; r += s3; r *= s4; r += r; r -= a;
    return r;
  }

  typedef interval (*IntervalLine)(const interval&, const interval&, const interval&, const interval&,
                                   const interval&, const interval&, const interval&, const interval&);
  typedef Reference (*ReferenceLine)(const Reference&, const Reference&, const Reference&, const Reference&,
                                     const Reference&, const Reference&, const Reference&, const Reference&);

  // The lines of long_lines() and the ones above, on operands drawn from the
  // pools (the sixth and seventh operands are p and e), whatever the
  // direction of the calling code: each has the bounds of the reference
  // evaluating it one operation after the other
  void lines_on_pools(const std::string& kind, const std::vector<Box>& pool, const std::vector<Box>& positive_pool, int iterations,
                      Random& random)
  {
    const struct { const char *name; IntervalLine on_intervals; ReferenceLine on_ranges; } lines[] = {
      { "a line of + (16)", sum16<interval>, sum16<Reference> },
      { "a line of + and - (16)", diff16<interval>, diff16<Reference> },
      { "a line of * (8)", mul8<interval>, mul8<Reference> },
      { "a line of * in a tree (8)", mul8_tree<interval>, mul8_tree<Reference> },
      { "a line of / (8)", quot8<interval>, quot8<Reference> },
      { "a line of + - * / (13)", mix13<interval>, mix13<Reference> },
      { "a line of + - * / in a tree (14)", mix_tree<interval>, mix_tree<Reference> },
      { "a line with operands used twice (13)", repeated_operands<interval>, repeated_operands<Reference> },
      { "a line of quotients by the same expression (13)", repeated_quotients<interval>, repeated_quotients<Reference> },
      { "a line of compound forms (12)", compound_line<interval>, compound_line<Reference> },
    };
    std::vector<std::string> names;
    for (const auto& l : lines) {
      names.push_back("lines against the reference, " + kind + ": " + l.name);
    }
    for (int i = 0; i < iterations; ++i) {
      const Box *o[8];
      for (int k = 0; k < 8; ++k) {
        const std::vector<Box>& from = (k == 2 || k == 3) ? positive_pool : pool;
        o[k] = &from[static_cast<std::size_t>(random.integer(0, static_cast<int>(from.size()) - 1))];
      }
      const CallerDirection& direction = caller_directions[static_cast<std::size_t>(i) % nb_caller_directions];
      for (std::size_t k = 0; k < sizeof(lines)/sizeof(lines[0]); ++k) {
        std::fesetround(direction.fenv);
        const interval r = lines[k].on_intervals(o[0]->x, o[1]->x, o[2]->x, o[3]->x, o[4]->x, o[5]->x, o[6]->x, o[7]->x);
        const Range e = lines[k].on_ranges({ o[0]->range }, { o[1]->range }, { o[2]->range }, { o[3]->range },
                                           { o[4]->range }, { o[5]->range }, { o[6]->range }, { o[7]->range }).range;
        check(names[k], is_range(r, e), [&] {
          std::string text_of_operands;
          for (int j = 0; j < 8; ++j) {
            text_of_operands += (j ? " " : "") + text(o[j]->range);
          }
          return std::string("the caller's direction ") + direction.name + ", operands " + text_of_operands + ": "
                 + (r.is_empty() ? std::string("empty") : hex(r)) + " rather than " + text(e);
        });
      }
    }
  }

  void lines_against_the_reference(Random& random)
  {
    const RestoreDirection restore;
    // Intervals of random bounds, mostly of one sign or the other, whose lines are finite
    std::vector<Box> pool, positive_pool;
    for (int i = 0; i < 300; ++i) {
      const double u = random(-8, 8), v = random(-8, 8);
      pool.push_back(box(std::min(u, v), std::max(u, v)));
      const double p = random.positive(-8, 8), q = random.positive(-8, 8);
      positive_pool.push_back(box(std::min(p, q), std::max(p, q)));
    }
    lines_on_pools("random intervals", pool, positive_pool, 3000, random);
    // The intervals of the grid and of any magnitude: zeros, infinities,
    // subnormal and large doubles, so that the lines end in half-lines, in
    // the whole line, in the empty set
    std::vector<Box> wild = special_boxes(), wild_positive;
    for (int i = 0; i < 60; ++i) {
      const double u = random.any(), v = random.any();
      wild.push_back(box(std::min(u, v), std::max(u, v)));
    }
    for (const Box& b : wild) {
      if (!b.range.empty && b.range.lo > 0.0) {
        wild_positive.push_back(b);
      }
    }
    lines_on_pools("special bounds and any doubles", wild, wild_positive, 1500, random);
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

  /*
    Squares at special values: bounds 0 of both signs or infinite, squares
    below the smallest subnormal or beyond the largest double, in the four
    cases of the signs of the stored bounds of the SSE2 intervals (I
    positive, negative, straddling 0, and [0, -0]). sqr() of the SSE2
    intervals computes its one product itself, where it took the binary
    exponentiation of the products rounded outward, for e = 2 (GAOL v5,
    point AA of TODO.md). The bounds are the tightest, worked out by hand.
  */
  void squares_at_special_values()
  {
    const double tiny = std::numeric_limits<double>::denorm_min(), largest = std::numeric_limits<double>::max();
    const struct { const char *name; interval x; double lo, hi; } cases[] = {
      { "sqr([0,2])", interval(0., 2.), 0., 4. },
      { "sqr([-0,2])", interval(-0., 2.), 0., 4. },
      { "sqr([-2,0])", interval(-2., 0.), 0., 4. },
      { "sqr([-2,-0])", interval(-2., -0.), 0., 4. },
      { "sqr([-3,2])", interval(-3., 2.), 0., 9. },
      { "sqr([-2,3])", interval(-2., 3.), 0., 9. },
      { "sqr([2,3])", interval(2., 3.), 4., 9. },
      { "sqr([-3,-2])", interval(-3., -2.), 4., 9. },
      { "sqr([0,0])", interval(0., 0.), 0., 0. },
      { "sqr([-0,0])", interval(-0., 0.), 0., 0. },
      { "sqr([0,-0])", interval(0., -0.), 0., 0. },
      { "sqr([-0,-0])", interval(-0., -0.), 0., 0. },
      { "sqr([1,+oo])", interval(1., inf), 1., inf },
      { "sqr([-oo,-1])", interval(-inf, -1.), 1., inf },
      { "sqr([0,+oo])", interval(0., inf), 0., inf },
      { "sqr([-oo,0])", interval(-inf, 0.), 0., inf },
      { "sqr([-oo,2])", interval(-inf, 2.), 0., inf },
      { "sqr([-1,+oo])", interval(-1., inf), 0., inf },
      { "sqr([-oo,+oo])", interval::universe(), 0., inf },
      { "sqr([2^-1074])", interval(tiny), 0., tiny },
      { "sqr([-2^-1074])", interval(-tiny), 0., tiny },
      { "sqr([-2^-1074,2^-1074])", interval(-tiny, tiny), 0., tiny },
      { "sqr([2^-600,2^-500])", interval(0x1p-600, 0x1p-500), 0., 0x1p-1000 },
      { "sqr([3 2^-538]), between two subnormals", interval(0x1.8p-537), 2*tiny, 3*tiny },
      { "sqr([-3 2^-538])", interval(-0x1.8p-537), 2*tiny, 3*tiny },
      { "sqr([2^511])", interval(0x1p511), 0x1p1022, 0x1p1022 },
      { "sqr([2^512])", interval(0x1p512), largest, inf },
      { "sqr([-largest])", interval(-largest), largest, inf },
      { "sqr([-largest,1])", interval(-largest, 1.), 0., inf },
      { "sqr([1 + 2^-52])", interval(1.0 + 0x1p-52), 1.0 + 0x1p-51, 1.0 + 3*0x1p-52 },
      { "sqr([-(1 + 2^-52),-1])", interval(-(1.0 + 0x1p-52), -1.), 1., 1.0 + 3*0x1p-52 },
    };
    for (const auto& c : cases) {
      const interval r = sqr(c.x);
      check(std::string(c.name) + ": the tightest", !r.is_empty() && r.left() == c.lo && r.right() == c.hi,
            [&] { return hex(r); });
    }
    check("sqr(empty): empty", sqr(interval::emptyset()).is_empty(), [] { return std::string(); });
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
    gaol/gaol_interval_core.cpp). The bounds have to be the tightest, as
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
    (pown_products() of gaol/gaol_interval_core.cpp). Without that,
    pow([2, 3], 3) was [8 - 2^-50, 27 + 2^-48]. x^n of an integer x is a
    double while it is below 2^53, and x^-n is one for a power of two x only.
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
    // x^2, one square whose rest is exact: the tightest bounds, those of
    // sqr(x), near 1 too, where the bound K of the error of the other powers
    // takes a double more (GAOL v5: pow([1 + 2^-52], 2) was [1 + 2^-52,
    // 1 + 3 2^-52], [1 + 2^-51, 1 + 3 2^-52] being the tightest)
    const interval just_above_one(1.0 + 0x1p-52), square = pow(just_above_one, 2);
    check("pow([1 + 2^-52], 2) = [1 + 2^-51, 1 + 3 2^-52]",
          square.left() == 1.0 + 0x1p-51 && square.right() == 1.0 + 3*0x1p-52, [&] { return hex(square); });
    for (int k = 1; k <= 16; ++k) {
      for (const double c : { 1.0 + k*0x1p-52, 1.0 - k*0x1p-53, 3.0 + k*0x1p-51, 0.75 - k*0x1p-54, 0x1p-500 + k*0x1p-552 }) {
        for (const interval& x : { interval(c), interval(-c), interval(-c, 2*c), interval(-2*c, c), interval(c, 2*c) }) {
          const interval r = pow(x, 2), s = sqr(x);
          check("pow(x, 2) is sqr(x)", !r.is_empty() && r.left() == s.left() && r.right() == s.right(),
                [&] { return "pow(" + hex(x) + ", 2) = " + hex(r) + ", sqr = " + hex(s); });
        }
      }
    }
    // The powers of -c, the opposites of those of c for an odd n, and the
    // bound c^n the same whether the other corner is computed with c, in the
    // other lane, or alone, being beyond the range of the lanes, as -1e300 is
    // (pown_point()), where c^n is within the bound K of the error of a
    // double too: the lanes compute the powers of |c|, whose products rounded
    // upward are not the opposites of those of -c (pow([-(1 + 2^-51)], 21)
    // would have the upper bound -0x1.0000000000029p+0 from c itself, rather
    // than -0x1.000000000002ap+0)
    for (int k = 1; k <= 16; ++k) {
      for (const double c : { 1.0 + k*0x1p-52, 1.0 - k*0x1p-53, 1.5 + k*0x1p-52, 3.0 + k*0x1p-51 }) {
        for (int n : { 3, 4, 5, 7, 9, 13, 21, 31, 33, 63, 101, -2, -3, -5, -21, -33, -64 }) {
          const interval p = pow(interval(c), n), q = pow(interval(-c), n), w = pow(interval(-1e300, -c), n);
          // -c gives the upper bound of x^n on [-1e300, -c] where x^n increases there
          const bool upper = (n > 0) == (n % 2 != 0);
          const double t = upper ? w.right() : w.left(), u = upper ? q.right() : q.left();
          check("pow([-c], n): the opposite of pow([c], n) for an odd n, pow([c], n) for an even one, "
                "and the bound of the corner -c alone",
                !p.is_empty() && !q.is_empty() && t == u
                  && ((n % 2 != 0) ? (q.left() == -p.right() && q.right() == -p.left())
                                   : (q.left() == p.left() && q.right() == p.right())),
                [&] { return "c = " + hex(c) + ", n = " + std::to_string(n) + ": " + hex(p) + " " + hex(q)
                             + " " + hex(w); });
        }
      }
    }
  }

  /*
    Integer powers for large n: the tightest bounds, from exact products
    within the ints, each of these powers being farther from a double than
    twice the bound K of the error of their products (pown_products() of
    gaol_interval_core.cpp), and from CORE-MATH's pow beyond them, for an
    unsigned n. The square of the rest carried along was left out of the
    lower bound (GAOL v5, review #7 of examples/examples.md), and the lower
    bound of x^n was 8 doubles below the tightest for x = 1.0000001 and
    n = 2^28 - 1, 557 for 2^31 - 1, 1962 for 2^32 - 1, more than one double
    below for most n above 2^28. The bounds are computed apart, with mpmath:

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
    The n-th roots of x below 2^-1020 (4 times the smallest normal number), the
    subnormal x included, as tight as those of the other x: within 2 doubles
    of the tightest bounds, and exact where x is an n-th power (GAOL v5, point
    AC of TODO.md). The proofs of the bounds took the powers l^k of the binary
    exponentiation as subnormal doubles, which have 33 bits at 2^-1041.5, and
    nth_root([2^-1074], 33) was [0x1.5ecf527769956p-33, 0x1.5ecf527771187p-33],
    1983 doubles below its tightest lower bound and 28785 above its upper one;
    n = 65 gave 3.5e8 and 6.9e8 (the bounds still enclosed the root). The roots
    for n = 4 to 1025 are compared with exact arithmetic, the others, and
    rootn(x, -n), with the tightest bounds computed apart with mpmath. A
    product rounded in a direction has an error of up to 2^-52 relatively, so
    that a bound of the root may be 2 doubles from the tightest (as for the
    other x, before the change as well), and 4 for rootn(x, -n), the inverse of
    a root, whose ulp is half that of the root where the root is at the bottom
    of a binade and its inverse at the top. The values below are within 1 and
    3:

      mpmath.mp.prec = 2000
      v = mpmath.mpf(x) ** (mpmath.mpf(1) / n)    # (-1 / n) for q = -n
      lo, hi = (float(mpmath.mpf(v, prec=53, rounding=r)) for r in 'du')
  */
  void roots_of_tiny_numbers()
  {
    const double tiny = std::numeric_limits<double>::denorm_min();
    const double threshold = 4.0*std::numeric_limits<double>::min(); // 2^-1020
    // The last two are on either side of the limit below which the proofs
    // take scaled powers (the bounds are the same on both sides)
    for (const double x : { tiny, 3*tiny, std::ldexp(1.0, -1060), std::numeric_limits<double>::min() - tiny, 1e-310,
                            5e-308, previous_double(threshold), threshold }) {
      for (const unsigned int n : { 4u, 5u, 33u, 65u, 129u, 1025u }) {
        const interval r = nth_root(interval(x), n);
        expect_root("nth_root([x],n) for x below 2^-1020", r, x, static_cast<int>(n), 2);
        if (n % 2 == 1) {
          const interval minus_r = nth_root(interval(-x), n);
          check("nth_root([-x],n) for x below 2^-1020 and an odd n: -nth_root([x],n)",
                !minus_r.is_empty() && minus_r.left() == -r.right() && minus_r.right() == -r.left(),
                [&] { return "x=" + hex(x) + " n=" + std::to_string(n) + ": " + hex(minus_r); });
        }
      }
    }

    // The root of a double that is an n-th power is that double, the proofs
    // keeping the equality (x is given: the pow() of the C library is not exact
    // in the upward rounding the test runs in for some subnormal results, the
    // one of macOS for 3^11 2^-1045 for instance, and the n-th power of the
    // root is checked with the exact arithmetic of the test)
    const struct { double x, root; unsigned int n; } exact_powers[] = {
      { std::ldexp(1.0, -1074), std::ldexp(1.0, -179), 6u }, { std::ldexp(1.0, -1074), std::ldexp(1.0, -6), 179u },
      { std::ldexp(1.0, -1060), std::ldexp(1.0, -265), 4u }, { std::ldexp(1.0, -1060), std::ldexp(1.0, -212), 5u },
      { std::ldexp(43046721.0, -1072), std::ldexp(3.0, -67), 16u },        // 3^16
      { std::ldexp(177147.0, -1045), std::ldexp(3.0, -95), 11u },           // 3^11
    };
    for (const auto& c : exact_powers) {
      const auto describe = [&] { return "x=" + hex(c.x) + " n=" + std::to_string(c.n); };
      if (check("nth_root([x],n) for x below 2^-1020, x an n-th power: the power is x",
                c.x < threshold && compare(c.x, power(c.root, static_cast<int>(c.n))) == 0, describe)) {
        const interval r = nth_root(interval(c.x), c.n);
        check("nth_root([x],n) for x below 2^-1020, x an n-th power: [root]",
              !r.is_empty() && r.left() == c.root && r.right() == c.root, [&] { return describe() + ": " + hex(r); });
      }
    }

    const auto expect = [](const std::string& name, double x, long long n, const interval& r, double lo, double hi,
                           int beyond) {
      double lowest = lo, highest = hi;
      for (int i = 0; i < beyond; ++i) {
        lowest = previous_double(lowest);
        highest = next_double(highest);
      }
      check(name, !r.is_empty() && r.left() <= lo && r.left() >= lowest && r.right() >= hi && r.right() <= highest,
            [&] { return "x=" + hex(x) + " n=" + std::to_string(n) + ": " + hex(r) + " for [" + hex(lo) + ", " + hex(hi) + "]"; });
    };
    const struct { double x; unsigned int n; double lo, hi; } large[] = {
      { 0x1p-1074, 1048577u, 0x1.ffa2fa650a018p-1, 0x1.ffa2fa650a019p-1 },    // 2^20 + 1
      { 0x1p-1074, 2147483647u, 0x1.fffff45e3d7f8p-1, 0x1.fffff45e3d7f9p-1 }, // 2^31 - 1
      { 0x1p-1074, 4294967295u, 0x1.fffffa2f1eb75p-1, 0x1.fffffa2f1eb76p-1 }, // 2^32 - 1
      { 0x1p-1060, 1048577u, 0x1.ffa430b4939dbp-1, 0x1.ffa430b4939dcp-1 },
      { 0x1p-1060, 2147483647u, 0x1.fffff4850e73ep-1, 0x1.fffff4850e73fp-1 },
      { 0x1p-1060, 4294967295u, 0x1.fffffa428731bp-1, 0x1.fffffa428731cp-1 },
      { 0x0.fffffffffffffp-1022, 2147483647u, 0x1.fffff4ee6a2fcp-1, 0x1.fffff4ee6a2fdp-1 },
      { 1e-310, 4294967295u, 0x1.fffffa6c65b96p-1, 0x1.fffffa6c65b97p-1 },
    };
    for (const auto& c : large) {
      expect("nth_root([x],n) for x below 2^-1020 and a large n", c.x, c.n, nth_root(interval(c.x), c.n), c.lo, c.hi, 2);
    }
    const struct { double x; int q; double lo, hi; } negative[] = {
      { 0x1p-1074, -33, 0x1.75a07cfb107edp+32, 0x1.75a07cfb107eep+32 },
      { 0x1p-1060, -65, 0x1.3cdba13cc9501p+16, 0x1.3cdba13cc9502p+16 },
      { 0x1p-1074, -1048577, 0x1.002e8b4248fbep+0, 0x1.002e8b4248fbfp+0 },
      { 1e-310, -129, 0x1.f9fa1c4cbde09p+7, 0x1.f9fa1c4cbde0ap+7 },
    };
    for (const auto& c : negative) {
      expect("rootn(x, q) for x below 2^-1020 and q < 0", c.x, c.q, nth_root(interval(c.x), c.q), c.lo, c.hi, 4);
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
      const double v = gaol_core::rnd_keep(std::fma(a, b, c));
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
        const double lo = gaol_core::rnd_keep(X.left() - Y.left());
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
  quotients_at_special_values();
  long_lines(random);
  special_bounds_grid();
  literal_operands();
  lines_against_the_reference(random);
  unsigned_powers();
  negative_powers_at_special_values();
  squares_at_special_values();
  powers_at_special_values();
  powers_near_the_ends();
  exact_powers();
  large_powers();
  roots_at_special_values();
  roots_of_tiny_numbers();
  fused_and_cancellative();
  const int status = summary();
  gaol::cleanup();
  return status;
}
