/*-*-C++-*------------------------------------------------------------------
 * gaol -- NOT Just Another Interval Library
 *--------------------------------------------------------------------------
 * Tests of GAOL v5: the cases of the unit tests of IBEX and of Codac on their
 * intervals.
 *
 * IBEX (https://github.com/ibex-team/ibex-lib) and Codac
 * (https://github.com/codac-team/codac) compute with intervals that are GAOL's.
 * The unit tests they run on a scalar interval and on interval arithmetic have
 * cases, intervals and a result to find: they are taken here and checked
 * against GAOL v5 itself, written with gaol::interval and the functions of
 * GAOL, not with the classes of the projects. The name of each case says where
 * it comes from ("IBEX TestArith::add05", "Codac Interval_operations:176").
 *
 * The result to find is the exact result of the operation on the exact
 * inputs, computed apart from GAOL and from the projects by the scripts of
 * tests/tools/ibex_codac/ (exact rational arithmetic, and mpmath with 2000 bits
 * for the elementary functions, as tests/elementary_values.py does): the
 * bounds of GAOL have to enclose it, with no double between them and the
 * tightest bounds for the operations that are exact in GAOL, and no more than
 * `limit` doubles for the elementary functions. What IBEX and Codac expect,
 * with their own tolerances (1e-10 absolute for IBEX, Approx for Codac), is
 * compared with the exact result by the scripts (--originals), not here. The
 * rows of the tables are written by the scripts between the lines "BEGIN" and
 * "END" of each table (--write), and checked by them (--check): do not edit
 * them by hand.
 *
 * Three sections, each with the list of what it leaves out and why:
 *   - ibex: the forward tests of IBEX (ibex-lib commit 65ed587): TestInterval
 *     and TestArith, arithmetic, sets, accessors, relations and elementary
 *     functions;
 *   - ibexb: the backward contractors of IBEX (bwd_*), the generalized division
 *     div2, and its distances, written with the operations and the reverse
 *     functions of GAOL;
 *   - codac: the tests of Codac on its Interval (Codac 2 commit 6419940, and the
 *     branch GAOL of the fork of Jordan Ninin, commit f0d06ff0): the intervals,
 *     their operations, and the rounding direction.
 *
 * The cases come from the tests of IBEX (Copyright Ecole des Mines de Nantes
 * (FRANCE), author Gilles Chabert) and of Codac (Copyright Codac Team; authors
 * Gilles Chabert and Simon Rohou, and, for the rounding test, Jordan Ninin).
 * IBEX and Codac are distributed under the GNU Lesser General Public License
 * (the COPYING.LESSER of each project is its version 3), GAOL under its
 * version 2 (COPYING.LIB).
 *
 * Copyright (c) 2026 ENSTA, France
 *
 * Created 2026-11-02 by Jordan NININ
 *--------------------------------------------------------------------------
 * gaol is a software distributed WITHOUT ANY WARRANTY. Read the associated
 * COPYING file for information.
 *--------------------------------------------------------------------------*/

#include "gaol_tests.h"

#include <algorithm>
#include <cfenv>
#include <cmath>
#include <cstdlib>
#include <limits>
#include <map>
#include <string>
#include <utility>
#include <vector>

using namespace gaol;
using namespace gaol_tests;

namespace
{
  // The largest distance from the tightest bounds allowed to a function, in
  // doubles: none, the elementary functions of GAOL giving the tightest bounds
  // (doc/accuracy.md), as the operations that are exact (+ - * / sqr sqrt, the
  // sets, the accessors...). The rows that need more say it (atan2_rel_x and
  // atan2_rel_y: two doubles, doc/accuracy.md)
  const int limit = 0;

  // The tightest enclosure of an exact result: lo is the greatest double not
  // above the exact lower bound, hi the least double not below the exact upper
  // bound (-inf or +inf beyond the doubles). A result that is a double has lo
  // equal to hi. The empty set is a Ref whose bounds are NaN
  struct Ref
  {
    double lo, hi;
  };

  inline bool is_none(const Ref& r)
  {
    return r.lo != r.lo;
  }

  // The number of doubles above from up to to, from <= to (max_doubles + 1
  // when there are more)
  inline int doubles_up_to(double from, double to, int max_doubles)
  {
    int n = 0;
    for (double t = from; t < to && n <= max_doubles; t = next_double(t)) {
      ++n;
    }
    return n;
  }

  // The exact result is the empty set (ref is none), or r encloses it and its
  // bounds are no more than max_doubles doubles from the tightest ones
  template<class Describe>
  void expect(const std::string& name, const interval& r, const Ref& ref, const Describe& describe, int max_doubles = 0)
  {
    if (is_none(ref)) {
      check(name + ": is empty", r.is_empty(), describe);
      return;
    }
    if (check(name + ": encloses", !r.is_empty() && r.left() <= ref.lo && r.right() >= ref.hi, describe)) {
      check_distance(name, std::max(doubles_up_to(r.left(), ref.lo, max_doubles), doubles_up_to(ref.hi, r.right(), max_doubles)),
                     max_doubles, describe);
    }
  }

  // A real result, as a double d: it is between ref.lo and ref.hi, or no more
  // than max_doubles doubles from them
  template<class Describe>
  void expect(const std::string& name, double d, const Ref& ref, const Describe& describe, int max_doubles = 0)
  {
    if (is_none(ref)) {
      check(name + ": is NaN", d != d, describe);
      return;
    }
    if (check(name + ": is not NaN", d == d, describe)) {
      check_distance(name, std::max(doubles_up_to(d, ref.lo, max_doubles), doubles_up_to(ref.hi, d, max_doubles)), max_doubles,
                     describe);
    }
  }

  // A yes or no
  template<class Describe>
  void expect(const std::string& name, bool actual, bool expected, const Describe& describe)
  {
    check(name, actual == expected, describe);
  }

  inline std::string show(const interval& x)
  {
    return hex(x);
  }
}

// The forward tests of IBEX (ibex-lib commit 65ed587, tests/TestInterval.cpp and
// tests/TestArith.cpp), as tests of GAOL: the intervals of each case, and the
// result to find, written directly with gaol::interval. The result is the
// exact one, computed apart from GAOL and from IBEX by
// tests/tools/ibex_codac/ibex_forward_cases.py, which writes the tables of this
// section (exact rationals, and mpmath with 2000 bits for the elementary
// functions): Ref{lo, hi} are the doubles around the exact bounds, and the
// result of GAOL has to enclose them, within no double of them for the
// operations that are exact in GAOL (+ - * / sqrt, the sets, floor, ceil,
// integer, the accessors) and within `limit` doubles for the elementary
// functions. What IBEX expects, with its tolerance of 1e-10, is only compared
// with the exact result, by the script (--originals).
//
// The functions IBEX calls are those of GAOL: root(x, n) is nth_root(x, n),
// diam() width(), mid() midpoint(), lb() and ub() left() and right(), EMPTY_SET
// interval::emptyset(), ALL_REALS interval::universe(), POS_REALS
// interval::positive(), NEG_REALS interval::negative(), next_float() and
// previous_float() those of GAOL. The relations IBEX writes itself are those
// of IEEE 1788-2015 that GAOL has: is_subset(y) is set_leq(y), is_superset(y)
// set_geq(y), is_disjoint(y) set_disjoint(y), contains(d) set_contains(d),
// interior_contains(d) set_strictly_contains(d), == and != set_eq() and
// set_neq(); the strict subset and superset of IBEX are the relation and
// set_neq(), and is_strict_interior_subset(y) is set_le(y) and set_neq(y).
// IBEX's own definitions are is_degenerated(), is_empty() || is_a_double(), and
// is_unbounded(), !is_finite() (the empty set is bounded). An infinite double
// as an operand is the empty set (interval(+oo) is empty in IBEX and in GAOL),
// so x + oo is empty. The midpoint of an unbounded interval is the convention
// of IBEX and of GAOL (-MAXREAL for [-oo, b], MAXREAL for [a, +oo], 0 for
// [-oo, +oo]), no exact value.
//
// Each test of IBEX is one case, one row of a table (the 30 components of the
// vector of mid08 are 30 rows), named as in IBEX. The variants a test performs
// are run for the case, each under its name: x+z, z+x, x+=z and the subtractions
// of the opposites for the addition; sin(pi-x), cos(x+pi/2)... for the sines,
// whose references are the exact images of the arguments GAOL computes (the
// sums and differences with pi are exact in GAOL, the script replays them).
// Three tests that TestArith.h does not register, so that IBEX never runs
// them, are here, and GAOL passes them: float10 (previous_float(-DBL_MAX) is
// -oo; commented out in TestArith.h for a bug between Gaol and eglibc), sqrt06
// and tan07. tan06 is not: its expectation, all the reals for tan(-pi), is
// wrong, tan(-pi) containing 0 and no pole.
// Not in this section: the tests of IBEX on its own types and algorithms
// (consCppArray, set_empty, delta, diff, the distances, the generalized
// division div2, the matrices), and the reverse functions (bwd_*).
namespace
{
  constexpr double ibex_oo = std::numeric_limits<double>::infinity();
  constexpr Ref ibex_empty = { std::numeric_limits<double>::quiet_NaN(), std::numeric_limits<double>::quiet_NaN() };

  // The largest distance allowed to asinh(sinh(x)) from x: sinh and asinh
  // each round outward, and sinh(x) is up to one double wider than the exact
  // value, whose asinh is up to a double further from x for |x| large
  const int ibex_round_trip = 3;

  // [r.lo, r.hi], or the empty set for ibex_empty
  inline interval ibex_interval(const Ref& r)
  {
    return is_none(r) ? interval::emptyset() : interval(r.lo, r.hi);
  }

  // The opposite of a reference
  inline Ref ibex_opposite(const Ref& r)
  {
    return is_none(r) ? r : Ref{ -r.hi, -r.lo };
  }

  inline std::string ibex_label(const char *name, const char *variant)
  {
    return std::string(name) + " " + variant;
  }

  //---------------------------------------------------------------------------
  // The constants of IBEX's intervals (Interval::pi() and the others)
  //---------------------------------------------------------------------------

  struct ibex_constant_case
  {
    const char *name, *constant;
    Ref result;
  };

  // BEGIN ibex_constant_cases (written by ibex_forward_cases.py: do not edit)
  static const ibex_constant_case ibex_constant_cases[] = {
    { "IBEX Interval::empty_set()", "emptyset", ibex_empty },
    { "IBEX Interval::all_reals()", "universe", { -ibex_oo, ibex_oo } },
    { "IBEX Interval::pos_reals()", "positive", { 0x0p+0, ibex_oo } },
    { "IBEX Interval::neg_reals()", "negative", { -ibex_oo, 0x0p+0 } },
    { "IBEX Interval::zero()", "zero", { 0x0p+0, 0x0p+0 } },
    { "IBEX Interval::one()", "one", { 0x1p+0, 0x1p+0 } },
    { "IBEX Interval::pi()", "pi", { 0x1.921fb54442d18p+1, 0x1.921fb54442d19p+1 } },
    { "IBEX Interval::two_pi()", "two_pi", { 0x1.921fb54442d18p+2, 0x1.921fb54442d19p+2 } },
    { "IBEX Interval::half_pi()", "half_pi", { 0x1.921fb54442d18p+0, 0x1.921fb54442d19p+0 } },
  };
  // END ibex_constant_cases

  void ibex_constants()
  {
    typedef interval (*Constant)();
    static const std::map<std::string, Constant> constants = {
      { "emptyset", [] { return interval::emptyset(); } },
      { "universe", [] { return interval::universe(); } },
      { "positive", [] { return interval::positive(); } },
      { "negative", [] { return interval::negative(); } },
      { "zero", [] { return interval::zero(); } },
      { "one", [] { return interval::one(); } },
      { "pi", [] { return interval::pi(); } },
      { "two_pi", [] { return interval::two_pi(); } },
      { "half_pi", [] { return interval::half_pi(); } },
    };
    for (const ibex_constant_case& c : ibex_constant_cases) {
      const interval r = constants.at(c.constant)();
      expect(c.name, r, c.result, [&] { return std::string(c.constant); });
    }
  }

  //---------------------------------------------------------------------------
  // TestInterval: cons01 to cons05, the constructors and the assignment
  //---------------------------------------------------------------------------

  // form: 'd' interval(), 's' interval(a) and the assignment of the double a,
  // 'p' interval(a, b)
  struct ibex_construction_case
  {
    const char *name;
    char form;
    double a, b;
    Ref result;
  };

  // BEGIN ibex_construction_cases (written by ibex_forward_cases.py: do not edit)
  static const ibex_construction_case ibex_construction_cases[] = {
    { "IBEX TestInterval::cons01", 'd', 0x0p+0, 0x0p+0, { -ibex_oo, ibex_oo } },
    { "IBEX TestInterval::cons02", 's', 0x1p+0, 0x0p+0, { 0x1p+0, 0x1p+0 } },
    { "IBEX TestInterval::cons03", 's', -ibex_oo, 0x0p+0, ibex_empty },
    { "IBEX TestInterval::cons04", 's', ibex_oo, 0x0p+0, ibex_empty },
    { "IBEX TestInterval::cons05", 'p', 0x1p+0, 0x0p+0, ibex_empty },
  };
  // END ibex_construction_cases

  void ibex_constructions()
  {
    for (const ibex_construction_case& c : ibex_construction_cases) {
      const auto describe = [&] { return std::string("form ") + c.form + ", a = " + hex(c.a) + ", b = " + hex(c.b); };
      interval constructed, assigned(5.0, 6.0);
      if (c.form == 'd') {
        constructed = interval();
        assigned = interval();
      } else if (c.form == 's') {
        constructed = interval(c.a);
        assigned = c.a;
      } else {
        constructed = interval(c.a, c.b);
        assigned = interval(c.a, c.b);
      }
      expect(ibex_label(c.name, "constructor"), constructed, c.result, describe);
      expect(ibex_label(c.name, "assignment"), assigned, c.result, describe);
    }
  }

  //---------------------------------------------------------------------------
  // TestInterval: eq01 to eq08, == and != (set_eq() and set_neq())
  //---------------------------------------------------------------------------

  struct ibex_equality_case
  {
    const char *name;
    Ref x, y;
    bool equal;
  };

  // BEGIN ibex_equality_cases (written by ibex_forward_cases.py: do not edit)
  static const ibex_equality_case ibex_equality_cases[] = {
    { "IBEX TestInterval::eq01", { -ibex_oo, ibex_oo }, { -ibex_oo, ibex_oo }, true },
    { "IBEX TestInterval::eq02", { -ibex_oo, ibex_oo }, { 0x0p+0, ibex_oo }, false },
    { "IBEX TestInterval::eq03", { 0x0p+0, 0x1p+0 }, { 0x0p+0, 0x1p+0 }, true },
    { "IBEX TestInterval::eq04", { 0x0p+0, 0x1p+1 }, { 0x0p+0, 0x1p+0 }, false },
    { "IBEX TestInterval::eq05", { 0x0p+0, 0x1p+1 }, { 0x1p+0, 0x1.8p+1 }, false },
    { "IBEX TestInterval::eq06", { 0x0p+0, ibex_oo }, { 0x0p+0, ibex_oo }, true },
    { "IBEX TestInterval::eq07", ibex_empty, ibex_empty, true },
    { "IBEX TestInterval::eq08", { 0x0p+0, 0x0p+0 }, { 0x0p+0, 0x0p+0 }, true },
  };
  // END ibex_equality_cases

  void ibex_equalities()
  {
    for (const ibex_equality_case& c : ibex_equality_cases) {
      const interval X = ibex_interval(c.x), Y = ibex_interval(c.y);
      const auto describe = [&] { return "x = " + show(X) + ", y = " + show(Y); };
      expect(ibex_label(c.name, "x==y"), X.set_eq(Y), c.equal, describe);
      expect(ibex_label(c.name, "x!=y"), X.set_neq(Y), !c.equal, describe);
      expect(ibex_label(c.name, "y==x"), Y.set_eq(X), c.equal, describe);
      expect(ibex_label(c.name, "y!=x"), Y.set_neq(X), !c.equal, describe);
    }
  }

  //---------------------------------------------------------------------------
  // TestInterval: hull01 to hull03 (x | z) and inter01 to inter06 (x & z)
  //---------------------------------------------------------------------------

  struct ibex_set_operation_case
  {
    const char *name;
    char operation; // '|' or '&'
    Ref x, z, result;
  };

  // BEGIN ibex_set_operation_cases (written by ibex_forward_cases.py: do not edit)
  static const ibex_set_operation_case ibex_set_operation_cases[] = {
    { "IBEX TestInterval::hull01", '|', { 0x0p+0, 0x1p+0 }, { 0x1p+1, 0x1.8p+1 }, { 0x0p+0, 0x1.8p+1 } },
    { "IBEX TestInterval::hull02", '|', { 0x0p+0, 0x1p+0 }, ibex_empty, { 0x0p+0, 0x1p+0 } },
    { "IBEX TestInterval::hull03", '|', { -0x1p+0, -0x1p+0 }, { 0x0p+0, ibex_oo }, { -0x1p+0, ibex_oo } },
    { "IBEX TestInterval::inter01", '&', { 0x0p+0, 0x1p+0 }, { 0x1p+1, 0x1.8p+1 }, ibex_empty },
    { "IBEX TestInterval::inter02", '&', { 0x0p+0, 0x1p+0 }, ibex_empty, ibex_empty },
    { "IBEX TestInterval::inter03", '&', { 0x0p+0, 0x1p+1 }, { -0x1p+0, 0x1p+0 }, { 0x0p+0, 0x1p+0 } },
    { "IBEX TestInterval::inter04", '&', { 0x0p+0, 0x1p+0 }, { 0x0p+0, ibex_oo }, { 0x0p+0, 0x1p+0 } },
    { "IBEX TestInterval::inter05", '&', { 0x0p+0, 0x1p+0 }, { -ibex_oo, 0x0p+0 }, { 0x0p+0, 0x0p+0 } },
    { "IBEX TestInterval::inter06", '&', { 0x0p+0, 0x1p+0 }, { 0x1p+0, 0x1p+1 }, { 0x1p+0, 0x1p+0 } },
  };
  // END ibex_set_operation_cases

  void ibex_set_operations()
  {
    for (const ibex_set_operation_case& c : ibex_set_operation_cases) {
      const interval X = ibex_interval(c.x), Z = ibex_interval(c.z);
      const auto describe = [&] { return "x = " + show(X) + ", z = " + show(Z); };
      const auto verify = [&](const char *variant, const interval& r) {
        expect(ibex_label(c.name, variant), r, c.result, describe);
      };
      interval a = X, b = Z;
      if (c.operation == '|') {
        verify("x|z", X | Z);
        verify("z|x", Z | X);
        a |= Z;
        verify("x|=z", a);
        b |= X;
        verify("z|=x", b);
        verify("hull(x,z)", gaol::hull(X, Z));
        verify("hull(z,x)", gaol::hull(Z, X));
      } else {
        verify("x&z", X & Z);
        verify("z&x", Z & X);
        a &= Z;
        verify("x&=z", a);
        b &= X;
        verify("z&=x", b);
        verify("intersect(x,z)", gaol::intersect(X, Z));
        verify("intersect(z,x)", gaol::intersect(Z, X));
      }
    }
  }

  //---------------------------------------------------------------------------
  // TestInterval: getters01 to getters06 (lb ub mid rad diam mig mag). The
  // midpoint of an unbounded interval is the convention of IBEX and of GAOL
  // (-MAXREAL for [-oo, b], MAXREAL for [a, +oo], 0 for [-oo, +oo]), the radius
  // and the width of an unbounded interval +oo (IEEE 1788-2015 for the radius,
  // IBEX's rad() of an unbounded interval too: GAOL's rad() is NaN for the
  // empty set where IBEX's is 0, which no test checks)
  //---------------------------------------------------------------------------

  struct ibex_getter_case
  {
    const char *name;
    Ref x;
    double lb, ub;
    Ref mid, rad, diam;
    double mig, mag;
  };

  // BEGIN ibex_getter_cases (written by ibex_forward_cases.py: do not edit)
  static const ibex_getter_case ibex_getter_cases[] = {
    { "IBEX TestInterval::getters01", { 0x0p+0, 0x1p+1 }, 0x0p+0, 0x1p+1, { 0x1p+0, 0x1p+0 }, { 0x1p+0, 0x1p+0 },
      { 0x1p+1, 0x1p+1 }, 0x0p+0, 0x1p+1 },
    { "IBEX TestInterval::getters02", { -0x1.8p+1, -0x1p+0 }, -0x1.8p+1, -0x1p+0, { -0x1p+1, -0x1p+1 },
      { 0x1p+0, 0x1p+0 }, { 0x1p+1, 0x1p+1 }, 0x1p+0, 0x1.8p+1 },
    { "IBEX TestInterval::getters03", { -0x1.8p+1, 0x1p+0 }, -0x1.8p+1, 0x1p+0, { -0x1p+0, -0x1p+0 },
      { 0x1p+1, 0x1p+1 }, { 0x1p+2, 0x1p+2 }, 0x0p+0, 0x1.8p+1 },
    { "IBEX TestInterval::getters04", { -ibex_oo, 0x0p+0 }, -ibex_oo, 0x0p+0,
      { -0x1.fffffffffffffp+1023, -0x1.fffffffffffffp+1023 }, { ibex_oo, ibex_oo }, { ibex_oo, ibex_oo }, 0x0p+0,
      ibex_oo },
    { "IBEX TestInterval::getters05", { -ibex_oo, ibex_oo }, -ibex_oo, ibex_oo, { 0x0p+0, 0x0p+0 },
      { ibex_oo, ibex_oo }, { ibex_oo, ibex_oo }, 0x0p+0, ibex_oo },
    { "IBEX TestInterval::getters06", { 0x1.fffffffffffffp+1023, ibex_oo }, 0x1.fffffffffffffp+1023, ibex_oo,
      { 0x1.fffffffffffffp+1023, 0x1.fffffffffffffp+1023 }, { ibex_oo, ibex_oo }, { ibex_oo, ibex_oo },
      0x1.fffffffffffffp+1023, ibex_oo },
  };
  // END ibex_getter_cases

  void ibex_getters()
  {
    for (const ibex_getter_case& c : ibex_getter_cases) {
      const interval X = ibex_interval(c.x);
      const auto describe = [&] { return "x = " + show(X); };
      expect(ibex_label(c.name, "lb"), X.left(), Ref{ c.lb, c.lb }, describe);
      expect(ibex_label(c.name, "ub"), X.right(), Ref{ c.ub, c.ub }, describe);
      expect(ibex_label(c.name, "mid"), X.midpoint(), c.mid, describe);
      expect(ibex_label(c.name, "rad"), X.rad(), c.rad, describe);
      expect(ibex_label(c.name, "diam"), X.width(), c.diam, describe);
      expect(ibex_label(c.name, "mig"), X.mig(), Ref{ c.mig, c.mig }, describe);
      expect(ibex_label(c.name, "mag"), X.mag(), Ref{ c.mag, c.mag }, describe);
    }
  }

  //---------------------------------------------------------------------------
  // TestInterval: mid01 to mid06, the midpoint. The midpoint of [MAXREAL, +oo]
  // is MAXREAL in GAOL, where IBEX's mid05 accepts -MAXREAL too
  //---------------------------------------------------------------------------

  struct ibex_midpoint_case
  {
    const char *name;
    Ref x, mid;
  };

  // BEGIN ibex_midpoint_cases (written by ibex_forward_cases.py: do not edit)
  static const ibex_midpoint_case ibex_midpoint_cases[] = {
    { "IBEX TestInterval::mid01", { 0x0p+0, 0x1p+1 }, { 0x1p+0, 0x1p+0 } },
    { "IBEX TestInterval::mid02", { -ibex_oo, ibex_oo }, { 0x0p+0, 0x0p+0 } },
    { "IBEX TestInterval::mid03", { 0x0p+0, ibex_oo }, { 0x1.fffffffffffffp+1023, 0x1.fffffffffffffp+1023 } },
    { "IBEX TestInterval::mid04", { -ibex_oo, 0x0p+0 }, { -0x1.fffffffffffffp+1023, -0x1.fffffffffffffp+1023 } },
    { "IBEX TestInterval::mid05", { 0x1.fffffffffffffp+1023, ibex_oo },
      { 0x1.fffffffffffffp+1023, 0x1.fffffffffffffp+1023 } },
    { "IBEX TestInterval::mid06", { 0x0p+0, 0x0p+0 }, { 0x0p+0, 0x0p+0 } },
  };
  // END ibex_midpoint_cases

  void ibex_midpoints()
  {
    for (const ibex_midpoint_case& c : ibex_midpoint_cases) {
      const interval X = ibex_interval(c.x);
      expect(ibex_label(c.name, "mid"), X.midpoint(), c.mid, [&] { return "x = " + show(X); });
    }
  }

  //---------------------------------------------------------------------------
  // TestInterval: mid07 and mid08, (x + m) - m contains x, m being the midpoint
  // of x (mid08: the 30 components of a vector of IBEX, one row each)
  //---------------------------------------------------------------------------

  // BEGIN ibex_midpoint_roundtrip_cases (written by ibex_forward_cases.py: do not edit)
  static const ibex_midpoint_case ibex_midpoint_roundtrip_cases[] = {
    { "IBEX TestInterval::mid07", { 0x1.61a81239b7e0cp+1, 0x1.61a81239b7e1ap+1 },
      { 0x1.61a81239b7e13p+1, 0x1.61a81239b7e13p+1 } },
    { "IBEX TestInterval::mid08 [0]", { 0x1.bde6ce0d28d5cp+1, 0x1.bde6ce0d28d64p+1 },
      { 0x1.bde6ce0d28d6p+1, 0x1.bde6ce0d28d6p+1 } },
    { "IBEX TestInterval::mid08 [1]", { 0x1.c9a6d64320a47p+1, 0x1.c9a6d64320a51p+1 },
      { 0x1.c9a6d64320a4cp+1, 0x1.c9a6d64320a4cp+1 } },
    { "IBEX TestInterval::mid08 [2]", { 0x1.9ebbc36d16ddp+1, 0x1.9ebbc36d16dd8p+1 },
      { 0x1.9ebbc36d16dd4p+1, 0x1.9ebbc36d16dd4p+1 } },
    { "IBEX TestInterval::mid08 [3]", { 0x1.c26719881c1dbp+1, 0x1.c26719881c1ep+1 },
      { 0x1.c26719881c1ddp+1, 0x1.c26719881c1dep+1 } },
    { "IBEX TestInterval::mid08 [4]", { 0x1.b4986653f4f6cp+1, 0x1.b4986653f4f78p+1 },
      { 0x1.b4986653f4f72p+1, 0x1.b4986653f4f72p+1 } },
    { "IBEX TestInterval::mid08 [5]", { 0x1.ab08ea6bba40ap+1, 0x1.ab08ea6bba411p+1 },
      { 0x1.ab08ea6bba40dp+1, 0x1.ab08ea6bba40ep+1 } },
    { "IBEX TestInterval::mid08 [6]", { 0x1.b2d3c69d3bf4fp+1, 0x1.b2d3c69d3bf62p+1 },
      { 0x1.b2d3c69d3bf58p+1, 0x1.b2d3c69d3bf59p+1 } },
    { "IBEX TestInterval::mid08 [7]", { 0x1.8b156def9d7ep+1, 0x1.8b156def9d7e7p+1 },
      { 0x1.8b156def9d7e3p+1, 0x1.8b156def9d7e4p+1 } },
    { "IBEX TestInterval::mid08 [8]", { 0x1.956d5bb3e2fdfp+1, 0x1.956d5bb3e2fedp+1 },
      { 0x1.956d5bb3e2fe6p+1, 0x1.956d5bb3e2fe6p+1 } },
    { "IBEX TestInterval::mid08 [9]", { 0x1.a3c92db0b5b9cp+1, 0x1.a3c92db0b5ba3p+1 },
      { 0x1.a3c92db0b5b9fp+1, 0x1.a3c92db0b5bap+1 } },
    { "IBEX TestInterval::mid08 [10]", { 0x1.7f90b8cd04e44p+1, 0x1.7f90b8cd04e4cp+1 },
      { 0x1.7f90b8cd04e48p+1, 0x1.7f90b8cd04e48p+1 } },
    { "IBEX TestInterval::mid08 [11]", { 0x1.bb275ccd1796cp+1, 0x1.bb275ccd17974p+1 },
      { 0x1.bb275ccd1797p+1, 0x1.bb275ccd1797p+1 } },
    { "IBEX TestInterval::mid08 [12]", { 0x1.9999999999996p-2, 0x1.99999999999a7p-2 },
      { 0x1.999999999999ep-2, 0x1.999999999999fp-2 } },
    { "IBEX TestInterval::mid08 [13]", { 0x1.f5a7cecdb6848p+0, 0x1.f5a7cecdb684cp+0 },
      { 0x1.f5a7cecdb684ap+0, 0x1.f5a7cecdb684ap+0 } },
    { "IBEX TestInterval::mid08 [14]", { 0x1.33a27a662e4eap+1, 0x1.33a27a662e4eep+1 },
      { 0x1.33a27a662e4ecp+1, 0x1.33a27a662e4ecp+1 } },
    { "IBEX TestInterval::mid08 [15]", { -0x1.80a6eacc78a9bp+0, -0x1.80a6eacc78a91p+0 },
      { -0x1.80a6eacc78a96p+0, -0x1.80a6eacc78a96p+0 } },
    { "IBEX TestInterval::mid08 [16]", { 0x1.b9475d80d24bbp+1, 0x1.b9475d80d24c5p+1 },
      { 0x1.b9475d80d24cp+1, 0x1.b9475d80d24cp+1 } },
    { "IBEX TestInterval::mid08 [17]", { 0x1.6bbda51fdc30cp+1, 0x1.6bbda51fdc311p+1 },
      { 0x1.6bbda51fdc30ep+1, 0x1.6bbda51fdc30fp+1 } },
    { "IBEX TestInterval::mid08 [18]", { 0x1.d1325f26b0595p+1, 0x1.d1325f26b05a5p+1 },
      { 0x1.d1325f26b059dp+1, 0x1.d1325f26b059dp+1 } },
    { "IBEX TestInterval::mid08 [19]", { 0x1.80ffdb09792e2p+1, 0x1.80ffdb09792eep+1 },
      { 0x1.80ffdb09792e8p+1, 0x1.80ffdb09792e8p+1 } },
    { "IBEX TestInterval::mid08 [20]", { 0x1.ef90f7b024bdcp+1, 0x1.ef90f7b024be9p+1 },
      { 0x1.ef90f7b024be2p+1, 0x1.ef90f7b024be3p+1 } },
    { "IBEX TestInterval::mid08 [21]", { 0x1.76ea482354de1p+1, 0x1.76ea482354df9p+1 },
      { 0x1.76ea482354dedp+1, 0x1.76ea482354dedp+1 } },
    { "IBEX TestInterval::mid08 [22]", { 0x1.d7a5f60a46bp+1, 0x1.d7a5f60a46b09p+1 },
      { 0x1.d7a5f60a46b04p+1, 0x1.d7a5f60a46b05p+1 } },
    { "IBEX TestInterval::mid08 [23]", { 0x1.61a81239b7e0cp+1, 0x1.61a81239b7e1ap+1 },
      { 0x1.61a81239b7e13p+1, 0x1.61a81239b7e13p+1 } },
    { "IBEX TestInterval::mid08 [24]", { 0x1.bfbaf46468a22p+1, 0x1.bfbaf46468a2cp+1 },
      { 0x1.bfbaf46468a27p+1, 0x1.bfbaf46468a27p+1 } },
    { "IBEX TestInterval::mid08 [25]", { 0x1.4c65dc501ae37p+1, 0x1.4c65dc501ae3cp+1 },
      { 0x1.4c65dc501ae39p+1, 0x1.4c65dc501ae3ap+1 } },
    { "IBEX TestInterval::mid08 [26]", { 0x1.4p+2, 0x1.4p+2 }, { 0x1.4p+2, 0x1.4p+2 } },
    { "IBEX TestInterval::mid08 [27]", { 0x0p+0, 0x0p+0 }, { 0x0p+0, 0x0p+0 } },
    { "IBEX TestInterval::mid08 [28]", { 0x0p+0, 0x0p+0 }, { 0x0p+0, 0x0p+0 } },
    { "IBEX TestInterval::mid08 [29]", { 0x0p+0, 0x0p+0 }, { 0x0p+0, 0x0p+0 } },
  };
  // END ibex_midpoint_roundtrip_cases

  void ibex_midpoint_roundtrips()
  {
    for (const ibex_midpoint_case& c : ibex_midpoint_roundtrip_cases) {
      const interval X = ibex_interval(c.x);
      const double m = X.midpoint();
      const interval around = (X + m) - m;
      const auto describe = [&] { return "x = " + show(X) + ", m = " + hex(m) + ", (x+m)-m = " + show(around); };
      expect(ibex_label(c.name, "mid"), m, c.mid, describe);
      expect(ibex_label(c.name, "(x+m)-m contains x"), around.set_contains(X), true, describe);
    }
  }

  //---------------------------------------------------------------------------
  // TestInterval: is_subset01 to is_strict_superset03, is_disjoint01 and 02: the
  // relations between two intervals, keyed by the GAOL expression
  //---------------------------------------------------------------------------

  struct ibex_relation_case
  {
    const char *name, *relation;
    Ref x, y;
    bool result;
  };

  // BEGIN ibex_relation_cases (written by ibex_forward_cases.py: do not edit)
  static const ibex_relation_case ibex_relation_cases[] = {
    { "IBEX TestInterval::is_subset01", "set_leq", { 0x0p+0, 0x1p+0 }, { 0x0p+0, 0x1p+1 }, true },
    { "IBEX TestInterval::is_subset02", "set_leq", { 0x0p+0, 0x1p+0 }, { 0x1p+0, 0x1p+1 }, false },
    { "IBEX TestInterval::is_strict_interior_subset01", "set_le_and_neq", { 0x0p+0, 0x1p+0 }, { 0x0p+0, 0x1p+1 },
      false },
    { "IBEX TestInterval::is_strict_interior_subset02", "set_le_and_neq", { -ibex_oo, 0x0p+0 }, { -ibex_oo, ibex_oo },
      true },
    { "IBEX TestInterval::is_strict_interior_subset03", "set_le_and_neq", ibex_empty, { 0x0p+0, 0x1p+0 }, true },
    { "IBEX TestInterval::is_superset01", "set_geq", { 0x0p+0, 0x1p+1 }, { 0x0p+0, 0x1p+0 }, true },
    { "IBEX TestInterval::is_superset02", "set_geq", { 0x1p+0, 0x1p+1 }, { 0x0p+0, 0x1p+0 }, false },
    { "IBEX TestInterval::is_strict_superset01", "set_geq_and_neq", { 0x0p+0, 0x1p+1 }, { 0x0p+0, 0x1p+1 }, false },
    { "IBEX TestInterval::is_strict_superset02", "set_geq_and_neq", { -ibex_oo, ibex_oo }, { -ibex_oo, 0x0p+0 }, true },
    { "IBEX TestInterval::is_strict_superset03", "set_geq_and_neq", ibex_empty, ibex_empty, false },
    { "IBEX TestInterval::is_disjoint01", "set_disjoint", { -ibex_oo, 0x0p+0 }, { 0x0p+0, ibex_oo }, false },
    { "IBEX TestInterval::is_disjoint02", "set_disjoint", { 0x0p+0, 0x1p+0 }, { 0x1p+1, 0x1.8p+1 }, true },
  };
  // END ibex_relation_cases

