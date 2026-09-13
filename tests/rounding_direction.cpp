/*-*-C++-*------------------------------------------------------------------
 * gaol -- NOT Just Another Interval Library
 *--------------------------------------------------------------------------
 * Tests of this fork of GAOL: the rounding direction after each operation.
 *
 * GAOL computes with the rounding direction set upward once for all, by
 * gaol::init() (GAOL_PRESERVE_ROUNDING undefined): every operation has to
 * leave it upward, or the operations after it return wrong bounds. After each
 * operation of GAOL's interface, the rounding direction is checked, then the
 * bounds of a product and a sum, which are the tightest ones only if it is
 * still upward.
 *--------------------------------------------------------------------------
 * gaol is a software distributed WITHOUT ANY WARRANTY. Read the associated
 * COPYING file for information.
 *--------------------------------------------------------------------------*/

#include "gaol_tests.h"

#if defined(__SSE2__) || defined(_M_X64) || (defined(_M_IX86_FP) && _M_IX86_FP >= 2)
#  include <xmmintrin.h>
#  define GAOL_TESTS_SSE 1
#endif

using namespace gaol;
using namespace gaol_tests;

namespace
{
  struct Operation
  {
    const char *name;
    void (*run)(const interval& x, const interval& y);
  };

  // Writes x in the format f
  void write(const interval& x, interval_format::format_t f)
  {
    const interval_format::format_t saved = interval::format();
    interval::format(f);
    std::ostringstream s;
    s << x;
    interval::format(saved);
  }
}

