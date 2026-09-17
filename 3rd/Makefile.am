#-*-Makefile-*-----------------------------------------------------------------
# The mathlib of 3rd/mathlib, for the autotools build of GAOL
#------------------------------------------------------------------------------
# configure builds this directory before gaol/ when GAOL uses the mathlib of
# 3rd/mathlib, which it does unless --with-mathlib-include or
# --with-mathlib-lib says where an installed one is (see configure.ac).
# mathlib/Makefile.am belongs to mathlib's own configure script, which GAOL
# does not run: this file builds libultim as mathlib/CMakeLists.txt does, with
# the flags configure gives GAOL's C code, those of interval arithmetic
# included, and installs it with its headers. See README.md.
#------------------------------------------------------------------------------

lib_LTLIBRARIES = libultim.la

# The version of the library mathlib 2.1.1 builds itself (mathlib/configure.ac)
libultim_la_LDFLAGS = -version-info 1:0:0 -release 2.1

# mathlib_configuration.h, written in this directory
AM_CPPFLAGS = -I$(builddir)
# mathlib's warnings turned off, as in mathlib/CMakeLists.txt: configure
# compiles GAOL's C code with -Wall -Wconversion
libultim_la_CFLAGS = @MATHLIB_CFLAGS@

# The sources of libultim, as listed in mathlib/src/Makefile.am
libultim_la_SOURCES = \
	mathlib/src/atnat2.c mathlib/src/halfulp.c mathlib/src/mplog.c \
	mathlib/src/slowexp2.c mathlib/src/ucot.c mathlib/src/upow.c \
	mathlib/src/atnat.c mathlib/src/mpa.c mathlib/src/mpsqrt.c \
	mathlib/src/slowexp.c mathlib/src/uexp2.c mathlib/src/urem.c \
	mathlib/src/branred.c mathlib/src/mpatan2.c mathlib/src/mptan.c \
	mathlib/src/slowlog2.c mathlib/src/uexp.c mathlib/src/uroot.c \
	mathlib/src/doasin.c mathlib/src/mpatan.c mathlib/src/sincos32.c \
	mathlib/src/slowpow.c mathlib/src/ulog2.c mathlib/src/usncs.c \
	mathlib/src/dosincos.c mathlib/src/mpexp.c mathlib/src/slowcot.c \
	mathlib/src/uasncs.c mathlib/src/ulog.c mathlib/src/utan.c \
	mathlib/src/DPChange.c

include_HEADERS = mathlib/src/MathLib.h mathlib/src/mathlib_config.h \
	mathlib/src/mathlib_config_msvc.h mathlib/src/mathlib_config_mingw.h
nodist_include_HEADERS = mathlib_configuration.h

BUILT_SOURCES = mathlib_configuration.h
CLEANFILES = mathlib_configuration.h

mathlib_configuration.h: $(srcdir)/mathlib/mathlib_configuration.h.in
	cp $(srcdir)/mathlib/mathlib_configuration.h.in $@

# The whole of mathlib's sources, without what an in-tree build leaves there
EXTRA_DIST = README.md mathlib
dist-hook:
	cd $(distdir)/mathlib && rm -rf `find . \( -name '*.o' -o -name '*.lo' \
	  -o -name .deps -o -name .libs -o -name .dirstamp \) -prune -print`
