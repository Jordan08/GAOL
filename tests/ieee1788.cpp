/*-*-C++-*----------------------------------------------------------------------
 * gaol -- NOT Just Another Interval Library
 *------------------------------------------------------------------------------
 * Tests of GAOL v5: gaol_ieee1788 as a program uses it, with
 * using namespace gaol_ieee1788 at file scope and without using namespace
 * gaol.
 *
 * A program opens one of the two namespaces. The names of the standard are
 * called unqualified, which compiles only if none of them is ambiguous with a
 * function that argument-dependent lookup finds on an interval, in gaol_core:
 * sin, min and the others brought in by using-declarations are gaol_core's
 * own. pow, which is in gaol_ieee1788 and in gaol but not in gaol_core, takes
 * an interval, an int or a double as exponent, and has to be the pow of
 * Table 9.1 for each, on intervals and on expressions, where GAOL's pow takes
 * pown for an integer exponent: pow(x, 2) is pow(x, [2]), the integer power
 * being pown(x, 2). Their bounds on boxes that reach each branch of the
 * function they share are checked, the bounds being the tightest ones that
 * mpmath gives but for one integer power (see pow_on_boxes()). The functions
 * of C on numbers have to remain those of C. intervalToExact() has to be
 * exact_string(), and to leave the global output format alone. The check of
 * it by a second thread writing intervals meanwhile is commented out: the
 * tests run no thread.
 * mulRevToPair() has to give the two parts of 10.5.5 in the standard's order.
 * overlap() has to give the state that the set specification of Table 10.7
 * gives, on a grid of intervals and on random ones (see
 * overlap_of_the_standard()).
 * The operators of intervals (+, -, *, /, %, & and |) have to be in
 * gaol_ieee1788 too, as the functions of gaol_core are: called by their
 * qualified names, which argument-dependent lookup does not help, they compile
 * only if the namespace holds them (see gaol_functions()).
 *
 * Copyright (c) 2026 ENSTA, France
 *
 * Created 2026-09-21 by Jordan NININ
 *------------------------------------------------------------------------------
 * gaol is a software distributed WITHOUT ANY WARRANTY. Read the associated
 * COPYING file for information.
 *----------------------------------------------------------------------------*/

#include "gaol_tests.h"
#include "gaol/parser/gaol_expr_eval.h"

#include <algorithm>
#include <cctype>
#include <clocale>
#include <cstdlib>
#include <iomanip>
#include <locale>
#include <map>
#include <sstream>
#include <stdexcept>
#include <string>
#include <type_traits>
#include <utility>
#include <vector>

// Commented out: the tests run no thread (GAOL v5)
// // std::thread, which libstdc++ has only when built with a thread model
// #if !defined(__GLIBCXX__) || defined(_GLIBCXX_HAS_GTHREADS)
// #  include <atomic>
// #  include <thread>
// #  define GAOL_TESTS_THREADS 1
// #endif

using namespace gaol_ieee1788;
using gaol_tests::check;
using gaol_tests::hex;

namespace
{
  const double oo = std::numeric_limits<double>::infinity();

  // The functions of C on numbers remain those of C: the templates of
  // gaol_ieee1788 take part only when an argument is an interval
  static_assert(std::is_same<decltype(sqrt(2.0)), double>::value, "sqrt(double) is C's");
  static_assert(std::is_same<decltype(floor(2.5)), double>::value, "floor(double) is C's");
  static_assert(std::is_same<decltype(atan2(1.0, 2.0)), double>::value, "atan2(double, double) is C's");
  static_assert(std::is_same<decltype(abs(-3)), int>::value, "abs(int) is C's");

  /* The functions of gaol_ieee1788 take the overloads of the expressions,
     gaol/parser/gaol_expression.h being included here after gaol/gaol_ieee1788.h,
     and before it in tests/expressions.cpp (GAOL v5) */
  static_assert(std::is_same<decltype(gaol_ieee1788::sin(std::declval<const gaol::expression&>())), const gaol::expression>::value,
                "gaol_ieee1788::sin of an expression, gaol/parser/gaol_expression.h included after gaol/gaol_ieee1788.h");

  /* A number converts neither to an interval nor to an expression (GAOL
     v5): min(y, 1.0) or atan2(y, 1.0), which took [1] while it converted,
     do not compile, and the interval is written interval(1.0) */
  template <typename A, typename B, typename = void> struct has_min : std::false_type {};
  template <typename A, typename B>
  struct has_min<A, B, decltype(void(min(std::declval<const A&>(), std::declval<const B&>())))> : std::true_type {};
  template <typename A, typename B, typename = void> struct has_atan2 : std::false_type {};
  template <typename A, typename B>
  struct has_atan2<A, B, decltype(void(atan2(std::declval<const A&>(), std::declval<const B&>())))> : std::true_type {};
  static_assert(!has_min<interval, double>::value && !has_min<double, interval>::value, "min(y, 1.0) does not compile");
  static_assert(!has_atan2<interval, double>::value && !has_atan2<double, interval>::value, "atan2(y, 1.0) does not compile");
  static_assert(has_min<interval, interval>::value && has_atan2<interval, interval>::value, "the detection works");

  // The value of an expression, which the evaluator of GAOL computes
  interval value_of(const gaol::expression& e)
  {
    gaol::expr_eval eval;
    e.get_root()->accept(eval);
    return eval.result();
  }