int main()
{
  gaol::init();
  check("rounding upward after gaol::init()", std::fegetround() == FE_UPWARD);

  // x is within [0,1] and y above 1, which is the domain of every function below
  const interval x(0.1, 0.3), y(1.5, 2.5);

  const Operation operations[] = {
    { "interval()", [](const interval&, const interval&) { interval z; (void)z; } },
    { "interval(a,b)", [](const interval&, const interval&) { (void)interval(0.1, 1./3.); } },
    { "interval(a)", [](const interval&, const interval&) { (void)interval(0.1); } },
    { "interval(const char*)", [](const interval&, const interval&) { (void)interval("[0.1, 1/3]"); } },
    { "interval(const char*, const char*)", [](const interval&, const interval&) { (void)interval("0.1", "0.3"); } },
    { "interval(\"sin(1)+exp(0.1)\")", [](const interval&, const interval&) { (void)interval("sin(1)+exp(0.1)"); } },
    { "operator+=(interval)", [](const interval& x, const interval& y) { interval z(x); z += y; } },
    { "operator-=(interval)", [](const interval& x, const interval& y) { interval z(x); z -= y; } },
    { "operator*=(interval)", [](const interval& x, const interval& y) { interval z(x); z *= y; } },
    { "operator/=(interval)", [](const interval& x, const interval& y) { interval z(x); z /= y; } },
    { "operator%=(interval)", [](const interval& x, const interval& y) { interval z(x); z %= y; } },
    { "operator+=(double)", [](const interval& x, const interval&) { interval z(x); z += 0.1; } },
    { "operator-=(double)", [](const interval& x, const interval&) { interval z(x); z -= 0.1; } },
    { "operator*=(double)", [](const interval& x, const interval&) { interval z(x); z *= 0.1; } },
    { "operator/=(double)", [](const interval& x, const interval&) { interval z(x); z /= 0.1; } },
    { "operator%=(double)", [](const interval& x, const interval&) { interval z(x); z %= 0.1; } },
    { "operator&=", [](const interval& x, const interval& y) { interval z(x); z &= y; } },
    { "operator|=", [](const interval& x, const interval& y) { interval z(x); z |= y; } },
    { "operator/ by an interval containing 0", [](const interval& x, const interval&) { (void)(x/interval(-1., 1.)); } },
    { "unary -", [](const interval& x, const interval&) { (void)-x; } },
    { "inverse()", [](const interval&, const interval& y) { (void)y.inverse(); } },
    { "midpoint()", [](const interval& x, const interval&) { (void)x.midpoint(); } },
    { "mid()", [](const interval& x, const interval&) { (void)x.mid(); } },
    { "width()", [](const interval& x, const interval&) { (void)x.width(); } },
    { "mig()", [](const interval& x, const interval&) { (void)x.mig(); } },
    { "mag()", [](const interval& x, const interval&) { (void)x.mag(); } },
    { "smig()", [](const interval& x, const interval&) { (void)x.smig(); } },
    { "split()", [](const interval& x, const interval&) { interval l, r; x.split(l, r); } },
    { "split_left()", [](const interval& x, const interval&) { (void)x.split_left(); } },
    { "split_right()", [](const interval& x, const interval&) { (void)x.split_right(); } },
    { "is_canonical()", [](const interval& x, const interval&) { (void)x.is_canonical(); } },
    { "certainly_le()", [](const interval& x, const interval& y) { (void)x.certainly_le(y); } },
    { "possibly_eq()", [](const interval& x, const interval& y) { (void)x.possibly_eq(y); } },
    { "set_contains()", [](const interval& x, const interval& y) { (void)x.set_contains(y); } },
    { "operator==", [](const interval& x, const interval& y) { (void)(x == y); } },
    { "operator<", [](const interval& x, const interval& y) { (void)(x < y); } },
    { "operator std::string", [](const interval& x, const interval&) { const std::string s = x; (void)s; } },
    { "operator<< (bounds)", [](const interval& x, const interval&) { write(x, interval_format::bounds); } },
    { "operator<< (width)", [](const interval& x, const interval&) { write(x, interval_format::width); } },
    { "operator<< (center)", [](const interval& x, const interval&) { write(x, interval_format::center); } },
    { "operator<< (hexa)", [](const interval& x, const interval&) { write(x, interval_format::hexa); } },
    { "operator<< (agreeing)", [](const interval& x, const interval&) { write(x, interval_format::agreeing); } },
    { "operator<< of a degenerate interval", [](const interval&, const interval&) { write(interval(0.1), interval_format::bounds); } },
    { "operator>>", [](const interval&, const interval&) { std::istringstream s("[0.1, 0.3]"); interval z; s >> z; } },
    { "sqr", [](const interval& x, const interval&) { (void)sqr(x); } },
    { "pow(x,int)", [](const interval&, const interval& y) { (void)pow(y, 3); } },
    { "pow(x,-int)", [](const interval&, const interval& y) { (void)pow(y, -3); } },
    { "pow(x,interval)", [](const interval& x, const interval& y) { (void)pow(y, x); } },
    { "pow(x,double)", [](const interval&, const interval& y) { (void)pow(y, 2.5); } },
    { "pow(x,interval) with a negative base", [](const interval&, const interval&) { (void)pow(interval(-4., 9.), interval(0.5)); } },
    { "sqrt", [](const interval& x, const interval&) { (void)sqrt(x); } },
    { "nth_root", [](const interval&, const interval& y) { (void)nth_root(y, 3); } },
    { "exp", [](const interval& x, const interval&) { (void)exp(x); } },
    { "log", [](const interval& x, const interval&) { (void)log(x); } },
    { "cos", [](const interval& x, const interval&) { (void)cos(x); } },
    { "sin", [](const interval& x, const interval&) { (void)sin(x); } },
    { "tan", [](const interval& x, const interval&) { (void)tan(x); } },
    { "acos", [](const interval& x, const interval&) { (void)acos(x); } },
    { "asin", [](const interval& x, const interval&) { (void)asin(x); } },
    { "atan", [](const interval& x, const interval&) { (void)atan(x); } },
    { "cosh", [](const interval& x, const interval&) { (void)cosh(x); } },
    { "sinh", [](const interval& x, const interval&) { (void)sinh(x); } },
    { "tanh", [](const interval& x, const interval&) { (void)tanh(x); } },
    { "acosh", [](const interval&, const interval& y) { (void)acosh(y); } },
    { "asinh", [](const interval& x, const interval&) { (void)asinh(x); } },
    { "atanh", [](const interval& x, const interval&) { (void)atanh(x); } },
    { "abs", [](const interval& x, const interval&) { (void)abs(x); } },
    { "min", [](const interval& x, const interval& y) { (void)min(x, y); } },
    { "max", [](const interval& x, const interval& y) { (void)max(x, y); } },
    { "floor", [](const interval&, const interval& y) { (void)floor(y); } },
    { "ceil", [](const interval&, const interval& y) { (void)ceil(y); } },
    { "integer", [](const interval&, const interval& y) { (void)integer(y); } },
    { "chi", [](const interval& x, const interval&) { (void)chi(x); } },
    { "hausdorff", [](const interval& x, const interval& y) { (void)hausdorff(x, y); } },
    { "sqrt_rel", [](const interval& x, const interval& y) { (void)sqrt_rel(y, x); } },
    { "div_rel", [](const interval& x, const interval& y) { (void)div_rel(y, y, x); } },
    { "nth_root_rel", [](const interval& x, const interval& y) { (void)nth_root_rel(y, 3, x); } },
    { "acos_rel", [](const interval& x, const interval& y) { (void)acos_rel(y, x); } },
    { "asin_rel", [](const interval& x, const interval&) { (void)asin_rel(x, x); } },
    { "atan_rel", [](const interval& x, const interval&) { (void)atan_rel(x, x); } },
    { "acosh_rel", [](const interval& x, const interval& y) { (void)acosh_rel(x, y); } },
    { "asinh_rel", [](const interval& x, const interval&) { (void)asinh_rel(x, x); } },
    { "atanh_rel", [](const interval& x, const interval&) { (void)atanh_rel(x, x); } },
    { "invabs_rel", [](const interval& x, const interval& y) { (void)invabs_rel(x, y); } },
    { "nb_fp_numbers", [](const interval& x, const interval&) { (void)nb_fp_numbers(x.left(), x.right()); } },
    { "feven", [](const interval&, const interval&) { (void)feven(2.0); } },
    { "interval::pi()", [](const interval&, const interval&) { (void)interval::pi(); } },
    { "interval::precision(n)", [](const interval&, const interval&) { interval::precision(interval::precision(17)); } },
  };

  Random random;
  for (const Operation& op : operations) {
    const std::string after = std::string(" after ") + op.name;
    evaluate(op.name, [&] { op.run(x, y); return x; }, [] { return std::string(); });
    check("rounding upward" + after, std::fegetround() == FE_UPWARD);
#if GAOL_TESTS_SSE
    check("SSE rounding upward" + after, (_mm_getcsr() & _MM_ROUND_MASK) == _MM_ROUND_UP);
#endif

    // A product and a sum whose exact results are not doubles, as the random
    // mantissas make almost certain: their bounds are the tightest ones only if
    // the rounding direction is still upward
    const double a = random(-30, 30), b = random(-30, 30);
    const interval p = interval(a) * interval(b), s = interval(a) + interval(b);
    check("[a]*[b] the tightest enclosure" + after, is_tightest_enclosure(p, exact(dyadic(a)*dyadic(b))),
          [&] { return "a=" + hex(a) + " b=" + hex(b) + ": " + hex(p); });
    check("[a]+[b] the tightest enclosure" + after, is_tightest_enclosure(s, exact(dyadic(a) + dyadic(b))),
          [&] { return "a=" + hex(a) + " b=" + hex(b) + ": " + hex(s); });
  }

  const int status = summary();
  gaol::cleanup();
  return status;
}
