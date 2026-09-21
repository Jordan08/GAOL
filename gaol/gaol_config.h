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
 *--------------------------------------------------------------------------
 * CVS: $Id: gaol_config.h 191 2012-03-06 17:08:58Z goualard $
 * Last modified:
 * By:
 *--------------------------------------------------------------------------*/

/*!
  \file   gaol_config.h
  \brief  Configuration file

  This file contains the declarations of all the macros used to control the
  way gaol is compiled, depending on the host. It acts as a front-end for
  the configuration file created by configure and the one used when
  compiling gaol with Borland C++ Builder.


  \author Frederic Goualard
  \date   2002-12-03
*/

#ifndef __gaol_config_h__
#define __gaol_config_h__

#if defined (_MSC_VER)

# define GAOL_ERRNO errno
# ifndef __GAOL_PUBLIC__
#   ifdef _COMPILING__GAOL_PUBLIC__
#     define __GAOL_PUBLIC__ __declspec(dllexport)
#   else
#     define __GAOL_PUBLIC__ __declspec(dllimport)
#   endif
# endif
# define INLINE inline
# include "gaol/gaol_config_msvc.h"

#elif defined (__MINGW32__)

# define GAOL_ERRNO errno
# undef PACKAGE
# undef VERSION
# include "gaol/gaol_config_mingw.h"
# ifndef __GAOL_PUBLIC__
#  if defined (HAVE_VISIBILITY_OPTIONS)
#     define __GAOL_PUBLIC__ __attribute__ ((visibility("default")))
#  else
#     define __GAOL_PUBLIC__
#  endif
# endif
# define INLINE inline

#elif defined (__GNUC__)
# define GAOL_ERRNO errno
# undef PACKAGE
# undef VERSION
# include "gaol/gaol_configuration.h"
# ifndef __GAOL_PUBLIC__
#  if defined (HAVE_VISIBILITY_OPTIONS)
#     define __GAOL_PUBLIC__ __attribute__ ((visibility("default")))
#  else
#     define __GAOL_PUBLIC__
#  endif
# endif
# define INLINE inline

#else

# define GAOL_ERRNO errno
# undef PACKAGE
# undef VERSION
# ifndef __GAOL_PUBLIC__
#  define __GAOL_PUBLIC__
# endif
# define INLINE inline
# include "gaol/gaol_configuration.h"
#endif


/* ---------------------------------------------------------------------------
   The target, from the macros of the compiler

   What GAOL needs to know of the processor and the system comes from the
   compiler rather than from the build system, so that the CMake, autotools
   and meson builds agree, and cross-compilation and containers (an armhf
   container on an arm64 machine) get the target rather than the machine
   building. GAOL reads the names of the system to allocate aligned memory
   (gaol/gaol_port.h) and to negate a double (gaol/gaol_fpu_fenv.h); both have
   a fallback for the other systems. The sizes of the integer types come from
   <limits.h> where the build system did not measure them.
   --------------------------------------------------------------------------- */

#include <limits.h>

#if defined(__linux__) && (defined(__i386__) || defined(__x86_64__))
/* Define this if your system is a Linux-based ix86 or compatible */
#  define IX86_LINUX 1
#elif defined(__linux__) && defined(__aarch64__)
/* Define this if your system is an AARCH64-based computer under Linux */
#  define AARCH64_LINUX 1
#elif defined(__APPLE__) && (defined(__i386__) || defined(__x86_64__))
/* Define this if your system is a ix86 running MacOSX */
#  define IX86_MACOSX 1
#elif defined(__APPLE__) && (defined(__aarch64__) || defined(__arm__))
/* Define this if your system is an ARM running MacOSX */
#  define ARM_MACOSX 1
#endif

#ifndef SIZEOF_INT
#  if UINT_MAX == 0xFFFFFFFFu
#    define SIZEOF_INT 4
#  elif UINT_MAX == 0xFFFFFFFFFFFFFFFFu
#    define SIZEOF_INT 8
#  endif
#endif
#ifndef SIZEOF_LONG
#  if ULONG_MAX == 0xFFFFFFFFul
#    define SIZEOF_LONG 4
#  elif ULONG_MAX == 0xFFFFFFFFFFFFFFFFul
#    define SIZEOF_LONG 8
#  endif
#endif
#ifndef SIZEOF_LONG_LONG_INT
#  if ULLONG_MAX == 0xFFFFFFFFFFFFFFFFull
#    define SIZEOF_LONG_LONG_INT 8
#  endif
#endif

