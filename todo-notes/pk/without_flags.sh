#!/bin/sh
# Task PK: compiles a bench WITHOUT -frounding-math (not supported by GAOL: the
# bounds are no longer certified) and with several sets of flags, to see which
# one makes the compiler vectorize the relations into signaling compares.
# Expected with Clang 18 and 20: loops that raise FE_INVALID, unless
# -frounding-math is given; GCC raises none, except with -fno-trapping-math.
#
# usage: [SRC=pk_const.cpp] [CXXS="clang++-21 g++"] [OUT=dir] without_flags.sh \
#          <build dir with libgaol.a, of an FPU build>
#
# Copyright (c) 2026 ENSTA, France
#
# By: Jordan NININ (created 2026-11-05)
#
# GAOL is a software distributed WITHOUT ANY WARRANTY. Read the associated
# COPYING.LIB file for information.

D=$(cd "$(dirname "$0")" && pwd)
R=$D/../..
B=$1
SRC=${SRC:-pk_loops.cpp}
CXXS=${CXXS:-"clang++-18 clang++-20 g++"}
OUT=${OUT:-$(mktemp -d)}
for cxx in $CXXS; do
  command -v "$cxx" >/dev/null || { echo "$cxx: not found"; continue; }
  for opt in "-O3 -march=native" "-O3 -march=native -fno-rounding-math" \
             "-O3 -march=native -fno-trapping-math" "-O3 -march=tigerlake" \
             "-O2 -march=native" "-O3 -march=native -fno-math-errno" \
             "-O3 -march=native -fno-math-errno -fno-signed-zeros -fno-trapping-math"; do
    out=$OUT/bin_$(echo "$cxx $opt" | tr ' =' '__')
    $cxx $opt -DNDEBUG -std=c++17 -Wno-psabi -I"$B" -I"$R" "$D/$SRC" -o "$out" \
      "$B/libgaol.a" 2>/dev/null || { echo "$cxx $opt: BUILD FAILED"; continue; }
    res=$("$out" | tail -1)
    echo "$cxx $opt: ${res:-no result (can this CPU run it? tigerlake needs AVX-512)}"
  done
done
