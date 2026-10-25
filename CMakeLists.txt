# ==================================================================
#  CMake build of GAOL
# ==================================================================
#
# GAOL, the interval arithmetic library written by Frederic Goualard
# (https://github.com/goualard-f/GAOL, GNU LGPL v2), comes with an autotools
# and a meson build, which GAOL v5 keeps. GAOL v5, written by Jordan Ninin,
# continues GAOL from its version 4.2.2. This CMake build is an addition of
# GAOL v5, written for Codac (https://github.com/codac-team/codac). See the
# section of README.md about GAOL v5.
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
# it compiles CORE-MATH, which GAOL computes its elementary functions with,
# into libgaol, from the sources of 3rd/math-core. It also builds as a part of
# another project, brought in with FetchContent.
#
# Copyright (c) 2026 ENSTA, France
#
# Created 2026-09-20 by Jordan NININ

cmake_minimum_required(VERSION 3.14...3.25)

# The version of GAOL is the line of the file VERSION.txt, which configure.ac
# and meson.build read too: project() gives it to PROJECT_VERSION, whence the
# macros GAOL_*VERSION, the shared library, gaol.pc, gaolConfigVersion.cmake
# and the names of the packages. Three numbers without leading zeros, which the
# macros write as C integers. The file is not named VERSION: the root of the
# sources is on the include path, and on file systems that ignore case (macOS,
# Windows) #include <version>, which libc++ includes from <vector> or <cmath>,
# would find it. The reading, which ignores a byte order mark and the line ends
# of Windows, is in cmake/gaol_version.cmake, where the test version_file
# checks it (GAOL v5)
include("${CMAKE_CURRENT_SOURCE_DIR}/cmake/gaol_version.cmake")
gaol_read_version("${CMAKE_CURRENT_SOURCE_DIR}/VERSION.txt" _gaol_version _gaol_version_error)
if(_gaol_version_error)
  message(FATAL_ERROR "${_gaol_version_error}")
endif()

# Whether the configurations of a generator of several were given, on the
# command line or in the environment, before project() sets their default
if(DEFINED CMAKE_CONFIGURATION_TYPES OR DEFINED ENV{CMAKE_CONFIGURATION_TYPES})
  set(_gaol_configuration_types_given ON)
else()
  set(_gaol_configuration_types_given OFF)
endif()

project(gaol VERSION ${_gaol_version} LANGUAGES C CXX)

# A change of VERSION.txt configures again
set_property(DIRECTORY APPEND PROPERTY CMAKE_CONFIGURE_DEPENDS "${CMAKE_CURRENT_SOURCE_DIR}/VERSION.txt")

include(GNUInstallDirs)
include(CMakePackageConfigHelpers)
include(CheckCCompilerFlag)
include(CheckCSourceCompiles)
include(CheckCXXCompilerFlag)
include(CheckCXXSourceCompiles)
include(CheckCXXSourceRuns)
include(CheckIncludeFileCXX)
include(CheckSymbolExists)

set(GAOL_IS_TOP_LEVEL OFF)
if(CMAKE_SOURCE_DIR STREQUAL CMAKE_CURRENT_SOURCE_DIR)
  set(GAOL_IS_TOP_LEVEL ON)
  if(NOT CMAKE_BUILD_TYPE AND NOT CMAKE_CONFIGURATION_TYPES)
    set(CMAKE_BUILD_TYPE Release CACHE STRING "Build type" FORCE)
  endif()
  # With a generator of several configurations (Visual Studio, Xcode, Ninja
  # Multi-Config), Release first among them, unless they were given: Ninja
  # Multi-Config builds the first by default. cmake --build with Visual Studio
  # builds Debug all the same without --config (GAOL v5)
  get_property(_gaol_multi_config GLOBAL PROPERTY GENERATOR_IS_MULTI_CONFIG)
  if(_gaol_multi_config AND NOT _gaol_configuration_types_given AND "Release" IN_LIST CMAKE_CONFIGURATION_TYPES)
    set(_gaol_types ${CMAKE_CONFIGURATION_TYPES})
    list(REMOVE_ITEM _gaol_types Release)
    list(INSERT _gaol_types 0 Release)
    set(CMAKE_CONFIGURATION_TYPES "${_gaol_types}" CACHE STRING "The configurations of the generator" FORCE)
  endif()
endif()

# libgaol, a static library, is compiled as position-independent
# code, so that they can be linked into shared libraries (Python bindings,
# plugins), unless the project building GAOL, or the command line, sets
# CMAKE_POSITION_INDEPENDENT_CODE, which is then followed. Brought in by
# FetchContent by a project that did not set it, GAOL was compiled without
# -fPIC, and a shared library linking gaol::gaol failed to link ("relocation
# R_X86_64_PC32 against symbol ... can not be used when making a shared object;
# recompile with -fPIC"). On x86_64, where GCC compiles with -fPIE by default,
# -fPIC changed the times of doc/compare by -5 to +2.4 %, the noise of the
# measure. The variable is set for this directory and tests/ only.
if(NOT DEFINED CMAKE_POSITION_INDEPENDENT_CODE)
  set(CMAKE_POSITION_INDEPENDENT_CODE ON)
endif()

option(GAOL_PRESERVE_ROUNDING "Restore the rounding direction found after each operation, rather than leaving it upward" OFF)
# The unit tests of tests/ and the examples of examples/, built by the build
# and run by make test (the tests) and make check (the tests and the
# examples), as --with-tests and --with-examples of configure, with-tests and
# with-examples of meson (GAOL v5; GAOL_BUILD_TESTS before). Only where GAOL
# is the project built: a project bringing GAOL in (add_subdirectory,
# FetchContent) may have options of the same names, for its own tests.
option(WITH_TESTS "Build the unit tests of tests/, which make test (ctest) and make check run" OFF)
option(WITH_EXAMPLES "Build the examples of examples/, which make check runs" OFF)
# libgaol is a static library, or with BUILD_SHARED_LIBS=ON, the variable of
# CMake, a shared one, libgaol.so.5.0.0 as with configure and meson (GAOL v5).
# Always static with Visual C++, whose declarations of GAOL carry no
# __declspec(dllexport).
# The options of configure (--enable-simd, --enable-arm64,
# --enable-verbose-mode), with its defaults, but for the verbose mode: GAOL is
# silent by default
option(GAOL_SIMD "Compute the intervals with SSE2 instructions on x86 processors" ON)
# On 64-bit ARM processors, the intervals are computed with NEON instructions
# (--enable-arm64 of configure, enable-arm64 of meson; GAOL v5). Independent of
# GAOL_SIMD, which stays for the x86 processors. Off, they are computed with
# the FPU, as before.
option(GAOL_ARM64 "Compute the intervals with NEON instructions on 64-bit ARM processors" ON)
option(GAOL_VERBOSE_MODE "Write a line on the standard error when GAOL initializes and cleans up" OFF)
# GAOL and CORE-MATH built for debugging whatever the build type or the
# configuration, a project bringing GAOL in included, as enable-debug of meson
# and --enable-debug of configure (GAOL v5; see the end of this file)
option(GAOL_DEBUG "Build GAOL and CORE-MATH for debugging, without optimization and with GAOL_DEBUGGING, whatever the build type or the configuration" OFF)
# No option for the intervals of floats, gaol::intervalf and gaol::interval2f,
# unfinished, which only a developer of GAOL compiles (see gaol/gaol_config.h)
# Coverage: the library and the tests compiled with the counters of gcov
# (--coverage), and the target "coverage", which runs the tests and writes the
# report in coverage/ (see doc/coverage.md). GCC and Clang only; the build has
# to be a Debug or an unoptimized one for the counts to follow the source.
# The 128-bit integer of gaol/gaol_u128.h computed with two 64-bit halves even
# where the compiler has a 128-bit type: the code Visual C++ and the 32-bit
# targets of GCC take, run on an ordinary machine (tests/u128.cpp compares the
# two paths, and the continuous integration builds GAOL this way)
# Interprocedural optimization (link-time optimization) is not offered
#
# It makes GAOL's basic operations two to three times faster where the code
# using GAOL is compiled with it too (an addition of intervals took 0.99 ns
# rather than 2.68 on an Intel i7-1185G7 with Clang 18.1 at -O3) and the
# elementary functions two to three times slower (cos 169 ns rather than 58),
# and **with GCC it breaks the bounds**: GAOL then fails
# tests/rounding_direction.cpp, which checks that an operation gives the same
# result whatever the rounding direction the calling code left. With
# GAOL_PRESERVE_ROUNDING, textToInterval("sin(1)+exp(0.1)") gave an upper bound
# one double below the right one when the caller left the direction downward or
# toward zero: the compiler moves floating-point operations across the changes
# of rounding direction, which -frounding-math is meant to forbid and which it
# does not across translation units. Clang 18.1 passes the tests with it, so
# this is one compiler's handling of the option rather than the option itself
# -- which is reason enough not to offer it.
#
# GAOL's whole point is that its intervals enclose the exact values, so there is
# no option for this (GAOL v5, see doc/three-builds.md). The sources of
# CORE-MATH are compiled apart all the same, which keeps a -flto given by hand
# away from them.


option(GAOL_U128_EMULATION "Compute the 128-bit integer of the accurate phases with two 64-bit halves, as where the compiler has no 128-bit type" OFF)

option(GAOL_COVERAGE "Compile GAOL and its tests with the counters of gcov, and add the target coverage, which runs the tests and writes the report in coverage/" OFF)

option(GAOL_FMA "Compile GAOL and CORE-MATH with the fused multiply-add instructions of the processor, where the compiler has a flag for them (OFF: a library for any processor of the architecture)" ON)

# GAOL_PREFER_AVX512, whether +, -, *, /, sqr, sqrt and, with FMA, the integer
# powers beyond 1 and -1 take the AVX-512 path, where the compiler takes it and
# the machine building runs it (GAOL_AVX512_ARITHMETIC below). ON by default
# where GAOL_PRESERVE_ROUNDING is: the path spares the store of the rounding
# direction every operation pays, which is what GAOL_PRESERVE_ROUNDING is
# about. What the command line
# gave wins either way: -DGAOL_PREFER_AVX512=OFF refuses the path under
# GAOL_PRESERVE_ROUNDING, as -DGAOL_PREFER_AVX512=ON takes it without it.
# The default derives at every configure where the command line said
# nothing, from GAOL_PRESERVE_ROUNDING above (GAOL v5).
#
# The entry cannot say by itself whether the command line gave it (GAOL v5,
# point 112 of TODO.md): option() made it at the first configure, whatever the
# default derived, and DEFINED took it at the second for a request, so that a
# build directory configured with GAOL_PRESERVE_ROUNDING lost the AVX-512 path
# at its second cmake. The entry now holds the value the configure derived,
# and the internal entry GAOL_PREFER_AVX512_DERIVED the same value. While the
# entry still holds it, it is a leftover, and the default is derived again.
# Another value is a request, and so is the help string of the command line,
# "No help, variable specified on the command line.", which CMake gives an
# entry that -D sets, even to the value it had (a fact of CMake that its
# documentation does not state: without it, the value alone tells). A request
# drops the record and stays the command line's until -UGAOL_PREFER_AVX512.
#
# Limits. (1) A build directory configured before this holds the OFF that
# option() cached, a request of OFF: -UGAOL_PREFER_AVX512 derives the default
# again. (2) A request that is not a -D (the value edited back to the one last
# derived, in a graphical interface) and equals the record is read as the
# default. (3) A normal variable of a project that brings GAOL in
# (add_subdirectory, FetchContent) hides the entry, and is a request only if
# it differs from the record: one set to the value last derived, in a
# configure that also changes the default (GAOL_PRESERVE_ROUNDING), is read as
# the default, where a variable set from the first configure on is a request.
set(GAOL_PREFER_AVX512_HELP "Have +, -, *, /, sqr, sqrt and, with FMA, the integer powers beyond 1 and -1 take the AVX-512 instructions and the rounding direction they carry in themselves, which sets no rounding direction, where the machine building runs them: the library then needs a processor with them (OFF: the operations set the direction upward; ON by default with GAOL_PRESERVE_ROUNDING)")
function(gaol_keep_prefer_avx512 value)
  set(GAOL_PREFER_AVX512 "${value}" CACHE BOOL "${GAOL_PREFER_AVX512_HELP}" FORCE)
  set(GAOL_PREFER_AVX512_DERIVED "${value}" CACHE INTERNAL "The value of GAOL_PREFER_AVX512 the last configure derived (GAOL v5): the entry is a request unless it still holds it")
