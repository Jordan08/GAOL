/*-*-C++-*------------------------------------------------------------------
 * GAOL -- not Just Another Interval Library
 *--------------------------------------------------------------------------
 * Copyright (c) 2001 Swiss Federal Institute of Technology, Switzerland
 * Copyright (c) 2002-2009 Laboratoire d'Informatique de
 *                         Nantes-Atlantique, France
 * Copyright (c) 2026 ENSTA, France
 *
 * By: Frederic Goualard, Jordan NININ
 *--------------------------------------------------------------------------
 * GAOL is a software distributed WITHOUT ANY WARRANTY. Read the associated
 * COPYING.LIB file for information.
 *--------------------------------------------------------------------------*/

/*!
  \file   gaol_interval_fpu_kernels.h
  \brief  The FPU kernels that several sources take inline: the inverse and the square, before
          and with the check of the rounding direction. Included by gaol_interval_internal.h.
*/

#ifndef GAOL_INTERVAL_FPU_KERNELS_H
#define GAOL_INTERVAL_FPU_KERNELS_H

/*
  A header of the sources, not installed, that gaol_interval_internal.h
  includes for the FPU intervals: the bodies of the kernels of
  gaol_interval_fpu_impl.h that the sources other than gaol_interval_core.cpp
  call, and the helpers they take. They are inline here, so that the integer
  powers and the roots of gaol_interval_pow.cpp take them inline, as they did
  when every definition was in gaol_interval_core.cpp, and the functions of
  gaol_interval_fpu_impl.h are calls of them. upward_access::inverse() and
  upward_access::sqr() are the inverse and the square with the rounding
  direction upward already (inverse_upward() and sqr_upward(), private
  functions of interval before), and upward_access::checked_sqr() and
  upward_access::checked_inverse() the bodies of sqr() and inverse(), with
  their check (see the kernels in gaol_interval_internal.h).
*/

namespace gaol_core {

  namespace fpu_detail {

  /*
    The larger of two doubles that are no NaN (GAOL v5): a comparison and a selection,
    the `maxsd` of x86 (`maxpd` of the SSE2 intervals), which returns the
    second operand when they compare equal, +0 and -0 included. What the
    compiler returns there is its own, and compilers differ: GCC 13 for POWER9
    makes `(a > b) ? a : b` an `xsmaxcdp`, whose result for a +0 and a -0 is
    the first operand (as QEMU computes it, in the continuous integration; the
    comment of GCC lets the instruction stand for `a >= b ? a : b` as well).
    The value of a bound is the same for all of them, but where two products are
    zeros of opposite signs, +0 and -0, the sign of the zero bound of a product
    is the compiler's and the target's: the lower bound of [-1, 0] * [-1, 0] is
    +0 with `maxsd` and -0 with `xsmaxcdp`. It is not specified for the FPU
    intervals (doc/using.md, The FPU intervals), and no test reads it. The
    maximum that ranks +0 above -0 (fmax(), the `fmax` of 64-bit ARM) is one
    instruction fewer, and is not one with Visual C++ or GCC for 32-bit ARM.
    The selection is a conditional move with GCC (13 to 16) for 32-bit ARM,
    arm64 and x86, with Clang, and with Visual C++ for x86 and arm64, an
    `xsmaxcdp` with GCC for POWER9 (a branch with GCC for POWER8, s390x and
    RISC-V, which have no conditional move of doubles)
  */
  GAOL_INLINE double larger_of(double a, double b)
  {
    return (a > b) ? a : b;
  }

  /*
    The bounds of the square of <a, b> (see upward_access::sqr()), forced
    inline into upward_access::sqr() and upward_access::checked_sqr(), the
    bodies of the square after the check and with it (GAOL v5): with the
    square after the check a function that sqr() called, the square took
    3.37 ns rather than 2.41 (GCC 15.2, x86-64, FPU intervals)
  */
#if defined(__GNUC__) || defined(__clang__)
#  define GAOL_FPU_ALWAYS_INLINE __attribute__((always_inline)) inline
#elif defined(_MSC_VER)
#  define GAOL_FPU_ALWAYS_INLINE __forceinline
#else
#  define GAOL_FPU_ALWAYS_INLINE inline
#endif
  GAOL_FPU_ALWAYS_INLINE void sqr_bounds(double a, double b, double& lower, double& upper)
  {
    double lo = larger_of(a*(-a), b*(-b));
    double hi = larger_of(a*a, b*b);
    // l < 0 < r: both stored bounds are above 0. & rather than &&, which is a
    // branch on the sign of a: the selection is a conditional move with Clang
    // and Visual C++ for arm64 and with GCC for arm64, POWER and 32-bit ARM
    // (-O2 and -O3: two branches at -Os with GCC 14 and 15), but a branch with
    // Visual C++ for x86
    lo = ((a > 0.0) & (b > 0.0)) ? -0.0 : lo;
    lower = lo;
    upper = hi;
  }

  } // namespace fpu_detail

