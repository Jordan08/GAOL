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
 *--------------------------------------------------------------------------
 * gaol is a software distributed WITHOUT ANY WARRANTY. Read the associated
 * COPYING file for information.
 *--------------------------------------------------------------------------*/

/*!
  \file   gaol_common.cpp
  \brief  Implementation for functions of common use

  \author Frederic Goualard
  \date   2001-10-03
*/


#include <iostream>
#include <cstdio>
#include <cmath>
#include "gaol/gaol_config.h"
#include "gaol/gaol_fpu.h"
#include "gaol/gaol_common.h"
#include "gaol/gaol_expression.h"
#include "gaol/gaol_init_cleanup.h"

#include "gaol/gaol_double_op.h"

namespace gaol_core {

  /* CORE-MATH has no state to save and no library to initialise: it computes
     in the rounding direction in effect, and changes nothing of the
     floating-point environment (GAOL v5). */

  static bool _already_cleaned = false;
  static bool _already_initialized = false;

#if GAOL_USING_SSE2_INSTRUCTIONS && GAOL_HAVE_AVX512_TARGET
  /* Whether +, -, *, / and sqrt take the AVX-512 path of
     gaol/gaol_interval_avx512.cpp: init() sets it, from the
     GAOL_PREFER_AVX512 of the build and the instructions the processor
     has, which __builtin_cpu_supports() tells. The operations read it,
     nothing else (GAOL v5). */
  bool avx512_arithmetic = false;
#endif

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
#if GAOL_USING_SSE2_INSTRUCTIONS && GAOL_HAVE_AVX512_TARGET
#  if GAOL_PREFER_AVX512
            // The AVX-512 path of +, -, *, / and sqrt, when the processor
            // has the instructions (GAOL v5, gaol/gaol_interval_avx512.cpp)
            avx512_arithmetic = __builtin_cpu_supports("avx512f") != 0;
#  endif
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
     (GAOL v5). */
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
    // Equal: +0 where one of them is +0
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
    // Equal: -0 where one of them is -0
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
