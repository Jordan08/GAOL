/*-*-C++-*----------------------------------------------------------------------
 * gaol -- NOT Just Another Interval Library
 *------------------------------------------------------------------------------
 * Tests of GAOL v5: user-defined literals (25g)
 *
 * Tests for the user-defined literal operators in gaol::literals namespace.
 *
 * Copyright (c) 2026 ENSTA, France
 *
 * Created 2026 by Jordan NININ
 *------------------------------------------------------------------------------
 * gaol is a software distributed WITHOUT ANY WARRANTY. Read the associated
 * COPYING file for information.
 *----------------------------------------------------------------------------*/

#include "gaol_tests.h"
#include "gaol/gaol_literals.h"

using namespace gaol::literals;
using gaol_tests::check;
using gaol_tests::hex;
using gaol_tests::summary;

namespace {

  const double oo = std::numeric_limits<double>::infinity();

  // Test raw string literal (enclosure via textToInterval)
  void test_raw_string_literal()
  {
    // String literal should give an enclosure of the decimal value
    auto x1 = "0.1"_iv;

    check("\"0.1\"_iv is not empty",
          !x1.is_empty(),
          [&] { return hex(x1); });

    // The enclosure should contain the decimal 0.1
    check("\"0.1\"_iv contains 0.1",
          x1.set_contains(0.1),
          [&] { return hex(x1); });

    // For a simple integer, string literal should give point interval
    auto x2 = "5"_iv;
    check("\"5\"_iv is [5,5]",
          x2.is_a_double() && x2.left() == 5.0 && x2.right() == 5.0,
          [&] { return hex(x2); });

    // Test with scientific notation
    auto x3 = "1e-3"_iv;
    check("\"1e-3\"_iv is not empty",
          !x3.is_empty(),
          [&] { return hex(x3); });
    check("\"1e-3\"_iv contains 0.001",
          x3.set_contains(0.001),
          [&] { return hex(x3); });
  }

  // Test integer literal (point interval)
  void test_integer_literal()
  {
    auto x1 = 3_iv;
    check("3_iv is [3,3]",
          x1.is_a_double() && x1.left() == 3.0 && x1.right() == 3.0,
          [&] { return hex(x1); });

    auto x2 = 0_iv;
    check("0_iv is [0,0]",
          x2.is_a_double() && x2.left() == 0.0 && x2.right() == 0.0,
          [&] { return hex(x2); });

    auto x3 = 100_iv;
    check("100_iv is [100,100]",
          x3.is_a_double() && x3.left() == 100.0 && x3.right() == 100.0,
          [&] { return hex(x3); });

    auto x4 = 9223372036854775807ULL_iv; // max unsigned long long
    check("ULLONG_MAX_iv is not empty",
          !x4.is_empty(),
          [&] { return hex(x4); });
  }

  // Test floating-point literal (point interval)
  void test_floating_point_literal()
  {
    auto x1 = 0.1_iv;
    // 0.1_iv is a point interval at the double 0.1
    check("0.1_iv is a point interval",
          x1.is_a_double(),
          [&] { return hex(x1); });

    // The double 0.1
    double d01 = 0.1;
    check("0.1_iv equals interval(0.1)",
          x1 == gaol_core::interval(d01),
          [&] { return hex(x1) + " vs " + hex(gaol_core::interval(d01)); });

    auto x2 = 1e-3_iv;
    check("1e-3_iv is a point interval",
          x2.is_a_double(),
          [&] { return hex(x2); });

    auto x3 = 3.14_iv;
    check("3.14_iv is a point interval",
          x3.is_a_double(),
          [&] { return hex(x3); });
  }

