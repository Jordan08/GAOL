# ==================================================================
#  The tests of GAOL built with a GAOL brought in by FetchContent
# ==================================================================
#
# Builds the tests of the parent directory as a project building GAOL for
# itself with FetchContent would: GAOL, holding the sources of CORE-MATH, is then
# targets of this project, built with it, and installed with it by
# cmake --install, CMake package included:
#
#   cmake -S tests/fetch_content -B build-fetch-content -DFETCHCONTENT_SOURCE_DIR_GAOL=<sources of GAOL> -DCMAKE_INSTALL_PREFIX=<prefix>
#   cmake --build build-fetch-content
#   ctest --test-dir build-fetch-content
#   cmake --install build-fetch-content
#
# tests/find_package then builds the tests with the GAOL installed so. Without
# FETCHCONTENT_SOURCE_DIR_GAOL, GAOL is cloned from the master branch of the
# fork, as a project using it would do.

cmake_minimum_required(VERSION 3.14)

project(gaol_tests_fetch_content CXX)

include(FetchContent)
FetchContent_Declare(gaol
  GIT_REPOSITORY https://github.com/Jordan08/GAOL.git
  GIT_TAG master
  GIT_SHALLOW TRUE
)
FetchContent_MakeAvailable(gaol)

foreach(target gaol::gaol)
  if(NOT TARGET ${target})
    message(FATAL_ERROR "FetchContent did not provide ${target}")
  endif()
endforeach()

# A shared library linking GAOL, as Python bindings do: this project does not
# set CMAKE_POSITION_INDEPENDENT_CODE, and GAOL, a static library holding the
# sources of CORE-MATH, has to be position-independent code for it to link
file(WRITE "${CMAKE_CURRENT_BINARY_DIR}/gaol_shared.cpp" [=[
#include <gaol/gaol.h>
#if defined(_WIN32)
__declspec(dllexport)
#endif
double gaol_shared_sin_upper(double x)
{
  return gaol::sin(gaol::interval(x)).right();
}
]=])
add_library(gaol_test_shared SHARED "${CMAKE_CURRENT_BINARY_DIR}/gaol_shared.cpp")
target_link_libraries(gaol_test_shared PRIVATE gaol::gaol)

enable_testing()

foreach(test arithmetic elementary rounding_direction numbers other_functions reverse)
  add_executable(gaol_test_${test} ../${test}.cpp)
  target_link_libraries(gaol_test_${test} PRIVATE gaol::gaol)
  set_target_properties(gaol_test_${test} PROPERTIES CXX_STANDARD 17 CXX_STANDARD_REQUIRED ON)
  if(MSVC)
    # getenv() and sscanf(), which the tests use as the C standard has them
    target_compile_definitions(gaol_test_${test} PRIVATE _CRT_SECURE_NO_WARNINGS)
  endif()
  add_test(NAME ${test} COMMAND gaol_test_${test})
endforeach()
