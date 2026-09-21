/*-*-C-*---------------------------------------------------------------------
 * gaol -- NOT Just Another Interval Library
 *--------------------------------------------------------------------------
 * The elementary functions of CORE-MATH (https://core-math.gitlabpages.inria.fr,
 * MIT licence, see 3rd/math-core), correctly rounded in double precision: the
 * double each returns is the one the rounding direction in effect gives of
 * the exact value.
 *
 * GAOL bounds every one of its elementary functions with them, on every
 * architecture and with every compiler (GAOL v5): computed in the upward
 * rounding GAOL keeps, they give the tightest bounds without switching the
 * rounding direction.
 *
 * Their sources are those of CORE-MATH, in 3rd/math-core/src/binary64,
 * compiled into GAOL's library under the names below, the three builds
 * including gaol/core_math_port.h in each of them (`-include`, `/FI`).
 *
 * Copyright (c) 2026 ENSTA, France
 *
 * Created 2026-09-20 by Jordan NININ
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
GAOL_CORE_MATH_PUBLIC double gaol_cr_exp2(double x);
GAOL_CORE_MATH_PUBLIC double gaol_cr_exp10(double x);
GAOL_CORE_MATH_PUBLIC double gaol_cr_log2(double x);
GAOL_CORE_MATH_PUBLIC double gaol_cr_log10(double x);
/* The forward functions IEEE 1788-2015 recommends (Table 10.5) (GAOL v5) */
GAOL_CORE_MATH_PUBLIC double gaol_cr_expm1(double x);
GAOL_CORE_MATH_PUBLIC double gaol_cr_exp2m1(double x);
GAOL_CORE_MATH_PUBLIC double gaol_cr_exp10m1(double x);
GAOL_CORE_MATH_PUBLIC double gaol_cr_sinpi(double x);
GAOL_CORE_MATH_PUBLIC double gaol_cr_cospi(double x);
GAOL_CORE_MATH_PUBLIC double gaol_cr_tanpi(double x);
GAOL_CORE_MATH_PUBLIC double gaol_cr_acospi(double x);
GAOL_CORE_MATH_PUBLIC double gaol_cr_atanpi(double x);
GAOL_CORE_MATH_PUBLIC double gaol_cr_log1p(double x);
GAOL_CORE_MATH_PUBLIC double gaol_cr_log2p1(double x);
GAOL_CORE_MATH_PUBLIC double gaol_cr_log10p1(double x);
GAOL_CORE_MATH_PUBLIC double gaol_cr_hypot(double x, double y);
GAOL_CORE_MATH_PUBLIC double gaol_cr_rsqrt(double x);
GAOL_CORE_MATH_PUBLIC double gaol_cr_asinpi(double x);
GAOL_CORE_MATH_PUBLIC double gaol_cr_atan2pi(double y, double x);

#ifdef __cplusplus
}
#endif

#endif /* __gaol_core_math_h__ */