endfunction()
set(GAOL_PREFER_AVX512_EXPLICIT OFF)
if(DEFINED GAOL_PREFER_AVX512)
  set(GAOL_PREFER_AVX512_EXPLICIT ON)
  get_property(_gaol_help CACHE GAOL_PREFER_AVX512 PROPERTY HELPSTRING)
  if(DEFINED GAOL_PREFER_AVX512_DERIVED
     AND "${GAOL_PREFER_AVX512}" STREQUAL "${GAOL_PREFER_AVX512_DERIVED}"
     AND NOT "${_gaol_help}" STREQUAL "No help, variable specified on the command line.")
    set(GAOL_PREFER_AVX512_EXPLICIT OFF)
  endif()
endif()
if(GAOL_PREFER_AVX512_EXPLICIT)
  unset(GAOL_PREFER_AVX512_DERIVED CACHE)
  option(GAOL_PREFER_AVX512 "${GAOL_PREFER_AVX512_HELP}" OFF)
else()
  if(GAOL_PRESERVE_ROUNDING)
    set(_gaol_prefer_avx512 ON)
  else()
    set(_gaol_prefer_avx512 OFF)
  endif()
  gaol_keep_prefer_avx512(${_gaol_prefer_avx512})
  set(GAOL_PREFER_AVX512 ${_gaol_prefer_avx512})
endif()

# As configure.ac and meson.build ask: -std=c++11; C99 for the sources of
# CORE-MATH, with the GNU extensions (CMake's default), which the inline
# assembly of atan2 needs (see 3rd/README.md)
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
# CORE-MATH, and to the targets linking gaol::gaol, GAOL's headers being full of
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

# mingw-w64 is refused by gaol/gaol_config.h only, when compiling, where the
# fma() or round() of the programs is wrong. On x86-64, before version 12 (the
# MinGW-w64 GCC 11 to 13 of Chocolatey) and linked with msvcrt.dll rather than
# the UCRT (MSYS2 MINGW64, the cross compilers of Debian and Ubuntu), they are
# those of mingw-w64's own math library, computed in doubles: that fma() is not
# correctly rounded, and CORE-MATH's functions compute with it where the
# compiler has no -mfma (GAOL_FMA OFF, GCC for 32-bit Windows, see below);
# that round() depends on the rounding direction. On 32-bit x86, where
# mingw-w64 computes them on the x87 unit, before version 11, as the
# mingw-w64 9 of WinLibs (GCC 11), whose fma() rounds each of its partial sums
# to a double (the one of mingw-w64 11, accepted, keeps them in extended
# precision and passes the tests, though not correctly rounded). On ARM, as on
# x86-64, before version 12 or linked with msvcrt.dll, the refusal not being
# tested there (MSYS2 CLANGARM64, which links the UCRT, is built and tested).
# From version 12 on, a toolchain linking the UCRT takes those of
# ucrtbase.dll, which pass the tests.
#
# Two reasons had GAOL refuse the MinGW-w64 of Chocolatey before version 15,
# and both are gone now that GAOL bounds its elementary functions with
# CORE-MATH (GAOL v5):
#
# - the math library of mingw-w64 before version 12 was not accurate enough for
#   the hyperbolic functions GAOL took from it (acosh() near 1 was up to 25
#   million floats away from the exact value, and asinh() of large negative
#   numbers NaN). GAOL takes no elementary function from the math library of
#   the system any more: CORE-MATH gives them all, correctly rounded, whatever
#   the system;
# - fesetround() of mingw-w64 12 runs the instruction cpuid at each call, which
#   takes microseconds in a virtual machine (3.5 against 130 nanoseconds with
#   mingw-w64 13), and GAOL changed the rounding direction four times for each
#   exp(), log(), sin() or cos() of an interval. It changes it no more: the
#   functions of CORE-MATH are correctly rounded in the direction in effect, so
#   GAOL calls them in the upward rounding it keeps (gaol/gaol_double_op.h).
#
# The continuous integration builds GAOL with the MinGW-w64 GCC 14.2.0 and
# 15.2.0 of Chocolatey on x86-64, and 12.2.0 to 15.2.0 on 32-bit x86, and with
# MSYS2 UCRT64, CLANG64 and CLANGARM64, runs the tests with them, and checks that GCC
# 11.2.0 to 13.2.0 and MSYS2 MINGW64 are refused on x86-64 and GCC 11.2.0 on
# 32-bit x86 (.github/workflows/windows.yml).

set(GAOL_INTERVAL_FLAGS "")
if(MSVC)
  list(APPEND GAOL_INTERVAL_FLAGS /fp:strict)
  # clang-cl for 32-bit x86 computes doubles on the x87 unit, even with
  # /arch:SSE2 (_M_IX86_FP 1, clang-cl 18), which gaol/gaol_config.h refuses:
  # -msse2 has it compute them in SSE2 (_M_IX86_FP 2, FLT_EVAL_METHOD 0), as
  # Visual C++ does by default. Given to the code using GAOL too (gaol::gaol),
  # which includes the headers the check is in.
  if(CMAKE_CXX_COMPILER_ID STREQUAL "Clang" AND CMAKE_SIZEOF_VOID_P EQUAL 4
     AND CMAKE_CXX_COMPILER_ARCHITECTURE_ID MATCHES "^[Xx]86$")
    list(APPEND GAOL_INTERVAL_FLAGS -msse2)
  endif()
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
  # x87. CORE-MATH assumes every operation on doubles rounded to a double
  # (FLT_EVAL_METHOD 0, see 3rd/math-core/README.md). Computed on the x87, in
  # extended precision, its results rounded to nearest are rounded twice and
  # 175 arguments gave the other neighbour of f(x) (Debian 12 i386, GCC 12);
  # GCC 9 rounded to nearest at compile time the constants CORE-MATH rounds in
  # the direction in effect, and GAOL's bounds of exp2(-1075), expm1(-800) or
  # atan2() of a tiny and a huge number did not enclose the exact values
  # (tests/extended_precision.cpp). Visual C++ computes in SSE2 already.
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
  # GAOL refuses -ffast-math, -ffinite-math-only and doubles computed on the
  # x87 unit at compile time (gaol/gaol_config.h): the flags above keep them away.

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
  # 32-bit ARM processors above. The C compiler is checked as well as the C++
  # one: it builds CORE-MATH (GAOL v5).
  set(_gaol_rounding_math_hint
    "This compiler does not honour the rounding direction on this target, and says so of -frounding-math "
    "(\"overriding currently unsupported rounding mode on this target\"). GAOL computes with the rounding "
    "direction set upward, so that the intervals it computes would not enclose the values they should "
    "(built by Clang 14 for 64-bit ARM processors, bounds of pow() and nth_root() did not). The C compiler "
    "builds CORE-MATH and the C++ compiler the rest of GAOL: neither may be such a compiler.\n"
    "Build GAOL with a compiler that honours the rounding direction on this target, such as GCC or a later "
    "version of Clang (Clang 18 does on 64-bit ARM processors), in a new build directory.")
  if(GAOL_COMPILER_SUPPORTS_FROUNDING_MATH)
    set(CMAKE_REQUIRED_FLAGS "-frounding-math")
    check_cxx_source_compiles("int main() { return 0; }" GAOL_COMPILER_HONOURS_ROUNDING_MATH
                              FAIL_REGEX "unsupported rounding mode")
    unset(CMAKE_REQUIRED_FLAGS)
    if(NOT GAOL_COMPILER_HONOURS_ROUNDING_MATH)
      message(FATAL_ERROR
        "GAOL cannot be built with the C++ compiler ${CMAKE_CXX_COMPILER} (${CMAKE_CXX_COMPILER_ID} ${CMAKE_CXX_COMPILER_VERSION}) "
        "for this target.\n" ${_gaol_rounding_math_hint})
    endif()
  endif()
  check_c_compiler_flag(-frounding-math GAOL_C_COMPILER_SUPPORTS_FROUNDING_MATH)
  if(GAOL_C_COMPILER_SUPPORTS_FROUNDING_MATH)
    set(CMAKE_REQUIRED_FLAGS "-frounding-math")
    check_c_source_compiles("int main(void) { return 0; }" GAOL_C_COMPILER_HONOURS_ROUNDING_MATH
                            FAIL_REGEX "unsupported rounding mode")
    unset(CMAKE_REQUIRED_FLAGS)
    if(NOT GAOL_C_COMPILER_HONOURS_ROUNDING_MATH)
      message(FATAL_ERROR
        "GAOL cannot be built with the C compiler ${CMAKE_C_COMPILER} (${CMAKE_C_COMPILER_ID} ${CMAKE_C_COMPILER_VERSION}) "
        "for this target.\n" ${_gaol_rounding_math_hint})
    endif()
  endif()
endif()
string(REPLACE ";" " " GAOL_INTERVAL_FLAGS_STRING "${GAOL_INTERVAL_FLAGS}")

