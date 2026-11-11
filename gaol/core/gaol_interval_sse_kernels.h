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
  \file   gaol_interval_sse_kernels.h
  \brief  The kernels of the SSE2 intervals that several sources take (GAOL v5)

  A header of the sources, not installed, that gaol_interval_internal.h
  includes for the SSE2 intervals: the bodies of the kernels of
  gaol_interval_sse_impl.h that the sources other than gaol_interval_core.cpp
  call, and what they take. They are inline here, so that the integer powers
  and the roots of gaol_interval_pow.cpp take them inline, as they did when
  every definition was in gaol_interval_core.cpp, and the functions of
  gaol_interval_sse_impl.h are calls of them. upward_access::inverse() and
  upward_access::sqr() are the inverse and the square with the rounding
  direction upward already (inverse_upward() and sqr_upward(), private
  functions of interval before), and sqr_bounds_inline() the SSE2 path of
  gaol_detail::sqr_bounds(), the square with its check (see the kernels in
  gaol_interval_internal.h).

  \author Goualard Frederic, NININ Jordan
  \date   2001-09-28
*/

#ifndef GAOL_INTERVAL_SSE_KERNELS_H
#define GAOL_INTERVAL_SSE_KERNELS_H

namespace gaol_core {

  /*
    The two stored bounds x of an operand, read again after the check of the
    rounding direction: the empty asm statement keeps its place after the
    check, and gives x back in the register it is in, at no cost, so that no
    compiler computes with it, or compares it, before the check has set the
    direction and cleared the modes that flush the subnormals to zero. The
    kernels take their operands in registers, where the operators read them
    from memory behind the barrier of the check (see rnd_reread() of
    gaol/gaol_fpu.h, for a double; GAOL v5, point ZZ of TODO.md). Visual C++
    has no asm statement on x64, and none is needed: it keeps the
    floating-point operations where the source puts them with respect to the
    writes of the control register under /fp:strict, with which GAOL is
    compiled (see GAOL_RND_BARRIER() of gaol/gaol_fpu.h)
  */
  GAOL_INLINE __m128d reread_bounds(__m128d x)
  {
#if defined(__GNUC__) || defined(__clang__)
    __asm__ __volatile__ ("" : "+x" (x));
#endif
    return x;
  }

  /*
    The lanes of the result that the interval straddling zero sets to 0 (see
    the definition, in gaol_interval_sse_impl.h): defined there once, and not
    static here, so that sqr_products(), inline in every source, reads the
    same array in all of them (the one-definition rule; GAOL v5)
  */
  alignas(16) extern const std::uint64_t sqr_lanes[4][2];

#if !GAOL_AVX512_ARITHMETIC
  /*
    The two products and the maximum of sqr_bounds_inline() below, the SSE2 path, on
    the stored bounds x of a non-empty interval, the rounding direction being
    upward and the modes that flush the subnormals to zero clear: the body of
    sqr_bounds_inline() after its check, which upward_access::sqr() takes for
    x^2 (GAOL v5)
  */
  GAOL_INLINE __m128d sqr_products(__m128d x)
  {
    const __m128d s1 = _mm_mul_pd(x, _mm_xor_pd(x, interval::lbsignmask));
    const __m128d xs = _mm_shuffle_pd(x, x, 1);
    const __m128d s2 = _mm_mul_pd(xs, _mm_xor_pd(xs, interval::lbsignmask));
    return _mm_and_pd(_mm_max_pd(s1, s2),
                      _mm_load_pd(reinterpret_cast<const double *>(sqr_lanes[_mm_movemask_pd(x)])));
  }
#endif

#if !GAOL_AVX512_ARITHMETIC
  /*
    The square of the stored bounds x of an interval, the empty set or not,
    with its check of the rounding direction: the SSE2 path of
    gaol_detail::sqr_bounds() (gaol_interval_sse_impl.h), which calls it, and
    the square that x^2 of gaol_pown() takes (upward_access::checked_sqr())
    (GAOL v5)
  */
  GAOL_INLINE __m128d sqr_bounds_inline(__m128d x)
  {
    if (_mm_movemask_pd(_mm_cmpunord_pd(x, x)) != 0) { // the empty set
      return x;
    }
    GAOL_RND_ENTER_SSE();
    x = reread_bounds(x);
    __m128d res = sqr_products(x);
    GAOL_RND_KEEP(res);
    GAOL_RND_LEAVE_SSE();
    return res;
  }
#endif

