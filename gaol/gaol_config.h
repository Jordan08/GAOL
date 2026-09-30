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

/* The configuration the build wrote, the same macros with the three builds
   and every compiler (see doc/building.md): CMake from
   cmake/gaol_configuration.h.in, configure from gaol/gaol_configuration.h.in,
   meson from gaol/gaol_configuration_meson.h.in. It no longer defines
   PACKAGE nor VERSION, which were undefined before including it, and
   undefined those of the code including GAOL with them; Visual C++ and MinGW
   included it through gaol/gaol_config_msvc.h and gaol/gaol_config_mingw.h,
   which included nothing else (GAOL v5). */
#include "gaol/gaol_configuration.h"

#define GAOL_ERRNO errno
#define INLINE inline

#if defined (_MSC_VER)

# ifndef __GAOL_PUBLIC__
#   ifdef _COMPILING__GAOL_PUBLIC__
#     define __GAOL_PUBLIC__ __declspec(dllexport)
#   else
#     define __GAOL_PUBLIC__ __declspec(dllimport)
#   endif
# endif

#elif defined (__GNUC__)

# ifndef __GAOL_PUBLIC__
#  if defined (HAVE_VISIBILITY_OPTIONS)
#     define __GAOL_PUBLIC__ __attribute__ ((visibility("default")))
#  else
#     define __GAOL_PUBLIC__
#  endif
# endif

#else

# ifndef __GAOL_PUBLIC__
#  define __GAOL_PUBLIC__
# endif

#endif


/* ---------------------------------------------------------------------------
   The intervals of floats, gaol::intervalf and gaol::interval2f

   Unfinished (sqrt(intervalf) returns its argument, interval2f::inverse()
   aborts, pow(interval2f, int) does not handle the empty set) and used by
   nothing: GAOL leaves them out, and none of its three builds has an option
   to compile them (GAOL v5). A developer of GAOL working on them defines
   GAOL_FLOAT_INTERVALS below, for GAOL and the code using them alike:
   gaol/gaol_interval.cpp then compiles them, gaol::interval2f where SSE3
   instructions are used (USING_SSE3_INSTRUCTIONS), and the code using them
   includes their headers, gaol/gaol_intervalf.h and gaol/gaol_interval2f.h,
   which the builds do not install and gaol/gaol does not include.
   --------------------------------------------------------------------------- */

/* #define GAOL_FLOAT_INTERVALS 1 */


/* ---------------------------------------------------------------------------
   The target, from the macros of the compiler

   What GAOL needs to know of the processor and the system comes from the
   compiler rather than from the build system, so that the CMake, autotools
   and meson builds agree, and cross-compilation and containers (an armhf
   container on an arm64 machine) get the target rather than the machine
   building. GAOL reads the names of the system to allocate aligned memory
   (gaol/gaol_port.h) and to negate a double (gaol/gaol_fpu_fenv.h); both have
   a fallback for the other systems. The sizes of the integer types come from
   <limits.h>, the builds no longer measuring them (GAOL v5).
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

/* GAOL_NODISCARD: the attribute on the functions whose result is all they do
   (GAOL v5), so that the compiler warns about a call whose result is thrown
   away, as sqrt(I); or I.emptyset(); written to change I, which they leave
   as it was. A program is compiled in the standard its project sets, or else
   in the one of its compiler, C++14 for GCC 6 to 10, Clang 6 to 15 and Visual
   C++; GAOL itself is compiled in C++11, in C++14 with Visual C++, which has
   no C++11 mode. The macro is [[nodiscard]] from C++17 on, and before C++17
   where __has_cpp_attribute(nodiscard) says the compiler takes it there: GCC
   7 and later, silent about it even with -pedantic, and Visual C++ 2019 16.4
   (_MSC_VER 1924) and later, whose warning is C4834. Clang says so too, but
   warns about it with -pedantic (-Wc++17-attribute-extensions): it takes
   __attribute__((warn_unused_result)), whose warning is -Wunused-result (on
   by default), as GCC before 7 does. Visual C++ 2017 and 2019 16.0 refuse
   [[nodiscard]] before C++17 (error C2429), and 2019 16.1 to 16.3 ignore it
   (warning C5051): __has_cpp_attribute(nodiscard) is 0 in 2019 16.0 to 16.3,
   but 2017 15.8 and 15.9, the first to have __has_cpp_attribute, were not
   checked, and the macro asks for _MSC_VER 1924 too. Older Visual C++ takes
   _Check_return_ of <sal.h>, which only the code analysis reports (/analyze,
   warning C6031). A cast to void silences them all but GCC's attribute.
   Visual C++ keeps __cplusplus at 199711L without /Zc:__cplusplus, and gives
   the standard in _MSVC_LANG; Clang for Windows defines __clang__ and
   _MSC_VER, and takes the attribute of Clang. Defining GAOL_NODISCARD before
   including GAOL replaces it, an empty definition removing the attribute. */
