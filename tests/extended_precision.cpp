/*-*-C++-*------------------------------------------------------------------
 * gaol -- NOT Just Another Interval Library
 *--------------------------------------------------------------------------
 * Tests of GAOL v5: the results that doubles computed in extended precision,
 * on the x87 unit of an x86 processor, round wrongly.
 *
 * The x87 unit computes with 64-bit significands and 15-bit exponents, and
 * rounds to a double only when a value leaves its registers. CORE-MATH
 * assumes every operation on doubles rounded to a double (FLT_EVAL_METHOD 0,
 * see 3rd/math-core/README.md), and gaol/gaol_config.h refuses to compile
 * GAOL otherwise. Built past that refusal, CORE-MATH gave the wrong double in
 * two ways, which this test checks at the arguments where it did so:
 *
 * - rounded to nearest, a result rounded twice, to 64 bits in the registers
 *   and then to a double, is the other neighbour of f(x) when f(x) lies within
 *   2^-64 or so of the middle of two doubles: exp(-745.13...) gave 0 rather
 *   than 2^-1074, tanh(19.06...) 1 rather than 1 - 2^-53, and one argument in
 *   several thousand the last bit wrong in most functions. Found on Debian 12
 *   i386 with GCC 12, where GAOL's own tests passed;
 * - rounded upward, downward or toward zero, CORE-MATH computes the results
 *   beyond the doubles as 0x1p-1074 * 0.5, -1.0 + 0x1p-54 or
 *   0x1p1023 + 0x1p1023, whose rounding gives the right double in every
 *   direction. Exact in the x87 registers, GCC 9 turned them into doubles at
 *   compile time, rounded to nearest: exp2(-2000) rounded upward gave 0
 *   rather than 2^-1074, expm1(-800) -1, cospi(2^-1074) rounded downward 1,
 *   cosh(MAX) +inf, and GAOL bounds that did not enclose the exact values.
 *   Found on x86-64 with -mfpmath=387.
 *
 * CORE-MATH (gaol/gaol_core_math.h) is checked in the four rounding
 * directions, and GAOL's bounds of [x, x] in the upward direction it keeps.
 * The values are computed with mpmath (tests/extended_precision_values.py).
 *
 * Copyright (c) 2026 ENSTA, France
 *
 * Created 2026-09-21 by Jordan NININ
 *--------------------------------------------------------------------------
 * gaol is a software distributed WITHOUT ANY WARRANTY. Read the associated
 * COPYING file for information.
 *--------------------------------------------------------------------------*/

#include "gaol_tests.h"
#include "gaol/gaol_core_math.h"
#include "extended_precision_values.h"

#include <cfenv>

using namespace gaol;
using namespace gaol_tests;

namespace
{
  double core_math(const ExtendedPrecisionValue& v)
  {
    const std::string f = v.function;
    const double a = v.a;
    if (f == "exp") return gaol_cr_exp(a);
    if (f == "exp2") return gaol_cr_exp2(a);
    if (f == "exp10") return gaol_cr_exp10(a);
    if (f == "expm1") return gaol_cr_expm1(a);
    if (f == "exp2m1") return gaol_cr_exp2m1(a);
    if (f == "exp10m1") return gaol_cr_exp10m1(a);
    if (f == "log2") return gaol_cr_log2(a);
    if (f == "log10") return gaol_cr_log10(a);
    if (f == "cos") return gaol_cr_cos(a);
    if (f == "tan") return gaol_cr_tan(a);
    if (f == "atan") return gaol_cr_atan(a);
    if (f == "sinh") return gaol_cr_sinh(a);
    if (f == "cosh") return gaol_cr_cosh(a);
    if (f == "tanh") return gaol_cr_tanh(a);
    if (f == "asinh") return gaol_cr_asinh(a);
    if (f == "acosh") return gaol_cr_acosh(a);
    if (f == "cbrt") return gaol_cr_cbrt(a);
    if (f == "sinpi") return gaol_cr_sinpi(a);
    if (f == "cospi") return gaol_cr_cospi(a);
    if (f == "tanpi") return gaol_cr_tanpi(a);
    if (f == "atan2") return gaol_cr_atan2(a, v.b);
    std::printf("no function %s\n", v.function);
    std::abort();
  }

