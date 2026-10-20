/*-*-C++-*------------------------------------------------------------------
 * gaol -- NOT Just Another Interval Library
 *--------------------------------------------------------------------------
 * Tests of GAOL v5: the SSE2 intervals on 32-bit Windows.
 *
 * Only the header gaol/gaol_config.h, which refuses to compile the SSE2
 * intervals on 32-bit Windows (_WIN32 without _WIN64) where the compiler is not
 * told to ask for aligned memory: the C runtime aligns the memory of new on 8
 * bytes, an SSE2 interval needs 16, and a std::vector of them stopped the
 * program on its first movaps (GCC takes the memory of new to be aligned on
 * 16). The tests refused_aligned_new, refused_aligned_new_cxx17 and
 * refused_aligned_new_positive of tests/CMakeLists.txt compile it with
 * -D_WIN32 -U_WIN64, which stand for the target on any system where the
 * intervals are the SSE2 ones: without -faligned-new=8 (in C++11, and in C++17,
 * where GCC and Clang ask for aligned memory for what is aligned on more than
 * 16 bytes only), the compilation has to fail with the message of
 * gaol/gaol_config.h; with it, it has to succeed. tests/refused_options.sh does
 * the same in the autotools and meson builds. The header alone, not
 * <gaol/gaol>: the system headers of a Linux or a macOS do not take _WIN32.
 *
 * Copyright (c) 2026 ENSTA, France
 *
 * Created 2026-10-20 by Jordan NININ
 *--------------------------------------------------------------------------
 * gaol is a software distributed WITHOUT ANY WARRANTY. Read the associated
 * COPYING file for information.
 *--------------------------------------------------------------------------*/

#include <gaol/gaol_config.h>

int main()
{
  return 0;
}
