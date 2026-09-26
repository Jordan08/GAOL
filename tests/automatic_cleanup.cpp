/*-*-C++-*------------------------------------------------------------------
 * gaol -- NOT Just Another Interval Library
 *--------------------------------------------------------------------------
 * Tests of GAOL v5: the rounding direction after the automatic cleanup.
 *
 * GAOL initializes itself before the static objects of the program and cleans
 * up when the program ends (gaol/gaol_common.h, gaol/gaol_init_cleanup.cpp).
 * The automatic cleanup has to set back the rounding direction the automatic
 * initialization found,
 * to nearest, as a program starts (C11, F.8.3), the x87 unit and the SSE
 * instructions each theirs, whatever direction main() left; with
 * GAOL_PRESERVE_ROUNDING, it has to leave the direction main() left.
 *
 * The direction is read by a function that a constructor of priority 101
 * registers with std::atexit(): the constructor runs before the dynamic
 * initialization of the static objects, where GAOL initializes itself, so the
 * function runs after the destructor of GAOL's static object, which calls
 * gaol::cleanup() ([basic.start.term]). The priorities of constructors are
 * those of ELF, and GAOL is linked statically, as the CMake build makes it: a
 * shared library would initialize itself before the constructor ran.
 * tests/CMakeLists.txt builds this test on Linux only.
 *
 * Copyright (c) 2026 ENSTA, France
 *
 * Created 2026-09-26 by Jordan NININ
 *--------------------------------------------------------------------------
 * gaol is a software distributed WITHOUT ANY WARRANTY. Read the associated
 * COPYING file for information.
 *--------------------------------------------------------------------------*/

#include "gaol_tests.h"

#if defined(__SSE2__)
#  include <xmmintrin.h>
#  define GAOL_TESTS_SSE 1
#  define SSE_DIRECTION(d) static_cast<unsigned>(d)
#else
#  define SSE_DIRECTION(d) 0u
#endif

using namespace gaol;
using namespace gaol_tests;

namespace
{
  // The rounding direction as fegetround() and the SSE control register give it
  struct State
  {
    int fenv;
    unsigned sse;

    bool operator==(const State& s) const { return fenv == s.fenv && sse == s.sse; }
  };

  State state()
  {
    State s;
    s.fenv = std::fegetround();
#if GAOL_TESTS_SSE
    s.sse = _mm_getcsr() & _MM_ROUND_MASK;
#else
    s.sse = 0u;
#endif
    return s;
  }

  void set(const State& s)
  {
    std::fesetround(s.fenv);
#if GAOL_TESTS_SSE
    _mm_setcsr((_mm_getcsr() & ~static_cast<unsigned int>(_MM_ROUND_MASK)) | s.sse);
#endif
  }

  std::string text(const State& s)
  {
    return "fegetround() " + std::to_string(s.fenv) + ", SSE rounding bits " + std::to_string(s.sse);
  }

  const State rounding_to_nearest = { FE_TONEAREST, SSE_DIRECTION(_MM_ROUND_NEAREST) };
  // The direction main() leaves: neither the one of a program that starts
  // nor GAOL's
  const State rounding_downward = { FE_DOWNWARD, SSE_DIRECTION(_MM_ROUND_DOWN) };

#if GAOL_PRESERVE_ROUNDING
  const State expected_after_cleanup = rounding_downward;
#else
  const State rounding_upward = { FE_UPWARD, SSE_DIRECTION(_MM_ROUND_UP) };
  const State expected_after_cleanup = rounding_to_nearest;
#endif

  void check_after_cleanup()
  {
    const State s = state();
    if (!(s == expected_after_cleanup)) {
      std::printf("FAILED rounding direction after the automatic cleanup: %s, rather than %s\n",
                  text(s).c_str(), text(expected_after_cleanup).c_str());
      std::fflush(stdout);
      std::_Exit(EXIT_FAILURE);
    }
    std::printf("rounding direction after the automatic cleanup: %s\n", text(s).c_str());
  }

  __attribute__((constructor(101))) void register_check_after_cleanup()
  {
    std::atexit(check_after_cleanup);
  }
}

int main()
{
#if GAOL_PRESERVE_ROUNDING
  check("rounding direction to nearest in main(), GAOL having initialized itself", state() == rounding_to_nearest,
        [] { return text(state()); });
#else
  check("rounding direction upward in main(), GAOL having initialized itself", state() == rounding_upward,
        [] { return text(state()); });
#endif

  // An operation of GAOL, then the direction downward, which main() leaves
  const interval x = interval(0.1, 0.3) * interval(1.5, 2.5) + 0.1;
  check("an operation of GAOL in main()", !x.is_empty());
  set(rounding_downward);

  return summary();
}
