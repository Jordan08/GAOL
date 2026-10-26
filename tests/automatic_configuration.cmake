# ==================================================================
#  The configuration of GAOL decided by itself (GAOL v5)
# ==================================================================
#
# The options of the user are GAOL_PRESERVE_ROUNDING, GAOL_SIMD and
# GAOL_COVERAGE (with WITH_TESTS, WITH_EXAMPLES and the variables of CMake),
# and the rest of the configuration is decided by checks of the compiler and of
# the machine: the fused multiply-add instructions, the AVX-512 path, the
# 128-bit integer, the debugging. This script configures GAOL as the project
# built, with the settings that a user or a developer gives, and checks what
# gaol_configuration.h and the messages of CMake say of each:
#
#   - the defaults: no AVX-512 path, no debugging, exceptions, silent, and the
#     vector instructions of the processor (SSE2 on x86-64, NEON on 64-bit ARM);
#   - GAOL_SIMD=OFF: the FPU, whatever the processor, and so no AVX-512 path;
#     the old GAOL_ARM64=OFF no longer reads (and says so);
#   - the AVX-512 path follows GAOL_PRESERVE_ROUNDING at every configure, in
#     the same directory too (a default that a second cmake took for a request
#     left the path out, point 112 of TODO.md), where the compiler has one;
#     -DGAOL_PREFER_AVX512=OFF refuses it under GAOL_PRESERVE_ROUNDING, =ON asks
#     for it without, and an error says so where no compiler could take it;
#   - the Debug build type is the build for debugging, GAOL_DEBUGGING follows it,
#     and nothing but it and -DGAOL_DEBUG=ON sets it: -DGAOL_DEBUGGING=ON is
#     overwritten;
#   - -DGAOL_FMA=OFF leaves the fused multiply-add instructions out; -DGAOL_U128_
#     EMULATION=ON has the two halves of the 128-bit integer; the settings of the
#     developers GAOL_EXCEPTIONS_ENABLED=OFF and GAOL_VERBOSE_MODE=ON;
#   - a cross-compilation, where no program can be run to check the machine,
#     leaves the fused multiply-add instructions and the AVX-512 path out, and
#     takes them on the word of -DGAOL_MACHINE_RUNS_FMA=1 and
#     -DGAOL_MACHINE_RUNS_AVX512=1.
#
# What a platform may not give (an AVX-512 path on ARM, an FMA flag on 64-bit
# ARM) is not asked for: the script compares each configuration with what its
# own checks found, which the cache of CMake holds.
#
# ctest runs it (tests/CMakeLists.txt), and by hand:
#
#   cmake -DGAOL_SOURCE_DIR=<sources of GAOL> -DGAOL_WORK_DIR=<directory>/automatic_configuration \
#         -DGAOL_GENERATOR=<generator> [-DGAOL_MAKE_PROGRAM=<its program>] \
#         "-DGAOL_C_COMPILER=<cc and its arguments>" "-DGAOL_CXX_COMPILER=<c++ and its arguments>" \
#         [-DGAOL_CONFIGURE_ARGS=<arguments for every configuration, a list>] \
#         -P tests/automatic_configuration.cmake
#
# GAOL_CONFIGURE_ARGS gives an architecture that is not the machine's: on an
# ARM Mac, a configuration for x86-64 whose programs cannot run (no Rosetta)
# takes the AVX-512 path where the results of the checks that run them are
# given,
#
#   "-DGAOL_CONFIGURE_ARGS=-DCMAKE_OSX_ARCHITECTURES=x86_64;-DGAOL_MACHINE_RUNS_AVX512=1;-DGAOL_MACHINE_RUNS_FMA=1"
#
# The configurations are made in the same directory of build, which keeps the
# results of the checks of the compiler: the first one takes seconds, and the
# others a tenth of a second; the settings of the developers a configuration
# gives are removed from the cache by the next one (-U), and the cross
# compilations are made in a directory of their own. The sources are those of
# the tree, which a configuration does not write into, and the compilers and
# the generator those that built GAOL, as cpack_stale_configure.cmake gives
# them.
#
# Copyright (c) 2026 ENSTA, France
#
# Created 2026-10-26 by Jordan NININ

cmake_minimum_required(VERSION 3.14)

foreach(_name GAOL_SOURCE_DIR GAOL_WORK_DIR GAOL_GENERATOR GAOL_C_COMPILER GAOL_CXX_COMPILER)
  if("${${_name}}" STREQUAL "")
    message(FATAL_ERROR "${_name} is not given (see the head of this file)")
  endif()
