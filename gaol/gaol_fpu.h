/*-*-C++-*------------------------------------------------------------------
* gaol -- Just Another Interval Library
*--------------------------------------------------------------------------
* This file is part of the gaol distribution. Gaol was primarily
* developed at the Swiss Federal Institute of Technology, Lausanne,
* Switzerland, and is now developed at the Institut de Recherche
* en Informatique de Nantes, France.
*
* Copyright (c) 2001 Swiss Federal Institute of Technology, Switzerland
* Copyright (c) 2002 Institut de Recherche en Informatique de Nantes, France
* Copyright (c) 2026 ENSTA, France
*--------------------------------------------------------------------------
* gaol is a software distributed WITHOUT ANY WARRANTY. Read the associated
* COPYING file for information.
*--------------------------------------------------------------------------
* By: Frederic Goualard <Frederic.Goualard@lina.univ-nantes.fr>, Jordan NININ
*--------------------------------------------------------------------------*/

/*!
\file   gaol_fpu.h
\brief

<long description>

\author Frederic Goualard, Jordan NININ
\date   2001-10-01
*/


#ifndef GAOL_FPU_H
#define GAOL_FPU_H

#include <cfloat>
#include <cmath>
#include <cstddef>
#include "gaol/gaol_config.h"

#if defined(__x86_64__) || defined(_M_X64) || defined(__SSE2__) || (defined(_M_IX86_FP) && _M_IX86_FP >= 2)
#  include <xmmintrin.h>
// The rounding direction of the SSE instructions, which compute the doubles
// there, is read and written through their control register: fegetround()
// may only read the one of the x87 unit (glibc on x86-64)
#  define GAOL_RND_SSE_REGISTER 1
#else
// 0, so that #if reads it under -Wundef, as the other macros of this header
// (GAOL v5, point Z of TODO.md)
#  define GAOL_RND_SSE_REGISTER 0
#endif

/* The modes that flush the subnormal numbers to zero, as bits of the control
register holding them, where GAOL can read and write them (GAOL v5). With
them, an operation with a subnormal operand or result gives a zero:
[1e-300]*[1e-20] is [0, 0]. GCC links crtfastmath.o into a program linked
with -Ofast, -ffast-math or -funsafe-math-optimizations, whose constructor
sets them on x86 and on ARM Linux; so does Clang on Linux. GAOL clears them
(see round_upward_if_needed()):
- on x86 processors, flush-to-zero (FTZ, bit 15), which flushes the
subnormal results, and denormals-are-zero (DAZ, bit 6), which reads the
subnormal operands as zeros, of MXCSR (_MM_FLUSH_ZERO_MASK, and
_MM_DENORMALS_ZERO_MASK of <pmmintrin.h>);
- on 64-bit ARM processors, with GCC and Clang, FZ (bit 24), which flushes
the subnormal operands and results, and FIZ (bit 0, from Armv8.7 on),
which flushes the subnormal operands, of FPCR (instructions mrs and msr).
Not with Visual C++, whose C runtime sets them only when the program asks
for it (_controlfp());
- on 32-bit ARM processors with a floating-point unit, FZ (bit 24) of FPSCR
(instructions vmrs and vmsr), with GCC, the only compiler GAOL takes
there.
On the other processors (POWER, s390x, RISC-V, MIPS...), whose FPU intervals
take the generic round_upward() of gaol_fpu_fenv.h, GAOL clears none: the
mode NI (non-IEEE) of the FPSCR of POWER and the bit FS of the FCSR of MIPS,
states of the same kind as FTZ and DAZ, stay as the program left them (not
tested on these processors; GAOL v5). */
#if GAOL_RND_SSE_REGISTER
#  define GAOL_RND_FLUSH_BITS 0x8040u
#elif defined(__aarch64__) && defined(__GNUC__)
#  define GAOL_RND_FPCR_REGISTER 1
#  define GAOL_RND_FLUSH_BITS 0x1000001u
#elif defined(__arm__) && defined(__ARM_FP) && defined(__GNUC__)
#  define GAOL_RND_FPSCR_REGISTER 1
#  define GAOL_RND_FLUSH_BITS 0x1000000u
#endif
#ifndef GAOL_RND_FPCR_REGISTER
#  define GAOL_RND_FPCR_REGISTER 0
#endif
#ifndef GAOL_RND_FPSCR_REGISTER
#  define GAOL_RND_FPSCR_REGISTER 0
#endif

/* A compiler barrier: the memory read after it is read after it, and no
comparison of a bound read after it is made before it (see
round_upward_if_needed()). None for Visual C++, which keeps the
floating-point operations where the source puts them with respect to the
writes of the control register under /fp:strict, with which GAOL is
compiled (tests/rounding_direction.cpp checks it in the continuous
integration); not the intrinsics with an embedded rounding of the AVX-512
path, which it moves past them (gaol/gaol_interval_avx512.cpp). */
#if defined(__GNUC__) || defined(__clang__)
#  define GAOL_RND_BARRIER() __asm__ __volatile__ ("" : : : "memory")
#else
#  define GAOL_RND_BARRIER() ((void)0)
#endif

/* GAOL_RND_NOINLINE: what Visual C++ is asked to call rather than expand, the
writes of the control registers of a 32-bit x86 processor
(gaol_core::x86_set_modes()). With them expanded in the check that comes
before them, Visual C++ 19 for x86 called the check from each operation; with
them out of line, it expands the check in the operation, two comparisons, and
calls the writes only when a register has to change (GAOL v5). Nothing for
the other compilers, which put the code of a branch marked GAOL_UNLIKELY out
of the way */
#if defined(_MSC_VER) && !defined(__clang__)
#  define GAOL_RND_NOINLINE __declspec(noinline)
#else
#  define GAOL_RND_NOINLINE
#endif

// Doubles computed in double precision, whose rounding direction an addition
// shows (see round_upward_if_needed())
#if (defined(FLT_EVAL_METHOD) && FLT_EVAL_METHOD == 0) || defined(_M_X64) || defined(_M_ARM64) \
|| defined(_M_ARM) || (defined(_M_IX86_FP) && _M_IX86_FP >= 2)
#  define GAOL_RND_PROBE 1
#endif
/* Not on a 32-bit x86 processor, where the x87 unit and the SSE instructions
each have their own rounding direction: the probe is an arithmetic operation,
so it only sees the direction the doubles are computed with (SSE), and the
other one could stay elsewhere. The elementary functions of CORE-MATH read
the direction with fegetround(), which gives the one of the x87 unit on
Windows: with the two differing, pow() rounded as if to nearest while GAOL
computed upward, and gave bounds one double apart from the ones it gives
when the two agree (GAOL v5, found by tests/rounding_direction.cpp in
the continuous integration, with Visual C++ for 32-bit x86). There the
direction of both units is read directly (GAOL_RND_X86_DIRECT below), which
fegetround() did through a call that cost about 80 ns with Visual C++, and
the one that is not upward is set. */
#if defined(__i386__) || defined(_M_IX86)
#  undef GAOL_RND_PROBE
#endif
#ifndef GAOL_RND_PROBE
#  define GAOL_RND_PROBE 0
#endif

