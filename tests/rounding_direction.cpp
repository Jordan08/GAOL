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
 * be the tightest ones. Powers with a subnormal result, which CORE-MATH rounds
 * itself in the direction fegetround() gives, have to be the tightest
 * enclosures whatever the direction, with the x87 unit and the SSE
 * instructions differing too (GAOL v5). On x86 and ARM, the same holds with
 * the modes that flush the subnormals to zero set, flush-to-zero and
 * denormals-are-zero of the SSE instructions, FZ of ARM (what a program linked
 * with -Ofast gets from crtfastmath.o, and a plug-in built so): the bounds of
 * the operations with a subnormal operand or result are the tightest ones, and
 * the modes are cleared after them, or restored with GAOL_PRESERVE_ROUNDING;
 * and every operation that checks the rounding direction gives, with a mode
 * set before it, what it gives with the modes cleared, and clears or restores
 * the modes (GAOL v5). At the end, gaol::cleanup() has to set back the
 * direction the first gaol::init() found (GAOL v5).
 *
 * The rest of the floating-point environment is checked too: the exceptions
 * stay masked, and an empty interval is told empty, with interval::emptyset(),
 * and every operation computes with an empty operand, without raising the
 * invalid-operation exception, which kills a program that enabled it (GAOL v5).
 * Nor do a NaN the program gives, the products with a zero and an infinite
 * bound, pow with an exponent of extreme magnitude and the relations in loops
 * a compiler vectorizes raise it, nor the midpoint of [DBL_MAX] the overflow
 * exception; every operation keeps the exception flags and masks of the
 * program (GAOL v5, point D.24 of TODO.md).
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
#include <cstring>
#include <exception>
#include <functional>
#include <initializer_list>
#include <sstream>
#include <string>
#include <utility>
#include <vector>

