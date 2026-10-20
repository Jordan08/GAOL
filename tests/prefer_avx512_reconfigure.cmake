# ==================================================================
#  A second cmake in the same directory, and GAOL_PREFER_AVX512 (GAOL v5)
# ==================================================================
#
# GAOL_PREFER_AVX512 is ON by default where GAOL_PRESERVE_ROUNDING is, and OFF
# otherwise, and what the command line gives wins. CMakeLists.txt told the two
# apart by DEFINED, which saw at the second configure of a directory the entry
# the first one had made, and took it for a request of the command line: a
# directory configured with -DGAOL_PRESERVE_ROUNDING=ON lost the AVX-512 path
# at its second cmake (point 112 of TODO.md). This script configures GAOL as
# the project built, several times in the same directory, and checks that
# what a configure derives or is given does not depend on the ones before:
#
#   - the same default, derived again at the second configure, without the
#     command line (cmake <directory>), and with it, gives the same: the same
#     messages about the path, the same gaol_configuration.h, and an entry
#     that says what the configuration uses;
#   - a default that follows GAOL_PRESERVE_ROUNDING at every configure, off
#     and on again;
#   - a request of OFF (-DGAOL_PREFER_AVX512=OFF) that is honoured under
#     GAOL_PRESERVE_ROUNDING, after a default that was ON and after one that
#     was OFF (the request is then the value the entry holds), and that stays
#     for the next configure; -UGAOL_PREFER_AVX512 derives the default again;
#   - a request of ON, which stays too: it is taken where the compiler has the
#     path, and refused with the message of CMakeLists.txt where it has not
#     (ARM), the same at the next configure, which is then no default derived
#     that leaves the path out without a message.
#
# Where the compiler has no AVX-512 path (ARM), the default derived under
# GAOL_PRESERVE_ROUNDING is left out with a message, which every configure has
# to give, and the macros of the header are the same either way; where it has
# one, the macros differ too (GAOL_PREFER_AVX512 and GAOL_AVX512_ARITHMETIC are
# defined or not). The script compares what one configure gives with what
# another does, and holds each to what its command line says, not to a value a
# platform may not give.
#
# ctest runs it (tests/CMakeLists.txt), and by hand:
#
#   cmake -DGAOL_SOURCE_DIR=<sources of GAOL> -DGAOL_WORK_DIR=<directory>/prefer_avx512_reconfigure \
#         -DGAOL_GENERATOR=<generator> [-DGAOL_MAKE_PROGRAM=<its program>] \
#         "-DGAOL_C_COMPILER=<cc and its arguments>" "-DGAOL_CXX_COMPILER=<c++ and its arguments>" \
#         [-DGAOL_CONFIGURE_ARGS=<arguments for every configuration, a list>] \
#         -P tests/prefer_avx512_reconfigure.cmake
#
# GAOL_CONFIGURE_ARGS gives the path an architecture that is not the machine's:
# on an ARM Mac, a configuration for x86-64 whose programs cannot run (no
# Rosetta) takes the AVX-512 path where the results of the checks that run
# them are given,
#
#   "-DGAOL_CONFIGURE_ARGS=-DCMAKE_OSX_ARCHITECTURES=x86_64;-DGAOL_MACHINE_RUNS_AVX512=1;-DGAOL_MACHINE_RUNS_FMA=1"
#
# and the header then differs between a default derived and a path refused.
#
# The configurations are made in the same directory of build, which keeps the
# results of the checks of the compiler: the first one takes seconds, and the
# others a tenth of a second. The sources are those of the tree, which a
# configuration does not write into, and the compilers and the generator those
# that built GAOL, as cpack_stale_configure.cmake gives them.
#
# Copyright (c) 2026 ENSTA, France
#
# Created 2026-10-20 by Jordan NININ

cmake_minimum_required(VERSION 3.14)

foreach(_name GAOL_SOURCE_DIR GAOL_WORK_DIR GAOL_GENERATOR GAOL_C_COMPILER GAOL_CXX_COMPILER)
  if("${${_name}}" STREQUAL "")
    message(FATAL_ERROR "${_name} is not given (see the head of this file)")
  endif()
endforeach()
# The script removes the whole of this directory, when it starts, and when it
# passes (a failure leaves it to be looked at): it has to be the directory of
# this test
if(NOT GAOL_WORK_DIR MATCHES "/prefer_avx512_reconfigure$")
  message(FATAL_ERROR "GAOL_WORK_DIR, ${GAOL_WORK_DIR}, is not a directory of this test (prefer_avx512_reconfigure)")
