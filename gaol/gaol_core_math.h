/*-*-C-*---------------------------------------------------------------------
 * gaol -- NOT Just Another Interval Library
 *--------------------------------------------------------------------------
 * The elementary functions of CORE-MATH (https://core-math.gitlabpages.inria.fr,
 * MIT licence, see 3rd/math-core), correctly rounded in double precision: the
 * double each returns is the one the rounding direction in effect gives of
 * the exact value.
 *
 * GAOL bounds every one of its elementary functions with them, on every
 * architecture and with every compiler (fork of GAOL): computed in the upward
 * rounding GAOL keeps, they give the tightest bounds without switching the
 * rounding direction, where the functions of mathlib, correctly rounded to
 * nearest only, had to be moved one double outward, and where those of the
 * math library of the system are not accurate enough for that to be a bound.
 *
 * Their sources are those of CORE-MATH, in 3rd/math-core/src/binary64,
 * compiled into GAOL's library under the names below, the three builds
 * including gaol/core_math_port.h in each of them (`-include`, `/FI`).
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

#ifdef __cplusplus
extern "C" {
#endif

GAOL_CORE_MATH_PUBLIC double gaol_cr_exp(double x);
GAOL_CORE_MATH_PUBLIC double gaol_cr_log(double x);
GAOL_CORE_MATH_PUBLIC double gaol_cr_pow(double x, double y);
GAOL_CORE_MATH_PUBLIC double gaol_cr_sin(double x);
GAOL_CORE_MATH_PUBLIC double gaol_cr_cos(double x);
GAOL_CORE_MATH_PUBLIC double gaol_cr_tan(double x);
GAOL_CORE_MATH_PUBLIC double gaol_cr_asin(double x);
GAOL_CORE_MATH_PUBLIC double gaol_cr_acos(double x);
GAOL_CORE_MATH_PUBLIC double gaol_cr_atan(double x);
GAOL_CORE_MATH_PUBLIC double gaol_cr_atan2(double y, double x);
GAOL_CORE_MATH_PUBLIC double gaol_cr_sinh(double x);
GAOL_CORE_MATH_PUBLIC double gaol_cr_cosh(double x);
GAOL_CORE_MATH_PUBLIC double gaol_cr_tanh(double x);
GAOL_CORE_MATH_PUBLIC double gaol_cr_asinh(double x);
GAOL_CORE_MATH_PUBLIC double gaol_cr_acosh(double x);
GAOL_CORE_MATH_PUBLIC double gaol_cr_atanh(double x);
GAOL_CORE_MATH_PUBLIC double gaol_cr_cbrt(double x);

#ifdef __cplusplus
}
#endif

#endif /* __gaol_core_math_h__ */
