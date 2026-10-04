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
# By: Frederic Goualard <Frederic.Goualard@lina.univ-nantes.fr>
#------------------------------------------------------------------------------

.PHONY: tags

lib_LTLIBRARIES = libgaol.la

# No MAINTAINERCLEANFILES: make maintainer-clean deleted the lexer and the
# parser, which are committed and which no build regenerates (GAOL v5)
DISTCLEANFILES = *.~* *.obj TAGS

# GAOL's own directories before those given in CPPFLAGS, which come first
# otherwise: a prefix given there may hold an installed GAOL too, whose headers
# were then included rather than those of the sources
AM_CPPFLAGS = -I$(top_builddir) -I$(top_srcdir)
# GAOL in C++11, as with CMake and meson (see configure.ac)
AM_CXXFLAGS = $(GAOL_CXX_STD)

libgaol_la_SOURCES = \
	gaol_interval.cpp gaol_profile.cpp gaol_common.cpp \
	gaol_parser.cpp gaol_expression.cpp \
	gaol_interval_parser.cpp gaol_interval_lexer.cpp gaol_port.cpp \
	gaol_exceptions.cpp gaol_init_cleanup.cpp s_nextafter.c \
    gaol_exact.c


# The lexer and the parser, gaol_interval_lexer.cpp, gaol_interval_parser.cpp
# and gaol_interval_parser.h, are committed and compiled as the other sources:
# make parser (../Makefile.am) regenerates them from gaol_interval_lexer.lpp
# and gaol_interval_parser.ypp with regenerate_parser.sh. The rules .lpp.cpp and
# .ypp.cpp, which ran flex and bison where make found the sources newer, are
# gone (GAOL v5).

# The elementary functions of CORE-MATH, compiled by 3rd/Makefile.am into a
# convenience library and put into libgaol here: nothing is linked along with
# GAOL.

# libgaol.so.5.0.0 and its soname libgaol.so.5 (see configure.ac)
libgaol_la_LDFLAGS = -version-info $(LT_VERSION_INFO)
libgaol_la_LIBADD = $(top_builddir)/3rd/libgaolcoremath.la -lm

library_includedir=$(includedir)/gaol

# The headers a program includes, gaol, gaol_expression.h and
# gaol_expr_eval.h for the expressions, gaol_assert.h for GAOL_ASSERT (see the
# manual), and those they include,
# gaol_fpu_msvc.h for Visual C++ among them: CMake and meson install the same
# ones (GAOL v5)
library_include_HEADERS = gaol gaol.h gaol_allocator.h gaol_assert.h \
	gaol_common.h gaol_config.h gaol_eval_stack.h gaol_exceptions.h \
	gaol_expr_eval.h gaol_expr_visitor.h gaol_expression.h gaol_flags.h \
	gaol_fpu.h gaol_fpu_fenv.h gaol_fpu_msvc.h gaol_ieee1788.h \
	gaol_interval.h gaol_interval_fpu.h gaol_interval_sse.h gaol_limits.h \
	gaol_parser.h gaol_port.h gaol_profile.h gaol_roundeven.h \
	gaol_version.h gaol_configuration.h

# Headers of the sources of GAOL, not installed (GAOL v5):
# gaol_interval_parser.h, which did not compile when included, the parts of the
# library, the bounds at doubles, *_dn() and *_up(), which are not part of its
# interface, and the intervals of floats, which gaol_interval.cpp includes
# where a developer of GAOL defines GAOL_FLOAT_INTERVALS (see gaol_config.h)
noinst_HEADERS = gaol_core_math.h gaol_exact.h \
	gaol_init_cleanup.h gaol_interval_parser.h gaol_parameters.h \
	gaol_u128.h sysdeps/gaol_exact_c99.h sysdeps/gaol_exact_msvc.h \
	gaol_double_op.h gaol_intervalf.h gaol_interval2f.h

LDADD = -lm

tags: TAGS
	etags --language=c++ *.cpp *.h *.c

TAGS: $(gaol_HEADERS) $(libgaol_la_SOURCES)

install-data-local:
	$(MKDIR_P) $(DESTDIR)$(libdir)/pkgconfig
	$(INSTALL_DATA) ../gaol.pc $(DESTDIR)$(libdir)/pkgconfig/gaol.pc


# make distclean removes the files of the dependencies one by one and leaves
# their directories: removed here, so that it gives the sources back as git has
# them (GAOL v5)
distclean-local:
	-rm -rf ./$(DEPDIR)