endforeach()
# The script removes the whole of this directory, when it starts, and when it
# passes (a failure leaves it to be looked at): it has to be the directory of
# this test
if(NOT GAOL_WORK_DIR MATCHES "/automatic_configuration$")
  message(FATAL_ERROR "GAOL_WORK_DIR, ${GAOL_WORK_DIR}, is not a directory of this test (automatic_configuration)")
endif()

file(REMOVE_RECURSE "${GAOL_WORK_DIR}")
file(MAKE_DIRECTORY "${GAOL_WORK_DIR}")

# The program of the generator, which CMake looks for in PATH otherwise: Ninja
# can be elsewhere
set(_make_program)
if(NOT "${GAOL_MAKE_PROGRAM}" STREQUAL "")
  set(_make_program "-DCMAKE_MAKE_PROGRAM=${GAOL_MAKE_PROGRAM}")
endif()

# The settings that a configuration gives and the next one must not keep
set(_unset
  -UGAOL_PRESERVE_ROUNDING -UGAOL_SIMD -UGAOL_ARM64 -UGAOL_FMA -UGAOL_PREFER_AVX512
  -UGAOL_PREFER_AVX512_DERIVED
  -UGAOL_U128_EMULATION -UGAOL_DEBUG -UGAOL_DEBUGGING -UGAOL_VERBOSE_MODE
  -UGAOL_EXCEPTIONS_ENABLED -UCMAKE_BUILD_TYPE)

# Configures GAOL in the directory of build <build> with the arguments given:
# <status> and <output> (all the messages of CMake) are set in the caller, and
# where it configured, GAOL_<NAME> for each macro of gaol_configuration.h that
# the checks decide, 1 or 0.
function(configure_gaol build)
  execute_process(
    COMMAND "${CMAKE_COMMAND}" -E env "CC=${GAOL_C_COMPILER}" "CXX=${GAOL_CXX_COMPILER}"
            "${CMAKE_COMMAND}" -S "${GAOL_SOURCE_DIR}" -B "${build}" -G "${GAOL_GENERATOR}" ${_make_program} ${GAOL_CONFIGURE_ARGS} ${ARGN}
    RESULT_VARIABLE _status OUTPUT_VARIABLE _out ERROR_VARIABLE _error)
  set(status "${_status}" PARENT_SCOPE)
  set(output "${_out}\n${_error}" PARENT_SCOPE)
  foreach(_macro DEBUGGING EXCEPTIONS_ENABLED PRESERVE_ROUNDING VERBOSE_MODE USING_SSE2_INSTRUCTIONS
                 USING_NEON_INSTRUCTIONS HAVE_AVX512_TARGET PREFER_AVX512 AVX512_ARITHMETIC)
    set(_value "")
    if(_status EQUAL 0)
      file(STRINGS "${build}/gaol/gaol_configuration.h" _lines REGEX "^#define GAOL_${_macro} ")
      if(_lines MATCHES "GAOL_${_macro} 1")
        set(_value 1)
      else()
        set(_value 0)
      endif()
    endif()
    set(${_macro} "${_value}" PARENT_SCOPE)
  endforeach()
endfunction()

# A value of the cache of a build: empty where a check failed, which the cache
# keeps as an empty string; a check that was not made has no entry, and
# <required> says whether it should have
function(cache_value build name result required)
  set(${result} "" PARENT_SCOPE)
  file(STRINGS "${build}/CMakeCache.txt" _lines REGEX "^${name}:")
  if(_lines MATCHES "^${name}:[A-Z]*=([^;]*)")
    set(${result} "${CMAKE_MATCH_1}" PARENT_SCOPE)
  elseif(required)
    message(FATAL_ERROR "The cache of ${build} has no ${name}: the configuration did not make the check")
  endif()
endfunction()

function(expect title condition)
  # The words of the condition as arguments of if(), not one argument
  string(REPLACE " " ";" _condition "${condition}")
  if(NOT (${_condition}))
    message(FATAL_ERROR "${title}: expected (${condition}) of the configuration:\n"
      "status ${status}, DEBUGGING ${DEBUGGING}, EXCEPTIONS_ENABLED ${EXCEPTIONS_ENABLED}, PRESERVE_ROUNDING ${PRESERVE_ROUNDING}, "
      "VERBOSE_MODE ${VERBOSE_MODE}, SSE2 ${USING_SSE2_INSTRUCTIONS}, NEON ${USING_NEON_INSTRUCTIONS}, "
      "HAVE_AVX512_TARGET ${HAVE_AVX512_TARGET}, PREFER_AVX512 ${PREFER_AVX512}, AVX512_ARITHMETIC ${AVX512_ARITHMETIC}\n${output}")
  endif()
