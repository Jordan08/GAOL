# ==================================================================
#  Prepares the sources of mathlib 2.1.1 for their CMake build
# ==================================================================
#
# Run by the CMake build of GAOL (CMakeLists.txt) as the patch step of the
# download of mathlib:
#
#   cmake -DSOURCE_DIR=<sources of mathlib> -P prepare.cmake
#
# It copies the CMake build of this directory to the root of the sources, and
# replaces src/mathlib_config_msvc.h and src/mathlib_config_mingw.h, the
# configuration files src/mathlib_config.h includes for Visual C++ and MinGW,
# with files including the configuration CMake generates for every compiler.
# The original configuration files only select the implementation of
# Init_Lib() for 32-bit x86, which mathlib_configuration.h.in does for every
# target.
#
# It also fixes bugs of mathlib (see below). mathlib's sources are not modified
# otherwise. Run again on the sources it prepared, it leaves them as they are.

cmake_minimum_required(VERSION 3.14)

if(NOT IS_DIRECTORY "${SOURCE_DIR}" OR NOT EXISTS "${SOURCE_DIR}/src/MathLib.h")
  message(FATAL_ERROR "Usage: cmake -DSOURCE_DIR=<sources of mathlib> -P ${CMAKE_CURRENT_LIST_FILE}")
endif()

configure_file("${CMAKE_CURRENT_LIST_DIR}/CMakeLists.txt" "${SOURCE_DIR}/CMakeLists.txt" COPYONLY)
configure_file("${CMAKE_CURRENT_LIST_DIR}/mathlib_configuration.h.in" "${SOURCE_DIR}/mathlib_configuration.h.in" COPYONLY)

foreach(file src/mathlib_config_msvc.h src/mathlib_config_mingw.h)
  file(WRITE "${SOURCE_DIR}/${file}"
"/* Written by the CMake build of GAOL (cmake/mathlib/prepare.cmake) in place of
   the configuration file for this compiler: CMake generates the configuration
   for every compiler, from mathlib_configuration.h.in. */

#include \"mathlib_configuration.h\"
")
endforeach()

# Replaces old, which has to be found once, with new in the file of mathlib's
# sources given, unless new is already there
function(fix_source file old new)
  set(path "${SOURCE_DIR}/${file}")
  file(READ "${path}" code)
  string(FIND "${code}" "${new}" done)
  if(NOT done EQUAL -1)
    return()
  endif()
  string(FIND "${code}" "${old}" first)
  string(FIND "${code}" "${old}" last REVERSE)
  if(first EQUAL -1 OR NOT first EQUAL last)
    message(FATAL_ERROR "${path} is not the one of mathlib 2.1.1: the code to fix is not found in it once:\n${old}")
  endif()
  string(REPLACE "${old}" "${new}" code "${code}")
  file(WRITE "${path}" "${code}")
endfunction()

# The cosine of mathlib. For the arguments hardest to round, ucos() falls back
# on mpcos(), which computes cos(x) with multiple-precision numbers, as
# sin(pi/2 - x) when x > 0.8. c32(x, y, z) sets y to cos(x) and z to sin(x),
# and mpcos() took the cosine of pi/2 - x, which is sin(x), rather than its
# sine: ucos() returned sin(x), and GAOL bounds not enclosing cos(x), at 54 of
# the hard-to-round arguments of cos of CORE-MATH, all between 0.80 and 0.853
# (https://github.com/dreal-deps/mathlib/issues/2). glibc, which took the same
# code from IBM, fixed the same line in 2003 (Debian bug 153548,
# https://sourceware.org/git/?p=glibc.git;a=commit;h=86583139a4d746743ccffcd72e25d96c5fb8d488).
fix_source(src/sincos32.c "c32(&b,&a,&c,p);" "c32(&b,&c,&a,p);")

# The square root of mathlib in multiple precision, which its arctangent uses at
# the arguments hardest to round (src/mpatan.c, src/mpatan2.c). fastiroot() in
# src/mpsqrt.c, which gives it a first approximation of 1/sqrt(x), read and
# wrote the halves of a double through an array of two longs: where long has 64
# bits (Linux and macOS on 64-bit processors), p.i[HIGH_HALF] is not the high
# half of the double, x was not scaled to [0.5, 2), and the approximation was
# wrong. atan() then returned values far from atan(x), or had not returned
# after 20 ms, at 12003 and 3967 of the 55190 hard-to-round arguments of atan
# of CORE-MATH (https://gitlab.inria.fr/core-math/core-math) on x86_64:
# atan(1.016527294692847) was 0.082 instead of 0.794. glibc, which took the
# same code from IBM, made them ints in 2003 (Michael Matz, "fastiroot: Fix
# 64-bit problem",
# https://sourceware.org/git/?p=glibc.git;a=commit;h=bb3f4825c411e676c51479fea59643af540810b5).
# atan() gave such values on Alpha as well (https://bugs.debian.org/210613,
# whose three arguments fail on x86_64 without this fix).
fix_source(src/mpsqrt.c
  "union {long i[2]; double d;} p,q;\n  double y,z, t;\n  long n;"
  "union {int i[2]; double d;} p,q;\n  double y,z, t;\n  int n;")
