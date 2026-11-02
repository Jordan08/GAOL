/*-*-C++-*------------------------------------------------------------------
 * gaol -- NOT Just Another Interval Library
 *--------------------------------------------------------------------------
 * Tests of GAOL v5: the unit tests of IBEX and of Codac on their intervals.
 *
 * IBEX (https://github.com/ibex-team/ibex-lib) and Codac
 * (https://github.com/codac-team/codac) compute with intervals that are GAOL's
 * (ibex::Interval and codac2::Interval hold a gaol::interval). The unit tests
 * they run on a scalar interval and on interval arithmetic are run here against
 * GAOL v5 itself, in two sections, each with the list of what it leaves out:
 *   - IBEX (ibex-lib commit 65ed587): tests/TestInterval.cpp and
 *     tests/TestArith.cpp, with the helpers of tests/utils.cpp;
 *   - Codac (Codac 2 commit 6419940, and the branch GAOL of the fork of Jordan
 *     Ninin, commit f0d06ff0): tests/core/domains/interval/codac2_tests_
 *     Interval.cpp, codac2_tests_Interval_operations.cpp and
 *     codac2_tests_Interval_rounding.cpp.
 * The bodies of the tests are those of the projects, copied by a script from
 * their sources with the few textual substitutions each section lists; the
 * expected values and the tolerances are theirs (IBEX compares to 1e-10, Codac
 * with its Approx), which is looser than the tests of GAOL, that compare the
 * bounds with the tightest ones: these tests check that GAOL gives the results
 * its two main users expect from it. Each section writes the environment the
 * tests need (CppUnit's assertions, Catch2's TEST_CASE and CHECK, the classes
 * Interval of IBEX and of Codac) once, as a thin layer that calls the function
 * of GAOL the wrapper of the project calls. What exists only in IBEX or in
 * Codac (their set difference, their distances, their vectors, their
 * algorithms of backward contraction that GAOL has no equivalent of, their
 * Python bindings...) is not ported.
 *
 * The code of IBEX and of Codac is under the GNU Lesser General Public License,
 * as GAOL is; the copyrights of the sections are kept.
 *
 * Copyright (c) 2026 ENSTA, France
 *
 * Created 2026-11-02 by Jordan NININ
 *--------------------------------------------------------------------------
 * gaol is a software distributed WITHOUT ANY WARRANTY. Read the associated
 * COPYING file for information.
 *--------------------------------------------------------------------------*/

#include "gaol_tests.h"

/*-*-C++-*------------------------------------------------------------------
 * gaol -- NOT Just Another Interval Library
 *--------------------------------------------------------------------------
 * The tests IBEX runs on its scalar intervals (ibex-lib, commit 65ed587,
 * tests/TestInterval.cpp and tests/TestArith.cpp, with the helpers check(),
 * almost_eq() of tests/utils.cpp), run against GAOL.
 *
 * The bodies of the tests and of their helpers are those of IBEX, extracted
 * from its sources by brace matching, in the order of the sources, with no
 * change but these substitutions:
 *   TestInterval:: and TestArith:: -> (removed): the qualifier of a member of
 *       the class: the functions are in the namespace of the same name
 *   ERROR -> ERROR_TOL: ERROR is a macro of <windows.h>
 *   M_PI -> M_PI_VALUE: not every C++ library defines M_PI in strict mode: a
 *       constant, of the value TestArith.cpp gives it
 *   ::exp( and ::log( -> std::exp( and std::log(: the functions of the C++
 *       standard library (a global ::exp is not guaranteed)
 *   else; -> else {}: an empty else; is a warning of GCC and Clang
 *       (-Wempty-body of -Wextra); in almost_eq() of utils.cpp
 *
 * The tolerance is that of IBEX, ERROR = 1e-10, absolute. What is not IBEX's
 * own text is the environment the tests run in, written once below:
 * CppUnit's assertions, that report through gaol_tests::check(), and
 * ibex::Interval, a class that holds a gaol::interval and calls GAOL for each
 * operation as the wrapper of IBEX for GAOL does (interval_lib_wrapper/gaol),
 * with the functions that IBEX writes itself and GAOL has an equivalent of
 * mapped on it: the set relations (set_eq, set_leq, set_le...), is_bisectable,
 * hausdorff, atan2, the reverse functions max_rel, min_rel, atan2_rel_y,
 * atan2_rel_x, floor_rel, nth_root_rel (for a positive exponent), and the
 * generalized division, mulRevToPair. Tests of IBEX not ported, and why:
 *   TestInterval consCppArray: IBEX only: Interval{1.0, 2.0}, the construction
 *       from a C++ brace list (std::array), no GAOL counterpart
 *   TestInterval set_empty01-02: IBEX only: Interval::set_empty(), which is
 *       *this = EMPTY_SET; GAOL has no mutator
 *   TestInterval delta01-03: IBEX only: Interval::delta(), a measure of IBEX
 *       (volume lost by a contraction), no GAOL counterpart
 *   TestInterval diff01-17: IBEX only: Interval::diff(), the set difference
 *       into two intervals with its own algorithm; GAOL has no counterpart
 *   TestInterval rel_distance01-21: IBEX only: Interval::rel_distance() =
 *       distance/diam with its own conventions for the infinite and empty
 *       cases
 *   TestInterval distance20-21: IBEX only: the distance to the empty set is
 *       the radius of the other interval (convention of IBEX);
 *       gaol::hausdorff() with the empty set is a NaN
 *   TestArith bwd_div01-09: IBEX algorithm: bwd_div() is two steps of its own
 *       on top of * and bwd_mul(); GAOL has no reverse of the division
 *   TestArith bwd_sqrt01-07: IBEX algorithm: bwd_sqrt() clips y to [0,+oo] by
 *       cases, then x &= sqr(y); GAOL has no reverse of sqrt
 *   TestArith bwd_atan01-08: IBEX algorithm: bwd_atan() treats the poles of
 *       tan by cases (hand-made tan of the bounds); GAOL has no reverse of
 *       atan (atan_rel() is the reverse of tan)
 *   TestArith bwd_add01-05: IBEX algorithm: bwd_add() is x1 &= y-x2 then x2 &=
 *       y-x1 with early exits; GAOL has no reverse of the addition
 *       (cancel_plus() is another operation)
 *   TestArith bwd_sub01-05: IBEX algorithm: bwd_sub(), as bwd_add()
 *   TestArith bwd_imod_01-08: IBEX only: bwd_imod(), the modulo by a period,
 *       with its own algorithm; GAOL has no counterpart
 *   TestArith bwd_ceil_01-13: IBEX algorithm: bwd_ceil(), by cases on
 *       std::ceil/std::floor of the bounds; GAOL has floor_rel() but no
 *       ceil_rel()
 *   TestArith mulMM01: matrix test (Matrix of IBEX), no scalar interval
 *   TestArith bwd_mulVV01: vector test (IntervalVector of IBEX)
 *   TestArith bwd_mulMV01: matrix/vector test (IntervalMatrix, IntervalVector,
 *       Matrix and Vector of IBEX)
 *   TestArith bwd_floor_12-13: empty body: the only statement is a comment
 *   TestArith float10: registered in a comment of TestArith.h ("disabled
 *       temporarily due to bug between Gaol and elibc"): never runs in IBEX
 *   TestArith sqrt06: defined in TestArith.cpp, not registered in TestArith.h:
 *       never runs in IBEX
 *   TestArith tan06: defined in TestArith.cpp, not registered: never runs in
 *       IBEX (its expectation is wrong)
 *   TestArith tan07: defined in TestArith.cpp, not registered: never runs in
 *       IBEX
 *
 * Tests ported: TestInterval 93 of 139; TestArith 283 of 343.
 *
 * IBEX: Copyright Ecole des Mines de Nantes / IMT Atlantique (FRANCE),
 * Author Gilles Chabert, distributed under the GNU LGPL (COPYING.LESSER).
 *
 * Needs gaol_tests.h (gaol_tests::check(), hex(), RoundingToNearest), which
 * the file including this section includes first.
 *--------------------------------------------------------------------------*/

#include <cfloat>
#include <cmath>
#include <cstddef>
#include <exception>
#include <limits>
#include <string>
#include <utility>
#include <vector>

#include <gaol/gaol>

// -------------------------------------------------------------------------
// The environment of the tests of IBEX: CppUnit's assertions, which report
// through gaol_tests::check(), and the name of the test being run.
// -------------------------------------------------------------------------