/* GAOL_RND_X86_DIRECT: 1 on a 32-bit x86 processor, where GAOL reads and
writes the control registers of the two units that round, the control word of
the x87 unit (fnstcw and fldcw) and MXCSR (stmxcsr and ldmxcsr), rather than
call fegetround() and fesetround() (GAOL v5; gaol/gaol_fpu_fenv.h has the
instructions, an asm statement with GCC and Clang, clang-cl included, and
inline assembly with Visual C++). Always defined, to 0 or 1. */
#if (defined(__i386__) || defined(_M_IX86)) && GAOL_RND_SSE_REGISTER && GAOL_HAVE_FENV_H \
    && (defined(__GNUC__) || defined(__clang__) || defined(_MSC_VER))
#  define GAOL_RND_X86_DIRECT 1
#else
#  define GAOL_RND_X86_DIRECT 0
#endif

/* GAOL_RND_MINGW64_X87: 1 with mingw-w64 (GCC or Clang) on x86-64, where
get_rounding() and the functions that set the direction back (GAOL_RND_PRESERVE(),
GAOL_RND_RESTORE(), rounding_guard...) read and write the control word of the
x87 unit and MXCSR themselves, as x86_set_modes() does for a 32-bit x86
processor, rather than call fegetround() and fesetround() (GAOL v5, point 110
of TODO.md, found by tests/rounding_direction.cpp in the continuous
integration). From mingw-w64 13 (commit 5c5973cf, "fenv: Use MSVC-compatible
ABI for fenv.h functions", in v13.0.0 and not in v12.0.0) both are
implemented with its own _controlfp() (__mingw_controlfp,
mingw-w64-crt/misc/mingw_controlfp.c): fegetround() gives the two fields RC of
the x87 unit and of MXCSR joined by OR, which is the direction of neither
where the units differ, as they do inside an operation that keeps MXCSR alone
(GAOL_RND_ENTER_SSE()): to nearest and upward give upward (FE_UPWARD), and
downward and upward give FE_UPWARD | FE_DOWNWARD, which is FE_TOWARDZERO
there, a direction that fesetround() then wrote into both units; and
fesetround() clears the six exception flags of MXCSR whenever it changes its
value (__mingw_setfp_sse(), "clear status word if anything changes"), so that
midpoint(), chi(), the writing and the reading of an interval, which set the
direction to nearest and back, lost the flags the program had raised, as did
gaol::cleanup() and gaol::restore_rounding(), through set_rounding(), even
without GAOL_PRESERVE_ROUNDING. Before version 13, fegetround() reads the
control word of the x87 unit alone and fesetround() keeps the flags. Not with
Visual C++ and clang-cl, whose C runtime reads MXCSR alone on x64. Always
defined, to 0 or 1. */
#if defined(__x86_64__) && defined(__MINGW32__) && GAOL_RND_SSE_REGISTER && GAOL_HAVE_FENV_H \
    && (defined(__GNUC__) || defined(__clang__))
#  define GAOL_RND_MINGW64_X87 1
#else
#  define GAOL_RND_MINGW64_X87 0
#endif

