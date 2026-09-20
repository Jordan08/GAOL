#-*-Makefile-*-----------------------------------------------------------------
# The third-party sources, for the autotools build of GAOL
#------------------------------------------------------------------------------
# The sixteen sources of CORE-MATH (3rd/math-core) GAOL bounds its elementary
# functions with, compiled into a convenience library that gaol/Makefile.am
# puts into libgaol: nothing is installed from here.
#
# They are compiled here rather than from gaol/Makefile.am so that libtool
# never sees a source under "../", which it does not place in .libs (the link
# of the shared library then stopped on "no such file: ../3rd/.../.libs/*.o").
# CMake and meson compile them apart too, which keeps the interprocedural
# optimization of GAOL's own sources away from them (see doc/three-builds.md).
#
# gaol/core_math_port.h is included first in each of them, by the compiler
# rather than by the source, so that importing a newer CORE-MATH stays a plain
# copy. See README.md.
#------------------------------------------------------------------------------

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
	$(CORE_MATH_DIR)/acosh/acosh.c $(CORE_MATH_DIR)/atanh/atanh.c

libgaolcoremath_la_CPPFLAGS = -I$(top_builddir) -I$(top_srcdir)
# -fno-lto keeps the interprocedural optimization away from these sources, where
# CFLAGS carries -flto: reached by it, sin and cos took half as long again, while
# it makes GAOL's own operations twice as fast (see doc/three-builds.md). The
# flag is given where the compiler takes it, which configure tested
# (CORE_MATH_NO_LTO).
libgaolcoremath_la_CFLAGS = $(AM_CFLAGS) @CORE_MATH_NO_LTO@ -include $(top_srcdir)/gaol/core_math_port.h

EXTRA_DIST = README.md math-core
