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
# target. mathlib's sources are not modified otherwise.

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
