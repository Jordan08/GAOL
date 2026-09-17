# ==================================================================
#  Prepares the sources of mathlib 2.1.1 for their CMake build
# ==================================================================
#
# Run by the CMake build of GAOL (CMakeLists.txt) as the patch step of the
# download of mathlib:
#
#   cmake -DSOURCE_DIR=<sources of mathlib> -P prepare.cmake
#
# It copies the CMake build of this directory to the root of the sources, and
# replaces src/mathlib_config_msvc.h and src/mathlib_config_mingw.h, the
# configuration files src/mathlib_config.h includes for Visual C++ and MinGW,
# with files including the configuration CMake generates for every compiler.
# The original configuration files only select the implementation of
# Init_Lib() for 32-bit x86, which mathlib_configuration.h.in does for every
# target.
#
# It also fixes bugs of mathlib (see below). mathlib's sources are not modified
# otherwise. Run again on the sources it prepared, it leaves them as they are.

cmake_minimum_required(VERSION 3.14)

if(NOT IS_DIRECTORY "${SOURCE_DIR}" OR NOT EXISTS "${SOURCE_DIR}/src/MathLib.h")
  message(FATAL_ERROR "Usage: cmake -DSOURCE_DIR=<sources of mathlib> -P ${CMAKE_CURRENT_LIST_FILE}")
endif()

configure_file("${CMAKE_CURRENT_LIST_DIR}/CMakeLists.txt" "${SOURCE_DIR}/CMakeLists.txt" COPYONLY)
configure_file("${CMAKE_CURRENT_LIST_DIR}/mathlib_configuration.h.in" "${SOURCE_DIR}/mathlib_configuration.h.in" COPYONLY)