endfunction()

# A configuration that has to succeed
function(configure_ok title build)
  configure_gaol("${build}" ${_unset} ${ARGN})
  if(NOT status EQUAL 0)
    message(FATAL_ERROR "${title}: CMake did not configure:\n${output}")
  endif()
  foreach(_name status output DEBUGGING EXCEPTIONS_ENABLED PRESERVE_ROUNDING VERBOSE_MODE USING_SSE2_INSTRUCTIONS
                USING_NEON_INSTRUCTIONS HAVE_AVX512_TARGET PREFER_AVX512 AVX512_ARITHMETIC)
    set(${_name} "${${_name}}" PARENT_SCOPE)
  endforeach()
endfunction()

set(_build "${GAOL_WORK_DIR}/build")

# The target of the compiler that built GAOL, which a script of CMake does not
# know (CMAKE_HOST_SYSTEM_PROCESSOR is empty in -P mode, and the processor of the
# machine is not that of the compiler in a 32-bit container on a 64-bit kernel,
# armhf on aarch64): its predefined macros. Not asked of a configuration of
# another architecture (GAOL_CONFIGURE_ARGS).
set(_target "")
if("${GAOL_CONFIGURE_ARGS}" STREQUAL "")
  separate_arguments(_cxx UNIX_COMMAND "${GAOL_CXX_COMPILER}")
  execute_process(COMMAND ${_cxx} -dM -E -x c++ -
    INPUT_FILE /dev/null OUTPUT_VARIABLE _predefined ERROR_QUIET RESULT_VARIABLE _status)
  if(_status EQUAL 0)
    if(_predefined MATCHES "#define __x86_64__ 1" OR _predefined MATCHES "#define __i386__ 1")
      set(_target x86)
    elseif(_predefined MATCHES "#define __aarch64__ 1")
      set(_target aarch64)
    endif()
  endif()
endif()

# ---- The defaults ------------------------------------------------------------

configure_ok("the defaults" "${_build}")
expect("the defaults" "DEBUGGING EQUAL 0 AND EXCEPTIONS_ENABLED EQUAL 1 AND PRESERVE_ROUNDING EQUAL 0 AND VERBOSE_MODE EQUAL 0")
expect("the defaults: the AVX-512 path is for GAOL_PRESERVE_ROUNDING" "PREFER_AVX512 EQUAL 0 AND AVX512_ARITHMETIC EQUAL 0")
math(EXPR _vector_kinds "${USING_SSE2_INSTRUCTIONS} + ${USING_NEON_INSTRUCTIONS}")
expect("the defaults: one kind of vector intervals at most" "_vector_kinds LESS 2")
# The vector instructions of the processor, as soon as it has them
if(_target STREQUAL "x86")
  expect("the defaults on x86-64: SSE2" "USING_SSE2_INSTRUCTIONS EQUAL 1 AND USING_NEON_INSTRUCTIONS EQUAL 0")
elseif(_target STREQUAL "aarch64")
  expect("the defaults on 64-bit ARM: NEON" "USING_NEON_INSTRUCTIONS EQUAL 1 AND USING_SSE2_INSTRUCTIONS EQUAL 0")
endif()
set(_default_sse2 ${USING_SSE2_INSTRUCTIONS})
set(_default_neon ${USING_NEON_INSTRUCTIONS})
set(_has_avx512_target ${HAVE_AVX512_TARGET})
cache_value("${_build}" GAOL_MACHINE_RUNS_FMA _runs_fma OFF)
string(REGEX MATCH "Fused multiply-add flags: [^\n]*\n" _default_fma_line "${output}")
cache_value("${_build}" GAOL_COMPILER_HAS_U128 _has_u128 ON)
# The fused multiply-add flags, as the message says them: not empty where the
# machine runs the instructions the flag gives, on x86
if(_runs_fma AND NOT output MATCHES "Fused multiply-add flags: [^\n]")
  message(FATAL_ERROR "this machine runs a program compiled with the flag of the fused multiply-add instructions, and the default configuration takes none:\n${output}")
