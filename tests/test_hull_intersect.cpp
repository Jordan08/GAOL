/*-*-C++-*----------------------------------------------------------------------
 * gaol -- NOT Just Another Interval Library
 *------------------------------------------------------------------------------
 * Tests of GAOL v5: hull and intersect free functions (25e)
 *
 * Tests for the free functions hull(a,b) and intersect(a,b).
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

  // Test basic hull
  void test_hull_basic()
  {
    interval a(0.0, 1.0);
    interval b(2.0, 3.0);
    interval h = gaol_core::hull(a, b);

    check("hull([0,1], [2,3]) = [0, 3]",
          h.left() == 0.0 && h.right() == 3.0,
          [&] { return hex(h); });

    // hull should equal a | b
    interval h_operator = a | b;
    check("hull(a, b) equals a | b",
          h == h_operator,
          [&] { return hex(h) + " vs " + hex(h_operator); });
  }

  // Test hull with overlapping intervals
  void test_hull_overlapping()
  {
    interval a(0.0, 2.0);
    interval b(1.0, 3.0);
    interval h = gaol_core::hull(a, b);

    check("hull([0,2], [1,3]) = [0, 3]",
          h.left() == 0.0 && h.right() == 3.0,
          [&] { return hex(h); });
  }

  // Test hull with nested intervals
  void test_hull_nested()
  {
    interval a(0.0, 3.0);
    interval b(1.0, 2.0);
    interval h = gaol_core::hull(a, b);

    check("hull([0,3], [1,2]) = [0, 3]",
          h == a,
          [&] { return hex(h) + " vs " + hex(a); });
  }

  // Test hull with empty interval
  void test_hull_empty()
  {
    interval a(0.0, 1.0);
    interval empty = interval::emptyset();
    interval h = gaol_core::hull(a, empty);

    check("hull([0,1], empty) = [0, 1]",
          h == a,
          [&] { return hex(h) + " vs " + hex(a); });

    interval h2 = gaol_core::hull(empty, a);
    check("hull(empty, [0,1]) = [0, 1]",
          h2 == a,
          [&] { return hex(h2) + " vs " + hex(a); });

    interval h3 = gaol_core::hull(empty, empty);
    check("hull(empty, empty) = empty",
          h3.is_empty());
  }

  // Test hull with unbounded intervals
  void test_hull_unbounded()
  {
    interval a(-oo, 1.0);
    interval b(0.0, oo);
    interval h = gaol_core::hull(a, b);

    check("hull([-oo,1], [0,oo]) = universe",
          h == interval::universe(),
          [&] { return hex(h); });

    interval a2(-oo, 0.0);
    interval b2(1.0, oo);
    interval h2 = gaol_core::hull(a2, b2);

    check("hull([-oo,0], [1,oo]) = universe",
          h2 == interval::universe(),
          [&] { return hex(h2); });
  }

  // Test hull commutativity
  void test_hull_commutative()
  {
    interval a(0.0, 1.0);
    interval b(2.0, 3.0);

    interval h1 = gaol_core::hull(a, b);
    interval h2 = gaol_core::hull(b, a);

    check("hull is commutative: hull(a, b) == hull(b, a)",
          h1 == h2,
          [&] { return hex(h1) + " vs " + hex(h2); });
  }

  // Test hull associativity
  void test_hull_associative()
  {
    interval a(0.0, 1.0);
    interval b(2.0, 3.0);
    interval c(4.0, 5.0);

    interval h1 = gaol_core::hull(gaol_core::hull(a, b), c);
    interval h2 = gaol_core::hull(a, gaol_core::hull(b, c));

    check("hull is associative: hull(hull(a,b),c) == hull(a,hull(b,c))",
          h1 == h2,
          [&] { return hex(h1) + " vs " + hex(h2); });
  }

  // Test basic intersect
  void test_intersect_basic()
  {
    interval a(0.0, 2.0);
    interval b(1.0, 3.0);
    interval i = gaol_core::intersect(a, b);

    check("intersect([0,2], [1,3]) = [1, 2]",
          i.left() == 1.0 && i.right() == 2.0,
          [&] { return hex(i); });

    // intersect should equal a & b
    interval i_operator = a & b;
    check("intersect(a, b) equals a & b",
          i == i_operator,
          [&] { return hex(i) + " vs " + hex(i_operator); });
  }

  // Test intersect with disjoint intervals
  void test_intersect_disjoint()
  {
    interval a(0.0, 1.0);
    interval b(2.0, 3.0);
    interval i = gaol_core::intersect(a, b);

    check("intersect([0,1], [2,3]) = empty",
          i.is_empty());
  }

  // Test intersect with nested intervals
  void test_intersect_nested()
  {
    interval a(0.0, 3.0);
    interval b(1.0, 2.0);
    interval i = gaol_core::intersect(a, b);

    check("intersect([0,3], [1,2]) = [1, 2]",
          i == b,
          [&] { return hex(i) + " vs " + hex(b); });
  }

  // Test intersect with empty interval
  void test_intersect_empty()
  {
    interval a(0.0, 1.0);
    interval empty = interval::emptyset();
    interval i = gaol_core::intersect(a, empty);

    check("intersect([0,1], empty) = empty",
          i.is_empty());

    interval i2 = gaol_core::intersect(empty, a);
    check("intersect(empty, [0,1]) = empty",
          i2.is_empty());

    interval i3 = gaol_core::intersect(empty, empty);
    check("intersect(empty, empty) = empty",
          i3.is_empty());
  }

  // Test intersect commutativity
  void test_intersect_commutative()
  {
    interval a(0.0, 2.0);
    interval b(1.0, 3.0);

    interval i1 = gaol_core::intersect(a, b);
    interval i2 = gaol_core::intersect(b, a);

    check("intersect is commutative: intersect(a, b) == intersect(b, a)",
          i1 == i2,
          [&] { return hex(i1) + " vs " + hex(i2); });
  }

  // Test intersect associativity
  void test_intersect_associative()
  {
    interval a(0.0, 3.0);
    interval b(1.0, 4.0);
    interval c(2.0, 5.0);

    interval i1 = gaol_core::intersect(gaol_core::intersect(a, b), c);
    interval i2 = gaol_core::intersect(a, gaol_core::intersect(b, c));

    check("intersect is associative: intersect(intersect(a,b),c) == intersect(a,intersect(b,c))",
          i1 == i2,
          [&] { return hex(i1) + " vs " + hex(i2); });
  }

  // Test hull and intersect with point intervals
  void test_point_intervals()
  {
    interval a(1.0, 1.0);
    interval b(2.0, 2.0);

    interval h = gaol_core::hull(a, b);
    check("hull([1,1], [2,2]) = [1, 2]",
          h.left() == 1.0 && h.right() == 2.0,
          [&] { return hex(h); });

    interval i = gaol_core::intersect(a, b);
    check("intersect([1,1], [2,2]) = empty",
          i.is_empty());
  }

  // Test hull and intersect with universe
  void test_universe()
  {
    interval a(0.0, 1.0);
    interval universe = interval::universe();

    interval h = gaol_core::hull(a, universe);
    check("hull([0,1], universe) = universe",
          h == universe,
          [&] { return hex(h) + " vs " + hex(universe); });

    interval i = gaol_core::intersect(a, universe);
    check("intersect([0,1], universe) = [0, 1]",
          i == a,
          [&] { return hex(i) + " vs " + hex(a); });
  }

  // Test hull and intersect idempotence
  void test_idempotence()
  {
    interval a(0.0, 1.0);

    interval h = gaol_core::hull(a, a);
    check("hull(a, a) = a (idempotent)",
          h == a,
          [&] { return hex(h) + " vs " + hex(a); });

    interval i = gaol_core::intersect(a, a);
    check("intersect(a, a) = a (idempotent)",
          i == a,
          [&] { return hex(i) + " vs " + hex(a); });
  }

  // Test absorption law: a & (a | b) = a
  void test_absorption()
  {
    interval a(0.0, 1.0);
    interval b(2.0, 3.0);

    interval h = gaol_core::hull(a, b);
    interval i = gaol_core::intersect(a, h);

    check("absorption: intersect(a, hull(a, b)) = a",
          i == a,
          [&] { return hex(i) + " vs " + hex(a); });
  }

} // namespace

int main()
{
  test_hull_basic();
  test_hull_overlapping();
  test_hull_nested();
  test_hull_empty();
  test_hull_unbounded();
  test_hull_commutative();
  test_hull_associative();
  test_intersect_basic();
  test_intersect_disjoint();
  test_intersect_nested();
  test_intersect_empty();
  test_intersect_commutative();
  test_intersect_associative();
  test_point_intervals();
  test_universe();
  test_idempotence();
  test_absorption();

  return summary();
}