  typedef bool (*ibex_set_relation)(const interval&, const interval&);

  const std::map<std::string, ibex_set_relation>& ibex_set_relations()
  {
    static const std::map<std::string, ibex_set_relation> relations = {
      { "set_leq", [](const interval& x, const interval& y) { return x.set_leq(y); } },
      { "set_geq", [](const interval& x, const interval& y) { return x.set_geq(y); } },
      { "set_le_and_neq", [](const interval& x, const interval& y) { return x.set_le(y) && x.set_neq(y); } },
      { "set_geq_and_neq", [](const interval& x, const interval& y) { return x.set_geq(y) && x.set_neq(y); } },
      { "set_disjoint", [](const interval& x, const interval& y) { return x.set_disjoint(y); } },
    };
    return relations;
  }

  // contains01 and 02, interior_contains01 and 02: a double in an interval
  struct ibex_point_relation_case
  {
    const char *name, *relation;
    Ref x;
    double d;
    bool result;
  };

  // BEGIN ibex_point_relation_cases (written by ibex_forward_cases.py: do not edit)
  static const ibex_point_relation_case ibex_point_relation_cases[] = {
    { "IBEX TestInterval::contains01", "set_contains", { 0x0p+0, 0x1p+1 }, 0x0p+0, true },
    { "IBEX TestInterval::contains02", "set_contains", { -ibex_oo, ibex_oo }, ibex_oo, true },
    { "IBEX TestInterval::interior_contains01", "set_strictly_contains", { 0x0p+0, 0x1p+1 }, 0x0p+0, false },
    { "IBEX TestInterval::interior_contains02", "set_strictly_contains", { 0x0p+0, 0x1p+1 }, 0x1p+0, true },
  };
  // END ibex_point_relation_cases

  typedef bool (*ibex_point_relation)(const interval&, double);

  const std::map<std::string, ibex_point_relation>& ibex_point_relations()
  {
    static const std::map<std::string, ibex_point_relation> relations = {
      { "set_contains", [](const interval& x, double d) { return x.set_contains(d); } },
      { "set_strictly_contains", [](const interval& x, double d) { return x.set_strictly_contains(d); } },
    };
    return relations;
  }

  // is_empty01 and 02, is_degenerated01 and 02, is_unbounded01 to 04,
  // is_bisectable01 to 10: a property of an interval
  struct ibex_predicate_case
  {
    const char *name, *predicate;
    Ref x;
    bool result;
  };

  // BEGIN ibex_predicate_cases (written by ibex_forward_cases.py: do not edit)
  static const ibex_predicate_case ibex_predicate_cases[] = {
    { "IBEX TestInterval::is_empty01", "is_empty", ibex_empty, true },
    { "IBEX TestInterval::is_empty02", "is_empty", { 0x0p+0, 0x1p+0 }, false },
    { "IBEX TestInterval::is_degenerated01", "is_empty_or_a_double", { 0x0p+0, 0x0p+0 }, true },
    { "IBEX TestInterval::is_degenerated02", "is_empty_or_a_double", ibex_empty, true },
    { "IBEX TestInterval::is_unbounded01", "is_not_finite", ibex_empty, false },
    { "IBEX TestInterval::is_unbounded02", "is_not_finite", { 0x0p+0, 0x1p+0 }, false },
    { "IBEX TestInterval::is_unbounded03", "is_not_finite", { -ibex_oo, 0x0p+0 }, true },
    { "IBEX TestInterval::is_unbounded04", "is_not_finite", { 0x0p+0, ibex_oo }, true },
    { "IBEX TestInterval::is_bisectable01", "is_bisectable", ibex_empty, false },
    { "IBEX TestInterval::is_bisectable02", "is_bisectable", { 0x1.fffffffffffffp+1023, ibex_oo }, false },
    { "IBEX TestInterval::is_bisectable03", "is_bisectable", { -ibex_oo, -0x1.fffffffffffffp+1023 }, false },
    { "IBEX TestInterval::is_bisectable04", "is_bisectable", { 0x0p+0, 0x0.0000000000001p-1022 }, false },
    { "IBEX TestInterval::is_bisectable05", "is_bisectable", { 0x0p+0, 0x0.0000000000002p-1022 }, true },
    { "IBEX TestInterval::is_bisectable06", "is_bisectable", { 0x1.4p+3, 0x1.4000000000002p+3 }, true },
    { "IBEX TestInterval::is_bisectable07", "is_bisectable", { -0x0.0000000000002p-1022, 0x0p+0 }, true },
    { "IBEX TestInterval::is_bisectable08", "is_bisectable", { 0x1.3fffffffffffep+3, 0x1.4p+3 }, true },
    { "IBEX TestInterval::is_bisectable09", "is_bisectable", { 0x1.4p+3, 0x1.4000000000001p+3 }, false },
    { "IBEX TestInterval::is_bisectable10", "is_bisectable", { -0x0.0000000000001p-1022, 0x0p+0 }, false },
  };
  // END ibex_predicate_cases

  typedef bool (*ibex_predicate)(const interval&);

  const std::map<std::string, ibex_predicate>& ibex_predicates()
  {
    static const std::map<std::string, ibex_predicate> predicates = {
      { "is_empty", [](const interval& x) { return x.is_empty(); } },
      { "is_empty_or_a_double", [](const interval& x) { return x.is_empty() || x.is_a_double(); } },
      { "is_not_finite", [](const interval& x) { return !x.is_finite(); } },
      { "is_bisectable", [](const interval& x) { return x.is_bisectable(); } },
    };
    return predicates;
  }

  void ibex_relations()
  {
    for (const ibex_relation_case& c : ibex_relation_cases) {
      const interval X = ibex_interval(c.x), Y = ibex_interval(c.y);
      expect(c.name, ibex_set_relations().at(c.relation)(X, Y), c.result,
             [&] { return std::string(c.relation) + "(" + show(X) + ", " + show(Y) + ")"; });
    }
    for (const ibex_point_relation_case& c : ibex_point_relation_cases) {
      const interval X = ibex_interval(c.x);
      expect(c.name, ibex_point_relations().at(c.relation)(X, c.d), c.result,
             [&] { return std::string(c.relation) + "(" + show(X) + ", " + hex(c.d) + ")"; });
    }
    for (const ibex_predicate_case& c : ibex_predicate_cases) {
      const interval X = ibex_interval(c.x);
      expect(c.name, ibex_predicates().at(c.predicate)(X), c.result,
             [&] { return std::string(c.predicate) + "(" + show(X) + ")"; });
    }
  }

  //---------------------------------------------------------------------------
  // TestArith: float01 to float09, next_float() and previous_float(), in the
  // rounding direction GAOL leaves and to nearest (the one IBEX's wrapper sets
  // on 64-bit ARM, where nextafter() crashed otherwise on macOS)
  //---------------------------------------------------------------------------

  // direction: 'n' next_float(), 'p' previous_float()
  struct ibex_float_case
  {
    const char *name;
    char direction;
    double x;
    Ref result;
  };

  // BEGIN ibex_float_cases (written by ibex_forward_cases.py: do not edit)
  static const ibex_float_case ibex_float_cases[] = {
    { "IBEX TestArith::float01", 'n', 0x0p+0, { 0x0.0000000000001p-1022, 0x0.0000000000001p-1022 } },
    { "IBEX TestArith::float02", 'p', 0x0p+0, { -0x0.0000000000001p-1022, -0x0.0000000000001p-1022 } },
    { "IBEX TestArith::float03", 'n', 0x1p+0, { 0x1.0000000000001p+0, 0x1.0000000000001p+0 } },
    { "IBEX TestArith::float04", 'p', 0x1p+0, { 0x1.fffffffffffffp-1, 0x1.fffffffffffffp-1 } },
    { "IBEX TestArith::float05", 'n', ibex_oo, { ibex_oo, ibex_oo } },
    { "IBEX TestArith::float06", 'n', -ibex_oo, { -0x1.fffffffffffffp+1023, -0x1.fffffffffffffp+1023 } },
    { "IBEX TestArith::float07", 'p', ibex_oo, { 0x1.fffffffffffffp+1023, 0x1.fffffffffffffp+1023 } },
    { "IBEX TestArith::float08", 'p', -ibex_oo, { -ibex_oo, -ibex_oo } },
    { "IBEX TestArith::float09", 'n', 0x1.fffffffffffffp+1023, { ibex_oo, ibex_oo } },
    { "IBEX TestArith::float10", 'p', -0x1.fffffffffffffp+1023, { -ibex_oo, -ibex_oo } },
  };
  // END ibex_float_cases

  void ibex_floats()
  {
    for (const ibex_float_case& c : ibex_float_cases) {
      const auto describe = [&] { return std::string(c.direction == 'n' ? "next_float(" : "previous_float(") + hex(c.x) + ")"; };
      const auto neighbour = [&] { return c.direction == 'n' ? gaol::next_float(c.x) : gaol::previous_float(c.x); };
      expect(ibex_label(c.name, "GAOL's rounding"), neighbour(), c.result, describe);
      const RoundingToNearest nearest;
      expect(ibex_label(c.name, "to nearest"), neighbour(), c.result, describe);
    }
  }

  //---------------------------------------------------------------------------
  // TestArith: add01 to add19 (check_add, check_add_scal), mul01 to mul19
  // (check_mul, check_mul_scal), div01 to div17 (check_div, check_div_scal).
  // scalar: z is the double z.lo (an infinite double is the empty set)
  //---------------------------------------------------------------------------

  struct ibex_binary_case
  {
    const char *name;
    char operation; // '+', '*' or '/'
    Ref x, z;
    bool scalar;
    Ref result;
  };

  // BEGIN ibex_binary_cases (written by ibex_forward_cases.py: do not edit)
  static const ibex_binary_case ibex_binary_cases[] = {
    { "IBEX TestArith::add01", '+', ibex_empty, { 0x0p+0, 0x1p+0 }, false, ibex_empty },
    { "IBEX TestArith::add02", '+', { 0x0p+0, 0x1p+0 }, ibex_empty, false, ibex_empty },
    { "IBEX TestArith::add03", '+', { -ibex_oo, 0x1p+0 }, { 0x0p+0, 0x1p+0 }, false, { -ibex_oo, 0x1p+1 } },
    { "IBEX TestArith::add04", '+', { 0x1p+0, ibex_oo }, { 0x0p+0, 0x1p+0 }, false, { 0x1p+0, ibex_oo } },
    { "IBEX TestArith::add05", '+', { -ibex_oo, ibex_oo }, { 0x0p+0, 0x1p+0 }, false, { -ibex_oo, ibex_oo } },
    { "IBEX TestArith::add06", '+', ibex_empty, { ibex_oo, ibex_oo }, true, ibex_empty },
    { "IBEX TestArith::add07", '+', ibex_empty, { 0x0p+0, 0x0p+0 }, true, ibex_empty },
    { "IBEX TestArith::add08", '+', { 0x0p+0, 0x1p+0 }, { 0x1p+0, 0x1p+0 }, true, { 0x1p+0, 0x1p+1 } },
    { "IBEX TestArith::add09", '+', { 0x0p+0, 0x1p+0 }, { -ibex_oo, -ibex_oo }, true, ibex_empty },
    { "IBEX TestArith::add10", '+', { 0x0p+0, 0x1p+0 }, { ibex_oo, ibex_oo }, true, ibex_empty },
    { "IBEX TestArith::add11", '+', { -ibex_oo, 0x1p+0 }, { 0x1p+0, 0x1p+0 }, true, { -ibex_oo, 0x1p+1 } },
    { "IBEX TestArith::add12", '+', { 0x1.fffffffffffffp+1023, ibex_oo }, { 0x1p+0, 0x1p+0 }, false,
      { 0x1.fffffffffffffp+1023, ibex_oo } },
    { "IBEX TestArith::add13", '+', { 0x1.fffffffffffffp+1023, ibex_oo }, { -0x1p+0, -0x1p+0 }, false,
      { 0x1.ffffffffffffep+1023, ibex_oo } },
    { "IBEX TestArith::add14", '+', { 0x1.fffffffffffffp+1023, ibex_oo }, { 0x1.fffffffffffffp+1023, ibex_oo }, false,
      { 0x1.fffffffffffffp+1023, ibex_oo } },
    { "IBEX TestArith::add15", '+', { 0x1.fffffffffffffp+1023, ibex_oo }, ibex_empty, false, ibex_empty },
    { "IBEX TestArith::add16", '+', { 0x1.fffffffffffffp+1023, ibex_oo }, ibex_empty, false, ibex_empty },
    { "IBEX TestArith::add17", '+', { -ibex_oo, -0x1.fffffffffffffp+1023 }, { 0x1p+0, 0x1p+0 }, false,
      { -ibex_oo, -0x1.ffffffffffffep+1023 } },
    { "IBEX TestArith::add18", '+', { -ibex_oo, -0x1.fffffffffffffp+1023 }, { -0x1p+0, -0x1p+0 }, false,
      { -ibex_oo, -0x1.fffffffffffffp+1023 } },
    { "IBEX TestArith::add19", '+', { -ibex_oo, -0x1.fffffffffffffp+1023 }, { -ibex_oo, -0x1.fffffffffffffp+1023 },
      false, { -ibex_oo, -0x1.fffffffffffffp+1023 } },
    { "IBEX TestArith::mul01", '*', ibex_empty, { 0x0p+0, 0x1p+0 }, false, ibex_empty },
    { "IBEX TestArith::mul02", '*', { 0x0p+0, 0x0p+0 }, { -ibex_oo, ibex_oo }, false, { 0x0p+0, 0x0p+0 } },
    { "IBEX TestArith::mul03", '*', { -0x1p+0, 0x1p+0 }, { -ibex_oo, 0x0p+0 }, false, { -ibex_oo, ibex_oo } },
    { "IBEX TestArith::mul04", '*', { -ibex_oo, -0x1p+0 }, { -0x1p+0, 0x0p+0 }, false, { 0x0p+0, ibex_oo } },
    { "IBEX TestArith::mul05", '*', { -ibex_oo, 0x1p+0 }, { -0x1p+0, 0x0p+0 }, false, { -0x1p+0, ibex_oo } },
    { "IBEX TestArith::mul06", '*', { 0x0p+0, 0x1p+0 }, { 0x1p+0, ibex_oo }, false, { 0x0p+0, ibex_oo } },
    { "IBEX TestArith::mul07", '*', { 0x0p+0, 0x1p+0 }, { -0x1p+0, ibex_oo }, false, { -0x1p+0, ibex_oo } },
    { "IBEX TestArith::mul08", '*', { -ibex_oo, -0x1p+0 }, { 0x0p+0, 0x1p+0 }, false, { -ibex_oo, 0x0p+0 } },
    { "IBEX TestArith::mul09", '*', { -ibex_oo, 0x1p+0 }, { 0x0p+0, 0x1p+0 }, false, { -ibex_oo, 0x1p+0 } },
    { "IBEX TestArith::mul10", '*', { 0x0p+0, 0x1p+0 }, { -ibex_oo, -0x1p+0 }, false, { -ibex_oo, 0x0p+0 } },
    { "IBEX TestArith::mul11", '*', { 0x0p+0, 0x1p+0 }, { -ibex_oo, 0x1p+0 }, false, { -ibex_oo, 0x1p+0 } },
    { "IBEX TestArith::mul12", '*', { 0x1p+0, ibex_oo }, { 0x0p+0, 0x1p+0 }, false, { 0x0p+0, ibex_oo } },
    { "IBEX TestArith::mul13", '*', { -0x1p+0, ibex_oo }, { 0x0p+0, 0x1p+0 }, false, { -0x1p+0, ibex_oo } },
    { "IBEX TestArith::mul14", '*', { 0x1p+0, 0x1p+1 }, { 0x1p+0, 0x1p+1 }, false, { 0x1p+0, 0x1p+2 } },
    { "IBEX TestArith::mul15", '*', { 0x1p+0, 0x1p+1 }, { -0x1p+1, 0x1.8p+1 }, false, { -0x1p+2, 0x1.8p+2 } },
    { "IBEX TestArith::mul16", '*', { -0x1p+0, 0x1p+0 }, { -0x1p+1, 0x1.8p+1 }, false, { -0x1.8p+1, 0x1.8p+1 } },
    { "IBEX TestArith::mul17", '*', { 0x1p+0, 0x1p+1 }, { -ibex_oo, -ibex_oo }, true, ibex_empty },
    { "IBEX TestArith::mul18", '*', { 0x1p+0, 0x1p+1 }, { ibex_oo, ibex_oo }, true, ibex_empty },
    { "IBEX TestArith::mul19", '*', { 0x1p+0, 0x1p+1 }, { -0x1p+0, -0x1p+0 }, true, { -0x1p+1, -0x1p+0 } },
    { "IBEX TestArith::div01", '/', ibex_empty, { 0x0p+0, 0x1p+0 }, false, ibex_empty },
    { "IBEX TestArith::div02", '/', { 0x0p+0, 0x0p+0 }, { 0x0p+0, 0x0p+0 }, false, ibex_empty },
    { "IBEX TestArith::div03", '/', { 0x1p+0, 0x1p+1 }, { 0x0p+0, 0x0p+0 }, false, ibex_empty },
    { "IBEX TestArith::div04", '/', { -ibex_oo, ibex_oo }, { 0x0p+0, 0x0p+0 }, false, ibex_empty },
    { "IBEX TestArith::div05", '/', { 0x0p+0, 0x0p+0 }, { 0x0p+0, 0x1p+0 }, false, { 0x0p+0, 0x0p+0 } },
    { "IBEX TestArith::div06", '/', { 0x0p+0, 0x0p+0 }, { -ibex_oo, ibex_oo }, false, { 0x0p+0, 0x0p+0 } },
    { "IBEX TestArith::div07", '/', { 0x1.8p+2, 0x1.8p+3 }, { 0x1p+1, 0x1.8p+1 }, false, { 0x1p+1, 0x1.8p+2 } },
    { "IBEX TestArith::div08", '/', { 0x1.8p+2, 0x1.8p+3 }, { -0x1.8p+1, -0x1p+1 }, false, { -0x1.8p+2, -0x1p+1 } },
    { "IBEX TestArith::div09", '/', { 0x1.8p+2, 0x1.8p+3 }, { -0x1p+1, 0x1.8p+1 }, false, { -ibex_oo, ibex_oo } },
    { "IBEX TestArith::div10", '/', { -ibex_oo, -0x1p+0 }, { -0x1p+0, 0x0p+0 }, false, { 0x1p+0, ibex_oo } },
    { "IBEX TestArith::div11", '/', { -ibex_oo, -0x1p+0 }, { 0x0p+0, 0x1p+0 }, false, { -ibex_oo, -0x1p+0 } },
    { "IBEX TestArith::div12", '/', { 0x1p+0, ibex_oo }, { -0x1p+0, 0x0p+0 }, false, { -ibex_oo, -0x1p+0 } },
    { "IBEX TestArith::div13", '/', { 0x1p+0, ibex_oo }, { 0x0p+0, 0x1p+0 }, false, { 0x1p+0, ibex_oo } },
    { "IBEX TestArith::div14", '/', { -0x1p+0, 0x1p+0 }, { -0x1p+0, 0x1p+0 }, false, { -ibex_oo, ibex_oo } },
    { "IBEX TestArith::div15", '/', { 0x1p+0, 0x1p+1 }, { -ibex_oo, -ibex_oo }, true, ibex_empty },
    { "IBEX TestArith::div16", '/', { 0x1p+0, 0x1p+1 }, { ibex_oo, ibex_oo }, true, ibex_empty },
    { "IBEX TestArith::div17", '/', { 0x1p+0, 0x1p+1 }, { -0x1p+0, -0x1p+0 }, true, { -0x1p+1, -0x1p+0 } },
  };
  // END ibex_binary_cases

  void ibex_binary(const ibex_binary_case& c)
  {
    const interval X = ibex_interval(c.x), Z = ibex_interval(c.z);
    const double d = c.z.lo;
    const Ref minus_result = ibex_opposite(c.result);
    const auto describe = [&] { return "x = " + show(X) + ", z = " + (c.scalar ? hex(d) : show(Z)); };
    const auto verify = [&](const char *variant, const interval& r, const Ref& expected) {
      expect(ibex_label(c.name, variant), r, expected, describe);
    };
    interval y;
    switch (c.operation) {
      case '+':
        if (c.scalar) {
          verify("x+d", X + d, c.result);
          verify("d+x", d + X, c.result);
          y = X;
          y += d;
          verify("x+=d", y, c.result);
          verify("(-x)-d", -X - d, minus_result);
          verify("(-d)-x", (-d) - X, minus_result);
          y = -X;
          y -= d;
          verify("(-x)-=d", y, minus_result);
        } else {
          verify("x+z", X + Z, c.result);
          verify("z+x", Z + X, c.result);
          y = X;
          y += Z;
          verify("x+=z", y, c.result);
          y = Z;
          y += X;
          verify("z+=x", y, c.result);
          verify("(-x)-z", -X - Z, minus_result);
          verify("(-z)-x", -Z - X, minus_result);
          y = -X;
          y -= Z;
          verify("(-x)-=z", y, minus_result);
          y = -Z;
          y -= X;
          verify("(-z)-=x", y, minus_result);
        }
        break;
      case '*':
        if (c.scalar) {
          verify("x*d", X * d, c.result);
          verify("d*x", d * X, c.result);
          y = X;
          y *= d;
          verify("x*=d", y, c.result);
        } else {
          verify("x*z", X * Z, c.result);
          verify("z*x", Z * X, c.result);
          y = X;
          y *= Z;
          verify("x*=z", y, c.result);
        }
        break;
      default:
        if (c.scalar) {
          verify("x/d", X / d, c.result);
          y = X;
          y /= d;
          verify("x/=d", y, c.result);
        } else {
          verify("x/z", X / Z, c.result);
          y = X;
          y /= Z;
          verify("x/=z", y, c.result);
        }
        break;
    }
  }

  //---------------------------------------------------------------------------
  // TestArith: minus01 to minus04 (-x), sqrt01 to sqrt05, log01 to log11,
  // exp01 to exp09, floor01 to floor08, ceil01 to ceil08, integer01 to
  // integer08, tan01, tan03, tan04 and tan_issue248 (tan of an interval).
  // log07 checks the whole interval, IBEX only its upper bound
  //---------------------------------------------------------------------------

  struct ibex_unary_case
  {
    const char *name, *function;
    Ref x, result;
  };

  // BEGIN ibex_unary_cases (written by ibex_forward_cases.py: do not edit)
  static const ibex_unary_case ibex_unary_cases[] = {
    { "IBEX TestArith::minus01", "neg", { 0x0p+0, 0x1p+0 }, { -0x1p+0, 0x0p+0 } },
    { "IBEX TestArith::minus02", "neg", { -ibex_oo, ibex_oo }, { -ibex_oo, ibex_oo } },
    { "IBEX TestArith::minus03", "neg", { -ibex_oo, 0x0p+0 }, { 0x0p+0, ibex_oo } },
    { "IBEX TestArith::minus04", "neg", { -ibex_oo, 0x1p+0 }, { -0x1p+0, ibex_oo } },
    { "IBEX TestArith::sqrt01", "sqrt", { -ibex_oo, ibex_oo }, { 0x0p+0, ibex_oo } },
    { "IBEX TestArith::sqrt02", "sqrt", { -ibex_oo, 0x0p+0 }, { 0x0p+0, 0x0p+0 } },
    { "IBEX TestArith::sqrt03", "sqrt", { -0x1.2p+3, 0x1p+2 }, { 0x0p+0, 0x1p+1 } },
    { "IBEX TestArith::sqrt04", "sqrt", { 0x1p+2, 0x1.2p+3 }, { 0x1p+1, 0x1.8p+1 } },
    { "IBEX TestArith::sqrt05", "sqrt", { -0x1.2p+3, -0x1p+2 }, ibex_empty },
    { "IBEX TestArith::sqrt06", "sqrt", { -0x1.2p+3, ibex_oo }, { 0x0p+0, ibex_oo } },
    { "IBEX TestArith::log01", "log", ibex_empty, ibex_empty },
    { "IBEX TestArith::log02", "log", { -ibex_oo, ibex_oo }, { -ibex_oo, ibex_oo } },
    { "IBEX TestArith::log03", "log", { 0x0p+0, ibex_oo }, { -ibex_oo, ibex_oo } },
    { "IBEX TestArith::log04", "log", { -ibex_oo, 0x0p+0 }, ibex_empty },
    { "IBEX TestArith::log05", "log", { 0x1p+0, 0x1p+1 }, { 0x0p+0, 0x1.62e42fefa39fp-1 } },
    { "IBEX TestArith::log06", "log", { -0x1p+0, 0x1p+0 }, { -ibex_oo, 0x0p+0 } },
    { "IBEX TestArith::log07", "log", { 0x0p+0, 0x0.0000000000001p-1022 }, { -ibex_oo, -0x1.74385446d71c3p+9 } },
    { "IBEX TestArith::log08", "log", { 0x0p+0, 0x1p+0 }, { -ibex_oo, 0x0p+0 } },
    { "IBEX TestArith::log09", "log", { 0x1p+0, ibex_oo }, { 0x0p+0, ibex_oo } },
    { "IBEX TestArith::log10", "log", { 0x0p+0, 0x0p+0 }, ibex_empty },
    { "IBEX TestArith::log11", "log", { -0x1p+1, -0x1p+0 }, ibex_empty },
    { "IBEX TestArith::exp01", "exp", ibex_empty, ibex_empty },
    { "IBEX TestArith::exp02", "exp", { -ibex_oo, ibex_oo }, { 0x0p+0, ibex_oo } },
    { "IBEX TestArith::exp03", "exp", { 0x0p+0, ibex_oo }, { 0x1p+0, ibex_oo } },
    { "IBEX TestArith::exp04", "exp", { -ibex_oo, 0x0p+0 }, { 0x0p+0, 0x1p+0 } },
    { "IBEX TestArith::exp05", "exp", { 0x0p+0, 0x1p+1 }, { 0x1p+0, 0x1.d8e64b8d4ddaep+2 } },
    { "IBEX TestArith::exp06", "exp", { -0x1p+0, 0x1p+0 }, { 0x1.78b56362cef37p-2, 0x1.5bf0a8b14576ap+1 } },
    { "IBEX TestArith::exp07", "exp", { 0x1.249ad2594c37dp+332, 0x1.a9cbc59b83a3dp+368 },
      { 0x1.fffffffffffffp+1023, ibex_oo } },
    { "IBEX TestArith::exp08", "exp", { 0x1.fffffffffffffp+1023, ibex_oo }, { 0x1.fffffffffffffp+1023, ibex_oo } },
    { "IBEX TestArith::exp09", "exp", { 0x0p+0, 0x1.fffffffffffffp+1023 }, { 0x1p+0, ibex_oo } },
    { "IBEX TestArith::floor01", "floor", ibex_empty, ibex_empty },
    { "IBEX TestArith::floor02", "floor", { -ibex_oo, -0x1.0c6f7a0b5ed8dp-20 }, { -ibex_oo, -0x1p+0 } },
    { "IBEX TestArith::floor03", "floor", { 0x1.5798ee2308c3ap-27, ibex_oo }, { 0x0p+0, ibex_oo } },
    { "IBEX TestArith::floor04", "floor", { -ibex_oo, ibex_oo }, { -ibex_oo, ibex_oo } },
    { "IBEX TestArith::floor05", "floor", { 0x1.47ae147ae147bp-7, 0x1.7eb851eb851ecp+1 }, { 0x0p+0, 0x1p+1 } },
    { "IBEX TestArith::floor06", "floor", { -0x1.47ae147ae147bp-7, 0x1.7eb851eb851ecp+1 }, { -0x1p+0, 0x1p+1 } },
    { "IBEX TestArith::floor07", "floor", { 0x1.000000044b83p+0, 0x1.ffffffff920c8p+0 }, { 0x1p+0, 0x1p+0 } },
    { "IBEX TestArith::floor08", "floor", { 0x1.7d784p+26, 0x1.fffffffffffffp+1023 },
      { 0x1.7d784p+26, 0x1.fffffffffffffp+1023 } },
    { "IBEX TestArith::ceil01", "ceil", ibex_empty, ibex_empty },
    { "IBEX TestArith::ceil02", "ceil", { -ibex_oo, -0x1.0c6f7a0b5ed8dp-20 }, { -ibex_oo, 0x0p+0 } },
    { "IBEX TestArith::ceil03", "ceil", { 0x1.5798ee2308c3ap-27, ibex_oo }, { 0x1p+0, ibex_oo } },
    { "IBEX TestArith::ceil04", "ceil", { -ibex_oo, ibex_oo }, { -ibex_oo, ibex_oo } },
    { "IBEX TestArith::ceil05", "ceil", { 0x1.47ae147ae147bp-7, 0x1.7eb851eb851ecp+1 }, { 0x1p+0, 0x1.8p+1 } },
    { "IBEX TestArith::ceil06", "ceil", { -0x1.47ae147ae147bp-7, 0x1.7eb851eb851ecp+1 }, { 0x0p+0, 0x1.8p+1 } },
    { "IBEX TestArith::ceil07", "ceil", { 0x1.000000044b83p+0, 0x1.ffffffff920c8p+0 }, { 0x1p+1, 0x1p+1 } },
    { "IBEX TestArith::ceil08", "ceil", { 0x1.7d784p+26, 0x1.fffffffffffffp+1023 },
      { 0x1.7d784p+26, 0x1.fffffffffffffp+1023 } },
    { "IBEX TestArith::integer01", "integer", ibex_empty, ibex_empty },
    { "IBEX TestArith::integer02", "integer", { -ibex_oo, -0x1.0c6f7a0b5ed8dp-20 }, { -ibex_oo, -0x1p+0 } },
    { "IBEX TestArith::integer03", "integer", { 0x1.5798ee2308c3ap-27, ibex_oo }, { 0x1p+0, ibex_oo } },
    { "IBEX TestArith::integer04", "integer", { -ibex_oo, ibex_oo }, { -ibex_oo, ibex_oo } },
    { "IBEX TestArith::integer05", "integer", { 0x1.47ae147ae147bp-7, 0x1.7eb851eb851ecp+1 }, { 0x1p+0, 0x1p+1 } },
    { "IBEX TestArith::integer06", "integer", { -0x1.47ae147ae147bp-7, 0x1.7eb851eb851ecp+1 }, { 0x0p+0, 0x1p+1 } },
    { "IBEX TestArith::integer07", "integer", { 0x1.000000044b83p+0, 0x1.ffffffff920c8p+0 }, ibex_empty },
    { "IBEX TestArith::integer08", "integer", { 0x1.7d784p+26, 0x1.fffffffffffffp+1023 },
      { 0x1.7d784p+26, 0x1.fffffffffffffp+1023 } },
    { "IBEX TestArith::tan01", "tan", { -ibex_oo, ibex_oo }, { -ibex_oo, ibex_oo } },
    { "IBEX TestArith::tan03", "tan", { 0x1.921fb54442d18p-1, 0x1.921fb543961b9p+0 },
      { 0x1.fffffffffffffp-1, 0x1.7b74896a2d0fcp+32 } },
    { "IBEX TestArith::tan04", "tan", { -0x1.921fb543961b8p+0, 0x1.921fb54442d18p-1 },
      { -0x1.7b7466430c12ap+32, 0x1p+0 } },
    { "IBEX TestArith::tan_issue248", "tan", { -0x1.921fb54442d19p+0, 0x1.2d97c7f3321d4p+0 }, { -ibex_oo, ibex_oo } },
  };
  // END ibex_unary_cases

  struct ibex_unary_function
  {
    interval (*f)(const interval&);
    int max_doubles;
  };

  const std::map<std::string, ibex_unary_function>& ibex_unary_functions()
  {
    static const std::map<std::string, ibex_unary_function> functions = {
      { "neg", { [](const interval& x) { return -x; }, 0 } },
      { "sqrt", { [](const interval& x) { return sqrt(x); }, 0 } },
      { "log", { [](const interval& x) { return log(x); }, limit } },
      { "exp", { [](const interval& x) { return exp(x); }, limit } },
      { "floor", { [](const interval& x) { return floor(x); }, 0 } },
      { "ceil", { [](const interval& x) { return ceil(x); }, 0 } },
      { "integer", { [](const interval& x) { return integer(x); }, 0 } },
      { "tan", { [](const interval& x) { return tan(x); }, limit } },
    };
    return functions;
  }

  void ibex_unary(const ibex_unary_case& c)
  {
    const std::string function = c.function;
    const ibex_unary_function& f = ibex_unary_functions().at(function);
    const interval X = ibex_interval(c.x);
    const auto describe = [&] { return std::string(c.function) + "(" + show(X) + ")"; };
    expect(c.name, evaluate(c.name, [&] { return f.f(X); }, describe), c.result, describe, f.max_doubles);
  }

  // tan02 and tan05: tan of an expression of the constants of pi
  struct ibex_expression_case
  {
    const char *name, *expression;
    Ref result;
  };

  // BEGIN ibex_expression_cases (written by ibex_forward_cases.py: do not edit)
  static const ibex_expression_case ibex_expression_cases[] = {
    { "IBEX TestArith::tan02", "tan((-pi/4)|(pi/4))", { -0x1.0000000000001p+0, 0x1.0000000000001p+0 } },
    { "IBEX TestArith::tan05", "tan(pi/2)", { -ibex_oo, ibex_oo } },
    { "IBEX TestArith::tan07", "tan(3*pi/4|5*pi/4)", { -0x1.0000000000001p+0, 0x1.0000000000007p+0 } },
  };
  // END ibex_expression_cases

  void ibex_expressions()
  {
    typedef interval (*Expression)();
    static const std::map<std::string, Expression> expressions = {
      { "tan((-pi/4)|(pi/4))", [] { return tan((-interval::pi()/4.0) | (interval::pi()/4.0)); } },
      { "tan(pi/2)", [] { return tan(interval::pi()/2.0); } },
      { "tan(3*pi/4|5*pi/4)", [] { return tan(3.0*interval::pi()/4.0 | 5.0*interval::pi()/4.0); } },
    };
    for (const ibex_expression_case& c : ibex_expression_cases) {
      const auto describe = [&] { return std::string(c.expression); };
      const interval r = evaluate(c.name, [&] { return expressions.at(c.expression)(); }, describe);
      expect(c.name, r, c.result, describe, limit);
    }
  }