# The fused multiply-add instructions of the processor (GAOL v5), with
# the flag that gives them to the compiler where it has one, as Codac takes
# them: GAOL's exact products (fma_once() in gaol/gaol_interval_core.cpp, for the
# integer powers and the reduction of sin and cos) and CORE-MATH's functions
# then compute fma() with one instruction, rather than with a call to the
# fma() of the math library. The flags are given to the code using GAOL too
# (gaol::gaol, gaol.pc), as Codac gives its own: that code runs only where
# GAOL's library runs, on a processor with these instructions, and is compiled
# for the same processor, the inline operations of GAOL's headers included.
# -ffp-contract=off stays: no multiplication and addition written apart are
# contracted into a fused one, which would round them as one operation.
#  - x86 and x86_64, GCC and Clang: -mfma, where it defines __FMA__ (Clang
#    takes -mfma for a target without these instructions, and ignores it).
#  - Visual C++ for x64: /arch:AVX2, which implies them. Not for 32-bit x86:
#    there, Visual C++ with /arch:AVX2 reused a product rounded upward after
#    the rounding direction had changed, in spite of /fp:strict (in Codac,
#    with Visual Studio 2022 and 2026).
#  - GCC for Windows (MinGW-w64, MSYS2, Cygwin) for x86-64 and 32-bit x86: -mfma
#    -mprefer-vector-width=128 (GAOL v5, point 110 of TODO.md). -mfma turns AVX
#    on, and GCC then vectorizes with the 256-bit registers (ymm), which it may
#    spill to a stack it takes to be aligned on 32 bytes, where the Windows x64
#    ABI aligns it on 16 (GCC bug 54412: an aligned vmovaps to such a slot
#    faults; crashes in Codac's MinGW builds). -mprefer-vector-width=128 keeps
#    the vectorizer to the 128-bit registers: objdump found no %ymm in the
#    objects of GAOL, CORE-MATH and the tests built so by GCC 16.2 (MSYS2
#    UCRT64), against 30598 lines with -mfma alone: the objects hold no 256-bit
#    register for the defect to act on, which windows.yml checks on the objects
#    it builds. Without these flags CORE-MATH called a software fma()
#    and the integer powers had no FMA kernel: pow(x, 3) took 52.3 ns rather
#    than 8.7, log 106.4 rather than 33.6, cos 260.5 rather than 55.4 (medians
#    of 3 rounds alternated in one job, GitHub runner, MSYS2 UCRT64 GCC 16.2).
#    The option is from GCC 8, and a GCC without it gets no flag. For 32-bit
#    x86 too (GAOL v5; MinGW-w64 15.2 and 14.2 for i686 on the runners of
#    GitHub, the 38 tests passing in each variant): pow(x, 3) took 70.5 ns
#    rather than 12.2, log 265.7 rather than 81.2 and cos 489.6 rather than
#    138.2 (15.2). The objects held 4709 lines with %ymm with -mfma alone (600
#    with 14.2) and none with the two options, at the same speed (GCC 13.2:
#    9 lines in two test objects, with both); that bug
#    54412 acts on x86 too was not shown (GCC realigns the stack itself for
#    the 256-bit spills it makes for 32-bit Linux), and the two options are
#    given together there so that the objects hold no 256-bit register.
#    -mstackrealign, for the 4-byte stack of Windows threads (GCC bug 40838),
#    changed no time with 15.2, and a program that runs pow, exp, log, sin
#    and cos on the stack of a Win32 thread and of a std::thread gave main's
#    results with and without -mfma. Code using GAOL that writes 256-bit
#    intrinsics itself stays exposed to the defect on x64, as with -mavx.
#  - 32-bit ARM: -mfpu=neon-vfpv4 -mfloat-abi=hard where FMA is not on
#    already (__ARM_FEATURE_FMA), and the libraries of the system take the
#    hard-float ABI.
#  - 64-bit ARM, POWER, s390x, RISC-V: none; fma() is an instruction of the
#    base architecture.
# A library so compiled needs a processor with these instructions (on x86,
# Intel Haswell and AMD Piledriver, 2012-2013, and later ones): GAOL_FMA OFF
# leaves them out, for a library running on any processor of the architecture.
# They are also left out where the machine building does not run a program
# compiled with them, but when cross-compiling: a compiler takes the flag
# whatever the processor, and under Rosetta on macOS 14, which has no AVX, the
# x86_64 programs of the tests stopped on an illegal instruction.
set(GAOL_FMA_FLAGS "")
if(GAOL_FMA)
  if(MSVC)
    check_cxx_source_compiles("
      #if defined(_M_IX86)
      #error /arch:AVX2 breaks the rounding direction with Visual C++ for 32-bit x86
      #endif
      int main() { return 0; }"
      GAOL_MSVC_TARGET_KEEPS_ROUNDING_WITH_AVX2)
    if(GAOL_MSVC_TARGET_KEEPS_ROUNDING_WITH_AVX2)
      check_cxx_compiler_flag(/arch:AVX2 GAOL_COMPILER_SUPPORTS_ARCH_AVX2)
      if(GAOL_COMPILER_SUPPORTS_ARCH_AVX2)
        list(APPEND GAOL_FMA_FLAGS /arch:AVX2)
      endif()
    endif()
  elseif((WIN32 OR CYGWIN) AND CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
    check_cxx_source_compiles("
      #if !defined(__x86_64__) && !defined(__i386__)
      #error not an x86 target
      #endif
      int main() { return 0; }"
      GAOL_GCC_WINDOWS_TARGET_IS_X86)
    if(GAOL_GCC_WINDOWS_TARGET_IS_X86)
      # Both options, or none: -mfma alone lets GCC spill 256-bit registers
      # to a stack it does not align on Windows (see above)
      set(CMAKE_REQUIRED_FLAGS "${GAOL_INTERVAL_FLAGS_STRING} -mfma -mprefer-vector-width=128")
      check_cxx_source_compiles("
        #if !defined(__FMA__)
        #error -mfma does not give the fused multiply-add instructions on this target
        #endif
        int main() { return 0; }"
        GAOL_GCC_WINDOWS_TAKES_MFMA_VECTOR_WIDTH_128)
      unset(CMAKE_REQUIRED_FLAGS)
      if(GAOL_GCC_WINDOWS_TAKES_MFMA_VECTOR_WIDTH_128)
        list(APPEND GAOL_FMA_FLAGS -mfma -mprefer-vector-width=128)
      else()
        message(STATUS "Fused multiply-add instructions left out: this GCC for Windows takes no -mfma -mprefer-vector-width=128 (the option is from GCC 8), and -mfma alone leaves its 256-bit registers on a stack it does not align")
      endif()
    else()
      message(STATUS "Fused multiply-add instructions left out: GCC for Windows takes them for x86 only")
    endif()
  else()
    set(CMAKE_REQUIRED_FLAGS "${GAOL_INTERVAL_FLAGS_STRING} -mfma")
    check_cxx_source_compiles("
      #if !defined(__FMA__)
      #error -mfma does not give the fused multiply-add instructions on this target
      #endif
      int main() { return 0; }"
      GAOL_COMPILER_SUPPORTS_MFMA)
    unset(CMAKE_REQUIRED_FLAGS)
    if(GAOL_COMPILER_SUPPORTS_MFMA)
      list(APPEND GAOL_FMA_FLAGS -mfma)
    else()
      check_cxx_source_compiles("
        #if !defined(__arm__) || defined(__ARM_FEATURE_FMA)
        #error not a 32-bit ARM target without FMA
        #endif
        int main() { return 0; }"
        GAOL_TARGET_IS_ARM32_WITHOUT_FMA)
      if(GAOL_TARGET_IS_ARM32_WITHOUT_FMA)
        set(CMAKE_REQUIRED_FLAGS "${GAOL_INTERVAL_FLAGS_STRING} -mfpu=neon-vfpv4 -mfloat-abi=hard")
        check_cxx_source_compiles("
          #if !defined(__ARM_FEATURE_FMA)
          #error -mfpu=neon-vfpv4 does not give the fused multiply-add instructions
          #endif
          int main() { return 0; }"
          GAOL_COMPILER_SUPPORTS_MFPU_NEON_VFPV4)
        unset(CMAKE_REQUIRED_FLAGS)
        if(GAOL_COMPILER_SUPPORTS_MFPU_NEON_VFPV4)
          list(APPEND GAOL_FMA_FLAGS -mfpu=neon-vfpv4 -mfloat-abi=hard)
        endif()
      endif()
    endif()
  endif()
  if(GAOL_FMA_FLAGS AND NOT CMAKE_CROSSCOMPILING)
    # fma(a, b, -1) is -2^-60 exactly, which a product rounded first gives as 0.
    # __builtin_fma, not std::fma: the test is built without optimization, and
    # std::fma is then a call of the fma() of the library, which is not the
    # instruction to run (and that of Cygwin gives 0: GAOL v5)
    string(REPLACE ";" " " _gaol_fma_flags "${GAOL_FMA_FLAGS}")
    set(CMAKE_REQUIRED_FLAGS "${GAOL_INTERVAL_FLAGS_STRING} ${_gaol_fma_flags}")
    check_cxx_source_runs("
      #include <cmath>
      #if defined(__GNUC__)
      #  define GAOL_TEST_FMA(a, b, c) __builtin_fma(a, b, c)
      #else
      #  define GAOL_TEST_FMA(a, b, c) std::fma(a, b, c)
      #endif
      int main() {
        volatile double a = 1.0 + 1.0 / 1073741824.0, b = 1.0 - 1.0 / 1073741824.0, c = -1.0;
        return GAOL_TEST_FMA(a, b, c) == -1.0 / 1152921504606846976.0 ? 0 : 1;
      }"
      GAOL_MACHINE_RUNS_FMA)
    unset(CMAKE_REQUIRED_FLAGS)
    if(NOT GAOL_MACHINE_RUNS_FMA)
      message(STATUS "Fused multiply-add instructions left out: this machine does not run a program compiled with ${_gaol_fma_flags}")
      set(GAOL_FMA_FLAGS "")
    endif()
  endif()
endif()
string(REPLACE ";" " " GAOL_FMA_FLAGS_STRING "${GAOL_FMA_FLAGS}")
message(STATUS "Fused multiply-add flags: ${GAOL_FMA_FLAGS_STRING}")

# The link option -mno-daz-ftz, where the compiler takes it (GCC 13 and later on
# x86, and from 11.4 and 12.4 in the series 11 and 12; GCC 9.4, 12.3, GCC for
# ARM and Clang 18 refuse it, with an error), which the code using GAOL is given
# (gaol::gaol, gaol.pc)
# (GAOL v5). GCC links crtfastmath.o into a program linked with -Ofast,
# -ffast-math or -funsafe-math-optimizations, even when -fno-fast-math follows,
# and its constructor sets the flush-to-zero and denormals-are-zero modes of the
# SSE instructions, which make the bounds of every operation with a subnormal
# operand or result wrong ([1e-300]*[1e-20] is [0, 0]). -mno-daz-ftz keeps that
# file out of the link. It is no protection against a plug-in built with -Ofast
# and loaded later, nor against the other compilers and processors: each
# operation of GAOL clears the modes when it starts (gaol/gaol_fpu.h). Checked
# by linking, since it is a link option (tests/fast_math_link.cpp).
set(GAOL_LINK_FLAGS "")
if(NOT MSVC)
  set(CMAKE_REQUIRED_LINK_OPTIONS "-mno-daz-ftz")
  check_cxx_source_compiles("int main() { return 0; }" GAOL_LINKER_TAKES_MNO_DAZ_FTZ)
  unset(CMAKE_REQUIRED_LINK_OPTIONS)
  if(GAOL_LINKER_TAKES_MNO_DAZ_FTZ)
    list(APPEND GAOL_LINK_FLAGS -mno-daz-ftz)
  endif()
endif()
string(REPLACE ";" " " GAOL_LINK_FLAGS_STRING "${GAOL_LINK_FLAGS}")
message(STATUS "Link flags of the code using GAOL: ${GAOL_LINK_FLAGS_STRING}")


################################################################################
# CORE-MATH
################################################################################
#
# GAOL bounds every one of its elementary functions with CORE-MATH
# (https://core-math.gitlabpages.inria.fr, MIT licence), whose sources are in
# 3rd/math-core and are compiled into libgaol itself: there is no library to
# find, to build apart or to install, and nothing to link along with GAOL.
#
# CORE-MATH is correctly rounded in the rounding direction in effect, so GAOL
# takes its values as bounds without moving them outward and without switching
# the rounding direction (see gaol/gaol_double_op.h), and the bounds are the
# same on every architecture and with every compiler.
#
# Each source is compiled with gaol/core_math_port.h included first, by the
# compiler rather than by the source (-include, /FI), so that importing a newer
# CORE-MATH stays a plain copy: that header gives them the names GAOL uses, the
# 128-bit integer of gaol/gaol_u128.h where the compiler has none (Visual C++,
# and GCC for a 32-bit target), and what Visual C++ has not of GCC.

set(GAOL_CORE_MATH_FUNCTIONS
  exp log pow sin cos tan asin acos atan atan2
  sinh cosh tanh asinh acosh atanh
  # cbrt, which nth_root(x, 3) takes, and the exponentials and logarithms in
  # base 2 and 10, which IEEE 1788-2015 requires (Table 9.1) (GAOL v5)
  cbrt exp2 exp10 log2 log10
  # The forward functions IEEE 1788-2015 recommends (Table 10.5), which
  # avoid the loss of accuracy of pi being irrational for the *Pi family
  # and of cancellation for the m1/p1 family (GAOL v5)
  expm1 exp2m1 exp10m1 sinpi cospi tanpi acospi atanpi
  log1p log2p1 log10p1 hypot rsqrt asinpi atan2pi
  # The error function and its complement, which IEEE 1788-2015 does not
  # name (GAOL v5)
  erf erfc)

set(GAOL_CORE_MATH_SOURCES "")
foreach(_f IN LISTS GAOL_CORE_MATH_FUNCTIONS)
  set(_src "${CMAKE_CURRENT_SOURCE_DIR}/3rd/math-core/src/binary64/${_f}/${_f}.c")
  if(NOT EXISTS "${_src}")
    message(FATAL_ERROR "The source of CORE-MATH's ${_f} is missing: ${_src}")
  endif()
  list(APPEND GAOL_CORE_MATH_SOURCES "${_src}")
endforeach()

if(MSVC)
  set(GAOL_CORE_MATH_FLAGS "/FI${CMAKE_CURRENT_SOURCE_DIR}/gaol/core_math_port.h")
else()
  set(GAOL_CORE_MATH_FLAGS "-include;${CMAKE_CURRENT_SOURCE_DIR}/gaol/core_math_port.h")
  # CORE-MATH asks for its computations to be rounded to double precision
  # (FLT_EVAL_METHOD = 0, see 3rd/math-core/README.md). GAOL gets that on x86
  # by computing with SSE2, which gaol/gaol_config.h refuses to do without, and
  # this asks the compiler for it as well, where it has the flag. It is for C
  # only: GCC answers "sorry, unimplemented" for C++.
  check_c_compiler_flag("-fexcess-precision=standard" GAOL_C_SUPPORTS_EXCESS_PRECISION)
  if(GAOL_C_SUPPORTS_EXCESS_PRECISION)
    list(APPEND GAOL_CORE_MATH_FLAGS "-fexcess-precision=standard")
  endif()
endif()

message(STATUS "Elementary functions: CORE-MATH, from 3rd/math-core (correctly rounded)")


################################################################################
# What the compiler provides
################################################################################

# GAOL sets the rounding direction with <fenv.h> (gaol/gaol_fpu.h)
check_include_file_cxx(fenv.h GAOL_HAVE_FENV_H)
if(NOT GAOL_HAVE_FENV_H)
  message(FATAL_ERROR "GAOL needs <fenv.h>, which this compiler does not provide")
endif()

# GAOL raises exceptions to signal errors, which --disable-exceptions of
# configure and -Denable-exception=false of meson turn off
set(GAOL_EXCEPTIONS_ENABLED ON)

# The Debug build has GAOL check its assertions (assert), as
# --enable-debug of configure and --buildtype=debug of meson (GAOL v5). With a
# generator of several configurations (Visual Studio, Xcode), whose
# configurations share gaol/gaol_configuration.h, GAOL_DEBUGGING is given to
# the sources of GAOL in the Debug configuration below.
if(CMAKE_BUILD_TYPE STREQUAL "Debug" OR GAOL_DEBUG)
  set(GAOL_DEBUGGING ON)
endif()

# On x86 processors, with GAOL_SIMD (--enable-simd of configure), the intervals
# of GAOL are computed with SSE2 instructions (gaol/gaol_interval_sse.h), and
# -msse3 is given where the compiler takes it, as the autotools build on a
# processor with SSE3. Visual C++ and clang-cl take them on x64 (GAOL v5),
# where SSE2 is the baseline of the processor and no flag is to be given
# (cl does not know -msse2): gaol/gaol_interval_sse.cpp is written for the
# __m128d of Visual C++, a structure and not a vector, and needs no asm
# statement there (clang-cl keeps the ones of Clang).
#
# On 32-bit Windows (GAOL v5), the C runtime aligns the memory of malloc() and
# new on 8 bytes, and an SSE2 interval, a __m128d, needs 16: GCC takes the
# memory of new to be aligned on 16 bytes (__STDCPP_DEFAULT_NEW_ALIGNMENT__),
# so that a std::vector of SSE2 intervals crashed on its first movaps
# (MinGW-w64 14.2 and 15.2 for x86, rounding_direction). The compiler has to
# know what the runtime gives, so that it asks for aligned memory (the
# operator new of C++17 taking an alignment) for the types that need it, the
# intervals and everything that holds one, in the code of GAOL and in the code
# using it, which allocates them. GCC and Clang (MinGW-w64, MSYS2) take
# -faligned-new=8, which sets __STDCPP_DEFAULT_NEW_ALIGNMENT__ to 8 and turns
# the aligned new on in every standard: GAOL_ALIGNED_NEW_FLAGS, given to both
# (gaol::gaol, gaol.pc), and checked by gaol/gaol_config.h. Visual C++ and
# clang-cl have no such flag, as Clang for a target of Visual C++ (_MSC_VER):
# their /Zc:alignedNew does nothing in C++14 (cl) or breaks the STL (clang-cl,
# align_val_t not being an enumeration there), and only C++17 turns the aligned
# new on, which would make every code using GAOL compile in C++17. They take the
# SSE2 intervals all the same (GAOL v5): the intervals keep their two bounds as
# two doubles, which need 8 bytes of alignment, and load them with movupd
# (GAOL_SSE2_UNALIGNED_STORAGE of gaol/gaol_config.h, which derives it from the
# target). A compiler of another kind that does not take the flag keeps the FPU
# intervals. On 64-bit Windows the memory is aligned on 16 bytes, and the tests
# pass without it (MinGW-w64, MSYS2 with GCC and Clang).
set(GAOL_USING_SSE2_INSTRUCTIONS OFF)
set(GAOL_USING_SSE3_INSTRUCTIONS OFF)
set(GAOL_USING_NEON_INSTRUCTIONS OFF)
set(GAOL_ALIGNED_NEW_FLAGS "")  # for GCC and Clang, C++ only
set(GAOL_ALIGNED_NEW_FLAGS_STRING "")
# A universal build of macOS (several architectures in CMAKE_OSX_ARCHITECTURES)
# compiles each source once per architecture with one gaol_configuration.h, and
# the tests below, made for one target, fail for the other: the intervals are
# chosen by the architecture being compiled instead, SSE2 for x86_64 and NEON
# for arm64, where GAOL_SIMD and GAOL_ARM64 allow them
# (GAOL_UNIVERSAL_SSE2 and GAOL_UNIVERSAL_NEON, which the configuration header
# turns into GAOL_USING_SSE2_INSTRUCTIONS and GAOL_USING_NEON_INSTRUCTIONS
# according to __x86_64__ and __aarch64__; GAOL v5). No flag: SSE2 and SSE3 are
# the baseline of the x86_64 of macOS, and NEON of its arm64. The other
# probes of the compiler (the fused multiply-add, the AVX-512 path) fail
# for one architecture or the other, and stay off.
set(GAOL_UNIVERSAL_SSE2 OFF)
set(GAOL_UNIVERSAL_NEON OFF)
set(GAOL_UNIVERSAL_MACOS OFF)
if(APPLE)
  list(LENGTH CMAKE_OSX_ARCHITECTURES _gaol_architectures_count)
  if(_gaol_architectures_count GREATER 1)
    set(GAOL_UNIVERSAL_MACOS ON)
    if(GAOL_SIMD AND "x86_64" IN_LIST CMAKE_OSX_ARCHITECTURES)
      set(GAOL_UNIVERSAL_SSE2 ON)
    endif()
    if(GAOL_ARM64 AND "arm64" IN_LIST CMAKE_OSX_ARCHITECTURES)
      set(GAOL_UNIVERSAL_NEON ON)
    endif()
    message(STATUS "Universal build of macOS (${CMAKE_OSX_ARCHITECTURES}): SSE2 intervals for x86_64: ${GAOL_UNIVERSAL_SSE2}, NEON intervals for arm64: ${GAOL_UNIVERSAL_NEON}")
  endif()
endif()
if(GAOL_SIMD AND NOT GAOL_UNIVERSAL_MACOS)
  set(CMAKE_REQUIRED_FLAGS "${GAOL_INTERVAL_FLAGS_STRING}")
  check_cxx_source_compiles("
    #if !(defined(__x86_64__) || defined(__i386__) || defined(_M_X64) || defined(_M_IX86))
    #error GAOL computes its intervals with SSE2 on x86 processors only
    #endif
    #include <emmintrin.h>
    int main() { return _mm_movemask_pd(_mm_set1_pd(-1.0)); }"
    GAOL_X86_TARGET_TAKES_SSE2)
  unset(CMAKE_REQUIRED_FLAGS)
  set(_gaol_sse2_intervals ${GAOL_X86_TARGET_TAKES_SSE2})
  if(_gaol_sse2_intervals)
    check_cxx_source_compiles("
      #if !(defined(_WIN32) && !defined(_WIN64))
      #error not 32-bit Windows
      #endif
      int main() { return 0; }"
      GAOL_TARGET_IS_WIN32)
    if(GAOL_TARGET_IS_WIN32)
      # Visual C++, clang-cl, and Clang with its driver for a target of Visual
      # C++ (MSVC is false for it): the macro tells, as it tells the STL
      check_cxx_source_compiles("
        #ifndef _MSC_VER
        #error neither Visual C++ nor a compiler that simulates it
        #endif
        int main() { return 0; }"
        GAOL_COMPILER_SIMULATES_MSVC)
      if(GAOL_COMPILER_SIMULATES_MSVC)
        message(STATUS "SSE2 intervals on 32-bit Windows with Visual C++ and the compilers that simulate it (clang-cl...): the bounds are two doubles, loaded unaligned, and need no aligned memory")
      else()
        set(CMAKE_REQUIRED_FLAGS "${GAOL_INTERVAL_FLAGS_STRING}")
        check_cxx_compiler_flag(-faligned-new=8 GAOL_COMPILER_SUPPORTS_FALIGNED_NEW)
        unset(CMAKE_REQUIRED_FLAGS)
        if(GAOL_COMPILER_SUPPORTS_FALIGNED_NEW)
          set(GAOL_ALIGNED_NEW_FLAGS -faligned-new=8)
          set(GAOL_ALIGNED_NEW_FLAGS_STRING "-faligned-new=8")
        else()
          message(STATUS "SSE2 intervals left out on 32-bit Windows: the compiler cannot be told to ask for the aligned memory they need (-faligned-new=8)")
          set(_gaol_sse2_intervals OFF)
        endif()
      endif()
    endif()
  endif()
  if(_gaol_sse2_intervals AND MSVC)
    set(GAOL_USING_SSE2_INSTRUCTIONS ON)
  elseif(_gaol_sse2_intervals)
    check_cxx_compiler_flag(-msse2 GAOL_COMPILER_SUPPORTS_MSSE2)
    if(GAOL_COMPILER_SUPPORTS_MSSE2)
      set(GAOL_USING_SSE2_INSTRUCTIONS ON)
      list(APPEND GAOL_INTERVAL_FLAGS -msse2)
      check_cxx_compiler_flag(-msse3 GAOL_COMPILER_SUPPORTS_MSSE3)
      if(GAOL_COMPILER_SUPPORTS_MSSE3)
        set(GAOL_USING_SSE3_INSTRUCTIONS ON)
        list(APPEND GAOL_INTERVAL_FLAGS -msse3)
      endif()
    endif()
  endif()
endif()
# Only the SSE2 intervals need the aligned memory
if(NOT GAOL_USING_SSE2_INSTRUCTIONS)
  set(GAOL_ALIGNED_NEW_FLAGS "")
  set(GAOL_ALIGNED_NEW_FLAGS_STRING "")
endif()
string(REPLACE ";" " " GAOL_INTERVAL_FLAGS_STRING "${GAOL_INTERVAL_FLAGS}")
message(STATUS "Interval arithmetic flags: ${GAOL_INTERVAL_FLAGS_STRING}")
if(GAOL_UNIVERSAL_MACOS)
  message(STATUS "SSE2 intervals: by architecture, ${GAOL_UNIVERSAL_SSE2} for x86_64")
else()
  message(STATUS "SSE2 intervals: ${GAOL_USING_SSE2_INSTRUCTIONS}, SSE3: ${GAOL_USING_SSE3_INSTRUCTIONS}")
endif()
if(GAOL_ALIGNED_NEW_FLAGS_STRING)
  message(STATUS "Aligned memory for the SSE2 intervals on 32-bit Windows: ${GAOL_ALIGNED_NEW_FLAGS_STRING}")
endif()
if(GAOL_SSE2_UNALIGNED_STORAGE)
  message(STATUS "SSE2 intervals with their two bounds as two doubles (GAOL_SSE2_UNALIGNED_STORAGE, a switch of a developer)")
endif()

# On 64-bit ARM processors, with GAOL_ARM64 (--enable-arm64 of configure,
# enable-arm64 of meson), the intervals are computed with NEON instructions
# (gaol/gaol_interval_neon.h; GAOL v5). Where the test below compiles: GCC,
# Clang and AppleClang for AArch64 under Linux and macOS, with the
# intrinsics of <arm_neon.h> and the registers of the rounding mode, which
# the code of gaol/gaol_fpu.h already writes. NEON needs no flag. The test is
# the same with the three builds. With Visual C++ (cl) for 64-bit ARM the
# registers of the rounding mode are those of _ReadStatusReg() and
# _WriteStatusReg() (GAOL v5), and the test is its own. Every other compiler
# of 64-bit ARM takes the first test, clang-cl (it is MSVC to CMake, its
# compiler id being Clang) and the Clang of MinGW-w64 and MSYS2 included (GAOL
# v5). Not for 32-bit ARM nor x86: the option is then ignored, and the
# intervals are computed with the FPU. The refusals above (Clang 14, Clang on
# 32-bit ARM) come first. A universal build of macOS skips the tests: see above.
if(GAOL_ARM64 AND NOT GAOL_UNIVERSAL_MACOS AND CMAKE_CXX_COMPILER_ID STREQUAL "MSVC")
  set(CMAKE_REQUIRED_FLAGS "${GAOL_INTERVAL_FLAGS_STRING}")
  check_cxx_source_compiles([=[
    #if !defined(_M_ARM64) || defined(__clang__)
    #error GAOL computes its intervals with NEON on 64-bit ARM processors only
    #endif
    #include <arm_neon.h>
    #include <intrin.h>
    int main()
    {
      const unsigned long long fpcr = static_cast<unsigned long long>(_ReadStatusReg(ARM64_FPCR));
      _WriteStatusReg(ARM64_FPCR, static_cast<__int64>(fpcr));
      const float64x2_t x = vaddq_f64(vdupq_n_f64(1.0), vdupq_n_f64(2.0));
      return static_cast<int>(vgetq_lane_f64(x, 0)) - 3;
    }]=]
    GAOL_NEON_INTERVALS_MSVC)
  unset(CMAKE_REQUIRED_FLAGS)
  if(GAOL_NEON_INTERVALS_MSVC)
    set(GAOL_USING_NEON_INSTRUCTIONS ON)
  endif()
elseif(GAOL_ARM64 AND NOT GAOL_UNIVERSAL_MACOS)
  set(CMAKE_REQUIRED_FLAGS "${GAOL_INTERVAL_FLAGS_STRING}")
  check_cxx_source_compiles([=[
    #if !defined(__aarch64__)
    #error GAOL computes its intervals with NEON on 64-bit ARM processors only
    #endif
    #include <arm_neon.h>
    int main()
    {
      unsigned long long fpcr;
      __asm__ __volatile__ ("mrs %0, fpcr" : "=r" (fpcr));
      __asm__ __volatile__ ("msr fpcr, %0" : : "r" (fpcr) : "memory");
      const float64x2_t x = vaddq_f64(vdupq_n_f64(1.0), vdupq_n_f64(2.0));
      return static_cast<int>(vgetq_lane_f64(x, 0)) - 3;
    }]=]
    GAOL_NEON_INTERVALS)
  unset(CMAKE_REQUIRED_FLAGS)
  if(GAOL_NEON_INTERVALS)
    set(GAOL_USING_NEON_INSTRUCTIONS ON)
  endif()
endif()
if(GAOL_UNIVERSAL_MACOS)
  message(STATUS "NEON intervals: by architecture, ${GAOL_UNIVERSAL_NEON} for arm64")
else()
  message(STATUS "NEON intervals: ${GAOL_USING_NEON_INSTRUCTIONS}")
endif()

# GAOL_HAVE_AVX512_TARGET: the compiler accepts the AVX-512 instructions in
# the functions that carry __attribute__((target("avx512f"))), and
# __builtin_cpu_supports("avx512f"), with GCC and Clang on x86-64, but
# GCC for Windows (MinGW-w64 and Cygwin), which does not align the stack
# beyond 16 bytes, the 64 of a zmm register it spills there (GCC bug 54412,
# fixed for GCC 17 only; GAOL_FMA gives GCC for Windows -mfma with
# -mprefer-vector-width=128, which keeps it out of the 32 bytes of the ymm
# registers of AVX). The code of the path compiled by MinGW-w64 GCC 13 makes no
# aligned access of a zmm register to the stack, but nothing keeps the
# compiler from doing it (GAOL v5, point AA of TODO.md). Visual C++ and
# clang-cl for x64 take it too (GAOL v5): Visual C++ needs no attribute, it
# takes the intrinsics of any instruction set in any function, clang-cl has
# the attribute, and both tell the processor by cpuid and xgetbv, Visual C++
# having no __builtin_cpu_supports() and the one of clang-cl needing the
# runtime library of LLVM at the link (cpu_runs_avx512f() of
# gaol/gaol_common.cpp). The result is
# cached under a name of its own, GAOL_COMPILER_TAKES_AVX512_PATH, so that a
# build directory configured before that check checks again. Only the
# functions of the AVX-512 path of +, -, *, /, sqr, sqrt and, with FMA, the
# integer powers beyond 1 and -1 (gaol/gaol_interval_avx512.cpp) carry the
# attribute, and the rest of the library is compiled as without it; the path
# is compiled in where GAOL_AVX512_ARITHMETIC below is (GAOL v5).
# The program that finds out whether Visual C++ and clang-cl for x64 run the
# AVX-512 instructions, which the checks below compile and run: the same
# cpuid and xgetbv as cpu_runs_avx512f() of gaol/gaol_common.cpp, and an
# addition by the embedded rounding, in a function that carries the target
# where the compiler has the attribute (clang-cl)
set(GAOL_AVX512_MSVC_PROBE [=[
    #include <immintrin.h>
    #include <intrin.h>
    #if defined(__clang__)
    #  define GAOL_TARGET __attribute__((target("avx512f")))
    #else
    #  define GAOL_TARGET
    #endif
    GAOL_TARGET double er_add(double a, double b)
    {
      const __m512d x = _mm512_set1_pd(a);
      const __m512d y = _mm512_set1_pd(b);
      return _mm_cvtsd_f64(_mm512_castpd512_pd128(_mm512_add_round_pd(x, y, _MM_FROUND_TO_POS_INF | _MM_FROUND_NO_EXC)));
    }
    bool cpu_runs_avx512f()
    {
      int regs[4];
      __cpuid(regs, 0);
      if (regs[0] < 7) {
        return false;
      }
      __cpuid(regs, 1);
      if ((regs[2] & (1 << 27)) == 0 || (_xgetbv(0) & 0xe6) != 0xe6) {
        return false;
      }
      __cpuidex(regs, 7, 0);
      return (regs[1] & (1 << 16)) != 0;
    }
]=])
set(GAOL_HAVE_AVX512_TARGET OFF)
if(GAOL_USING_SSE2_INSTRUCTIONS AND MSVC AND CMAKE_SIZEOF_VOID_P EQUAL 8)
  check_cxx_source_compiles("${GAOL_AVX512_MSVC_PROBE}
    int main() { return cpu_runs_avx512f() ? 0 : 1; }"
    GAOL_COMPILER_TAKES_AVX512_PATH)
  if(GAOL_COMPILER_TAKES_AVX512_PATH)
    set(GAOL_HAVE_AVX512_TARGET ON)
  endif()
elseif(GAOL_USING_SSE2_INSTRUCTIONS AND CMAKE_SIZEOF_VOID_P EQUAL 8)
  # The source in a bracket argument: verbatim, no quote to escape
  check_cxx_source_compiles([=[
    #if (defined(_WIN32) || defined(__CYGWIN__)) && defined(__GNUC__) && !defined(__clang__)
    #error GCC for Windows does not align the stack for AVX-512
    #endif
    #include <immintrin.h>
    __attribute__((target("avx512f"))) double er_add(double a, double b)
    {
      const __m512d x = _mm512_set1_pd(a);
      const __m512d y = _mm512_set1_pd(b);
      return _mm_cvtsd_f64(_mm512_castpd512_pd128(_mm512_add_round_pd(x, y, _MM_FROUND_TO_POS_INF | _MM_FROUND_NO_EXC)));
    }
    int main() { return __builtin_cpu_supports("avx512f") != 0 ? 0 : 1; }]=]
    GAOL_COMPILER_TAKES_AVX512_PATH)
  if(GAOL_COMPILER_TAKES_AVX512_PATH)
    set(GAOL_HAVE_AVX512_TARGET ON)
  endif()
endif()
if(GAOL_PREFER_AVX512 AND NOT GAOL_HAVE_AVX512_TARGET)
  if(GAOL_PREFER_AVX512_EXPLICIT)
    message(FATAL_ERROR "GAOL_PREFER_AVX512 takes the SSE2 intervals of x86-64, with Clang, Visual C++, or GCC except on Windows (MinGW-w64, Cygwin)")
  endif()
  # The default GAOL_PRESERVE_ROUNDING derived, and the compiler or the
  # architecture takes no path: the operations keep setting the direction
  # upward and giving it back, as a build with GAOL_PRESERVE_ROUNDING
  # always did (GAOL v5). The entry follows, and its record: the next
  # configure derives the default again, and finds the same here
  message(STATUS "GAOL_PREFER_AVX512, from GAOL_PRESERVE_ROUNDING: no AVX-512 path here, the operations keep setting the direction upward")
  set(GAOL_PREFER_AVX512 OFF)
  gaol_keep_prefer_avx512(OFF)
endif()
# GAOL_AVX512_ARITHMETIC: +, -, *, /, sqr, sqrt and, with FMA, the integer
# powers beyond 1 and -1 take the AVX-512 path, which is decided here:
# GAOL_PREFER_AVX512 asks for it, the compiler takes the path, and the machine
# building runs it, as GAOL_FMA checks it for the fused multiply-add (never
# when cross-compiling). The operations then are that path alone, and the SSE2
# one otherwise, with no flag to test as they run.
# gaol::init() chose the path, and every operation tested the flag it set,
# in the default build too, though only GAOL_PREFER_AVX512 could make it
# true: without the test, the line (a + b) * (a - b) / p of the benchmark of
# doc/compare takes 25.9 ns rather than 26.3 in the default build, Shekel 5
# 259 ns rather than 263 (Clang 18, Intel Xeon E3-1220 v6, the library and
# the program built with -mbranches-within-32B-boundaries; GAOL v5, point AA
# of TODO.md). A library so built needs a processor with the AVX-512
# instructions, as one built with GAOL_FMA needs FMA: -DGAOL_PREFER_AVX512=OFF
# builds it for any processor of the architecture.
set(GAOL_AVX512_ARITHMETIC OFF)
if(GAOL_PREFER_AVX512 AND GAOL_HAVE_AVX512_TARGET)
  if(CMAKE_CROSSCOMPILING)
    message(STATUS "GAOL_PREFER_AVX512: cross-compiling, the AVX-512 path left out")
  else()
    if(MSVC)
      check_cxx_source_runs("${GAOL_AVX512_MSVC_PROBE}
        int main()
        {
          volatile double one = 1.0, tiny = 8.67361737988403547205962240695953369140625e-19;
          return cpu_runs_avx512f() && er_add(one, tiny) > 1.0 ? 0 : 1;
        }"
        GAOL_MACHINE_RUNS_AVX512)
    else()
      check_cxx_source_runs([=[
        #include <immintrin.h>
        __attribute__((target("avx512f"))) double er_add(double a, double b)
        {
          const __m512d x = _mm512_set1_pd(a);
          const __m512d y = _mm512_set1_pd(b);
          return _mm_cvtsd_f64(_mm512_castpd512_pd128(_mm512_add_round_pd(x, y, _MM_FROUND_TO_POS_INF | _MM_FROUND_NO_EXC)));
        }
        int main()
        {
          volatile double one = 1.0, tiny = 8.67361737988403547205962240695953369140625e-19;
          return __builtin_cpu_supports("avx512f") != 0 && er_add(one, tiny) > 1.0 ? 0 : 1;
        }]=]
        GAOL_MACHINE_RUNS_AVX512)
    endif()
    if(GAOL_MACHINE_RUNS_AVX512)
      set(GAOL_AVX512_ARITHMETIC ON)
    else()
      message(STATUS "GAOL_PREFER_AVX512: this machine does not run the AVX-512 instructions, the AVX-512 path left out")
    endif()
  endif()
endif()
message(STATUS "AVX-512 interval arithmetic: the compiler takes it: ${GAOL_HAVE_AVX512_TARGET}, preferred: ${GAOL_PREFER_AVX512}, taken: ${GAOL_AVX512_ARITHMETIC}")

# The library compiled with hidden visibility, as configure builds it, and with
# the warnings of configure; GAOL's public declarations carry
# __attribute__ ((visibility("default"))) (GAOL_HAVE_VISIBILITY_OPTIONS)
set(GAOL_LIBRARY_FLAGS "")
if(NOT MSVC)
  check_cxx_compiler_flag("-fvisibility-inlines-hidden -fvisibility=hidden" GAOL_HAVE_VISIBILITY_OPTIONS)
  if(GAOL_HAVE_VISIBILITY_OPTIONS)
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
  # (not with GAOL_DEBUG)
  set(GAOL_RELEASE_FLAGS "")
  foreach(flag -funroll-loops -fomit-frame-pointer -fexpensive-optimizations)
    string(MAKE_C_IDENTIFIER "${flag}" _flag_id)
    string(TOUPPER "GAOL_COMPILER_SUPPORTS${_flag_id}" _flag_var)
    check_cxx_compiler_flag("${flag}" ${_flag_var})
    if(${_flag_var} AND NOT GAOL_DEBUG)
      list(APPEND GAOL_LIBRARY_FLAGS "$<$<CONFIG:Release>:${flag}>")
      list(APPEND GAOL_RELEASE_FLAGS ${flag})
    endif()
  endforeach()
endif()

configure_file(cmake/gaol_configuration.h.in "${CMAKE_CURRENT_BINARY_DIR}/gaol/gaol_configuration.h")


################################################################################
# libgaol
################################################################################
#
# The sources listed in gaol/meson.build. gaol_interval_core.cpp includes
# gaol_interval_neon.cpp (GAOL v5), gaol_interval_sse.cpp or
# gaol_interval_fpu.cpp itself, according to GAOL_USING_NEON_INSTRUCTIONS and
# GAOL_USING_SSE2_INSTRUCTIONS, and gaol/float/gaol_intervalf.cpp and
# gaol/float/gaol_interval2f.cpp where a developer of GAOL defines
# GAOL_FLOAT_INTERVALS (see gaol/gaol_config.h).
#
# A static library, or a shared one with BUILD_SHARED_LIBS=ON: for Visual
# C++, always a static one, GAOL's declarations being compiled without
# __declspec(dllexport) or dllimport (GAOL_PUBLIC, gaol/gaol_config.h).

set(GAOL_SOURCES
  gaol_interval.cpp gaol_interval_core.cpp gaol_ieee1788.cpp gaol_common.cpp
  parser/gaol_parser.cpp parser/gaol_expression.cpp gaol_port.cpp gaol_exceptions.cpp gaol_init_cleanup.cpp
  parser/gaol_interval_lexer.cpp
  parser/gaol_interval_parser.cpp
)
list(TRANSFORM GAOL_SOURCES PREPEND "gaol/")

# The sources of CORE-MATH, compiled apart and put into libgaol: apart so that
# the interprocedural optimization GAOL is compiled with does not reach them,
# which made sin and cos half as fast again while it makes the basic
# operations twice as fast (GAOL v5, see doc/three-builds.md)
add_library(gaol_core_math OBJECT ${GAOL_CORE_MATH_SOURCES})
set_target_properties(gaol_core_math PROPERTIES
  INTERPROCEDURAL_OPTIMIZATION OFF
  POSITION_INDEPENDENT_CODE ON)
# gaol/core_math_port.h included first in each of them, by the compiler
target_compile_options(gaol_core_math PRIVATE ${GAOL_CORE_MATH_FLAGS} ${GAOL_INTERVAL_FLAGS} ${GAOL_FMA_FLAGS} ${GAOL_LIBRARY_FLAGS})
target_include_directories(gaol_core_math PRIVATE
  "${CMAKE_CURRENT_BINARY_DIR}" "${CMAKE_CURRENT_SOURCE_DIR}")
# /W4 of Visual C++ (the job warnings of windows.yml) on what CORE-MATH writes
# on purpose (GAOL v5): its #pragma STDC FENV_ACCESS, which /fp:strict makes
# redundant (C4068), the anonymous structures of its unions (C4201), masks
# written as negative constants (C4245), and the volatile variables it
# computes for the flags they raise alone (C4189); and in erfc.c the call of a
# void function returned from cr_erf_accurate(), void too, which C does not
# allow but which calls the function and returns all the same (C4098; erf.c
# calls it without return)
if(CMAKE_C_COMPILER_ID STREQUAL "MSVC")
  target_compile_options(gaol_core_math PRIVATE /wd4068 /wd4201 /wd4245 /wd4189 /wd4098)
endif()

if(MSVC)
  add_library(gaol STATIC ${GAOL_SOURCES} $<TARGET_OBJECTS:gaol_core_math>)
else()
  add_library(gaol ${GAOL_SOURCES} $<TARGET_OBJECTS:gaol_core_math>)
endif()
# The shared library libgaol.so.5.0.0, whose soname is libgaol.so.5, and the
# link libgaol.so, as configure (libtool) and meson name it (GAOL v5)
set_target_properties(gaol PROPERTIES VERSION ${PROJECT_VERSION} SOVERSION ${PROJECT_VERSION_MAJOR})
if(CMAKE_CONFIGURATION_TYPES)
  target_compile_definitions(gaol PRIVATE $<$<CONFIG:Debug>:GAOL_DEBUGGING=1>)
endif()


if(GAOL_U128_EMULATION)
  message(STATUS "128-bit integer: the two halves of gaol/gaol_u128.h, as with Visual C++ and on a 32-bit target")
  target_compile_definitions(gaol PUBLIC GAOL_U128_FORCE_EMULATION=1)
  # The sources of CORE-MATH, which are the ones computing with the 128-bit
  # integer, are compiled in gaol_core_math, which the definitions of gaol do
  # not reach: without this line they took the native type, and the option
  # only changed tests/u128.cpp (GAOL v5)
  target_compile_definitions(gaol_core_math PRIVATE GAOL_U128_FORCE_EMULATION=1)
endif()
add_library(gaol::gaol ALIAS gaol)

# The generated headers, then GAOL's own: its sources include "gaol/..."
target_include_directories(gaol PUBLIC
  "$<BUILD_INTERFACE:${CMAKE_CURRENT_BINARY_DIR}>"
  "$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>"
  "$<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>"
)
target_compile_options(gaol PUBLIC ${GAOL_INTERVAL_FLAGS} ${GAOL_FMA_FLAGS})
# The aligned memory the SSE2 intervals need on 32-bit Windows, for the code of
# C++ only (-faligned-new=8 is not an option of C), and for the library as for
# the code using it (GAOL v5)
if(GAOL_ALIGNED_NEW_FLAGS)
  target_compile_options(gaol PUBLIC "$<$<COMPILE_LANGUAGE:CXX>:${GAOL_ALIGNED_NEW_FLAGS}>")
endif()
target_compile_options(gaol PRIVATE ${GAOL_LIBRARY_FLAGS})
# The parser that bison writes has unreachable code in a Debug build with
# Visual C++ (C4702), which /WX turns into an error: the warning is off for
# that file (GAOL v5).
if(MSVC)
  set_source_files_properties(gaol/parser/gaol_interval_parser.cpp PROPERTIES COMPILE_OPTIONS "/wd4702")
endif()
# To the link of the code using GAOL, when it is built by a compiler that takes
# the option: GCC 13 or a later one, or the compiler that built GAOL, of its
# major version and not older than it. Another compiler, as Clang 18, would stop
# on the option, and the option is not in every later release: GCC has it from
# 11.4 in the series 11 and from 12.4 in the series 12, so that a GAOL built by
# GCC 11.4 (Ubuntu 22.04) and a program built by GCC 12.3 (its g++-12) have it
# and do not (GAOL v5, review of point 4: gcc/config/i386/i386.opt of the
# release tags)
if(GAOL_LINK_FLAGS)
  string(REGEX MATCH "^[0-9]+" GAOL_COMPILER_MAJOR "${CMAKE_CXX_COMPILER_VERSION}")
  math(EXPR GAOL_COMPILER_NEXT_MAJOR "${GAOL_COMPILER_MAJOR} + 1")
  set(GAOL_LINK_FLAGS_SERIES "$<AND:$<CXX_COMPILER_ID:${CMAKE_CXX_COMPILER_ID}>,$<VERSION_GREATER_EQUAL:$<CXX_COMPILER_VERSION>,${CMAKE_CXX_COMPILER_VERSION}>,$<VERSION_LESS:$<CXX_COMPILER_VERSION>,${GAOL_COMPILER_NEXT_MAJOR}>>")
  set(GAOL_LINK_FLAGS_GCC13 "$<AND:$<CXX_COMPILER_ID:GNU>,$<VERSION_GREATER_EQUAL:$<CXX_COMPILER_VERSION>,13>>")
  target_link_options(gaol INTERFACE
    "$<$<OR:${GAOL_LINK_FLAGS_GCC13},${GAOL_LINK_FLAGS_SERIES}>:${GAOL_LINK_FLAGS}>")
endif()
# Without a build type, CMake gives no optimization flag at all. GAOL sets
# Release when it is the top-level project, but brought in by a project that
# sets none (add_subdirectory, FetchContent), GAOL and CORE-MATH were compiled
# without optimization: they are compiled then as in Release, with -O3 (/O2
# with Visual C++), NDEBUG and the optimizations configure adds, as configure
# and meson build GAOL brought into another project (GAOL v5: GAOL alone had
# -O3, and CORE-MATH, compiled apart, no optimization)
if(NOT CMAKE_BUILD_TYPE AND NOT CMAKE_CONFIGURATION_TYPES AND NOT GAOL_DEBUG)
  foreach(target gaol gaol_core_math)
    if(MSVC)
      target_compile_options(${target} PRIVATE /O2)
    else()
      target_compile_options(${target} PRIVATE -O3 ${GAOL_RELEASE_FLAGS})
    endif()
    target_compile_definitions(${target} PRIVATE NDEBUG)
  endforeach()
endif()
if(MSVC)
  # The lexer includes <unistd.h>, which Visual C++ does not have
  target_include_directories(gaol PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}/cmake/msvc")
  # Visual C++ only: clang-cl does not know the option, and warned of it for
  # each source (GAOL v5)
  if(CMAKE_CXX_COMPILER_ID STREQUAL "MSVC")
    target_compile_options(gaol PRIVATE /Zc:strictStrings-)
  endif()
  target_compile_definitions(gaol PRIVATE _CRT_SECURE_NO_WARNINGS _CRT_NONSTDC_NO_WARNINGS)
  # No __GAOL_PUBLIC__= for GAOL and the code using it any longer:
  # gaol/gaol_config.h declares GAOL's classes and functions without
  # __declspec(dllimport) for Visual C++, where GAOL_PUBLIC is empty (GAOL v5)
endif()
if(FALSE)
endif()


################################################################################
# Installation
################################################################################
#
# find_package(gaol) then provides gaol::gaol, which carries the flags of
# interval arithmetic. CORE-MATH is compiled into libgaol, so there is no
# other target and nothing else to install.

install(TARGETS gaol EXPORT gaolTargets
  ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
  LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
  RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)

# The headers a program includes, gaol/gaol, gaol/parser/gaol_expression.h and
# gaol/parser/gaol_expr_eval.h for the expressions, and those they include,
# gaol/gaol_fpu_msvc.h for Visual C++ among them: gaol/Makefile.am and
# gaol/meson.build install the same ones. Not the headers of the sources of
# GAOL only (GAOL v5): gaol/parser/gaol_interval_parser.h, which did not
# compile when included, the parts of the library (gaol_init_cleanup.h,
# gaol_core_math.h, gaol_u128.h),
# what CORE-MATH's sources need to compile within GAOL (core_math_port.h,
# core_math_log_dint.h, gaol_ldexp.h), the bounds at doubles, *_dn() and
# *_up(), which are not part of GAOL's interface (gaol_double_op.h), and the
# intervals of floats, which only a developer of GAOL compiles (see
# gaol/gaol_config.h)
set(GAOL_HEADERS
  gaol/gaol.h gaol/gaol_common.h
  gaol/gaol_config.h gaol/gaol_exceptions.h
  gaol/gaol_fpu.h gaol/gaol_fpu_fenv.h gaol/gaol_fpu_msvc.h
  gaol/gaol_ieee1788.h gaol/gaol_interval.h gaol/gaol_interval_core.h gaol/gaol_interval_fpu.h
  gaol/gaol_interval_neon.h gaol/gaol_interval_sse.h
  gaol/gaol_port.h gaol/gaol_roundeven.h
  gaol/gaol_version.h)
# The headers of the parser of the strings and of the expressions, which a
# program includes as gaol/parser/gaol_expression.h and
# gaol/parser/gaol_parser.h: installed in gaol/parser/ as in the sources
set(GAOL_PARSER_HEADERS
  gaol/parser/gaol_eval_stack.h gaol/parser/gaol_expr_eval.h
  gaol/parser/gaol_expr_visitor.h gaol/parser/gaol_expression.h
  gaol/parser/gaol_parser.h)
install(FILES
  gaol/gaol
  ${GAOL_HEADERS}
  "${CMAKE_CURRENT_BINARY_DIR}/gaol/gaol_configuration.h"
  DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/gaol
)
install(FILES
  ${GAOL_PARSER_HEADERS}
  DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/gaol/parser
)

set(GAOL_INSTALL_CMAKEDIR ${CMAKE_INSTALL_LIBDIR}/cmake/gaol)
install(EXPORT gaolTargets NAMESPACE gaol:: DESTINATION ${GAOL_INSTALL_CMAKEDIR})
configure_package_config_file(cmake/gaolConfig.cmake.in "${CMAKE_CURRENT_BINARY_DIR}/gaolConfig.cmake"
  INSTALL_DESTINATION ${GAOL_INSTALL_CMAKEDIR})
write_basic_package_version_file("${CMAKE_CURRENT_BINARY_DIR}/gaolConfigVersion.cmake"
  VERSION ${PROJECT_VERSION} COMPATIBILITY SameMajorVersion)
install(FILES "${CMAKE_CURRENT_BINARY_DIR}/gaolConfig.cmake" "${CMAKE_CURRENT_BINARY_DIR}/gaolConfigVersion.cmake"
  DESTINATION ${GAOL_INSTALL_CMAKEDIR})

# The pkg-config file, for the builds that are not CMake (pkg-config --cflags
# --libs gaol): the flags of interval arithmetic, as gaol::gaol carries them,
# and libgaol, into which the functions of CORE-MATH are compiled. Not with
# Visual C++, whose libraries pkg-config does not name.
if(NOT MSVC)
  # Relocatable: the prefix from the directory of gaol.pc, ${pcfiledir}, so
  # that the file is right wherever the files are put (cmake --install
  # --prefix, the archive and the .deb of CPack below), unless the directories
  # given are absolute (GAOL v5)
  if(IS_ABSOLUTE "${CMAKE_INSTALL_LIBDIR}" OR IS_ABSOLUTE "${CMAKE_INSTALL_INCLUDEDIR}")
    set(GAOL_PC_PREFIX "${CMAKE_INSTALL_PREFIX}")
    set(GAOL_PC_LIBDIR "${CMAKE_INSTALL_FULL_LIBDIR}")
    set(GAOL_PC_INCLUDEDIR "${CMAKE_INSTALL_FULL_INCLUDEDIR}")
  else()
    file(RELATIVE_PATH _gaol_pc_to_prefix "/prefix/${CMAKE_INSTALL_LIBDIR}/pkgconfig" "/prefix")
    string(REGEX REPLACE "/$" "" _gaol_pc_to_prefix "${_gaol_pc_to_prefix}")
    set(GAOL_PC_PREFIX "\${pcfiledir}/${_gaol_pc_to_prefix}")
    set(GAOL_PC_LIBDIR "\${prefix}/${CMAKE_INSTALL_LIBDIR}")
    set(GAOL_PC_INCLUDEDIR "\${prefix}/${CMAKE_INSTALL_INCLUDEDIR}")
  endif()
  configure_file(cmake/gaol.pc.in "${CMAKE_CURRENT_BINARY_DIR}/gaol.pc" @ONLY)
  install(FILES "${CMAKE_CURRENT_BINARY_DIR}/gaol.pc" DESTINATION ${CMAKE_INSTALL_LIBDIR}/pkgconfig)
endif()


################################################################################
# Packages (CPack)
################################################################################
#
# The archive of the sources, which make dist of the autotools build made
# before (GAOL v5), and the packages of the library built:
#
#   cmake --build <build> --target package_source
#       gaol-<version>.tar.gz: the sources, in gaol-<version>/
#   cmake --build <build> --target package
#       gaol-<version>-<system>.tar.gz: what cmake --install installs, in
#       gaol-<version>-<system>/, and on Linux libgaol-dev_<version>_<arch>.deb,
#       the same files in /usr
#
# CPack takes the source tree as it is: the archive of the sources is made
# from a clean checkout, its untracked files and those of the builds left
# out below. The packages hold libgaol as the build compiled it: with GAOL_FMA
# ON, the default, only processors with the fused multiply-add instructions
# run it, and with GAOL_AVX512_ARITHMETIC only those with the AVX-512 ones,
# so a package for other machines is built with -DGAOL_FMA=OFF and
# -DGAOL_PREFER_AVX512=OFF (see doc/building.md). Only where GAOL is the
# project built, not brought in by another (add_subdirectory, FetchContent),
# whose packages are its own.
if(CMAKE_SOURCE_DIR STREQUAL PROJECT_SOURCE_DIR)
  set(CPACK_PACKAGE_NAME "gaol")
  set(CPACK_PACKAGE_VENDOR "ENSTA")
  set(CPACK_PACKAGE_CONTACT "Jordan Ninin <jordan.ninin@ensta.fr>")
  set(CPACK_PACKAGE_DESCRIPTION_SUMMARY "GAOL, Not Just Another Interval Library: interval arithmetic in C++")
  set(CPACK_PACKAGE_DESCRIPTION "GAOL v5 computes the tightest enclosures of the arithmetic operations and of the elementary functions, with the correctly rounded functions of CORE-MATH, compiled into the library. The package holds the library libgaol, its headers, and the files find_package(gaol) and pkg-config read.")
  set(CPACK_PACKAGE_HOMEPAGE_URL "https://github.com/Jordan08/GAOL")
  set(CPACK_RESOURCE_FILE_LICENSE "${PROJECT_SOURCE_DIR}/COPYING.LIB")
  set(CPACK_RESOURCE_FILE_README "${PROJECT_SOURCE_DIR}/README.md")
  if(CMAKE_SYSTEM_NAME STREQUAL "Linux")
    set(CPACK_GENERATOR "TGZ;DEB")
  else()
    set(CPACK_GENERATOR "TGZ")
  endif()

  # The .deb: a static library and its headers, hence the name and the
  # section of Debian's development packages; nothing to depend on at run time
  set(CPACK_DEBIAN_PACKAGE_NAME "libgaol-dev")
  set(CPACK_DEBIAN_FILE_NAME DEB-DEFAULT)
  set(CPACK_DEBIAN_PACKAGE_SECTION "libdevel")

  # The archive of the sources: the files of the repository, without what git
  # ignores (.gitignore), the builds within the sources, the packages, and the
  # notes of the work on GAOL v5 (TODO.md, process.md, todo-notes/), which
  # serve its maintainers, not the users of a version. CPack writes the CPACK_*
  # variables into CPackConfig.cmake and CPackSourceConfig.cmake, which it reads
  # when it runs: CPACK_VERBATIM_VARIABLES has it escape them, so that the
  # backslashes of these regular expressions reach it as they are, where CMake
  # 3 warned of an invalid escape sequence, \. (CMP0010, GAOL v5)
  set(CPACK_VERBATIM_VARIABLES ON)
  set(CPACK_SOURCE_GENERATOR "TGZ")
  set(CPACK_SOURCE_PACKAGE_FILE_NAME "gaol-${PROJECT_VERSION}")
  string(REGEX REPLACE "([][+.*()^$?|\\])" "\\\\\\1" _gaol_source_regex "${PROJECT_SOURCE_DIR}")
  set(CPACK_SOURCE_IGNORE_FILES
    "/\\.git/" "/\\.git$" "/\\.claude/" "/\\.vscode/" "/\\.idea/" "/\\.cache/"
    "\\.swp$" "~$" "/#" "/\\.#"
    "/autom4te\\.cache/" "/doc/compare/code/work/" "/doc/compare/itf1788/work/"
    "/doc/compare/tang2021/work/" "/tests/meson_subproject/subprojects/"
    "/coverage/[^/]*\\.(css|js)$" "/coverage/coverage\\.json$"
    "\\.o$" "\\.lo$" "/\\.libs/" "/\\.deps/" "\\.dirstamp$" "/check/performances_result\\.txt$"
    "^${_gaol_source_regex}/build[^/]*/" "/_CPack_Packages/"
    "^${_gaol_source_regex}/gaol-[^/]*\\.tar\\.gz$" "^${_gaol_source_regex}/libgaol-dev_[^/]*\\.deb$"
    "^${_gaol_source_regex}/TODO\\.md$" "^${_gaol_source_regex}/process\\.md$" "^${_gaol_source_regex}/todo-notes/"
  )

  # The archive holds configure as it is committed, and configure says the
  # version autoconf read from VERSION.txt when it generated it (the line
  # PACKAGE_VERSION= of the script; configure --version and --help give it).
  # After a change of VERSION.txt without a new generation of configure, the
  # archive of the new version holds a configure that says the old one, and
  # warns when it runs: only the continuous integration reported it. CMake can
  # warn while it configures only, so it does here, whether or not the archive
  # is made. A tree without configure, or with a configure that has no such
  # line, has nothing to compare (tests/cpack_stale_configure.cmake checks it,
  # GAOL v5). As the archive takes its name from the version CMake read when it
  # configured, CPack includes cmake/gaol_package_source.cmake, which configures
  # the build directory again first when VERSION.txt holds another version:
  # Ninja does before package_source, the Makefile generators did not.
  set(CPACK_PROJECT_CONFIG_FILE "${PROJECT_SOURCE_DIR}/cmake/gaol_package_source.cmake")
  set(CPACK_GAOL_VERSION "${PROJECT_VERSION}")
  set(CPACK_GAOL_CMAKE_COMMAND "${CMAKE_COMMAND}")
  set(CPACK_GAOL_SOURCE_DIR "${PROJECT_SOURCE_DIR}")
  set(CPACK_GAOL_BINARY_DIR "${PROJECT_BINARY_DIR}")
  if(EXISTS "${PROJECT_SOURCE_DIR}/configure")
    file(STRINGS "${PROJECT_SOURCE_DIR}/configure" _gaol_configure_version REGEX "^PACKAGE_VERSION='" LIMIT_COUNT 1)
    if(_gaol_configure_version MATCHES "^PACKAGE_VERSION='([^']*)'")
      set(_gaol_configure_version "${CMAKE_MATCH_1}")
      if(NOT _gaol_configure_version STREQUAL PROJECT_VERSION)
        message(WARNING
          "configure was generated for GAOL ${_gaol_configure_version}, and VERSION.txt holds ${PROJECT_VERSION}: "
          "the archive of the sources (package_source) would hold configure as it is, which says "
          "${_gaol_configure_version} for --version and --help. Generate configure again before "
          "VERSION.txt is committed, as doc/building.md says.")
      endif()
    endif()
  endif()

  include(CPack)
endif()


################################################################################
# Tests and examples
################################################################################
#
# WITH_TESTS builds the unit tests of tests/ and WITH_EXAMPLES the examples of
# examples/, which check what they print. ctest runs both; make test (the test
# target of CMake: RUN_TESTS with Visual Studio) runs the unit tests only, and
# make check (the target check) builds and runs the tests and the examples, as
# with configure and meson (GAOL v5):
#
#   cmake -S . -B build -DWITH_TESTS=ON -DWITH_EXAMPLES=ON
#   cmake --build build
#   cmake --build build --target test     # make test: the unit tests
#   cmake --build build --target check    # make check: the tests and the examples
#
# The examples carry the label example, which make test leaves out
# (CMAKE_CTEST_ARGUMENTS, CMake 3.17 and later: an earlier CMake runs them too).

if(GAOL_IS_TOP_LEVEL AND (WITH_TESTS OR WITH_EXAMPLES))
  set(CMAKE_CTEST_ARGUMENTS --label-exclude example)
  enable_testing()
  set(GAOL_CHECK_TARGETS "")
  if(WITH_TESTS)
    add_subdirectory(tests)
  endif()
  if(WITH_EXAMPLES)
    add_subdirectory(examples)
  endif()
  add_custom_target(check
    COMMAND "${CMAKE_CTEST_COMMAND}" --output-on-failure -C $<CONFIG>
    DEPENDS ${GAOL_CHECK_TARGETS}
    WORKING_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}"
    COMMENT "Running the unit tests and the examples"
    USES_TERMINAL)
