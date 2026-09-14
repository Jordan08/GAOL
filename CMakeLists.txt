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
option(GAOL_FIND_MATHLIB "Look for an installed mathlib before building one" ON)
option(GAOL_PRESERVE_ROUNDING "Restore the rounding direction found after each operation, rather than leaving it upward" OFF)
option(GAOL_BUILD_TESTS "Build the tests of GAOL's bounds, run by ctest" ${GAOL_IS_TOP_LEVEL})
set(MATHLIB_DIR "" CACHE PATH "Installation prefix of an installed mathlib (MathLib.h and the ultim library)")
# The options of configure (--enable-simd, --enable-asm, --enable-verbose-mode),
# with its defaults
option(GAOL_SIMD "Compute the intervals with SSE2 instructions on x86 processors, and gaol::interval2f with SSE3" ON)
option(GAOL_ASM "Use assembler code (GAOL_USING_ASM)" ON)
option(GAOL_VERBOSE_MODE "Write a line on the standard error when GAOL initializes and cleans up" ON)

# As configure.ac and meson.build ask: -std=c++11; C99 with the GNU extensions,
# which mathlib's DPChange.c needs on 32-bit x86 (asm)
set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_EXTENSIONS OFF)
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

# The math library of mingw-w64 before its version 12, which MinGW-w64 GCC 11 to
# 13 come with (MinGW-Builds with rt_v10 and rt_v11, WinLibs), is not accurate
# enough for the hyperbolic functions GAOL takes from it: acosh() near 1 was up
# to 25 million floats away from the exact value, and asinh() of large negative
# numbers NaN, so that GAOL's bounds did not enclose the exact values
# (tests/elementary.cpp, in the continuous integration, with the MinGW-w64 GCC
# 11.2.0, 12.2.0 and 13.2.0 of Chocolatey, while 14.2.0, with rt_v12, and 15.2.0
# passed). Computed without the libm, acosh() still gave an inverted interval
# there, which tracing its computation made disappear. GAOL is not built with
# it.
check_cxx_source_compiles("
  #include <stdlib.h>
  #if defined(__MINGW64_VERSION_MAJOR) && __MINGW64_VERSION_MAJOR < 12
  #error mingw-w64 before version 12
  #endif
  int main() { return 0; }"
  GAOL_MINGW_W64_RECENT_ENOUGH)
if(NOT GAOL_MINGW_W64_RECENT_ENOUGH)
  message(FATAL_ERROR
    "GAOL cannot be built with ${CMAKE_CXX_COMPILER} (${CMAKE_CXX_COMPILER_ID} ${CMAKE_CXX_COMPILER_VERSION}), "
    "whose mingw-w64 is older than version 12.\n"
    "The math library of mingw-w64 before version 12 is not accurate enough for the hyperbolic functions GAOL "
    "takes from it: acosh() near 1 was up to 25 million floats away from the exact value, and asinh() of large "
    "negative numbers NaN, so that the intervals GAOL computes did not enclose the values they should (with the "
    "MinGW-w64 GCC 11.2.0, 12.2.0 and 13.2.0 of Chocolatey).\n"
    "Build GAOL with a MinGW-w64 of version 13 or later (version 12 is refused too, its fesetround() being slow), "
    "such as the MinGW-w64 GCC 15.2.0 (rt_v13) of MinGW-Builds, which Chocolatey installs (choco install mingw "
    "--version=15.2.0), or the one of MSYS2, or with Visual Studio, in a new build directory.")
endif()

# fesetround() of mingw-w64 12 on x86 and x64 processors (MinGW-Builds with
# rt_v12, as the MinGW-w64 GCC 14.2.0 of Chocolatey) runs the instruction cpuid
# at each call, to know whether the processor has SSE (__mingw_has_sse(), whose
# result mingw-w64 13 keeps). cpuid exits to the hypervisor in a virtual
# machine: in the continuous integration, fesetround() took 3.5 microseconds per
# call, against 130 nanoseconds with mingw-w64 13. GAOL changes the rounding
# direction four times for each exp(), log(), sin() or cos() of an interval,
# which took 10.6 to 13.8 microseconds rather than 0.5 to 0.7. GAOL's bounds were
# right, but GAOL is not built with it.
check_cxx_source_compiles("
  #include <stdlib.h>
  #if defined(__MINGW64_VERSION_MAJOR) && __MINGW64_VERSION_MAJOR < 13
  #error mingw-w64 before version 13
  #endif
  int main() { return 0; }"
  GAOL_MINGW_W64_FAST_ENOUGH)