endif()
# The 128-bit integer: the two halves where the compiler has no type, and then
# only
if(_has_u128)
  if(output MATCHES "128-bit integer: the two halves")
    message(FATAL_ERROR "the compiler has a 128-bit integer type, and the default configuration takes the two halves:\n${output}")
  endif()
else()
  if(NOT output MATCHES "128-bit integer: the two halves")
    message(FATAL_ERROR "the compiler has no 128-bit integer type (GAOL_COMPILER_HAS_U128 is \"${_has_u128}\"), and the default configuration does not take the two halves:\n${output}")
  endif()
endif()
message(STATUS "The defaults: as expected")

# ---- GAOL_SIMD, and GAOL_ARM64 that is gone ---------------------------------

configure_ok("GAOL_SIMD=OFF" "${_build}" -DGAOL_SIMD=OFF)
expect("GAOL_SIMD=OFF: the FPU" "USING_SSE2_INSTRUCTIONS EQUAL 0 AND USING_NEON_INSTRUCTIONS EQUAL 0")
expect("GAOL_SIMD=OFF: no AVX-512 path, which is for the SSE2 intervals" "HAVE_AVX512_TARGET EQUAL 0 AND AVX512_ARITHMETIC EQUAL 0")
configure_ok("GAOL_SIMD=OFF with GAOL_PRESERVE_ROUNDING=ON" "${_build}" -DGAOL_SIMD=OFF -DGAOL_PRESERVE_ROUNDING=ON)
expect("GAOL_SIMD=OFF under GAOL_PRESERVE_ROUNDING: no AVX-512 path" "PREFER_AVX512 EQUAL 0 AND AVX512_ARITHMETIC EQUAL 0")

# -DGAOL_ARM64=OFF with GAOL_SIMD on: the vector intervals all the same, and a
# warning that it no longer reads. The cache of the directory holds it: the next
# configuration (-UGAOL_ARM64) forgets it.
configure_ok("GAOL_ARM64=OFF" "${_build}" -DGAOL_ARM64=OFF)
expect("GAOL_ARM64=OFF: the vector intervals of the defaults" "USING_SSE2_INSTRUCTIONS EQUAL ${_default_sse2} AND USING_NEON_INSTRUCTIONS EQUAL ${_default_neon}")
if(NOT output MATCHES "GAOL_ARM64=OFF is no longer an option")
  message(FATAL_ERROR "-DGAOL_ARM64=OFF says nothing of being no longer read:\n${output}")
endif()
message(STATUS "GAOL_SIMD=OFF, and GAOL_ARM64=OFF that is read no more: as expected")

# ---- The AVX-512 path --------------------------------------------------------

# Where the compiler has the path: preferred under GAOL_PRESERVE_ROUNDING, taken
# if the machine runs it (the check of the cache). Where it has none (ARM), left
# out with a message, and the macros are the same as without
configure_ok("GAOL_PRESERVE_ROUNDING=ON" "${_build}" -DGAOL_PRESERVE_ROUNDING=ON)
expect("GAOL_PRESERVE_ROUNDING=ON" "PRESERVE_ROUNDING EQUAL 1 AND PREFER_AVX512 EQUAL ${_has_avx512_target}")
cache_value("${_build}" GAOL_MACHINE_RUNS_AVX512 _runs_avx512 OFF)
if(PREFER_AVX512 AND _runs_avx512)
  expect("GAOL_PRESERVE_ROUNDING=ON on a machine with AVX-512" "AVX512_ARITHMETIC EQUAL 1")
else()
  expect("GAOL_PRESERVE_ROUNDING=ON without AVX-512 here" "AVX512_ARITHMETIC EQUAL 0")
endif()
set(_with_preserve_prefer ${PREFER_AVX512})
set(_with_preserve_taken ${AVX512_ARITHMETIC})
if(NOT _has_avx512_target AND NOT output MATCHES "AVX-512: no path here")
  message(FATAL_ERROR "GAOL_PRESERVE_ROUNDING=ON, and no AVX-512 path with this compiler, with no message:\n${output}")
endif()

