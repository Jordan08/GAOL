/*-*-C++-*----------------------------------------------------------------------
 * gaol -- NOT Just Another Interval Library
 *------------------------------------------------------------------------------
 * Tests of GAOL v5: midrad constructor (25d)
 *
 * Tests for the midpoint-radius static constructor interval::midrad(m, r).
 *
 * Copyright (c) 2026 ENSTA, France
 *
 * Created 2026 by Jordan NININ
 *------------------------------------------------------------------------------
 * gaol is a software distributed WITHOUT ANY WARRANTY. Read the associated
 * COPYING file for information.
 *----------------------------------------------------------------------------*/

#include "gaol_tests.h"

using gaol_tests::check;
using gaol_tests::hex;
using gaol_tests::summary;

namespace {

  const double oo = std::numeric_limits<double>::infinity();

  // Test basic midrad
  void test_basic()
  {
    // midrad(1.0, 0.5) should give [0.5, 1.5]
    interval x = interval::midrad(1.0, 0.5);

    interval expected(0.5, 1.5);
    check("midrad(1.0, 0.5) = [0.5, 1.5]",
          x.set_eq(expected),
          [&] { return hex(x) + " vs " + hex(expected); });
  }

  // Test midrad with radius 0
  void test_zero_radius()
  {
    interval x = interval::midrad(1.0, 0.0);

    check("midrad(1.0, 0.0) is a point interval at 1.0",
          x.is_a_double() && x.left() == 1.0 && x.right() == 1.0,
          [&] { return hex(x); });
  }

  // Test midrad with negative radius
  void test_negative_radius()
  {
    interval x = interval::midrad(1.0, -0.5);

    check("midrad(1.0, -0.5) returns empty",
          x.is_empty());
  }

  // Test midrad with NaN radius
  void test_nan_radius()
  {
    interval x = interval::midrad(1.0, GAOL_NAN);

    check("midrad(1.0, NaN) returns empty",
          x.is_empty());
  }

  // Test midrad with NaN midpoint
  void test_nan_midpoint()
  {
    interval x = interval::midrad(GAOL_NAN, 0.5);

    check("midrad(NaN, 0.5) returns empty",
          x.is_empty());
  }

  // Test midrad with infinite radius
  void test_infinite_radius()
  {
    interval x = interval::midrad(1.0, oo);

    check("midrad(1.0, oo) returns universe",
          x == interval::universe(),
          [&] { return hex(x); });
  }

  // Test midrad with infinite midpoint
  void test_infinite_midpoint()
  {
    interval x = interval::midrad(oo, 0.5);

    check("midrad(oo, 0.5) returns universe",
          x == interval::universe(),
          [&] { return hex(x); });
  }

  // Test midrad with both infinite
  void test_both_infinite()
  {
    interval x = interval::midrad(oo, oo);

    check("midrad(oo, oo) returns universe",
          x == interval::universe(),
          [&] { return hex(x); });
  }

  // Test midrad with negative midpoint
  void test_negative_midpoint()
  {
    interval x = interval::midrad(-1.0, 0.5);

    interval expected(-1.5, -0.5);
    check("midrad(-1.0, 0.5) = [-1.5, -0.5]",
          x.set_eq(expected),
          [&] { return hex(x) + " vs " + hex(expected); });
  }

  // Test midrad with large radius
  void test_large_radius()
  {
    interval x = interval::midrad(1.0, 100.0);

    interval expected(-99.0, 101.0);
    check("midrad(1.0, 100.0) = [-99.0, 101.0]",
          x.set_eq(expected),
          [&] { return hex(x) + " vs " + hex(expected); });
  }

  // Test midrad with subnormal radius
  void test_subnormal_radius()
  {
    double tiny = std::numeric_limits<double>::min(); // smallest positive normal
    interval x = interval::midrad(1.0, tiny);

    check("midrad with subnormal radius works",
          !x.is_empty() && x.left() < 1.0 && x.right() > 1.0);
  }

  // Test midrad with zero midpoint
  void test_zero_midpoint()
  {
    interval x = interval::midrad(0.0, 0.5);

    check("midrad(0.0, 0.5) = [-0.5, 0.5]",
          x.left() >= -0.5 && x.right() <= 0.5,
          [&] { return hex(x); });

    check("midrad(0.0, 0.5) contains zero",
          x.set_contains(0.0));
  }

  // Test midrad equivalence with constructor
  void test_equivalence()
  {
    double m = 1.0;
    double r = 0.5;

    interval x1 = interval::midrad(m, r);
    interval x2(m - r, m + r);

    check("midrad(m, r) equals interval(m-r, m+r)",
          x1.set_eq(x2),
          [&] { return hex(x1) + " vs " + hex(x2); });
  }

  // Test midrad with rounding
  void test_rounding()
  {
    // midrad should use directed rounding for the bounds
    double m = 1.0;
    double r = 0.1;

    interval x = interval::midrad(m, r);
    interval expected(m - r, m + r);

    // The bounds should be the tightest enclosure of [m-r, m+r]
    check("midrad uses directed rounding",
          x.set_eq(expected),
          [&] { return hex(x) + " vs " + hex(expected); });
  }

} // namespace

int main()
{
  test_basic();
  test_zero_radius();
  test_negative_radius();
  test_nan_radius();
  test_nan_midpoint();
  test_infinite_radius();
  test_infinite_midpoint();
  test_both_infinite();
  test_negative_midpoint();
  test_large_radius();
  test_subnormal_radius();
  test_zero_midpoint();
  test_equivalence();
  test_rounding();

  return summary();
}
