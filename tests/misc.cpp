// GAOL 4's unit test check/misc.cpp, run with the checks of unit_tests.h
// rather than CppUnit (GAOL v5)
// The expressions, with their nodes, visitor and evaluator, for the names of
// the namespace gaol below
#include "gaol/parser/gaol_expr_eval.h"
#include "unit_tests.h"

#include <cstddef>
#include <cstdint>
#include <cstdio>
#include <deque>
#include <istream>
#include <list>
#include <map>
#include <memory>
#include <new>
#include <ostream>
#include <string>
#include <type_traits>
#include <utility>
#include <vector>

// A namespace detail of the program's own, with using namespace gaol (in
// unit_tests.h): GAOL's helpers were in gaol_core::detail, which the namespace
// gaol took whole, and detail::of_the_program() did not compile, "reference
// to 'detail' is ambiguous" (GAOL v5, before its release). They are in
// gaol_detail, a namespace of its own
namespace detail {
  inline int of_the_program() { return 1; }
}

/*
  The names of the namespace gaol (GAOL v5), which names those of gaol_core one
  by one rather than taking it whole (see gaol/gaol_interval.h): the names of
  GAOL 4 that Codac and IBEX write gaol::f, the other classes and functions of
  GAOL 4 that gaol still names, the functions on intervals GAOL v5 adds, and
  restore_rounding() and exact_string(), each with the overloads of the
  expressions where there are some. A name missing does not compile. The names
  that gaol does not hold, and that a program names in gaol_core, are checked
  absent below.
*/
namespace gaol_names
{
  using gaol_core::interval;
  using gaol_core::expression;

  template <class A, class B>
  constexpr bool same() { return std::is_same<A, B>::value; }

  const interval& x();
  const expression& e();
  std::ostream& out();

#define GAOL_NAMES_CLASS(c) static_assert(same<gaol::c, gaol_core::c>(), "gaol::" #c)
  GAOL_NAMES_CLASS(interval);
  GAOL_NAMES_CLASS(interval_format);
  GAOL_NAMES_CLASS(expression);
  GAOL_NAMES_CLASS(expr_node);
  GAOL_NAMES_CLASS(expr_visitor);
  GAOL_NAMES_CLASS(expr_eval);
  GAOL_NAMES_CLASS(null_node);
  GAOL_NAMES_CLASS(double_node);
  GAOL_NAMES_CLASS(interval_node);
  GAOL_NAMES_CLASS(add_node);
  GAOL_NAMES_CLASS(sub_node);
  GAOL_NAMES_CLASS(mult_node);
  GAOL_NAMES_CLASS(div_node);
  GAOL_NAMES_CLASS(unary_minus_node);
  GAOL_NAMES_CLASS(pow_node);
  GAOL_NAMES_CLASS(pow_itv_node);
  GAOL_NAMES_CLASS(nth_root_node);
  GAOL_NAMES_CLASS(sin_node);
  GAOL_NAMES_CLASS(asinh_node);
  GAOL_NAMES_CLASS(log_node);
  // The nodes of the functions GAOL v5 adds to the expressions
  GAOL_NAMES_CLASS(exp2_node);
  GAOL_NAMES_CLASS(log2_node);
  GAOL_NAMES_CLASS(sign_node);
  GAOL_NAMES_CLASS(trunc_node);
  // The comparator of the containers of the standard library (GAOL v5, point V)
  GAOL_NAMES_CLASS(lexicographic_less);
#undef GAOL_NAMES_CLASS

  // The version of the library, in gaol and in gaol_ieee1788 (GAOL v5)
  static_assert(same<decltype(gaol::version), const char *const>() && gaol::version_major == GAOL_MAJOR_VERSION
                && gaol::version_minor == GAOL_MINOR_VERSION && gaol::version_micro == GAOL_MICRO_VERSION,
                "gaol::version, gaol::version_major...");
  static_assert(same<decltype(gaol_ieee1788::version), const char *const>() && gaol_ieee1788::version_major == GAOL_MAJOR_VERSION
                && gaol_ieee1788::version_minor == GAOL_MINOR_VERSION && gaol_ieee1788::version_micro == GAOL_MICRO_VERSION,
                "gaol_ieee1788::version, gaol_ieee1788::version_major...");

  // The doubles around pi and pi/2 of gaol_core, in gaol and in gaol_ieee1788 (GAOL v5): a using-declaration
  // names the same object, so that the names compile and are doubles
  static_assert(same<decltype(gaol::gaol_pi_dn), const double>() && same<decltype(gaol::gaol_pi_up), const double>()
                && same<decltype(gaol::gaol_half_pi_dn), const double>() && same<decltype(gaol::gaol_half_pi_up), const double>(),
                "gaol::gaol_pi_dn, gaol::gaol_pi_up, gaol::gaol_half_pi_dn, gaol::gaol_half_pi_up");
  static_assert(same<decltype(gaol_ieee1788::gaol_pi_dn), const double>() && same<decltype(gaol_ieee1788::gaol_pi_up), const double>()
                && same<decltype(gaol_ieee1788::gaol_half_pi_dn), const double>()
                && same<decltype(gaol_ieee1788::gaol_half_pi_up), const double>(),
                "gaol_ieee1788::gaol_pi_dn, gaol_ieee1788::gaol_pi_up...");