  /*
    The inverse of a non-empty I (inverse_upward() before), the rounding
    direction being upward already (see gaol_interval_core.h). The bound
    that is 0 in the other branch, I.rb_ for N1 and I.lb_ for P1, is
    read in its branch (read_in_branch(), gaol/core/gaol_interval_internal.h):
    AppleClang computed -1/I.rb_ for N0 too, inverse() choosing between the
    branches afterwards, and inverse([-2, 0]) raised the division-by-zero
    exception (macOS arm64, GAOL_PRESERVE_ROUNDING; GAOL v5). Not the
    quotient of [1] by I of fpu_detail::quotient(), whose selections took more
    time than the branches of these five cases, which the operands of an
    inverse follow (3.2 ns rather than 2.1 for operands of one sign, 6.2
    rather than 5.8 for operands of random signs, Apple M2)
  */
  inline interval upward_access::inverse(const interval& I)
  {
    interval res;
    if (I.right() <= 0.0) { // (Z N0 N1)
      if (I.left() < 0) { // (N0 N1)
        if (I.right() < 0) { // N1
          res.lb_ = (-1.0)/read_in_branch(I.rb_);
          res.rb_ = (-1.0)/I.lb_;
        } else { // N0
          res.lb_ = GAOL_INFINITY;
          res.rb_ = (-1.0)/I.lb_;
        }
      } else {  // Z
        res = interval::emptyset();
      }
    } else { // (M P0 P1)
      if (I.left() < 0) { // M
        res = interval::universe();
      } else { // (P0 P1)
        if (I.left() == 0.0) { // P0
          res.lb_ = (-1.0)/I.rb_;
          res.rb_ = GAOL_INFINITY;
        } else { // P1
          res.lb_ = (-1.0)/I.rb_;
          res.rb_ = (-1.0)/read_in_branch(I.lb_);
        }
      }
    }
    return res;
  }

  /*
    The square from two products and a maximum, no branch but the one of the
    empty set (GAOL v5), as sqr_bounds() of the SSE2 and the NEON intervals:
    with I = <a, b> = <-l, r>, the stored lower bound is the larger of a * (-a)
    and b * (-b), which are -l^2 and -r^2 rounded upward, the opposite of the
    smaller of the squares, and the upper one the larger of a * a and b * b,
    whatever the signs: <-l^2, r^2> for l >= 0, <-r^2, l^2> for r <= 0. For an
    interval that strictly straddles zero the lower bound is 0, which a
    selection sets, stored -0 so that left() is +0 (the former code set
    l = 0.0 too, and GAOL gives [0, 4] for [-1, 2], where the SSE2 and the
    NEON intervals give [-0, 4]). The bounds are those of the former code, bit
    for bit: every product of a bound by itself is the same one. The check of
    the rounding direction covers the three cases, which all compute. A zero
    or an infinite bound needs no case of its own: no product of a zero by an
    infinity is formed.
    The rounding direction being upward already (see gaol_interval_core.h), of
    a non-empty I: the body of sqr() after its check, which pow() takes for
    x^2 after its own (GAOL v5). The bounds are read here, after the check
    of the caller, as sqr() reads them.
  */
  inline interval upward_access::sqr(const interval& I)
  {
    const double a = rnd_reread(I.lb_), b = rnd_reread(I.rb_);
    interval res;
    fpu_detail::sqr_bounds(a, b, res.lb_, res.rb_);
    return res;
  }

  // sqr() of I, the empty set or not, with its check: its body, and x^2 of
  // gaol_pown() (GAOL v5)
  inline interval upward_access::checked_sqr(const interval& I)
  {
    if (I.is_empty()) { // the empty set
      return interval::emptyset();
    }

    GAOL_RND_ENTER();
    const double a = rnd_reread(I.lb_), b = rnd_reread(I.rb_);
    interval res;
    fpu_detail::sqr_bounds(a, b, res.lb_, res.rb_);
    GAOL_RND_KEEP(res);
    GAOL_RND_LEAVE();
    return res;
  }

  // inverse() of I, the empty set or not, with its check: its body (GAOL v5)
  inline interval upward_access::checked_inverse(const interval& I)
  {
    if (I.is_empty()) {
      return interval::emptyset();
    }

    GAOL_RND_ENTER();
    interval res = upward_access::inverse(I);
    GAOL_RND_KEEP(res);
    GAOL_RND_LEAVE();
    return res;
  }

} // namespace gaol_core

#endif /* GAOL_INTERVAL_FPU_KERNELS_H */
