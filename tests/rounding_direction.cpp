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
 * instructions differing too (GAOL v5). At the end, gaol::cleanup() has to set
 * back the direction the first gaol::init() found (GAOL v5).
 *
 * The rest of the floating-point environment is checked too: the exceptions
 * stay masked, and an empty interval is told empty, with interval::emptyset(),
 * and every operation computes with an empty operand, without raising the
 * invalid-operation exception, which kills a program that enabled it (GAOL v5).
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
#include <functional>
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

  // The empty sets of the checks below, each computed in a way of its own from
  // operands the compiler does not know: the constant, an interval whose
  // bounds are in the wrong order, a function outside its domain, disjoint
  // intervals. None of them compares a NaN, so that each is computed without
  // an exception with the invalid-operation exception enabled.
  struct EmptySet
  {
    const char *name;
    interval (*make)();
  };

  const EmptySet empty_sets[] = {
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
  const EmptySet nonempty_sets[] = {
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
    them were already quiet, but the last: a choice the program makes on the
    result of an intersection with an empty left operand, which GCC for 32-bit
    ARM compiled with a signaling comparison where operator&= told that
    operand empty otherwise than is_empty() does.
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
    // A value the program chooses with is_empty() of an intersection whose
    // left operand is empty: GCC for 32-bit ARM made the choice a conditional
    // move, and compared the NaN bounds again with the signaling vcmpe where
    // it did not know the answer from operator&= (see gaol/gaol_interval_fpu.h).
    // Such a choice is not quiet everywhere (doc/using.md): the same one with
    // an empty right operand is not checked, as GCC for POWER9 compiles it
    // with a signaling comparison
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
  };

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
    const double a = random.uniform(0.01, 0.6), b = random.uniform(1.1, 2.0);
    operands.push_back(std::make_pair(interval(a, a + random.uniform(0.001, 0.39)), interval(b, b + random.uniform(0.001, 1.0))));
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
  for (const EmptySet& e : empty_sets) {
    const interval empty = e.make();
    std::feclearexcept(FE_ALL_EXCEPT);
    const volatile bool told_empty = empty.is_empty();
    const int raised = std::fetestexcept(FE_ALL_EXCEPT);
    check("is_empty() of an empty set raises no exception flag", raised == 0,
          [&] { return std::string(e.name) + ": flags " + std::to_string(raised) + " raised"; });
    check("is_empty() of an empty set is true", told_empty, [&] { return std::string(e.name); });
  }
  for (const EmptySet& e : nonempty_sets) {
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

#if GAOL_TESTS_TRAPS
  // The empty sets, computed and told empty with the invalid-operation
  // exception enabled: what the processor delivers where it can, which the
  // control shows first (the FPU of some processors, and of the emulators of
  // others, has no exception to enable)
  if (run_with_exceptions_enabled(FE_INVALID, invalid_operation) != trapped) {
    std::printf("The processor does not trap an invalid operation here: the checks with the "
                "exception enabled are skipped\n");
  } else {
    for (const EmptySet& e : empty_sets) {
      const Outcome o = run_with_exceptions_enabled(FE_INVALID, [&e] { return e.make().is_empty(); });
      check("is_empty() of an empty set, FE_INVALID enabled", o == returned_true,
            [&] { return std::string(e.name) + ": " + outcome_text(o); });
    }
    for (const EmptySet& e : nonempty_sets) {
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
  return summary();
}
