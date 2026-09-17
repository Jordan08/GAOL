/*-*-C++-*------------------------------------------------------------------
 * gaol -- NOT Just Another Interval Library
 *--------------------------------------------------------------------------
 * Tests of this fork of GAOL: the arithmetic operations.
 *
 * On random doubles and intervals, the bounds are compared exactly with the
 * exact results: sums, differences, products, quotients, squares and inverses
 * have to be enclosed as tightly as possible, which GAOL does computing with
 * the rounding direction set upward, and so do square roots, which IEEE
 * 1788-2015 requires (12.10.2); integer powers and n-th roots, which GAOL
 * computes in several rounded operations, to be enclosed within a few
 * doubles. Doubles of every magnitude are drawn, subnormal ones and results
 * beyond the largest double included. The operators of an interval with a
 * double are also compared with the operators with the degenerate interval of
 * the double, on bounds and doubles of special values: zeros of both signs,
 * infinities and NaN. Products of intervals with zero and infinite bounds have
 * to be the hull of the products of the bounds, a zero bound times an infinite
 * one counting as 0, the pairs of bounds that are not an interval being the
 * empty set. The n-th roots are the rootn of IEEE 1788-2015: for an odd n,
 * the root of a negative number is the opposite of the root of its magnitude.
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
  // twice those found on the platforms tested: GAOL computes integer powers by
  // repeated rounded products, at most n+1 doubles away for x^n and 4 for x^-n.
  // The n-th roots are proved with integer powers rounded upward and downward,
  // at most one double away (GAOL took powers with a rounded exponent, up to
  // 8 doubles away between 2^-30 and 2^30 and 234 over all the doubles).
  // Square roots are the tightest (GAOL's lower bound was one double below it
  // for half of the doubles)
  int power_limit(int n)
  {
    return 2*n;
  }

  const int negative_power_limit = 8;
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

  // Operations on doubles, drawn by draw
  template<class Draw>
  void operations_on_doubles(const std::string& range, Draw draw)
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

      for (int n = 2; n <= 7; ++n) {
        const Exact p = power(a, n);
        const interval N(n);
        expect_close("pow([a],n) for n=" + std::to_string(n) + in, pow(A, n), p, p, power_limit(n), A, N);
      }
      if (a != 0.0) {
        for (int n = 1; n <= 3; ++n) {
          const Dyadic an = dyadic_power(a, n);
          const Exact p = quotient(dyadic(1.0), an);
          const std::string name = "pow([a],-n) for n=" + std::to_string(n) + in;
          // (1/a)^n where a^n is beyond the largest double: GAOL computed
          // 1/a^n, [0, 1/max], wide at that scale
          expect_close(name, pow(A, -n), p, p, negative_power_limit, A, interval(-n));
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
      for (int n = 2; n <= 5; ++n) {
        const std::vector<Exact> powers = { power(X.left(), n), power(X.right(), n) };
        const Exact lo = (n % 2 == 1) ? powers[0] : (x_has_zero ? zero : min(powers));
        const Exact hi = (n % 2 == 1) ? powers[1] : max(powers);
        expect_close("pow([x],n) for n=" + std::to_string(n) + in, pow(X, n), lo, hi, power_limit(n), X, interval(n));
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

  // gaol::uipow(), the pown of IEEE 1788 for an unsigned exponent, as
  // pow(x, n): GAOL declared it public but defined it INLINE with the SSE2
  // intervals, and a program calling it did not link (fork of GAOL)
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
        const interval r = gaol::uipow(x, n), p = pow(x, static_cast<int>(n));
        check("gaol::uipow([x],n) as pow([x],n)",
              r.is_empty() ? p.is_empty() : (!p.is_empty() && r.left() == p.left() && r.right() == p.right()),
              [&] { return hex(x) + ", n=" + std::to_string(n) + ": " + hex(r) + " and " + hex(p); });
      }
    }
    const interval one = gaol::uipow(interval(-2., 3.), 0), none = gaol::uipow(interval::emptyset(), 0);
    check("gaol::uipow([-2,3],0) = [1]", !one.is_empty() && one.left() == 1.0 && one.right() == 1.0, [&] { return hex(one); });
    check("gaol::uipow(empty,0): empty", none.is_empty(), [&] { return hex(none); });
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
    // the root is x exactly, which proves it to be both bounds
    for (unsigned int n = 3; n <= 40; ++n) {
      for (const double root : { 2., 3., 5., 7., 10., 1.5, 0.75, 0x1.8p-40, 0x1.4p+25 }) {
        const double x = std::pow(root, static_cast<double>(n));
        if (x > 0. && x < inf && compare(x, power(root, static_cast<int>(n))) == 0) {
          const interval r = nth_root(interval(x), n), minus_r = nth_root(interval(-x), n);
          const auto describe = [&] { return "x=" + hex(x) + " n=" + std::to_string(n) + ": " + hex(r); };
          check("nth_root([a^n],n): [a]", !r.is_empty() && r.left() == root && r.right() == root, describe);
          if (n % 2 == 1) {
            check("nth_root([-a^n],n) for an odd n: [-a]", !minus_r.is_empty() && minus_r.left() == -root && minus_r.right() == -root,
                  describe);
          }
        }
      }
    }
    // The extreme doubles, and large n, where the power with a rounded
    // exponent was the furthest from the root
    for (const double x : { std::numeric_limits<double>::max(), std::numeric_limits<double>::min(),
                            std::numeric_limits<double>::denorm_min(), 3*std::numeric_limits<double>::denorm_min(),
                            0x1.fffffffffffffp-1, 0x1.0000000000001p+0, 2., 10., 1e300, 1e-300 }) {
      for (const unsigned int n : { 3u, 4u, 17u, 100u }) {
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
}

int main()
{
  gaol::init();
  Random random;
  operations_on_doubles("exponents from -30 to 30", [&] { return random(-30, 30); });
  operations_on_doubles("any doubles", [&] { return random.any(); });
  operations_on_intervals("exponents from -30 to 30", [&] { return random(-30, 30); });
  operations_on_intervals("any doubles", [&] { return random.any(); });
  divisions_by_zero();
  operations_with_special_doubles();
  products_with_infinite_bounds();
  unsigned_powers();
  negative_powers_at_special_values();
  roots_at_special_values();
  const int status = summary();
  gaol::cleanup();
  return status;
}
