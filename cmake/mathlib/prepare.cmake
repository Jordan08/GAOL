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
# It also fixes the cosine of mathlib, in src/sincos32.c (see below). mathlib's
# sources are not modified otherwise. Run again on the sources it prepared, it
# leaves them as they are.

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

# The cosine of mathlib. For the arguments hardest to round, ucos() falls back
# on mpcos(), which computes cos(x) with multiple-precision numbers, as
# sin(pi/2 - x) when x > 0.8. c32(x, y, z) sets y to cos(x) and z to sin(x),
# and mpcos() took the cosine of pi/2 - x, which is sin(x), rather than its
# sine: ucos() returned sin(x), and GAOL bounds not enclosing cos(x), at 54 of
# the hard-to-round arguments of cos of CORE-MATH, all between 0.80 and 0.853
# (https://github.com/dreal-deps/mathlib/issues/2). glibc, which took the same
# code from IBM, fixed the same line in 2003 (Debian bug 153548).
set(sincos32 "${SOURCE_DIR}/src/sincos32.c")
file(READ "${sincos32}" code)
string(FIND "${code}" "c32(&b,&a,&c,p);" wrong)
string(FIND "${code}" "c32(&b,&c,&a,p);" right)
if(NOT wrong EQUAL -1)
  string(REPLACE "c32(&b,&a,&c,p);" "c32(&b,&c,&a,p);" code "${code}")
  file(WRITE "${sincos32}" "${code}")
elseif(right EQUAL -1)
  message(FATAL_ERROR "${sincos32} is not the one of mathlib 2.1.1: the call of c32() to fix in mpcos() is not found")
endif()
