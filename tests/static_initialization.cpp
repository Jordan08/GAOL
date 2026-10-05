/*-*-C++-*------------------------------------------------------------------
 * gaol -- NOT Just Another Interval Library
 *--------------------------------------------------------------------------
 * Tests of GAOL v5: the operations computed in the initialization of the
 * static objects of a program.
 *
 * The program's files are initialized before those of the static library
 * GAOL, which the CMake build makes, with ELF (GNU ld or lld, glibc or musl
 * running .init_array in the order of the objects linked, the program's
 * first), ld64 on macOS and the linker of Visual C++. MinGW-w64 runs its
 * .ctors from the last object linked to the first: there GAOL's files come
 * first, the bug did not show, and this test passes with or without the fix.
 * GAOL's constants were computed by the dynamic
 * initialization of its files: pi_dn and the other doubles of
 * gaol/gaol_port.h, read from unions, the intervals interval::cst_pi,
 * cst_one... of gaol/gaol_interval.cpp, and the masks of the SSE2 operations
 * of gaol/gaol_interval_sse.cpp. A static object computing intervals found
 * them 0: pi() was [-0, 0], [1, 2]/[-1, 1] was [-0, 0] rather than
 * [-oo, +oo], sin([0, 4]) was [-0, 0], with the SSE2 intervals and with the
 * others, without any warning. They are now initialized when compiling
 * (GAOL v5).
 *
 * Each operation below is computed before main(), in the initialization of a
 * static object of this file, and again in main(): both have to give the same
 * interval, bit for bit.
 *
 * Copyright (c) 2026 ENSTA, France
 *
 * Created 2026-09-26 by Jordan NININ
 *--------------------------------------------------------------------------
 * gaol is a software distributed WITHOUT ANY WARRANTY. Read the associated
 * COPYING file for information.
 *--------------------------------------------------------------------------*/

#include "gaol_tests.h"

#include <string>
#include <vector>

using namespace gaol;
using namespace gaol_tests;

namespace
{
  struct Operation
  {
    const char *name;
    interval (*compute)();
  };

