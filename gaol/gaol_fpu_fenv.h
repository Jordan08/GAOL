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
 * By: Frederic Goualard <Frederic.Goualard@lina.univ-nantes.fr>
 *--------------------------------------------------------------------------*/

/*!
  \file   gaol_fpu_fenv.h
  \brief

  FPU handling through C99 fenv.h facilities

  \author Frédéric Goualard
  \date   2010-04-22
*/

/* FIXME: fesetenv() does not seem to work correctly on Linux at present. 
          It is not a problem as long as proper restriction to 64 bits
          for floatint-point operands has been selected beforehand 
          (e.g., through Mathlib's Init_Lib()).
*/

#ifndef __gaol_fpu_fenv_h__
#define __gaol_fpu_fenv_h__

#include "gaol/gaol_port.h"
#include <fenv.h>

//  Mask 0x0a7f: 53 bits precision, all exceptions masked, rounding to +oo
// FIXME: Using an hexadecimal constant is not portable!
#define GAOL_FPU_MASK 0x0a3f

#if USING_SSE2_INSTRUCTIONS
#  include <xmmintrin.h>
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


INLINE  void
round_downward(void)
{
  fesetround(FE_DOWNWARD);
}

INLINE  void
round_upward(void)
{
  fesetround(FE_UPWARD);
}

INLINE  void
round_nearest(void)
{
  fesetround(FE_TONEAREST);
}

/* The rounding direction, with the functions of <fenv.h>. GAOL's operations
   save and restore it with get_rounding() and set_rounding() (gaol_fpu.h),
   which also read and write the one of the SSE instructions: these functions
   are kept for the code using them. They read and wrote the control word of
   the x87 unit on x86 Linux and macOS, which left the direction of the SSE
   instructions unrestored, and 16 bits of the FPCR on 64-bit ARM, which left
   out its rounding bits. */
INLINE unsigned short int get_fpu_cw()
{
  return (unsigned short int)fegetround();
}

INLINE void reset_fpu_cw(unsigned short int st)
{
  fesetround(st);
}

  /*!
    \brief Returns the opposite of the argument

    This macro is used to avoid the optimization if the negation is required
    for trust rounding.
   */
#if GAOL_USING_ASM
#   if IX86_LINUX || IX86_MACOSX
        INLINE double f_negate(double x)
        {
            asm volatile ("fldl %1; fchs; fstpl %0" : "=m" (x) : "m" (x));
            return x;
        }
#   else
        INLINE double f_negate(double x)
        {
            uintdouble id;
            id.d = x;
            HI_UINTDOUBLE(id) ^= 0x80000000; // XOR on sign bit
            return id.d;
        }
#   endif // IX86_LINUX
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

#endif /* __gaol_fpu_fenv_h__ */
