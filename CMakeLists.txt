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
# computes its elementary functions with, from the sources of 3rd/mathlib. It
# also builds as a part of another project, brought in with FetchContent.

cmake_minimum_required(VERSION 3.14...3.25)

project(gaol VERSION 5.0.0 LANGUAGES C CXX)

include(GNUInstallDirs)
include(CMakePackageConfigHelpers)
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
# measure. The variable is set for this directory and 3rd/mathlib only.
if(NOT DEFINED CMAKE_POSITION_INDEPENDENT_CODE)
  set(CMAKE_POSITION_INDEPENDENT_CODE ON)
endif()

option(GAOL_PRESERVE_ROUNDING "Restore the rounding direction found after each operation, rather than leaving it upward" OFF)
option(GAOL_BUILD_TESTS "Build the tests of GAOL's bounds, run by ctest (OFF: the tests are not built by default, with the three builds)" OFF)
# The options of configure (--enable-simd, --enable-asm, --enable-verbose-mode),
# with its defaults, but for the verbose mode: GAOL is silent by default
option(GAOL_SIMD "Compute the intervals with SSE2 instructions on x86 processors, and gaol::interval2f with SSE3" ON)
option(GAOL_ASM "Use assembler code (GAOL_USING_ASM)" ON)
option(GAOL_VERBOSE_MODE "Write a line on the standard error when GAOL initializes and cleans up" OFF)
option(GAOL_FLOAT_INTERVALS "Compile gaol::intervalf, and gaol::interval2f with SSE3" OFF)
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
# It made GAOL's basic operations about twice as fast where the code using GAOL
# was compiled with it too (an addition of intervals took 1.48 ns rather than
# 3.02 on an Intel i7-1185G7 with GCC 9 at -O3), and it **breaks the bounds**:
# compiled with it, GAOL failed tests/rounding_direction.cpp, which checks that
# an operation gives the same result whatever the rounding direction the
# calling code left. With GAOL_PRESERVE_ROUNDING, interval("sin(1)+exp(0.1)")
# gave an upper bound one double below the right one when the caller left the
# direction downward or toward zero: the compiler moves floating-point
# operations across the changes of rounding direction, which -frounding-math is
# meant to forbid and which it does not across translation units.
#
# GAOL's whole point is that its intervals enclose the exact values, so there is
# no option for this (fork of GAOL, see doc/three-builds.md). The sources of
# CORE-MATH are compiled apart all the same, which keeps a -flto given by hand
# away from them.


option(GAOL_U128_EMULATION "Compute the 128-bit integer of the accurate phases with two 64-bit halves, as where the compiler has no 128-bit type" OFF)

option(GAOL_COVERAGE "Compile GAOL and its tests with the counters of gcov, and add the target coverage, which runs the tests and writes the report in coverage/" OFF)

option(GAOL_FMA "Compile GAOL, CORE-MATH and mathlib with the fused multiply-add instructions of the processor, where the compiler has a flag for them (OFF: a library for any processor of the architecture)" ON)

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

# mingw-w64 is no longer refused for its version
#
# Two reasons had GAOL refuse the MinGW-w64 of Chocolatey before version 15,
# and both are gone now that GAOL bounds its elementary functions with
# CORE-MATH (fork of GAOL):
#
# - the math library of mingw-w64 before version 12 was not accurate enough for
#   the hyperbolic functions GAOL took from it (acosh() near 1 was up to 25
#   million floats away from the exact value, and asinh() of large negative
#   numbers NaN). GAOL takes no function from the math library of the system any
#   more: CORE-MATH gives them all, correctly rounded, whatever the system;
# - fesetround() of mingw-w64 12 runs the instruction cpuid at each call, which
#   takes microseconds in a virtual machine (3.5 against 130 nanoseconds with
#   mingw-w64 13), and GAOL changed the rounding direction four times for each
#   exp(), log(), sin() or cos() of an interval. It changes it no more: the
#   functions of CORE-MATH are correctly rounded in the direction in effect, so
#   GAOL calls them in the upward rounding it keeps (gaol/gaol_double_op.h).
#
# The continuous integration builds GAOL with the MinGW-w64 GCC 11.2.0, 12.2.0,
# 13.2.0 and 14.2.0 of Chocolatey again, and runs the tests with them.

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

