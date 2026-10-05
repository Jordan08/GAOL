/*-*-C++-*------------------------------------------------------------------
 * gaol -- NOT Just Another Interval Library
 *--------------------------------------------------------------------------
 * This file is part of the gaol distribution. Gaol was primarily
 * developed at the Swiss Federal Institute of Technology, Lausanne,
 * Switzerland, and is now developed at the Laboratoire d'Informatique de
 * Nantes-Atlantique, France.
 *
 * Copyright (c) 2001 Swiss Federal Institute of Technology, Switzerland
 * Copyright (c) 2002-2006 Laboratoire d'Informatique de
 *                         Nantes-Atlantique, France
 *--------------------------------------------------------------------------
 * gaol is a software distributed WITHOUT ANY WARRANTY. Read the associated
 * COPYING file for information.
 *--------------------------------------------------------------------------*/
/*!
  \file   gaol_port.h
  \brief  Functions defined for portability, in case they do not exist on one
  platform.

  <long description>

  \author Frederic Goualard
  \date   2001-10-03
*/


#ifndef GAOL_PORT_H
#define GAOL_PORT_H

#include "gaol/gaol_config.h"
#include "gaol/gaol_limits.h"

#include <cmath>
#include <limits>

// _mm_ucomigt_sd() and the others, for the quiet comparisons of Visual C++
// below
#if defined(_MSC_VER) && !defined(__clang__) \
    && (defined(_M_X64) || (defined(_M_IX86_FP) && _M_IX86_FP >= 2))
#  include <emmintrin.h>
#endif

// Alignment on an 'nbytes' bytes boundary
#if defined(_MSC_VER)
#	define GAOL_ALIGN(what,nbytes)	__declspec(align(nbytes)) what
#else
#	define GAOL_ALIGN(what,nbytes) what __attribute__((aligned(nbytes)))
#endif // defined(MSC_VER)
#define GAOL_ALIGN16(what) GAOL_ALIGN(what,16)

// Allocation of 'size' bytes on 'boundary' bytes.
// NOTE: GAOL_MEMALIGN() must return null value if no allocation error
// GAOL_MEMFREE() releases the memory GAOL_MEMALIGN() allocated (operator new
// and operator delete of the SSE2 intervals, gaol_interval_sse.cpp, and of
// the intervals of floats, gaol_interval2f.cpp): the two have to match.
// gaol_allocator.h, the allocator of the containers that used them too, is
// gone (GAOL v5).
#if defined (__MINGW32__) || defined (_MSC_VER)
/* _aligned_malloc() and _aligned_free(), of the C runtime of Windows: malloc()
   aligns on 8 bytes only on 32-bit Windows, and GAOL's SSE2 intervals need
   memory aligned on 16 bytes. */
#  include <malloc.h>
#  define GAOL_MEMALIGN(buf,boundary,size) (!(buf=_aligned_malloc(size,boundary)))
#  define GAOL_MEMFREE(buf) _aligned_free(buf)
#elif defined(GAOL_IX86_MACOSX) || defined(GAOL_ARM_MACOSX)
// According to man page, Intel/MacOSX's malloc aligns correctly for SSE-related types
#  include <stdlib.h>
#  define GAOL_MEMALIGN(buf,boundary,size) (!(buf=malloc(size)))
#  define GAOL_MEMFREE(buf) free(buf)
#else
/* Any other POSIX system, as Linux. _XOPEN_SOURCE is no longer defined again
   here for x86 and ARM (GAOL v5): the code including GAOL had it changed, and
   g++ and clang++ define _GNU_SOURCE there, under which <stdlib.h> declares
   posix_memalign(). */
#  include <stdlib.h>
#  define GAOL_MEMALIGN(buf,boundary,size) posix_memalign(&buf,boundary,size)
#  define GAOL_MEMFREE(buf) free(buf)
#endif


// nextafter and isnan are no longer redefined for Visual C++, which has both
// since Visual Studio 2013, unless the build defined HAVE_NEXTAFTER and
// HAVE_ISNAN (GAOL v5)


namespace gaol_core {

#if GAOL_HAVE_ROUNDING_MATH_OPTION
  GAOL_INLINE double f_negate_simple(double x) { return -x; }
#  define gaol_opposite(x) f_negate_simple(x)
#else
#  define gaol_opposite(x) f_negate(x)
#endif // GAOL_HAVE_ROUNDING_MATH_OPTION

