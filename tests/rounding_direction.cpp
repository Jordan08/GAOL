/*-*-C++-*------------------------------------------------------------------
 * gaol -- NOT Just Another Interval Library
 *--------------------------------------------------------------------------
 * Tests of GAOL v5: the rounding direction of the code using GAOL.
 *
 * GAOL computes its bounds with the rounding direction upward. By default,
 * each operation sets it upward when it is not, and leaves it upward; with
 * GAOL_PRESERVE_ROUNDING defined, each operation also restores the direction
 * it found (see gaol/gaol_fpu.h).
 *
 * Every operation of GAOL's interface is run on several operands with the
 * rounding direction of the calling code upward, to nearest, downward and
 * toward zero, and on x86 with the x87 unit and the SSE instructions in
 * different directions. Whatever the direction, it has to give the same
 * result, written exactly, as with the direction upward: a bound computed in
 * another direction differs, as does one a compiler computes after a change
 * of direction that the source code writes after its computation. It has to
 * leave the direction as it found it, or upward (as it found it only, with
 * GAOL_PRESERVE_ROUNDING). After it, the bounds of a product and a sum have to
 * be the tightest ones.
 *
 * Copyright (c) 2026 ENSTA, France
 *
 * Created 2026-09-20 by Jordan NININ
 *--------------------------------------------------------------------------
 * gaol is a software distributed WITHOUT ANY WARRANTY. Read the associated
 * COPYING file for information.
 *--------------------------------------------------------------------------*/

#include "gaol_tests.h"

#include <exception>
#include <sstream>
#include <string>
#include <utility>
#include <vector>

#if defined(__SSE2__) || defined(_M_X64) || (defined(_M_IX86_FP) && _M_IX86_FP >= 2)
#  include <xmmintrin.h>
#  define GAOL_TESTS_SSE 1
#  define SSE_DIRECTION(d) static_cast<unsigned>(d)
#else
#  define SSE_DIRECTION(d) 0u
#endif

using namespace gaol;
using namespace gaol_tests;

namespace
{
  // The results of the operations, written exactly
  std::string S(const interval& x) { return hex(x); }
  std::string S(double x) { return hex(x); }
  std::string S(bool b) { return b ? "true" : "false"; }
  std::string S(int n) { return std::to_string(n); }
  std::string S(unsigned n) { return std::to_string(n); }
  std::string S(long n) { return std::to_string(n); }
  std::string S(unsigned long n) { return std::to_string(n); }
  std::string S(long long n) { return std::to_string(n); }
  std::string S(unsigned long long n) { return std::to_string(n); }
  std::string S(const std::string& s) { return s; }

  struct Operation
  {
    const char *name;
    std::string (*run)(const interval& x, const interval& y);
  };

  // Writes x in the format f
  std::string write(const interval& x, interval_format::format_t f)
  {
    const interval_format::format_t saved = interval::format();
    interval::format(f);
    std::ostringstream s;
    s << x;
    interval::format(saved);
    return s.str();
  }

  // A rounding direction of the calling code: the one fesetround() sets, then
  // the one of the SSE instructions, where there are some
  struct Direction
  {
    const char *name;
    int fenv;
    unsigned sse;
  };

  const Direction directions[] = {
    { "upward", FE_UPWARD, SSE_DIRECTION(_MM_ROUND_UP) },
    { "to nearest", FE_TONEAREST, SSE_DIRECTION(_MM_ROUND_NEAREST) },
    { "downward", FE_DOWNWARD, SSE_DIRECTION(_MM_ROUND_DOWN) },
    { "toward zero", FE_TOWARDZERO, SSE_DIRECTION(_MM_ROUND_TOWARD_ZERO) },
#if GAOL_TESTS_SSE
    { "upward, and to nearest for SSE", FE_UPWARD, SSE_DIRECTION(_MM_ROUND_NEAREST) },
    { "to nearest, and upward for SSE", FE_TONEAREST, SSE_DIRECTION(_MM_ROUND_UP) },
#endif
  };

  void set(const Direction& d)
  {
    std::fesetround(d.fenv);
#if GAOL_TESTS_SSE
    _mm_setcsr((_mm_getcsr() & ~static_cast<unsigned int>(_MM_ROUND_MASK)) | d.sse);
#endif
  }