  /*
    The rounding direction being upward already (see gaol_interval_core.h). A zero
    bound is divided by 1 rather than 0, its quotient being thrown away: -1/0
    raised the division-by-zero exception, inverse([0, 2]) and pow([0, 2], -1)
    among them, where the FPU intervals raise nothing (GAOL v5)
  */
  inline interval upward_access::inverse(const interval& I)
  {
    const __m128d xmmbounds = I.xmmbounds;
    __m128d r1 = _mm_xor_pd(xmmbounds,interval::lbsignmask); // r1 == < d, c >
    __m128d r2 = _mm_shuffle_pd(r1,r1,1); // r2 == <c, d>
    __m128d r4 = _mm_cmpeq_pd(r2,interval::m128_zero); // r4 == < c==0, d==0 >
    // r3 == < -1/c, -1/d >, -1 or 1 where c or d is +-0
    __m128d r3 = _mm_div_pd(interval::m128_minus_one,_mm_or_pd(r2,_mm_and_pd(r4,_mm_set1_pd(1.0))));
    __m128d r5 = _mm_shuffle_pd(r4,r4,1); // r5 == < d==0, c==0 >
    __m128d r6 = _mm_and_pd(r4,r5); // r6 == < c==0 && d==0, c==0 && d==0 >
    __m128d r7 = _mm_and_pd(r6,interval::m128_nan); // < r7 == < (c==0 && d==0) ? NaN : 0, (c==0 && d==0) ? NaN : 0 >
    __m128d r8 = _mm_and_pd(r4,interval::m128_infinf); // r8 == < (c==0) ? inf : 0, (d==0) ? inf : 0 >
    __m128d r9 = _mm_andnot_pd(r4,r3); // r9 == < (c!=0) ? -1/c : 0, (d!=0) ? -1/d : 0 >
    __m128d r10 = _mm_or_pd(r9,r8); // r10 == < (c==0) ? inf : -1/c, (d==0) ? inf : -1/d >
    __m128d r11 = _mm_andnot_pd(r6,r10); // r11 == < (c!=0 || d!=0) ? hi(r10) : 0, (c!=0 || d!=0) ? lo(r10) : 0 >
    __m128d r12 = _mm_or_pd(r11,r7); // r12 == < (c==0 && d==0) ? NaN : hi(r10), (c==0 && d==0) ? NaN : lo(r10) >
    __m128d r13 = _mm_cmpgt_pd(xmmbounds,interval::m128_zero); // r13 == < d>0, (-c)>0 >
    __m128d r14 = _mm_shuffle_pd(r13,r13,1); // r14 == < (-c)>0, d>0 >
    __m128d r15 = _mm_and_pd(r13,r14); // r15 == < d>0 && (-c)>0, (-c)>0 && d>0 >
    __m128d r16 = _mm_and_pd(r15, interval::m128_infinf); // r14 == < ((-c)>0 && d>0) ? inf : 0, ((-c)>0 && d>0) ? inf : 0 >
    __m128d r17 = _mm_andnot_pd(r15, r12); // r17 ==  < ((-c)<=0 || d<=0) ? hi(r12) : 0, ((-c)<=0 || d<=0) ? lo(r12) : 0 >
    __m128d r18 = _mm_or_pd(r16,r17); // < ((-c)>0 && d>0) ? inf : hi(r12), ((-c)>0 && d>0) ? inf : lo(r12) >
    return interval(r18, gaol_detail::unchecked_bounds);
  }

  /*
    sqr() of a non-empty I, the rounding direction being upward already (see
    gaol_interval_core.h): the square that pow() takes for x^2 after its own
    check (GAOL v5). The AVX-512 path sets no direction, so that its kernel
    is the function itself, whose guard of the modes that flush the
    subnormals to zero finds them clear.
  */
  inline interval upward_access::sqr(const interval& I)
  {
#if GAOL_AVX512_ARITHMETIC
    return interval(gaol_detail::sqr_bounds(I.xmmbounds), gaol_detail::unchecked_bounds);
#else
    return interval(sqr_products(I.xmmbounds), gaol_detail::unchecked_bounds);
#endif
  }

  /*
    sqr() of I, the empty set or not, with its check: x^2 of gaol_pown()
    (GAOL v5). The AVX-512 path is gaol_detail::sqr_bounds() itself, which
    has its target (gaol_interval_avx512_impl.h) and is not taken inline into
    a function without it, as when the two were in gaol_interval_core.cpp
  */
  inline interval upward_access::checked_sqr(const interval& I)
  {
#if GAOL_AVX512_ARITHMETIC
    return gaol_core::sqr(I);
#else
    return interval(sqr_bounds_inline(I.xmmbounds), gaol_detail::unchecked_bounds);
#endif
  }

} // namespace gaol_core

#endif /* GAOL_INTERVAL_SSE_KERNELS_H */