  /*!
    \brief Sign of double

    \return 0 if the argument is positive and 1 otherwise
    \note Returns 1 for -0.0
  */
  extern GAOL_PUBLIC int gaol_signbit(double);


// GAOL_SIZEOF_INT and GAOL_SIZEOF_LONG_LONG_INT come from gaol/gaol_config.h; no build
// defined SIZEOF_LONG_INT, whose branches are gone (GAOL v5)
#if GAOL_SIZEOF_INT==4
#  define GAOL_INT_FOR_DOUBLE int
#else
#  error "Cannot find a 32 bits integer type!"
#endif

#if GAOL_SIZEOF_LONG_LONG_INT==8
#  define GAOL_ULONGLONGINT unsigned long long int
#else
#  error "Cannot find a 64 bits integer type!"
#endif

  typedef union {
    GAOL_ULONGLONGINT i;
    double d;
  } ullidouble;

  typedef union {
    unsigned GAOL_INT_FOR_DOUBLE i[2];
    double d;
  } uintdouble;


#if GAOL_WORDS_BIGENDIAN
#  define GAOL_IFBIGENDIAN(a,b)   (a), (b)
#  define GAOL_HI(x) (*(GAOL_INT_FOR_DOUBLE*)&(x))
#  define GAOL_LO(x) (*((GAOL_INT_FOR_DOUBLE)1+(GAOL_INT_FOR_DOUBLE*)&(x)))
#  define GAOL_LO_UINTDOUBLE(a) ((a).i[1])
#  define GAOL_HI_UINTDOUBLE(a) ((a).i[0])
#else
#  define GAOL_IFBIGENDIAN(a,b)   (b), (a)
#  define GAOL_HI(x) *((GAOL_INT_FOR_DOUBLE)1+(GAOL_INT_FOR_DOUBLE*)&(x))
#  define GAOL_LO(x) *(GAOL_INT_FOR_DOUBLE*)&(x)
#  define GAOL_LO_UINTDOUBLE(a) ((a).i[0])
#  define GAOL_HI_UINTDOUBLE(a) ((a).i[1])
#endif

#ifndef GAOL_NAN
  static const uintdouble NaN_val = {{GAOL_IFBIGENDIAN(0x7ff80000, 0x0)}};
#define GAOL_NAN (gaol_core::NaN_val.d)
#endif

  /*
    +oo, a constant of the compiler (GAOL v5). GAOL took the HUGE_VAL of the C
    library, which the UCRT of Windows writes ((double)(float)1e+300): Visual
    C++ folds the conversion, but clang-cl under /fp:strict makes it when the
    program runs, in the rounding direction of the moment, which gives FLT_MAX
    downward or toward zero, and an overflow flag. interval() was then
    [-FLT_MAX, FLT_MAX], interval(1e300) empty, and [1] / [-1, 1] did not
    contain 1e300. GAOL_INFINITY is in the inline code of the public headers,
    hence in the program too. numeric_limits had been put aside for versions
    of libc++ that GAOL no longer builds with; it already gives the infinite
    bounds of the SSE2 intervals.
  */
#define GAOL_INFINITY (std::numeric_limits<double>::infinity())

  /*
    The bounds of pi and pi/2, which interval::pi(), interval::two_pi() and
    interval::half_pi() give (gaol/gaol_interval.h), written exactly in
    decimal: a literal is converted when compiling, where the unions of GAOL 4
    were read when the program started, by the dynamic initialization of each
    file including this header, and a static object of the program calling
    GAOL before found them 0 (tests/static_initialization.cpp). In their own
    namespace (GAOL v5): GAOL 4 declared these doubles in the namespace of
    GAOL, with two_pi, pi, half_pi, ln2_dn, ln2_up, two_power_51 and
    two_power_52, which met the names of the program that opened it, a pi of
    its own being ambiguous. The manual of GAOL v5 no longer documents them.
    That namespace, gaol_detail, holds what GAOL's headers define for their
    own use; it is a namespace of its own, not gaol_core::detail, which the
    namespace gaol took whole: a program with a namespace detail of its own
    writing using namespace gaol; found two, and detail::f() did not compile
    ("reference to 'detail' is ambiguous", GAOL v5 before its release).
  */
} // namespace gaol_core