  interval bounds(const ExtendedPrecisionValue& v)
  {
    const std::string f = v.function;
    const interval a(v.a, v.a);
    if (f == "exp") return exp(a);
    if (f == "exp2") return exp2(a);
    if (f == "exp10") return exp10(a);
    if (f == "expm1") return expm1(a);
    if (f == "exp2m1") return exp2m1(a);
    if (f == "exp10m1") return exp10m1(a);
    if (f == "log2") return log2(a);
    if (f == "log10") return log10(a);
    if (f == "cos") return cos(a);
    if (f == "tan") return tan(a);
    if (f == "atan") return atan(a);
    if (f == "sinh") return sinh(a);
    if (f == "cosh") return cosh(a);
    if (f == "tanh") return tanh(a);
    if (f == "asinh") return asinh(a);
    if (f == "acosh") return acosh(a);
    if (f == "cbrt") return nth_root(a, 3);
    if (f == "sinpi") return sinpi(a);
    if (f == "cospi") return cospi(a);
    if (f == "tanpi") return tanpi(a);
    if (f == "atan2") return atan2(a, interval(v.b, v.b));
    std::printf("no function %s\n", v.function);
    std::abort();
  }

  std::string arguments(const ExtendedPrecisionValue& v)
  {
    return std::string(v.function) + "(" + hex(v.a) + (std::string(v.function) == "atan2" ? ", " + hex(v.b) : "") + ")";
  }

  // CORE-MATH at v in the four rounding directions: toward zero, the
  // neighbour of f(x) on the side of 0
  void core_math_rounding(const std::string& table, const ExtendedPrecisionValue& v)
  {
    const struct { int mode; const char *name; double expected; } directions[] = {
      {FE_TONEAREST, "to nearest", v.nearest},
      {FE_UPWARD, "upward", v.above},
      {FE_DOWNWARD, "downward", v.below},
      {FE_TOWARDZERO, "toward zero", v.above > 0 ? v.below : v.above},
    };
    for (const auto& d : directions) {
      std::fesetround(d.mode);
      const double got = core_math(v);
      std::fesetround(FE_UPWARD);
      check(std::string("gaol_cr_") + v.function + " rounded " + d.name + " (" + table + ")", got == d.expected,
            [&] { return arguments(v) + " = " + hex(got) + " rather than " + hex(d.expected); });
    }
  }

  // GAOL's bounds of [x, x]: they enclose f(x), and are the tightest ones
  void gaol_bounds(const std::string& table, const ExtendedPrecisionValue& v)
  {
    const std::string name = std::string(v.function) + " (" + table + ")";
    const interval got = evaluate(name, [&] { return bounds(v); }, [&] { return arguments(v); });
    const auto describe = [&] {
      return arguments(v) + " = " + hex(got) + ", exact value between " + hex(v.below) + " and " + hex(v.above);
    };
    if (!check(name + ": encloses", !got.is_empty() && got.left() <= v.below && got.right() >= v.above, describe)) {
      return;
    }
    check(name + ": the tightest bounds", got.left() == v.below && got.right() == v.above, describe);
  }
}

int main()
{
  gaol::init();
  for (const auto& v : nearest_values) {
    core_math_rounding("rounded twice to nearest", v);
    gaol_bounds("rounded twice to nearest", v);
  }
  for (const auto& v : directed_values) {
    core_math_rounding("beyond the doubles", v);
    gaol_bounds("beyond the doubles", v);
  }
  std::fesetround(FE_UPWARD);
  const int status = summary();
  gaol::cleanup();
  return status;
}