endif()

set(_build "${GAOL_WORK_DIR}/build")
file(REMOVE_RECURSE "${GAOL_WORK_DIR}")
file(MAKE_DIRECTORY "${GAOL_WORK_DIR}")

# The program of the generator, which CMake looks for in PATH otherwise: Ninja
# can be elsewhere
set(_make_program)
if(NOT "${GAOL_MAKE_PROGRAM}" STREQUAL "")
  set(_make_program "-DCMAKE_MAKE_PROGRAM=${GAOL_MAKE_PROGRAM}")
endif()

# Configures GAOL in the directory of build with the arguments given, and
# describes what came of it in <signature>: the status, the lines of the
# messages about the AVX-512 path (the ones the configuration writes at every
# configure, not the checks of the compiler, which only the first one runs),
# the entry GAOL_PREFER_AVX512 of the cache and its record, and where it
# configured the header it wrote. Also in <output>, the whole of the messages.
function(configure_gaol signature output)
  execute_process(
    COMMAND "${CMAKE_COMMAND}" -E env "CC=${GAOL_C_COMPILER}" "CXX=${GAOL_CXX_COMPILER}"
            "${CMAKE_COMMAND}" -S "${GAOL_SOURCE_DIR}" -B "${_build}" -G "${GAOL_GENERATOR}" ${_make_program} ${GAOL_CONFIGURE_ARGS} ${ARGN}
    RESULT_VARIABLE _status OUTPUT_VARIABLE _out ERROR_VARIABLE _error)
  set(_text "${_out}\n${_error}")
  # Wrapped lines of CMake's errors begin with blanks: kept as they are, the
  # two configurations wrap the same text at the same places
  string(REGEX MATCHALL "[^\n]*(AVX-512|GAOL_PREFER_AVX512)[^\n]*" _lines "${_text}")
  string(REPLACE ";" "\n" _lines "${_lines}")
  set(_entry "(no entry)")
  set(_record "(no record)")
  if(EXISTS "${_build}/CMakeCache.txt")
    file(STRINGS "${_build}/CMakeCache.txt" _cache REGEX "^GAOL_PREFER_AVX512(_DERIVED)?:")
    foreach(_line ${_cache})
      if(_line MATCHES "^GAOL_PREFER_AVX512:[A-Z]*=(.*)$")
        set(_entry "${CMAKE_MATCH_1}")
      elseif(_line MATCHES "^GAOL_PREFER_AVX512_DERIVED:[A-Z]*=(.*)$")
        set(_record "${CMAKE_MATCH_1}")
      endif()
    endforeach()
  endif()
  set(_header "")
  if(_status EQUAL 0)
    file(STRINGS "${_build}/gaol/gaol_configuration.h" _header
      REGEX "^(#define|/\\* #undef) GAOL_(HAVE_AVX512_TARGET|PREFER_AVX512|AVX512_ARITHMETIC)")
    string(REPLACE ";" "\n" _header "${_header}")
  endif()
  set(${signature}
    "status ${_status}\n${_lines}\nentry ${_entry}, record ${_record}\n${_header}" PARENT_SCOPE)
  set(${output} "${_text}" PARENT_SCOPE)
endfunction()

# The configurations are told apart by the words their messages hold
function(contains text regex result)
  if("${text}" MATCHES "${regex}")
    set(${result} ON PARENT_SCOPE)
  else()
    set(${result} OFF PARENT_SCOPE)
  endif()
endfunction()

