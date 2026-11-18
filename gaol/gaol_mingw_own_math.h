/*-*-C-*--------------------------------------------------------------------
 * GAOL -- not Just Another Interval Library
 *--------------------------------------------------------------------------
 * GAOL_MINGW_OWN_MATH: 1 where the fma() and round() that a program gets from
 * the C library are those of mingw-w64's own math library, which are wrong, 0
 * everywhere else (GAOL v5).
 *
 * Those of mingw-w64 (math/fma.c and math/round.c, the same code from version
 * 9 to 13) are in libmingwex up to version 11, which every program links, the
 * UCRT ones included, and from version 12 on only in the library of
 * msvcrt.dll, so that a toolchain linking the UCRT takes those of
 * ucrtbase.dll. The macro is 1 for the first, and for the second linked with
 * msvcrt.dll (MSYS2 MINGW64 and MINGW32, the cross compilers of Debian and
 * Ubuntu): mingw-w64 before version 12, or without _UCRT, which the headers of
 * a UCRT toolchain define. gaol/gaol_config.h says what is wrong, and where
 * GAOL calls its own gaol_round() (gaol/math/gaol_round.h) and the instruction
 * instead.
 *
 * The test is written once, here, for the two files that need it: the C++
 * header gaol/gaol_config.h, and gaol/math/gaol_core_math_port.h, which the
 * three builds give to the C sources of CORE-MATH and which cannot include
 * gaol_config.h. This header is C, which is why it has C comments only, and it
 * is installed, as gaol_config.h includes it. It is always defined to 0 or 1,
 * so that #if reads it under -Wundef.
 *--------------------------------------------------------------------------
 * Copyright (c) 2026 ENSTA, France
 *
 * By: Jordan NININ (created 2026-11-18)
 *--------------------------------------------------------------------------
 * GAOL is a software distributed WITHOUT ANY WARRANTY. Read the associated
 * COPYING.LIB file for information.
 *--------------------------------------------------------------------------*/

#ifndef GAOL_MINGW_OWN_MATH_H
#define GAOL_MINGW_OWN_MATH_H

/* The headers of the C library of mingw-w64 define __MINGW64_VERSION_MAJOR,
   and _UCRT with the UCRT, from their _mingw.h, which math.h includes */
#include <math.h>

#if defined(__MINGW32__) && defined(__MINGW64_VERSION_MAJOR) \
    && (__MINGW64_VERSION_MAJOR < 12 || !defined(_UCRT))
#  define GAOL_MINGW_OWN_MATH 1
#else
#  define GAOL_MINGW_OWN_MATH 0
#endif

#endif /* GAOL_MINGW_OWN_MATH_H */