  // The functions of one interval, of GAOL 4 and of GAOL v5
#define GAOL_NAMES_UNARY(f) static_assert(same<decltype(gaol::f(x())), interval>(), "gaol::" #f "(x)")
  GAOL_NAMES_UNARY(abs);
  GAOL_NAMES_UNARY(sqr);
  GAOL_NAMES_UNARY(sqrt);
  GAOL_NAMES_UNARY(exp);
  GAOL_NAMES_UNARY(log);
  GAOL_NAMES_UNARY(sin);
  GAOL_NAMES_UNARY(cos);
  GAOL_NAMES_UNARY(tan);
  GAOL_NAMES_UNARY(asin);
  GAOL_NAMES_UNARY(acos);
  GAOL_NAMES_UNARY(atan);
  GAOL_NAMES_UNARY(sinh);
  GAOL_NAMES_UNARY(cosh);
  GAOL_NAMES_UNARY(tanh);
  GAOL_NAMES_UNARY(asinh);
  GAOL_NAMES_UNARY(acosh);
  GAOL_NAMES_UNARY(atanh);
  GAOL_NAMES_UNARY(floor);
  GAOL_NAMES_UNARY(ceil);
  GAOL_NAMES_UNARY(integer);
  GAOL_NAMES_UNARY(inverse);
  GAOL_NAMES_UNARY(exp2);
  GAOL_NAMES_UNARY(exp10);
  GAOL_NAMES_UNARY(log2);
  GAOL_NAMES_UNARY(log10);
  GAOL_NAMES_UNARY(expm1);
  GAOL_NAMES_UNARY(exp2m1);
  GAOL_NAMES_UNARY(exp10m1);
  GAOL_NAMES_UNARY(log1p);
  GAOL_NAMES_UNARY(log2p1);
  GAOL_NAMES_UNARY(log10p1);
  GAOL_NAMES_UNARY(rsqrt);
  GAOL_NAMES_UNARY(sinpi);
  GAOL_NAMES_UNARY(cospi);
  GAOL_NAMES_UNARY(tanpi);
  GAOL_NAMES_UNARY(asinpi);
  GAOL_NAMES_UNARY(acospi);
  GAOL_NAMES_UNARY(atanpi);
  GAOL_NAMES_UNARY(erf);
  GAOL_NAMES_UNARY(erfc);
  GAOL_NAMES_UNARY(trunc);
  GAOL_NAMES_UNARY(sign);
  GAOL_NAMES_UNARY(round_ties_to_even);
  GAOL_NAMES_UNARY(round_ties_to_away);
#undef GAOL_NAMES_UNARY

  // The functions of several intervals, the relational ones among them
#define GAOL_NAMES_BINARY(f) static_assert(same<decltype(gaol::f(x(), x())), interval>(), "gaol::" #f "(x, y)")
  GAOL_NAMES_BINARY(min);
  GAOL_NAMES_BINARY(max);
  GAOL_NAMES_BINARY(atan2);
  GAOL_NAMES_BINARY(atan2pi);
  GAOL_NAMES_BINARY(hypot);
  GAOL_NAMES_BINARY(hull);
  GAOL_NAMES_BINARY(intersect);
  GAOL_NAMES_BINARY(cancel_minus);
  GAOL_NAMES_BINARY(cancel_plus);
  GAOL_NAMES_BINARY(sqrt_rel);
  GAOL_NAMES_BINARY(invabs_rel);
  GAOL_NAMES_BINARY(acos_rel);
  GAOL_NAMES_BINARY(asin_rel);
  GAOL_NAMES_BINARY(atan_rel);
  GAOL_NAMES_BINARY(acosh_rel);
  GAOL_NAMES_BINARY(asinh_rel);
  GAOL_NAMES_BINARY(atanh_rel);
  GAOL_NAMES_BINARY(sign_rel);
  GAOL_NAMES_BINARY(floor_rel);
#undef GAOL_NAMES_BINARY
  static_assert(same<decltype(gaol::div_rel(x(), x(), x())), interval>(), "gaol::div_rel(z, y, x)");
  static_assert(same<decltype(gaol::max_rel(x(), x(), x())), interval>() && same<decltype(gaol::min_rel(x(), x(), x())), interval>(),
                "gaol::max_rel(z, y, x), gaol::min_rel(z, y, x)");
  static_assert(same<decltype(gaol::atan2_rel_y(x(), x(), x())), interval>()
                && same<decltype(gaol::atan2_rel_x(x(), x(), x())), interval>(),
                "gaol::atan2_rel_y(c, x, y), gaol::atan2_rel_x(c, y, x)");
  static_assert(same<decltype(gaol::fma(x(), x(), x())), interval>(), "gaol::fma(x, y, z)");
  static_assert(same<decltype(gaol::nth_root(x(), 3u)), interval>() && same<decltype(gaol::nth_root(x(), 3)), interval>(),
                "gaol::nth_root(x, n)");
  static_assert(same<decltype(gaol::nth_root_rel(x(), 3u, x())), interval>(), "gaol::nth_root_rel(y, n, x)");
  static_assert(same<decltype(gaol::pow_rel(x(), x(), x())), interval>()
                && same<decltype(gaol::pow_exponent_rel(x(), x(), x())), interval>(),
                "gaol::pow_rel(z, y, x), gaol::pow_exponent_rel(z, x, y)");
  static_assert(same<decltype(gaol::pow(x(), 3)), interval>() && same<decltype(gaol::pow(x(), x())), interval>(),
                "gaol::pow, declared in gaol");
  static_assert(same<decltype(gaol::textToInterval(std::string())), interval>(), "gaol::textToInterval, declared in gaol");
  static_assert(same<decltype(gaol::exact_string(x())), std::string>(), "gaol::exact_string(x)");
  static_assert(same<decltype(gaol::chi(x())), double>() && same<decltype(gaol::hausdorff(x(), x())), double>(),
                "gaol::chi(x), gaol::hausdorff(x, y)");

  // The operators, which Codac writes gaol::operator+(x, y)...
  static_assert(same<decltype(gaol::operator+(x(), x())), interval>() && same<decltype(gaol::operator-(x(), x())), interval>()
                && same<decltype(gaol::operator*(x(), x())), interval>() && same<decltype(gaol::operator/(x(), x())), interval>()
                && same<decltype(gaol::operator%(x(), x())), interval>() && same<decltype(gaol::operator&(x(), x())), interval>()
                && same<decltype(gaol::operator|(x(), x())), interval>() && same<decltype(gaol::operator+(x(), 1.0)), interval>()
                && same<decltype(gaol::operator<(x(), x())), bool>() && same<decltype(gaol::operator<=(x(), 1.0)), bool>()
                && same<decltype(gaol::operator>(x(), x())), bool>() && same<decltype(gaol::operator>=(x(), x())), bool>(),
                "gaol::operator+, gaol::operator|...");
  static_assert(same<decltype(gaol::operator<<(out(), x())), std::ostream&>()
                && same<decltype(gaol::operator>>(std::declval<std::istream&>(), std::declval<interval&>())), std::istream&>(),
                "gaol::operator<<, gaol::operator>> of intervals");