# The checks that hold of every configuration that succeeded: it said what it
# prefers on its last line, the entry says the same, and the header has the
# macro as the preference and the taking of the path say it (never taken
# unless preferred)
function(check_consistent title signature output)
  if(NOT "${output}" MATCHES "AVX-512 interval arithmetic: the compiler takes it: ([A-Z]+), preferred: ([A-Z]+), taken: ([A-Z]+)")
    message(FATAL_ERROR "${title}: no line \"AVX-512 interval arithmetic: ...\" in the output of CMake:\n${output}")
  endif()
  set(_preferred "${CMAKE_MATCH_2}")
  set(_taken "${CMAKE_MATCH_3}")
  if(NOT signature MATCHES "entry ([A-Z]+), record")
    message(FATAL_ERROR "${title}: no entry GAOL_PREFER_AVX512 in the cache (signature:\n${signature})")
  endif()
  if(NOT "${CMAKE_MATCH_1}" STREQUAL "${_preferred}")
    message(FATAL_ERROR "${title}: the entry GAOL_PREFER_AVX512 holds ${CMAKE_MATCH_1}, and the configuration prefers ${_preferred}:\n${signature}")
  endif()
  if(_preferred STREQUAL "ON")
    set(_define "#define GAOL_PREFER_AVX512 1")
  else()
    set(_define "/\\* #undef GAOL_PREFER_AVX512 \\*/")
  endif()
  if(NOT signature MATCHES "\n${_define}")
    message(FATAL_ERROR "${title}: the configuration prefers ${_preferred}, and the header does not say it:\n${signature}")
  endif()
  if(_taken STREQUAL "ON" AND NOT _preferred STREQUAL "ON")
    message(FATAL_ERROR "${title}: the path is taken without being preferred:\n${output}")
  endif()
endfunction()

# Two configurations that have to be the same
function(check_same title first second)
  if(NOT "${first}" STREQUAL "${second}")
    message(FATAL_ERROR "${title}: the configurations differ.\n--- the first:\n${first}\n--- the second:\n${second}")
  endif()
endfunction()

# The message of a default derived under GAOL_PRESERVE_ROUNDING that the
# compiler has no path for
set(_left_out "GAOL_PREFER_AVX512, from GAOL_PRESERVE_ROUNDING: no AVX-512 path here")

# ---- The default derived at every configure --------------------------------

configure_gaol(_first _first_out -DGAOL_PRESERVE_ROUNDING=ON)
if(NOT _first MATCHES "^status 0\n")
  message(FATAL_ERROR "GAOL_PRESERVE_ROUNDING=ON: CMake did not configure (the first time):\n${_first_out}")
endif()
# Whether the compiler has the path, by what it says: the default left out, or
# a path preferred (taken, or left out on a machine that does not run it)
contains("${_first_out}" "${_left_out}" _no_path)
if(_no_path)
  message(STATUS "prefer_avx512_reconfigure: no AVX-512 path with this compiler, the default is left out with a message")
endif()

configure_gaol(_second _second_out)
check_same("the second cmake, without the command line, after -DGAOL_PRESERVE_ROUNDING=ON" "${_first}" "${_second}")
check_consistent("GAOL_PRESERVE_ROUNDING=ON, the first configure" "${_first}" "${_first_out}")
message(STATUS "GAOL_PRESERVE_ROUNDING=ON, a second cmake: the same configuration, as expected")

configure_gaol(_again _again_out -DGAOL_PRESERVE_ROUNDING=ON)
check_same("the same command line again, -DGAOL_PRESERVE_ROUNDING=ON" "${_first}" "${_again}")
message(STATUS "GAOL_PRESERVE_ROUNDING=ON, the same command line again: the same configuration, as expected")

# The default follows GAOL_PRESERVE_ROUNDING: off, it is derived OFF (no
# message that it was left out, the macro undefined, the path not taken), and
# on again it is what it was
configure_gaol(_off _off_out -DGAOL_PRESERVE_ROUNDING=OFF)
check_consistent("GAOL_PRESERVE_ROUNDING=OFF after =ON" "${_off}" "${_off_out}")
contains("${_off_out}" "${_left_out}" _message)
if(_message OR NOT _off_out MATCHES "preferred: OFF, taken: OFF")
  message(FATAL_ERROR "GAOL_PRESERVE_ROUNDING=OFF after =ON: the default should be derived OFF, without the message that it was left out:\n${_off_out}")
endif()
configure_gaol(_on _on_out -DGAOL_PRESERVE_ROUNDING=ON)
check_same("GAOL_PRESERVE_ROUNDING=ON again, after =OFF" "${_first}" "${_on}")
message(STATUS "The default follows GAOL_PRESERVE_ROUNDING, off and on: as expected")

# ---- A request of OFF -------------------------------------------------------