  // The rounding direction as fegetround() and the SSE control register give it
  struct State
  {
    int fenv;
    unsigned sse;

    bool operator==(const State& s) const { return fenv == s.fenv && sse == s.sse; }
  };

  State state()
  {
    State s;
    s.fenv = std::fegetround();
#if GAOL_TESTS_SSE
    s.sse = _mm_getcsr() & _MM_ROUND_MASK;
#else
    s.sse = 0u;
#endif
    return s;
  }

  std::string text(const State& s)
  {
    return "fegetround() " + std::to_string(s.fenv) + ", SSE rounding bits " + std::to_string(s.sse);
  }

  // Products and sums computed in a loop that changes the rounding direction
  // before each of them. A compiler may read the rounding direction once for
  // the whole loop, when it does not know that fesetround() changes it: Clang 18
  // did so with a read of MXCSR. GAOL would then not set the direction upward.
  void products_and_sums_in_a_loop(const std::vector<double>& a, const std::vector<double>& b,
                                   std::vector<interval>& p, std::vector<interval>& s)
  {
    const std::size_t nb_directions = sizeof(directions)/sizeof(directions[0]);
    for (std::size_t i = 0; i < a.size(); ++i) {
      set(directions[i % nb_directions]);
      p[i] = interval(a[i]) * interval(b[i]);
      s[i] = interval(a[i]) + interval(b[i]);
    }
  }

  std::string run(const Operation& op, const interval& x, const interval& y)
  {
    try {
      return op.run(x, y);
    } catch (const std::exception& e) {
      return std::string("exception: ") + e.what();
    } catch (...) {
      return "exception";
    }
  }
}

