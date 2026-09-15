/*-*-C++-*------------------------------------------------------------------
 * gaol -- NOT Just Another Interval Library
 *--------------------------------------------------------------------------
 * Tests of this fork of GAOL: the arithmetic operations.
 *
 * On random doubles and intervals, the bounds are compared exactly with the
 * exact results: sums, differences, products, quotients, squares and inverses
 * have to be enclosed as tightly as possible, which GAOL does computing with
 * the rounding direction set upward; integer powers and roots, which GAOL
 * computes in several rounded operations, to be enclosed within a few
 * doubles. Doubles of every magnitude are drawn, subnormal ones and results
 * beyond the largest double included. The operators of an interval with a
 * double are also compared with the operators with the degenerate interval of
 * the double, on bounds and doubles of special values: zeros of both signs,
 * infinities and NaN.
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
  // repeated rounded products, at most n+1 doubles away for x^n, 4 for x^-n
  // and 1 for square roots; and n-th roots as powers with a rounded exponent,
  // whose error grows with |log x|, up to 8 doubles between 2^-30 and 2^30 and
  // 234 over all the doubles
  int power_limit(int n)
  {
    return 2*n;
  }

  const int negative_power_limit = 8;
  const int square_root_limit = 2;

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
          const interval r = pow(A, -n);
          if (compare(exact(an.sign < 0 ? -an : an), exact(std::numeric_limits<double>::max())) > 0) {
            // GAOL computes 1/a^n, and a^n beyond the largest double is enclosed
            // by [max, +oo], whose inverse, [0, 1/max], is wide at that scale
            check(name + ": encloses", is_enclosure(r, p), [&] { return operands(A, interval(-n)) + ": " + hex(r); });
          } else {
            expect_close(name, r, p, p, negative_power_limit, A, interval(-n));
          }
        }
      }

      const double m = std::fabs(a);
      expect_root("sqrt([|a|])" + in, sqrt(interval(m)), m, 2, square_root_limit);
      for (int n = 2; n <= 7; ++n) {
        expect_root("nth_root([|a|],n) for n=" + std::to_string(n) + in, nth_root(interval(m), static_cast<unsigned int>(n)), m, n, root_limit);
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
  const int status = summary();
  gaol::cleanup();
  return status;
}
