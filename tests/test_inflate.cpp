/*-*-C++-*----------------------------------------------------------------------
 * gaol -- NOT Just Another Interval Library
 *------------------------------------------------------------------------------
 * Tests of GAOL v5: inflate (25b)
 *
 * Tests for the inflate function which widens an interval by an absolute radius.
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

  // Test basic inflate
  void test_basic()
  {
    interval x(1.0, 2.0);
    interval inflated = x.inflate(0.5);

    // [1, 2] inflated by 0.5 should be [0.5, 2.5]
    check("inflate([1,2], 0.5) = [0.5, 2.5]",
          inflated.left() >= 0.5 && inflated.right() <= 2.5,
          [&] { return hex(inflated); });

    // Check that it equals x + [-0.5, 0.5]
    interval expected = x + interval(-0.5, 0.5);
    check("inflate(x, 0.5) equals x + [-0.5, 0.5]",
          inflated == expected,
          [&] { return hex(inflated) + " vs " + hex(expected); });
  }

  // Test inflate with radius 0
  void test_zero_radius()
  {
    interval x(1.0, 2.0);
    interval inflated = x.inflate(0.0);

    check("inflate(x, 0) returns x",
          inflated == x,
          [&] { return hex(inflated) + " vs " + hex(x); });
  }

  // Test inflate of empty interval
  void test_empty()
  {
    interval empty = interval::emptyset();
    interval inflated = empty.inflate(0.5);

    check("inflate(empty, 0.5) returns empty",
          inflated.is_empty());
  }

  // Test inflate with negative radius
  void test_negative_radius()
  {
    interval x(1.0, 2.0);
    interval inflated = x.inflate(-0.5);

    check("inflate(x, -0.5) returns empty",
          inflated.is_empty());
  }

  // Test inflate with NaN radius
  void test_nan_radius()
  {
    interval x(1.0, 2.0);
    interval inflated = x.inflate(GAOL_NAN);

    check("inflate(x, NaN) returns empty",
          inflated.is_empty());
  }

  // Test inflate with infinite radius
  void test_infinite_radius()
  {
    interval x(1.0, 2.0);
    interval inflated = x.inflate(oo);

    check("inflate(x, oo) returns universe",
          inflated == interval::universe(),
          [&] { return hex(inflated); });
  }

  // Test inflate of point interval
  void test_point_interval()
  {
    interval x(1.0, 1.0);
    interval inflated = x.inflate(0.5);

    check("inflate([1,1], 0.5) = [0.5, 1.5]",
          inflated.left() >= 0.5 && inflated.right() <= 1.5,
          [&] { return hex(inflated); });
  }

  // Test inflate of unbounded interval
  void test_unbounded()
  {
    interval x(-oo, 1.0);
    interval inflated = x.inflate(0.5);

    check("inflate([-oo,1], 0.5) left bound is -oo",
          inflated.left() == -oo);
    check("inflate([-oo,1], 0.5) right bound is 1.5",
          inflated.right() <= 1.5);

    interval x2(1.0, oo);
    interval inflated2 = x2.inflate(0.5);

    check("inflate([1,oo], 0.5) left bound is 0.5",
          inflated2.left() >= 0.5);
    check("inflate([1,oo], 0.5) right bound is oo",
          inflated2.right() == oo);

    interval x3(-oo, oo);
    interval inflated3 = x3.inflate(0.5);

    check("inflate([-oo,oo], 0.5) is universe",
          inflated3 == interval::universe());
  }

  // Test inflate with very small radius (subnormal)
  void test_subnormal_radius()
  {
    interval x(1.0, 2.0);
    double tiny = std::numeric_limits<double>::min(); // smallest positive normal
    interval inflated = x.inflate(tiny);

    check("inflate with subnormal radius works",
          !inflated.is_empty() && inflated.left() < 1.0 && inflated.right() > 2.0);
  }

  // Test inflate preserves directed rounding
  void test_rounding()
  {
    // inflate(x, r) should be x + [-r, r] with directed rounding
    interval x(1.0, 2.0);
    double r = 0.1;

    interval inflated = x.inflate(r);
    interval expected = x + interval(-r, r);

    check("inflate uses directed rounding same as x + [-r, r]",
          inflated == expected,
          [&] { return hex(inflated) + " vs " + hex(expected); });
  }

  // Test inflate of negative interval
  void test_negative_interval()
  {
    interval x(-2.0, -1.0);
    interval inflated = x.inflate(0.5);

    // [-2, -1] inflated by 0.5 should be [-2.5, -0.5]
    check("inflate([-2,-1], 0.5) = [-2.5, -0.5]",
          inflated.left() >= -2.5 && inflated.right() <= -0.5,
          [&] { return hex(inflated); });
  }

  // Test inflate crossing zero
  void test_crossing_zero()
  {
    interval x(-0.5, 0.5);
    interval inflated = x.inflate(1.0);

    // [-0.5, 0.5] inflated by 1.0 should be [-1.5, 1.5]
    check("inflate([-0.5,0.5], 1.0) = [-1.5, 1.5]",
          inflated.left() >= -1.5 && inflated.right() <= 1.5,
          [&] { return hex(inflated); });

    check("inflated interval contains zero",
          inflated.set_contains(0.0));
  }

} // namespace

int main()
{
  test_basic();
  test_zero_radius();
  test_empty();
  test_negative_radius();
  test_nan_radius();
  test_infinite_radius();
  test_point_interval();
  test_unbounded();
  test_subnormal_radius();
  test_rounding();
  test_negative_interval();
  test_crossing_zero();

  return summary();
}
