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
 *--------------------------------------------------------------------------
 * gaol is a software distributed WITHOUT ANY WARRANTY. Read the associated
 * COPYING file for information.
 *--------------------------------------------------------------------------
 * By: Frederic Goualard <Frederic.Goualard@lina.univ-nantes.fr>
 *--------------------------------------------------------------------------*/

/*!
  \file   gaol_fpu.h
  \brief

  <long description>

  \author Frederic Goualard
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
     there. */
#if GAOL_RND_SSE_REGISTER
#  define GAOL_RND_FLUSH_BITS 0x8040u
#elif defined(__aarch64__) && defined(__GNUC__)
#  define GAOL_RND_FPCR_REGISTER 1
#  define GAOL_RND_FLUSH_BITS 0x1000001u
#elif defined(__arm__) && defined(__ARM_FP) && defined(__GNUC__)
#  define GAOL_RND_FPSCR_REGISTER 1
#  define GAOL_RND_FLUSH_BITS 0x1000000u
#endif

/* A compiler barrier: the memory read after it is read after it, and no
   comparison of a bound read after it is made before it (see
   round_upward_if_needed()). None for Visual C++, which keeps the
   floating-point operations where the source puts them with respect to the
   writes of the control register under /fp:strict, with which GAOL is
   compiled (tests/rounding_direction.cpp checks it in the continuous
   integration). */
#if defined(__GNUC__) || defined(__clang__)
#  define GAOL_RND_BARRIER() __asm__ __volatile__ ("" : : : "memory")
#else
#  define GAOL_RND_BARRIER() ((void)0)
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
   direction is read with fegetround() and set with round_upward(), which set
   both units. */
#if defined(__i386__) || defined(_M_IX86)
#  undef GAOL_RND_PROBE
#endif

/*
  The rounding direction of GAOL's operations

  GAOL computes its bounds with the rounding direction upward. Its operations
  start with GAOL_RND_ENTER(), or GAOL_RND_ENTER_SSE() for the ones computed
  with SSE instructions only.

  By default (GAOL_PRESERVE_ROUNDING undefined), GAOL_RND_ENTER() sets the
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
  by value goes through rnd_reread() after the check. With
  GAOL_PRESERVE_ROUNDING, an operation makes its result, the maxima and the
  minima of its bounds included, before GAOL_RND_LEAVE() restores the modes,
  and keeps it with GAOL_RND_KEEP(): with denormals-are-zero restored, sinpi()
  took the minimum of two subnormals as if they were zeros, and
  sinpi([100, 1000] 2^-1074) was empty (GAOL v5, review of point 4,
  tests/rounding_direction.cpp). The operations that make no check, which
  compare bounds without clearing the modes, are listed in doc/using.md.

  Each operation checks the direction once, at its entry. What it calls after
  its GAOL_RND_ENTER() takes the direction to be upward and does not check it
  again: the functions of namespace upward (gaol/gaol_double_op.h) and the
  bodies of the operations of intervals that other operations use, such as
  uipow_rounded_upward() and interval::inverse_upward(), which the operations
  checking the direction call too (GAOL v5). A check within another gives the
  same bounds, the inner GAOL_RND_LEAVE() setting back the direction upward
  that the inner GAOL_RND_ENTER() found, but it costs an addition, and a save
  and a restore of the direction with GAOL_PRESERVE_ROUNDING.

  With GAOL_PRESERVE_ROUNDING defined, the operations also restore the
  direction they found, and the flush-to-zero modes they cleared, which makes
  the arithmetic operations several times slower.

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
#     define GAOL_RND_ENTER_SSE() const unsigned int _save_state_sse = _mm_getcsr(); _mm_setcsr((_save_state_sse & ~(unsigned int)(_MM_ROUND_MASK | GAOL_RND_FLUSH_BITS)) | (unsigned int)_MM_ROUND_UP)
#     define GAOL_RND_LEAVE_SSE() _mm_setcsr((_mm_getcsr() & ~(unsigned int)(_MM_ROUND_MASK | GAOL_RND_FLUSH_BITS)) | (_save_state_sse & (unsigned int)(_MM_ROUND_MASK | GAOL_RND_FLUSH_BITS)))
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

  /*!
    \brief The rounding direction GAOL_RND_ENTER() and GAOL_RND_PRESERVE()
    find, which GAOL_RND_LEAVE() and GAOL_RND_RESTORE() restore, with the
    flush-to-zero modes that round_upward_if_needed() clears
  */
  struct rounding_state
  {
    int direction; // fegetround()
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
    return (unsigned int)fpcr & GAOL_RND_FLUSH_BITS;
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
#  elif GAOL_RND_FPCR_REGISTER
    unsigned long long fpcr;
    __asm__ __volatile__ ("mrs %0, fpcr" : "=r" (fpcr));
    fpcr = (fpcr & ~(unsigned long long)GAOL_RND_FLUSH_BITS) | modes;
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

  GAOL_INLINE rounding_state get_rounding()
  {
    rounding_state s;
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
    fesetround(s.direction);
#if GAOL_RND_SSE_REGISTER
    _mm_setcsr((_mm_getcsr() & ~(unsigned int)_MM_ROUND_MASK) | s.sse);
#endif
  }

  //! set_rounding(), and the flush-to-zero modes as well
  GAOL_INLINE void set_rounding_and_flush_modes(const rounding_state& s)
  {
    fesetround(s.direction);
#if GAOL_RND_SSE_REGISTER
    _mm_setcsr((_mm_getcsr() & ~(unsigned int)(_MM_ROUND_MASK | GAOL_RND_FLUSH_BITS)) | s.sse | s.flush);
#elif defined(GAOL_RND_FLUSH_BITS)
    set_flush_modes(s.flush);
#endif
  }

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
    direction. On x86-64 the x87 unit may keep another direction without
    changing GAOL's bounds: it computes none of GAOL's doubles, GAOL sets the
    direction of both units before the C library reads or writes a number
    (round_nearest()), and the sources of CORE-MATH, which round some of their
    results in the direction fegetround() gives, read it from MXCSR there
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
    would pay there: none was measured (GAOL v5).
  */
  GAOL_INLINE void round_upward_if_needed()
  {
#if GAOL_RND_PROBE && defined(GAOL_RND_FLUSH_BITS)
    // 2^-1060, as a literal for the reason of the one of 2^-60 below
    static const volatile double subnormal = 8.0947715414629834e-320;
    if (1.0 + (subnormal + 0.0) == 1.0) {
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
#elif (defined(__i386__) || defined(_M_IX86)) && GAOL_RND_SSE_REGISTER
    /* On a 32-bit x86 processor the x87 unit and the SSE instructions each
       have their own rounding direction, and both have to be upward: GAOL
       computes its bounds with SSE, and the elementary functions of CORE-MATH
       read the direction with fegetround(), which gives the one of the x87
       unit. Either being elsewhere, round_upward() sets both (GAOL v5,
       found by tests/rounding_direction.cpp in the continuous integration,
       which leaves the two differing on purpose). The control register is read
       already: FTZ and DAZ are checked in the same read (GAOL v5). */
    if (fegetround() != FE_UPWARD
        || (_mm_getcsr() & (unsigned int)(_MM_ROUND_MASK | GAOL_RND_FLUSH_BITS)) != (unsigned int)_MM_ROUND_UP) {
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
    computed to nearest (gaol/gaol_interval.cpp).
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