  /*
    Integers (GAOL v5, point D.21): numsToInterval() and isMember() take an
    integer as the integer it is, 2^53 + 1 being in no double; pown() and
    gaol::pow() take an exponent of any integer type: pown(x, 3000000000u)
    converted it to a negative int, and gaol::pow(x, 5L) was ambiguous.
  */
  void integers()
  {
    const double two53 = std::ldexp(1.0, 53), dmax = std::numeric_limits<double>::max();
    const volatile long long n = 9007199254740993LL; // 2^53 + 1
    const interval around(two53, two53 + 2), at(two53);
    const interval nums = numsToInterval(1, n);
    check("numsToInterval(1, 2^53 + 1) = [1, 2^53 + 2]", nums.left() == 1.0 && nums.right() == two53 + 2,
          [&] { return hex(nums); });
    check("numsToInterval(2^53 + 1, 1.0) and numsToInterval(2^53 + 1, 2^53) are empty",
          numsToInterval(n, 1.0).is_empty() && numsToInterval(n, n - 1).is_empty(),
          [&] { return hex(numsToInterval(n, n - 1)); });
    const interval negative = numsToInterval(-n, 1.0);
    check("numsToInterval(-(2^53 + 1), 1.0) = [-(2^53 + 2), 1]",
          negative.left() == -(two53 + 2) && negative.right() == 1.0, [&] { return hex(negative); });
    check("isMember(2^53 + 1, x)", isMember(n, around) && !isMember(n, at) && isMember(3, numsToInterval(1.0, 4.0)));
    const interval two(2.0), x = numsToInterval(-4.0, -1.0), y = numsToInterval(0.25, 0.5);
    const auto bounds_are = [](const interval& z, double l, double r) {
      return !z.is_empty() && z.left() == l && z.right() == r;
    };
    const interval p = pown(two, 3000000000u);
    check("pown([2], 3000000000u) = [DBL_MAX, +oo]", p.left() == dmax && p.right() == oo, [&] { return hex(p); });
    check("pown(x, 3L) and gaol::pow(x, 3L) = [-64, -1]",
          pown(x, 3L).set_eq(numsToInterval(-64.0, -1.0)) && gaol::pow(x, 3L).set_eq(numsToInterval(-64.0, -1.0))
          && gaol::pow(x, static_cast<short>(3)).set_eq(numsToInterval(-64.0, -1.0)),
          [&] { return hex(pown(x, 3L)); });
    check("pown(x, -2L) and gaol::pow(x, -3LL): the negative powers, as for an int",
          pown(x, -2L).set_eq(pown(x, -2)) && gaol::pow(x, -3LL).set_eq(gaol::pow(x, -3))
          && pown(x, -2L).set_eq(numsToInterval(0.0625, 1.0)),
          [&] { return hex(pown(x, -2L)); });
    check("gaol::pow([2], 3000000000ul) = gaol::pow([2], 3000000000u)",
          gaol::pow(two, 3000000000ul).set_eq(gaol::pow(two, 3000000000u)), [&] { return hex(gaol::pow(two, 3000000000ul)); });
    // Beyond the unsigned ints, pown from CORE-MATH's pow at the corners
    // (GAOL v5, point 74 of TODO.md: [-oo, +oo]); beyond the long longs, as
    // for +-(2^63 - 1) or +-(2^63 - 2), of the parity of n, all the powers of
    // these bases being beyond the doubles there
    const interval below_one = numsToInterval(-0.5, -0.25), near_one(-0x1.0000000000001p+0);
    check("pown(x, 5e9) and gaol::pow(x, 5e9), beyond the unsigned ints: pown, empty for an empty x",
          bounds_are(pown(x, 5000000000LL), 1.0, oo) && bounds_are(gaol::pow(x, -5000000000LL), 0.0, 1.0)
          && bounds_are(pown(x, 5000000001LL), -oo, -1.0) && bounds_are(pown(below_one, 5000000001LL), -0x1p-1074, -0.0)
          && gaol::pow(interval::emptyset(), 5000000000LL).is_empty()
          && pown(numsToInterval(0.0, 0.0), -5000000000LL).is_empty(),
          [&] { return hex(gaol::pow(x, 5000000000LL)) + " " + hex(gaol::pow(x, -5000000000LL)); });
    // An n of more than 53 bits that is no double: x^h x^l, n = h + l, h a
    // double and |l| < 2^11, within three doubles of the tightest bounds
    // (mpmath, 600 bits), which the bounds have to enclose
    const auto within_three = [](const interval& z, double l, double r) {
      return !z.is_empty() && z.left() <= l && z.right() >= r && z.left() >= std::nextafter(std::nextafter(
             std::nextafter(l, -oo), -oo), -oo) && z.right() <= std::nextafter(std::nextafter(std::nextafter(r, oo), oo), oo);
    };
    const interval just_above_one(0x1.0000000000001p+0), just_below_one(0x1.fffffffffffffp-1);
    check("pown(x, n) beyond 2^53, n no double: within three doubles of the tightest bounds",
          within_three(pown(just_above_one, 9007199254740993LL), 0x1.d8e64b8d4ddadp+2, 0x1.d8e64b8d4ddaep+2)
          && within_three(pown(just_above_one, -9007199254740995LL), 0x1.152aaa3bf81c9p-3, 0x1.152aaa3bf81cap-3)
          && within_three(pown(just_below_one, 4611686018427387905LL), 0x1.44109edb2088ep-739, 0x1.44109edb2088fp-739)
          && within_three(pown(-just_above_one, 9007199254740993LL), -0x1.d8e64b8d4ddaep+2, -0x1.d8e64b8d4ddadp+2),
          [&] { return hex(pown(just_above_one, 9007199254740993LL)) + " " + hex(pown(just_below_one, 4611686018427387905LL)); });
    // An n beyond 2^53 that is a double, an even one: CORE-MATH's pow with n,
    // the tightest bounds, as gaol::pow(x, [n]) (GAOL v5: the 11 lowest bits
    // of n were split off beyond 2^53, giving x^(2^53 + 2) within three
    // doubles only)
    const interval tight_ll = pown(just_above_one, 9007199254740994LL);
    check("pown(x, n) beyond 2^53, n a double: the tightest bounds, those of gaol::pow(x, [n])",
          bounds_are(tight_ll, 0x1.d8e64b8d4ddafp+2, 0x1.d8e64b8d4ddb0p+2)
          && bounds_are(pown(just_above_one, -9007199254740994LL), 0x1.152aaa3bf81cap-3, 0x1.152aaa3bf81cbp-3)
          && bounds_are(pown(just_above_one, 1152921504606847232LL), 0x1.41c7a8814bf5ap+369, 0x1.41c7a8814bf5bp+369)
          && bounds_are(pown(-just_above_one, -1152921504606847232LL), 0x1.9755956ad4dd0p-370, 0x1.9755956ad4dd1p-370)
          && bounds_are(pown(just_below_one, 4611686018427388928LL), 0x1.44109edb20607p-739, 0x1.44109edb20608p-739)
          && tight_ll.set_eq(gaol::pow(just_above_one, interval(9007199254740994.0))),
          [&] { return hex(tight_ll) + " " + hex(pown(just_above_one, 1152921504606847232LL)); });
    check("pown(x, n) beyond the long longs: x^(2^64 - 1) as x^(2^63 - 1), x^(2^64 - 2) as x^(2^63 - 2)",
          bounds_are(pown(near_one, 18446744073709551615ULL), -oo, -dmax)
          && bounds_are(pown(near_one, 18446744073709551614ULL), dmax, oo)
          && bounds_are(pown(numsToInterval(-1.0, -1.0), 18446744073709551615ULL), -1.0, -1.0)
          && bounds_are(pown(below_one, 18446744073709551614ULL), 0.0, 0x1p-1074),
          [&] { return hex(pown(near_one, 18446744073709551615ULL)); });
    check("pow(y, 2LL) is pow(y, [2])", pow(y, 2LL).set_eq(pow(y, interval(2.0))) && pow(x, 2LL).is_empty(),
          [&] { return hex(pow(y, 2LL)); });

    // rootn(), gaol::nth_root() and pownRev() for an integer of another type
    // (GAOL v5): nth_root(x, 3L) was ambiguous, rootn(x, 3000000000u) and
    // pownRev(c, 3000000000u) converted the order to a negative int (rootn
    // gave [0.99999999839, 0.99999999946] for [2, 8], and pownRev threw), and
    // rootn(x, 5000000001LL) cut it to an int. Beyond the unsigned ints, the
    // roots are enclosed through pow, here the tightest ones: the doubles
    // around 8^(1/q) and 2^(1/q), for q = 5 10^9 + 1 and 5 10^9 alike, and
    // around 8^(-1/q) and 2^(-1/q) for q = 5 10^9 (mpmath, 400 bits)
    const interval c8(2.0, 8.0), m8(-8.0, 8.0);
    const double r8 = 0x1.00000001c9463p+0, r2 = 0x1.00000000986cbp+0;
    check("rootn(x, 3L), rootn(x, -3LL), gaol::nth_root(x, 3L) and gaol::nth_root(x, 3000000000ul) as for an int",
          rootn(m8, 3L).set_eq(rootn(m8, 3)) && rootn(c8, -3LL).set_eq(rootn(c8, -3))
          && gaol::nth_root(m8, 3L).set_eq(gaol::nth_root(m8, 3))
          && gaol::nth_root(c8, 3000000000ul).set_eq(gaol::nth_root(c8, 3000000000u)),
          [&] { return hex(rootn(m8, 3L)); });
    const interval rootn_u = rootn(c8, 3000000000u), rootn_ll = rootn(c8, 5000000001LL);
    check("rootn(x, 3000000000u) and rootn(x, 5000000001LL): the roots of that order",
          rootn_u.set_eq(gaol::nth_root(c8, 3000000000u)) && bounds_are(rootn_ll, r2, r8),
          [&] { return hex(rootn_u) + " " + hex(rootn_ll); });
    const interval odd = gaol::nth_root(m8, 5000000001LL), even = gaol::nth_root(m8, 5000000000LL);
    const interval inverse = gaol::nth_root(c8, -5000000000LL);
    check("nth_root(x, q) beyond the unsigned ints: the tightest enclosures of the roots",
          bounds_are(odd, -r8, r8) && bounds_are(even, 0.0, r8)
          && bounds_are(inverse, 0x1.fffffffc6d73bp-1, 0x1.fffffffecf26ap-1),
          [&] { return hex(odd) + " " + hex(even) + " " + hex(inverse); });
    // The same orders as unsigned long long, which nth_root(I, unsigned long
    // long) takes, and through the template, with the type of the integer
    // that holds them, a long on 64-bit Linux and macOS, a long long elsewhere
    // (GAOL v5)
    const interval odd_u = gaol::nth_root(m8, 5000000001ULL), even_u = gaol::nth_root(m8, 5000000000ULL);
    const interval odd_t = gaol::nth_root(m8, 5000000001L), inverse_t = gaol::nth_root(c8, -5000000000L);
    check("nth_root(x, q) beyond the unsigned ints for an unsigned long long q and through the template",
          bounds_are(odd_u, -r8, r8) && bounds_are(even_u, 0.0, r8) && bounds_are(odd_t, -r8, r8)
          && bounds_are(inverse_t, 0x1.fffffffc6d73bp-1, 0x1.fffffffecf26ap-1),
          [&] { return hex(odd_u) + " " + hex(even_u) + " " + hex(odd_t) + " " + hex(inverse_t); });
    const interval rev = pownRev(c8, 5000000001LL), rev_even = pownRev(c8, 5000000000LL);
    const interval rev_positive = pownRev(c8, numsToInterval(0.0, oo), 5000000000LL);
    check("pownRev(c, 3000000000u) is nth_root_rel(), and beyond the unsigned ints the roots",
          pownRev(c8, 3000000000u).set_eq(gaol_core::nth_root_rel(c8, 3000000000u, interval::universe()))
          && bounds_are(rev, r2, r8) && bounds_are(rev_even, -r8, r8) && bounds_are(rev_positive, r2, r8),
          [&] { return hex(rev) + " " + hex(rev_even) + " " + hex(rev_positive); });
    /* pownRev() for p <= 0 (GAOL v5), which threw std::domain_error: for p = 0
       x where 1 is in c, and for p < 0 the hull of the x != 0 with x^p in c,
       nth_root_rel(c, p, x), with the int of the function, with the integer
       of the template, and beyond the ints. The bounds are the exact roots of
       the reciprocals where they are doubles, and the tightest ones around
       8^(-1/q) and 2^(-1/q) for q = 5 10^9 and 5 10^9 + 1 alike (mpmath, 600
       bits) */
    const interval c18(1.0, 8.0), x23 = numsToInterval(-2.0, 3.0);
    check("pownRev(c, x, 0) is x where 1 is in c, the empty set elsewhere, with and without x",
          pownRev(c18, 0L).set_eq(interval::universe()) && pownRev(c8, 0L).is_empty()
          && pownRev(c18, x23, 0LL).set_eq(x23) && pownRev(c8, x23, 0).is_empty()
          && pownRev(interval::emptyset(), 0).is_empty() && pownRev(c18, interval::emptyset(), 0).is_empty(),
          [&] { return hex(pownRev(c18, 0L)) + " " + hex(pownRev(c8, 0L)) + " " + hex(pownRev(c18, x23, 0LL)); });
    const interval minus3 = pownRev(c18, -3), minus4 = pownRev(interval(1.0, 16.0), numsToInterval(0.0, oo), -4L);
    const interval minus1 = pownRev(interval(2.0, 4.0), -1LL), below = pownRev(numsToInterval(-8.0, -1.0), -3);
    check("pownRev(c, p) for a negative p: the roots of the reciprocals of c, of either sign for an even p",
          bounds_are(minus3, 0.5, 1.0) && bounds_are(minus4, 0.5, 1.0) && bounds_are(minus1, 0.25, 0.5)
          && bounds_are(below, -1.0, -0.5) && bounds_are(pownRev(interval(1.0, 16.0), -4), -1.0, 1.0),
          [&] { return hex(minus3) + " " + hex(minus4) + " " + hex(minus1) + " " + hex(below); });
    // c is cut at 0, and 0 is no solution: the hull of the roots of [-1, 2], the
    // whole line, would keep the part of x in the gap, and the roots of
    // [1, +oo] reach 0
    const interval part = pownRev(numsToInterval(1.0, oo), numsToInterval(0.0, 3.0), -2);
    check("pownRev(c, x, p) for a negative p: the gap of c holds no solution, and neither does 0",
          pownRev(interval(-1.0, 2.0), numsToInterval(-0.5, 0.5), -3).is_empty()
          && pownRev(interval(-1.0, 2.0), numsToInterval(-0.5, 0.5), -2).is_empty()
          && pownRev(numsToInterval(1.0, oo), interval(0.0), -2).is_empty()
          && pownRev(numsToInterval(-oo, -1.0), interval(0.0), -3).is_empty()
          && pownRev(interval::universe(), interval(0.0), -5).is_empty()
          && pownRev(interval(0.0), interval::universe(), -3).is_empty() && bounds_are(part, 0.0, 1.0),
          [&] { return hex(part); });
    // Beyond the ints, and INT_MIN, whose opposite no int holds
    const double q_lo = 0x1.fffffffc6d73bp-1, q_hi = 0x1.fffffffecf26ap-1;
    const interval huge_odd = pownRev(c8, -5000000001LL), huge_even = pownRev(c8, -5000000000LL);
    const interval huge_below = pownRev(numsToInterval(-8.0, -2.0), -5000000001LL), huge_long = pownRev(c8, -5000000001L);
    const interval smallest = pownRev(c8, std::numeric_limits<int>::min());
    check("pownRev(c, p) for a negative p beyond the ints: the tightest roots of the reciprocals, cut at 0",
          bounds_are(huge_odd, q_lo, q_hi) && bounds_are(huge_even, -q_hi, q_hi) && bounds_are(huge_below, -q_hi, -q_lo)
          && bounds_are(huge_long, q_lo, q_hi)
          && pownRev(numsToInterval(-8.0, 8.0), numsToInterval(-0.5, 0.5), -5000000001LL).is_empty(),
          [&] { return hex(huge_odd) + " " + hex(huge_even) + " " + hex(huge_below) + " " + hex(huge_long); });
    check("pownRev(c, INT_MIN): the order is 2^31, which an int cannot negate",
          !smallest.is_empty() && smallest.right() > 0.9999999 && smallest.right() < 1.0
          && smallest.left() == -smallest.right(),
          [&] { return hex(smallest); });
    // pown(e, n) of the expressions for an unsigned beyond the ints, which
    // made [1, 2]^3000000000u [0, 1], converting n to a negative int: pown,
    // as for an interval (GAOL v5: [-oo, +oo])
    const interval pown_u = value_of(pown(gaol::expression(numsToInterval(1.0, 2.0)), 3000000000u));
    check("pown(e, 3000000000u) of an expression: pown", bounds_are(pown_u, 1.0, oo),
          [&] { return hex(pown_u); });
    // Beyond 2^53, an n that is no double: x^h x^l, n = h + l, h a double and
    // |l| < 2^11, as gaol_pown_ll() splits n (GAOL v5: [-oo, +oo]), not the
    // power of the double nearest n, x^(2^53) or x^(2^53 + 2), [1] for
    // [-1]^(2^53 + 1); an n that is a double, x^n, as for an interval
    const interval minus_one = value_of(pown(gaol::expression(interval(-1.0)), 9007199254740993LL));
    const interval minus_one_inverse = value_of(pown(gaol::expression(interval(-1.0)), -9007199254740993LL));
    const interval split = value_of(pown(gaol::expression(just_above_one), 9007199254740993LL));
    const interval split_negative = value_of(gaol::pow(gaol::expression(-just_above_one), -9007199254740995LL));
    const interval whole = value_of(pown(gaol::expression(just_above_one), 9007199254740994LL));
    check("pown(e, n) and gaol::pow(e, n) of an expression beyond 2^53: x^n, within three doubles",
          bounds_are(minus_one, -1.0, -1.0) && bounds_are(minus_one_inverse, -1.0, -1.0)
          && within_three(split, 0x1.d8e64b8d4ddadp+2, 0x1.d8e64b8d4ddaep+2)
          && within_three(split_negative, -0x1.152aaa3bf81cap-3, -0x1.152aaa3bf81c9p-3)
          && whole.set_eq(tight_ll),
          [&] { return hex(minus_one) + " " + hex(minus_one_inverse) + " " + hex(split) + " " + hex(split_negative) + " " + hex(whole); });
  }