endif()


################################################################################
# The lexer and the parser of the strings
################################################################################
#
# gaol/parser/gaol_interval_lexer.cpp, gaol/parser/gaol_interval_parser.cpp and
# gaol/parser/gaol_interval_parser.h are generated from
# gaol/parser/gaol_interval_lexer.lpp (flex) and
# gaol/parser/gaol_interval_parser.ypp (bison) and committed, so that no
# build needs flex or bison. After a change of the .lpp or of the .ypp, the
# target parser regenerates them in the sources, with the flex and the bison of
# the system, where both are found (GAOL v5):
#
#   cmake --build build --target parser
#
# gaol/parser/regenerate_parser.sh runs the same commands without CMake. The
# committed files were generated by flex 2.6.4 and bison 3.8.2, which give them
# again byte for byte; the programs write lex.gaol_.c and y.tab.c, the names of
# the #line directives of the committed files, in a directory of the build.
# Only where GAOL is the project built, as the targets check and perf.

if(GAOL_IS_TOP_LEVEL)
  find_program(GAOL_FLEX NAMES flex win_flex)
  find_program(GAOL_BISON NAMES bison win_bison)
  mark_as_advanced(GAOL_FLEX GAOL_BISON)
endif()
if(GAOL_IS_TOP_LEVEL AND GAOL_FLEX AND GAOL_BISON)
  set(_parser_dir "${CMAKE_CURRENT_BINARY_DIR}/parser")
  set(_gaol_parser_dir "${CMAKE_CURRENT_SOURCE_DIR}/gaol/parser")
  # The directory the commands run in, which has to exist before they run
  file(MAKE_DIRECTORY "${_parser_dir}")
  add_custom_target(parser
    COMMAND "${CMAKE_COMMAND}" -E copy "${_gaol_parser_dir}/gaol_interval_lexer.lpp" "${_gaol_parser_dir}/gaol_interval_parser.ypp" "${_parser_dir}"
    COMMAND "${GAOL_FLEX}" -Pgaol_ gaol_interval_lexer.lpp
    COMMAND "${GAOL_BISON}" -y -Wno-yacc -Wno-deprecated -Wno-other -d -p gaol_ gaol_interval_parser.ypp
    COMMAND "${CMAKE_COMMAND}" -E copy lex.gaol_.c "${_gaol_parser_dir}/gaol_interval_lexer.cpp"
    COMMAND "${CMAKE_COMMAND}" -E copy y.tab.c "${_gaol_parser_dir}/gaol_interval_parser.cpp"
    COMMAND "${CMAKE_COMMAND}" -E copy y.tab.h "${_gaol_parser_dir}/gaol_interval_parser.h"
    WORKING_DIRECTORY "${_parser_dir}"
    COMMENT "Regenerating gaol/parser/gaol_interval_lexer.cpp, gaol/parser/gaol_interval_parser.cpp and gaol/parser/gaol_interval_parser.h with ${GAOL_FLEX} and ${GAOL_BISON}"
    VERBATIM)
