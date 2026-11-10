#!/bin/sh
# Task PK: compiles a bench with the flags of GAOL (-frounding-math
# -fno-fast-math -ffp-contract=off, those of gaol.pc), with several compilers
# and optimization levels, and runs it. Nothing must be raised or wrong.
#
# usage: [SRC=pk_const.cpp] [CXXS="clang++-21 g++"] [OUT=dir] with_flags.sh \
#          <build dir with libgaol.a, of an FPU build> [extra flags...]
#
# The build dir comes from: cmake -S . -B build -DGAOL_SIMD=OFF
#
# Copyright (c) 2026 ENSTA, France
#
# By: Jordan NININ (created 2026-11-05)
#
# GAOL is a software distributed WITHOUT ANY WARRANTY. Read the associated
# COPYING.LIB file for information.

D=$(cd "$(dirname "$0")" && pwd)
R=$D/../..
B=$1; shift
SRC=${SRC:-pk_loops.cpp}
CXXS=${CXXS:-"clang++-18 clang++-20 g++"}
OUT=${OUT:-$(mktemp -d)}
for cxx in $CXXS; do
  command -v "$cxx" >/dev/null || { echo "$cxx: not found"; continue; }
  for opt in "-O2" "-O3" "-O3 -march=native" "-O3 -march=native -funroll-loops" \
             "-O2 -march=native" "-O3 -mavx2 -mno-fma" "-O3 -mavx2 -mfma"; do
    out=$OUT/bin_$(echo "$cxx $opt $*" | tr ' =' '__')
    $cxx $opt "$@" -DNDEBUG -std=c++17 -Wno-psabi -frounding-math -fno-fast-math \
      -ffp-contract=off -I"$B" -I"$R" "$D/$SRC" -o "$out" "$B/libgaol.a" 2>/dev/null \
      || { echo "$cxx $opt $*: BUILD FAILED"; continue; }
    res=$("$out" | tail -1)
    echo "$cxx $opt $*: ${res:-no result (can this CPU run it?)}"
  done
done
