/*-*-C-*---------------------------------------------------------------------
 * gaol -- NOT Just Another Interval Library
 *--------------------------------------------------------------------------
 * What the sources of CORE-MATH (gaol/core_math_*.c, see gaol_core_math.h)
 * need to compile as a part of GAOL, with every compiler GAOL is built with.
 * Included first by each of them; not installed.
 *
 * - The functions are named gaol_cr_sinh()... rather than cr_sinh()..., so
 *   that GAOL does not clash with a program or a C library that has
 *   CORE-MATH's functions too, and are declared here as gaol_core_math.h
 *   declares them, visible from a shared library.
 * - Visual C++ has none of the builtins of GCC the sources use, nor
 *   __attribute__: the functions of <math.h> stand for the former.
 * - The warnings on conversions GAOL's library is compiled with (-Wall
 *   -Wconversion) are turned off for them.
 *
 * The sources themselves are those of CORE-MATH, but for this header being
 * included and for ~0ul written ~(u64)0 (sinh, cosh, tanh): unsigned long has
 * 32 bits on Windows, where ~0ul>>12 is not the mask of the 52 low bits.
 *--------------------------------------------------------------------------
 * gaol is a software distributed WITHOUT ANY WARRANTY. Read the associated
 * COPYING file for information.
 *--------------------------------------------------------------------------*/

#ifndef __gaol_core_math_port_h__
#define __gaol_core_math_port_h__

#define cr_sinh gaol_cr_sinh
#define cr_cosh gaol_cr_cosh
#define cr_tanh gaol_cr_tanh
#define cr_asinh gaol_cr_asinh
#define cr_acosh gaol_cr_acosh
#define cr_atanh gaol_cr_atanh

#include "gaol/gaol_core_math.h"

#if defined(__GNUC__) || defined(__clang__)
#  pragma GCC diagnostic ignored "-Wconversion"
#  pragma GCC diagnostic ignored "-Wsign-conversion"
#endif

#if defined(_MSC_VER) && !defined(__clang__)
#  pragma warning(disable: 4244 4267 4146 4305)
#  include <math.h>
#  define __builtin_expect(x, y) (x)
#  define __builtin_fma(x, y, z) fma(x, y, z)
#  define __builtin_fabs(x) fabs(x)
#  define __builtin_copysign(x, y) copysign(x, y)
#  define __builtin_sqrt(x) sqrt(x)
#  define __attribute__(x)
#endif

#endif /* __gaol_core_math_port_h__ */