  //---------------------------------------------------------------------------
  // TestArith: root01 to root04, root(x, n) of IBEX is nth_root(x, n): the root
  // of order n, 1/x^(1/|n|) for a negative n, the real root of a negative x for
  // an odd n
  //---------------------------------------------------------------------------

  struct ibex_root_case
  {
    const char *name;
    Ref x;
    int n;
    Ref result;
  };

  // BEGIN ibex_root_cases (written by ibex_forward_cases.py: do not edit)
  static const ibex_root_case ibex_root_cases[] = {
    { "IBEX TestArith::root01", { 0x0p+0, 0x1p+0 }, -1, { 0x1p+0, ibex_oo } },
    { "IBEX TestArith::root02", { -0x1.bp+4, -0x1p+3 }, 3, { -0x1.8p+1, -0x1p+1 } },
    { "IBEX TestArith::root03", { -0x1p+2, 0x1p+0 }, 2, { 0x0p+0, 0x1p+0 } },
    { "IBEX TestArith::root04", { -0x1p+3, 0x1p+0 }, 3, { -0x1p+1, 0x1p+0 } },
  };
  // END ibex_root_cases

  void ibex_roots()
  {
    for (const ibex_root_case& c : ibex_root_cases) {
      const interval X = ibex_interval(c.x);
      const auto describe = [&] { return "nth_root(" + show(X) + ", " + std::to_string(c.n) + ")"; };
      expect(c.name, evaluate(c.name, [&] { return nth_root(X, c.n); }, describe), c.result, describe, limit);
    }
  }

  //---------------------------------------------------------------------------
  // TestArith: pow01 to pow13, check_pow(): x^p and (-x)^p, p an int
  //---------------------------------------------------------------------------

  struct ibex_pow_case
  {
    const char *name;
    Ref x;
    int p;
    Ref result, result_of_opposite;
  };

  // BEGIN ibex_pow_cases (written by ibex_forward_cases.py: do not edit)
  static const ibex_pow_case ibex_pow_cases[] = {
    { "IBEX TestArith::pow01", { -ibex_oo, ibex_oo }, 4, { 0x0p+0, ibex_oo }, { 0x0p+0, ibex_oo } },
    { "IBEX TestArith::pow02", ibex_empty, 4, ibex_empty, ibex_empty },
    { "IBEX TestArith::pow03", { 0x1p+1, 0x1.8p+1 }, 4, { 0x1p+4, 0x1.44p+6 }, { 0x1p+4, 0x1.44p+6 } },
    { "IBEX TestArith::pow04", { -0x1p+1, 0x1.8p+1 }, 4, { 0x0p+0, 0x1.44p+6 }, { 0x0p+0, 0x1.44p+6 } },
    { "IBEX TestArith::pow05", { -0x1.8p+1, 0x1p+1 }, 4, { 0x0p+0, 0x1.44p+6 }, { 0x0p+0, 0x1.44p+6 } },
    { "IBEX TestArith::pow06", { 0x1p+1, ibex_oo }, 4, { 0x1p+4, ibex_oo }, { 0x1p+4, ibex_oo } },
    { "IBEX TestArith::pow07", { -ibex_oo, ibex_oo }, 3, { -ibex_oo, ibex_oo }, { -ibex_oo, ibex_oo } },
    { "IBEX TestArith::pow08", ibex_empty, 3, ibex_empty, ibex_empty },
    { "IBEX TestArith::pow09", { 0x1p+1, 0x1.8p+1 }, 3, { 0x1p+3, 0x1.bp+4 }, { -0x1.bp+4, -0x1p+3 } },
    { "IBEX TestArith::pow10", { -0x1p+1, 0x1.8p+1 }, 3, { -0x1p+3, 0x1.bp+4 }, { -0x1.bp+4, 0x1p+3 } },
    { "IBEX TestArith::pow11", { -0x1.8p+1, 0x1p+1 }, 3, { -0x1.bp+4, 0x1p+3 }, { -0x1p+3, 0x1.bp+4 } },
    { "IBEX TestArith::pow12", { 0x1p+1, ibex_oo }, 3, { 0x1p+3, ibex_oo }, { -ibex_oo, -0x1p+3 } },
    { "IBEX TestArith::pow13", { -0x1.4p+3, 0x1.4p+3 }, -2, { 0x1.47ae147ae147ap-7, ibex_oo },
      { 0x1.47ae147ae147ap-7, ibex_oo } },
  };
  // END ibex_pow_cases

  void ibex_powers()
  {
    for (const ibex_pow_case& c : ibex_pow_cases) {
      const interval X = ibex_interval(c.x);
      const auto describe = [&] { return "pow(" + show(X) + ", " + std::to_string(c.p) + ")"; };
      expect(ibex_label(c.name, "pow(x,p)"), pow(X, c.p), c.result, describe, limit);
      expect(ibex_label(c.name, "pow(-x,p)"), pow(-X, c.p), c.result_of_opposite, describe, limit);
    }
  }

  //---------------------------------------------------------------------------
  // TestArith: atan2_01 to atan2_15, atan2(y, x): the angles of the points of
  // the box y x x, in [-pi, pi] (the half-line y = 0, x < 0 has the angle pi)
  //---------------------------------------------------------------------------

  struct ibex_atan2_case
  {
    const char *name;
    Ref y, x, result;
  };

  // BEGIN ibex_atan2_cases (written by ibex_forward_cases.py: do not edit)
  static const ibex_atan2_case ibex_atan2_cases[] = {
    { "IBEX TestArith::atan2_01", { -ibex_oo, ibex_oo }, ibex_empty, ibex_empty },
    { "IBEX TestArith::atan2_02", { 0x1p+0, 0x1p+0 }, { 0x1p+0, 0x1p+0 },
      { 0x1.921fb54442d18p-1, 0x1.921fb54442d19p-1 } },
    { "IBEX TestArith::atan2_03", { -0x1p+0, -0x1p+0 }, { -0x1p+0, -0x1p+0 },
      { -0x1.2d97c7f3321d3p+1, -0x1.2d97c7f3321d2p+1 } },
    { "IBEX TestArith::atan2_04", { -0x1p+0, -0x1p+0 }, { 0x1p+0, 0x1p+0 },
      { -0x1.921fb54442d19p-1, -0x1.921fb54442d18p-1 } },
    { "IBEX TestArith::atan2_05", { 0x1p+0, 0x1p+0 }, { -0x1p+0, -0x1p+0 },
      { 0x1.2d97c7f3321d2p+1, 0x1.2d97c7f3321d3p+1 } },
    { "IBEX TestArith::atan2_06", { 0x0p+0, ibex_oo }, { 0x0p+0, ibex_oo }, { 0x0p+0, 0x1.921fb54442d19p+0 } },
    { "IBEX TestArith::atan2_07", { -ibex_oo, -0x0.0000000000001p-1022 }, { -ibex_oo, 0x0p+0 },
      { -0x1.921fb54442d19p+1, -0x1.921fb54442d18p+0 } },
    { "IBEX TestArith::atan2_08", { -ibex_oo, 0x0p+0 }, { 0x0p+0, ibex_oo }, { -0x1.921fb54442d19p+0, 0x0p+0 } },
    { "IBEX TestArith::atan2_09", { 0x0p+0, ibex_oo }, { -ibex_oo, 0x0p+0 },
      { 0x1.921fb54442d18p+0, 0x1.921fb54442d19p+1 } },
    { "IBEX TestArith::atan2_10", { 0x1p+0, ibex_oo }, { -0x1p+0, 0x1p+0 },
      { 0x1.921fb54442d18p-1, 0x1.2d97c7f3321d3p+1 } },
    { "IBEX TestArith::atan2_11", { -ibex_oo, -0x1p+0 }, { -0x1p+0, 0x1p+0 },
      { -0x1.2d97c7f3321d3p+1, -0x1.921fb54442d18p-1 } },
    { "IBEX TestArith::atan2_12", { -0x1p+0, 0x1p+0 }, { 0x1p+0, ibex_oo },
      { -0x1.921fb54442d19p-1, 0x1.921fb54442d19p-1 } },
    { "IBEX TestArith::atan2_13", { -0x1p+0, 0x1p+0 }, { -ibex_oo, -0x1p+0 },
      { -0x1.921fb54442d19p+1, 0x1.921fb54442d19p+1 } },
    { "IBEX TestArith::atan2_14", { -ibex_oo, ibex_oo }, { -ibex_oo, ibex_oo },
      { -0x1.921fb54442d19p+1, 0x1.921fb54442d19p+1 } },
    { "IBEX TestArith::atan2_15", { 0x0p+0, 0x0p+0 }, { 0x0p+0, 0x0p+0 }, ibex_empty },
  };
  // END ibex_atan2_cases

  void ibex_atan2s()
  {
    for (const ibex_atan2_case& c : ibex_atan2_cases) {
      const interval Y = ibex_interval(c.y), X = ibex_interval(c.x);
      const auto describe = [&] { return "atan2(" + show(Y) + ", " + show(X) + ")"; };
      expect(c.name, evaluate(c.name, [&] { return atan2(Y, X); }, describe), c.result, describe, limit);
    }
  }

  //---------------------------------------------------------------------------
  // TestArith: sin01 to sin13, check_trigo(): the eight forms of the sine of x
  // that IBEX checks, the cosines being those of x shifted by a half pi. The
  // reference of each is the exact image of the argument GAOL computes (the
  // sums and differences with pi, 2 pi and half pi are exact in GAOL), and the
  // sines and cosines stay within [-1, 1]
  //---------------------------------------------------------------------------

  struct ibex_trig_case
  {
    const char *name;
    Ref x;
    Ref result[8];
  };

  // BEGIN ibex_trig_cases (written by ibex_forward_cases.py: do not edit)
  static const ibex_trig_case ibex_trig_cases[] = {
    { "IBEX TestArith::sin01", { -ibex_oo, ibex_oo },
      { { -0x1p+0, 0x1p+0 }, { -0x1p+0, 0x1p+0 }, { -0x1p+0, 0x1p+0 }, { -0x1p+0, 0x1p+0 }, { -0x1p+0, 0x1p+0 },
        { -0x1p+0, 0x1p+0 }, { -0x1p+0, 0x1p+0 }, { -0x1p+0, 0x1p+0 } } },
    { "IBEX TestArith::sin02", ibex_empty,
      { ibex_empty, ibex_empty, ibex_empty, ibex_empty, ibex_empty, ibex_empty, ibex_empty, ibex_empty } },
    { "IBEX TestArith::sin03", { 0x0p+0, 0x1.921fb54442d19p+0 },
      { { 0x0p+0, 0x1p+0 }, { -0x1.72cece675d1fdp-52, 0x1p+0 }, { -0x1.1a62633145c07p-52, 0x1p+0 }, { -0x1p+0, 0x0p+0 },
        { -0x1.72cece675d1fdp-53, 0x1p+0 }, { -0x1.72cece675d1fdp-53, 0x1p+0 }, { -0x1p+0, 0x1.1a62633145c07p-54 },
        { -0x1.34f272993d142p-50, 0x1p+0 } } },
    { "IBEX TestArith::sin04", { 0x0p+0, 0x1.921fb54442d19p+1 },
      { { -0x1.72cece675d1fdp-52, 0x1p+0 }, { -0x1p-51, 0x1p+0 }, { -0x1.961b1acd85d7ep-50, 0x1p+0 },
        { -0x1p+0, 0x1.72cece675d1fdp-52 }, { -0x1.b9676733ae8ffp-52, 0x1p+0 }, { -0x1.b9676733ae8ffp-52, 0x1p+0 },
        { -0x1p+0, 0x1.961b1acd85d7ep-51 }, { -0x1.a7c141009a33ep-50, 0x1p+0 } } },
    { "IBEX TestArith::sin05", { 0x0p+0, 0x1.2d97c7f3321d3p+2 },
      { { -0x1p+0, 0x1p+0 }, { -0x1p+0, 0x1p+0 }, { -0x1p+0, 0x1p+0 }, { -0x1p+0, 0x1p+0 }, { -0x1p+0, 0x1p+0 },
        { -0x1p+0, 0x1p+0 }, { -0x1p+0, 0x1p+0 }, { -0x1p+0, 0x1p+0 } } },
    { "IBEX TestArith::sin06", { 0x1.921fb54442d18p+1, 0x1.2d97c7f3321d3p+2 },
      { { -0x1p+0, 0x1.1a62633145c07p-53 }, { -0x1p+0, 0x1p-51 }, { -0x1p+0, 0x1.a79394c9e8a0bp-52 },
        { -0x1.1a62633145c07p-53, 0x1p+0 }, { -0x1p+0, 0x1.469898cc51702p-52 }, { -0x1p+0, 0x1.469898cc51702p-52 },
        { -0x1.a79394c9e8a0bp-53, 0x1p+0 }, { -0x1p+0, 0x1.583ebeff65cc3p-50 } } },
    { "IBEX TestArith::sin07", { 0x1p-1, 0x1.8p+0 },
      { { 0x1.eaee8744b05efp-2, 0x1.feb7a9b2c6d8bp-1 }, { 0x1.eaee8744b05eap-2, 0x1.feb7a9b2c6d8bp-1 },
        { 0x1.eaee8744b05ecp-2, 0x1.feb7a9b2c6d8cp-1 }, { -0x1.feb7a9b2c6d8bp-1, -0x1.eaee8744b05efp-2 },
        { 0x1.eaee8744b05edp-2, 0x1.feb7a9b2c6d8bp-1 }, { 0x1.eaee8744b05edp-2, 0x1.feb7a9b2c6d8bp-1 },
        { -0x1.feb7a9b2c6d8cp-1, -0x1.eaee8744b05eep-2 }, { 0x1.eaee8744b05dep-2, 0x1.feb7a9b2c6d8cp-1 } } },
    { "IBEX TestArith::sin08", { 0x1.8p+0, 0x1.8p+1 },
      { { 0x1.210386db6d55bp-3, 0x1p+0 }, { 0x1.210386db6d556p-3, 0x1p+0 }, { 0x1.210386db6d524p-3, 0x1p+0 },
        { -0x1p+0, -0x1.210386db6d55bp-3 }, { 0x1.210386db6d559p-3, 0x1p+0 }, { 0x1.210386db6d559p-3, 0x1p+0 },
        { -0x1p+0, -0x1.210386db6d53dp-3 }, { 0x1.210386db6d522p-3, 0x1p+0 } } },
    { "IBEX TestArith::sin09", { 0x1.8p+1, 0x1p+2 },
      { { -0x1.837b9dddc1eafp-1, 0x1.210386db6d55cp-3 }, { -0x1.837b9dddc1ebp-1, 0x1.210386db6d567p-3 },
        { -0x1.837b9dddc1eb8p-1, 0x1.210386db6d565p-3 }, { -0x1.210386db6d55cp-3, 0x1.837b9dddc1eafp-1 },
        { -0x1.837b9dddc1eafp-1, 0x1.210386db6d562p-3 }, { -0x1.837b9dddc1eafp-1, 0x1.210386db6d562p-3 },
        { -0x1.210386db6d55ep-3, 0x1.837b9dddc1eb4p-1 }, { -0x1.837b9dddc1eb8p-1, 0x1.210386db6d582p-3 } } },
    { "IBEX TestArith::sin10", { 0x1.8p+1, 0x1.4p+2 },
      { { -0x1p+0, 0x1.210386db6d55cp-3 }, { -0x1p+0, 0x1.210386db6d567p-3 }, { -0x1p+0, 0x1.210386db6d565p-3 },
        { -0x1.210386db6d55cp-3, 0x1p+0 }, { -0x1p+0, 0x1.210386db6d562p-3 }, { -0x1p+0, 0x1.210386db6d562p-3 },
        { -0x1.210386db6d55ep-3, 0x1p+0 }, { -0x1p+0, 0x1.210386db6d582p-3 } } },
    { "IBEX TestArith::sin11", { 0x1.8p+1, 0x1.f21fb54442d19p+2 },
      { { -0x1p+0, 0x1.feb7a9b2c6d8cp-1 }, { -0x1p+0, 0x1.feb7a9b2c6d8cp-1 }, { -0x1p+0, 0x1.feb7a9b2c6d8cp-1 },
        { -0x1.feb7a9b2c6d8cp-1, 0x1p+0 }, { -0x1p+0, 0x1.feb7a9b2c6d8cp-1 }, { -0x1p+0, 0x1.feb7a9b2c6d8cp-1 },
        { -0x1.feb7a9b2c6d8cp-1, 0x1p+0 }, { -0x1p+0, 0x1.feb7a9b2c6d8cp-1 } } },
    { "IBEX TestArith::sin12", { 0x1.4p+2, 0x1.f21fb54442d19p+2 },
      { { -0x1.eaf81f5e09934p-1, 0x1.feb7a9b2c6d8cp-1 }, { -0x1.eaf81f5e09934p-1, 0x1.feb7a9b2c6d8cp-1 },
        { -0x1.eaf81f5e09934p-1, 0x1.feb7a9b2c6d8cp-1 }, { -0x1.feb7a9b2c6d8cp-1, 0x1.eaf81f5e09934p-1 },
        { -0x1.eaf81f5e09935p-1, 0x1.feb7a9b2c6d8cp-1 }, { -0x1.eaf81f5e09935p-1, 0x1.feb7a9b2c6d8cp-1 },
        { -0x1.feb7a9b2c6d8cp-1, 0x1.eaf81f5e09934p-1 }, { -0x1.eaf81f5e09939p-1, 0x1.feb7a9b2c6d8cp-1 } } },
    { "IBEX TestArith::sin13", { 0x1.4p+2, 0x1.290fdaa22168dp+3 },
      { { -0x1.eaf81f5e09934p-1, 0x1p+0 }, { -0x1.eaf81f5e09934p-1, 0x1p+0 }, { -0x1.eaf81f5e09934p-1, 0x1p+0 },
        { -0x1p+0, 0x1.eaf81f5e09934p-1 }, { -0x1.eaf81f5e09935p-1, 0x1p+0 }, { -0x1.eaf81f5e09935p-1, 0x1p+0 },
        { -0x1p+0, 0x1.eaf81f5e09934p-1 }, { -0x1.eaf81f5e09939p-1, 0x1p+0 } } },
  };
  // END ibex_trig_cases

  void ibex_trigs()
  {
    static const char *const variants[8] = { "sin(x)", "sin(pi-x)", "sin(x+2pi)", "sin(-x)",
                                             "cos(x-pi/2)", "cos(pi/2-x)", "cos(x+pi/2)", "cos(x+2pi-pi/2)" };
    const interval pi = interval::pi(), two_pi = interval::two_pi(), half_pi = interval::half_pi();
    for (const ibex_trig_case& c : ibex_trig_cases) {
      const interval X = ibex_interval(c.x);
      const interval r[8] = { sin(X), sin(pi - X), sin(X + two_pi), sin(-X), cos(X - half_pi), cos(half_pi - X),
                              cos(X + half_pi), cos(X + two_pi - half_pi) };
      for (std::size_t i = 0; i < 8; ++i) {
        const std::string name = ibex_label(c.name, variants[i]);
        const auto describe = [&] { return "x = " + show(X) + ", " + variants[i] + " = " + show(r[i]); };
        expect(name, r[i], c.result[i], describe, limit);
        check(name + ": within [-1, 1]", r[i].is_empty() || (r[i].left() >= -1.0 && r[i].right() <= 1.0), describe);
      }
    }
  }

  //---------------------------------------------------------------------------
  // TestArith: sinh01 to sinh07 (check_sinh), cosh01 to cosh07 (check_cosh).
  // IBEX also checks asinh(sinh(x)) = x
  //---------------------------------------------------------------------------

  struct ibex_hyperbolic_case
  {
    const char *name;
    Ref x, result, result_of_opposite;
  };

  // BEGIN ibex_sinh_cases (written by ibex_forward_cases.py: do not edit)
  static const ibex_hyperbolic_case ibex_sinh_cases[] = {
    { "IBEX TestArith::sinh01", { -ibex_oo, ibex_oo }, { -ibex_oo, ibex_oo }, { -ibex_oo, ibex_oo } },
    { "IBEX TestArith::sinh02", { 0x0p+0, ibex_oo }, { 0x0p+0, ibex_oo }, { -ibex_oo, 0x0p+0 } },
    { "IBEX TestArith::sinh03", { 0x0p+0, 0x1p+0 }, { 0x0p+0, 0x1.2cd9fc44eb983p+0 },
      { -0x1.2cd9fc44eb983p+0, 0x0p+0 } },
    { "IBEX TestArith::sinh04", { 0x1p+0, ibex_oo }, { 0x1.2cd9fc44eb982p+0, ibex_oo },
      { -ibex_oo, -0x1.2cd9fc44eb982p+0 } },
    { "IBEX TestArith::sinh05", { 0x1p+0, 0x1p+0 }, { 0x1.2cd9fc44eb982p+0, 0x1.2cd9fc44eb983p+0 },
      { -0x1.2cd9fc44eb983p+0, -0x1.2cd9fc44eb982p+0 } },
    { "IBEX TestArith::sinh06", { 0x1p+1, 0x1.8p+1 }, { 0x1.d03cf63b6e19fp+1, 0x1.40926e70949aep+3 },
      { -0x1.40926e70949aep+3, -0x1.d03cf63b6e19fp+1 } },
    { "IBEX TestArith::sinh07", { 0x1p+2, 0x1.4p+2 }, { 0x1.b4a380370363p+4, 0x1.28d0166f07375p+6 },
      { -0x1.28d0166f07375p+6, -0x1.b4a380370363p+4 } },
  };
  // END ibex_sinh_cases

  // BEGIN ibex_cosh_cases (written by ibex_forward_cases.py: do not edit)
  static const ibex_hyperbolic_case ibex_cosh_cases[] = {
    { "IBEX TestArith::cosh01", { -ibex_oo, ibex_oo }, { 0x1p+0, ibex_oo }, { 0x1p+0, ibex_oo } },
    { "IBEX TestArith::cosh02", { 0x0p+0, ibex_oo }, { 0x1p+0, ibex_oo }, { 0x1p+0, ibex_oo } },
    { "IBEX TestArith::cosh03", { 0x0p+0, 0x1p+0 }, { 0x1p+0, 0x1.8b07551d9f551p+0 },
      { 0x1p+0, 0x1.8b07551d9f551p+0 } },
    { "IBEX TestArith::cosh04", { 0x1p+0, ibex_oo }, { 0x1.8b07551d9f55p+0, ibex_oo },
      { 0x1.8b07551d9f55p+0, ibex_oo } },
    { "IBEX TestArith::cosh05", { 0x1p+0, 0x1p+0 }, { 0x1.8b07551d9f55p+0, 0x1.8b07551d9f551p+0 },
      { 0x1.8b07551d9f55p+0, 0x1.8b07551d9f551p+0 } },
    { "IBEX TestArith::cosh06", { 0x1p+1, 0x1.8p+1 }, { 0x1.e18fa0df2d9bcp+1, 0x1.422a497d6185fp+3 },
      { 0x1.e18fa0df2d9bcp+1, 0x1.422a497d6185fp+3 } },
    { "IBEX TestArith::cosh07", { 0x1p+2, 0x1.4p+2 }, { 0x1.b4ee858de3e8p+4, 0x1.28d6fcbeff3aap+6 },
      { 0x1.b4ee858de3e8p+4, 0x1.28d6fcbeff3aap+6 } },
  };
  // END ibex_cosh_cases

  void ibex_hyperbolics()
  {
    for (const ibex_hyperbolic_case& c : ibex_sinh_cases) {
      const interval X = ibex_interval(c.x), minus_X = -X;
      const auto describe = [&] { return "x = " + show(X); };
      expect(ibex_label(c.name, "sinh(x)"), sinh(X), c.result, describe, limit);
      expect(ibex_label(c.name, "sinh(-x)"), sinh(minus_X), c.result_of_opposite, describe, limit);
      expect(ibex_label(c.name, "asinh(sinh(x))"), asinh(sinh(X)), c.x, describe, ibex_round_trip);
      expect(ibex_label(c.name, "asinh(sinh(-x))"), asinh(sinh(minus_X)), ibex_opposite(c.x), describe,
             ibex_round_trip);
    }
    for (const ibex_hyperbolic_case& c : ibex_cosh_cases) {
      const interval X = ibex_interval(c.x);
      const auto describe = [&] { return "x = " + show(X); };
      expect(ibex_label(c.name, "cosh(x)"), cosh(X), c.result, describe, limit);
      expect(ibex_label(c.name, "cosh(-x)"), cosh(-X), c.result_of_opposite, describe, limit);
    }
  }

  //---------------------------------------------------------------------------
  // TestInterval: issue_463, [3, 8] + ([1, 3] & [6, 7]): the intersection is
  // empty, and so is the sum
  //---------------------------------------------------------------------------

  void ibex_issue_463()
  {
    const interval a(3.0, 8.0);
    const interval b = interval(1.0, 3.0) & interval(6.0, 7.0);
    const interval c = a + b;
    const auto describe = [&] { return "b = " + show(b) + ", c = " + show(c); };
    expect("IBEX TestInterval::issue_463 b", b, ibex_empty, describe);
    expect("IBEX TestInterval::issue_463 c", c, ibex_empty, describe);
  }

  void ibex_run()
  {
    ibex_constants();
    ibex_constructions();
    ibex_equalities();
    ibex_set_operations();
    ibex_getters();
    ibex_midpoints();
    ibex_midpoint_roundtrips();
    ibex_relations();
    ibex_issue_463();
    ibex_floats();
    for (const ibex_binary_case& c : ibex_binary_cases) {
      ibex_binary(c);
    }
    for (const ibex_unary_case& c : ibex_unary_cases) {
      ibex_unary(c);
    }
    ibex_expressions();
    ibex_roots();
    ibex_powers();
    ibex_atan2s();
    ibex_trigs();
    ibex_hyperbolics();
  }
}

// The backward tests of IBEX (ibex-lib commit 65ed587, tests/TestArith.cpp: the
// bwd_* contractors and div2; tests/TestInterval.cpp: distance and
// rel_distance), as tests of GAOL: the intervals of each case, and the result to
// find, written directly with gaol::interval. The result is the exact one,
// computed apart from GAOL and from IBEX by tests/tools/ibex_codac/ibexb_cases.py,
// which writes the tables of this section (exact rationals, and mpmath with 2000
// bits for the roots, the exponential, the sine, the cosine, the tangent and the
// angles): Ref{lo, hi} are the doubles around the exact bounds, and the result
// of GAOL has to enclose them, within no double of them where the bounds are
// doubles (the sums, the differences, the products, the squares, the sets, the
// floor, the integers, the distances) and within `limit` doubles where one is
// not (a quotient, an exponential, an angle, a tangent); the contractions of the
// angles atan2_rel_y and atan2_rel_x are allowed two, as doc/accuracy.md says.
// What IBEX expects, with its tolerance of 1e-10, is only compared with the
// exact result, by the script (--originals).
//
// Each contractor of IBEX is written as the computation of GAOL it stands for,
// the contractions in the order of IBEX (the second one runs on the result of the
// first): bwd_add(y, x1, x2) is x1 & (y - x2) then x2 & (y - x1), bwd_sub(y, x1,
// x2) is x1 & (y + x2) then x2 & (x1 - y), bwd_mul(y, x1, x2) is div_rel(y, x2,
// x1) then div_rel(y, x1, x2), bwd_div(y, x1, x2) is x1 & (y * x2), t = div_rel(x1,
// x2, y), then div_rel(x1, t, x2), bwd_max(z, x, y) and bwd_min(z, x, y) are
// max_rel(z, y, x) then max_rel(z, x, y) (min_rel), bwd_sqr(y, x) is sqrt_rel(y,
// x), bwd_sqrt(y, x) is x & sqr(y & [0, +oo]), bwd_log(y, x) is x & exp(y),
// bwd_sin(y, x) and bwd_cos(y, x) are asin_rel(y, x) and acos_rel(y, x),
// bwd_pow(y, p, x) is nth_root_rel(y, p, x) for p > 0 (for p < 0 the roots
// nth_root(y, p) of y, and their opposites for an even p, meet x), bwd_atan2(c,
// y, x) is atan2_rel_y(c, x, y) then atan2_rel_x(c, y, x), bwd_floor(y, x) is
// floor_rel(y, x), bwd_ceil(y, x) is -floor_rel(-y, -x); bwd_atan, bwd_imod and
// div2_inter are written below, over the operations of GAOL. div2(num, div) is
// gaol_ieee1788::mulRevToPair(div, num), distance(x, y) is hausdorff(x, y) and
// rel_distance(x, y) is hausdorff(x, y) / width(x). The header of ibexb_cases.py says,
// for each, whether IBEX computes the same.
//
// Each test of IBEX is one case, one row of a table, named as in IBEX. The
// variants a test performs are run for the case, each under its name: the
// contractions in either order and on the opposites for bwd_add, bwd_sub,
// bwd_mul, bwd_div, bwd_max and bwd_min, the sine of x and of -x and the cosines
// of x -/+ pi/2 for bwd_sin, x and -x for bwd_pow and bwd_atan, the variants of
// div2_inter for div2. The argument y of the sine tests, sin(Interval(0.5,
// 1.5)) in IBEX, is the tightest enclosure of it, in the table, so that the
// result does not depend on the sine of GAOL (a bound of x near pi/2 moves by
// several doubles for one double of y). The doubles that IBEX computes before it
// contracts (5*piU/2.0, M_PI/6., -sqrt(3.)/2...) are in the tables, rounded upward
// as IBEX computes them. Not in this section: the vectors and matrices
// (bwd_mulVV01, bwd_mulMV01, mulMM01), the empty bodies (bwd_floor_12-13,
// bwd_ceil_12-13), and the distances to the empty set (distance20-21: the radius
// of the other interval in IBEX, a NaN in GAOL).
namespace
{
  constexpr double ibexb_oo = std::numeric_limits<double>::infinity();
  constexpr Ref ibexb_empty = { std::numeric_limits<double>::quiet_NaN(), std::numeric_limits<double>::quiet_NaN() };

  // The largest distance from the tightest bounds allowed to atan2_rel_y and atan2_rel_x: two doubles
  // (doc/accuracy.md: the slopes tan(c) of the bounds of c are rounded once, and their products and
  // quotients with the bounds of the box once more)
  const int ibexb_atan2_limit = 2;

  // [r.lo, r.hi], or the empty set for ibexb_empty
  inline interval ibexb_interval(const Ref& r)
  {
    return is_none(r) ? interval::emptyset() : interval(r.lo, r.hi);
  }

  // The opposite of a reference
  inline Ref ibexb_opposite(const Ref& r)
  {
    return is_none(r) ? r : Ref{ -r.hi, -r.lo };
  }

  // The hull of two references
  inline Ref ibexb_hull(const Ref& a, const Ref& b)
  {
    if (is_none(a)) {
      return b;
    }
    return is_none(b) ? a : Ref{ a.lo < b.lo ? a.lo : b.lo, a.hi > b.hi ? a.hi : b.hi };
  }

  inline std::string ibexb_label(const char *name, const std::string& variant)
  {
    return std::string(name) + " " + variant;
  }

  //---------------------------------------------------------------------------
  // TestInterval: distance01 to distance23 (without distance20 and distance21),
  // the Hausdorff distance max(|a - c|, |b - d|) of [a, b] and [c, d], the
  // distance of equal infinite bounds being 0: hausdorff(x, y), rounded upward
  //---------------------------------------------------------------------------

  struct ibexb_distance_case
  {
    const char *name;
    Ref x, y, result;
  };

  // BEGIN ibexb_distance_cases (written by ibexb_cases.py: do not edit)
  static const ibexb_distance_case ibexb_distance_cases[] = {
    { "IBEX TestInterval::distance01", { 0x0p+0, 0x1.4p+3 }, { -0x1.4p+2, 0x1.4p+2 }, { 0x1.4p+2, 0x1.4p+2 } },
    { "IBEX TestInterval::distance02", { 0x0p+0, 0x1.4p+3 }, { 0x1.4p+2, 0x1.4p+3 }, { 0x1.4p+2, 0x1.4p+2 } },
    { "IBEX TestInterval::distance03", { 0x0p+0, 0x1.4p+3 }, { 0x1.4p+2, 0x1.ep+3 }, { 0x1.4p+2, 0x1.4p+2 } },
    { "IBEX TestInterval::distance04", { 0x0p+0, 0x1.4p+3 }, { 0x1p+0, 0x1p+1 }, { 0x1p+3, 0x1p+3 } },
    { "IBEX TestInterval::distance05", { 0x0p+0, 0x1.4p+3 }, { 0x0p+0, 0x1.4p+3 }, { 0x0p+0, 0x0p+0 } },
    { "IBEX TestInterval::distance06", { 0x0p+0, 0x1.4p+3 }, { -0x1.4p+3, 0x1.4p+4 }, { 0x1.4p+3, 0x1.4p+3 } },
    { "IBEX TestInterval::distance07", { 0x0p+0, ibexb_oo }, { 0x0p+0, 0x1.4p+3 }, { ibexb_oo, ibexb_oo } },
    { "IBEX TestInterval::distance08", { -ibexb_oo, ibexb_oo }, { 0x0p+0, 0x1.4p+3 }, { ibexb_oo, ibexb_oo } },
    { "IBEX TestInterval::distance09", { -ibexb_oo, ibexb_oo }, { -ibexb_oo, ibexb_oo }, { 0x0p+0, 0x0p+0 } },
    { "IBEX TestInterval::distance10", { -ibexb_oo, ibexb_oo }, { -ibexb_oo, 0x0p+0 }, { ibexb_oo, ibexb_oo } },
    { "IBEX TestInterval::distance11", { -ibexb_oo, ibexb_oo }, { 0x0p+0, ibexb_oo }, { ibexb_oo, ibexb_oo } },
    { "IBEX TestInterval::distance12", { 0x0p+0, ibexb_oo }, { -ibexb_oo, 0x1p+0 }, { ibexb_oo, ibexb_oo } },
    { "IBEX TestInterval::distance13", { 0x0p+0, ibexb_oo }, { 0x0p+0, 0x1p+0 }, { ibexb_oo, ibexb_oo } },
    { "IBEX TestInterval::distance14", { -ibexb_oo, 0x0p+0 }, { -0x1p+0, ibexb_oo }, { ibexb_oo, ibexb_oo } },
    { "IBEX TestInterval::distance15", { -ibexb_oo, 0x0p+0 }, { -ibexb_oo, -0x1p+0 }, { 0x1p+0, 0x1p+0 } },
    { "IBEX TestInterval::distance15b", { 0x0p+0, ibexb_oo }, { 0x1p+0, ibexb_oo }, { 0x1p+0, 0x1p+0 } },
    { "IBEX TestInterval::distance16", { 0x0p+0, 0x1.4p+3 }, { 0x1.4p+2, 0x1.4p+2 }, { 0x1.4p+2, 0x1.4p+2 } },
    { "IBEX TestInterval::distance17", { 0x0p+0, 0x1.4p+3 }, { 0x0p+0, 0x0p+0 }, { 0x1.4p+3, 0x1.4p+3 } },
    { "IBEX TestInterval::distance18", { 0x0p+0, 0x1.4p+3 }, { 0x1.4p+3, 0x1.4p+3 }, { 0x1.4p+3, 0x1.4p+3 } },
    { "IBEX TestInterval::distance19", { 0x0p+0, 0x0p+0 }, { 0x0p+0, 0x0p+0 }, { 0x0p+0, 0x0p+0 } },
    { "IBEX TestInterval::distance22", { 0x0p+0, 0x1.4p+3 }, { 0x1.6p+3, 0x1.4p+4 }, { 0x1.6p+3, 0x1.6p+3 } },
    { "IBEX TestInterval::distance23", { 0x0p+0, 0x1.4p+3 }, { 0x0p+0, 0x1.4p+2 }, { 0x1.4p+2, 0x1.4p+2 } },
  };
  // END ibexb_distance_cases