/*
The rounding direction of GAOL's operations

GAOL computes its bounds with the rounding direction upward. Its operations
start with GAOL_RND_ENTER(), or GAOL_RND_ENTER_SSE() for the ones computed
with SSE instructions only.

By default (GAOL_PRESERVE_ROUNDING 0), GAOL_RND_ENTER() sets the
rounding direction upward when it is not, and the operations leave it
upward: their bounds are right whatever direction the code using GAOL set,
for the cost of an addition that shows the direction.
GAOL_RND_PRESERVE() and GAOL_RND_RESTORE() frame the computations made in
another direction, after which the direction is set upward again.

On x86 and ARM processors, GAOL_RND_ENTER() also clears the modes that
flush the subnormal numbers to zero (GAOL_RND_FLUSH_BITS above), which make
a subnormal operand or result a zero: with them, [1e-300]*[1e-20] is
[0, 0]. A program linked with -Ofast, -ffast-math or
-funsafe-math-optimizations gets them from crtfastmath.o, whose constructor
runs after GAOL initializes itself with GCC, and so does one that loads a
plug-in built so. The probe of the direction sees them too (see
round_upward_if_needed()), and GAOL leaves them cleared, as it leaves the
direction upward (GAOL v5).

A comparison or a sum with a subnormal operand depends on these modes, and
no compiler models them: GCC 13 at -O3 computed the comparisons of asinpi()
with 0 before the check, on a lower bound read before it, and with the
modes still set asinpi([100 2^-1074]) was [32 2^-1074], above the exact
value. So an operation makes its check before it reads, compares or copies
a bound, the emptiness test aside, whose answer the modes do not change (a
NaN stays one, and flushing keeps two bounds in their order): its
bounds are read from memory after the check, which clears the modes with a
compiler barrier after it (round_upward_if_needed()), and a double it takes
by value goes through rnd_reread() after the check; the kernels of the SSE2
operations, which receive the bounds in a register, pass them through an
empty asm statement after it (reread_bounds(), gaol_interval_sse.cpp). With
GAOL_PRESERVE_ROUNDING, an operation makes its result, the maxima and the
minima of its bounds included, before GAOL_RND_LEAVE() restores the modes,
and keeps it with GAOL_RND_KEEP(): with denormals-are-zero restored, sinpi()
took the minimum of two subnormals as if they were zeros, and
sinpi([100, 1000] 2^-1074) was empty (GAOL v5, review of point 4,
tests/rounding_direction.cpp). The functions that make no check, the
constructor from two bounds, the relations, & and |, max(), abs()...,
compare the bounds as bounds (gaol_detail::bound_less()..., gaol_port.h),
which these modes do not change, and leave them as they found them;
midpoint(), chi() and the writing of an interval make the check (GAOL v5,
point Q of TODO.md, doc/using.md).

Each operation checks the direction once, at its entry. What it calls after
its GAOL_RND_ENTER() takes the direction to be upward and does not check it
again: the functions of namespace upward (gaol/gaol_double_op.h) and the
bodies of the operations of intervals that other operations use, such as
pown_upward() and interval::inverse_upward(), which the operations
checking the direction call too (GAOL v5). A check within another gives the
same bounds, the inner GAOL_RND_LEAVE() setting back the direction upward
that the inner GAOL_RND_ENTER() found, but it costs an addition, and a save
and a restore of the direction with GAOL_PRESERVE_ROUNDING.

With GAOL_PRESERVE_ROUNDING 1, the operations also restore the
direction they found, and the flush-to-zero modes they cleared, which makes
the arithmetic operations slower: 1.3 to 1.7 times with the AVX-512 path,
several times without it (doc/using.md).

The rest of the floating-point environment is the program's, whichever way
GAOL is built: an operation changes neither the exception masks nor the
exception flags the program had, and only adds the flags it raises itself
(doc/using.md). GAOL_RND_ENTER_SSE() and GAOL_RND_LEAVE_SSE() of the SSE2
operations with GAOL_PRESERVE_ROUNDING (+, -, *, /, %, div_rel(), sqr(),
inverse(), the integer powers and the operations with a double) write the
rounding and flush-to-zero bits of MXCSR only, as round_upward() and
set_rounding_and_flush_modes() do: they wrote MXCSR whole with
round_upward_sse(), every exception masked and every flag cleared, so that
an exception the program had enabled was masked again by the first of
these operations, and the flags it had raised were lost (GAOL v5).

GCC does not honour #pragma STDC FENV_ACCESS ON, even with -frounding-math
(https://gcc.gnu.org/bugzilla/show_bug.cgi?id=34678): it may compute a
result after a change of rounding direction that the source code writes
after the computation, when the result is only used after the change, and
then in the wrong direction. Such a result goes through rnd_keep() before
the change, which writes it to volatile memory: the writes to volatile
memory are made where the source code makes them. GAOL_RND_KEEP() does so
before GAOL_RND_LEAVE() and GAOL_RND_RESTORE(), which only change the
direction when it is preserved.
*/
#if GAOL_PRESERVE_ROUNDING
#  define GAOL_RND_ENTER()      const gaol_core::rounding_state _save_state = gaol_core::get_rounding(); gaol_core::round_upward_if_needed()
#  define GAOL_RND_LEAVE()      gaol_core::set_rounding_and_flush_modes(_save_state)
#  define GAOL_RND_PRESERVE()   const gaol_core::rounding_state _save_state = gaol_core::get_rounding()
#  define GAOL_RND_RESTORE()    gaol_core::set_rounding_and_flush_modes(_save_state)
#  define GAOL_RND_KEEP(x)      ((x) = gaol_core::rnd_keep(x))
#  if GAOL_USING_SSE2_INSTRUCTIONS
#     define GAOL_RND_ENTER_SSE() const unsigned int _save_state_sse = _mm_getcsr(); _mm_setcsr((_save_state_sse & ~static_cast<unsigned int>(_MM_ROUND_MASK | GAOL_RND_FLUSH_BITS)) | static_cast<unsigned int>(_MM_ROUND_UP))
#     define GAOL_RND_LEAVE_SSE() _mm_setcsr((_mm_getcsr() & ~static_cast<unsigned int>(_MM_ROUND_MASK | GAOL_RND_FLUSH_BITS)) | (_save_state_sse & static_cast<unsigned int>(_MM_ROUND_MASK | GAOL_RND_FLUSH_BITS)))
#  endif
#else // !GAOL_PRESERVE_ROUNDING
#  define GAOL_RND_ENTER()      gaol_core::round_upward_if_needed()
#  define GAOL_RND_LEAVE()
#  define GAOL_RND_PRESERVE()
#  define GAOL_RND_RESTORE()    gaol_core::round_upward()
#  define GAOL_RND_KEEP(x)
#  if GAOL_USING_SSE2_INSTRUCTIONS
#     define GAOL_RND_ENTER_SSE() gaol_core::round_upward_if_needed()
#     define GAOL_RND_LEAVE_SSE()
#  endif
#endif // GAOL_PRESERVE_ROUNDING

/*
On x86-64 with SSE2 intervals and the fused multiply-add as an instruction
(__FMA__, GAOL_FMA), GAOL_RND_ENTER() and GAOL_RND_LEAVE() are
GAOL_RND_ENTER_SSE() and GAOL_RND_LEAVE_SSE() with GAOL_PRESERVE_ROUNDING
(GAOL v5, point ZZ of TODO.md): every double GAOL computes is an SSE one
there, the elementary functions of CORE-MATH read the direction from MXCSR
(gaol/core_math_port.h), and no code of GAOL uses the x87 unit, a long double
or a function of the C library that the x87 unit would round (the fma() of
mingw-w64 is an instruction with __FMA__, ldexp() is gaol_ldexp(), and the
square root is a bound whatever its rounding): the direction of the x87 unit
changes no bound, and is left as the program had it. The operations then read
and write MXCSR once each at their start and at their end, where the macros
for the other builds read the direction through fegetround() and set it
back through fesetround() as well, which write the control word of the x87
unit and MXCSR again, and clear the flush-to-zero modes and set the direction
upward as two separate writes: pown(x, 3) took 19.0 ns with them, 8.4 with
the others, and 5.1 with the direction left upward (an i7-1185G7, Clang 21;
doc/compare/performance.md). The 32-bit x86 builds, where the x87 unit computes doubles too, the FPU
intervals and a library built without the fused multiply-add keep the macros
above. The functions that restore what GAOL_RND_PRESERVE() found (get_rounding()
and the others) read and write the control word of the x87 unit and MXCSR
themselves with mingw-w64 on x86-64, which does not call fegetround() and
fesetround() any more (GAOL_RND_MINGW64_X87 above).
*/
#if GAOL_PRESERVE_ROUNDING && GAOL_USING_SSE2_INSTRUCTIONS && (defined(__x86_64__) || defined(_M_X64)) && defined(__FMA__)
#  undef GAOL_RND_ENTER
#  undef GAOL_RND_LEAVE
#  define GAOL_RND_ENTER() GAOL_RND_ENTER_SSE()
#  define GAOL_RND_LEAVE() GAOL_RND_LEAVE_SSE()
#endif