if(NOT GAOL_MINGW_W64_FAST_ENOUGH)
  message(FATAL_ERROR
    "GAOL cannot be built with ${CMAKE_CXX_COMPILER} (${CMAKE_CXX_COMPILER_ID} ${CMAKE_CXX_COMPILER_VERSION}), "
    "whose mingw-w64 is version 12.\n"
    "fesetround() of mingw-w64 12 runs the instruction cpuid at each call, which takes microseconds in a virtual "
    "machine: 3.5 microseconds per call in GitHub Actions, against 130 nanoseconds with mingw-w64 13. GAOL changes "
    "the rounding direction four times for each exp(), log(), sin() or cos() of an interval, which then took 10.6 "
    "to 13.8 microseconds rather than 0.5 to 0.7 (with the MinGW-w64 GCC 14.2.0 of Chocolatey, rt_v12): the "
    "intervals GAOL computes are right, but some 20 times slower.\n"
    "Build GAOL with a MinGW-w64 of version 13 or later, such as the MinGW-w64 GCC 15.2.0 (rt_v13) of MinGW-Builds, "
    "which Chocolatey installs (choco install mingw --version=15.2.0), or the one of MSYS2, or with Visual Studio, "
    "in a new build directory.")
endif()

set(GAOL_INTERVAL_FLAGS "")
if(MSVC)
  list(APPEND GAOL_INTERVAL_FLAGS /fp:strict)
else()
  # From ibex_init_common(): each flag is kept where the compiler takes it
  foreach(flag -frounding-math -fno-fast-math -ffp-contract=off)
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
  # GAOL refuses -ffast-math and doubles computed on the x87 unit at compile
  # time (gaol/gaol_config.h): the flags above keep both away.

  # -ffloat-store only where doubles are still computed on the x87 unit
  # (FLT_EVAL_METHOD not 0: a 32-bit x86 target without SSE2), whose 80-bit
  # registers keep more digits than a double. Computed in double precision
  # (SSE2, ARM, and the other processors), doubles gain nothing from it, and
  # GCC stored every double variable in memory rather than in a register.
  string(REPLACE ";" " " CMAKE_REQUIRED_FLAGS "${GAOL_INTERVAL_FLAGS}")
  check_cxx_source_compiles("
    #include <cfloat>
    #if defined(FLT_EVAL_METHOD) && FLT_EVAL_METHOD == 0
    #error doubles are computed in double precision
    #endif
    int main() { return 0; }"
    GAOL_DOUBLES_IN_EXTENDED_PRECISION)
  unset(CMAKE_REQUIRED_FLAGS)
  if(GAOL_DOUBLES_IN_EXTENDED_PRECISION)
    check_cxx_compiler_flag(-ffloat-store GAOL_COMPILER_SUPPORTS_FFLOAT_STORE)
    if(GAOL_COMPILER_SUPPORTS_FFLOAT_STORE)
      list(APPEND GAOL_INTERVAL_FLAGS -ffloat-store)
    endif()
  endif()

  # A compiler may take -frounding-math and still not honour the rounding
  # direction on the target, and say so ("overriding currently unsupported
  # rounding mode on this target"), as Clang 14 does for 64-bit ARM processors:
  # built by it, GAOL gave bounds of pow() and nth_root() that did not enclose
  # the exact values (tests/elementary.cpp and tests/arithmetic.cpp), which
  # Clang 18 did not. GAOL is not built with such a compiler, as with Clang for
  # 32-bit ARM processors above.
  if(GAOL_COMPILER_SUPPORTS_FROUNDING_MATH)
    set(CMAKE_REQUIRED_FLAGS "-frounding-math")
    check_cxx_source_compiles("int main() { return 0; }" GAOL_COMPILER_HONOURS_ROUNDING_MATH
                              FAIL_REGEX "unsupported rounding mode")
    unset(CMAKE_REQUIRED_FLAGS)
    if(NOT GAOL_COMPILER_HONOURS_ROUNDING_MATH)
      message(FATAL_ERROR
        "GAOL cannot be built with ${CMAKE_CXX_COMPILER} (${CMAKE_CXX_COMPILER_ID} ${CMAKE_CXX_COMPILER_VERSION}) "
        "for this target.\n"
        "This compiler does not honour the rounding direction on this target, and says so of -frounding-math "
        "(\"overriding currently unsupported rounding mode on this target\"). GAOL computes with the rounding "
        "direction set upward, so that the intervals it computes would not enclose the values they should "
        "(built by Clang 14 for 64-bit ARM processors, bounds of pow() and nth_root() did not).\n"
        "Build GAOL with a compiler that honours the rounding direction on this target, such as GCC or a later "
        "version of Clang (Clang 18 does on 64-bit ARM processors), in a new build directory.")
    endif()
  endif()