int main()
{
  gaol::init();
#if !GAOL_PRESERVE_ROUNDING
  check("rounding upward after gaol::init()", std::fegetround() == FE_UPWARD);
#endif

  // x within [0,1] and y above 1, the domain of every function below
  Random random;
  std::vector<std::pair<interval, interval> > operands;
  operands.push_back(std::make_pair(interval(0.1, 0.3), interval(1.5, 2.5)));
  for (int i = 0; i < 5; ++i) {
    const double a = random.uniform(0.01, 0.6), b = random.uniform(1.1, 2.0);
    operands.push_back(std::make_pair(interval(a, a + random.uniform(0.001, 0.39)), interval(b, b + random.uniform(0.001, 1.0))));
  }

  const Operation operations[] = {
    { "interval()", [](const interval&, const interval&) { interval z; return S(z); } },
    { "interval(a,b)", [](const interval&, const interval&) { return S(interval(0x1.999999999999ap-4, 0x1.5555555555555p-2)); } },
    { "interval(a)", [](const interval&, const interval&) { return S(interval(0.1)); } },
    { "interval(const char*)", [](const interval&, const interval&) { return S(interval("[0.1, 1/3]")); } },
    { "interval(const char*, const char*)", [](const interval&, const interval&) { return S(interval("0.1", "0.3")); } },
    { "interval(\"sin(1)+exp(0.1)\")", [](const interval&, const interval&) { return S(interval("sin(1)+exp(0.1)")); } },
    { "interval(\"3.56?1e-2\")", [](const interval&, const interval&) { return S(interval("3.56?1e-2")); } },
    { "interval(\"[-0x1.00000000000001p0, 2/3]\")", [](const interval&, const interval&) { return S(interval("[-0x1.00000000000001p0, 2/3]")); } },
    { "operator+", [](const interval& x, const interval& y) { return S(x + y); } },
    { "operator-", [](const interval& x, const interval& y) { return S(x - y); } },
    { "operator*", [](const interval& x, const interval& y) { return S(x * y); } },
    { "operator/", [](const interval& x, const interval& y) { return S(x / y); } },
    { "operator+(interval,double)", [](const interval& x, const interval&) { return S(x + 0.1); } },
    { "operator-(double,interval)", [](const interval& x, const interval&) { return S(0.1 - x); } },
    { "operator*(double,interval)", [](const interval& x, const interval&) { return S(0.1 * x); } },
    { "operator/(interval,double)", [](const interval& x, const interval&) { return S(x / 0.3); } },
    { "operator+=(interval)", [](const interval& x, const interval& y) { interval z(x); z += y; return S(z); } },
    { "operator-=(interval)", [](const interval& x, const interval& y) { interval z(x); z -= y; return S(z); } },
    { "operator*=(interval)", [](const interval& x, const interval& y) { interval z(x); z *= y; return S(z); } },
    { "operator/=(interval)", [](const interval& x, const interval& y) { interval z(x); z /= y; return S(z); } },
    { "operator%=(interval)", [](const interval& x, const interval& y) { interval z(x); z %= y; return S(z); } },
    { "operator+=(double)", [](const interval& x, const interval&) { interval z(x); z += 0.1; return S(z); } },
    { "operator-=(double)", [](const interval& x, const interval&) { interval z(x); z -= 0.1; return S(z); } },
    { "operator*=(double)", [](const interval& x, const interval&) { interval z(x); z *= 0.1; return S(z); } },
    { "operator/=(double)", [](const interval& x, const interval&) { interval z(x); z /= 0.1; return S(z); } },
    { "operator%=(double)", [](const interval& x, const interval&) { interval z(x); z %= 0.1; return S(z); } },
    { "operator&=", [](const interval& x, const interval& y) { interval z(x); z &= y; return S(z); } },
    { "operator|=", [](const interval& x, const interval& y) { interval z(x); z |= y; return S(z); } },
    { "operator/ by an interval containing 0", [](const interval& x, const interval&) { return S(x/interval(-1., 1.)); } },
    { "unary -", [](const interval& x, const interval&) { return S(-x); } },
    { "inverse()", [](const interval&, const interval& y) { return S(y.inverse()); } },
    { "midpoint()", [](const interval& x, const interval&) { return S(x.midpoint()); } },
    { "mid()", [](const interval& x, const interval&) { return S(x.mid()); } },
    { "width()", [](const interval& x, const interval&) { return S(x.width()); } },
    { "rad()", [](const interval& x, const interval&) { return S(x.rad()); } },
    { "mid_rad()", [](const interval& x, const interval&) { double m, r; x.mid_rad(m, r); return S(m) + " " + S(r); } },
    { "mig()", [](const interval& x, const interval&) { return S(x.mig()); } },
    { "mag()", [](const interval& x, const interval&) { return S(x.mag()); } },
    { "smig()", [](const interval& x, const interval&) { return S(x.smig()); } },
    { "split()", [](const interval& x, const interval&) { interval l, r; x.split(l, r); return S(l) + " " + S(r); } },
    { "split_left()", [](const interval& x, const interval&) { return S(x.split_left()); } },
    { "split_right()", [](const interval& x, const interval&) { return S(x.split_right()); } },
    { "is_canonical()", [](const interval& x, const interval&) { return S(x.is_canonical()); } },
    { "certainly_le()", [](const interval& x, const interval& y) { return S(x.certainly_le(y)); } },
    { "set_disjoint()", [](const interval& x, const interval& y) { return S(x.set_disjoint(y)); } },
    { "set_contains()", [](const interval& x, const interval& y) { return S(x.set_contains(y)); } },
    { "operator<=", [](const interval& x, const interval& y) { return S(x <= y); } },
    { "operator<", [](const interval& x, const interval& y) { return S(x < y); } },
    { "operator std::string", [](const interval& x, const interval&) { const std::string s = x; return s; } },
    { "operator<< (bounds)", [](const interval& x, const interval&) { return write(x, interval_format::bounds); } },
    { "operator<< (width)", [](const interval& x, const interval&) { return write(x, interval_format::width); } },
    { "operator<< (center)", [](const interval& x, const interval&) { return write(x, interval_format::center); } },
    { "operator<< (hexa)", [](const interval& x, const interval&) { return write(x, interval_format::hexa); } },
    { "operator<< (agreeing)", [](const interval& x, const interval&) { return write(x, interval_format::agreeing); } },
    { "operator<< of a degenerate interval", [](const interval&, const interval&) { return write(interval(0.1), interval_format::bounds); } },
    { "operator>>", [](const interval&, const interval&) { std::istringstream s("[0.1, 0.3]"); interval z; s >> z; return S(z); } },
    { "sqr", [](const interval& x, const interval&) { return S(sqr(x)); } },
    { "pow(x,int)", [](const interval&, const interval& y) { return S(pow(y, 3)); } },
    { "pow(x,-int)", [](const interval&, const interval& y) { return S(pow(y, -3)); } },
    { "pow(x,interval)", [](const interval& x, const interval& y) { return S(pow(y, x)); } },
    { "pow(x,double)", [](const interval&, const interval& y) { return S(pow(y, 2.5)); } },
    { "pow(x,interval) with a negative base", [](const interval&, const interval&) { return S(pow(interval(-4., 9.), interval(0.5))); } },
    { "sqrt", [](const interval& x, const interval&) { return S(sqrt(x)); } },
    { "nth_root", [](const interval&, const interval& y) { return S(nth_root(y, 3)); } },
    { "exp", [](const interval& x, const interval&) { return S(exp(x)); } },
    { "log", [](const interval& x, const interval&) { return S(log(x)); } },
    { "cos", [](const interval& x, const interval&) { return S(cos(x)); } },
    { "sin", [](const interval& x, const interval&) { return S(sin(x)); } },
    { "tan", [](const interval& x, const interval&) { return S(tan(x)); } },
    { "acos", [](const interval& x, const interval&) { return S(acos(x)); } },
    { "asin", [](const interval& x, const interval&) { return S(asin(x)); } },
    { "atan", [](const interval& x, const interval&) { return S(atan(x)); } },
    { "atan2", [](const interval& x, const interval& y) { return S(atan2(x, y)); } },
    { "atan2 across the half-line y = 0, x < 0", [](const interval& x, const interval&) { return S(atan2(x - x, -abs(x))); } },
    { "cosh", [](const interval& x, const interval&) { return S(cosh(x)); } },
    { "sinh", [](const interval& x, const interval&) { return S(sinh(x)); } },
    { "tanh", [](const interval& x, const interval&) { return S(tanh(x)); } },
    { "acosh", [](const interval&, const interval& y) { return S(acosh(y)); } },
    { "asinh", [](const interval& x, const interval&) { return S(asinh(x)); } },
    { "atanh", [](const interval& x, const interval&) { return S(atanh(x)); } },
    { "abs", [](const interval& x, const interval&) { return S(abs(x)); } },
    { "min", [](const interval& x, const interval& y) { return S(min(x, y)); } },
    { "max", [](const interval& x, const interval& y) { return S(max(x, y)); } },
    { "floor", [](const interval&, const interval& y) { return S(floor(y)); } },
    { "ceil", [](const interval&, const interval& y) { return S(ceil(y)); } },
    { "integer", [](const interval&, const interval& y) { return S(integer(y)); } },
    { "chi", [](const interval& x, const interval&) { return S(chi(x)); } },
    { "hausdorff", [](const interval& x, const interval& y) { return S(hausdorff(x, y)); } },
    { "sqrt_rel", [](const interval& x, const interval& y) { return S(sqrt_rel(y, x)); } },
    { "div_rel", [](const interval& x, const interval& y) { return S(div_rel(y, y, x)); } },
    { "nth_root_rel", [](const interval& x, const interval& y) { return S(nth_root_rel(y, 3, x)); } },
    { "acos_rel", [](const interval& x, const interval& y) { return S(acos_rel(y, x)); } },
    { "asin_rel", [](const interval& x, const interval&) { return S(asin_rel(x, x)); } },
    { "atan_rel", [](const interval& x, const interval&) { return S(atan_rel(x, x)); } },
    { "acosh_rel", [](const interval& x, const interval& y) { return S(acosh_rel(x, y)); } },
    { "asinh_rel", [](const interval& x, const interval&) { return S(asinh_rel(x, x)); } },
    { "atanh_rel", [](const interval& x, const interval&) { return S(atanh_rel(x, x)); } },
    { "invabs_rel", [](const interval& x, const interval& y) { return S(invabs_rel(x, y)); } },
    // The bounds at doubles of the elementary functions (gaol/gaol_double_op.h)
    { "exp_dn(), exp_up()", [](const interval& x, const interval&) { return S(exp_dn(x.left())) + " " + S(exp_up(x.right())); } },
    { "log_dn(), log_up()", [](const interval&, const interval& y) { return S(log_dn(y.left())) + " " + S(log_up(y.right())); } },
    { "sin_dn(), sin_up()", [](const interval& x, const interval&) { return S(sin_dn(x.left())) + " " + S(sin_up(x.right())); } },
    { "cos_dn(), cos_up()", [](const interval& x, const interval&) { return S(cos_dn(x.right())) + " " + S(cos_up(x.left())); } },
    { "tan_dn(), tan_up()", [](const interval& x, const interval&) { return S(tan_dn(x.left())) + " " + S(tan_up(x.right())); } },
    { "asin_dn(), asin_up()", [](const interval& x, const interval&) { return S(asin_dn(x.left())) + " " + S(asin_up(x.right())); } },
    { "acos_dn(), acos_up()", [](const interval& x, const interval&) { return S(acos_dn(x.right())) + " " + S(acos_up(x.left())); } },
    { "atan_dn(), atan_up()", [](const interval&, const interval& y) { return S(atan_dn(y.left())) + " " + S(atan_up(y.right())); } },
    { "atan2_dn(), atan2_up()", [](const interval& x, const interval& y) { return S(atan2_dn(x.left(), y.right())) + " " + S(atan2_up(x.right(), y.left())); } },
    { "nthroot_dn(), nthroot_up()", [](const interval&, const interval& y) { return S(nthroot_dn(y.left(), 0.25)) + " " + S(nthroot_up(y.right(), 0.25)); } },
    { "sinh_dn(), sinh_up()", [](const interval& x, const interval&) { return S(sinh_dn(x.left())) + " " + S(sinh_up(x.right())); } },
    { "cosh_dn(), cosh_up()", [](const interval&, const interval& y) { return S(cosh_dn(y.left())) + " " + S(cosh_up(y.right())); } },
    { "tanh_dn(), tanh_up()", [](const interval& x, const interval&) { return S(tanh_dn(x.left())) + " " + S(tanh_up(x.right())); } },
    { "asinh_dn(), asinh_up()", [](const interval&, const interval& y) { return S(asinh_dn(y.left())) + " " + S(asinh_up(y.right())); } },
    { "acosh_dn(), acosh_up()", [](const interval&, const interval& y) { return S(acosh_dn(y.left())) + " " + S(acosh_up(y.right())); } },
    { "atanh_dn(), atanh_up()", [](const interval& x, const interval&) { return S(atanh_dn(x.left())) + " " + S(atanh_up(x.right())); } },
    { "nb_fp_numbers", [](const interval& x, const interval&) { return S(nb_fp_numbers(x.left(), x.right())); } },
    { "feven", [](const interval&, const interval&) { return S(feven(2.0)); } },
    { "interval::pi()", [](const interval&, const interval&) { return S(interval::pi()); } },
    { "interval::precision(n)", [](const interval&, const interval&) { const int p = interval::precision(17); interval::precision(p); return S(p); } },
  };

  set(directions[0]);
  const State upward = state();

  for (const Operation& op : operations) {
    const std::string name = op.name;
    for (const std::pair<interval, interval>& operand : operands) {
      const interval& x = operand.first;
      const interval& y = operand.second;
      set(directions[0]);
      const std::string reference = run(op, x, y);
      for (const Direction& d : directions) {
        set(d);
        const State before = state();
        const std::string result = run(op, x, y);
        const State after = state();
        const auto describe = [&] { return std::string("rounding direction ") + d.name + ", x=" + hex(x) + " y=" + hex(y); };
        check(name + ": the same result whatever the rounding direction", result == reference,
              [&] { return describe() + ": " + result + " rather than " + reference; });
#if GAOL_PRESERVE_ROUNDING
        check("rounding direction restored after " + name, after == before,
              [&] { return describe() + ": " + text(after) + " after it, " + text(before) + " before"; });
#else
        check("rounding direction unchanged or upward after " + name, after == before || after == upward,
              [&] { return describe() + ": " + text(after) + " after it, " + text(before) + " before"; });
#endif

        // A product and a sum whose exact results are not doubles, as the
        // random mantissas make almost certain: their bounds are the tightest
        // ones only if they are computed upward
        const double a = random(-30, 30), b = random(-30, 30);
        const interval p = interval(a) * interval(b), s = interval(a) + interval(b);
        check("[a]*[b] the tightest enclosure after " + name, is_tightest_enclosure(p, exact(dyadic(a)*dyadic(b))),
              [&] { return describe() + ", a=" + hex(a) + " b=" + hex(b) + ": " + hex(p); });
        check("[a]+[b] the tightest enclosure after " + name, is_tightest_enclosure(s, exact(dyadic(a) + dyadic(b))),
              [&] { return describe() + ", a=" + hex(a) + " b=" + hex(b) + ": " + hex(s); });
      }
    }
  }

  // Products and sums in a loop changing the rounding direction before each
  {
    const std::size_t n = 600, nb_directions = sizeof(directions)/sizeof(directions[0]);
    std::vector<double> a(n), b(n);
    for (std::size_t i = 0; i < n; ++i) {
      a[i] = random(-30, 30);
      b[i] = random(-30, 30);
    }
    std::vector<interval> p(n), s(n);
    products_and_sums_in_a_loop(a, b, p, s);
    set(directions[0]);
    for (std::size_t i = 0; i < n; ++i) {
      const auto describe = [&] { return std::string("rounding direction ") + directions[i % nb_directions].name
                                         + ", a=" + hex(a[i]) + " b=" + hex(b[i]); };
      check("[a]*[b] the tightest enclosure in a loop changing the rounding direction",
            is_tightest_enclosure(p[i], exact(dyadic(a[i])*dyadic(b[i]))), [&] { return describe() + ": " + hex(p[i]); });
      check("[a]+[b] the tightest enclosure in a loop changing the rounding direction",
            is_tightest_enclosure(s[i], exact(dyadic(a[i]) + dyadic(b[i]))), [&] { return describe() + ": " + hex(s[i]); });
    }
  }

  /*
    The floating-point exceptions stay masked (GAOL v5). CORE-MATH's cbrt, pow
    and atan2 keep the exception flags around their work with
    fegetexceptflag() and fesetexceptflag() outside x86-64, and the
    fesetexceptflag() of mingw-w64 for a 32-bit target cleared the mask bits of
    MXCSR along with the flags: an invalid operation then trapped rather than
    raised a flag, and the first comparison of the NaN bounds of an empty
    interval killed the program. Where the SSE control register is readable,
    its mask bits have to be the same after each of these functions, in each
    rounding direction; everywhere, an empty interval has to be told empty
    after them, which is where the program died.
  */
  for (const Direction& d : directions) {
    set(d);
#if GAOL_TESTS_SSE
    const unsigned masks_before = _mm_getcsr() & _MM_MASK_MASK;
#endif
    const interval roots = nth_root(interval(1.0, 8.0), 3);     // cbrt
    const interval powers = pow(interval(1.5, 2.5), interval(0.1, 0.3));
    const interval angles = atan2(interval(1.0, 2.0), interval(3.0, 4.0));
#if GAOL_TESTS_SSE
    const unsigned masks_after = _mm_getcsr() & _MM_MASK_MASK;
    check("the SSE exception masks unchanged by cbrt, pow and atan2", masks_after == masks_before,
          [&] {
            return std::string("rounding direction ") + d.name + ": masks " + std::to_string(masks_after)
                 + " after them, " + std::to_string(masks_before) + " before";
          });
#endif
    const interval empty = interval::emptyset();
    check("an empty interval still told empty after cbrt, pow and atan2", empty.is_empty(),
          [&] { return std::string("rounding direction ") + d.name; });
    check("cbrt, pow and atan2 not empty on their domain",
          !roots.is_empty() && !powers.is_empty() && !angles.is_empty(),
          [&] { return std::string("rounding direction ") + d.name; });
  }

  set(directions[0]);
  const int status = summary();
  gaol::cleanup();
  return status;
}
