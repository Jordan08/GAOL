/*-*-C++-*------------------------------------------------------------------
 * GAOL -- not Just Another Interval Library
 *--------------------------------------------------------------------------
 * Tests of GAOL v5: the arithmetic of IEEE 1788-2015, with doubles, in the
 * namespace gaol_ieee1788.
 *
 * A short test to read first. Each line CHECK( ... ) says what a function
 * gives for intervals whose bounds are doubles, as one writes it by hand:
 * [1, 2] + [3, 4] is [4, 6]. Where the exact result is no double (1/3,
 * 0.1 + 0.2, the square root of 2), the bounds written are the two doubles
 * around it: the narrowest interval of doubles that holds it. The program
 * prints the lines that fail, and the number of checks, and its exit status
 * is 1 if one failed. The checks go from the bounds and the empty set to the
 * four operations, then the reciprocal, the square, the square root and the
 * integer power; == is the equality of sets (equal() of the standard), and
 * the operators +, -, * and / are those of the functions add(), sub(), mul()
 * and div().
 *--------------------------------------------------------------------------
 * Copyright (c) 2026 ENSTA, France
 *
 * By: Jordan NININ (created 2026-11-15)
 *--------------------------------------------------------------------------
 * GAOL is a software distributed WITHOUT ANY WARRANTY. Read the associated
 * COPYING.LIB file for information.
 *--------------------------------------------------------------------------*/

#include <cstdio>
#include <limits>

#include <gaol/gaol.h>

using namespace gaol_ieee1788;

namespace
{
  const double oo = std::numeric_limits<double>::infinity();
  int checks = 0, failures = 0;
}