# After a default that was ON (where there is a path): the entry then differs
# from the request, or holds the same on ARM, where the default was left out
configure_gaol(_refused _refused_out -DGAOL_PREFER_AVX512=OFF)
check_consistent("-DGAOL_PREFER_AVX512=OFF under GAOL_PRESERVE_ROUNDING" "${_refused}" "${_refused_out}")
contains("${_refused_out}" "${_left_out}" _message)
if(_message OR NOT _refused_out MATCHES "preferred: OFF, taken: OFF")
  message(FATAL_ERROR "-DGAOL_PREFER_AVX512=OFF under GAOL_PRESERVE_ROUNDING: the path should be refused, not derived and left out:\n${_refused_out}")
endif()
configure_gaol(_refused_again _refused_again_out)
check_same("the cmake after -DGAOL_PREFER_AVX512=OFF" "${_refused}" "${_refused_again}")
message(STATUS "-DGAOL_PREFER_AVX512=OFF under GAOL_PRESERVE_ROUNDING, and the next cmake: refused, as expected")

# -U: the default is derived again, as the first time
configure_gaol(_unset _unset_out -UGAOL_PREFER_AVX512)
check_same("-UGAOL_PREFER_AVX512 after the request of OFF" "${_first}" "${_unset}")
message(STATUS "-UGAOL_PREFER_AVX512: the default derived again, as expected")

# After a default that was OFF, which the request equals: the entry holds what
# the request says, and only the command line's mark tells them apart
configure_gaol(_off _off_out -DGAOL_PRESERVE_ROUNDING=OFF)
configure_gaol(_refused _refused_out -DGAOL_PRESERVE_ROUNDING=ON -DGAOL_PREFER_AVX512=OFF)
check_consistent("-DGAOL_PREFER_AVX512=OFF after a default OFF" "${_refused}" "${_refused_out}")
contains("${_refused_out}" "${_left_out}" _message)
if(_message OR NOT _refused_out MATCHES "preferred: OFF, taken: OFF")
  message(FATAL_ERROR "-DGAOL_PREFER_AVX512=OFF after a default OFF, under GAOL_PRESERVE_ROUNDING: the path should be refused, not derived (and left out where the compiler has none):\n${_refused_out}")
endif()
message(STATUS "-DGAOL_PREFER_AVX512=OFF equal to the default last derived, then GAOL_PRESERVE_ROUNDING=ON: refused, as expected")

# ---- A request of ON --------------------------------------------------------

# From a default derived under GAOL_PRESERVE_ROUNDING, the request of ON
# without it: taken where the compiler has the path, an error where it has
# none, and the same at the next cmake, which does not derive OFF and go on
configure_gaol(_unset _unset_out -UGAOL_PREFER_AVX512 -DGAOL_PRESERVE_ROUNDING=ON)
check_same("-UGAOL_PREFER_AVX512 again" "${_first}" "${_unset}")
configure_gaol(_asked _asked_out -DGAOL_PRESERVE_ROUNDING=OFF -DGAOL_PREFER_AVX512=ON)
configure_gaol(_asked_again _asked_again_out)
check_same("the cmake after -DGAOL_PREFER_AVX512=ON" "${_asked}" "${_asked_again}")
if(_asked MATCHES "^status 0\n")
  check_consistent("-DGAOL_PREFER_AVX512=ON" "${_asked}" "${_asked_out}")
  if(NOT _asked_out MATCHES "preferred: ON,")
    message(FATAL_ERROR "-DGAOL_PREFER_AVX512=ON, without GAOL_PRESERVE_ROUNDING: the path should be preferred:\n${_asked_out}")
  endif()
  if(_no_path)
    message(FATAL_ERROR "-DGAOL_PREFER_AVX512=ON configured, where the default left the path out for want of one:\n${_asked_out}")
  endif()
  message(STATUS "-DGAOL_PREFER_AVX512=ON without GAOL_PRESERVE_ROUNDING, and the next cmake: preferred, as expected")
else()
  # CMake wraps the text of an error at its blanks
  string(REGEX REPLACE "[ \t\r\n]+" " " _text "${_asked_out}")
  if(NOT _no_path OR NOT _text MATCHES "GAOL_PREFER_AVX512 takes the SSE2 intervals of x86-64")
    message(FATAL_ERROR "-DGAOL_PREFER_AVX512=ON did not configure, and not for want of a path:\n${_asked_out}")
  endif()
  message(STATUS "-DGAOL_PREFER_AVX512=ON, refused for want of an AVX-512 path, and the next cmake: the same, as expected")
endif()

file(REMOVE_RECURSE "${GAOL_WORK_DIR}")
