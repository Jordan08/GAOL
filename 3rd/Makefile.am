#-*-Makefile-*-----------------------------------------------------------------
# The third-party sources, for the autotools build of GAOL
#------------------------------------------------------------------------------
# The thirty-eight sources of CORE-MATH (3rd/math-core) GAOL bounds its
# elementary functions with, compiled into a convenience library that
# gaol/Makefile.am puts into libgaol: nothing is installed from here.
#
# They are compiled here rather than from gaol/Makefile.am so that libtool
# never sees a source under "../", which it does not place in .libs (the link
# of the shared library then stopped on "no such file: ../3rd/.../.libs/*.o").
# CMake and meson compile them apart too, which keeps the interprocedural
# optimization of GAOL's own sources away from them (see doc/three-builds.md).
#
# gaol/math/gaol_core_math_port.h is included first in each of them, by the
# compiler rather than by the source, so that importing a newer CORE-MATH stays
# a plain copy. See README.md.
#------------------------------------------------------------------------------
#
# Copyright (c) 2026 ENSTA, France
#
# By: Jordan NININ (created 2026-09-20)
#
# GAOL is a software distributed WITHOUT ANY WARRANTY. Read the associated
# COPYING.LIB file for information.

noinst_LTLIBRARIES = libgaolcoremath.la

CORE_MATH_DIR = math-core/src/binary64

libgaolcoremath_la_SOURCES = \
	$(CORE_MATH_DIR)/exp/exp.c $(CORE_MATH_DIR)/log/log.c \
	$(CORE_MATH_DIR)/pow/pow.c $(CORE_MATH_DIR)/sin/sin.c \
	$(CORE_MATH_DIR)/cos/cos.c $(CORE_MATH_DIR)/tan/tan.c \
	$(CORE_MATH_DIR)/asin/asin.c $(CORE_MATH_DIR)/acos/acos.c \
	$(CORE_MATH_DIR)/atan/atan.c $(CORE_MATH_DIR)/atan2/atan2.c \
	$(CORE_MATH_DIR)/sinh/sinh.c $(CORE_MATH_DIR)/cosh/cosh.c \
	$(CORE_MATH_DIR)/tanh/tanh.c $(CORE_MATH_DIR)/asinh/asinh.c \
	$(CORE_MATH_DIR)/acosh/acosh.c $(CORE_MATH_DIR)/atanh/atanh.c \
	$(CORE_MATH_DIR)/cbrt/cbrt.c \
	$(CORE_MATH_DIR)/exp2/exp2.c $(CORE_MATH_DIR)/exp10/exp10.c \
	$(CORE_MATH_DIR)/log2/log2.c $(CORE_MATH_DIR)/log10/log10.c \
	$(CORE_MATH_DIR)/expm1/expm1.c $(CORE_MATH_DIR)/exp2m1/exp2m1.c \
	$(CORE_MATH_DIR)/exp10m1/exp10m1.c $(CORE_MATH_DIR)/sinpi/sinpi.c \
	$(CORE_MATH_DIR)/cospi/cospi.c $(CORE_MATH_DIR)/tanpi/tanpi.c \
	$(CORE_MATH_DIR)/acospi/acospi.c $(CORE_MATH_DIR)/atanpi/atanpi.c \
	$(CORE_MATH_DIR)/log1p/log1p.c $(CORE_MATH_DIR)/log2p1/log2p1.c \
	$(CORE_MATH_DIR)/log10p1/log10p1.c $(CORE_MATH_DIR)/hypot/hypot.c \
	$(CORE_MATH_DIR)/rsqrt/rsqrt.c $(CORE_MATH_DIR)/asinpi/asinpi.c \
	$(CORE_MATH_DIR)/atan2pi/atan2pi.c \
	$(CORE_MATH_DIR)/erf/erf.c $(CORE_MATH_DIR)/erfc/erfc.c

