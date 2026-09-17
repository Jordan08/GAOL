#!/bin/sh
# Builds the mathlib of 3rd/mathlib with the flags of interval arithmetic (the
# CMake build of 3rd/mathlib, the one CMakeLists.txt uses when it builds mathlib
# itself) and installs it under the prefix given:
#
#   sh scripts/install-mathlib.sh <prefix>
#
# For the builds of GAOL told to use an installed mathlib rather than the one
# of 3rd/mathlib: configure (--with-mathlib-include=<prefix>/include
# --with-mathlib-lib=<prefix>/lib), meson (-Dwith-mathlib-include,
# -Dwith-mathlib-lib) and CMake with MATHLIB_DIR=<prefix>. Needs cmake and a C
# compiler. To be run from the root of GAOL's sources.
set -e

prefix=$1
work=${RUNNER_TEMP:-/tmp}/mathlib-work
rm -rf "$work"
mkdir -p "$work"

# The flags GAOL builds mathlib with, CMakeLists.txt giving them to it: on a
# 32-bit x86 processor, the doubles computed with SSE2 rather than on the x87
flags="-frounding-math -fno-fast-math -ffp-contract=off"
case "$(${CC:-cc} -dumpmachine 2>/dev/null)" in
  i?86-*) flags="$flags -msse2 -mfpmath=sse" ;;
esac
cmake -S 3rd/mathlib -B "$work/build" -DCMAKE_BUILD_TYPE=Release -DCMAKE_POSITION_INDEPENDENT_CODE=ON \
  "-DCMAKE_C_FLAGS=$flags" \
  -DCMAKE_INSTALL_PREFIX="$prefix" -DCMAKE_INSTALL_LIBDIR=lib
cmake --build "$work/build" -j 4
cmake --build "$work/build" --target install