  /*
    x^n for an n of more than 64 bits, an __int128 where it is an integer type
    (the GNU modes of GCC and Clang): as x^+-(2^63 - 1) or x^+-(2^63 - 2), of
    the sign and the parity of n (GAOL v5), the powers of every base but 0,
    +-1 and +-oo being beyond the doubles there
  */
  template <class T>
  void exponents_beyond_64_bits()
  {
#if __cplusplus >= 201703L
    if constexpr (gaol_detail::is_integer<T>::value) {
      const T odd_n = (static_cast<T>(1) << 100) + 1, even_n = odd_n + 1;
      const double dmax = std::numeric_limits<double>::max();
      const struct { const char *name; interval r; double lo, hi; } cases[] = {
        { "pown([-2], 2^100 + 1)", pown(interval(-2.0), odd_n), -oo, -dmax },
        { "pown([-2], -(2^100 + 1))", pown(interval(-2.0), -odd_n), -0x1p-1074, -0.0 },
        { "pown([-0.5], -(2^100 + 2))", pown(interval(-0.5), -even_n), dmax, oo },
        { "pown([-1, -0.5], 2^100 + 2)", pown(numsToInterval(-1.0, -0.5), even_n), 0.0, 1.0 },
      };
      for (const auto& c : cases) {
        check(std::string(c.name) + " beyond 64 bits", !c.r.is_empty() && c.r.left() == c.lo && c.r.right() == c.hi,
              [&] { return hex(c.r); });
      }
    }
#endif
  }

  void pow_of_the_standard()
  {
    // With a number as exponent, the pow of gaol was found here too, and gave
    // [1, 16] for pow([-4, -1], 2) and pow([-4, -1], 2.0), and [1] for pow([0], 0)
    const interval x = numsToInterval(-4.0, -1.0);
    check("pow(x, 2), pow(x, 2.0), pow(x, [2]) are the pow of Table 9.1: empty for x < 0",
          pow(x, 2).is_empty() && pow(x, 2.0).is_empty() && pow(x, interval(2.0)).is_empty()
          && pow(x, 3).is_empty() && pow(x, 0.5).is_empty(),
          [&] { return hex(pow(x, 2)) + " " + hex(pow(x, 2.0)); });
    const interval y = numsToInterval(0.25, 0.5), z = numsToInterval(-1.0, 2.0);
    check("pow(x, y) unqualified is gaol_ieee1788::pow", pow(y, z).set_eq(gaol_ieee1788::pow(y, z)),
          [&] { return hex(pow(y, z)); });
    check("pown(x, 2) is the integer power, [1, 16] for x = [-4, -1]",
          pown(x, 2).set_eq(numsToInterval(1.0, 16.0)) && pown(x, 3).set_eq(numsToInterval(-64.0, -1.0))
          && pown(interval(0.0), 0).set_eq(interval(1.0)),
          [&] { return hex(pown(x, 2)); });
    check("gaol::pow(x, 2) remains GAOL's, [1, 16]",
          gaol::pow(x, 2).set_eq(numsToInterval(1.0, 16.0)) && gaol::pow(x, 2.0).set_eq(numsToInterval(1.0, 16.0))
          && gaol::pow(x, interval(2.0)).set_eq(numsToInterval(1.0, 16.0)),
          [&] { return hex(gaol::pow(x, 2)); });
    const interval zero(0.0);
    check("pow([0], 0) and pow([0], [0]) have no value, pow([0], 1) = [0]",
          pow(zero, 0).is_empty() && pow(zero, interval(0.0)).is_empty() && pow(zero, 1).set_eq(zero),
          [&] { return hex(pow(zero, 0)); });
    const interval b(0.0, 4.0);
    check("pow([0, 4], n) = pown([0, 4], n) for x >= 0: [1] for n = 0, [1/4, +oo] for n = -1",
          pow(b, 0).set_eq(interval(1.0)) && pow(b, -1).set_eq(numsToInterval(0.25, oo))
          && pow(b, 2).set_eq(numsToInterval(0.0, 16.0)),
          [&] { return hex(pow(b, 0)) + " " + hex(pow(b, -1)); });
    check("pow(x, +oo) and pow(x, NaN) contain no real exponent: empty",
          pow(b, oo).is_empty() && pow(b, std::numeric_limits<double>::quiet_NaN()).is_empty(),
          [&] { return hex(pow(b, oo)); });

    // An integer exponent beyond the ints: the pow of the standard on x >= 0,
    // where it gave [-oo, +oo] by taking pown
    const double dmax = std::numeric_limits<double>::max();
    const interval big = pow(numsToInterval(2.0, 3.0), interval(1e10));
    check("pow([2, 3], [1e10]) = [DBL_MAX, +oo]", big.left() == dmax && big.right() == oo,
          [&] { return hex(big); });
    const interval small = pow(numsToInterval(0.5, 0.9), interval(1e10));
    check("pow([0.5, 0.9], [1e10]) is in [0, 2^-1074]",
          small.left() == 0.0 && small.right() > 0.0 && small.right() <= std::nextafter(0.0, 1.0),
          [&] { return hex(small); });
    const interval one = pow(interval(1.0), interval(-1e12));
    check("pow([1], [-1e12]) = [1]", one.set_eq(interval(1.0)), [&] { return hex(one); });
    // A base with no point in [0, +oo], whose part there is empty: the powers
    // beyond 2^1014, which GAOL computed itself before upstream commit
    // c01f08c5, gave [0, 2^-1074] to its NaN bounds if the empty part were not
    // caught first (GAOL v5)
    const interval no_base = pow(numsToInterval(-4.0, -1.0), interval(dmax));
    check("pow([-4, -1], [DBL_MAX]): empty", no_base.is_empty(), [&] { return hex(no_base); });

    // The expressions of pow and pown, evaluated: pow(e1, e2) is the pow of
    // the standard, where the node of gaol::pow(e1, e2) computes GAOL's
    const gaol::expression base(x), exponent(interval(2.0));
    const interval std_pow = value_of(pow(base, exponent)), std_pown = value_of(pown(base, 2));
    const interval gaol_pow = value_of(gaol::pow(base, exponent));
    check("pow(e1, e2) and pown(e, n) on expressions are those of the standard",
          std_pow.is_empty() && std_pown.set_eq(numsToInterval(1.0, 16.0)) && gaol_pow.set_eq(numsToInterval(1.0, 16.0)),
          [&] { return hex(std_pow) + " " + hex(std_pown) + " " + hex(gaol_pow); });
  }

