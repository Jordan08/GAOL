/*-*-C++-*------------------------------------------------------------------
 * GAOL -- not Just Another Interval Library
 *--------------------------------------------------------------------------
 * Tests of GAOL v5: mingw-w64 for ARM, which gaol/gaol_config.h accepts or
 * refuses by what the headers of the toolchain say.
 *
 * Only the header gaol/gaol_config.h, compiled on an x86 system that stands for
 * the target: the system headers come first, then the program says what the
 * headers of a mingw-w64 for ARM say, and the compilation has to fail with the
 * message of the refusal, or to succeed, as the tests of tests/CMakeLists.txt
 * and tests/refused_options.sh ask:
 *
 *  - GAOL_TEST_MINGW_ARM_BITS 32, with mingw-w64 11: refused, a 32-bit ARM
 *    having an fma() that is not fused (fmacd, which is vmla.f64);
 *  - 64, with mingw-w64 11: accepted, llvm-mingw 20230320, whose fma() is the
 *    instruction fmadd, which the job of windows.yml builds and checks;
 *  - 64, with mingw-w64 12 that does not say _UCRT, so linked with msvcrt.dll:
 *    refused, no toolchain being known that builds it (GAOL_TEST_MINGW_UCRT
 *    left out);
 *  - 64, with mingw-w64 12 saying _UCRT (GAOL_TEST_MINGW_UCRT 1): accepted, the
 *    fma() and the round() being those of ucrtbase.dll, as MSYS2 CLANGARM64.
 *
 * GAOL_TEST_MINGW_MAJOR is the version of mingw-w64 the headers say.
 *--------------------------------------------------------------------------
 * Copyright (c) 2026 ENSTA, France
 *
 * By: Jordan NININ (created 2026-11-18)
 *--------------------------------------------------------------------------
 * GAOL is a software distributed WITHOUT ANY WARRANTY. Read the associated
 * COPYING.LIB file for information.
 *--------------------------------------------------------------------------*/

#include <cmath>
#include <cstdint>
#include <cstdlib>

#if defined(GAOL_TEST_MINGW_ARM_BITS) && defined(GAOL_TEST_MINGW_MAJOR)
#  undef __x86_64__
#  undef __x86_64
#  undef __i386__
#  undef __i386
#  undef __MINGW32__
#  undef __MINGW64_VERSION_MAJOR
#  undef _UCRT
#  define __MINGW32__ 1
#  define __MINGW64_VERSION_MAJOR GAOL_TEST_MINGW_MAJOR
#  if defined(GAOL_TEST_MINGW_UCRT)
#    define _UCRT 1
#  endif
#  if GAOL_TEST_MINGW_ARM_BITS == 64
#    define __aarch64__ 1
#  else
#    define __arm__ 1
#  endif
#endif

#include <gaol/gaol_config.h>

int main()
{
  return 0;
}