endif()


################################################################################
# Performance
################################################################################
#
# make perf (the target perf) measures GAOL v5 alone on the benchmark of
# doc/compare/performance.md, compiled with the flags of this build, and
# writes the new times of GAOL v5 into its tables, those of the operations and
# those of the ten long lines of + - * / of doc/compare/code/run_lines.sh; the
# columns of the other libraries keep the times measured with them, which
# doc/compare/code/results.csv and results_lines.csv hold (see
# doc/compare/code/run_perf.sh). It needs bash and Python 3 with numpy, which
# draws the intervals, and runs on Linux and macOS. GAOL 4.2.3, filib++,
# libieeep1788 and PROFIL/BIAS are not run: doc/compare/code/run_all.sh
# measures them all again. Not with GAOL_DEBUG, which would measure GAOL built
# for debugging (GAOL v5).
#
# The program of the benchmark, gaol_bench, is a target of its own, not built
# by default, on every system: .github/scripts/performance-ab.sh builds it
# in two trees of GAOL, the base of a pull request and its head, to run the
# long lines on the same intervals (.github/workflows/performance.yml).

if(GAOL_IS_TOP_LEVEL AND NOT GAOL_DEBUG)
  set(_bench_dir "${CMAKE_CURRENT_SOURCE_DIR}/doc/compare/code")
  add_executable(gaol_bench EXCLUDE_FROM_ALL "${_bench_dir}/bench_gaol.cpp")
  target_link_libraries(gaol_bench PRIVATE gaol::gaol)
  target_include_directories(gaol_bench PRIVATE "${_bench_dir}")
  target_compile_definitions(gaol_bench PRIVATE GAOL_BENCH_NAME="gaol5")
  if(CMAKE_SYSTEM_NAME STREQUAL "Linux" OR CMAKE_SYSTEM_NAME STREQUAL "Darwin")
    find_program(GAOL_BASH bash)
    mark_as_advanced(GAOL_BASH)
    if(GAOL_BASH)
      add_custom_target(perf
        COMMAND "${GAOL_BASH}" "${_bench_dir}/run_perf.sh" "$<TARGET_FILE:gaol_bench>"
                "CMake ${CMAKE_BUILD_TYPE}" "${CMAKE_CXX_COMPILER}"
        DEPENDS gaol_bench
        WORKING_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}"
        COMMENT "Measuring GAOL v5 on the benchmark of doc/compare/performance.md"
        USES_TERMINAL
        VERBATIM)
    endif()
  endif()