  // The overloads of the expressions, which gaol/parser/gaol_expression.h names
  // again after gaol/gaol_interval.h did: a using-declaration names those
  // declared before it only
#define GAOL_NAMES_EXPRESSION(f) static_assert(same<decltype(gaol::f(e())), const expression>(), "gaol::" #f "(e)")
  GAOL_NAMES_EXPRESSION(cos);
  GAOL_NAMES_EXPRESSION(sin);
  GAOL_NAMES_EXPRESSION(tan);
  GAOL_NAMES_EXPRESSION(acos);
  GAOL_NAMES_EXPRESSION(asin);
  GAOL_NAMES_EXPRESSION(atan);
  GAOL_NAMES_EXPRESSION(cosh);
  GAOL_NAMES_EXPRESSION(sinh);
  GAOL_NAMES_EXPRESSION(tanh);
  GAOL_NAMES_EXPRESSION(acosh);
  GAOL_NAMES_EXPRESSION(asinh);
  GAOL_NAMES_EXPRESSION(atanh);
  GAOL_NAMES_EXPRESSION(exp);
  GAOL_NAMES_EXPRESSION(log);
  GAOL_NAMES_EXPRESSION(exp2);
  GAOL_NAMES_EXPRESSION(log2);
  GAOL_NAMES_EXPRESSION(sign);
  GAOL_NAMES_EXPRESSION(trunc);
  GAOL_NAMES_EXPRESSION(operator-);
#undef GAOL_NAMES_EXPRESSION
  static_assert(same<decltype(gaol::atan2(e(), e())), const expression>()
                && same<decltype(gaol::nth_root(e(), 3u)), const expression>()
                && same<decltype(gaol::nth_root(e(), 3)), const expression>()
                && same<decltype(gaol::pow(e(), 3)), const expression>() && same<decltype(gaol::pow(e(), e())), const expression>()
                && same<decltype(gaol::operator+(e(), e())), const expression>()
                && same<decltype(gaol::operator-(e(), e())), const expression>()
                && same<decltype(gaol::operator*(e(), e())), const expression>()
                && same<decltype(gaol::operator/(e(), e())), const expression>(),
                "gaol::atan2(e1, e2), gaol::nth_root(e, n), gaol::pow(e, n), gaol::operator+(e1, e2)...");
  static_assert(same<decltype(gaol::operator<<(out(), e())), std::ostream&>(),
                "gaol::operator<< of the expressions");
  static_assert(same<decltype(gaol::evaluate_expr(e(), std::declval<interval&>())), bool>(), "gaol::evaluate_expr");

  // The functions of doubles, the initialization and the rounding of GAOL 4,
  // and restore_rounding() of GAOL v5
  static_assert(same<decltype(gaol::next_float(1.0)), double>() && same<decltype(gaol::previous_float(1.0)), double>()
                && same<decltype(gaol::maximum(1.0, 2.0)), double>() && same<decltype(gaol::minimum(1.0, 2.0)), double>()
                && same<decltype(gaol::is_finite(1.0)), int>(),
                "gaol::next_float, gaol::previous_float, gaol::maximum, gaol::minimum, gaol::is_finite");
  static_assert(same<decltype(gaol::init()), bool>() && same<decltype(gaol::cleanup()), bool>()
                && same<decltype(gaol::restore_rounding()), void>() && same<decltype(gaol::round_upward()), void>()
                && same<decltype(gaol::round_downward()), void>() && same<decltype(gaol::round_nearest()), void>(),
                "gaol::init, gaol::round_upward, gaol::restore_rounding...");
}

/*
  What gaol no longer names: what GAOL's code uses for itself, and the
  exceptions, rnd_keep(), NaN_val, eval_stack and the reader of
  strings, which stay in gaol_core (GAOL v5). gaol took gaol_core
  whole before, and these names were found through it. Each is looked for in the namespaces that using namespace
  gaol (unit_tests.h) opens, along with gaol_names_absent below:
  - a function, without argument-dependent lookup (the name between
    parentheses), which would find the functions of an interval in gaol_core
    whatever gaol holds. Its fallback below takes any arguments through the
    ellipsis, the worst of conversions, or none, as a function template,
    which a function that is not one beats: a function of GAOL is chosen when
    gaol names it, and the fallback otherwise, of the type absent;
  - a type or a variable, which has a fallback of the same name: had gaol one
    too, the name would be ambiguous, and the test would not compile.
*/
namespace gaol_names_absent
{
  struct absent {};
  template <class T = void> absent get_rounding();
  absent set_rounding(...);
  absent set_rounding_and_flush_modes(...);
  template <class T = void> absent round_upward_if_needed();
  absent gaol_set_rounding_x86(...);
  template <class T = void> absent get_flush_modes();
  absent set_flush_modes(...);
  template <class T = void> absent clear_flush_to_zero();
  absent gaol_pown(...);
  absent gaol_uipow(...);
  absent gaol_pown_ll(...);
  absent gaol_pow_real(...);
  absent gaol_pow_hybrid(...);
  absent gaol_pown_exp(...);
  absent gaol_pow_exp(...);
  absent gaol_sign_of(...);
  absent rnd_reread(...);
  absent rnd_keep(...);
  absent parse_interval(...);
  // The internals GAOL 4 declared in gaol too, which its manual did not document
  absent gaol_signbit(...);
  absent modulo_k_pi(...);
  template <class T = void> absent round_downward_sse();
  template <class T = void> absent round_to_nearest_sse();
  template <class T = void> absent round_upward_sse();
  using uintdouble = absent;
  using ullidouble = absent;
  using Interval_struct = absent;
  using prec_t = absent;
  constexpr absent the_null_expr{};
  using rounding_state = absent;
  using rounding_guard = absent;
  using gaol_initializer = absent;
  constexpr absent _gaol_initializer{};
  // The exceptions, the reader of strings, the stack of evaluation and NaN_val,
  // which GAOL 4 declared in gaol and which are in gaol_core only
  using gaol_exception = absent;
  using input_format_error = absent;
  using unavailable_feature_error = absent;
  using invalid_action_error = absent;
  using parsing_names = absent;
  template <class... T> using eval_stack = absent;
  constexpr absent NaN_val{};
  // For the check that the fallbacks lose to the functions of gaol
  absent next_float(...);
  template <class T = void> absent round_upward();
}

