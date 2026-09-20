#-*-Makefile-*-----------------------------------------------------------------
#  gaol -- Just Another Interval Library
#------------------------------------------------------------------------------
# This file is part of the gaol distribution. Gaol was primarily
# developed at the Swiss Federal Institute of Technology, Lausanne,
# Switzerland, and is now developed at the Institut de Recherche
# en Informatique de Nantes, France.
#
# Copyright (c) 2001 Swiss Federal Institute of Technology, Switzerland
# Copyright (c) 2002-2006 Laboratoire d'Informatique de Nantes-Atlantique, France
#------------------------------------------------------------------------------
# gaol is a software distributed WITHOUT ANY WARRANTY. Read the associated
# COPYING file for information.
#------------------------------------------------------------------------------
# Note: To be used with GNU Make
# If this file has the name "Makefile.in" then it is a template for a
# Makefile; to generate the actual Makefile, run "./configure" from the base
# directory of gaol.
#------------------------------------------------------------------------------
# CVS: $Id: Makefile.am,v 1.22 2007/09/12 12:41:57 goualard Exp $
# Last modified: Wed Sep 27 18:12:40 2006 on pc-goualard.lina.sciences.univ-nantes.prive
# By: Frederic Goualard <Frederic.Goualard@lina.univ-nantes.fr>
#------------------------------------------------------------------------------

.PHONY: tags

SUFFIXES = .lpp .ypp

EXTRA_DIST = sysdeps/*.h stamp-h.in s_nextafter.c core_math_port.h \
	gaol_interval_lexer.lpp gaol_interval_parser.ypp dllmain.cpp gaol_config_mingw.h \
	gaol_config_msvc.h gaol_version_msvc.h gaol_configuration.h.in gaol_interval_sse.cpp


lib_LTLIBRARIES = libgaol.la

DISTCLEANFILES = *.~* *.obj TAGS
MAINTAINERCLEANFILES = *.~* *.obj gaol_interval_lexer.cpp gaol_interval_parser.cpp \
	gaol_interval_parser.h TAGS

if FLOAT_INTERVALS_COND
FLOAT_INTERVALS_SOURCE=gaol_intervalf.cpp
FLOAT_INTERVALS_HEADER=gaol_intervalf.h
else
FLOAT_INTERVALS_SOURCE=
FLOAT_INTERVALS_HEADER=
endif

if COMPILE_INTERVAL2
INTERVAL2_SOURCE=gaol_interval2f.cpp
INTERVAL2_HEADER=gaol_interval2f.h
else
INTERVAL2_SOURCE=
INTERVAL2_HEADER=
endif

# GAOL's own directories before those configure adds to CPPFLAGS
# (--with-mathlib-include), which come first otherwise: a prefix holding
# mathlib may hold an installed GAOL too, whose headers were then included
# rather than those of the sources
AM_CPPFLAGS = -I$(top_builddir) -I$(top_srcdir)

libgaol_la_SOURCES = \
	gaol_interval.cpp gaol_profile.cpp gaol_common.cpp \
	gaol_parser.cpp gaol_expression.cpp \
	gaol_interval_parser.cpp gaol_interval_lexer.cpp gaol_port.cpp \
	gaol_exceptions.cpp gaol_init_cleanup.cpp s_nextafter.c \
    gaol_exact.c \
	$(CORE_MATH_SOURCES) \
    $(FLOAT_INTERVALS_SOURCE) $(INTERVAL2_SOURCE)


YFLAGS = -d -p gaol_
LFLAGS = -Pgaol_
# The elementary functions of CORE-MATH (3rd/math-core), compiled into libgaol
# itself: nothing is linked along with GAOL. Each source is compiled with
# gaol/core_math_port.h included first, by the compiler rather than by the
# source, so that importing a newer CORE-MATH stays a plain copy.
CORE_MATH_DIR = $(top_srcdir)/3rd/math-core/src/binary64
CORE_MATH_SOURCES = \
	$(CORE_MATH_DIR)/exp/exp.c $(CORE_MATH_DIR)/log/log.c \
	$(CORE_MATH_DIR)/pow/pow.c $(CORE_MATH_DIR)/sin/sin.c \
	$(CORE_MATH_DIR)/cos/cos.c $(CORE_MATH_DIR)/tan/tan.c \
	$(CORE_MATH_DIR)/asin/asin.c $(CORE_MATH_DIR)/acos/acos.c \
	$(CORE_MATH_DIR)/atan/atan.c $(CORE_MATH_DIR)/atan2/atan2.c \
	$(CORE_MATH_DIR)/sinh/sinh.c $(CORE_MATH_DIR)/cosh/cosh.c \
	$(CORE_MATH_DIR)/tanh/tanh.c $(CORE_MATH_DIR)/asinh/asinh.c \
	$(CORE_MATH_DIR)/acosh/acosh.c $(CORE_MATH_DIR)/atanh/atanh.c
libgaol_la_CFLAGS = $(AM_CFLAGS) -include $(top_srcdir)/gaol/core_math_port.h

libgaol_la_LDFLAGS = -version-info $(LT_VERSION_INFO) -release $(LT_RELEASE)
libgaol_la_LIBADD = -lm

library_includedir=$(includedir)/gaol

library_include_HEADERS = gaol gaol_assert.h gaol_exact.h gaol_core_math.h gaol_interval.h gaol_common.h gaol_exceptions.h \
	gaol_interval_parser.h gaol_config.h gaol_config_mingw.h gaol_config_msvc.h gaol_version_msvc.h \
	gaol_configuration.h gaol_expression.h gaol_limits.h \
	gaol_expr_eval.h  gaol_expr_visitor.h gaol_parameters.h \
	gaol_flags.h gaol_parser.h gaol_double_op.h gaol_u128.h \
	gaol_port.h gaol_fpu.h gaol_fpu_fenv.h gaol_profile.h gaol.h \
	gaol_eval_stack.h gaol_init_cleanup.h gaol_version.h gaol_allocator.h \
	gaol_interval_fpu.h gaol_interval_sse.h $(FLOAT_INTERVALS_HEADER) $(INTERVAL2_HEADER)

LDADD = -lm

tags: TAGS
	etags --language=c++ *.cpp *.h *.c

TAGS: $(gaol_HEADERS) $(libgaol_la_SOURCES)

.lpp.cpp:
	$(LEX) $(AM_LFLAGS) $(LFLAGS) $< && mv lex.gaol_.c $@

.ypp.cpp:
	$(YACC) $(AM_YFLAGS) $(YFLAGS) $< && mv y.tab.c $*.cpp
	if test -f y.tab.h; then \
	if cmp -s y.tab.h $*.h; then rm -f y.tab.h; else mv y.tab.h $*.h; fi; \
	else :; fi


install-data-local:
	$(mkinstalldirs) $(DESTDIR)$(includedir)/gaol/sysdeps
	$(INSTALL_DATA) $(srcdir)/sysdeps/*.h $(DESTDIR)$(includedir)/gaol/sysdeps
	$(mkinstalldirs) $(DESTDIR)$(libdir)/pkgconfig
	$(INSTALL_DATA) ../gaol.pc $(DESTDIR)$(libdir)/pkgconfig/gaol.pc

