# Copyright (c) 2026 ENSTA, France
#
# Created 2026-09-20 by Jordan NININ
#
# ==================================================================
#  Tests of GAOL's bounds (fork of GAOL)
# ==================================================================
#
# Each test program compares the bounds GAOL computes with the exact results
# (see gaol_tests.h). They are compiled as any code using GAOL, with the flags
# gaol::gaol carries, and in C++17 for the hexadecimal floating literals of
# elementary_values.h.

set(GAOL_TESTS
  arithmetic
  elementary
  rounding_direction
  numbers
  other_functions
  reverse
  # The 128-bit integer GAOL computes the accurate phases of CORE-MATH with
  # where the compiler has none, and the bounds CORE-MATH gives (fork of GAOL)
  u128
  core_math
  expressions
)

foreach(test ${GAOL_TESTS})
  add_executable(gaol_test_${test} ${test}.cpp gaol_tests.h)
  target_link_libraries(gaol_test_${test} PRIVATE gaol::gaol)
  set_target_properties(gaol_test_${test} PROPERTIES CXX_STANDARD 17 CXX_STANDARD_REQUIRED ON)
  if(MSVC)
    # getenv() and sscanf(), which the tests use as the C standard has them
    target_compile_definitions(gaol_test_${test} PRIVATE _CRT_SECURE_NO_WARNINGS)
  endif()
  add_test(NAME ${test} COMMAND gaol_test_${test})
endforeach()

# The time per operation of GAOL and of the same operation on doubles, which
# the continuous integration prints: not a test ctest runs
add_executable(gaol_performance performance.cpp)
target_link_libraries(gaol_performance PRIVATE gaol::gaol)
set_target_properties(gaol_performance PROPERTIES CXX_STANDARD 17 CXX_STANDARD_REQUIRED ON)