namespace gaol_names
{
  using namespace gaol_names_absent;

#define GAOL_NAMES_ABSENT(call) static_assert(same<decltype(call), absent>(), #call ": not in gaol")
  GAOL_NAMES_ABSENT((get_rounding)());
  GAOL_NAMES_ABSENT((set_rounding)(std::declval<const gaol_core::rounding_state&>()));
  GAOL_NAMES_ABSENT((set_rounding_and_flush_modes)(std::declval<const gaol_core::rounding_state&>()));
  GAOL_NAMES_ABSENT((round_upward_if_needed)());
  GAOL_NAMES_ABSENT((gaol_set_rounding_x86)(0, 0u));
  GAOL_NAMES_ABSENT((get_flush_modes)());
  GAOL_NAMES_ABSENT((set_flush_modes)(0u));
  GAOL_NAMES_ABSENT((clear_flush_to_zero)());
  GAOL_NAMES_ABSENT((gaol_pown)(x(), 3));
  GAOL_NAMES_ABSENT((gaol_uipow)(x(), 3u));
  GAOL_NAMES_ABSENT((gaol_pown_ll)(x(), 3LL));
  GAOL_NAMES_ABSENT((gaol_pow_real)(x(), 0.5));
  GAOL_NAMES_ABSENT((gaol_pow_hybrid)(x(), x()));
  GAOL_NAMES_ABSENT((gaol_pown_exp)(e(), 3));
  GAOL_NAMES_ABSENT((gaol_pow_exp)(e(), e()));
  GAOL_NAMES_ABSENT((gaol_sign_of)(1.0));
  GAOL_NAMES_ABSENT((rnd_reread)(1.0));
  GAOL_NAMES_ABSENT((rnd_keep)(1.0));
  GAOL_NAMES_ABSENT((parse_interval)("1", std::declval<gaol_core::interval&>()));
  GAOL_NAMES_ABSENT((gaol_signbit)(1.0));
  GAOL_NAMES_ABSENT((modulo_k_pi)(x(), std::declval<double&>(), std::declval<double&>()));
  GAOL_NAMES_ABSENT((round_downward_sse)());
  GAOL_NAMES_ABSENT((round_to_nearest_sse)());
  GAOL_NAMES_ABSENT((round_upward_sse)());
#undef GAOL_NAMES_ABSENT
  static_assert(same<uintdouble, absent>() && same<ullidouble, absent>() && same<Interval_struct, absent>()
                && same<prec_t, absent>() && same<decltype(the_null_expr), const absent>(),
                "uintdouble, ullidouble, Interval_struct, prec_t, the_null_expr: not in gaol");
  static_assert(same<rounding_state, absent>() && same<rounding_guard, absent>() && same<gaol_initializer, absent>(),
                "rounding_state, rounding_guard, gaol_initializer: not in gaol");
  static_assert(same<gaol_exception, absent>() && same<input_format_error, absent>()
                && same<unavailable_feature_error, absent>() && same<invalid_action_error, absent>(),
                "gaol_exception, input_format_error, unavailable_feature_error, invalid_action_error: not in gaol");
  static_assert(same<parsing_names, absent>() && same<eval_stack<int>, absent>(),
                "parsing_names, eval_stack: not in gaol");
  static_assert(same<decltype(NaN_val), const absent>(), "NaN_val: not in gaol");
  static_assert(same<decltype(_gaol_initializer), const absent>(), "_gaol_initializer: not in gaol");
  // The detection works: the functions of gaol beat the fallbacks
  static_assert(same<decltype((next_float)(1.0)), double>() && same<decltype((round_upward)()), void>(),
                "(next_float)(1.0) and (round_upward)() are found in gaol");
}

/*
  gaol_ieee1788 names the output operator of gaol_core as it names its functions
  (GAOL v5), for the intervals and, with the headers of the expressions
  (gaol/parser/gaol_expr_eval.h above), for the expressions: a name missing
  does not compile.
*/
namespace gaol_ieee1788_names
{
  static_assert(std::is_same<decltype(gaol_ieee1788::operator<<(std::declval<std::ostream&>(),
                                                                std::declval<const gaol_core::interval&>())),
                             std::ostream&>::value,
                "gaol_ieee1788::operator<< of the intervals");
  static_assert(std::is_same<decltype(gaol_ieee1788::operator<<(std::declval<std::ostream&>(),
                                                                std::declval<const gaol_core::expression&>())),
                             std::ostream&>::value,
                "gaol_ieee1788::operator<< of the expressions");
}

/*
  A call f(x) on intervals, where no other f is found, finds the functions of
  gaol_core by argument-dependent lookup, as before: these f hide every other
  one, those of gaol and of <cmath> among them.
*/
namespace gaol_names_adl
{
  struct hidden {};
  void sin(hidden);
  void exp2(hidden);
  void trunc(hidden);
  void nth_root(hidden);
  void cancel_minus(hidden);

  inline bool same_as_gaol_core(const gaol_core::interval& x)
  {
    return sin(x).set_eq(gaol_core::sin(x)) && exp2(x).set_eq(gaol_core::exp2(x))
      && trunc(x).set_eq(gaol_core::trunc(x)) && nth_root(x, 3).set_eq(gaol_core::nth_root(x, 3))
      && cancel_minus(x, x).set_eq(gaol_core::cancel_minus(x, x));
  }
}

// A type of the alignment of an interval, 16 bytes with the SSE2 and NEON
// intervals, that holds no vector: see test_heap_alignment(). Its constructor
// writes one byte, which the compiler cannot widen into a store of 16 bytes
// (movaps) that would stop the program on a misaligned address before the test
// looks at it. As large as it is aligned, which Visual C++ warns it is not
// otherwise (C4324, an error under /WX)
namespace {
  struct alignas(alignof(gaol_core::interval)) as_aligned_as_an_interval {
    as_aligned_as_an_interval() { c[0] = 0; }
    volatile char c[alignof(gaol_core::interval)];
  };
}