  void ibexb_distances()
  {
    for (const ibexb_distance_case& c : ibexb_distance_cases) {
      const interval X = ibexb_interval(c.x), Y = ibexb_interval(c.y);
      expect(c.name, hausdorff(X, Y), c.result, [&] { return "hausdorff(" + show(X) + ", " + show(Y) + ")"; });
    }
  }

  //---------------------------------------------------------------------------
  // TestInterval: rel_distance01 to rel_distance21, the distance over the width of
  // x. The distance and the width are checked in every case, the quotient where it
  // is a quotient of reals (a finite distance over a finite width other than 0) or
  // a finite distance over an infinite width (0): IBEX settles the other cases
  // itself, by convention (1 for an infinite distance, 0 for a null width)
  //---------------------------------------------------------------------------

  struct ibexb_relative_distance_case
  {
    const char *name;
    Ref x, y, distance, width;
    bool quotient;
    Ref relative;
  };

  // BEGIN ibexb_relative_distance_cases (written by ibexb_cases.py: do not edit)
  static const ibexb_relative_distance_case ibexb_relative_distance_cases[] = {
    { "IBEX TestInterval::rel_distance01", { 0x0p+0, 0x1.4p+3 }, { -0x1.4p+2, 0x1.4p+2 }, { 0x1.4p+2, 0x1.4p+2 },
      { 0x1.4p+3, 0x1.4p+3 }, true, { 0x1p-1, 0x1p-1 } },
    { "IBEX TestInterval::rel_distance02", { 0x0p+0, 0x1.4p+3 }, { 0x1.4p+2, 0x1.4p+3 }, { 0x1.4p+2, 0x1.4p+2 },
      { 0x1.4p+3, 0x1.4p+3 }, true, { 0x1p-1, 0x1p-1 } },
    { "IBEX TestInterval::rel_distance03", { 0x0p+0, 0x1.4p+3 }, { 0x1.4p+2, 0x1.ep+3 }, { 0x1.4p+2, 0x1.4p+2 },
      { 0x1.4p+3, 0x1.4p+3 }, true, { 0x1p-1, 0x1p-1 } },
    { "IBEX TestInterval::rel_distance04", { 0x0p+0, 0x1.4p+3 }, { 0x1p+0, 0x1p+1 }, { 0x1p+3, 0x1p+3 },
      { 0x1.4p+3, 0x1.4p+3 }, true, { 0x1.9999999999999p-1, 0x1.999999999999ap-1 } },
    { "IBEX TestInterval::rel_distance05", { 0x0p+0, 0x1.4p+3 }, { 0x0p+0, 0x1.4p+3 }, { 0x0p+0, 0x0p+0 },
      { 0x1.4p+3, 0x1.4p+3 }, true, { 0x0p+0, 0x0p+0 } },
    { "IBEX TestInterval::rel_distance06", { 0x0p+0, 0x1.4p+3 }, { -0x1.4p+3, 0x1.4p+4 }, { 0x1.4p+3, 0x1.4p+3 },
      { 0x1.4p+3, 0x1.4p+3 }, true, { 0x1p+0, 0x1p+0 } },
    { "IBEX TestInterval::rel_distance07", { 0x0p+0, ibexb_oo }, { 0x0p+0, 0x1.4p+3 }, { ibexb_oo, ibexb_oo },
      { ibexb_oo, ibexb_oo }, false, ibexb_empty },
    { "IBEX TestInterval::rel_distance08", { -ibexb_oo, ibexb_oo }, { 0x0p+0, 0x1.4p+3 }, { ibexb_oo, ibexb_oo },
      { ibexb_oo, ibexb_oo }, false, ibexb_empty },
    { "IBEX TestInterval::rel_distance09", { -ibexb_oo, ibexb_oo }, { -ibexb_oo, ibexb_oo }, { 0x0p+0, 0x0p+0 },
      { ibexb_oo, ibexb_oo }, true, { 0x0p+0, 0x0p+0 } },
    { "IBEX TestInterval::rel_distance10", { -ibexb_oo, ibexb_oo }, { -ibexb_oo, 0x0p+0 }, { ibexb_oo, ibexb_oo },
      { ibexb_oo, ibexb_oo }, false, ibexb_empty },
    { "IBEX TestInterval::rel_distance11", { -ibexb_oo, ibexb_oo }, { 0x0p+0, ibexb_oo }, { ibexb_oo, ibexb_oo },
      { ibexb_oo, ibexb_oo }, false, ibexb_empty },
    { "IBEX TestInterval::rel_distance12", { 0x0p+0, ibexb_oo }, { -ibexb_oo, 0x1p+0 }, { ibexb_oo, ibexb_oo },
      { ibexb_oo, ibexb_oo }, false, ibexb_empty },
    { "IBEX TestInterval::rel_distance13", { 0x0p+0, ibexb_oo }, { 0x0p+0, 0x1p+0 }, { ibexb_oo, ibexb_oo },
      { ibexb_oo, ibexb_oo }, false, ibexb_empty },
    { "IBEX TestInterval::rel_distance14", { -ibexb_oo, 0x0p+0 }, { -0x1p+0, ibexb_oo }, { ibexb_oo, ibexb_oo },
      { ibexb_oo, ibexb_oo }, false, ibexb_empty },
    { "IBEX TestInterval::rel_distance15", { -ibexb_oo, 0x0p+0 }, { -ibexb_oo, -0x1p+0 }, { 0x1p+0, 0x1p+0 },
      { ibexb_oo, ibexb_oo }, true, { 0x0p+0, 0x0p+0 } },
    { "IBEX TestInterval::rel_distance16", { 0x0p+0, 0x1.4p+3 }, { 0x1.4p+2, 0x1.4p+2 }, { 0x1.4p+2, 0x1.4p+2 },
      { 0x1.4p+3, 0x1.4p+3 }, true, { 0x1p-1, 0x1p-1 } },
    { "IBEX TestInterval::rel_distance17", { 0x0p+0, 0x1.4p+3 }, { 0x0p+0, 0x0p+0 }, { 0x1.4p+3, 0x1.4p+3 },
      { 0x1.4p+3, 0x1.4p+3 }, true, { 0x1p+0, 0x1p+0 } },
    { "IBEX TestInterval::rel_distance18", { 0x0p+0, 0x1.4p+3 }, { 0x1.4p+3, 0x1.4p+3 }, { 0x1.4p+3, 0x1.4p+3 },
      { 0x1.4p+3, 0x1.4p+3 }, true, { 0x1p+0, 0x1p+0 } },
    { "IBEX TestInterval::rel_distance19", { 0x0p+0, 0x0p+0 }, { 0x0p+0, 0x0p+0 }, { 0x0p+0, 0x0p+0 },
      { 0x0p+0, 0x0p+0 }, false, ibexb_empty },
    { "IBEX TestInterval::rel_distance20", { 0x1p+0, 0x1p+2 }, { 0x1.8p+0, 0x1.8p+1 }, { 0x1p+0, 0x1p+0 },
      { 0x1.8p+1, 0x1.8p+1 }, true, { 0x1.5555555555555p-2, 0x1.5555555555556p-2 } },
    { "IBEX TestInterval::rel_distance21", { 0x0p+0, 0x1.4p+3 }, { 0x0p+0, 0x1.4p+2 }, { 0x1.4p+2, 0x1.4p+2 },
      { 0x1.4p+3, 0x1.4p+3 }, true, { 0x1p-1, 0x1p-1 } },
  };
  // END ibexb_relative_distance_cases

  void ibexb_relative_distances()
  {
    for (const ibexb_relative_distance_case& c : ibexb_relative_distance_cases) {
      const interval X = ibexb_interval(c.x), Y = ibexb_interval(c.y);
      const auto describe = [&] { return "x = " + show(X) + ", y = " + show(Y); };
      const double distance = hausdorff(X, Y), width = X.width();
      expect(ibexb_label(c.name, "hausdorff(x,y)"), distance, c.distance, describe);
      expect(ibexb_label(c.name, "width(x)"), width, c.width, describe);
      if (c.quotient) {
        expect(ibexb_label(c.name, "hausdorff(x,y)/width(x)"), distance / width, c.relative, describe);
      }
    }
  }

  //---------------------------------------------------------------------------
  // TestArith: div2_01 to div2_13. div2(num, div, out1, out2) is the pair of parts
  // mulRevToPair(div, num) of IEEE 1788-2015 (the closures of the parts of the
  // x with div * x in num, the lower part first). div2_inter(num, div, out2) of
  // IBEX is x & the first part, the second part in out2 (x & the second part
  // where x holds nothing of the first, out2 empty), and div2_inter(num, div)
  // the hull of both. out1bis = [out1.mid(), out1.ub()] and out2bis =
  // [out2.lb(), out2.mid()] are IBEX's own, where the interval is not a point
  // (the middle of an unbounded interval being the convention of IBEX and of
  // GAOL, +-MAXREAL); empty here where IBEX does not build them
  //---------------------------------------------------------------------------

  struct ibexb_div2_case
  {
    const char *name;
    Ref num, div, out1, out2, out1bis, out2bis;
  };

  // BEGIN ibexb_div2_cases (written by ibexb_cases.py: do not edit)
  static const ibexb_div2_case ibexb_div2_cases[] = {
    { "IBEX TestArith::div2_01", { 0x1.8p+2, 0x1.8p+3 }, { 0x0p+0, 0x0p+0 }, ibexb_empty, ibexb_empty, ibexb_empty,
      ibexb_empty },
    { "IBEX TestArith::div2_02", { 0x1.8p+2, 0x1.8p+3 }, { 0x1p+1, 0x1.8p+1 }, { 0x1p+1, 0x1.8p+2 }, ibexb_empty,
      { 0x1p+2, 0x1.8p+2 }, ibexb_empty },
    { "IBEX TestArith::div2_03", { -0x1p+1, 0x1.8p+1 }, { -0x1p+0, 0x1p+0 }, { -ibexb_oo, ibexb_oo }, ibexb_empty,
      { 0x0p+0, ibexb_oo }, ibexb_empty },
    { "IBEX TestArith::div2_04", { 0x1p+1, 0x1.8p+1 }, { -0x1p+0, 0x1p+0 }, { -ibexb_oo, -0x1p+1 },
      { 0x1p+1, ibexb_oo }, { -0x1.fffffffffffffp+1023, -0x1p+1 }, { 0x1p+1, 0x1.fffffffffffffp+1023 } },
    { "IBEX TestArith::div2_05", { -0x1.8p+1, -0x1p+1 }, { -0x1p+0, 0x1p+0 }, { -ibexb_oo, -0x1p+1 },
      { 0x1p+1, ibexb_oo }, { -0x1.fffffffffffffp+1023, -0x1p+1 }, { 0x1p+1, 0x1.fffffffffffffp+1023 } },
    { "IBEX TestArith::div2_06", { 0x1p+1, 0x1.8p+1 }, { -ibexb_oo, 0x1p+0 }, { -ibexb_oo, 0x0p+0 },
      { 0x1p+1, ibexb_oo }, { -0x1.fffffffffffffp+1023, 0x0p+0 }, { 0x1p+1, 0x1.fffffffffffffp+1023 } },
    { "IBEX TestArith::div2_07", { 0x1p+1, 0x1.8p+1 }, { -0x1p+0, ibexb_oo }, { -ibexb_oo, -0x1p+1 },
      { 0x0p+0, ibexb_oo }, { -0x1.fffffffffffffp+1023, -0x1p+1 }, { 0x0p+0, 0x1.fffffffffffffp+1023 } },
    { "IBEX TestArith::div2_08", { -0x1.8p+1, -0x1p+1 }, { -ibexb_oo, 0x1p+0 }, { -ibexb_oo, -0x1p+1 },
      { 0x0p+0, ibexb_oo }, { -0x1.fffffffffffffp+1023, -0x1p+1 }, { 0x0p+0, 0x1.fffffffffffffp+1023 } },
    { "IBEX TestArith::div2_09", { -0x1.8p+1, -0x1p+1 }, { -0x1p+0, ibexb_oo }, { -ibexb_oo, 0x0p+0 },
      { 0x1p+1, ibexb_oo }, { -0x1.fffffffffffffp+1023, 0x0p+0 }, { 0x1p+1, 0x1.fffffffffffffp+1023 } },
    { "IBEX TestArith::div2_10", { 0x1p+1, 0x1.8p+1 }, { -ibexb_oo, 0x0p+0 }, { -ibexb_oo, 0x0p+0 }, ibexb_empty,
      { -0x1.fffffffffffffp+1023, 0x0p+0 }, ibexb_empty },
    { "IBEX TestArith::div2_11", { 0x1p+1, 0x1.8p+1 }, { 0x0p+0, ibexb_oo }, { 0x0p+0, ibexb_oo }, ibexb_empty,
      { 0x1.fffffffffffffp+1023, ibexb_oo }, ibexb_empty },
    { "IBEX TestArith::div2_12", { -0x1.8p+1, -0x1p+1 }, { -ibexb_oo, 0x0p+0 }, { 0x0p+0, ibexb_oo }, ibexb_empty,
      { 0x1.fffffffffffffp+1023, ibexb_oo }, ibexb_empty },
    { "IBEX TestArith::div2_13", { -0x1.8p+1, -0x1p+1 }, { 0x0p+0, ibexb_oo }, { -ibexb_oo, 0x0p+0 }, ibexb_empty,
      { -0x1.fffffffffffffp+1023, 0x0p+0 }, ibexb_empty },
  };
  // END ibexb_div2_cases

  interval ibexb_div2_inter(const interval& x, const interval& num, const interval& div, interval& out2)
  {
    const std::pair<interval, interval> parts = gaol_ieee1788::mulRevToPair(div, num);
    const interval first = x & parts.first;
    if (first.is_empty()) {
      out2 = interval::emptyset();
      return x & parts.second;
    }
    out2 = x & parts.second;
    return first;
  }

  interval ibexb_div2_inter(const interval& x, const interval& num, const interval& div)
  {
    interval out2;
    const interval first = ibexb_div2_inter(x, num, div, out2);
    return first | out2;
  }

  void ibexb_div2(const ibexb_div2_case& c)
  {
    const interval num = ibexb_interval(c.num), div = ibexb_interval(c.div);
    const auto describe = [&] { return "num = " + show(num) + ", div = " + show(div); };
    const std::pair<interval, interval> parts = gaol_ieee1788::mulRevToPair(div, num);
    expect(ibexb_label(c.name, "div2 out1"), parts.first, c.out1, describe);
    expect(ibexb_label(c.name, "div2 out2"), parts.second, c.out2, describe);
    // start.div2_inter(num, div) is hull_ref, start.div2_inter(num, div, out2) first_ref and second_ref
    const auto verify = [&](const std::string& start_name, const interval& start, const Ref& hull_ref,
                            const Ref& first_ref, const Ref& second_ref) {
      expect(ibexb_label(c.name, start_name + ".div2_inter(num,div)"), ibexb_div2_inter(start, num, div), hull_ref,
             describe);
      interval second;
      const interval first = ibexb_div2_inter(start, num, div, second);
      expect(ibexb_label(c.name, start_name + ".div2_inter(num,div,out2)"), first, first_ref, describe);
      expect(ibexb_label(c.name, start_name + ".div2_inter(num,div,out2) out2"), second, second_ref, describe);
    };
    verify("out1", ibexb_interval(c.out1), c.out1, c.out1, ibexb_empty);
    verify("out2", ibexb_interval(c.out2), c.out2, c.out2, ibexb_empty);
    verify("all reals", interval::universe(), ibexb_hull(c.out1, c.out2), c.out1, c.out2);
    if (is_none(c.out1bis)) {
      return;
    }
    // IBEX starts from [out1.mid(), (out1.ub() + out2.lb()) / 2] where out2 lies above out1: the part of
    // it in out1 is out1bis
    const interval bis1 = ibexb_interval(c.out1bis);
    const interval start = (!is_none(c.out2) && c.out2.lo > c.out1.hi)
                             ? interval(c.out1bis.lo, 0.5 * (c.out1.hi + c.out2.lo)) : bis1;
    expect(ibexb_label(c.name, "out1bis (shortened).div2_inter(num,div)"), ibexb_div2_inter(start, num, div), c.out1bis,
           describe);
    verify("out1bis", bis1, c.out1bis, c.out1bis, ibexb_empty);
    if (!is_none(c.out2bis)) {
      verify("out1bis|out2bis", bis1 | ibexb_interval(c.out2bis), ibexb_hull(c.out1bis, c.out2bis), c.out1bis,
             c.out2bis);
    }
  }

  //---------------------------------------------------------------------------
  // TestArith: bwd_add01 to bwd_add05, bwd_sub01 to bwd_sub05, bwd_mul01 to
  // bwd_mul05, bwd_div01 to bwd_div09. The helper of IBEX contracts x1 and x2 in
  // four ways, whose results are the same: with the operands in either order,
  // and on the opposites (y, x1 and x2, or y alone, as the operation allows)
  //---------------------------------------------------------------------------

  struct ibexb_binary_case
  {
    const char *name;
    Ref y, x1, x2, x1_after, x2_after;
    int doubles;
  };

  // The call of the helper: the signs of y, x1 and x2, and the operands swapped (the contractor
  // is called with x2 first)
  struct ibexb_variant
  {
    const char *label;
    int sign_y, sign_x1, sign_x2;
    bool swapped;
  };

  typedef void (*ibexb_contractor)(const interval& y, interval& a, interval& b);

  void ibexb_bwd_add(const interval& y, interval& x1, interval& x2)
  {
    x1 = x1 & (y - x2);
    x2 = x2 & (y - x1);
  }

  void ibexb_bwd_sub(const interval& y, interval& x1, interval& x2)
  {
    x1 = x1 & (y + x2);
    x2 = x2 & (x1 - y);
  }

  void ibexb_bwd_mul(const interval& y, interval& x1, interval& x2)
  {
    x1 = div_rel(y, x2, x1);
    x2 = div_rel(y, x1, x2);
  }

  void ibexb_bwd_div(const interval& y, interval& x1, interval& x2)
  {
    x1 = x1 & (y * x2);
    const interval t = div_rel(x1, x2, y);
    x2 = div_rel(x1, t, x2);
  }

  // BEGIN ibexb_add_cases (written by ibexb_cases.py: do not edit)
  static const ibexb_binary_case ibexb_add_cases[] = {
    { "IBEX TestArith::bwd_add01", { 0x1p+0, 0x1.8p+1 }, { 0x1p+0, 0x1p+1 }, { -0x1.4p+3, 0x1.4p+2 },
      { 0x1p+0, 0x1p+1 }, { -0x1p+0, 0x1p+1 }, 0 },
    { "IBEX TestArith::bwd_add02", ibexb_empty, { 0x1.999999999999ap-4, 0x1p+1 }, { -0x1.4p+3, 0x1p+1 }, ibexb_empty,
      ibexb_empty, 0 },
    { "IBEX TestArith::bwd_add03", { 0x0p+0, 0x0p+0 }, { 0x0p+0, 0x0p+0 }, { 0x1.999999999999ap-4, 0x1.4p+2 },
      ibexb_empty, ibexb_empty, 0 },
    { "IBEX TestArith::bwd_add04", { 0x0p+0, 0x0p+0 }, { -0x1p+0, 0x1p+0 }, { -0x1p+0, 0x1p+0 }, { -0x1p+0, 0x1p+0 },
      { -0x1p+0, 0x1p+0 }, 0 },
    { "IBEX TestArith::bwd_add05", { -0x1p+0, 0x1p+0 }, { 0x1p+0, 0x1p+1 }, { -0x1.4p+3, 0x1.4p+2 }, { 0x1p+0, 0x1p+1 },
      { -0x1.8p+1, 0x0p+0 }, 0 },
  };
  // END ibexb_add_cases

  // BEGIN ibexb_sub_cases (written by ibexb_cases.py: do not edit)
  static const ibexb_binary_case ibexb_sub_cases[] = {
    { "IBEX TestArith::bwd_sub01", { 0x1p+0, 0x1.8p+1 }, { 0x1p+0, 0x1p+1 }, { -0x1.4p+3, 0x1.4p+2 },
      { 0x1p+0, 0x1p+1 }, { -0x1p+1, 0x1p+0 }, 0 },
    { "IBEX TestArith::bwd_sub02", ibexb_empty, { 0x1.999999999999ap-4, 0x1p+1 }, { -0x1.4p+3, 0x1p+1 }, ibexb_empty,
      ibexb_empty, 0 },
    { "IBEX TestArith::bwd_sub03", { 0x0p+0, 0x0p+0 }, { 0x0p+0, 0x0p+0 }, { 0x1.999999999999ap-4, 0x1.4p+2 },
      ibexb_empty, ibexb_empty, 0 },
    { "IBEX TestArith::bwd_sub04", { 0x0p+0, 0x0p+0 }, { -0x1p+0, 0x1p+0 }, { -0x1p+0, 0x1p+0 }, { -0x1p+0, 0x1p+0 },
      { -0x1p+0, 0x1p+0 }, 0 },
    { "IBEX TestArith::bwd_sub05", { -0x1p+0, 0x1p+0 }, { 0x1p+0, 0x1p+1 }, { -0x1.4p+3, 0x1.4p+2 }, { 0x1p+0, 0x1p+1 },
      { 0x0p+0, 0x1.8p+1 }, 0 },
  };
  // END ibexb_sub_cases

  // BEGIN ibexb_mul_cases (written by ibexb_cases.py: do not edit)
  static const ibexb_binary_case ibexb_mul_cases[] = {
    { "IBEX TestArith::bwd_mul01", { 0x1p+0, 0x1p+1 }, { 0x1.999999999999ap-4, 0x1p+1 }, { -0x1.4p+3, 0x1p+1 },
      { 0x1p-1, 0x1p+1 }, { 0x1p-1, 0x1p+1 }, 0 },
    { "IBEX TestArith::bwd_mul02", ibexb_empty, { 0x1.999999999999ap-4, 0x1p+1 }, { -0x1.4p+3, 0x1p+1 }, ibexb_empty,
      ibexb_empty, 0 },
    { "IBEX TestArith::bwd_mul03", { 0x0p+0, 0x0p+0 }, { 0x1.999999999999ap-4, 0x1p+1 }, { -0x1.4p+3, 0x1p+1 },
      { 0x1.999999999999ap-4, 0x1p+1 }, { 0x0p+0, 0x0p+0 }, 0 },
    { "IBEX TestArith::bwd_mul04", { 0x0p+0, 0x0p+0 }, { -0x1p+0, 0x1p+0 }, { -0x1p+0, 0x1p+0 }, { -0x1p+0, 0x1p+0 },
      { -0x1p+0, 0x1p+0 }, 0 },
    { "IBEX TestArith::bwd_mul05", { 0x1p+0, 0x1p+0 }, { 0x0p+0, 0x1.4p+3 }, { 0x0p+0, 0x1.4p+3 },
      { 0x1.9999999999999p-4, 0x1.4p+3 }, { 0x1.9999999999999p-4, 0x1.4p+3 }, limit },
  };
  // END ibexb_mul_cases

  // BEGIN ibexb_div_cases (written by ibexb_cases.py: do not edit)
  static const ibexb_binary_case ibexb_div_cases[] = {
    { "IBEX TestArith::bwd_div01", { 0x1p+0, 0x1p+1 }, { 0x0p+0, 0x1p+0 }, { 0x1p+1, 0x1.8p+1 }, ibexb_empty,
      ibexb_empty, 0 },
    { "IBEX TestArith::bwd_div02", { 0x1p+0, 0x1p+1 }, { 0x0p+0, 0x1p+0 }, { 0x1p+0, 0x1.8p+1 }, { 0x1p+0, 0x1p+0 },
      { 0x1p+0, 0x1p+0 }, 0 },
    { "IBEX TestArith::bwd_div03", { 0x1p+0, 0x1p+1 }, { 0x1p+0, 0x1.8p+1 }, { 0x0p+0, 0x1p+0 }, { 0x1p+0, 0x1p+1 },
      { 0x1p-1, 0x1p+0 }, 0 },
    { "IBEX TestArith::bwd_div04", { -0x1p+0, 0x1p+0 }, { -0x1p+1, 0x1p+1 }, { 0x0p+0, 0x1p+0 }, { -0x1p+0, 0x1p+0 },
      { 0x0p+0, 0x1p+0 }, 0 },
    { "IBEX TestArith::bwd_div05", { -0x1p+0, 0x1p+0 }, { -0x1p+1, 0x1p+1 }, { 0x0p+0, 0x0p+0 }, { 0x0p+0, 0x0p+0 },
      { 0x0p+0, 0x0p+0 }, 0 },
    { "IBEX TestArith::bwd_div06", { 0x0p+0, 0x0p+0 }, { -0x1p+1, 0x1p+1 }, { -0x1p+1, 0x1p+1 }, { 0x0p+0, 0x0p+0 },
      { -0x1p+1, 0x1p+1 }, 0 },
    { "IBEX TestArith::bwd_div07", { 0x0p+0, ibexb_oo }, { 0x0p+0, 0x1p+0 }, { -0x1p+0, 0x0p+0 }, { 0x0p+0, 0x0p+0 },
      { -0x1p+0, 0x0p+0 }, 0 },
    { "IBEX TestArith::bwd_div08", { 0x0.0000000000001p-1022, ibexb_oo }, { 0x0p+0, 0x1p+0 }, { -0x1p+0, 0x0p+0 },
      { 0x0p+0, 0x0p+0 }, { 0x0p+0, 0x0p+0 }, 0 },
    { "IBEX TestArith::bwd_div09", { 0x0p+0, ibexb_oo }, { 0x0.0000000000001p-1022, 0x1p+0 }, { -0x1p+0, 0x0p+0 },
      ibexb_empty, ibexb_empty, 0 },
  };
  // END ibexb_div_cases

  static const ibexb_variant ibexb_add_variants[] = {
    { "bwd_add(y,x1,x2)", 1, 1, 1, false }, { "bwd_add(y,x2,x1)", 1, 1, 1, true },
    { "bwd_add(-y,-x1,-x2)", -1, -1, -1, false }, { "bwd_add(-y,-x2,-x1)", -1, -1, -1, true } };
  static const ibexb_variant ibexb_sub_variants[] = {
    { "bwd_sub(y,x1,x2)", 1, 1, 1, false }, { "bwd_sub(-y,-x1,-x2)", -1, -1, -1, false },
    { "bwd_sub(-y,x2,x1)", -1, 1, 1, true }, { "bwd_sub(y,-x2,-x1)", 1, -1, -1, true } };
  static const ibexb_variant ibexb_mul_variants[] = {
    { "bwd_mul(y,x1,x2)", 1, 1, 1, false }, { "bwd_mul(y,x2,x1)", 1, 1, 1, true },
    { "bwd_mul(-y,x2,-x1)", -1, -1, 1, true }, { "bwd_mul(-y,-x2,x1)", -1, 1, -1, true } };
  static const ibexb_variant ibexb_div_variants[] = {
    { "bwd_div(y,x1,x2)", 1, 1, 1, false }, { "bwd_div(y,-x1,-x2)", 1, -1, -1, false },
    { "bwd_div(-y,-x1,x2)", -1, -1, 1, false }, { "bwd_div(-y,x1,-x2)", -1, 1, -1, false } };

  inline interval ibexb_signed(int sign, const interval& x)
  {
    return sign < 0 ? -x : x;
  }

  inline Ref ibexb_signed(int sign, const Ref& r)
  {
    return sign < 0 ? ibexb_opposite(r) : r;
  }

  template<std::size_t N>
  void ibexb_binary(const ibexb_binary_case& c, ibexb_contractor contract, const ibexb_variant (&variants)[N])
  {
    const interval Y = ibexb_interval(c.y), X1 = ibexb_interval(c.x1), X2 = ibexb_interval(c.x2);
    for (const ibexb_variant& v : variants) {
      const interval y = ibexb_signed(v.sign_y, Y), a1 = ibexb_signed(v.sign_x1, X1), a2 = ibexb_signed(v.sign_x2, X2);
      interval x1 = a1, x2 = a2;
      if (v.swapped) {
        contract(y, x2, x1);
      } else {
        contract(y, x1, x2);
      }
      const auto describe = [&] { return "y = " + show(y) + ", x1 = " + show(a1) + ", x2 = " + show(a2); };
      expect(ibexb_label(c.name, std::string(v.label) + " x1"), x1, ibexb_signed(v.sign_x1, c.x1_after), describe,
             c.doubles);
      expect(ibexb_label(c.name, std::string(v.label) + " x2"), x2, ibexb_signed(v.sign_x2, c.x2_after), describe,
             c.doubles);
    }
  }

  void ibexb_binaries()
  {
    for (const ibexb_binary_case& c : ibexb_add_cases) {
      ibexb_binary(c, ibexb_bwd_add, ibexb_add_variants);
    }
    for (const ibexb_binary_case& c : ibexb_sub_cases) {
      ibexb_binary(c, ibexb_bwd_sub, ibexb_sub_variants);
    }
    for (const ibexb_binary_case& c : ibexb_mul_cases) {
      ibexb_binary(c, ibexb_bwd_mul, ibexb_mul_variants);
    }
    for (const ibexb_binary_case& c : ibexb_div_cases) {
      ibexb_binary(c, ibexb_bwd_div, ibexb_div_variants);
    }
  }

  //---------------------------------------------------------------------------
  // TestArith: bwd_maxmin01 to bwd_maxmin05, bwd_max(z, x, y) and bwd_min(z, x, y)
  // with the operands in either order, min on the opposites. IBEX compares with ==
  //---------------------------------------------------------------------------

  struct ibexb_maxmin_case
  {
    const char *name;
    Ref z, x, y, x_after, y_after;
  };

  // BEGIN ibexb_maxmin_cases (written by ibexb_cases.py: do not edit)
  static const ibexb_maxmin_case ibexb_maxmin_cases[] = {
    { "IBEX TestArith::bwd_maxmin01", ibexb_empty, { -0x1p+1, -0x1p+0 }, { -0x1p+1, 0x1.8p+1 }, ibexb_empty,
      ibexb_empty },
    { "IBEX TestArith::bwd_maxmin02", { 0x0p+0, 0x1p+0 }, { -0x1p+1, -0x1p+0 }, { -0x1p+1, 0x1.8p+1 },
      { -0x1p+1, -0x1p+0 }, { 0x0p+0, 0x1p+0 } },
    { "IBEX TestArith::bwd_maxmin03", { 0x0p+0, 0x1p+0 }, { -0x1p+1, -0x1p+0 }, { 0x1p+1, 0x1.8p+1 }, ibexb_empty,
      ibexb_empty },
    { "IBEX TestArith::bwd_maxmin04", { 0x0p+0, 0x1p+0 }, { -0x1p+1, 0x0p+0 }, { -0x1p+1, 0x1.8p+1 },
      { -0x1p+1, 0x0p+0 }, { -0x1p+1, 0x1p+0 } },
    { "IBEX TestArith::bwd_maxmin05", { 0x0p+0, 0x1p+0 }, { -0x1p+1, 0x1p+1 }, { -0x1p+1, 0x1.8p+1 },
      { -0x1p+1, 0x1p+0 }, { -0x1p+1, 0x1p+0 } },
  };
  // END ibexb_maxmin_cases

  void ibexb_bwd_max(const interval& z, interval& x, interval& y)
  {
    x = max_rel(z, y, x);
    y = max_rel(z, x, y);
  }

  void ibexb_bwd_min(const interval& z, interval& x, interval& y)
  {
    x = min_rel(z, y, x);
    y = min_rel(z, x, y);
  }

  void ibexb_maxmins()
  {
    for (const ibexb_maxmin_case& c : ibexb_maxmin_cases) {
      const interval Z = ibexb_interval(c.z), X = ibexb_interval(c.x), Y = ibexb_interval(c.y);
      const auto describe = [&] { return "z = " + show(Z) + ", x = " + show(X) + ", y = " + show(Y); };
      // sign: the results are those of -x and -y, bwd_min on the opposites
      const auto verify = [&](const char *variant, const interval& x, const interval& y, int sign) {
        expect(ibexb_label(c.name, std::string(variant) + " x"), ibexb_signed(sign, x), c.x_after, describe);
        expect(ibexb_label(c.name, std::string(variant) + " y"), ibexb_signed(sign, y), c.y_after, describe);
      };
      interval x = X, y = Y;
      ibexb_bwd_max(Z, x, y);
      verify("bwd_max(z,x,y)", x, y, 1);
      x = X;
      y = Y;
      ibexb_bwd_max(Z, y, x);
      verify("bwd_max(z,y,x)", x, y, 1);
      x = -X;
      y = -Y;
      ibexb_bwd_min(-Z, x, y);
      verify("bwd_min(-z,-x,-y)", x, y, -1);
      x = -X;
      y = -Y;
      ibexb_bwd_min(-Z, y, x);
      verify("bwd_min(-z,-y,-x)", x, y, -1);
    }
  }

  //---------------------------------------------------------------------------
  // TestArith: bwd_sqr01 to bwd_sqr07, bwd_log01 to bwd_log08, bwd_sqrt01 to
  // bwd_sqrt07, bwd_floor_01 to bwd_floor_11, bwd_ceil_01 to bwd_ceil_11: the
  // contraction of x by y. IBEX compares the floor and the ceil with ==
  //---------------------------------------------------------------------------

  struct ibexb_projection_case
  {
    const char *name, *function;
    Ref y, x, result;
    int doubles;
  };

