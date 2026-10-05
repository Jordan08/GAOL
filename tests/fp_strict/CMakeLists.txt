# ==================================================================
#  Refusal of Visual C++ and clang-cl without /fp:strict (GAOL v5)
# ==================================================================
#
# Compiles a file including the headers of an installed GAOL with Visual C++,
# or with clang-cl from Clang 16, which defines the same macros of the
# floating-point model: it has to compile with /fp:strict, and
# gaol/gaol_config.h has to refuse it with /fp:precise and without any /fp
# option, the default of both.
# Configuring is the check, which fails otherwise:
#
#   cmake -S tests/fp_strict -B build-fp-strict -DCMAKE_PREFIX_PATH=<prefix of GAOL>
#
# Copyright (c) 2026 ENSTA, France
#
# Created 2026-09-20 by Jordan NININ

cmake_minimum_required(VERSION 3.14)
project(gaol_fp_strict CXX)

# MSVC is true for clang-cl too, whose model gaol/gaol_config.h checks from
# Clang 16, the first to define _M_FP_STRICT (GAOL v5)
set(_clang_cl FALSE)
if(MSVC AND CMAKE_CXX_COMPILER_ID STREQUAL "Clang")
  set(_clang_cl TRUE)
endif()
if(NOT (CMAKE_CXX_COMPILER_ID STREQUAL "MSVC"
        OR (_clang_cl AND CMAKE_CXX_COMPILER_VERSION VERSION_GREATER_EQUAL 16)))
  message(STATUS "Neither Visual C++ nor clang-cl from Clang 16: nothing to check")
  return()
endif()

find_package(gaol REQUIRED)
get_target_property(_gaol_include gaol::gaol INTERFACE_INCLUDE_DIRECTORIES)

set(_source "${CMAKE_CURRENT_BINARY_DIR}/uses_gaol.cpp")
file(WRITE "${_source}" "#include <gaol/gaol>\nint main() { gaol::interval x(0.1); return x.is_empty() ? 1 : 0; }\n")

# Compiled, not linked: gaol::gaol, which the library comes with, gives /fp:strict.
# try_compile() takes the flags from CMAKE_CXX_FLAGS of this project, not from
# a CMAKE_CXX_FLAGS given in CMAKE_FLAGS.
set(CMAKE_TRY_COMPILE_TARGET_TYPE STATIC_LIBRARY)
set(_cxx_flags "${CMAKE_CXX_FLAGS}")
# clang-cl for 32-bit x86 computes doubles in SSE2 with -msse2 only, which
# gaol::gaol gives it: without it, gaol/gaol_config.h refuses the x87 unit
set(_arch "")
if(_clang_cl AND CMAKE_SIZEOF_VOID_P EQUAL 4 AND CMAKE_CXX_COMPILER_ARCHITECTURE_ID MATCHES "^[Xx]86$")
  set(_arch "-msse2")
endif()
foreach(model strict precise default)
  if(model STREQUAL "default")
    set(_fp "")
  else()
    set(_fp "/fp:${model}")
  endif()
  set(CMAKE_CXX_FLAGS "${_cxx_flags} /D__GAOL_PUBLIC__= ${_arch} ${_fp}")
  try_compile(_compiled_${model} "${CMAKE_CURRENT_BINARY_DIR}/${model}" SOURCES "${_source}"
    CMAKE_FLAGS "-DINCLUDE_DIRECTORIES=${_gaol_include}"
    OUTPUT_VARIABLE _output_${model})
endforeach()
set(CMAKE_CXX_FLAGS "${_cxx_flags}")

if(NOT _compiled_strict)
  message(FATAL_ERROR "GAOL's headers do not compile with /fp:strict:\n${_output_strict}")
endif()
foreach(model precise default)
  if(_compiled_${model})
    message(FATAL_ERROR "GAOL's headers compile with the ${model} floating-point model: gaol/gaol_config.h did not refuse them")
  endif()
  if(NOT _output_${model} MATCHES "without /fp:strict")
    message(FATAL_ERROR "GAOL's headers do not compile with the ${model} floating-point model, but not because gaol/gaol_config.h refused them:\n${_output_${model}}")
  endif()
endforeach()
message(STATUS "GAOL's headers compile with /fp:strict, and gaol/gaol_config.h refuses them with /fp:precise and without /fp option")