  // A constant array of pointers to functions, initialized when compiling
  const Operation operations[] = {
    // The constants of intervals
    { "interval::pi()", [] { return interval::pi(); } },
    { "interval::two_pi()", [] { return interval::two_pi(); } },
    { "interval::half_pi()", [] { return interval::half_pi(); } },
    { "interval::one()", [] { return interval::one(); } },
    { "interval::minus_one_plus_one()", [] { return interval::minus_one_plus_one(); } },
    { "interval::one_plus_infinity()", [] { return interval::one_plus_infinity(); } },
    { "interval::zero()", [] { return interval::zero(); } },
    { "interval::universe()", [] { return interval::universe(); } },
    { "interval::emptyset()", [] { return interval::emptyset(); } },
    { "interval::positive()", [] { return interval::positive(); } },
    { "interval::negative()", [] { return interval::negative(); } },
    // The operations using the masks of the SSE2 intervals
    { "[0.1, 0.3]*[-1.5, 2.5]", [] { return interval(0.1, 0.3) * interval(-1.5, 2.5); } },
    { "[-0.1, 0.3]*[-1.5, -0.5]", [] { return interval(-0.1, 0.3) * interval(-1.5, -0.5); } },
    { "[1, 2]/[3, 7]", [] { return interval(1.0, 2.0) / interval(3.0, 7.0); } },
    { "[1]/[-3, -1]", [] { return interval(1.0) / interval(-3.0, -1.0); } },
    { "[1, 2]/[-1, 1]", [] { return interval(1.0, 2.0) / interval(-1.0, 1.0); } },
    { "[1, 2].inverse()", [] { return interval(1.0, 2.0).inverse(); } },
    { "[0.1, 0.3]+[0.2, 0.7]", [] { return interval(0.1, 0.3) + interval(0.2, 0.7); } },
    { "[0.1, 0.3]-[0.2, 0.7]", [] { return interval(0.1, 0.3) - interval(0.2, 0.7); } },
    { "-[0.1, 0.3]", [] { return -interval(0.1, 0.3); } },
    { "abs([-0.3, 0.1])", [] { return abs(interval(-0.3, 0.1)); } },
    { "sqr([-0.1, 0.3])", [] { return sqr(interval(-0.1, 0.3)); } },
    { "pow([-0.1, 0.3], 3)", [] { return pow(interval(-0.1, 0.3), 3); } },
    { "sqrt([0.1, 2])", [] { return sqrt(interval(0.1, 2.0)); } },
    { "[-0.3, 0.1]|[0.5, 0.6]", [] { return interval(-0.3, 0.1) | interval(0.5, 0.6); } },
    { "[-0.3, 0.1]&[0, 0.6]", [] { return interval(-0.3, 0.1) & interval(0.0, 0.6); } },
    { "midpoint of [0.1, 0.3]", [] { return interval(interval(0.1, 0.3).midpoint()); } },
    // The elementary functions, the trigonometric ones using pi
    { "exp([0.1, 0.3])", [] { return exp(interval(0.1, 0.3)); } },
    { "log([0.1, 0.3])", [] { return log(interval(0.1, 0.3)); } },
    { "sin([0, 4])", [] { return sin(interval(0.0, 4.0)); } },
    { "sin([1, 1.5])", [] { return sin(interval(1.0, 1.5)); } },
    { "cos([1, 3])", [] { return cos(interval(1.0, 3.0)); } },
    { "tan([0.5, 1])", [] { return tan(interval(0.5, 1.0)); } },
    { "asin([1])", [] { return asin(interval(1.0)); } },
    { "acos([0])", [] { return acos(interval(0.0)); } },
    { "acos([-1])", [] { return acos(interval(-1.0)); } },
    { "atan([1, +oo])", [] { return atan(interval(1.0, GAOL_INFINITY)); } },
    { "atan2([1], [-1, 1])", [] { return atan2(interval(1.0), interval(-1.0, 1.0)); } },
    { "atan2([-1, 1], [-1, -0.5])", [] { return atan2(interval(-1.0, 1.0), interval(-1.0, -0.5)); } },
    // The reader of strings, whose "pi" is interval::pi()
    { "textToInterval(\"pi\")", [] { return textToInterval("pi"); } },
    { "textToInterval(\"[0.1, 1/3]\")", [] { return textToInterval("[0.1, 1/3]"); } },
    { "textToInterval(\"sin(1)+exp(0.1)\")", [] { return textToInterval("sin(1)+exp(0.1)"); } },
  };

  const std::size_t nb_operations = sizeof(operations)/sizeof(operations[0]);

  std::vector<interval> compute_all()
  {
    std::vector<interval> results;
    for (std::size_t i = 0; i < nb_operations; ++i) {
      results.push_back(operations[i].compute());
    }
    return results;
  }

  // Computed before main(), in the initialization of a static object
  const std::vector<interval> computed_before_main = compute_all();
}

int main()
{
  for (std::size_t i = 0; i < nb_operations; ++i) {
    const interval in_main = operations[i].compute();
    const interval& before_main = computed_before_main[i];
    check(std::string(operations[i].name) + ": the same before main() as in it",
          hex(before_main) == hex(in_main),
          [&] { return hex(before_main) + " before main(), " + hex(in_main) + " in it"; });
  }

  // The bounds of pi and pi/2 of gaol/gaol_port.h, written in decimal, are
  // the doubles next to pi and pi/2, which the unions of GAOL 4 wrote in bits
  using namespace gaol_detail;
  check("pi_dn and pi_up", pi_dn == 0x1.921fb54442d18p+1 && pi_up == 0x1.921fb54442d19p+1);
  check("half_pi_dn and half_pi_up", half_pi_dn == 0x1.921fb54442d18p+0 && half_pi_up == 0x1.921fb54442d19p+0);

  const int status = summary();
  gaol::cleanup();
  return status;
}