  // BEGIN ibexb_projection_cases (written by ibexb_cases.py: do not edit)
  static const ibexb_projection_case ibexb_projection_cases[] = {
    { "IBEX TestArith::bwd_sqr01", "sqr", { 0x1p+0, 0x1.2p+3 }, { 0x0p+0, 0x1p+2 }, { 0x1p+0, 0x1.8p+1 }, 0 },
    { "IBEX TestArith::bwd_sqr02", "sqr", { 0x1p+0, 0x1.2p+3 }, { 0x0p+0, 0x1p+1 }, { 0x1p+0, 0x1p+1 }, 0 },
    { "IBEX TestArith::bwd_sqr03", "sqr", { 0x1p+0, 0x1.2p+3 }, { -0x1p+2, 0x1p+1 }, { -0x1.8p+1, 0x1p+1 }, 0 },
    { "IBEX TestArith::bwd_sqr04", "sqr", { 0x1p+0, 0x1.2p+3 }, { -0x1p+2, -0x1.8p+1 }, { -0x1.8p+1, -0x1.8p+1 }, 0 },
    { "IBEX TestArith::bwd_sqr05", "sqr", { -ibexb_oo, 0x1.2p+3 }, { -0x1p+2, 0x1p+0 }, { -0x1.8p+1, 0x1p+0 }, 0 },
    { "IBEX TestArith::bwd_sqr06", "sqr", { 0x1p+2, 0x1.2p+3 }, { -0x1p+0, 0x1.4p+2 }, { 0x1p+1, 0x1.8p+1 }, 0 },
    { "IBEX TestArith::bwd_sqr07", "sqr", { -0x1p+2, -0x1p+1 }, { -ibexb_oo, ibexb_oo }, ibexb_empty, 0 },
    { "IBEX TestArith::bwd_log01", "log", { -ibexb_oo, ibexb_oo }, { -ibexb_oo, ibexb_oo }, { 0x0p+0, ibexb_oo }, 0 },
    { "IBEX TestArith::bwd_log02", "log", { -ibexb_oo, 0x0p+0 }, { -ibexb_oo, ibexb_oo }, { 0x0p+0, 0x1p+0 }, 0 },
    { "IBEX TestArith::bwd_log03", "log", { 0x0p+0, ibexb_oo }, { 0x0p+0, 0x1p+0 }, { 0x1p+0, 0x1p+0 }, 0 },
    { "IBEX TestArith::bwd_log04", "log", { 0x0p+0, 0x1p+0 }, { -0x1p+0, 0x1.8p+1 }, { 0x1p+0, 0x1.5bf0a8b14576ap+1 },
      limit },
    { "IBEX TestArith::bwd_log05", "log", { -ibexb_oo, 0x1p+0 }, { -0x1p+0, 0x1.8p+1 },
      { 0x0p+0, 0x1.5bf0a8b14576ap+1 }, limit },
    { "IBEX TestArith::bwd_log06", "log", { -ibexb_oo, 0x1p+0 }, { -0x1p+0, 0x1p+1 }, { 0x0p+0, 0x1p+1 }, 0 },
    { "IBEX TestArith::bwd_log07", "log", { -ibexb_oo, 0x1p+0 }, { 0x1.8p+1, 0x1p+2 }, ibexb_empty, 0 },
    { "IBEX TestArith::bwd_log08", "log", { -0x1p+0, 0x1p+0 }, { -0x1.47ae147ae147bp-7, 0x1.47ae147ae147bp-7 },
      ibexb_empty, 0 },
    { "IBEX TestArith::bwd_sqrt01", "sqrt", { 0x1p+0, 0x1.8p+1 }, { 0x0p+0, 0x1.ep+3 }, { 0x1p+0, 0x1.2p+3 }, 0 },
    { "IBEX TestArith::bwd_sqrt02", "sqrt", { 0x1p+0, 0x1.8p+1 }, { 0x0p+0, 0x1p+1 }, { 0x1p+0, 0x1p+1 }, 0 },
    { "IBEX TestArith::bwd_sqrt03", "sqrt", { 0x1p+0, 0x1.8p+1 }, { -0x1p+2, 0x1p+1 }, { 0x1p+0, 0x1p+1 }, 0 },
    { "IBEX TestArith::bwd_sqrt04", "sqrt", { 0x1p+0, 0x1.8p+1 }, { -0x1p+2, -0x1.8p+1 }, ibexb_empty, 0 },
    { "IBEX TestArith::bwd_sqrt05", "sqrt", { -ibexb_oo, 0x1.2p+3 }, { -0x1p+2, 0x1p+0 }, { 0x0p+0, 0x1p+0 }, 0 },
    { "IBEX TestArith::bwd_sqrt06", "sqrt", { 0x1p+1, 0x1.4p+2 }, { -0x1p+0, 0x1.4p+2 }, { 0x1p+2, 0x1.4p+2 }, 0 },
    { "IBEX TestArith::bwd_sqrt07", "sqrt", { -0x1p+2, -0x1p+1 }, { -ibexb_oo, ibexb_oo }, ibexb_empty, 0 },
    { "IBEX TestArith::bwd_floor_01", "floor", ibexb_empty, { -ibexb_oo, ibexb_oo }, ibexb_empty, 0 },
    { "IBEX TestArith::bwd_floor_02", "floor", { -ibexb_oo, -0x1p+0 }, { -ibexb_oo, -0x1.0c6f7a0b5ed8dp-20 },
      { -ibexb_oo, -0x1.0c6f7a0b5ed8dp-20 }, 0 },
    { "IBEX TestArith::bwd_floor_03", "floor", { -ibexb_oo, -0x1p+0 }, { -ibexb_oo, 0x1.0c6f7a0b5ed8dp-20 },
      { -ibexb_oo, 0x0p+0 }, 0 },
    { "IBEX TestArith::bwd_floor_04", "floor", { -ibexb_oo, -0x1.0c6f7a0b5ed8dp-20 },
      { -ibexb_oo, 0x1.0c6f7a0b5ed8dp-20 }, { -ibexb_oo, 0x0p+0 }, 0 },
    { "IBEX TestArith::bwd_floor_05", "floor", { 0x1p+0, ibexb_oo }, { 0x1.0c6f7a0b5ed8dp-20, ibexb_oo },
      { 0x1p+0, ibexb_oo }, 0 },
    { "IBEX TestArith::bwd_floor_06", "floor", { 0x1p+0, ibexb_oo }, { -0x1.0c6f7a0b5ed8dp-20, ibexb_oo },
      { 0x1p+0, ibexb_oo }, 0 },
    { "IBEX TestArith::bwd_floor_07", "floor", { 0x1.0c6f7a0b5ed8dp-20, ibexb_oo },
      { -0x1.0c6f7a0b5ed8dp-20, ibexb_oo }, { 0x1p+0, ibexb_oo }, 0 },
    { "IBEX TestArith::bwd_floor_08", "floor", { 0x1p+0, 0x1p+1 }, { 0x1.47ae147ae147bp-7, 0x1.7eb851eb851ecp+1 },
      { 0x1p+0, 0x1.7eb851eb851ecp+1 }, 0 },
    { "IBEX TestArith::bwd_floor_09", "floor", { 0x1p+0, 0x1p+1 }, { 0x1.7eb851eb851ecp+1, 0x1.8147ae147ae14p+1 },
      { 0x1.7eb851eb851ecp+1, 0x1.8p+1 }, 0 },
    { "IBEX TestArith::bwd_floor_10", "floor", { 0x1p+0, 0x1p+1 }, { 0x1.8147ae147ae14p+1, 0x1.feb851eb851ecp+1 },
      ibexb_empty, 0 },
    { "IBEX TestArith::bwd_floor_11", "floor", { 0x1.47ae147ae147bp-7, 0x1.7eb851eb851ecp+1 },
      { 0x1.47ae147ae147bp-7, 0x1.7eb851eb851ecp+1 }, { 0x1p+0, 0x1.7eb851eb851ecp+1 }, 0 },
    { "IBEX TestArith::bwd_ceil_01", "ceil", ibexb_empty, { -ibexb_oo, ibexb_oo }, ibexb_empty, 0 },
    { "IBEX TestArith::bwd_ceil_02", "ceil", { -ibexb_oo, -0x1p+0 }, { -ibexb_oo, -0x1.0c6f7a0b5ed8dp-20 },
      { -ibexb_oo, -0x1p+0 }, 0 },
    { "IBEX TestArith::bwd_ceil_03", "ceil", { -ibexb_oo, -0x1p+0 }, { -ibexb_oo, 0x1.0c6f7a0b5ed8dp-20 },
      { -ibexb_oo, -0x1p+0 }, 0 },
    { "IBEX TestArith::bwd_ceil_04", "ceil", { -ibexb_oo, -0x1.0c6f7a0b5ed8dp-20 },
      { -ibexb_oo, 0x1.0c6f7a0b5ed8dp-20 }, { -ibexb_oo, -0x1p+0 }, 0 },
    { "IBEX TestArith::bwd_ceil_05", "ceil", { 0x1p+0, ibexb_oo }, { 0x1.0c6f7a0b5ed8dp-20, ibexb_oo },
      { 0x1.0c6f7a0b5ed8dp-20, ibexb_oo }, 0 },
    { "IBEX TestArith::bwd_ceil_06", "ceil", { 0x1p+0, ibexb_oo }, { -0x1.0c6f7a0b5ed8dp-20, ibexb_oo },
      { 0x0p+0, ibexb_oo }, 0 },
    { "IBEX TestArith::bwd_ceil_07", "ceil", { 0x1.0c6f7a0b5ed8dp-20, ibexb_oo }, { -0x1.0c6f7a0b5ed8dp-20, ibexb_oo },
      { 0x0p+0, ibexb_oo }, 0 },
    { "IBEX TestArith::bwd_ceil_08", "ceil", { 0x1p+0, 0x1p+1 }, { 0x1.47ae147ae147bp-7, 0x1.7eb851eb851ecp+1 },
      { 0x1.47ae147ae147bp-7, 0x1p+1 }, 0 },
    { "IBEX TestArith::bwd_ceil_09", "ceil", { 0x1p+0, 0x1p+1 }, { -0x1.47ae147ae147bp-7, 0x1.47ae147ae147bp-7 },
      { 0x0p+0, 0x1.47ae147ae147bp-7 }, 0 },
    { "IBEX TestArith::bwd_ceil_10", "ceil", { 0x1p+0, 0x1p+1 }, { -0x1.028f5c28f5c29p+0, -0x1.47ae147ae147bp-7 },
      ibexb_empty, 0 },
    { "IBEX TestArith::bwd_ceil_11", "ceil", { 0x1.47ae147ae147bp-7, 0x1.7eb851eb851ecp+1 },
      { 0x1.47ae147ae147bp-7, 0x1.7eb851eb851ecp+1 }, { 0x1.47ae147ae147bp-7, 0x1p+1 }, 0 },
  };
  // END ibexb_projection_cases

  typedef interval (*ibexb_projector)(const interval& y, const interval& x);

  const std::map<std::string, ibexb_projector>& ibexb_projectors()
  {
    static const std::map<std::string, ibexb_projector> projectors = {
      { "sqr", [](const interval& y, const interval& x) { return sqrt_rel(y, x); } },
      { "sqrt", [](const interval& y, const interval& x) { return x & sqr(y & interval::positive()); } },
      { "log", [](const interval& y, const interval& x) { return x & exp(y); } },
      { "floor", [](const interval& y, const interval& x) { return floor_rel(y, x); } },
      { "ceil", [](const interval& y, const interval& x) { return -floor_rel(-y, -x); } },
    };
    return projectors;
  }

  void ibexb_projections()
  {
    for (const ibexb_projection_case& c : ibexb_projection_cases) {
      const interval Y = ibexb_interval(c.y), X = ibexb_interval(c.x);
      const auto describe = [&] { return std::string(c.function) + ": y = " + show(Y) + ", x = " + show(X); };
      const ibexb_projector f = ibexb_projectors().at(c.function);
      expect(c.name, evaluate(c.name, [&] { return f(Y, X); }, describe), c.result, describe, c.doubles);
    }
  }

  //---------------------------------------------------------------------------
  // TestArith: bwd_sin01 to bwd_sin24. The helper of IBEX contracts x with the sine
  // of x in y, with the sine of -x in -y, and with the cosines of x - pi/2 in y and
  // of x + pi/2 in -y, which are the same sines
  //---------------------------------------------------------------------------

  struct ibexb_trig_case
  {
    const char *name;
    Ref y, x;
    Ref result[4];
    int doubles;
  };

  // BEGIN ibexb_trig_cases (written by ibexb_cases.py: do not edit)
  static const ibexb_trig_case ibexb_trig_cases[] = {
    { "IBEX TestArith::bwd_sin01", { 0x1.eaee8744b05efp-2, 0x1.feb7a9b2c6d8bp-1 }, { 0x0p+0, 0x1.921fb54442d19p+0 },
      { { 0x1.ffffffffffffep-2, 0x1.8000000000001p+0 }, { -0x1.8000000000001p+0, -0x1.ffffffffffffep-2 },
        { -0x1.121fb54442d19p+0, -0x1.21fb54442d175p-4 }, { 0x1.090fdaa22168cp+1, 0x1.890fdaa22168dp+1 } }, limit },
    { "IBEX TestArith::bwd_sin02", { 0x1.eaee8744b05efp-2, 0x1.feb7a9b2c6d8bp-1 }, { 0x0p+0, 0x1.f6a7a2955386p+2 },
      { { 0x1.ffffffffffffep-2, 0x1.f21fb54442d19p+2 }, { -0x1.f21fb54442d19p+2, -0x1.ffffffffffffep-2 },
        { -0x1.121fb54442d19p+0, 0x1.8d97c7f3321d3p+2 }, { 0x1.090fdaa22168cp+1, 0x1.2b53d14aa9c3p+3 } }, limit },
    { "IBEX TestArith::bwd_sin03", { 0x1.eaee8744b05efp-2, 0x1.feb7a9b2c6d8bp-1 },
      { -0x1.921fb54442d19p+2, 0x1.921fb54442d19p+0 },
      { { -0x1.721fb54442d19p+2, 0x1.8000000000001p+0 }, { -0x1.8000000000001p+0, 0x1.721fb54442d19p+2 },
        { -0x1.d6a7a2955385fp+2, -0x1.21fb54442d175p-4 }, { -0x1.0d97c7f3321d3p+2, 0x1.890fdaa22168dp+1 } }, limit },
    { "IBEX TestArith::bwd_sin04", { 0x1.eaee8744b05efp-2, 0x1.feb7a9b2c6d8bp-1 },
      { -0x1.921fb54442d19p+2, 0x1.f6a7a2955386p+2 },
      { { -0x1.721fb54442d19p+2, 0x1.f21fb54442d19p+2 }, { -0x1.f21fb54442d19p+2, 0x1.721fb54442d19p+2 },
        { -0x1.d6a7a2955385fp+2, 0x1.8d97c7f3321d3p+2 }, { -0x1.0d97c7f3321d3p+2, 0x1.2b53d14aa9c3p+3 } }, limit },
    { "IBEX TestArith::bwd_sin05", { 0x1.eaee8744b05efp-2, 0x1.feb7a9b2c6d8bp-1 },
      { -0x1.c552e8777604bp+1, 0x1.921fb54442d19p+0 },
      { { 0x1.ffffffffffffep-2, 0x1.8000000000001p+0 }, { -0x1.8000000000001p+0, -0x1.ffffffffffffep-2 },
        { -0x1.121fb54442d19p+0, -0x1.21fb54442d175p-4 }, { 0x1.090fdaa22168cp+1, 0x1.890fdaa22168dp+1 } }, limit },
    { "IBEX TestArith::bwd_sin06", { 0x1.eaee8744b05efp-2, 0x1.feb7a9b2c6d8bp-1 },
      { -0x1.d21fb54442d19p+1, 0x1.921fb54442d19p+0 },
      { { -0x1.d21fb54442d19p+1, 0x1.8000000000001p+0 }, { -0x1.8000000000001p+0, 0x1.d21fb54442d19p+1 },
        { -0x1.4d97c7f3321d3p+2, -0x1.21fb54442d175p-4 }, { -0x1.090fdaa22168dp+1, 0x1.890fdaa22168dp+1 } }, limit },
    { "IBEX TestArith::bwd_sin07", { 0x1.eaee8744b05efp-2, 0x1.feb7a9b2c6d8bp-1 },
      { -0x1.2d97c7f3321d2p+2, 0x1.921fb54442d19p+0 },
      { { -0x1.290fdaa22168dp+2, 0x1.8000000000001p+0 }, { -0x1.8000000000001p+0, 0x1.290fdaa22168dp+2 },
        { -0x1.8d97c7f3321d3p+2, -0x1.21fb54442d175p-4 }, { -0x1.890fdaa22168dp+1, 0x1.890fdaa22168dp+1 } }, limit },
    { "IBEX TestArith::bwd_sin08", { 0x1.eaee8744b05efp-2, 0x1.feb7a9b2c6d8bp-1 }, { 0x1p-1, 0x1.8aa5d0eeec096p+0 },
      { { 0x1p-1, 0x1.8000000000001p+0 }, { -0x1.8000000000001p+0, -0x1p-1 },
        { -0x1.121fb54442d19p+0, -0x1.21fb54442d175p-4 }, { 0x1.090fdaa22168cp+1, 0x1.890fdaa22168dp+1 } }, limit },
    { "IBEX TestArith::bwd_sin09", { 0x1.eaee8744b05efp-2, 0x1.feb7a9b2c6d8bp-1 }, { 0x1p-1, 0x1.a43f6a8885a32p+0 },
      { { 0x1p-1, 0x1.a43f6a8885a32p+0 }, { -0x1.a43f6a8885a32p+0, -0x1p-1 },
        { -0x1.121fb54442d19p+0, 0x1.21fb54442d1ap-4 }, { 0x1.090fdaa22168cp+1, 0x1.9b2f8fe6643a6p+1 } }, limit },
    { "IBEX TestArith::bwd_sin10", { 0x1.eaee8744b05efp-2, 0x1.feb7a9b2c6d8bp-1 }, { 0x1p-1, 0x1.921fb54442d19p+1 },
      { { 0x1p-1, 0x1.521fb54442d19p+1 }, { -0x1.521fb54442d19p+1, -0x1p-1 },
        { -0x1.121fb54442d19p+0, 0x1.121fb54442d19p+0 }, { 0x1.090fdaa22168cp+1, 0x1.0d97c7f3321d3p+2 } }, limit },
    { "IBEX TestArith::bwd_sin11", { 0x1.eaee8744b05efp-2, 0x1.feb7a9b2c6d8bp-1 },
      { -0x1.d21fb54442d19p+1, 0x1.a43f6a8885a32p+0 },
      { { -0x1.d21fb54442d19p+1, 0x1.a43f6a8885a32p+0 }, { -0x1.a43f6a8885a32p+0, 0x1.d21fb54442d19p+1 },
        { -0x1.4d97c7f3321d3p+2, 0x1.21fb54442d1ap-4 }, { -0x1.090fdaa22168dp+1, 0x1.9b2f8fe6643a6p+1 } }, 0 },
    { "IBEX TestArith::bwd_sin12", { 0x1.eaee8744b05efp-2, 0x1.feb7a9b2c6d8bp-1 },
      { -0x1.2d97c7f3321d2p+2, 0x1.921fb54442d19p+1 },
      { { -0x1.290fdaa22168dp+2, 0x1.521fb54442d19p+1 }, { -0x1.521fb54442d19p+1, 0x1.290fdaa22168dp+2 },
        { -0x1.8d97c7f3321d3p+2, 0x1.121fb54442d19p+0 }, { -0x1.890fdaa22168dp+1, 0x1.0d97c7f3321d3p+2 } }, limit },
    { "IBEX TestArith::bwd_sin13", { 0x1.eaee8744b05efp-2, 0x1.feb7a9b2c6d8bp-1 },
      { -0x1.921fb54442d19p+0, 0x1.921fb54442d19p+0 },
      { { 0x1.ffffffffffffep-2, 0x1.8000000000001p+0 }, { -0x1.8000000000001p+0, -0x1.ffffffffffffep-2 },
        { -0x1.121fb54442d19p+0, -0x1.21fb54442d175p-4 }, { 0x1.090fdaa22168cp+1, 0x1.890fdaa22168dp+1 } }, limit },
    { "IBEX TestArith::bwd_sin14", { 0x1p+1, 0x1.8p+1 }, { -ibexb_oo, ibexb_oo },
      { ibexb_empty, ibexb_empty, ibexb_empty, ibexb_empty }, 0 },
    { "IBEX TestArith::bwd_sin15", { 0x1.eaee8744b05fp-2, 0x1p+0 }, { 0x0p+0, 0x1.921fb54442d19p+0 },
      { { 0x1p-1, 0x1.921fb54442d19p+0 }, { -0x1.921fb54442d19p+0, -0x1p-1 }, { -0x1.121fb54442d19p+0, 0x1p-52 },
        { 0x1.090fdaa22168cp+1, 0x1.921fb54442d19p+1 } }, limit },
    { "IBEX TestArith::bwd_sin16", { 0x1.eaee8744b05fp-2, 0x1p+0 }, { 0x0p+0, 0x1.f6a7a2955386p+2 },
      { { 0x1p-1, 0x1.f6a7a2955386p+2 }, { -0x1.f6a7a2955386p+2, -0x1p-1 },
        { -0x1.121fb54442d19p+0, 0x1.921fb54442d1ap+2 }, { 0x1.090fdaa22168cp+1, 0x1.2d97c7f3321d4p+3 } }, limit },
    { "IBEX TestArith::bwd_sin17", { 0x1.eaee8744b05fp-2, 0x1p+0 }, { -0x1.921fb54442d19p+2, 0x1.921fb54442d19p+0 },
      { { -0x1.721fb54442d19p+2, 0x1.921fb54442d19p+0 }, { -0x1.921fb54442d19p+0, 0x1.721fb54442d19p+2 },
        { -0x1.d6a7a2955385fp+2, 0x1p-52 }, { -0x1.0d97c7f3321d3p+2, 0x1.921fb54442d19p+1 } }, limit },
    { "IBEX TestArith::bwd_sin18", { 0x1.eaee8744b05fp-2, 0x1p+0 }, { -0x1.921fb54442d19p+2, 0x1.f6a7a2955386p+2 },
      { { -0x1.721fb54442d19p+2, 0x1.f6a7a2955386p+2 }, { -0x1.f6a7a2955386p+2, 0x1.721fb54442d19p+2 },
        { -0x1.d6a7a2955385fp+2, 0x1.921fb54442d1ap+2 }, { -0x1.0d97c7f3321d3p+2, 0x1.2d97c7f3321d4p+3 } }, limit },
    { "IBEX TestArith::bwd_sin19", { 0x1.eaee8744b05fp-2, 0x1p+0 }, { -0x1.c552e8777604bp+1, 0x1.921fb54442d19p+0 },
      { { 0x1p-1, 0x1.921fb54442d19p+0 }, { -0x1.921fb54442d19p+0, -0x1p-1 }, { -0x1.121fb54442d19p+0, 0x1p-52 },
        { 0x1.090fdaa22168cp+1, 0x1.921fb54442d19p+1 } }, limit },
    { "IBEX TestArith::bwd_sin20", { 0x1.eaee8744b05fp-2, 0x1p+0 }, { -0x1.d21fb54442d19p+1, 0x1.921fb54442d19p+0 },
      { { -0x1.d21fb54442d19p+1, 0x1.921fb54442d19p+0 }, { -0x1.921fb54442d19p+0, 0x1.d21fb54442d19p+1 },
        { -0x1.4d97c7f3321d3p+2, 0x1p-52 }, { -0x1.090fdaa22168dp+1, 0x1.921fb54442d19p+1 } }, 0 },
    { "IBEX TestArith::bwd_sin21", { 0x1.eaee8744b05fp-2, 0x1p+0 }, { -0x1.2d97c7f3321d2p+2, 0x1.921fb54442d19p+0 },
      { { -0x1.2d97c7f3321d2p+2, 0x1.921fb54442d19p+0 }, { -0x1.921fb54442d19p+0, 0x1.2d97c7f3321d2p+2 },
        { -0x1.921fb54442d19p+2, 0x1p-52 }, { -0x1.921fb54442d18p+1, 0x1.921fb54442d19p+1 } }, 0 },
    { "IBEX TestArith::bwd_sin22", { 0x1.eaee8744b05fp-2, 0x1p+0 }, { 0x1p-1, 0x1.921fb54442d19p+1 },
      { { 0x1p-1, 0x1.521fb54442d19p+1 }, { -0x1.521fb54442d19p+1, -0x1p-1 },
        { -0x1.121fb54442d19p+0, 0x1.121fb54442d19p+0 }, { 0x1.090fdaa22168cp+1, 0x1.0d97c7f3321d3p+2 } }, limit },
    { "IBEX TestArith::bwd_sin23", { 0x1.eaee8744b05fp-2, 0x1p+0 }, { -0x1.2d97c7f3321d2p+2, 0x1.921fb54442d19p+1 },
      { { -0x1.2d97c7f3321d2p+2, 0x1.521fb54442d19p+1 }, { -0x1.521fb54442d19p+1, 0x1.2d97c7f3321d2p+2 },
        { -0x1.921fb54442d19p+2, 0x1.121fb54442d19p+0 }, { -0x1.921fb54442d18p+1, 0x1.0d97c7f3321d3p+2 } }, limit },
    { "IBEX TestArith::bwd_sin24", { 0x1p+0, 0x1p+0 }, { 0x1p+2, 0x1.8p+2 },
      { ibexb_empty, ibexb_empty, ibexb_empty, ibexb_empty }, 0 },
  };
  // END ibexb_trig_cases

  void ibexb_trigs()
  {
    static const char *const variants[4] = { "bwd_sin(y,x)", "bwd_sin(-y,-x)", "bwd_cos(y,x-pi/2)",
                                             "bwd_cos(-y,x+pi/2)" };
    const interval half_pi = interval::half_pi();
    for (const ibexb_trig_case& c : ibexb_trig_cases) {
      const interval y = ibexb_interval(c.y), x = ibexb_interval(c.x);
      const auto describe = [&] { return "y = " + show(y) + ", x = " + show(x); };
      const interval r[4] = { asin_rel(y, x), asin_rel(-y, -x), acos_rel(y, x - half_pi), acos_rel(-y, x + half_pi) };
      for (std::size_t i = 0; i < 4; ++i) {
        expect(ibexb_label(c.name, variants[i]), r[i], c.result[i], describe, c.doubles);
      }
    }
  }

  //---------------------------------------------------------------------------
  // TestArith: bwd_pow01 to bwd_pow17, x^p in y, for x and for -x (and -y for an
  // odd p). IBEX's wrapper projects through root() for every p (nth_root_rel was
  // wrong in GAOL 4); nth_root_rel(y, p, x) is the hull of the x of x with x^p in
  // y, for p >= 1. For p < 0 IBEX's own: the roots of y, and their opposites for an
  // even p, meet x
  //---------------------------------------------------------------------------

  struct ibexb_pow_case
  {
    const char *name;
    Ref y, x;
    int p;
    Ref result;
    int doubles;
  };

  // BEGIN ibexb_pow_cases (written by ibexb_cases.py: do not edit)
  static const ibexb_pow_case ibexb_pow_cases[] = {
    { "IBEX TestArith::bwd_pow01", { 0x1p+4, 0x1.44p+6 }, { 0x1p+0, 0x1p+2 }, 4, { 0x1p+1, 0x1.8p+1 }, 0 },
    { "IBEX TestArith::bwd_pow02", { 0x1p+4, 0x1.44p+6 }, { 0x1.8p+1, 0x1p+2 }, 4, { 0x1.8p+1, 0x1.8p+1 }, 0 },
    { "IBEX TestArith::bwd_pow03", { 0x1p+4, 0x1.44p+6 }, { -0x1p+0, 0x1p+2 }, 4, { 0x1p+1, 0x1.8p+1 }, 0 },
    { "IBEX TestArith::bwd_pow04", { 0x1p+4, 0x1.44p+6 }, { -0x1p+1, 0x1p+2 }, 4, { -0x1p+1, 0x1.8p+1 }, 0 },
    { "IBEX TestArith::bwd_pow05", { 0x1p+4, 0x1.44p+6 }, { -0x1.4p+2, 0x1p+2 }, 4, { -0x1.8p+1, 0x1.8p+1 }, 0 },
    { "IBEX TestArith::bwd_pow06", { 0x1p+4, 0x1.44p+6 }, { 0x1p+0, 0x1p+0 }, 4, ibexb_empty, 0 },
    { "IBEX TestArith::bwd_pow07", { 0x1p+4, 0x1.44p+6 }, { 0x1p+2, 0x1p+2 }, 4, ibexb_empty, 0 },
    { "IBEX TestArith::bwd_pow08", { 0x1p+3, 0x1.bp+4 }, { 0x1p+0, 0x1p+2 }, 3, { 0x1p+1, 0x1.8p+1 }, 0 },
    { "IBEX TestArith::bwd_pow09", { 0x1p+3, 0x1.bp+4 }, { 0x1.8p+1, 0x1p+2 }, 3, { 0x1.8p+1, 0x1.8p+1 }, 0 },
    { "IBEX TestArith::bwd_pow10", { 0x1p+3, 0x1.bp+4 }, { -0x1.4p+2, 0x1p+2 }, 3, { 0x1p+1, 0x1.8p+1 }, 0 },
    { "IBEX TestArith::bwd_pow11", { -0x1p+3, 0x1.bp+4 }, { -0x1.4p+2, 0x1p+2 }, 3, { -0x1p+1, 0x1.8p+1 }, 0 },
    { "IBEX TestArith::bwd_pow12", { -0x1.bp+4, 0x1.bp+4 }, { -0x1.4p+2, 0x1p+2 }, 3, { -0x1.8p+1, 0x1.8p+1 }, 0 },
    { "IBEX TestArith::bwd_pow13", { 0x1p+3, 0x1.bp+4 }, { 0x1p+0, 0x1p+0 }, 3, ibexb_empty, 0 },
    { "IBEX TestArith::bwd_pow14", { 0x1p+3, 0x1.bp+4 }, { 0x1p+2, 0x1p+2 }, 3, ibexb_empty, 0 },
    { "IBEX TestArith::bwd_pow15", { -0x1.bp+4, -0x1p+3 }, { -0x1p+0, -0x1p+0 }, 3, ibexb_empty, 0 },
    { "IBEX TestArith::bwd_pow16", { -0x1.bp+4, -0x1p+3 }, { -0x1p+2, -0x1p+2 }, 3, ibexb_empty, 0 },
    { "IBEX TestArith::bwd_pow17", { 0x0p+0, 0x1p+0 }, { -0x1.4p+3, 0x1.4p+3 }, -2, { -0x1.4p+3, 0x1.4p+3 }, 0 },
  };
  // END ibexb_pow_cases

  interval ibexb_bwd_pow(const interval& y, int p, const interval& x)
  {
    if (p > 0) {
      return nth_root_rel(y, static_cast<unsigned int>(p), x);
    }
    const interval root = nth_root(y, p);
    return p % 2 == 0 ? ((root & x) | ((-root) & x)) : (root & x);
  }

  void ibexb_pows()
  {
    for (const ibexb_pow_case& c : ibexb_pow_cases) {
      const interval y = ibexb_interval(c.y), x = ibexb_interval(c.x);
      const auto describe = [&] { return "y = " + show(y) + ", x = " + show(x) + ", p = " + std::to_string(c.p); };
      expect(ibexb_label(c.name, "bwd_pow(y,p,x)"), ibexb_bwd_pow(y, c.p, x), c.result, describe, c.doubles);
      expect(ibexb_label(c.name, c.p % 2 == 0 ? "bwd_pow(y,p,-x)" : "bwd_pow(-y,p,-x)"),
             ibexb_bwd_pow(c.p % 2 == 0 ? y : -y, c.p, -x), ibexb_opposite(c.result), describe, c.doubles);
    }
  }

  //---------------------------------------------------------------------------
  // TestArith: bwd_atan2_01 to bwd_atan2_10 and bwd_atan2_issue134: the points of
  // the box y x x whose angle atan2(y, x) is in the angles, y then x. The angles
  // are the doubles IBEX computes (M_PI/6., -3*M_PI/4. ...), rounded upward
  //---------------------------------------------------------------------------

  struct ibexb_atan2_case
  {
    const char *name;
    Ref theta, y, x, y_after, x_after;
    int doubles;
  };

  // BEGIN ibexb_atan2_cases (written by ibexb_cases.py: do not edit)
  static const ibexb_atan2_case ibexb_atan2_cases[] = {
    { "IBEX TestArith::bwd_atan2_01", { 0x1.0c152382d7366p-1, 0x1.0c152382d7366p+0 }, { 0x1p-1, 0x1.4p+3 },
      { 0x1p-1, 0x1p+1 }, { 0x1p-1, 0x1.bb67ae8584cadp+1 }, { 0x1p-1, 0x1p+1 }, ibexb_atan2_limit },
    { "IBEX TestArith::bwd_atan2_02", { 0x1.0c152382d7366p-1, 0x1.0c152382d7366p+0 }, { 0x1p-1, 0x1p+1 },
      { 0x1p-1, 0x1.4p+3 }, { 0x1p-1, 0x1p+1 }, { 0x1p-1, 0x1.bb67ae8584caap+1 }, ibexb_atan2_limit },
    { "IBEX TestArith::bwd_atan2_03", { -0x1.921fb54442d18p-1, 0x1.921fb54442d18p-1 }, { 0x1p+0, 0x1p+1 },
      { 0x1p-1, 0x1p+1 }, { 0x1p+0, 0x1p+1 }, { 0x1p+0, 0x1p+1 }, ibexb_atan2_limit },
    { "IBEX TestArith::bwd_atan2_04", { -0x1.921fb54442d18p+0, 0x0p+0 }, { 0x1p-1, 0x1p+1 }, { 0x1p-1, 0x1.4p+3 },
      ibexb_empty, ibexb_empty, 0 },
    { "IBEX TestArith::bwd_atan2_05", { 0x1.921fb54442d18p+2, 0x1.2d97c7f3321d2p+3 }, { -0x1p-1, 0x1p+1 },
      { 0x1p-1, 0x1.4p+3 }, ibexb_empty, ibexb_empty, 0 },
    { "IBEX TestArith::bwd_atan2_06", { 0x1.0c152382d7366p+1, 0x1.4f1a6c638d03fp+1 }, { 0x0p+0, 0x1.9p+6 },
      { -0x1.4p+4, -0x1.bb67ae8584cabp-1 }, { 0x1.fffffffffffffp-2, 0x1.1520cd1372fe9p+5 },
      { -0x1.4p+4, -0x1.bb67ae8584cabp-1 }, ibexb_atan2_limit },
    { "IBEX TestArith::bwd_atan2_07", { -0x1.2d97c7f3321d2p+1, -0x1.0c152382d7365p+1 },
      { -0x1.bb67ae8584cabp-1, 0x1p+1 }, { -0x1.6a09e667f3bcdp-1, 0x0p+0 }, { -0x1.bb67ae8584cabp-1, 0x0p+0 },
      { -0x1.6a09e667f3bcdp-1, 0x0p+0 }, 0 },
    { "IBEX TestArith::bwd_atan2_08", { -0x1.2d97c7f3321d2p+1, -0x1.0c152382d7365p+1 },
      { -0x1.bb67ae8584cabp-1, 0x1p+1 }, { -0x1p+0, -0x1p-1 }, { -0x1.bb67ae8584cabp-1, -0x1p-1 },
      { -0x1.bb67ae8584caap-1, -0x1p-1 }, ibexb_atan2_limit },
    { "IBEX TestArith::bwd_atan2_09", { -0x1.2d97c7f3321d2p+1, -0x1.921fb54442d18p-1 }, { -0x1.4p+2, -0x1p-1 },
      { -ibexb_oo, ibexb_oo }, { -0x1.4p+2, -0x1p-1 }, { -0x1.3ffffffffffffp+2, 0x1.4000000000001p+2 },
      ibexb_atan2_limit },
    { "IBEX TestArith::bwd_atan2_10", { -0x1.0c152382d7365p+0, 0x1.921fb54442d18p-1 }, { -ibexb_oo, ibexb_oo },
      { 0x1.bb67ae8584cabp-1, 0x1.bb67ae8584cabp-1 }, { -0x1.7ffffffffffffp+0, 0x1.bb67ae8584cabp-1 },
      { 0x1.bb67ae8584cabp-1, 0x1.bb67ae8584cabp-1 }, ibexb_atan2_limit },
    { "IBEX TestArith::bwd_atan2_issue134", { 0x1.921fb54442d18p+0, 0x1.921fb54442d19p+0 }, { 0x1p+0, 0x1p+0 },
      { 0x0p+0, 0x0p+0 }, { 0x1p+0, 0x1p+0 }, { 0x0p+0, 0x0p+0 }, 0 },
  };
  // END ibexb_atan2_cases