  /*
    The pow of Table 9.1 is written once, in gaol/core/gaol_interval_core.cpp:
    gaol_ieee1788::pow(x, y) is that function, and gaol::pow(x, y) calls it for
    every exponent but a degenerate integer, which takes pown on the whole of x,
    beyond the ints too. Each box below reaches a branch of the
    shared function, or of what gaol::pow adds to it, the corners where the
    base is 0 or +oo or the exponent infinite included. The bounds were taken
    from the code, and each was checked against the exact power, computed with
    500 bits by mpmath (tests/tools/pow/checkrows.py): it encloses it, and is
    the tightest bound, but for the integer power [0, 1 + 2^-52]^-2, whose
    lower bound is one double below, (1 + 2^-52)^-2 being within the bound K
    of the error of the integer powers of a double
    (gaol/core/gaol_interval_core.cpp). They are compared with ==, which does not
    tell -0 from +0: the sign of a zero bound is not checked.
  */
  void pow_on_boxes()
  {
    struct Box
    {
      interval x, y, standard, hybrid;
      Box(const interval& x_, const interval& y_, const interval& s) : x(x_), y(y_), standard(s), hybrid(s) {}
      Box(const interval& x_, const interval& y_, const interval& s, const interval& h)
        : x(x_), y(y_), standard(s), hybrid(h) {}
    };
    const auto I = [](double l, double u) { return interval(l, u); };
    const auto P = [](double p) { return interval(p); };
    const interval none = interval::emptyset(), all = interval::universe();
    // {x, y, gaol_ieee1788::pow(x, y)} where gaol::pow(x, y) is the same, {x, y, gaol_ieee1788::pow(x, y),
    // gaol::pow(x, y)} where it is not
    const Box boxes[] = {
      // an empty base or exponent
      {none, P(2), none},
      {P(2), none, none},
      {none, P(2147483648.0), none},
      // x < 0: no value for the pow of the standard, pown for a degenerate integer exponent of gaol::pow
      {I(-4, -1), P(0.5), none},
      {I(-4, -1), I(1.5, 2.5), none},
      {I(-4, -1), P(2), none, I(1, 16)},
      {I(-4, -1), P(3), none, I(-64, -1)},
      {I(-4, -1), P(-2), none, I(0.0625, 1)},
      {I(-oo, -1), P(2), none, I(1, oo)},
      {I(-3, 5), P(0.5), I(0, 0x1.1e3779b97f4a8p+1)},
      {I(-3, 5), I(0.5, 1.5), I(0, 0x1.65c55827df1d2p+3)},
      {I(-3, 5), I(-0.5, 1.5), I(0, oo)},
      // x cut to {0} (x = {0}, or x from below to 0): 0^y is 0 for y > 0, and has no value for y <= 0
      {P(0), I(1, 2), P(0)},
      {P(0), I(-1, 2), P(0)},
      {P(0), P(0.5), P(0)},
      {P(0), P(3), P(0)},
      {P(0), P(-0.5), none},
      {P(0), I(-2, 0), none},
      {P(0), P(0), none, P(1)},
      {P(0), P(-1), none},
      {I(-1, -0.0), P(0.5), P(0)},
      {I(-2, 0), P(2), P(0), I(0, 4)},
      {P(0), P(1e10), P(0)},
      {P(0), P(-1e10), none},
      // a degenerate integer exponent within the ints: pown, on x >= 0 for the standard's pow, on the whole of x for gaol::pow
      {I(2, 3), P(5), I(32, 243)},
      {I(-2, 3), P(3), I(0, 27), I(-8, 27)},
      {I(-2, 3), P(2), I(0, 9)},
      {I(-3, 2), P(2), I(0, 4), I(0, 9)},
      {I(0, 4), P(0), P(1)},
      {I(0, 4), P(-1), I(0.25, oo)},
      {I(0, 4), P(2), I(0, 16)},
      {I(0.5, 2), P(-3), I(0.125, 8)},
      {P(1.1), P(3), I(0x1.54bc6a7ef9db3p+0, 0x1.54bc6a7ef9db4p+0)},
      {P(1), P(7), P(1)},
      {I(-oo, oo), P(2), I(0, oo)},
      {I(-oo, oo), P(3), I(0, oo), all},
      {I(1e-300, 1e300), P(3), I(0, oo)},
      {I(-1, 2), P(2147483647.0), I(0, oo), I(-1, oo)},
      {I(0.5, 1), P(-2147483648.0), I(1, oo)},
      {I(-2, 3), P(100), I(0, 0x1.69194f299cddbp+158)},
      // pown, one double below the tightest lower bound 0x1.ffffffffffffcp-1: (1 + 2^-52)^-2 is within twice the
      // error bound of pown of it (decided on 13 October 2026, TODO.md B), and the corners would give it
      {I(0, 0x1.0000000000001p+0), P(-2), I(0x1.ffffffffffffbp-1, oo)},
      // an integer exponent beyond the ints: the corners at the bounds of x for the standard's pow, pown on the whole
      // of x for gaol::pow, the same on x >= 0 (GAOL v5: [-oo, +oo])
      {I(2, 3), P(1e10), I(0x1.fffffffffffffp+1023, oo)},
      {I(0.5, 0.9), P(1e10), I(0, 0x1p-1074)},
      {I(2, 3), P(-1e10), I(0, 0x1p-1074)},
      {P(1), P(-1e12), P(1)},
      {I(-1, 1), P(2147483649.0), I(0, 1), I(-1, 1)},
      {I(-1, 0.5), P(-2147483649.0), I(0x1.fffffffffffffp+1023, oo), all},
      {I(0, 2), P(1e10), I(0, oo)},
      {I(0, 0.5), P(-1e10), I(0x1.fffffffffffffp+1023, oo)},
      {I(2, oo), P(1e10), I(0x1.fffffffffffffp+1023, oo)},
      {I(0, oo), P(1e12), I(0, oo)},
      {I(1, 2), P(2147483648.0), I(1, oo)},
      {I(0, 0x1.00000004p+0), P(2147483649.0), I(0, 0x1.d8e64b8d4ddaep+2)},
      {I(0, 0x1.00000004p+0), P(-2147483649.0), I(0x1.152aaa3bf81cbp-3, oo)},
      {I(-0.0, 0x1.00000004p+0), P(-2147483649.0), I(0x1.152aaa3bf81cbp-3, oo)},
      {I(0x1.fffffff8p-1, oo), P(2147483649.0), I(0x1.152aaa334ec76p-3, oo)},
      {I(0x1.fffffff8p-1, oo), P(-2147483649.0), I(0, 0x1.d8e64b9c150d4p+2)},
      {I(-2, -1), P(1e10), none, I(1, oo)},
      {I(-0x1.00000004p+0, -1), P(2147483649.0), none, I(-0x1.d8e64b8d4ddaep+2, -1)},
      // CORE-MATH's pow at the corners of the box, from a base from 0, a base above 1, below 1, and around 1
      {I(2, 3), P(0.5), I(0x1.6a09e667f3bccp+0, 0x1.bb67ae8584cabp+0)},
      // where the power at a corner is a double, the lower bound is that double
      {P(4), P(0.5), P(2)},
      {I(4, 16), P(-0.5), I(0.25, 0.5)},
      {I(3, 4), I(2, 3), I(9, 64)},
      {I(0, 2), P(0.5), I(0, 0x1.6a09e667f3bcdp+0)},
      {I(0, 0.5), I(0.5, 1.5), I(0, 0x1.6a09e667f3bcdp-1)},
      {I(1.5, 2.5), I(0.5, 1.5), I(0x1.3988e1409212ep+0, 0x1.f9f6e4990f228p+1)},
      {I(1.5, 2.5), I(-1.5, -0.5), I(0x1.030dc4ea03a72p-2, 0x1.a20bd700c2c3ep-1)},
      {I(1.5, 2.5), I(-1, 2), I(0x1.9999999999999p-2, 6.25)},
      {I(0.3, 0.7), I(0.5, 1.5), I(0x1.50854f2c3d222p-3, 0x1.ac5eb3f7ab2f8p-1)},
      {I(0.3, 0.7), I(-1.5, -0.5), I(0x1.31fa808c55b43p+0, 0x1.857dd943cb7f5p+2)},
      {I(0.3, 0.7), I(-1.5, 2.5), I(0x1.93d32bceafc29p-5, 0x1.857dd943cb7f5p+2)},
      {I(0.5, 2), I(1.5, 2.5), I(0x1.6a09e667f3bccp-3, 0x1.6a09e667f3bcdp+2)},
      {I(0.5, 2), I(-2.5, -1.5), I(0x1.6a09e667f3bccp-3, 0x1.6a09e667f3bcdp+2)},
      {I(0.5, 2), I(-1.5, 2.5), I(0x1.6a09e667f3bccp-3, 0x1.6a09e667f3bcdp+2)},
      {I(0.1, 2), I(-2, 0.5), I(0.25, 100)},
      {I(1, 2), I(-0.5, 1.5), I(0x1.6a09e667f3bccp-1, 0x1.6a09e667f3bcdp+1)},
      {P(1), I(-0.5, 1.5), P(1)},
      {P(1), P(0.1), P(1)},
      {I(0.3, 1), I(0.5, 1.5), I(0x1.50854f2c3d222p-3, 1)},
      {I(0.3, 3), I(-0.5, 1.5), I(0x1.50854f2c3d222p-3, 0x1.4c8dc2e42398p+2)},
      {I(3, 7), I(1.5, 2.5), I(0x1.4c8dc2e42397fp+2, 0x1.03489be058693p+7)},
      {I(1e-5, 1e5), I(-2.5, 3.5), I(0x1.d2ab78e5b8624p-59, 0x1.18ddde9363225p+58)},
      {P(2), P(1023.5), I(0x1.6a09e667f3bccp+1023, 0x1.6a09e667f3bcdp+1023)},
      {I(2, 3), I(2147483647.0, 2147483649.0), I(0x1.fffffffffffffp+1023, oo)},
      {P(1e-300), P(1.5), I(0, 0x1p-1074)},
      {P(1e300), P(1.5), I(0x1.fffffffffffffp+1023, oo)},
      // the corners where the base is 0 or +oo or the exponent infinite, the limits of x^y there: an infinite
      // bound, or a base from 0 with an exponent that is not above 0
      {I(2, oo), I(0.5, 1.5), I(0x1.6a09e667f3bccp+0, oo)},
      {I(0, 2), I(-1.5, -0.5), I(0x1.6a09e667f3bccp-2, oo)},
      {I(0, 1), I(-1, 1), I(0, oo)},
      {I(0, 1), I(0, 2), I(0, 1)},
      {I(0.5, 2), I(0, oo), I(0, oo)},
      {I(1, 3), I(-oo, 2), I(0, 9)},
      {I(0.25, 0.5), I(-oo, -1), I(2, oo)},
      {I(2, 4), I(1, oo), I(2, oo)},
      {I(-oo, oo), P(0.5), I(0, oo)},
      {I(0, oo), I(1, 2), I(0, oo)},
      {I(4, oo), P(-0.5), I(0, 0.5)},
      {I(0, 3), P(-0.5), I(0x1.279a74590331cp-1, oo)},
      // the limits at the corners, each kind (GAOL v5): x^y is 0 or +oo where x is 0 or +oo or y infinite
      {I(4, oo), P(0.5), I(2, oo)},
      {I(-oo, 0x1p-1000), I(-oo, -1), I(0x1p+1000, oo)},
      {I(2, oo), I(-1.5, -0.5), I(0, 0x1.6a09e667f3bcdp-1)},
      {I(2, oo), I(-1, 0), I(0, 1)},
      {I(2, 3), I(-oo, -1), I(0, 0.5)},
      {I(0.25, 0.5), I(1, oo), I(0, 0.5)},
      {I(0.5, 2), I(-oo, oo), I(0, oo)},
      {P(1), I(-oo, oo), P(1)},
      {I(0, oo), I(-oo, oo), I(0, oo)},
      {I(0.5, oo), I(-oo, -1), I(0, oo)},
      {I(0.5, oo), P(0.5), I(0x1.6a09e667f3bccp-1, oo)},
      {I(0.5, oo), I(-2, -0.5), I(0, 4)},
      {I(0, 3), I(0, 1), I(0, 3)},
      {I(0, 1), I(-oo, 0), I(1, oo)},
      {I(0, 0.5), I(-1, 0), I(1, oo)},
      {I(0, 2), I(-1, 0), I(0.5, oo)},
      {I(0, 4), P(-0.5), I(0.5, oo)},
      {I(0, oo), P(-0.5), I(0, oo)},
      {I(-0.0, 1), I(-1, 1), I(0, oo)},
    };
    const auto same = [](const interval& a, const interval& b) {
      return a.is_empty() ? b.is_empty() : (!b.is_empty() && a.left() == b.left() && a.right() == b.right());
    };
    const auto text = [](const interval& a) { return a.is_empty() ? std::string("[empty]") : hex(a); };
    for (const Box& box : boxes) {
      const interval s = gaol_ieee1788::pow(box.x, box.y), h = gaol::pow(box.x, box.y);
      const std::string what = "pow(" + text(box.x) + ", " + text(box.y) + ")";
      check("gaol_ieee1788::pow(x, y) on boxes: the pow of Table 9.1", same(s, box.standard),
            [&] { return what + " is " + text(s) + " rather than " + text(box.standard); });
      check("gaol::pow(x, y) on boxes: pown for a degenerate integer exponent, the pow of Table 9.1 otherwise",
            same(h, box.hybrid), [&] { return what + " is " + text(h) + " rather than " + text(box.hybrid); });
      if (!box.y.is_empty() && box.y.left() == box.y.right()) {
        // With the exponent as a double: the same functions, gaol::pow(x, p) taking pown for an integer p
        // within the ints without the call of gaol::pow(x, [p])
        const double p = box.y.left();
        const interval sp = gaol_ieee1788::pow(box.x, p), hp = gaol::pow(box.x, p);
        check("gaol_ieee1788::pow(x, p) on boxes is gaol_ieee1788::pow(x, [p])", same(sp, box.standard),
              [&] { return what + " is " + text(sp) + " rather than " + text(box.standard); });
        check("gaol::pow(x, p) on boxes is gaol::pow(x, [p])", same(hp, box.hybrid),
              [&] { return what + " is " + text(hp) + " rather than " + text(box.hybrid); });
      }
    }
  }

  void gaol_functions()
  {
    const interval x = numsToInterval(0.25, 0.5), y = numsToInterval(-1.0, 2.0);
    check("sin, exp, sqrt, abs... on an interval are GAOL's",
          sin(x).set_eq(gaol::sin(x)) && exp(x).set_eq(gaol::exp(x)) && sqrt(x).set_eq(gaol::sqrt(x))
          && abs(y).set_eq(gaol::abs(y)) && sqr(y).set_eq(gaol::sqr(y)) && sign(y).set_eq(gaol::sign(y))
          && fma(x, y, x).set_eq(gaol::fma(x, y, x)) && expm1(x).set_eq(gaol::expm1(x))
          && min(x, y).set_eq(gaol::min(x, y)) && hypot(x, y).set_eq(gaol::hypot(x, y))
          && compoundm1(x, y).set_eq(gaol::compoundm1(x, y))
          && sinPi(x).set_eq(gaol::sinpi(x)),
          [&] { return hex(sin(x)); });
    check("the qualified names take intervals",
          gaol_ieee1788::sin(x).set_eq(gaol::sin(x)) && gaol_ieee1788::min(x, y).set_eq(gaol::min(x, y)),
          [&] { return hex(gaol_ieee1788::sin(x)); });
    // The operators of intervals, brought in by using-declarations of those of
    // gaol_core (GAOL v5): the qualified names compile only if the namespace
    // holds them, and the results are exact in doubles
    const interval a = numsToInterval(1.0, 2.0), b = numsToInterval(4.0, 8.0), c = numsToInterval(1.5, 6.0);
    check("the operators of intervals are in gaol_ieee1788",
          gaol_ieee1788::operator+(a, b).set_eq(numsToInterval(5.0, 10.0))
          && gaol_ieee1788::operator-(a, b).set_eq(numsToInterval(-7.0, -2.0))
          && gaol_ieee1788::operator*(a, b).set_eq(numsToInterval(4.0, 16.0))
          && gaol_ieee1788::operator/(b, a).set_eq(numsToInterval(2.0, 8.0))
          && gaol_ieee1788::operator%(a, b).set_eq(numsToInterval(0.125, 0.5))
          && gaol_ieee1788::operator&(a, c).set_eq(numsToInterval(1.5, 2.0))
          && gaol_ieee1788::operator|(a, b).set_eq(numsToInterval(1.0, 8.0)),
          [&] {
            return hex(gaol_ieee1788::operator+(a, b)) + " " + hex(gaol_ieee1788::operator-(a, b)) + " "
              + hex(gaol_ieee1788::operator*(a, b)) + " " + hex(gaol_ieee1788::operator/(b, a)) + " "
              + hex(gaol_ieee1788::operator%(a, b)) + " " + hex(gaol_ieee1788::operator&(a, c)) + " "
              + hex(gaol_ieee1788::operator|(a, b));
          });
    check("the functions of C on numbers are C's", sqrt(4.0) == 2.0 && floor(2.5) == 2.0 && abs(-3) == 3,
          [] { return std::string(); });
    // erf and erfc, which IEEE 1788-2015 does not name and gaol_ieee1788
    // does not hold: argument-dependent lookup finds those of gaol_core on an
    // interval, and erf(0.0) remains C's (GAOL v5)
    check("erf and erfc on an interval are GAOL's, on a number C's",
          erf(x).set_eq(gaol::erf(x)) && erfc(y).set_eq(gaol::erfc(y)) && erf(0.0) == 0.0 && erfc(0.0) == 1.0,
          [&] { return hex(erf(x)) + " " + hex(erfc(y)); });
  }