#if GAOL_USING_SSE2_INSTRUCTIONS || GAOL_USING_NEON_INSTRUCTIONS
/*
  The functions of the program that have the names and the types of the
  kernels of the SSE2 and NEON intervals (add_bounds()...), and a result of
  another type, to tell which one a call finds when the program opens
  gaol_core. Only declared: the calls are in decltype (registers_of_the_program()
  gives the arguments, rather than std::declval, which makes the vector type a
  template argument: older GCC warns that its attributes are ignored)
*/
namespace kernels_of_the_program {
  struct result {};
#  if GAOL_USING_SSE2_INSTRUCTIONS
  typedef __m128d registers;
#  else
  typedef float64x2_t registers;
#  endif
}
kernels_of_the_program::registers registers_of_the_program();
kernels_of_the_program::result add_bounds(kernels_of_the_program::registers, kernels_of_the_program::registers);
kernels_of_the_program::result sub_bounds(kernels_of_the_program::registers, kernels_of_the_program::registers);
kernels_of_the_program::result mul_bounds(kernels_of_the_program::registers, kernels_of_the_program::registers);
kernels_of_the_program::result div_bounds(kernels_of_the_program::registers, kernels_of_the_program::registers);
kernels_of_the_program::result sqr_bounds(kernels_of_the_program::registers);
#endif

/*
  32-bit Windows with GCC or Clang (GAOL v5, point 106 (AI) of TODO.md): the
  system calls its threads and its callbacks with the stack aligned on 4 bytes,
  where GCC takes a function to be entered with it aligned on 16 and puts the
  intervals it keeps in locals at addresses that movaps needs aligned (bug
  40838 of GCC). test_stack_entered_aligned_on_4_bytes() enters a function that
  works on local intervals with the stack at each of the four alignments of 4
  bytes, and checks that it was entered so. Not elsewhere: the ABI of Linux and
  of macOS has the stack aligned on 16 at every call, and Visual C++ aligns the
  stack by itself. Clang for the targets of Windows does too (Clang 18 and 21
  put an andl of the stack in the prologue of a function that keeps a vector in
  a local, for i686-w64-mingw32 and i686-pc-windows-msvc, and not for
  i686-linux-gnu), and the intervals of Clang for Visual C++ and of clang-cl
  keep their two bounds as two doubles: the test runs on them all the same, and
  has to pass.
  A developer can run it on Linux for a 32-bit target with
  -DGAOL_TESTS_STACK_ON_4_BYTES=1 (GCC and Clang then stop the program for the
  three misaligned entries, as GCC does on Windows when it does not align the
  stack of the function, and the test says so)
*/
#if !defined(GAOL_TESTS_STACK_ON_4_BYTES)
#  if defined(__i386__) && defined(_WIN32) && (defined(__GNUC__) || defined(__clang__)) && GAOL_USING_SSE2_INSTRUCTIONS
#    define GAOL_TESTS_STACK_ON_4_BYTES 1
#  else
#    define GAOL_TESTS_STACK_ON_4_BYTES 0
#  endif
#endif

#if GAOL_TESTS_STACK_ON_4_BYTES
static_assert(sizeof(void*) == 4, "the test enters a function with the stack of a 32-bit x86 processor");
#  include <csetjmp>
#  include <csignal>
#  if defined(__unix__)
#    include <setjmp.h>
#    include <signal.h>
#  endif

namespace {
  // What the function entered with a misaligned stack works on and gives
  struct stack_work {
    double k;
    double left, right;
  };

  // Local intervals kept alive across calls of the library, so that the compiler spills them to the stack
  // with movaps if it takes the stack to be aligned
  __attribute__((noinline)) void work_on_local_intervals(void* argument)
  {
    stack_work* w = static_cast<stack_work*>(argument);
    const interval a(w->k, w->k + 1.0), b(w->k + 2.0, w->k + 3.0), c(w->k * 2.0, w->k * 2.0 + 0.5);
    interval r = a + b;
    r = r * c - a;
    r = sqr(r) + exp(a / (b + 1.0)) + sin(c) * cos(a);
    w->left = r.left() + a.left() + b.left() + c.left();
    w->right = r.right() + a.right() + b.right() + c.right();
  }

  struct stack_call {
    void (*f)(void*);
    void* argument;
    int pad;
    unsigned int seen;     // (esp + 4) % 16 at the entry of f, as the trampoline saw it
  };

  // f(argument) entered with (esp + 4) modulo 16 equal to 0 (the ABI of Linux), 4, 8 or 12 (the
  // alignments on 4 bytes that Windows can give): the stack is aligned on 16, then
  // moved down by pad bytes, and the argument and the return address take 8. The
  // trampoline writes the modulo it gave, esp before the call, into c.seen
  void call_with_stack_entry(stack_call& c)
  {
    __asm__ __volatile__(
      "movl 8(%%edi), %%edx\n\t"
      "movl 4(%%edi), %%ecx\n\t"
      "movl (%%edi), %%eax\n\t"
      "movl %%esp, %%esi\n\t"
      "andl $-16, %%esp\n\t"
      "subl %%edx, %%esp\n\t"
      "pushl %%ecx\n\t"
      "movl %%esp, %%edx\n\t"
      "andl $15, %%edx\n\t"
      "movl %%edx, 12(%%edi)\n\t"
      "call *%%eax\n\t"
      "movl %%esi, %%esp\n\t"
      : : "D" (&c) : "eax", "ecx", "edx", "esi", "memory", "cc",
          "xmm0", "xmm1", "xmm2", "xmm3", "xmm4", "xmm5", "xmm6", "xmm7");
  }

#  if defined(__unix__)
  sigjmp_buf stack_env;
  void on_stack_fault(int) { siglongjmp(stack_env, 1); }
#  else
  std::jmp_buf stack_env;
  void on_stack_fault(int) { std::signal(SIGSEGV, on_stack_fault); std::longjmp(stack_env, 1); }
#  endif

