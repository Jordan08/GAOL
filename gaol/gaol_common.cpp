/*-*-C++-*------------------------------------------------------------------
 * GAOL -- not Just Another Interval Library
 *--------------------------------------------------------------------------
 * Copyright (c) 2001 Swiss Federal Institute of Technology, Switzerland
 * Copyright (c) 2002-2006 Laboratoire d'Informatique de
 *                         Nantes-Atlantique, France
 * Copyright (c) 2026 ENSTA, France
 *
 * By: Frederic Goualard, Jordan NININ
 *--------------------------------------------------------------------------
 * GAOL is a software distributed WITHOUT ANY WARRANTY. Read the associated
 * COPYING.LIB file for information.
 *--------------------------------------------------------------------------*/

/*!
\file   gaol_common.cpp
\brief  Implementation for functions of common use

\author Frederic Goualard, Jordan NININ
\date   2001-10-03
*/


#include <iostream>
#include <cstdio>
#include <cstdlib>
#include <cmath>
#include <cstdint>
#include "gaol/gaol_config.h"
#include "gaol/gaol_fpu.h"
#include "gaol/gaol_common.h"
#include "gaol/parser/gaol_expression.h"
#include "gaol/gaol_init_cleanup.h"

#include "gaol/math/gaol_double_op.h"

#if GAOL_USING_SSE2_INSTRUCTIONS && GAOL_AVX512_ARITHMETIC
/* Whether the processor runs the AVX-512 instructions (AVX512F), and the
   system keeps the state of their registers, for init() below. GCC and Clang
   tell it with __builtin_cpu_supports(), which needs __builtin_cpu_init()
   first in a function run before the constructors. Visual C++ has neither,
   and the built-ins of clang-cl need the runtime library of LLVM at the
   link: for both, cpuid tells the processor has AVX512F (leaf 7, ebx bit 16)
   and the operating system uses XSAVE (leaf 1, ecx bit 27), and xgetbv that
   it keeps the state of the SSE, AVX and AVX-512 registers (bits 1, 2, 5, 6
   and 7 of XCR0), as __builtin_cpu_supports() checks (GAOL v5). */
#  if defined(_MSC_VER)
#    include <intrin.h>
namespace {
  bool cpu_runs_avx512f()
  {
    int regs[4];
    __cpuid(regs, 0);
    if (regs[0] < 7) {
      return false;
    }
    __cpuid(regs, 1);
    if ((regs[2] & (1 << 27)) == 0 || (_xgetbv(0) & 0xe6) != 0xe6) {
      return false;
    }
    __cpuidex(regs, 7, 0);
    return (regs[1] & (1 << 16)) != 0;
  }
}
#    define GAOL_CPU_RUNS_AVX512F() cpu_runs_avx512f()
#  else
#    define GAOL_CPU_RUNS_AVX512F() (__builtin_cpu_init(), __builtin_cpu_supports("avx512f") != 0)
#  endif
#endif

namespace gaol_core {
  
  /* CORE-MATH has no state to save and no library to initialise: it computes
  in the rounding direction in effect, and changes nothing of the
  floating-point environment (GAOL v5). */
  
  static bool _already_cleaned = false;
  static bool _already_initialized = false;
  
  #if !GAOL_PRESERVE_ROUNDING
  /* The rounding direction the first call of init() found, before setting it
  upward: cleanup() sets it back, the x87 unit and the SSE instructions
  each theirs (GAOL v5). That first call is the automatic one, before the
  static objects of the program are constructed (gaol_initializer,
  gaol/gaol_common.h), and cleanup() runs when the program ends or the
  library is unloaded, if the program did not call it. Constant-initialized,
  as the two flags above: the automatic init() may run before the dynamic
  initialization of this file. */
  static rounding_state _initial_rounding;
  #endif
  
  int debug_level;