foreach(file src/mathlib_config_msvc.h src/mathlib_config_mingw.h)
  file(WRITE "${SOURCE_DIR}/${file}"
"/* Written by the CMake build of GAOL (cmake/mathlib/prepare.cmake) in place of
   the configuration file for this compiler: CMake generates the configuration
   for every compiler, from mathlib_configuration.h.in. */

#include \"mathlib_configuration.h\"
")
endforeach()

# Replaces old, which has to be found once, with new in the file of mathlib's
# sources given, unless new is already there
function(fix_source file old new)
  set(path "${SOURCE_DIR}/${file}")
  file(READ "${path}" code)
  string(FIND "${code}" "${new}" done)
  if(NOT done EQUAL -1)
    return()
  endif()
  string(FIND "${code}" "${old}" first)
  string(FIND "${code}" "${old}" last REVERSE)
  if(first EQUAL -1 OR NOT first EQUAL last)
    message(FATAL_ERROR "${path} is not the one of mathlib 2.1.1: the code to fix is not found in it once:\n${old}")
  endif()
  string(REPLACE "${old}" "${new}" code "${code}")
  file(WRITE "${path}" "${code}")
endfunction()

# The cosine of mathlib. For the arguments hardest to round, ucos() falls back
# on mpcos(), which computes cos(x) with multiple-precision numbers, as
# sin(pi/2 - x) when x > 0.8. c32(x, y, z) sets y to cos(x) and z to sin(x),
# and mpcos() took the cosine of pi/2 - x, which is sin(x), rather than its
# sine: ucos() returned sin(x), and GAOL bounds not enclosing cos(x), at 54 of
# the hard-to-round arguments of cos of CORE-MATH, all between 0.80 and 0.853
# (https://github.com/dreal-deps/mathlib/issues/2). glibc, which took the same
# code from IBM, fixed the same line in 2003 (Debian bug 153548,
# https://sourceware.org/git/?p=glibc.git;a=commit;h=86583139a4d746743ccffcd72e25d96c5fb8d488).
fix_source(src/sincos32.c "c32(&b,&a,&c,p);" "c32(&b,&c,&a,p);")

# The square root of mathlib in multiple precision, which its arctangent uses at
# the arguments hardest to round (src/mpatan.c, src/mpatan2.c). fastiroot() in
# src/mpsqrt.c, which gives it a first approximation of 1/sqrt(x), read and
# wrote the halves of a double through an array of two longs: where long has 64
# bits (Linux and macOS on 64-bit processors), p.i[HIGH_HALF] is not the high
# half of the double, x was not scaled to [0.5, 2), and the approximation was
# wrong. atan() then returned values far from atan(x), or had not returned
# after 20 ms, at 12003 and 3967 of the 55190 hard-to-round arguments of atan
# of CORE-MATH (https://gitlab.inria.fr/core-math/core-math) on x86_64:
# atan(1.016527294692847) was 0.082 instead of 0.794. glibc, which took the
# same code from IBM, made them ints in 2003 (Michael Matz, "fastiroot: Fix
# 64-bit problem",
# https://sourceware.org/git/?p=glibc.git;a=commit;h=bb3f4825c411e676c51479fea59643af540810b5).
# atan() gave such values on Alpha as well (https://bugs.debian.org/210613,
# whose three arguments fail on x86_64 without this fix).
fix_source(src/mpsqrt.c
  "union {long i[2]; double d;} p,q;\n  double y,z, t;\n  long n;"
  "union {int i[2]; double d;} p,q;\n  double y,z, t;\n  int n;")

# The logarithm of mathlib at subnormal arguments. ulog() in src/ulog.c scales
# a subnormal x by 2^54, which its exponent n accounts for, but its last,
# multiple-precision stage computed the logarithm from the scaled x and from an
# approximation y of the logarithm of the unscaled one: ulog() returned about
# 2^54 at 26 of the 53 subnormal hard-to-round arguments of log of CORE-MATH
# (https://gitlab.inria.fr/core-math/core-math) on x86_64:
# log(0x0.8819864d7985dp-1022) was 1.8e16 instead of -709.03. That stage is
# given the unscaled x, kept in x0. glibc, which took the same code from IBM,
# kept it in __ieee754_log until it removed the multiple-precision stages in
# 2018 (Wilco Dijkstra, "Remove slow paths from log",
# https://sourceware.org/git/?p=glibc.git;a=commit;h=b7c83ca30ef8e85b6642151d95600a36535f8d97).
fix_source(src/ulog.c "double dbl_n,u,p0,q,r0,w," "double x0,dbl_n,u,p0,q,r0,w,")
fix_source(src/ulog.c "  n=0;\n  if (ux < 0x00100000) {" "  n=0;  x0 = x;\n  if (ux < 0x00100000) {")
fix_source(src/ulog.c "dbl_mp(x,&mpx,p);" "dbl_mp(x0,&mpx,p);")

# The tangent of mathlib. In 7 of the tests with which utan() in src/utan.c
# decides whether its result is rounded right, the error bound t4 was assigned
# in one operand of == and read in the other, with no sequence point between
# them: undefined behaviour, which Clang warns about (-Wunsequenced). GCC and
# Clang 18, from -O0 to -O3, gave the same results as with t4 assigned before
# the test, at the 1077348 hard-to-round arguments of tan of CORE-MATH
# (https://gitlab.inria.fr/core-math/core-math) and 10 million others. t4 is
# assigned before the test, as glibc did in 2009 (Ulrich Drepper, "Fix
# -Wsequence-point warnings",
# https://sourceware.org/git/?p=glibc.git;a=commit;h=82a1a4dae1b699a394e213866e789eacef1728fc)
# and Fabrice Le Bars in his fork of mathlib
# (https://github.com/lebarsfa/mathlib/commit/04a3dfe75cd3f0e49e22bca9ed688462d18c6c52).
foreach(case IN ITEMS "fi + 3" "gi - 10" "fi + 9" "gi - 18" "fi + 17" "gi - 26" "fi + 25")
  separate_arguments(case)
  list(GET case 0 f)
  list(GET case 1 op)
  list(GET case 2 i)
  set(bound "${f}*ua${i}.d+t3*ub${i}.d")
  fix_source(src/utan.c
    "if ((y=${f}${op}(t2-(t4=${bound})))==${f}${op}(t2+t4))"
    "t4=${bound};  if ((y=${f}${op}(t2-t4))==${f}${op}(t2+t4))")
endforeach()

# The include guard of src/sincos32.h tested SINCOS32_H but defined
# SINCCOS32_H, which Clang warns about (-Wheader-guard), as Fabrice Le Bars
# fixed in his fork of mathlib
# (https://github.com/lebarsfa/mathlib/commit/ad40312506d2297c75e3169816d1878b791d74a6).
fix_source(src/sincos32.h "#define SINCCOS32_H" "#define SINCOS32_H")

# The pragma of C99 (7.6.1) that tells the compiler the code may be executed
# with a rounding direction other than the default, and that it must not fold
# nor reorder its floating-point operations as if the rounding were to nearest,
# is written at the end of src/mathlib_config.h, which every source of mathlib
# includes. GAOL sets the rounding direction to nearest before calling mathlib
# and back upward afterwards, and mathlib is compiled with the flags of
# interval arithmetic: the pragma states for the compiler what those flags ask
# of it, as Fabrice Le Bars does in his fork of mathlib
# (https://github.com/lebarsfa/mathlib/commit/5ac52c2bd817e44d33d4f9af6c1045d4b8577449),
# with the uppercase ON that macOS warns about in the lowercase.
#
# What the compilers do with it, measured here: Clang 18 honours it, and the
# elementary functions of an interval took the same time with it as without
# (exp, log, sin and cos within 0.8%, below the dispersion of the measures);
# GCC 13 ignores it and gave a libultim.a identical byte for byte; Visual C++
# is given /fp:strict, which its documentation says makes it behave as if
# fenv_access(on) were set, and reads the pragma for Visual C++ rather than the
# one of C99, which it does not know.
fix_source(src/mathlib_config.h
  "# include \"mathlib_configuration.h\"\n#endif"
  "# include \"mathlib_configuration.h\"\n#endif

#if defined(_MSC_VER)
#   pragma fenv_access(on)
#else
#   pragma STDC FENV_ACCESS ON
#endif")