/*
With the NEON intervals (FPCR), or the FPU intervals of 32-bit ARM processors
with GAOL_RND_FPSCR_DIRECT (FPSCR), and GAOL_PRESERVE_ROUNDING (GAOL v5), the
operations read the control register once at their start, write it only when it is not upward with the
modes that flush the subnormals to zero clear, and write back the direction
and the modes that they found at their end only if they differ from these (see
gaol_core::round_upward_if_needed()): none of the two msr, 11 ns each (an
Apple M2), when the program computes upward already, where the macros above
read the direction through fegetround() and wrote it back through fesetround()
and the flush modes as two writes.
*/
#if GAOL_PRESERVE_ROUNDING && GAOL_RND_DIRECT
#  undef GAOL_RND_ENTER
#  undef GAOL_RND_LEAVE
#  define GAOL_RND_ENTER() const gaol_core::fp_control _save_fpcr = gaol_core::fpcr_get(); gaol_core::fpcr_enter_upward(_save_fpcr)
#  define GAOL_RND_LEAVE() gaol_core::fpcr_leave(_save_fpcr)
#endif

/*
With GAOL_PRESERVE_ROUNDING on a 32-bit x86 processor (GAOL v5), the operations
read the control word of the x87 unit and MXCSR once at their start, write
the one that is not upward, or that has the modes that flush the subnormals to
zero set, and write back at their end what they found, the register that
differs from it only, if what they found was not upward with these modes clear
(see gaol_core::x86_enter_upward() and gaol_core::x86_leave()): none of the
writes (fldcw, ldmxcsr) when the program computes upward already, where the
macros above read the direction through fegetround(), a call, wrote it back
through fesetround(), which writes both registers, and wrote MXCSR twice more
for the flush modes. The SSE2 operations (GAOL_RND_ENTER_SSE() above), which
use no x87 instruction, keep MXCSR alone and leave the x87 unit as it is.
*/
#if GAOL_PRESERVE_ROUNDING && GAOL_RND_X86_DIRECT
#  undef GAOL_RND_ENTER
#  undef GAOL_RND_LEAVE
#  define GAOL_RND_ENTER() const gaol_core::rounding_state _save_state = gaol_core::get_rounding(); gaol_core::x86_enter_upward(_save_state)
#  define GAOL_RND_LEAVE() gaol_core::x86_leave(_save_state)
#endif

/*
GAOL_RND_NEAREST_ENTER() and GAOL_RND_NEAREST_LEAVE() frame the evaluations
of the mathematical library of an operation, made with the functions of
gaol::nearest, which need the rounding direction to nearest (GAOL v5):
the direction is set twice for both bounds of an interval rather than twice
for each bound, which cost 11 ns more in sin() and cos() on an Intel
i7-1185G7 with glibc. After GAOL_RND_NEAREST_LEAVE(), the direction is
upward, or the one GAOL_RND_NEAREST_ENTER() found with
GAOL_PRESERVE_ROUNDING. Between them, an operation only negates and compares
doubles, which the direction does not change: its computations rounded
upward precede GAOL_RND_NEAREST_ENTER(). A double computed between them,
which the direction changes, goes through rnd_keep() before
GAOL_RND_NEAREST_LEAVE(): GCC computes it after, in the direction set back,
where it is wrong (bug 34678 above), which the TwoSum of the differences of
the bounds of cancel_minus() no longer made exact.
*/
#if GAOL_PRESERVE_ROUNDING
#  define GAOL_RND_NEAREST_ENTER() const gaol_core::rounding_state _save_state_nearest = gaol_core::get_rounding(); gaol_core::round_nearest()
#  define GAOL_RND_NEAREST_LEAVE() gaol_core::set_rounding(_save_state_nearest)
#else
#  define GAOL_RND_NEAREST_ENTER() gaol_core::round_nearest()
#  define GAOL_RND_NEAREST_LEAVE() gaol_core::round_upward()
#endif


#if GAOL_HAVE_FENV_H
#  include "gaol/gaol_fpu_fenv.h"
#elif defined (_MSC_VER)
#  include <fenv.h>
#  include "gaol/gaol_fpu_msvc.h"
#else
#  error "Don't know how to define FPU manipulation functions"
#endif // GAOL_HAVE_FENV_H

namespace gaol_core {

#if GAOL_RND_DIRECT
  /*
    The start and the end of an operation under GAOL_PRESERVE_ROUNDING
    (GAOL_RND_ENTER() and GAOL_RND_LEAVE() of gaol_fpu.h): fpcr is the value
    that mrs gave at the start. The direction is set upward and the modes that
    flush the subnormals to zero are cleared, in one write, if they are not;
    at the end the direction and the modes the program had are written back,
    in one write, if they differ from what the operation left. The comparison
    is of the bits, which the modes do not change, and the statement that
    writes is a barrier for the compiler: what the operation computed goes
    through GAOL_RND_KEEP() before it.
  */
  GAOL_INLINE void fpcr_enter_upward(fp_control fpcr)
  {
    const fp_control mode_bits = fpcr_rmode_mask | static_cast<fp_control>(GAOL_RND_FLUSH_BITS);
    if (GAOL_UNLIKELY((fpcr & mode_bits) != fpcr_upward)) {
      fpcr_set((fpcr & ~mode_bits) | fpcr_upward);
      GAOL_RND_BARRIER();
    }
  }

  GAOL_INLINE void fpcr_leave(fp_control saved)
  {
    const fp_control mode_bits = fpcr_rmode_mask | static_cast<fp_control>(GAOL_RND_FLUSH_BITS);
    if (GAOL_UNLIKELY((saved & mode_bits) != fpcr_upward)) {
      fpcr_update(mode_bits, saved & mode_bits);
    }
  }
#endif // GAOL_RND_DIRECT
  
  /*
    The rounding direction of a 32-bit x86 processor, read and written as the
    hardware has it (GAOL v5). The x87 unit has one in the field RC of its
    control word (bits 11 and 10: 0 to nearest, 1 downward, 2 upward, 3 toward
    zero), the SSE instructions another in the field of MXCSR (bits 14 and 13,
    the same codes), and the bits FTZ (15) and DAZ (6) of MXCSR flush the
    subnormals to zero. These are not the FE_* of <fenv.h>, which are the values
    of the C library (0x0400 and 0x0800 for downward and upward with glibc,
    0x0100 and 0x0200 with the C runtime of Windows, whose fegetround() gives
    -1 when the two units differ): a field read from a register is never
    compared with one of them.

    The functions below do not touch the processor: the registers are those of
    a type with three members, read_control(cw, mxcsr), write_x87_control(cw)
    and write_mxcsr(mxcsr), x86_registers for the processor and a pair of
    variables in tests/rounding_direction.cpp, which checks on every platform
    what they decide to write, and the values they write. The state they save
    and restore is any type with the three members x87, sse and flush (the
    rounding_state of a 32-bit x86 processor): the field RC of the control
    word, that of MXCSR and its bits FTZ and DAZ, each as the register has it.
  */
  const unsigned int x87_rc_mask = 0x0c00u;
  const unsigned int x87_rc_upward = 0x0800u;
  const unsigned int sse_rc_mask = 0x6000u;
  const unsigned int sse_rc_upward = 0x4000u;
  const unsigned int x86_flush_mask = 0x8040u;
  #if GAOL_RND_SSE_REGISTER
  static_assert(sse_rc_mask == static_cast<unsigned int>(_MM_ROUND_MASK)
                && sse_rc_upward == static_cast<unsigned int>(_MM_ROUND_UP)
                && x86_flush_mask == static_cast<unsigned int>(GAOL_RND_FLUSH_BITS),
                "the fields of MXCSR");
  #endif

