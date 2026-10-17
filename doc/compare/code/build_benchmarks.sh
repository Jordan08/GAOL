#!/usr/bin/env bash
# The compilation of the C++ programs of the benchmark and the description of
# the machine, which run_bench.sh and run_lines.sh share (sourced after env.sh,
# not run): OUT is where the programs go, CODE_DIR this directory.
#
# Copyright (c) 2026 ENSTA, France
#
# Created 2026-10-17 by Jordan NININ

# bench_double, bench_gaol5 (the GAOL of this repository), bench_gaol (the last
# GAOL of Frédéric Goualard), bench_p1788, bench_filib, bench_profil and
# bench_boost
compile_cpp_benchmarks() {
$CXX -std=c++11 $CXXFLAGS_BENCH $FMA_FLAGS -I"$CODE_DIR" "$CODE_DIR/bench_double.cpp" -o "$OUT/bench_double"
# The same source for the two GAOL: this repository's, under the name gaol5,
# and Goualard's last one, under the name gaol
$CXX -std=c++11 $CXXFLAGS_BENCH $(gaol_cflags) -DGAOL_BENCH_NAME='"gaol5"' -I"$CODE_DIR" \
     "$CODE_DIR/bench_gaol.cpp" -o "$OUT/bench_gaol5" $(gaol_libs)
$CXX -std=c++11 $CXXFLAGS_BENCH $(gaol_goualard_cflags) -I"$CODE_DIR" "$CODE_DIR/bench_gaol.cpp" \
     -o "$OUT/bench_gaol" $(gaol_goualard_libs)
$CXX -std=c++11 $CXXFLAGS_BENCH $FMA_FLAGS $IA_CXXFLAGS $(p1788_cflags) -I"$CODE_DIR" "$CODE_DIR/bench_p1788.cpp" \
     -o "$OUT/bench_p1788" $(p1788_libs)
$CXX $CXXFLAGS_BENCH $FMA_FLAGS $IA_CXXFLAGS $(filib_cflags) -I"$CODE_DIR" "$CODE_DIR/bench_filib.cpp" \
     -o "$OUT/bench_filib" $(filib_libs)
$CXX -std=c++11 $CXXFLAGS_BENCH $FMA_FLAGS $IA_CXXFLAGS $(profil_cflags) -I"$CODE_DIR" "$CODE_DIR/bench_profil.cpp" \
     -o "$OUT/bench_profil" $(profil_libs)
$CXX -std=c++11 $CXXFLAGS_BENCH $FMA_FLAGS $IA_CXXFLAGS $(boost_cflags) -I"$CODE_DIR" "$CODE_DIR/bench_boost.cpp" \
     -o "$OUT/bench_boost"
}

# The lines of machine.txt: what the results were measured on; "fortran" as
# the argument adds the Fortran compiler's, which run_lines.sh does not use
machine_description() {
  echo "Date:            $(date '+%Y-%m-%d')"
  echo "Processor:       $(grep -m1 'model name' /proc/cpuinfo | cut -d: -f2 | sed 's/^ *//')${CPU:+ (taskset -c $CPU)}"
  echo "System:          $(uname -sr), $(. /etc/os-release 2>/dev/null && echo "$PRETTY_NAME")"
  echo "C++ compiler:    $($CXX --version | head -1)"
  echo "C++ flags:       -std=c++11 $CXXFLAGS_BENCH $FMA_FLAGS (GAOL: $(gaol_cflags | sed -e "s# *-I[^ ]*##g" -e "s#^ *##"); libieeep1788, filib++, PROFIL/BIAS and Boost.Interval: $IA_CXXFLAGS)"
  [ "${1:-}" = fortran ] && echo "Fortran:         $("$F90" -V 2>&1 | head -1), flags: $F90FLAGS_BENCH"
  # -dirty only for what GAOL is built from: the reports of doc/ are being rewritten
  gaol_commit="$(git -C "$ROOT_DIR" describe --always 2>/dev/null || echo "?")"
  git -C "$ROOT_DIR" diff --quiet HEAD -- . ':!doc' 2>/dev/null || gaol_commit="$gaol_commit-dirty"
  echo "GAOL V5.0.0:     the branch of this checkout ($gaol_commit), CMake Release, CORE-MATH of 3rd/math-core compiled into the library, the rounding direction not preserved"
  echo "GAOL $GAOL_GOUALARD_VERSION:      the last version of Frédéric Goualard (${GAOL_GOUALARD_COMMIT:0:7} of $GAOL_GOUALARD_REPO), its configure, the rounding direction not preserved, mathlib $MATHLIB_VERSION"
  echo "libieeep1788:    ${P1788_COMMIT:0:7}, MPFR $(grep -m1 '#define MPFR_VERSION_STRING' "$PREFIX/include/mpfr.h" 2>/dev/null | cut -d'"' -f2), GMP $(grep -m1 -E '^#define __GNU_MP_VERSION ' "$PREFIX/include/gmp.h" 2>/dev/null | awk '{print $3}').$(grep -m1 -E '^#define __GNU_MP_VERSION_MINOR ' "$PREFIX/include/gmp.h" 2>/dev/null | awk '{print $3}').$(grep -m1 -E '^#define __GNU_MP_VERSION_PATCHLEVEL ' "$PREFIX/include/gmp.h" 2>/dev/null | awk '{print $3}')"
  echo "filib++:         $FILIB_VERSION, interval<double, native_switched, i_mode_extended_flag>"
  echo "PROFIL/BIAS:     $PROFIL_VERSION, x86-64-Linux-compat-gcc configuration, built by $CC and $CXX"
  echo "Boost.Interval:  Boost $BOOST_VERSION, interval<double> with save_state<rounded_transc_opp<double> > and checking_strict<double>, the elementary functions of the C library, $(getconf GNU_LIBC_VERSION 2>/dev/null || echo "?")"
}
