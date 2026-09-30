/*-*-C++-*------------------------------------------------------------------
 * gaol -- NOT Just Another Interval Library
 *--------------------------------------------------------------------------
 * Tests of GAOL v5: the warning for a result thrown away.
 *
 * A program using GAOL, which the tests nodiscard_*_cxx11, _cxx14 and _cxx17
 * of tests/CMakeLists.txt compile in each language standard with
 * -Werror=unused-result and -Werror=attributes, GCC and Clang only. With
 * GAOL_DISCARD defined, it throws away the result of sqrt(x), which leaves x
 * as it was, and the compilation has to fail with the warning of the
 * compiler: GAOL_NODISCARD (gaol/gaol_config.h) is [[nodiscard]] from C++17
 * only, and a project with GCC 9 compiles in C++14 unless it says otherwise.
 * Without GAOL_DISCARD, the program uses every result, and has to compile:
 * the compiler ignores the attribute on no declaration of GAOL's headers,
 * and their own inline functions throw no result away. It also throws a
 * result away on purpose, with a cast to void, where doc/using.md says that
 * it does not warn: with [[nodiscard]] (C++17) and with Clang's attribute,
 * not with GCC's before C++17.
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
  const gaol::interval x(2.0, 3.0);
#ifdef GAOL_DISCARD
  sqrt(x);
#endif
#if __cplusplus >= 201703L || defined(__clang__)
  (void)sqrt(x);
#endif
  const gaol::interval y = sqrt(x);
  const bool empty = y.is_empty();
  gaol::cleanup();
  return empty ? 1 : 0;
}