# The same directory, the same command line again, and without the option
# (-DGAOL_PRESERVE_ROUNDING stays in the cache): nothing of the first configure
# is kept as a request
configure_gaol("${_build}")
expect("a second cmake, with no command line" "status EQUAL 0 AND PREFER_AVX512 EQUAL ${_with_preserve_prefer} AND AVX512_ARITHMETIC EQUAL ${_with_preserve_taken}")
# GAOL_PRESERVE_ROUNDING off in the same directory: the path goes with it (the
# default that a second cmake took for a request), and comes back
configure_gaol("${_build}" -DGAOL_PRESERVE_ROUNDING=OFF)
expect("GAOL_PRESERVE_ROUNDING=OFF after =ON, in the same directory" "status EQUAL 0 AND PREFER_AVX512 EQUAL 0 AND AVX512_ARITHMETIC EQUAL 0")
configure_gaol("${_build}" -DGAOL_PRESERVE_ROUNDING=ON)
expect("GAOL_PRESERVE_ROUNDING=ON again" "status EQUAL 0 AND PREFER_AVX512 EQUAL ${_with_preserve_prefer} AND AVX512_ARITHMETIC EQUAL ${_with_preserve_taken}")
message(STATUS "The AVX-512 path follows GAOL_PRESERVE_ROUNDING at every configure: as expected")

# A request of OFF under GAOL_PRESERVE_ROUNDING: refused, and the next cmake
# keeps it; -U gives the default again
configure_ok("-DGAOL_PREFER_AVX512=OFF" "${_build}" -DGAOL_PRESERVE_ROUNDING=ON -DGAOL_PREFER_AVX512=OFF)
expect("-DGAOL_PREFER_AVX512=OFF under GAOL_PRESERVE_ROUNDING" "PREFER_AVX512 EQUAL 0 AND AVX512_ARITHMETIC EQUAL 0")
configure_gaol("${_build}")
expect("the cmake after -DGAOL_PREFER_AVX512=OFF" "status EQUAL 0 AND PREFER_AVX512 EQUAL 0")
configure_gaol("${_build}" -UGAOL_PREFER_AVX512)
expect("-UGAOL_PREFER_AVX512" "status EQUAL 0 AND PREFER_AVX512 EQUAL ${_with_preserve_prefer}")
message(STATUS "-DGAOL_PREFER_AVX512=OFF under GAOL_PRESERVE_ROUNDING: refused, as expected")

# A request of ON without GAOL_PRESERVE_ROUNDING: taken where the compiler has
# the path, an error where it has none
configure_gaol("${_build}" ${_unset} -DGAOL_PREFER_AVX512=ON)
if(_has_avx512_target)
  expect("-DGAOL_PREFER_AVX512=ON" "status EQUAL 0 AND PRESERVE_ROUNDING EQUAL 0 AND PREFER_AVX512 EQUAL 1")
else()
  string(REGEX REPLACE "[ \t\r\n]+" " " _text "${output}")
  if(status EQUAL 0 OR NOT _text MATCHES "GAOL_PREFER_AVX512 takes the SSE2 intervals of x86-64")
    message(FATAL_ERROR "-DGAOL_PREFER_AVX512=ON, where the compiler has no AVX-512 path, should be an error:\n${output}")
  endif()
endif()
message(STATUS "-DGAOL_PREFER_AVX512=ON without GAOL_PRESERVE_ROUNDING: as expected")

# A build directory of the older CMake, which kept GAOL_PREFER_AVX512 with the
# record GAOL_PREFER_AVX512_DERIVED: an entry that differs from the record, or
# that the command line gave, is a request and is kept; the record goes
if(_has_avx512_target)
  configure_gaol("${_build}" ${_unset} -DGAOL_PREFER_AVX512_DERIVED=OFF -DGAOL_PREFER_AVX512=ON)
  expect("a request of ON next to the record of an older CMake" "status EQUAL 0 AND PREFER_AVX512 EQUAL 1")
  file(STRINGS "${_build}/CMakeCache.txt" _records REGEX "^GAOL_PREFER_AVX512_DERIVED")
  if(_records)
    message(FATAL_ERROR "the record GAOL_PREFER_AVX512_DERIVED of an older CMake is kept:\n${_records}")
  endif()
  message(STATUS "The record of an older CMake: as expected")
endif()

# ---- Debugging ----------------------------------------------------------------

