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
* Copyright (c) 2026 ENSTA, France
*--------------------------------------------------------------------------
* gaol is a software distributed WITHOUT ANY WARRANTY. Read the associated
* COPYING file for information.
*--------------------------------------------------------------------------
* By: Frederic Goualard <Frederic.Goualard@lina.univ-nantes.fr>, Jordan NININ
*--------------------------------------------------------------------------*/

/*!
\file   gaol_fpu_fenv.h
\brief

FPU handling through C99 fenv.h facilities

\author Frederic Goualard, Jordan NININ
\date   2010-04-22
*/

/* fesetenv() on Linux

The original GAOL noted here that fesetenv() did not seem to work
correctly on Linux. fesetenv() was not at fault, but
what GAOL gave it: reset_fpu_cw() of GAOL 4 wrote GAOL_FPU_MASK into the control word
of fenv_t with fegetenv() and fesetenv(), the control word of the x87 unit
on x86 Linux and macOS, whereas GAOL's doubles are computed with SSE2
instructions, under MXCSR, which kept rounding to nearest. With glibc 2.31
on x86-64, the x87 control word then read 0x0a7f and fegetround(), which
reads it, returned FE_UPWARD, while MXCSR was 0x1f80 and 1 + 2^-60 was 1.
Only the x87 unit has a precision to set, and it computes none of GAOL's
doubles but exact ones and square roots GAOL corrects (the ldexp() of
mingw-w64, fscale, and the sqrt() of the C library, GAOL v5): gaol_config.h
refuses doubles computed on it, and the three builds give -msse2
-mfpmath=sse on 32-bit x86.

Since then:
- get_rounding() and set_rounding() (gaol_fpu.h) read and write the rounding
direction with fegetround() and fesetround(), and the rounding bits of MXCSR
as well; on a 32-bit x86 processor, and with mingw-w64 on x86-64
(GAOL_RND_MINGW64_X87, whose fegetround() joins the two units and whose
fesetround() clears the exception flags of MXCSR), the rounding bits of the
x87 control word and of MXCSR directly, with fnstcw and stmxcsr, fldcw and
ldmxcsr (GAOL v5). The get_fpu_cw() and reset_fpu_cw() of GAOL 4 read and wrote the
control word of the x87 unit on x86 Linux and macOS, which left the direction
of the SSE instructions unrestored, and 16 bits of the FPCR on 64-bit ARM,
which left out its rounding bits.
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

#ifndef GAOL_FPU_FENV_H
#define GAOL_FPU_FENV_H

#include "gaol/gaol_port.h"
#include <fenv.h>
#include <limits>

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

#if GAOL_USING_SSE2_INSTRUCTIONS
#  include <xmmintrin.h>
// Mask for SSE arithmetic (53 bits precision, rounding nearest, all exceptions masked)
#  define GAOL_SSE_MASK _MM_MASK_MASK
#endif


namespace gaol_core {
  
  /*
    nextafter(-DBL_MAX, -oo) is -oo, and nextafter(DBL_MAX, +oo) is +oo; the
    nextafter() of Cygwin computes that overflow as x + x, rounded in the
    direction in effect, which is upward for GAOL: -DBL_MAX, so that sinh of
    -711, whose value is below -DBL_MAX, had -DBL_MAX for lower bound and did
    not enclose it (GAOL v5). The two cases are told apart here.
  */
  GAOL_INLINE double previous_float(double d)
  {
    if (d == -(std::numeric_limits<double>::max)()) {
      return -GAOL_INFINITY;
    }
    return nextafter(d,-GAOL_INFINITY);
  }
  
  GAOL_INLINE double next_float(double d)
  {
    if (d == (std::numeric_limits<double>::max)()) {
      return GAOL_INFINITY;
    }
    return nextafter(d,GAOL_INFINITY);
  }
  
  #if GAOL_USING_SSE2_INSTRUCTIONS
  //! Sets rounding direction to -oo for SSE operations only
  GAOL_INLINE void round_downward_sse(void)
  {
    _mm_setcsr(GAOL_SSE_MASK|_MM_ROUND_DOWN);
  }
  
  //! Sets rounding direction to the nearest for SSE operations only
  GAOL_INLINE void round_to_nearest_sse(void)
  {
    _mm_setcsr(GAOL_SSE_MASK|_MM_ROUND_NEAREST);
  }
  
  
  /*! Sets rounding direction to +oo for SSE operations only, writing the
  whole of MXCSR: every exception masked, the flags and the modes
  flushing the subnormals to zero cleared. Only gaol::init() calls it,
  after fesetenv(FE_DFL_ENV); the operations write the rounding bits
  alone (gaol/gaol_fpu.h, GAOL v5) */
  GAOL_INLINE void round_upward_sse(void)
  {
    _mm_setcsr(GAOL_SSE_MASK|_MM_ROUND_UP);
  }
  #endif // GAOL_USING_SSE2_INSTRUCTIONS
  
  
  /*
  The rounding direction of the doubles computed from here on.
  
  On x86 processors, the control registers of the two floating-point units
  are written directly: the rounding bits of the x87 control word (fnstcw,
  fldcw) and of the SSE control register MXCSR (stmxcsr, ldmxcsr), what
  fesetround() does after checking its argument, through a call, which cost
  130 ns with mingw-w64 13, 50 ns with the C runtime of Visual C++ for x64 and
  250 ns for x86, 8.5 ns with glibc. The doubles of GAOL and of CORE-MATH are
  computed with SSE2 instructions (MXCSR); the x87 unit serves the C library's
  long doubles and, with MinGW-w64, some of its functions, whose results GAOL
  bounds whatever their rounding (sqrt). Both are set, as
  fesetround() sets them, so that fegetround() reads the direction set (with
  Visual C++ for x86, fegetround() returns -1 when the two differ). With
  Visual C++ for x64, MXCSR only: the x87 unit is not used there, and Visual
  C++ has no inline assembly for x64. Elsewhere, fesetround(), which the C
  library implements for the processor.
  
  The asm statements are volatile, with memory clobbered: the compiler keeps
  them where they are written and does not move loads and stores across them.
  As with fesetround(), the values computed before a change of direction go
  through rnd_keep() (gaol_fpu.h): GCC does not model the rounding direction.
  
  The mingw-w64 of 32-bit x86 before version 12 (MinGW-Builds GCC 12 and 13)
  had the direction read and written with <fenv.h> only, for a state of
  their own that fesetround() was thought to keep, which writing the
  registers directly would leave behind. Their fegetround() reads the x87
  control word, which is written here as well, and the wrong bounds seen
  then came from their fma() (gaol/gaol_config.h): with the mingw-w64 11 of
  Ubuntu, the tests pass with the registers written directly, under wine,
  and the registers are written so with every mingw-w64 for x86 (GAOL v5,
  point A.6 of TODO.md).
  */
  #if (defined(__i386__) || defined(__x86_64__)) && (defined(__GNUC__) || defined(__clang__))
  #  define GAOL_RND_X86_REGISTERS 1
  #  include <xmmintrin.h>
  GAOL_INLINE void gaol_set_rounding_x86(unsigned short x87_rc, unsigned int sse_rc)
  {
    unsigned short cw;
    __asm__ __volatile__ ("fnstcw %0" : "=m" (cw));
    cw = static_cast<unsigned short>((cw & 0xF3FFu) | x87_rc);
    __asm__ __volatile__ ("fldcw %0" : : "m" (cw) : "memory");
    _mm_setcsr((_mm_getcsr() & ~static_cast<unsigned int>(_MM_ROUND_MASK)) | sse_rc);
  }

  /*
  The control word of the x87 unit and MXCSR, read in one asm statement and
  without a call (GAOL v5): what the checks of the rounding direction of a
  32-bit x86 processor read (round_upward_if_needed() of gaol/gaol_fpu.h),
  where fegetround() cost about 80 ns with Visual C++. Volatile, so that no
  compiler reuses a read of them, as Clang 18 did with MXCSR in a loop that
  changed the direction. It does not order the read with the floating-point
  operations on values in registers, which no compiler models (GCC bug 34678):
  the operations read their bounds after the check, or through rnd_reread()
  (gaol/gaol_fpu.h). The values are the hardware ones, the rounding field of the control word
  being its bits 11 and 10 (0x0c00) and the one of MXCSR its bits 14 and 13
  (0x6000), and not the FE_* of <fenv.h>, which the C libraries give other
  values (0x0400 and 0x0800 with glibc, 0x0100 and 0x0200 with the C runtime
  of Windows).
  */
  GAOL_INLINE void gaol_read_x86_control(unsigned int& x87_cw, unsigned int& mxcsr)
  {
    unsigned short cw;
    unsigned int csr;
    __asm__ __volatile__ ("fnstcw %0\n\tstmxcsr %1" : "=m" (cw), "=m" (csr));
    x87_cw = cw;
    mxcsr = csr;
  }

  //! Writes the whole control word of the x87 unit
  GAOL_INLINE void gaol_write_x87_control(unsigned int x87_cw)
  {
    const unsigned short cw = static_cast<unsigned short>(x87_cw);
    __asm__ __volatile__ ("fldcw %0" : : "m" (cw) : "memory");
  }
  #elif defined(_MSC_VER) && (defined(_M_IX86) || defined(_M_X64))
  #  define GAOL_RND_X86_REGISTERS 1
  #  include <xmmintrin.h>
  GAOL_INLINE void gaol_set_rounding_x86(unsigned short x87_rc, unsigned int sse_rc)
  {
    #  if defined(_M_IX86)
    unsigned short cw;
    __asm fnstcw cw
    // The mask of the 16 bits of the control word, which the cast of
    // ~0x0C00u truncated (C4310 of Visual C++, GAOL v5)
    cw = static_cast<unsigned short>((cw & 0xF3FFu) | x87_rc);
    __asm fldcw cw
    #  else
    (void)x87_rc;
    #  endif
    _mm_setcsr((_mm_getcsr() & ~static_cast<unsigned int>(_MM_ROUND_MASK)) | sse_rc);
  }

  #  if defined(_M_IX86)
  /*
  The control word of the x87 unit and MXCSR, read by one block of inline
  assembly (GAOL v5; see the version of GCC and Clang above): Visual C++ for
  x86 takes it as opaque and keeps it where the source puts it, and has no
  intrinsic that reads the control word of the x87 unit without a call
  (_control87(), _controlfp() and __control87_2() are calls, and fegetround()
  of its C runtime costs about 80 ns). Not for x64, which has no inline
  assembly and does not use the x87 unit
  */
  GAOL_INLINE void gaol_read_x86_control(unsigned int& x87_cw, unsigned int& mxcsr)
  {
    unsigned short cw;
    unsigned int csr;
    __asm {
      fnstcw cw
      stmxcsr csr
    }
    x87_cw = cw;
    mxcsr = csr;
  }

  //! Writes the whole control word of the x87 unit
  GAOL_INLINE void gaol_write_x87_control(unsigned int x87_cw)
  {
    unsigned short cw = static_cast<unsigned short>(x87_cw);
    __asm fldcw cw
  }
  #  endif
  #else
  // 0, so that #if reads it under -Wundef (GAOL v5, point Z of TODO.md)
  #  define GAOL_RND_X86_REGISTERS 0
  #endif
  
  #if GAOL_RND_X86_REGISTERS
  GAOL_INLINE  void
  round_downward(void)
  {
    gaol_set_rounding_x86(0x0400, _MM_ROUND_DOWN);
  }
  
  GAOL_INLINE  void
  round_upward(void)
  {
    gaol_set_rounding_x86(0x0800, _MM_ROUND_UP);
  }
  
  GAOL_INLINE  void
  round_nearest(void)
  {
    gaol_set_rounding_x86(0x0000, _MM_ROUND_NEAREST);
  }
  #elif GAOL_RND_DIRECT
  /*
    The rounding direction of 64-bit ARM processors, which computes the
    doubles of the scalar and the vector instructions alike, is the field
    RMode (bits 23:22) of the control register FPCR, which mrs reads and msr
    writes; that of the 32-bit ARM processors with a floating-point unit, the
    same field of FPSCR, which vmrs reads and vmsr writes, and where the flags
    of the exceptions are too: fpcr_update() reads the register again just
    before it writes it, so that no flag raised since is lost, and the
    operations that write it from a value read at their start
    (fpcr_enter_upward()) do so before they compute (GAOL v5). Written only
    when it differs: msr on FPCR takes 4 ns when it writes the value it holds
    and 11 ns when it changes the direction (an Apple M2), where mrs takes
    0.8 ns, and fesetround() of the C library is a call that reads and writes
    it. FE_TONEAREST,
    FE_UPWARD, FE_DOWNWARD and FE_TOWARDZERO of the C libraries of 64-bit and
    32-bit ARM are these bits, which the assertion checks.
  */
  static_assert(FE_TONEAREST == 0 && FE_UPWARD == 0x400000 && FE_DOWNWARD == 0x800000
                && FE_TOWARDZERO == 0xc00000, "FE_* are the RMode bits of FPCR");

  // The control register, FPCR on 64-bit ARM and FPSCR on 32-bit ARM
