# ==================================================================
#  CMake build of GAOL
# ==================================================================
#
# GAOL, the interval arithmetic library written by Frederic Goualard
# (https://github.com/goualard-f/GAOL, GNU LGPL v2), comes with an autotools
# and a meson build, which this fork keeps. This CMake build is an addition of
# the fork, written for Codac (https://github.com/codac-team/codac). See the
# section of README.md about this fork.
#
# Origin
# ------
# It derives from the CMake build of GAOL in IBEX
# (https://github.com/ibex-team/ibex-lib, GNU LGPL v3,
# interval_lib_wrapper/gaol/3rd/gaol-4.2.3alpha0/CMakeLists.txt), written by
# Cyril Bouvier and updated by Gilles Chabert, with the list of sources of
# GAOL's meson build (gaol/meson.build). The compilation flags of interval
# arithmetic are those ibex_init_common() sets in the IBEX fork of Fabrice Le
# Bars (https://github.com/lebarsfa/ibex-lib), and what Visual C++ needs
# follows his fork of GAOL (https://github.com/lebarsfa/GAOL).
#
# It differs from IBEX's build in that the configuration is decided by the
# preprocessor (cmake/gaol_configuration.h.in) and by compilation checks
# rather than from CMAKE_SYSTEM_NAME and CMAKE_SYSTEM_PROCESSOR, which works the
# same in a cross-compilation, in that it builds with Visual C++, and in that
# it builds mathlib, the IBM Accurate Portable Mathematical Library GAOL
# computes its elementary functions with, from Frederic Goualard's archive when
# it is not installed (cmake/mathlib/).

cmake_minimum_required(VERSION 3.14...3.25)

project(gaol VERSION 4.2.3 LANGUAGES C CXX)

include(GNUInstallDirs)
include(CMakePackageConfigHelpers)
include(CheckCXXCompilerFlag)
include(CheckCXXSourceCompiles)
include(CheckIncludeFileCXX)
include(CheckSymbolExists)

set(GAOL_IS_TOP_LEVEL OFF)
if(CMAKE_SOURCE_DIR STREQUAL CMAKE_CURRENT_SOURCE_DIR)
  set(GAOL_IS_TOP_LEVEL ON)
  if(NOT CMAKE_BUILD_TYPE AND NOT CMAKE_CONFIGURATION_TYPES)
    set(CMAKE_BUILD_TYPE Release CACHE STRING "Build type" FORCE)
  endif()
  # The static libraries can then be linked into shared libraries
  if(NOT DEFINED CMAKE_POSITION_INDEPENDENT_CODE)
    set(CMAKE_POSITION_INDEPENDENT_CODE ON)
  endif()
endif()

option(GAOL_BUILD_MATHLIB "Download and build mathlib when no installed mathlib is found" ON)
option(GAOL_BUILD_TESTS "Build the tests of GAOL's bounds, run by ctest" ${GAOL_IS_TOP_LEVEL})
set(MATHLIB_DIR "" CACHE PATH "Installation prefix of an installed mathlib (MathLib.h and the ultim library)")

# As gaol/meson.build asks: cpp_std=c++11, c_std=c99
set(CMAKE_CXX_STANDARD 11)
set(CMAKE_C_STANDARD 99)


################################################################################
# The compilation flags of interval arithmetic
################################################################################
#
# GAOL computes with the rounding direction set upward once for all, and needs
# every floating-point operation to be rounded in the direction in effect, as
# it is written: not evaluated at compile time in the default rounding, nor
# contracted into a fused multiply-add. These flags are given to GAOL and
# mathlib, and to the targets linking gaol::gaol, GAOL's headers being full of
# inline interval operations.

# Clang does not honour the rounding direction on 32-bit ARM processors: it
# does not support -frounding-math there ("overriding currently unsupported
# rounding mode on this target") and optimizes floating-point operations as if
# they were always rounded to nearest. Built by Clang 21 for 32-bit ARM, 4556 of
# 16000 random products, squares and cubes computed by GAOL did not enclose
# their exact value; built by GCC 15, none. GAOL is not built with it there.
check_cxx_source_compiles("
  #if !(defined(__arm__) || defined(_M_ARM)) || defined(__aarch64__)
  #error not a 32-bit ARM target
  #endif
  int main() { return 0; }"
  GAOL_TARGET_IS_ARM32)
