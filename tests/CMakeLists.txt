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
)

foreach(test ${GAOL_TESTS})
  add_executable(gaol_test_${test} ${test}.cpp gaol_tests.h)
  target_link_libraries(gaol_test_${test} PRIVATE gaol::gaol)
  set_target_properties(gaol_test_${test} PROPERTIES CXX_STANDARD 17 CXX_STANDARD_REQUIRED ON)
  add_test(NAME ${test} COMMAND gaol_test_${test})
endforeach()
