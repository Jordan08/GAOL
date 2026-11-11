# Note: To be used with GNU Make
# If this file has the name "Makefile.in" then it is a template for a
# Makefile; to generate the actual Makefile, run "./configure" from the base
# directory of gaol.
#
# Copyright (c) 2001 Swiss Federal Institute of Technology, Switzerland
# Copyright (c) 2002 Institut de Recherche en Informatique de Nantes, France
# Copyright (c) 2026 ENSTA, France
#
# By: Frederic Goualard, Jordan NININ
#
# GAOL is a software distributed WITHOUT ANY WARRANTY. Read the associated
# COPYING.LIB file for information.


# 3rd/ builds nothing: the sources of CORE-MATH are compiled into libgaol by
# gaol/Makefile.am. DIST_SUBDIRS is what make dist and make distclean go
# through (GAOL v5).
SUBDIRS = 3rd gaol tests examples
DIST_SUBDIRS = 3rd gaol tests examples manual
ACLOCAL_AMFLAGS = -I m4
# make distcheck builds the archive with the unit tests and the examples, and
# runs them (make check), as meson dist builds and tests its archive (GAOL v5)
AM_DISTCHECK_CONFIGURE_FLAGS = --with-tests --with-examples
# configure reads the version of GAOL from VERSION.txt: make runs it again
# (config.status --recheck) when VERSION.txt changes, as CMake and meson
# configure again. With --enable-maintainer-mode, make also generates configure
# again (the m4_include of VERSION.txt in configure.ac) (GAOL v5)
CONFIG_STATUS_DEPENDENCIES = $(top_srcdir)/VERSION.txt

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
# gaol/parser/gaol_interval_lexer.cpp, gaol/parser/gaol_interval_parser.cpp and
# gaol/parser/gaol_interval_parser.h, from gaol/parser/gaol_interval_lexer.lpp
# and gaol/parser/gaol_interval_parser.ypp, with the flex and the bison of the
# system (FLEX, BISON). The three files are committed: no build needs flex nor
# bison otherwise (GAOL v5).
parser:
	$(SHELL) $(top_srcdir)/gaol/parser/regenerate_parser.sh

# make coverage, with --enable-coverage (GAOL v5, as the target coverage of
# CMake and meson): the tests run and gcovr writes coverage/coverage.html and
# the page of conclusions coverage/README.md. GAOL's own sources (gaol/) are
# counted apart from those of CORE-MATH (3rd/math-core), whose accurate phases
# are reached by a handful of arguments in a million; the raw data of gcovr
# (coverage.json) is written in the build directory.
if COVERAGE
coverage:
	find . -name '*.gcda' -delete
	$(MAKE) $(AM_MAKEFLAGS) check
	$(MKDIR_P) $(abs_srcdir)/coverage
	$(GCOVR) --root $(abs_srcdir) --gcov-ignore-errors=no_working_dir_found --merge-mode-functions=merge-use-line-min \
	  --filter $(abs_srcdir)/gaol/ --filter $(abs_srcdir)/3rd/math-core/ \
	  --html --html-self-contained --html-title "Coverage of GAOL" \
	  --output $(abs_srcdir)/coverage/coverage.html \
	  --json coverage.json --print-summary .
	$(PYTHON3) $(abs_srcdir)/coverage/coverage_summary.py coverage.json $(abs_srcdir)/coverage/README.md
else
# Without --enable-coverage the target fails, as those of CMake and meson do
coverage:
	@echo "make coverage needs ./configure --enable-coverage (and --enable-debug, --with-tests)" >&2; exit 1
endif

.PHONY: test perf parser coverage

