# ==================================================================
#  Refusal of Visual C++ without /fp:strict (fork of GAOL)
# ==================================================================
#
# Compiles a file including the headers of an installed GAOL with Visual C++:
# it has to compile with /fp:strict, and gaol/gaol_config.h has to refuse it
# with /fp:precise and without any /fp option, the default of Visual C++.
# Configuring is the check, which fails otherwise:
#
#   cmake -S tests/fp_strict -B build-fp-strict -DCMAKE_PREFIX_PATH=<prefix of GAOL>
#
# Copyright (c) 2026 ENSTA, France
#
# Created 2026-09-20 by Jordan NININ

cmake_minimum_required(VERSION 3.14)
project(gaol_fp_strict CXX)

if(NOT MSVC)
  message(STATUS "Not Visual C++: nothing to check")
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
foreach(model strict precise default)
  if(model STREQUAL "default")
    set(_fp "")
  else()
    set(_fp "/fp:${model}")
  endif()
  set(CMAKE_CXX_FLAGS "${_cxx_flags} /D__GAOL_PUBLIC__= ${_fp}")
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