  /* Stops the program where the rounding direction set upward does not round
  upward (GAOL v5): the generic path of gaol_fpu_fenv.h (POWER, s390x,
  RISC-V, MIPS...) sets it with fesetround(FE_UPWARD), whose result it does
  not read, and which does nothing where there is no floating-point unit
  (32-bit ARM with the floating point in software, MIPS or RISC-V without
  it...), so that every bound would be rounded to nearest, in silence. The
  sum 1 + 2^-60, inexact, is above 1 upward only; its operands are volatile,
  so that it is computed here, under the direction set. The flag of the
  inexact exception it raises is set back as it was. Told in C stdio, as the
  processor without AVX-512 in init(). On Debian armel (32-bit ARM, the
  floating point in software, GCC 12), fesetround(FE_UPWARD) returns 0 and
  changes nothing, and GAOL computed every bound to nearest; valgrind
  (memcheck 3.26 on x86-64) computes the SSE instructions to nearest whatever
  MXCSR says, and a program using GAOL stops there too. */
  static void check_upward_rounding()
  {
    fexcept_t inexact;
    fegetexceptflag(&inexact, FE_INEXACT);
    volatile double one = 1.0, tiny = std::ldexp(1.0, -60);
    const bool upward = one + tiny > 1.0;
    fesetexceptflag(&inexact, FE_INEXACT);
    if (!upward) {
      std::fputs("[gaol error]: the rounding direction cannot be set upward on this"
                 " target (fesetround(FE_UPWARD) has no effect there, as without a"
                 " floating-point unit): every bound GAOL computes would be rounded"
                 " to nearest, and would not enclose the exact results\n", stderr);
      std::abort();
    }
  }
  
  bool init(int dbg_lvl)
  {
    if (!_already_initialized) {
      debug_level = dbg_lvl;
      
      #if GAOL_USING_SSE2_INSTRUCTIONS && GAOL_AVX512_ARITHMETIC
      /* The AVX-512 path, which the configuration took (gaol_configuration.h):
      on a processor without the instructions, the first operation would stop
      on an illegal instruction without a word, as on a node of a cluster
      that has not the processor of the machine that built GAOL, or under
      valgrind, which does not decode them. Told once here, in C stdio,
      which the initialization of a static object can use before the
      streams of C++ are built, and __builtin_cpu_init() first, which
      __builtin_cpu_supports() needs in a function run before the
      constructors (GAOL v5). */
      if (!GAOL_CPU_RUNS_AVX512F()) {
        std::fputs("[gaol error]: this GAOL was built for the AVX-512 instructions"
                   " (GAOL_PREFER_AVX512, on by default with GAOL_PRESERVE_ROUNDING),"
                   " which this processor or system does not run: build it with"
                   " -DGAOL_PREFER_AVX512=OFF, --disable-prefer-avx512 or"
                   " -Denable-prefer-avx512=false\n", stderr);
        std::abort();
      }
      #endif

      #if !GAOL_PRESERVE_ROUNDING
      _initial_rounding = get_rounding();
      fesetenv(FE_DFL_ENV);
      round_upward();
      #   if GAOL_USING_SSE2_INSTRUCTIONS
      round_upward_sse();
      #   endif
      check_upward_rounding();
      #else
      {
        // As an operation sets it, the program's direction and modes set back
        GAOL_RND_ENTER();
        check_upward_rounding();
        GAOL_RND_LEAVE();
      }
      #endif
      // Not counted: expr_node::inc_refcount() leaves it alone (GAOL v5)
      the_null_expr = new null_node;
      interval::precision(16);
      _already_initialized=true;
      return true;
    } else {
      debug_level = dbg_lvl;
      #if !GAOL_PRESERVE_ROUNDING
      // The rounding direction may have changed since the automatic
      // initialization: the C runtime of Windows resets it when the
      // program starts, after the constructor of GAOL's DLL ran
      round_upward();
      #endif
      return false;
    }
  }
  
  bool cleanup(void)
  {
    if (!_already_cleaned) {
      #if !GAOL_PRESERVE_ROUNDING
      // Only the rounding direction: the exception flags raised since
      // init() are the program's
      if (_already_initialized) {
        set_rounding(_initial_rounding);
      }
      #endif
      _already_cleaned=true;
      return true;
    } else {
      return false;
    }
  }
  