  void ibexb_atan2s()
  {
    for (const ibexb_atan2_case& c : ibexb_atan2_cases) {
      const interval theta = ibexb_interval(c.theta), Y = ibexb_interval(c.y), X = ibexb_interval(c.x);
      const auto describe = [&] { return "theta = " + show(theta) + ", y = " + show(Y) + ", x = " + show(X); };
      const interval y = evaluate(c.name, [&] { return atan2_rel_y(theta, X, Y); }, describe);
      const interval x = evaluate(c.name, [&] { return atan2_rel_x(theta, y, X); }, describe);
      expect(ibexb_label(c.name, "y"), y, c.y_after, describe, c.doubles);
      expect(ibexb_label(c.name, "x"), x, c.x_after, describe, c.doubles);
    }
  }

  //---------------------------------------------------------------------------
  // TestArith: bwd_atan01 to bwd_atan08: the x of x with atan(x) in y, the tangent of
  // the bounds of y inside (-pi/2, pi/2) (empty beyond it, infinite past a pole). No
  // double lies between half_pi().left() and half_pi().right(), the bounds of an
  // interval that holds pi/2: y reaches a pole where a bound is at or beyond
  // half_pi().right(). IBEX takes the double below pi/2 for a pole as well (its
  // bwd_atan07 and 08 expect only a bound above 1000: the exact one is the tangent
  // of that double, 1.6e16). IBEX runs bwd_atan01 to 06 on -y and -x too
  //---------------------------------------------------------------------------

  struct ibexb_atan_case
  {
    const char *name;
    Ref y, x, result;
    bool mirror;
    int doubles;
  };

  // BEGIN ibexb_atan_cases (written by ibexb_cases.py: do not edit)
  static const ibexb_atan_case ibexb_atan_cases[] = {
    { "IBEX TestArith::bwd_atan01", { 0x0p+0, 0x1.0c152382d7366p-1 }, { -0x1p+0, 0x1.8p+1 },
      { 0x0p+0, 0x1.279a74590331dp-1 }, true, limit },
    { "IBEX TestArith::bwd_atan02", { -0x1.921fb54442d18p+1, 0x1.8p+0 }, { 0x0p+0, 0x1.f6a7a2955385ep+2 },
      { 0x0p+0, 0x1.f6a7a2955385ep+2 }, true, 0 },
    { "IBEX TestArith::bwd_atan03", { 0x0p+0, 0x1.0c152382d7366p-1 }, { 0x1.999999999999ap-3, 0x1p-1 },
      { 0x1.999999999999ap-3, 0x1p-1 }, true, 0 },
    { "IBEX TestArith::bwd_atan04", { -0x1.abb94edddc6b1p+0, 0x1.abb94edddc6b2p+0 }, { -0x1.9p+6, 0x1.9p+6 },
      { -0x1.9p+6, 0x1.9p+6 }, true, 0 },
    { "IBEX TestArith::bwd_atan05", { 0x1.abb94edddc6b2p+0, 0x1.921fb54442d18p+1 }, { -0x1.9p+6, 0x1.9p+6 },
      ibexb_empty, true, 0 },
    { "IBEX TestArith::bwd_atan06", { -0x1.921fb54442d18p+1, -0x1.abb94edddc6b1p+0 }, { -0x1.9p+6, 0x1.9p+6 },
      ibexb_empty, true, 0 },
    { "IBEX TestArith::bwd_atan07", { -0x1.921fb54442d18p-1, 0x1.921fb54442d18p+0 }, { -ibexb_oo, ibexb_oo },
      { -0x1p+0, 0x1.d02967c31cdb5p+53 }, false, limit },
    { "IBEX TestArith::bwd_atan08", { -0x1.921fb54442d18p+0, 0x1.921fb54442d18p-1 }, { -ibexb_oo, ibexb_oo },
      { -0x1.d02967c31cdb5p+53, 0x1p+0 }, false, limit },
  };
  // END ibexb_atan_cases

  interval ibexb_bwd_atan(const interval& y, const interval& x)
  {
    const double beyond = interval::half_pi().right();
    if (y.is_empty() || y.right() <= -beyond || y.left() >= beyond) {
      return interval::emptyset();
    }
    const double lo = y.left() <= -beyond ? -ibexb_oo : tan(interval(y.left())).left();
    const double hi = y.right() >= beyond ? ibexb_oo : tan(interval(y.right())).right();
    return x & interval(lo, hi);
  }

  void ibexb_atans()
  {
    for (const ibexb_atan_case& c : ibexb_atan_cases) {
      const interval y = ibexb_interval(c.y), x = ibexb_interval(c.x);
      const auto describe = [&] { return "y = " + show(y) + ", x = " + show(x); };
      const interval r = evaluate(c.name, [&] { return ibexb_bwd_atan(y, x); }, describe);
      expect(ibexb_label(c.name, "bwd_atan(y,x)"), r, c.result, describe, c.doubles);
      if (c.mirror) {
        const interval m = evaluate(c.name, [&] { return ibexb_bwd_atan(-y, -x); }, describe);
        expect(ibexb_label(c.name, "bwd_atan(-y,-x)"), m, ibexb_opposite(c.result), describe, c.doubles);
      }
    }
  }

  //---------------------------------------------------------------------------
  // TestArith: bwd_imod_01 to bwd_imod_08: x = y mod p, that is x - y = k p for an
  // integer k. IBEX computes the integers of (x - y) / p, and contracts x and y with
  // each in turn (the hulls of the contractions): the same computation, written
  // here for any number of integers (IBEX stops at two, and where x or y is wider
  // than p: none of the eight tests); the multiples k p are exact for the periods
  // of the tests, 2 * M_PI being a double
  //---------------------------------------------------------------------------

  struct ibexb_imod_case
  {
    const char *name;
    double period;
    Ref x, y, x_after, y_after;
  };

  // BEGIN ibexb_imod_cases (written by ibexb_cases.py: do not edit)
  static const ibexb_imod_case ibexb_imod_cases[] = {
    { "IBEX TestArith::bwd_imod_01", 0x1.8p+1, { 0x1.8p+1, 0x1.4p+2 }, { 0x1p+0, 0x1p+1 }, { 0x1p+2, 0x1.4p+2 },
      { 0x1p+0, 0x1p+1 } },
    { "IBEX TestArith::bwd_imod_02", 0x1p+1, { 0x1.cp+2, 0x1p+3 }, { 0x1p-1, 0x1p+1 }, { 0x1.cp+2, 0x1p+3 },
      { 0x1p+0, 0x1p+1 } },
    { "IBEX TestArith::bwd_imod_03", 0x1p+1, { 0x1.cp+2, 0x1p+3 }, { 0x0p+0, 0x1p+1 }, { 0x1.cp+2, 0x1p+3 },
      { 0x0p+0, 0x1p+1 } },
    { "IBEX TestArith::bwd_imod_04", 0x1.921fb54442d18p+2, { 0x1.921fb54442d18p+2, 0x1.2d97c7f3321d2p+3 },
      { 0x1.0c152382d7366p-1, 0x1.921fb54442d18p+0 }, { 0x1.b3a259b49db84p+2, 0x1.f6a7a2955385ep+2 },
      { 0x1.0c152382d7366p-1, 0x1.921fb54442d18p+0 } },
    { "IBEX TestArith::bwd_imod_05", 0x1.921fb54442d18p+2, { 0x1.2d97c7f3321d2p+3, 0x1.921fb54442d18p+3 },
      { 0x1.0c152382d7366p+0, 0x1.921fb54442d18p+0 }, ibexb_empty, ibexb_empty },
    { "IBEX TestArith::bwd_imod_06", 0x1.921fb54442d18p+2, { 0x1.2d97c7f3321d2p+3, 0x1.921fb54442d18p+3 },
      { 0x0p+0, 0x1.921fb54442d18p+0 }, { 0x1.921fb54442d18p+3, 0x1.921fb54442d18p+3 }, { 0x0p+0, 0x0p+0 } },
    { "IBEX TestArith::bwd_imod_07", 0x1.921fb54442d18p+2, { 0x1.921fb54442d18p+2, 0x1.921fb54442d18p+3 },
      { -0x1.0c152382d7365p-1, 0x1.921fb54442d18p+0 }, { 0x1.921fb54442d18p+2, 0x1.921fb54442d18p+3 },
      { -0x1.0c152382d7365p-1, 0x1.921fb54442d18p+0 } },
    { "IBEX TestArith::bwd_imod_08", 0x1.921fb54442d18p+2, { 0x1.5fdbbe9bba775p+2, 0x1.0c152382d7366p+3 },
      { -0x1.921fb54442d18p+0, 0x1.921fb54442d18p+0 }, { 0x1.5fdbbe9bba775p+2, 0x1.f6a7a2955385ep+2 },
      { -0x1.921fb54442d18p-1, 0x1.921fb54442d18p+0 } },
  };
  // END ibexb_imod_cases

  void ibexb_bwd_imod(interval& x, interval& y, double period)
  {
    const interval k = integer((x - y) / period);
    interval x_hull = interval::emptyset(), y_hull = interval::emptyset();
    if (!k.is_empty()) {
      // IBEX gives up beyond two multiples of the period: x and y are left as they are
      if (!k.is_finite() || k.right() - k.left() > 2.0) {
        return;
      }
      for (double n = k.left(); n <= k.right(); n += 1.0) {
        const interval multiple = interval(n) * period;
        x_hull |= x & (y + multiple);
        y_hull |= y & (x - multiple);
      }
    }
    x = x_hull;
    y = y_hull;
  }

  void ibexb_imods()
  {
    for (const ibexb_imod_case& c : ibexb_imod_cases) {
      const interval X = ibexb_interval(c.x), Y = ibexb_interval(c.y);
      const auto describe = [&] { return "x = " + show(X) + ", y = " + show(Y) + ", period = " + hex(c.period); };
      interval x = X, y = Y;
      ibexb_bwd_imod(x, y, c.period);
      expect(ibexb_label(c.name, "x"), x, c.x_after, describe);
      expect(ibexb_label(c.name, "y"), y, c.y_after, describe);
    }
  }

  void ibexb_run()
  {
    ibexb_distances();
    ibexb_relative_distances();
    for (const ibexb_div2_case& c : ibexb_div2_cases) {
      ibexb_div2(c);
    }
    ibexb_binaries();
    ibexb_maxmins();
    ibexb_projections();
    ibexb_trigs();
    ibexb_pows();
    ibexb_atan2s();
    ibexb_atans();
    ibexb_imods();
  }
}

/*
  The interval tests of Codac (https://github.com/codac-team/codac), a wrapper
  of gaol::interval, written for GAOL v5 itself: the cases of
  codac2_tests_Interval.cpp, codac2_tests_Interval_operations.cpp and
  codac2_tests_Interval_rounding.cpp (Codac 2, commit 6419940, and the branch
  GAOL of the fork of Jordan Ninin, commit f0d06ff0), with the functions of
  GAOL that the wrapper calls, or that do what the wrapper adds to them:
  nth_root for root, width for diam, midpoint for mid, rad, is_bisectable,
  bisect, inflate, the relations set_* and certainly_*, and the unary minus
  that the wrapper writes 0 - x.

  A case is a row of a table, named after the file and the line of Codac it
  comes from ("Codac Interval_operations:176"), with the operands and the
  tightest bounds of the exact result of the operation: lo is the greatest
  double not above the exact lower bound, hi the least double not below the
  exact upper bound (-oo or +oo beyond the doubles), and none is the empty set.
  tests/tools/ibex_codac/codac_cases.py wrote the rows. It computes the exact
  results apart from GAOL, with rational numbers and mpmath on 2000 bits,
  compares what Codac expects with them (it prints every disagreement), and
  checks that the rows below are the ones the cases give (--check). GAOL's
  result has to enclose the exact one, and to be as close to it as the
  operation can be: the tightest bounds for the arithmetic, the square root,
  the sets and the integer functions, within one double for the elementary
  functions (limit). A function with variants runs each of them, as Codac
  does: x+y, y+x, x+=y...

  When Codac gives a function an argument it computes with interval::pi() (sin
  of pi - x, tan of 3 pi / 4 | 5 pi / 4), the exact result is the hull of the
  function over the interval of doubles that argument is: the tightest
  operations on the two doubles around pi, which is what GAOL's operations give
  (the cases of the arithmetic show that they are the tightest). It encloses the
  hull over the real argument, wider by what the width of pi's enclosure adds,
  and the tool checks Codac's expected value, which Codac compares to within
  1e-9, against that exact hull over the real argument.

  The values that are no result of the real operations but conventions are the
  ones Codac expects, as GAOL documents them: the midpoint of an unbounded
  interval (0, DBL_MAX, -DBL_MAX), the point at which bisect() cuts it, the
  empty set that the constructors give for an infinite argument and for bounds
  in the wrong order, and [-oo, +oo] for no argument.

  What is not here, because it is Codac's own: the constructors from an
  initializer list, the literal _i, init_from_list(), set_empty(), smag(),
  rand(), diff(), the ternary chi(x,y,z), the block under #if 0 and the Python
  tests; and the two comparisons of doubles (oo > MAX). is_degenerated() of the
  empty set (true for Codac) is is_empty() || is_a_double() here, as in the
  IBEX section: GAOL's is_a_double() alone is false for the empty set, as
  tests/relations.cpp checks.
  pow == sqrt of the same empty set is the checks that both are empty.
*/

namespace
{
  constexpr double codac_oo = std::numeric_limits<double>::infinity();
  constexpr Ref codac_none = { std::numeric_limits<double>::quiet_NaN(), std::numeric_limits<double>::quiet_NaN() };

  // The interval of a row, the empty set for none
  interval codac_interval(const Ref& r)
  {
    return is_none(r) ? interval::emptyset() : interval(r.lo, r.hi);
  }

  // The opposite of a reference
  Ref codac_negated(const Ref& r)
  {
    return is_none(r) ? r : Ref{ -r.hi, -r.lo };
  }

  // The tightest bounds of the result of an operation on two intervals, and its variants
  struct codac_Binary
  {
    const char* name;
    Ref x, y, ref;
  };

  // The same with a double for the second operand
  struct codac_Scalar
  {
    const char* name;
    Ref x;
    double y;
    Ref ref;
  };

  //---------------------------------------------------------------------------
  // The constructors: IBEX's rules, [-oo, +oo] for no argument, the empty set for
  // an infinite argument and for the bounds in the wrong order
  //---------------------------------------------------------------------------
  struct codac_Constructor
  {
    const char* name;
    int arguments;
    double a, b;
    Ref ref;
  };

  const codac_Constructor codac_constructors[] = {
    // BEGIN constructors
    { "Codac Interval:26", 0, 0x0.0p+0, 0x0.0p+0, { -codac_oo, codac_oo } },
    { "Codac Interval:27", 1, 0x1.0000000000000p+0, 0x0.0p+0, { 0x1.0000000000000p+0, 0x1.0000000000000p+0 } },
    { "Codac Interval:28", 1, -codac_oo, 0x0.0p+0, codac_none },
    { "Codac Interval:29", 1, codac_oo, 0x0.0p+0, codac_none },
    { "Codac Interval:30", 2, 0x1.0000000000000p+0, 0x0.0p+0, codac_none },
    { "Codac Interval:216", 1, codac_oo, 0x0.0p+0, codac_none },
    // END constructors
  };

  void codac_constructors_run()
  {
    for (const codac_Constructor& c : codac_constructors) {
      const interval r = (c.arguments == 0) ? interval() : ((c.arguments == 1) ? interval(c.a) : interval(c.a, c.b));
      expect(c.name, r, c.ref, [&] { return "interval(" + hex(c.a) + ", " + hex(c.b) + ") with " + std::to_string(c.arguments) + " arguments"; });
    }
  }

  //---------------------------------------------------------------------------
  // CHECK_add, CHECK_add_scalar, CHECK_mul, CHECK_mul_scalar, CHECK_div and
  // CHECK_div_scalar of Codac: x op y, y op x, x op= y, and for the sum the
  // difference of the opposites, with the operands as intervals or a double.
  // Exact: the tightest bounds. A double that is infinite is no real number: the
  // operation has no value, and the result is the empty set
  //---------------------------------------------------------------------------
  const codac_Binary codac_additions[] = {
    // BEGIN additions
    { "Codac Interval_operations:109", codac_none, { 0x0.0p+0, 0x1.0000000000000p+0 }, codac_none },
    { "Codac Interval_operations:110", { 0x0.0p+0, 0x1.0000000000000p+0 }, codac_none, codac_none },
    { "Codac Interval_operations:111", { -codac_oo, 0x1.0000000000000p+0 }, { 0x0.0p+0, 0x1.0000000000000p+0 }, { -codac_oo, 0x1.0000000000000p+1 } },
    { "Codac Interval_operations:112", { 0x1.0000000000000p+0, codac_oo }, { 0x0.0p+0, 0x1.0000000000000p+0 }, { 0x1.0000000000000p+0, codac_oo } },
    { "Codac Interval_operations:113", { -codac_oo, codac_oo }, { 0x0.0p+0, 0x1.0000000000000p+0 }, { -codac_oo, codac_oo } },
    { "Codac Interval_operations:114", { 0x1.fffffffffffffp+1023, codac_oo }, { 0x1.0000000000000p+0, 0x1.0000000000000p+0 }, { 0x1.fffffffffffffp+1023, codac_oo } },
    { "Codac Interval_operations:115", { 0x1.fffffffffffffp+1023, codac_oo }, { -0x1.0000000000000p+0, -0x1.0000000000000p+0 }, { 0x1.ffffffffffffep+1023, codac_oo } },
    { "Codac Interval_operations:116", { 0x1.fffffffffffffp+1023, codac_oo }, { 0x1.fffffffffffffp+1023, codac_oo }, { 0x1.fffffffffffffp+1023, codac_oo } },
    { "Codac Interval_operations:117", { 0x1.fffffffffffffp+1023, codac_oo }, codac_none, codac_none },
    { "Codac Interval_operations:118", { 0x1.fffffffffffffp+1023, codac_oo }, codac_none, codac_none },
    { "Codac Interval_operations:119", { -codac_oo, -0x1.fffffffffffffp+1023 }, { 0x1.0000000000000p+0, 0x1.0000000000000p+0 }, { -codac_oo, -0x1.ffffffffffffep+1023 } },
    { "Codac Interval_operations:120", { -codac_oo, -0x1.fffffffffffffp+1023 }, { -0x1.0000000000000p+0, -0x1.0000000000000p+0 }, { -codac_oo, -0x1.fffffffffffffp+1023 } },
    { "Codac Interval_operations:121", { -codac_oo, -0x1.fffffffffffffp+1023 }, { -codac_oo, -0x1.fffffffffffffp+1023 }, { -codac_oo, -0x1.fffffffffffffp+1023 } },
    { "Codac Interval:270", { 0x1.8000000000000p+1, 0x1.0000000000000p+3 }, codac_none, codac_none },
    // END additions
  };

  const codac_Scalar codac_scalar_additions[] = {
    // BEGIN scalar_additions
    { "Codac Interval_operations:123", codac_none, codac_oo, codac_none },
    { "Codac Interval_operations:124", codac_none, 0x0.0p+0, codac_none },
    { "Codac Interval_operations:125", { 0x0.0p+0, 0x1.0000000000000p+0 }, 0x1.0000000000000p+0, { 0x1.0000000000000p+0, 0x1.0000000000000p+1 } },
    { "Codac Interval_operations:126", { 0x0.0p+0, 0x1.0000000000000p+0 }, -codac_oo, codac_none },
    { "Codac Interval_operations:127", { 0x0.0p+0, 0x1.0000000000000p+0 }, codac_oo, codac_none },
    { "Codac Interval_operations:128", { -codac_oo, 0x1.0000000000000p+0 }, 0x1.0000000000000p+0, { -codac_oo, 0x1.0000000000000p+1 } },
    // END scalar_additions
  };

  const codac_Binary codac_products[] = {
    // BEGIN products
    { "Codac Interval_operations:130", codac_none, { 0x0.0p+0, 0x1.0000000000000p+0 }, codac_none },
    { "Codac Interval_operations:131", { 0x0.0p+0, 0x0.0p+0 }, { -codac_oo, codac_oo }, { 0x0.0p+0, 0x0.0p+0 } },
    { "Codac Interval_operations:132", { -0x1.0000000000000p+0, 0x1.0000000000000p+0 }, { -codac_oo, 0x0.0p+0 }, { -codac_oo, codac_oo } },
    { "Codac Interval_operations:133", { -codac_oo, -0x1.0000000000000p+0 }, { -0x1.0000000000000p+0, 0x0.0p+0 }, { 0x0.0p+0, codac_oo } },
    { "Codac Interval_operations:134", { -codac_oo, 0x1.0000000000000p+0 }, { -0x1.0000000000000p+0, 0x0.0p+0 }, { -0x1.0000000000000p+0, codac_oo } },
    { "Codac Interval_operations:135", { 0x0.0p+0, 0x1.0000000000000p+0 }, { 0x1.0000000000000p+0, codac_oo }, { 0x0.0p+0, codac_oo } },
    { "Codac Interval_operations:136", { 0x0.0p+0, 0x1.0000000000000p+0 }, { -0x1.0000000000000p+0, codac_oo }, { -0x1.0000000000000p+0, codac_oo } },
    { "Codac Interval_operations:137", { -codac_oo, -0x1.0000000000000p+0 }, { 0x0.0p+0, 0x1.0000000000000p+0 }, { -codac_oo, 0x0.0p+0 } },
    { "Codac Interval_operations:138", { -codac_oo, 0x1.0000000000000p+0 }, { 0x0.0p+0, 0x1.0000000000000p+0 }, { -codac_oo, 0x1.0000000000000p+0 } },
    { "Codac Interval_operations:139", { 0x0.0p+0, 0x1.0000000000000p+0 }, { -codac_oo, -0x1.0000000000000p+0 }, { -codac_oo, 0x0.0p+0 } },
    { "Codac Interval_operations:140", { 0x0.0p+0, 0x1.0000000000000p+0 }, { -codac_oo, 0x1.0000000000000p+0 }, { -codac_oo, 0x1.0000000000000p+0 } },
    { "Codac Interval_operations:141", { 0x1.0000000000000p+0, codac_oo }, { 0x0.0p+0, 0x1.0000000000000p+0 }, { 0x0.0p+0, codac_oo } },
    { "Codac Interval_operations:142", { -0x1.0000000000000p+0, codac_oo }, { 0x0.0p+0, 0x1.0000000000000p+0 }, { -0x1.0000000000000p+0, codac_oo } },
    { "Codac Interval_operations:143", { 0x1.0000000000000p+0, 0x1.0000000000000p+1 }, { 0x1.0000000000000p+0, 0x1.0000000000000p+1 }, { 0x1.0000000000000p+0, 0x1.0000000000000p+2 } },
    { "Codac Interval_operations:144", { 0x1.0000000000000p+0, 0x1.0000000000000p+1 }, { -0x1.0000000000000p+1, 0x1.8000000000000p+1 }, { -0x1.0000000000000p+2, 0x1.8000000000000p+2 } },
    { "Codac Interval_operations:145", { -0x1.0000000000000p+0, 0x1.0000000000000p+0 }, { -0x1.0000000000000p+1, 0x1.8000000000000p+1 }, { -0x1.8000000000000p+1, 0x1.8000000000000p+1 } },
    // END products
  };

  const codac_Scalar codac_scalar_products[] = {
    // BEGIN scalar_products
    { "Codac Interval_operations:146", { 0x1.0000000000000p+0, 0x1.0000000000000p+1 }, -codac_oo, codac_none },
    { "Codac Interval_operations:147", { 0x1.0000000000000p+0, 0x1.0000000000000p+1 }, codac_oo, codac_none },
    { "Codac Interval_operations:148", { 0x1.0000000000000p+0, 0x1.0000000000000p+1 }, -0x1.0000000000000p+0, { -0x1.0000000000000p+1, -0x1.0000000000000p+0 } },
    // END scalar_products
  };

  const codac_Binary codac_quotients[] = {
    // BEGIN quotients
    { "Codac Interval_operations:150", codac_none, { 0x0.0p+0, 0x1.0000000000000p+0 }, codac_none },
    { "Codac Interval_operations:151", { 0x0.0p+0, 0x0.0p+0 }, { 0x0.0p+0, 0x0.0p+0 }, codac_none },
    { "Codac Interval_operations:152", { 0x1.0000000000000p+0, 0x1.0000000000000p+1 }, { 0x0.0p+0, 0x0.0p+0 }, codac_none },
    { "Codac Interval_operations:153", { -codac_oo, codac_oo }, { 0x0.0p+0, 0x0.0p+0 }, codac_none },
    { "Codac Interval_operations:154", { 0x0.0p+0, 0x0.0p+0 }, { 0x0.0p+0, 0x1.0000000000000p+0 }, { 0x0.0p+0, 0x0.0p+0 } },
    { "Codac Interval_operations:155", { 0x0.0p+0, 0x0.0p+0 }, { -codac_oo, codac_oo }, { 0x0.0p+0, 0x0.0p+0 } },
    { "Codac Interval_operations:156", { 0x1.8000000000000p+2, 0x1.8000000000000p+3 }, { 0x1.0000000000000p+1, 0x1.8000000000000p+1 }, { 0x1.0000000000000p+1, 0x1.8000000000000p+2 } },
    { "Codac Interval_operations:157", { 0x1.8000000000000p+2, 0x1.8000000000000p+3 }, { -0x1.8000000000000p+1, -0x1.0000000000000p+1 }, { -0x1.8000000000000p+2, -0x1.0000000000000p+1 } },
    { "Codac Interval_operations:158", { 0x1.8000000000000p+2, 0x1.8000000000000p+3 }, { -0x1.0000000000000p+1, 0x1.8000000000000p+1 }, { -codac_oo, codac_oo } },
    { "Codac Interval_operations:159", { -codac_oo, -0x1.0000000000000p+0 }, { -0x1.0000000000000p+0, 0x0.0p+0 }, { 0x1.0000000000000p+0, codac_oo } },
    { "Codac Interval_operations:160", { -codac_oo, -0x1.0000000000000p+0 }, { 0x0.0p+0, 0x1.0000000000000p+0 }, { -codac_oo, -0x1.0000000000000p+0 } },
    { "Codac Interval_operations:161", { 0x1.0000000000000p+0, codac_oo }, { -0x1.0000000000000p+0, 0x0.0p+0 }, { -codac_oo, -0x1.0000000000000p+0 } },
    { "Codac Interval_operations:162", { 0x1.0000000000000p+0, codac_oo }, { 0x0.0p+0, 0x1.0000000000000p+0 }, { 0x1.0000000000000p+0, codac_oo } },
    { "Codac Interval_operations:163", { -0x1.0000000000000p+0, 0x1.0000000000000p+0 }, { -0x1.0000000000000p+0, 0x1.0000000000000p+0 }, { -codac_oo, codac_oo } },
    // END quotients
  };

  const codac_Scalar codac_scalar_quotients[] = {
    // BEGIN scalar_quotients
    { "Codac Interval_operations:164", { 0x1.0000000000000p+0, 0x1.0000000000000p+1 }, -codac_oo, codac_none },
    { "Codac Interval_operations:165", { 0x1.0000000000000p+0, 0x1.0000000000000p+1 }, codac_oo, codac_none },
    { "Codac Interval_operations:166", { 0x1.0000000000000p+0, 0x1.0000000000000p+1 }, -0x1.0000000000000p+0, { -0x1.0000000000000p+1, -0x1.0000000000000p+0 } },
    // END scalar_quotients
  };

  void codac_arithmetic_run()
  {
    for (const codac_Binary& c : codac_additions) {
      const interval x = codac_interval(c.x), y = codac_interval(c.y);
      const Ref minus = codac_negated(c.ref);
      const std::string n = c.name;
      const auto describe = [&] { return "x = " + show(x) + ", y = " + show(y); };
      expect(n + " x+y", x + y, c.ref, describe);
      expect(n + " y+x", y + x, c.ref, describe);
      { interval r(x); r += y; expect(n + " x+=y", r, c.ref, describe); }
      { interval r(y); r += x; expect(n + " y+=x", r, c.ref, describe); }
      expect(n + " (-x)-y", (-x) - y, minus, describe);
      expect(n + " (-y)-x", (-y) - x, minus, describe);
      { interval r(-x); r -= y; expect(n + " (-x)-=y", r, minus, describe); }
      { interval r(-y); r -= x; expect(n + " (-y)-=x", r, minus, describe); }
    }
    for (const codac_Scalar& c : codac_scalar_additions) {
      const interval x = codac_interval(c.x);
      const double y = c.y;
      const Ref minus = codac_negated(c.ref);
      const std::string n = c.name;
      const auto describe = [&] { return "x = " + show(x) + ", y = " + hex(y); };
      expect(n + " x+y", x + y, c.ref, describe);
      expect(n + " y+x", y + x, c.ref, describe);
      { interval r(x); r += y; expect(n + " x+=y", r, c.ref, describe); }
      expect(n + " (-x)-y", (-x) - y, minus, describe);
      expect(n + " (-y)-x", (-y) - x, minus, describe);
      { interval r(-x); r -= y; expect(n + " (-x)-=y", r, minus, describe); }
    }
    for (const codac_Binary& c : codac_products) {
      const interval x = codac_interval(c.x), y = codac_interval(c.y);
      const std::string n = c.name;
      const auto describe = [&] { return "x = " + show(x) + ", y = " + show(y); };
      expect(n + " x*y", x * y, c.ref, describe);
      expect(n + " y*x", y * x, c.ref, describe);
      { interval r(x); r *= y; expect(n + " x*=y", r, c.ref, describe); }
    }
    for (const codac_Scalar& c : codac_scalar_products) {
      const interval x = codac_interval(c.x);
      const double y = c.y;
      const std::string n = c.name;
      const auto describe = [&] { return "x = " + show(x) + ", y = " + hex(y); };
      expect(n + " x*y", x * y, c.ref, describe);
      expect(n + " y*x", y * x, c.ref, describe);
      { interval r(x); r *= y; expect(n + " x*=y", r, c.ref, describe); }
    }
    for (const codac_Binary& c : codac_quotients) {
      const interval x = codac_interval(c.x), y = codac_interval(c.y);
      const std::string n = c.name;
      const auto describe = [&] { return "x = " + show(x) + ", y = " + show(y); };
      expect(n + " x/y", x / y, c.ref, describe);
      { interval r(x); r /= y; expect(n + " x/=y", r, c.ref, describe); }
    }
    for (const codac_Scalar& c : codac_scalar_quotients) {
      const interval x = codac_interval(c.x);
      const double y = c.y;
      const std::string n = c.name;
      const auto describe = [&] { return "x = " + show(x) + ", y = " + hex(y); };
      expect(n + " x/y", x / y, c.ref, describe);
      { interval r(x); r /= y; expect(n + " x/=y", r, c.ref, describe); }
    }
  }

  //---------------------------------------------------------------------------
  // The hull | and the intersection &, with |=, &= and both orders. Exact
  //---------------------------------------------------------------------------
  const codac_Binary codac_hulls[] = {
    // BEGIN hulls
    { "Codac Interval:77", { 0x0.0p+0, 0x1.0000000000000p+0 }, { 0x1.0000000000000p+1, 0x1.8000000000000p+1 }, { 0x0.0p+0, 0x1.8000000000000p+1 } },
    { "Codac Interval:81", { 0x0.0p+0, 0x1.0000000000000p+0 }, codac_none, { 0x0.0p+0, 0x1.0000000000000p+0 } },
    { "Codac Interval:85", { -0x1.0000000000000p+0, -0x1.0000000000000p+0 }, { 0x0.0p+0, codac_oo }, { -0x1.0000000000000p+0, codac_oo } },
    // END hulls
  };

  const codac_Binary codac_intersections[] = {
    // BEGIN intersections
    { "Codac Interval:89", { 0x0.0p+0, 0x1.0000000000000p+0 }, { 0x1.0000000000000p+1, 0x1.8000000000000p+1 }, codac_none },
    { "Codac Interval:93", { 0x0.0p+0, 0x1.0000000000000p+0 }, codac_none, codac_none },
    { "Codac Interval:97", { 0x0.0p+0, 0x1.0000000000000p+1 }, { -0x1.0000000000000p+0, 0x1.0000000000000p+0 }, { 0x0.0p+0, 0x1.0000000000000p+0 } },
    { "Codac Interval:101", { 0x0.0p+0, 0x1.0000000000000p+0 }, { 0x0.0p+0, codac_oo }, { 0x0.0p+0, 0x1.0000000000000p+0 } },
    { "Codac Interval:105", { 0x0.0p+0, 0x1.0000000000000p+0 }, { -codac_oo, 0x0.0p+0 }, { 0x0.0p+0, 0x0.0p+0 } },
    { "Codac Interval:109", { 0x0.0p+0, 0x1.0000000000000p+0 }, { 0x1.0000000000000p+0, 0x1.0000000000000p+1 }, { 0x1.0000000000000p+0, 0x1.0000000000000p+0 } },
    { "Codac Interval:269", { 0x1.0000000000000p+0, 0x1.8000000000000p+1 }, { 0x1.8000000000000p+2, 0x1.c000000000000p+2 }, codac_none },
    // END intersections
  };

