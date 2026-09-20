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

  \author Frederic Goualard
  \date   2010-04-22
*/

/* fesetenv() on Linux

   The original GAOL noted here that fesetenv() did not seem to work
   correctly on Linux, which did not matter once mathlib's Init_Lib() had set
   the precision of the x87 unit to 53 bits. fesetenv() was not at fault, but
   what GAOL gave it: reset_fpu_cw() wrote GAOL_FPU_MASK into the control word
   of fenv_t with fegetenv() and fesetenv(), the control word of the x87 unit
   on x86 Linux and macOS, whereas GAOL's doubles are computed with SSE2
   instructions, under MXCSR, which kept rounding to nearest. With glibc 2.31
   on x86-64, the x87 control word then read 0x0a7f and fegetround(), which
   reads it, returned FE_UPWARD, while MXCSR was 0x1f80 and 1 + 2^-60 was 1.
   Only the x87 unit has a precision to set, and it computes none of GAOL's
   doubles: gaol_config.h refuses doubles computed on it, and the three builds
   give -msse2 -mfpmath=sse on 32-bit x86.

   Since then:
   - get_fpu_cw() and reset_fpu_cw() (below) read and write the rounding
     direction with fegetround() and fesetround(), and get_rounding() and
     set_rounding() (gaol_fpu.h) the rounding bits of MXCSR as well.
   - gaol::init() calls fesetenv(FE_DFL_ENV), then round_upward(), which sets
     both units: with glibc 2.31 on x86-64, the x87 control word is then
     0x0b7f and MXCSR 0x5f80, and 1 + 2^-60 is above 1. FE_DFL_ENV sets the
     precision of the x87 unit back to 64 bits (control word 0x037f),
     whatever Init_Lib() set: doubles computed on the x87 unit would be
     wrong, which is why gaol_config.h refuses them.
   - GAOL's bounds do not depend on the direction gaol::init() leaves: each
     operation sets it upward when it is not (round_upward_if_needed() in
     gaol_fpu.h), which tests/rounding_direction checks on each platform of
     the continuous integration, on x86 with the x87 and SSE directions
     differing too.
*/

#ifndef __gaol_fpu_fenv_h__
#define __gaol_fpu_fenv_h__

#include "gaol/gaol_port.h"
#include <fenv.h>

// The control word of the x87 unit that reset_fpu_cw() of the original GAOL
// wrote: 53 bits of precision (0x0200), all exceptions masked (0x003f),
// rounding upward (0x0800), read back as 0x0a7f, bit 6 being reserved and set.
// No longer used, and kept for the code using it. The original GAOL noted that
// the hexadecimal constant was not portable: its fields are those of the x87
// unit only. Written into the FPCR of 64-bit ARM Linux, the control word of
// fenv_t there, it leaves the rounding to nearest (bits 22 and 23 clear) and
// enables the traps of division by zero and of underflow (bits 9 and 11); the
// patch IBEX applies to GAOL removed the call, which crashed an ARM64 Mac.
// The rounding direction is now written with fesetround(), or with the
// constants of <xmmintrin.h> for MXCSR. The rounding bits of the x87 control
// word are written in hexadecimal in gaol_set_rounding_x86() below, compiled
// for x86 processors only: glibc names them in <fpu_control.h> (_FPU_RC_UP is
// 0x800), not every C library.
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


/*
  The rounding direction of the doubles computed from here on.

  On x86 processors, the control registers of the two floating-point units
  are written directly: the rounding bits of the x87 control word (fnstcw,
  fldcw) and of the SSE control register MXCSR (stmxcsr, ldmxcsr), what
  fesetround() does after checking its argument, through a call. GAOL changes
  the direction twice for each exp(), log(), sin() or cos() of an interval
  (to nearest before mathlib, upward after, see GAOL_RND_NEAREST_ENTER()), and
  fesetround() cost 130 ns per call with mingw-w64 13, 50 ns with the C
  runtime of Visual C++ for x64 and 250 ns for x86, 8.5 ns with glibc.
  The doubles of GAOL and of mathlib are computed with SSE2 instructions
  (MXCSR); the x87 unit serves the C library's long doubles and, with
  MinGW-w64, some of its functions, whose results GAOL widens (the hyperbolic
  functions) or bounds whatever their rounding (sqrt). Both are set, as
  fesetround() sets them, so that fegetround() reads the direction set (with
  Visual C++ for x86, fegetround() returns -1 when the two differ). With
  Visual C++ for x64, MXCSR only: the x87 unit is not used there, and Visual
  C++ has no inline assembly for x64. Elsewhere, fesetround(), which the C
  library implements for the processor.

  The asm statements are volatile, with memory clobbered: the compiler keeps
  them where they are written and does not move loads and stores across them.
  As with fesetround(), the values computed before a change of direction go
  through rnd_keep() (gaol_fpu.h): GCC does not model the rounding direction.
*/
#if (defined(__i386__) || defined(__x86_64__)) && (defined(__GNUC__) || defined(__clang__))
#  define GAOL_RND_X86_REGISTERS 1
#  include <xmmintrin.h>
INLINE void gaol_set_rounding_x86(unsigned short x87_rc, unsigned int sse_rc)
{
  unsigned short cw;
  __asm__ __volatile__ ("fnstcw %0" : "=m" (cw));
  cw = (unsigned short)((cw & (unsigned short)~0x0C00u) | x87_rc);
  __asm__ __volatile__ ("fldcw %0" : : "m" (cw) : "memory");
  _mm_setcsr((_mm_getcsr() & ~(unsigned int)_MM_ROUND_MASK) | sse_rc);
}
#elif defined(_MSC_VER) && (defined(_M_IX86) || defined(_M_X64))
#  define GAOL_RND_X86_REGISTERS 1
#  include <xmmintrin.h>
INLINE void gaol_set_rounding_x86(unsigned short x87_rc, unsigned int sse_rc)
{
#  if defined(_M_IX86)
  unsigned short cw;
  __asm fnstcw cw
  cw = (unsigned short)((cw & (unsigned short)~0x0C00u) | x87_rc);
  __asm fldcw cw
#  else
  (void)x87_rc;
#  endif
  _mm_setcsr((_mm_getcsr() & ~(unsigned int)_MM_ROUND_MASK) | sse_rc);
}
#endif

#if GAOL_RND_X86_REGISTERS
INLINE  void
round_downward(void)
{
  gaol_set_rounding_x86(0x0400, _MM_ROUND_DOWN);
}

INLINE  void
round_upward(void)
{
  gaol_set_rounding_x86(0x0800, _MM_ROUND_UP);
}

INLINE  void
round_nearest(void)
{
  gaol_set_rounding_x86(0x0000, _MM_ROUND_NEAREST);
}
#else
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
#endif

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
