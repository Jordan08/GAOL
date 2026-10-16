/*-*-C++-*------------------------------------------------------------------
* gaol -- NOT Just Another Interval Library
*--------------------------------------------------------------------------
* This file is part of the gaol distribution. Gaol was primarily
* developed at the Swiss Federal Institute of Technology, Lausanne,
* Switzerland, and is now developed at the Laboratoire d'Informatique de
* Nantes-Atlantique, France.
*
* Copyright (c) 2001 Swiss Federal Institute of Technology, Switzerland
* Copyright (c) 2002-2006 Laboratoire d'Informatique de
*                         Nantes-Atlantique, France
* Copyright (c) 2026 ENSTA, France
*--------------------------------------------------------------------------
* gaol is a software distributed WITHOUT ANY WARRANTY. Read the associated
* COPYING file for information.
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

#include "gaol/gaol_double_op.h"

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
      constructors (GAOL v5, point AA of TODO.md). */
      __builtin_cpu_init();
      if (__builtin_cpu_supports("avx512f") == 0) {
        std::fputs("[gaol error]: this GAOL was built for the AVX-512 instructions"
                   " (GAOL_PREFER_AVX512, on by default with GAOL_PRESERVE_ROUNDING),"
                   " which this processor does not have: build it with"
                   " -DGAOL_PREFER_AVX512=OFF, --disable-prefer-avx512 or"
                   " -Denable-prefer-avx512=false\n", stderr);
        std::abort();
      }
      #endif

      #if !GAOL_PRESERVE_ROUNDING
      _initial_rounding = get_rounding();
      fesetenv(FE_DFL_ENV);
      round_upward();
      // next instruction crashes on MacOS ARM64 platform
      //reset_fpu_cw(GAOL_FPU_MASK); // 53 bits precision, all exceptions masked, rounding to +oo
      #   if GAOL_USING_SSE2_INSTRUCTIONS
      round_upward_sse();
      #   endif
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
  
  void gaol_warning(const char *file, int line, const char *warn)
  {
    std::cerr << "[gaol warning in " << file << ':' << line << "]: " << warn << std::endl;
  }
  
  void gaol_warning(const char *warn)
  {
    std::cerr << "[gaol warning]: " << warn << std::endl;
  }
  
  void gaol_error(const char *file, int line, const char *err)
  {
    std::cerr << "[gaol error in " << file << ':' << line << "]: " << err << std::endl;
  }
  
  void gaol_error(const char *err)
  {
    std::cerr << "[gaol error]: " << err << std::endl;
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
  equal, and maximum(3*2^-1074, 100*2^-1074) was 3*2^-1074 (GAOL v5,
  point Q of TODO.md). Where the modes are cleared, this is made only for
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