  //! Whether the x87 unit and the SSE instructions round upward, and no mode flushes the subnormals to zero
  GAOL_INLINE bool x86_is_upward(unsigned int x87_cw, unsigned int mxcsr)
  {
    return (x87_cw & x87_rc_mask) == x87_rc_upward
        && (mxcsr & (sse_rc_mask | x86_flush_mask)) == sse_rc_upward;
  }

  /*
    Sets the field RC of the control word of the x87 unit to x87_rc, and the
    bits of MXCSR that sse_mask selects to sse_bits (a subset of sse_mask),
    and nothing else: a register is written only if one of these bits has to
    change, with the bits that the other fields and the exception flags hold
    just before the write, read here.
  */
  template <class Registers>
  GAOL_RND_NOINLINE void x86_set_modes(Registers& r, unsigned int x87_rc, unsigned int sse_mask, unsigned int sse_bits)
  {
    unsigned int cw, csr;
    r.read_control(cw, csr);
    if ((cw & x87_rc_mask) != x87_rc) {
      r.write_x87_control((cw & ~x87_rc_mask) | x87_rc);
    }
    if ((csr & sse_mask) != sse_bits) {
      r.write_mxcsr((csr & ~sse_mask) | sse_bits);
    }
  }

  /*
    The writes of the start of an operation: the direction is set upward in the
    unit that is not, and the modes that flush the subnormals to zero are
    cleared, in the same write of MXCSR. The barrier keeps what the operation
    reads after the writes (see round_upward_if_needed()).
  */
  template <class Registers>
  GAOL_INLINE void x86_set_upward(Registers& r)
  {
    x86_set_modes(r, x87_rc_upward, sse_rc_mask | x86_flush_mask, sse_rc_upward);
    GAOL_RND_BARRIER();
  }

  //! The check of the start of an operation, from the registers it has read
  template <class Registers>
  GAOL_INLINE void x86_enter_registers(Registers& r, unsigned int x87_cw, unsigned int mxcsr)
  {
    if (GAOL_UNLIKELY(!x86_is_upward(x87_cw, mxcsr))) {
      x86_set_upward(r);
    }
  }

  //! round_upward_if_needed() of a 32-bit x86 processor: one read of the registers, and a write only if one is not upward
  template <class Registers>
  GAOL_INLINE void x86_round_upward_if_needed(Registers& r)
  {
    unsigned int cw, csr;
    r.read_control(cw, csr);
    x86_enter_registers(r, cw, csr);
  }

  //! get_rounding() of a 32-bit x86 processor: one read of the registers
  template <class Registers, class State>
  GAOL_INLINE void x86_get_state(Registers& r, State& s)
  {
    unsigned int cw, csr;
    r.read_control(cw, csr);
    s.x87 = cw & x87_rc_mask;
    s.sse = csr & sse_rc_mask;
    s.flush = csr & x86_flush_mask;
  }

  //! The start of an operation under GAOL_PRESERVE_ROUNDING, from the state it found (see GAOL_RND_ENTER())
  template <class Registers, class State>
  GAOL_INLINE void x86_enter_upward(Registers& r, const State& s)
  {
    x86_enter_registers(r, s.x87, s.sse | s.flush);
  }

  //! set_rounding() of a 32-bit x86 processor: the directions of the state, and no flush mode
  template <class Registers, class State>
  GAOL_INLINE void x86_set_rounding(Registers& r, const State& s)
  {
    x86_set_modes(r, s.x87, sse_rc_mask, s.sse);
  }

  //! set_rounding_and_flush_modes() of a 32-bit x86 processor
  template <class Registers, class State>
  GAOL_INLINE void x86_set_rounding_and_flush_modes(Registers& r, const State& s)
  {
    x86_set_modes(r, s.x87, sse_rc_mask | x86_flush_mask, s.sse | s.flush);
  }

  /*
    The end of an operation under GAOL_PRESERVE_ROUNDING (see GAOL_RND_LEAVE()):
    what it found is written back, in the register that differs from it, unless
    it was upward with the modes that flush the subnormals to zero clear, which
    is what the operation leaves. The comparison is of the bits, which the
    modes do not change, and the statement that writes is no barrier: what the
    operation computed goes through GAOL_RND_KEEP() before it.
  */
  template <class Registers, class State>
  GAOL_INLINE void x86_leave(Registers& r, const State& s)
  {
    if (GAOL_UNLIKELY(!x86_is_upward(s.x87, s.sse | s.flush))) {
      x86_set_rounding_and_flush_modes(r, s);
    }
  }

  #if GAOL_RND_X86_DIRECT || GAOL_RND_MINGW64_X87
  //! The registers of the processor
  struct x86_registers
  {
    void read_control(unsigned int& x87_cw, unsigned int& mxcsr) const { gaol_read_x86_control(x87_cw, mxcsr); }
    void write_x87_control(unsigned int x87_cw) const { gaol_write_x87_control(x87_cw); }
    void write_mxcsr(unsigned int mxcsr) const { _mm_setcsr(mxcsr); }
  };
  #endif

  /*!
  \brief The rounding direction GAOL_RND_ENTER() and GAOL_RND_PRESERVE()
  find, which GAOL_RND_LEAVE() and GAOL_RND_RESTORE() restore, with the
  flush-to-zero modes that round_upward_if_needed() clears
  */
  struct rounding_state
  {
    #if GAOL_RND_X86_DIRECT
    unsigned int x87; // The rounding bits of the control word of the x87 unit, as it holds them
    #else
    int direction; // fegetround(); with GAOL_RND_MINGW64_X87, the field RC of the control word of the x87 unit
    #endif
    #if GAOL_RND_SSE_REGISTER
    unsigned int sse; // The rounding bits of the SSE control register
    #endif
    #if defined(GAOL_RND_FLUSH_BITS)
    unsigned int flush; // The flush-to-zero modes (get_flush_modes())
    #endif
  };
  
