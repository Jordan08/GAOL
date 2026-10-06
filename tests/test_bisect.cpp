/*-*-C++-*----------------------------------------------------------------------
 * gaol -- NOT Just Another Interval Library
 *------------------------------------------------------------------------------
 * Tests of GAOL v5: bisect and is_bisectable (25c)
 *
 * Tests for the bisect(ratio) member function and is_bisectable() predicate.
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

  // Test basic bisect at 0.5 (should equal split)
  void test_bisect_half()
  {
    interval x(0.0, 1.0);
    auto halves = x.bisect(0.5);

    check("bisect([0,1], 0.5): first half is [0, 0.5]",
          halves.first.left() == 0.0 && halves.first.right() <= 0.5,
          [&] { return hex(halves.first); });

    check("bisect([0,1], 0.5): second half is [0.5, 1]",
          halves.second.left() >= 0.5 && halves.second.right() == 1.0,
          [&] { return hex(halves.second); });

    // bisect(x, 0.5) should equal split(x)
    interval left, right;
    x.split(left, right);
    check("bisect(x, 0.5) equals split(x)",
          halves.first == left && halves.second == right,
          [&] { return hex(halves.first) + ", " + hex(halves.second) + " vs " + hex(left) + ", " + hex(right); });
  }

  // Test bisect at different ratios
  void test_various_ratios()
  {
    interval x(0.0, 10.0);

    // bisect at 0.25
    auto halves25 = x.bisect(0.25);
    check("bisect([0,10], 0.25): first half is [0, 2.5]",
          halves25.first.left() == 0.0 && halves25.first.right() <= 2.5,
          [&] { return hex(halves25.first); });
    check("bisect([0,10], 0.25): second half is [2.5, 10]",
          halves25.second.left() >= 2.5 && halves25.second.right() == 10.0,
          [&] { return hex(halves25.second); });

    // bisect at 0.75
    auto halves75 = x.bisect(0.75);
    check("bisect([0,10], 0.75): first half is [0, 7.5]",
          halves75.first.left() == 0.0 && halves75.first.right() <= 7.5,
          [&] { return hex(halves75.first); });
    check("bisect([0,10], 0.75): second half is [7.5, 10]",
          halves75.second.left() >= 7.5 && halves75.second.right() == 10.0,
          [&] { return hex(halves75.second); });
  }

  // Test bisect at ratio 0
  void test_bisect_zero()
  {
    interval x(0.0, 1.0);
    auto halves = x.bisect(0.0);

    check("bisect(x, 0.0): first half is empty",
          halves.first.is_empty());
    check("bisect(x, 0.0): second half is x",
          halves.second == x,
          [&] { return hex(halves.second) + " vs " + hex(x); });
  }

  // Test bisect at ratio 1
  void test_bisect_one()
  {
    interval x(0.0, 1.0);
    auto halves = x.bisect(1.0);

    check("bisect(x, 1.0): first half is x",
          halves.first == x,
          [&] { return hex(halves.first) + " vs " + hex(x); });
    check("bisect(x, 1.0): second half is empty",
          halves.second.is_empty());
  }

  // Test bisect at ratio < 0
  void test_bisect_negative()
  {
    interval x(0.0, 1.0);
    auto halves = x.bisect(-0.5);

    check("bisect(x, -0.5): first half is empty",
          halves.first.is_empty());
    check("bisect(x, -0.5): second half is x",
          halves.second == x,
          [&] { return hex(halves.second) + " vs " + hex(x); });
  }

  // Test bisect at ratio > 1
  void test_bisect_greater_than_one()
  {
    interval x(0.0, 1.0);
    auto halves = x.bisect(1.5);

    check("bisect(x, 1.5): first half is x",
          halves.first == x,
          [&] { return hex(halves.first) + " vs " + hex(x); });
    check("bisect(x, 1.5): second half is empty",
          halves.second.is_empty());
  }

  // Test bisect of empty interval
  void test_bisect_empty()
  {
    interval empty = interval::emptyset();
    auto halves = empty.bisect(0.5);

    check("bisect(empty, 0.5): both halves are empty",
          halves.first.is_empty() && halves.second.is_empty());
  }

  // Test bisect with NaN ratio
  void test_bisect_nan()
  {
    interval x(0.0, 1.0);
    auto halves = x.bisect(GAOL_NAN);

    check("bisect(x, NaN): both halves are empty",
          halves.first.is_empty() && halves.second.is_empty());
  }

  // Test bisect of point interval
  void test_bisect_point()
  {
    interval x(1.0, 1.0);
    auto halves = x.bisect(0.5);

    check("bisect([1,1], 0.5): first half is [1,1]",
          halves.first == x,
          [&] { return hex(halves.first) + " vs " + hex(x); });
    check("bisect([1,1], 0.5): second half is [1,1]",
          halves.second == x,
          [&] { return hex(halves.second) + " vs " + hex(x); });
  }

  // Test bisect of unbounded interval
  void test_bisect_unbounded()
  {
    interval x(-oo, 1.0);
    auto halves = x.bisect(0.5);

    check("bisect([-oo,1], 0.5): first half left is -oo",
          halves.first.left() == -oo);
    check("bisect([-oo,1], 0.5): second half right is 1.0",
          halves.second.right() == 1.0);

    interval x2(1.0, oo);
    auto halves2 = x2.bisect(0.5);

    check("bisect([1,oo], 0.5): first half left is 1.0",
          halves2.first.left() == 1.0);
    check("bisect([1,oo], 0.5): second half right is oo",
          halves2.second.right() == oo);
  }

  // Test is_bisectable
  void test_is_bisectable()
  {
    interval empty = interval::emptyset();
    interval point(1.0, 1.0);
    interval normal(0.0, 1.0);
    interval unbounded(-oo, 1.0);

    check("is_bisectable(empty) is false",
          !empty.is_bisectable());
    check("is_bisectable([1,1]) is false",
          !point.is_bisectable());
    check("is_bisectable([0,1]) is true",
          normal.is_bisectable());
    check("is_bisectable([-oo,1]) is true",
          unbounded.is_bisectable());
  }

  // Test that bisect shares the cut point
  void test_shared_cut_point()
  {
    interval x(0.0, 1.0);
    auto halves = x.bisect(0.5);

    // Both halves should share the cut point
    check("bisect shares cut point: first.right() == second.left()",
          halves.first.right() == halves.second.left(),
          [&] { return hex(halves.first.right()) + " vs " + hex(halves.second.left()); });
  }

  // Test bisect with negative interval
  void test_bisect_negative()
  {
    interval x(-2.0, -1.0);
    auto halves = x.bisect(0.5);

    check("bisect([-2,-1], 0.5): first half is [-2, -1.5]",
          halves.first.left() == -2.0 && halves.first.right() <= -1.5,
          [&] { return hex(halves.first); });
    check("bisect([-2,-1], 0.5): second half is [-1.5, -1]",
          halves.second.left() >= -1.5 && halves.second.right() == -1.0,
          [&] { return hex(halves.second); });
  }

} // namespace

int main()
{
  test_bisect_half();
  test_various_ratios();
  test_bisect_zero();
  test_bisect_one();
  test_bisect_negative();
  test_bisect_greater_than_one();
  test_bisect_empty();
  test_bisect_nan();
  test_bisect_point();
  test_bisect_unbounded();
  test_is_bisectable();
  test_shared_cut_point();
  test_bisect_negative();

  return summary();
}
