// GAOL 4's unit test check/misc.cpp, run with the checks of unit_tests.h
// rather than CppUnit (GAOL v5)
// The expressions, with their nodes, visitor and evaluator, for the names of
// the namespace gaol below
#include "gaol/gaol_expr_eval.h"
#include "unit_tests.h"

#include <istream>
#include <ostream>
#include <string>
#include <type_traits>
#include <utility>

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
  GAOL 4 (rnd_keep() of GAOL 4.3.2 among them), the functions on intervals
  GAOL v5 adds, and restore_rounding() and exact_string(), each with the
  overloads of the expressions where there are some. A name missing does not
  compile.
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
  GAOL_NAMES_CLASS(gaol_exception);
  GAOL_NAMES_CLASS(input_format_error);
  GAOL_NAMES_CLASS(invalid_action_error);
  GAOL_NAMES_CLASS(unavailable_feature_error);
  GAOL_NAMES_CLASS(timepiece);
  GAOL_NAMES_CLASS(expression);
  GAOL_NAMES_CLASS(expr_node);
  GAOL_NAMES_CLASS(expr_visitor);
  GAOL_NAMES_CLASS(expr_eval);
  GAOL_NAMES_CLASS(eval_stack<interval>);
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

  static_assert(same<decltype(gaol::version), const char *const>() && gaol::version_major == GAOL_MAJOR_VERSION,
                "gaol::version, gaol::version_major...");

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
#undef GAOL_NAMES_BINARY
  static_assert(same<decltype(gaol::div_rel(x(), x(), x())), interval>(), "gaol::div_rel(z, y, x)");
  static_assert(same<decltype(gaol::fma(x(), x(), x())), interval>(), "gaol::fma(x, y, z)");
  static_assert(same<decltype(gaol::nth_root(x(), 3u)), interval>() && same<decltype(gaol::nth_root(x(), 3)), interval>(),
                "gaol::nth_root(x, n)");
  static_assert(same<decltype(gaol::nth_root_rel(x(), 3u, x())), interval>(), "gaol::nth_root_rel(y, n, x)");
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

  // The overloads of the expressions, which gaol/gaol_expression.h names
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
  static_assert(same<decltype(gaol::operator<<(out(), e())), std::ostream&>()
                && same<decltype(gaol::operator<<(out(), std::declval<const gaol::gaol_exception&>())), std::ostream&>(),
                "gaol::operator<< of the expressions and of the exceptions");
  static_assert(same<decltype(gaol::evaluate_expr(e(), std::declval<interval&>())), bool>(), "gaol::evaluate_expr");

  // The functions of doubles, the initialization and the rounding of GAOL 4,
  // rnd_keep() of GAOL 4.3.2, and restore_rounding() of GAOL v5
  static_assert(same<decltype(gaol::next_float(1.0)), double>() && same<decltype(gaol::previous_float(1.0)), double>()
                && same<decltype(gaol::maximum(1.0, 2.0)), double>() && same<decltype(gaol::minimum(1.0, 2.0)), double>(),
                "gaol::next_float, gaol::previous_float, gaol::maximum, gaol::minimum");
  static_assert(same<decltype(gaol::init()), bool>() && same<decltype(gaol::cleanup()), bool>()
                && same<decltype(gaol::restore_rounding()), void>() && same<decltype(gaol::round_upward()), void>()
                && same<decltype(gaol::round_downward()), void>() && same<decltype(gaol::round_nearest()), void>()
                && same<decltype(gaol::rnd_keep(1.0)), double>(),
                "gaol::init, gaol::round_upward, gaol::restore_rounding, gaol::rnd_keep...");
  // NaN_val of GAOL 4, which the macro GAOL_NAN names
  static_assert(same<decltype(gaol::NaN_val), const gaol_core::uintdouble>(), "gaol::NaN_val");
}

/*
  What gaol no longer names: what GAOL's code uses for itself, which stays in
  gaol_core (GAOL v5). gaol took gaol_core whole before, and these names were
  found through it. Each is looked for in the namespaces that using namespace
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
  absent gaol_pow_real(...);
  absent gaol_pow_hybrid(...);
  absent gaol_pown_exp(...);
  absent gaol_pow_exp(...);
  absent gaol_sign_of(...);
  absent rnd_reread(...);
  // The internals GAOL 4 declared in gaol too, which its manual did not document
  absent f_negate(...);
  absent f_negate_simple(...);
  absent gaol_signbit(...);
  absent modulo_k_pi(...);
  absent reset_fpu_cw(...);
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
  // For the check that the fallbacks lose to the functions of gaol
  absent next_float(...);
  absent rnd_keep(...);
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
  GAOL_NAMES_ABSENT((gaol_pow_real)(x(), 0.5));
  GAOL_NAMES_ABSENT((gaol_pow_hybrid)(x(), x()));
  GAOL_NAMES_ABSENT((gaol_pown_exp)(e(), 3));
  GAOL_NAMES_ABSENT((gaol_pow_exp)(e(), e()));
  GAOL_NAMES_ABSENT((gaol_sign_of)(1.0));
  GAOL_NAMES_ABSENT((rnd_reread)(1.0));
  GAOL_NAMES_ABSENT((f_negate)(1.0));
  GAOL_NAMES_ABSENT((f_negate_simple)(1.0));
  GAOL_NAMES_ABSENT((gaol_signbit)(1.0));
  GAOL_NAMES_ABSENT((modulo_k_pi)(x(), std::declval<double&>(), std::declval<double&>()));
  GAOL_NAMES_ABSENT((reset_fpu_cw)(static_cast<unsigned short>(0)));
  GAOL_NAMES_ABSENT((round_downward_sse)());
  GAOL_NAMES_ABSENT((round_to_nearest_sse)());
  GAOL_NAMES_ABSENT((round_upward_sse)());
#undef GAOL_NAMES_ABSENT
  static_assert(same<uintdouble, absent>() && same<ullidouble, absent>() && same<Interval_struct, absent>()
                && same<prec_t, absent>() && same<decltype(the_null_expr), const absent>(),
                "uintdouble, ullidouble, Interval_struct, prec_t, the_null_expr: not in gaol");
  static_assert(same<rounding_state, absent>() && same<rounding_guard, absent>() && same<gaol_initializer, absent>(),
                "rounding_state, rounding_guard, gaol_initializer: not in gaol");
  static_assert(same<decltype(_gaol_initializer), const absent>(), "_gaol_initializer: not in gaol");
  // The detection works: the functions of gaol beat the fallbacks
  static_assert(same<decltype((next_float)(1.0)), double>() && same<decltype((rnd_keep)(1.0)), double>()
                && same<decltype((round_upward)()), void>(),
                "(next_float)(1.0), (rnd_keep)(1.0) and (round_upward)() are found in gaol");
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
    TEST_SEQ(gaol::textToInterval("[1, 2]"), interval(1.0, 2.0));
    TEST_TRUE(gaol::next_float(1.0) > 1.0 && gaol::previous_float(1.0) < 1.0);
    interval y;
    TEST_TRUE(gaol::evaluate_expr(gaol::exp2(gaol::expression(interval(3.0))), y) && y.set_eq(interval(8.0)));
    TEST_TRUE(gaol_names_adl::same_as_gaol_core(interval(0.5, 2.0)));
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
               GAOL_UNIT_TEST(test_names_of_gaol))