  // Every name of the standard gaol_ieee1788 provides, called unqualified
  void names_of_the_standard()
  {
    const interval x = numsToInterval(0.25, 0.5), y = numsToInterval(-1.0, 2.0);
    const interval forward[] = {
      neg(x), add(x, y), sub(x, y), mul(x, y), div(x, y), recip(x), sqr(y), sqrt(x), fma(x, y, x),
      pown(y, 3), pow(x, y), pow(x, 3), pow(x, 0.5), exp(x), exp2(x), exp10(x), log(x), log2(x),
      log10(x), sin(x), cos(x), tan(x), asin(x), acos(x), atan(x), atan2(y, x), sinh(x), cosh(x),
      tanh(x), asinh(x), acosh(y), atanh(x), sign(y), ceil(y), floor(y), trunc(y),
      roundTiesToEven(y), roundTiesToAway(y), abs(y), min(x, y), max(x, y), rootn(x, -3), expm1(x),
      exp2m1(x), exp10m1(x), logp1(x), log2p1(x), log10p1(x), hypot(x, y), compoundm1(x, y), rSqrt(x), sinPi(x),
      cosPi(x), tanPi(x), asinPi(x), acosPi(x), atanPi(x), atan2Pi(y, x),
    };
    const interval reverse[] = {
      sqrRev(x), sqrRev(x, y), absRev(x), pownRev(x, 3), sinRev(x), cosRev(x), tanRev(x),
      coshRev(y), mulRev(x, y), cancelMinus(y, x), cancelPlus(y, x), intersection(x, y),
      convexHull(x, y), empty(), entire(), powRev1(y, x), powRev1(y, x, y), powRev2(x, x), powRev2(x, x, y),
    };
    double m, r;
    midRad(x, m, r);
    check("pow and pown have the results of IEEE 1788",
          forward[9].set_eq(numsToInterval(-1.0, 8.0))
          && forward[10].set_eq(numsToInterval(1.0 / 16.0, 4.0))
          && forward[11].set_eq(numsToInterval(1.0 / 64.0, 1.0 / 8.0))
          && forward[12].set_eq(numsToInterval(0.5, 0x1.6a09e667f3bcdp-1)),
          [&] { return hex(forward[9]) + " " + hex(forward[10]) + " " + hex(forward[11]) + " " + hex(forward[12]); });
    const bool b = isEmpty(empty()) && isEntire(entire()) && equal(x, x) && subset(x, y)
      && less(x, x) && !strictLess(x, x) && !precedes(y, x) && !strictPrecedes(x, y)
      && interior(x, y) && !disjoint(x, y) && isCommonInterval(x) && !isSingleton(x)
      && isMember(0.3, x);
    check("the functions of the standard's names",
          b && inf(x) == 0.25 && sup(x) == 0.5 && m == mid(x) && r == rad(x) && wid(x) == 0.25
          && mag(y) == 2.0 && mig(y) == 0.0 && forward[0].set_eq(-x) && reverse[8].set_eq(y / x)
          && exactToInterval(intervalToExact(x)).set_eq(x) && !intervalToText(x).empty(),
          [] { return std::string(); });
    // powRev1(b, c, x) and powRev2(a, c, x), x being [-oo, +oo] when left
    // out (GAOL v5): x^y in [0.25, 0.5] for x, y in [0.25, 0.5] asks y in
    // [log(0.5)/log(0.25), log(0.25)/log(0.5)] = [0.5, 2], and x^2 in [4, 9]
    // x in [2, 3]
    check("powRev1 and powRev2 have the results of IEEE 1788",
          reverse[17].set_eq(numsToInterval(0.5, 2.0)) && reverse[18].set_eq(numsToInterval(0.5, 2.0))
          && powRev1(numsToInterval(2.0, 2.0), numsToInterval(4.0, 9.0)).set_eq(numsToInterval(2.0, 3.0))
          && powRev1(numsToInterval(2.0, 2.0), numsToInterval(4.0, 9.0), y).set_eq(numsToInterval(2.0, 2.0)),
          [&] { return hex(reverse[17]) + " " + hex(reverse[18]); });
    // atan2Rev1(b, c, x) and atan2Rev2(a, c, x), x being [-oo, +oo] when left
    // out (GAOL v5): the angle 0 is that of the points (x, 0), x > 0
    const interval zero = numsToInterval(0.0, 0.0), positive = numsToInterval(1.0, 2.0);
    check("atan2Rev1 and atan2Rev2, with and without their last argument",
          atan2Rev1(positive, zero).set_eq(zero) && atan2Rev1(-positive, zero, y).is_empty()
          && atan2Rev2(zero, zero).set_eq(numsToInterval(0.0, oo)) && atan2Rev2(zero, zero, y).set_eq(numsToInterval(0.0, 2.0)),
          [&] { return hex(atan2Rev1(positive, zero)) + " " + hex(atan2Rev2(zero, zero, y)); });
  }

  /*
    overlap(a, b) of 10.6.4, the 16 states of Table 10.7 (GAOL v5), against
    the set specification of the table, which is read as it is written: the
    quantifiers of each of the 13 nonempty states, over the points of a and
    of b, and the rule of the standard for a point interval on a bound of the
    other one (never meets or metBy, whose specifications hold for it too).
    overlap() tells the states from the order of the bounds; the oracle
    tells them from the quantifiers, and uses the bounds only to place the
    intervals in the model. A statement of the table is made of <, <= and =
    on at most two points, in intervals whose ends are given numbers, so that
    a finite model of the real line holds it exactly: the given numbers (the
    finite bounds of the two intervals), and, in each of the gaps between them
    and beyond the first and the last, three points, a gap having more points
    than a statement can tell apart. An infinite bound is no point: the
    interval goes to the end of the model. The pairs are those of a grid of
    bounds next to the difficult ones (the infinities, the zeros of both signs,
    the subnormals, the largest double, and the doubles on either side of
    the numbers where two bounds meet), and random ones whose bounds are drawn
    from a few doubles so that they meet often.
  */
  namespace overlap_model
  {
    typedef std::vector<int> points;

    template <class F> bool for_all(const points& s, F f)
    {
      for (int p : s) {
        if (!f(p)) {
          return false;
        }
      }
      return true;
    }

    template <class F> bool there_is(const points& s, F f)
    {
      for (int p : s) {
        if (f(p)) {
          return true;
        }
      }
      return false;
    }

#define GAOL_ALL(s, v, e) for_all(s, [&](int v) { return (e); })
#define GAOL_SOME(s, v, e) there_is(s, [&](int v) { return (e); })

    // The set specification of Table 10.7 of the state k of the 13 nonempty
    // ones, in the order of the table: before, meets, overlaps, starts,
    // containedBy, finishes, equals, finishedBy, contains, startedBy,
    // overlappedBy, metBy, after. a and b are the points of the first and of
    // the second interval, A and B the points of each
    bool specification_holds(int k, const points& A, const points& B)
    {
      switch (k) {
      case 0:
        return GAOL_ALL(A, a, GAOL_ALL(B, b, a < b));
      case 1:
        return GAOL_ALL(A, a, GAOL_ALL(B, b, a <= b)) && GAOL_SOME(A, a, GAOL_ALL(B, b, a < b))
          && GAOL_SOME(A, a, GAOL_SOME(B, b, a == b));
      case 2:
        return GAOL_SOME(A, a, GAOL_ALL(B, b, a < b)) && GAOL_SOME(B, b, GAOL_ALL(A, a, a < b))
          && GAOL_SOME(A, a, GAOL_SOME(B, b, b < a));
      case 3:
        return GAOL_ALL(B, b, GAOL_SOME(A, a, a <= b)) && GAOL_ALL(A, a, GAOL_SOME(B, b, b <= a))
          && GAOL_SOME(B, b, GAOL_ALL(A, a, a < b));
      case 4:
        return GAOL_SOME(B, b, GAOL_ALL(A, a, b < a)) && GAOL_SOME(B, b, GAOL_ALL(A, a, a < b));
      case 5:
        return GAOL_SOME(B, b, GAOL_ALL(A, a, b < a)) && GAOL_ALL(B, b, GAOL_SOME(A, a, b <= a))
          && GAOL_ALL(A, a, GAOL_SOME(B, b, a <= b));
      case 6:
        return GAOL_ALL(A, a, GAOL_SOME(B, b, a == b)) && GAOL_ALL(B, b, GAOL_SOME(A, a, b == a));
      case 7:
        return GAOL_SOME(A, a, GAOL_ALL(B, b, a < b)) && GAOL_ALL(A, a, GAOL_SOME(B, b, a <= b))
          && GAOL_ALL(B, b, GAOL_SOME(A, a, b <= a));
      case 8:
        return GAOL_SOME(A, a, GAOL_ALL(B, b, a < b)) && GAOL_SOME(A, a, GAOL_ALL(B, b, b < a));
      case 9:
        return GAOL_ALL(A, a, GAOL_SOME(B, b, b <= a)) && GAOL_ALL(B, b, GAOL_SOME(A, a, a <= b))
          && GAOL_SOME(A, a, GAOL_ALL(B, b, b < a));
      case 10:
        return GAOL_SOME(B, b, GAOL_ALL(A, a, b < a)) && GAOL_SOME(A, a, GAOL_ALL(B, b, b < a))
          && GAOL_SOME(B, b, GAOL_SOME(A, a, a < b));
      case 11:
        return GAOL_ALL(B, b, GAOL_ALL(A, a, b <= a)) && GAOL_SOME(B, b, GAOL_SOME(A, a, b == a))
          && GAOL_SOME(B, b, GAOL_ALL(A, a, b < a));
      default:
        return GAOL_ALL(B, b, GAOL_ALL(A, a, b < a));
      }
    }

#undef GAOL_ALL
#undef GAOL_SOME

    const overlapping_state nonempty_states[13] = {
      overlapping_state::before, overlapping_state::meets, overlapping_state::overlaps,
      overlapping_state::starts, overlapping_state::containedBy, overlapping_state::finishes,
      overlapping_state::equals, overlapping_state::finishedBy, overlapping_state::contains,
      overlapping_state::startedBy, overlapping_state::overlappedBy, overlapping_state::metBy,
      overlapping_state::after };

    const char *const state_names[16] = {
      "bothEmpty", "firstEmpty", "secondEmpty", "before", "meets", "overlaps", "starts",
      "containedBy", "finishes", "equals", "finishedBy", "contains", "startedBy",
      "overlappedBy", "metBy", "after" };

    const char *name_of(overlapping_state s)
    {
      return state_names[static_cast<unsigned>(s)];
    }

    // The points of the model that x holds: the numbers c[0] < ... < c[n-1]
    // are the points 4i + 3, the gap below c[i] the points 4i to 4i + 2, and
    // the gap above the last one 4n to 4n + 2
    points model_points(const interval& x, const std::vector<double>& c)
    {
      const int n = static_cast<int>(c.size());
      int first = 0, last = 4 * n + 2;
      if (std::isfinite(x.left())) {
        first = 4 * static_cast<int>(std::find(c.begin(), c.end(), x.left()) - c.begin()) + 3;
      }
      if (std::isfinite(x.right())) {
        last = 4 * static_cast<int>(std::find(c.begin(), c.end(), x.right()) - c.begin()) + 3;
      }
      points p;
      for (int q = first; q <= last; ++q) {
        p.push_back(q);
      }
      return p;
    }

