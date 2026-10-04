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

# The examples of GAOL v5 (see examples.md), as examples/CMakeLists.txt builds
# them: ./configure --with-examples builds them with GAOL (make), and make
# check runs them, each checking what it prints (see ../Makefile.am). In C++17,
# with the threads of 13_rounding_environment (PTHREAD_FLAGS) (GAOL v5).

# GAOL's own directories before those given in CPPFLAGS, which come first
# otherwise: a prefix given there may hold an installed GAOL too, whose headers
# were then included rather than those of the sources
AM_CPPFLAGS = -I$(top_builddir) -I$(top_srcdir)
AM_CXXFLAGS = -std=c++17 $(PTHREAD_FLAGS)
AM_LDFLAGS = $(PTHREAD_FLAGS)
AM_DEFAULT_SOURCE_EXT = .cpp

# The library just built, by its libtool archive, rather than -lgaol with
# -L../gaol/.libs, which came after the -L given in LDFLAGS: a prefix given
# there may hold an installed GAOL too, which was then linked. With the link
# option gaol.pc gives (-mno-daz-ftz, where the compiler takes it), as the
# examples of CMake and meson get it from gaol::gaol and gaol_dep (GAOL v5)
LDADD = $(top_builddir)/gaol/libgaol.la -lm $(GAOL_PC_LIBS)

# The same list as examples/CMakeLists.txt
GAOL_EXAMPLES = 01_first_steps 02_decimals_and_rump 03_dependency_problem \
	04_centered_form 05_interval_newton 06_global_optimization 07_contractors \
	08_sivia 09_parameter_estimation 10_krawczyk 11_ode_enclosure \
	12_affine_arithmetic 13_rounding_environment 14_generic_programming \
	15_text_and_ieee1788 16_Goldstein_Price

if WITH_EXAMPLES
noinst_PROGRAMS = $(GAOL_EXAMPLES)
TESTS = $(GAOL_EXAMPLES)
endif

# make distclean removes the files of the dependencies one by one and leaves
# their directories: removed here, so that it gives the sources back as git has
# them (GAOL v5)
distclean-local:
	-rm -rf ./$(DEPDIR)