#  if GAOL_USING_NEON_INSTRUCTIONS
  typedef unsigned long long fp_control;
#  else
  typedef unsigned int fp_control;
#  endif

  GAOL_INLINE fp_control fpcr_get(void)
  {
    fp_control fpcr;
#  if GAOL_USING_NEON_INSTRUCTIONS
    __asm__ __volatile__ ("mrs %0, fpcr" : "=r" (fpcr));
#  else
    __asm__ __volatile__ ("vmrs %0, fpscr" : "=r" (fpcr));
#  endif
    return fpcr;
  }

  // The "memory" clobber is a compiler barrier: the memory read after it is read after it
  GAOL_INLINE void fpcr_set(fp_control fpcr)
  {
#  if GAOL_USING_NEON_INSTRUCTIONS
    __asm__ __volatile__ ("msr fpcr, %0" : : "r" (fpcr) : "memory");
#  else
    __asm__ __volatile__ ("vmsr fpscr, %0" : : "r" (fpcr) : "memory");
#  endif
  }

  // The bits of FPCR selected by mask set to value (a subset of mask), and nothing else
  GAOL_INLINE void fpcr_update(fp_control mask, fp_control value)
  {
    const fp_control fpcr = fpcr_get();
    if ((fpcr & mask) != value) {
      fpcr_set((fpcr & ~mask) | value);
    }
  }

  const fp_control fpcr_rmode_mask = 0xc00000u;
  const fp_control fpcr_upward = 0x400000u;

  GAOL_INLINE  void
  round_downward(void)
  {
    fpcr_update(fpcr_rmode_mask, 0x800000u);
  }

  GAOL_INLINE  void
  round_upward(void)
  {
    fpcr_update(fpcr_rmode_mask, fpcr_upward);
  }

  GAOL_INLINE  void
  round_nearest(void)
  {
    fpcr_update(fpcr_rmode_mask, 0u);
  }
  #else
  // The generic path (POWER, s390x, RISC-V, MIPS...): the direction only. The
  // return value of fesetround() is not read, and the modes that flush the
  // subnormal numbers (NI of the FPSCR of POWER, FS of the FCSR of MIPS) are
  // not cleared, as FTZ, DAZ and FZ are by round_upward_if_needed() of
  // gaol_fpu.h (GAOL v5, see doc/using.md, "Flush-to-zero and
  // denormals-are-zero")
  GAOL_INLINE  void
  round_downward(void)
  {
    fesetround(FE_DOWNWARD);
  }
  
  GAOL_INLINE  void
  round_upward(void)
  {
    fesetround(FE_UPWARD);
  }
  
  GAOL_INLINE  void
  round_nearest(void)
  {
    fesetround(FE_TONEAREST);
  }
  #endif

} // namespace gaol_core

#endif /* GAOL_FPU_FENV_H */
