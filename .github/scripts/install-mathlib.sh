#!/bin/sh
# Downloads mathlib 2.1.1 from Frederic Goualard's site, checks its checksum,
# and installs it in the directory given, with the CMake build of cmake/mathlib
# (the one CMakeLists.txt uses when it builds mathlib itself). For the builds
# of GAOL that do not build mathlib: autotools, meson, and CMake with
# MATHLIB_DIR. To be run from the root of GAOL's sources.
set -e

prefix=$1
work=${RUNNER_TEMP:-/tmp}/mathlib-work
rm -rf "$work"
mkdir -p "$work"

curl -sSL -o "$work/mathlib-2.1.1.tar.gz" https://frederic.goualard.net/software/mathlib-2.1.1.tar.gz
cd "$work"
if command -v sha256sum > /dev/null; then
  echo "f299848aa3e57ebb6248cd3cf54ecc7661a945aeac9e420e71db194965f87281  mathlib-2.1.1.tar.gz" | sha256sum -c -
else
  echo "f299848aa3e57ebb6248cd3cf54ecc7661a945aeac9e420e71db194965f87281  mathlib-2.1.1.tar.gz" | shasum -a 256 -c -
fi
tar xzf mathlib-2.1.1.tar.gz
cd - > /dev/null

cmake -DSOURCE_DIR="$work/mathlib-2.1.1" -P cmake/mathlib/prepare.cmake
# The flags GAOL builds mathlib with, CMakeLists.txt giving them to it
cmake -S "$work/mathlib-2.1.1" -B "$work/build" -DCMAKE_BUILD_TYPE=Release -DCMAKE_POSITION_INDEPENDENT_CODE=ON \
  "-DCMAKE_C_FLAGS=-frounding-math -fno-fast-math -ffp-contract=off" \
  -DCMAKE_INSTALL_PREFIX="$prefix" -DCMAKE_INSTALL_LIBDIR=lib
cmake --build "$work/build" -j 4
cmake --build "$work/build" --target install
