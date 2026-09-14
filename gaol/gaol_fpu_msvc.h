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

  \author Frédéric Goualard
  \date   2001-10-01
*/


#ifndef __gaol_fpu_msvc_h__
#define __gaol_fpu_msvc_h__

#include "gaol/gaol_port.h"

#include <float.h>

//  Mask 0x0a7f: 53 bits precision, all exceptions masked, rounding to +oo
// FIXME: Using an hexadecimal constant is not portable!
#define GAOL_FPU_MASK 0x0a3f


#if USING_SSE2_INSTRUCTIONS
#  include <xmmintrin.h>
#  include <intrin.h>
   // Mask for SSE arithmetic (53 bits precision, rounding nearest, all exceptions masked)
#  define GAOL_SSE_MASK _MM_MASK_MASK
#endif




namespace gaol {

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
  The rounding direction of the doubles computed from here on (see
  gaol_fpu_fenv.h). With Visual C++ for x64, the doubles are computed with
  SSE2 instructions and the x87 unit is not used: the rounding bits of MXCSR
  are written directly (_mm_setcsr), where _control87() cost some 60 ns per
  call. For x86, the x87 control word too, in assembly. For ARM, _control87().
*/
#if defined(_M_X64)
#  include <xmmintrin.h>
INLINE void gaol_set_rounding_msvc(unsigned int x87_rc, unsigned int sse_rc)
{
	(void)x87_rc;
	_mm_setcsr((_mm_getcsr() & ~_MM_ROUND_MASK) | sse_rc);
}
#elif defined(_M_IX86)
#  include <xmmintrin.h>
INLINE void gaol_set_rounding_msvc(unsigned int x87_rc, unsigned int sse_rc)
{
	unsigned short cw;
	__asm fnstcw cw
	cw = (unsigned short)((cw & (unsigned short)~0x0C00u) | (unsigned short)x87_rc);
	__asm fldcw cw
	_mm_setcsr((_mm_getcsr() & ~_MM_ROUND_MASK) | sse_rc);
}
#endif

#if defined(_M_X64) || defined(_M_IX86)
INLINE  void
round_downward(void)
{
	gaol_set_rounding_msvc(0x0400, _MM_ROUND_DOWN);
}

INLINE  void
round_upward(void)
{
	gaol_set_rounding_msvc(0x0800, _MM_ROUND_UP);
}

INLINE  void
round_nearest(void)
{
	gaol_set_rounding_msvc(0x0000, _MM_ROUND_NEAREST);
}
#else
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
#endif

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

} // namespace gaol

#endif /* __gaol_fpu_msvc_h__ */