  void restore_rounding(void)
  {
    #if !GAOL_PRESERVE_ROUNDING
    // Only the rounding direction, as cleanup() above: the exception flags
    // raised since init() are the program's. As many times as the program
    // needs, where cleanup() sets it back at its first call only
    if (_already_initialized) {
      set_rounding(_initial_rounding);
    }
    #endif
  }
  
  /* What init() allocated, freed by the automatic cleanup only, when the
  program ends or the library is unloaded, after the static objects
  constructed since GAOL initialized itself are destroyed (GAOL v5).
  cleanup(), which the program calls after its last use of GAOL, deleted
  the_null_expr while expressions still referred to it: their destructors
  then wrote into freed memory. */
  void free_initialization(void)
  {
    delete the_null_expr;
    the_null_expr = 0;
  }
  
  // A null file name is "(unknown file)", and a null message an empty text
  // (gaol_detail::known_file() and known_text() of gaol/gaol_common.h; GAOL v5)
  void gaol_warning(const char *file, int line, const char *warn)
  {
    std::cerr << "[gaol warning in " << ::gaol_detail::known_file(file) << ':' << line << "]: " << ::gaol_detail::known_text(warn) << std::endl;
  }
  
  void gaol_warning(const char *warn)
  {
    std::cerr << "[gaol warning]: " << ::gaol_detail::known_text(warn) << std::endl;
  }
  
  void gaol_error(const char *file, int line, const char *err)
  {
    std::cerr << "[gaol error in " << ::gaol_detail::known_file(file) << ':' << line << "]: " << ::gaol_detail::known_text(err) << std::endl;
  }
  
  void gaol_error(const char *err)
  {
    std::cerr << "[gaol error]: " << ::gaol_detail::known_text(err) << std::endl;
  }
  
  /* The larger and the smaller of two doubles, +0 and -0 for two zeros of
  different signs. The equal values are told apart by their sign bit, which
  the comparisons do not see: Visual C++ compiled (b <= a) ? b : a as the
  instruction minsd, which gives its second operand for two zeros, and
  minimum(0.0, -0.0) was +0 and maximum(0.0, -0.0) -0, which
  tests/float_functions.cpp (check/ of GAOL 4) found once built with it
  (GAOL v5). Two values the comparisons find equal are compared as bounds
  first (gaol_detail::bound_key(), gaol_port.h): under denormals-are-zero,
  which max(), min(), abs() and mag() leave set, two subnormals compare
  equal, and maximum(3*2^-1074, 100*2^-1074) was 3*2^-1074 (GAOL v5). Where the
  modes are cleared, this is made only for
  equal values. */
  double maximum(double a, double b)
  {
    if (std::isnan(a) || std::isnan(b)) {
      return GAOL_NAN;
    }
    if (a > b) {
      return a;
    }
    if (b > a) {
      return b;
    }
    // Equal as doubles: the greater as bounds, +0 where one of them is +0
    const std::int64_t ka = gaol_detail::bound_key(a), kb = gaol_detail::bound_key(b);
    if (ka != kb) {
      return (ka > kb) ? a : b;
    }
    return is_signed(a) ? b : a;
  }
  
  double minimum(double a, double b)
  {
    if (std::isnan(a) || std::isnan(b)) {
      return GAOL_NAN;
    }
    if (a < b) {
      return a;
    }
    if (b < a) {
      return b;
    }
    // Equal as doubles: the smaller as bounds, -0 where one of them is -0
    const std::int64_t ka = gaol_detail::bound_key(a), kb = gaol_detail::bound_key(b);
    if (ka != kb) {
      return (ka < kb) ? a : b;
    }
    return is_signed(a) ? a : b;
  }
  
  
  
  
  
  
  
  
  
  
} // namespace gaol_core



/**
* \mainpage gaol (Just Another Interval Library)
* \section authors Authors
* Jordan Ninin    \<jordan.ninin@ensta.fr\>: GAOL v5, which continues the
* GAOL of Frederic Goualard from its version 4.2.2 <P>
* Frederic Goualard    \<Frederic.Goualard@irin.univ-nantes.fr\> <P>
* \section copyright Copyright Notice
* <tt>
<hr>
Read the COPYING file for the LEGAL NOTICE.


<hr>
</tt>
*/