    // The state of x and y by the set specification, and the number of the 13
    // specifications that hold (1 for two nonempty intervals, 0 for an empty
    // one). The set specifications of meets and metBy hold for a point
    // interval on a bound of the other one too, with those of starts,
    // finishes, finishedBy or startedBy: the standard allocates that case to
    // the latter, "never to meets or metBy" (10.6.4), which this does, and
    // point_case tells that no other pair of specifications holds together
    overlapping_state by_sets(const interval& x, const interval& y, int& holding, bool& point_case)
    {
      holding = 0;
      point_case = true;
      if (x.is_empty() || y.is_empty()) {
        return x.is_empty() ? (y.is_empty() ? overlapping_state::bothEmpty : overlapping_state::firstEmpty)
                            : overlapping_state::secondEmpty;
      }
      std::vector<double> c;
      const double ends[4] = { x.left(), x.right(), y.left(), y.right() };
      for (double v : ends) {
        if (std::isfinite(v)) {
          c.push_back(v);
        }
      }
      std::sort(c.begin(), c.end());
      c.erase(std::unique(c.begin(), c.end()), c.end());
      const points A = model_points(x, c), B = model_points(y, c);
      int found[13], count = 0;
      for (int k = 0; k < 13; ++k) {
        if (specification_holds(k, A, B)) {
          found[count++] = k;
        }
      }
      if (count > 1) {
        const bool meets_or_met_by = (found[0] == 1 || found[0] == 11 || found[1] == 1 || found[1] == 11);
        point_case = count == 2 && meets_or_met_by && (x.is_a_double() || y.is_a_double());
        int kept = 0;
        for (int i = 0; i < count; ++i) {
          if (found[i] != 1 && found[i] != 11) {
            found[kept++] = found[i];
          }
        }
        count = kept;
      }
      holding = count;
      return count > 0 ? nonempty_states[found[0]] : overlapping_state::bothEmpty;
    }

    // The state of the pair (y, x): before and after, meets and metBy...
    // are the converse of each other, firstEmpty and secondEmpty too
    overlapping_state converse(overlapping_state s)
    {
      const unsigned k = static_cast<unsigned>(s);
      return static_cast<overlapping_state>(k < 3 ? (k == 0 ? 0u : 3u - k) : 18u - k);
    }

    // The 16-bit patterns of the note of 10.6.4: the state of index k is the
    // bit k from the left, so that disjointMask = 1111000000000001
    unsigned pattern(overlapping_state s)
    {
      return 0x8000u >> static_cast<unsigned>(s);
    }
  }

  void overlap_of_the_standard()
  {
    using namespace overlap_model;
    static_assert(std::is_same<decltype(overlap(std::declval<interval>(), std::declval<interval>())), overlapping_state>::value,
                  "overlap(a, b) is an overlapping_state");
    static_assert(std::is_same<gaol::overlapping_state, gaol_ieee1788::overlapping_state>::value,
                  "gaol and gaol_ieee1788 have the same overlapping_state");
    // The order of Table 10.7, from 0 to 15, which the masks of the note use
    check("the states of overlap have the order of Table 10.7",
          static_cast<int>(overlapping_state::bothEmpty) == 0 && static_cast<int>(overlapping_state::firstEmpty) == 1
          && static_cast<int>(overlapping_state::secondEmpty) == 2 && static_cast<int>(overlapping_state::before) == 3
          && static_cast<int>(overlapping_state::meets) == 4 && static_cast<int>(overlapping_state::overlaps) == 5
          && static_cast<int>(overlapping_state::starts) == 6 && static_cast<int>(overlapping_state::containedBy) == 7
          && static_cast<int>(overlapping_state::finishes) == 8 && static_cast<int>(overlapping_state::equals) == 9
          && static_cast<int>(overlapping_state::finishedBy) == 10 && static_cast<int>(overlapping_state::contains) == 11
          && static_cast<int>(overlapping_state::startedBy) == 12 && static_cast<int>(overlapping_state::overlappedBy) == 13
          && static_cast<int>(overlapping_state::metBy) == 14 && static_cast<int>(overlapping_state::after) == 15);

    // operator<< writes the name of Table 10.7, in the states' order, whatever
    // the locale, and the width of the stream applies to the whole name; a
    // value that is no state is written as such (GAOL v5)
    for (unsigned k = 0; k < 16; ++k) {
      std::ostringstream plain, padded;
      plain << static_cast<overlapping_state>(k);
      padded << std::setw(14) << static_cast<overlapping_state>(k);
      check("operator<< writes the name of an overlapping_state",
            plain.str() == state_names[k] && padded.str() == std::string(14 - plain.str().size(), ' ') + state_names[k],
            [&] { return "state " + std::to_string(k) + ": '" + plain.str() + "', '" + padded.str() + "'"; });
    }
    std::ostringstream beyond;
    beyond << static_cast<overlapping_state>(200) << ' ' << overlap(numsToInterval(1.0, 2.0), numsToInterval(2.0, 3.0));
    check("operator<< of a value that is no overlapping_state, and of a state", beyond.str() == "overlapping_state(200) meets",
          [&] { return beyond.str(); });

    // A pair, against the set specification, its converse, and the
    // comparisons of the standard as masks of states (10.6.4: disjoint is
    // true for bothEmpty, firstEmpty, secondEmpty, before and after)
    const unsigned disjoint_mask = 0xF001u, equal_mask = 0x8040u, subset_mask = 0xC3C0u;
    long pairs = 0;
    const auto check_pair = [&](const interval& x, const interval& y) {
      ++pairs;
      int holding = 0;
      bool point_case = true;
      const overlapping_state expected = by_sets(x, y, holding, point_case);
      const overlapping_state state = overlap(x, y);
      const bool nonempty = !x.is_empty() && !y.is_empty();
      check("overlap is the state of the set specification of Table 10.7", state == expected,
            [&] { return "overlap(" + hex(x) + ", " + hex(y) + ") is " + name_of(state) + ", the specification says " + name_of(expected); });
      check("exactly one specification of Table 10.7 holds for two nonempty intervals, but for meets and metBy with a point",
            holding == (nonempty ? 1 : 0) && point_case,
            [&] { return hex(x) + " " + hex(y) + ": " + std::to_string(holding) + (point_case ? "" : ", two specifications hold"); });
      check("overlap(y, x) is the converse of overlap(x, y)", overlap(y, x) == converse(state),
            [&] { return "overlap(" + hex(y) + ", " + hex(x) + ") is " + name_of(overlap(y, x)) + ", after " + name_of(state); });
      check("disjoint, equal and subset are masks of the states of overlap",
            disjoint(x, y) == ((pattern(state) & disjoint_mask) != 0) && equal(x, y) == ((pattern(state) & equal_mask) != 0)
            && subset(x, y) == ((pattern(state) & subset_mask) != 0),
            [&] { return name_of(state) + std::string(" ") + hex(x) + " " + hex(y); });
    };

    // The grid: every pair of intervals whose bounds are in it, and the empty set
    const double grid[] = { -oo, -std::numeric_limits<double>::max(), -1.0, -0x1p-1022, -0x1p-1074, -0.0, 0.0,
                            0x1p-1074, 0x1p-1073, 0x1p-1022, 1.0, 0x1.0000000000001p+0, 0x1.fffffffffffffp+1023, oo };
    const int n = static_cast<int>(sizeof grid / sizeof grid[0]);
    std::vector<interval> intervals;
    intervals.push_back(empty());
    intervals.push_back(numsToInterval(2.0, 1.0));
    intervals.push_back(intersection(numsToInterval(0.0, 1.0), numsToInterval(2.0, 3.0)));
    for (int i = 0; i < n; ++i) {
      for (int j = 0; j < n; ++j) {
        if (grid[i] <= grid[j] && grid[i] != oo && grid[j] != -oo) {
          intervals.push_back(numsToInterval(grid[i], grid[j]));
        }
      }
    }
    for (const interval& x : intervals) {
      for (const interval& y : intervals) {
        check_pair(x, y);
      }
    }

    // Random pairs, whose bounds are drawn from a few doubles and their
    // neighbours so that the bounds of the two intervals meet or nearly meet
    gaol_tests::Random random;
    for (int trial = 0; trial < 20000; ++trial) {
      double pool[5];
      for (double& p : pool) {
        p = random.any();
      }
      interval drawn[2];
      for (interval& x : drawn) {
        const auto pick = [&]() -> double {
          const double p = pool[random.integer(0, 4)];
          switch (random.integer(0, 11)) {
          case 0: return -oo;
          case 1: return oo;
          case 2: return 0.0;
          case 3: return -0.0;
          case 4: return gaol_tests::next_double(p);
          case 5: return gaol_tests::previous_double(p);
          default: return p;
          }
        };
        double l = pick(), u = pick();
        if (l > u) {
          std::swap(l, u);
        }
        x = (l == oo || u == -oo) ? empty() : numsToInterval(l, u);
      }
      check_pair(drawn[0], drawn[1]);
    }
    std::printf("overlap: %ld pairs checked against the set specification of Table 10.7\n", pairs);
  }

  /* mulRevToPair(b, c), the two-output division of 10.5.5 (GAOL v5): the
     closures of the parts of {x : b'x = c', b' in b, c' in c} in the order of
     the standard, u before v, against exact quotients on structured cases;
     on integer b and c, every sampled x of the set, a dyadic number whose
     products with the bounds of b are exact, in u or v, and the hull of the
     two parts mulRev(b, c). Its first version put the part of the positive
     x first, whatever the sign of c. */
  void mul_rev_to_pair()
  {
    using gaol_tests::quotient;
    using gaol_tests::dyadic;
    const auto pair = [](double bl, double bu, double cl, double cu) {
      return mulRevToPair(numsToInterval(bl, bu), numsToInterval(cl, cu));
    };
    const auto same = [](const interval& x, const interval& y) {
      return (x.is_empty() && y.is_empty()) || x.set_eq(y);
    };
    const auto is = [&](const std::pair<interval, interval>& p, const interval& u, const interval& v) {
      return same(p.first, u) && same(p.second, v);
    };
    const auto show = [](const std::pair<interval, interval>& p) { return hex(p.first) + " " + hex(p.second); };
    const interval none = empty();

    struct { double bl, bu, cl, cu; interval u, v; } const exact_parts[] = {
      { -1.0, 1.0, 1.0, 2.0, numsToInterval(-oo, -1.0), numsToInterval(1.0, oo) },
      { -1.0, 1.0, -2.0, -1.0, numsToInterval(-oo, -1.0), numsToInterval(1.0, oo) },
      { -1.0, 1.0, 1.0, oo, numsToInterval(-oo, -1.0), numsToInterval(1.0, oo) },
      { -oo, oo, 1.0, 2.0, numsToInterval(-oo, 0.0), numsToInterval(0.0, oo) },
      { 0.0, 4.0, 1.0, 2.0, numsToInterval(0.25, oo), none },
      { -2.0, 0.0, 1.0, 2.0, numsToInterval(-oo, -0.5), none },
      { 0.0, 0.0, 1.0, 2.0, none, none },
      { -1.0, 1.0, 0.0, 1.0, entire(), none },
      { -1.0, 1.0, 0.0, 0.0, entire(), none },
      { 1.0, 2.0, 0.0, 0.0, numsToInterval(0.0, 0.0), none },
      { 1.0, 2.0, -2.0, 4.0, numsToInterval(-2.0, 4.0), none },
      { -oo, -1.0, 1.0, 2.0, numsToInterval(-2.0, 0.0), none },
    };
    for (const auto& e : exact_parts) {
      const std::pair<interval, interval> p = pair(e.bl, e.bu, e.cl, e.cu);
      check("mulRevToPair(b, c): the parts of the standard, in its order", is(p, e.u, e.v),
            [&] { return hex(numsToInterval(e.bl, e.bu)) + " " + hex(numsToInterval(e.cl, e.cu)) + ": " + show(p); });
    }
    const std::pair<interval, interval> thirds = pair(-3.0, 3.0, 1.0, 2.0), third = pair(1.0, 3.0, 1.0, 2.0);
    check("mulRevToPair([-3, 3], [1, 2]): [-oo, -1/3] and [1/3, +oo], the tightest",
          thirds.first.left() == -oo && gaol_tests::is_tightest_upper_bound(thirds.first.right(), quotient(dyadic(-1.0), dyadic(3.0)))
          && gaol_tests::is_tightest_lower_bound(thirds.second.left(), quotient(dyadic(1.0), dyadic(3.0)))
          && thirds.second.right() == oo, [&] { return show(thirds); });
    check("mulRevToPair([1, 3], [1, 2]): [1/3, 2], the tightest",
          gaol_tests::is_tightest_lower_bound(third.first.left(), quotient(dyadic(1.0), dyadic(3.0)))
          && third.first.right() == 2.0 && third.second.is_empty(), [&] { return show(third); });
    check("mulRevToPair() of the empty set",
          is(mulRevToPair(empty(), numsToInterval(1.0, 2.0)), none, none)
          && is(mulRevToPair(numsToInterval(1.0, 2.0), empty()), none, none));

    // On integer intervals: the x of the set, sampled, and the hull
    for (int bl = -3; bl <= 3; ++bl) {
      for (int bu = bl; bu <= 3; ++bu) {
        for (int cl = -3; cl <= 3; ++cl) {
          for (int cu = cl; cu <= 3; ++cu) {
            const interval b = numsToInterval(bl, bu), c = numsToInterval(cl, cu);
            const std::pair<interval, interval> p = mulRevToPair(b, c);
            const auto describe = [&] { return hex(b) + " " + hex(c) + ": " + show(p); };
            bool kept = true;
            for (int k = -48; k <= 48; ++k) {
              // b'x for b' in b is the interval between bl x and bu x
              const double x = k/4.0, low = std::min(bl*x, bu*x), high = std::max(bl*x, bu*x);
              if (std::max(low, static_cast<double>(cl)) <= std::min(high, static_cast<double>(cu))) {
                kept = kept && (p.first.set_contains(x) || p.second.set_contains(x));
              }
            }
            check("mulRevToPair(b, c): every x of the set in its parts", kept, describe);
            check("mulRevToPair(b, c): u before v, a second part only after a first",
                  p.second.is_empty() || (!p.first.is_empty() && p.first.right() <= p.second.left()), describe);
            check("mulRevToPair(b, c): the hull of the parts mulRev(b, c)",
                  same(convexHull(p.first, p.second), mulRev(b, c)), describe);
          }
        }
      }
    }
  }