  // Whether f(argument) returned, entered with (esp + 4) % 16 equal to misalign, which seen says it was
  bool returns_with_stack_entry(stack_work& w, int misalign, bool& entered_so)
  {
    const int pad = 12 - misalign; // (esp + 4) % 16 == misalign after the push of the argument and the call
    stack_call c = { work_on_local_intervals, &w, pad, 99u };
    volatile bool returned = false;
#  if defined(__unix__)
    struct sigaction action = {}, old_segv, old_bus;
    sigemptyset(&action.sa_mask);
    action.sa_flags = 0;
    action.sa_handler = on_stack_fault;
    sigaction(SIGSEGV, &action, &old_segv);
    sigaction(SIGBUS, &action, &old_bus);
    if (sigsetjmp(stack_env, 1) == 0) {
      call_with_stack_entry(c);
      returned = true;
    }
    sigaction(SIGSEGV, &old_segv, nullptr);
    sigaction(SIGBUS, &old_bus, nullptr);
#  else
    void (*old)(int) = std::signal(SIGSEGV, on_stack_fault);
    if (setjmp(stack_env) == 0) {
      call_with_stack_entry(c);
      returned = true;
    }
    std::signal(SIGSEGV, old);
#  endif
    entered_so = c.seen == static_cast<unsigned int>(misalign);
    return returned;
  }
}
#endif // GAOL_TESTS_STACK_ON_4_BYTES

class misc_test {
public:
  void setUp() {
  }
  void tearDown() {
  }

  // --> Beginning of tests

  void test_namespace_detail_of_the_program() {
    TEST_TRUE(detail::of_the_program() == 1);
  }

  // The names of gaol, which the static_assert above check, give the
  // functions of gaol_core (GAOL v5)
  void test_names_of_gaol() {
    TEST_SEQ(gaol::exp2(interval(3.0)), interval(8.0));
    TEST_SEQ(gaol::trunc(interval(-1.5, 2.5)), interval(-1.0, 2.0));
    TEST_SEQ(gaol::sign(interval(-2.0, 3.0)), interval(-1.0, 1.0));
    TEST_SEQ(gaol::hypot(interval(3.0), interval(4.0)), interval(5.0));
    TEST_SEQ(gaol::pow(interval(-2.0), 3), interval(-8.0));
    TEST_SEQ(gaol::nth_root(interval(8.0), 3u), interval(2.0));
    TEST_SEQ(gaol::operator|(interval(1.0), interval(3.0)), interval(1.0, 3.0));
    TEST_SEQ(gaol::hull(interval(1.0), interval(3.0)), interval(1.0, 3.0));
    TEST_SEQ(gaol::intersect(interval(1.0, 3.0), interval(2.0, 4.0)), interval(2.0, 3.0));
    TEST_SEQ(gaol::textToInterval("[1, 2]"), interval(1.0, 2.0));
    TEST_TRUE(gaol::next_float(1.0) > 1.0 && gaol::previous_float(1.0) < 1.0);
    interval y;
    TEST_TRUE(gaol::evaluate_expr(gaol::exp2(gaol::expression(interval(3.0))), y) && y.set_eq(interval(8.0)));
    TEST_TRUE(gaol_names_adl::same_as_gaol_core(interval(0.5, 2.0)));
  }

  // The kernels that the operators of the SSE2 and NEON intervals call are in
  // gaol_detail, with the other helpers of the library, and not in gaol_core
  // (GAOL v5, point AP of TODO.md). Opened, gaol_core brings none of them: a
  // call finds the function of the program, which the kernels in gaol_core
  // made ambiguous, and the program did not compile. The FPU intervals have no
  // such kernels, and nothing to check
  void test_kernels_are_not_in_gaol_core() {
#if GAOL_USING_SSE2_INSTRUCTIONS || GAOL_USING_NEON_INSTRUCTIONS
    using namespace gaol_core;
    typedef kernels_of_the_program::result result;
    static_assert(std::is_same<decltype(add_bounds(registers_of_the_program(), registers_of_the_program())), result>::value,
                  "add_bounds() is found in gaol_core");
    static_assert(std::is_same<decltype(sub_bounds(registers_of_the_program(), registers_of_the_program())), result>::value,
                  "sub_bounds() is found in gaol_core");
    static_assert(std::is_same<decltype(mul_bounds(registers_of_the_program(), registers_of_the_program())), result>::value,
                  "mul_bounds() is found in gaol_core");
    static_assert(std::is_same<decltype(div_bounds(registers_of_the_program(), registers_of_the_program())), result>::value,
                  "div_bounds() is found in gaol_core");
    static_assert(std::is_same<decltype(sqr_bounds(registers_of_the_program())), result>::value,
                  "sqr_bounds() is found in gaol_core");

    // The kernels of gaol_detail, on the bounds in the registers (the left one
    // stored negated), give the bounds of the sum, the difference, the
    // product, the quotient and the square, which are doubles here
    const interval x(-3.0, 2.0), y(0.5, 4.0);
#  if GAOL_USING_SSE2_INSTRUCTIONS
#    define KERNEL_ARGUMENT(I) (I).get_xmminterval()
#  else
#    define KERNEL_ARGUMENT(I) (I).get_neoninterval()
#  endif
    TEST_SEQ(interval(gaol_detail::add_bounds(KERNEL_ARGUMENT(x), KERNEL_ARGUMENT(y))), interval(-2.5, 6.0));
    TEST_SEQ(interval(gaol_detail::sub_bounds(KERNEL_ARGUMENT(x), KERNEL_ARGUMENT(y))), interval(-7.0, 1.5));
    TEST_SEQ(interval(gaol_detail::mul_bounds(KERNEL_ARGUMENT(x), KERNEL_ARGUMENT(y))), interval(-12.0, 8.0));
    TEST_SEQ(interval(gaol_detail::div_bounds(KERNEL_ARGUMENT(x), KERNEL_ARGUMENT(y))), interval(-6.0, 4.0));
    TEST_SEQ(interval(gaol_detail::sqr_bounds(KERNEL_ARGUMENT(x))), interval(0.0, 9.0));
#  undef KERNEL_ARGUMENT
#endif
  }