if(GAOL_TARGET_IS_ARM32)
  set(_clang_compilers "")
  foreach(lang C CXX)
    if(CMAKE_${lang}_COMPILER_ID MATCHES "Clang")
      list(APPEND _clang_compilers "${CMAKE_${lang}_COMPILER} (${CMAKE_${lang}_COMPILER_ID} ${CMAKE_${lang}_COMPILER_VERSION})")
    endif()
  endforeach()
  if(_clang_compilers)
    string(REPLACE ";" "\n  " _clang_compilers "${_clang_compilers}")
    message(FATAL_ERROR
      "GAOL cannot be built with Clang for a 32-bit ARM processor, and this build uses\n"
      "  ${_clang_compilers}\n"
      "Clang does not honour the rounding direction on 32-bit ARM processors: it does not support "
      "-frounding-math there (\"overriding currently unsupported rounding mode on this target\") and "
      "optimizes floating-point operations as if they were always rounded to nearest. GAOL computes "
      "with the rounding direction set upward, so that the intervals it computes would not enclose the "
      "values they should (built by Clang 21, 4556 of 16000 random products, squares and cubes did not).\n"
      "Build GAOL with GCC, which honours the rounding direction on 32-bit ARM processors, for "
      "instance:\n"
      "  cmake -S . -B build -DCMAKE_C_COMPILER=gcc -DCMAKE_CXX_COMPILER=g++\n"
      "on a 32-bit ARM system (package g++ of Debian, Ubuntu or Raspberry Pi OS), or\n"
      "  cmake -S . -B build -DCMAKE_C_COMPILER=arm-linux-gnueabihf-gcc -DCMAKE_CXX_COMPILER=arm-linux-gnueabihf-g++\n"
      "to cross-compile (package g++-arm-linux-gnueabihf of Debian and Ubuntu), in a new build "
      "directory.")
  endif()
endif()

set(GAOL_INTERVAL_FLAGS "")
if(MSVC)
  list(APPEND GAOL_INTERVAL_FLAGS /fp:strict)
else()
  # From ibex_init_common(): each flag is kept where the compiler takes it
  foreach(flag -frounding-math -ffloat-store -fno-fast-math -ffp-contract=off)
    string(MAKE_C_IDENTIFIER "${flag}" _flag_id)
    string(TOUPPER "GAOL_COMPILER_SUPPORTS${_flag_id}" _flag_var)
    check_cxx_compiler_flag("${flag}" ${_flag_var})
    if(${_flag_var})
      list(APPEND GAOL_INTERVAL_FLAGS ${flag})
    endif()
  endforeach()

  # On a 32-bit x86 processor, doubles are computed in SSE2 rather than on the
  # x87. Computed on the x87, GAOL's bounds and mathlib's results are only right
  # while the precision of the x87 is set to 53 bits, which GAOL's
  # initialisation does not keep (it restores the default floating-point
  # environment, whose precision is 64 bits on Linux and with MinGW): built for
  # an i686 computing on the x87, exp, sin and cos missed the exact value for
  # most arguments. Visual C++ computes in SSE2 already.
  check_cxx_source_compiles("
    #if !defined(__i386__)
    #error not a 32-bit x86 target
    #endif
    int main() { return 0; }"
    GAOL_TARGET_IS_X86_32)
  if(GAOL_TARGET_IS_X86_32)
    check_cxx_compiler_flag("-msse2 -mfpmath=sse" GAOL_COMPILER_SUPPORTS_MSSE2_MFPMATH_SSE)
    if(GAOL_COMPILER_SUPPORTS_MSSE2_MFPMATH_SSE)
      list(APPEND GAOL_INTERVAL_FLAGS -msse2 -mfpmath=sse)
    endif()
  endif()

  # A compiler may take -frounding-math and still not honour the rounding
  # direction on the target, and says so, as Clang does for 32-bit ARM
  # processors ("overriding currently unsupported rounding mode on this
  # target"): built by Clang 21 for 32-bit ARM, 4556 of 16000 random products,
  # squares and cubes computed by GAOL did not enclose their exact value; built
  # by GCC 15, none.
  if(GAOL_COMPILER_SUPPORTS_FROUNDING_MATH)
    set(CMAKE_REQUIRED_FLAGS "-frounding-math")
    check_cxx_source_compiles("int main() { return 0; }" GAOL_COMPILER_HONOURS_ROUNDING_MATH
                              FAIL_REGEX "unsupported rounding mode")
    unset(CMAKE_REQUIRED_FLAGS)
    if(NOT GAOL_COMPILER_HONOURS_ROUNDING_MATH)
      message(WARNING "${CMAKE_CXX_COMPILER_ID} ${CMAKE_CXX_COMPILER_VERSION} does not honour the rounding "
                      "direction on this target (-frounding-math): the intervals computed by GAOL may not "
                      "enclose the values they should. Use a compiler that does, such as GCC.")
    endif()
  endif()