  void codac_sets_run()
  {
    for (const codac_Binary& c : codac_hulls) {
      const interval x = codac_interval(c.x), y = codac_interval(c.y);
      const std::string n = c.name;
      const auto describe = [&] { return "x = " + show(x) + ", y = " + show(y); };
      expect(n + " x|y", x | y, c.ref, describe);
      expect(n + " y|x", y | x, c.ref, describe);
      { interval r(x); r |= y; expect(n + " x|=y", r, c.ref, describe); }
      { interval r(y); r |= x; expect(n + " y|=x", r, c.ref, describe); }
    }
    for (const codac_Binary& c : codac_intersections) {
      const interval x = codac_interval(c.x), y = codac_interval(c.y);
      const std::string n = c.name;
      const auto describe = [&] { return "x = " + show(x) + ", y = " + show(y); };
      expect(n + " x&y", x & y, c.ref, describe);
      expect(n + " y&x", y & x, c.ref, describe);
      { interval r(x); r &= y; expect(n + " x&=y", r, c.ref, describe); }
      { interval r(y); r &= x; expect(n + " y&=x", r, c.ref, describe); }
    }
  }

  //---------------------------------------------------------------------------
  // The parts of an interval: left(), right(), midpoint() for mid, rad(),
  // width() for diam, mag(), mig(), smig(). The reference of each is the two
  // doubles around the exact value: the middle of [a, b] is (a + b) / 2, its
  // radius (b - a) / 2 (the midpoint of these intervals being a double), its
  // width b - a, and the magnitude, the mignitude and the signed mignitude
  // as their definitions say. The midpoint of [-oo, +oo] is 0, that of
  // [a, +oo] and [-oo, b] the largest double and its opposite; the radius and
  // the width of an unbounded interval are +oo
  //---------------------------------------------------------------------------
  struct codac_Accessors
  {
    const char* name;
    Ref x;
    Ref ref[6]; // midpoint, rad, width, mag, mig, smig
  };

  const codac_Accessors codac_accessors[] = {
    // BEGIN accessors
    { "Codac Interval:113", { 0x0.0p+0, 0x1.0000000000000p+1 }, { { 0x1.0000000000000p+0, 0x1.0000000000000p+0 }, { 0x1.0000000000000p+0, 0x1.0000000000000p+0 }, { 0x1.0000000000000p+1, 0x1.0000000000000p+1 }, { 0x1.0000000000000p+1, 0x1.0000000000000p+1 }, { 0x0.0p+0, 0x0.0p+0 }, { 0x0.0p+0, 0x0.0p+0 } } },
    { "Codac Interval:125", { -0x1.8000000000000p+1, -0x1.0000000000000p+0 }, { { -0x1.0000000000000p+1, -0x1.0000000000000p+1 }, { 0x1.0000000000000p+0, 0x1.0000000000000p+0 }, { 0x1.0000000000000p+1, 0x1.0000000000000p+1 }, { 0x1.8000000000000p+1, 0x1.8000000000000p+1 }, { 0x1.0000000000000p+0, 0x1.0000000000000p+0 }, { -0x1.0000000000000p+0, -0x1.0000000000000p+0 } } },
    { "Codac Interval:137", { -0x1.8000000000000p+1, 0x1.0000000000000p+0 }, { { -0x1.0000000000000p+0, -0x1.0000000000000p+0 }, { 0x1.0000000000000p+1, 0x1.0000000000000p+1 }, { 0x1.0000000000000p+2, 0x1.0000000000000p+2 }, { 0x1.8000000000000p+1, 0x1.8000000000000p+1 }, { 0x0.0p+0, 0x0.0p+0 }, { 0x0.0p+0, 0x0.0p+0 } } },
    { "Codac Interval:149", { -codac_oo, 0x0.0p+0 }, { { -0x1.fffffffffffffp+1023, -0x1.fffffffffffffp+1023 }, { codac_oo, codac_oo }, { codac_oo, codac_oo }, { codac_oo, codac_oo }, { 0x0.0p+0, 0x0.0p+0 }, { 0x0.0p+0, 0x0.0p+0 } } },
    { "Codac Interval:161", { -codac_oo, codac_oo }, { { 0x0.0p+0, 0x0.0p+0 }, { codac_oo, codac_oo }, { codac_oo, codac_oo }, { codac_oo, codac_oo }, { 0x0.0p+0, 0x0.0p+0 }, { 0x0.0p+0, 0x0.0p+0 } } },
    { "Codac Interval:173", { 0x1.fffffffffffffp+1023, codac_oo }, { { 0x1.fffffffffffffp+1023, 0x1.fffffffffffffp+1023 }, { codac_oo, codac_oo }, { codac_oo, codac_oo }, { codac_oo, codac_oo }, { 0x1.fffffffffffffp+1023, 0x1.fffffffffffffp+1023 }, { 0x1.fffffffffffffp+1023, 0x1.fffffffffffffp+1023 } } },
    // END accessors
  };

  void codac_accessors_run()
  {
    for (const codac_Accessors& c : codac_accessors) {
      const interval x = codac_interval(c.x);
      const std::string n = c.name;
      const auto describe = [&] { return "x = " + show(x); };
      expect(n + " left()", x.left(), Ref{ c.x.lo, c.x.lo }, describe);
      expect(n + " right()", x.right(), Ref{ c.x.hi, c.x.hi }, describe);
      expect(n + " midpoint()", x.midpoint(), c.ref[0], describe);
      expect(n + " rad()", x.rad(), c.ref[1], describe);
      expect(n + " width()", x.width(), c.ref[2], describe);
      expect(n + " mag()", x.mag(), c.ref[3], describe);
      expect(n + " mig()", x.mig(), c.ref[4], describe);
      expect(n + " smig()", x.smig(), c.ref[5], describe);
    }
  }

  //---------------------------------------------------------------------------
  // The relations between sets, by their definitions: x = y, x != y, x is a
  // subset of y (set_leq), a superset (set_geq), in the interior of y and
  // different (set_le and set_neq: is_strict_interior_subset of Codac), a
  // superset and different (set_geq and set_neq: is_strict_superset), and
  // disjoint; the empty set is in every set, and disjoint from every one
  //---------------------------------------------------------------------------
  struct codac_Relation
  {
    const char* name;
    const char* relation;
    Ref x, y;
    bool expected;
  };

  const codac_Relation codac_relations[] = {
    // BEGIN relations
    { "Codac Interval:34", "set_eq", { -codac_oo, codac_oo }, { -codac_oo, codac_oo }, true },
    { "Codac Interval:35", "set_neq", { -codac_oo, codac_oo }, { 0x0.0p+0, codac_oo }, true },
    { "Codac Interval:36", "set_eq", { 0x0.0p+0, 0x1.0000000000000p+0 }, { 0x0.0p+0, 0x1.0000000000000p+0 }, true },
    { "Codac Interval:37", "set_neq", { 0x0.0p+0, 0x1.0000000000000p+1 }, { 0x0.0p+0, 0x1.0000000000000p+0 }, true },
    { "Codac Interval:38", "set_neq", { 0x0.0p+0, 0x1.0000000000000p+1 }, { 0x1.0000000000000p+0, 0x1.8000000000000p+1 }, true },
    { "Codac Interval:39", "set_eq", { 0x0.0p+0, codac_oo }, { 0x0.0p+0, codac_oo }, true },
    { "Codac Interval:40", "set_eq", codac_none, codac_none, true },
    { "Codac Interval:41", "set_eq", { 0x0.0p+0, 0x0.0p+0 }, { 0x0.0p+0, 0x0.0p+0 }, true },
    { "Codac Interval:200", "set_leq", { 0x0.0p+0, 0x1.0000000000000p+0 }, { 0x0.0p+0, 0x1.0000000000000p+1 }, true },
    { "Codac Interval:201", "set_leq", { 0x0.0p+0, 0x1.0000000000000p+0 }, { 0x1.0000000000000p+0, 0x1.0000000000000p+1 }, false },
    { "Codac Interval:202", "set_le && set_neq", { 0x0.0p+0, 0x1.0000000000000p+0 }, { 0x0.0p+0, 0x1.0000000000000p+1 }, false },
    { "Codac Interval:203", "set_le && set_neq", { -codac_oo, 0x0.0p+0 }, { -codac_oo, codac_oo }, true },
    { "Codac Interval:204", "set_le && set_neq", codac_none, { 0x0.0p+0, 0x1.0000000000000p+0 }, true },
    { "Codac Interval:205", "set_geq", { 0x0.0p+0, 0x1.0000000000000p+1 }, { 0x0.0p+0, 0x1.0000000000000p+0 }, true },
    { "Codac Interval:206", "set_geq", { 0x1.0000000000000p+0, 0x1.0000000000000p+1 }, { 0x0.0p+0, 0x1.0000000000000p+0 }, false },
    { "Codac Interval:207", "set_geq && set_neq", { 0x0.0p+0, 0x1.0000000000000p+1 }, { 0x0.0p+0, 0x1.0000000000000p+1 }, false },
    { "Codac Interval:208", "set_geq && set_neq", { -codac_oo, codac_oo }, { -codac_oo, 0x0.0p+0 }, true },
    { "Codac Interval:209", "set_geq && set_neq", codac_none, codac_none, false },
    { "Codac Interval:214", "set_disjoint", { -codac_oo, 0x0.0p+0 }, { 0x0.0p+0, codac_oo }, false },
    { "Codac Interval:215", "set_disjoint", { 0x0.0p+0, 0x1.0000000000000p+0 }, { 0x1.0000000000000p+1, 0x1.8000000000000p+1 }, true },
    // END relations
  };

  // Whether the name of a relation or a function of a row is one the section knows: a name
  // the tool wrote and the section does not run would be a case that checks nothing
  bool codac_unknown(const std::string& name, const std::string& what)
  {
    check(name + ": the name " + what + " of the row is one the section runs", false);
    return false;
  }

  bool codac_relation(const std::string& name, const std::string& relation, const interval& x, const interval& y)
  {
    if (relation == "set_eq") {
      return x.set_eq(y);
    }
    if (relation == "set_neq") {
      return x.set_neq(y);
    }
    if (relation == "set_leq") {
      return x.set_leq(y);
    }
    if (relation == "set_geq") {
      return x.set_geq(y);
    }
    if (relation == "set_disjoint") {
      return x.set_disjoint(y);
    }
    if (relation == "set_le && set_neq") {
      return x.set_le(y) && x.set_neq(y);
    }
    if (relation == "set_geq && set_neq") {
      return x.set_geq(y) && x.set_neq(y);
    }
    return codac_unknown(name, relation);
  }

  // An interval and a double: contains() and interior_contains() of Codac. An
  // infinite bound holds its infinity: [-oo, +oo] contains +oo
  struct codac_PointRelation
  {
    const char* name;
    const char* relation;
    Ref x;
    double d;
    bool expected;
  };

  const codac_PointRelation codac_point_relations[] = {
    // BEGIN point_relations
    { "Codac Interval:210", "set_contains", { 0x0.0p+0, 0x1.0000000000000p+1 }, 0x0.0p+0, true },
    { "Codac Interval:211", "set_contains", { -codac_oo, codac_oo }, codac_oo, true },
    { "Codac Interval:212", "set_strictly_contains", { 0x0.0p+0, 0x1.0000000000000p+1 }, 0x0.0p+0, false },
    { "Codac Interval:213", "set_strictly_contains", { 0x0.0p+0, 0x1.0000000000000p+1 }, 0x1.0000000000000p+0, true },
    // END point_relations
  };

  // One interval: is_empty(), is_a_double() (is_degenerated() of Codac, with is_empty()
  // || is_a_double() for the empty set), is_finite()
  // (the opposite of is_unbounded(), true for the empty set) and is_bisectable():
  // a double lies strictly between the bounds
  struct codac_Predicate
  {
    const char* name;
    const char* predicate;
    Ref x;
    bool expected;
  };

  const codac_Predicate codac_predicates[] = {
    // BEGIN predicates
    { "Codac Interval:217", "is_empty", { 0x0.0p+0, 0x1.0000000000000p+0 }, false },
    { "Codac Interval:218", "is_a_double", { 0x0.0p+0, 0x0.0p+0 }, true },
    { "Codac Interval:219", "is_empty_or_a_double", codac_none, true },
    { "Codac Interval:220", "is_finite", codac_none, true },
    { "Codac Interval:221", "is_finite", { 0x0.0p+0, 0x1.0000000000000p+0 }, true },
    { "Codac Interval:222", "is_finite", { -codac_oo, 0x0.0p+0 }, false },
    { "Codac Interval:223", "is_finite", { 0x0.0p+0, codac_oo }, false },
    { "Codac Interval:224", "is_bisectable", codac_none, false },
    { "Codac Interval:225", "is_bisectable", { 0x1.fffffffffffffp+1023, codac_oo }, false },
    { "Codac Interval:226", "is_bisectable", { -codac_oo, -0x1.fffffffffffffp+1023 }, false },
    { "Codac Interval:227", "is_bisectable", { 0x0.0p+0, 0x0.0000000000001p-1022 }, false },
    { "Codac Interval:228", "is_bisectable", { 0x0.0p+0, 0x0.0000000000002p-1022 }, true },
    { "Codac Interval:229", "is_bisectable", { 0x1.4000000000000p+3, 0x1.4000000000002p+3 }, true },
    { "Codac Interval:230", "is_bisectable", { -0x0.0000000000002p-1022, 0x0.0p+0 }, true },
    { "Codac Interval:231", "is_bisectable", { 0x1.3fffffffffffep+3, 0x1.4000000000000p+3 }, true },
    { "Codac Interval:232", "is_bisectable", { 0x1.4000000000000p+3, 0x1.4000000000001p+3 }, false },
    { "Codac Interval:233", "is_bisectable", { -0x0.0000000000001p-1022, 0x0.0p+0 }, false },
    // END predicates
  };

  // x < y and x > y of Codac, which tell TRUE, FALSE, UNKNOWN or EMPTY, by two
  // relations of GAOL. For x < y: certainly_le (x < y for all x and y) and
  // certainly_geq (x >= y for all x and y): TRUE is yes and no, FALSE no and
  // yes, UNKNOWN no and no, and EMPTY, as the relations of an empty set hold
  // for all x and y, yes and yes. For x > y: certainly_ge and certainly_leq
  struct codac_Comparison
  {
    const char* name;
    Ref x, y;
    bool first, second;
  };

  const codac_Comparison codac_less[] = {
    // BEGIN less
    { "Codac Interval:300", { 0x0.0p+0, 0x1.0000000000000p+0 }, { 0x1.0000000000000p+1, 0x1.8000000000000p+1 }, true, false },
    { "Codac Interval:301", { 0x0.0p+0, 0x1.0000000000000p+0 }, { 0x1.0000000000000p+0, 0x1.0000000000000p+1 }, false, false },
    { "Codac Interval:302", { 0x0.0p+0, 0x1.0000000000000p+0 }, { 0x0.0p+0, 0x1.0000000000000p+0 }, false, false },
    { "Codac Interval:303", { 0x1.0000000000000p+1, 0x1.8000000000000p+1 }, { 0x0.0p+0, 0x1.0000000000000p+0 }, false, true },
    { "Codac Interval:304", { 0x1.0000000000000p+1, 0x1.8000000000000p+1 }, { -codac_oo, 0x1.0000000000000p+1 }, false, true },
    { "Codac Interval:305", { -codac_oo, 0x1.0000000000000p+1 }, { 0x1.8000000000000p+1, codac_oo }, true, false },
    { "Codac Interval:306", codac_none, { 0x0.0p+0, 0x1.0000000000000p+0 }, true, true },
    { "Codac Interval:307", { 0x0.0p+0, 0x1.0000000000000p+0 }, codac_none, true, true },
    // END less
  };

  const codac_Comparison codac_greater[] = {
    // BEGIN greater
    { "Codac Interval:309", { 0x1.0000000000000p+1, 0x1.8000000000000p+1 }, { 0x0.0p+0, 0x1.0000000000000p+0 }, true, false },
    { "Codac Interval:310", { 0x1.0000000000000p+0, 0x1.0000000000000p+1 }, { 0x0.0p+0, 0x1.0000000000000p+0 }, false, false },
    { "Codac Interval:311", { 0x0.0p+0, 0x1.0000000000000p+0 }, { 0x0.0p+0, 0x1.0000000000000p+0 }, false, false },
    { "Codac Interval:312", { 0x0.0p+0, 0x1.0000000000000p+0 }, { 0x1.0000000000000p+1, 0x1.8000000000000p+1 }, false, true },
    { "Codac Interval:313", { -codac_oo, 0x1.0000000000000p+1 }, { 0x1.0000000000000p+1, 0x1.8000000000000p+1 }, false, true },
    { "Codac Interval:314", { 0x1.8000000000000p+1, codac_oo }, { -codac_oo, 0x1.0000000000000p+1 }, true, false },
    { "Codac Interval:315", codac_none, { 0x0.0p+0, 0x1.0000000000000p+0 }, true, true },
    { "Codac Interval:316", { 0x0.0p+0, 0x1.0000000000000p+0 }, codac_none, true, true },
    // END greater
  };

  void codac_relations_run()
  {
    for (const codac_Relation& c : codac_relations) {
      const interval x = codac_interval(c.x), y = codac_interval(c.y);
      expect(std::string(c.name) + " " + c.relation, codac_relation(c.name, c.relation, x, y), c.expected,
             [&] { return "x = " + show(x) + ", y = " + show(y); });
    }
    for (const codac_PointRelation& c : codac_point_relations) {
      const interval x = codac_interval(c.x);
      const std::string relation = c.relation;
      const bool actual = (relation == "set_contains") ? x.set_contains(c.d)
                        : (relation == "set_strictly_contains") ? x.set_strictly_contains(c.d)
                        : codac_unknown(c.name, relation);
      expect(std::string(c.name) + " " + c.relation, actual, c.expected, [&] { return "x = " + show(x) + ", d = " + hex(c.d); });
    }
    for (const codac_Predicate& c : codac_predicates) {
      const interval x = codac_interval(c.x);
      const std::string predicate = c.predicate;
      const bool actual = (predicate == "is_empty") ? x.is_empty()
                        : (predicate == "is_a_double") ? x.is_a_double()
                        : (predicate == "is_empty_or_a_double") ? (x.is_empty() || x.is_a_double())
                        : (predicate == "is_finite") ? x.is_finite()
                        : (predicate == "is_bisectable") ? x.is_bisectable()
                        : codac_unknown(c.name, predicate);
      expect(std::string(c.name) + " " + c.predicate, actual, c.expected, [&] { return "x = " + show(x); });
    }
    for (const codac_Comparison& c : codac_less) {
      const interval x = codac_interval(c.x), y = codac_interval(c.y);
      const auto describe = [&] { return "x = " + show(x) + ", y = " + show(y); };
      expect(std::string(c.name) + " certainly_le", x.certainly_le(y), c.first, describe);
      expect(std::string(c.name) + " certainly_geq", x.certainly_geq(y), c.second, describe);
    }
    for (const codac_Comparison& c : codac_greater) {
      const interval x = codac_interval(c.x), y = codac_interval(c.y);
      const auto describe = [&] { return "x = " + show(x) + ", y = " + show(y); };
      expect(std::string(c.name) + " certainly_ge", x.certainly_ge(y), c.first, describe);
      expect(std::string(c.name) + " certainly_leq", x.certainly_leq(y), c.second, describe);
    }
  }

  //---------------------------------------------------------------------------
  // next_float() and previous_float(): the double next to x, the infinities
  // being next to the largest doubles (next_float(-oo) = -MAX, previous_float(+oo)
  // = MAX), +oo after +oo and -oo before -oo
  //---------------------------------------------------------------------------
  struct codac_Neighbour
  {
    const char* name;
    const char* function;
    double x;
    Ref ref;
  };

  const codac_Neighbour codac_neighbours[] = {
    // BEGIN neighbours
    { "Codac Interval_operations:94", "next_float", 0x0.0p+0, { 0x0.0000000000001p-1022, 0x0.0000000000001p-1022 } },
    { "Codac Interval_operations:95", "previous_float", 0x0.0p+0, { -0x0.0000000000001p-1022, -0x0.0000000000001p-1022 } },
    { "Codac Interval_operations:96", "next_float", 0x1.0000000000000p+0, { 0x1.0000000000001p+0, 0x1.0000000000001p+0 } },
    { "Codac Interval_operations:97", "previous_float", 0x1.0000000000000p+0, { 0x1.fffffffffffffp-1, 0x1.fffffffffffffp-1 } },
    { "Codac Interval_operations:98", "next_float", codac_oo, { codac_oo, codac_oo } },
    { "Codac Interval_operations:99", "previous_float", -codac_oo, { -codac_oo, -codac_oo } },
    { "Codac Interval_operations:100", "next_float", -codac_oo, { -0x1.fffffffffffffp+1023, -0x1.fffffffffffffp+1023 } },
    { "Codac Interval_operations:101", "previous_float", codac_oo, { 0x1.fffffffffffffp+1023, 0x1.fffffffffffffp+1023 } },
    // END neighbours
  };

  void codac_neighbours_run()
  {
    for (const codac_Neighbour& c : codac_neighbours) {
      const std::string function = c.function;
      if (function != "next_float" && function != "previous_float") {
        codac_unknown(c.name, function);
        continue;
      }
      expect(std::string(c.name) + " " + c.function, (function == "next_float") ? next_float(c.x) : previous_float(c.x), c.ref,
             [&] { return function + "(" + hex(c.x) + ")"; });
    }
  }

  //---------------------------------------------------------------------------
  // The functions of one interval: the exact bounds for the opposite, the
  // square root and the integer functions, within one double of the tightest
  // for the others. The rows after "rounded_values" are the points of the
  // rounding test of Codac that tests/elementary_values.h does not hold,
  // with the doubles next to the exact value
  //---------------------------------------------------------------------------
  struct codac_Unary
  {
    const char* name;
    const char* function;
    Ref x, ref;
  };

  struct codac_Function
  {
    interval (*f)(const interval&);
    int max_doubles;
  };

  const std::map<std::string, codac_Function>& codac_functions()
  {
    static const std::map<std::string, codac_Function> functions = {
      { "neg", { [](const interval& x) { return -x; }, 0 } },
      { "sqrt", { [](const interval& x) { return sqrt(x); }, 0 } },
      { "floor", { [](const interval& x) { return floor(x); }, 0 } },
      { "ceil", { [](const interval& x) { return ceil(x); }, 0 } },
      { "integer", { [](const interval& x) { return integer(x); }, 0 } },
      { "exp", { [](const interval& x) { return exp(x); }, limit } },
      { "log", { [](const interval& x) { return log(x); }, limit } },
      { "sin", { [](const interval& x) { return sin(x); }, limit } },
      { "cos", { [](const interval& x) { return cos(x); }, limit } },
      { "tan", { [](const interval& x) { return tan(x); }, limit } },
      { "atan", { [](const interval& x) { return atan(x); }, limit } },
    };
    return functions;
  }

  const codac_Unary codac_unary_cases[] = {
    // BEGIN negations
    { "Codac Interval_operations:104", "neg", { 0x0.0p+0, 0x1.0000000000000p+0 }, { -0x1.0000000000000p+0, 0x0.0p+0 } },
    { "Codac Interval_operations:105", "neg", { -codac_oo, codac_oo }, { -codac_oo, codac_oo } },
    { "Codac Interval_operations:106", "neg", { -codac_oo, 0x0.0p+0 }, { 0x0.0p+0, codac_oo } },
    { "Codac Interval_operations:107", "neg", { -codac_oo, 0x1.0000000000000p+0 }, { -0x1.0000000000000p+0, codac_oo } },
    // END negations
    // BEGIN roots_of_squares
    { "Codac Interval_operations:168", "sqrt", { -codac_oo, codac_oo }, { 0x0.0p+0, codac_oo } },
    { "Codac Interval_operations:169", "sqrt", { -codac_oo, 0x0.0p+0 }, { 0x0.0p+0, 0x0.0p+0 } },
    { "Codac Interval_operations:170", "sqrt", { -0x1.2000000000000p+3, 0x1.0000000000000p+2 }, { 0x0.0p+0, 0x1.0000000000000p+1 } },
    { "Codac Interval_operations:171", "sqrt", { 0x1.0000000000000p+2, 0x1.2000000000000p+3 }, { 0x1.0000000000000p+1, 0x1.8000000000000p+1 } },
    { "Codac Interval_operations:172", "sqrt", { -0x1.2000000000000p+3, -0x1.0000000000000p+2 }, codac_none },
    { "Codac Interval_operations:173", "sqrt", { -0x1.2000000000000p+3, codac_oo }, { 0x0.0p+0, codac_oo } },
    // END roots_of_squares
    // BEGIN logarithms
    { "Codac Interval_operations:175", "log", codac_none, codac_none },
    { "Codac Interval_operations:176", "log", { -codac_oo, codac_oo }, { -codac_oo, codac_oo } },
    { "Codac Interval_operations:177", "log", { 0x0.0p+0, codac_oo }, { -codac_oo, codac_oo } },
    { "Codac Interval_operations:178", "log", { -codac_oo, 0x0.0p+0 }, codac_none },
    { "Codac Interval_operations:179", "log", { 0x1.0000000000000p+0, 0x1.0000000000000p+1 }, { 0x0.0p+0, 0x1.62e42fefa39f0p-1 } },
    { "Codac Interval_operations:180", "log", { -0x1.0000000000000p+0, 0x1.0000000000000p+0 }, { -codac_oo, 0x0.0p+0 } },
    { "Codac Interval_operations:181", "log", { 0x0.0p+0, 0x1.0000000000000p+0 }, { -codac_oo, 0x0.0p+0 } },
    { "Codac Interval_operations:182", "log", { 0x1.0000000000000p+0, codac_oo }, { 0x0.0p+0, codac_oo } },
    { "Codac Interval_operations:183", "log", { 0x0.0p+0, 0x0.0p+0 }, codac_none },
    { "Codac Interval_operations:184", "log", { -0x1.0000000000000p+1, -0x1.0000000000000p+0 }, codac_none },
    { "Codac Interval_operations:185", "log", { 0x0.0p+0, 0x0.0000000000001p-1022 }, { -codac_oo, -0x1.74385446d71c3p+9 } },
    // END logarithms
    // BEGIN exponentials
    { "Codac Interval_operations:187", "exp", codac_none, codac_none },
    { "Codac Interval_operations:188", "exp", { -codac_oo, codac_oo }, { 0x0.0p+0, codac_oo } },
    { "Codac Interval_operations:189", "exp", { 0x0.0p+0, codac_oo }, { 0x1.0000000000000p+0, codac_oo } },
    { "Codac Interval_operations:190", "exp", { -codac_oo, 0x0.0p+0 }, { 0x0.0p+0, 0x1.0000000000000p+0 } },
    { "Codac Interval_operations:191", "exp", { 0x0.0p+0, 0x1.0000000000000p+1 }, { 0x1.0000000000000p+0, 0x1.d8e64b8d4ddaep+2 } },
    { "Codac Interval_operations:192", "exp", { -0x1.0000000000000p+0, 0x1.0000000000000p+0 }, { 0x1.78b56362cef37p-2, 0x1.5bf0a8b14576ap+1 } },
    { "Codac Interval_operations:193", "exp", { 0x1.249ad2594c37dp+332, 0x1.a9cbc59b83a3dp+368 }, { 0x1.fffffffffffffp+1023, codac_oo } },
    { "Codac Interval_operations:194", "exp", { 0x1.fffffffffffffp+1023, codac_oo }, { 0x1.fffffffffffffp+1023, codac_oo } },
    { "Codac Interval_operations:195", "exp", { 0x0.0p+0, 0x1.fffffffffffffp+1023 }, { 0x1.0000000000000p+0, codac_oo } },
    // END exponentials
    // BEGIN integer_functions
    { "Codac Interval_operations:214", "floor", codac_none, codac_none },
    { "Codac Interval_operations:215", "floor", { -codac_oo, -0x1.0c6f7a0b5ed8dp-20 }, { -codac_oo, -0x1.0000000000000p+0 } },
    { "Codac Interval_operations:216", "floor", { 0x1.5798ee2308c3ap-27, codac_oo }, { 0x0.0p+0, codac_oo } },
    { "Codac Interval_operations:217", "floor", { -codac_oo, codac_oo }, { -codac_oo, codac_oo } },
    { "Codac Interval_operations:218", "floor", { 0x1.47ae147ae147bp-7, 0x1.7eb851eb851ecp+1 }, { 0x0.0p+0, 0x1.0000000000000p+1 } },
    { "Codac Interval_operations:219", "floor", { -0x1.47ae147ae147bp-7, 0x1.7eb851eb851ecp+1 }, { -0x1.0000000000000p+0, 0x1.0000000000000p+1 } },
    { "Codac Interval_operations:220", "floor", { 0x1.000000044b830p+0, 0x1.ffffffff920c8p+0 }, { 0x1.0000000000000p+0, 0x1.0000000000000p+0 } },
    { "Codac Interval_operations:221", "floor", { 0x1.7d78400000000p+26, 0x1.fffffffffffffp+1023 }, { 0x1.7d78400000000p+26, 0x1.fffffffffffffp+1023 } },
    { "Codac Interval_operations:223", "ceil", codac_none, codac_none },
    { "Codac Interval_operations:224", "ceil", { -codac_oo, -0x1.0c6f7a0b5ed8dp-20 }, { -codac_oo, 0x0.0p+0 } },
    { "Codac Interval_operations:225", "ceil", { 0x1.5798ee2308c3ap-27, codac_oo }, { 0x1.0000000000000p+0, codac_oo } },
    { "Codac Interval_operations:226", "ceil", { -codac_oo, codac_oo }, { -codac_oo, codac_oo } },
    { "Codac Interval_operations:227", "ceil", { 0x1.47ae147ae147bp-7, 0x1.7eb851eb851ecp+1 }, { 0x1.0000000000000p+0, 0x1.8000000000000p+1 } },
    { "Codac Interval_operations:228", "ceil", { -0x1.47ae147ae147bp-7, 0x1.7eb851eb851ecp+1 }, { 0x0.0p+0, 0x1.8000000000000p+1 } },
    { "Codac Interval_operations:229", "ceil", { 0x1.000000044b830p+0, 0x1.ffffffff920c8p+0 }, { 0x1.0000000000000p+1, 0x1.0000000000000p+1 } },
    { "Codac Interval_operations:230", "ceil", { 0x1.7d78400000000p+26, 0x1.fffffffffffffp+1023 }, { 0x1.7d78400000000p+26, 0x1.fffffffffffffp+1023 } },
    { "Codac Interval_operations:232", "integer", codac_none, codac_none },
    { "Codac Interval_operations:233", "integer", { -codac_oo, -0x1.0c6f7a0b5ed8dp-20 }, { -codac_oo, -0x1.0000000000000p+0 } },
    { "Codac Interval_operations:234", "integer", { 0x1.5798ee2308c3ap-27, codac_oo }, { 0x1.0000000000000p+0, codac_oo } },
    { "Codac Interval_operations:235", "integer", { -codac_oo, codac_oo }, { -codac_oo, codac_oo } },
    { "Codac Interval_operations:236", "integer", { 0x1.47ae147ae147bp-7, 0x1.7eb851eb851ecp+1 }, { 0x1.0000000000000p+0, 0x1.0000000000000p+1 } },
    { "Codac Interval_operations:237", "integer", { -0x1.47ae147ae147bp-7, 0x1.7eb851eb851ecp+1 }, { 0x0.0p+0, 0x1.0000000000000p+1 } },
    { "Codac Interval_operations:238", "integer", { 0x1.000000044b830p+0, 0x1.ffffffff920c8p+0 }, codac_none },
    { "Codac Interval_operations:239", "integer", { 0x1.7d78400000000p+26, 0x1.fffffffffffffp+1023 }, { 0x1.7d78400000000p+26, 0x1.fffffffffffffp+1023 } },
    // END integer_functions
    // BEGIN tangents
    { "Codac Interval_operations:249", "tan", { -codac_oo, codac_oo }, { -codac_oo, codac_oo } },
    { "Codac Interval_operations:254", "tan", { 0x1.921fb54442d18p-1, 0x1.921fb543961b9p+0 }, { 0x1.fffffffffffffp-1, 0x1.7b74896a2d0fcp+32 } },
    { "Codac Interval_operations:261", "tan", { -0x1.921fb543961b8p+0, 0x1.921fb54442d18p-1 }, { -0x1.7b7466430c12ap+32, 0x1.0000000000000p+0 } },
    // END tangents
    // BEGIN rounded_values
    { "Codac Interval_rounding:303", "exp", { 0x1.0624dd2f1a9fcp-10, 0x1.0624dd2f1a9fcp-10 }, { 0x1.0041919b7ee33p+0, 0x1.0041919b7ee34p+0 } },
    { "Codac Interval_rounding:309", "log", { 0x1.4f8b588e368f1p-17, 0x1.4f8b588e368f1p-17 }, { -0x1.7069e2aa2aa5bp+3, -0x1.7069e2aa2aa5ap+3 } },
    { "Codac Interval_rounding:310", "log", { 0x1.2a05f20000000p+33, 0x1.2a05f20000000p+33 }, { 0x1.7069e2aa2aa5ap+4, 0x1.7069e2aa2aa5bp+4 } },
    { "Codac Interval_rounding:322", "cos", { 0x1.8000000000000p+0, 0x1.8000000000000p+0 }, { 0x1.21bd54fc5f9a7p-4, 0x1.21bd54fc5f9a8p-4 } },
    { "Codac Interval_rounding:325", "tan", { -0x1.3333333333333p+0, -0x1.3333333333333p+0 }, { -0x1.493c43acb164dp+1, -0x1.493c43acb164cp+1 } },
    { "Codac Interval_rounding:334", "sqrt", { 0x1.ad7f29abcaf48p-24, 0x1.ad7f29abcaf48p-24 }, { 0x1.4b96be9c2da2bp-12, 0x1.4b96be9c2da2cp-12 } },
    // END rounded_values
  };

  void codac_unary_run()
  {
    for (const codac_Unary& c : codac_unary_cases) {
      const std::map<std::string, codac_Function>::const_iterator found = codac_functions().find(c.function);
      if (found == codac_functions().end()) {
        codac_unknown(c.name, c.function);
        continue;
      }
      const codac_Function& f = found->second;
      const interval x = codac_interval(c.x);
      const std::string n = std::string(c.name) + " " + c.function;
      const auto describe = [&] { return std::string(c.function) + "(" + show(x) + ")"; };
      expect(n, evaluate(n, [&] { return f.f(x); }, describe), c.ref, describe, f.max_doubles);
    }
  }

  //---------------------------------------------------------------------------
  // CHECK_trigo: sin and cos of x, and of the intervals the other forms give
  // (pi - x, x + 2 pi, -x, x - pi/2, pi/2 - x, x + pi/2, x + 2 pi - pi/2),
  // computed with interval::pi(), interval::two_pi() and interval::half_pi().
  // The reference is the hull of the function over the interval of doubles that
  // the tightest operations give for the argument
  //---------------------------------------------------------------------------
  struct codac_Trigonometric
  {
    const char* name;
    Ref x;
    Ref ref[8];
  };