  // The memory the heap gives holds the intervals aligned on alignof(interval)
  // bytes, 16 with the SSE2 and NEON intervals, whose vector instructions
  // stop on a misaligned address (GAOL v5). On 32-bit Windows the C runtime
  // aligns the memory of new on 8 bytes: only a compiler told so asks for
  // aligned memory for the types that need it (-faligned-new=8; see
  // gaol/gaol_config.h), and GCC taking it to be aligned on 16 made a
  // std::vector of SSE2 intervals stop the program. With Visual C++ and
  // clang-cl the alignment of an interval is 8, and the test below checks that
  // the intervals work at such an address. A type as aligned as an
  // interval, with no vector instruction to stop on a misaligned address, stands
  // for the interval, so that the test fails where the program used to stop.
  void test_heap_alignment() {
    typedef as_aligned_as_an_interval probe;
    const std::uintptr_t alignment = alignof(interval);
    bool aligned = true;
    const auto check = [&](const void* address) {
      aligned = aligned && reinterpret_cast<std::uintptr_t>(address) % alignment == 0;
    };

    std::allocator<probe> allocator;
    probe* storage = allocator.allocate(3);
    check(storage);
    allocator.deallocate(storage, 3);

    std::vector<probe> vector(3);
    for (std::size_t i = 0; i < vector.size(); ++i) { check(&vector[i]); }
    std::list<probe> list(3);
    for (std::list<probe>::const_iterator i = list.begin(); i != list.end(); ++i) { check(&*i); }
    std::deque<probe> deque(3);
    for (std::size_t i = 0; i < deque.size(); ++i) { check(&deque[i]); }
    std::map<int, probe> map;
    map[0]; map[1];
    for (std::map<int, probe>::const_iterator i = map.begin(); i != map.end(); ++i) { check(&i->second); }
    const std::shared_ptr<probe> shared = std::make_shared<probe>();
    check(shared.get());

    probe* one = new probe;
    check(one);
    delete one;
    probe* several = new probe[3];
    for (int i = 0; i < 3; ++i) { check(several + i); }
    delete[] several;

    TEST_TRUE(aligned);
    if (!aligned) {
      return; // the intervals below would stop the program
    }
    // The intervals themselves, in the same containers and with new, which
    // GAOL's own operator new aligns whatever the compiler assumes
    std::vector<interval> intervals(3, interval(1.0, 2.0));
    interval sum(0.0);
    for (std::size_t i = 0; i < intervals.size(); ++i) {
      sum += intervals[i];
    }
    TEST_SEQ(sum, interval(3.0, 6.0));
    interval* heap = new interval(1.0, 2.0);
    TEST_TRUE(reinterpret_cast<std::uintptr_t>(heap) % alignment == 0);
    TEST_SEQ(*heap + *heap, interval(2.0, 4.0));
    delete heap;
  }

  // The SSE2 intervals that keep their bounds as two doubles (Visual C++ and
  // clang-cl on 32-bit Windows, where the memory of new is aligned on 8 bytes
  // and the compiler cannot be told otherwise; any SSE2 build of CMake with
  // -DGAOL_SSE2_UNALIGNED_STORAGE=ON; GAOL v5; see gaol/gaol_config.h) work at an
  // address aligned on 8 bytes only, one in two of the addresses of the heap
  // there, which a member of a __m128d would stop on (movaps). Three intervals
  // are put at addresses of 8 modulo 16 and a representative set of operations
  // reads and writes them; with a load or a store aligned in gaol_detail::unaligned_xmm,
  // the program stops here
  void test_intervals_aligned_on_8_bytes() {
#if GAOL_USING_SSE2_INSTRUCTIONS && GAOL_SSE2_UNALIGNED_STORAGE
    static_assert(alignof(interval) == alignof(double), "the bounds of the interval are two doubles");
    alignas(16) unsigned char buffer[3*sizeof(interval) + 16];
    unsigned char* odd = buffer + 8;
    TEST_TRUE(reinterpret_cast<std::uintptr_t>(odd) % 16 == 8);
    interval* x = new (odd) interval(1.0, 2.0);
    interval* y = new (odd + sizeof(interval)) interval(3.0, 4.0);
    interval* z = new (odd + 2*sizeof(interval)) interval(*x);
    TEST_TRUE(reinterpret_cast<std::uintptr_t>(y) % 16 == 8 && reinterpret_cast<std::uintptr_t>(z) % 16 == 8);
    TEST_SEQ(*z, interval(1.0, 2.0));
    *z += *y;
    TEST_SEQ(*z, interval(4.0, 6.0));
    *z = *x * *y;
    TEST_SEQ(*z, interval(3.0, 8.0));
    *z = *y / *x;
    TEST_SEQ(*z, interval(1.5, 4.0));
    *z = sqr(*x);
    TEST_SEQ(*z, interval(1.0, 4.0));
    *z = -*x;
    TEST_SEQ(*z, interval(-2.0, -1.0));
    *z = *x;
    *z &= *y;
    TEST_TRUE(z->is_empty());
    *z = *x | *y;
    TEST_SEQ(*z, interval(1.0, 4.0));
    *z = sqrt(*y);
    TEST_TRUE(z->set_contains(interval(1.7320508075688772, 2.0)));
    *z = *x;
    *z *= 2.5;
    TEST_TRUE(z->set_contains(interval(2.5, 5.0)));
    TEST_TRUE(x->midpoint() == 1.5);
    x->~interval(); y->~interval(); z->~interval();
#else
    std::printf("The bounds of the SSE2 intervals are a __m128d here (or the intervals are not SSE2 ones): nothing to check at an address of 8 modulo 16\n");
#endif
  }

