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
  # A program linked with -ffast-math, which links crtfastmath.o and the modes
  # flushing the subnormals to zero that it sets, unless gaol::gaol keeps it
  # out with -mno-daz-ftz (GAOL v5): see below
  fast_math_link
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
# fast_math_link: -ffast-math to its link only, with GCC and Clang, its source
# being compiled as any code using GAOL, and GAOL_TESTS_NO_DAZ_FTZ where
# gaol::gaol gives the link -mno-daz-ftz
if(NOT MSVC AND CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
  target_link_options(gaol_test_fast_math_link PRIVATE -ffast-math)
endif()
if(GAOL_LINK_FLAGS)
  target_compile_definitions(gaol_test_fast_math_link PRIVATE GAOL_TESTS_NO_DAZ_FTZ=1)
endif()

# Under a locale writing a decimal comma, the reading of a number never
# returned (GAOL v5): numbers, which reads numbers under such a locale, fails
# after 5 minutes should it hang again, ctest giving a test no time limit of
# its own (the project does not include CTest). A slower build can give it
# more: numbers took 235 s of its 300 with GCC, ASan and UBSan in Debug on
# macOS x86_64, whose job gives it 400 (.github/workflows/macos.yml)
set(GAOL_NUMBERS_TIMEOUT 300 CACHE STRING "Time limit of the test numbers, in seconds")
set_tests_properties(numbers PROPERTIES TIMEOUT ${GAOL_NUMBERS_TIMEOUT})

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
# The options gaol/gaol_config.h refuses where GCC and Clang define a macro for
# them (GAOL v5): the tests refused_finite_math_only and refused_fast_math
# compile refused_options.cpp, a program using GAOL, with -ffinite-math-only
# and with -ffast-math, and the compilation has to fail with the message of the
# refusal. The object is a target of its own, left out of the build of GAOL and
# made by the test (cmake --build), so that it gets the compiler and the flags
# of the platform as the files of GAOL do. A build fails for many reasons, so
# the test does not look at its exit status: it passes if the output of the
# build holds the message.
#
# The option has to come after the flags of interval arithmetic, whose
# -fno-fast-math turns it off otherwise. gaol::gaol is therefore not linked,
# CMake putting the options of a linked library after those of the target: the
# flags it carries are written out here. Not with Visual C++, which has neither
# option nor macro (see doc/three-builds.md). The autotools and meson builds
# have no such test: the header it checks is the same.
function(gaol_refused_test name flag text)
  add_library(gaol_${name} OBJECT refused_options.cpp)
  set_target_properties(gaol_${name} PROPERTIES EXCLUDE_FROM_ALL TRUE)
  target_include_directories(gaol_${name} PRIVATE "${CMAKE_BINARY_DIR}" "${CMAKE_SOURCE_DIR}")
  target_compile_options(gaol_${name} PRIVATE ${GAOL_INTERVAL_FLAGS} ${GAOL_FMA_FLAGS} ${flag})
  add_test(NAME ${name}
    COMMAND "${CMAKE_COMMAND}" --build "${CMAKE_BINARY_DIR}" --config $<CONFIG> --target gaol_${name})
  # One build at a time in the build directory, whatever ctest -j says: the
  # lock of every compile test
  set_tests_properties(${name} PROPERTIES LABELS unit RESOURCE_LOCK gaol_compile_tests
                       PASS_REGULAR_EXPRESSION "${text}")
endfunction()
if(NOT MSVC AND CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
  gaol_refused_test(refused_finite_math_only -ffinite-math-only
                    "GAOL cannot be compiled with -ffinite-math-only")
  gaol_refused_test(refused_fast_math -ffast-math
                    "GAOL cannot be compiled with -ffast-math")
endif()

# The warning of the CPack block of CMakeLists.txt, when the configure that the
# archive of the sources holds was generated for another version than VERSION.txt
# holds (GAOL v5): not a program, a script that configures copies of the tree
# with the compiler that built GAOL (cpack_stale_configure.cmake). Only the CMake
# build makes the archive; the copies are made of symbolic links, so the script
# runs on the machine that builds, where the system has them, and it asks the
# configure of the tree its version, so there has to be one. The time limit is
# that of a few configurations, on a slow machine.
if(CMAKE_HOST_UNIX AND NOT CMAKE_CROSSCOMPILING AND EXISTS "${PROJECT_SOURCE_DIR}/configure")
  add_test(NAME cpack_stale_configure
    COMMAND "${CMAKE_COMMAND}"
      "-DGAOL_SOURCE_DIR=${PROJECT_SOURCE_DIR}"
      "-DGAOL_WORK_DIR=${CMAKE_CURRENT_BINARY_DIR}/cpack_stale_configure"
      "-DGAOL_GENERATOR=${CMAKE_GENERATOR}"
      "-DGAOL_C_COMPILER=${CMAKE_C_COMPILER}"
      "-DGAOL_CXX_COMPILER=${CMAKE_CXX_COMPILER}"
      -P "${CMAKE_CURRENT_SOURCE_DIR}/cpack_stale_configure.cmake")
  set_tests_properties(cpack_stale_configure PROPERTIES LABELS unit TIMEOUT 600)
endif()

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

# The warning for a result thrown away, in each language standard (GAOL v5):
# [[nodiscard]] is C++17, and gaol::gaol sets no standard, so that a project
# with GCC 9 compiles in C++14 (GAOL_NODISCARD, gaol/gaol_config.h, then takes
# [[nodiscard]] where the compiler has it before C++17, and the attribute of
# the compiler otherwise). For each of C++11, C++14 and C++17 (C++14 for C++11
# with Visual C++), nodiscard.cpp is compiled with the warning for a result
# thrown away made an error (-Werror=unused-result, /we4834 of Visual C++), in
# two tests: nodiscard_discard_cxx<N> has it throw away sqrt(x), and passes if
# the build fails with the message of the compiler; nodiscard_used_cxx<N> has
# it use the result, and passes if the build succeeds, GAOL's headers throwing
# no result away. -Werror=attributes (/we5030 and /we5051 of Visual C++, an
# attribute it does not know or ignores before C++17) makes an attribute the
# compiler ignores fail them too, as [[nodiscard]] written after the
# declarator, int f() [[nodiscard]];, of which GCC 13 only warns. The object
# is a target of its own, left out of the build of GAOL and made by the test
# (cmake --build), with the flags, the definitions (__GAOL_PUBLIC__ empty with
# Visual C++) and the include directories of GAOL's headers as gaol::gaol
# carries them, written out here (those of the project GAOL, should another
# include it): only the headers are read, so that the test does not wait for
# the library, which gaol::gaol would build first. The tests run in the C
# locale, the message of GCC and Clang being translated otherwise; that of
# Visual C++ is looked for by its number. Not with Visual C++ before 2019
# 16.4, whose _Check_return_ before C++17 only /analyze reports, nor with
# Clang for Windows. The autotools and meson builds have no such test: the
# header it checks is the same.
function(gaol_nodiscard_test standard flags message)
  foreach(kind discard used)
    set(name nodiscard_${kind}_cxx${standard})
    add_library(gaol_${name} OBJECT nodiscard.cpp)
    set_target_properties(gaol_${name} PROPERTIES EXCLUDE_FROM_ALL TRUE
                          CXX_STANDARD ${standard} CXX_STANDARD_REQUIRED ON)
    target_include_directories(gaol_${name} PRIVATE "${PROJECT_BINARY_DIR}" "${PROJECT_SOURCE_DIR}")
    target_compile_options(gaol_${name} PRIVATE ${GAOL_INTERVAL_FLAGS} ${GAOL_FMA_FLAGS} ${flags})
    target_compile_definitions(gaol_${name} PRIVATE $<TARGET_PROPERTY:gaol,INTERFACE_COMPILE_DEFINITIONS>)
    add_test(NAME ${name}
      COMMAND "${CMAKE_COMMAND}" --build "${CMAKE_BINARY_DIR}" --config $<CONFIG> --target gaol_${name})
    # One build at a time in the build directory, whatever ctest -j says: the
    # lock of every compile test
    set_tests_properties(${name} PROPERTIES LABELS unit RESOURCE_LOCK gaol_compile_tests
                         ENVIRONMENT "LC_ALL=C;LANGUAGE=C")
    if(kind STREQUAL "discard")
      target_compile_definitions(gaol_${name} PRIVATE GAOL_DISCARD)
      set_tests_properties(${name} PROPERTIES PASS_REGULAR_EXPRESSION "${message}")
    endif()
  endforeach()
endfunction()
if(CMAKE_CXX_COMPILER_ID STREQUAL "MSVC" AND NOT MSVC_VERSION LESS 1924)
  foreach(_standard 11 14 17)
    gaol_nodiscard_test(${_standard} "/we4834;/we5030;/we5051" "C4834")
  endforeach()
elseif(NOT MSVC AND CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
  foreach(_standard 11 14 17)
    gaol_nodiscard_test(${_standard} "-Werror=unused-result;-Werror=attributes"
                        "ignoring return value of")
  endforeach()
endif()
