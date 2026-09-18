/*-*-C-*---------------------------------------------------------------------
 * gaol -- NOT Just Another Interval Library
 *--------------------------------------------------------------------------
 * The hyperbolic functions of CORE-MATH (https://core-math.gitlabpages.inria.fr,
 * MIT licence, see 3rd/core-math), correctly rounded in double precision:
 * the double they return is the one the rounding direction in effect gives
 * of the exact value. GAOL bounds sinh, cosh, tanh, asinh, acosh and atanh
 * with them (fork of GAOL, issue #1): neither mathlib nor CRlibm has them all,
 * and the ones of the math library of the system, which GAOL took, are not
 * accurate enough on every system for their values moved outward to be
 * bounds. GAOL bounds log with CORE-MATH's too, where the compiler has a
 * 128-bit integer type, which its accurate phase computes with
 * (GAOL_CORE_MATH_LOG: __int128 of GCC and Clang for 64-bit targets), and
 * with mathlib's elsewhere (Visual C++, 32-bit targets): correctly rounded in
 * the upward rounding GAOL computes in, it gives the tightest bounds without
 * switching the rounding direction, twice as fast as mathlib's log moved
 * outward (fork of GAOL).
 *
 * Their sources are gaol/core_math_*.c, compiled into GAOL's library under
 * the names below (gaol/core_math_port.h).
 *--------------------------------------------------------------------------
 * gaol is a software distributed WITHOUT ANY WARRANTY. Read the associated
 * COPYING file for information.
 *--------------------------------------------------------------------------*/

#ifndef __gaol_core_math_h__
#define __gaol_core_math_h__

#if defined(__GNUC__) || defined(__clang__)
#  define GAOL_CORE_MATH_PUBLIC __attribute__ ((visibility("default")))
#else
#  define GAOL_CORE_MATH_PUBLIC
#endif

#if defined(__SIZEOF_INT128__)
#  define GAOL_CORE_MATH_LOG 1
#else
#  define GAOL_CORE_MATH_LOG 0
#endif

#ifdef __cplusplus
extern "C" {
#endif

GAOL_CORE_MATH_PUBLIC double gaol_cr_sinh(double x);
GAOL_CORE_MATH_PUBLIC double gaol_cr_cosh(double x);
GAOL_CORE_MATH_PUBLIC double gaol_cr_tanh(double x);
GAOL_CORE_MATH_PUBLIC double gaol_cr_asinh(double x);
GAOL_CORE_MATH_PUBLIC double gaol_cr_acosh(double x);
GAOL_CORE_MATH_PUBLIC double gaol_cr_atanh(double x);
#if GAOL_CORE_MATH_LOG
GAOL_CORE_MATH_PUBLIC double gaol_cr_log(double x);
#endif

#ifdef __cplusplus
}
#endif

#endif /* __gaol_core_math_h__ */