endif()
string(REPLACE ";" " " GAOL_INTERVAL_FLAGS_STRING "${GAOL_INTERVAL_FLAGS}")
message(STATUS "Interval arithmetic flags: ${GAOL_INTERVAL_FLAGS_STRING}")


################################################################################
# mathlib
################################################################################
#
# libgaol, a static library, does not link mathlib itself, but whatever links
# GAOL has to: gaol::gaol links it.

find_path(MATHLIB_INCDIR MathLib.h HINTS "${MATHLIB_DIR}" PATH_SUFFIXES include)
find_library(MATHLIB_LIB ultim HINTS "${MATHLIB_DIR}" PATH_SUFFIXES lib)

set(GAOL_MATHLIB "")
if(MATHLIB_INCDIR AND MATHLIB_LIB)
  message(STATUS "Found mathlib: ${MATHLIB_LIB}")
  set(GAOL_MATHLIB "${MATHLIB_LIB}")
elseif(GAOL_BUILD_MATHLIB)
  message(STATUS "mathlib not found: it will be downloaded and built")
  include(FetchContent)
  # The CMake build of cmake/mathlib/ exports libultim along with libgaol
  set(MATHLIB_EXPORT_NAME gaolTargets)
  set(_download_options "")
  if(NOT CMAKE_VERSION VERSION_LESS 3.24)
    list(APPEND _download_options DOWNLOAD_EXTRACT_TIMESTAMP TRUE)
  endif()
  # mathlib 2.1.1, as Frederic Goualard distributes it (GNU GPL v2 or later),
  # the archive GAOL's README points to; its checksum is checked.
  FetchContent_Declare(mathlib
    URL https://frederic.goualard.net/software/mathlib-2.1.1.tar.gz
    URL_HASH SHA256=f299848aa3e57ebb6248cd3cf54ecc7661a945aeac9e420e71db194965f87281
    ${_download_options}
    PATCH_COMMAND ${CMAKE_COMMAND} -DSOURCE_DIR=<SOURCE_DIR> -P "${CMAKE_CURRENT_SOURCE_DIR}/cmake/mathlib/prepare.cmake"
  )
  FetchContent_MakeAvailable(mathlib)
  target_compile_options(ultim PRIVATE ${GAOL_INTERVAL_FLAGS})
  set(GAOL_MATHLIB ultim)
else()
  message(WARNING "mathlib not found and GAOL_BUILD_MATHLIB is OFF: link the ultim library along with GAOL.")
endif()


################################################################################
# What the compiler provides
################################################################################

# GAOL sets the rounding direction with <fenv.h> (gaol/gaol_fpu.h)
check_include_file_cxx(fenv.h HAVE_FENV_H)
if(NOT HAVE_FENV_H)
  message(FATAL_ERROR "GAOL needs <fenv.h>, which this compiler does not provide")
endif()

# gaol/gaol_profile.cpp measures time with getrusage() where it exists, and with
# clock() otherwise
check_symbol_exists(getrusage "sys/resource.h" HAVE_GETRUSAGE)
set(GETRUSAGE_IN_HEADER ${HAVE_GETRUSAGE})

# With -frounding-math, GAOL negates a double with the minus sign rather than by
# flipping its sign bit (gaol_opposite() in gaol/gaol_port.h)
set(HAVE_ROUNDING_MATH_OPTION ${GAOL_COMPILER_SUPPORTS_FROUNDING_MATH})

# On x86 processors, the intervals of GAOL are computed with SSE2 instructions
# (gaol/gaol_interval_sse.h), except on Windows: IBEX builds GAOL there without
# them ("it can generate failures during execution"), and so does the fork of
# Fabrice Le Bars. GAOL's SSE3 instructions are left out: they only serve
# gaol::interval2f, intervals of floats, and without them the processor needs
# no more than SSE2, which every x86_64 processor has.
set(CMAKE_REQUIRED_FLAGS "${GAOL_INTERVAL_FLAGS_STRING}")
check_cxx_source_compiles("
  #if !(defined(__x86_64__) || defined(__i386__)) || defined(_WIN32)
  #error GAOL computes its intervals with SSE2 on x86 processors outside Windows only
  #endif
  #include <emmintrin.h>
  int main() { return _mm_movemask_pd(_mm_set1_pd(-1.0)); }"
  USING_SSE2_INSTRUCTIONS)
unset(CMAKE_REQUIRED_FLAGS)

configure_file(cmake/gaol_configuration.h.in "${CMAKE_CURRENT_BINARY_DIR}/gaol/gaol_configuration.h")
# The double operations of the mathematical library GAOL is built with, mathlib
# here, as gaol/meson.build and IBEX name them
configure_file(gaol/gaol_double_op_apmathlib.h "${CMAKE_CURRENT_BINARY_DIR}/gaol/gaol_double_op.h" COPYONLY)


