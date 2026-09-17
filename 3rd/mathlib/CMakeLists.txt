# ==================================================================
#  CMake build of mathlib, for the CMake build of GAOL
# ==================================================================
#
# mathlib is the IBM Accurate Portable Mathematical Library (libultim), with
# which GAOL computes its elementary functions, in the version Frederic Goualard
# distributes along with GAOL, https://frederic.goualard.net/software/mathlib-2.1.1.tar.gz
# (GNU GPL v2 or later), whose sources this directory holds. It comes with an
# autotools build and a batch file for Visual C++, but no CMake build: this file
# and mathlib_configuration.h.in were added for the CMake build of GAOL, which
# builds this directory with add_subdirectory(). The sources of mathlib
# themselves are only modified to fix bugs of it (see ../README.md).
#
# Origin
# ------
# This is the CMakeLists.txt with which IBEX builds the same mathlib
# (https://github.com/ibex-team/ibex-lib, GNU LGPL v3,
# interval_lib_wrapper/gaol/3rd/mathlib-2.1.1/CMakeLists.txt), written by
# Cyril Bouvier and updated by Gilles Chabert. It differs in two respects:
#
#  - IBEX chooses the implementation of Init_Lib() (src/DPChange.c) and the
#    byte order from CMAKE_SYSTEM_NAME and CMAKE_SYSTEM_PROCESSOR, and stops on
#    a system that mathlib's own build does not know. The choice is made by the
#    preprocessor here, in mathlib_configuration.h.in: it then follows the
#    target the compiler actually builds for, in a cross-compilation too, and
#    extends to Visual C++, MinGW and 32-bit ARM.
#
#  - IBEX moves src/endian.h to src/utils/, so that this header, once its
#    directory is on the include path, does not hide the <endian.h> of the
#    system. src/ is not put on the include path here at all: mathlib's sources
#    include their own headers with quotes, which finds them in their own
#    directory.

cmake_minimum_required(VERSION 3.14...3.25)

project(mathlib VERSION 2.1.1 LANGUAGES C)

include(GNUInstallDirs)

# mathlib is C89 code. C99 is asked for explicitly, since a compiler defaulting
# to C23, such as GCC 15, would read a declaration such as "int f();" as taking
# no argument.
set(CMAKE_C_STANDARD 99)

configure_file(mathlib_configuration.h.in "${CMAKE_CURRENT_BINARY_DIR}/mathlib_configuration.h" COPYONLY)

# The sources of libultim, as listed in src/Makefile.am
set(MATHLIB_SOURCES
  atnat2.c    halfulp.c  mplog.c     slowexp2.c  ucot.c   upow.c
  atnat.c     mpa.c      mpsqrt.c    slowexp.c   uexp2.c  urem.c
  branred.c   mpatan2.c  mptan.c     slowlog2.c  uexp.c   uroot.c
  doasin.c    mpatan.c   sincos32.c  slowpow.c   ulog2.c  usncs.c
  dosincos.c  mpexp.c    slowcot.c   uasncs.c    ulog.c   utan.c
  DPChange.c
)
list(TRANSFORM MATHLIB_SOURCES PREPEND "src/")

add_library(ultim STATIC ${MATHLIB_SOURCES})
# For mathlib_configuration.h
target_include_directories(ultim PRIVATE "${CMAKE_CURRENT_BINARY_DIR}")
# mathlib's warnings turned off, whatever flags the project building GAOL gives:
# with -Wall -Wextra -Wconversion, GCC 9 printed 25255 of them over mathlib's
# sources, 24612 of -Wsign-conversion. -w turns them all off, before or after
# the -W options, with GCC, Clang and Visual C++ alike.
include(CheckCCompilerFlag)
check_c_compiler_flag(-w MATHLIB_C_COMPILER_SUPPORTS_W)
if(MATHLIB_C_COMPILER_SUPPORTS_W)
  target_compile_options(ultim PRIVATE -w)
endif()
if(MSVC)
  # src/mathlib_config.h declares mathlib's functions __declspec(dllexport) for
  # Visual C++, as for a DLL. They would then be exported by every DLL linking
  # this static library.
  target_compile_definitions(ultim PRIVATE _MATHLIB_DLL_=extern)
endif()

# Exported along with GAOL when built by it (MATHLIB_EXPORT_NAME=gaolTargets)
if(NOT DEFINED MATHLIB_EXPORT_NAME)
  set(MATHLIB_EXPORT_NAME mathlibTargets)
endif()
install(TARGETS ultim EXPORT ${MATHLIB_EXPORT_NAME}
  ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
  LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
  RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)
install(FILES
  src/MathLib.h
  src/mathlib_config.h
  src/mathlib_config_msvc.h
  src/mathlib_config_mingw.h
  "${CMAKE_CURRENT_BINARY_DIR}/mathlib_configuration.h"
  DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
)