/* WORDS_BIGENDIAN: the processor stores words with the most significant byte
   first. The test on __BYTE_ORDER__, which GCC and Clang define, is the one of
   the fork of mathlib by Fabrice Le Bars (https://github.com/lebarsfa/mathlib,
   src/mathlib_endian.h); Visual C++ only targets little-endian processors. */
#if !defined(WORDS_BIGENDIAN) && defined(__BYTE_ORDER__) && defined(__ORDER_BIG_ENDIAN__) \
    && (__BYTE_ORDER__ == __ORDER_BIG_ENDIAN__)
#  define WORDS_BIGENDIAN 1
#endif


/* ---------------------------------------------------------------------------
   What GAOL requires of the compiler and the target

   The CMake, autotools and meson builds refuse these first, with messages
   naming what to use instead; checked here again for the code that includes
   GAOL's headers, whose interval operations are inline. Each of them made
   GAOL compute bounds not enclosing the exact results, or worse (see the
   README and CMakeLists.txt). /fp:strict, which the builds give to GAOL and
   gaol::gaol to the code linking it, is required rather than refused when
   missing: without it, Visual C++ assumes rounding to nearest.
   --------------------------------------------------------------------------- */

#if defined(__FAST_MATH__)
#  error "GAOL cannot be compiled with -ffast-math (nor -Ofast): the bounds it computes would not enclose the exact results"
#endif
#if defined(_M_FP_FAST)
#  error "GAOL cannot be compiled with /fp:fast: the bounds it computes would not enclose the exact results (it needs /fp:strict)"
#endif
/* Visual C++, not clang-cl (which says __clang__, and whose macros of the
   floating-point model are not checked): /fp:strict defines _M_FP_STRICT, on
   x86, x64 and arm64 with Visual Studio 2022 and 2026, and nothing else does. */
#if defined(_MSC_VER) && !defined(__clang__) && !defined(_M_FP_FAST) && !defined(_M_FP_STRICT)
#  error "GAOL cannot be compiled by Visual C++ without /fp:strict: Visual C++ then assumes rounding to nearest, and may evaluate or rewrite floating-point operations accordingly, so that the bounds GAOL computes would not be certified (gaol::gaol, of the CMake package of GAOL, gives /fp:strict to the code linking it)"
#endif
#if (defined(__i386__) || defined(__x86_64__)) && defined(__GNUC__) && !defined(__SSE2_MATH__)
#  error "GAOL needs doubles computed with SSE2 on x86 processors (-msse2 -mfpmath=sse): computed on the x87 unit, in extended precision, its bounds are wrong"
#endif
#if defined(_M_IX86_FP) && (_M_IX86_FP < 2)
#  error "GAOL needs doubles computed with SSE2 on x86 processors (/arch:SSE2): computed on the x87 unit, in extended precision, its bounds are wrong"
#endif
#if defined(__arm__) && !defined(__aarch64__) && defined(__clang__)
#  error "GAOL cannot be compiled by Clang for 32-bit ARM processors: Clang does not honour the rounding direction there (see CMakeLists.txt)"
#endif
/* mingw-w64 whose <fenv.h> answers fegetround() from a state of its own rather
   than from the registers. GAOL sets the rounding direction by writing the
   registers, and the elementary functions of CORE-MATH read fegetround() to
   know it: those versions then computed for another direction, and the bounds
   did not enclose the exact values (tan, asin and atan of small arguments, in
   the continuous integration).

   The two reasons that had GAOL refuse mingw-w64 before are gone (the
   hyperbolic functions of its math library, which GAOL no longer uses, and the
   cost of its fesetround(), which GAOL no longer calls for its elementary
   functions), so more versions are accepted than before: MinGW-w64 GCC 14 and
   15 on x86-64 (mingw-w64 12 and 13), and GCC 12 to 15 on 32-bit x86
   (mingw-w64 11 and later), where GAOL sets both units itself. GAOL was built
   and tested with each of them (GAOL v5). */
#if defined(__MINGW64_VERSION_MAJOR) \
    && ((defined(__x86_64__) && __MINGW64_VERSION_MAJOR < 12) \
        || (!defined(__x86_64__) && __MINGW64_VERSION_MAJOR < 11))
#  error "GAOL cannot be compiled with this mingw-w64: its <fenv.h> answers fegetround() from a state of its own rather than from the registers, and the elementary functions of CORE-MATH read fegetround() to know the rounding direction GAOL set by writing the registers, so that the bounds would not enclose the exact values. Build GAOL with the MinGW-w64 GCC 14 or 15 of MinGW-Builds (choco install mingw --version=15.2.0), or the one of MSYS2, or with Visual Studio"
#endif

#endif /* __gaol_config_h__ */