// Counts a check, and prints the line of the checks that fail
#define CHECK(condition) \
  do { ++checks; if (!(condition)) { ++failures; std::printf("FAILED line %d: %s\n", __LINE__, #condition); } } while (0)

int main()
{
  // ---- Bounds, midpoint, width, radius
  CHECK( inf(interval(1, 2)) == 1 );
  CHECK( sup(interval(1, 2)) == 2 );
  CHECK( mid(interval(1, 3)) == 2 );
  CHECK( wid(interval(1, 3)) == 2 );
  CHECK( rad(interval(1, 3)) == 1 );

  // ---- The empty set and the whole line
  CHECK( isEmpty(empty()) );
  CHECK( isEntire(entire()) );
  CHECK( !isEmpty(interval(1, 2)) );
  CHECK( !isEntire(interval(1, 2)) );
  CHECK( entire() == interval(-oo, oo) );
  CHECK( inf(empty()) == oo );
  CHECK( sup(empty()) == -oo );
  CHECK( interval(2, 1) == empty() );    // bounds in the wrong order

  // ---- Equality: == is the equality of two sets, != its negation
  CHECK( interval(1, 2) == interval(1, 2) );
  CHECK( !(interval(1, 2) == interval(1, 3)) );
  CHECK( !(interval(1, 2) == interval(0, 2)) );
  CHECK( interval(1, 2) != interval(1, 3) );
  CHECK( !(interval(1, 2) != interval(1, 2)) );
  CHECK( empty() == empty() );
  CHECK( empty() != interval(1, 2) );
  CHECK( interval(0) == interval(-0.0) );    // the sign of a zero is not seen

  // ---- Addition
  CHECK( add(interval(1, 2), interval(3, 4)) == interval(4, 6) );
  CHECK( add(interval(-1, 2), interval(-3, 4)) == interval(-4, 6) );
  CHECK( add(interval(1, oo), interval(1, 2)) == interval(2, oo) );
  CHECK( add(interval(0.1), interval(0.2)) == interval(0.3, 0.30000000000000004) );
  CHECK( add(interval(1), interval(1e-20)) == interval(1, 1.0000000000000002) );    // rounded upward

  // ---- Subtraction and opposite
  CHECK( sub(interval(1, 2), interval(3, 4)) == interval(-3, -1) );
  CHECK( sub(interval(-oo, 1), interval(1, 2)) == interval(-oo, 0) );
  CHECK( sub(interval(1, 2), interval(1, 2)) == interval(-1, 1) );    // not 0
  CHECK( sub(interval(1), interval(1e-20)) == interval(0.9999999999999999, 1) );    // rounded downward
  CHECK( neg(interval(1, 2)) == interval(-2, -1) );
  CHECK( neg(interval(-1, 3)) == interval(-3, 1) );

  // ---- Multiplication
  CHECK( mul(interval(1, 2), interval(3, 4)) == interval(3, 8) );
  CHECK( mul(interval(-1, 2), interval(3, 4)) == interval(-4, 8) );
  CHECK( mul(interval(-1, 2), interval(-3, 4)) == interval(-6, 8) );
  CHECK( mul(interval(-2, -1), interval(-4, -3)) == interval(3, 8) );
  CHECK( mul(interval(-2, -1), interval(3, 4)) == interval(-8, -3) );
  CHECK( mul(interval(1), interval(-3, 5)) == interval(-3, 5) );
  CHECK( mul(interval(0), interval(5, 6)) == interval(0) );
  CHECK( mul(interval(0), entire()) == interval(0) );           // 0 times oo is 0
  CHECK( mul(interval(0, 1), interval(2, oo)) == interval(0, oo) );
  CHECK( mul(interval(3), interval(0.1)) == interval(0.3, 0.30000000000000004) );
  CHECK( mul(interval(1.1), interval(1.1)) == interval(1.2100000000000002, 1.2100000000000004) );
  CHECK( mul(interval(-1.1), interval(1.1)) == interval(-1.2100000000000004, -1.2100000000000002) );

  // ---- Division
  CHECK( div(interval(6, 8), interval(2, 4)) == interval(1.5, 4) );
  CHECK( div(interval(-8, -6), interval(2, 4)) == interval(-4, -1.5) );
  CHECK( div(interval(0), interval(2, 4)) == interval(0) );
  CHECK( div(interval(1), interval(3)) == interval(0.3333333333333333, 0.33333333333333337) );
  CHECK( div(interval(3), interval(3)) == interval(1) );                // exact, not 3 * (1/3)
  CHECK( div(interval(1, 2), interval(0, 2)) == interval(0.5, oo) );
  CHECK( div(interval(1, 2), interval(-2, 0)) == interval(-oo, -0.5) );
  CHECK( div(interval(1, 2), interval(-1, 1)) == entire() );        // 0 inside the divisor
  CHECK( div(interval(-1, 1), interval(-1, 1)) == entire() );
  CHECK( div(interval(1, 2), interval(0)) == empty() );             // division by [0, 0]

  // ---- Reciprocal
  CHECK( recip(interval(2, 4)) == interval(0.25, 0.5) );
  CHECK( recip(interval(-4, -2)) == interval(-0.5, -0.25) );
  CHECK( recip(interval(-1, 1)) == entire() );
  CHECK( recip(interval(3)) == interval(0.3333333333333333, 0.33333333333333337) );

  // ---- Square: x^2 is never negative, x * x takes its two factors as independent
  CHECK( sqr(interval(2, 3)) == interval(4, 9) );
  CHECK( sqr(interval(-3, 2)) == interval(0, 9) );
  CHECK( sqr(interval(1.1)) == interval(1.2100000000000002, 1.2100000000000004) );
  CHECK( mul(interval(-3, 2), interval(-3, 2)) == interval(-6, 9) );

  // ---- Square root: the negative part has no root
  CHECK( sqrt(interval(4, 9)) == interval(2, 3) );
  CHECK( sqrt(interval(-4, 9)) == interval(0, 3) );
  CHECK( sqrt(interval(-4, -1)) == empty() );
  CHECK( sqrt(interval(2)) == interval(1.4142135623730949, 1.4142135623730951) );
  CHECK( sqrt(interval(3)) == interval(1.7320508075688772, 1.7320508075688774) );

  // ---- Integer power
  CHECK( pown(interval(2, 3), 2) == interval(4, 9) );
  CHECK( pown(interval(2, 3), 3) == interval(8, 27) );
  CHECK( pown(interval(-2, 3), 2) == interval(0, 9) );              // even: starts at 0
  CHECK( pown(interval(-2, 3), 3) == interval(-8, 27) );            // odd: keeps the sign
  CHECK( pown(interval(-2, -1), 3) == interval(-8, -1) );
  CHECK( pown(interval(2, 3), 0) == interval(1) );
  CHECK( pown(interval(2, 3), 1) == interval(2, 3) );
  CHECK( pown(interval(2, 4), -1) == interval(0.25, 0.5) );
  CHECK( pown(interval(2, 4), -2) == interval(0.0625, 0.25) );
  CHECK( pown(interval(3), -1) == interval(0.3333333333333333, 0.33333333333333337) );
  CHECK( pown(interval(3), -2) == interval(0.1111111111111111, 0.11111111111111112) );
  CHECK( pown(interval(1.1), 3) == interval(1.3310000000000002, 1.3310000000000004) );

  // ---- The operators do the same
  CHECK( interval(1, 2) + interval(3, 4) == interval(4, 6) );
  CHECK( interval(1, 2) - interval(3, 4) == interval(-3, -1) );
  CHECK( interval(1, 2) * interval(3, 4) == interval(3, 8) );
  CHECK( interval(6, 8) / interval(2, 4) == interval(1.5, 4) );
  CHECK( -interval(1, 2) == interval(-2, -1) );

  // ---- operator+
  CHECK( interval(5, 6) + interval(1, 2) == interval(6, 8) );
  CHECK( interval(-3, -1) + interval(2, 5) == interval(-1, 4) );
  CHECK( interval(0.5, 1) + interval(0.25, 2) == interval(0.75, 3) );
  CHECK( interval(0.1) + interval(0.2) == interval(0.3, 0.30000000000000004) );
  CHECK( interval(1, oo) + interval(-oo, 2) == entire() );

  // ---- operator-, binary and unary
  CHECK( interval(5, 6) - interval(1, 2) == interval(3, 5) );
  CHECK( interval(-3, -1) - interval(2, 5) == interval(-8, -3) );
  CHECK( interval(3, 4) - interval(3, 4) == interval(-1, 1) );
  CHECK( interval(0.3) - interval(0.1) == interval(0.19999999999999998) );
  CHECK( -interval(-4, 7) == interval(-7, 4) );

  // ---- operator*
  CHECK( interval(-3, 5) * interval(-2, 4) == interval(-12, 20) );
  CHECK( interval(-3, -2) * interval(-5, -4) == interval(8, 15) );
  CHECK( interval(2, 3) * interval(-4, -1) == interval(-12, -2) );
  CHECK( interval(0.5, 4) * interval(2, 8) == interval(1, 32) );
  CHECK( interval(0) * interval(-oo, oo) == interval(0) );

  // ---- operator/
  CHECK( interval(10, 20) / interval(2, 5) == interval(2, 10) );
  CHECK( interval(-6, -3) / interval(3, 6) == interval(-2, -0.5) );
  CHECK( interval(2, 4) / interval(0, 1) == interval(2, oo) );
  CHECK( interval(-1, 1) / interval(2, 4) == interval(-0.5, 0.5) );
  CHECK( interval(1) / interval(10) == interval(0.09999999999999999, 0.1) );

  // ---- The empty set in an operation: no value
  CHECK( add(interval(1, 2), empty()) == empty() );
  CHECK( sub(empty(), interval(1, 2)) == empty() );
  CHECK( mul(interval(1, 2), empty()) == empty() );
  CHECK( div(interval(1, 2), empty()) == empty() );
  CHECK( neg(empty()) == empty() );
  CHECK( sqr(empty()) == empty() );
  CHECK( sqrt(empty()) == empty() );

  std::printf("%d checks, %d failed\n", checks, failures);
  gaol::cleanup();
  return failures == 0 ? 0 : 1;
}