  /* textToInterval reads the names of the standard (GAOL v5): each name of
     Tables 9.1 and 10.5 gives the function of the same name, pow the pow of
     Table 9.1, and a name of GAOL alone is no function, which gives the
     empty set; gaol::textToInterval reads the names of GAOL. textToInterval
     read those of GAOL, and pow([-4,-1],2) was [1, 16], where
     pow([-4,-1], 2) is the empty set. */
  void text_with_the_names_of_the_standard()
  {
    const interval x = numsToInterval(0.25, 0.5), y = numsToInterval(-1.0, 2.0),
      p = numsToInterval(2.0, 5.0);
    struct Case { const char *text; interval value; };
    const Case cases[] = {
      {"pown([2,5],5)", pown(p, 5)}, {"pow([2,5],5)", pow(p, 5.0)},
      {"pow([-4,-1],2)", pow(numsToInterval(-4.0, -1.0), 2.0)},
      {"rootn([-8,27],3)", rootn(numsToInterval(-8.0, 27.0), 3)},
      {"rootn([0.25,0.5],-3)", rootn(x, -3)},
      {"neg([0.25,0.5])", neg(x)}, {"add([0.25,0.5],[-1,2])", add(x, y)},
      {"sub([0.25,0.5],[-1,2])", sub(x, y)}, {"mul([0.25,0.5],[-1,2])", mul(x, y)},
      {"div([0.25,0.5],[-1,2])", div(x, y)}, {"recip([0.25,0.5])", recip(x)},
      {"sqr([-1,2])", sqr(y)}, {"sqrt([0.25,0.5])", sqrt(x)},
      {"fma([0.25,0.5],[-1,2],[0.25,0.5])", fma(x, y, x)},
      {"exp([0.25,0.5])", exp(x)}, {"exp2([0.25,0.5])", exp2(x)}, {"exp10([0.25,0.5])", exp10(x)},
      {"log([0.25,0.5])", log(x)}, {"log2([0.25,0.5])", log2(x)}, {"log10([0.25,0.5])", log10(x)},
      {"sin([0.25,0.5])", sin(x)}, {"cos([0.25,0.5])", cos(x)}, {"tan([0.25,0.5])", tan(x)},
      {"asin([0.25,0.5])", asin(x)}, {"acos([0.25,0.5])", acos(x)}, {"atan([0.25,0.5])", atan(x)},
      {"atan2([-1,2],[0.25,0.5])", atan2(y, x)},
      {"sinh([0.25,0.5])", sinh(x)}, {"cosh([0.25,0.5])", cosh(x)}, {"tanh([0.25,0.5])", tanh(x)},
      {"asinh([0.25,0.5])", asinh(x)}, {"acosh([-1,2])", acosh(y)}, {"atanh([0.25,0.5])", atanh(x)},
      {"sign([-1,2])", sign(y)}, {"ceil([-1,2])", ceil(y)}, {"floor([-1,2])", floor(y)},
      {"trunc([-1,2])", trunc(y)}, {"roundTiesToEven([0.5,2.5])", roundTiesToEven(numsToInterval(0.5, 2.5))},
      {"roundTiesToAway([0.5,2.5])", roundTiesToAway(numsToInterval(0.5, 2.5))},
      {"abs([-1,2])", abs(y)}, {"min([0.25,0.5],[-1,2])", min(x, y)}, {"max([0.25,0.5],[-1,2])", max(x, y)},
      {"expm1([0.25,0.5])", expm1(x)}, {"exp2m1([0.25,0.5])", exp2m1(x)},
      {"exp10m1([0.25,0.5])", exp10m1(x)}, {"logp1([0.25,0.5])", logp1(x)},
      {"log2p1([0.25,0.5])", log2p1(x)}, {"log10p1([0.25,0.5])", log10p1(x)},
      {"hypot([0.25,0.5],[-1,2])", hypot(x, y)}, {"compoundm1([0.25,0.5],[-1,2])", compoundm1(x, y)},
      {"rSqrt([0.25,0.5])", rSqrt(x)},
      {"sinPi([0.25,0.5])", sinPi(x)}, {"cosPi([0.25,0.5])", cosPi(x)}, {"tanPi([0.25,0.5])", tanPi(x)},
      {"asinPi([0.25,0.5])", asinPi(x)}, {"acosPi([0.25,0.5])", acosPi(x)},
      {"atanPi([0.25,0.5])", atanPi(x)}, {"atan2Pi([-1,2],[0.25,0.5])", atan2Pi(y, x)},
      {"SINPI([0.25,0.5])", sinPi(x)}, {"[pown(2,3), rootn(27,3)*3]", numsToInterval(8.0, 9.0)},
    };
    for (const Case& c : cases) {
      const interval got = textToInterval(c.text);
      check("textToInterval reads the names of IEEE 1788-2015", got.set_eq(c.value),
            [&] { return std::string(c.text) + ": " + hex(got) + " rather than " + hex(c.value); });
    }
    // The names of GAOL alone, and the calls that are wrong: the empty set,
    // with exceptions or without them (configure --disable-exceptions, meson
    // -Denable-exception=false), where the error of a wrong call, as
    // pown([2,5],2.5), ended the program (GAOL v5, #98)
    const char *const not_the_standard[] = {
      "nth_root(8,3)", "cbrt(8)", "inverse(2)", "integer([1.5,3])", "log1p(0)",
      "round_ties_to_even(1)", "pown([2,5],2.5)", "sin(1,2)", "fma(1,2)",
    };
    for (const char *t : not_the_standard) {
      const interval got = textToInterval(t);
      check("textToInterval gives the empty set for a name of GAOL alone or a wrong call",
            got.is_empty(), [&] { return std::string(t) + ": " + hex(got); });
    }
    check("gaol::textToInterval reads the names of GAOL: pow([-4,-1],2) is [1, 16]",
          gaol::textToInterval("pow([-4,-1],2)").set_eq(numsToInterval(1.0, 16.0)),
          [] { return hex(gaol::textToInterval("pow([-4,-1],2)")); });
  }

  void exact_text()
  {
    const interval third(1.0 / 3.0, 2.0 / 3.0);
    const gaol::interval_format::format_t saved = interval::format();
    interval::format(gaol::interval_format::bounds);
    const std::string exact = intervalToExact(third);
    check("intervalToExact does not change the output format",
          interval::format() == gaol::interval_format::bounds, [] { return std::string(); });
    check("intervalToExact(x) = exact_string(x), read back bit for bit",
          exact == gaol::exact_string(third) && exactToInterval(exact).set_eq(third), [&] { return exact; });
    interval::format(gaol::interval_format::hexa);
    std::ostringstream s;
    s << third << ' ' << interval::emptyset();
    check("operator<< in interval_format::hexa writes exact_string",
          s.str() == gaol::exact_string(third) + " [empty]", [&] { return s.str(); });
    interval::format(saved);
  }

  /*
    The text of a point interval is read back (GAOL v5): intervalToText(
    interval(0.1)) was <0.1, 0.1000000000000001>, whose two numbers are not the
    same double, which the reader takes for a point only, so that
    textToInterval() gave the empty set, where the recovery requirement of
    IEEE 1788-2015 (13.4) asks for an interval containing the one written. A
    point that the digits write exactly is written [a], the literal of the
    standard for a point, and read back as the point itself.
  */
  void text_of_a_point_interval()
  {
    const gaol::interval_format::format_t saved = interval::format();
    interval::format(gaol::interval_format::bounds);
    const interval tenth(0.1);
    const interval back = textToInterval(intervalToText(tenth));
    check("textToInterval(intervalToText(interval(0.1))) contains 0.1", back.set_contains(0.1),
          [&] { return intervalToText(tenth) + " read " + hex(back); });
    const double points[] = { 0.1, -0.1, 1.0 / 3.0, 3.14159265358979, 1e23, 4.0, 0.5, 0.0, -0.0,
                              std::numeric_limits<double>::denorm_min(), -std::numeric_limits<double>::max() };
    for (double x : points) {
      const std::string text = intervalToText(interval(x));
      const interval y = textToInterval(text);
      check("textToInterval(intervalToText(point)) contains the point", y.set_contains(x),
            [&] { return text + " read " + hex(y); });
      if (text.find(',') == std::string::npos) {
        check("textToInterval(intervalToText(point)) written [a]: the point itself", y.left() == x && y.right() == x,
              [&] { return text + " read " + hex(y); });
      }
    }
    interval::format(saved);
  }

  /*
    Whether s is an interval literal of the standard that intervalToText()
    writes: [ ], [empty], [m] or [l, u], m, l and u being decimal numbers, or
    inf, with a sign, as in Tables 9.5 and 12.2 (12.11.5), whose letters may be
    of either case. The angles <a, a> of GAOL, a width "c (+/- w)", the
    agreeing digits and a decimal comma are none.
  */
  bool is_portable_literal(const std::string& text)
  {
    std::string s;
    for (char c : text) {
      s += static_cast<char>(std::tolower(static_cast<unsigned char>(c)));
    }
    std::size_t i = 0;
    const auto spaces = [&] { while (i < s.size() && (s[i] == ' ' || s[i] == '\t')) { ++i; } };
    const auto digits = [&] { const std::size_t from = i; while (i < s.size() && std::isdigit(static_cast<unsigned char>(s[i]))) { ++i; } return i - from; };
    const auto expect = [&](char c) { if (i < s.size() && s[i] == c) { ++i; return true; } return false; };
    const auto number = [&] {
      if (i < s.size() && (s[i] == '+' || s[i] == '-')) {
        ++i;
      }
      if (s.compare(i, 3, "inf") == 0) {
        i += 3;
        return true;
      }
      const std::size_t before = digits();
      std::size_t after = 0;
      if (expect('.')) {
        after = digits();
      }
      if (before + after == 0) {
        return false;
      }
      if (expect('e')) {
        if (i < s.size() && (s[i] == '+' || s[i] == '-')) {
          ++i;
        }
        return digits() > 0;
      }
      return true;
    };
    if (!expect('[')) {
      return false;
    }
    spaces();
    if (expect(']')) {
      return i == s.size();
    }
    if (s.compare(i, 5, "empty") == 0) {
      i += 5;
      spaces();
      return expect(']') && i == s.size();
    }
    if (!number()) {
      return false;
    }
    spaces();
    if (expect(',')) {
      spaces();
      if (!number()) {
        return false;
      }
      spaces();
    }
    return expect(']') && i == s.size();
  }

