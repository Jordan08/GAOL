/*-*-C++-*------------------------------------------------------------------
 * gaol -- Just Another Interval Library
 *--------------------------------------------------------------------------
 * This file is part of the gaol distribution. Gaol was primarily
 * developed at the Swiss Federal Institute of Technology, Lausanne,
 * Switzerland, and is now developed at the Institut de Recherche
 * en Informatique de Nantes, France.
 *
 * Copyright (c) 2001 Swiss Federal Institute of Technology, Switzerland
 * Copyright (c) 2002-2010 Laboratoire d'Informatique de Nantes Atlantique, France
 *--------------------------------------------------------------------------
 * gaol is a software distributed WITHOUT ANY WARRANTY. Read the associated
 * COPYING file for information.
 *--------------------------------------------------------------------------
 * CVS: $Id$
 * By: Frederic Goualard <Frederic.Goualard@univ-nantes.fr>
 *--------------------------------------------------------------------------*/

/*!
  \file   gaol_fpu_msvc.h
  \brief

  FPU Handling when compiling with Microsoft Visual C++

  \author Frederic Goualard
  \date   2001-10-01
*/


#ifndef __gaol_fpu_msvc_h__
#define __gaol_fpu_msvc_h__

#include "gaol/gaol_port.h"

#include <float.h>

// The control word of the x87 unit of gaol_fpu_fenv.h: 53 bits of precision
// (0x0200), all exceptions masked (0x003f), rounding upward (0x0800). No
// longer used (see gaol_fpu_fenv.h), and kept for the code using it. The
// original GAOL noted that the hexadecimal constant was not portable; here it
// would not even mean the same: reset_fpu_cw() below gives its argument to
// _control87(), which takes the flags of <float.h> (_RC_UP, _PC_53...), not
// the bits of the register.
#define GAOL_FPU_MASK 0x0a3f


#if USING_SSE2_INSTRUCTIONS
#  include <xmmintrin.h>
#  include <intrin.h>
   // Mask for SSE arithmetic (53 bits precision, rounding nearest, all exceptions masked)
#  define GAOL_SSE_MASK _MM_MASK_MASK
#endif




namespace gaol_core {

INLINE double previous_float(double d)
{
	return nextafter(d,-GAOL_INFINITY);
}

INLINE double next_float(double d)
{
	return nextafter(d,GAOL_INFINITY);
}

#if USING_SSE2_INSTRUCTIONS
	//! Sets rounding direction to -oo for SSE operations only
	INLINE void round_downward_sse(void)
	{
		_mm_setcsr(GAOL_SSE_MASK|_MM_ROUND_DOWN);
	}

	//! Sets rounding direction to the nearest for SSE operations only
	INLINE void round_to_nearest_sse(void)
	{
		_mm_setcsr(GAOL_SSE_MASK|_MM_ROUND_NEAREST);
	}


	//! Sets rounding direction to +oo for SSE operations only
	INLINE void round_upward_sse(void)
	{
		_mm_setcsr(GAOL_SSE_MASK|_MM_ROUND_UP);
	}
#endif // USING_SSE2_INSTRUCTIONS


/*
  The rounding direction of the doubles computed from here on, with
  _control87() of the C runtime. This header serves a Visual C++ without
  <fenv.h> only (gaol_fpu.h): with <fenv.h>, which Visual C++ has had since
  2013, gaol_fpu_fenv.h is used, and writes the control registers itself.
*/
INLINE  void
round_downward(void)
{
	_control87(_RC_DOWN,_MCW_RC);
}

INLINE  void
round_upward(void)
{
	_control87(_RC_UP,_MCW_RC);
}

INLINE  void
round_nearest(void)
{
	_control87(_RC_NEAR,_MCW_RC);
}

INLINE unsigned short int get_fpu_cw()
{
  return _control87(0,0);
}

INLINE void reset_fpu_cw(unsigned short int st)
{
	_control87(st,_MCW_DN|_MCW_EM|_MCW_IC|_MCW_RC|_MCW_PC);
}

  /*!
    \brief Returns the opposite of the argument

    This macro is used to avoid the optimization if the negation is required
    for trust rounding.
   */
#if GAOL_USING_ASM
	INLINE double f_negate(double x)
    {
		__asm {
            fld x
            fchs
            fstp x
        }
         return x;
    }
#else
    INLINE double f_negate(double x)
    {
        uintdouble id;
        id.d = x;
        HI_UINTDOUBLE(id) ^= 0x80000000; // XOR on sign bit
        return id.d;
    }
#endif // GAOL_USING_ASM

} // namespace gaol_core

#endif /* __gaol_fpu_msvc_h__ */