# The fused multiply-add instructions of the processor (fork of GAOL), with
# the flag that gives them to the compiler where it has one, as Codac takes
# them: GAOL's exact products (std::fma() in gaol/gaol_interval.cpp, for the
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
#  - GCC for Windows (MinGW): none. -mfma turns AVX on, whose 32-byte aligned
#    stack GCC does not keep on Windows (crashes in Codac's MinGW builds).
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
  elseif(WIN32 AND CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
    message(STATUS "Fused multiply-add instructions left out: GCC for Windows does not align the stack for AVX")
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
    # fma(a, b, -1) is -2^-60 exactly, which a product rounded first gives as 0
    string(REPLACE ";" " " _gaol_fma_flags "${GAOL_FMA_FLAGS}")
    set(CMAKE_REQUIRED_FLAGS "${GAOL_INTERVAL_FLAGS_STRING} ${_gaol_fma_flags}")
    check_cxx_source_runs("
      #include <cmath>
      int main() {
        volatile double a = 1.0 + 1.0 / 1073741824.0, b = 1.0 - 1.0 / 1073741824.0, c = -1.0;
        return std::fma(a, b, c) == -1.0 / 1152921504606846976.0 ? 0 : 1;
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


################################################################################
# CORE-MATH
################################################################################
#
# GAOL bounds every one of its elementary functions with CORE-MATH
# (https://core-math.gitlabpages.inria.fr, MIT licence), whose sources are in
# 3rd/math-core and are compiled into libgaol itself: there is no library to
# find, to build apart or to install, and nothing to link along with GAOL.
#
# This replaces mathlib (the IBM Accurate Portable Mathematical Library),
# CRlibm and the math library of the system, which GAOL could be built with:
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
  # cbrt, which nth_root(x, 3) takes (fork of GAOL)
  cbrt)

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
  include(CheckCCompilerFlag)
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
  s_nextafter.c gaol_interval_lexer.cpp
  gaol_interval_parser.cpp
)
if(GAOL_FLOAT_INTERVALS)
  list(APPEND GAOL_SOURCES gaol_intervalf.cpp)
  if(USING_SSE3_INSTRUCTIONS)
    list(APPEND GAOL_SOURCES gaol_interval2f.cpp)
  endif()
endif()
if(NOT MSVC)
  list(APPEND GAOL_SOURCES gaol_exact.c)
endif()
list(TRANSFORM GAOL_SOURCES PREPEND "gaol/")

# The sources of CORE-MATH, compiled apart and put into libgaol: apart so that
# the interprocedural optimization GAOL is compiled with does not reach them,
# which made sin and cos half as fast again while it makes the basic
# operations twice as fast (fork of GAOL, see doc/three-builds.md)
add_library(gaol_core_math OBJECT ${GAOL_CORE_MATH_SOURCES})
set_target_properties(gaol_core_math PROPERTIES
  INTERPROCEDURAL_OPTIMIZATION OFF
  POSITION_INDEPENDENT_CODE ON)
# gaol/core_math_port.h included first in each of them, by the compiler
target_compile_options(gaol_core_math PRIVATE ${GAOL_CORE_MATH_FLAGS} ${GAOL_INTERVAL_FLAGS} ${GAOL_FMA_FLAGS} ${GAOL_LIBRARY_FLAGS})
target_include_directories(gaol_core_math PRIVATE
  "${CMAKE_CURRENT_BINARY_DIR}" "${CMAKE_CURRENT_SOURCE_DIR}")

add_library(gaol STATIC ${GAOL_SOURCES} $<TARGET_OBJECTS:gaol_core_math>)


if(GAOL_U128_EMULATION)
  message(STATUS "128-bit integer: the two halves of gaol/gaol_u128.h, as with Visual C++ and on a 32-bit target")
  target_compile_definitions(gaol PUBLIC GAOL_U128_FORCE_EMULATION=1)
endif()
add_library(gaol::gaol ALIAS gaol)

# The generated headers, then GAOL's own: its sources include "gaol/..."
target_include_directories(gaol PUBLIC
  "$<BUILD_INTERFACE:${CMAKE_CURRENT_BINARY_DIR}>"
  "$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>"
  "$<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>"
)
target_compile_options(gaol PUBLIC ${GAOL_INTERVAL_FLAGS} ${GAOL_FMA_FLAGS})
target_compile_options(gaol PRIVATE ${GAOL_LIBRARY_FLAGS})
# Without a build type, CMake gives no optimization flag at all. GAOL sets
# Release when it is the top-level project, but brought in by a project that
# sets none (add_subdirectory, FetchContent), GAOL and CORE-MATH were compiled
# without optimization: they are compiled with -O3 then, as in Release (/O2
# with Visual C++). 3rd/mathlib/CMakeLists.txt does the same for mathlib.
if(NOT CMAKE_BUILD_TYPE AND NOT CMAKE_CONFIGURATION_TYPES)
  if(MSVC)
    target_compile_options(gaol PRIVATE /O2)
  else()
    target_compile_options(gaol PRIVATE -O3)
  endif()
endif()
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

# GAOL's headers, including those gaol/gaol_config.h includes for Visual C++
# and MinGW, which gaol/meson.build does not install
file(GLOB GAOL_HEADERS gaol/*.h)
# What CORE-MATH's sources need to compile within GAOL, and nothing else does
list(REMOVE_ITEM GAOL_HEADERS "${CMAKE_CURRENT_SOURCE_DIR}/gaol/core_math_port.h"
  "${CMAKE_CURRENT_SOURCE_DIR}/gaol/core_math_log_dint.h")
if(NOT GAOL_FLOAT_INTERVALS)
  list(REMOVE_ITEM GAOL_HEADERS "${CMAKE_CURRENT_SOURCE_DIR}/gaol/gaol_intervalf.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/gaol/gaol_interval2f.h")
endif()
install(FILES
  gaol/gaol
  ${GAOL_HEADERS}
  "${CMAKE_CURRENT_BINARY_DIR}/gaol/gaol_configuration.h"
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

# The pkg-config file, for the builds that are not CMake (pkg-config --cflags
# --libs gaol): the flags of interval arithmetic, as gaol::gaol carries them.
# No mathematical library is named any more: the functions of CORE-MATH are
# compiled into libgaol itself. Not with Visual C++, whose libraries pkg-config
# does not name.
if(NOT MSVC)
  set(GAOL_PC_MATHLIB "")
  configure_file(cmake/gaol.pc.in "${CMAKE_CURRENT_BINARY_DIR}/gaol.pc" @ONLY)
  install(FILES "${CMAKE_CURRENT_BINARY_DIR}/gaol.pc" DESTINATION ${CMAKE_INSTALL_LIBDIR}/pkgconfig)
endif()


################################################################################
# Tests
################################################################################

if(GAOL_BUILD_TESTS)
  enable_testing()
  add_subdirectory(tests)
endif()


################################################################################
# Coverage
################################################################################
#
# cmake -S . -B build-coverage -DCMAKE_BUILD_TYPE=Debug -DGAOL_BUILD_TESTS=ON \
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
    COMMAND ${Python3_EXECUTABLE} "${CMAKE_CURRENT_SOURCE_DIR}/scripts/coverage_summary.py"
            "${CMAKE_CURRENT_BINARY_DIR}/coverage.json" "${_coverage_dir}/README.md"
    WORKING_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}"
    COMMENT "Running the tests and writing the coverage report in coverage/"
    VERBATIM)
endif()
