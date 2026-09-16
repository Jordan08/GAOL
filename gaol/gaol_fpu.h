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
 * CVS: $Id: gaol_fpu.h 191 2012-03-06 17:08:58Z goualard $
 * Last modified: Tue Sep 12 10:26:06 2006 on pc-goualard.lina.sciences.univ-nantes.prive
 * By: Frederic Goualard <Frederic.Goualard@lina.univ-nantes.fr>
 *--------------------------------------------------------------------------*/

/*!
  \file   gaol_fpu.h
  \brief

  <long description>

  \author Frederic Goualard
  \date   2001-10-01
*/


#ifndef __gaol_fpu_h__
#define __gaol_fpu_h__

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

// Doubles computed in double precision, whose rounding direction an addition
// shows (see round_upward_if_needed())
#if (defined(FLT_EVAL_METHOD) && FLT_EVAL_METHOD == 0) || defined(_M_X64) || defined(_M_ARM64) \
    || defined(_M_ARM) || (defined(_M_IX86_FP) && _M_IX86_FP >= 2)
#  define GAOL_RND_PROBE 1
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

  With GAOL_PRESERVE_ROUNDING defined, the operations also restore the
  direction they found, which makes the arithmetic operations several times
  slower.

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
#  define GAOL_RND_ENTER()      const gaol::rounding_state _save_state = gaol::get_rounding(); gaol::round_upward_if_needed()
#  define GAOL_RND_LEAVE()      gaol::set_rounding(_save_state)
#  define GAOL_RND_PRESERVE()   const gaol::rounding_state _save_state = gaol::get_rounding()
#  define GAOL_RND_RESTORE()    gaol::set_rounding(_save_state)
#  define GAOL_RND_KEEP(x)      ((x) = gaol::rnd_keep(x))
#  if USING_SSE2_INSTRUCTIONS
#     define GAOL_RND_ENTER_SSE() const unsigned int _save_state_sse = _mm_getcsr() & _MM_ROUND_MASK; gaol::round_upward_sse()
#     define GAOL_RND_LEAVE_SSE() _mm_setcsr((_mm_getcsr() & ~(unsigned int)_MM_ROUND_MASK) | _save_state_sse)
#  endif
#else // !GAOL_PRESERVE_ROUNDING
#  define GAOL_RND_ENTER()      gaol::round_upward_if_needed()
#  define GAOL_RND_LEAVE()
#  define GAOL_RND_PRESERVE()
#  define GAOL_RND_RESTORE()    gaol::round_upward()
#  define GAOL_RND_KEEP(x)
#  if USING_SSE2_INSTRUCTIONS
#     define GAOL_RND_ENTER_SSE() gaol::round_upward_if_needed()
#     define GAOL_RND_LEAVE_SSE()
#  endif
#endif // GAOL_PRESERVE_ROUNDING

/*
  GAOL_RND_NEAREST_ENTER() and GAOL_RND_NEAREST_LEAVE() frame the evaluations
  of the mathematical library of an operation, made with the functions of
  gaol::nearest, which need the rounding direction to nearest (fork of GAOL):
  the direction is set twice for both bounds of an interval rather than twice
  for each bound, which cost 11 ns more in sin() and cos() on an Intel
  i7-1185G7 with glibc. After GAOL_RND_NEAREST_LEAVE(), the direction is
  upward, or the one GAOL_RND_NEAREST_ENTER() found with
  GAOL_PRESERVE_ROUNDING. Between them, an operation only negates and compares
  doubles, which the direction does not change: its computations rounded
  upward precede GAOL_RND_NEAREST_ENTER().
*/
#if GAOL_PRESERVE_ROUNDING
#  define GAOL_RND_NEAREST_ENTER() const gaol::rounding_state _save_state_nearest = gaol::get_rounding(); gaol::round_nearest()
#  define GAOL_RND_NEAREST_LEAVE() gaol::set_rounding(_save_state_nearest)
#else
#  define GAOL_RND_NEAREST_ENTER() gaol::round_nearest()
#  define GAOL_RND_NEAREST_LEAVE() gaol::round_upward()
#endif


#if HAVE_FENV_H
#  include "gaol/gaol_fpu_fenv.h"
#elif defined (_MSC_VER)
#  include <fenv.h>
#  include "gaol/gaol_fpu_msvc.h"
#else
#  error "Don't know how to define FPU manipulation functions"
#endif // HAVE_FENV_H

namespace gaol {

  /*!
    \brief The rounding direction GAOL_RND_ENTER() and GAOL_RND_PRESERVE()
    find, which GAOL_RND_LEAVE() and GAOL_RND_RESTORE() restore
  */
  struct rounding_state
  {
    int direction; // fegetround()
#if GAOL_RND_SSE_REGISTER
    unsigned int sse; // The rounding bits of the SSE control register
#endif
  };

  INLINE rounding_state get_rounding()
  {
    rounding_state s;
    s.direction = fegetround();
#if GAOL_RND_SSE_REGISTER
    s.sse = _mm_getcsr() & _MM_ROUND_MASK;
#endif
    return s;
  }

  INLINE void set_rounding(const rounding_state& s)
  {
    fesetround(s.direction);
#if GAOL_RND_SSE_REGISTER
    _mm_setcsr((_mm_getcsr() & ~(unsigned int)_MM_ROUND_MASK) | s.sse);
#endif
  }

  /*!
    \brief Sets the rounding direction upward, unless it already is

    Where doubles are computed in double precision (GAOL_RND_PROBE), the
    direction is shown by 1 + 2^-60, above 1 only when rounded upward, in the
    unit computing GAOL's doubles. 2^-60 is read from volatile memory: no
    compiler can compute the sum at compile time, or reuse the one of another
    call, and no rewriting of 1 + x != 1 is valid with doubles. Reading the
    direction cost more: reading MXCSR about 800 ns under Rosetta 2,
    fegetround() and MXCSR about 80 ns with 32-bit Visual C++, and Clang 18
    reused a single read of MXCSR in a loop that changed the rounding
    direction. The C library computes in the direction GAOL sets before
    calling it (round_nearest()), whatever the x87 unit had.
  */
  INLINE void round_upward_if_needed()
  {
#if GAOL_RND_PROBE
    // 2^-60, as a literal rather than 1.0/2^60: Visual C++ 2022 computed the
    // quotient when the function first ran with /fp:strict, writing it into
    // tiny, and at compile time otherwise, putting tiny in read-only memory;
    // a program compiled with both, which the linker gives one tiny, crashed
    // writing it. A literal is converted at compile time in every model.
    static const volatile double tiny = 8.67361737988403547205962240695953369140625e-19;
    if (1.0 + tiny == 1.0) {
      round_upward();
    }
#else
    if (fegetround() != FE_UPWARD) {
      round_upward();
    }
#endif
  }

  /*!
    \brief Returns x, having written it to volatile memory

    x is then computed before the change of rounding direction that follows
    (see above).
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

  INLINE double rnd_keep(double x)
  {
    volatile double kept = x;
    return kept;
  }

  INLINE float rnd_keep(float x)
  {
    volatile float kept = x;
    return kept;
  }

} // namespace gaol

#endif /* __gaol_fpu_h__ */
