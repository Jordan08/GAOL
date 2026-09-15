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
# SVN: $Id$
#------------------------------------------------------------------------------

# GAOL's own directories before those configure adds to CPPFLAGS
# (--with-mathlib-include), which come first otherwise: a prefix holding
# mathlib may hold an installed GAOL too, whose headers were then included
# rather than those of the sources
AM_CPPFLAGS = -I$(top_builddir) -I$(top_srcdir)

# The library just built, by its libtool archive, rather than -lgaol with
# -L../gaol/.libs, which came after the -L of --with-mathlib-lib: a prefix
# holding mathlib may hold an installed GAOL too, which was then linked
LDADD = -lm $(top_builddir)/gaol/libgaol.la @MATHLIB@

noinst_PROGRAMS = Goldstein_Price

Goldstein_Price_SOURCES = Goldstein_Price.cpp