// A program that enables the floating-point exceptions of the processor
// (feenableexcept(), a GNU extension of glibc's <fenv.h>), run in a child
// process (fork(), POSIX) that a signal kills without stopping the test: glibc
// only. Elsewhere the traps are not tested, and the test says so.
#if defined(__GLIBC__) && defined(_GNU_SOURCE) && defined(__unix__)
#  include <csignal>
#  include <cerrno>
#  include <sys/resource.h>
#  include <sys/types.h>
#  include <sys/wait.h>
#  include <unistd.h>
#  define GAOL_TESTS_TRAPS 1
#else
#  define GAOL_TESTS_TRAPS 0
#endif

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
  // An operation of GAOL computed before main(), in the initialization of a
  // static object: it sets the rounding direction upward. GAOL has to
  // initialize itself before, so that the first gaol::init() finds the
  // direction the program started with: with GCC and Clang, the constructor of
  // the static library GAOL ran after those of the program, except with
  // MinGW-w64, and gaol::cleanup() left the direction upward (GAOL v5).
  const interval computed_before_main = interval(0.1, 0.3) * interval(1.5, 2.5);

  // The results of the operations, written exactly
  std::string S(const interval& x) { return hex(x); }
  std::string S(double x) { return hex(x); }
  std::string S(bool b) { return b ? "true" : "false"; }
  std::string S(int n) { return std::to_string(n); }
  std::string S(unsigned long long n) { return std::to_string(n); }

  struct Operation
  {
    const char *name;
    std::string (*run)(const interval& x, const interval& y);
  };

  // Powers x^y with a result below the smallest normal double, which CORE-MATH
  // rounds by itself, in the direction fegetround() gives (GAOL v5). Their
  // tightest enclosure [lo, hi] is written as multiples of the smallest
  // subnormal, 2^-1074, and was computed apart from GAOL, with mpmath at 600
  // bits: exp(y*log(x)), rounded down and up to a multiple of 2^-1074. The
  // exact values are 1458662373659.29, 567863.26 and 1.30 of them: with a
  // fractional part below one half, rounding them to nearest, rather than
  // upward, gives a value below the exact one.
  struct SubnormalPower
  {
    double x, y;
    long long lo, hi;
  };

  const SubnormalPower subnormal_powers[] = {
    { 0x1.4a1249ba9b6d5p-1, 0x1.97f9e85175cf7p+10, 1458662373659LL, 1458662373660LL },
    { 0x1.0d00c0deaf87fp-2, 0x1.117ee54c23de1p+9, 567863LL, 567864LL },
    { 0x1.999999999999ap-1, 0x1.a0df226893p+11, 1LL, 2LL },
  };

  // A multiple of 2^-1074, exact in every rounding direction
  double subnormal(long long multiple) { return static_cast<double>(multiple) * 0x1p-1074; }

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

  // An interval the compiler knows nothing about: it would else compute at
  // compile time that the empty sets below are empty, and the comparison of
  // their NaN bounds, which is what raises the exception, would never run
  interval opaque(const interval& x)
  {
    return gaol::rnd_keep(x);
  }

  // An interval computed by make() from operands the compiler does not know
  struct NamedInterval
  {
    const char *name;
    interval (*make)();
  };

  // The empty sets of the checks below, each computed in a way of its own:
  // the constant, an interval whose bounds are in the wrong order, a function
  // outside its domain, disjoint intervals. None of them compares a NaN, so
  // that each is computed without an exception with the invalid-operation
  // exception enabled.
  const NamedInterval empty_sets[] = {
    { "interval::emptyset()", [] { return opaque(interval::emptyset()); } },
    { "interval(3, 2)", [] { return opaque(interval(gaol::rnd_keep(3.0), gaol::rnd_keep(2.0))); } },
    { "sqrt([-2, -1])", [] { return opaque(sqrt(interval(gaol::rnd_keep(-2.0), gaol::rnd_keep(-1.0)))); } },
    { "log([-2, -1])", [] { return opaque(log(interval(gaol::rnd_keep(-2.0), gaol::rnd_keep(-1.0)))); } },
    { "[1, 2] & [3, 4]", [] {
        return opaque(interval(gaol::rnd_keep(1.0), gaol::rnd_keep(2.0)) & interval(gaol::rnd_keep(3.0), gaol::rnd_keep(4.0))); } },
    { "[3, 4] & [1, 2]", [] {
        return opaque(interval(gaol::rnd_keep(3.0), gaol::rnd_keep(4.0)) & interval(gaol::rnd_keep(1.0), gaol::rnd_keep(2.0))); } },
  };

  // The nonempty intervals is_empty() has to tell nonempty as well
  const NamedInterval nonempty_sets[] = {
    { "[1, 2]", [] { return opaque(interval(gaol::rnd_keep(1.0), gaol::rnd_keep(2.0))); } },
    { "[1]", [] { return opaque(interval(gaol::rnd_keep(1.0))); } },
    { "[-oo, +oo]", [] { return opaque(interval::universe()); } },
  };

  // The empty set and [-1, 2], operands the compiler knows nothing about
  interval E() { return opaque(interval::emptyset()); }
  interval X() { return opaque(interval(gaol::rnd_keep(-1.0), gaol::rnd_keep(2.0))); }

  /*
    The operations of GAOL's interface with an empty operand, each on either
    side, run with the invalid-operation exception enabled below (GAOL v5).
    Each returns whether its result is the one it gives for the empty set.
    The first 48 compared a bound of the empty set with <, <=, >= or >, or
    gave its NaN bounds to the constructor, which compares them: each raised
    the exception, and died on SIGFPE where it was enabled (all 48 with the
    FPU intervals, all but the negations with the SSE2 ones). The groups after
    them were already quiet. The last five are choices the program makes on
    is_empty() of an empty interval: GCC for 32-bit ARM made them conditional
    moves with a signaling comparison, of the NaN bounds of the left operand
    of an intersection when operator&= told that operand empty otherwise than
    is_empty() did (#60), and of the bounds of an empty interval itself in the
    choice on its own is_empty(), and GCC for POWER9 (-mcpu=power9) too, where
    is_empty() was one quiet comparison (GAOL v5, point D.24 of TODO.md).
  */
  struct EmptyOperand
  {
    const char *name;
    bool (*run)();
  };

  const EmptyOperand empty_operands[] = {
    // The predicates and the relations
    { "is_canonical()", [] { return !E().is_canonical(); } },
    { "straddles_zero()", [] { return !E().straddles_zero(); } },
    { "strictly_straddles_zero()", [] { return !E().strictly_straddles_zero(); } },
    { "set_contains(double)", [] { return !E().set_contains(1.5); } },
    { "set_strictly_contains(double)", [] { return !E().set_strictly_contains(1.5); } },
    { "gaol_ieee1788::isMember(m, empty)", [] { return !gaol_ieee1788::isMember(1.5, E()); } },
    { "empty.set_contains(x)", [] { return !E().set_contains(X()); } },
    { "empty.set_geq(x)", [] { return !E().set_geq(X()); } },
    { "empty.set_disjoint(x)", [] { return E().set_disjoint(X()); } },
    { "x.set_disjoint(empty)", [] { return X().set_disjoint(E()); } },
    { "gaol_ieee1788::disjoint(empty, x)", [] { return gaol_ieee1788::disjoint(E(), X()); } },
    { "x.set_leq(empty)", [] { return !X().set_leq(E()); } },
    { "gaol_ieee1788::subset(x, empty)", [] { return !gaol_ieee1788::subset(X(), E()); } },
    // The intersection with an empty right operand, and the functions
    // intersecting their result with it
    { "x & empty", [] { return (X() & E()).is_empty(); } },
    { "x &= empty", [] { interval z = X(); z &= E(); return z.is_empty(); } },
    { "gaol_ieee1788::intersection(x, empty)", [] { return gaol_ieee1788::intersection(X(), E()).is_empty(); } },
    { "nth_root_rel(x, 1, empty)", [] { return nth_root_rel(X(), 1u, E()).is_empty(); } },
    { "asinh_rel(x, empty)", [] { return asinh_rel(X(), E()).is_empty(); } },
    { "atanh_rel(x, empty)", [] { return atanh_rel(X(), E()).is_empty(); } },
    { "invabs_rel(x, empty)", [] { return invabs_rel(X(), E()).is_empty(); } },
    // The functions that built their result from the NaN bounds
    { "sqrt(empty)", [] { return sqrt(E()).is_empty(); } },
    { "nth_root(empty, 2)", [] { return nth_root(E(), 2u).is_empty(); } },
    { "nth_root(empty, -2)", [] { return nth_root(E(), -2).is_empty(); } },
    { "sqrt_rel(empty, x)", [] { return sqrt_rel(E(), X()).is_empty(); } },
    { "sqrt_rel(empty, empty)", [] { return sqrt_rel(E(), E()).is_empty(); } },
    { "nth_root_rel(empty, 2, x)", [] { return nth_root_rel(E(), 2u, X()).is_empty(); } },
    { "gaol_ieee1788::sqrRev(empty)", [] { return gaol_ieee1788::sqrRev(E()).is_empty(); } },
    { "exp(empty)", [] { return exp(E()).is_empty(); } },
    { "min(empty, x)", [] { return min(E(), X()).is_empty(); } },
    { "min(x, empty)", [] { return min(X(), E()).is_empty(); } },
    { "max(empty, x)", [] { return max(E(), X()).is_empty(); } },
    { "max(x, empty)", [] { return max(X(), E()).is_empty(); } },
    { "floor(empty)", [] { return floor(E()).is_empty(); } },
    { "ceil(empty)", [] { return ceil(E()).is_empty(); } },
    { "integer(empty)", [] { return integer(E()).is_empty(); } },
    { "split()", [] { interval l, r; E().split(l, r); return l.is_empty() && r.is_empty(); } },
    { "split_left()", [] { return E().split_left().is_empty(); } },
    { "split_right()", [] { return E().split_right().is_empty(); } },
    { "modulo_k_pi(empty)", [] {
        double k_left = 0.0, k_right = 0.0;
        return modulo_k_pi(E(), k_left, k_right) == 0 && std::isnan(k_left) && std::isnan(k_right); } },
    { "textToInterval(\"[empty]\", \"[1, 2]\")", [] { return textToInterval("[empty]", "[1, 2]").is_empty(); } },
    { "textToInterval(\"[1, 2]\", \"[empty]\")", [] { return textToInterval("[1, 2]", "[empty]").is_empty(); } },
    // The negation of the FPU intervals, and cancel_plus, which negates
    { "-empty", [] { return (-E()).is_empty(); } },
    { "gaol_ieee1788::neg(empty)", [] { return gaol_ieee1788::neg(E()).is_empty(); } },
    { "cancel_plus(x, empty)", [] { return cancel_plus(X(), E()).is_entire(); } },
    // The outputs
    { "chi(empty)", [] { return std::isnan(chi(E())); } },
    { "operator<<", [] { return write(E(), interval_format::bounds) == "[empty]"; } },
    { "operator std::string", [] { const std::string s = E(); return s == "[empty]"; } },
    { "gaol_ieee1788::intervalToText(empty)", [] { return gaol_ieee1788::intervalToText(E()) == "[empty]"; } },
    // Already quiet
    { "the arithmetic operations", [] {
        return (E() + X()).is_empty() && (X() + E()).is_empty() && (E() - X()).is_empty() && (X() - E()).is_empty()
          && (E() * X()).is_empty() && (X() * E()).is_empty() && (E() / X()).is_empty() && (X() / E()).is_empty()
          && (E() % X()).is_empty() && (X() % E()).is_empty() && (E() + 1.5).is_empty() && (1.5 - E()).is_empty()
          && (E() * 0.0).is_empty() && (1.5 / E()).is_empty() && (E() & X()).is_empty() && (E() | X()).set_eq(X())
          && (X() | E()).set_eq(X()) && inverse(E()).is_empty() && sqr(E()).is_empty()
          && fma(E(), X(), X()).is_empty() && fma(X(), E(), X()).is_empty() && fma(X(), X(), E()).is_empty()
          && cancel_minus(E(), X()).is_empty() && cancel_minus(X(), E()).is_entire(); } },
    { "the powers and the roots", [] {
        return pow(E(), 3).is_empty() && pow(E(), -2).is_empty() && pow(E(), 0).is_empty() && pow(E(), 2u).is_empty()
          && pow(E(), 2.5).is_empty() && pow(E(), X()).is_empty() && pow(X(), E()).is_empty()
          && gaol_ieee1788::pow(E(), X()).is_empty() && gaol_ieee1788::pow(X(), E()).is_empty()
          && nth_root(E(), 3u).is_empty() && nth_root(E(), 4u).is_empty() && nth_root(E(), -3).is_empty(); } },
    { "the exponentials and the logarithms", [] {
        return log(E()).is_empty() && exp2(E()).is_empty() && exp10(E()).is_empty() && log2(E()).is_empty()
          && log10(E()).is_empty() && expm1(E()).is_empty() && exp2m1(E()).is_empty() && exp10m1(E()).is_empty()
          && log1p(E()).is_empty() && log2p1(E()).is_empty() && log10p1(E()).is_empty() && rsqrt(E()).is_empty(); } },
    { "the trigonometric and hyperbolic functions", [] {
        return cos(E()).is_empty() && sin(E()).is_empty() && tan(E()).is_empty() && acos(E()).is_empty()
          && asin(E()).is_empty() && atan(E()).is_empty() && atan2(E(), X()).is_empty() && atan2(X(), E()).is_empty()
          && sinpi(E()).is_empty() && cospi(E()).is_empty() && tanpi(E()).is_empty() && asinpi(E()).is_empty()
          && acospi(E()).is_empty() && atanpi(E()).is_empty() && atan2pi(E(), X()).is_empty()
          && atan2pi(X(), E()).is_empty() && cosh(E()).is_empty() && sinh(E()).is_empty() && tanh(E()).is_empty()
          && acosh(E()).is_empty() && asinh(E()).is_empty() && atanh(E()).is_empty() && hypot(E(), X()).is_empty()
          && hypot(X(), E()).is_empty(); } },
    { "abs, sign and the roundings", [] {
        return abs(E()).is_empty() && sign(E()).is_empty() && trunc(E()).is_empty()
          && round_ties_to_even(E()).is_empty() && round_ties_to_away(E()).is_empty(); } },
    { "the reverse functions", [] {
        return acos_rel(E(), X()).is_empty() && acos_rel(X(), E()).is_empty() && asin_rel(E(), X()).is_empty()
          && asin_rel(X(), E()).is_empty() && atan_rel(E(), X()).is_empty() && atan_rel(X(), E()).is_empty()
          && acosh_rel(E(), X()).is_empty() && acosh_rel(X(), E()).is_empty() && asinh_rel(E(), X()).is_empty()
          && atanh_rel(E(), X()).is_empty() && invabs_rel(E(), X()).is_empty() && sqrt_rel(X(), E()).is_empty()
          && nth_root_rel(E(), 3u, X()).is_empty() && nth_root_rel(X(), 3u, E()).is_empty()
          && div_rel(E(), X(), X()).is_empty() && div_rel(X(), E(), X()).is_empty()
          && div_rel(X(), X(), E()).is_empty(); } },
    { "the numeric functions", [] {
        double m, r;
        E().mid_rad(m, r);
        return std::isnan(E().midpoint()) && E().mid().is_empty() && std::isnan(E().width()) && std::isnan(E().rad())
          && std::isnan(m) && std::isnan(r) && std::isnan(E().mig()) && std::isnan(E().mag()) && std::isnan(E().smig())
          && std::isnan(hausdorff(E(), X())) && std::isnan(hausdorff(X(), E()))
          && gaol_ieee1788::inf(E()) == GAOL_INFINITY && gaol_ieee1788::sup(E()) == -GAOL_INFINITY; } },
    { "the other predicates", [] {
        return E().certainly_positive() && E().certainly_negative() && E().certainly_strictly_positive()
          && E().certainly_strictly_negative() && !E().is_entire() && !E().is_common_interval()
          && !E().is_symmetric() && !E().is_zero() && !E().is_a_double() && !E().is_an_int(); } },
    { "the other relations", [] {
        return E().certainly_leq(X()) && X().certainly_leq(E()) && E().certainly_le(X()) && X().certainly_le(E())
          && E().certainly_geq(X()) && X().certainly_geq(E()) && E().certainly_ge(X()) && X().certainly_ge(E())
          && (E() <= X()) && (X() < E()) && (E() >= X()) && (X() > E()) && X().set_contains(E())
          && E().set_contains(E()) && !E().set_strictly_contains(X()) && X().set_strictly_contains(E())
          && !E().set_eq(X()) && E().set_neq(X()) && E().set_leq(X()) && E().set_le(X()) && !X().set_le(E())
          && !E().less(X()) && !X().strictly_less(E()); } },
    { "the other outputs", [] {
        return write(E(), interval_format::width) == "[empty]" && write(E(), interval_format::center) == "[empty]"
          && write(E(), interval_format::hexa) == "[empty]" && write(E(), interval_format::agreeing) == "[empty]"
          && exact_string(E()) == "[empty]"; } },
    // A value the program chooses with is_empty() of an empty interval: GCC
    // for 32-bit ARM made the choice a conditional move, and compared the NaN
    // bounds with the signaling vcmpe, after an intersection whose left
    // operand is empty where it did not know the answer from operator&= (#60),
    // and on the empty interval alone, as GCC for POWER9 did with xscmpgedp;
    // is_empty() now tells the NaN bounds with std::isunordered() first there
    // (see gaol/gaol_interval.h). Where the compiler optimizes for size (-Os),
    // GCC 14 for 32-bit ARM and GCC 13 for POWER9 call operator& or operator&=
    // rather than inlining it, and the choice is then made on is_empty() of an
    // empty interval alone: these choices were not checked there
    // (GAOL_TESTS_CHOICES) until jobs of the continuous integration built the
    // tests for size, on armhf and for POWER9 (containers.yml)
    { "(empty & x).is_empty() ? 0 : right()", [] {
        const interval z = E() & X();
        const volatile double r = z.is_empty() ? 0.0 : z.right();
        return r == 0.0; } },
    { "(empty & empty).is_empty() ? 0 : right()", [] {
        const interval z = E() & E();
        const volatile double r = z.is_empty() ? 0.0 : z.right();
        return r == 0.0; } },
    { "gaol_ieee1788::intersection(empty, x).is_empty() ? 0 : right()", [] {
        const interval z = gaol_ieee1788::intersection(E(), X());
        const volatile double r = z.is_empty() ? 0.0 : z.right();
        return r == 0.0; } },
    { "empty.is_empty() ? 0 : right()", [] {
        const interval z = E();
        const volatile double r = z.is_empty() ? 0.0 : z.right();
        return r == 0.0; } },
    { "(x & empty).is_empty() ? 0 : right()", [] {
        const interval z = X() & E();
        const volatile double r = z.is_empty() ? 0.0 : z.right();
        return r == 0.0; } },
  };

  /*
    A NaN the program gives GAOL, which interval(d) makes the empty set, to
    the constructors, the operations and the relations with a double (GAOL v5,
    point D.24 of TODO.md): the first comparison of the constructors, and of
    the operations with a double testing d, compared the NaN with <, <= or >,
    which raised the invalid-operation exception, and so did pow(x, d). Each
    returns whether its result is the one of the empty set.
  */
  double nan_double() { return gaol::rnd_keep(std::numeric_limits<double>::quiet_NaN()); }

  const EmptyOperand nan_operands[] = {
    { "interval(NAN)", [] { return interval(nan_double()).is_empty(); } },
    { "interval(NAN, 1)", [] { return interval(nan_double(), gaol::rnd_keep(1.0)).is_empty(); } },
    { "interval(1, NAN)", [] { return interval(gaol::rnd_keep(1.0), nan_double()).is_empty(); } },
    { "interval(NAN, NAN)", [] { return interval(nan_double(), nan_double()).is_empty(); } },
    { "x = NAN", [] { interval z = X(); z = nan_double(); return z.is_empty(); } },
    { "x += NAN", [] { interval z = X(); z += nan_double(); return z.is_empty(); } },
    { "x -= NAN", [] { interval z = X(); z -= nan_double(); return z.is_empty(); } },
    { "x *= NAN", [] { interval z = X(); z *= nan_double(); return z.is_empty(); } },
    { "x /= NAN", [] { interval z = X(); z /= nan_double(); return z.is_empty(); } },
    { "x %= NAN", [] { interval z = X(); z %= nan_double(); return z.is_empty(); } },
    { "x &= NAN", [] { interval z = X(); z &= nan_double(); return z.is_empty(); } },
    { "x |= NAN", [] { interval z = X(); z |= nan_double(); return z.set_eq(X()); } },
    { "x + NAN, NAN - x, x * NAN, NAN / x, x % NAN", [] {
        return (X() + nan_double()).is_empty() && (nan_double() - X()).is_empty() && (X()*nan_double()).is_empty()
          && (nan_double()/X()).is_empty() && (X() % nan_double()).is_empty(); } },
    { "x <= NAN, NAN < x, x >= NAN, NAN > x", [] {
        return (X() <= nan_double()) && (nan_double() < X()) && (X() >= nan_double()) && (nan_double() > X()); } },
    { "set_contains(NAN)", [] { return !X().set_contains(nan_double()); } },
    { "set_strictly_contains(NAN)", [] { return !X().set_strictly_contains(nan_double()); } },
    { "gaol_ieee1788::isMember(NAN, x)", [] { return !gaol_ieee1788::isMember(nan_double(), X()); } },
    { "pow(x, NAN)", [] { return pow(X(), nan_double()).is_empty(); } },
  };

  /*
    Products of intervals with a zero bound and an infinite one (GAOL v5,
    point D.24): the product of the SSE2 intervals multiplied the bounds
    pairwise, 0 by +oo among them, which raised the invalid-operation
    exception, and made a 0 of the NaN afterwards: [0]*[1, +oo] and
    [0, +oo]*[0] raised it, and so did pow([1], [1, +oo]) through them. The
    bounds of every product of two intervals with bounds in
    {-oo, -2, -0, 0, 3, +oo} are compared with the extrema of the products of
    their bounds, a zero bound and an infinite one giving 0 (IEEE 1788-2015),
    computed on these small integers, where they are exact.
  */
  double bound_product(double a, double b)
  {
    return (a == 0.0 || b == 0.0) ? 0.0 : a*b;
  }

  // The nonempty intervals with bounds in bounds
  std::vector<interval> intervals_with_bounds(std::initializer_list<double> bounds)
  {
    std::vector<interval> intervals;
    for (double l : bounds) {
      for (double r : bounds) {
        const interval z(l, r);
        if (!z.is_empty()) {
          intervals.push_back(z);
        }
      }
    }
    return intervals;
  }

  bool products_with_zero_and_infinite_bounds_are_right()
  {
    const std::vector<interval> intervals = intervals_with_bounds({ -GAOL_INFINITY, -2.0, -0.0, 0.0, 3.0, GAOL_INFINITY });
    bool right = true;
    for (const interval& x : intervals) {
      for (const interval& y : intervals) {
        const double p[4] = { bound_product(x.left(), y.left()), bound_product(x.left(), y.right()),
                              bound_product(x.right(), y.left()), bound_product(x.right(), y.right()) };
        const interval z = opaque(x)*opaque(y);
        right = right && z.left() == std::min(std::min(p[0], p[1]), std::min(p[2], p[3]))
                && z.right() == std::max(std::max(p[0], p[1]), std::max(p[2], p[3]));
      }
    }
    return right && pow(opaque(interval(1.0)), opaque(interval(1.0, GAOL_INFINITY))).set_eq(interval(1.0));
  }

  /*
    div_rel(K, J, [-oo, +oo]) for every K and J with bounds in
    {-oo, -2, -0, 0, 4, +oo} (GAOL v5, point D.24): with the SSE2 intervals,
    the branches keeping one half of a division divided both halves, and the
    half thrown away divided +oo by +oo, which raised the invalid-operation
    exception (div_rel([4, +oo], [-oo, 4], I)), or a nonzero bound by a zero
    one, which raised the division-by-zero one (div_rel([-2], [-2, 0], I)).
    Where J does not contain 0, the bounds are compared with the extrema of
    the quotients of the bounds, exact on these, the quotient of two infinite
    bounds left out (any positive number is a limit of such quotients, and
    the others reach 0 and +oo then), and computed apart from GAOL, without
    raising anything.
  */
  bool quotients_with_zero_and_infinite_bounds_are_right()
  {
    const std::vector<interval> intervals = intervals_with_bounds({ -GAOL_INFINITY, -2.0, -0.0, 0.0, 4.0, GAOL_INFINITY });
    bool right = true;
    for (const interval& k : intervals) {
      for (const interval& j : intervals) {
        const interval z = div_rel(opaque(k), opaque(j), opaque(interval::universe()));
        if (j.left() > 0.0 || j.right() < 0.0) {
          double l = GAOL_INFINITY, r = -GAOL_INFINITY;
          for (double a : { k.left(), k.right() }) {
            for (double b : { j.left(), j.right() }) {
              if (!(std::isinf(a) && std::isinf(b))) {
                const double q = a/b;
                l = std::min(l, q);
                r = std::max(r, q);
              }
            }
          }
          right = right && z.left() == l && z.right() == r;
        }
      }
    }
    return right;
  }

  /*
    pow for an exponent of extreme magnitude (GAOL v5, point D.24): CORE-MATH's
    pow makes its approximation of log(x) a NaN on purpose for |y| < 2^-969 and
    |y| >= 2^1014, and compares the NaN product with <, which raised the
    invalid-operation exception at the corners of these boxes. The bounds are
    the tightest ones, known apart from any pow: for |y| < 2^-969 and a
    positive x other than 1, |y log(x)| < 745 2^-969 < 2^-959, and x^y is
    within 2^-958 of 1, above 1 where y log(x) > 0, so that its tightest
    enclosure is [1, 1 + 2^-52] there and [1 - 2^-53, 1] otherwise; for
    |y| >= 2^1014, |y log(x)| > 2^1014 2^-53, and x^y is above DBL_MAX or
    below 2^-1074.
  */
  bool bounds_are(const interval& z, double l, double r)
  {
    return z.left() == l && z.right() == r;
  }

  const double above_one = 1.0 + std::numeric_limits<double>::epsilon();
  const double below_one = 1.0 - 0.5*std::numeric_limits<double>::epsilon();
  const double largest = (std::numeric_limits<double>::max)();
  const double smallest = std::numeric_limits<double>::denorm_min();

  // [l, r], an interval the compiler knows nothing about
  interval make(double l, double r) { return opaque(interval(gaol::rnd_keep(l), gaol::rnd_keep(r))); }

  const EmptyOperand extreme_powers[] = {
    { "pow([1, 2], [2^-1074])", [] { return bounds_are(pow(make(1.0, 2.0), make(smallest, smallest)), 1.0, above_one); } },
    { "pow([1, 2], [1e-300])", [] { return bounds_are(pow(make(1.0, 2.0), make(1e-300, 1e-300)), 1.0, above_one); } },
    { "pow([0.5], [1e-300])", [] { return bounds_are(pow(make(0.5, 0.5), make(1e-300, 1e-300)), below_one, 1.0); } },
    { "pow([0.5], [-1e-300])", [] { return bounds_are(pow(make(0.5, 0.5), make(-1e-300, -1e-300)), 1.0, above_one); } },
    { "pow([2, 3], [-2^-1074, 2^-1074])", [] { return bounds_are(pow(make(2.0, 3.0), make(-smallest, smallest)), below_one, above_one); } },
    { "pow([2], 1e-300)", [] { return bounds_are(pow(make(2.0, 2.0), gaol::rnd_keep(1e-300)), 1.0, above_one); } },
    { "pow([1, 2], [1e300, 1e308])", [] { return bounds_are(pow(make(1.0, 2.0), make(1e300, 1e308)), 1.0, GAOL_INFINITY); } },
    { "pow([1.5], [1e300, 1e308])", [] { return bounds_are(pow(make(1.5, 1.5), make(1e300, 1e308)), largest, GAOL_INFINITY); } },
    { "pow([0.5], [1e300, 1e308])", [] { return bounds_are(pow(make(0.5, 0.5), make(1e300, 1e308)), 0.0, smallest); } },
    { "pow([0.5], [-1e308, -1e300])", [] { return bounds_are(pow(make(0.5, 0.5), make(-1e308, -1e300)), largest, GAOL_INFINITY); } },
    // An integer beyond the ints, from 2^1014 on, as gaol_ieee1788::pow takes it
    { "gaol_ieee1788::pow([1.5], [1e308])", [] {
        return bounds_are(gaol_ieee1788::pow(make(1.5, 1.5), make(1e308, 1e308)), largest, GAOL_INFINITY); } },
    { "gaol_ieee1788::pow([0, 0.5], [1e308])", [] {
        return bounds_are(gaol_ieee1788::pow(make(0.0, 0.5), make(1e308, 1e308)), 0.0, smallest); } },
    { "gaol_ieee1788::pow([0.5, 2], [-1e308])", [] {
        return bounds_are(gaol_ieee1788::pow(make(0.5, 2.0), make(-1e308, -1e308)), 0.0, GAOL_INFINITY); } },
  };

  /*
    The midpoint of an interval with a bound of 2^1023 or more in magnitude,
    finite (GAOL v5, point D.24): midpoint() and mid() computed the sum of the
    bounds first, which overflowed for [DBL_MAX] and raised the overflow
    exception, before they took the sum of the halves. The midpoints are
    computed by midpoints_of(), which calls GAOL only, and checked against the
    exact midpoint, computed with dyadic numbers, apart.
  */
  struct HugeInterval
  {
    const char *name;
    double l, r;
  };

  /*
    The intervals, made at run time from doubles read back from volatile
    memory: GCC 12.1 to 12.3 and 13.1 to 13.2 with -frounding-math wrote the
    long double 2^-1074 into a double member of an aggregate given
    std::numeric_limits<double>::denorm_min(), as a table at namespace scope
    or in a function. Its significand read as -0 and the rest overwrote the
    next member, so that [2^-1074, DBL_MAX] was [-0, 0x3bcd 2^-1074] on
    x86-64, where its checks passed, and [-0, NaN], the empty set, for 32-bit
    x86, where the checks of its midpoints failed (Debian 12 i386 with GCC
    12.2, MinGW-w64 12.2 and 13.2 for x86), though midpoint() and mid() give
    the right ones there. A scalar double or an array of doubles initialized
    so have the right value, as with GCC 12.4, 13.3 and 14.
  */
  std::vector<HugeInterval> huge_intervals()
  {
    const double dbl_max = gaol::rnd_keep((std::numeric_limits<double>::max)());
    const double smallest_subnormal = gaol::rnd_keep(std::numeric_limits<double>::denorm_min());
    const double two_1023 = gaol::rnd_keep(0x1p1023);
    std::vector<HugeInterval> intervals;
    intervals.push_back(HugeInterval{ "[DBL_MAX]", dbl_max, dbl_max });
    intervals.push_back(HugeInterval{ "[-DBL_MAX]", -dbl_max, -dbl_max });
    intervals.push_back(HugeInterval{ "[1e308, DBL_MAX]", gaol::rnd_keep(1e308), dbl_max });
    intervals.push_back(HugeInterval{ "[-DBL_MAX, -2^1023]", -dbl_max, -two_1023 });
    intervals.push_back(HugeInterval{ "[2^-1074, DBL_MAX]", smallest_subnormal, dbl_max });
    return intervals;
  }

  struct Midpoints
  {
    double midpoint, rad, mid_rad_m, mid_rad_r;
    interval mid, split_l, split_r, split_left, split_right;

    bool operator==(const Midpoints& m) const
    {
      return std::memcmp(&midpoint, &m.midpoint, sizeof midpoint) == 0 && std::memcmp(&rad, &m.rad, sizeof rad) == 0
        && mid_rad_m == m.mid_rad_m && mid_rad_r == m.mid_rad_r && mid.set_eq(m.mid) && split_l.set_eq(m.split_l)
        && split_r.set_eq(m.split_r) && split_left.set_eq(m.split_left) && split_right.set_eq(m.split_right);
    }
  };

  Midpoints midpoints_of(const HugeInterval& h)
  {
    const interval x = opaque(interval(gaol::rnd_keep(h.l), gaol::rnd_keep(h.r)));
    Midpoints m;
    m.midpoint = x.midpoint();
    m.rad = x.rad();
    x.mid_rad(m.mid_rad_m, m.mid_rad_r);
    m.mid = x.mid();
    x.split(m.split_l, m.split_r);
    m.split_left = x.split_left();
    m.split_right = x.split_right();
    return m;
  }

  bool midpoints_are_right(const HugeInterval& h, const Midpoints& m)
  {
    const Exact middle = exact(dyadic(h.l)*dyadic(0.5) + dyadic(h.r)*dyadic(0.5));
    const double c = m.midpoint;
    return is_tightest_enclosure(m.mid, middle) && m.mid.set_contains(c) && (h.l != h.r || c == h.l)
      && m.mid_rad_m == c && m.mid_rad_r == m.rad && m.rad >= 0.0 && (h.l != h.r || m.rad == 0.0)
      && bounds_are(m.split_l, h.l, c) && bounds_are(m.split_r, c, h.r)
      && m.split_left.set_eq(m.split_l) && m.split_right.set_eq(m.split_r);
  }

  /*
    The relations in loops, which a compiler vectorizes or if-converts (GAOL
    v5, point D.24): the comparison that follows a test of the empty set is
    then made for the empty operands too, and <, <=, >= or > raise the
    invalid-operation exception on their NaN bounds, as GCC 9.4 at -O3 made
    for the FPU intervals the constructor raise it (and with -mavx2,
    certainly_positive(), less() and others). Every third x and every fifth y
    is empty; each result is compared with the relation computed on its own,
    outside the loop.
  */