libgaolcoremath_la_CPPFLAGS = -I$(top_builddir) -I$(top_srcdir)
# -fno-lto, for the interprocedural optimization: reached by it, sin and cos
# took half as long again, while it makes GAOL's own operations faster (see
# doc/three-builds.md). It comes before CFLAGS on the command line, and the
# last of the two wins: a -flto in CFLAGS reaches these sources all the same
# (GAOL v5). The flag is given where the compiler takes it, which configure
# tested (CORE_MATH_NO_LTO).
libgaolcoremath_la_CFLAGS = $(AM_CFLAGS) @GAOL_C_STD@ @CORE_MATH_NO_LTO@ @CORE_MATH_EXCESS_PRECISION@ @GAOL_ALIGN_CFLAGS@ -include $(top_srcdir)/gaol/math/gaol_core_math_port.h


# make distclean removes the files of the dependencies one by one and leaves
# their directories: removed here, so that it gives the sources back as git has
# them (GAOL v5)
distclean-local:
	-rm -rf $(CORE_MATH_DIR)/*/$(DEPDIR)

# The archive of the sources, make dist (GAOL v5): automake takes the sources,
# the headers and the scripts it knows from the variables of the Makefile.am,
# and EXTRA_DIST names the other files of the repository, here CORE-MATH as GAOL takes it, the sources that libgaolcoremath does not compile among them (its tests, its tools, its documentation).
# make dist, package_source of CMake and meson dist hold the same files, those
# of the repository but the notes of the work on GAOL v5 of the branch dev: a
# file added to the repository is added to this list too, or the job Archives
# of the sources of the CI, which compares the three archives, fails
EXTRA_DIST = \
	math-core/.gitignore \
	math-core/.gitlab-ci.yml \
	math-core/LICENSE \
	math-core/README.md \
	math-core/check.sh \
	math-core/ci.sh \
	math-core/ci/00-prepare-docker.sh \
	math-core/ci/01-prepare-llvm.sh \
	math-core/ci/02-prepare-rlibm.sh \
	math-core/ci/int128test.c \
	math-core/ci/ldbl80test.c \
	math-core/ci/llvm_test.sh \
	math-core/ci/perf_test.sh \
	math-core/ci/quadmath_test.c \
	math-core/ci/rescale.sh \
	math-core/ci/rlibm.patch \
	math-core/ci/rlibm_test.c \
	math-core/ci/rlibm_test1.sh \
	math-core/ci/rlibm_test2.sh \
	math-core/ci/rlibm_test3.sh \
	math-core/ci/rlibm_test4.sh \
	math-core/ci/rlibm_test5.sh \
	math-core/ci/rlibm_test6.sh \
	math-core/ci/rlibm_wrapper.c \
	math-core/doc/Fortran.sh \
	math-core/doc/bacsel.txt \
	math-core/doc/my_j0.c \
	math-core/doc/rminimax.txt \
	math-core/doc/sage.txt \
	math-core/doc/sollya.txt \
	math-core/perf-all.sh \
	math-core/perf.sh \
	math-core/prof.sh \
	math-core/src/binary64/Makefile \
	math-core/src/binary64/acos/Makefile \
	math-core/src/binary64/acos/acos_mpfr.c \
	math-core/src/binary64/acos/check_special.c \
	math-core/src/binary64/acos/function_under_test.h \
	math-core/src/binary64/acosh/Makefile \
	math-core/src/binary64/acosh/acosh_mpfr.c \
	math-core/src/binary64/acosh/check_special.c \
	math-core/src/binary64/acosh/function_under_test.h \
	math-core/src/binary64/acospi/Makefile \
	math-core/src/binary64/acospi/acos.sollya \
	math-core/src/binary64/acospi/acos2.sollya \
	math-core/src/binary64/acospi/acos2_0.sollya \
	math-core/src/binary64/acospi/acos3.sollya \
	math-core/src/binary64/acospi/acos_sqrt.sollya \
	math-core/src/binary64/acospi/acospi.sage \
	math-core/src/binary64/acospi/acospi_mpfr.c \
	math-core/src/binary64/acospi/check_special.c \
	math-core/src/binary64/acospi/doit.sh \
	math-core/src/binary64/acospi/doit2.sh \
	math-core/src/binary64/acospi/doit2_0.sh \
	math-core/src/binary64/acospi/doit3.sh \
	math-core/src/binary64/acospi/doit_sqrt.sh \
	math-core/src/binary64/acospi/function_under_test.h \
	math-core/src/binary64/asin/Makefile \
	math-core/src/binary64/asin/asin_mpfr.c \
	math-core/src/binary64/asin/check_special.c \
	math-core/src/binary64/asin/function_under_test.h \
	math-core/src/binary64/asinh/Makefile \
	math-core/src/binary64/asinh/asinh.sollya \
	math-core/src/binary64/asinh/asinh_mpfr.c \
	math-core/src/binary64/asinh/check_special.c \
	math-core/src/binary64/asinh/function_under_test.h \
	math-core/src/binary64/asinpi/Makefile \
	math-core/src/binary64/asinpi/asinpi.sage \
	math-core/src/binary64/asinpi/asinpi_mpfr.c \
	math-core/src/binary64/asinpi/check_special.c \
	math-core/src/binary64/asinpi/function_under_test.h \
	math-core/src/binary64/atan/Makefile \
	math-core/src/binary64/atan/atan_mpfr.c \
	math-core/src/binary64/atan/check_special.c \
	math-core/src/binary64/atan/function_under_test.h \
	math-core/src/binary64/atan2/Makefile \
	math-core/src/binary64/atan2/atan2.sage \
	math-core/src/binary64/atan2/atan2.sollya \
	math-core/src/binary64/atan2/atan2_0.sollya \
	math-core/src/binary64/atan2/atan2_mpfr.c \
	math-core/src/binary64/atan2/atan2_small.sollya \
	math-core/src/binary64/atan2/check_special.c \
	math-core/src/binary64/atan2/doit.sh \
	math-core/src/binary64/atan2/function_under_test.h \
	math-core/src/binary64/atan2/tint.h \
	math-core/src/binary64/atan2/worst.c \
	math-core/src/binary64/atan2pi/Makefile \
	math-core/src/binary64/atan2pi/atan2.sollya \
	math-core/src/binary64/atan2pi/atan2pi.sage \
	math-core/src/binary64/atan2pi/atan2pi_mpfr.c \
	math-core/src/binary64/atan2pi/check_special.c \
	math-core/src/binary64/atan2pi/doit.sh \
	math-core/src/binary64/atan2pi/function_under_test.h \
	math-core/src/binary64/atan2pi/tint.h \
	math-core/src/binary64/atan2pi/worst.c \
	math-core/src/binary64/atanh/Makefile \
	math-core/src/binary64/atanh/atanh.sollya \
	math-core/src/binary64/atanh/atanh_mpfr.c \
	math-core/src/binary64/atanh/check_special.c \
	math-core/src/binary64/atanh/function_under_test.h \
	math-core/src/binary64/atanpi/Makefile \
	math-core/src/binary64/atanpi/atanpi.sage \
	math-core/src/binary64/atanpi/atanpi_mpfr.c \
	math-core/src/binary64/atanpi/check_special.c \
	math-core/src/binary64/atanpi/function_under_test.h \
	math-core/src/binary64/cbrt/Makefile \
	math-core/src/binary64/cbrt/cbrt_mpfr.c \
	math-core/src/binary64/cbrt/check_special.c \
	math-core/src/binary64/cbrt/function_under_test.h \
	math-core/src/binary64/cos/Makefile \
	math-core/src/binary64/cos/check_special.c \
	math-core/src/binary64/cos/cos_mpfr.c \
	math-core/src/binary64/cos/function_under_test.h \
	math-core/src/binary64/cosh/Makefile \
	math-core/src/binary64/cosh/check_special.c \
	math-core/src/binary64/cosh/cosh_mpfr.c \
	math-core/src/binary64/cosh/function_under_test.h \
	math-core/src/binary64/cospi/Makefile \
	math-core/src/binary64/cospi/check_special.c \
	math-core/src/binary64/cospi/cospi.sage \
	math-core/src/binary64/cospi/cospi_mpfr.c \
	math-core/src/binary64/cospi/function_under_test.h \
	math-core/src/binary64/erf/Makefile \
	math-core/src/binary64/erf/check_special.c \
	math-core/src/binary64/erf/erf.sage \
	math-core/src/binary64/erf/erf.sollya \
	math-core/src/binary64/erf/erf0.sollya \
	math-core/src/binary64/erf/erf0_accurate.sollya \
	math-core/src/binary64/erf/erf_mpfr.c \
	math-core/src/binary64/erf/function_under_test.h \
	math-core/src/binary64/erfc/Makefile \
	math-core/src/binary64/erfc/asympt0.sollya \
	math-core/src/binary64/erfc/asympt1.sollya \
	math-core/src/binary64/erfc/asympt2.sollya \
	math-core/src/binary64/erfc/asympt3.sollya \
	math-core/src/binary64/erfc/asympt4.sollya \
	math-core/src/binary64/erfc/asympt5.sollya \
	math-core/src/binary64/erfc/asympt_acc0.sollya \
	math-core/src/binary64/erfc/asympt_acc1.sollya \
	math-core/src/binary64/erfc/asympt_acc2.sollya \
	math-core/src/binary64/erfc/asympt_acc3.sollya \
	math-core/src/binary64/erfc/asympt_acc4.sollya \
	math-core/src/binary64/erfc/asympt_acc5.sollya \
	math-core/src/binary64/erfc/asympt_acc6.sollya \
	math-core/src/binary64/erfc/asympt_acc7.sollya \
	math-core/src/binary64/erfc/asympt_acc8.sollya \
	math-core/src/binary64/erfc/asympt_acc9.sollya \
	math-core/src/binary64/erfc/check_special.c \
	math-core/src/binary64/erfc/erfc_mpfr.c \
	math-core/src/binary64/erfc/exp_accurate.sollya \
	math-core/src/binary64/erfc/function_under_test.h \
	math-core/src/binary64/exp/Makefile \
	math-core/src/binary64/exp/check_special.c \
	math-core/src/binary64/exp/exp.sage \
	math-core/src/binary64/exp/exp_mpfr.c \
	math-core/src/binary64/exp/function_under_test.h \
	math-core/src/binary64/exp10/Makefile \
	math-core/src/binary64/exp10/check_special.c \
	math-core/src/binary64/exp10/exp10_mpfr.c \
	math-core/src/binary64/exp10/function_under_test.h \
	math-core/src/binary64/exp10m1/Makefile \
	math-core/src/binary64/exp10m1/check_special.c \
	math-core/src/binary64/exp10m1/exp10m1_mpfr.c \
	math-core/src/binary64/exp10m1/function_under_test.h \
	math-core/src/binary64/exp2/Makefile \
	math-core/src/binary64/exp2/check_special.c \
	math-core/src/binary64/exp2/exp2_mpfr.c \
	math-core/src/binary64/exp2/function_under_test.h \
	math-core/src/binary64/exp2m1/Makefile \
	math-core/src/binary64/exp2m1/check_special.c \
	math-core/src/binary64/exp2m1/exp2m1.sage \
	math-core/src/binary64/exp2m1/exp2m1_accurate.sollya \
	math-core/src/binary64/exp2m1/exp2m1_fast.sollya \
	math-core/src/binary64/exp2m1/exp2m1_mpfr.c \
	math-core/src/binary64/exp2m1/function_under_test.h \
	math-core/src/binary64/expm1/Makefile \
	math-core/src/binary64/expm1/check_special.c \
	math-core/src/binary64/expm1/exp_accurate.sollya \
	math-core/src/binary64/expm1/expm1.sage \
	math-core/src/binary64/expm1/expm1_accurate.sollya \
	math-core/src/binary64/expm1/expm1_fast.sollya \
	math-core/src/binary64/expm1/expm1_mpfr.c \
	math-core/src/binary64/expm1/function_under_test.h \
	math-core/src/binary64/hypot/Makefile \
	math-core/src/binary64/hypot/check_special.c \
	math-core/src/binary64/hypot/function_under_test.h \
	math-core/src/binary64/hypot/hypot_mpfr.c \
	math-core/src/binary64/hypot/triples.c \
	math-core/src/binary64/lgamma/Makefile \
	math-core/src/binary64/lgamma/check_special.c \
	math-core/src/binary64/lgamma/function_under_test.h \
	math-core/src/binary64/lgamma/lgamma.c \
	math-core/src/binary64/lgamma/lgamma_mpfr.c \
	math-core/src/binary64/log/Makefile \
	math-core/src/binary64/log/check_special.c \
	math-core/src/binary64/log/dint.h \
	math-core/src/binary64/log/doit.sh \
	math-core/src/binary64/log/function_under_test.h \
	math-core/src/binary64/log/gappa.sage \
	math-core/src/binary64/log/log.sage \
	math-core/src/binary64/log/log1_template.g \
	math-core/src/binary64/log/log_mpfr.c \
	math-core/src/binary64/log/logfast.sollya \
	math-core/src/binary64/log/lognearzero.g \
	math-core/src/binary64/log/lognearzero.sh \
	math-core/src/binary64/log/logtiny.g \
	math-core/src/binary64/log/logtiny.sh \
	math-core/src/binary64/log/logtiny.sollya \
	math-core/src/binary64/log10/Makefile \
	math-core/src/binary64/log10/P.sollya \
	math-core/src/binary64/log10/check_special.c \
	math-core/src/binary64/log10/dint.h \
	math-core/src/binary64/log10/function_under_test.h \
	math-core/src/binary64/log10/log10.sage \
	math-core/src/binary64/log10/log10_mpfr.c \
	math-core/src/binary64/log10p1/Makefile \
	math-core/src/binary64/log10p1/check_special.c \
	math-core/src/binary64/log10p1/dint.h \
	math-core/src/binary64/log10p1/function_under_test.h \
	math-core/src/binary64/log10p1/log10p1_accurate.sollya \
	math-core/src/binary64/log10p1/log10p1_mpfr.c \
	math-core/src/binary64/log1p/Makefile \
	math-core/src/binary64/log1p/P.sollya \
	math-core/src/binary64/log1p/check_special.c \
	math-core/src/binary64/log1p/dint.h \
	math-core/src/binary64/log1p/function_under_test.h \
	math-core/src/binary64/log1p/log1p.sage \
	math-core/src/binary64/log1p/log1p_mpfr.c \
	math-core/src/binary64/log2/Makefile \
	math-core/src/binary64/log2/check_special.c \
	math-core/src/binary64/log2/function_under_test.h \
	math-core/src/binary64/log2/log2.sage \
	math-core/src/binary64/log2/log2_mpfr.c \
	math-core/src/binary64/log2p1/Makefile \
	math-core/src/binary64/log2p1/check_special.c \
	math-core/src/binary64/log2p1/dint_log2p1.h \
	math-core/src/binary64/log2p1/function_under_test.h \
	math-core/src/binary64/log2p1/log2p1.sage \
	math-core/src/binary64/log2p1/log2p1_accurate.sollya \
	math-core/src/binary64/log2p1/log2p1_mpfr.c \
	math-core/src/binary64/pow/Makefile \
	math-core/src/binary64/pow/check_special.c \
	math-core/src/binary64/pow/dint.h \
	math-core/src/binary64/pow/dint.sage \
	math-core/src/binary64/pow/function_under_test.h \
	math-core/src/binary64/pow/pow.h \
	math-core/src/binary64/pow/pow.sage \
	math-core/src/binary64/pow/pow_mpfr.c \
	math-core/src/binary64/pow/qint.h \
	math-core/src/binary64/pow/qint.sage \
	math-core/src/binary64/pow/sollya/P_1.sollya \
	math-core/src/binary64/pow/sollya/Q_1.sollya \
	math-core/src/binary64/pow/sollya/approximations_r2.sollya \
	math-core/src/binary64/pow/sollya/approximations_r3.sollya \
	math-core/src/binary64/rsqrt/Makefile \
	math-core/src/binary64/rsqrt/check_special.c \
	math-core/src/binary64/rsqrt/function_under_test.h \
	math-core/src/binary64/rsqrt/rsqrt_mpfr.c \
	math-core/src/binary64/sin/Makefile \
	math-core/src/binary64/sin/buildSC.c \
	math-core/src/binary64/sin/check_special.c \
	math-core/src/binary64/sin/cos_accurate.sollya \
	math-core/src/binary64/sin/cos_fast.sollya \
	math-core/src/binary64/sin/function_under_test.h \
	math-core/src/binary64/sin/sin.sage \
	math-core/src/binary64/sin/sin_accurate.sollya \
	math-core/src/binary64/sin/sin_fast.sollya \
	math-core/src/binary64/sin/sin_mpfr.c \
	math-core/src/binary64/sin/sinfast.sollya \
	math-core/src/binary64/sincos/Makefile \
	math-core/src/binary64/sincos/check_special.c \
	math-core/src/binary64/sincos/function_under_test.h \
	math-core/src/binary64/sincos/sincos.c \
	math-core/src/binary64/sincos/sincos_mpfr.c \
	math-core/src/binary64/sinh/Makefile \
	math-core/src/binary64/sinh/Pcosh.sollya \
	math-core/src/binary64/sinh/Pcosh2.sollya \
	math-core/src/binary64/sinh/Psinh.sollya \
	math-core/src/binary64/sinh/Psinh2.sollya \
	math-core/src/binary64/sinh/buildu.c \
	math-core/src/binary64/sinh/check_special.c \
	math-core/src/binary64/sinh/function_under_test.h \
	math-core/src/binary64/sinh/sinh.sage \
	math-core/src/binary64/sinh/sinh_mpfr.c \
	math-core/src/binary64/sinpi/Makefile \
	math-core/src/binary64/sinpi/check_special.c \
	math-core/src/binary64/sinpi/function_under_test.h \
	math-core/src/binary64/sinpi/sinpi.sage \
	math-core/src/binary64/sinpi/sinpi_mpfr.c \
	math-core/src/binary64/support/Makefile.bivariate \
	math-core/src/binary64/support/Makefile.sincos \
	math-core/src/binary64/support/Makefile.univariate \
	math-core/src/binary64/support/check_worst.c \
	math-core/src/binary64/support/check_worst_sincos.c \
	math-core/src/binary64/support/check_worst_uni.c \
	math-core/src/binary64/support/fenv_mpfr.h \
	math-core/src/binary64/tan/Makefile \
	math-core/src/binary64/tan/check_special.c \
	math-core/src/binary64/tan/function_under_test.h \
	math-core/src/binary64/tan/tan.sage \
	math-core/src/binary64/tan/tan_mpfr.c \
	math-core/src/binary64/tanh/Makefile \
	math-core/src/binary64/tanh/check_special.c \
	math-core/src/binary64/tanh/function_under_test.h \
	math-core/src/binary64/tanh/tanh.sage \
	math-core/src/binary64/tanh/tanh_mpfr.c \
	math-core/src/binary64/tanpi/Makefile \
	math-core/src/binary64/tanpi/check_special.c \
	math-core/src/binary64/tanpi/function_under_test.h \
	math-core/src/binary64/tanpi/tanpi_mpfr.c \
	math-core/src/binary64/tgamma/Makefile \
	math-core/src/binary64/tgamma/check_special.c \
	math-core/src/binary64/tgamma/function_under_test.h \
	math-core/src/binary64/tgamma/tgamma.c \
	math-core/src/binary64/tgamma/tgamma_mpfr.c
