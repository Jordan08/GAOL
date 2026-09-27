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


#ifndef __gaol_port_h__
#define __gaol_port_h__

#include "gaol/gaol_config.h"
#include "gaol/gaol_limits.h"

#include <cmath>

// Alignment on an 'nbytes' bytes boundary
#if defined(_MSC_VER)
#	define GAOL_ALIGN(what,nbytes)	__declspec(align(nbytes)) what
#else
#	define GAOL_ALIGN(what,nbytes) what __attribute__((aligned(nbytes)))
#endif // defined(MSC_VER)
#define GAOL_ALIGN16(what) GAOL_ALIGN(what,16)

// Allocation of 'size' bytes on 'boundary' bytes.
// NOTE: MEMALIGN() must return null value if no allocation error
// MEMFREE() releases the memory MEMALIGN() allocated (gaol_allocator.h,
// gaol_interval_sse.cpp): the two have to match.
#if defined (__MINGW32__) || defined (_MSC_VER)
/* _aligned_malloc() and _aligned_free(), of the C runtime of Windows: malloc()
   aligns on 8 bytes only on 32-bit Windows, and GAOL's SSE2 intervals need
   memory aligned on 16 bytes. */
#  include <malloc.h>
#  define MEMALIGN(buf,boundary,size) (!(buf=_aligned_malloc(size,boundary)))
#  define MEMFREE(buf) _aligned_free(buf)
#elif defined(IX86_LINUX) || defined(AARCH64_LINUX)
#  undef _XOPEN_SOURCE
#  define _XOPEN_SOURCE 600
#  include <stdlib.h>
#  define MEMALIGN(buf,boundary,size) posix_memalign(&buf,boundary,size)
#  define MEMFREE(buf) free(buf)
#elif defined(IX86_MACOSX) || defined(ARM_MACOSX)
// According to man page, Intel/MacOSX's malloc aligns correctly for SSE-related types
#  include <stdlib.h>
#  define MEMALIGN(buf,boundary,size) (!(buf=malloc(size)))
#  define MEMFREE(buf) free(buf)
#else
/* Any other POSIX system, as Linux */
#  include <stdlib.h>
#  define MEMALIGN(buf,boundary,size) posix_memalign(&buf,boundary,size)
#  define MEMFREE(buf) free(buf)
#endif


// nextafter and isnan are no longer redefined for Visual C++, which has both
// since Visual Studio 2013, unless the build defined HAVE_NEXTAFTER and
// HAVE_ISNAN (GAOL v5)


namespace gaol_core {

#if HAVE_ROUNDING_MATH_OPTION
  INLINE double f_negate_simple(double x) { return -x; }
#  define gaol_opposite(x) f_negate_simple(x)
#else
#  define gaol_opposite(x) f_negate(x)
#endif // HAVE_ROUNDING_MATH_OPTION

  /*!
    \brief Sign of double

    \return 0 if the argument is positive and 1 otherwise
    \note Returns 1 for -0.0
  */
  extern __GAOL_PUBLIC__ int gaol_signbit(double);


// SIZEOF_INT and SIZEOF_LONG_LONG_INT come from gaol/gaol_config.h; no build
// defined SIZEOF_LONG_INT, whose branches are gone (GAOL v5)
#if SIZEOF_INT==4
#  define INT_FOR_DOUBLE int
#else
#  error "Cannot find a 32 bits integer type!"
#endif

#if SIZEOF_LONG_LONG_INT==8
#  define ULONGLONGINT unsigned long long int
#else
#  error "Cannot find a 64 bits integer type!"
#endif

  typedef union {
    ULONGLONGINT i;
    double d;
  } ullidouble;

  typedef union {
    unsigned INT_FOR_DOUBLE i[2];
    double d;
  } uintdouble;


#if WORDS_BIGENDIAN
#  define IFBIGENDIAN(a,b)   (a), (b)
#  define __HI(x) (*(INT_FOR_DOUBLE*)&(x))
#  define __LO(x) (*((INT_FOR_DOUBLE)1+(INT_FOR_DOUBLE*)&(x)))
#  define LO_UINTDOUBLE(a) ((a).i[1])
#  define HI_UINTDOUBLE(a) ((a).i[0])
#else
#  define IFBIGENDIAN(a,b)   (b), (a)
#  define __HI(x) *((INT_FOR_DOUBLE)1+(INT_FOR_DOUBLE*)&(x))
#  define __LO(x) *(INT_FOR_DOUBLE*)&(x)
#  define LO_UINTDOUBLE(a) ((a).i[0])
#  define HI_UINTDOUBLE(a) ((a).i[1])
#endif

#ifndef GAOL_NAN
  static const uintdouble NaN_val = {{IFBIGENDIAN(0x7ff80000, 0x0)}};
#define GAOL_NAN (gaol_core::NaN_val.d)
#endif

  // #define GAOL_INFINITY std::numeric_limits<double>::infinity()
  /* We cannot use the definition above because some versions of libc++ do not
     define the infinity() method correctly.
  */
#define GAOL_INFINITY HUGE_VAL

  /*
    Various constants rounded up and down
  */
  //! Pi rounded towards -oo.
  const uintdouble upi_dn = {{IFBIGENDIAN(1074340347,1413754136)}};
  //! Pi rounded towards +oo.
  const uintdouble upi_up   = {{IFBIGENDIAN(1074340347,1413754137)}};
  //! Pi/2 rounded towards -oo.
  const uintdouble uhalfpi_dn = {{IFBIGENDIAN(1073291771,1413754136)}};
  //! Pi/2 rounded towards +oo.
  const uintdouble uhalfpi_up = {{IFBIGENDIAN(1073291771,1413754137)}};
  // ln(2) rounded towards -oo
  const uintdouble uln2_dn = {{IFBIGENDIAN(0x3fe62e42,0xFEFA39EF)}};
  // ln(2) rounded towards +oo
  const uintdouble uln2_up = {{IFBIGENDIAN(0x3fe62e42,0xFEFA39F0)}};


  /* The same doubles, written exactly in decimal (GAOL v5). Read from the
     unions above, they were computed when the program started, by the
     dynamic initialization of each file including this header: the
     functions of the static library GAOL found them 0 when a static object of
     the program called them before the files of GAOL were initialized, and
     gave bounds that did not enclose the results. A literal is converted when
     compiling, and the exact value of a double needs no rounding.
     tests/static_initialization.cpp checks them against the bits above. */
  const double pi_dn = 3.141592653589793115997963468544185161590576171875;
  const double pi_up = 3.141592653589793560087173318606801331043243408203125;

  const double half_pi_dn = 1.5707963267948965579989817342720925807952880859375;
  const double half_pi_up = 1.5707963267948967800435866593034006655216217041015625;

  const double two_pi = 6.28318530717958647693;
  const double pi = 3.14159265358979323846;
  const double half_pi = 1.57079632679489661923;

  const double ln2_dn = 0.69314718055994528622676398299518041312694549560546875;
  const double ln2_up = 0.6931471805599453972490664455108344554901123046875;

  const double two_power_51 = 2251799813685248.0;
  const double two_power_52 = 4503599627370496.0;

  /*!
    \brief Returns 1 if d is neither a NaN nor an infinity

    std::isfinite() of C++11, the same in every build: finite() of the C
    library was used where the build system found it, and is not declared by
    every C library (Visual C++, recent C++ libraries with -std=c++11).
   */
  INLINE int is_finite(double d)
  {
    return std::isfinite(d);
  }


} // namespace gaol_core

#endif /* __gaol_port_h__ */
