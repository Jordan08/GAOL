#-*-Makefile-*-----------------------------------------------------------------
#  gaol -- Just Another Interval Library
#------------------------------------------------------------------------------
# This file is part of the gaol distribution. Gaol was primarily 
# developed at the Swiss Federal Institute of Technology, Lausanne, 
# Switzerland, and is now developed at the Institut de Recherche 
# en Informatique de Nantes, France.
#
# Copyright (c) 2001 Swiss Federal Institute of Technology, Switzerland
# Copyright (c) 2002 Institut de Recherche en Informatique de Nantes, France
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


# 3rd/ builds nothing: the sources of CORE-MATH are compiled into libgaol by
# gaol/Makefile.am. No make dist (no-dist in configure.ac): the archives of the
# sources and the packages are made by CMake (see doc/building.md) (GAOL v5);
# DIST_SUBDIRS is what make distclean goes through.
SUBDIRS = 3rd gaol tests examples
DIST_SUBDIRS = 3rd gaol tests examples manual
ACLOCAL_AMFLAGS = -I m4

# Requires automake 1.4
AUTOMAKE_OPTIONS = 1.4

# make check runs the unit tests of tests/ (--with-tests) and the examples of
# examples/ (--with-examples), make test the unit tests only, as with CMake and
# meson (GAOL v5)
test:
	cd tests && $(MAKE) $(AM_MAKEFLAGS) check

# make perf measures GAOL v5 on the benchmark of doc/compare/performance.md and
# writes its times into the tables (see tests/Makefile.am) (GAOL v5)
perf:
	cd tests && $(MAKE) $(AM_MAKEFLAGS) perf

# make parser regenerates the lexer and the parser of the strings,
# gaol/gaol_interval_lexer.cpp, gaol/gaol_interval_parser.cpp and
# gaol/gaol_interval_parser.h, from gaol/gaol_interval_lexer.lpp and
# gaol/gaol_interval_parser.ypp, with the flex and the bison of the system
# (FLEX, BISON). The three files are committed: no build needs flex nor bison
# otherwise (GAOL v5).
parser:
	$(SHELL) $(top_srcdir)/gaol/regenerate_parser.sh

.PHONY: test perf parser