  // A function that works on local intervals, entered with the stack aligned on 4 bytes, as
  // 32-bit Windows enters the functions it calls (threads, callbacks): it has to return
  // the bounds it gives when entered with the stack aligned on 16 (see GAOL_TESTS_STACK_ON_4_BYTES above)
  void test_stack_entered_aligned_on_4_bytes() {
#if GAOL_TESTS_STACK_ON_4_BYTES
    // The function that entered with the stack aligned as the ABI of Linux has it works: its bounds
    // are far from 0, and are those the other entries have to give
    stack_work reference = { 1.5, 0.0, 0.0 };
    bool entered_so = false;
    TEST_TRUE(returns_with_stack_entry(reference, 0, entered_so));
    TEST_TRUE(entered_so);
    TEST_TRUE(reference.left > 8.0 && reference.left <= reference.right);
    std::string failed;
    for (int misalign = 4; misalign < 16; misalign += 4) {
      stack_work w = { 1.5, 0.0, 0.0 };
      const bool returned = returns_with_stack_entry(w, misalign, entered_so);
      TEST_TRUE(entered_so);
      if (!returned) {
        failed += (failed.empty() ? "" : ", ") + std::to_string(misalign);
      } else {
        TEST_TRUE(w.left == reference.left && w.right == reference.right);
      }
    }
    if (!failed.empty()) {
      std::printf("The function stopped the program with the stack aligned on 16 minus %s bytes at its entry\n", failed.c_str());
    }
    TEST_TRUE(failed.empty());
    if (failed.empty()) {
      std::printf("The function returned the bounds of the entry aligned on 16 with the stack aligned on 16 minus 4, 8 and 12 bytes at its entry\n");
    }
#else
    std::printf("The functions of the system are entered with the stack aligned on 16 here (or not with GCC or Clang for 32-bit Windows): nothing to check\n");
#endif
  }

  // The builds that write MXCSR alone in GAOL_RND_ENTER() and GAOL_RND_LEAVE() with
  // GAOL_PRESERVE_ROUNDING (GAOL v5, point 109 (AI) of TODO.md; see gaol/gaol_fpu.h): x86-64
  // with the SSE2 intervals and the fused multiply-add, which is __FMA__ for GCC, Clang and
  // clang-cl and /arch:AVX2 (__AVX2__) for Visual C++. Written again here so that a build
  // that loses them, and takes the macros that call fegetround() and fesetround() and write the
  // control word of the x87 unit as well (24 ns per operation with Visual C++ x64), does not go unseen
  void test_rounding_macros_write_mxcsr_alone() {
    // GAOL_FMA_AS_INSTRUCTION, which the macros below and the packed products of the integer
    // powers share, is the condition of the compiler written again
#if defined(__FMA__) || (defined(_MSC_VER) && !defined(__clang__) && defined(__AVX2__))
    TEST_TRUE(GAOL_FMA_AS_INSTRUCTION == 1);
#else
    TEST_TRUE(GAOL_FMA_AS_INSTRUCTION == 0);
#endif
#if GAOL_PRESERVE_ROUNDING && GAOL_USING_SSE2_INSTRUCTIONS && (defined(__x86_64__) || defined(_M_X64))
#  if defined(__FMA__) || (defined(_MSC_VER) && !defined(__clang__) && defined(__AVX2__))
    TEST_TRUE(GAOL_RND_MXCSR_ONLY == 1);
#  else
    TEST_TRUE(GAOL_RND_MXCSR_ONLY == 0);
#  endif
#elif GAOL_PRESERVE_ROUNDING
    TEST_TRUE(GAOL_RND_MXCSR_ONLY == 0);
#else
    std::printf("The rounding direction is not preserved (or the processor is not an x86-64): nothing to check\n");
    TEST_TRUE(GAOL_RND_MXCSR_ONLY == 0);
#endif
#if GAOL_PRESERVE_ROUNDING
    // What the build has, as the compiler gave it: the jobs of the continuous integration for Visual C++ and
    // clang-cl on x64 look for the "is 1" (ctest -V), which the copy of the condition above would not see go
    std::printf("GAOL_RND_MXCSR_ONLY is %d: GAOL_RND_ENTER() and GAOL_RND_LEAVE() %s\n", GAOL_RND_MXCSR_ONLY,
                GAOL_RND_MXCSR_ONLY ? "write MXCSR alone" : "are those of the other builds");
#endif
  }

  // Tests of logical predicates ===================================================================
  void test_predicates() {
    // is_empty()
    CPPUNIT_ASSERT(interval::emptyset().is_empty());
    CPPUNIT_ASSERT(interval(1.0,-1.0).is_empty());

    // is_symmetric()
    CPPUNIT_ASSERT(!interval::emptyset().is_symmetric());
    CPPUNIT_ASSERT(interval::universe().is_symmetric());
    CPPUNIT_ASSERT(interval(-6.0,6.0).is_symmetric());
    CPPUNIT_ASSERT(!interval(-6.0,-5.0).is_symmetric());
    CPPUNIT_ASSERT(!interval(5.0,6.0).is_symmetric());

  }

  // Tests of constants
  void test_constants() {
    CPPUNIT_ASSERT(interval::emptyset().is_empty());
    TEST_PEQ(interval::universe(),interval(-GAOL_INFINITY,+GAOL_INFINITY));
    TEST_SEQ(interval::zero(),interval(0.0,0.0));
    TEST_SEQ(interval::one(),interval(1.0,1.0));
    TEST_SEQ(interval::positive(),interval(0.0,+GAOL_INFINITY));
    TEST_SEQ(interval::negative(),interval(-GAOL_INFINITY,0.0));
    TEST_SEQ(interval::minus_one_plus_one(),interval(-1.0,1.0));
    TEST_PEQ(interval::pi(),interval(3.14159,3.1416));
    TEST_PEQ(interval::two_pi(),interval(6.2831,6.2832));
    TEST_PEQ(interval::half_pi(),interval(1.5707,1.5708));
    TEST_SEQ(interval::one_plus_infinity(),interval(1.0,+GAOL_INFINITY));
  }


  // <-- End of tests
};

GAOL_UNIT_MAIN(misc_test, "misc",
               GAOL_UNIT_TEST(test_constants),
               GAOL_UNIT_TEST(test_predicates),
               GAOL_UNIT_TEST(test_namespace_detail_of_the_program),
               GAOL_UNIT_TEST(test_names_of_gaol),
               GAOL_UNIT_TEST(test_kernels_are_not_in_gaol_core),
               GAOL_UNIT_TEST(test_heap_alignment),
               GAOL_UNIT_TEST(test_intervals_aligned_on_8_bytes),
               GAOL_UNIT_TEST(test_stack_entered_aligned_on_4_bytes),
               GAOL_UNIT_TEST(test_rounding_macros_write_mxcsr_alone))
