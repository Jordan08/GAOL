# ==================================================================
#  The unit tests of GAOL (GAOL v5)
# ==================================================================
#
# Each test program of GAOL v5 compares the bounds GAOL computes with the
# exact results (see gaol_tests.h); the unit tests of GAOL 4, which were in
# check/ and ran with CppUnit, check the results of the operations on chosen
# intervals (see unit_tests.h). They are compiled as any code using GAOL, with
# the flags gaol::gaol carries, and in C++17 for the hexadecimal floating
# literals of elementary_values.h. WITH_TESTS builds them (CMakeLists.txt);
# make test and make check run them, with the label unit.
#
# Copyright (c) 2026 ENSTA, France
#
# Created 2026-09-20 by Jordan NININ

set(GAOL_TESTS
  arithmetic
  elementary
  rounding_direction
  numbers
  other_functions
  # gaol_ieee1788 under using namespace gaol_ieee1788 alone, as a program
  # opens it: pow, pown, the names of the standard and intervalToExact()
  # (GAOL v5)
  ieee1788
  reverse
  # The 128-bit integer GAOL computes the accurate phases of CORE-MATH with
  # where the compiler has none, and the bounds CORE-MATH gives (GAOL v5)
  u128
  core_math
  expressions
  # The results that doubles computed in extended precision, on the x87 unit,
  # round wrongly (GAOL v5)
  extended_precision
  # The operations computed in the initialization of static objects, before
  # the files of the static library GAOL are initialized, except with
  # MinGW-w64 (GAOL v5)
  static_initialization
  # GAOL's headers with GAOL_DEBUGGING, which the Debug builds define (GAOL v5)
  debugging
  # The unit tests of GAOL 4 (check/), with the checks of unit_tests.h rather
  # than CppUnit (GAOL v5); arithmetic is arithmetic_operators, the name
  # arithmetic being taken
  arithmetic_operators
  assignment
  constants
  constructor
  float_functions
  generic_functions
  input_output
  interval_functions
  misc
  non_arithmetic
  relations
  reverse_mappings
  trigonometric
  # The intervals of floats, which only a developer of GAOL compiles (see
  # gaol/gaol_config.h): without GAOL_FLOAT_INTERVALS, the tests are skipped
  # (exit status 77)
  intervalf
  interval2f
)
# The rounding direction after the automatic cleanup of GAOL, read after the
# end of main() by a function registered in a constructor of priority 101,
# which ELF runs before GAOL initializes itself (GAOL v5): a static libgaol
# only, a shared one initializing itself before that constructor runs
# (BUILD_SHARED_LIBS)
get_target_property(_gaol_type gaol TYPE)
if(CMAKE_SYSTEM_NAME STREQUAL "Linux" AND _gaol_type STREQUAL "STATIC_LIBRARY")
  list(APPEND GAOL_TESTS automatic_cleanup)
endif()

foreach(test ${GAOL_TESTS})
  add_executable(gaol_test_${test} ${test}.cpp gaol_tests.h)
  target_link_libraries(gaol_test_${test} PRIVATE gaol::gaol)
  set_target_properties(gaol_test_${test} PROPERTIES CXX_STANDARD 17 CXX_STANDARD_REQUIRED ON)
  if(MSVC)
    # getenv() and sscanf(), which the tests use as the C standard has them
    target_compile_definitions(gaol_test_${test} PRIVATE _CRT_SECURE_NO_WARNINGS)
  endif()
  add_test(NAME ${test} COMMAND gaol_test_${test})
  set_tests_properties(${test} PROPERTIES LABELS unit SKIP_RETURN_CODE 77)
  list(APPEND GAOL_CHECK_TARGETS gaol_test_${test})
endforeach()
set(GAOL_CHECK_TARGETS ${GAOL_CHECK_TARGETS} PARENT_SCOPE)

# Under a locale writing a decimal comma, the reading of a number never
# returned (GAOL v5): numbers, which reads numbers under such a locale, fails
# after 5 minutes should it hang again, ctest giving a test no time limit of
# its own (the project does not include CTest)
set_tests_properties(numbers PROPERTIES TIMEOUT 300)

# The reading of VERSION.txt by CMakeLists.txt, in cmake/gaol_version.cmake,
# which has to ignore a UTF-8 byte order mark and the line ends of Windows
# (GAOL v5). A script of CMake, run by CMake: neither configure nor meson, which
# have their own reading, has this test; .github/scripts/version-file.sh checks
# theirs
add_test(NAME version_file
  COMMAND "${CMAKE_COMMAND}" "-DGAOL_SOURCE_DIR=${PROJECT_SOURCE_DIR}"
          "-DWORK_DIR=${CMAKE_CURRENT_BINARY_DIR}/version_file"
          -P "${CMAKE_CURRENT_SOURCE_DIR}/version_file.cmake")
set_tests_properties(version_file PROPERTIES LABELS unit)

# Commented out: the tests run no thread (GAOL v5)
# # ieee1788 calls intervalToExact() in a thread of its own (std::thread), and
# # expressions reads strings in four threads at once, which needs -pthread with
# # the glibc before 2.34
# find_package(Threads REQUIRED)
# target_link_libraries(gaol_test_ieee1788 PRIVATE Threads::Threads)
# target_link_libraries(gaol_test_expressions PRIVATE Threads::Threads)

# The time per operation of GAOL and of the same operation on doubles, which
# the continuous integration prints: not a test ctest runs
add_executable(gaol_performance performance.cpp)
target_link_libraries(gaol_performance PRIVATE gaol::gaol)
set_target_properties(gaol_performance PROPERTIES CXX_STANDARD 17 CXX_STANDARD_REQUIRED ON)
