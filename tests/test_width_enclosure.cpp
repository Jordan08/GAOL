/*-*-C++-*----------------------------------------------------------------------
 * gaol -- NOT Just Another Interval Library
 *------------------------------------------------------------------------------
 * Tests of GAOL v5: width_enclosure (25f)
 *
 * Tests for the width_enclosure() member function.
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

  // Test basic width_enclosure
  void test_basic()
  {
    interval x(0.0, 1.0);
    interval we = x.width_enclosure();

    // width of [0,1] is 1, so width_enclosure should be [1, 1]
    check("width_enclosure([0,1]) = [1, 1]",
          we.is_a_double() && we.left() == 1.0 && we.right() == 1.0,
          [&] { return hex(we); });

    // The upper bound of the enclosure should equal width()
    check("width_enclosure().right() == width()",
          we.right() == x.width(),
          [&] { return hex(we.right()) + " vs " + hex(x.width()); });
  }

  // Test width_enclosure of empty interval
  void test_empty()
  {
    interval empty = interval::emptyset();
    interval we = empty.width_enclosure();

    check("width_enclosure(empty) = empty",
          we.is_empty());
  }

  // Test width_enclosure of point interval
  void test_point()
  {
    interval x(1.0, 1.0);
    interval we = x.width_enclosure();

    // width of [1,1] is 0
    check("width_enclosure([1,1]) = [0, 0]",
          we.is_a_double() && we.left() == 0.0 && we.right() == 0.0,
          [&] { return hex(we); });
  }

  // Test width_enclosure of unbounded interval
  void test_unbounded()
  {
    interval x(-oo, 1.0);
    interval we = x.width_enclosure();

    check("width_enclosure([-oo,1]) = oo",
          we.left() == oo && we.right() == oo,
          [&] { return hex(we); });

    interval x2(1.0, oo);
    interval we2 = x2.width_enclosure();

    check("width_enclosure([1,oo]) = oo",
          we2.left() == oo && we2.right() == oo,
          [&] { return hex(we2); });

    interval x3(-oo, oo);
    interval we3 = x3.width_enclosure();

    check("width_enclosure([-oo,oo]) = oo",
          we3.left() == oo && we3.right() == oo,
          [&] { return hex(we3); });
  }

  // Test width_enclosure with negative interval
  void test_negative()
  {
    interval x(-2.0, -1.0);
    interval we = x.width_enclosure();

    // width of [-2,-1] is 1
    check("width_enclosure([-2,-1]) = [1, 1]",
          we.is_a_double() && we.left() == 1.0 && we.right() == 1.0,
          [&] { return hex(we); });
  }

  // Test width_enclosure with interval crossing zero
  void test_crossing_zero()
  {
    interval x(-1.0, 1.0);
    interval we = x.width_enclosure();

    // width of [-1,1] is 2
    check("width_enclosure([-1,1]) = [2, 2]",
          we.is_a_double() && we.left() == 2.0 && we.right() == 2.0,
          [&] { return hex(we); });
  }

  // Test width_enclosure with large interval
  void test_large()
  {
    interval x(0.0, 1e300);
    interval we = x.width_enclosure();

    check("width_enclosure([0,1e300]) upper bound equals width()",
          we.right() == x.width(),
          [&] { return hex(we.right()) + " vs " + hex(x.width()); });
  }

  // Test width_enclosure with subnormal width
  void test_subnormal()
  {
    // Create an interval with subnormal width
    double a = 1.0;
    double b = a + std::numeric_limits<double>::min();
    interval x(a, b);
    interval we = x.width_enclosure();

    check("width_enclosure with subnormal width works",
          !we.is_empty() && we.right() == x.width(),
          [&] { return hex(we) + " vs width " + hex(x.width()); });
  }

  // Test width_enclosure is an enclosure and minimal
  void test_is_enclosure()
  {
    interval x(0.0, 1.0);
    interval we = x.width_enclosure();
    double exact_width = 1.0;

    // The enclosure should contain the exact width
    check("width_enclosure contains exact width",
          we.left() <= exact_width && we.right() >= exact_width,
          [&] { return hex(we) + " vs exact " + std::to_string(exact_width); });

    // Test minimality: we should be the tightest enclosure
    // For width = 1.0, the tightest enclosure is [1.0, 1.0]
    check("width_enclosure is minimal for width=1.0",
          we.left() == 1.0 && we.right() == 1.0,
          [&] { return hex(we); });
  }

  // Test width_enclosure with very narrow interval
  void test_narrow()
  {
    double a = 1.0;
    double b = std::nextafter(a, a + 1.0);
    interval x(a, b);
    interval we = x.width_enclosure();

    check("width_enclosure of narrow interval works",
          !we.is_empty() && we.right() == x.width(),
          [&] { return hex(we) + " vs width " + hex(x.width()); });
  }

  // Test width_enclosure with interval of width 0 (point)
  void test_width_zero()
  {
    interval x(5.0, 5.0);
    interval we = x.width_enclosure();

    check("width_enclosure of point interval is [0,0]",
          we.left() == 0.0 && we.right() == 0.0,
          [&] { return hex(we); });
  }

  // Test width_enclosure upper bound equals width and minimality
  void test_upper_bound_equals_width()
  {
    // Test with various intervals
    struct TestCase {
      interval x;
      double expected_width;
    };
    TestCase cases[] = {
      {interval(0.0, 1.0), 1.0},
      {interval(-1.0, 1.0), 2.0},
      {interval(10.0, 20.0), 10.0},
      {interval(-100.0, -50.0), 50.0},
      {interval(0.0, 0.5), 0.5}
    };

    for (const auto& tc : cases) {
      interval we = tc.x.width_enclosure();
      check("width_enclosure().right() == width() for " + gaol_tests::hex(tc.x),
            we.right() == tc.x.width(),
            [&] { return hex(we.right()) + " vs " + hex(tc.x.width()); });
      
      // Test minimality
      check("width_enclosure lower bound is the greatest double <= width for " + gaol_tests::hex(tc.x),
            we.left() <= tc.expected_width &&
            (we.left() == tc.expected_width || 
             std::nextafter(we.left(), std::numeric_limits<double>::infinity()) > tc.expected_width),
            [&] { return hex(we.left()) + " vs " + std::to_string(tc.expected_width); });
      
      check("width_enclosure upper bound is the smallest double >= width for " + gaol_tests::hex(tc.x),
            we.right() >= tc.expected_width &&
            (we.right() == tc.expected_width || 
             std::nextafter(we.right(), -std::numeric_limits<double>::infinity()) < tc.expected_width),
            [&] { return hex(we.right()) + " vs " + std::to_string(tc.expected_width); });
    }
  }

} // namespace

int main()
{
  test_basic();
  test_empty();
  test_point();
  test_unbounded();
  test_negative();
  test_crossing_zero();
  test_large();
  test_subnormal();
  test_is_enclosure();
  test_narrow();
  test_width_zero();
  test_upper_bound_equals_width();

  return summary();
}
