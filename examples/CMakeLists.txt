# ==================================================================
#  Examples of GAOL v5: interval arithmetic as it is used
# ==================================================================
#
# A project using an installed GAOL, as a user's project would:
#
#   cmake -S examples -B build-examples -DCMAKE_PREFIX_PATH=<prefix of GAOL>
#   cmake --build build-examples
#   ctest --test-dir build-examples
#
# or a part of the build of GAOL, with the GAOL it builds, where WITH_EXAMPLES
# is ON (see CMakeLists.txt): make check then runs the examples, with the unit
# tests, and make test leaves them out (label example).
#
# The sixteen examples check what they print (the enclosures contain the
# values computed apart, the proofs hold) and exit with a failure otherwise,
# so that ctest runs them as tests. 16_Goldstein_Price is the example of
# GAOL 4, a timing loop that checks its enclosure too. The autotools and
# meson builds compile the sixteen as well (Makefile.am, meson.build).
# examples.md describes them all.
#
# Copyright (c) 2026 ENSTA, France
#
# Created 2026-09-27 by Jordan NININ

cmake_minimum_required(VERSION 3.14)

project(gaol_examples CXX)

# gaol::gaol is the library being built where the examples are a part of the
# build of GAOL
if(NOT TARGET gaol::gaol)
  find_package(gaol REQUIRED)
endif()
find_package(Threads REQUIRED)

enable_testing()

set(GAOL_EXAMPLES
  01_first_steps
  02_decimals_and_rump
  03_dependency_problem
  04_centered_form
  05_interval_newton
  06_global_optimization
  07_contractors
  08_sivia
  09_parameter_estimation
  10_krawczyk
  11_ode_enclosure
  12_affine_arithmetic
  13_rounding_environment
  14_generic_programming
  15_text_and_ieee1788
  16_Goldstein_Price
)

foreach(ex ${GAOL_EXAMPLES})
  add_executable(${ex} ${ex}.cpp)
  target_link_libraries(${ex} PRIVATE gaol::gaol Threads::Threads)
  set_target_properties(${ex} PROPERTIES CXX_STANDARD 17 CXX_STANDARD_REQUIRED ON)
  add_test(NAME ${ex} COMMAND ${ex})
  set_tests_properties(${ex} PROPERTIES LABELS example)
  list(APPEND GAOL_CHECK_TARGETS ${ex})
endforeach()
# The examples, which make check builds before running them, in the build of
# GAOL (CMakeLists.txt)
if(NOT CMAKE_CURRENT_SOURCE_DIR STREQUAL CMAKE_SOURCE_DIR)
  set(GAOL_CHECK_TARGETS ${GAOL_CHECK_TARGETS} PARENT_SCOPE)
endif()
