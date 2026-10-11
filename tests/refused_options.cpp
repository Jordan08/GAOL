/*-*-C++-*------------------------------------------------------------------
 * gaol -- NOT Just Another Interval Library
 *--------------------------------------------------------------------------
 * Tests of GAOL v5: the options gaol/gaol_config.h refuses.
 *
 * A program using GAOL, which the tests refused_finite_math_only and
 * refused_fast_math of tests/CMakeLists.txt compile with -ffinite-math-only
 * and with -ffast-math: the compilation has to fail with the message of
 * gaol/gaol_config.h; refused_positive compiles it with neither, and the
 * compilation has to succeed. tests/refused_options.sh does the same in the
 * autotools and meson builds. With -ffinite-math-only, the compiler takes NaN
 * and infinities never to occur, the empty interval, whose bounds are NaN, is
 * no longer told empty (([1, 2] & [3, 4]).is_empty() is false), and the
 * bounds computed are wrong. The bounds are read from volatile doubles: with
 * constants, Clang 18 computed the intersection at compile time at -O2 and
 * -O3, and the program told it empty with -ffinite-math-only. Compiled without
 * the refused options, this program is right: it computes what the tests are
 * about, and returns 0.
 *
 * Copyright (c) 2026 ENSTA, France
 *
 * Created 2026-09-29 by Jordan NININ
 *--------------------------------------------------------------------------
 * gaol is a software distributed WITHOUT ANY WARRANTY. Read the associated
 * COPYING file for information.
 *--------------------------------------------------------------------------*/

#include <gaol/gaol>

int main()
{
  volatile double one = 1.0, two = 2.0, three = 3.0, four = 4.0;
  const gaol::interval a(one, two), b(three, four);
  const bool empty = (a & b).is_empty();
  gaol::cleanup();
  return empty ? 0 : 1;
}