endif()
string(REPLACE ";" " " GAOL_INTERVAL_FLAGS_STRING "${GAOL_INTERVAL_FLAGS}")


################################################################################
# mathlib
################################################################################
#
# libgaol, a static library, does not link mathlib itself, but whatever links
# GAOL has to: gaol::gaol links it.
#
# With GAOL_FIND_MATHLIB OFF, no installed mathlib is looked for, and the one
# built here is the one GAOL links and installs: a project building GAOL for
# itself (as Codac does, with ExternalProject) can then count on finding
# libultim and MathLib.h under the installation prefix it gave, whatever
# mathlib the machine has. GAOL_FIND_MATHLIB is tested after the search too, so
# that a mathlib found by an earlier configuration, kept in the cache, is not
# used either.

if(GAOL_FIND_MATHLIB)
  find_path(MATHLIB_INCDIR MathLib.h HINTS "${MATHLIB_DIR}" PATH_SUFFIXES include)
  find_library(MATHLIB_LIB ultim HINTS "${MATHLIB_DIR}" PATH_SUFFIXES lib)
endif()

set(GAOL_MATHLIB "")
if(GAOL_FIND_MATHLIB AND MATHLIB_INCDIR AND MATHLIB_LIB)
  message(STATUS "Found mathlib: ${MATHLIB_LIB}")
  set(GAOL_MATHLIB "${MATHLIB_LIB}")
elseif(GAOL_BUILD_MATHLIB)
  if(GAOL_FIND_MATHLIB)
    message(STATUS "mathlib not found: it will be downloaded and built")
  else()
    message(STATUS "GAOL_FIND_MATHLIB is OFF: mathlib will be downloaded and built")
  endif()
  include(FetchContent)
  # The CMake build of cmake/mathlib/ exports libultim along with libgaol
  set(MATHLIB_EXPORT_NAME gaolTargets)
  set(_download_options "")
  if(NOT CMAKE_VERSION VERSION_LESS 3.24)
    list(APPEND _download_options DOWNLOAD_EXTRACT_TIMESTAMP TRUE)
  endif()
  # mathlib 2.1.1, as Frederic Goualard distributes it (GNU GPL v2 or later),
  # the archive GAOL's README points to; its checksum is checked. prepare.cmake
  # adds the CMake build and fixes bugs of mathlib. It is given its own
  # checksum: the patch step runs again when its command changes, not when the
  # script does, and a new prepare.cmake then reaches the mathlib already
  # downloaded in a build directory.
  set(_prepare "${CMAKE_CURRENT_SOURCE_DIR}/cmake/mathlib/prepare.cmake")
  file(SHA256 "${_prepare}" _prepare_sha256)
  set_property(DIRECTORY APPEND PROPERTY CMAKE_CONFIGURE_DEPENDS "${_prepare}")
  FetchContent_Declare(mathlib
    URL https://frederic.goualard.net/software/mathlib-2.1.1.tar.gz
    URL_HASH SHA256=f299848aa3e57ebb6248cd3cf54ecc7661a945aeac9e420e71db194965f87281
    ${_download_options}
    PATCH_COMMAND ${CMAKE_COMMAND} -DSOURCE_DIR=<SOURCE_DIR> -DPREPARE_SHA256=${_prepare_sha256} -P "${_prepare}"
  )
  FetchContent_MakeAvailable(mathlib)
  target_compile_options(ultim PRIVATE ${GAOL_INTERVAL_FLAGS})
  set(GAOL_MATHLIB ultim)
elseif(GAOL_FIND_MATHLIB)
  message(WARNING "mathlib not found and GAOL_BUILD_MATHLIB is OFF: link the ultim library along with GAOL.")
else()
  message(WARNING "GAOL_FIND_MATHLIB and GAOL_BUILD_MATHLIB are OFF: link the ultim library along with GAOL.")
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