  #if defined(GAOL_RND_FLUSH_BITS)
  //! The flush-to-zero modes set, as their bits of GAOL_RND_FLUSH_BITS
  GAOL_INLINE unsigned int get_flush_modes()
  {
    #  if GAOL_RND_SSE_REGISTER
    return _mm_getcsr() & GAOL_RND_FLUSH_BITS;
    #  elif GAOL_RND_FPCR_REGISTER
    unsigned long long fpcr; // mrs and msr take a 64-bit register
    __asm__ __volatile__ ("mrs %0, fpcr" : "=r" (fpcr));
    return static_cast<unsigned int>(fpcr) & GAOL_RND_FLUSH_BITS;
    #  else
    unsigned int fpscr;
    __asm__ __volatile__ ("vmrs %0, fpscr" : "=r" (fpscr));
    return fpscr & GAOL_RND_FLUSH_BITS;
    #  endif
  }
  
  //! Sets the flush-to-zero modes as modes has them, and nothing else
  GAOL_INLINE void set_flush_modes(unsigned int modes)
  {
    #  if GAOL_RND_SSE_REGISTER
    _mm_setcsr((_mm_getcsr() & ~GAOL_RND_FLUSH_BITS) | modes);
    #  elif GAOL_RND_DIRECT
    fpcr_update(GAOL_RND_FLUSH_BITS, modes);
    #  elif GAOL_RND_FPCR_REGISTER
    unsigned long long fpcr;
    __asm__ __volatile__ ("mrs %0, fpcr" : "=r" (fpcr));
    fpcr = (fpcr & ~static_cast<unsigned long long>(GAOL_RND_FLUSH_BITS)) | modes;
    __asm__ __volatile__ ("msr fpcr, %0" : : "r" (fpcr) : "memory");
    #  else
    unsigned int fpscr;
    __asm__ __volatile__ ("vmrs %0, fpscr" : "=r" (fpscr));
    fpscr = (fpscr & ~GAOL_RND_FLUSH_BITS) | modes;
    __asm__ __volatile__ ("vmsr fpscr, %0" : : "r" (fpscr) : "memory");
    #  endif
  }
  
  //! Clears the flush-to-zero modes, and nothing else
  GAOL_INLINE void clear_flush_to_zero()
  {
    set_flush_modes(0u);
  }
  #endif
  
  #if GAOL_RND_X86_DIRECT
  //! The directions of the two units and the flush-to-zero modes: one read of the registers (GAOL v5)
  GAOL_INLINE rounding_state get_rounding()
  {
    rounding_state s;
    x86_registers hw;
    x86_get_state(hw, s);
    return s;
  }

  //! Sets the rounding direction back, in the unit that differs, and nothing else of the environment
  GAOL_INLINE void set_rounding(const rounding_state& s)
  {
    x86_registers hw;
    x86_set_rounding(hw, s);
  }

  //! set_rounding(), and the flush-to-zero modes as well
  GAOL_INLINE void set_rounding_and_flush_modes(const rounding_state& s)
  {
    x86_registers hw;
    x86_set_rounding_and_flush_modes(hw, s);
  }

  //! The start of an operation under GAOL_PRESERVE_ROUNDING (GAOL_RND_ENTER()), from what get_rounding() found
  GAOL_INLINE void x86_enter_upward(const rounding_state& s)
  {
    x86_registers hw;
    x86_enter_upward(hw, s);
  }

  //! The end of an operation under GAOL_PRESERVE_ROUNDING (GAOL_RND_LEAVE())
  GAOL_INLINE void x86_leave(const rounding_state& s)
  {
    x86_registers hw;
    x86_leave(hw, s);
  }
  #else // !GAOL_RND_X86_DIRECT
  GAOL_INLINE rounding_state get_rounding()
  {
    rounding_state s;
    #if GAOL_RND_DIRECT
    // One read of FPCR gives the direction and the flush-to-zero modes; the
    // direction is its RMode field, which is what fegetround() returns
    const fp_control fpcr = fpcr_get();
    s.direction = static_cast<int>(fpcr & fpcr_rmode_mask);
    s.flush = static_cast<unsigned int>(fpcr) & GAOL_RND_FLUSH_BITS;
    return s;
    #endif
    #if GAOL_RND_MINGW64_X87
    {
      // The direction of the x87 unit as its control word holds it (GAOL_RND_MINGW64_X87)
      unsigned int cw, csr;
      gaol_read_x86_control(cw, csr);
      s.direction = static_cast<int>(cw & x87_rc_mask);
      s.sse = csr & static_cast<unsigned int>(_MM_ROUND_MASK);
      s.flush = csr & static_cast<unsigned int>(GAOL_RND_FLUSH_BITS);
      return s;
    }
    #endif
    s.direction = fegetround();
    #if GAOL_RND_SSE_REGISTER
    // One read of MXCSR, which cost 800 ns under Rosetta 2 (see below)
    const unsigned int csr = _mm_getcsr();
    s.sse = csr & _MM_ROUND_MASK;
    s.flush = csr & GAOL_RND_FLUSH_BITS;
    #elif defined(GAOL_RND_FLUSH_BITS)
    s.flush = get_flush_modes();
    #endif
    return s;
  }
  
  //! Sets the rounding direction back, and nothing else of the environment
  GAOL_INLINE void set_rounding(const rounding_state& s)
  {
    #if GAOL_RND_MINGW64_X87
    // In the unit that differs, and the exception flags kept (GAOL_RND_MINGW64_X87)
    x86_registers hw;
    x86_set_modes(hw, static_cast<unsigned int>(s.direction), sse_rc_mask, s.sse);
    return;
    #endif
    #if GAOL_RND_DIRECT
    fpcr_update(fpcr_rmode_mask, static_cast<fp_control>(s.direction));
    #else
    fesetround(s.direction);
    #endif
    #if GAOL_RND_SSE_REGISTER
    _mm_setcsr((_mm_getcsr() & ~static_cast<unsigned int>(_MM_ROUND_MASK)) | s.sse);
    #endif
  }
  