# The archive of the sources, make dist (GAOL v5): automake takes the sources,
# the headers and the scripts it knows from the variables of the Makefile.am,
# and EXTRA_DIST names the other files of the repository, here the files at the top of the tree that no Makefile.am of a directory takes (the builds of CMake and meson, the documents, cmake/, coverage/, doc/, the CI in .github/).
# make dist, package_source of CMake and meson dist hold the same files, those
# of the repository but the notes of the work on GAOL v5 (.gitattributes): a
# file added to the repository is added to this list too, or the job Archives
# of the sources of the CI, which compares the three archives, fails
EXTRA_DIST = \
	.gitattributes \
	.github/actionlint.yaml \
	.github/audit/README.md \
	.github/audit/audit.sh \
	.github/audit/compare.py \
	.github/audit/logs.sh \
	.github/audit/make_probe.py \
	.github/audit/probe_build.py \
	.github/scripts/check-gcc-windows-fma.sh \
	.github/scripts/check-intervals.sh \
	.github/scripts/comma-locale.sh \
	.github/scripts/compare-performance.py \
	.github/scripts/headers.sh \
	.github/scripts/newlib-fe-values.h \
	.github/scripts/performance-ab.sh \
	.github/scripts/sde-tests.sh \
	.github/scripts/test-performance-tools.py \
	.github/scripts/tests.sh \
	.github/scripts/version-file.sh \
	.github/workflows/build-systems.yml \
	.github/workflows/containers.yml \
	.github/workflows/linux.yml \
	.github/workflows/macos.yml \
	.github/workflows/manual.yml \
	.github/workflows/performance.yml \
	.github/workflows/windows.yml \
	.gitignore \
	.shellcheckrc \
	BUGS \
	CMakeLists.txt \
	README.md \
	cmake/gaol.pc.in \
	cmake/gaolConfig.cmake.in \
	cmake/gaol_configuration.h.in \
	cmake/gaol_package_source.cmake \
	cmake/gaol_version.cmake \
	cmake/msvc/unistd.h \
	coverage/README.md \
	coverage/coverage.html \
	coverage/coverage_summary.py \
	doc/accuracy.md \
	doc/building.md \
	doc/compare/README.md \
	doc/compare/code/.gitignore \
	doc/compare/code/README.md \
	doc/compare/code/bench.py \
	doc/compare/code/bench_boost.cpp \
	doc/compare/code/bench_common.h \
	doc/compare/code/bench_double.cpp \
	doc/compare/code/bench_filib.cpp \
	doc/compare/code/bench_gaol.cpp \
	doc/compare/code/bench_ops.h \
	doc/compare/code/bench_p1788.cpp \
	doc/compare/code/bench_profil.cpp \
	doc/compare/code/bench_sun.f90 \
	doc/compare/code/boost_policies.h \
	doc/compare/code/build_benchmarks.sh \
	doc/compare/code/cases.py \
	doc/compare/code/enclosure.h \
	doc/compare/code/enclosure.py \
	doc/compare/code/enclosure_boost.cpp \
	doc/compare/code/enclosure_gaol.cpp \
	doc/compare/code/env.sh \
	doc/compare/code/machine.txt \
	doc/compare/code/machine_lines.txt \
	doc/compare/code/machine_lines_arm64.txt \
	doc/compare/code/results.csv \
	doc/compare/code/results_lines.csv \
	doc/compare/code/results_lines_arm64.csv \
	doc/compare/code/run_all.sh \
	doc/compare/code/run_bench.sh \
	doc/compare/code/run_cases.sh \
	doc/compare/code/run_enclosure.sh \
	doc/compare/code/run_lines.sh \
	doc/compare/code/run_lines_arm64.sh \
	doc/compare/code/run_perf.sh \
	doc/compare/code/setup.sh \
	doc/compare/enclosure.md \
	doc/compare/itf1788.md \
	doc/compare/itf1788/.gitignore \
	doc/compare/itf1788/fetch.sh \
	doc/compare/itf1788/itf1788.py \
	doc/compare/itf1788/itf1788_gaol.h \
	doc/compare/itf1788/run.sh \
	doc/compare/performance.md \
	doc/compare/special_cases.md \
	doc/compare/tang2021.md \
	doc/compare/tang2021/.gitignore \
	doc/compare/tang2021/check.py \
	doc/compare/tang2021/env.sh \
	doc/compare/tang2021/fetch.sh \
	doc/compare/tang2021/gaol.patch \
	doc/compare/tang2021/run.sh \
	doc/continuous-integration.md \
	doc/differences.md \
	doc/math-libraries.md \
	doc/simd.md \
	doc/tests.md \
	doc/three-builds.md \
	doc/tools.md \
	doc/using.md \
	meson.build \
	meson_options.txt