#ifndef GAOL_NODISCARD
#  if defined(__cplusplus) \
      && (__cplusplus >= 201703L || (defined(_MSVC_LANG) && _MSVC_LANG >= 201703L))
#    define GAOL_NODISCARD [[nodiscard]]
#  elif defined(__cplusplus) && defined(__has_cpp_attribute) && !defined(__clang__) \
        && (!defined(_MSC_VER) || _MSC_VER >= 1924)
#    if __has_cpp_attribute(nodiscard)
#      define GAOL_NODISCARD [[nodiscard]]
#    endif
#  endif
#  ifndef GAOL_NODISCARD
#    if defined(__GNUC__) || defined(__clang__)
#      define GAOL_NODISCARD __attribute__((warn_unused_result))
#    elif defined(_MSC_VER) && _MSC_VER >= 1400
#      include <sal.h>
#      define GAOL_NODISCARD _Check_return_
#    else
#      define GAOL_NODISCARD
#    endif
#  endif
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
/* -ffinite-math-only, which -ffast-math and -Ofast turn on, has the compiler
   take NaN and infinities never to occur, in the inline functions of GAOL's
   headers as anywhere else: the empty interval has NaN bounds, and is_empty()
   reads it as !(left() <= right()). The compiler then folds the test away, and
   ([1, 2] & [3, 4]).is_empty() is false (GCC 9, Clang 18): the tests
   refused_finite_math_only and refused_fast_math (tests/CMakeLists.txt) check
   the refusal. GCC and Clang define __FINITE_MATH_ONLY__, as 0 or 1; Visual
   C++ defines nothing of the kind (its /fp:fast is refused below). This also
   refuses what __FAST_MATH__ does not show: with Clang, -Ofast or -ffast-math
   followed by -frounding-math leave it undefined, and __FINITE_MATH_ONLY__ at
   1. -fno-fast-math, one of the flags of gaol.pc and gaol::gaol, turns the
   option off when it comes after it on the command line, and does nothing when
   it comes before.

   No macro shows what follows, which GAOL cannot refuse:
   -funsafe-math-optimizations and -ffast-math -fno-finite-math-only, with
   which GCC rewrites the probe 1.0 + tiny == 1.0 of round_upward_if_needed()
   (gaol/gaol_fpu.h) as tiny == 0.0, so that an operation does not set the
   rounding direction upward again after the code using GAOL left it elsewhere,
   and width() is below the exact width; and -fno-honor-nans of Clang, given
   without -fno-honor-infinities, which does to the empty interval what
   -ffinite-math-only does. */