  //! set_rounding(), and the flush-to-zero modes as well
  GAOL_INLINE void set_rounding_and_flush_modes(const rounding_state& s)
  {
    #if GAOL_RND_MINGW64_X87
    // In the unit that differs, and the exception flags kept (GAOL_RND_MINGW64_X87)
    x86_registers hw;
    x86_set_modes(hw, static_cast<unsigned int>(s.direction), sse_rc_mask | x86_flush_mask, s.sse | s.flush);
    return;
    #endif
    #if GAOL_RND_DIRECT
    // One write of FPCR, and none when the direction and the modes are those
    // that it holds (GAOL v5)
    fpcr_update(fpcr_rmode_mask | GAOL_RND_FLUSH_BITS, static_cast<fp_control>(s.direction) | s.flush);
    return;
    #endif
    fesetround(s.direction);
    #if GAOL_RND_SSE_REGISTER
    _mm_setcsr((_mm_getcsr() & ~static_cast<unsigned int>(_MM_ROUND_MASK | GAOL_RND_FLUSH_BITS)) | s.sse | s.flush);
    #elif defined(GAOL_RND_FLUSH_BITS)
    set_flush_modes(s.flush);
    #endif
  }
  #endif // GAOL_RND_X86_DIRECT
  
  /*!
  \brief Sets the rounding direction upward, unless it already is, and on x86
  and ARM processors clears the modes that flush the subnormals to zero
  
  Where doubles are computed in double precision (GAOL_RND_PROBE), the
  direction is shown by 1 + 2^-60, above 1 only when rounded upward, in the
  unit computing GAOL's doubles. 2^-60 is read from volatile memory: no
  compiler can compute the sum at compile time, or reuse the one of another
  call, and no rewriting of 1 + x != 1 is valid with doubles. Reading the
  direction cost more: reading MXCSR about 800 ns under Rosetta 2,
  fegetround() and MXCSR about 80 ns with 32-bit Visual C++, and Clang 18
  reused a single read of MXCSR in a loop that changed the rounding
  direction. A 32-bit x86 processor, where the probe cannot be used
  (GAOL_RND_PROBE), reads the two registers itself, without fegetround()
  (GAOL_RND_X86_DIRECT, below). On x86-64 the x87 unit may keep another
  direction without changing GAOL's bounds: it computes none of GAOL's
  doubles but exact ones and square roots GAOL corrects (the ldexp() of
  mingw-w64 is the fscale of the x87 unit, which GAOL calls for exact results
  only, CORE-MATH computing with gaol_ldexp(), and gaol_sqrt_up() makes a
  bound of the sqrt() of the C library whatever its rounding, GAOL v5), GAOL
  sets the direction of both units before the C library reads or writes a
  number (round_nearest()), and the sources of CORE-MATH, which round some of
  their results in the direction fegetround() gives, read it from MXCSR there
  (gaol/core_math_port.h).
  
  Where GAOL can clear the modes that flush the subnormals to zero
  (GAOL_RND_FLUSH_BITS: x86, and ARM with GCC and Clang), the probe is
  1 + (2^-1060 + 0), which shows them as well as the direction, in one
  comparison: 2^-1060 is a subnormal, and 2^-1060 + 0 is 0 when the
  subnormal operands are read as 0 (DAZ on x86, FZ and FIZ on ARM) and when
  the subnormal results are written as 0 (FTZ on x86, FZ on ARM), and
  1 + 2^-1060 is above 1 only when rounded upward. The sum with +0 is exact,
  and no compiler may drop it without -fno-signed-zeros, -0 + 0 being +0.
  With one of these modes set, every operation with a subnormal operand or
  result gives a wrong bound ([1e-300]*[1e-20] is [0, 0]): a program linked
  with -Ofast, -ffast-math or -funsafe-math-optimizations gets them from
  crtfastmath.o, whose constructor runs after GAOL initializes itself with
  GCC, and so does one that loads a plug-in built so. -mno-daz-ftz, which
  gaol.pc and gaol::gaol give to the link where the compiler has it (GCC 13
  and later on x86, and from 11.4 and 12.4 in the series 11 and 12), only
  keeps crtfastmath.o out of the program: this probe is the one defence
  against the plug-in, and against the other platforms (GAOL v5,
  tests/rounding_direction.cpp, tests/fast_math_link.cpp).
  
  On an Intel Xeon of the Cascade Lake generation (Clang 18), as on the Intel
  i7-1185G7 of the review of 2026-09-27, an addition with a subnormal operand
  and result took as long as one of normal doubles (1.2 ns in a chain of
  dependent additions), and the probe as long as 1 + 2^-60 in front of the
  addition of two SSE2 intervals. Through the library, x * y took 4.0 ns
  rather than 3.4 ns, sqrt 11.0 ns rather than 10.3 ns and pow(x, 3)
  13.7 ns rather than 13.0 ns, and as long with the same probe made of
  2^-60: the cost of the second addition, not of the subnormal; x + y,
  x / y, sqr, exp, log, sin and cos stayed within the noise. Some x86
  processors take a microcode assist, of the order of a hundred cycles, for
  an operation with a subnormal operand or result, which each operation
  would pay there: none of those of the continuous integration did, where
  the probe took 0.35 to 1.43 ns, as long as the same probe made of 2^-60
  within the noise (gaol_performance, 68 jobs on x86-64, i386, arm64 and
  armhf; GAOL v5, point Q of TODO.md).
  */
  GAOL_INLINE void round_upward_if_needed()
  {
    #if GAOL_USING_NEON_INSTRUCTIONS
    /* On 64-bit ARM processors one register, FPCR, holds the direction and the
    modes that flush the subnormals to zero of the scalar and the vector
    instructions alike, and mrs reads it in 0.8 ns, raising no exception flag
    where the probe below raises the inexact one (GAOL v5). It is written when
    it is not upward with the modes clear. asm volatile: no compiler reuses a
    read of it, as Clang 18 did with MXCSR (see above). Not with the FPSCR of
    the 32-bit ARM processors (GAOL_RND_FPSCR_DIRECT): vmrs took longer than
    the two additions of the probe there, x + y 6.1 ns rather than 4.1 and a
    line of 15 additions 87 ns rather than 57 (Debian armhf on the arm64
    runners of GitHub), so that they keep the probe; the read of FPSCR serves
    where the direction is restored, GAOL_PRESERVE_ROUNDING, where it spares
    the C library's reads and writes */
    const fp_control fpcr = fpcr_get();
    if (GAOL_UNLIKELY((fpcr & (fpcr_rmode_mask | GAOL_RND_FLUSH_BITS)) != fpcr_upward)) {
      fpcr_set((fpcr & ~(fpcr_rmode_mask | static_cast<fp_control>(GAOL_RND_FLUSH_BITS))) | fpcr_upward);
      GAOL_RND_BARRIER();
    }
    #elif GAOL_RND_PROBE && defined(GAOL_RND_FLUSH_BITS)
    // 2^-1060, as a literal for the reason of the one of 2^-60 below
    static const volatile double subnormal = 8.0947715414629834e-320;
    // Not above 1, rather than == 1: no operand is a NaN, and the compiler
    // makes one comparison and one branch of it where == takes the three
    // instructions that tell an unordered result (GAOL v5, point ZZ of
    // TODO.md). The branch is the unlikely one, so that the code that sets the
    // direction is out of the way of the one that does not
    if (GAOL_UNLIKELY(!(1.0 + (subnormal + 0.0) > 1.0))) {
      clear_flush_to_zero();
      round_upward();
      GAOL_RND_BARRIER();
    }
    #elif GAOL_RND_PROBE
    // 2^-60, as a literal rather than 1.0/2^60: Visual C++ 2022 computed the
    // quotient when the function first ran with /fp:strict, writing it into
    // tiny, and at compile time otherwise, putting tiny in read-only memory;
    // a program compiled with both, which the linker gives one tiny, crashed
    // writing it. A literal is converted at compile time in every model.
    static const volatile double tiny = 8.67361737988403547205962240695953369140625e-19;
    if (1.0 + tiny == 1.0) {
      round_upward();
    }
    #elif GAOL_RND_X86_DIRECT
    /* On a 32-bit x86 processor the x87 unit and the SSE instructions each
    have their own rounding direction, and both have to be upward: GAOL
    computes its bounds with SSE, and the elementary functions of CORE-MATH
    read the direction with fegetround(), which gives the one of the x87
    unit. Either being elsewhere, it is set upward (GAOL v5, found by
    tests/rounding_direction.cpp in the continuous integration, which leaves
    the two differing on purpose). Not the probe above, which shows the
    direction of the unit computing GAOL's doubles only. The two registers
    are read in one volatile asm statement, the control word of the x87 unit
    with fnstcw and MXCSR with stmxcsr, which raises no exception flag, FTZ
    and DAZ being checked in the same read, and the one that differs is
    written, if the direction is not upward in one of them or a mode is set
    (x86_round_upward_if_needed()). This was a call to fegetround(), about
    80 ns with Visual C++, a read of MXCSR, and round_upward() and
    clear_flush_to_zero(), which wrote both registers whichever differed */
    x86_registers hw;
    x86_round_upward_if_needed(hw);
    #elif (defined(__i386__) || defined(_M_IX86)) && GAOL_RND_SSE_REGISTER
    /* The same, with the functions of the C library, for a compiler whose
    control registers GAOL does not read directly (GAOL_RND_X86_DIRECT is 0):
    either being elsewhere, round_upward() sets both. The control register is read
    already: FTZ and DAZ are checked in the same read (GAOL v5). */
    if (fegetround() != FE_UPWARD
    || (_mm_getcsr() & static_cast<unsigned int>(_MM_ROUND_MASK | GAOL_RND_FLUSH_BITS))
    != static_cast<unsigned int>(_MM_ROUND_UP)) {
      clear_flush_to_zero();
      round_upward();
      GAOL_RND_BARRIER();
    }
    #else
    if (fegetround() != FE_UPWARD) {
      round_upward();
    }
    #endif
  }
  
