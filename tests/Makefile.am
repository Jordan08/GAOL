# The unit tests of GAOL (GAOL v5), as tests/CMakeLists.txt builds them
#
# ./configure --with-tests builds them with GAOL (make), and make check and
# make test run them (see ../Makefile.am). They are compiled as any code using
# GAOL, with the flags of interval arithmetic configure gives, in C++17 for the
# hexadecimal floating literals of the tests (those of gaol_tests.h, which all
# but u128.cpp include, of the *_values.h, and of several of the programs; see
# CMakeLists.txt), with -Wno-psabi where the compiler takes it
# (GAOL_TESTS_CXXFLAGS, see CMakeLists.txt), and linked with the static
# libgaol just built (-static: a libtool library not installed), and with the
# link option gaol.pc gives (-mno-daz-ftz, where the compiler takes it), as
# CMake links them. Exit status 77 is a test skipped (intervalf and interval2f
# where the intervals of floats are not compiled).
#
# make perf measures GAOL v5 on the benchmark of doc/compare/performance.md
# (doc/compare/code/run_perf.sh).
#
# Copyright (c) 2026 ENSTA, France
#
# By: Jordan NININ (created 2026-09-27)
#
# GAOL is a software distributed WITHOUT ANY WARRANTY. Read the associated
# COPYING.LIB file for information.

AM_CPPFLAGS = -I$(top_builddir) -I$(top_srcdir)
AM_CXXFLAGS = -std=c++17 $(GAOL_TESTS_CXXFLAGS)
AM_LDFLAGS = -static
LDADD = $(top_builddir)/gaol/libgaol.la $(GAOL_PC_LIBS)
AM_DEFAULT_SOURCE_EXT = .cpp

# The same list as tests/CMakeLists.txt
GAOL_TESTS = arithmetic elementary rounding_direction numbers other_functions \
	ieee1788 reverse u128 core_math compoundm1 expressions extended_precision \
	static_initialization debugging fast_math_link test_ibex_codac \
	arithmetic_operators assignment constants constructor float_functions \
	generic_functions input_output interval_functions misc non_arithmetic \
	relations reverse_mappings trigonometric intervalf interval2f
# The rounding direction after the automatic cleanup of GAOL, read by a
# function registered in a constructor of priority 101, which ELF runs before
# GAOL initializes itself; and the bounds written with a snprintf() that
# writes wrong digits, which the link of bad_snprintf gives, by -Wl,--wrap, to
# the calls of the static libgaol: the functions of the test replace
# snprintf() and __snprintf_chk(), which the fortified calls of the C library
# of GNU reach (GAOL v5)
if HOST_LINUX
GAOL_TESTS += automatic_cleanup bad_snprintf
bad_snprintf_LDFLAGS = $(AM_LDFLAGS) -Wl,--wrap=snprintf -Wl,--wrap=__snprintf_chk
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
gaol_performance_CXXFLAGS = -std=c++14 $(GAOL_TESTS_CXXFLAGS)

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

# The archive of the sources, make dist (GAOL v5): automake takes the sources,
# the headers and the scripts it knows from the variables of the Makefile.am,
# and EXTRA_DIST names the other files of the repository, here the headers of the tests, the programs that generate their values, the tests of CMake and meson, and the tools of tests/tools/.
# make dist, package_source of CMake and meson dist hold the same files, those
# of the repository but the notes of the work on GAOL v5 of the branch dev: a
# file added to the repository is added to this list too, or the job Archives
# of the sources of the CI, which compares the three archives, fails
EXTRA_DIST = \
	CMakeLists.txt \
	atan2_rel_values.h \
	atan2_rel_values.py \
	automatic_configuration.cmake \
	compile_error.cmake \
	compoundm1_values.h \
	compoundm1_values.py \
	cpack_stale_configure.cmake \
	elementary_values.h \
	elementary_values.py \
	extended_precision_values.h \
	extended_precision_values.py \
	fetch_content/CMakeLists.txt \
	find_package/CMakeLists.txt \
	fp_strict/CMakeLists.txt \
	gaol_tests.h \
	meson.build \
	meson_subproject/check_flags.py \
	meson_subproject/meson.build \
	nodiscard.cpp \
	pow_rel_values.h \
	pow_rel_values.py \
	refused_aligned_new.cpp \
	refused_options.cpp \
	refused_options.sh \
	release_flags.py \
	reverse_values.h \
	reverse_values.py \
	tools/arith/README.md \
	tools/arith/diffarith.cpp \
	tools/arith/diffarith.py \
	tools/asm-order/README.md \
	tools/asm-order/check_order.py \
	tools/asm-order/inline.cpp \
	tools/cos/README.md \
	tools/cos/accurate_values.py \
	tools/cos/mutate.py \
	tools/ibex_codac/README.md \
	tools/ibex_codac/codac_cases.py \
	tools/ibex_codac/ibex_forward_cases.py \
	tools/ibex_codac/ibexb_cases.py \
	tools/pow/README.md \
	tools/pow/bench.cpp \
	tools/pow/build.py \
	tools/pow/checkrows.py \
	tools/pow/diffpow.cpp \
	tools/pow/diffpow.py \
	tools/pow/evalrows.cpp \
	tools/pow/gen_table.py \
	tools/pow/mutate.py \
	tools/pow/negative_power_rows.py \
	tools/pow/pown_bound.py \
	tools/pow/pownedge.cpp \
	tools/pow/pownhash.cpp \
	tools/x86-csr/README.md \
	tools/x86-csr/amalgamate.py \
	tools/x86-csr/mutants.py \
	unit_tests.h \
	version_file.cmake \
	version_file/nul_after_version.txt \
	version_file/utf16le_bom.txt