#if defined(__FINITE_MATH_ONLY__) && __FINITE_MATH_ONLY__
#  error "GAOL cannot be compiled with -ffinite-math-only (which -ffast-math and -Ofast turn on): its empty interval has NaN bounds and its unbounded ones infinite bounds, which the compiler then takes never to occur (-fno-fast-math, one of the flags of gaol.pc and gaol::gaol, turns it off when it comes after)"
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
#  error "GAOL needs doubles computed with SSE2 on x86 processors (-msse2 -mfpmath=sse): computed on the x87 unit, in extended precision, CORE-MATH rounds some results twice and some of its bounds are wrong (see tests/extended_precision.cpp)"
#endif
#if defined(_M_IX86_FP) && (_M_IX86_FP < 2)
#  error "GAOL needs doubles computed with SSE2 on x86 processors (/arch:SSE2): computed on the x87 unit, in extended precision, CORE-MATH rounds some results twice and some of its bounds are wrong (see tests/extended_precision.cpp)"
#endif
#if defined(__arm__) && !defined(__aarch64__) && defined(__clang__)
#  error "GAOL cannot be compiled by Clang for 32-bit ARM processors: Clang does not honour the rounding direction there (see CMakeLists.txt)"
#endif
/* mingw-w64 whose fma() or round() is wrong. Those of its own math library
   (math/fma.c and math/round.c, the same code from version 9 to 13) are, on
   x86-64:

   - fma() adds the four products of the halves of x and y to z with four
     roundings. With mingw-w64 11 on x86-64 (under wine), 10.7 % of the
     error-free products fma(a, b, -a*b) and 25 to 73 % of other triples,
     depending on the rounding direction, were wrong. CORE-MATH computes with
     __builtin_fma(), a call to fma() without -mfma (GCC for Windows, see
     CMakeLists.txt), and GAOL's exact products with std::fma(): bounds of
     tan, asin, atan and others at small arguments did not enclose the exact
     values (tan(0x1.56e1fc2f8f359p-997) rounded upward was two doubles above
     the argument), in tests/core_math.cpp, elementary.cpp and reverse.cpp,
     and in the continuous integration with GCC 11 to 13 of Chocolatey;
   - round() is ceil(x), less 1 when that is more than 1/2 above x, a
     difference computed in doubles, rounded in the direction in effect:
     round(0x1.fffffffffffffp-2) is 1 in every direction but upward, and
     round_ties_to_away() (gaol_interval.h) calls it in the caller's.

   Before version 12 both are in libmingwex, which every program links; from
   12 on, only in the library of msvcrt.dll, which lacks them. A toolchain
   linking the UCRT then takes those of ucrtbase.dll, which pass the tests;
   one linking msvcrt.dll still takes mingw-w64's (those of MSYS2 MINGW64,
   whose headers say mingw-w64 14, gave the same wrong bounds). _UCRT tells
   them apart, for GCC and Clang: the headers of a UCRT toolchain define it
   (__MSVCRT_VERSION__ 0xE00, the default from mingw-w64 12 on), and so does
   -mcrtdll=ucrt of GCC 14 and later; nothing does for msvcrt.dll (MSYS2
   MINGW64, the cross compilers of Debian and Ubuntu). A program defining
   _UCRT itself while linking msvcrt.dll is not refused here, and fails
   tests/core_math.cpp. Accepted and tested: MinGW-Builds GCC 14 and 15,
   MSYS2 UCRT64 and CLANG64. Cygwin, whose headers of Windows are those of
   mingw-w64 but whose math library is its own, does not say __MINGW32__.

   On 32-bit x86, the same fma() and round() compute on the x87 unit, in
   extended precision where the compiler keeps the values in its registers.
   The GCC 11.2 of WinLibs (mingw-w64 9) compiled fma() without optimization,
   each partial sum rounded to a double: the continuous integration saw the
   same wrong bounds as on x86-64, and mingw-w64 before 11 is refused.
   MinGW-Builds GCC 12 and 13 (mingw-w64 11) and the mingw-w64 11 of Ubuntu,
   accepted, keep the sums in extended precision: the error-free products
   tried were exact and the tests pass, but 125 of 400,000 other results were
   one double off. round() is right there, in the four builds looked at, but
   for the sign of a zero rounding downward. From version 12 on, a toolchain
   linking the UCRT (MinGW-Builds GCC 14 and 15) takes those of ucrtbase.dll,
   as on x86-64.

   On ARM, mingw-w64 before 11 stays refused, as before, not tested with GAOL
   v5. Its fma() is an instruction: fmadd, fused, on 64-bit ARM; fmacd, which
   is vmla.f64, a product and a sum each rounded, on 32-bit ARM, where Clang,
   the only compiler of mingw-w64 there, is refused above. Its round() is the
   code above, computed in doubles, though Clang 18 for 64-bit ARM computes
   round() with the instruction frinta, even without optimization. */
#if defined(__MINGW32__) && defined(__MINGW64_VERSION_MAJOR)
#  if defined(__x86_64__) && (__MINGW64_VERSION_MAJOR < 12 || !defined(_UCRT))
#    error "GAOL cannot be compiled with this mingw-w64: before version 12, or linked with msvcrt.dll rather than the UCRT, its fma() and round() are those of mingw-w64's own math library, whose fma() is not correctly rounded and whose round() depends on the rounding direction, and the bounds of the elementary functions (CORE-MATH computes with fma()) would not enclose the exact values. Build GAOL with a toolchain of mingw-w64 12 or later linking the UCRT: the MinGW-w64 GCC 14 or 15 of MinGW-Builds (choco install mingw --version=15.2.0), MSYS2 UCRT64 or CLANG64 (pacman -S mingw-w64-ucrt-x86_64-gcc), or with Visual Studio"
#  elif defined(__i386__) && __MINGW64_VERSION_MAJOR < 11
#    error "GAOL cannot be compiled with this mingw-w64 for 32-bit x86 before version 11, as the mingw-w64 9 of WinLibs (GCC 11), whose fma(), which CORE-MATH computes with, rounds each of its partial sums to a double: the bounds of the elementary functions did not enclose the exact values. Build GAOL with the MinGW-w64 GCC 14 or 15 of MinGW-Builds (choco install mingw --version=15.2.0 --x86), which link the UCRT, or with Visual Studio"
#  elif !defined(__x86_64__) && !defined(__i386__) && __MINGW64_VERSION_MAJOR < 11
#    error "GAOL cannot be compiled with this mingw-w64 for ARM before version 11: GAOL v5 was not tested with it, and the round() of mingw-w64's own math library, computed in doubles, depends on the rounding direction. Build GAOL with Visual Studio"
#  endif
#endif

#endif /* __gaol_config_h__ */
