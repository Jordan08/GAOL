# The unit tests of GAOL (GAOL v5), as tests/CMakeLists.txt builds them
#
# ./configure --with-tests builds them with GAOL (make), and make check and
# make test run them (see ../Makefile.am). They are compiled as any code using
# GAOL, with the flags of interval arithmetic configure gives, in C++17 for the
# hexadecimal floating literals of elementary_values.h, and linked with the
# static libgaol just built (-static: a libtool library not installed), as
# CMake links them. Exit status 77 is a test skipped (intervalf and interval2f
# where the intervals of floats are not compiled).
#
# make perf measures GAOL v5 on the benchmark of doc/compare/performance.md
# (doc/compare/code/run_perf.sh).
#
# Copyright (c) 2026 ENSTA, France
#
# Created 2026-09-27 by Jordan NININ

AM_CPPFLAGS = -I$(top_builddir) -I$(top_srcdir)
AM_CXXFLAGS = -std=c++17
AM_LDFLAGS = -static
LDADD = $(top_builddir)/gaol/libgaol.la -lm
AM_DEFAULT_SOURCE_EXT = .cpp

# The same list as tests/CMakeLists.txt
GAOL_TESTS = arithmetic elementary rounding_direction numbers other_functions \
	ieee1788 reverse u128 core_math expressions extended_precision \
	static_initialization debugging \
	arithmetic_operators assignment constants constructor float_functions \
	generic_functions input_output interval_functions misc non_arithmetic \
	relations reverse_mappings trigonometric intervalf interval2f
# The rounding direction after the automatic cleanup of GAOL, read by a
# function registered in a constructor of priority 101, which ELF runs before
# GAOL initializes itself
if HOST_LINUX
GAOL_TESTS += automatic_cleanup
endif

if WITH_TESTS
noinst_PROGRAMS = $(GAOL_TESTS) gaol_performance
TESTS = $(GAOL_TESTS)
else
check-local:
	@echo "No unit tests: configure with --with-tests"
endif

gaol_performance_SOURCES = performance.cpp

# The benchmark of doc/compare/performance.md, compiled as run_bench.sh
# compiles it (C++11) and linked with the static libgaol, as setup.sh builds
# GAOL v5 for it
gaol_bench$(EXEEXT): $(top_srcdir)/doc/compare/code/bench_gaol.cpp $(top_builddir)/gaol/libgaol.la
	$(LIBTOOL) --tag=CXX --mode=link $(CXX) $(DEFS) $(AM_CPPFLAGS) $(CPPFLAGS) \
	  -I$(top_srcdir)/doc/compare/code -DGAOL_BENCH_NAME='"gaol5"' $(GAOL_CXX_STD) $(CXXFLAGS) \
	  $(LDFLAGS) -static $(top_srcdir)/doc/compare/code/bench_gaol.cpp $(top_builddir)/gaol/libgaol.la -lm -o $@
CLEANFILES = gaol_bench$(EXEEXT)

perf: gaol_bench$(EXEEXT)
	$(BASH) $(top_srcdir)/doc/compare/code/run_perf.sh ./gaol_bench$(EXEEXT) autotools "$(CXX)"

BASH = bash

.PHONY: perf
