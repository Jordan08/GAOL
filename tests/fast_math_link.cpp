/*-*-C++-*------------------------------------------------------------------
 * gaol -- NOT Just Another Interval Library
 *--------------------------------------------------------------------------
 * Tests of GAOL v5: a program linked with -ffast-math.
 *
 * GCC and Clang link crtfastmath.o into a program linked with -Ofast,
 * -ffast-math or -funsafe-math-optimizations, on x86 and on ARM Linux, and
 * its constructor, which runs after GAOL initialized itself, sets the modes
 * that flush the subnormal numbers to zero (flush-to-zero and
 * denormals-are-zero of the SSE instructions, FZ of ARM): [1e-300]*[1e-20] was
 * [0, 0]. This program is linked so, with GCC and Clang, its source being
 * compiled as any code using GAOL: the three builds add -ffast-math to its link
 * only.
 *
 * Where the build gives the link of the code using GAOL -mno-daz-ftz, which
 * keeps crtfastmath.o out of it (GCC 13 and later, 11.4 and 12.4, on x86),
 * the build defines GAOL_TESTS_NO_DAZ_FTZ, and the modes have to be clear when
 * main() starts. Otherwise they may be set: the first operation of GAOL has
 * to clear them. Where they are clear, the test sets them, as loading a
 * plug-in built with -Ofast does. The bounds of operations with subnormal
 * results and operands have then to be the tightest ones, computed apart with
 * exact rational arithmetic, or with mpmath for the exponential, and the
 * modes have to be cleared after them, or restored with
 * GAOL_PRESERVE_ROUNDING. tests/rounding_direction.cpp checks more operations
 * under each mode and each rounding direction.
 *
 * Copyright (c) 2026 ENSTA, France
 *
 * Created 2026-09-30 by Jordan NININ
 *--------------------------------------------------------------------------
 * gaol is a software distributed WITHOUT ANY WARRANTY. Read the associated
 * COPYING file for information.
 *--------------------------------------------------------------------------*/

#include "gaol_tests.h"

#include <string>

using namespace gaol;
using namespace gaol_tests;

namespace
{
  // A multiple of 2^-1074, exact in every rounding direction, but 0 when the
  // subnormals are flushed: computed with the modes clear
  double subnormal(long long multiple) { return static_cast<double>(multiple) * 0x1p-1074; }

  struct Case
  {
    const char *name;
    interval x, y;
    interval (*run)(const interval&, const interval&);
    Exact value; // the exact result, or a number between the same two doubles
  };
}

int main()
{
#if GAOL_TESTS_FLUSH
  // The modes as the link left them, read before anything else computes
  const unsigned int at_start = flush_bits();
  set_flush_bits(0u);
  std::printf("The modes that flush the subnormals to zero were %s when main() started (%#x)\n",
              at_start == 0u ? "clear" : "set", at_start);
#  if GAOL_TESTS_NO_DAZ_FTZ
  check("-mno-daz-ftz: the modes that flush the subnormals to zero clear when main() starts", at_start == 0u,
        [&] { return "bits " + std::to_string(at_start); });
#  endif
  // Those of the link, or all of them, as a plug-in sets them
  const unsigned int modes = (at_start != 0u) ? at_start : all_flush_bits;
#else
  std::printf("No mode flushing the subnormals to zero that the test can read: the operations are checked as they are\n");
#endif

  const Case cases[] = {
    { "[1e-300]*[1e-20]", interval(1e-300), interval(1e-20),
      [](const interval& x, const interval& y) { return x*y; },
      exact(dyadic(1e-300)*dyadic(1e-20)) },
    { "[3e-308]-[2.9e-308]", interval(3e-308), interval(2.9e-308),
      [](const interval& x, const interval& y) { return x - y; },
      exact(dyadic(3e-308) - dyadic(2.9e-308)) },
    { "[100*2^-1074]*[1e10]", interval(subnormal(100)), interval(1e10),
      [](const interval& x, const interval& y) { return x*y; },
      exact(dyadic(subnormal(100))*dyadic(1e10)) },
    // exp(-740) is 84.78 times 2^-1074 (mpmath), between the same two doubles
    // as 84.5 times 2^-1074
    { "exp([-740])", interval(-740.0), interval(),
      [](const interval& x, const interval&) { return exp(x); },
      exact(dyadic(subnormal(169))*dyadic(0.5)) },
  };

  for (const Case& c : cases) {
#if GAOL_TESTS_FLUSH
    set_flush_bits(modes);
    const unsigned int found = flush_bits();
#endif
    const interval r = c.run(c.x, c.y);
#if GAOL_TESTS_FLUSH
    const unsigned int left = flush_bits();
    set_flush_bits(0u);
    const auto describe = [&] { return "modes " + std::to_string(left) + " after it, " + std::to_string(found) + " before"; };
#  if GAOL_PRESERVE_ROUNDING
    check(std::string(c.name) + ": flush-to-zero modes restored", left == found, describe);
#  else
    check(std::string(c.name) + ": flush-to-zero modes cleared", left == 0u, describe);
#  endif
#endif
    check(std::string(c.name) + " the tightest enclosure", is_tightest_enclosure(r, c.value), [&] { return hex(r); });
  }

  const int status = summary();
  gaol::cleanup();
  return status;
}