#if defined(__GNUC__) || defined(__clang__)
#  define GAOL_TESTS_NOINLINE __attribute__((noinline))
#elif defined(_MSC_VER)
#  define GAOL_TESTS_NOINLINE __declspec(noinline)
#else
#  define GAOL_TESTS_NOINLINE
#endif

  const int loop_size = 64;

  // name() computes expression of the arrays x and y for each index i, and
  // name_at() for one pair, outside any loop
#define GAOL_TESTS_RELATION_LOOP(name, expression)                                                    \
  GAOL_TESTS_NOINLINE void name(const interval *x, const interval *y, unsigned char *result)          \
  {                                                                                                    \
    static_cast<void>(y);                                                                              \
    for (int i = 0; i < loop_size; ++i) {                                                              \
      result[i] = static_cast<unsigned char>(expression);                                              \
    }                                                                                                  \
  }                                                                                                    \
  unsigned char name##_at(const interval& x_at, const interval& y_at)                                  \
  {                                                                                                    \
    const interval *x = &x_at, *y = &y_at;                                                             \
    static_cast<void>(y);                                                                              \
    const int i = 0;                                                                                   \
    return static_cast<unsigned char>(expression);                                                     \
  }

  GAOL_TESTS_RELATION_LOOP(loop_less, x[i].less(y[i]))
  GAOL_TESTS_RELATION_LOOP(loop_strictly_less, x[i].strictly_less(y[i]))
  GAOL_TESTS_RELATION_LOOP(loop_is_symmetric, x[i].is_symmetric())
  GAOL_TESTS_RELATION_LOOP(loop_certainly_positive, x[i].certainly_positive())
  GAOL_TESTS_RELATION_LOOP(loop_certainly_negative, x[i].certainly_negative())
  GAOL_TESTS_RELATION_LOOP(loop_certainly_strictly_positive, x[i].certainly_strictly_positive())
  GAOL_TESTS_RELATION_LOOP(loop_certainly_strictly_negative, x[i].certainly_strictly_negative())
  GAOL_TESTS_RELATION_LOOP(loop_certainly_ge, x[i].certainly_ge(y[i]))
  GAOL_TESTS_RELATION_LOOP(loop_certainly_geq, x[i].certainly_geq(y[i]))
  GAOL_TESTS_RELATION_LOOP(loop_certainly_le, x[i].certainly_le(y[i]))
  GAOL_TESTS_RELATION_LOOP(loop_certainly_leq, x[i].certainly_leq(y[i]))
  GAOL_TESTS_RELATION_LOOP(loop_is_zero, x[i].is_zero())
  GAOL_TESTS_RELATION_LOOP(loop_is_a_double, x[i].is_a_double())
  GAOL_TESTS_RELATION_LOOP(loop_is_an_int, x[i].is_an_int())
  GAOL_TESTS_RELATION_LOOP(loop_is_entire, x[i].is_entire())
  GAOL_TESTS_RELATION_LOOP(loop_is_empty, x[i].is_empty())
  GAOL_TESTS_RELATION_LOOP(loop_straddles_zero, x[i].straddles_zero())
  GAOL_TESTS_RELATION_LOOP(loop_set_contains, x[i].set_contains(y[i]))
  GAOL_TESTS_RELATION_LOOP(loop_set_contains_double, x[i].set_contains(1.5))
  GAOL_TESTS_RELATION_LOOP(loop_set_strictly_contains, x[i].set_strictly_contains(y[i]))
  GAOL_TESTS_RELATION_LOOP(loop_set_strictly_contains_double, x[i].set_strictly_contains(1.5))
  GAOL_TESTS_RELATION_LOOP(loop_set_eq, x[i].set_eq(y[i]))
  GAOL_TESTS_RELATION_LOOP(loop_set_disjoint, x[i].set_disjoint(y[i]))
  GAOL_TESTS_RELATION_LOOP(loop_set_le, x[i].set_le(y[i]))
  GAOL_TESTS_RELATION_LOOP(loop_set_leq, x[i].set_leq(y[i]))
  GAOL_TESTS_RELATION_LOOP(loop_operators, (x[i] <= y[i]) + 2*(x[i] < y[i]) + 4*(x[i] >= 1.5) + 8*(x[i] > 1.5))
  GAOL_TESTS_RELATION_LOOP(loop_constructor, interval(x[i].left(), y[i].right()).is_empty())
  GAOL_TESTS_RELATION_LOOP(loop_intersection, (x[i] & y[i]).is_empty())
  GAOL_TESTS_RELATION_LOOP(loop_floor, floor(x[i]).is_empty())

  struct RelationLoop
  {
    const char *name;
    void (*loop)(const interval *x, const interval *y, unsigned char *result);
    unsigned char (*at)(const interval& x, const interval& y);
  };

  const RelationLoop relation_loops[] = {
    { "less()", loop_less, loop_less_at },
    { "strictly_less()", loop_strictly_less, loop_strictly_less_at },
    { "is_symmetric()", loop_is_symmetric, loop_is_symmetric_at },
    { "certainly_positive()", loop_certainly_positive, loop_certainly_positive_at },
    { "certainly_negative()", loop_certainly_negative, loop_certainly_negative_at },
    { "certainly_strictly_positive()", loop_certainly_strictly_positive, loop_certainly_strictly_positive_at },
    { "certainly_strictly_negative()", loop_certainly_strictly_negative, loop_certainly_strictly_negative_at },
    { "certainly_ge()", loop_certainly_ge, loop_certainly_ge_at },
    { "certainly_geq()", loop_certainly_geq, loop_certainly_geq_at },
    { "certainly_le()", loop_certainly_le, loop_certainly_le_at },
    { "certainly_leq()", loop_certainly_leq, loop_certainly_leq_at },
    { "is_zero()", loop_is_zero, loop_is_zero_at },
    { "is_a_double()", loop_is_a_double, loop_is_a_double_at },
    { "is_an_int()", loop_is_an_int, loop_is_an_int_at },
    { "is_entire()", loop_is_entire, loop_is_entire_at },
    { "is_empty()", loop_is_empty, loop_is_empty_at },
    { "straddles_zero()", loop_straddles_zero, loop_straddles_zero_at },
    { "set_contains()", loop_set_contains, loop_set_contains_at },
    { "set_contains(double)", loop_set_contains_double, loop_set_contains_double_at },
    { "set_strictly_contains()", loop_set_strictly_contains, loop_set_strictly_contains_at },
    { "set_strictly_contains(double)", loop_set_strictly_contains_double, loop_set_strictly_contains_double_at },
    { "set_eq()", loop_set_eq, loop_set_eq_at },
    { "set_disjoint()", loop_set_disjoint, loop_set_disjoint_at },
    { "set_le()", loop_set_le, loop_set_le_at },
    { "set_leq()", loop_set_leq, loop_set_leq_at },
    { "<=, < and >=, > with a double", loop_operators, loop_operators_at },
    { "interval(l, r)", loop_constructor, loop_constructor_at },
    { "&", loop_intersection, loop_intersection_at },
    { "floor()", loop_floor, loop_floor_at },
  };

  // The operands of the loops, from operands the compiler does not know
  void loop_operands(std::vector<interval>& x, std::vector<interval>& y)
  {
    x.resize(loop_size);
    y.resize(loop_size);
    for (int i = 0; i < loop_size; ++i) {
      const double d = static_cast<double>(i);
      x[i] = (i % 3 == 0) ? E() : make(d - 10.0, d + 0.5);
      y[i] = (i % 5 == 0) ? E() : make(0.5*d, d + 1.0);
    }
  }

  // Runs loop on the operands, and compares its results with at()
  bool relation_loop_is_right(const RelationLoop& r)
  {
    std::vector<interval> x, y;
    loop_operands(x, y);
    unsigned char result[loop_size];
    r.loop(x.data(), y.data(), result);
    bool right = true;
    for (int i = 0; i < loop_size; ++i) {
      right = right && result[i] == r.at(opaque(x[i]), opaque(y[i]));
    }
    return right;
  }

