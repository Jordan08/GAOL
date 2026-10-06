/*-*-C++-*----------------------------------------------------------------------
 * gaol -- NOT Just Another Interval Library
 *------------------------------------------------------------------------------
 * Tests of GAOL v5: mulRevToPair (25a)
 *
 * Tests for the IEEE 1788-2015 two-output division mulRevToPair(b, c) which
 * returns the solution set {x : b*x in c} as the union of two intervals.
 *
 * Copyright (c) 2026 ENSTA, France
 *
 * Created 2026 by Jordan NININ
 *------------------------------------------------------------------------------
 * gaol is a software distributed WITHOUT ANY WARRANTY. Read the associated
 * COPYING file for information.
 *----------------------------------------------------------------------------*/

#include "gaol_tests.h"
#include "gaol/gaol_ieee1788.h"

using namespace gaol_ieee1788;
using gaol_tests::check;
using gaol_tests::hex;
using gaol_tests::summary;

namespace {

  const double oo = std::numeric_limits<double>::infinity();

  // Test that the hull of the two pieces equals b % c
  void test_hull_equals_div_rel()
  {
    // Basic case: b does not contain 0
    {
      interval b(1.0, 2.0);
      interval c(2.0, 4.0);
      auto pieces = mulRevToPair(b, c);
      interval hull_result = pieces.first | pieces.second;
      interval div_result = ::gaol_core::div_rel(c, b, interval::universe());
      check("hull of pieces equals div_rel when b does not contain 0",
            hull_result.set_eq(div_result),
            [&] { return hex(hull_result) + " vs " + hex(div_result); });
    }

    // Case: b contains 0, c contains 0 -> connected solution
    {
      interval b(-1.0, 1.0);
      interval c(-2.0, 2.0);
      auto pieces = mulRevToPair(b, c);
      interval hull_result = pieces.first | pieces.second;
      interval div_result = ::gaol_core::div_rel(c, b, interval::universe());
      check("hull of pieces equals div_rel when 0 in b and 0 in c",
            hull_result.set_eq(div_result),
            [&] { return hex(hull_result) + " vs " + hex(div_result); });
    }

    // Case: b contains 0, c does not contain 0 -> disconnected solution
    {
      interval b(-1.0, 1.0);
      interval c(2.0, 4.0);
      auto pieces = mulRevToPair(b, c);
      interval hull_result = pieces.first | pieces.second;
      interval div_result = ::gaol_core::div_rel(c, b, interval::universe());
      check("hull of pieces equals div_rel when 0 in b and 0 not in c",
            hull_result.set_eq(div_result),
            [&] { return hex(hull_result) + " vs " + hex(div_result); });
    }
  }

  // Test empty inputs
  void test_empty_inputs()
  {
    interval empty = interval::emptyset();
    interval non_empty(1.0, 2.0);

    auto result1 = mulRevToPair(empty, non_empty);
    check("mulRevToPair(empty, non_empty) returns (empty, empty)",
          result1.first.is_empty() && result1.second.is_empty());

    auto result2 = mulRevToPair(non_empty, empty);
    check("mulRevToPair(non_empty, empty) returns (empty, empty)",
          result2.first.is_empty() && result2.second.is_empty());

    auto result3 = mulRevToPair(empty, empty);
    check("mulRevToPair(empty, empty) returns (empty, empty)",
          result3.first.is_empty() && result3.second.is_empty());
  }

  // Test with 0 in b and 0 not in c (disconnected case)
  void test_disconnected()
  {
    // b = [-1, 1], c = [2, 4]
    // Solution: x >= 2 (for positive b) and x <= -2 (for negative b)
    {
      interval b(-1.0, 1.0);
      interval c(2.0, 4.0);
      auto pieces = mulRevToPair(b, c);

      check("disconnected: first piece is non-empty (positive part)",
            !pieces.first.is_empty());
      check("disconnected: second piece is non-empty (negative part)",
            !pieces.second.is_empty());

      // The first piece should be [2, +oo) - the tightest enclosure of c / b_pos
      interval b_pos = b & interval(0.0, interval::universe().right());
      interval expected_pos = ::gaol_core::div_rel(c, b_pos, interval::universe());
      check("disconnected: first piece equals div_rel(c, b_pos, entire())",
            pieces.first.set_eq(expected_pos),
            [&] { return hex(pieces.first) + " vs " + hex(expected_pos); });
      
      // The second piece should be (-oo, -2] - the tightest enclosure of c / b_neg
      interval b_neg = b & interval(interval::universe().left(), 0.0);
      interval expected_neg = ::gaol_core::div_rel(c, b_neg, interval::universe());
      check("disconnected: second piece equals div_rel(c, b_neg, entire())",
            pieces.second.set_eq(expected_neg),
            [&] { return hex(pieces.second) + " vs " + hex(expected_neg); });
    }

    // b = [-2, 2], c = [1, 3]
    {
      interval b(-2.0, 2.0);
      interval c(1.0, 3.0);
      auto pieces = mulRevToPair(b, c);

      check("disconnected b=[-2,2], c=[1,3]: both pieces non-empty",
            !pieces.first.is_empty() && !pieces.second.is_empty());
    }
  }