endif()


################################################################################
# Coverage
################################################################################
#
# cmake -S . -B build-coverage -DCMAKE_BUILD_TYPE=Debug -DWITH_TESTS=ON \
#       -DGAOL_COVERAGE=ON && cmake --build build-coverage --target coverage
#
# The counters of gcov are compiled into GAOL and its tests (--coverage), the
# tests are run, and gcovr writes into coverage/ of the source tree:
#
# - coverage.html, the report line by line, one page holding its own style;
# - README.md, the page of conclusions, which says what is covered and what is
#   not, and whether the 80 % the fork aims at is reached.
#
# The raw data of gcovr (coverage.json) is written in the build directory, not
# in coverage/, which holds only those two files.
#
# GAOL's own sources (gaol/) are counted apart from those of CORE-MATH
# (3rd/math-core), whose accurate phases are reached by a handful of arguments
# in a million: counting them together would say nothing of either.

if(GAOL_COVERAGE)
  if(NOT (CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang"))
    message(FATAL_ERROR "GAOL_COVERAGE needs GCC or Clang, and this build uses ${CMAKE_CXX_COMPILER_ID}.")
  endif()
  if(CMAKE_BUILD_TYPE STREQUAL "Release")
    message(WARNING
      "GAOL_COVERAGE with CMAKE_BUILD_TYPE=Release: the optimizations move the code around, and the counts "
      "no longer follow the source. Configure with -DCMAKE_BUILD_TYPE=Debug.")
  endif()
  message(STATUS "Coverage: ON, target coverage (report in ${CMAKE_CURRENT_SOURCE_DIR}/coverage)")
  target_compile_options(gaol PUBLIC --coverage -fprofile-update=atomic)
  target_link_options(gaol PUBLIC --coverage)

  find_program(GAOL_GCOVR gcovr)
  if(NOT GAOL_GCOVR)
    message(FATAL_ERROR "GAOL_COVERAGE is ON but gcovr was not found: pip install gcovr, or apt install gcovr.")
  endif()
  find_package(Python3 COMPONENTS Interpreter QUIET)
  set(_coverage_dir "${CMAKE_CURRENT_SOURCE_DIR}/coverage")
  add_custom_target(coverage
    COMMAND "${CMAKE_COMMAND}" -E make_directory "${_coverage_dir}"
    # the tests, which ctest runs but does not build
    COMMAND "${CMAKE_COMMAND}" --build "${CMAKE_CURRENT_BINARY_DIR}"
    # the counters of a previous run, which gcov would add to these
    COMMAND find . -name "*.gcda" -delete
    COMMAND "${CMAKE_CTEST_COMMAND}" --output-on-failure
    # --gcov-ignore-errors: gcov cannot infer the working directory of the
    # sources of CORE-MATH, which are compiled from outside the build directory
    COMMAND "${GAOL_GCOVR}" --root "${CMAKE_CURRENT_SOURCE_DIR}"
            --gcov-ignore-errors=no_working_dir_found
            --filter "${CMAKE_CURRENT_SOURCE_DIR}/gaol/"
            --filter "${CMAKE_CURRENT_SOURCE_DIR}/3rd/math-core/"
            # one page, holding its own style: coverage/ keeps no raw file
            --html --html-self-contained --html-title "Coverage of GAOL"
            --output "${_coverage_dir}/coverage.html"
            --json "${CMAKE_CURRENT_BINARY_DIR}/coverage.json"
            --print-summary .
    COMMAND ${Python3_EXECUTABLE} "${CMAKE_CURRENT_SOURCE_DIR}/coverage/coverage_summary.py"
            "${CMAKE_CURRENT_BINARY_DIR}/coverage.json" "${_coverage_dir}/README.md"
    WORKING_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}"
    COMMENT "Running the tests and writing the coverage report in coverage/"
    VERBATIM)
endif()

# GAOL_DEBUG: the targets of this directory, libgaol and the sources of
# CORE-MATH, compiled without optimization nor NDEBUG and with debugging
# information in every configuration (GAOL v5). Each configuration keeps its
# own flags but those of optimization and NDEBUG (the C runtime of Visual C++,
# sanitizers, macros of the project...), so that GAOL still links with the
# code of that configuration; where no build type is given, the flags are
# added to those of the project. The flags of a configuration are those of the
# directory of the target at its end: the tests and the examples, whose
# directories were added before, keep theirs, as does a project bringing GAOL
# in. With Visual C++, the debugging information is the property
# MSVC_DEBUG_INFORMATION_FORMAT where CMP0141 is NEW (CMake 3.25 and later), and
# /Zi otherwise.
if(GAOL_DEBUG)
  if(MSVC)
    set(_gaol_debug_flags /Od /Ob0)
    set(_gaol_cmp0141 OLD)
    if(POLICY CMP0141)
      cmake_policy(GET CMP0141 _gaol_cmp0141)
    endif()
    if(_gaol_cmp0141 STREQUAL "NEW")
      set_property(TARGET gaol gaol_core_math PROPERTY MSVC_DEBUG_INFORMATION_FORMAT ProgramDatabase)
    else()
      list(APPEND _gaol_debug_flags /Zi)
    endif()
  else()
    set(_gaol_debug_flags -O0 -g)
  endif()
  string(REPLACE ";" " " _gaol_debug_string "${_gaol_debug_flags}")
  foreach(lang C CXX)
    if(CMAKE_CONFIGURATION_TYPES OR CMAKE_BUILD_TYPE)
      foreach(config IN LISTS CMAKE_CONFIGURATION_TYPES CMAKE_BUILD_TYPE)
        string(TOUPPER "${config}" config)
        separate_arguments(_gaol_flags UNIX_COMMAND "${CMAKE_${lang}_FLAGS_${config}}")
        list(FILTER _gaol_flags EXCLUDE REGEX "^[-/](O[0-9sgdxz]?|Ofast|Ob[0-3]|DNDEBUG(=.*)?)$")
        list(APPEND _gaol_flags ${_gaol_debug_flags})
        string(REPLACE ";" " " _gaol_flags "${_gaol_flags}")
        set(CMAKE_${lang}_FLAGS_${config} "${_gaol_flags}")
      endforeach()
    else()
      set(CMAKE_${lang}_FLAGS "${CMAKE_${lang}_FLAGS} ${_gaol_debug_string}")
    endif()
  endforeach()
endif()