################################################################################
# libgaol
################################################################################
#
# The sources listed in gaol/meson.build. gaol_interval.cpp includes
# gaol_interval_sse.cpp or gaol_interval_fpu.cpp itself, according to
# USING_SSE2_INSTRUCTIONS. gaol_exact.c is left out for Visual C++, for which
# gaol/gaol_exact.h defines get_inexact() and clear_inexact() itself.
#
# A static library: for Visual C++, GAOL's declarations are then compiled
# without __declspec(dllexport) or dllimport (see __GAOL_PUBLIC__ below).

set(GAOL_SOURCES
  gaol_interval.cpp gaol_profile.cpp gaol_common.cpp gaol_parser.cpp
  gaol_expression.cpp gaol_port.cpp gaol_exceptions.cpp gaol_init_cleanup.cpp
  s_nextafter.c gaol_intervalf.cpp gaol_interval_lexer.cpp
  gaol_interval_parser.cpp
)
if(NOT MSVC)
  list(APPEND GAOL_SOURCES gaol_exact.c)
endif()
list(TRANSFORM GAOL_SOURCES PREPEND "gaol/")

add_library(gaol STATIC ${GAOL_SOURCES})
add_library(gaol::gaol ALIAS gaol)

# The generated headers, then GAOL's own: its sources include "gaol/..."
target_include_directories(gaol PUBLIC
  "$<BUILD_INTERFACE:${CMAKE_CURRENT_BINARY_DIR}>"
  "$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>"
  "$<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>"
)
target_compile_options(gaol PUBLIC ${GAOL_INTERVAL_FLAGS})
if(MSVC)
  # The lexer includes <unistd.h>, which Visual C++ does not have
  target_include_directories(gaol PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}/cmake/msvc")
  target_compile_options(gaol PRIVATE /Zc:strictStrings-)
  target_compile_definitions(gaol PRIVATE _CRT_SECURE_NO_WARNINGS _CRT_NONSTDC_NO_WARNINGS)
  # gaol/gaol_config.h declares GAOL's classes and functions
  # __declspec(dllimport) for Visual C++ unless __GAOL_PUBLIC__ is defined, as
  # for a DLL: every file including GAOL's headers has to define it, empty
  target_compile_definitions(gaol PUBLIC __GAOL_PUBLIC__=)
endif()
if(GAOL_MATHLIB)
  target_link_libraries(gaol PUBLIC ${GAOL_MATHLIB})
endif()


################################################################################
# Installation
################################################################################
#
# find_package(gaol) then provides gaol::gaol, and gaol::ultim when mathlib was
# built here.

install(TARGETS gaol EXPORT gaolTargets
  ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
  LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
  RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)

# GAOL's headers, including those gaol/gaol_config.h includes for Visual C++
# and MinGW, which gaol/meson.build does not install
file(GLOB GAOL_HEADERS gaol/*.h)
install(FILES
  gaol/gaol
  ${GAOL_HEADERS}
  "${CMAKE_CURRENT_BINARY_DIR}/gaol/gaol_configuration.h"
  "${CMAKE_CURRENT_BINARY_DIR}/gaol/gaol_double_op.h"
  DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/gaol
)
install(FILES gaol/sysdeps/gaol_exact_c99.h gaol/sysdeps/gaol_exact_msvc.h DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/gaol/sysdeps)

set(GAOL_INSTALL_CMAKEDIR ${CMAKE_INSTALL_LIBDIR}/cmake/gaol)
install(EXPORT gaolTargets NAMESPACE gaol:: DESTINATION ${GAOL_INSTALL_CMAKEDIR})
configure_package_config_file(cmake/gaolConfig.cmake.in "${CMAKE_CURRENT_BINARY_DIR}/gaolConfig.cmake"
  INSTALL_DESTINATION ${GAOL_INSTALL_CMAKEDIR})
write_basic_package_version_file("${CMAKE_CURRENT_BINARY_DIR}/gaolConfigVersion.cmake"
  VERSION ${PROJECT_VERSION} COMPATIBILITY SameMajorVersion)
install(FILES "${CMAKE_CURRENT_BINARY_DIR}/gaolConfig.cmake" "${CMAKE_CURRENT_BINARY_DIR}/gaolConfigVersion.cmake"
  DESTINATION ${GAOL_INSTALL_CMAKEDIR})


################################################################################
# Tests
################################################################################

if(GAOL_BUILD_TESTS)
  enable_testing()
  add_subdirectory(tests)
endif()