  // Test with 0 in b and 0 in c (connected case)
  void test_connected_with_zero()
  {
    interval b(-1.0, 1.0);
    interval c(-2.0, 2.0);
    auto pieces = mulRevToPair(b, c);

    check("connected with 0 in both: first piece non-empty",
          !pieces.first.is_empty());
    check("connected with 0 in both: second piece is empty",
          pieces.second.is_empty());

    // The solution should be entire() since any x satisfies b*x in c
    // when b contains 0 and c contains 0
    check("connected with 0 in both: solution is unbounded",
          pieces.first == interval::universe());
  }

  // Test with b strictly positive
  void test_positive_b()
  {
    interval b(1.0, 2.0);
    interval c(2.0, 4.0);
    auto pieces = mulRevToPair(b, c);

    check("positive b: first piece non-empty",
          !pieces.first.is_empty());
    check("positive b: second piece is empty",
          pieces.second.is_empty());

    // x = c / b, so for b=[1,2] and c=[2,4], x should be [1,4]
    interval expected = ::gaol_core::div_rel(c, b, interval::universe());
    check("positive b: result equals div_rel(c, b, entire())",
          pieces.first.set_eq(expected),
          [&] { return hex(pieces.first) + " vs " + hex(expected); });
  }

  // Test with b strictly negative
  void test_negative_b()
  {
    interval b(-2.0, -1.0);
    interval c(2.0, 4.0);
    auto pieces = mulRevToPair(b, c);

    check("negative b: first piece is empty",
          pieces.first.is_empty());
    check("negative b: second piece non-empty",
          !pieces.second.is_empty());

    // x = c / b where b < 0, so for b=[-2,-1] and c=[2,4], x should be [-4,-1]
    interval expected = ::gaol_core::div_rel(c, b, interval::universe());
    check("negative b: second piece equals div_rel(c, b, entire())",
          pieces.second.set_eq(expected),
          [&] { return hex(pieces.second) + " vs " + hex(expected); });
  }

  // Test with b = [0, 0]
  void test_b_zero()
  {
    interval b(0.0, 0.0);
    interval c(1.0, 2.0);
    auto pieces = mulRevToPair(b, c);

    // 0*x = 0, which is not in [1,2], so solution is empty
    check("b = [0,0], c = [1,2]: both pieces empty",
          pieces.first.is_empty() && pieces.second.is_empty());
  }

  // Test with b = [0, 0] and c contains 0
  void test_b_zero_c_contains_zero()
  {
    interval b(0.0, 0.0);
    interval c(-1.0, 1.0);
    auto pieces = mulRevToPair(b, c);

    // 0*x = 0, which is in [-1,1], so any x is a solution
    check("b = [0,0], c contains 0: solution is universe",
          pieces.first == interval::universe() && pieces.second.is_empty());
  }

  // Test with infinite bounds
  void test_infinite_bounds()
  {
    interval b(1.0, oo);
    interval c(2.0, oo);
    auto pieces = mulRevToPair(b, c);

    check("b = [1,oo], c = [2,oo]: first piece non-empty",
          !pieces.first.is_empty());
    check("b = [1,oo], c = [2,oo]: second piece empty",
          pieces.second.is_empty());
  }

  // Test with b = entire()
  void test_b_entire()
  {
    interval b = interval::universe();
    interval c(1.0, 2.0);
    auto pieces = mulRevToPair(b, c);

    // entire() * x = c has solution x = c / entire() = entire()
    check("b = entire(), c = [1,2]: solution is universe",
          pieces.first == interval::universe() && pieces.second.is_empty());
  }

  // Test with c = entire()
  void test_c_entire()
  {
    interval b(1.0, 2.0);
    interval c = interval::universe();
    auto pieces = mulRevToPair(b, c);

    // b * x in entire() is always true for any x
    check("b = [1,2], c = entire(): solution is universe",
          pieces.first == interval::universe() && pieces.second.is_empty());
  }

  // Test symmetry: mulRevToPair(b, c) should be related to mulRevToPair(c, b)
  void test_symmetry()
  {
    interval b(1.0, 2.0);
    interval c(2.0, 4.0);
    auto pieces_bc = mulRevToPair(b, c);
    auto pieces_cb = mulRevToPair(c, b);

    // For b=[1,2], c=[2,4]: b*x in c means x in [1,4]
    // For c=[2,4], b=[1,2]: c*x in b means x in [0.5, 2]
    // These are reciprocals of each other
    check("mulRevToPair symmetry: pieces are different",
          pieces_bc.first != pieces_cb.first);
  }

} // namespace

int main()
{
  test_hull_equals_div_rel();
  test_empty_inputs();
  test_disconnected();
  test_connected_with_zero();
  test_positive_b();
  test_negative_b();
  test_b_zero();
  test_b_zero_c_contains_zero();
  test_infinite_bounds();
  test_b_entire();
  test_c_entire();
  test_symmetry();

  return summary();
}
