# The unit tests of GAOL (GAOL v5), as tests/CMakeLists.txt builds them
#
# ./configure --with-tests builds them with GAOL (make), and make check and
# make test run them (see ../Makefile.am). They are compiled as any code using
# GAOL, with the flags of interval arithmetic configure gives, in C++17 for the
# hexadecimal floating literals of elementary_values.h, and linked with the
# static libgaol just built (-static: a libtool library not installed), and
# with the link option gaol.pc gives (-mno-daz-ftz, where the compiler takes
# it), as CMake links them. Exit status 77 is a test skipped (intervalf and
# interval2f where the intervals of floats are not compiled).
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
LDADD = $(top_builddir)/gaol/libgaol.la $(GAOL_PC_LIBS)
AM_DEFAULT_SOURCE_EXT = .cpp

# The same list as tests/CMakeLists.txt
GAOL_TESTS = arithmetic elementary rounding_direction numbers other_functions \
	ieee1788 reverse u128 core_math expressions extended_precision \
	static_initialization debugging fast_math_link \
	arithmetic_operators assignment constants constructor float_functions \
	generic_functions input_output interval_functions misc non_arithmetic \
	relations reverse_mappings trigonometric intervalf interval2f
# The rounding direction after the automatic cleanup of GAOL, read by a
# function registered in a constructor of priority 101, which ELF runs before
# GAOL initializes itself
if HOST_LINUX
GAOL_TESTS += automatic_cleanup
endif

# fast_math_link: -ffast-math to its link only, its source being compiled as
# any code using GAOL, and GAOL_TESTS_NO_DAZ_FTZ where its link has
# -mno-daz-ftz (GAOL v5)
fast_math_link_LDFLAGS = $(AM_LDFLAGS) -ffast-math
if GAOL_NO_DAZ_FTZ
fast_math_link_CPPFLAGS = $(AM_CPPFLAGS) -DGAOL_TESTS_NO_DAZ_FTZ=1
else
fast_math_link_CPPFLAGS = $(AM_CPPFLAGS)
endif

# refused_options.sh: the options gaol/gaol_config.h refuses, which the
# compile tests refused_* of tests/CMakeLists.txt check in the CMake build
# (GAOL v5), compiled with the compiler and the flags of the tests
TEST_EXTENSIONS = .sh
SH_LOG_COMPILER = $(SHELL)
AM_TESTS_ENVIRONMENT = GAOL_CXX='$(CXX)'; \
	GAOL_CXXFLAGS='$(DEFS) $(AM_CPPFLAGS) $(CPPFLAGS) $(AM_CXXFLAGS) $(CXXFLAGS)'; \
	export GAOL_CXX GAOL_CXXFLAGS;

if WITH_TESTS
noinst_PROGRAMS = $(GAOL_TESTS) gaol_performance
TESTS = $(GAOL_TESTS) refused_options.sh
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
	  $(LDFLAGS) -static $(top_srcdir)/doc/compare/code/bench_gaol.cpp $(top_builddir)/gaol/libgaol.la -o $@
CLEANFILES = gaol_bench$(EXEEXT)

perf: gaol_bench$(EXEEXT)
	$(BASH) $(top_srcdir)/doc/compare/code/run_perf.sh ./gaol_bench$(EXEEXT) autotools "$(CXX)"

BASH = bash

.PHONY: perf

# make distclean removes the files of the dependencies one by one and leaves
# their directories: removed here, so that it gives the sources back as git has
# them (GAOL v5)
distclean-local:
	-rm -rf ./$(DEPDIR)
