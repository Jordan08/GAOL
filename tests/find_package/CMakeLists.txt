# ==================================================================
#  The tests of GAOL built with an installed GAOL (fork of GAOL)
# ==================================================================
#
# Builds the tests of the parent directory with the GAOL that find_package()
# finds, as a project using GAOL would, which checks that gaol::gaol carries
# what they need (include directories, mathlib, compilation flags):
#
#   cmake -S tests/find_package -B build-find-package -DCMAKE_PREFIX_PATH=<prefix of GAOL>
#   cmake --build build-find-package
#   ctest --test-dir build-find-package

cmake_minimum_required(VERSION 3.14)

project(gaol_tests_find_package CXX)

find_package(gaol REQUIRED)
message(STATUS "GAOL found in ${gaol_DIR}")

enable_testing()

foreach(test arithmetic elementary rounding_direction numbers other_functions)
  add_executable(gaol_test_${test} ../${test}.cpp)
  target_link_libraries(gaol_test_${test} PRIVATE gaol::gaol)
  set_target_properties(gaol_test_${test} PROPERTIES CXX_STANDARD 17 CXX_STANDARD_REQUIRED ON)
  add_test(NAME ${test} COMMAND gaol_test_${test})
endforeach()