#define CPPUNIT_ASSERT(condition) \
  ((void)::ibex_port::assertion(static_cast<bool>(condition), "CPPUNIT_ASSERT(" #condition ")", __LINE__))
#define CPPUNIT_ASSERT_DOUBLES_EQUAL(expected, actual, delta) \
  ((void)::ibex_port::assert_doubles_equal((expected), (actual), (delta), \
    "CPPUNIT_ASSERT_DOUBLES_EQUAL(" #expected ", " #actual ", " #delta ")", __LINE__))
#define CPPUNIT_ASSERT_DOUBLES_EQUAL_MESSAGE(message, expected, actual, delta) \
  ((void)::ibex_port::assert_doubles_equal((expected), (actual), (delta), \
    "CPPUNIT_ASSERT_DOUBLES_EQUAL_MESSAGE(" #message ", " #expected ", " #actual ", " #delta ")", __LINE__))

namespace ibex_port
{
  // "IBEX TestArith::add01", set by run() before it calls a test
  inline std::string& current_test()
  {
    static std::string name;
    return name;
  }

  // CPPUNIT_ASSERT(condition)
  inline bool assertion(bool ok, const char* expression, int line)
  {
    return gaol_tests::check(current_test(), ok,
                             [&] { return std::string(expression) + ", line " + std::to_string(line); });
  }

  // CPPUNIT_ASSERT_DOUBLES_EQUAL(expected, actual, delta): the doubles are equal
  // when they are the same number, infinities of the same sign included, or less
  // than delta apart. The test fabs(expected - actual) > delta of the first
  // CppUnit is false for a NaN and for inf - inf: a NaN is never equal here
  // (the tests never expect one)
  inline bool assert_doubles_equal(double expected, double actual, double delta, const char* expression, int line)
  {
    const bool ok = (expected == actual) || (std::fabs(expected - actual) <= delta);
    return gaol_tests::check(current_test(), ok, [&] {
      return std::string(expression) + ", line " + std::to_string(line) + ": expected " + gaol_tests::hex(expected)
             + ", actual " + gaol_tests::hex(actual) + ", delta " + gaol_tests::hex(delta);
    });
  }

  // -----------------------------------------------------------------------
  // namespace ibex: the intervals of IBEX (src/arithmetic/ibex_Interval.h and
  // interval_lib_wrapper/gaol/ibex_IntervalLibWrapper.inl) on GAOL
  // -----------------------------------------------------------------------

  namespace ibex
  {
    // IBEX_INTERVAL_LIB_POS_INFINITY and IBEX_INTERVAL_LIB_NEG_INFINITY of the wrapper
    const double POS_INFINITY = std::numeric_limits<double>::infinity();
    const double NEG_INFINITY = -std::numeric_limits<double>::infinity();

    // wrapper: previous_float() and next_float(). The wrapper sets the rounding
    // to nearest around the call on 64-bit ARM, where nextafter() crashes
    // otherwise on macOS
    inline double previous_float(double x)
    {
#if defined(__arm64__) || defined(__aarch64__)
      const gaol_tests::RoundingToNearest nearest;
#endif
      return gaol::previous_float(x);
    }

    inline double next_float(double x)
    {
#if defined(__arm64__) || defined(__aarch64__)
      const gaol_tests::RoundingToNearest nearest;
#endif
      return gaol::next_float(x);
    }

    class Interval
    {
      public:

        // ibex_Interval.h: Interval() is (-oo,+oo), Interval(a,b) is the empty
        // set for a == +oo, b == -oo or a > b, and Interval(a) for a == -oo or +oo
        Interval()
          : itv(NEG_INFINITY, POS_INFINITY)
        {
        }

        Interval(double a, double b)
          : itv(a, b)
        {
          if (a == POS_INFINITY || b == NEG_INFINITY || a > b) {
            *this = empty_set();
          }
        }

        Interval(double a)
          : itv(a, a)
        {
          if (a == NEG_INFINITY || a == POS_INFINITY) {
            *this = empty_set();
          }
        }

        explicit Interval(const gaol::interval& x)
          : itv(x)
        {
        }

        // The constants of ibex_IntervalLibWrapper.cpp, which IBEX builds from the
        // GAOL constants of the same values
        static Interval empty_set() { return Interval(gaol::interval::emptyset()); }
        static Interval all_reals() { return Interval(gaol::interval::universe()); }
        static Interval pos_reals() { return Interval(gaol::interval::positive()); }
        static Interval neg_reals() { return Interval(gaol::interval::negative()); }
        static Interval zero() { return Interval(gaol::interval::zero()); }
        static Interval one() { return Interval(gaol::interval::one()); }
        static Interval pi() { return Interval(gaol::interval::pi()); }
        static Interval two_pi() { return Interval(gaol::interval::two_pi()); }
        static Interval half_pi() { return Interval(gaol::interval::half_pi()); }

        Interval& operator=(double x)
        {
          if (x == NEG_INFINITY || x == POS_INFINITY) {
            *this = empty_set();
          } else {
            itv = gaol::interval(x);
          }
          return *this;
        }

        void set_empty() { *this = empty_set(); }

        // The set operations of IBEX, written on the bounds in ibex_SetMembership.cpp,
        // are those of GAOL: IEEE 1788-2015 set_eq, subset (set_leq), interior
        // (set_le), and so on. The strict ones of IBEX are the relation and "not equal"
        bool operator==(const Interval& x) const { return itv.set_eq(x.itv); }
        bool operator!=(const Interval& x) const { return itv.set_neq(x.itv); }
        bool is_subset(const Interval& x) const { return itv.set_leq(x.itv); }
        bool is_strict_subset(const Interval& x) const { return itv.set_leq(x.itv) && itv.set_neq(x.itv); }
        bool is_strict_interior_subset(const Interval& x) const { return itv.set_le(x.itv) && itv.set_neq(x.itv); }
        bool is_superset(const Interval& x) const { return itv.set_geq(x.itv); }
        bool is_strict_superset(const Interval& x) const { return itv.set_geq(x.itv) && itv.set_neq(x.itv); }
        bool contains(const double& d) const { return itv.set_contains(d); }
        bool interior_contains(const double& d) const { return itv.set_strictly_contains(d); }
        bool is_disjoint(const Interval& x) const { return itv.set_disjoint(x.itv); }

        double lb() const { return itv.left(); }
        double ub() const { return itv.right(); }
        double mid() const { return itv.midpoint(); }
        double rad() const { return itv.rad(); }
        double diam() const { return itv.width(); }
        double mig() const { return itv.mig(); }
        double mag() const { return itv.mag(); }
        bool is_empty() const { return itv.is_empty(); }
        bool is_degenerated() const { return is_empty() || itv.is_a_double(); }
        bool is_unbounded() const { return !itv.is_finite(); }
        bool is_bisectable() const { return itv.is_bisectable(); }

        Interval operator-() const { return Interval(-itv); }

        // wrapper: an infinite double is an empty interval
        Interval& operator+=(double d)
        {
          if (d == POS_INFINITY || d == NEG_INFINITY) {
            set_empty();
          } else {
            itv += d;
          }
          return *this;
        }

        Interval& operator-=(double d)
        {
          if (d == POS_INFINITY || d == NEG_INFINITY) {
            set_empty();
          } else {
            itv -= d;
          }
          return *this;
        }

        Interval& operator*=(double d)
        {
          if (d == POS_INFINITY || d == NEG_INFINITY) {
            set_empty();
          } else {
            itv *= d;
          }
          return *this;
        }

        Interval& operator/=(double d)
        {
          if (d == POS_INFINITY || d == NEG_INFINITY) {
            set_empty();
          } else {
            itv /= d;
          }
          return *this;
        }

        Interval& operator+=(const Interval& x) { itv += x.itv; return *this; }
        Interval& operator-=(const Interval& x) { itv -= x.itv; return *this; }
        Interval& operator*=(const Interval& x) { itv *= x.itv; return *this; }
        Interval& operator/=(const Interval& x) { itv /= x.itv; return *this; }
        Interval& operator&=(const Interval& x) { itv &= x.itv; return *this; }
        Interval& operator|=(const Interval& x) { itv |= x.itv; return *this; }

        // ibex_Interval.cpp and the wrapper, as they are: the intersection of
        // the generalized division with *this, in terms of div2() and &
        bool div2_inter(const Interval& num, const Interval& div, Interval& out2);
        Interval& div2_inter(const Interval& x, const Interval& y);

        gaol::interval itv;
    };

    // The wrapper: operator& is empty when the intervals are apart by their bounds
    inline Interval operator&(const Interval& x1, const Interval& x2)
    {
      if (x1.is_empty() || x2.is_empty() || x1.ub() < x2.lb()) {
        return Interval::empty_set();
      } else {
        return Interval(x1.itv & x2.itv);
      }
    }

    inline Interval operator|(const Interval& x1, const Interval& x2) { return Interval(x1.itv | x2.itv); }

    // The wrapper: an infinite double operand gives the empty set
    inline Interval operator+(const Interval& x, double d)
    {
      if (d == NEG_INFINITY || d == POS_INFINITY) {
        return Interval::empty_set();
      } else {
        return Interval(x.itv + d);
      }
    }

    inline Interval operator-(const Interval& x, double d)
    {
      if (d == NEG_INFINITY || d == POS_INFINITY) {
        return Interval::empty_set();
      } else {
        return Interval(x.itv - d);
      }
    }

    inline Interval operator*(const Interval& x, double d)
    {
      if (d == NEG_INFINITY || d == POS_INFINITY) {
        return Interval::empty_set();
      } else {
        return Interval(x.itv * d);
      }
    }

    inline Interval operator/(const Interval& x, double d)
    {
      if (d == NEG_INFINITY || d == POS_INFINITY) {
        return Interval::empty_set();
      } else {
        return Interval(x.itv / d);
      }
    }

    inline Interval operator+(double d, const Interval& x)
    {
      if (d == NEG_INFINITY || d == POS_INFINITY) {
        return Interval::empty_set();
      } else {
        return Interval(d + x.itv);
      }
    }

    inline Interval operator-(double d, const Interval& x)
    {
      if (d == NEG_INFINITY || d == POS_INFINITY) {
        return Interval::empty_set();
      } else {
        return Interval(d - x.itv);
      }
    }

    inline Interval operator*(double d, const Interval& x)
    {
      if (d == NEG_INFINITY || d == POS_INFINITY) {
        return Interval::empty_set();
      } else {
        return Interval(d * x.itv);
      }
    }

    inline Interval operator/(double d, const Interval& x)
    {
      if (d == NEG_INFINITY || d == POS_INFINITY) {
        return Interval::empty_set();
      } else {
        return Interval(d / x.itv);
      }
    }

    inline Interval operator+(const Interval& x1, const Interval& x2) { return Interval(x1.itv + x2.itv); }
    inline Interval operator-(const Interval& x1, const Interval& x2) { return Interval(x1.itv - x2.itv); }
    inline Interval operator*(const Interval& x1, const Interval& x2) { return Interval(x1.itv * x2.itv); }
    inline Interval operator/(const Interval& x1, const Interval& x2) { return Interval(x1.itv / x2.itv); }

    // ibex::distance(): the Hausdorff distance of GAOL. IBEX (ibex_Interval.h) settles
    // the cases of infinite bounds itself before it calls hausdorff(), which GAOL v5
    // gives right (0 for equal bounds, infinite ones included, +oo for an infinite
    // bound against a finite one); the distance to the empty set, which IBEX defines
    // as a radius and GAOL as a NaN, is in no ported test
    inline double distance(const Interval& x1, const Interval& x2) { return gaol::hausdorff(x1.itv, x2.itv); }

    inline Interval sqr(const Interval& x) { return Interval(gaol::sqr(x.itv)); }
    inline Interval sqrt(const Interval& x) { return Interval(gaol::sqrt(x.itv)); }
    inline Interval pow(const Interval& x, int n) { return Interval(gaol::pow(x.itv, n)); }

    // The wrapper, as it is: the root, for an odd n the union with the root of the
    // negative part, and 1/root for a negative n
    inline Interval root(const Interval& x, int n)
    {
      gaol::interval res = gaol::nth_root(x.itv, n >= 0 ? n : -n);

      if (n % 2 == 1 && x.lb() < 0) {
        res |= -gaol::nth_root(-x.itv, n >= 0 ? n : -n);
      }

      if (n < 0) {
        res = 1.0 / res;
      }

      return Interval(res);
    }

    inline Interval exp(const Interval& x) { return Interval(gaol::exp(x.itv)); }

    // The wrapper: gaol::log([-oo,0]) is (-oo,-DBL_MAX), IBEX wants the empty set
    inline Interval log(const Interval& x)
    {
      if (x.ub() <= 0) {
        return Interval::empty_set();
      } else {
        return Interval(gaol::log(x.itv));
      }
    }

    inline Interval cos(const Interval& x) { return Interval(gaol::cos(x.itv)); }
    inline Interval sin(const Interval& x) { return Interval(gaol::sin(x.itv)); }
    inline Interval tan(const Interval& x) { return Interval(gaol::tan(x.itv)); }
    inline Interval atan2(const Interval& y, const Interval& x) { return Interval(gaol::atan2(y.itv, x.itv)); }

    // The wrapper: cosh is [cosh(lb), +oo] when a bound is infinite
    inline Interval cosh(const Interval& x)
    {
      Interval res;
      if (!x.itv.is_finite()) {
        res = Interval(gaol::cosh(x.itv).left(), POS_INFINITY);
      } else {
        res = Interval(gaol::cosh(x.itv));
      }
      return res;
    }

    inline Interval sinh(const Interval& x) { return Interval(gaol::sinh(x.itv)); }

    // The wrapper: asinh of the two sides of 0 apart
    inline Interval asinh(const Interval& x)
    {
      Interval res;
      if (x.is_empty()) {
        res = Interval::empty_set();
      } else if (x.lb() >= 0) {
        res = Interval(gaol::asinh(x.itv));
      } else if (x.ub() <= 0) {
        res = Interval(-gaol::asinh(-x.itv));
      } else {
        gaol::interval upper = gaol::asinh(gaol::interval(0, x.ub()));
        gaol::interval lower = gaol::asinh(gaol::interval(0, -x.lb()));
        res = Interval(-lower.right(), upper.right());
      }
      return res;
    }

    inline Interval floor(const Interval& x) { return Interval(gaol::floor(x.itv)); }
    inline Interval integer(const Interval& x) { return Interval(gaol::integer(x.itv)); }
    inline Interval ceil(const Interval& x) { return Interval(gaol::ceil(x.itv)); }

    // div2(): IBEX splits the generalized division (ibex_Interval.cpp) by cases
    // on the bounds; GAOL v5 has the two-output division of IEEE 1788-2015,
    // mulRevToPair(b, c), x such that b*x is in c, the lower part first. As
    // div2(num, div, out1, out2) divides num by div, b is div and c is num
    inline void div2(const Interval& num, const Interval& div, Interval& out1, Interval& out2)
    {
      const std::pair<gaol::interval, gaol::interval> parts = gaol_ieee1788::mulRevToPair(div.itv, num.itv);
      out1 = Interval(parts.first);
      out2 = Interval(parts.second);
    }

    // ibex_Interval.cpp, Interval::div2_inter(num, div, out2), as it is
    inline bool Interval::div2_inter(const Interval& num, const Interval& div, Interval& out2)
    {
      // warning: we may have &num==this
      Interval out1, _out2;
      div2(num, div, out1, _out2);
      out1 &= *this;
      if (out1.is_empty()) {
        *this &= _out2;
        out2.set_empty();
        return !this->is_empty();
      } else {
        out2 = *this & _out2;
        *this = out1;
        return true;
      }
    }

    // wrapper: Interval::div2_inter(x, y), as it is
    inline Interval& Interval::div2_inter(const Interval& x, const Interval& y)
    {
      Interval out2;
      div2_inter(x, y, out2);
      return *this |= out2;
    }

    // The reverse functions. bwd_mul, bwd_sqr, bwd_cos and bwd_sin are those of the
    // wrapper. Those that IBEX computes by cases on the bounds (ibex_Interval.h) are
    // the reverse functions of GAOL v5: bwd_floor() is floor_rel(), bwd_max() and
    // bwd_min() are max_rel() and min_rel(), and bwd_atan2() is atan2_rel_y() then
    // atan2_rel_x(), each variable in turn given the one before it contracted, as the
    // bwd_mul() of the wrapper is div_rel() for x1 then for x2. bwd_log() is that of
    // IBEX, which is one call of exp()
    inline bool bwd_mul(const Interval& y, Interval& x1, Interval& x2)
    {
      x1 = Interval(gaol::div_rel(y.itv, x2.itv, x1.itv));
      x2 = Interval(gaol::div_rel(y.itv, x1.itv, x2.itv));
      return (!x1.is_empty()) && (!x2.is_empty());
    }

    inline bool bwd_sqr(const Interval& y, Interval& x)
    {
      x = Interval(gaol::sqrt_rel(y.itv, x.itv));
      return !x.is_empty();
    }

    // bwd_pow(y, n, x): for a positive exponent, nth_root_rel() of GAOL, the hull of
    // the x of x for which x^n is in y. The wrapper of IBEX does not call it ("there
    // is a bug in nth_root_rel", GAOL 4) and projects through root() for every n,
    // which it still does here for the exponents it would not take (bwd_pow17)
    inline bool bwd_pow(const Interval& y, int expon, Interval& x)
    {
      if (expon > 0) {
        x = Interval(gaol::nth_root_rel(y.itv, static_cast<unsigned int>(expon), x.itv));
      } else if (expon % 2 == 0) {
        Interval proj = root(y, expon);
        Interval pos_proj = proj & x;
        Interval neg_proj = (-proj) & x;
        x = pos_proj | neg_proj;
      } else {
        x &= root(y, expon);
      }

      return !x.is_empty();
    }

    inline bool bwd_cos(const Interval& y, Interval& x)
    {
      x &= Interval(gaol::acos_rel(y.itv, x.itv));
      return !x.is_empty();
    }

    inline bool bwd_sin(const Interval& y, Interval& x)
    {
      x &= Interval(gaol::asin_rel(y.itv, x.itv));
      return !x.is_empty();
    }

    inline bool bwd_log(const Interval& y, Interval& x)
    {
      x &= exp(y);
      return !x.is_empty();
    }

    inline bool bwd_floor(const Interval& y, Interval& x)
    {
      x = Interval(gaol::floor_rel(y.itv, x.itv));
      return !x.is_empty();
    }

    inline bool bwd_max(const Interval& y, Interval& x1, Interval& x2)
    {
      x1 = Interval(gaol::max_rel(y.itv, x2.itv, x1.itv));
      x2 = Interval(gaol::max_rel(y.itv, x1.itv, x2.itv));
      return (!x1.is_empty()) && (!x2.is_empty());
    }

    inline bool bwd_min(const Interval& y, Interval& x1, Interval& x2)
    {
      x1 = Interval(gaol::min_rel(y.itv, x2.itv, x1.itv));
      x2 = Interval(gaol::min_rel(y.itv, x1.itv, x2.itv));
      return (!x1.is_empty()) && (!x2.is_empty());
    }

    inline bool bwd_atan2(const Interval& theta, Interval& y, Interval& x)
    {
      y = Interval(gaol::atan2_rel_y(theta.itv, x.itv, y.itv));
      x = Interval(gaol::atan2_rel_x(theta.itv, y.itv, x.itv));
      return (!x.is_empty()) && (!y.is_empty());
    }

    // The vectors of IBEX that TestInterval::mid08 uses, with the operations
    // it calls, as in ibex_TemplateVector.h and ibex_LinearArith.h:
    // component by component, with the operators of Interval above
    class Vector
    {
      public:

        explicit Vector(int n)
          : v_(static_cast<std::size_t>(n))
        {
        }

        int size() const { return static_cast<int>(v_.size()); }
        double& operator[](int i) { return v_[static_cast<std::size_t>(i)]; }
        const double& operator[](int i) const { return v_[static_cast<std::size_t>(i)]; }

      private:

        std::vector<double> v_;
    };

    class IntervalVector
    {
      public:

        // IntervalVector(int n, double bounds[][2])
        IntervalVector(int n, double bounds[][2])
          : v_(static_cast<std::size_t>(n))
        {
          for (int i = 0; i < n; i++) {
            v_[static_cast<std::size_t>(i)] = Interval(bounds[i][0], bounds[i][1]);
          }
        }

        int size() const { return static_cast<int>(v_.size()); }
        Interval& operator[](int i) { return v_[static_cast<std::size_t>(i)]; }
        const Interval& operator[](int i) const { return v_[static_cast<std::size_t>(i)]; }
        bool is_empty() const { return (*this)[0].is_empty(); }

        Vector mid() const
        {
          Vector mV(size());
          for (int i = 0; i < size(); i++) {
            mV[i] = (*this)[i].mid();
          }
          return mV;
        }

        IntervalVector& operator+=(const Vector& x)
        {
          for (int i = 0; i < size(); i++) {
            (*this)[i] += x[i];
          }
          return *this;
        }

        IntervalVector& operator-=(const Vector& x)
        {
          for (int i = 0; i < size(); i++) {
            (*this)[i] -= x[i];
          }
          return *this;
        }

        // is_superset(x) is x.is_subset(*this): every component of x in the one of *this
        bool is_superset(const IntervalVector& x) const
        {
          if (x.is_empty()) {
            return true;
          }
          if (is_empty()) {
            return false;
          }
          for (int i = 0; i < size(); i++) {
            if (!(*this)[i].is_superset(x[i])) {
              return false;
            }
          }
          return true;
        }

      private:

        std::vector<Interval> v_;
    };

    inline IntervalVector operator+(const IntervalVector& m1, const Vector& m2) { return IntervalVector(m1) += m2; }
    inline IntervalVector operator-(const IntervalVector& m1, const Vector& m2) { return IntervalVector(m1) -= m2; }
  } // namespace ibex

  // -----------------------------------------------------------------------
  // The program of the tests of IBEX: tests/utils.h and utils.cpp, then the
  // classes TestInterval and TestArith as namespaces (their members are
  // functions of the namespace, so that test names read as in IBEX)
  // -----------------------------------------------------------------------

  namespace ibex_tests
  {
    // tests/*.cpp: using namespace std; utils.h: using namespace ibex;. The functions of
    // ibex are found by argument-dependent lookup, so that sin(0.5) is the one of
    // the C library as in IBEX, where the overload resolution picks it by the type
    using namespace std;
    using ibex::Interval;
    using ibex::IntervalVector;
    using ibex::Vector;
    using ibex::POS_INFINITY;
    using ibex::NEG_INFINITY;

    // utils.cpp: double ERROR = 1e-10;
    const double ERROR_TOL = 1e-10;
    // TestArith.cpp: #define M_PI 3.14159265358979323846
    const double M_PI_VALUE = 3.14159265358979323846;

    // utils.h
void check(double y_actual, double y_expected, double err = ERROR_TOL);
void check(const Interval& y_actual, const Interval& y_expected, double err = ERROR_TOL);
bool almost_eq(const Interval& y_actual, const Interval& y_expected, double err = ERROR_TOL);

    // utils.cpp
void check(double y_actual, double y_expected, double err) {
	CPPUNIT_ASSERT(!std::isnan(y_expected));
	if (y_expected==POS_INFINITY) { CPPUNIT_ASSERT(y_actual==POS_INFINITY); }
	else if (y_expected==NEG_INFINITY) { CPPUNIT_ASSERT(y_actual==NEG_INFINITY); }
	else {
		CPPUNIT_ASSERT(y_actual!=POS_INFINITY);
		CPPUNIT_ASSERT(y_actual!=NEG_INFINITY);
		CPPUNIT_ASSERT_DOUBLES_EQUAL(y_expected,y_actual,err);
	}
}

void check(const Interval& y_actual, const Interval& y_expected, double err) {
	//std::cout << "check:    " << y_expected << " (expected)        " << y_actual << " (actual)"<< std::endl;
	if (y_expected.is_empty()) { CPPUNIT_ASSERT(y_actual.is_empty()); return; }

	CPPUNIT_ASSERT(!y_actual.is_empty());
	CPPUNIT_ASSERT(!std::isnan(y_actual.lb()));
	CPPUNIT_ASSERT(!std::isnan(y_actual.ub()));
	CPPUNIT_ASSERT_DOUBLES_EQUAL(y_expected.lb(),y_actual.lb(),err);
	CPPUNIT_ASSERT_DOUBLES_EQUAL(y_expected.ub(),y_actual.ub(),err);
}

bool almost_eq(const Interval& y_actual, const Interval& y_expected, double err) {
	if (y_actual.is_empty() && y_expected.is_empty()) return true;
	if (y_actual.is_empty() || y_expected.is_empty()) return false;

	if (y_actual.lb()==NEG_INFINITY)
		if (y_expected.lb()!=NEG_INFINITY) return false;
		else {}
	else if (fabs(y_actual.lb()-y_expected.lb())> err) return false;

	if (y_actual.ub()==POS_INFINITY)
		if (y_expected.ub()!=POS_INFINITY) return false;
		else {}
	else if (fabs(y_actual.ub()-y_expected.ub())> err) return false;

	return true;
}

    namespace TestInterval
    {

void cons01() {
	check(Interval(),Interval(NEG_INFINITY,POS_INFINITY));
	check((Interval())=Interval(),Interval(NEG_INFINITY,POS_INFINITY));
}

void cons02() {
	check(Interval(1.0),Interval(1.0,1.0));
	check((Interval())=1.0, Interval(1.0,1.0));
}

void cons03() {
	check(Interval(NEG_INFINITY),Interval::empty_set());
	check((Interval())=NEG_INFINITY, Interval::empty_set());
}

void cons04() {
	check(Interval(POS_INFINITY),Interval::empty_set());
	check((Interval())=POS_INFINITY, Interval::empty_set());
}

void cons05() {
	check(Interval(1,0), Interval::empty_set());
	check((Interval())=Interval(1,0), Interval::empty_set());
}

void check_eq(const Interval& x, const Interval& y, bool is_eq) {
	CPPUNIT_ASSERT(is_eq? x==y : !(x==y));
	CPPUNIT_ASSERT(is_eq? !(x!=y) : x!=y);
	CPPUNIT_ASSERT(is_eq? y==x : !(y==x));
	CPPUNIT_ASSERT(is_eq? !(y!=x) : y!=x);
}

void eq01() { check_eq(Interval::all_reals(),Interval::all_reals(), true); }

void eq02() { check_eq(Interval::all_reals(),Interval::pos_reals(), false); }

void eq03() { check_eq(Interval(0,1), Interval(0,1), true); }

void eq04() { check_eq(Interval(0,2), Interval(0,1), false); }

void eq05() { check_eq(Interval(0,2), Interval(1,3), false); }

void eq06() { check_eq(Interval::pos_reals(),Interval::pos_reals(), true); }

void eq07() { check_eq(Interval::empty_set(),Interval::empty_set(), true); }

void eq08() { check_eq(Interval(0,0),Interval(0,0), true); }

void distance01() { check(ibex::distance(Interval(0,10), Interval(-5,5)), 5); }

void distance02() { check(ibex::distance(Interval(0,10), Interval(5,10)), 5); }

void distance03() { check(ibex::distance(Interval(0,10), Interval(5,15)), 5); }

void distance04() { check(ibex::distance(Interval(0,10), Interval(1,2)), 8); /* 9 */}

void distance05() { check(ibex::distance(Interval(0,10), Interval(0,10)), 0); }

void distance06() { check(ibex::distance(Interval(0,10), Interval(-10,20)), 10); /* 0 */}

void distance07() { check(ibex::distance(Interval::pos_reals(), Interval(0,10)), POS_INFINITY); }

void distance08() { check(ibex::distance(Interval::all_reals(), Interval(0,10)), POS_INFINITY); }

void distance09() { check(ibex::distance(Interval::all_reals(), Interval::all_reals()), 0); }

void distance10() { check(ibex::distance(Interval::all_reals(), Interval::neg_reals()), POS_INFINITY); }

void distance11() { check(ibex::distance(Interval::all_reals(), Interval::pos_reals()), POS_INFINITY); }

void distance12() { check(ibex::distance(Interval::pos_reals(), Interval(NEG_INFINITY,1)), POS_INFINITY); }

void distance13() { check(ibex::distance(Interval::pos_reals(), Interval(0,1)), POS_INFINITY); }

void distance14() { check(ibex::distance(Interval::neg_reals(), Interval(-1,POS_INFINITY)), POS_INFINITY); }

void distance15() { check(ibex::distance(Interval::neg_reals(), Interval(NEG_INFINITY,-1)), 1); }

void distance15b(){ check(ibex::distance(Interval::pos_reals(), Interval(1,POS_INFINITY)), 1); }

void distance16() { check(ibex::distance(Interval(0,10), Interval(5,5)), 5); /* 10 */}

void distance17() { check(ibex::distance(Interval(0,10), Interval(0,0)), 10); }

void distance18() { check(ibex::distance(Interval(0,10), Interval(10,10)), 10); }

void distance19() { check(ibex::distance(Interval(0,0), Interval(0,0)), 0); }

void distance22() { check(ibex::distance(Interval(0,10), Interval(11,20)), 11); }

void distance23() { check(ibex::distance(Interval(0,10), Interval(0,5)), 5); }

void check_hull(const Interval& x, const Interval& z, const Interval& y_expected) {
	//std::cout << x<< "  " << z<< "  "<<  (x | z) << std::endl;
	check(x|z, y_expected);
	check(z|x, y_expected);
    check((Interval(x)|=z), y_expected);
    check((Interval(z)|=x), y_expected);
}

void hull01() { check_hull(Interval(0,1), Interval(2,3), Interval(0,3)); }

void hull02() { check_hull(Interval(0,1), Interval::empty_set(), Interval(0,1)); }

void hull03() { check_hull(Interval(-1), Interval::pos_reals(), Interval(-1,POS_INFINITY)); }

void check_inter(const Interval& x, const Interval& z, const Interval& y_expected) {
	check(x&z, y_expected);
	check(z&x, y_expected);
    check((Interval(x)&=z), y_expected);
    check((Interval(z)&=x), y_expected);
}

void inter01() { check_inter(Interval(0,1), Interval(2,3), Interval::empty_set()); }

void inter02() { check_inter(Interval(0,1), Interval::empty_set(), Interval::empty_set()); }

void inter03() { check_inter(Interval(0,2), Interval(-1,1), Interval(0,1)); }

void inter04() { check_inter(Interval(0,1), Interval::pos_reals(), Interval(0,1)); }

void inter05() { check_inter(Interval(0,1), Interval::neg_reals(), Interval::zero()); }

void inter06() { check_inter(Interval(0,1), Interval(1,2), Interval(1,1)); }

void getters01() {
	Interval x(0,2);
	CPPUNIT_ASSERT_DOUBLES_EQUAL_MESSAGE("lb",0,x.lb(), ERROR_TOL);
	CPPUNIT_ASSERT_DOUBLES_EQUAL_MESSAGE("ub",2,x.ub(), ERROR_TOL);
	CPPUNIT_ASSERT_DOUBLES_EQUAL_MESSAGE("mid",1,x.mid(), ERROR_TOL);
	CPPUNIT_ASSERT_DOUBLES_EQUAL_MESSAGE("rad",1,x.rad(), ERROR_TOL);
	CPPUNIT_ASSERT_DOUBLES_EQUAL_MESSAGE("diam",2,x.diam(), ERROR_TOL);
	CPPUNIT_ASSERT_DOUBLES_EQUAL_MESSAGE("mig",0,x.mig(), ERROR_TOL);
	CPPUNIT_ASSERT_DOUBLES_EQUAL_MESSAGE("mag",2,x.mag(), ERROR_TOL);
}

void getters02() {
	Interval x(-3,-1);
	CPPUNIT_ASSERT_DOUBLES_EQUAL_MESSAGE("lb",-3,x.lb(), ERROR_TOL);
	CPPUNIT_ASSERT_DOUBLES_EQUAL_MESSAGE("ub",-1,x.ub(), ERROR_TOL);
	CPPUNIT_ASSERT_DOUBLES_EQUAL_MESSAGE("mid",-2,x.mid(), ERROR_TOL);
	CPPUNIT_ASSERT_DOUBLES_EQUAL_MESSAGE("rad",1,x.rad(), ERROR_TOL);
	CPPUNIT_ASSERT_DOUBLES_EQUAL_MESSAGE("diam",2,x.diam(), ERROR_TOL);
	CPPUNIT_ASSERT_DOUBLES_EQUAL_MESSAGE("mig",1,x.mig(), ERROR_TOL);
	CPPUNIT_ASSERT_DOUBLES_EQUAL_MESSAGE("mag",3,x.mag(), ERROR_TOL);
}

void getters03() {
	Interval x(-3,1);
	CPPUNIT_ASSERT_DOUBLES_EQUAL_MESSAGE("lb",-3,x.lb(), ERROR_TOL);
	CPPUNIT_ASSERT_DOUBLES_EQUAL_MESSAGE("ub",1,x.ub(), ERROR_TOL);
	CPPUNIT_ASSERT_DOUBLES_EQUAL_MESSAGE("mid",-1,x.mid(), ERROR_TOL);
	CPPUNIT_ASSERT_DOUBLES_EQUAL_MESSAGE("rad",2,x.rad(), ERROR_TOL);
	CPPUNIT_ASSERT_DOUBLES_EQUAL_MESSAGE("diam",4,x.diam(), ERROR_TOL);
	CPPUNIT_ASSERT_DOUBLES_EQUAL_MESSAGE("mig",0,x.mig(), ERROR_TOL);
	CPPUNIT_ASSERT_DOUBLES_EQUAL_MESSAGE("mag",3,x.mag(), ERROR_TOL);
}

void getters04() {
	Interval x(Interval::neg_reals());
	CPPUNIT_ASSERT_DOUBLES_EQUAL_MESSAGE("lb",NEG_INFINITY,x.lb(), ERROR_TOL);
	CPPUNIT_ASSERT_DOUBLES_EQUAL_MESSAGE("ub",0,x.ub(), ERROR_TOL);
	CPPUNIT_ASSERT_DOUBLES_EQUAL_MESSAGE("mid",-DBL_MAX,x.mid(), ERROR_TOL);
	CPPUNIT_ASSERT_DOUBLES_EQUAL_MESSAGE("rad",POS_INFINITY,x.rad(), ERROR_TOL);
	CPPUNIT_ASSERT_DOUBLES_EQUAL_MESSAGE("diam",POS_INFINITY,x.diam(), ERROR_TOL);
	CPPUNIT_ASSERT_DOUBLES_EQUAL_MESSAGE("mig",0,x.mig(), ERROR_TOL);
	CPPUNIT_ASSERT_DOUBLES_EQUAL_MESSAGE("mag",POS_INFINITY,x.mag(), ERROR_TOL);
}

void getters05() {
	Interval x(Interval::all_reals());
	CPPUNIT_ASSERT_DOUBLES_EQUAL_MESSAGE("lb",NEG_INFINITY,x.lb(), ERROR_TOL);
	CPPUNIT_ASSERT_DOUBLES_EQUAL_MESSAGE("ub",POS_INFINITY,x.ub(), ERROR_TOL);
	CPPUNIT_ASSERT_DOUBLES_EQUAL_MESSAGE("mid",0,x.mid(), ERROR_TOL);
	CPPUNIT_ASSERT_DOUBLES_EQUAL_MESSAGE("rad",POS_INFINITY,x.rad(), ERROR_TOL);
	CPPUNIT_ASSERT_DOUBLES_EQUAL_MESSAGE("diam",POS_INFINITY,x.diam(), ERROR_TOL);
	CPPUNIT_ASSERT_DOUBLES_EQUAL_MESSAGE("mig",0,x.mig(), ERROR_TOL);
	CPPUNIT_ASSERT_DOUBLES_EQUAL_MESSAGE("mag",POS_INFINITY,x.mag(), ERROR_TOL);
}

void getters06() {
	Interval x(DBL_MAX,POS_INFINITY);
	CPPUNIT_ASSERT_DOUBLES_EQUAL_MESSAGE("lb",DBL_MAX,x.lb(), ERROR_TOL);
	CPPUNIT_ASSERT_DOUBLES_EQUAL_MESSAGE("ub",POS_INFINITY,x.ub(), ERROR_TOL);
	CPPUNIT_ASSERT_DOUBLES_EQUAL_MESSAGE("mid",DBL_MAX,x.mid(), ERROR_TOL);
	CPPUNIT_ASSERT_DOUBLES_EQUAL_MESSAGE("rad",POS_INFINITY,x.rad(), ERROR_TOL);
	CPPUNIT_ASSERT_DOUBLES_EQUAL_MESSAGE("diam",POS_INFINITY,x.diam(), ERROR_TOL);
	CPPUNIT_ASSERT_DOUBLES_EQUAL_MESSAGE("mig",DBL_MAX,x.mig(), ERROR_TOL);
	CPPUNIT_ASSERT_DOUBLES_EQUAL_MESSAGE("mag",POS_INFINITY,x.mag(), ERROR_TOL);
}

void is_subset01()          { CPPUNIT_ASSERT(Interval(0,1).is_subset(Interval(0,2))); }

void is_subset02()          { CPPUNIT_ASSERT(!Interval(0,1).is_subset(Interval(1,2))); }

void is_strict_interior_subset01()   { CPPUNIT_ASSERT(!Interval(0,1).is_strict_interior_subset(Interval(0,2))); }

void is_strict_interior_subset02()   { CPPUNIT_ASSERT(Interval::neg_reals().is_strict_interior_subset(Interval::all_reals())); }

void is_strict_interior_subset03()   { CPPUNIT_ASSERT(Interval::empty_set().is_strict_interior_subset(Interval(0,1))); }

void is_superset01()        { CPPUNIT_ASSERT(Interval(0,2).is_superset(Interval(0,1))); }

void is_superset02()        { CPPUNIT_ASSERT(!Interval(1,2).is_superset(Interval(0,1))); }

void is_strict_superset01() { CPPUNIT_ASSERT(!Interval(0,2).is_strict_superset(Interval(0,2))); }

void is_strict_superset02() { CPPUNIT_ASSERT(Interval::all_reals().is_strict_superset(Interval::neg_reals())); }

void is_strict_superset03() { CPPUNIT_ASSERT(!Interval::empty_set().is_strict_superset(Interval::empty_set())); }

void contains01()           { CPPUNIT_ASSERT(Interval(0,2).contains(0)); }

void contains02()           { CPPUNIT_ASSERT(Interval::all_reals().contains(POS_INFINITY)); }

void interior_contains01()  { CPPUNIT_ASSERT(!Interval(0,2).interior_contains(0)); }

void interior_contains02()  { CPPUNIT_ASSERT(Interval(0,2).interior_contains(1)); }

void is_disjoint01()        { CPPUNIT_ASSERT(!Interval::neg_reals().is_disjoint(Interval::pos_reals())); }

void is_disjoint02()        { CPPUNIT_ASSERT(Interval(0,1).is_disjoint(Interval(2,3))); }

void is_empty01()           { CPPUNIT_ASSERT(Interval(POS_INFINITY).is_empty()); }

void is_empty02()           { CPPUNIT_ASSERT(!Interval(0,1).is_empty()); }

void is_degenerated01()     { CPPUNIT_ASSERT(Interval::zero().is_degenerated()); }

void is_degenerated02()     { CPPUNIT_ASSERT(Interval::empty_set().is_degenerated()); }

void is_unbounded01()       { CPPUNIT_ASSERT(!Interval::empty_set().is_unbounded()); }

void is_unbounded02()       { CPPUNIT_ASSERT(!Interval(0,1).is_unbounded()); }

void is_unbounded03()       { CPPUNIT_ASSERT(Interval::neg_reals().is_unbounded()); }

void is_unbounded04()       { CPPUNIT_ASSERT(Interval::pos_reals().is_unbounded()); }

void is_bisectable01()      { CPPUNIT_ASSERT(!Interval::empty_set().is_bisectable()); }

void is_bisectable02()      { CPPUNIT_ASSERT(!Interval(DBL_MAX,POS_INFINITY).is_bisectable()); }

void is_bisectable03()      { CPPUNIT_ASSERT(!Interval(NEG_INFINITY,-DBL_MAX).is_bisectable()); }

void is_bisectable04()      { CPPUNIT_ASSERT(!Interval(0,ibex::next_float(0)).is_bisectable()); }

void is_bisectable05()      {	CPPUNIT_ASSERT(Interval(0,ibex::next_float(ibex::next_float(0))).is_bisectable());}

void is_bisectable06()      {CPPUNIT_ASSERT(Interval(10,ibex::next_float(ibex::next_float(10))).is_bisectable());}

void is_bisectable07()      {CPPUNIT_ASSERT(Interval(ibex::previous_float(ibex::previous_float(0)),0).is_bisectable());}

void is_bisectable08()      {CPPUNIT_ASSERT(Interval(ibex::previous_float(ibex::previous_float(10)),10).is_bisectable());}

void is_bisectable09()      {CPPUNIT_ASSERT(!Interval(10,ibex::next_float(10)).is_bisectable()); }

void is_bisectable10()      {CPPUNIT_ASSERT(!Interval(ibex::previous_float(0),0).is_bisectable()); }

void mid01() {
	check(Interval(0,2).mid(),1.0);
}

void mid02() {
	CPPUNIT_ASSERT(Interval(NEG_INFINITY,POS_INFINITY).mid()==0);
}

void mid03() {
	CPPUNIT_ASSERT(Interval::pos_reals().mid()==DBL_MAX);
}

void mid04() {
	CPPUNIT_ASSERT(Interval::neg_reals().mid()==-DBL_MAX);
}

void mid05() {
	double d=Interval(DBL_MAX,POS_INFINITY).mid();
	CPPUNIT_ASSERT(d==DBL_MAX || d==-DBL_MAX);
}

void mid06() {
	CPPUNIT_ASSERT(Interval::zero().mid()==0.0);
}

void mid07() {
	Interval x(2.7629416257693772963,2.7629416257693835135);
	Interval y=x;
	//cout.precision(15);
	//cout << "x=" << y << endl;
	static double m=x.mid();
	//cout << "milieu = " << m << endl;
	Interval x2=x+m;
	//cout << "x+m=" << x2 << endl;
	Interval x3=x2-m;
	//cout << "(x+m)-m" <<x3 << endl;
	CPPUNIT_ASSERT(x3.is_superset(y));
}

void mid08() {
	double _box[][2]= {
			{3.4836061062102725572,3.4836061062102761099},
			{3.5754039600153961764,3.5754039600154006173},
			{3.2401050837195057852,3.2401050837195093379},
			{3.5187713541601914891,3.5187713541601937095},
			{3.4109008703180077049,3.410900870318013034},
			{3.336209585763303842,3.3362095857633069507},
			{3.3970878856415889224,3.3970878856415973601},
			{3.0865914745327103219,3.0865914745327134305},
			{3.1673998478272404888,3.1673998478272467061},
			{3.2795769799080982665,3.2795769799081013751},
			{2.9966040612287390132,2.996604061228742566},
			{3.4621387483049854694,3.4621387483049890221},
			{0.39999999999999980016,0.40000000000000074385},
			{1.9595917942265419498,1.959591794226542838},
			{2.4033959387712586775,2.4033959387712604538},
			{-1.5025469540784459621,-1.5025469540784437417},
			{3.4474903944979735648,3.4474903944979780057},
			{2.8417250066344248438,2.8417250066344270643},
			{3.6343497218046629804,3.6343497218046700858},
			{3.0078080936676636625,3.0078080936676689916},
			{3.8716115579677374825,3.8716115579677432557},
			{2.9290247128026156709,2.929024712802626329},
			{3.6847522306610471787,3.6847522306610511755},
			{2.7629416257693772963,2.7629416257693835135},
			{3.4978929033543559868,3.4978929033543604277},
			{2.5968585387361389216,2.5968585387361411421},
			{5,5},
			{-0,0},
			{-0,0},
			{-0,0} };

	IntervalVector box(30,_box);
	CPPUNIT_ASSERT((box+box.mid()-box.mid()).is_superset(box));
}

void issue_463() {
	Interval a = Interval(3,8);
	Interval b = Interval(1,3) & Interval(6,7); // [b] is empty
	Interval c = a+b;
	CPPUNIT_ASSERT(c.is_empty());
}

    } // namespace TestInterval

    namespace TestArith
    {

void check_add(const Interval& x, const Interval& z, const Interval& y_expected) {
	Interval y_actual=x+z;
	check(y_actual, y_expected);

	// test the symmetrical case
	y_actual = z+x;
	check(y_actual, y_expected);

	// test the +=operator
	y_actual = x;
	y_actual += z;
	check(y_actual, y_expected);

	// test the +=operator in the other direction
	y_actual = z;
	y_actual += x;
	check(y_actual, y_expected);

	// test subtraction
	y_actual=(-x)-z;
	check(y_actual, -y_expected);

	// test the symmetrical case
	y_actual = (-z)-x;
	check(y_actual, -y_expected);

	// test the -=operator
	y_actual = -x;
	y_actual -= z;
	check(y_actual, -y_expected);

	// test the -=operator in the other direction
	y_actual = -z;
	y_actual -= x;
	check(y_actual, -y_expected);

}

void check_add_scal(const Interval& x, double z, const Interval& y_expected) {
	check(x+z, y_expected);
	check(z+x, y_expected);
	check(Interval(x)+=z, y_expected); // test the +=operator
	check((-x)-z, -y_expected);
	check((-z)-x, -y_expected);
	check(Interval(-x)-=z, -y_expected); // test the -=operator
}

void check_mul(const Interval& x, const Interval& z, const Interval& y_expected) {
	Interval y_actual=x*z;
	//cout << "check:    " << x << " * " << z << ", " << y_expected << endl;
	check(y_actual, y_expected);

	// test the symmetrical case
	y_actual = z*x;
	check(y_actual, y_expected);

	// test the *=operator
	y_actual = x;
	y_actual *= z;
	check(y_actual, y_expected);
}

void check_mul_scal(const Interval& x, double z, const Interval& y_expected) {
	check(x*z, y_expected);
	check(z*x, y_expected);
	check(Interval(x)*=z, y_expected); // test the +=operator
}

void check_div(const Interval& x, const Interval& z, const Interval& y_expected) {
	Interval y_actual=x/z;
	//cout << "check:    " << x << " / " << z << ", " << y_expected << " (expected)        " << y_actual << " (actual)"<< endl;
	check(y_actual, y_expected);

	// test the /=operator
	y_actual = x;
	y_actual /= z;
	check(y_actual, y_expected);
}

void check_div_scal(const Interval& x, double z, const Interval& y_expected) {
	Interval y_actual=x/z;
	check(y_actual, y_expected);

	// test the /=operator
	y_actual = x;
	y_actual /= z;
	check(y_actual, y_expected);
}

void float01() { CPPUNIT_ASSERT(0 < ibex::next_float(0)); }

void float02() { CPPUNIT_ASSERT(0 > ibex::previous_float(0)); }

void float03() { CPPUNIT_ASSERT( 1 < ibex::next_float(1)); }

void float04() { CPPUNIT_ASSERT(1 > ibex::previous_float(1)); }

void float05() { CPPUNIT_ASSERT( POS_INFINITY == ibex::next_float(POS_INFINITY)); }

void float06() { CPPUNIT_ASSERT( -DBL_MAX == ibex::next_float(NEG_INFINITY)); }

void float07() { CPPUNIT_ASSERT( DBL_MAX == ibex::previous_float(POS_INFINITY)); }

void float08() { CPPUNIT_ASSERT( NEG_INFINITY == ibex::previous_float(NEG_INFINITY)); }

void float09() { CPPUNIT_ASSERT( POS_INFINITY == ibex::next_float(DBL_MAX)); }

void minus01() { check(-Interval(0,1), Interval(-1,0)); }

void minus02() { check(-Interval::all_reals(), Interval::all_reals()); }

void minus03() { check(-Interval::neg_reals(), Interval::pos_reals()); }

void minus04() { check(-Interval(NEG_INFINITY,1), Interval(-1,POS_INFINITY)); }

void add01() { check_add(Interval::empty_set(),      Interval(0,1),      Interval::empty_set()); }

void add02() { check_add(Interval(0,1),            Interval::empty_set(), Interval::empty_set()); }

void add03() { check_add(Interval(NEG_INFINITY,1), Interval(0,1),      Interval(NEG_INFINITY, 2)); }

void add04() { check_add(Interval(1,POS_INFINITY), Interval(0,1),      Interval(1,POS_INFINITY)); }

void add05() { check_add(Interval::all_reals(),      Interval(0,1),      Interval::all_reals()); }

void add06() { check_add_scal(Interval::empty_set(),      POS_INFINITY,       Interval::empty_set()); }

void add07() { check_add_scal(Interval::empty_set(),      0,                  Interval::empty_set()); }

void add08() { check_add_scal(Interval(0,1),            1,                  Interval(1,2)); }

void add09() { check_add_scal(Interval(0,1),            NEG_INFINITY,       Interval::empty_set()); }

void add10() { check_add_scal(Interval(0,1),            POS_INFINITY,       Interval::empty_set()); }

void add11() { check_add_scal(Interval(NEG_INFINITY,1), 1,                  Interval(NEG_INFINITY,2)); }

void add12() { check_add(Interval(DBL_MAX,POS_INFINITY), 1, Interval(DBL_MAX,POS_INFINITY)); }

void add13() { check_add(Interval(DBL_MAX,POS_INFINITY), -1, Interval(ibex::previous_float(DBL_MAX),POS_INFINITY)); }

void add14() { check_add(Interval(DBL_MAX,POS_INFINITY), Interval(DBL_MAX,POS_INFINITY), Interval(DBL_MAX,POS_INFINITY)); }

void add15() { check_add(Interval(DBL_MAX,POS_INFINITY), NEG_INFINITY, Interval::empty_set()); }

void add16() { check_add(Interval(DBL_MAX,POS_INFINITY), POS_INFINITY, Interval::empty_set()); }

void add17() { check_add(Interval(NEG_INFINITY,-DBL_MAX),  1, Interval(NEG_INFINITY,ibex::next_float(-DBL_MAX))); }

void add18() { check_add(Interval(NEG_INFINITY,-DBL_MAX), -1, Interval(NEG_INFINITY,-DBL_MAX)); }

void add19() { check_add(Interval(NEG_INFINITY,-DBL_MAX),  Interval(NEG_INFINITY,-DBL_MAX), Interval(NEG_INFINITY,-DBL_MAX)); }

void mul01() { check_mul(Interval::empty_set(),         Interval(0,1),               Interval::empty_set()); }

void mul02() { check_mul(Interval::zero(),              Interval::all_reals(),         Interval::zero()); }

void mul03() { check_mul(Interval(-1,1),              Interval::neg_reals(), 	     Interval::all_reals()); }

void mul04() { check_mul(Interval(NEG_INFINITY,-1),   Interval(-1,0),              Interval::pos_reals()); }

void mul05() { check_mul(Interval(NEG_INFINITY, 1),   Interval(-1,0),              Interval(-1,POS_INFINITY)); }

void mul06() { check_mul(Interval(0, 1),              Interval(1,POS_INFINITY), 	 Interval::pos_reals()); }

void mul07() { check_mul(Interval(0, 1),              Interval(-1,POS_INFINITY),   Interval(-1,POS_INFINITY)); }

void mul08() { check_mul(Interval(NEG_INFINITY,-1),   Interval(0,1),               Interval::neg_reals()); }

void mul09() { check_mul(Interval(NEG_INFINITY, 1),   Interval(0,1),               Interval(NEG_INFINITY,1)); }

void mul10() { check_mul(Interval(0, 1),              Interval(NEG_INFINITY,-1),   Interval::neg_reals()); }

void mul11() { check_mul(Interval(0, 1),              Interval(NEG_INFINITY,1),    Interval(NEG_INFINITY,1)); }

void mul12() { check_mul(Interval(1,POS_INFINITY),    Interval(0,1),               Interval::pos_reals()); }

void mul13() { check_mul(Interval(-1,POS_INFINITY),   Interval(0,1),               Interval(-1,POS_INFINITY)); }

void mul14() { check_mul(Interval(1,2),               Interval(1,2), 	        	 Interval(1,4)); }

void mul15() { check_mul(Interval(1,2),               Interval(-2,3), 	         Interval(-4,6)); }

void mul16() { check_mul(Interval(-1,1),              Interval(-2,3), 	         Interval(-3,3)); }

void mul17() { check_mul_scal(Interval(1,2),          NEG_INFINITY, 	        	 Interval::empty_set()); }

void mul18() { check_mul_scal(Interval(1,2),          POS_INFINITY, 	        	 Interval::empty_set()); }

void mul19() { check_mul_scal(Interval(1,2),          -1, 	        	         Interval(-2,-1)); }

void div01() { check_div(Interval::empty_set(),         Interval(0,1),               Interval::empty_set()); }

void div02() { check_div(Interval::zero(),              Interval::zero(),              Interval::empty_set()); }

void div03() { check_div(Interval(1,2),               Interval::zero(),              Interval::empty_set()); }

void div04() { check_div(Interval::all_reals(),         Interval::zero(),              Interval::empty_set()); }

void div05() { check_div(Interval::zero(),              Interval(0,1),               Interval::zero()); }

void div06() { check_div(Interval::zero(),              Interval::all_reals(),         Interval::zero()); }

void div07() { check_div(Interval(6,12),              Interval(2,3),               Interval(2,6)); }

void div08() { check_div(Interval(6,12),              Interval(-3,-2),             Interval(-6,-2)); }

void div09() { check_div(Interval(6,12),              Interval(-2,3),              Interval::all_reals()); }

void div10() { check_div(Interval(NEG_INFINITY,-1),   Interval(-1,0),              Interval(1,POS_INFINITY)); }

void div11() { check_div(Interval(NEG_INFINITY,-1),   Interval(0,1),               Interval(NEG_INFINITY,-1)); }

void div12() { check_div(Interval(1,POS_INFINITY),    Interval(-1,0),              Interval(NEG_INFINITY,-1)); }

void div13() { check_div(Interval(1,POS_INFINITY),    Interval(0,1),               Interval(1,POS_INFINITY)); }

void div14() { check_div(Interval(-1,1),              Interval(-1,1), 	         Interval::all_reals()); }

void div15() { check_div_scal(Interval(1,2),          NEG_INFINITY, 	        	 Interval::empty_set()); }

void div16() { check_div_scal(Interval(1,2),          POS_INFINITY, 	        	 Interval::empty_set()); }

void div17() { check_div_scal(Interval(1,2),          -1, 	        	         Interval(-2,-1)); }

void sqrt01() { check(sqrt(Interval::all_reals()), Interval::pos_reals()); }

void sqrt02() { check(sqrt(Interval::neg_reals()), Interval::zero()); }

void sqrt03() { check(sqrt(Interval(-9,4)),      Interval(0,2)); }

void sqrt04() { check(sqrt(Interval(4,9)),       Interval(2,3)); }

void sqrt05() { check(sqrt(Interval(-9,-4)),     Interval::empty_set()); }

#define piL Interval::pi().lb()

#define piU Interval::pi().ub()

void check_sinh(const Interval& x) {
	double xl=x.lb();
	double xu=x.ub();
	double yl=xl==NEG_INFINITY? NEG_INFINITY : 0.5*(exp(xl)-exp(-xl));
	double yu=xu==POS_INFINITY? POS_INFINITY : 0.5*(exp(xu)-exp(-xu));

	check(sinh(x), Interval(yl,yu));
	check(sinh(-x), Interval(-yu,-yl));
	check(x,asinh(sinh(x)));
	check(-x,asinh(sinh(-x)));
}

void sinh01() { check_sinh(Interval::all_reals()); }

void sinh02() { check_sinh(Interval::pos_reals()); }

void sinh03() { check_sinh(Interval(0,1)); }

void sinh04() { check_sinh(Interval(1,POS_INFINITY)); }

void sinh05() { check_sinh(Interval(1,1)); }

void sinh06() { check_sinh(Interval(2,3)); }

void sinh07() { check_sinh(Interval(4,5)); }

void check_cosh(const Interval& x) {

	Interval y ;
	if (x.ub()==POS_INFINITY) {
		if (x.lb()<=0) y=Interval(1,POS_INFINITY);
		else y=Interval((cosh(x.lb())),POS_INFINITY);
	}
	else if (x.lb()==NEG_INFINITY) {
		if (x.ub()>=0) y=Interval(1,POS_INFINITY);
		else y=Interval((cosh(x.ub())),POS_INFINITY);
	}
	else if (x.lb()>=0)
		y=Interval((cosh(x.lb())),(cosh(x.ub())));
	else if (x.ub()<=0)
		y=Interval((cosh(x.ub())),(cosh(x.lb())));
	else
		y=((fabs(x.lb())> fabs(x.ub())) ? Interval(1,(cosh(x.lb()))) :Interval(1,(cosh(x.ub()))));


//	std::cout << x<<" : " << cosh(x)<< "|||  "<<y<<std::endl;
	check(cosh(x), y);
	check(cosh(-x), y);
//	check(x,acosh(cosh(x)));
//	check(x,acosh(cosh(-x)));
}

void cosh01() { check_cosh(Interval::all_reals()); }

void cosh02() { check_cosh(Interval::pos_reals()); }

void cosh03() { check_cosh(Interval(0,1)); }

void cosh04() { check_cosh(Interval(1,POS_INFINITY)); }

void cosh05() { check_cosh(Interval(1,1)); }

void cosh06() { check_cosh(Interval(2,3)); }

void cosh07() { check_cosh(Interval(4,5)); }

void check_trigo(const Interval& x, const Interval& sin_x_expected) {
	check(sin(x), sin_x_expected);
	check(sin(Interval::pi()-x), sin_x_expected);
	check(sin(x+Interval::two_pi()), sin_x_expected);
	check(sin(-x), -sin_x_expected);
	check(cos(x-Interval::half_pi()), sin_x_expected);
	check(cos(Interval::half_pi()-x), sin_x_expected);
	check(cos(x+Interval::half_pi()), -sin_x_expected);
	check(cos(x+Interval::two_pi()-Interval::half_pi()), sin_x_expected);
}

void log01() { check(log(Interval::empty_set()), Interval::empty_set()); }

void log02() { check(log(Interval::all_reals()), Interval::all_reals()); }

void log03() { check(log(Interval::pos_reals()), Interval::all_reals()); }

void log04() { check(log(Interval::neg_reals()),  Interval::empty_set()); /*Interval(NEG_INFINITY,-DBL_MAX));*/ }

void log05() { check(log(Interval(1,2)),       Interval(0,std::log(2))); }

void log06() { check(log(Interval(-1,1)),      Interval(NEG_INFINITY,0)); }

void log07() { CPPUNIT_ASSERT((log(Interval(0,ibex::next_float(0)))).ub()> -744.5); }

void log08() { check(log(Interval(0,1)),       Interval(NEG_INFINITY,0)); }

void log09() { check(log(Interval(1,POS_INFINITY)), Interval::pos_reals()); }

void log10() { check(log(Interval(0)), Interval::empty_set()); /* Interval(NEG_INFINITY,-DBL_MAX)); */ }

void log11() { check(log(Interval(-2,-1)), Interval::empty_set()); }

void exp01() { check(exp(Interval::empty_set()), Interval::empty_set()); }

void exp02() { check(exp(Interval::all_reals()), Interval::pos_reals()); }

void exp03() { check(exp(Interval::pos_reals()), Interval(1,POS_INFINITY)); }

void exp04() { check(exp(Interval::neg_reals()), Interval(0,1)); }

void exp05() { check(exp(Interval(0,2)),       Interval(1,std::exp(2))); }

void exp06() { check(exp(Interval(-1,1)),      Interval(std::exp(-1),std::exp(1))); }

void exp07() { check(exp(Interval(1.e100,1.e111)),Interval(DBL_MAX,POS_INFINITY)); }

void exp08() { check(exp(Interval(DBL_MAX,POS_INFINITY)),Interval(DBL_MAX,POS_INFINITY)); }

void exp09() { check(exp(Interval(0,DBL_MAX)),Interval(1,POS_INFINITY)); }

void sin01() { check_trigo(Interval::all_reals(), Interval(-1,1)); }

void sin02() { check_trigo(Interval::empty_set(), Interval::empty_set()); }

void sin03() { check_trigo(Interval(0,piU/2.0), Interval(0,1)); }

void sin04() { check_trigo(Interval(0,piU), Interval(0,1)); }

void sin05() { check_trigo(Interval(0,3*piU/2.0), Interval(-1,1)); }

void sin06() { check_trigo(Interval(piL,3*piU/2.0), Interval(-1,0)); }

void sin07() { check_trigo(Interval(0.5,1.5), Interval(sin(0.5),sin(1.5))); }

void sin08() { check_trigo(Interval(1.5,3), Interval(sin(3.0),1)); }

void sin09() { check_trigo(Interval(3,4), Interval(sin(4.0),sin(3.0))); }

void sin10() { check_trigo(Interval(3,5), Interval(-1,sin(3.0))); }

void sin11() { check_trigo(Interval(3,2*piU+1.5), Interval(-1,sin(1.5))); }

void sin12() { check_trigo(Interval(5,2*piU+1.5), Interval(sin(5.0),sin(1.5))); }

void sin13() { check_trigo(Interval(5,2*piU+3), Interval(sin(5.0),1)); }

void tan01() { check(tan(Interval::all_reals()), Interval::all_reals()); }

void tan02() { check(tan(-Interval::pi()/4.0 | Interval::pi()/4.0), Interval(-1,1)); }

void tan03() {	// tan(pi/4,pi/2)=[1,+oo)
	Interval x=Interval(piL/4.0,(1-1e-10)*piL/2.0); // upper bound of x is close to pi/2
	Interval y=tan(x);
	check(y.lb(),1.0);
	CPPUNIT_ASSERT(y.ub()>1000); // upper bound of tan(x) is close to +oo
}

void tan04() {	// tan(-pi/2,pi/4)=(-oo,1]
	Interval y=tan(Interval(-(1-1e-10)*piL/2.0,piL/4.0));
	CPPUNIT_ASSERT(y.lb()<-1000); // lower bound is close to -oo
	check(y.ub(),1.0);
}

void tan05() { check(tan(Interval::pi()/2.0),Interval::all_reals()); }

void floor01() { CPPUNIT_ASSERT(floor(Interval::empty_set())==Interval::empty_set()); }

void floor02() { CPPUNIT_ASSERT(floor(Interval(NEG_INFINITY,-0.000001))==Interval(NEG_INFINITY,-1)); }

void floor03() { CPPUNIT_ASSERT(floor(Interval(0.00000001,POS_INFINITY))==Interval::pos_reals()); }

void floor04() { CPPUNIT_ASSERT(floor(Interval::all_reals())==Interval::all_reals()); }

void floor05() { CPPUNIT_ASSERT(floor(Interval(0.01,2.99))==Interval(0,2)); }

void floor06() { CPPUNIT_ASSERT(floor(Interval(-0.01,2.99))==Interval(-1,2)); }

void floor07() { CPPUNIT_ASSERT(floor(Interval(1.000000001,1.9999999999))==Interval::one()); }

void floor08() { CPPUNIT_ASSERT(floor(Interval(1e8, DBL_MAX))==Interval(1e8,DBL_MAX)); }

void ceil01() { CPPUNIT_ASSERT(ceil(Interval::empty_set())==Interval::empty_set()); }

void ceil02() { CPPUNIT_ASSERT(ceil(Interval(NEG_INFINITY,-0.000001))==Interval::neg_reals()); }

void ceil03() { CPPUNIT_ASSERT(ceil(Interval(0.00000001,POS_INFINITY))==Interval(1.0,POS_INFINITY)); }

void ceil04() { CPPUNIT_ASSERT(ceil(Interval::all_reals())==Interval::all_reals()); }

void ceil05() { CPPUNIT_ASSERT(ceil(Interval(0.01,2.99))==Interval(1,3)); }

void ceil06() { CPPUNIT_ASSERT(ceil(Interval(-0.01,2.99))==Interval(0,3)); }

void ceil07() { CPPUNIT_ASSERT(ceil(Interval(1.000000001,1.9999999999))==Interval(2,2)); }

void ceil08() { CPPUNIT_ASSERT(ceil(Interval(1e8, DBL_MAX))==Interval(1e8,DBL_MAX)); }

void integer01() { CPPUNIT_ASSERT(integer(Interval::empty_set())==Interval::empty_set()); }

void integer02() { CPPUNIT_ASSERT(integer(Interval(NEG_INFINITY,-0.000001))==Interval(NEG_INFINITY,-1)); }

void integer03() { CPPUNIT_ASSERT(integer(Interval(0.00000001,POS_INFINITY))==Interval(1.0,POS_INFINITY)); }

void integer04() { CPPUNIT_ASSERT(integer(Interval::all_reals())==Interval::all_reals()); }

void integer05() { CPPUNIT_ASSERT(integer(Interval(0.01,2.99))==Interval(1,2)); }

void integer06() { CPPUNIT_ASSERT(integer(Interval(-0.01,2.99))==Interval(0,2)); }

void integer07() { CPPUNIT_ASSERT(integer(Interval(1.000000001,1.9999999999))==Interval::empty_set()); }

void integer08() { CPPUNIT_ASSERT(integer(Interval(1e8, DBL_MAX))==Interval(1e8,DBL_MAX)); }

void atan2_01() { check(atan2(Interval::all_reals(),Interval::empty_set()),Interval::empty_set()); }

void atan2_02() { check(atan2(Interval::one(),Interval::one()),     Interval::pi()/4.0); }

void atan2_03() { check(atan2(-Interval::one(),-Interval::one()),-3*Interval::pi()/4.0); }

void atan2_04() { check(atan2(-Interval::one(),Interval::one()),   -Interval::pi()/4.0); }

void atan2_05() { check(atan2(Interval::one(),-Interval::one()),  3*Interval::pi()/4.0); }

void atan2_06() { check(atan2(Interval::pos_reals(),Interval::pos_reals()), Interval(0,1)*Interval::half_pi()); }

void atan2_07() { check(atan2(Interval(NEG_INFINITY,ibex::previous_float(0.0)),Interval::neg_reals()), -(Interval::half_pi() | Interval::pi())); }

void atan2_08() { check(atan2(Interval::neg_reals(),Interval::pos_reals()), Interval(-1,0)*Interval::half_pi()); }

void atan2_09() { check(atan2(Interval::pos_reals(),Interval::neg_reals()), (Interval::half_pi() | Interval::pi())); }

void atan2_10() { check(atan2(Interval(1,POS_INFINITY),Interval(-1,1)), Interval::pi()/4.0 | 3*Interval::pi()/4.0); }

void atan2_11() { check(atan2(Interval(NEG_INFINITY,-1),Interval(-1,1)), -(Interval::pi()/4.0 | 3*Interval::pi()/4.0)); }

void atan2_12() { check(atan2(Interval(-1,1),Interval(1,POS_INFINITY)), Interval(-1,1)*Interval::pi()/4.0); }

void atan2_13() { check(atan2(Interval(-1,1),Interval(NEG_INFINITY,-1)), Interval(-1,1)*Interval::pi()); }

void atan2_14() { check(atan2(Interval::all_reals(),Interval::all_reals()),Interval(-1,1)*Interval::pi()); }

void atan2_15() { check(atan2(Interval::zero(),Interval::zero()),Interval::empty_set()); }

void check_pow(const Interval& x, int p, const Interval& y_expected) {
	check(pow(x,p),y_expected);
	check(pow(-x,p),(p%2==0)? y_expected : -y_expected);
}

void pow01() { check_pow(Interval::all_reals(), 4, Interval::pos_reals()); }

void pow02() { check_pow(Interval::empty_set(), 4, Interval::empty_set()); }

void pow03() { check_pow(Interval(2,3), 4, Interval(16,81)); }

void pow04() { check_pow(Interval(-2,3), 4, Interval(0,81)); }

void pow05() { check_pow(Interval(-3,2), 4, Interval(0,81)); }

void pow06() { check_pow(Interval(2,POS_INFINITY), 4, Interval(16,POS_INFINITY)); }

void pow07() { check_pow(Interval::all_reals(), 3, Interval::all_reals()); }

void pow08() { check_pow(Interval::empty_set(), 3, Interval::empty_set()); }

void pow09() { check_pow(Interval(2,3), 3, Interval(8,27)); }

void pow10() { check_pow(Interval(-2,3), 3, Interval(-8,27)); }

void pow11() { check_pow(Interval(-3,2), 3, Interval(-27,8)); }

void pow12() { check_pow(Interval(2,POS_INFINITY), 3, Interval(8,POS_INFINITY)); }

void pow13() { check_pow(Interval(-10,10), -2, Interval(1.0/100,POS_INFINITY)); }

void root01() { check(root(Interval(0,1),-1), Interval(1.0,POS_INFINITY)); }

void root02() { CPPUNIT_ASSERT(almost_eq(root(Interval(-27,-8), 3),Interval(-3,-2),1e-10)); }

void root03() { check(root(Interval(-4,1),2), Interval(0,1)); }

void root04() { check(root(Interval(-8,1),3), Interval(-2,1)); }

#define checkproj(func,y,xbefore,xafter) { Interval x=xbefore; NAME2(bwd_,func)(y,x); check(x,xafter); }

#define NAME2(a,b)         NAME2_HIDDEN(a,b)

#define NAME2_HIDDEN(a,b)  a ## b

void checkbwd_trigo(const Interval& y, const Interval& xbefore, const Interval& xafter) {
    checkproj(sin,  y, xbefore, xafter);
	checkproj(sin, -y,-xbefore,-xafter);
	checkproj(cos,  y, xbefore-Interval::half_pi(), xafter-Interval::half_pi());
	checkproj(cos, -y, xbefore+Interval::half_pi(), xafter+Interval::half_pi());
}

void checkbwd_pow(const Interval& y, const Interval& xbefore, const Interval& xafter, int p) {
	Interval x;
	x=xbefore; bwd_pow(y,p,x); check(x,xafter);
	x=-xbefore; bwd_pow(p%2==0? y : -y,p,x); check(x,-xafter);
}

bool checkbwd_mul(const Interval& y, const Interval& x1_bef, const Interval& x2_bef,
								const Interval& x1_aft, const Interval& x2_aft) {
	bool res=true;
	Interval x1;
	Interval x2;

	x1=x1_bef;
	x2=x2_bef;
	bwd_mul(y,x1,x2);
	//cout << "x1=" << x1 << " x2=" << x2 << endl;
	res &= almost_eq(x1,x1_aft,ERROR_TOL) && almost_eq(x2,x2_aft,ERROR_TOL);

	x1=x1_bef;
	x2=x2_bef;
	bwd_mul(y,x2,x1);
	res &= almost_eq(x1,x1_aft,ERROR_TOL) && almost_eq(x2,x2_aft,ERROR_TOL);

	x1=-x1_bef;
	x2=x2_bef;
	bwd_mul(-y,x2,x1);
	res &= almost_eq(x1,-x1_aft,ERROR_TOL) && almost_eq(x2,x2_aft,ERROR_TOL);

	x1=x1_bef;
	x2=-x2_bef;
	bwd_mul(-y,x2,x1);
	res &= almost_eq(x1,x1_aft,ERROR_TOL) && almost_eq(x2,-x2_aft,ERROR_TOL);

	return res;
}

void bwd_mul01() {
	CPPUNIT_ASSERT(checkbwd_mul(Interval(1,2),Interval(0.1,2.0),Interval(-10,2),Interval(0.5,2.0),Interval(0.5,2)));
}

void bwd_mul02() {
	CPPUNIT_ASSERT(checkbwd_mul(Interval::empty_set(),Interval(0.1,2.0),Interval(-10,2),Interval::empty_set(),Interval::empty_set()));
}

void bwd_mul03() {
	CPPUNIT_ASSERT(checkbwd_mul(Interval(0,0),Interval(0.1,2.0),Interval(-10,2),Interval(0.1,2.0),Interval(0,0)));
}

void bwd_mul04() {
	CPPUNIT_ASSERT(checkbwd_mul(Interval(0,0),Interval(-1,1),Interval(-1,1),Interval(-1,1),Interval(-1,1)));
}

void bwd_mul05() {
	CPPUNIT_ASSERT(checkbwd_mul(Interval(1,1),Interval(0,10),Interval(0,10),Interval(0.1,10.0),Interval(0.1,10.0)));
}

void checkbwd_max(const Interval& z,  const Interval& x, const Interval& y, const Interval& x_expected, const Interval& y_expected) {

	Interval _x=x;
	Interval _y=y;
	bwd_max(z,_x,_y);

	CPPUNIT_ASSERT(x_expected==_x);
	CPPUNIT_ASSERT(y_expected==_y);

	// swap the inputs
	_x=x;
	_y=y;
	bwd_max(z,_y,_x);

	CPPUNIT_ASSERT(x_expected==_x);
	CPPUNIT_ASSERT(y_expected==_y);

	_x=-x;
	_y=-y;
	bwd_min(-z,_x,_y);

	CPPUNIT_ASSERT(x_expected==-_x);
	CPPUNIT_ASSERT(y_expected==-_y);

	// swap the inputs
	_x=-x;
	_y=-y;
	bwd_min(-z,_y,_x);

	CPPUNIT_ASSERT(x_expected==-_x);
	CPPUNIT_ASSERT(y_expected==-_y);
}

void bwd_maxmin01() { checkbwd_max(Interval::empty_set(), Interval(-2,-1),Interval(-2,3),Interval::empty_set(),Interval::empty_set()); }

void bwd_maxmin02() { checkbwd_max(Interval(0,1),       Interval(-2,-1),Interval(-2,3),Interval(-2,-1),    Interval(0,1)); }

void bwd_maxmin03() { checkbwd_max(Interval(0,1),       Interval(-2,-1),Interval(2,3), Interval::empty_set(),Interval::empty_set()); }

void bwd_maxmin04() { checkbwd_max(Interval(0,1),       Interval(-2,0), Interval(-2,3),Interval(-2,0),    Interval(-2,1)); }

void bwd_maxmin05() { checkbwd_max(Interval(0,1),       Interval(-2,2), Interval(-2,3),Interval(-2,1),    Interval(-2,1)); }

void bwd_sqr01() { checkproj(sqr, Interval(1,9),            Interval(0,4),       Interval(1,3)); }

void bwd_sqr02() { checkproj(sqr, Interval(1,9),            Interval(0,2),       Interval(1,2)); }

void bwd_sqr03() { checkproj(sqr, Interval(1,9),            Interval(-4,2),      Interval(-3,2)); }

void bwd_sqr04() { checkproj(sqr, Interval(1,9),            Interval(-4,-3),     Interval(-3,-3)); }

void bwd_sqr05() { checkproj(sqr, Interval(NEG_INFINITY,9), Interval(-4,1),      Interval(-3,1)); }

void bwd_sqr06() { checkproj(sqr, Interval(4,9),            Interval(-1,5),      Interval(2,3)); }

void bwd_sqr07() { checkproj(sqr, Interval(-4,-2),          Interval::all_reals(), Interval::empty_set()); }

void bwd_log01() { checkproj(log, Interval::all_reals(),      Interval::all_reals(), Interval::pos_reals()); }

void bwd_log02() { checkproj(log, Interval::neg_reals(),      Interval::all_reals(), Interval(0,1)); }

void bwd_log03() { checkproj(log, Interval::pos_reals(),      Interval(0,1),       1); }

void bwd_log04() { checkproj(log, Interval(0,1),      		Interval(-1,3),      Interval(1,std::exp(1))); }

void bwd_log05() { checkproj(log, Interval(NEG_INFINITY,1), Interval(-1,3),      Interval(0,std::exp(1))); }

void bwd_log06() { checkproj(log, Interval(NEG_INFINITY,1), Interval(-1,2),      Interval(0,2)); }

void bwd_log07() { checkproj(log, Interval(NEG_INFINITY,1), Interval(3,4),       Interval::empty_set()); }

void bwd_log08() { checkproj(log, Interval(-1,1),           Interval(-0.01,0.01),Interval::empty_set()); }

void bwd_sin01() { checkbwd_trigo(sin(Interval(0.5,1.5)), Interval(0,piU/2.0),        Interval(0.5,1.5)); }

void bwd_sin02() { checkbwd_trigo(sin(Interval(0.5,1.5)), Interval(0,5*piU/2.0),      Interval(0.5,2*piU+1.5)); }

void bwd_sin03() { checkbwd_trigo(sin(Interval(0.5,1.5)), Interval(-2*piU,piU/2.0),   Interval(-2*piU+0.5,1.5)); }

void bwd_sin04() { checkbwd_trigo(sin(Interval(0.5,1.5)), Interval(-2*piU,5*piU/2.0), Interval(-2*piU+0.5,2*piU+1.5)); }

void bwd_sin05() { checkbwd_trigo(sin(Interval(0.5,1.5)), Interval(-piL-0.4,piU/2.0), Interval(0.5,1.5)); }

void bwd_sin06() { checkbwd_trigo(sin(Interval(0.5,1.5)), Interval(-piU-0.5,piU/2.0), Interval(-piU-0.5,1.5)); }

void bwd_sin07() { checkbwd_trigo(sin(Interval(0.5,1.5)), Interval(-3*piU/2,piU/2.0), Interval(-piU-1.5,1.5)); }

void bwd_sin08() { checkbwd_trigo(sin(Interval(0.5,1.5)), Interval(0.5,piL-1.6),      Interval(0.5,1.5)); }

void bwd_sin09() { checkbwd_trigo(sin(Interval(0.5,1.5)), Interval(0.5,piU-1.5),      Interval(0.5,piU-1.5)); }

void bwd_sin10() { checkbwd_trigo(sin(Interval(0.5,1.5)), Interval(0.5,piU),          Interval(0.5,piU-0.5)); }

void bwd_sin11() { checkbwd_trigo(sin(Interval(0.5,1.5)), Interval(-piU-0.5,piU-1.5), Interval(-piU-0.5,piU-1.5)); }

void bwd_sin12() { checkbwd_trigo(sin(Interval(0.5,1.5)), Interval(-3*piU/2,piU),     Interval(-piU-1.5,piU-0.5)); }

void bwd_sin13() { checkbwd_trigo(sin(Interval(0.5,1.5)), Interval(-piU/2,piU/2.0),   Interval(0.5,1.5)); }

void bwd_sin14() { checkbwd_trigo(Interval(2,3),          Interval::all_reals(),        Interval::empty_set()); }

void bwd_sin15() { checkbwd_trigo(Interval(sin(0.5),1.0), Interval(0,piU/2.0),        Interval(0.5,piU/2.0)); }

void bwd_sin16() { checkbwd_trigo(Interval(sin(0.5),1.0), Interval(0,5*piU/2.0),      Interval(0.5,5*piU/2.0)); }

void bwd_sin17() { checkbwd_trigo(Interval(sin(0.5),1.0), Interval(-2*piU,piU/2.0),   Interval(-2*piU+0.5,piU/2.0)); }

void bwd_sin18() { checkbwd_trigo(Interval(sin(0.5),1.0), Interval(-2*piU,5*piU/2.0), Interval(-2*piU+0.5,5*piU/2.0)); }

void bwd_sin19() { checkbwd_trigo(Interval(sin(0.5),1.0), Interval(-piL-0.4,piU/2.0), Interval(0.5,piU/2.0)); }

void bwd_sin20() { checkbwd_trigo(Interval(sin(0.5),1.0), Interval(-piU-0.5,piU/2.0), Interval(-piU-0.5,piU/2.0)); }

void bwd_sin21() { checkbwd_trigo(Interval(sin(0.5),1.0), Interval(-3*piU/2,piU/2.0), Interval(-3*piU/2,piU/2.0)); }

void bwd_sin22() { checkbwd_trigo(Interval(sin(0.5),1.0), Interval(0.5,piU),          Interval(0.5,piU-0.5)); }

void bwd_sin23() { checkbwd_trigo(Interval(sin(0.5),1.0), Interval(-3*piU/2,piU),     Interval(-3*piU/2,piU-0.5)); }

void bwd_sin24() { checkbwd_trigo(Interval(1.0),          Interval(4.0,6.0),          Interval::empty_set()); }

void bwd_pow01() { checkbwd_pow(Interval(16,81), Interval(1,4), Interval(2,3), 4); }

void bwd_pow02() { checkbwd_pow(Interval(16,81), Interval(3,4), Interval(3,3), 4); }

void bwd_pow03() { checkbwd_pow(Interval(16,81), Interval(-1,4), Interval(2,3), 4); }

void bwd_pow04() { checkbwd_pow(Interval(16,81), Interval(-2,4), Interval(-2,3), 4); }

void bwd_pow05() { checkbwd_pow(Interval(16,81), Interval(-5,4), Interval(-3,3), 4); }

void bwd_pow06() { checkbwd_pow(Interval(16,81), Interval(1,1), Interval::empty_set(), 4); }

void bwd_pow07() { checkbwd_pow(Interval(16,81), Interval(4,4), Interval::empty_set(), 4); }

void bwd_pow08() { checkbwd_pow(Interval(8,27), Interval(1,4), Interval(2,3), 3); }

void bwd_pow09() { checkbwd_pow(Interval(8,27), Interval(3,4), Interval(3,3), 3); }

void bwd_pow10() { checkbwd_pow(Interval(8,27), Interval(-5,4), Interval(2,3), 3); }

void bwd_pow11() { checkbwd_pow(Interval(-8,27), Interval(-5,4), Interval(-2,3), 3); }

void bwd_pow12() { checkbwd_pow(Interval(-27,27), Interval(-5,4), Interval(-3,3), 3); }

void bwd_pow13() { checkbwd_pow(Interval(8,27), Interval(1,1), Interval::empty_set(), 3); }

void bwd_pow14() { checkbwd_pow(Interval(8,27), Interval(4,4), Interval::empty_set(), 3); }

void bwd_pow15() { checkbwd_pow(Interval(-27,-8), Interval(-1,-1), Interval::empty_set(), 3); }

void bwd_pow16() { checkbwd_pow(Interval(-27,-8), Interval(-4,-4), Interval::empty_set(), 3); }

void bwd_pow17() { checkbwd_pow(Interval(0,1), Interval(-10,10), Interval(-10,10), -2); }

void check_div2(const Interval& x, const Interval& y, const Interval& out1, const Interval& out2) {
	Interval _out1,_out2;
	div2(x,y,_out1,_out2);
	check(_out1,out1);
	check(_out2,out2);

	Interval _inout=out1;
	_inout.div2_inter(x,y);
	check(_inout,out1);

	_inout=out1;
	_inout.div2_inter(x,y,_out2);
	check(_inout,out1);
	CPPUNIT_ASSERT(_out2.is_empty());

	_inout=out2;
	_inout.div2_inter(x,y);
	check(_inout,out2);

	_inout=out2;
	_inout.div2_inter(x,y,_out2);
	check(_inout,out2);
	CPPUNIT_ASSERT(_out2.is_empty());

	_inout=Interval::all_reals();
	_inout.div2_inter(x,y);
	check(_inout,out1 | out2);

	_inout=Interval::all_reals();
	_inout.div2_inter(x,y,_out2);
	check(_inout,out1);
	check(_out2,out2);

	if (!out1.is_empty() && out1.rad()>0) {
		Interval out1bis(out1.mid(),out1.ub());

		if (!out2.is_empty() && out2.lb()>out1.ub())
			_inout = Interval(out1.mid(),0.5*(out1.ub()+out2.lb()));
		else
			_inout = out1bis;

		_inout.div2_inter(x,y);
		check(_inout,out1bis);

		_inout = out1bis;
		_inout.div2_inter(x,y,_out2);
		check(_inout,out1bis);
		CPPUNIT_ASSERT(_out2.is_empty());

		if (!out2.is_empty() && out2.rad()>0) {
			Interval out2bis(out2.lb(),out2.mid());

			_inout = out1bis | out2bis;
			_inout.div2_inter(x,y);
			check(_inout,out1bis | out2bis);

			_inout = out1bis | out2bis;
			_inout.div2_inter(x,y,_out2);
			check(_inout,out1bis);
			check(_out2,out2bis);
		}
	}

}

void div2_01() { check_div2(Interval(6,12),  Interval(0,0),             Interval::empty_set(),       Interval::empty_set()); }

void div2_02() { check_div2(Interval(6,12),  Interval(2,3),             Interval(2,6),             Interval::empty_set()); }

void div2_03() { check_div2(Interval(-2,3),  Interval(-1,1),            Interval::all_reals(),       Interval::empty_set()); }

void div2_04() { check_div2(Interval(2,3),   Interval(-1,1),            Interval(NEG_INFINITY,-2), Interval(2,POS_INFINITY)); }

void div2_05() { check_div2(Interval(-3,-2), Interval(-1,1),            Interval(NEG_INFINITY,-2), Interval(2,POS_INFINITY)); }

void div2_06() { check_div2(Interval(2,3),   Interval(NEG_INFINITY,1),  Interval::neg_reals(),       Interval(2,POS_INFINITY)); }

void div2_07() { check_div2(Interval(2,3),   Interval(-1,POS_INFINITY), Interval(NEG_INFINITY,-2), Interval::pos_reals()); }

void div2_08() { check_div2(Interval(-3,-2), Interval(NEG_INFINITY,1),  Interval(NEG_INFINITY,-2), Interval::pos_reals()); }

void div2_09() { check_div2(Interval(-3,-2), Interval(-1,POS_INFINITY), Interval::neg_reals(),       Interval(2,POS_INFINITY)); }

void div2_10() { check_div2(Interval(2,3),   Interval::neg_reals(),       Interval::neg_reals(),       Interval::empty_set()); }

void div2_11() { check_div2(Interval(2,3),   Interval::pos_reals(),       Interval::pos_reals(),       Interval::empty_set()); }

void div2_12() { check_div2(Interval(-3,-2), Interval::neg_reals(),       Interval::pos_reals(),       Interval::empty_set()); }

void div2_13() { check_div2(Interval(-3,-2), Interval::pos_reals(),       Interval::neg_reals(),       Interval::empty_set()); }

bool checkbwd_atan2(const Interval& a, const Interval& y_bef, const Interval& x_bef,
								const Interval& y_aft, const Interval& x_aft) {
	bool res=true;
	Interval y;
	Interval x;

	y=y_bef;
	x=x_bef;
	bwd_atan2(a,y,x);
	//cout << "x1=" << x1 << " x2=" << x2 << endl;
	res &= almost_eq(x,x_aft,ERROR_TOL) && almost_eq(y,y_aft,ERROR_TOL);

	return res;
}

void bwd_atan2_01() { //first quadrant
	CPPUNIT_ASSERT(checkbwd_atan2(Interval(M_PI_VALUE/6.,M_PI_VALUE/3.),Interval(.5,10.),Interval(.5,2.),Interval(.5,2.*sqrt(3.)),Interval(.5,2.)));
}

void bwd_atan2_02() { // first quadrant
	CPPUNIT_ASSERT(checkbwd_atan2(Interval(M_PI_VALUE/6.,M_PI_VALUE/3.),Interval(.5,2.),Interval(.5,10.),Interval(.5,2.),Interval(.5,2.*sqrt(3.))));
}

void bwd_atan2_03() { // left half
	CPPUNIT_ASSERT(checkbwd_atan2(Interval(-M_PI_VALUE/4.,M_PI_VALUE/4.),Interval(1.,2.),Interval(.5,2.),Interval(1.,2.),Interval(1.,2.)));
}

void bwd_atan2_04() { //fourth quadrant
	CPPUNIT_ASSERT(checkbwd_atan2(Interval(-M_PI_VALUE/2.,0.),Interval(.5,2.),Interval(.5,10.),Interval::empty_set(),Interval::empty_set()));
}

void bwd_atan2_05() { //upper half
	// ** confusion with angle constraint **
	//CPPUNIT_ASSERT(checkbwd_atan2(Interval(2.*M_PI_VALUE,3.*M_PI_VALUE),Interval(-.5,2.),Interval(.5,10.),Interval(0. ,2.),Interval(.5,10.)));
	CPPUNIT_ASSERT(checkbwd_atan2(Interval(2.*M_PI_VALUE,3.*M_PI_VALUE),Interval(-.5,2.),Interval(.5,10.),Interval::empty_set(), Interval::empty_set()));
}

void bwd_atan2_06() { //second quadrant
	CPPUNIT_ASSERT(checkbwd_atan2(Interval(2*M_PI_VALUE/3.,5.*M_PI_VALUE/6.),Interval(0.,100.),Interval(-20.,-sqrt(3.)/2.),Interval(.5,20.*sqrt(3.)),Interval(-20.,-sqrt(3.)/2)));
}

void bwd_atan2_07() { //third quadrant
	CPPUNIT_ASSERT(checkbwd_atan2(Interval(-3*M_PI_VALUE/4.,-2*M_PI_VALUE/3.),Interval(-sqrt(3.)/2.,2.),Interval(-sqrt(2.)/2.,0.),Interval(-sqrt(3.)/2.,0.),Interval(-sqrt(2.)/2.,0.)));
}

void bwd_atan2_08() { //third quadrant
	CPPUNIT_ASSERT(checkbwd_atan2(Interval(-3*M_PI_VALUE/4.,-2*M_PI_VALUE/3.),Interval(-sqrt(3.)/2.,2.),Interval(-1.,-.5),Interval(-sqrt(3.)/2.,-.5),Interval(-sqrt(3.)/2.,-.5)));
}

void bwd_atan2_09() { //lower half
	CPPUNIT_ASSERT(checkbwd_atan2(Interval(-3*M_PI_VALUE/4.,-M_PI_VALUE/4.),Interval(-5.,-.5),Interval::all_reals(),Interval(-5.,-.5),Interval(-5.,5.)));
}

void bwd_atan2_10() { //right half
	CPPUNIT_ASSERT(checkbwd_atan2(Interval(-M_PI_VALUE/3.,M_PI_VALUE/4.),Interval::all_reals(),Interval(sqrt(3.)/2.),Interval(-1.5,sqrt(3.)/2.),Interval(sqrt(3.)/2.)));
}

void bwd_atan2_issue134() {
	CPPUNIT_ASSERT(checkbwd_atan2(Interval::half_pi(),Interval::one(), Interval::zero(), Interval::one(), Interval::zero()));
}

bool checkbwd_floor(const Interval& y, const Interval& xbefore, const Interval& x_after_expected) {
	Interval x(xbefore);
	bwd_floor(y,x);
	return x==x_after_expected;
}

void bwd_floor_01() { CPPUNIT_ASSERT(checkbwd_floor(Interval::empty_set(), Interval::all_reals(), Interval::empty_set())); }

void bwd_floor_02() { CPPUNIT_ASSERT(checkbwd_floor(Interval(NEG_INFINITY,-1), Interval(NEG_INFINITY,-0.000001), Interval(NEG_INFINITY,-0.000001))); }

void bwd_floor_03() { CPPUNIT_ASSERT(checkbwd_floor(Interval(NEG_INFINITY,-1), Interval(NEG_INFINITY, 0.000001), Interval::neg_reals())); }

void bwd_floor_04() { CPPUNIT_ASSERT(checkbwd_floor(Interval(NEG_INFINITY,-0.000001), Interval(NEG_INFINITY, 0.000001), Interval::neg_reals())); }

void bwd_floor_05() { CPPUNIT_ASSERT(checkbwd_floor(Interval(1,POS_INFINITY), Interval(0.000001, POS_INFINITY), Interval(1,POS_INFINITY))); }

void bwd_floor_06() { CPPUNIT_ASSERT(checkbwd_floor(Interval(1,POS_INFINITY), Interval(-0.000001, POS_INFINITY), Interval(1,POS_INFINITY))); }

void bwd_floor_07() { CPPUNIT_ASSERT(checkbwd_floor(Interval(0.000001,POS_INFINITY), Interval(-0.000001, POS_INFINITY), Interval(1,POS_INFINITY))); }

void bwd_floor_08() { CPPUNIT_ASSERT(checkbwd_floor(Interval(1,2), Interval(0.01,2.99), Interval(1,2.99))); }

void bwd_floor_09() { CPPUNIT_ASSERT(checkbwd_floor(Interval(1,2), Interval(2.99,3.01), Interval(2.99,3))); }

void bwd_floor_10() { CPPUNIT_ASSERT(checkbwd_floor(Interval(1,2), Interval(3.01,3.99), Interval::empty_set())); }

void bwd_floor_11() { CPPUNIT_ASSERT(checkbwd_floor(Interval(0.01,2.99), Interval(0.01,2.99), Interval(1,2.99))); }

void tan_issue248() {
	//Interval itv = Interval(-Interval::pi().lb()/2,3*Interval::pi().ub()/8);
	Interval itv = Interval(-1.57079632679489678, 1.1780972450961728626);
	CPPUNIT_ASSERT(!(tan(itv).is_empty()));
}

    } // namespace TestArith

  } // namespace ibex_tests

  // The tests of IBEX that run against GAOL, in the order of their registration in
  // TestInterval.h and TestArith.h (CPPUNIT_TEST_SUITE)
  inline void run()
  {
    struct Test
    {
      const char* group;
      const char* name;
      void (*function)();
    };

    static const Test tests[] = {
      { "TestInterval", "cons01", &ibex_tests::TestInterval::cons01 },
      { "TestInterval", "cons02", &ibex_tests::TestInterval::cons02 },
      { "TestInterval", "cons03", &ibex_tests::TestInterval::cons03 },
      { "TestInterval", "cons04", &ibex_tests::TestInterval::cons04 },
      { "TestInterval", "cons05", &ibex_tests::TestInterval::cons05 },
      { "TestInterval", "eq01", &ibex_tests::TestInterval::eq01 },
      { "TestInterval", "eq02", &ibex_tests::TestInterval::eq02 },
      { "TestInterval", "eq03", &ibex_tests::TestInterval::eq03 },
      { "TestInterval", "eq04", &ibex_tests::TestInterval::eq04 },
      { "TestInterval", "eq05", &ibex_tests::TestInterval::eq05 },
      { "TestInterval", "eq06", &ibex_tests::TestInterval::eq06 },
      { "TestInterval", "eq07", &ibex_tests::TestInterval::eq07 },
      { "TestInterval", "eq08", &ibex_tests::TestInterval::eq08 },
      { "TestInterval", "inter01", &ibex_tests::TestInterval::inter01 },
      { "TestInterval", "inter02", &ibex_tests::TestInterval::inter02 },
      { "TestInterval", "inter03", &ibex_tests::TestInterval::inter03 },
      { "TestInterval", "inter04", &ibex_tests::TestInterval::inter04 },
      { "TestInterval", "inter05", &ibex_tests::TestInterval::inter05 },
      { "TestInterval", "inter06", &ibex_tests::TestInterval::inter06 },
      { "TestInterval", "hull01", &ibex_tests::TestInterval::hull01 },
      { "TestInterval", "hull02", &ibex_tests::TestInterval::hull02 },
      { "TestInterval", "hull03", &ibex_tests::TestInterval::hull03 },
      { "TestInterval", "getters01", &ibex_tests::TestInterval::getters01 },
      { "TestInterval", "getters02", &ibex_tests::TestInterval::getters02 },
      { "TestInterval", "getters03", &ibex_tests::TestInterval::getters03 },
      { "TestInterval", "getters04", &ibex_tests::TestInterval::getters04 },
      { "TestInterval", "getters05", &ibex_tests::TestInterval::getters05 },
      { "TestInterval", "getters06", &ibex_tests::TestInterval::getters06 },
      { "TestInterval", "distance01", &ibex_tests::TestInterval::distance01 },
      { "TestInterval", "distance02", &ibex_tests::TestInterval::distance02 },
      { "TestInterval", "distance03", &ibex_tests::TestInterval::distance03 },
      { "TestInterval", "distance04", &ibex_tests::TestInterval::distance04 },
      { "TestInterval", "distance05", &ibex_tests::TestInterval::distance05 },
      { "TestInterval", "distance06", &ibex_tests::TestInterval::distance06 },
      { "TestInterval", "distance07", &ibex_tests::TestInterval::distance07 },
      { "TestInterval", "distance08", &ibex_tests::TestInterval::distance08 },
      { "TestInterval", "distance09", &ibex_tests::TestInterval::distance09 },
      { "TestInterval", "distance10", &ibex_tests::TestInterval::distance10 },
      { "TestInterval", "distance11", &ibex_tests::TestInterval::distance11 },
      { "TestInterval", "distance12", &ibex_tests::TestInterval::distance12 },
      { "TestInterval", "distance13", &ibex_tests::TestInterval::distance13 },
      { "TestInterval", "distance14", &ibex_tests::TestInterval::distance14 },
      { "TestInterval", "distance15", &ibex_tests::TestInterval::distance15 },
      { "TestInterval", "distance15b", &ibex_tests::TestInterval::distance15b },
      { "TestInterval", "distance16", &ibex_tests::TestInterval::distance16 },
      { "TestInterval", "distance17", &ibex_tests::TestInterval::distance17 },
      { "TestInterval", "distance18", &ibex_tests::TestInterval::distance18 },
      { "TestInterval", "distance19", &ibex_tests::TestInterval::distance19 },
      { "TestInterval", "distance22", &ibex_tests::TestInterval::distance22 },
      { "TestInterval", "distance23", &ibex_tests::TestInterval::distance23 },
      { "TestInterval", "is_subset01", &ibex_tests::TestInterval::is_subset01 },
      { "TestInterval", "is_subset02", &ibex_tests::TestInterval::is_subset02 },
      { "TestInterval", "is_strict_interior_subset01", &ibex_tests::TestInterval::is_strict_interior_subset01 },
      { "TestInterval", "is_strict_interior_subset02", &ibex_tests::TestInterval::is_strict_interior_subset02 },
      { "TestInterval", "is_strict_interior_subset03", &ibex_tests::TestInterval::is_strict_interior_subset03 },
      { "TestInterval", "is_superset01", &ibex_tests::TestInterval::is_superset01 },
      { "TestInterval", "is_superset02", &ibex_tests::TestInterval::is_superset02 },
      { "TestInterval", "is_strict_superset01", &ibex_tests::TestInterval::is_strict_superset01 },
      { "TestInterval", "is_strict_superset02", &ibex_tests::TestInterval::is_strict_superset02 },
      { "TestInterval", "is_strict_superset03", &ibex_tests::TestInterval::is_strict_superset03 },
      { "TestInterval", "contains01", &ibex_tests::TestInterval::contains01 },
      { "TestInterval", "contains02", &ibex_tests::TestInterval::contains02 },
      { "TestInterval", "interior_contains01", &ibex_tests::TestInterval::interior_contains01 },
      { "TestInterval", "interior_contains02", &ibex_tests::TestInterval::interior_contains02 },
      { "TestInterval", "is_disjoint01", &ibex_tests::TestInterval::is_disjoint01 },
      { "TestInterval", "is_disjoint02", &ibex_tests::TestInterval::is_disjoint02 },
      { "TestInterval", "is_empty01", &ibex_tests::TestInterval::is_empty01 },
      { "TestInterval", "is_empty02", &ibex_tests::TestInterval::is_empty02 },
      { "TestInterval", "is_degenerated01", &ibex_tests::TestInterval::is_degenerated01 },
      { "TestInterval", "is_degenerated02", &ibex_tests::TestInterval::is_degenerated02 },
      { "TestInterval", "is_unbounded01", &ibex_tests::TestInterval::is_unbounded01 },
      { "TestInterval", "is_unbounded02", &ibex_tests::TestInterval::is_unbounded02 },
      { "TestInterval", "is_unbounded03", &ibex_tests::TestInterval::is_unbounded03 },
      { "TestInterval", "is_unbounded04", &ibex_tests::TestInterval::is_unbounded04 },
      { "TestInterval", "is_bisectable01", &ibex_tests::TestInterval::is_bisectable01 },
      { "TestInterval", "is_bisectable02", &ibex_tests::TestInterval::is_bisectable02 },
      { "TestInterval", "is_bisectable03", &ibex_tests::TestInterval::is_bisectable03 },
      { "TestInterval", "is_bisectable04", &ibex_tests::TestInterval::is_bisectable04 },
      { "TestInterval", "is_bisectable05", &ibex_tests::TestInterval::is_bisectable05 },
      { "TestInterval", "is_bisectable06", &ibex_tests::TestInterval::is_bisectable06 },
      { "TestInterval", "is_bisectable07", &ibex_tests::TestInterval::is_bisectable07 },
      { "TestInterval", "is_bisectable08", &ibex_tests::TestInterval::is_bisectable08 },
      { "TestInterval", "is_bisectable09", &ibex_tests::TestInterval::is_bisectable09 },
      { "TestInterval", "is_bisectable10", &ibex_tests::TestInterval::is_bisectable10 },
      { "TestInterval", "mid01", &ibex_tests::TestInterval::mid01 },
      { "TestInterval", "mid02", &ibex_tests::TestInterval::mid02 },
      { "TestInterval", "mid03", &ibex_tests::TestInterval::mid03 },
      { "TestInterval", "mid04", &ibex_tests::TestInterval::mid04 },
      { "TestInterval", "mid05", &ibex_tests::TestInterval::mid05 },
      { "TestInterval", "mid06", &ibex_tests::TestInterval::mid06 },
      { "TestInterval", "mid07", &ibex_tests::TestInterval::mid07 },
      { "TestInterval", "mid08", &ibex_tests::TestInterval::mid08 },
      { "TestInterval", "issue_463", &ibex_tests::TestInterval::issue_463 },
      { "TestArith", "tan_issue248", &ibex_tests::TestArith::tan_issue248 },
      { "TestArith", "float01", &ibex_tests::TestArith::float01 },
      { "TestArith", "float02", &ibex_tests::TestArith::float02 },
      { "TestArith", "float03", &ibex_tests::TestArith::float03 },
      { "TestArith", "float04", &ibex_tests::TestArith::float04 },
      { "TestArith", "float05", &ibex_tests::TestArith::float05 },
      { "TestArith", "float06", &ibex_tests::TestArith::float06 },
      { "TestArith", "float07", &ibex_tests::TestArith::float07 },
      { "TestArith", "float08", &ibex_tests::TestArith::float08 },
      { "TestArith", "float09", &ibex_tests::TestArith::float09 },
      { "TestArith", "minus01", &ibex_tests::TestArith::minus01 },
      { "TestArith", "minus02", &ibex_tests::TestArith::minus02 },
      { "TestArith", "minus03", &ibex_tests::TestArith::minus03 },
      { "TestArith", "minus04", &ibex_tests::TestArith::minus04 },
      { "TestArith", "add01", &ibex_tests::TestArith::add01 },
      { "TestArith", "add02", &ibex_tests::TestArith::add02 },
      { "TestArith", "add03", &ibex_tests::TestArith::add03 },
      { "TestArith", "add04", &ibex_tests::TestArith::add04 },
      { "TestArith", "add05", &ibex_tests::TestArith::add05 },
      { "TestArith", "add06", &ibex_tests::TestArith::add06 },
      { "TestArith", "add07", &ibex_tests::TestArith::add07 },
      { "TestArith", "add08", &ibex_tests::TestArith::add08 },
      { "TestArith", "add09", &ibex_tests::TestArith::add09 },
      { "TestArith", "add10", &ibex_tests::TestArith::add10 },
      { "TestArith", "add11", &ibex_tests::TestArith::add11 },
      { "TestArith", "add12", &ibex_tests::TestArith::add12 },
      { "TestArith", "add13", &ibex_tests::TestArith::add13 },
      { "TestArith", "add14", &ibex_tests::TestArith::add14 },
      { "TestArith", "add15", &ibex_tests::TestArith::add15 },
      { "TestArith", "add16", &ibex_tests::TestArith::add16 },
      { "TestArith", "add17", &ibex_tests::TestArith::add17 },
      { "TestArith", "add18", &ibex_tests::TestArith::add18 },
      { "TestArith", "add19", &ibex_tests::TestArith::add19 },
      { "TestArith", "mul01", &ibex_tests::TestArith::mul01 },
      { "TestArith", "mul02", &ibex_tests::TestArith::mul02 },
      { "TestArith", "mul03", &ibex_tests::TestArith::mul03 },
      { "TestArith", "mul04", &ibex_tests::TestArith::mul04 },
      { "TestArith", "mul05", &ibex_tests::TestArith::mul05 },
      { "TestArith", "mul06", &ibex_tests::TestArith::mul06 },
      { "TestArith", "mul07", &ibex_tests::TestArith::mul07 },
      { "TestArith", "mul08", &ibex_tests::TestArith::mul08 },
      { "TestArith", "mul09", &ibex_tests::TestArith::mul09 },
      { "TestArith", "mul10", &ibex_tests::TestArith::mul10 },
      { "TestArith", "mul11", &ibex_tests::TestArith::mul11 },
      { "TestArith", "mul12", &ibex_tests::TestArith::mul12 },
      { "TestArith", "mul13", &ibex_tests::TestArith::mul13 },
      { "TestArith", "mul14", &ibex_tests::TestArith::mul14 },
      { "TestArith", "mul15", &ibex_tests::TestArith::mul15 },
      { "TestArith", "mul16", &ibex_tests::TestArith::mul16 },
      { "TestArith", "mul17", &ibex_tests::TestArith::mul17 },
      { "TestArith", "mul18", &ibex_tests::TestArith::mul18 },
      { "TestArith", "mul19", &ibex_tests::TestArith::mul19 },
      { "TestArith", "div01", &ibex_tests::TestArith::div01 },
      { "TestArith", "div02", &ibex_tests::TestArith::div02 },
      { "TestArith", "div03", &ibex_tests::TestArith::div03 },
      { "TestArith", "div04", &ibex_tests::TestArith::div04 },
      { "TestArith", "div05", &ibex_tests::TestArith::div05 },
      { "TestArith", "div06", &ibex_tests::TestArith::div06 },
      { "TestArith", "div07", &ibex_tests::TestArith::div07 },
      { "TestArith", "div08", &ibex_tests::TestArith::div08 },
      { "TestArith", "div09", &ibex_tests::TestArith::div09 },
      { "TestArith", "div10", &ibex_tests::TestArith::div10 },
      { "TestArith", "div11", &ibex_tests::TestArith::div11 },
      { "TestArith", "div12", &ibex_tests::TestArith::div12 },
      { "TestArith", "div13", &ibex_tests::TestArith::div13 },
      { "TestArith", "div14", &ibex_tests::TestArith::div14 },
      { "TestArith", "div15", &ibex_tests::TestArith::div15 },
      { "TestArith", "div16", &ibex_tests::TestArith::div16 },
      { "TestArith", "div17", &ibex_tests::TestArith::div17 },
      { "TestArith", "div2_01", &ibex_tests::TestArith::div2_01 },
      { "TestArith", "div2_02", &ibex_tests::TestArith::div2_02 },
      { "TestArith", "div2_03", &ibex_tests::TestArith::div2_03 },
      { "TestArith", "div2_04", &ibex_tests::TestArith::div2_04 },
      { "TestArith", "div2_05", &ibex_tests::TestArith::div2_05 },
      { "TestArith", "div2_06", &ibex_tests::TestArith::div2_06 },
      { "TestArith", "div2_07", &ibex_tests::TestArith::div2_07 },
      { "TestArith", "div2_08", &ibex_tests::TestArith::div2_08 },
      { "TestArith", "div2_09", &ibex_tests::TestArith::div2_09 },
      { "TestArith", "div2_10", &ibex_tests::TestArith::div2_10 },
      { "TestArith", "div2_11", &ibex_tests::TestArith::div2_11 },
      { "TestArith", "div2_12", &ibex_tests::TestArith::div2_12 },
      { "TestArith", "div2_13", &ibex_tests::TestArith::div2_13 },
      { "TestArith", "log01", &ibex_tests::TestArith::log01 },
      { "TestArith", "log02", &ibex_tests::TestArith::log02 },
      { "TestArith", "log03", &ibex_tests::TestArith::log03 },
      { "TestArith", "log04", &ibex_tests::TestArith::log04 },
      { "TestArith", "log05", &ibex_tests::TestArith::log05 },
      { "TestArith", "log06", &ibex_tests::TestArith::log06 },
      { "TestArith", "log07", &ibex_tests::TestArith::log07 },
      { "TestArith", "log08", &ibex_tests::TestArith::log08 },
      { "TestArith", "log09", &ibex_tests::TestArith::log09 },
      { "TestArith", "log10", &ibex_tests::TestArith::log10 },
      { "TestArith", "log11", &ibex_tests::TestArith::log11 },
      { "TestArith", "exp01", &ibex_tests::TestArith::exp01 },
      { "TestArith", "exp02", &ibex_tests::TestArith::exp02 },
      { "TestArith", "exp03", &ibex_tests::TestArith::exp03 },
      { "TestArith", "exp04", &ibex_tests::TestArith::exp04 },
      { "TestArith", "exp05", &ibex_tests::TestArith::exp05 },
      { "TestArith", "exp06", &ibex_tests::TestArith::exp06 },
      { "TestArith", "exp07", &ibex_tests::TestArith::exp07 },
      { "TestArith", "exp08", &ibex_tests::TestArith::exp08 },
      { "TestArith", "exp09", &ibex_tests::TestArith::exp09 },
      { "TestArith", "sin01", &ibex_tests::TestArith::sin01 },
      { "TestArith", "sin02", &ibex_tests::TestArith::sin02 },
      { "TestArith", "sin03", &ibex_tests::TestArith::sin03 },
      { "TestArith", "sin04", &ibex_tests::TestArith::sin04 },
      { "TestArith", "sin05", &ibex_tests::TestArith::sin05 },
      { "TestArith", "sin06", &ibex_tests::TestArith::sin06 },
      { "TestArith", "sin07", &ibex_tests::TestArith::sin07 },
      { "TestArith", "sin08", &ibex_tests::TestArith::sin08 },
      { "TestArith", "sin09", &ibex_tests::TestArith::sin09 },
      { "TestArith", "sin10", &ibex_tests::TestArith::sin10 },
      { "TestArith", "sin11", &ibex_tests::TestArith::sin11 },
      { "TestArith", "sin12", &ibex_tests::TestArith::sin12 },
      { "TestArith", "sin13", &ibex_tests::TestArith::sin13 },
      { "TestArith", "tan01", &ibex_tests::TestArith::tan01 },
      { "TestArith", "tan02", &ibex_tests::TestArith::tan02 },
      { "TestArith", "tan03", &ibex_tests::TestArith::tan03 },
      { "TestArith", "tan04", &ibex_tests::TestArith::tan04 },
      { "TestArith", "tan05", &ibex_tests::TestArith::tan05 },
      { "TestArith", "pow01", &ibex_tests::TestArith::pow01 },
      { "TestArith", "pow02", &ibex_tests::TestArith::pow02 },
      { "TestArith", "pow03", &ibex_tests::TestArith::pow03 },
      { "TestArith", "pow04", &ibex_tests::TestArith::pow04 },
      { "TestArith", "pow05", &ibex_tests::TestArith::pow05 },
      { "TestArith", "pow06", &ibex_tests::TestArith::pow06 },
      { "TestArith", "pow07", &ibex_tests::TestArith::pow07 },
      { "TestArith", "pow08", &ibex_tests::TestArith::pow08 },
      { "TestArith", "pow09", &ibex_tests::TestArith::pow09 },
      { "TestArith", "pow10", &ibex_tests::TestArith::pow10 },
      { "TestArith", "pow11", &ibex_tests::TestArith::pow11 },
      { "TestArith", "pow12", &ibex_tests::TestArith::pow12 },
      { "TestArith", "pow13", &ibex_tests::TestArith::pow13 },
      { "TestArith", "root01", &ibex_tests::TestArith::root01 },
      { "TestArith", "root02", &ibex_tests::TestArith::root02 },
      { "TestArith", "root03", &ibex_tests::TestArith::root03 },
      { "TestArith", "root04", &ibex_tests::TestArith::root04 },
      { "TestArith", "sqrt01", &ibex_tests::TestArith::sqrt01 },
      { "TestArith", "sqrt02", &ibex_tests::TestArith::sqrt02 },
      { "TestArith", "sqrt03", &ibex_tests::TestArith::sqrt03 },
      { "TestArith", "sqrt04", &ibex_tests::TestArith::sqrt04 },
      { "TestArith", "sqrt05", &ibex_tests::TestArith::sqrt05 },
      { "TestArith", "sinh01", &ibex_tests::TestArith::sinh01 },
      { "TestArith", "sinh02", &ibex_tests::TestArith::sinh02 },
      { "TestArith", "sinh03", &ibex_tests::TestArith::sinh03 },
      { "TestArith", "sinh04", &ibex_tests::TestArith::sinh04 },
      { "TestArith", "sinh05", &ibex_tests::TestArith::sinh05 },
      { "TestArith", "sinh06", &ibex_tests::TestArith::sinh06 },
      { "TestArith", "sinh07", &ibex_tests::TestArith::sinh07 },
      { "TestArith", "cosh01", &ibex_tests::TestArith::cosh01 },
      { "TestArith", "cosh02", &ibex_tests::TestArith::cosh02 },
      { "TestArith", "cosh03", &ibex_tests::TestArith::cosh03 },
      { "TestArith", "cosh04", &ibex_tests::TestArith::cosh04 },
      { "TestArith", "cosh05", &ibex_tests::TestArith::cosh05 },
      { "TestArith", "cosh06", &ibex_tests::TestArith::cosh06 },
      { "TestArith", "cosh07", &ibex_tests::TestArith::cosh07 },
      { "TestArith", "atan2_01", &ibex_tests::TestArith::atan2_01 },
      { "TestArith", "atan2_02", &ibex_tests::TestArith::atan2_02 },
      { "TestArith", "atan2_03", &ibex_tests::TestArith::atan2_03 },
      { "TestArith", "atan2_04", &ibex_tests::TestArith::atan2_04 },
      { "TestArith", "atan2_05", &ibex_tests::TestArith::atan2_05 },
      { "TestArith", "atan2_06", &ibex_tests::TestArith::atan2_06 },
      { "TestArith", "atan2_07", &ibex_tests::TestArith::atan2_07 },
      { "TestArith", "atan2_08", &ibex_tests::TestArith::atan2_08 },
      { "TestArith", "atan2_09", &ibex_tests::TestArith::atan2_09 },
      { "TestArith", "atan2_10", &ibex_tests::TestArith::atan2_10 },
      { "TestArith", "atan2_11", &ibex_tests::TestArith::atan2_11 },
      { "TestArith", "atan2_12", &ibex_tests::TestArith::atan2_12 },
      { "TestArith", "atan2_13", &ibex_tests::TestArith::atan2_13 },
      { "TestArith", "atan2_14", &ibex_tests::TestArith::atan2_14 },
      { "TestArith", "atan2_15", &ibex_tests::TestArith::atan2_15 },
      { "TestArith", "floor01", &ibex_tests::TestArith::floor01 },
      { "TestArith", "floor02", &ibex_tests::TestArith::floor02 },
      { "TestArith", "floor03", &ibex_tests::TestArith::floor03 },
      { "TestArith", "floor04", &ibex_tests::TestArith::floor04 },
      { "TestArith", "floor05", &ibex_tests::TestArith::floor05 },
      { "TestArith", "floor06", &ibex_tests::TestArith::floor06 },
      { "TestArith", "floor07", &ibex_tests::TestArith::floor07 },
      { "TestArith", "floor08", &ibex_tests::TestArith::floor08 },
      { "TestArith", "ceil01", &ibex_tests::TestArith::ceil01 },
      { "TestArith", "ceil02", &ibex_tests::TestArith::ceil02 },
      { "TestArith", "ceil03", &ibex_tests::TestArith::ceil03 },
      { "TestArith", "ceil04", &ibex_tests::TestArith::ceil04 },
      { "TestArith", "ceil05", &ibex_tests::TestArith::ceil05 },
      { "TestArith", "ceil06", &ibex_tests::TestArith::ceil06 },
      { "TestArith", "ceil07", &ibex_tests::TestArith::ceil07 },
      { "TestArith", "ceil08", &ibex_tests::TestArith::ceil08 },
      { "TestArith", "integer01", &ibex_tests::TestArith::integer01 },
      { "TestArith", "integer02", &ibex_tests::TestArith::integer02 },
      { "TestArith", "integer03", &ibex_tests::TestArith::integer03 },
      { "TestArith", "integer04", &ibex_tests::TestArith::integer04 },
      { "TestArith", "integer05", &ibex_tests::TestArith::integer05 },
      { "TestArith", "integer06", &ibex_tests::TestArith::integer06 },
      { "TestArith", "integer07", &ibex_tests::TestArith::integer07 },
      { "TestArith", "integer08", &ibex_tests::TestArith::integer08 },
      { "TestArith", "bwd_mul01", &ibex_tests::TestArith::bwd_mul01 },
      { "TestArith", "bwd_mul02", &ibex_tests::TestArith::bwd_mul02 },
      { "TestArith", "bwd_mul03", &ibex_tests::TestArith::bwd_mul03 },
      { "TestArith", "bwd_mul04", &ibex_tests::TestArith::bwd_mul04 },
      { "TestArith", "bwd_mul05", &ibex_tests::TestArith::bwd_mul05 },
      { "TestArith", "bwd_maxmin01", &ibex_tests::TestArith::bwd_maxmin01 },
      { "TestArith", "bwd_maxmin02", &ibex_tests::TestArith::bwd_maxmin02 },
      { "TestArith", "bwd_maxmin03", &ibex_tests::TestArith::bwd_maxmin03 },
      { "TestArith", "bwd_maxmin04", &ibex_tests::TestArith::bwd_maxmin04 },
      { "TestArith", "bwd_maxmin05", &ibex_tests::TestArith::bwd_maxmin05 },
      { "TestArith", "bwd_sqr01", &ibex_tests::TestArith::bwd_sqr01 },
      { "TestArith", "bwd_sqr02", &ibex_tests::TestArith::bwd_sqr02 },
      { "TestArith", "bwd_sqr03", &ibex_tests::TestArith::bwd_sqr03 },
      { "TestArith", "bwd_sqr04", &ibex_tests::TestArith::bwd_sqr04 },
      { "TestArith", "bwd_sqr05", &ibex_tests::TestArith::bwd_sqr05 },
      { "TestArith", "bwd_sqr06", &ibex_tests::TestArith::bwd_sqr06 },
      { "TestArith", "bwd_sqr07", &ibex_tests::TestArith::bwd_sqr07 },
      { "TestArith", "bwd_log01", &ibex_tests::TestArith::bwd_log01 },
      { "TestArith", "bwd_log02", &ibex_tests::TestArith::bwd_log02 },
      { "TestArith", "bwd_log03", &ibex_tests::TestArith::bwd_log03 },
      { "TestArith", "bwd_log04", &ibex_tests::TestArith::bwd_log04 },
      { "TestArith", "bwd_log05", &ibex_tests::TestArith::bwd_log05 },
      { "TestArith", "bwd_log06", &ibex_tests::TestArith::bwd_log06 },
      { "TestArith", "bwd_log07", &ibex_tests::TestArith::bwd_log07 },
      { "TestArith", "bwd_log08", &ibex_tests::TestArith::bwd_log08 },
      { "TestArith", "bwd_sin01", &ibex_tests::TestArith::bwd_sin01 },
      { "TestArith", "bwd_sin02", &ibex_tests::TestArith::bwd_sin02 },
      { "TestArith", "bwd_sin03", &ibex_tests::TestArith::bwd_sin03 },
      { "TestArith", "bwd_sin04", &ibex_tests::TestArith::bwd_sin04 },
      { "TestArith", "bwd_sin05", &ibex_tests::TestArith::bwd_sin05 },
      { "TestArith", "bwd_sin06", &ibex_tests::TestArith::bwd_sin06 },
      { "TestArith", "bwd_sin07", &ibex_tests::TestArith::bwd_sin07 },
      { "TestArith", "bwd_sin08", &ibex_tests::TestArith::bwd_sin08 },
      { "TestArith", "bwd_sin09", &ibex_tests::TestArith::bwd_sin09 },
      { "TestArith", "bwd_sin10", &ibex_tests::TestArith::bwd_sin10 },
      { "TestArith", "bwd_sin11", &ibex_tests::TestArith::bwd_sin11 },
      { "TestArith", "bwd_sin12", &ibex_tests::TestArith::bwd_sin12 },
      { "TestArith", "bwd_sin13", &ibex_tests::TestArith::bwd_sin13 },
      { "TestArith", "bwd_sin14", &ibex_tests::TestArith::bwd_sin14 },
      { "TestArith", "bwd_sin15", &ibex_tests::TestArith::bwd_sin15 },
      { "TestArith", "bwd_sin16", &ibex_tests::TestArith::bwd_sin16 },
      { "TestArith", "bwd_sin17", &ibex_tests::TestArith::bwd_sin17 },
      { "TestArith", "bwd_sin18", &ibex_tests::TestArith::bwd_sin18 },
      { "TestArith", "bwd_sin19", &ibex_tests::TestArith::bwd_sin19 },
      { "TestArith", "bwd_sin20", &ibex_tests::TestArith::bwd_sin20 },
      { "TestArith", "bwd_sin21", &ibex_tests::TestArith::bwd_sin21 },
      { "TestArith", "bwd_sin22", &ibex_tests::TestArith::bwd_sin22 },
      { "TestArith", "bwd_sin23", &ibex_tests::TestArith::bwd_sin23 },
      { "TestArith", "bwd_sin24", &ibex_tests::TestArith::bwd_sin24 },
      { "TestArith", "bwd_pow01", &ibex_tests::TestArith::bwd_pow01 },
      { "TestArith", "bwd_pow02", &ibex_tests::TestArith::bwd_pow02 },
      { "TestArith", "bwd_pow03", &ibex_tests::TestArith::bwd_pow03 },
      { "TestArith", "bwd_pow04", &ibex_tests::TestArith::bwd_pow04 },
      { "TestArith", "bwd_pow05", &ibex_tests::TestArith::bwd_pow05 },
      { "TestArith", "bwd_pow06", &ibex_tests::TestArith::bwd_pow06 },
      { "TestArith", "bwd_pow07", &ibex_tests::TestArith::bwd_pow07 },
      { "TestArith", "bwd_pow08", &ibex_tests::TestArith::bwd_pow08 },
      { "TestArith", "bwd_pow09", &ibex_tests::TestArith::bwd_pow09 },
      { "TestArith", "bwd_pow10", &ibex_tests::TestArith::bwd_pow10 },
      { "TestArith", "bwd_pow11", &ibex_tests::TestArith::bwd_pow11 },
      { "TestArith", "bwd_pow12", &ibex_tests::TestArith::bwd_pow12 },
      { "TestArith", "bwd_pow13", &ibex_tests::TestArith::bwd_pow13 },
      { "TestArith", "bwd_pow14", &ibex_tests::TestArith::bwd_pow14 },
      { "TestArith", "bwd_pow15", &ibex_tests::TestArith::bwd_pow15 },
      { "TestArith", "bwd_pow16", &ibex_tests::TestArith::bwd_pow16 },
      { "TestArith", "bwd_pow17", &ibex_tests::TestArith::bwd_pow17 },
      { "TestArith", "bwd_atan2_01", &ibex_tests::TestArith::bwd_atan2_01 },
      { "TestArith", "bwd_atan2_02", &ibex_tests::TestArith::bwd_atan2_02 },
      { "TestArith", "bwd_atan2_03", &ibex_tests::TestArith::bwd_atan2_03 },
      { "TestArith", "bwd_atan2_04", &ibex_tests::TestArith::bwd_atan2_04 },
      { "TestArith", "bwd_atan2_05", &ibex_tests::TestArith::bwd_atan2_05 },
      { "TestArith", "bwd_atan2_06", &ibex_tests::TestArith::bwd_atan2_06 },
      { "TestArith", "bwd_atan2_07", &ibex_tests::TestArith::bwd_atan2_07 },
      { "TestArith", "bwd_atan2_08", &ibex_tests::TestArith::bwd_atan2_08 },
      { "TestArith", "bwd_atan2_09", &ibex_tests::TestArith::bwd_atan2_09 },
      { "TestArith", "bwd_atan2_10", &ibex_tests::TestArith::bwd_atan2_10 },
      { "TestArith", "bwd_atan2_issue134", &ibex_tests::TestArith::bwd_atan2_issue134 },
      { "TestArith", "bwd_floor_01", &ibex_tests::TestArith::bwd_floor_01 },
      { "TestArith", "bwd_floor_02", &ibex_tests::TestArith::bwd_floor_02 },
      { "TestArith", "bwd_floor_03", &ibex_tests::TestArith::bwd_floor_03 },
      { "TestArith", "bwd_floor_04", &ibex_tests::TestArith::bwd_floor_04 },
      { "TestArith", "bwd_floor_05", &ibex_tests::TestArith::bwd_floor_05 },
      { "TestArith", "bwd_floor_06", &ibex_tests::TestArith::bwd_floor_06 },
      { "TestArith", "bwd_floor_07", &ibex_tests::TestArith::bwd_floor_07 },
      { "TestArith", "bwd_floor_08", &ibex_tests::TestArith::bwd_floor_08 },
      { "TestArith", "bwd_floor_09", &ibex_tests::TestArith::bwd_floor_09 },
      { "TestArith", "bwd_floor_10", &ibex_tests::TestArith::bwd_floor_10 },
      { "TestArith", "bwd_floor_11", &ibex_tests::TestArith::bwd_floor_11 }
    };

    for (const Test& t : tests) {
      current_test() = std::string("IBEX ") + t.group + "::" + t.name;
      try {
        t.function();
      } catch (const std::exception& e) {
        gaol_tests::check(current_test() + ": no exception", false, [&] { return std::string(e.what()); });
      } catch (...) {
        gaol_tests::check(current_test() + ": no exception", false);
      }
    }
  }
} // namespace ibex_port

#undef CPPUNIT_ASSERT
#undef CPPUNIT_ASSERT_DOUBLES_EQUAL
#undef CPPUNIT_ASSERT_DOUBLES_EQUAL_MESSAGE
#undef NAME2
#undef NAME2_HIDDEN
#undef checkproj
#undef piL
#undef piU

// Tests of Codac (https://github.com/codac-team/codac) on its scalar Interval,
// a wrapper of gaol::interval, run against GAOL v5 itself.
//
// The TEST_CASEs and the helper functions below are the lines of three files of
// the branch GAOL of the fork of Codac by Jordan Ninin (commit f0d06ff0),
// copied VERBATIM, but for the lines left out and the substitutions listed
// next, which another script checks mechanically against the files:
//   - codac2_tests_Interval.cpp, identical to that of Codac 2 master (commit
//     6419940; Copyright 2024 Codac Team, authors Gilles Chabert and Simon
//     Rohou, GNU Lesser General Public License; most of these tests come from
//     the IBEX library, Gilles Chabert);
//   - codac2_tests_Interval_operations.cpp, that of master and 40 lines more,
//     the checks of pow with real exponents;
//   - codac2_tests_Interval_rounding.cpp (Copyright 2026 Codac Team, Jordan
//     Ninin, GNU Lesser General Public License).
// Each block of lines copied is between the comments ">>> file lines a-b" and
// "<<< file lines a-b", the numbers being those of the lines in the file.
//
// What the tests need of Codac is written once, in the namespace codac2 of the
// namespace codac_port: the class Interval (a value holding a gaol::interval,
// with the constructors of Codac), the functions the tests call, Approx, the
// BoolInterval that < and > of the intervals return, and the three macros of
// Catch2 the tests use: TEST_CASE, CHECK and INFO (CHECK_THROWS_AS, REQUIRE
// and SECTION are not used by the checks run here). Each function calls the
// native function of GAOL v5: where the wrapper of Codac adds logic of its own
// on top of GAOL (sqrt, log, root, atan2, cosh, asinh, sign, pow with an
// infinite exponent, the operations with an infinite double, operator-,
// operator&, mid, rad, diam, interior_contains, is_subset, is_disjoint,
// inflate, is_bisectable, bisect, the order < and >), GAOL v5 handles the
// cases itself and its function is called. The calls of gaol::round_upward()
// that the wrapper makes after most of the functions are not made, so that the
// tests of the rounding direction check GAOL, not the wrapper.
// CHECK reports through gaol_tests::check() (gaol_tests.h, to be included
// before this section): the name of a check is "Codac <file> <TEST_CASE>", and
// its description the expression, its line in this file, its number in the
// TEST_CASE and the INFO messages in scope.
//
// Textual substitutions applied to the lines of Codac (the only ones):
//   BoolInterval::TRUE, FALSE, EMPTY, UNKNOWN -> BoolInterval::True, False,
//   Empty, Unknown (TRUE and FALSE are macros of <windows.h>).
// The lines of Codac that are replaced, not copied: the #include lines, and
// "using namespace std; using namespace codac2;" (the namespace of each file
// below does the same). Around the last TEST_CASE of the rounding file, a
// #pragma warning for Visual C++ (C4456 and C4457: the parameters x and y of
// its lambdas hide the locals x and y).
//
// Left out, because Codac adds them to GAOL or tests Codac itself and not GAOL
// (the comments "[not ported, ...]" in the blocks say where): in
// codac2_tests_Interval.cpp the constructors from an initializer list, the
// literal _i, init_from_list() and set_empty(), smag(), rand(), is_strict_
// interior_subset(), is_strict_superset(), diff(), and the #if 0 block; in
// codac2_tests_Interval_rounding.cpp rand() and the ternary chi(x,y,z) of
// Codac; the tests of the Python bindings.
//
// Requires gaol_tests.h, and a main() that calls gaol::init() before
// codac_port::run(), and sets the rounding direction upward after it.

#include <cfenv>
#include <cmath>
#include <cstdint>
#include <cstring>
#include <exception>
#include <limits>
#include <math.h>
#include <ostream>
#include <sstream>
#include <string>
#include <type_traits>
#include <utility>
#include <vector>

#include <gaol/gaol>

// The macros of the section take the place of those of the same names that
// another section may have defined, which are put back at its end; min and max
// are macros of <windows.h>, and the names of functions of Codac
#pragma push_macro("TEST_CASE")
#pragma push_macro("CHECK")
#pragma push_macro("INFO")
#pragma push_macro("min")
#pragma push_macro("max")
#undef TEST_CASE
#undef CHECK
#undef INFO
#undef min
#undef max

#define CODAC_PORT_CAT_(a, b) a##b
#define CODAC_PORT_CAT(a, b) CODAC_PORT_CAT_(a, b)

// TEST_CASE("name") { ... } defines a function and the constant name, which the
// table at the end of the section names (a registration by an object of the
// namespace, which runs code before main(), is not wanted)
#define CODAC_PORT_TEST_CASE_2(id, name) \
  static constexpr const char* const CODAC_PORT_CAT(id, _name) = name; \
  static void id()
#define CODAC_PORT_TEST_CASE_1(id, name) CODAC_PORT_TEST_CASE_2(id, name)
#define TEST_CASE(name) CODAC_PORT_TEST_CASE_1(CODAC_PORT_TEST_ID, name)

// CHECK(expression): an exception thrown by the expression is a failure
#define CHECK(...) \
  ::codac_port::catch2::check(::codac_port::catch2::expression([&]() -> bool { return !!(__VA_ARGS__); }), #__VA_ARGS__, __LINE__)

// INFO(message): a message streamed with <<, kept until the end of the scope
#define INFO(...) \
  const ::codac_port::catch2::Info CODAC_PORT_CAT(codac_port_info_, __COUNTER__)(::codac_port::catch2::Message() << __VA_ARGS__)

namespace codac_port
{
  //---------------------------------------------------------------------------
  // The part of Catch2 the tests use
  //---------------------------------------------------------------------------
  namespace catch2
  {
    struct TestCase
    {
      const char* file;
      const char* name;
      void (*run)();
    };

    inline const TestCase*& running()
    {
      static const TestCase* t = nullptr;
      return t;
    }

    // The messages of INFO in scope
    inline std::vector<std::string>& messages()
    {
      static std::vector<std::string> m;
      return m;
    }

    class Message
    {
      public:

        template<class T>
        Message& operator<<(const T& value)
        {
          stream_ << value;
          return *this;
        }

        std::string str() const
        {
          return stream_.str();
        }

      private:

        std::ostringstream stream_;
    };

    class Info
    {
      public:

        explicit Info(const Message& m)
        {
          messages().push_back(m.str());
        }

        Info(const Info&) = delete;
        Info& operator=(const Info&) = delete;

        ~Info()
        {
          messages().pop_back();
        }
    };

    // The expression of a CHECK, as a function pointer and the object it
    // calls: check() is then compiled once, not once for each CHECK
    struct Expression
    {
      const void* object;
      bool (*evaluate)(const void*);
    };

    template<class F>
    Expression expression(const F& f)
    {
      return Expression{ &f, [](const void* object) { return (*static_cast<const F*>(object))(); } };
    }

    inline bool check(const Expression& expression, const char* text, int line)
    {
      bool ok = false;
      std::string exception;
      try {
        ok = expression.evaluate(expression.object);
      } catch (const std::exception& e) {
        exception = std::string("exception: ") + e.what();
      } catch (...) {
        exception = "unknown exception";
      }
      const TestCase& t = *running();
      const std::string name = std::string("Codac ") + t.file + " " + t.name;
      return gaol_tests::check(name, ok, [&] {
        std::string d = std::string(text) + " (line " + std::to_string(line) + " of this file, check "
          + std::to_string(gaol_tests::tallies()[name].checks) + " of the TEST_CASE)";
        if (!exception.empty()) {
          d += ": " + exception;
        }
        for (const std::string& m : messages()) {
          d += "; " + m;
        }
        return d;
      });
    }

    // Runs a TEST_CASE: an exception it does not catch is a failure
    inline void run(const TestCase& t)
    {
      running() = &t;
      try {
        t.run();
      } catch (const std::exception& e) {
        gaol_tests::check(std::string("Codac ") + t.file + " " + t.name, false,
                          [&] { return std::string("exception: ") + e.what(); });
      } catch (...) {
        gaol_tests::check(std::string("Codac ") + t.file + " " + t.name, false,
                          [] { return std::string("unknown exception"); });
      }
      running() = nullptr;
    }
  }

  //---------------------------------------------------------------------------
  // The part of Codac the tests use: Codac's names, GAOL's functions
  //---------------------------------------------------------------------------
  namespace codac2
  {
    // Codac initializes oo with gaol::round_upward(): run() does
    const double oo = std::numeric_limits<double>::infinity();

    // The four-valued result of < and > (BoolInterval of Codac)
    enum class BoolInterval { Empty, False, True, Unknown };

    // The interval of Codac: a gaol::interval, with the constructors of Codac
    // (a double converts to it; an infinite one gives the empty set, as in GAOL
    // v5), and the operations that call the native functions of GAOL v5
    class Interval
    {
      public:

        Interval()
          : v_() // [-oo,oo]
        { }

        Interval(double a)
          : v_(a) // [a], and the empty set for an infinite a
        { }

        Interval(double a, double b)
          : v_(a, b) // [a,b], and the empty set for a > b
        { }

        explicit Interval(const gaol::interval& x)
          : v_(x)
        { }

        const gaol::interval& native() const
        {
          return v_;
        }

        double lb() const
        {
          return v_.left();
        }

        double ub() const
        {
          return v_.right();
        }

        double mid() const
        {
          return v_.midpoint();
        }

        double rad() const
        {
          return v_.rad();
        }

        double diam() const
        {
          return v_.width();
        }

        double mag() const
        {
          return v_.mag();
        }

        double mig() const
        {
          return v_.mig();
        }

        double smig() const
        {
          return v_.smig();
        }

        bool is_empty() const
        {
          return v_.is_empty();
        }

        bool contains(const double& x) const
        {
          return v_.set_contains(x);
        }

        bool interior_contains(const double& x) const
        {
          return v_.set_strictly_contains(x);
        }

        bool is_unbounded() const
        {
          return !v_.is_finite();
        }

        // The empty set is degenerated for Codac, and is_a_double() is false for it
        bool is_degenerated() const
        {
          return v_.is_empty() || v_.is_a_double();
        }

        bool is_disjoint(const Interval& x) const
        {
          return v_.set_disjoint(x.v_);
        }

        bool is_subset(const Interval& x) const
        {
          return x.v_.set_contains(v_);
        }

        bool is_superset(const Interval& x) const
        {
          return v_.set_contains(x.v_);
        }

        bool is_bisectable() const
        {
          return v_.is_bisectable();
        }

        // The ratio is a float in Codac
        std::pair<Interval, Interval> bisect(double ratio = 0.49) const
        {
          const std::pair<gaol::interval, gaol::interval> p = v_.bisect(static_cast<double>(static_cast<float>(ratio)));
          return std::pair<Interval, Interval>(Interval(p.first), Interval(p.second));
        }

        Interval& inflate(const double& r)
        {
          v_ = v_.inflate(r);
          return *this;
        }

        Interval& operator|=(const Interval& x)
        {
          v_ |= x.v_;
          return *this;
        }

        Interval& operator&=(const Interval& x)
        {
          v_ &= x.v_;
          return *this;
        }

        Interval& operator+=(double x)
        {
          v_ += x;
          return *this;
        }

        Interval& operator+=(const Interval& x)
        {
          v_ += x.v_;
          return *this;
        }

        Interval operator-() const
        {
          return Interval(-v_);
        }

        Interval& operator-=(double x)
        {
          v_ -= x;
          return *this;
        }

        Interval& operator-=(const Interval& x)
        {
          v_ -= x.v_;
          return *this;
        }

        Interval& operator*=(double x)
        {
          v_ *= x;
          return *this;
        }

        Interval& operator*=(const Interval& x)
        {
          v_ *= x.v_;
          return *this;
        }

        Interval& operator/=(double x)
        {
          v_ /= x;
          return *this;
        }

        Interval& operator/=(const Interval& x)
        {
          v_ /= x.v_;
          return *this;
        }

        // The sets are equal: both empty, or the same bounds (set_eq of IEEE 1788-2015)
        friend bool operator==(const Interval& x, const Interval& y)
        {
          return x.v_.set_eq(y.v_);
        }

        friend bool operator!=(const Interval& x, const Interval& y)
        {
          return x.v_.set_neq(y.v_);
        }

        // Empty if one is; true if every t of this is below every s of x
        // (certainly less, strictPrecedes of IEEE 1788-2015); false if no t is
        // below an s (certainly greater or equal); unknown otherwise
        BoolInterval operator<(const Interval& x) const
        {
          if (v_.is_empty() || x.v_.is_empty()) {
            return BoolInterval::Empty;
          }
          if (v_ < x.v_) {
            return BoolInterval::True;
          }
          if (v_ >= x.v_) {
            return BoolInterval::False;
          }
          return BoolInterval::Unknown;
        }

        BoolInterval operator>(const Interval& x) const
        {
          return x < *this;
        }

        static Interval empty()
        {
          return Interval(gaol::interval::emptyset());
        }

        static Interval zero()
        {
          return Interval(gaol::interval::zero());
        }

        static Interval one()
        {
          return Interval(gaol::interval::one());
        }

        static Interval half_pi()
        {
          return Interval(gaol::interval::half_pi());
        }

        static Interval pi()
        {
          return Interval(gaol::interval::pi());
        }

        static Interval two_pi()
        {
          return Interval(gaol::interval::two_pi());
        }

      private:

        gaol::interval v_;
    };

    // The output of Codac sets the precision of GAOL's to that of the stream;
    // the precision of GAOL is put back
    inline std::ostream& operator<<(std::ostream& os, const Interval& x)
    {
      const std::streamsize saved = gaol::interval::precision(os.precision());
      os << x.native();
      gaol::interval::precision(saved);
      return os;
    }

    inline Interval operator&(const Interval& x, const Interval& y)
    {
      return Interval(x.native() & y.native());
    }

    inline Interval operator|(const Interval& x, const Interval& y)
    {
      return Interval(x.native() | y.native());
    }

    inline Interval operator+(const Interval& x, double y)
    {
      return Interval(x.native() + y);
    }

    inline Interval operator+(double x, const Interval& y)
    {
      return Interval(x + y.native());
    }

    inline Interval operator+(const Interval& x, const Interval& y)
    {
      return Interval(x.native() + y.native());
    }

    inline Interval operator-(const Interval& x, double y)
    {
      return Interval(x.native() - y);
    }

    inline Interval operator-(double x, const Interval& y)
    {
      return Interval(x - y.native());
    }

    inline Interval operator-(const Interval& x, const Interval& y)
    {
      return Interval(x.native() - y.native());
    }

    inline Interval operator*(const Interval& x, double y)
    {
      return Interval(x.native() * y);
    }

    inline Interval operator*(double x, const Interval& y)
    {
      return Interval(x * y.native());
    }

    inline Interval operator*(const Interval& x, const Interval& y)
    {
      return Interval(x.native() * y.native());
    }

    inline Interval operator/(const Interval& x, double y)
    {
      return Interval(x.native() / y);
    }

    inline Interval operator/(double x, const Interval& y)
    {
      return Interval(x / y.native());
    }

    inline Interval operator/(const Interval& x, const Interval& y)
    {
      return Interval(x.native() / y.native());
    }

    inline double prev_float(double x)
    {
      return gaol::previous_float(x);
    }

    inline double next_float(double x)
    {
      return gaol::next_float(x);
    }

    // The functions on intervals: the native functions of GAOL v5, with no
    // logic of Codac before or round_upward() after. Where Codac's wrapper has
    // its own (sqrt, log, root, atan2, cosh, asinh, sign, pow with an infinite
    // exponent, operations with an infinite double, operator-, operator&), GAOL's
    // native function is called instead.
    inline Interval sqr(const Interval& x)
    {
      return Interval(gaol::sqr(x.native()));
    }

    inline Interval sqrt(const Interval& x)
    {
      return Interval(gaol::sqrt(x.native()));
    }

    inline Interval pow(const Interval& x, int p)
    {
      return Interval(gaol::pow(x.native(), p));
    }

    inline Interval pow(const Interval& x, double p)
    {
      return Interval(gaol::pow(x.native(), p));
    }

    inline Interval pow(const Interval& x, const Interval& p)
    {
      return Interval(gaol::pow(x.native(), p.native()));
    }

    // Codac: the root of the positive part, the opposite of that of the
    // opposite for an odd p and a negative x, the inverse for a negative p
    inline Interval root(const Interval& x, int p)
    {
      return Interval(gaol::nth_root(x.native(), p));
    }

    inline Interval exp(const Interval& x)
    {
      return Interval(gaol::exp(x.native()));
    }

    inline Interval log(const Interval& x)
    {
      return Interval(gaol::log(x.native()));
    }

    inline Interval cos(const Interval& x)
    {
      return Interval(gaol::cos(x.native()));
    }

    inline Interval sin(const Interval& x)
    {
      return Interval(gaol::sin(x.native()));
    }

    inline Interval tan(const Interval& x)
    {
      return Interval(gaol::tan(x.native()));
    }

    inline Interval acos(const Interval& x)
    {
      return Interval(gaol::acos(x.native()));
    }

    inline Interval asin(const Interval& x)
    {
      return Interval(gaol::asin(x.native()));
    }

    inline Interval atan(const Interval& x)
    {
      return Interval(gaol::atan(x.native()));
    }

    inline Interval atan2(const Interval& y, const Interval& x)
    {
      return Interval(gaol::atan2(y.native(), x.native()));
    }

    inline Interval cosh(const Interval& x)
    {
      return Interval(gaol::cosh(x.native()));
    }

    inline Interval sinh(const Interval& x)
    {
      return Interval(gaol::sinh(x.native()));
    }

    inline Interval tanh(const Interval& x)
    {
      return Interval(gaol::tanh(x.native()));
    }

    inline Interval acosh(const Interval& x)
    {
      return Interval(gaol::acosh(x.native()));
    }

    inline Interval asinh(const Interval& x)
    {
      return Interval(gaol::asinh(x.native()));
    }

    inline Interval atanh(const Interval& x)
    {
      return Interval(gaol::atanh(x.native()));
    }

    inline Interval abs(const Interval& x)
    {
      return Interval(gaol::abs(x.native()));
    }

    inline Interval min(const Interval& x, const Interval& y)
    {
      return Interval(gaol::min(x.native(), y.native()));
    }

    inline Interval max(const Interval& x, const Interval& y)
    {
      return Interval(gaol::max(x.native(), y.native()));
    }

    inline Interval sign(const Interval& x)
    {
      return Interval(gaol::sign(x.native()));
    }

    inline Interval integer(const Interval& x)
    {
      return Interval(gaol::integer(x.native()));
    }

    inline Interval floor(const Interval& x)
    {
      return Interval(gaol::floor(x.native()));
    }

    inline Interval ceil(const Interval& x)
    {
      return Interval(gaol::ceil(x.native()));
    }

    // Approx of Codac, for the scalar Interval and the double (codac2_Approx.h):
    // Approx(x, eps) == y holds if x and y are equal, or both nonempty with the
    // bounds equal or less than eps apart
    template<typename T>
    class Approx
    {
      public:

        explicit Approx(const T& x, double eps = std::numeric_limits<double>::epsilon()*10)
          : x_(x), eps_(eps)
        { }

        friend bool operator==(const T& x1, const Approx<T>& x2)
        {
          return x2.equal(x1);
        }

        // Codac is compiled as C++20, which reverses the operands of ==
        friend bool operator==(const Approx<T>& x2, const T& x1)
        {
          return x2.equal(x1);
        }

      private:

        bool equal(const T& x1) const
        {
          if constexpr (std::is_same<T, double>::value) {
            return std::fabs(x1 - x_) < eps_;
          } else {
            static_assert(std::is_same<T, Interval>::value, "Approx of an Interval or a double");
            if (x1 == x_) {
              return true;
            }
            if ((x1.is_empty() && !x_.is_empty()) || (!x1.is_empty() && x_.is_empty())) {
              return false;
            }
            return (x1.lb() == x_.lb() || x1.lb() == Approx<double>(x_.lb(), eps_))
              && (x1.ub() == x_.ub() || x1.ub() == Approx<double>(x_.ub(), eps_));
          }
        }

        const T x_;
        const double eps_;
    };
  }

  //---------------------------------------------------------------------------
  // codac2_tests_Interval.cpp
  //---------------------------------------------------------------------------
  namespace file_interval
  {
    // The namespaces the file opens; the functions of codac2 on an Interval are found
    // through its argument (a call sin(0.5) is that of std, as in Codac)
    using namespace std;
    using codac2::Interval;
    using codac2::Approx;
    using codac2::BoolInterval;
    using codac2::oo;

#define CODAC_PORT_TEST_ID test_case_1
// >>> codac2_tests_Interval.cpp lines 24-272
TEST_CASE("Interval - tests from IBEX")
{
  CHECK(Interval() == Interval(-oo,oo));
  CHECK(Interval(1.0) == Interval(1.0,1.0));
  CHECK(Interval(-oo) == Interval::empty());
  CHECK(Interval(oo) == Interval::empty());
  CHECK(Interval(1,0) == Interval::empty());
// [not ported, lines 31-32 of codac2_tests_Interval.cpp: initializer-list constructor Interval{...} (Codac only)]

  CHECK(Interval(-oo,oo) == Interval(-oo,oo));
  CHECK(Interval(-oo,oo) != Interval(0,oo));
  CHECK(Interval(0,1) == Interval(0,1));
  CHECK(Interval(0,2) != Interval(0,1));
  CHECK(Interval(0,2) != Interval(1,3));
  CHECK(Interval(0,oo) == Interval(0,oo));
  CHECK(Interval::empty() == Interval::empty());
  CHECK(Interval(0,0) == Interval(0,0));

// [not ported, lines 43-50 of codac2_tests_Interval.cpp: initializer-list constructor Interval({...}) (Codac only)]

// [not ported, lines 52-53 of codac2_tests_Interval.cpp: literal _i (Codac only)]

  Interval x, y, z;

// [not ported, lines 57-68 of codac2_tests_Interval.cpp: init_from_list() and CHECK_THROWS_AS of the assertion layer of Codac (Codac only)]

// [not ported, lines 70-75 of codac2_tests_Interval.cpp: set_empty() (Codac only)]

  x = Interval(0,1); y = Interval(2,3); z = Interval(0,3);
  CHECK((x|y) == z); CHECK((y|x) == z);
  CHECK((Interval(x)|=y) == z); CHECK((Interval(y)|=x) == z);

  x = Interval(0,1); y = Interval::empty(); z = Interval(0,1);
  CHECK((x|y) == z); CHECK((y|x) == z);
  CHECK((Interval(x)|=y) == z); CHECK((Interval(y)|=x) == z);

  x = Interval(-1); y = Interval(0,oo); z = Interval(-1,oo);
  CHECK((x|y) == z); CHECK((y|x) == z);
  CHECK((Interval(x)|=y) == z); CHECK((Interval(y)|=x) == z);

  x = Interval(0,1); y = Interval(2,3); z = Interval::empty();
  CHECK((x&y) == z); CHECK((y&x) == z);
  CHECK((Interval(x)&=y) == z); CHECK((Interval(y)&=x) == z);

  x = Interval(0,1); y = Interval::empty(); z = Interval::empty();
  CHECK((x&y) == z); CHECK((y&x) == z);
  CHECK((Interval(x)&=y) == z); CHECK((Interval(y)&=x) == z);

  x = Interval(0,2); y = Interval(-1,1); z = Interval(0,1);
  CHECK((x&y) == z); CHECK((y&x) == z);
  CHECK((Interval(x)&=y) == z); CHECK((Interval(y)&=x) == z);

  x = Interval(0,1); y = Interval(0,oo); z = Interval(0,1);
  CHECK((x&y) == z); CHECK((y&x) == z);
  CHECK((Interval(x)&=y) == z); CHECK((Interval(y)&=x) == z);

  x = Interval(0,1); y = Interval(-oo,0); z = Interval(0);
  CHECK((x&y) == z); CHECK((y&x) == z);
  CHECK((Interval(x)&=y) == z); CHECK((Interval(y)&=x) == z);

  x = Interval(0,1); y = Interval(1,2); z = Interval(1,1);
  CHECK((x&y) == z); CHECK((y&x) == z);
  CHECK((Interval(x)&=y) == z); CHECK((Interval(y)&=x) == z);

  x = Interval(0,2);

  CHECK(x.lb() == 0);
  CHECK(x.ub() == 2);
  CHECK(x.mid() == 1);
  CHECK(x.rad() == 1);
  CHECK(x.diam() == 2);
  CHECK(x.mag() == 2);
  CHECK(x.mig() == 0);
// [not ported, line 122 of codac2_tests_Interval.cpp: smag() (Codac only, no GAOL counterpart)]
  CHECK(x.smig() == 0);

  x = Interval(-3,-1);

  CHECK(x.lb() == -3);
  CHECK(x.ub() == -1);
  CHECK(x.mid() == -2);
  CHECK(x.rad() == 1);
  CHECK(x.diam() == 2);
  CHECK(x.mag() == 3);
  CHECK(x.mig() == 1);
// [not ported, line 134 of codac2_tests_Interval.cpp: smag() (Codac only, no GAOL counterpart)]
  CHECK(x.smig() == -1);

  x = Interval(-3,1);

  CHECK(x.lb() == -3);
  CHECK(x.ub() == 1);
  CHECK(x.mid() == -1);
  CHECK(x.rad() == 2);
  CHECK(x.diam() == 4);
  CHECK(x.mag() == 3);
  CHECK(x.mig() == 0);
// [not ported, line 146 of codac2_tests_Interval.cpp: smag() (Codac only, no GAOL counterpart)]
  CHECK(x.smig() == 0);

  x = Interval(-oo,0);

  CHECK(x.lb() == -oo);
  CHECK(x.ub() == 0);
  CHECK(x.mid() == -std::numeric_limits<double>::max());
  CHECK(x.rad() == oo);
  CHECK(x.diam() == oo);
  CHECK(x.mag() == oo);
  CHECK(x.mig() == 0);
// [not ported, line 158 of codac2_tests_Interval.cpp: smag() (Codac only, no GAOL counterpart)]
  CHECK(x.smig() == 0);

  x = Interval(-oo,oo);

  CHECK(x.lb() == -oo);
  CHECK(x.ub() == oo);
  CHECK(x.mid() == 0);
  CHECK(x.rad() == oo);
  CHECK(x.diam() == oo);
  CHECK(x.mag() == oo);
  CHECK(x.mig() == 0);
// [not ported, line 170 of codac2_tests_Interval.cpp: smag() (Codac only, no GAOL counterpart)]
  CHECK(x.smig() == 0);

  x = Interval(std::numeric_limits<double>::max(),oo);

  CHECK(x.lb() == std::numeric_limits<double>::max());
  CHECK(x.ub() == oo);
  CHECK(x.mid() == std::numeric_limits<double>::max());
  CHECK(x.rad() == oo);
  CHECK(x.diam() == oo);
  CHECK(x.mag() == oo);
  CHECK(x.mig() == std::numeric_limits<double>::max());
// [not ported, line 182 of codac2_tests_Interval.cpp: smag() (Codac only, no GAOL counterpart)]
  CHECK(x.smig() == std::numeric_limits<double>::max());

// [not ported, lines 185-198 of codac2_tests_Interval.cpp: rand() (Codac only)]

  CHECK(Interval(0,1).is_subset(Interval(0,2)));
  CHECK(!Interval(0,1).is_subset(Interval(1,2)));
// [not ported, lines 202-204 of codac2_tests_Interval.cpp: is_strict_interior_subset() (notion of Codac, no GAOL counterpart)]
  CHECK(Interval(0,2).is_superset(Interval(0,1)));
  CHECK(!Interval(1,2).is_superset(Interval(0,1)));
// [not ported, lines 207-209 of codac2_tests_Interval.cpp: is_strict_superset() (notion of Codac, no GAOL counterpart)]
  CHECK(Interval(0,2).contains(0));
  CHECK(Interval(-oo,oo).contains(oo));
  CHECK(!Interval(0,2).interior_contains(0));
  CHECK(Interval(0,2).interior_contains(1));
  CHECK(!Interval(-oo,0).is_disjoint(Interval(0,oo)));
  CHECK(Interval(0,1).is_disjoint(Interval(2,3)));
  CHECK(Interval(oo).is_empty());
  CHECK(!Interval(0,1).is_empty());
  CHECK(Interval(0).is_degenerated());
  CHECK(Interval::empty().is_degenerated());
  CHECK(!Interval::empty().is_unbounded());
  CHECK(!Interval(0,1).is_unbounded());
  CHECK(Interval(-oo,0).is_unbounded());
  CHECK(Interval(0,oo).is_unbounded());
  CHECK(!Interval::empty().is_bisectable());
  CHECK(!Interval(std::numeric_limits<double>::max(),oo).is_bisectable());
  CHECK(!Interval(-oo,-std::numeric_limits<double>::max()).is_bisectable());
  CHECK(!Interval(0,codac2::next_float(0)).is_bisectable());
  CHECK(Interval(0,codac2::next_float(codac2::next_float(0))).is_bisectable());
  CHECK(Interval(10,codac2::next_float(codac2::next_float(10))).is_bisectable());
  CHECK(Interval(codac2::prev_float(codac2::prev_float(0)),0).is_bisectable());
  CHECK(Interval(codac2::prev_float(codac2::prev_float(10)),10).is_bisectable());
  CHECK(!Interval(10,codac2::next_float(10)).is_bisectable());
  CHECK(!Interval(codac2::prev_float(0),0).is_bisectable());

  CHECK(Interval(0,2).mid() == 1.0);
  CHECK(Interval(-oo,oo).mid() == 0);
  CHECK(Interval(0,oo).mid() == std::numeric_limits<double>::max());
  CHECK(Interval(-oo,0).mid() == -std::numeric_limits<double>::max());
  double d = Interval(std::numeric_limits<double>::max(),oo).mid();
  CHECK((d == std::numeric_limits<double>::max() || d == -std::numeric_limits<double>::max()));
  CHECK(Interval(0).mid() == 0.0);

  x = Interval(2.7629416257693772963,2.7629416257693835135);
  y = x;
  double m = x.mid();
  Interval x2 = x+m;
  Interval x3 = x2-m;
  CHECK(x3.is_superset(y));

// [not ported, lines 250-266 of codac2_tests_Interval.cpp: diff() (Codac only)]

  Interval a = Interval(3,8);
  Interval b = Interval(1,3) & Interval(6,7); // [b] is empty
  Interval c = a+b;
  CHECK(c.is_empty());
}
// <<< codac2_tests_Interval.cpp lines 24-272
#undef CODAC_PORT_TEST_ID

#define CODAC_PORT_TEST_ID test_case_2
// >>> codac2_tests_Interval.cpp lines 274-296
TEST_CASE("Interval - other tests")
{
  Interval x;

  auto a = x.bisect(0.5);
  CHECK(a.first == Interval(-oo,0));
  CHECK(a.second == Interval(0,oo));

  x = Interval(0,oo);
  a = x.bisect(0.5);
  CHECK(a.first == Interval(0,codac2::prev_float(oo)));
  CHECK(a.second == Interval(codac2::prev_float(oo),oo));

  x = Interval(-oo,0);
  a = x.bisect(0.5);
  CHECK(a.first == Interval(-oo,codac2::next_float(-oo)));
  CHECK(a.second == Interval(codac2::next_float(-oo),0));

  x = Interval(0,10);
  a = x.bisect(0.2);
  CHECK(Approx(a.first,1e-7) == Interval(0,2));
  CHECK(Approx(a.second,1e-7) == Interval(2,10));
}
// <<< codac2_tests_Interval.cpp lines 274-296
#undef CODAC_PORT_TEST_ID

#define CODAC_PORT_TEST_ID test_case_3
// >>> codac2_tests_Interval.cpp lines 298-317
TEST_CASE("Interval - comparison")
{
  CHECK((Interval(0,1) < Interval(2,3)) == BoolInterval::True);
  CHECK((Interval(0,1) < Interval(1,2)) == BoolInterval::Unknown);
  CHECK((Interval(0,1) < Interval(0,1)) == BoolInterval::Unknown);
  CHECK((Interval(2,3) < Interval(0,1)) == BoolInterval::False);
  CHECK((Interval(2,3) < Interval(-oo,2)) == BoolInterval::False);
  CHECK((Interval(-oo,2) < Interval(3,oo)) == BoolInterval::True);
  CHECK((Interval::empty() < Interval(0,1)) == BoolInterval::Empty);
  CHECK((Interval(0,1) < Interval::empty()) == BoolInterval::Empty);

  CHECK((Interval(2,3) > Interval(0,1)) == BoolInterval::True);
  CHECK((Interval(1,2) > Interval(0,1)) == BoolInterval::Unknown);
  CHECK((Interval(0,1) > Interval(0,1)) == BoolInterval::Unknown);
  CHECK((Interval(0,1) > Interval(2,3)) == BoolInterval::False);
  CHECK((Interval(-oo,2) > Interval(2,3)) == BoolInterval::False);
  CHECK((Interval(3,oo) > Interval(-oo,2)) == BoolInterval::True);
  CHECK((Interval::empty() > Interval(0,1)) == BoolInterval::Empty);
  CHECK((Interval(0,1) > Interval::empty()) == BoolInterval::Empty);
}
// <<< codac2_tests_Interval.cpp lines 298-317
#undef CODAC_PORT_TEST_ID

  }

  //---------------------------------------------------------------------------
  // codac2_tests_Interval_operations.cpp
  //---------------------------------------------------------------------------
  namespace file_operations
  {
    // The namespaces the file opens; the functions of codac2 on an Interval are found
    // through its argument (a call sin(0.5) is that of std, as in Codac)
    using namespace std;
    using codac2::Interval;
    using codac2::Approx;
    using codac2::oo;

// >>> codac2_tests_Interval_operations.cpp lines 24-24
const double MAX_DOUBLE = std::numeric_limits<double>::max();
// <<< codac2_tests_Interval_operations.cpp lines 24-24

// >>> codac2_tests_Interval_operations.cpp lines 26-90
void CHECK_add(const Interval& x, const Interval& y, const Interval& expected)
{
  CHECK((x + y) == expected);
  CHECK((y + x) == expected); // symmetrical case
  CHECK((Interval(x) += y) == expected); // +=operator
  CHECK((Interval(y) += x) == expected); // +=operator in the other direction
  CHECK(((-x)-y) == -expected); // substraction
  CHECK(((-y)-x) == -expected); // symmetrical case
  CHECK((Interval(-x) -= y) == -expected); // -=operator
  CHECK((Interval(-y) -= x) == -expected); // -=operator in the other direction
}

void CHECK_add_scalar(const Interval& x, double y, const Interval& expected)
{
  CHECK((x+y) == expected);
  CHECK((y+x) == expected);
  CHECK((Interval(x) += y) == expected); // test the +=operator
  CHECK(((-x)-y) == -expected);
  CHECK(((-y)-x) == -expected);
  CHECK((Interval(-x) -= y) == -expected); // test the -=operator
}

void CHECK_mul(const Interval& x, const Interval& y, const Interval& expected)
{
  CHECK(x * y == expected);
  CHECK(y * x == expected); // symmetrical case
  CHECK((Interval(x) *= y) == expected); // *=operator
}

void CHECK_mul_scalar(const Interval& x, double y, const Interval& expected)
{
  CHECK(x * y == expected);
  CHECK(y * x == expected); // symmetrical case
  CHECK((Interval(x) *= y) == expected); // *=operator
}

void CHECK_div(const Interval& x, const Interval& y, const Interval& expected)
{
  CHECK(x / y == expected);
  CHECK((Interval(x) /= y) == expected); // /=operator
}

void CHECK_div_scalar(const Interval& x, double y, const Interval& expected)
{
  CHECK(x / y == expected);
  CHECK((Interval(x) /= y) == expected); // /=operator
}

void CHECK_trigo(const Interval& x, const Interval& expected)
{
  CHECK(Approx(sin(x), 1e-9) == expected);
  CHECK(Approx(sin(Interval::pi()-x), 1e-9) == expected);
  CHECK(Approx(sin(x+Interval::two_pi()), 1e-9) == expected);
  CHECK(Approx(sin(-x), 1e-9) == -expected);
  CHECK(Approx(cos(x-Interval::half_pi()), 1e-9) == expected);
  CHECK(Approx(cos(Interval::half_pi()-x), 1e-9) == expected);
  CHECK(Approx(cos(x+Interval::half_pi()), 1e-9) == -expected);
  CHECK(Approx(cos(x+Interval::two_pi()-Interval::half_pi()), 1e-9) == expected);
}

void CHECK_pow(const Interval& x, int p, const Interval& expected, double eps = 0.)
{
  CHECK(Approx(pow(x,p),eps) == expected);
  CHECK(Approx(pow(-x,p),eps) == ((p%2==0) ? expected : -expected));
}
// <<< codac2_tests_Interval_operations.cpp lines 26-90

#define CODAC_PORT_TEST_ID test_case_1
// >>> codac2_tests_Interval_operations.cpp lines 92-340
TEST_CASE("Interval operations")
{
  CHECK(0 < codac2::next_float(0));
  CHECK(0 > codac2::prev_float(0));
  CHECK(1 < codac2::next_float(1));
  CHECK(1 > codac2::prev_float(1));
  CHECK(oo == codac2::next_float(oo));
  CHECK(-oo == codac2::prev_float(-oo));
  CHECK(-MAX_DOUBLE >= codac2::next_float(-oo));
  CHECK(MAX_DOUBLE <= codac2::prev_float(oo));
  CHECK(oo > MAX_DOUBLE);
  CHECK(-oo < -MAX_DOUBLE);
  CHECK(-Interval(0,1) == Interval(-1,0));
  CHECK(-Interval(-oo,oo) == Interval(-oo,oo));
  CHECK(-Interval(-oo,0) == Interval(0,oo));
  CHECK(-Interval(-oo,1) == Interval(-1,oo));

  CHECK_add(Interval::empty(), Interval(0,1), Interval::empty());
  CHECK_add(Interval(0,1), Interval::empty(), Interval::empty());
  CHECK_add(Interval(-oo,1), Interval(0,1), Interval(-oo, 2));
  CHECK_add(Interval(1,oo), Interval(0,1), Interval(1,oo));
  CHECK_add(Interval(-oo,oo), Interval(0,1), Interval(-oo,oo));
  CHECK_add(Interval(MAX_DOUBLE,oo), 1, Interval(MAX_DOUBLE,oo));
  CHECK_add(Interval(MAX_DOUBLE,oo), -1, Interval(codac2::prev_float(MAX_DOUBLE),oo));
  CHECK_add(Interval(MAX_DOUBLE,oo), Interval(MAX_DOUBLE,oo), Interval(MAX_DOUBLE,oo));
  CHECK_add(Interval(MAX_DOUBLE,oo), -oo, Interval::empty());
  CHECK_add(Interval(MAX_DOUBLE,oo), oo, Interval::empty());
  CHECK_add(Interval(-oo,-MAX_DOUBLE), 1, Interval(-oo,codac2::next_float(-MAX_DOUBLE)));
  CHECK_add(Interval(-oo,-MAX_DOUBLE), -1, Interval(-oo,-MAX_DOUBLE));
  CHECK_add(Interval(-oo,-MAX_DOUBLE), Interval(-oo,-MAX_DOUBLE), Interval(-oo,-MAX_DOUBLE));

  CHECK_add_scalar(Interval::empty(), oo, Interval::empty());
  CHECK_add_scalar(Interval::empty(), 0, Interval::empty());
  CHECK_add_scalar(Interval(0,1), 1, Interval(1,2));
  CHECK_add_scalar(Interval(0,1), -oo, Interval::empty());
  CHECK_add_scalar(Interval(0,1), oo, Interval::empty());
  CHECK_add_scalar(Interval(-oo,1), 1, Interval(-oo,2));

  CHECK_mul(Interval::empty(), Interval(0,1), Interval::empty());
  CHECK_mul(0, Interval(-oo,oo), 0);
  CHECK_mul(Interval(-1,1), Interval(-oo,0), Interval(-oo,oo));
  CHECK_mul(Interval(-oo,-1), Interval(-1,0), Interval(0,oo));
  CHECK_mul(Interval(-oo, 1), Interval(-1,0), Interval(-1,oo));
  CHECK_mul(Interval(0, 1), Interval(1,oo), Interval(0,oo));
  CHECK_mul(Interval(0, 1), Interval(-1,oo), Interval(-1,oo));
  CHECK_mul(Interval(-oo,-1), Interval(0,1), Interval(-oo,0));
  CHECK_mul(Interval(-oo, 1), Interval(0,1), Interval(-oo,1));
  CHECK_mul(Interval(0, 1), Interval(-oo,-1), Interval(-oo,0));
  CHECK_mul(Interval(0, 1), Interval(-oo,1), Interval(-oo,1));
  CHECK_mul(Interval(1,oo), Interval(0,1), Interval(0,oo));
  CHECK_mul(Interval(-1,oo), Interval(0,1), Interval(-1,oo));
  CHECK_mul(Interval(1,2), Interval(1,2), Interval(1,4));
  CHECK_mul(Interval(1,2), Interval(-2,3), Interval(-4,6));
  CHECK_mul(Interval(-1,1), Interval(-2,3), Interval(-3,3));
  CHECK_mul_scalar(Interval(1,2), -oo, Interval::empty());
  CHECK_mul_scalar(Interval(1,2), oo, Interval::empty());
  CHECK_mul_scalar(Interval(1,2), -1, Interval(-2,-1));

  CHECK_div(Interval::empty(), Interval(0,1), Interval::empty());
  CHECK_div(0, 0, Interval::empty());
  CHECK_div(Interval(1,2), 0, Interval::empty());
  CHECK_div(Interval(-oo,oo), 0, Interval::empty());
  CHECK_div(0, Interval(0,1), 0);
  CHECK_div(0, Interval(-oo,oo), 0);
  CHECK_div(Interval(6,12), Interval(2,3), Interval(2,6));
  CHECK_div(Interval(6,12), Interval(-3,-2), Interval(-6,-2));
  CHECK_div(Interval(6,12), Interval(-2,3), Interval(-oo,oo));
  CHECK_div(Interval(-oo,-1), Interval(-1,0), Interval(1,oo));
  CHECK_div(Interval(-oo,-1), Interval(0,1), Interval(-oo,-1));
  CHECK_div(Interval(1,oo), Interval(-1,0), Interval(-oo,-1));
  CHECK_div(Interval(1,oo), Interval(0,1), Interval(1,oo));
  CHECK_div(Interval(-1,1), Interval(-1,1), Interval(-oo,oo));
  CHECK_div_scalar(Interval(1,2), -oo, Interval::empty());
  CHECK_div_scalar(Interval(1,2), oo, Interval::empty());
  CHECK_div_scalar(Interval(1,2), -1, Interval(-2,-1));

  CHECK(sqrt(Interval(-oo,oo)) == Interval(0,oo));
  CHECK(sqrt(Interval(-oo,0)) == 0);
  CHECK(sqrt(Interval(-9,4)) == Interval(0,2));
  CHECK(sqrt(Interval(4,9)) == Interval(2,3));
  CHECK(sqrt(Interval(-9,-4)) == Interval::empty());
  CHECK(sqrt(Interval(-9,oo)) == Interval(0,oo));

  CHECK(log(Interval::empty()) == Interval::empty());
  CHECK(log(Interval(-oo,oo)) == Interval(-oo,oo));
  CHECK(log(Interval(0,oo)) == Interval(-oo,oo));
  CHECK(log(Interval(-oo,0)) ==  Interval::empty());
  CHECK(Approx(log(Interval(1,2))) == Interval(0,::log(2)));
  CHECK(Approx(log(Interval(-1,1))) == Interval(-oo,0));
  CHECK(Approx(log(Interval(0,1))) == Interval(-oo,0));
  CHECK(Approx(log(Interval(1,oo))) == Interval(0,oo));
  CHECK(log(Interval(0)) == Interval::empty());
  CHECK(log(Interval(-2,-1)) == Interval::empty());
  CHECK((log(Interval(0,codac2::next_float(0)))).ub() > -744.5);

  CHECK(exp(Interval::empty()) == Interval::empty());
  CHECK(exp(Interval(-oo,oo)) == Interval(0,oo));
  CHECK(Approx(exp(Interval(0,oo))) == Interval(1,oo));
  CHECK(Approx(exp(Interval(-oo,0))) == Interval(0,1));
  CHECK(Approx(exp(Interval(0,2))) == Interval(1,::exp(2)));
  CHECK(Approx(exp(Interval(-1,1))) == Interval(::exp(-1),::exp(1)));
  CHECK(exp(Interval(1.e100,1.e111)) == Interval(MAX_DOUBLE,oo));
  CHECK(exp(Interval(MAX_DOUBLE,oo)) == Interval(MAX_DOUBLE,oo));
  CHECK(Approx(exp(Interval(0,MAX_DOUBLE))) == Interval(1,oo));

  const double pi_lb = Interval::pi().lb();
  const double pi_ub = Interval::pi().ub();

  CHECK_trigo(Interval(-oo,oo), Interval(-1,1));
  CHECK_trigo(Interval::empty(), Interval::empty());
  CHECK_trigo(Interval(0,pi_ub/2.0), Interval(0,1));
  CHECK_trigo(Interval(0,pi_ub), Interval(0,1));
  CHECK_trigo(Interval(0,3*pi_ub/2.0), Interval(-1,1));
  CHECK_trigo(Interval(pi_lb,3*pi_ub/2.0), Interval(-1,0));
  CHECK_trigo(Interval(0.5,1.5), Interval(sin(0.5),sin(1.5)));
  CHECK_trigo(Interval(1.5,3), Interval(sin(3.0),1));
  CHECK_trigo(Interval(3,4), Interval(sin(4.0),sin(3.0)));
  CHECK_trigo(Interval(3,5), Interval(-1,sin(3.0)));
  CHECK_trigo(Interval(3,2*pi_ub+1.5), Interval(-1,sin(1.5)));
  CHECK_trigo(Interval(5,2*pi_ub+1.5), Interval(sin(5.0),sin(1.5)));
  CHECK_trigo(Interval(5,2*pi_ub+3), Interval(sin(5.0),1));

  CHECK(floor(Interval::empty()) == Interval::empty());
  CHECK(floor(Interval(-oo,-0.000001)) == Interval(-oo,-1));
  CHECK(floor(Interval(0.00000001,oo)) == Interval(0,oo));
  CHECK(floor(Interval(-oo,oo)) == Interval(-oo,oo));
  CHECK(floor(Interval(0.01,2.99)) == Interval(0,2));
  CHECK(floor(Interval(-0.01,2.99)) == Interval(-1,2));
  CHECK(floor(Interval(1.000000001,1.9999999999)) == 1.);
  CHECK(floor(Interval(1e8, MAX_DOUBLE)) == Interval(1e8,MAX_DOUBLE));

  CHECK(ceil(Interval::empty()) == Interval::empty());
  CHECK(ceil(Interval(-oo,-0.000001)) == Interval(-oo,0));
  CHECK(ceil(Interval(0.00000001,oo)) == Interval(1.0,oo));
  CHECK(ceil(Interval(-oo,oo)) == Interval(-oo,oo));
  CHECK(ceil(Interval(0.01,2.99)) == Interval(1,3));
  CHECK(ceil(Interval(-0.01,2.99)) == Interval(0,3));
  CHECK(ceil(Interval(1.000000001,1.9999999999)) == Interval(2,2));
  CHECK(ceil(Interval(1e8, MAX_DOUBLE)) == Interval(1e8,MAX_DOUBLE));

  CHECK(integer(Interval::empty()) == Interval::empty());
  CHECK(integer(Interval(-oo,-0.000001)) == Interval(-oo,-1));
  CHECK(integer(Interval(0.00000001,oo)) == Interval(1.0,oo));
  CHECK(integer(Interval(-oo,oo)) == Interval(-oo,oo));
  CHECK(integer(Interval(0.01,2.99)) == Interval(1,2));
  CHECK(integer(Interval(-0.01,2.99)) == Interval(0,2));
  CHECK(integer(Interval(1.000000001,1.9999999999)) == Interval::empty());
  CHECK(integer(Interval(1e8, MAX_DOUBLE)) == Interval(1e8,MAX_DOUBLE));

  CHECK(tan(Interval::pi()/2.0) == Interval(-oo,oo));
  {
    Interval x = tan(-Interval::pi());
    CHECK(x.diam() < 1e-8);
    CHECK(x.contains(0.));
  }

  CHECK(Approx(tan((3*Interval::pi()/4.0)|(5*Interval::pi()/4.0))) == Interval(-1,1));
  CHECK(tan(Interval(-oo,oo)) == Interval(-oo,oo));
  CHECK(Approx(tan((-Interval::pi()/4.0)|(Interval::pi()/4.0))) == Interval(-1,1));

  {
    // tan(pi/4,pi/2)=[1,+oo)
    Interval x = Interval(pi_lb/4.0,(1-1e-10)*pi_lb/2.0);
    Interval y = tan(x);
    CHECK(Approx(Interval(y.lb())) == Interval(1.0));
    CHECK(y.ub() > 1000);
  }
  {
    // tan(-pi/2,pi/4)=(-oo,1]
    Interval y = tan(Interval(-(1-1e-10)*pi_lb/2.0,pi_lb/4.0));
    CHECK(y.lb() < -1000);
    CHECK(y.ub() == 1.0);
  }

  CHECK(atan2(Interval(-oo,oo),Interval::empty()) == Interval::empty());
  CHECK(Approx(atan2(Interval(1),1)) == Interval::pi()/4.0);
  CHECK(Approx(atan2(Interval(-1),-1)) == -3*Interval::pi()/4.0);
  CHECK(Approx(atan2(Interval(-1),1)) == -Interval::pi()/4.0);
  CHECK(Approx(atan2(Interval(1),-1)) == 3*Interval::pi()/4.0);
  CHECK(Approx(atan2(Interval(0,oo),Interval(0,oo))) == Interval(0,1)*Interval::half_pi());
  CHECK(Approx(atan2(Interval(-oo,codac2::prev_float(0.0)),Interval(-oo,0))) == -(Interval::half_pi() | Interval::pi()));
  CHECK(Approx(atan2(Interval(-oo,0),Interval(0,oo))) == Interval(-1,0)*Interval::half_pi());
  CHECK(Approx(atan2(Interval(0,oo),Interval(-oo,0))) == (Interval::half_pi() | Interval::pi()));
  CHECK(Approx(atan2(Interval(1,oo),Interval(-1,1))) == (Interval::pi()/4.0 | 3*Interval::pi()/4.0));
  CHECK(Approx(atan2(Interval(-oo,-1),Interval(-1,1))) == -(Interval::pi()/4.0 | 3*Interval::pi()/4.0));
  CHECK(atan2(Interval(-1,1),Interval(1,oo)) == Interval(-1,1)*Interval::pi()/4.0);
  CHECK(atan2(Interval(-1,1),Interval(-oo,-1)) == Interval(-1,1)*Interval::pi());
  CHECK(atan2(Interval(-oo,oo),Interval(-oo,oo)) == Interval(-1,1)*Interval::pi());
  CHECK(atan2(Interval(0),0) == Interval::empty());

  CHECK_pow(Interval(-oo,oo), 4, Interval(0,oo));
  CHECK_pow(Interval::empty(), 4, Interval::empty());
  CHECK_pow(Interval(2,3), 4, Interval(16,81));
  CHECK_pow(Interval(-2,3), 4, Interval(0,81));
  CHECK_pow(Interval(-3,2), 4, Interval(0,81));
  CHECK_pow(Interval(2,oo), 4, Interval(16,oo));
  CHECK_pow(Interval(-oo,oo), 3, Interval(-oo,oo));
  CHECK_pow(Interval::empty(), 3, Interval::empty());
  CHECK_pow(Interval(2,3), 3, Interval(8,27));
  CHECK_pow(Interval(-2,3), 3, Interval(-8,27));
  CHECK_pow(Interval(-3,2), 3, Interval(-27,8));
  CHECK_pow(Interval(2,oo), 3, Interval(8,oo));
  CHECK_pow(Interval(-10,10), -2, Interval(1.0/100,oo), std::numeric_limits<double>::epsilon());

  // Non-regression: pow(Interval,double) used to reach gaol through its
  // pow(interval,int) overload (double->int is a standard conversion, while
  // double->interval is a user-defined one), so the exponent was silently
  // truncated: pow([4,4],1.5) returned [4,4] and pow([4,4],0.5) returned [1,1].
  CHECK(Approx(pow(Interval(4),0.5)) == Interval(2));
  CHECK(Approx(pow(Interval(4),1.5)) == Interval(8));
  CHECK(Approx(pow(Interval(4,9),0.5)) == Interval(2,3));
  CHECK(Approx(pow(Interval(4),-0.5)) == Interval(0.5));
  CHECK(Approx(pow(Interval(0,4),0.5)) == Interval(0,2));
  CHECK(Approx(pow(Interval(2,3),1.5)) == pow(Interval(2,3),Interval(1.5)));

  // An exponent that happens to be an integer must keep the integer semantics,
  // which is defined for a negative base as well
  CHECK(pow(Interval(-2,3),3.0) == pow(Interval(-2,3),3));
  CHECK(pow(Interval(-2,3),3.0) == Interval(-8,27));
  CHECK(pow(Interval(-2),2.0) == Interval(4));
  CHECK(pow(Interval(2,3),4.0) == Interval(16,81));
  CHECK(pow(Interval(4),0.0) == Interval::one());
  CHECK(Approx(pow(Interval(-10,10),-2.0), std::numeric_limits<double>::epsilon()) == Interval(1.0/100,oo));

  // A non-integer power is real-valued only for a non-negative base: gaol
  // computes it on the magnitude instead, and used to return [-1,2] here.
  CHECK(pow(Interval(-4,-1),0.5) == Interval::empty());
  CHECK(pow(Interval(-4,-1),Interval(0.5)) == Interval::empty());
  CHECK(pow(Interval(-4,-1),0.5) == sqrt(Interval(-4,-1)));
  CHECK(Approx(pow(Interval(-4,9),0.5)) == Interval(0,3));
  CHECK(pow(Interval(-4,9),0.5).lb() >= 0.0);
  CHECK(Approx(pow(Interval(-2,3),Interval(1,2))) == Interval(0,9));

  // A negative base stays valid at an integer exponent, whichever
  // overload spells it
  CHECK(pow(Interval(-2,3),Interval(3)) == Interval(-8,27));
  CHECK(pow(Interval(-2,3),Interval(3)) == pow(Interval(-2,3),3));
  CHECK(pow(Interval(-2,3),Interval(2)) == Interval(0,9));

  // Infinite exponents and the empty set are unchanged
  CHECK(pow(Interval(4),oo) == Interval::empty());
  CHECK(pow(Interval(4),-oo) == Interval::empty());
  CHECK(pow(Interval::empty(),1.5) == Interval::empty());

  CHECK(root(Interval(0,1),-1) == Interval(1.0,oo));
  CHECK(Approx(root(Interval(-27,-8),3)) == Interval(-3,-2));
  CHECK(root(Interval(-4,1),2) == Interval(0,1));
  CHECK(Approx(root(Interval(-8,1),3)) == Interval(-2,1));
}
// <<< codac2_tests_Interval_operations.cpp lines 92-340
#undef CODAC_PORT_TEST_ID

  }

  //---------------------------------------------------------------------------
  // codac2_tests_Interval_rounding.cpp
  //---------------------------------------------------------------------------
  namespace file_rounding
  {
    // The namespaces the file opens; the functions of codac2 on an Interval are found
    // through its argument (a call sin(0.5) is that of std, as in Codac)
    using namespace std;
    using codac2::Interval;
    using codac2::oo;

// >>> codac2_tests_Interval_rounding.cpp lines 32-210
namespace
{
  // The checks below compute with rounding to nearest, which this object sets
  // for its lifetime, restoring the rounding direction it found afterwards.
  class RoundingToNearest
  {
    public:

      RoundingToNearest()
        : _saved(std::fegetround())
      {
        std::fesetround(FE_TONEAREST);
      }

      ~RoundingToNearest()
      {
        std::fesetround(_saved);
      }

    private:

      const int _saved;
  };

  // The exact result of an operation on two doubles, as the sum hi+lo of two
  // doubles, hi being that result rounded to nearest. It is computed by the
  // error-free transformations of Knuth (sum) and Dekker (product), which are
  // exact with rounding to nearest, far from overflow and underflow.
  //
  // The operands are read back from memory (volatile) once rounding to nearest
  // is set. Visual C++ for 32-bit x86 with /arch:AVX or /arch:AVX2 otherwise
  // reuses a+b or a*b as the interval operation just checked computed it, with
  // rounding upward, in spite of /fp:strict: about half of the random sums and
  // products then came out rounded the wrong way. Codac leaves /arch:AVX2 out
  // for that target (see the top-level CMakeLists.txt), and the checks stay
  // right for a build that would add it.
  struct Exact
  {
    double hi, lo;
  };

  Exact exact_sum(double a0, double b0)
  {
    RoundingToNearest nearest;
    volatile double va = a0, vb = b0;
    const double a = va, b = vb;
    const double s = a + b;
    const double bb = s - a;
    return { s, (a - (s - bb)) + (b - bb) };
  }

  Exact exact_product(double a0, double b0)
  {
    RoundingToNearest nearest;
    volatile double va = a0, vb = b0;
    const double a = va, b = vb;
    const double p = a * b;
    // 2^27+1 splits a double into two halves of at most 27 bits
    const double ta = 134217729. * a;
    const double ah = ta - (ta - a);
    const double al = a - ah;
    const double tb = 134217729. * b;
    const double bh = tb - (tb - b);
    const double bl = b - bh;
    return { p, ((ah * bh - p) + ah * bl + al * bh) + al * bl };
  }

  // x <= v, compared exactly
  bool is_below(double x, const Exact& v)
  {
    return x < v.hi || (x == v.hi && v.lo >= 0.);
  }

  // x >= v, compared exactly
  bool is_above(double x, const Exact& v)
  {
    return x > v.hi || (x == v.hi && v.lo <= 0.);
  }

  // x <= a/b, compared exactly through the product x*b
  bool is_below_quotient(double x, double a, double b)
  {
    const Exact xb = exact_product(x, b);
    return b > 0. ? is_above(a, xb) : is_below(a, xb);
  }

  // x >= a/b, compared exactly through the product x*b
  bool is_above_quotient(double x, double a, double b)
  {
    const Exact xb = exact_product(x, b);
    return b > 0. ? is_below(a, xb) : is_above(a, xb);
  }

  double next_double(double x)
  {
    RoundingToNearest nearest;
    return std::nextafter(x, numeric_limits<double>::infinity());
  }

  double previous_double(double x)
  {
    RoundingToNearest nearest;
    return std::nextafter(x, -numeric_limits<double>::infinity());
  }

  // Whether [x] is the tightest interval of doubles enclosing v
  bool is_tightest_enclosure(const Interval& x, const Exact& v)
  {
    return is_below(x.lb(), v) && !is_below(next_double(x.lb()), v)
      && is_above(x.ub(), v) && !is_above(previous_double(x.ub()), v);
  }

  // Whether [x] is the tightest interval of doubles enclosing a/b
  bool is_tightest_quotient(const Interval& x, double a, double b)
  {
    return is_below_quotient(x.lb(), a, b) && !is_below_quotient(next_double(x.lb()), a, b)
      && is_above_quotient(x.ub(), a, b) && !is_above_quotient(previous_double(x.ub()), a, b);
  }

  // A deterministic generator of doubles (xorshift64): a random sign and
  // mantissa, and an exponent between emin and emax, which keeps the exact
  // computations above far from overflow and underflow.
  class RandomDoubles
  {
    public:

      double operator()(int emin, int emax)
      {
        const uint64_t mantissa = next_bits() & 0x000FFFFFFFFFFFFFull;
        const uint64_t exponent = static_cast<uint64_t>(1023 + emin)
          + next_bits() % static_cast<uint64_t>(emax - emin + 1);
        const uint64_t sign = next_bits() & 1u;
        const uint64_t bits = (sign << 63) | (exponent << 52) | mantissa;
        double x;
        std::memcpy(&x, &bits, sizeof x);
        return x;
      }

    private:

      uint64_t next_bits()
      {
        _state ^= _state << 13;
        _state ^= _state >> 7;
        _state ^= _state << 17;
        return _state;
      }

      uint64_t _state = 0x9E3779B97F4A7C15ull;
  };

  string hex(double x)
  {
    ostringstream s;
    s << hexfloat << x;
    return s.str();
  }

  string hex(const Interval& x)
  {
    return "[" + hex(x.lb()) + ", " + hex(x.ub()) + "]";
  }

  // Counts the failures of a check repeated on many values, and describes the
  // first one
  struct Failures
  {
    int count = 0;
    string first;

    void add(bool ok, const string& description)
    {
      if(!ok && count++ == 0)
        first = description;
    }
  };

  const int nb_random_values = 5000;
}
// <<< codac2_tests_Interval_rounding.cpp lines 32-210

#define CODAC_PORT_TEST_ID test_case_1
// >>> codac2_tests_Interval_rounding.cpp lines 212-276
TEST_CASE("Interval rounding - arithmetic operations")
{
  RandomDoubles random;
  Failures sum, difference, product, product_by_double, quotient, square, square_root, interval_product;

  for(int i = 0; i < nb_random_values; i++)
  {
    const double a = random(-30,30), b = random(-30,30);
    const string values = "a=" + hex(a) + " b=" + hex(b);

    const Interval s = Interval(a) + Interval(b);
    sum.add(is_tightest_enclosure(s, exact_sum(a,b)), values + ": a+b=" + hex(s));

    const Interval d = Interval(a) - Interval(b);
    difference.add(is_tightest_enclosure(d, exact_sum(a,-b)), values + ": a-b=" + hex(d));

    const Interval p = Interval(a) * Interval(b);
    product.add(is_tightest_enclosure(p, exact_product(a,b)), values + ": a*b=" + hex(p));

    const Interval pd = Interval(a) * b;
    product_by_double.add(is_tightest_enclosure(pd, exact_product(a,b)), values + ": a*b=" + hex(pd));

    const Interval q = Interval(a) / Interval(b);
    quotient.add(is_tightest_quotient(q, a, b), values + ": a/b=" + hex(q));

    const Interval sq = sqr(Interval(a));
    square.add(is_tightest_enclosure(sq, exact_product(a,a)), values + ": a^2=" + hex(sq));

    // GAOL encloses square roots, not always as tightly as possible:
    // lb^2 <= |a| <= ub^2 only
    const double m = std::fabs(a);
    const Interval r = sqrt(Interval(m));
    square_root.add(r.lb() >= 0. && is_above(m, exact_product(r.lb(),r.lb())) && is_below(m, exact_product(r.ub(),r.ub())),
      "|a|=" + hex(m) + ": sqrt(|a|)=" + hex(r));

    // Products of intervals, whose bounds have all the signs GAOL distinguishes:
    // the tightest enclosure of the four products of bounds
    const double c = random(-30,30), e = random(-30,30);
    const Interval x(a < c ? a : c, a < c ? c : a), y(b < e ? b : e, b < e ? e : b);
    const Interval xy = x * y;
    const vector<Exact> corners = {
      exact_product(x.lb(),y.lb()), exact_product(x.lb(),y.ub()),
      exact_product(x.ub(),y.lb()), exact_product(x.ub(),y.ub())
    };
    bool lb_below_all = true, next_lb_above_one = false, ub_above_all = true, previous_ub_below_one = false;
    for(const auto& v : corners)
    {
      lb_below_all = lb_below_all && is_below(xy.lb(), v);
      next_lb_above_one = next_lb_above_one || !is_below(next_double(xy.lb()), v);
      ub_above_all = ub_above_all && is_above(xy.ub(), v);
      previous_ub_below_one = previous_ub_below_one || !is_above(previous_double(xy.ub()), v);
    }
    interval_product.add(lb_below_all && next_lb_above_one && ub_above_all && previous_ub_below_one,
      "x=" + hex(x) + " y=" + hex(y) + ": x*y=" + hex(xy));
  }

  { INFO("a+b, first failure: " << sum.first); CHECK(sum.count == 0); }
  { INFO("a-b, first failure: " << difference.first); CHECK(difference.count == 0); }
  { INFO("a*b, first failure: " << product.first); CHECK(product.count == 0); }
  { INFO("[a]*b, first failure: " << product_by_double.first); CHECK(product_by_double.count == 0); }
  { INFO("a/b, first failure: " << quotient.first); CHECK(quotient.count == 0); }
  { INFO("sqr(a), first failure: " << square.first); CHECK(square.count == 0); }
  { INFO("sqrt(|a|), first failure: " << square_root.first); CHECK(square_root.count == 0); }
  { INFO("[x]*[y], first failure: " << interval_product.first); CHECK(interval_product.count == 0); }
}
// <<< codac2_tests_Interval_rounding.cpp lines 212-276
#undef CODAC_PORT_TEST_ID

#define CODAC_PORT_TEST_ID test_case_2
// >>> codac2_tests_Interval_rounding.cpp lines 278-350
TEST_CASE("Interval rounding - elementary functions")
{
  // The doubles immediately below and above the exact value of each function at
  // x, computed with 400 bits of precision (mpmath): the enclosure computed by
  // Codac has to contain both.
  struct Value
  {
    const char* function;
    Interval (*f)(const Interval&);
    double x, below, above;
  };

  const auto f_exp = [](const Interval& x) { return exp(x); };
  const auto f_log = [](const Interval& x) { return log(x); };
  const auto f_sin = [](const Interval& x) { return sin(x); };
  const auto f_cos = [](const Interval& x) { return cos(x); };
  const auto f_tan = [](const Interval& x) { return tan(x); };
  const auto f_atan = [](const Interval& x) { return atan(x); };
  const auto f_sqrt = [](const Interval& x) { return sqrt(x); };

  const vector<Value> values = {
    { "exp", f_exp,  0x1.0000000000000p+0,  0x1.5bf0a8b145769p+1,  0x1.5bf0a8b14576ap+1 },
    { "exp", f_exp,  0x1.0000000000000p-1,  0x1.a61298e1e069bp+0,  0x1.a61298e1e069cp+0 },
    { "exp", f_exp, -0x1.0000000000000p+0,  0x1.78b56362cef37p-2,  0x1.78b56362cef38p-2 },
    { "exp", f_exp,  0x1.4000000000000p+3,  0x1.5829dcf95055fp+14, 0x1.5829dcf950560p+14 },
    { "exp", f_exp,  0x1.0624dd2f1a9fcp-10, 0x1.0041919b7ee33p+0,  0x1.0041919b7ee34p+0 },
    { "exp", f_exp, -0x1.4000000000000p+4,  0x1.1b48655f37266p-29, 0x1.1b48655f37267p-29 },
    { "log", f_log,  0x1.0000000000000p+1,  0x1.62e42fefa39efp-1,  0x1.62e42fefa39f0p-1 },
    { "log", f_log,  0x1.4000000000000p+3,  0x1.26bb1bbb55515p+1,  0x1.26bb1bbb55516p+1 },
    { "log", f_log,  0x1.999999999999ap-4, -0x1.26bb1bbb55516p+1, -0x1.26bb1bbb55515p+1 },
    { "log", f_log,  0x1.8000000000000p+0,  0x1.9f323ecbf984bp-2,  0x1.9f323ecbf984cp-2 },
    { "log", f_log,  0x1.4f8b588e368f1p-17,-0x1.7069e2aa2aa5bp+3, -0x1.7069e2aa2aa5ap+3 },
    { "log", f_log,  0x1.2a05f20000000p+33, 0x1.7069e2aa2aa5ap+4,  0x1.7069e2aa2aa5bp+4 },
    { "sin", f_sin,  0x1.0000000000000p+0,  0x1.aed548f090ceep-1,  0x1.aed548f090cefp-1 },
    { "sin", f_sin,  0x1.0000000000000p-1,  0x1.eaee8744b05efp-2,  0x1.eaee8744b05f0p-2 },
    { "sin", f_sin,  0x1.8000000000000p+1,  0x1.210386db6d55bp-3,  0x1.210386db6d55cp-3 },
    { "sin", f_sin, -0x1.0000000000000p+1, -0x1.d18f6ead1b446p-1, -0x1.d18f6ead1b445p-1 },
    { "sin", f_sin,  0x1.9000000000000p+6, -0x1.03425b78c4db9p-1, -0x1.03425b78c4db8p-1 },
    { "sin", f_sin,  0x1.0624dd2f1a9fcp-10, 0x1.0624da5218a62p-10, 0x1.0624da5218a63p-10 },
    { "cos", f_cos,  0x1.0000000000000p+0,  0x1.14a280fb5068bp-1,  0x1.14a280fb5068cp-1 },
    { "cos", f_cos,  0x1.0000000000000p-1,  0x1.c1528065b7d4fp-1,  0x1.c1528065b7d50p-1 },
    { "cos", f_cos,  0x1.8000000000000p+1, -0x1.fae04be85e5d3p-1, -0x1.fae04be85e5d2p-1 },
    { "cos", f_cos, -0x1.0000000000000p+1, -0x1.aa22657537205p-2, -0x1.aa22657537204p-2 },
    { "cos", f_cos,  0x1.9000000000000p+6,  0x1.b981dbf665fdfp-1,  0x1.b981dbf665fe0p-1 },
    { "cos", f_cos,  0x1.8000000000000p+0,  0x1.21bd54fc5f9a7p-4,  0x1.21bd54fc5f9a8p-4 },
    { "tan", f_tan,  0x1.0000000000000p+0,  0x1.8eb245cbee3a5p+0,  0x1.8eb245cbee3a6p+0 },
    { "tan", f_tan,  0x1.0000000000000p-1,  0x1.17b4f5bf3474ap-1,  0x1.17b4f5bf3474bp-1 },
    { "tan", f_tan, -0x1.3333333333333p+0, -0x1.493c43acb164dp+1, -0x1.493c43acb164cp+1 },
    { "tan", f_tan,  0x1.8000000000000p+1, -0x1.23ef71254b870p-3, -0x1.23ef71254b86fp-3 },
    { "atan", f_atan,  0x1.0000000000000p+0,  0x1.921fb54442d18p-1,  0x1.921fb54442d19p-1 },
    { "atan", f_atan,  0x1.0000000000000p-1,  0x1.dac670561bb4fp-2,  0x1.dac670561bb50p-2 },
    { "atan", f_atan, -0x1.0000000000000p+1, -0x1.1b6e192ebbe45p+0, -0x1.1b6e192ebbe44p+0 },
    { "atan", f_atan,  0x1.f400000000000p+9,  0x1.91de2c0e658bcp+0,  0x1.91de2c0e658bdp+0 },
    { "sqrt", f_sqrt,  0x1.0000000000000p+1,  0x1.6a09e667f3bccp+0,  0x1.6a09e667f3bcdp+0 },
    { "sqrt", f_sqrt,  0x1.8000000000000p+1,  0x1.bb67ae8584caap+0,  0x1.bb67ae8584cabp+0 },
    { "sqrt", f_sqrt,  0x1.999999999999ap-4,  0x1.43d136248490fp-2,  0x1.43d1362484910p-2 },
    { "sqrt", f_sqrt,  0x1.ad7f29abcaf48p-24, 0x1.4b96be9c2da2bp-12, 0x1.4b96be9c2da2cp-12 },
  };

  for(const auto& v : values)
  {
    const Interval y = v.f(Interval(v.x));
    INFO(v.function << "(" << hex(v.x) << ") = " << hex(y));
    CHECK(y.lb() <= v.below);
    CHECK(y.ub() >= v.above);
  }

  // pi, whose neighbours scaled by a power of two are those of pi/2 and 2pi
  const double pi_below = 0x1.921fb54442d18p+1, pi_above = 0x1.921fb54442d19p+1;
  CHECK((Interval::pi().lb() <= pi_below && Interval::pi().ub() >= pi_above));
  CHECK((Interval::half_pi().lb() <= pi_below/2. && Interval::half_pi().ub() >= pi_above/2.));
  CHECK((Interval::two_pi().lb() <= pi_below*2. && Interval::two_pi().ub() >= pi_above*2.));
}
// <<< codac2_tests_Interval_rounding.cpp lines 278-350
#undef CODAC_PORT_TEST_ID

#define CODAC_PORT_TEST_ID test_case_3
#ifdef _MSC_VER
#  pragma warning(push)
#  pragma warning(disable : 4456 4457) // the parameters x and y of the lambdas hide the locals x and y
#endif
// >>> codac2_tests_Interval_rounding.cpp lines 352-435
TEST_CASE("Interval rounding - after any operation")
{
  // Codac and GAOL compute with the rounding direction set upward, once for
  // all (see the initialisation of oo in codac2_math.h): every operation has to
  // leave it so, or the operations after it return wrong bounds.
  CHECK(std::fegetround() == FE_UPWARD);

  struct Operation
  {
    const char* name;
    void (*run)(const Interval& x, const Interval& y);
  };

  // x is within [0,1] and y above 1, which is the domain of every function below
  const Interval x(0.1,0.3), y(1.5,2.5);

  const vector<Operation> operations = {
    { "Interval(a,b)", [](const Interval&, const Interval&) { (void)Interval(0.1,1./3.); } },
    { "Interval::mid()", [](const Interval& x, const Interval&) { (void)x.mid(); } },
    { "Interval::rad()", [](const Interval& x, const Interval&) { (void)x.rad(); } },
    { "Interval::diam()", [](const Interval& x, const Interval&) { (void)x.diam(); } },
    { "Interval::mag()", [](const Interval& x, const Interval&) { (void)x.mag(); } },
    { "Interval::mig()", [](const Interval& x, const Interval&) { (void)x.mig(); } },
// [not ported, line 375 of codac2_tests_Interval_rounding.cpp: entry "Interval::rand()" of the list of operations (rand() is Codac only)]
    { "Interval::bisect()", [](const Interval& x, const Interval&) { (void)x.bisect(); } },
    { "Interval::inflate()", [](const Interval& x, const Interval&) { Interval z(x); z.inflate(0.1); } },
    { "operator<<", [](const Interval& x, const Interval&) { ostringstream s; s << x; } },
    { "operator+=", [](const Interval& x, const Interval& y) { Interval z(x); z += y; } },
    { "operator-=", [](const Interval& x, const Interval& y) { Interval z(x); z -= y; } },
    { "operator*=", [](const Interval& x, const Interval& y) { Interval z(x); z *= y; } },
    { "operator/=", [](const Interval& x, const Interval& y) { Interval z(x); z /= y; } },
    { "operator|=", [](const Interval& x, const Interval& y) { Interval z(x); z |= y; } },
    { "operator&=", [](const Interval& x, const Interval& y) { Interval z(x); z &= y; } },
    { "sqr", [](const Interval& x, const Interval&) { (void)sqr(x); } },
    { "sqrt", [](const Interval& x, const Interval&) { (void)sqrt(x); } },
    { "pow(x,int)", [](const Interval&, const Interval& y) { (void)pow(y,3); } },
    { "pow(x,double)", [](const Interval&, const Interval& y) { (void)pow(y,2.5); } },
    { "pow(x,Interval)", [](const Interval& x, const Interval& y) { (void)pow(y,x); } },
    { "root", [](const Interval&, const Interval& y) { (void)root(y,3); } },
    { "exp", [](const Interval& x, const Interval&) { (void)exp(x); } },
    { "log", [](const Interval& x, const Interval&) { (void)log(x); } },
    { "cos", [](const Interval& x, const Interval&) { (void)cos(x); } },
    { "sin", [](const Interval& x, const Interval&) { (void)sin(x); } },
    { "tan", [](const Interval& x, const Interval&) { (void)tan(x); } },
    { "acos", [](const Interval& x, const Interval&) { (void)acos(x); } },
    { "asin", [](const Interval& x, const Interval&) { (void)asin(x); } },
    { "atan", [](const Interval& x, const Interval&) { (void)atan(x); } },
    { "atan2", [](const Interval& x, const Interval& y) { (void)atan2(y,x); } },
    { "cosh", [](const Interval& x, const Interval&) { (void)cosh(x); } },
    { "sinh", [](const Interval& x, const Interval&) { (void)sinh(x); } },
    { "tanh", [](const Interval& x, const Interval&) { (void)tanh(x); } },
    { "acosh", [](const Interval&, const Interval& y) { (void)acosh(y); } },
    { "asinh", [](const Interval& x, const Interval&) { (void)asinh(x); } },
    { "atanh", [](const Interval& x, const Interval&) { (void)atanh(x); } },
    { "abs", [](const Interval& x, const Interval&) { (void)abs(x); } },
    { "min", [](const Interval& x, const Interval& y) { (void)min(x,y); } },
    { "max", [](const Interval& x, const Interval& y) { (void)max(x,y); } },
    { "sign", [](const Interval& x, const Interval&) { (void)sign(x); } },
    { "integer", [](const Interval&, const Interval& y) { (void)integer(y); } },
    { "floor", [](const Interval&, const Interval& y) { (void)floor(y); } },
    { "ceil", [](const Interval&, const Interval& y) { (void)ceil(y); } },
// [not ported, line 413 of codac2_tests_Interval_rounding.cpp: entry "chi" of the list of operations (the ternary chi(x,y,z) is Codac only)]
    { "Interval::pi()", [](const Interval&, const Interval&) { (void)Interval::pi(); } },
  };

  RandomDoubles random;
  for(const auto& op : operations)
  {
    op.run(x,y);
    INFO("after " << op.name);
    CHECK(std::fegetround() == FE_UPWARD);

    // A product and a sum whose exact results are not doubles, as the random
    // mantissas make almost certain: their bounds are the tightest ones only if
    // the rounding direction is still upward
    const double a = random(-30,30), b = random(-30,30);
    const Interval p = Interval(a) * Interval(b);
    INFO("a=" << hex(a) << " b=" << hex(b) << ": a*b=" << hex(p));
    CHECK(is_tightest_enclosure(p, exact_product(a,b)));
    const Interval s = Interval(a) + Interval(b);
    INFO("a+b=" << hex(s));
    CHECK(is_tightest_enclosure(s, exact_sum(a,b)));
  }
}
// <<< codac2_tests_Interval_rounding.cpp lines 352-435
#ifdef _MSC_VER
#  pragma warning(pop)
#endif
#undef CODAC_PORT_TEST_ID

  }

  //---------------------------------------------------------------------------
  // The TEST_CASEs, in the order of the files
  //---------------------------------------------------------------------------
  const catch2::TestCase test_cases[] = {
    { "codac2_tests_Interval.cpp", file_interval::test_case_1_name, &file_interval::test_case_1 },
    { "codac2_tests_Interval.cpp", file_interval::test_case_2_name, &file_interval::test_case_2 },
    { "codac2_tests_Interval.cpp", file_interval::test_case_3_name, &file_interval::test_case_3 },
    { "codac2_tests_Interval_operations.cpp", file_operations::test_case_1_name, &file_operations::test_case_1 },
    { "codac2_tests_Interval_rounding.cpp", file_rounding::test_case_1_name, &file_rounding::test_case_1 },
    { "codac2_tests_Interval_rounding.cpp", file_rounding::test_case_2_name, &file_rounding::test_case_2 },
    { "codac2_tests_Interval_rounding.cpp", file_rounding::test_case_3_name, &file_rounding::test_case_3 },
  };

  // Runs the TEST_CASEs. Codac sets the rounding direction upward once for all
  // by the initialization of oo (codac2_math.h), before its first operation
  inline void run()
  {
    gaol::round_upward();
    for (const catch2::TestCase& t : test_cases) {
      catch2::run(t);
    }
  }
}

#undef INFO
#undef CHECK
#undef TEST_CASE
#undef CODAC_PORT_TEST_CASE_1
#undef CODAC_PORT_TEST_CASE_2
#undef CODAC_PORT_CAT
#undef CODAC_PORT_CAT_
#pragma pop_macro("max")
#pragma pop_macro("min")
#pragma pop_macro("INFO")
#pragma pop_macro("CHECK")
#pragma pop_macro("TEST_CASE")

int main()
{
  gaol::init();
  ibex_port::run();
  codac_port::run();
  std::fesetround(FE_UPWARD);
  const int status = gaol_tests::summary();
  gaol::cleanup();
  return status;
}