#if GAOL_TESTS_TRAPS
  // How a child process that enabled the floating-point exceptions of excepts
  // and ran f() ended: f() returned true or false, or the process was stopped
  // by SIGFPE, which the processor raises for an exception that is enabled, or
  // otherwise. The exit status tells which: 40 and 41 are the answers of f(),
  // 42 the SIGFPE the child caught, 43 an exception that could not be enabled.
  enum Outcome { returned_true, returned_false, trapped, other };

  const int status_true = 40, status_false = 41, status_trapped = 42, status_not_enabled = 43;

  void exit_on_sigfpe(int)
  {
    _exit(status_trapped);
  }

  Outcome run_with_exceptions_enabled(int excepts, const std::function<bool()>& f)
  {
    std::fflush(stdout);
    std::fflush(stderr);
    const pid_t child = fork();
    if (child < 0) {
      return other;
    }
    if (child == 0) {
      // The SIGFPE is caught to exit with a status of its own: the sanitizers
      // report it as an error and abort otherwise, and no core file is wanted
      // for the one that would kill the process where the handler is ignored
      const struct rlimit no_core = { 0, 0 };
      setrlimit(RLIMIT_CORE, &no_core);
      struct sigaction action;
      std::memset(&action, 0, sizeof action);
      action.sa_handler = exit_on_sigfpe;
      sigemptyset(&action.sa_mask);
      sigaction(SIGFPE, &action, nullptr);
      std::feclearexcept(FE_ALL_EXCEPT);
      if (feenableexcept(excepts) == -1) {
        _exit(status_not_enabled);
      }
      const bool result = f();
      fedisableexcept(FE_ALL_EXCEPT);
      _exit(result ? status_true : status_false);
    }
    int status = 0;
    pid_t waited;
    do {
      waited = waitpid(child, &status, 0);
    } while (waited < 0 && errno == EINTR);
    if (waited != child) {
      return other;
    }
    if (WIFSIGNALED(status)) {
      return WTERMSIG(status) == SIGFPE ? trapped : other;
    }
    if (WIFEXITED(status)) {
      switch (WEXITSTATUS(status)) {
        case status_true: return returned_true;
        case status_false: return returned_false;
        case status_trapped: return trapped;
      }
    }
    return other;
  }

  const char *outcome_text(Outcome o)
  {
    switch (o) {
      case returned_true: return "returned true";
      case returned_false: return "returned false";
      case trapped: return "died on SIGFPE";
      default: return "ended otherwise (an error, another signal)";
    }
  }

  // An invalid operation, 0/0, which traps where the processor delivers the
  // exception: the control of the checks below, whose result is only
  // meaningful there
  bool invalid_operation()
  {
    volatile double zero = 0.0;
    volatile double quotient = zero/zero;
    return quotient != quotient;
  }

  // An overflow, 2 DBL_MAX: the control of the checks with the overflow
  // exception enabled
  bool overflow_operation()
  {
    volatile double big = (std::numeric_limits<double>::max)();
    volatile double product = 2.0*big;
    return product > big;
  }