configure_ok("Debug" "${_build}" -DCMAKE_BUILD_TYPE=Debug)
expect("CMAKE_BUILD_TYPE=Debug" "DEBUGGING EQUAL 1")
configure_ok("Release after Debug" "${_build}" -DCMAKE_BUILD_TYPE=Release)
expect("CMAKE_BUILD_TYPE=Release after Debug" "DEBUGGING EQUAL 0")
# Nothing but the build type and -DGAOL_DEBUG=ON sets GAOL_DEBUGGING
configure_ok("-DGAOL_DEBUGGING=ON" "${_build}" -DGAOL_DEBUGGING=ON)
expect("-DGAOL_DEBUGGING=ON is overwritten" "DEBUGGING EQUAL 0")
configure_ok("-DGAOL_DEBUG=ON" "${_build}" -DGAOL_DEBUG=ON)
expect("-DGAOL_DEBUG=ON" "DEBUGGING EQUAL 1")
message(STATUS "GAOL_DEBUGGING follows the Debug build type: as expected")

# ---- The fused multiply-add instructions and the 128-bit integer -------------

configure_ok("-DGAOL_FMA=OFF" "${_build}" -DGAOL_FMA=OFF)
if(NOT output MATCHES "Fused multiply-add flags: \n")
  message(FATAL_ERROR "-DGAOL_FMA=OFF, and the configuration takes a flag of the fused multiply-add instructions:\n${output}")
endif()
configure_ok("-DGAOL_U128_EMULATION=ON" "${_build}" -DGAOL_U128_EMULATION=ON)
if(NOT output MATCHES "128-bit integer: the two halves")
  message(FATAL_ERROR "-DGAOL_U128_EMULATION=ON, and the configuration does not take the two halves:\n${output}")
endif()
message(STATUS "-DGAOL_FMA=OFF and -DGAOL_U128_EMULATION=ON: as expected")

# ---- The settings of the developers -------------------------------------------

configure_ok("-DGAOL_EXCEPTIONS_ENABLED=OFF" "${_build}" -DGAOL_EXCEPTIONS_ENABLED=OFF -DGAOL_VERBOSE_MODE=ON)
expect("-DGAOL_EXCEPTIONS_ENABLED=OFF, -DGAOL_VERBOSE_MODE=ON" "EXCEPTIONS_ENABLED EQUAL 0 AND VERBOSE_MODE EQUAL 1")
configure_ok("the defaults again" "${_build}")
expect("the settings of the developers, taken out of the cache" "EXCEPTIONS_ENABLED EQUAL 1 AND VERBOSE_MODE EQUAL 0 AND DEBUGGING EQUAL 0")
message(STATUS "GAOL_EXCEPTIONS_ENABLED=OFF and GAOL_VERBOSE_MODE=ON: as expected")

# ---- A cross-compilation --------------------------------------------------------

# CMAKE_SYSTEM_NAME given makes CMake cross-compile, here for the machine's own
# system, so that the program that checks the machine cannot be run
set(_cross "${GAOL_WORK_DIR}/cross")
set(_cross_args -DCMAKE_SYSTEM_NAME=${CMAKE_HOST_SYSTEM_NAME} -DGAOL_PRESERVE_ROUNDING=ON)
configure_ok("a cross-compilation" "${_cross}" ${_cross_args})
if(NOT output MATCHES "Fused multiply-add flags: \n")
  message(FATAL_ERROR "a cross-compilation takes the fused multiply-add instructions on the word of the compiler:\n${output}")
endif()
expect("a cross-compilation leaves the AVX-512 path out" "AVX512_ARITHMETIC EQUAL 0")
set(_cross_prefer ${PREFER_AVX512})
# ... unless the machine running GAOL is said to run it
file(REMOVE_RECURSE "${_cross}")
configure_ok("a cross-compilation for a machine that runs them" "${_cross}" ${_cross_args}
  -DGAOL_MACHINE_RUNS_FMA=1 -DGAOL_MACHINE_RUNS_AVX512=1)
expect("-DGAOL_MACHINE_RUNS_AVX512=1 in a cross-compilation" "PREFER_AVX512 EQUAL ${_cross_prefer} AND AVX512_ARITHMETIC EQUAL ${_cross_prefer}")
# ... and -DGAOL_MACHINE_RUNS_FMA=1 has it take the flags a native configuration took
string(FIND "${output}" "${_default_fma_line}" _fma_found)
if(_fma_found EQUAL -1)
  message(FATAL_ERROR "-DGAOL_MACHINE_RUNS_FMA=1 in a cross-compilation does not give the flags of the native configuration, ${_default_fma_line}:\n${output}")
endif()
message(STATUS "A cross-compilation: as expected")

file(REMOVE_RECURSE "${GAOL_WORK_DIR}")