  /*!
  \brief Sets back, when it is destroyed, the rounding state it found, as
  GAOL_RND_RESTORE() does, by an exception as well (GAOL v5)
  
  A computation that changes the rounding direction and may leave by an
  exception holds one, rather than GAOL_RND_PRESERVE() followed by
  GAOL_RND_RESTORE(): the reading of a number compares it with std::vector
  and std::string, and the construction of a text allocates, and a failed
  allocation left the direction as the computation had set it, to nearest
  for the reader, even without GAOL_PRESERVE_ROUNDING. The destructor sets
  back what GAOL_RND_RESTORE() sets: the state of the program, the
  flush-to-zero modes included, when the direction is preserved, and the
  direction upward otherwise. What the computation keeps goes through
  rnd_keep() as before.
  */
  class rounding_guard
  {
    public:
    rounding_guard()
    #if GAOL_PRESERVE_ROUNDING
    : _saved(get_rounding())
    #endif
    {
    }
    
    ~rounding_guard()
    {
      #if GAOL_PRESERVE_ROUNDING
      set_rounding_and_flush_modes(_saved);
      #else
      round_upward();
      #endif
    }
    
    rounding_guard(const rounding_guard&) = delete;
    rounding_guard& operator=(const rounding_guard&) = delete;
    
    private:
    #if GAOL_PRESERVE_ROUNDING
    rounding_state _saved;
    #endif
  };
  
  /*!
  \brief Returns x, having written it to volatile memory
  
  x is then computed before the change of rounding direction that follows:
  rnd_keep() is the barrier against a compiler that computes x after the
  change, in the direction it sets back (see above, and doc/using.md). GCC
  does so even with -frounding-math, and the terms of the TwoSum of the
  differences of the bounds of cancel_minus(), kept this way, are the ones
  computed to nearest (gaol/gaol_interval_core.cpp).
  */
  template<class T>
  T rnd_keep(const T& x)
  {
    volatile unsigned char bytes[sizeof(T)];
    const unsigned char *from = reinterpret_cast<const unsigned char *>(&x);
    for (std::size_t i = 0; i < sizeof(T); ++i) {
      bytes[i] = from[i];
    }
    T y(x);
    unsigned char *to = reinterpret_cast<unsigned char *>(&y);
    for (std::size_t i = 0; i < sizeof(T); ++i) {
      to[i] = bytes[i];
    }
    return y;
  }
  
  GAOL_INLINE double rnd_keep(double x)
  {
    volatile double kept = x;
    return kept;
  }
  
  GAOL_INLINE float rnd_keep(float x)
  {
    volatile float kept = x;
    return kept;
  }
  
  /*!
  \brief Returns x, a double an operation takes by value, as read after its
  GAOL_RND_ENTER()
  
  The compiler may otherwise compare x before the check, with the modes that
  flush the subnormals to zero still set, x == 0.0 being true for a
  subnormal (see above): the empty asm statement, which keeps its place
  after the check, gives x back in the register it is computed in (SSE on
  x86, the floating-point registers on ARM), at no cost. Not needed where
  the doubles are computed by the x87 unit, which has no such mode, nor with
  Visual C++ (see GAOL_RND_BARRIER()).
  */
  GAOL_INLINE double rnd_reread(double x)
  {
    #if (defined(__GNUC__) || defined(__clang__)) && GAOL_RND_SSE_REGISTER
    #  if defined(__SSE2_MATH__)
    __asm__ __volatile__ ("" : "+x" (x));
    #  endif
    #elif (defined(__GNUC__) || defined(__clang__)) && defined(GAOL_RND_FLUSH_BITS)
    __asm__ __volatile__ ("" : "+w" (x));
    #endif
    return x;
  }
  
} // namespace gaol_core

#endif /* GAOL_FPU_H */
