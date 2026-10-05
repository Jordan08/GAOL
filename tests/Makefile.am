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

if WITH_TESTS
noinst_PROGRAMS = $(GAOL_TESTS) gaol_performance
TESTS = $(GAOL_TESTS)
# Compile tests: refused_options.cpp with -ffinite-math-only and -ffast-math
# (GAOL v5). autotools builds GAOL with -frounding-math and its link option,
# whose -fno-fast-math turns -ffinite-math-only off when it comes after it,
# so these options come before the flags of interval arithmetic. The test
# passes if the compilation fails with the message of gaol_config.h.
# No such test for Visual C++, which has neither option nor macro.
if HOST_LINUX
# GCC and Clang on Linux refuse -ffinite-math-only and -ffast-math
check-refused_finite_math_only:
	@echo "Testing -ffinite-math-only refusal..."
	@$(CXX) $(DEFS) -ffinite-math-only $(AM_CPPFLAGS) $(CPPFLAGS) $(CXXFLAGS) \
	  -I$(top_builddir) -I$(top_srcdir) -c $(top_srcdir)/tests/refused_options.cpp -o /dev/null 2>&1 | \
	  grep -q "GAOL cannot be compiled with -ffinite-math-only" || exit 1

check-refused_fast_math:
	@echo "Testing -ffast-math refusal..."
	@$(CXX) $(DEFS) -ffast-math $(AM_CPPFLAGS) $(CPPFLAGS) $(CXXFLAGS) \
	  -I$(top_builddir) -I$(top_srcdir) -c $(top_srcdir)/tests/refused_options.cpp -o /dev/null 2>&1 | \
	  grep -q "GAOL cannot be compiled with -ffast-math" || exit 1

# Positive witness: compilation succeeds without refused options
check-refused_positive:
	@echo "Testing compilation without refused options..."
	@$(CXX) $(DEFS) $(AM_CPPFLAGS) $(CPPFLAGS) $(CXXFLAGS) \
	  -I$(top_builddir) -I$(top_srcdir) -c $(top_srcdir)/tests/refused_options.cpp -o /dev/null

TESTS += check-refused_finite_math_only check-refused_fast_math check-refused_positive
endif
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