#endif // GAOL_TESTS_TRAPS
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
    // One draw per declarator, in order: the two arguments of std::make_pair
    // were evaluated in an order the compiler chose, and gave other operands
    // with other compilers
    const double a = random.uniform(0.01, 0.6), b = random.uniform(1.1, 2.0);
    const double wa = random.uniform(0.001, 0.39), wb = random.uniform(0.001, 1.0);
    operands.push_back(std::make_pair(interval(a, a + wa), interval(b, b + wb)));
  }

  const Operation operations[] = {
    { "interval()", [](const interval&, const interval&) { interval z; return S(z); } },
    { "interval(a,b)", [](const interval&, const interval&) { return S(interval(0x1.999999999999ap-4, 0x1.5555555555555p-2)); } },
    { "interval(a)", [](const interval&, const interval&) { return S(interval(0.1)); } },
    { "textToInterval(const std::string&)", [](const interval&, const interval&) { return S(textToInterval("[0.1, 1/3]")); } },
    { "textToInterval(sl, sr)", [](const interval&, const interval&) { return S(textToInterval("0.1", "0.3")); } },
    { "textToInterval(\"sin(1)+exp(0.1)\")", [](const interval&, const interval&) { return S(textToInterval("sin(1)+exp(0.1)")); } },
    { "textToInterval(\"3.56?1e-2\")", [](const interval&, const interval&) { return S(textToInterval("3.56?1e-2")); } },
    { "textToInterval(\"[-0x1.00000000000001p0, 2/3]\")", [](const interval&, const interval&) { return S(textToInterval("[-0x1.00000000000001p0, 2/3]")); } },
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
    { "gaol_ieee1788::intervalToText", [](const interval& x, const interval&) {
        return gaol_ieee1788::intervalToText(x) + " " + gaol_ieee1788::intervalToText(interval(0.1)); } },
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
    { "nb_fp_numbers", [](const interval& x, const interval&) { return S(nb_fp_numbers(x.left(), x.right())); } },
    // Operations that compute after their one check of the rounding direction
    // what they computed with operations of intervals checking it again, or
    // that the operations above do not reach (GAOL v5)
    { "tan next to a pole", [](const interval& x, const interval&) {
        return S(tan(x + 1.0)) + " " + S(tan(interval(1.5, 0x1.921fb54442d18p+0))); } },
    { "acos_rel, asin_rel and atan_rel on their pieces", [](const interval& x, const interval& y) {
        return S(acos_rel(x, y)) + " " + S(acos_rel(x + 0.8, y)) + " " + S(asin_rel(x - 1.5, y)) + " " + S(atan_rel(y, x + 4.0)); } },
    { "nth_root(x,n) for n >= 4", [](const interval&, const interval& y) { return S(nth_root(y, 4)) + " " + S(nth_root(-y, 5)); } },
    { "pow(x,n) from the rounded products", [](const interval&, const interval& y) {
        return S(pow(y*0x1p-330, 3)) + " " + S(pow(interval(0., 1.)*y, 3)) + " " + S(pow(y*1e200, 3)); } },
    { "pow(x,-n) where x^n overflows", [](const interval&, const interval& y) {
        return S(pow(y*1e200, -2)) + " " + S(pow((y - 2.)*1e200, -2)) + " " + S(pow((y - 2.)*1e200, -3)) + " "
          + S(pow(interval(0., 1.)*y, -1)); } },
    { "gaol_ieee1788::pow with an integer exponent beyond the ints", [](const interval&, const interval& y) {
        return S(gaol_ieee1788::pow(y, interval(2147483649.))) + " " + S(gaol_ieee1788::pow(interval(0.5, 0.9), interval(1e10))) + " "
          + S(gaol_ieee1788::pow(1. + (y - 1.)*0x1p-40, interval(-2147483649.))); } },
    // Powers with a subnormal result, which CORE-MATH rounds by itself in the
    // direction fegetround() gives, where the doubles GAOL and CORE-MATH
    // compute are rounded in the direction of MXCSR (GAOL v5)
    { "gaol_ieee1788::pow with a subnormal result", [](const interval&, const interval&) {
        std::string s;
        for (const SubnormalPower& p : subnormal_powers) {
          s += S(gaol_ieee1788::pow(interval(p.x), interval(p.y))) + " ";
        }
        return s; } },
    { "feven", [](const interval&, const interval&) { return S(feven(2.0)); } },
    { "interval::pi()", [](const interval&, const interval&) { return S(interval::pi()); } },
    { "interval::precision(n)", [](const interval&, const interval&) { const int p = static_cast<int>(interval::precision(17)); interval::precision(p); return S(p); } },
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
    The exception flags the program raised, and its exception masks, are
    kept by every operation, whichever way GAOL is built (GAOL v5, point D.24
    of TODO.md): with GAOL_PRESERVE_ROUNDING, the SSE2 operations (+, -, *,
    /, %, sqr(), inverse(), the operations with a double...) wrote the SSE
    control register whole, every exception masked and every flag cleared.
    Each operation is run with the five flags of IEEE 754 raised, and on x86
    the six of the SSE control register: they have to be raised after it, and
    the masks unchanged. Where feraiseexcept() raises no flag that
    fetestexcept() reads back (an emulator, a processor without the flags),
    this is not checked, and the test says so. An exception the program
    enabled is checked below, with the invalid-operation exception enabled.
  */
  std::feclearexcept(FE_ALL_EXCEPT);
  std::feraiseexcept(FE_ALL_EXCEPT);
  const bool flags_readable = std::fetestexcept(FE_ALL_EXCEPT) == FE_ALL_EXCEPT;
  std::feclearexcept(FE_ALL_EXCEPT);
  if (!flags_readable) {
    std::printf("The exception flags raised by feraiseexcept() are not read back here: the flags kept by "
                "the operations are not checked\n");
  } else {
    for (const Operation& op : operations) {
      const std::string name = op.name;
      set(directions[0]);
      std::feraiseexcept(FE_ALL_EXCEPT);
#if GAOL_TESTS_SSE
      _mm_setcsr(_mm_getcsr() | 0x3fu); // the six flags of MXCSR, denormal-operand included
      const unsigned masks_before = _mm_getcsr() & _MM_MASK_MASK;
#endif
      static_cast<void>(run(op, operands[0].first, operands[0].second));
      const int raised = std::fetestexcept(FE_ALL_EXCEPT);
#if GAOL_TESTS_SSE
      const unsigned csr = _mm_getcsr();
#endif
      std::feclearexcept(FE_ALL_EXCEPT);
      check("the exception flags the program raised kept by " + name, raised == FE_ALL_EXCEPT,
            [&] { return "flags " + std::to_string(raised) + " after it"; });
#if GAOL_TESTS_SSE
      check("the SSE exception flags and masks kept by " + name, (csr & 0x3fu) == 0x3fu && (csr & _MM_MASK_MASK) == masks_before,
            [&] { return "MXCSR " + std::to_string(csr) + " after it, masks " + std::to_string(masks_before) + " before"; });
#endif
    }
    set(directions[0]);
  }

#if GAOL_TESTS_FLUSH
  /*
    The modes that flush the subnormals to zero (GAOL v5): flush-to-zero,
    denormals-are-zero and both on x86, FZ (and FIZ, where the processor has
    it) on ARM. With one of them set, an operation whose result or operand is
    subnormal gave a zero: [1e-300]*[1e-20] was [0, 0], when a program linked
    with -Ofast, whose crtfastmath.o sets them after GAOL initialized itself,
    or a plug-in built so, set them, and the rounding direction upward that the
    probe of each operation looked at was still there; the FPU intervals
    compared the bounds, and the operations with a double the double, with 0
    before their probe: [1e-300]/[100*2^-1074] was the empty set. The direction
    is tried with each of the modes, upward included, which is what such a
    program has. The bounds have to be the tightest ones, computed apart with
    exact rational arithmetic, or with mpmath for the exponential: the operands,
    the exact values and the bounds are read with the modes cleared, which
    change frexp(), on which the exact arithmetic of the tests relies, for a
    subnormal. The modes are cleared after the operation, or restored with
    GAOL_PRESERVE_ROUNDING. tests/fast_math_link.cpp checks the program linked
    with -ffast-math.
  */
  const std::vector<FlushMode> flush_modes_honoured = flush_modes();
  {
    const Dyadic half = dyadic(0.5);
    struct Case
    {
      const char *name;
      interval x, y;
      interval (*run)(const interval&, const interval&);
      Exact value; // the exact result, or a number between the same two doubles
    };
    const Case cases[] = {
      // Subnormal results of normal operands
      { "[1e-300]*[1e-20]", interval(1e-300), interval(1e-20),
        [](const interval& x, const interval& y) { return x*y; },
        exact(dyadic(1e-300)*dyadic(1e-20)) },
      { "[1e-300]/[1e20]", interval(1e-300), interval(1e20),
        [](const interval& x, const interval& y) { return x/y; },
        quotient(dyadic(1e-300), dyadic(1e20)) },
      // The exact difference, a subnormal
      { "[3e-308]-[2.9e-308]", interval(3e-308), interval(2.9e-308),
        [](const interval& x, const interval& y) { return x - y; },
        exact(dyadic(3e-308) - dyadic(2.9e-308)) },
      { "sqr([1e-160])", interval(1e-160), interval(),
        [](const interval& x, const interval&) { return sqr(x); },
        exact(dyadic(1e-160)*dyadic(1e-160)) },
      // Subnormal operands
      { "[100*2^-1074]*[1e10]", interval(subnormal(100)), interval(1e10),
        [](const interval& x, const interval& y) { return x*y; },
        exact(dyadic(subnormal(100))*dyadic(1e10)) },
      { "[100*2^-1074]+[3*2^-1074]", interval(subnormal(100)), interval(subnormal(3)),
        [](const interval& x, const interval& y) { return x + y; },
        exact(dyadic(subnormal(100)) + dyadic(subnormal(3))) },
      { "[100*2^-1074]/[3]", interval(subnormal(100)), interval(3.0),
        [](const interval& x, const interval& y) { return x/y; },
        quotient(dyadic(subnormal(100)), dyadic(3.0)) },
      // A normal result of a subnormal divisor
      { "[1e-300]/[100*2^-1074]", interval(1e-300), interval(subnormal(100)),
        [](const interval& x, const interval& y) { return x/y; },
        quotient(dyadic(1e-300), dyadic(subnormal(100))) },
      // A subnormal double, a literal rather than subnormal(100), which the
      // modes would flush: the operations compared it with 0 before their check
      // of the rounding direction, and it was 0
      { "[1e10]*(100*2^-1074)", interval(1e10), interval(),
        [](const interval& x, const interval&) { return x*0x64p-1074; },
        exact(dyadic(1e10)*dyadic(subnormal(100))) },
      { "[1e-300]/(100*2^-1074)", interval(1e-300), interval(),
        [](const interval& x, const interval&) { return x/0x64p-1074; },
        quotient(dyadic(1e-300), dyadic(subnormal(100))) },
      // exp(-740) is 84.78 times 2^-1074 (mpmath), between the same two
      // doubles as 84.5 times 2^-1074
      { "exp([-740])", interval(-740.0), interval(),
        [](const interval& x, const interval&) { return exp(x); },
        exact(dyadic(subnormal(169))*half) },
    };

    for (const FlushMode& m : flush_modes_honoured) {
      for (const Case& c : cases) {
        for (const Direction& d : directions) {
          set(d);
          set_flush_bits(m.bits);
          const unsigned int found = flush_bits();
          const interval r = c.run(c.x, c.y);
          const unsigned int left = flush_bits();
          set_flush_bits(0u);
          const auto describe = [&] {
            return m.name + " set, rounding direction " + d.name + ": modes " + std::to_string(left) + " after it, "
                 + std::to_string(found) + " before";
          };
          check(std::string(c.name) + " the tightest enclosure with a flush-to-zero mode", is_tightest_enclosure(r, c.value),
                [&] { return describe() + ", " + hex(r); });
#if GAOL_PRESERVE_ROUNDING
          check(std::string(c.name) + ": flush-to-zero modes restored", left == found, describe);
#else
          check(std::string(c.name) + ": flush-to-zero modes cleared", left == 0u, describe);
#endif
        }
      }
    }
    set(directions[0]);
  }

  /*
    Every operation that checks the rounding direction, with each of the modes
    set just before it, gives what it gives with the modes cleared, which the
    cases above show to be the tightest bounds, and leaves them cleared, or
    restored with GAOL_PRESERVE_ROUNDING (GAOL v5, review of point 4).
    Its check clears the modes before it reads, compares or copies a bound, and
    with GAOL_PRESERVE_ROUNDING it makes its result before it restores them;
    the compilers do not model the modes, so this is checked on what each
    compiler of the continuous integration emits. With GCC 13 at -O3, asinpi()
    compared the bounds it had read with 0 before the check, and with the modes
    set asinpi([100*2^-1074]) was [32*2^-1074], above the exact value,
    31.83*2^-1074 (mpmath); sqrt([-1e-310, 4]), whose intersection with
    [0, +oo] kept -1e-310 under denormals-are-zero, was empty once the check
    had cleared it; atan2([100*2^-1074], [100*2^-1074]) was empty, the point
    taken for (0, 0); and with GAOL_PRESERVE_ROUNDING sinpi([100, 1000]*2^-1074)
    was empty, its minimum taken once the modes were restored. acos_rel() and
    asin_rel() of a J outside [-1, 1], or containing it, returned before their
    check, with the right result but the modes still set. The operations
    that make no check, which compare the bounds under the modes, are left out
    (doc/using.md). Nothing of the C library runs while a mode is set: the
    results are read and written with the modes cleared.
  */
  {
    struct FlushOperation
    {
      const char *name;
      int arity; // 1: x, 2: x and y, a double being y.left(), 3: x, y and z
      interval (*run)(const interval& x, const interval& y, const interval& z);
    };
    const FlushOperation flush_operations[] = {
      { "sqr(x)", 1, [](const interval& x, const interval&, const interval&) { return sqr(x); } },
      { "sqrt(x)", 1, [](const interval& x, const interval&, const interval&) { return sqrt(x); } },
      { "rsqrt(x)", 1, [](const interval& x, const interval&, const interval&) { return rsqrt(x); } },
      { "inverse(x)", 1, [](const interval& x, const interval&, const interval&) { return inverse(x); } },
      { "exp(x)", 1, [](const interval& x, const interval&, const interval&) { return exp(x); } },
      { "exp2(x)", 1, [](const interval& x, const interval&, const interval&) { return exp2(x); } },
      { "exp10(x)", 1, [](const interval& x, const interval&, const interval&) { return exp10(x); } },
      { "expm1(x)", 1, [](const interval& x, const interval&, const interval&) { return expm1(x); } },
      { "exp2m1(x)", 1, [](const interval& x, const interval&, const interval&) { return exp2m1(x); } },
      { "exp10m1(x)", 1, [](const interval& x, const interval&, const interval&) { return exp10m1(x); } },
      { "log(x)", 1, [](const interval& x, const interval&, const interval&) { return log(x); } },
      { "log2(x)", 1, [](const interval& x, const interval&, const interval&) { return log2(x); } },
      { "log10(x)", 1, [](const interval& x, const interval&, const interval&) { return log10(x); } },
      { "log1p(x)", 1, [](const interval& x, const interval&, const interval&) { return log1p(x); } },
      { "log2p1(x)", 1, [](const interval& x, const interval&, const interval&) { return log2p1(x); } },
      { "log10p1(x)", 1, [](const interval& x, const interval&, const interval&) { return log10p1(x); } },
      { "sin(x)", 1, [](const interval& x, const interval&, const interval&) { return sin(x); } },
      { "cos(x)", 1, [](const interval& x, const interval&, const interval&) { return cos(x); } },
      { "tan(x)", 1, [](const interval& x, const interval&, const interval&) { return tan(x); } },
      { "sinpi(x)", 1, [](const interval& x, const interval&, const interval&) { return sinpi(x); } },
      { "cospi(x)", 1, [](const interval& x, const interval&, const interval&) { return cospi(x); } },
      { "tanpi(x)", 1, [](const interval& x, const interval&, const interval&) { return tanpi(x); } },
      { "asin(x)", 1, [](const interval& x, const interval&, const interval&) { return asin(x); } },
      { "acos(x)", 1, [](const interval& x, const interval&, const interval&) { return acos(x); } },
      { "atan(x)", 1, [](const interval& x, const interval&, const interval&) { return atan(x); } },
      { "asinpi(x)", 1, [](const interval& x, const interval&, const interval&) { return asinpi(x); } },
      { "acospi(x)", 1, [](const interval& x, const interval&, const interval&) { return acospi(x); } },
      { "atanpi(x)", 1, [](const interval& x, const interval&, const interval&) { return atanpi(x); } },
      { "sinh(x)", 1, [](const interval& x, const interval&, const interval&) { return sinh(x); } },
      { "cosh(x)", 1, [](const interval& x, const interval&, const interval&) { return cosh(x); } },
      { "tanh(x)", 1, [](const interval& x, const interval&, const interval&) { return tanh(x); } },
      { "asinh(x)", 1, [](const interval& x, const interval&, const interval&) { return asinh(x); } },
      { "acosh(x)", 1, [](const interval& x, const interval&, const interval&) { return acosh(x); } },
      { "atanh(x)", 1, [](const interval& x, const interval&, const interval&) { return atanh(x); } },
      { "pow(x, 2)", 1, [](const interval& x, const interval&, const interval&) { return gaol::pow(x, 2); } },
      { "pow(x, 3)", 1, [](const interval& x, const interval&, const interval&) { return gaol::pow(x, 3); } },
      { "pow(x, -2)", 1, [](const interval& x, const interval&, const interval&) { return gaol::pow(x, -2); } },
      { "pow(x, -3)", 1, [](const interval& x, const interval&, const interval&) { return gaol::pow(x, -3); } },
      { "pow(x, 5u)", 1, [](const interval& x, const interval&, const interval&) { return gaol::pow(x, 5u); } },
      { "nth_root(x, 2)", 1, [](const interval& x, const interval&, const interval&) { return nth_root(x, 2); } },
      { "nth_root(x, 3)", 1, [](const interval& x, const interval&, const interval&) { return nth_root(x, 3); } },
      { "nth_root(x, 4)", 1, [](const interval& x, const interval&, const interval&) { return nth_root(x, 4); } },
      { "nth_root(x, 5)", 1, [](const interval& x, const interval&, const interval&) { return nth_root(x, 5); } },
      { "nth_root(x, -2)", 1, [](const interval& x, const interval&, const interval&) { return nth_root(x, -2); } },
      { "x.mid()", 1, [](const interval& x, const interval&, const interval&) { return x.mid(); } },
      { "x.width()", 1, [](const interval& x, const interval&, const interval&) { return interval(x.width()); } },
      { "x.rad()", 1, [](const interval& x, const interval&, const interval&) { return interval(x.rad()); } },
      { "x + y", 2, [](const interval& x, const interval& y, const interval&) { return x + y; } },
      { "x - y", 2, [](const interval& x, const interval& y, const interval&) { return x - y; } },
      { "x * y", 2, [](const interval& x, const interval& y, const interval&) { return x * y; } },
      { "x / y", 2, [](const interval& x, const interval& y, const interval&) { return x / y; } },
      { "x % y", 2, [](const interval& x, const interval& y, const interval&) { return x % y; } },
      { "hypot(x, y)", 2, [](const interval& x, const interval& y, const interval&) { return hypot(x, y); } },
      { "atan2(x, y)", 2, [](const interval& x, const interval& y, const interval&) { return atan2(x, y); } },
      { "atan2pi(x, y)", 2, [](const interval& x, const interval& y, const interval&) { return atan2pi(x, y); } },
      { "pow(x, y)", 2, [](const interval& x, const interval& y, const interval&) { return gaol::pow(x, y); } },
      { "gaol_ieee1788::pow(x, y)", 2, [](const interval& x, const interval& y, const interval&) { return gaol_ieee1788::pow(x, y); } },
      { "pow(x, double)", 2, [](const interval& x, const interval& y, const interval&) { return gaol::pow(x, y.left()); } },
      { "sqrt_rel(x, y)", 2, [](const interval& x, const interval& y, const interval&) { return sqrt_rel(x, y); } },
      { "nth_root_rel(x, 3, y)", 2, [](const interval& x, const interval& y, const interval&) { return nth_root_rel(x, 3, y); } },
      { "nth_root_rel(x, 4, y)", 2, [](const interval& x, const interval& y, const interval&) { return nth_root_rel(x, 4, y); } },
      { "acos_rel(x, y)", 2, [](const interval& x, const interval& y, const interval&) { return acos_rel(x, y); } },
      { "asin_rel(x, y)", 2, [](const interval& x, const interval& y, const interval&) { return asin_rel(x, y); } },
      { "atan_rel(x, y)", 2, [](const interval& x, const interval& y, const interval&) { return atan_rel(x, y); } },
      { "acosh_rel(x, y)", 2, [](const interval& x, const interval& y, const interval&) { return acosh_rel(x, y); } },
      { "asinh_rel(x, y)", 2, [](const interval& x, const interval& y, const interval&) { return asinh_rel(x, y); } },
      { "atanh_rel(x, y)", 2, [](const interval& x, const interval& y, const interval&) { return atanh_rel(x, y); } },
      { "cancel_minus(x, y)", 2, [](const interval& x, const interval& y, const interval&) { return cancel_minus(x, y); } },
      { "cancel_plus(x, y)", 2, [](const interval& x, const interval& y, const interval&) { return cancel_plus(x, y); } },
      { "hausdorff(x, y)", 2, [](const interval& x, const interval& y, const interval&) { return interval(hausdorff(x, y)); } },
      { "x * double", 2, [](const interval& x, const interval& y, const interval&) { return x * y.left(); } },
      { "double * x", 2, [](const interval& x, const interval& y, const interval&) { return y.left() * x; } },
      { "x / double", 2, [](const interval& x, const interval& y, const interval&) { return x / y.left(); } },
      { "double / x", 2, [](const interval& x, const interval& y, const interval&) { return y.left() / x; } },
      { "x % double", 2, [](const interval& x, const interval& y, const interval&) { return x % y.left(); } },
      { "x + double", 2, [](const interval& x, const interval& y, const interval&) { return x + y.left(); } },
      { "double - x", 2, [](const interval& x, const interval& y, const interval&) { return y.left() - x; } },
      { "fma(x, y, z)", 3, [](const interval& x, const interval& y, const interval& z) { return fma(x, y, z); } },
      { "div_rel(x, y, z)", 3, [](const interval& x, const interval& y, const interval& z) { return div_rel(x, y, z); } },
    };
    // Subnormal, mixed and normal bounds; the first ones for two and three operands
    const interval flush_operands[] = {
      interval(subnormal(100)), interval(-subnormal(100)), interval(subnormal(3), subnormal(100)),
      interval(-subnormal(100), subnormal(1000)), interval(0.0, subnormal(100)), interval(-1e-310, 4.0),
      interval(1e-300), interval(1.0, 2.0), interval(-2.0, 3.0), interval(-1.0, subnormal(5)), interval(3.0),
      interval(subnormal(100), subnormal(1000)), interval(-subnormal(100), -subnormal(3)), interval(0.5),
      interval(-subnormal(100), 0.0), interval(1e-310, 1e-309), interval(-1e-309, -1e-310), interval(1e-310, 2.0),
      interval(1e-160), interval(3e-308), interval(1e-308, 3e-308), interval(-740.0), interval(-1070.0),
      interval(-320.0), interval(1e-300, 1e-200), interval(1e10), interval(0x1p-1022), interval(100.0, 1000.0),
    };
    const std::size_t all_operands = sizeof(flush_operands)/sizeof(flush_operands[0]);
    const std::size_t operands_of[] = { 0, all_operands, 14, 6 }; // by arity
    const auto same = [](const interval& a, const interval& b) {
      if (a.is_empty() || b.is_empty()) {
        return a.is_empty() && b.is_empty();
      }
      const double ab[2] = { a.left(), a.right() }, bb[2] = { b.left(), b.right() };
      return std::memcmp(ab, bb, sizeof ab) == 0;
    };
    for (const FlushOperation& op : flush_operations) {
      const std::string name = std::string(op.name) + " with a flush-to-zero mode set before it, as with the modes cleared";
#if GAOL_PRESERVE_ROUNDING
      const std::string modes_name = std::string(op.name) + ": flush-to-zero modes restored after it";
#else
      const std::string modes_name = std::string(op.name) + ": flush-to-zero modes cleared after it";
#endif
      const std::size_t n = operands_of[op.arity];
      const std::size_t ny = (op.arity >= 2) ? n : 1, nz = (op.arity == 3) ? n : 1;
      for (std::size_t i = 0; i < n; ++i) {
        for (std::size_t j = 0; j < ny; ++j) {
          for (std::size_t k = 0; k < nz; ++k) {
            const interval& x = flush_operands[i];
            const interval& y = flush_operands[j];
            const interval& z = flush_operands[k];
            set(directions[0]);
            set_flush_bits(0u);
            const interval cleared = op.run(x, y, z);
            for (const FlushMode& m : flush_modes_honoured) {
              for (const Direction& d : directions) {
                set(d);
                set_flush_bits(m.bits);
                const interval r = op.run(x, y, z);
                const unsigned int left = flush_bits();
                set_flush_bits(0u);
                const auto args = [&] {
                  std::string a = hex(x);
                  if (op.arity >= 2) {
                    a += ", " + hex(y);
                  }
                  if (op.arity == 3) {
                    a += ", " + hex(z);
                  }
                  return m.name + " set, rounding direction " + d.name + ", (" + a + ")";
                };
                check(name, same(r, cleared),
                      [&] { return args() + ": " + hex(r) + " rather than " + hex(cleared); });
                const auto modes_left = [&] { return args() + ": modes " + std::to_string(left) + " after it"; };
#if GAOL_PRESERVE_ROUNDING
                check(modes_name, left == m.bits, modes_left);
#else
                check(modes_name, left == 0u, modes_left);
#endif
              }
            }
          }
        }
      }
    }
    set(directions[0]);
  }
#else
  std::printf("No mode flushing the subnormals to zero that the test can set: the operations under one are not checked\n");
#endif // GAOL_TESTS_FLUSH

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

  /*
    pow with a subnormal result gives the tightest enclosure of the exact
    value, whatever the rounding direction of the calling code (GAOL v5). The
    check of the operations above compares it with GAOL's own result when the
    direction is upward; here the bounds are those computed apart, with mpmath
    (see subnormal_powers). With the x87 unit to nearest and the SSE
    instructions upward, the state the predicates of Shewchuk and of Triangle
    leave, CORE-MATH rounded these results to nearest, the direction
    fegetround() gives on x86-64 with glibc, and the upper bound was below the
    exact value.
  */
  for (const Direction& d : directions) {
    for (const SubnormalPower& p : subnormal_powers) {
      set(d);
      const interval z = gaol_ieee1788::pow(interval(p.x), interval(p.y));
      const double lo = subnormal(p.lo), hi = subnormal(p.hi);
      check("gaol_ieee1788::pow with a subnormal result: the tightest enclosure",
            z.left() == lo && z.right() == hi,
            [&] {
              return std::string("rounding direction ") + d.name + ", x=" + hex(p.x) + " y=" + hex(p.y) + ": "
                     + hex(z) + " rather than [" + hex(lo) + ", " + hex(hi) + "]";
            });
    }
  }

  /*
    An empty interval is told empty without an exception (GAOL v5). GAOL holds
    the empty set as the two bounds NaN, and is_empty() compared them with <=,
    which signals the invalid-operation exception on a quiet NaN: a program
    that enabled that exception (feenableexcept() of glibc) died on SIGFPE at
    each emptiness test of an empty interval, and so did the operations, which
    start with it; with the exception masked, the test raised the flag
    FE_INVALID, which a program reading the flags to find its own NaNs took for
    one of its own. interval::emptyset() did as much in a build without
    optimization, where it compared the NaN it builds. Both are quiet now
    (std::islessequal, and no comparison): neither raises a flag, on any
    platform, and neither traps on glibc, where the checks enable the exception
    in a child process.
    The operations with an empty operand, which compared its NaN bounds as
    well, are checked the same way below. The exceptions that GAOL's
    operations raise legitimately (the inexact one for almost each, the
    divide-by-zero and the overflow ones for an infinite bound), and the
    invalid-operation one some raise on nonempty operands, are not checked
    here: the exceptions have to be masked while GAOL computes (doc/using.md).
  */
  for (const NamedInterval& e : empty_sets) {
    const interval empty = e.make();
    std::feclearexcept(FE_ALL_EXCEPT);
    const volatile bool told_empty = empty.is_empty();
    const int raised = std::fetestexcept(FE_ALL_EXCEPT);
    check("is_empty() of an empty set raises no exception flag", raised == 0,
          [&] { return std::string(e.name) + ": flags " + std::to_string(raised) + " raised"; });
    check("is_empty() of an empty set is true", told_empty, [&] { return std::string(e.name); });
  }
  for (const NamedInterval& e : nonempty_sets) {
    const interval nonempty = e.make();
    std::feclearexcept(FE_ALL_EXCEPT);
    const volatile bool told_empty = nonempty.is_empty();
    const int raised = std::fetestexcept(FE_ALL_EXCEPT);
    check("is_empty() of a nonempty interval raises no exception flag", raised == 0,
          [&] { return std::string(e.name) + ": flags " + std::to_string(raised) + " raised"; });
    check("is_empty() of a nonempty interval is false", !told_empty, [&] { return std::string(e.name); });
  }
  {
    std::feclearexcept(FE_ALL_EXCEPT);
    const interval empty = interval::emptyset();
    static_cast<void>(empty);
    const int raised = std::fetestexcept(FE_ALL_EXCEPT);
    check("interval::emptyset() raises no exception flag", raised == 0,
          [&] { return "flags " + std::to_string(raised) + " raised"; });
  }

  // Every operation with an empty operand gives its result without the
  // invalid-operation exception (GAOL v5): the other exceptions (inexact, for
  // the rounding direction checked with an addition) are not checked
  for (const EmptyOperand& e : empty_operands) {
    std::feclearexcept(FE_ALL_EXCEPT);
    const volatile bool right = e.run();
    const int raised = std::fetestexcept(FE_INVALID);
    check("an operation with an empty operand raises no invalid-operation flag", raised == 0,
          [&] { return std::string(e.name); });
    check("an operation with an empty operand gives the result of the empty set", right,
          [&] { return std::string(e.name); });
  }

  // A NaN the program gives, the products with a zero and an infinite bound,
  // the powers with an exponent of extreme magnitude and the relations in
  // loops raise no invalid-operation flag, and the midpoints of intervals
  // with a huge bound no overflow flag (GAOL v5, point D.24)
  for (const EmptyOperand& e : nan_operands) {
    std::feclearexcept(FE_ALL_EXCEPT);
    const volatile bool right = e.run();
    const int raised = std::fetestexcept(FE_INVALID);
    check("an operation with a NaN double raises no invalid-operation flag", raised == 0, [&] { return std::string(e.name); });
    check("an operation with a NaN double gives the result of the empty set", right, [&] { return std::string(e.name); });
  }
  {
    std::feclearexcept(FE_ALL_EXCEPT);
    const volatile bool right = products_with_zero_and_infinite_bounds_are_right();
    const int raised = std::fetestexcept(FE_INVALID);
    check("the products with a zero and an infinite bound raise no invalid-operation flag", raised == 0);
    check("the products with a zero and an infinite bound are right", right);
  }
  {
    std::feclearexcept(FE_ALL_EXCEPT);
    const volatile bool right = quotients_with_zero_and_infinite_bounds_are_right();
    const int raised = std::fetestexcept(FE_INVALID | FE_DIVBYZERO);
    check("div_rel with zero and infinite bounds raises no invalid-operation nor division-by-zero flag", raised == 0,
          [&] { return std::string((raised & FE_INVALID) ? "invalid-operation" : "division-by-zero"); });
    check("div_rel with zero and infinite bounds: the quotients", right);
  }
  for (const EmptyOperand& e : extreme_powers) {
    std::feclearexcept(FE_ALL_EXCEPT);
    const volatile bool right = e.run();
    const int raised = std::fetestexcept(FE_INVALID);
    check("pow with an exponent of extreme magnitude raises no invalid-operation flag", raised == 0,
          [&] { return std::string(e.name); });
    check("pow with an exponent of extreme magnitude: the tightest enclosure", right, [&] { return std::string(e.name); });
  }
  {
    /* The differences of the bounds of cancel_minus() are computed by TwoSum
       in the rounding to nearest, their terms kept before the direction
       changes back: GCC computed them after it, upward, where TwoSum is no
       longer exact, and the terms of the error of
       cancel_minus([DBL_MAX], [1]) read +oo and inf - inf, and raised the
       invalid-operation flag (GAOL v5, point 48; the bounds are checked in
       tests/arithmetic.cpp) */
    std::feclearexcept(FE_ALL_EXCEPT);
    const interval z = cancel_minus(interval((std::numeric_limits<double>::max)()), interval(1.0));
    const int raised = std::fetestexcept(FE_INVALID);
    check("cancel_minus with a huge bound raises no invalid-operation flag", raised == 0,
          [&] { return "flags " + std::to_string(raised) + " raised"; });
    check("cancel_minus([DBL_MAX], [1]) is the hull of DBL_MAX",
          z.left() == std::nextafter((std::numeric_limits<double>::max)(), 0.0)
              && z.right() == (std::numeric_limits<double>::max)(),
          [&] { return hex(z); });
  }
  const std::vector<HugeInterval> huge = huge_intervals();
  check("the intervals with a huge bound: the subnormal bound is 2^-1074",
        huge.back().l == std::ldexp(gaol::rnd_keep(1.0), -1074) && huge.back().r == (std::numeric_limits<double>::max)(),
        [&] { return "[" + hex(huge.back().l) + ", " + hex(huge.back().r) + "]"; });
  std::vector<Midpoints> huge_midpoints;
  for (const HugeInterval& h : huge) {
    std::feclearexcept(FE_ALL_EXCEPT);
    huge_midpoints.push_back(midpoints_of(h));
    const int raised = std::fetestexcept(FE_OVERFLOW);
    const Midpoints& m = huge_midpoints.back();
    check("the midpoints of an interval with a huge bound raise no overflow flag", raised == 0, [&] { return std::string(h.name); });
    check("the midpoints of an interval with a huge bound are right", midpoints_are_right(h, m),
          [&] {
            return std::string(h.name) + ": midpoint() " + hex(m.midpoint) + ", mid() " + hex(m.mid) + ", rad() " + hex(m.rad)
                 + ", split() " + hex(m.split_l) + " " + hex(m.split_r);
          });
  }
  for (const RelationLoop& r : relation_loops) {
    std::feclearexcept(FE_ALL_EXCEPT);
    const volatile bool right = relation_loop_is_right(r);
    const int raised = std::fetestexcept(FE_INVALID);
    check("a loop of relations with empty operands raises no invalid-operation flag", raised == 0, [&] { return std::string(r.name); });
    check("a loop of relations with empty operands gives the relations", right, [&] { return std::string(r.name); });
  }

#if GAOL_TESTS_TRAPS
  // The empty sets, computed and told empty with the invalid-operation
  // exception enabled: what the processor delivers where it can, which the
  // control shows first (the FPU of some processors, and of the emulators of
  // others, has no exception to enable)
  if (run_with_exceptions_enabled(FE_INVALID, invalid_operation) != trapped) {
    std::printf("The processor does not trap an invalid operation here: the checks with the "
                "exception enabled are skipped\n");
  } else {
    for (const NamedInterval& e : empty_sets) {
      const Outcome o = run_with_exceptions_enabled(FE_INVALID, [&e] { return e.make().is_empty(); });
      check("is_empty() of an empty set, FE_INVALID enabled", o == returned_true,
            [&] { return std::string(e.name) + ": " + outcome_text(o); });
    }
    for (const NamedInterval& e : nonempty_sets) {
      const Outcome o = run_with_exceptions_enabled(FE_INVALID, [&e] { return !e.make().is_empty(); });
      check("is_empty() of a nonempty interval, FE_INVALID enabled", o == returned_true,
            [&] { return std::string(e.name) + ": " + outcome_text(o); });
    }
    const Outcome o = run_with_exceptions_enabled(FE_INVALID, [] { return interval::emptyset().is_empty(); });
    check("interval::emptyset().is_empty(), FE_INVALID enabled", o == returned_true,
          [&] { return outcome_text(o); });
    for (const EmptyOperand& e : empty_operands) {
      const Outcome oe = run_with_exceptions_enabled(FE_INVALID, e.run);
      check("an operation with an empty operand, FE_INVALID enabled", oe == returned_true,
            [&] { return std::string(e.name) + ": " + outcome_text(oe); });
    }
    for (const EmptyOperand& e : nan_operands) {
      const Outcome oe = run_with_exceptions_enabled(FE_INVALID, e.run);
      check("an operation with a NaN double, FE_INVALID enabled", oe == returned_true,
            [&] { return std::string(e.name) + ": " + outcome_text(oe); });
    }
    const Outcome op = run_with_exceptions_enabled(FE_INVALID, products_with_zero_and_infinite_bounds_are_right);
    check("the products with a zero and an infinite bound, FE_INVALID enabled", op == returned_true,
          [&] { return outcome_text(op); });
    const Outcome oq = run_with_exceptions_enabled(FE_INVALID | FE_DIVBYZERO, quotients_with_zero_and_infinite_bounds_are_right);
    check("div_rel with zero and infinite bounds, FE_INVALID and FE_DIVBYZERO enabled", oq == returned_true,
          [&] { return outcome_text(oq); });
    for (const EmptyOperand& e : extreme_powers) {
      const Outcome oe = run_with_exceptions_enabled(FE_INVALID, e.run);
      check("pow with an exponent of extreme magnitude, FE_INVALID enabled", oe == returned_true,
            [&] { return std::string(e.name) + ": " + outcome_text(oe); });
    }
    const Outcome oc = run_with_exceptions_enabled(FE_INVALID, [] {
      const interval z = cancel_minus(interval((std::numeric_limits<double>::max)()), interval(1.0));
      return z.left() == std::nextafter((std::numeric_limits<double>::max)(), 0.0)
          && z.right() == (std::numeric_limits<double>::max)();
    });
    check("cancel_minus([DBL_MAX], [1]), FE_INVALID enabled", oc == returned_true,
          [&] { return outcome_text(oc); });
    for (const RelationLoop& r : relation_loops) {
      const Outcome ol = run_with_exceptions_enabled(FE_INVALID, [&r] { return relation_loop_is_right(r); });
      check("a loop of relations with empty operands, FE_INVALID enabled", ol == returned_true,
            [&] { return std::string(r.name) + ": " + outcome_text(ol); });
    }
    // The exception the program enabled stays enabled after each operation
    // (GAOL v5, point D.24): with GAOL_PRESERVE_ROUNDING, the SSE2 operations
    // wrote the SSE control register with every exception masked
    const Outcome om = run_with_exceptions_enabled(FE_INVALID, [&] {
      bool enabled = true;
      for (const Operation& o : operations) {
        static_cast<void>(run(o, operands[0].first, operands[0].second));
#if GAOL_TESTS_SSE
        enabled = enabled && (_mm_getcsr() & _MM_MASK_INVALID) == 0u;
#else
        enabled = enabled && (fegetexcept() & FE_INVALID) != 0;
#endif
      }
      return enabled;
    });
    check("the invalid-operation exception the program enabled stays enabled after every operation", om == returned_true,
          [&] { return outcome_text(om); });
  }
  if (run_with_exceptions_enabled(FE_OVERFLOW, overflow_operation) != trapped) {
    std::printf("The processor does not trap an overflow here: the midpoints with the overflow exception "
                "enabled are not checked\n");
  } else {
    for (std::size_t i = 0; i < huge_midpoints.size(); ++i) {
      // The midpoints computed above, apart from the exception enabled
      const Outcome oe = run_with_exceptions_enabled(FE_OVERFLOW, [&] { return midpoints_of(huge[i]) == huge_midpoints[i]; });
      check("the midpoints of an interval with a huge bound, FE_OVERFLOW enabled", oe == returned_true,
            [&] { return std::string(huge[i].name) + ": " + outcome_text(oe); });
    }
  }
#else
  std::printf("feenableexcept() and fork() are those of glibc: the checks with the exceptions enabled are skipped\n");
#endif

  /*
    gaol::cleanup() sets back the rounding direction the first gaol::init()
    found, the x87 unit and the SSE instructions each theirs (GAOL v5): to
    nearest, as a program starts (C11, F.8.3), GAOL initializing itself before
    the static objects of the program. With GAOL_PRESERVE_ROUNDING, it leaves
    the direction as it is.
  */
  set(directions[0]);
  const State before_cleanup = state();
  gaol::cleanup();
  const State after_cleanup = state();
#if GAOL_PRESERVE_ROUNDING
  check("rounding direction unchanged by gaol::cleanup()", after_cleanup == before_cleanup,
        [&] { return text(after_cleanup) + " after it, " + text(before_cleanup) + " before"; });
#else
  const State to_nearest = { FE_TONEAREST, SSE_DIRECTION(_MM_ROUND_NEAREST) };
  check("rounding direction set back to nearest by gaol::cleanup()", after_cleanup == to_nearest,
        [&] { return text(after_cleanup) + " after it, " + text(before_cleanup) + " before"; });
#endif

  /*
    gaol::restore_rounding() sets back the direction that the first
    gaol::init() found, as many times as the program needs, where gaol::cleanup()
    does so at its first call only (GAOL v5): a program that goes back to GAOL
    after cleanup() calls it after each use, the operations of GAOL setting the
    direction upward again. With GAOL_PRESERVE_ROUNDING, the operations restore
    the direction themselves and restore_rounding() does nothing.
  */
#if !GAOL_PRESERVE_ROUNDING
  gaol::restore_rounding();
  const State after_restore = state();
  check("rounding direction set back to nearest by gaol::restore_rounding()", after_restore == to_nearest,
        [&] { return text(after_restore); });
  const interval back = interval(1.0) + interval(1.0);   // sets the direction upward again
  static_cast<void>(back);
  const State after_operation = state();
  gaol::restore_rounding();
  gaol::restore_rounding();
  const State after_twice = state();
  /* With GAOL_PREFER_AVX512, on a processor that has the AVX-512
     instructions, the addition takes the embedded rounding: it sets no
     direction, and the one the program left stays (to nearest here, as
     restore_rounding() set it), as it stays with GAOL_PRESERVE_ROUNDING
     (GAOL v5, gaol/gaol_interval_avx512.cpp) */
#if defined(GAOL_USING_SSE2_INSTRUCTIONS) && GAOL_USING_SSE2_INSTRUCTIONS \
    && defined(GAOL_HAVE_AVX512_TARGET) && GAOL_HAVE_AVX512_TARGET && GAOL_PREFER_AVX512
  const bool embedded_add = __builtin_cpu_supports("avx512f") != 0;
  check("rounding direction set back by gaol::restore_rounding() as many times as needed",
        embedded_add ? (after_operation == to_nearest && after_twice == to_nearest)
                     : (!(after_operation == to_nearest) && after_twice == to_nearest),
        [&] {
          return text(after_twice) + " after it, " + text(after_operation) + " after the operation";
        });
#else
  check("rounding direction set back by gaol::restore_rounding() as many times as needed",
        !(after_operation == to_nearest) && after_twice == to_nearest,
        [&] {
          return text(after_twice) + " after it, " + text(after_operation) + " after the operation";
        });
#endif
#else
  gaol::restore_rounding();
  const State after_restore = state();
  check("gaol::restore_rounding() leaves the direction of the program with GAOL_PRESERVE_ROUNDING",
        after_restore == after_cleanup, [&] { return text(after_restore); });
#endif
  return summary();
}