namespace gaol_detail {
  const double pi_dn = 3.141592653589793115997963468544185161590576171875;
  const double pi_up = 3.141592653589793560087173318606801331043243408203125;
  const double half_pi_dn = 1.5707963267948965579989817342720925807952880859375;
  const double half_pi_up = 1.5707963267948967800435866593034006655216217041015625;
} // namespace gaol_detail

namespace gaol_core {

  /*
    The quiet comparisons of <cmath>, false for a NaN operand, which they
    compare without raising the invalid-operation exception, and which GAOL
    makes wherever a bound may be NaN (GAOL v5): std::isless(),
    std::islessequal(), std::isgreater(), std::isgreaterequal() and
    std::isunordered() are one instruction with GCC and Clang (ucomisd on
    x86), but calls to _dpcomp() of the C library with Visual C++, which made
    x * y and sqrt() 34 to 61% slower once the first comparison of the
    constructors and is_empty() were quiet ones (Visual Studio 2022 and 2026,
    x64, continuous integration). With Visual C++ for x64, and for x86 with
    SSE2, they are therefore ucomisd in line, through _mm_ucomigt_sd() and
    _mm_ucomige_sd(), the operands exchanged for "less", and cmpunordsd for
    std::isunordered(): "greater" and "greater or equal" are false for
    unordered operands with every compiler, where _mm_ucomilt_sd(),
    _mm_ucomile_sd() and _mm_ucomieq_sd() are true for them with GCC 9.4,
    which reads the flags ucomisd sets without its parity flag. Elsewhere they
    are those of <cmath>.
  */
} // namespace gaol_core

namespace gaol_detail {
#if defined(_MSC_VER) && !defined(__clang__) \
  && (defined(_M_X64) || (defined(_M_IX86_FP) && _M_IX86_FP >= 2))
  GAOL_INLINE bool quiet_less(double x, double y)
  {
    return _mm_ucomigt_sd(_mm_set_sd(y), _mm_set_sd(x)) != 0;
  }

  GAOL_INLINE bool quiet_less_equal(double x, double y)
  {
    return _mm_ucomige_sd(_mm_set_sd(y), _mm_set_sd(x)) != 0;
  }

  GAOL_INLINE bool quiet_greater(double x, double y)
  {
    return _mm_ucomigt_sd(_mm_set_sd(x), _mm_set_sd(y)) != 0;
  }

  GAOL_INLINE bool quiet_greater_equal(double x, double y)
  {
    return _mm_ucomige_sd(_mm_set_sd(x), _mm_set_sd(y)) != 0;
  }

  GAOL_INLINE bool quiet_unordered(double x, double y)
  {
    return (_mm_movemask_pd(_mm_cmpunord_sd(_mm_set_sd(x), _mm_set_sd(y))) & 1) != 0;
  }
#else
  GAOL_INLINE bool quiet_less(double x, double y)
  {
    return std::isless(x, y);
  }

  GAOL_INLINE bool quiet_less_equal(double x, double y)
  {
    return std::islessequal(x, y);
  }

  GAOL_INLINE bool quiet_greater(double x, double y)
  {
    return std::isgreater(x, y);
  }

  GAOL_INLINE bool quiet_greater_equal(double x, double y)
  {
    return std::isgreaterequal(x, y);
  }

  GAOL_INLINE bool quiet_unordered(double x, double y)
  {
    return std::isunordered(x, y);
  }
#endif
} // namespace gaol_detail

namespace gaol_core {

  /*!
    \brief Returns 1 if d is neither a NaN nor an infinity

    std::isfinite() of C++11, the same in every build: finite() of the C
    library was used where the build system found it, and is not declared by
    every C library (Visual C++, recent C++ libraries with -std=c++11).
   */
  GAOL_INLINE int is_finite(double d)
  {
    return std::isfinite(d);
  }


} // namespace gaol_core

// In the namespace gaol too, as in GAOL 4, but for NaN_val, which GAOL uses
// for itself (see gaol/gaol_interval.h)
namespace gaol {
#if GAOL_HAVE_ROUNDING_MATH_OPTION
  using gaol_core::f_negate_simple;
#endif
  using gaol_core::gaol_signbit;
  using gaol_core::ullidouble;
  using gaol_core::uintdouble;
  using gaol_core::is_finite;
} // namespace gaol

#endif /* GAOL_PORT_H */