  /*
    intervalToText(x) is an interval literal that textToInterval() reads back
    as an interval containing x, [l, r], [a] for a point that the digits write
    exactly, and [empty] for the empty set, whatever the global output format
    and the locale of the program (GAOL v5), as the standard asks of it
    (13.3), with the digits of interval::precision(): it wrote what operator<<
    does, the width "1.5 (+/- 0.5)", the agreeing digits, a decimal comma under
    the locale of a program that sets one, and <4, 4> for the point interval
    4, which is no literal of the standard. The texts written by hand are
    those of 16 digits and of 1 digit; with the other precisions, it has to be
    the text operator<< writes in the bounds format. Under a locale writing a
    decimal comma, operator<< writes the bounds format with a decimal point
    too, and the text of a point has to be read back as the point, or as an
    interval containing it when it has two bounds (GAOL v5: the literal [a]
    made interval(-2.5) [-2,5], read as the interval [-2, 5], and GAOL wrote
    [-2,5, -2,5], which the reader refused). Where no such locale is
    installed, that part is not checked.
  */
  void text_independent_of_the_output_settings()
  {
    const gaol::interval_format::format_t saved_format = interval::format();
    const std::streamsize saved_precision = interval::precision();
    const double smallest = std::numeric_limits<double>::denorm_min(), largest = std::numeric_limits<double>::max();
    // The texts to expect, written by hand, with 16 digits and with 1
    const struct { const char *name; interval x; const char *text; const char *text1; } forms[] = {
      { "[1, 2]", interval(1.0, 2.0), "[1, 2]", "[1, 2]" },
      { "interval(4)", interval(4.0), "[4]", "[4]" },
      { "interval(0.5)", interval(0.5), "[0.5]", "[0.5]" },
      { "interval(-1024)", interval(-1024.0), "[-1024]", "[-2e+03, -1e+03]" },
      { "interval(0.1)", interval(0.1), "[0.1, 0.1000000000000001]", "[0.1, 0.2]" },
      { "[-0, 0]", interval(-0.0, 0.0), "[0]", "[0]" },
      { "interval(-0.0)", interval(-0.0), "[0]", "[0]" },
      { "interval::zero()", interval::zero(), "[0]", "[0]" },
      { "the double nearest 1/3", interval(0x1.5555555555555p-2), "[0.3333333333333333, 0.3333333333333334]",
        "[0.3, 0.4]" },
      { "[1, +oo]", interval(1.0, oo), "[1, inf]", "[1, inf]" },
      { "[-oo, -1.5]", interval(-oo, -1.5), "[-inf, -1.5]", "[-inf, -1]" },
      { "the universe", interval::universe(), "[-inf, inf]", "[-inf, inf]" },
      { "the empty set", interval::emptyset(), "[empty]", "[empty]" },
      { "[5e-324, 1e-323]", interval(smallest, 2.0*smallest), "[4.940656458412465e-324, 9.881312916824931e-324]",
        "[4e-324, 1e-323]" },
      { "[-MAX, MAX]", interval(-largest, largest), "[-1.797693134862316e+308, 1.797693134862316e+308]",
        "[-2e+308, 2e+308]" },
    };
    const interval others[] = { interval(0.1, 0.3), interval::pi(), interval(1.25, 1.2567), interval(-2.0/3.0, 1e23),
                                interval(1e-5, 2e-5) };

    const auto expect_text = [&](const interval& x, const std::string& expected, const std::string& situation) {
      const std::string text = intervalToText(x);
      const auto describe = [&] { return hex(x) + " " + situation + ": \"" + text + "\""; };
      if (!expected.empty()) {
        check("intervalToText: the text of the manual, whatever the format and the locale", text == expected,
              [&] { return describe() + " rather than \"" + expected + "\""; });
      }
      check("intervalToText: an interval literal of IEEE 1788-2015", is_portable_literal(text), describe);
      const interval back = textToInterval(text);
      check("intervalToText: read back as an interval containing x",
            x.is_empty() ? back.is_empty() : back.set_contains(x), [&] { return describe() + " read " + hex(back); });
    };

    const std::streamsize precisions[] = { 1, 3, 8, 16, 17, 30 };
    const std::size_t nb_others = sizeof others / sizeof others[0];
    // The text operator<< writes in the bounds format, in a stream of the C
    // locale and its first flags, with each precision: what intervalToText()
    // has to give with that precision whatever the other settings
    std::map<std::streamsize, std::vector<std::string> > reference;
    interval::format(gaol::interval_format::bounds);
    for (std::streamsize precision : precisions) {
      interval::precision(precision);
      for (std::size_t i = 0; i < nb_others; ++i) {
        std::ostringstream os;
        os.imbue(std::locale::classic());
        os << others[i];
        reference[precision].push_back(os.str());
      }
      for (const auto& form : forms) {
        std::ostringstream os;
        os.imbue(std::locale::classic());
        os << form.x;
        reference[precision].push_back(os.str());
      }
    }

    const struct { const char *name; gaol::interval_format::format_t format; } formats[] = {
      { "bounds", gaol::interval_format::bounds }, { "width", gaol::interval_format::width },
      { "center", gaol::interval_format::center }, { "hexa", gaol::interval_format::hexa },
      { "agreeing", gaol::interval_format::agreeing },
    };
    const auto expect_all = [&](const std::string& where) {
      for (const auto& f : formats) {
        for (std::streamsize precision : precisions) {
          interval::format(f.format);
          interval::precision(precision);
          const std::string situation = "in the " + std::string(f.name) + " format with the precision "
                                        + std::to_string(precision) + where;
          std::size_t k = 0;
          for (std::size_t i = 0; i < nb_others; ++i) {
            expect_text(others[i], reference[precision][k++], situation);
          }
          for (const auto& form : forms) {
            expect_text(form.x, (precision == 16) ? form.text : (precision == 1) ? form.text1 : "", situation);
            expect_text(form.x, reference[precision][k++], situation);
          }
          check("intervalToText leaves the global format as it was", interval::format() == f.format, [&] { return situation; });
          check("intervalToText leaves the global precision as it was", interval::precision() == precision, [&] { return situation; });
        }
      }
    };
    expect_all("");

    // Under a locale writing a decimal comma, which a program sets for all its
    // streams with std::locale::global()
    const std::locale saved_locale;
    const std::string saved_c_locale = std::setlocale(LC_ALL, nullptr);
    bool commas = false;
    for (const char *name : { "fr_FR.UTF-8", "fr_FR.utf8", "de_DE.UTF-8", "de_DE.utf8", "French_France.1252", "fr_FR" }) {
      try {
        std::locale::global(std::locale(name));
      } catch (const std::runtime_error&) {
        continue;
      }
      std::ostringstream probe;
      probe << 1.5;
      if (probe.str() == "1,5") {
        commas = true;
        break;
      }
    }
    if (commas) {
      expect_all(" under a locale writing a decimal comma");
      std::ostringstream os;
      interval::format(gaol::interval_format::bounds);
      interval::precision(16);
      os << interval(0.25, 0.5);
      check("operator<< writes the bounds format with a decimal point under that locale, the text of intervalToText",
            os.str() == "[0.25, 0.5]" && intervalToText(interval(0.25, 0.5)) == "[0.25, 0.5]",
            [&] { return os.str() + " and " + intervalToText(interval(0.25, 0.5)); });
      // A point written by operator<< under that locale: the literal [a] of a
      // number with a decimal comma would be two numbers, which the reader
      // takes, [-2,5] being [-2, 5], [12,5] the empty set and [0,] (no digit,
      // showpoint) [0, +oo]; the text has to be read back as an interval
      // containing the point, and as the point itself when it is one number
      const struct { const char *name; std::ios_base::fmtflags flags; std::streamsize precision; } settings[] = {
        { "with 16 digits", std::ios_base::fmtflags(), 16 },
        { "in the fixed format with the showpoint flag and no digit", std::ios_base::fixed | std::ios_base::showpoint, 0 },
        { "in the fixed format with 1074 digits", std::ios_base::fixed, 1074 },
      };
      for (const auto& setting : settings) {
        interval::precision(setting.precision);
        for (double x : { -2.5, -0.5, 12.5, 0.5, 4.0, 0.0, -smallest }) {
          std::ostringstream point;
          point.setf(setting.flags);
          point << interval(x);
          const std::string text = point.str();
          bool refused = false;
          interval back;
#if GAOL_EXCEPTIONS_ENABLED
          try {
            back = gaol::textToInterval(text);
          } catch (const std::invalid_argument&) {
            refused = true;
          }
#else
          // Without exceptions, a text the reader refuses ends the program
          back = gaol::textToInterval(text);
#endif
          const bool one_number = (text.find(", ") == std::string::npos);
          check("operator<< of a point under that locale: read back as an interval containing it, [a] as the point",
                !refused && back.set_contains(x) && (!one_number || (back.left() == x && back.right() == x)),
                [&] { return text.substr(0, 60) + " " + setting.name + ", read " + hex(back); });
        }
      }
      interval::precision(16);
      std::ostringstream four;
      four << interval(4.0);
      check("operator<< of a point without a comma under that locale: [a]", four.str() == "[4]",
            [&] { return four.str(); });
    } else {
      std::printf("No locale writing a decimal comma: intervalToText under such a locale is not checked\n");
    }
    std::locale::global(saved_locale);
    std::setlocale(LC_ALL, saved_c_locale.c_str());
    interval::precision(saved_precision);
    interval::format(saved_format);
  }

// Commented out: the tests run no thread (GAOL v5)
// #if GAOL_TESTS_THREADS
//   /*
//     intervalToExact() in one thread while another writes intervals in
//     interval_format::bounds: the other one has to write them in that format.
//     intervalToExact() switched the global output format to hexa and back,
//     which the other threads saw meanwhile.
//   */
//   void exact_text_in_another_thread()
//   {
//     const interval third(1.0 / 3.0, 2.0 / 3.0);
//     const gaol::interval_format::format_t saved = interval::format();
//     interval::format(gaol::interval_format::bounds);
//     std::atomic<bool> done(false);
//     std::thread exact_writer([&] {
//       for (int i = 0; i < 20000; ++i) {
//         (void)intervalToExact(third);
//       }
//       done = true;
//     });
//     long written = 0, in_hexa = 0;
//     std::string seen;
//     while (!done) {
//       std::ostringstream s;
//       s << third;
//       ++written;
//       if (s.str().find("0x") != std::string::npos) {
//         ++in_hexa;
//         seen = s.str();
//       }
//     }
//     exact_writer.join();
//     check("intervalToExact in one thread leaves the output format of the others", in_hexa == 0,
//           [&] { return std::to_string(in_hexa) + " of " + std::to_string(written) + " intervals written in hexa, " + seen; });
//     interval::format(saved);
//   }
// #endif
}

int main()
{
  gaol::init();
  pow_of_the_standard();
  integers();
#if defined(__SIZEOF_INT128__)
  exponents_beyond_64_bits<__int128>();
#endif
  pow_on_boxes();
  gaol_functions();
  names_of_the_standard();
  overlap_of_the_standard();
  mul_rev_to_pair();
  text_with_the_names_of_the_standard();
  exact_text();
  text_of_a_point_interval();
  text_independent_of_the_output_settings();
// Commented out: the tests run no thread (GAOL v5)
// #if GAOL_TESTS_THREADS
//   exact_text_in_another_thread();
// #endif
  const int status = gaol_tests::summary();
  gaol::cleanup();
  return status;
}