  // Test that string and numeric literals differ
  void test_string_vs_numeric()
  {
    // String literal "0.1"_iv encloses the decimal 0.1
    auto x_str = "0.1"_iv;
    // Numeric literal 0.1_iv is the point at the double 0.1
    auto x_num = 0.1_iv;

    // They should be different: the string gives an enclosure, the numeric gives a point
    check("\"0.1\"_iv != 0.1_iv (enclosure vs point)",
          x_str != x_num,
          [&] { return hex(x_str) + " vs " + hex(x_num); });

    // The string version should contain the numeric version
    // (the enclosure contains the double 0.1)
    check("\"0.1\"_iv contains 0.1_iv",
          (x_str & x_num) == x_num,
          [&] { return hex(x_str & x_num) + " vs " + hex(x_num); });
  }

  // Test literals with negative numbers
  void test_negative_literals()
  {
    // Note: C++ does not allow negative integer literals with UDL
    // because the lexer sees -3_iv as -(3_iv), not as a single token
    // So we test with floating-point
    auto x1 = -0.5_iv;
    check("-0.5_iv is a point interval at -0.5",
          x1.is_a_double() && x1.left() == -0.5 && x1.right() == -0.5,
          [&] { return hex(x1); });

    // String literal can be negative
    auto x2 = "-5"_iv;
    check("\"-5\"_iv is [-5,-5]",
          x2.is_a_double() && x2.left() == -5.0 && x2.right() == -5.0,
          [&] { return hex(x2); });
  }

  // Test literals with special values
  void test_special_values()
  {
    // String literals for special values
    auto x1 = "empty"_iv;
    check("\"empty\"_iv is empty",
          x1.is_empty());

    auto x2 = "entire"_iv;
    check("\"entire\"_iv is universe",
          x2 == gaol_core::interval::universe(),
          [&] { return hex(x2); });

    auto x3 = "[1,2]"_iv;
    check("\"[1,2]\"_iv is [1,2]",
          x3.left() == 1.0 && x3.right() == 2.0,
          [&] { return hex(x3); });
  }

  // Test literals in expressions
  void test_in_expressions()
  {
    auto x = "0.1"_iv;
    auto y = 2_iv;
    auto z = x + y;

    check("\"0.1\"_iv + 2_iv works",
          !z.is_empty(),
          [&] { return hex(z); });

    // The result should contain 2.1
    check("\"0.1\"_iv + 2_iv contains 2.1",
          z.set_contains(2.1),
          [&] { return hex(z); });
  }

  // Test that literals are in gaol::literals namespace
  void test_namespace()
  {
    // Without using namespace gaol::literals, we need to qualify
    auto x1 = gaol::literals::operator"" _iv("1.0", 3);
    check("gaol::literals::operator\"\" _iv works",
          !x1.is_empty(),
          [&] { return hex(x1); });

    auto x2 = gaol::literals::operator"" _iv(42ULL);
    check("gaol::literals::operator\"\" _iv(ULL) works",
          x2.is_a_double() && x2.left() == 42.0,
          [&] { return hex(x2); });

    auto x3 = gaol::literals::operator"" _iv(3.14L);
    check("gaol::literals::operator\"\" _iv(long double) works",
          x3.is_a_double(),
          [&] { return hex(x3); });
  }

  // Test zero with different signs
  void test_zero_signs()
  {
    auto x1 = 0_iv;
    check("0_iv is [0,0]",
          x1.left() == 0.0 && x1.right() == 0.0,
          [&] { return hex(x1); });

    auto x2 = "0"_iv;
    check("\"0\"_iv is [0,0]",
          x2.left() == 0.0 && x2.right() == 0.0,
          [&] { return hex(x2); });

    // Both should be the same point
    check("0_iv == \"0\"_iv",
          x1 == x2,
          [&] { return hex(x1) + " vs " + hex(x2); });
  }

} // namespace

int main()
{
  test_raw_string_literal();
  test_integer_literal();
  test_floating_point_literal();
  test_string_vs_numeric();
  test_negative_literals();
  test_special_values();
  test_in_expressions();
  test_namespace();
  test_zero_signs();

  return summary();
}