# On x86 processors, with GAOL_SIMD (--enable-simd of configure), the intervals
# of GAOL are computed with SSE2 instructions (gaol/gaol_interval_sse.h), and
# SSE3 instructions serve gaol::interval2f (gaol/gaol_interval2f.h), as the
# autotools build on a processor with SSE3. Not with Visual C++, which IBEX and
# the fork of Fabrice Le Bars build GAOL without them, nor on 32-bit Windows:
# there, GCC takes the memory of new to be aligned on 16 bytes
# (__STDCPP_DEFAULT_NEW_ALIGNMENT__), while the C runtime aligns it on 8, so
# that a std::vector of SSE2 intervals, aligned on 16 bytes, crashes on its
# first movaps (MinGW-w64 15.2 for x86, rounding_direction). On 64-bit Windows
# (MinGW-w64, MSYS2 with GCC and Clang) the tests pass.
set(USING_SSE2_INSTRUCTIONS OFF)
set(USING_SSE3_INSTRUCTIONS OFF)
if(GAOL_SIMD AND NOT MSVC)
  set(CMAKE_REQUIRED_FLAGS "${GAOL_INTERVAL_FLAGS_STRING}")
  check_cxx_source_compiles("
    #if !(defined(__x86_64__) || defined(__i386__))
    #error GAOL computes its intervals with SSE2 on x86 processors only
    #endif
    #if defined(_WIN32) && !defined(_WIN64)
    #error not on 32-bit Windows
    #endif
    #include <emmintrin.h>
    int main() { return _mm_movemask_pd(_mm_set1_pd(-1.0)); }"
    GAOL_SSE2_INTERVALS)
  unset(CMAKE_REQUIRED_FLAGS)
  if(GAOL_SSE2_INTERVALS)
    check_cxx_compiler_flag(-msse2 GAOL_COMPILER_SUPPORTS_MSSE2)
    if(GAOL_COMPILER_SUPPORTS_MSSE2)
      set(USING_SSE2_INSTRUCTIONS ON)
      list(APPEND GAOL_INTERVAL_FLAGS -msse2)
      check_cxx_compiler_flag(-msse3 GAOL_COMPILER_SUPPORTS_MSSE3)
      if(GAOL_COMPILER_SUPPORTS_MSSE3)
        set(USING_SSE3_INSTRUCTIONS ON)
        list(APPEND GAOL_INTERVAL_FLAGS -msse3)
      endif()
    endif()
  endif()
endif()
string(REPLACE ";" " " GAOL_INTERVAL_FLAGS_STRING "${GAOL_INTERVAL_FLAGS}")
message(STATUS "Interval arithmetic flags: ${GAOL_INTERVAL_FLAGS_STRING}")
message(STATUS "SSE2 intervals: ${USING_SSE2_INSTRUCTIONS}, SSE3 (interval2f): ${USING_SSE3_INSTRUCTIONS}")

# GAOL_USING_ASM, as configure defines it by default: the assembly of
# gaol/gaol_fpu_fenv.h serves GCC and Clang on x86; the one for Visual C++
# only compiles for 32-bit x86, and is left out
if(GAOL_ASM AND NOT MSVC)
  set(GAOL_USING_ASM ON)
endif()

# The library compiled with hidden visibility, as configure builds it, and with
# the warnings of configure; GAOL's public declarations carry
# __attribute__ ((visibility("default"))) (HAVE_VISIBILITY_OPTIONS)
set(GAOL_LIBRARY_FLAGS "")
if(NOT MSVC)
  check_cxx_compiler_flag("-fvisibility-inlines-hidden -fvisibility=hidden" HAVE_VISIBILITY_OPTIONS)
  if(HAVE_VISIBILITY_OPTIONS)
    list(APPEND GAOL_LIBRARY_FLAGS "$<$<COMPILE_LANGUAGE:CXX>:-fvisibility-inlines-hidden>" -fvisibility=hidden)
  endif()
  foreach(flag -Wall -Wconversion)
    string(MAKE_C_IDENTIFIER "${flag}" _flag_id)
    string(TOUPPER "GAOL_COMPILER_SUPPORTS${_flag_id}" _flag_var)
    check_cxx_compiler_flag("${flag}" ${_flag_var})
    if(${_flag_var})
      list(APPEND GAOL_LIBRARY_FLAGS ${flag})
    endif()
  endforeach()
  # The optimizations configure adds to -O3, in the Release configuration
  foreach(flag -funroll-loops -fomit-frame-pointer -fexpensive-optimizations)
    string(MAKE_C_IDENTIFIER "${flag}" _flag_id)
    string(TOUPPER "GAOL_COMPILER_SUPPORTS${_flag_id}" _flag_var)
    check_cxx_compiler_flag("${flag}" ${_flag_var})
    if(${_flag_var})
      list(APPEND GAOL_LIBRARY_FLAGS "$<$<CONFIG:Release>:${flag}>")
    endif()
  endforeach()
endif()

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
if(USING_SSE3_INSTRUCTIONS)
  list(APPEND GAOL_SOURCES gaol_interval2f.cpp)
endif()
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
target_compile_options(gaol PRIVATE ${GAOL_LIBRARY_FLAGS})
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