  const codac_Trigonometric codac_trigonometric[] = {
    // BEGIN trigonometric
    { "Codac Interval_operations:200", { -codac_oo, codac_oo }, { { -0x1.0000000000000p+0, 0x1.0000000000000p+0 }, { -0x1.0000000000000p+0, 0x1.0000000000000p+0 }, { -0x1.0000000000000p+0, 0x1.0000000000000p+0 }, { -0x1.0000000000000p+0, 0x1.0000000000000p+0 }, { -0x1.0000000000000p+0, 0x1.0000000000000p+0 }, { -0x1.0000000000000p+0, 0x1.0000000000000p+0 }, { -0x1.0000000000000p+0, 0x1.0000000000000p+0 }, { -0x1.0000000000000p+0, 0x1.0000000000000p+0 } } },
    { "Codac Interval_operations:201", codac_none, { codac_none, codac_none, codac_none, codac_none, codac_none, codac_none, codac_none, codac_none } },
    { "Codac Interval_operations:202", { 0x0.0p+0, 0x1.921fb54442d19p+0 }, { { 0x0.0p+0, 0x1.0000000000000p+0 }, { -0x1.72cece675d1fdp-52, 0x1.0000000000000p+0 }, { -0x1.1a62633145c07p-52, 0x1.0000000000000p+0 }, { -0x1.0000000000000p+0, 0x0.0p+0 }, { -0x1.72cece675d1fdp-53, 0x1.0000000000000p+0 }, { -0x1.72cece675d1fdp-53, 0x1.0000000000000p+0 }, { -0x1.0000000000000p+0, 0x1.1a62633145c07p-54 }, { -0x1.34f272993d142p-50, 0x1.0000000000000p+0 } } },
    { "Codac Interval_operations:203", { 0x0.0p+0, 0x1.921fb54442d19p+1 }, { { -0x1.72cece675d1fdp-52, 0x1.0000000000000p+0 }, { -0x1.0000000000000p-51, 0x1.0000000000000p+0 }, { -0x1.961b1acd85d7ep-50, 0x1.0000000000000p+0 }, { -0x1.0000000000000p+0, 0x1.72cece675d1fdp-52 }, { -0x1.b9676733ae8ffp-52, 0x1.0000000000000p+0 }, { -0x1.b9676733ae8ffp-52, 0x1.0000000000000p+0 }, { -0x1.0000000000000p+0, 0x1.961b1acd85d7ep-51 }, { -0x1.a7c141009a33ep-50, 0x1.0000000000000p+0 } } },
    { "Codac Interval_operations:204", { 0x0.0p+0, 0x1.2d97c7f3321d3p+2 }, { { -0x1.0000000000000p+0, 0x1.0000000000000p+0 }, { -0x1.0000000000000p+0, 0x1.0000000000000p+0 }, { -0x1.0000000000000p+0, 0x1.0000000000000p+0 }, { -0x1.0000000000000p+0, 0x1.0000000000000p+0 }, { -0x1.0000000000000p+0, 0x1.0000000000000p+0 }, { -0x1.0000000000000p+0, 0x1.0000000000000p+0 }, { -0x1.0000000000000p+0, 0x1.0000000000000p+0 }, { -0x1.0000000000000p+0, 0x1.0000000000000p+0 } } },
    { "Codac Interval_operations:205", { 0x1.921fb54442d18p+1, 0x1.2d97c7f3321d3p+2 }, { { -0x1.0000000000000p+0, 0x1.1a62633145c07p-53 }, { -0x1.0000000000000p+0, 0x1.0000000000000p-51 }, { -0x1.0000000000000p+0, 0x1.a79394c9e8a0bp-52 }, { -0x1.1a62633145c07p-53, 0x1.0000000000000p+0 }, { -0x1.0000000000000p+0, 0x1.469898cc51702p-52 }, { -0x1.0000000000000p+0, 0x1.469898cc51702p-52 }, { -0x1.a79394c9e8a0bp-53, 0x1.0000000000000p+0 }, { -0x1.0000000000000p+0, 0x1.583ebeff65cc3p-50 } } },
    { "Codac Interval_operations:206", { 0x1.0000000000000p-1, 0x1.8000000000000p+0 }, { { 0x1.eaee8744b05efp-2, 0x1.feb7a9b2c6d8bp-1 }, { 0x1.eaee8744b05eap-2, 0x1.feb7a9b2c6d8bp-1 }, { 0x1.eaee8744b05ecp-2, 0x1.feb7a9b2c6d8cp-1 }, { -0x1.feb7a9b2c6d8bp-1, -0x1.eaee8744b05efp-2 }, { 0x1.eaee8744b05edp-2, 0x1.feb7a9b2c6d8bp-1 }, { 0x1.eaee8744b05edp-2, 0x1.feb7a9b2c6d8bp-1 }, { -0x1.feb7a9b2c6d8cp-1, -0x1.eaee8744b05eep-2 }, { 0x1.eaee8744b05dep-2, 0x1.feb7a9b2c6d8cp-1 } } },
    { "Codac Interval_operations:207", { 0x1.8000000000000p+0, 0x1.8000000000000p+1 }, { { 0x1.210386db6d55bp-3, 0x1.0000000000000p+0 }, { 0x1.210386db6d556p-3, 0x1.0000000000000p+0 }, { 0x1.210386db6d524p-3, 0x1.0000000000000p+0 }, { -0x1.0000000000000p+0, -0x1.210386db6d55bp-3 }, { 0x1.210386db6d559p-3, 0x1.0000000000000p+0 }, { 0x1.210386db6d559p-3, 0x1.0000000000000p+0 }, { -0x1.0000000000000p+0, -0x1.210386db6d53dp-3 }, { 0x1.210386db6d522p-3, 0x1.0000000000000p+0 } } },
    { "Codac Interval_operations:208", { 0x1.8000000000000p+1, 0x1.0000000000000p+2 }, { { -0x1.837b9dddc1eafp-1, 0x1.210386db6d55cp-3 }, { -0x1.837b9dddc1eb0p-1, 0x1.210386db6d567p-3 }, { -0x1.837b9dddc1eb8p-1, 0x1.210386db6d565p-3 }, { -0x1.210386db6d55cp-3, 0x1.837b9dddc1eafp-1 }, { -0x1.837b9dddc1eafp-1, 0x1.210386db6d562p-3 }, { -0x1.837b9dddc1eafp-1, 0x1.210386db6d562p-3 }, { -0x1.210386db6d55ep-3, 0x1.837b9dddc1eb4p-1 }, { -0x1.837b9dddc1eb8p-1, 0x1.210386db6d582p-3 } } },
    { "Codac Interval_operations:209", { 0x1.8000000000000p+1, 0x1.4000000000000p+2 }, { { -0x1.0000000000000p+0, 0x1.210386db6d55cp-3 }, { -0x1.0000000000000p+0, 0x1.210386db6d567p-3 }, { -0x1.0000000000000p+0, 0x1.210386db6d565p-3 }, { -0x1.210386db6d55cp-3, 0x1.0000000000000p+0 }, { -0x1.0000000000000p+0, 0x1.210386db6d562p-3 }, { -0x1.0000000000000p+0, 0x1.210386db6d562p-3 }, { -0x1.210386db6d55ep-3, 0x1.0000000000000p+0 }, { -0x1.0000000000000p+0, 0x1.210386db6d582p-3 } } },
    { "Codac Interval_operations:210", { 0x1.8000000000000p+1, 0x1.f21fb54442d19p+2 }, { { -0x1.0000000000000p+0, 0x1.feb7a9b2c6d8cp-1 }, { -0x1.0000000000000p+0, 0x1.feb7a9b2c6d8cp-1 }, { -0x1.0000000000000p+0, 0x1.feb7a9b2c6d8cp-1 }, { -0x1.feb7a9b2c6d8cp-1, 0x1.0000000000000p+0 }, { -0x1.0000000000000p+0, 0x1.feb7a9b2c6d8cp-1 }, { -0x1.0000000000000p+0, 0x1.feb7a9b2c6d8cp-1 }, { -0x1.feb7a9b2c6d8cp-1, 0x1.0000000000000p+0 }, { -0x1.0000000000000p+0, 0x1.feb7a9b2c6d8cp-1 } } },
    { "Codac Interval_operations:211", { 0x1.4000000000000p+2, 0x1.f21fb54442d19p+2 }, { { -0x1.eaf81f5e09934p-1, 0x1.feb7a9b2c6d8cp-1 }, { -0x1.eaf81f5e09934p-1, 0x1.feb7a9b2c6d8cp-1 }, { -0x1.eaf81f5e09934p-1, 0x1.feb7a9b2c6d8cp-1 }, { -0x1.feb7a9b2c6d8cp-1, 0x1.eaf81f5e09934p-1 }, { -0x1.eaf81f5e09935p-1, 0x1.feb7a9b2c6d8cp-1 }, { -0x1.eaf81f5e09935p-1, 0x1.feb7a9b2c6d8cp-1 }, { -0x1.feb7a9b2c6d8cp-1, 0x1.eaf81f5e09934p-1 }, { -0x1.eaf81f5e09939p-1, 0x1.feb7a9b2c6d8cp-1 } } },
    { "Codac Interval_operations:212", { 0x1.4000000000000p+2, 0x1.290fdaa22168dp+3 }, { { -0x1.eaf81f5e09934p-1, 0x1.0000000000000p+0 }, { -0x1.eaf81f5e09934p-1, 0x1.0000000000000p+0 }, { -0x1.eaf81f5e09934p-1, 0x1.0000000000000p+0 }, { -0x1.0000000000000p+0, 0x1.eaf81f5e09934p-1 }, { -0x1.eaf81f5e09935p-1, 0x1.0000000000000p+0 }, { -0x1.eaf81f5e09935p-1, 0x1.0000000000000p+0 }, { -0x1.0000000000000p+0, 0x1.eaf81f5e09934p-1 }, { -0x1.eaf81f5e09939p-1, 0x1.0000000000000p+0 } } },
    // END trigonometric
  };

  void codac_trigonometric_run()
  {
    const char* const names[8] = { "sin(x)", "sin(pi-x)", "sin(x+2pi)", "sin(-x)", "cos(x-pi/2)", "cos(pi/2-x)", "cos(x+pi/2)",
                                   "cos(x+2pi-pi/2)" };
    for (const codac_Trigonometric& c : codac_trigonometric) {
      const interval x = codac_interval(c.x);
      const interval results[8] = {
        sin(x), sin(interval::pi() - x), sin(x + interval::two_pi()), sin(-x),
        cos(x - interval::half_pi()), cos(interval::half_pi() - x), cos(x + interval::half_pi()),
        cos(x + interval::two_pi() - interval::half_pi())
      };
      for (int k = 0; k < 8; ++k) {
        expect(std::string(c.name) + " " + names[k], results[k], c.ref[k], [&] { return "x = " + show(x); }, limit);
      }
    }
  }

  //---------------------------------------------------------------------------
  // tan of an argument computed with interval::pi() (lines 241 to 250): the
  // hull over the interval of doubles that argument is; tan(-pi) holds 0, a
  // zero of the function between the poles -3 pi/2 and -pi/2
  //---------------------------------------------------------------------------
  struct codac_TangentArgument
  {
    const char* name;
    interval (*argument)();
    Ref ref;
    bool holds_zero; // the original checks that the result holds 0
  };

  const codac_TangentArgument codac_tangent_arguments[] = {
    // BEGIN tangent_arguments
    { "Codac Interval_operations:241", [] { return (interval::pi() / 2.0); }, { -codac_oo, codac_oo }, false },
    { "Codac Interval_operations:243", [] { return (-interval::pi()); }, { -0x1.72cece675d1fdp-52, 0x1.1a62633145c07p-53 }, true },
    { "Codac Interval_operations:248", [] { return (((3.0 * interval::pi()) / 4.0) | ((5.0 * interval::pi()) / 4.0)); }, { -0x1.0000000000001p+0, 0x1.0000000000007p+0 }, false },
    { "Codac Interval_operations:250", [] { return (((-interval::pi()) / 4.0) | (interval::pi() / 4.0)); }, { -0x1.0000000000001p+0, 0x1.0000000000001p+0 }, false },
    // END tangent_arguments
  };

  void codac_tangent_arguments_run()
  {
    for (const codac_TangentArgument& c : codac_tangent_arguments) {
      const interval x = c.argument();
      const interval r = tan(x);
      expect(std::string(c.name) + " tan", r, c.ref, [&] { return "tan(" + show(x) + ")"; }, limit);
      if (c.holds_zero) {
        expect(std::string(c.name) + " tan holds 0", r.set_contains(0.0), true, [&] { return "tan(" + show(x) + ") = " + show(r); });
      }
    }
  }

  //---------------------------------------------------------------------------
  // atan2 (the box of angles of IEEE 1788-2015, atan2(0, x) = pi for x < 0, no
  // value at the origin) over boxes, and the powers: pow(x, n) and pow(-x, n)
  // for an int n; pow(x, y) for an interval y, and pow(x, d), pow(x, n) for an
  // exponent that is a double or an int (the integer power on every x for an
  // exponent that is an integer, the pow of IEEE 1788-2015 otherwise: the part
  // of x in [0, +oo]); nth_root
  //---------------------------------------------------------------------------
  struct codac_Atan2
  {
    const char* name;
    Ref y, x, ref;
  };

  const codac_Atan2 codac_atan2_cases[] = {
    // BEGIN atan2
    { "Codac Interval_operations:266", { -codac_oo, codac_oo }, codac_none, codac_none },
    { "Codac Interval_operations:267", { 0x1.0000000000000p+0, 0x1.0000000000000p+0 }, { 0x1.0000000000000p+0, 0x1.0000000000000p+0 }, { 0x1.921fb54442d18p-1, 0x1.921fb54442d19p-1 } },
    { "Codac Interval_operations:268", { -0x1.0000000000000p+0, -0x1.0000000000000p+0 }, { -0x1.0000000000000p+0, -0x1.0000000000000p+0 }, { -0x1.2d97c7f3321d3p+1, -0x1.2d97c7f3321d2p+1 } },
    { "Codac Interval_operations:269", { -0x1.0000000000000p+0, -0x1.0000000000000p+0 }, { 0x1.0000000000000p+0, 0x1.0000000000000p+0 }, { -0x1.921fb54442d19p-1, -0x1.921fb54442d18p-1 } },
    { "Codac Interval_operations:270", { 0x1.0000000000000p+0, 0x1.0000000000000p+0 }, { -0x1.0000000000000p+0, -0x1.0000000000000p+0 }, { 0x1.2d97c7f3321d2p+1, 0x1.2d97c7f3321d3p+1 } },
    { "Codac Interval_operations:271", { 0x0.0p+0, codac_oo }, { 0x0.0p+0, codac_oo }, { 0x0.0p+0, 0x1.921fb54442d19p+0 } },
    { "Codac Interval_operations:272", { -codac_oo, -0x0.0000000000001p-1022 }, { -codac_oo, 0x0.0p+0 }, { -0x1.921fb54442d19p+1, -0x1.921fb54442d18p+0 } },
    { "Codac Interval_operations:273", { -codac_oo, 0x0.0p+0 }, { 0x0.0p+0, codac_oo }, { -0x1.921fb54442d19p+0, 0x0.0p+0 } },
    { "Codac Interval_operations:274", { 0x0.0p+0, codac_oo }, { -codac_oo, 0x0.0p+0 }, { 0x1.921fb54442d18p+0, 0x1.921fb54442d19p+1 } },
    { "Codac Interval_operations:275", { 0x1.0000000000000p+0, codac_oo }, { -0x1.0000000000000p+0, 0x1.0000000000000p+0 }, { 0x1.921fb54442d18p-1, 0x1.2d97c7f3321d3p+1 } },
    { "Codac Interval_operations:276", { -codac_oo, -0x1.0000000000000p+0 }, { -0x1.0000000000000p+0, 0x1.0000000000000p+0 }, { -0x1.2d97c7f3321d3p+1, -0x1.921fb54442d18p-1 } },
    { "Codac Interval_operations:277", { -0x1.0000000000000p+0, 0x1.0000000000000p+0 }, { 0x1.0000000000000p+0, codac_oo }, { -0x1.921fb54442d19p-1, 0x1.921fb54442d19p-1 } },
    { "Codac Interval_operations:278", { -0x1.0000000000000p+0, 0x1.0000000000000p+0 }, { -codac_oo, -0x1.0000000000000p+0 }, { -0x1.921fb54442d19p+1, 0x1.921fb54442d19p+1 } },
    { "Codac Interval_operations:279", { -codac_oo, codac_oo }, { -codac_oo, codac_oo }, { -0x1.921fb54442d19p+1, 0x1.921fb54442d19p+1 } },
    { "Codac Interval_operations:280", { 0x0.0p+0, 0x0.0p+0 }, { 0x0.0p+0, 0x0.0p+0 }, codac_none },
    // END atan2
  };

  struct codac_IntegerPower
  {
    const char* name;
    Ref x;
    int n;
    Ref ref;
  };

  const codac_IntegerPower codac_integer_powers[] = {
    // BEGIN integer_powers
    { "Codac Interval_operations:282", { -codac_oo, codac_oo }, 4, { 0x0.0p+0, codac_oo } },
    { "Codac Interval_operations:283", codac_none, 4, codac_none },
    { "Codac Interval_operations:284", { 0x1.0000000000000p+1, 0x1.8000000000000p+1 }, 4, { 0x1.0000000000000p+4, 0x1.4400000000000p+6 } },
    { "Codac Interval_operations:285", { -0x1.0000000000000p+1, 0x1.8000000000000p+1 }, 4, { 0x0.0p+0, 0x1.4400000000000p+6 } },
    { "Codac Interval_operations:286", { -0x1.8000000000000p+1, 0x1.0000000000000p+1 }, 4, { 0x0.0p+0, 0x1.4400000000000p+6 } },
    { "Codac Interval_operations:287", { 0x1.0000000000000p+1, codac_oo }, 4, { 0x1.0000000000000p+4, codac_oo } },
    { "Codac Interval_operations:288", { -codac_oo, codac_oo }, 3, { -codac_oo, codac_oo } },
    { "Codac Interval_operations:289", codac_none, 3, codac_none },
    { "Codac Interval_operations:290", { 0x1.0000000000000p+1, 0x1.8000000000000p+1 }, 3, { 0x1.0000000000000p+3, 0x1.b000000000000p+4 } },
    { "Codac Interval_operations:291", { -0x1.0000000000000p+1, 0x1.8000000000000p+1 }, 3, { -0x1.0000000000000p+3, 0x1.b000000000000p+4 } },
    { "Codac Interval_operations:292", { -0x1.8000000000000p+1, 0x1.0000000000000p+1 }, 3, { -0x1.b000000000000p+4, 0x1.0000000000000p+3 } },
    { "Codac Interval_operations:293", { 0x1.0000000000000p+1, codac_oo }, 3, { 0x1.0000000000000p+3, codac_oo } },
    { "Codac Interval_operations:294", { -0x1.4000000000000p+3, 0x1.4000000000000p+3 }, -2, { 0x1.47ae147ae147ap-7, codac_oo } },
    // END integer_powers
  };

  const codac_Binary codac_powers[] = {
    // BEGIN powers
    { "Codac Interval_operations:300", { 0x1.0000000000000p+2, 0x1.0000000000000p+2 }, { 0x1.0000000000000p-1, 0x1.0000000000000p-1 }, { 0x1.0000000000000p+1, 0x1.0000000000000p+1 } },
    { "Codac Interval_operations:301", { 0x1.0000000000000p+2, 0x1.0000000000000p+2 }, { 0x1.8000000000000p+0, 0x1.8000000000000p+0 }, { 0x1.0000000000000p+3, 0x1.0000000000000p+3 } },
    { "Codac Interval_operations:302", { 0x1.0000000000000p+2, 0x1.2000000000000p+3 }, { 0x1.0000000000000p-1, 0x1.0000000000000p-1 }, { 0x1.0000000000000p+1, 0x1.8000000000000p+1 } },
    { "Codac Interval_operations:303", { 0x1.0000000000000p+2, 0x1.0000000000000p+2 }, { -0x1.0000000000000p-1, -0x1.0000000000000p-1 }, { 0x1.0000000000000p-1, 0x1.0000000000000p-1 } },
    { "Codac Interval_operations:304", { 0x0.0p+0, 0x1.0000000000000p+2 }, { 0x1.0000000000000p-1, 0x1.0000000000000p-1 }, { 0x0.0p+0, 0x1.0000000000000p+1 } },
    { "Codac Interval_operations:305", { 0x1.0000000000000p+1, 0x1.8000000000000p+1 }, { 0x1.8000000000000p+0, 0x1.8000000000000p+0 }, { 0x1.6a09e667f3bccp+1, 0x1.4c8dc2e423980p+2 } },
    { "Codac Interval_operations:310", { -0x1.0000000000000p+1, 0x1.8000000000000p+1 }, { 0x1.8000000000000p+1, 0x1.8000000000000p+1 }, { -0x1.0000000000000p+3, 0x1.b000000000000p+4 } },
    { "Codac Interval_operations:311", { -0x1.0000000000000p+1, -0x1.0000000000000p+1 }, { 0x1.0000000000000p+1, 0x1.0000000000000p+1 }, { 0x1.0000000000000p+2, 0x1.0000000000000p+2 } },
    { "Codac Interval_operations:312", { 0x1.0000000000000p+1, 0x1.8000000000000p+1 }, { 0x1.0000000000000p+2, 0x1.0000000000000p+2 }, { 0x1.0000000000000p+4, 0x1.4400000000000p+6 } },
    { "Codac Interval_operations:313", { 0x1.0000000000000p+2, 0x1.0000000000000p+2 }, { 0x0.0p+0, 0x0.0p+0 }, { 0x1.0000000000000p+0, 0x1.0000000000000p+0 } },
    { "Codac Interval_operations:314", { -0x1.4000000000000p+3, 0x1.4000000000000p+3 }, { -0x1.0000000000000p+1, -0x1.0000000000000p+1 }, { 0x1.47ae147ae147ap-7, codac_oo } },
    { "Codac Interval_operations:318", { -0x1.0000000000000p+2, -0x1.0000000000000p+0 }, { 0x1.0000000000000p-1, 0x1.0000000000000p-1 }, codac_none },
    { "Codac Interval_operations:319", { -0x1.0000000000000p+2, -0x1.0000000000000p+0 }, { 0x1.0000000000000p-1, 0x1.0000000000000p-1 }, codac_none },
    { "Codac Interval_operations:321", { -0x1.0000000000000p+2, 0x1.2000000000000p+3 }, { 0x1.0000000000000p-1, 0x1.0000000000000p-1 }, { 0x0.0p+0, 0x1.8000000000000p+1 } },
    { "Codac Interval_operations:323", { -0x1.0000000000000p+1, 0x1.8000000000000p+1 }, { 0x1.0000000000000p+0, 0x1.0000000000000p+1 }, { 0x0.0p+0, 0x1.2000000000000p+3 } },
    { "Codac Interval_operations:327", { -0x1.0000000000000p+1, 0x1.8000000000000p+1 }, { 0x1.8000000000000p+1, 0x1.8000000000000p+1 }, { -0x1.0000000000000p+3, 0x1.b000000000000p+4 } },
    { "Codac Interval_operations:329", { -0x1.0000000000000p+1, 0x1.8000000000000p+1 }, { 0x1.0000000000000p+1, 0x1.0000000000000p+1 }, { 0x0.0p+0, 0x1.2000000000000p+3 } },
    { "Codac Interval_operations:332", { 0x1.0000000000000p+2, 0x1.0000000000000p+2 }, { codac_oo, codac_oo }, codac_none },
    { "Codac Interval_operations:333", { 0x1.0000000000000p+2, 0x1.0000000000000p+2 }, { -codac_oo, -codac_oo }, codac_none },
    { "Codac Interval_operations:334", codac_none, { 0x1.8000000000000p+0, 0x1.8000000000000p+0 }, codac_none },
    // END powers
  };

  // The lower bound of pow(x, y) is not below the one of the powers of the part of x in [0, +oo]
  // (lb() >= 0 of Codac, where the part of the base below 0 is out of the domain)
  const codac_Binary codac_lower_bounds[] = {
    // BEGIN lower_bounds
    { "Codac Interval_operations:322", { -0x1.0000000000000p+2, 0x1.2000000000000p+3 }, { 0x1.0000000000000p-1, 0x1.0000000000000p-1 }, { 0x0.0p+0, 0x0.0p+0 } },
    // END lower_bounds
  };

  const codac_IntegerPower codac_roots[] = {
    // BEGIN roots
    { "Codac Interval_operations:336", { 0x0.0p+0, 0x1.0000000000000p+0 }, -1, { 0x1.0000000000000p+0, codac_oo } },
    { "Codac Interval_operations:337", { -0x1.b000000000000p+4, -0x1.0000000000000p+3 }, 3, { -0x1.8000000000000p+1, -0x1.0000000000000p+1 } },
    { "Codac Interval_operations:338", { -0x1.0000000000000p+2, 0x1.0000000000000p+0 }, 2, { 0x0.0p+0, 0x1.0000000000000p+0 } },
    { "Codac Interval_operations:339", { -0x1.0000000000000p+3, 0x1.0000000000000p+0 }, 3, { -0x1.0000000000000p+1, 0x1.0000000000000p+0 } },
    // END roots
  };

  void codac_powers_run()
  {
    for (const codac_Atan2& c : codac_atan2_cases) {
      const interval y = codac_interval(c.y), x = codac_interval(c.x);
      expect(std::string(c.name) + " atan2", atan2(y, x), c.ref, [&] { return "atan2(" + show(y) + ", " + show(x) + ")"; }, limit);
    }
    for (const codac_IntegerPower& c : codac_integer_powers) {
      const interval x = codac_interval(c.x);
      const std::string n = c.name;
      const auto describe = [&] { return "x = " + show(x) + ", n = " + std::to_string(c.n); };
      expect(n + " pow(x,n)", pow(x, c.n), c.ref, describe, limit);
      expect(n + " pow(-x,n)", pow(-x, c.n), (c.n % 2 == 0) ? c.ref : codac_negated(c.ref), describe, limit);
    }
    for (const codac_Binary& c : codac_powers) {
      const interval x = codac_interval(c.x), y = codac_interval(c.y);
      const std::string n = c.name;
      const auto describe = [&] { return "x = " + show(x) + ", y = " + show(y); };
      expect(n + " pow(x,[y])", pow(x, y), c.ref, describe, limit);
      if (c.y.lo == c.y.hi) {
        expect(n + " pow(x,y)", pow(x, c.y.lo), c.ref, describe, limit);
        if (std::fabs(c.y.lo) < 1e9 && c.y.lo == std::floor(c.y.lo)) {
          expect(n + " pow(x,int)", pow(x, static_cast<int>(c.y.lo)), c.ref, describe, limit);
        }
      }
    }
    for (const codac_Binary& c : codac_lower_bounds) {
      const interval x = codac_interval(c.x), y = codac_interval(c.y);
      const interval r = pow(x, y);
      expect(std::string(c.name) + " pow(x,[y]).left() is not below the exact lower bound", !r.is_empty() && r.left() >= c.ref.lo, true,
             [&] { return "pow(" + show(x) + ", " + show(y) + ") = " + show(r); });
    }
    for (const codac_IntegerPower& c : codac_roots) {
      const interval x = codac_interval(c.x);
      expect(std::string(c.name) + " nth_root", nth_root(x, c.n), c.ref, [&] { return "nth_root(" + show(x) + ", " + std::to_string(c.n) + ")"; }, limit);
    }
  }

  //---------------------------------------------------------------------------
  // (x + midpoint(x)) - midpoint(x) (line 243) holds x, whose exact value it is,
  // within two doubles of it: x + m is as large as twice the bounds of x, so that
  // rounding its bounds outward moves them by a double of the sum at most, two
  // doubles of x (the difference with m is exact for this x)
  //---------------------------------------------------------------------------
  struct codac_Cut
  {
    const char* name;
    Ref x;
    Ref ref;
  };

  // mid() of Codac, midpoint() of GAOL
  const codac_Cut codac_midpoints[] = {
    // BEGIN midpoints
    { "Codac Interval:235", { 0x0.0p+0, 0x1.0000000000000p+1 }, { 0x1.0000000000000p+0, 0x1.0000000000000p+0 } },
    { "Codac Interval:236", { -codac_oo, codac_oo }, { 0x0.0p+0, 0x0.0p+0 } },
    { "Codac Interval:237", { 0x0.0p+0, codac_oo }, { 0x1.fffffffffffffp+1023, 0x1.fffffffffffffp+1023 } },
    { "Codac Interval:238", { -codac_oo, 0x0.0p+0 }, { -0x1.fffffffffffffp+1023, -0x1.fffffffffffffp+1023 } },
    { "Codac Interval:240", { 0x1.fffffffffffffp+1023, codac_oo }, { 0x1.fffffffffffffp+1023, 0x1.fffffffffffffp+1023 } },
    { "Codac Interval:241", { 0x0.0p+0, 0x0.0p+0 }, { 0x0.0p+0, 0x0.0p+0 } },
    // END midpoints
  };

  const codac_Cut codac_midpoint_property[] = {
    // BEGIN midpoint_property
    { "Codac Interval:248", { 0x1.61a81239b7e0cp+1, 0x1.61a81239b7e1ap+1 }, { 0x1.61a81239b7e0cp+1, 0x1.61a81239b7e1ap+1 } },
    // END midpoint_property
  };

  // bisect(ratio): the two parts share the cut, a double of [l, r] that is within the doubles
  // around l + ratio (r - l); the parts of a half-line are cut at the largest double or its
  // opposite, those of the whole line at 0. Codac's bisect() takes a float: the ratio of
  // Codac:292 is 0.2f, 0x1.99999ap-3, not the double 0.2
  struct codac_Bisection
  {
    const char* name;
    Ref x;
    double ratio;
    Ref cut;
  };

  const codac_Bisection codac_bisections[] = {
    // BEGIN bisections
    { "Codac Interval:278", { -codac_oo, codac_oo }, 0x1.0000000000000p-1, { 0x0.0p+0, 0x0.0p+0 } },
    { "Codac Interval:282", { 0x0.0p+0, codac_oo }, 0x1.0000000000000p-1, { 0x1.fffffffffffffp+1023, 0x1.fffffffffffffp+1023 } },
    { "Codac Interval:287", { -codac_oo, 0x0.0p+0 }, 0x1.0000000000000p-1, { -0x1.fffffffffffffp+1023, -0x1.fffffffffffffp+1023 } },
    { "Codac Interval:292", { 0x0.0p+0, 0x1.4000000000000p+3 }, 0x1.99999a0000000p-3, { 0x1.0000004000000p+1, 0x1.0000004000000p+1 } },
    // END bisections
  };

  void codac_parts_run()
  {
    for (const codac_Cut& c : codac_midpoints) {
      const interval x = codac_interval(c.x);
      expect(std::string(c.name) + " midpoint()", x.midpoint(), c.ref, [&] { return "x = " + show(x); });
    }
    for (const codac_Cut& c : codac_midpoint_property) {
      const interval x = codac_interval(c.x);
      const double m = x.midpoint();
      const interval x2 = x + m, x3 = x2 - m;
      expect(std::string(c.name) + " (x+m)-m", x3, c.ref, [&] { return "x = " + show(x) + ", m = " + hex(m) + ", x+m = " + show(x2); }, 2);
    }
    for (const codac_Bisection& c : codac_bisections) {
      const interval x = codac_interval(c.x);
      const std::pair<interval, interval> parts = x.bisect(c.ratio);
      const std::string n = c.name;
      const auto describe = [&] {
        return "x = " + show(x) + ", ratio = " + hex(c.ratio) + ": " + show(parts.first) + " and " + show(parts.second);
      };
      expect(n + " first part, lower bound", parts.first.left(), Ref{ c.x.lo, c.x.lo }, describe);
      expect(n + " second part, upper bound", parts.second.right(), Ref{ c.x.hi, c.x.hi }, describe);
      expect(n + " first part, upper bound", parts.first.right(), c.cut, describe);
      expect(n + " second part, lower bound", parts.second.left(), c.cut, describe);
      expect(n + " the parts share the cut", parts.first.right() == parts.second.left(), true, describe);
    }
  }

  //---------------------------------------------------------------------------
  // The rounding test of Codac (lines 352 to 435), for what the tests of GAOL do
  // not hold: after every operation of its list the rounding direction is
  // upward, and the bounds of a product and a sum of random doubles, whose exact
  // results are no doubles, are the tightest ones. 41 of the 46 operations are
  // in the list of tests/rounding_direction.cpp, which makes these checks
  // after each of them, and more; here are sign, bisect and inflate, which it
  // does not hold (rand and the ternary chi are Codac's). The arithmetic on
  // random doubles and the 30 elementary points that tests/elementary_values.h
  // holds are in tests/arithmetic.cpp and tests/elementary.cpp
  //---------------------------------------------------------------------------
  struct codac_Operation
  {
    const char* name;
    void (*run)(const interval& x, const interval& y);
  };

  void codac_after_operations_run()
  {
    // Codac sets the direction upward once, with the initialization of oo
    gaol::round_upward();
    check("Codac Interval_rounding:357 the rounding direction is upward before any operation", std::fegetround() == FE_UPWARD);

    // x is within [0,1] and y above 1, the domain of every function
    const interval x(0.1, 0.3), y(1.5, 2.5);
    const codac_Operation operations[] = {
      { "Codac Interval_rounding:376 bisect(0.5)", [](const interval& a, const interval&) { (void)a.bisect(0.5); } },
      { "Codac Interval_rounding:377 inflate(0.1)", [](const interval& a, const interval&) { (void)a.inflate(0.1); } },
      { "Codac Interval_rounding:409 sign", [](const interval& a, const interval&) { (void)sign(a); } },
    };
    Random random;
    for (const codac_Operation& op : operations) {
      op.run(x, y);
      const std::string n = op.name;
      check(n + ": the rounding direction is upward after", std::fegetround() == FE_UPWARD);
      const double a = random(-30, 30), b = random(-30, 30);
      const interval p = interval(a) * interval(b), s = interval(a) + interval(b);
      check(n + ": [a]*[b] the tightest enclosure after", is_tightest_enclosure(p, exact(dyadic(a)*dyadic(b))),
            [&] { return "a = " + hex(a) + ", b = " + hex(b) + ": " + hex(p); });
      check(n + ": [a]+[b] the tightest enclosure after", is_tightest_enclosure(s, exact(dyadic(a) + dyadic(b))),
            [&] { return "a = " + hex(a) + ", b = " + hex(b) + ": " + hex(s); });
    }
  }

  void codac_run()
  {
    codac_constructors_run();
    codac_arithmetic_run();
    codac_sets_run();
    codac_accessors_run();
    codac_relations_run();
    codac_neighbours_run();
    codac_unary_run();
    codac_trigonometric_run();
    codac_tangent_arguments_run();
    codac_powers_run();
    codac_parts_run();
    codac_after_operations_run();
  }
}

int main()
{
  gaol::init();
  ibex_run();
  ibexb_run();
  codac_run();
  std::fesetround(FE_UPWARD);
  const int status = gaol_tests::summary(true);
  gaol::cleanup();
  return status;
}
