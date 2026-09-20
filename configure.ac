#                                               -*- Autoconf -*-
# Process this file with autoconf to produce a configure script.
# ------------------------------------------------------------------------
# gaol -- NOT Just Another Interval Library
# ------------------------------------------------------------------------
# This file is part of the gaol distribution. Gaol was primarily
# developed at the Swiss Federal Institute of Technology, Lausanne,
# Switzerland, and is now developed at the Laboratoire d'Informatique
# de Nantes-Atlantique, France.
dnl
# Copyright (c) 2001 Swiss Federal Institute of Technology, Switzerland
# Copyright (c) 2002-2016 Laboratoire d'Informatique de Nantes-Atlantique,
#                         France
# Copyright (c) 2017-2020 Laboratoire des Sciences du Numérique de Nantes,
#                         France
# ------------------------------------------------------------------------
# gaol is a software distributed WITHOUT ANY WARRANTY. Read the associated
# COPYING file for information.
# ------------------------------------------------------------------------
# This file is an input file used by the GNU "autoconf" program to
# generate the file "configure", which is run during gaol installation
# to configure the system for the local environment.
# ------------------------------------------------------------------------

AC_PREREQ(2.61)

# gaol library version information
# !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
# Do NOT forget to modify gaol_version_msvc.h and gaol_version_mingw.h as well
# gaol library version information.
# (shamelessly stolen from GiNaC (http://www.ginac.de/)
# Making releases:
#   gaol_micro_version += 1;
#   gaol_interface_age += 1;
#   gaol_binary_age += 1;
# if any functions have been added, set gaol_interface_age to 0.
# if backwards compatibility has been broken,
# set gaol_binary_age and gaol_interface_age to 0.
# Version information for gaol
m4_define([gaol_major_version], [5])
m4_define([gaol_minor_version], [0])
m4_define([gaol_micro_version], [0])
m4_define([gaol_version], ["gaol_major_version.gaol_minor_version.gaol_micro_version"])
m4_define([gaol_release], [gaol_major_version.gaol_minor_version])
m4_define([gaol_interface_age], [0])
m4_define([gaol_binary_age], [0])

AC_INIT([gaol],[gaol_version],[goualard@users.sourceforge.net])
AM_INIT_AUTOMAKE([gnu 1.7 dist-bzip2 subdir-objects])

AM_MAINTAINER_MODE([disable])

AC_REVISION([$Revision: 291 $])
AC_CONFIG_SRCDIR([gaol/gaol_config.h])
AC_CONFIG_HEADERS([gaol/gaol_configuration.h])
AC_COPYRIGHT([Copyright (c) 2002-2020 Laboratoire d'Informatique de Nantes-Atlantique, France])

AC_CONFIG_MACRO_DIR([m4])

AC_DEFINE([GAOL_MAJOR_VERSION],gaol_major_version,[Major version for gaol])
AC_DEFINE([GAOL_MINOR_VERSION],gaol_minor_version,[Minor version for gaol])
AC_DEFINE([GAOL_MICRO_VERSION],gaol_micro_version,[Micro version for gaol])
AC_DEFINE([GAOL_VERSION],gaol_version,[Version number for gaol])
# Version for the manual
GAOL_EDITION="4.6"
GAOL_UPDATED="18 September 2026"
GAOL_VERSION=gaol_version
# !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
AC_SUBST(GAOL_EDITION)
AC_SUBST(GAOL_UPDATED)
AC_SUBST(GAOL_VERSION)



dnl libtool versioning
m4_define([lt_revision], [gaol_interface_age])
m4_define([lt_current], [m4_eval(gaol_micro_version - gaol_interface_age)])
m4_define([lt_age], [m4_eval(gaol_binary_age - gaol_interface_age)])
LT_VERSION_INFO="lt_current:lt_revision:lt_age"
LT_RELEASE="gaol_release"

AC_SUBST(LT_VERSION_INFO)
AC_SUBST(LT_RELEASE)

LT_PREREQ([2.2.6])
# CFLAGS and CXXFLAGS left empty when not given, rather than the "-g -O2" that
# AC_PROG_CC and AC_PROG_CXX give them otherwise: LT_INIT calls AC_PROG_CC, and
# the C sources (CORE-MATH, mathlib) were compiled with "-g -O2 ... -O3".
# --enable-optimize gives the C and C++ sources -O3 alike (see below).
: ${CFLAGS=""}
: ${CXXFLAGS=""}
LT_INIT([dlopen])

# RPM file versioning
GAOL_RPM_RELEASE=1
AC_SUBST(GAOL_RPM_RELEASE)


# Checks for programs.
AC_PROG_YACC

CFLAGS="${CFLAGS} -I.."
CXXFLAGS="${CXXFLAGS} -std=c++11 -I.."

AC_PROG_CC([gcc])
AC_PROG_CXX

# GAOL_CHECK_CXX_FLAG(flag, variable): whether the C++ compiler accepts flag,
# with -Werror, so that a flag it only warns about (Clang and
# -fexpensive-optimizations) counts as refused. GAOL_CHECK_C_FLAG likewise for
# the C compiler.
AC_DEFUN([GAOL_CHECK_CXX_FLAG],
[AC_LANG_PUSH([C++])
gaol_save_CXXFLAGS=$CXXFLAGS
CXXFLAGS="-Werror $1"
AC_MSG_CHECKING([whether the C++ compiler accepts $1])
AC_COMPILE_IFELSE([AC_LANG_PROGRAM([[]], [[]])],[$2=yes],[$2=no])
AC_MSG_RESULT([${$2}])
CXXFLAGS=$gaol_save_CXXFLAGS
AC_LANG_POP([C++])])
AC_DEFUN([GAOL_CHECK_C_FLAG],
[AC_LANG_PUSH([C])
gaol_save_CFLAGS=$CFLAGS
CFLAGS="-Werror $1"
AC_MSG_CHECKING([whether the C compiler accepts $1])
AC_COMPILE_IFELSE([AC_LANG_PROGRAM([[]], [[]])],[$2=yes],[$2=no])
AC_MSG_RESULT([${$2}])
CFLAGS=$gaol_save_CFLAGS
AC_LANG_POP([C])])
# GAOL_ADD_FLAG(flag): adds flag to CXXFLAGS and CFLAGS where each compiler
# accepts it. GAOL_ADD_INTERVAL_FLAG(flag) also adds it to GAOL_PC_CFLAGS, the
# flags of interval arithmetic gaol.pc gives the code using GAOL.
AC_DEFUN([GAOL_ADD_FLAG],
[GAOL_CHECK_CXX_FLAG([$1], [gaol_cxx_takes])
if test "${gaol_cxx_takes}" = yes; then
    CXXFLAGS="${CXXFLAGS} $1"
fi
GAOL_CHECK_C_FLAG([$1], [gaol_c_takes])
if test "${gaol_c_takes}" = yes; then
    CFLAGS="${CFLAGS} $1"
fi])
AC_DEFUN([GAOL_ADD_INTERVAL_FLAG],
[GAOL_ADD_FLAG([$1])
if test "${gaol_cxx_takes}" = yes; then
    GAOL_PC_CFLAGS="${GAOL_PC_CFLAGS} $1"
fi])
GAOL_PC_CFLAGS=""

# The compilers CMakeLists.txt refuses, for the same reasons (see there):
# - Clang for 32-bit ARM processors, which does not honour the rounding
#   direction: built by Clang 21, 4556 of 16000 random products, squares and
#   cubes did not enclose their exact values;
# - a compiler that takes -frounding-math and says it does not honour the
#   rounding direction on the target ("overriding currently unsupported
#   rounding mode on this target"), as Clang 14 for 64-bit ARM processors;
# - mingw-w64 before version 12, whose math library is not accurate enough for
#   the hyperbolic functions GAOL takes from it, and version 12, whose
#   fesetround() runs the instruction cpuid at each call.
AC_LANG_PUSH([C++])
AC_MSG_CHECKING([whether the C++ compiler is Clang for a 32-bit ARM processor])
AC_COMPILE_IFELSE([AC_LANG_PROGRAM([[#if defined(__arm__) && !defined(__aarch64__) && defined(__clang__)
#error Clang for a 32-bit ARM processor
#endif]], [[]])],[clang_arm32=no],[clang_arm32=yes])
AC_MSG_RESULT(${clang_arm32})
if test "${clang_arm32}" = yes; then
    AC_MSG_ERROR([GAOL cannot be built with $CXX, Clang for a 32-bit ARM processor.
Clang does not honour the rounding direction on 32-bit ARM processors: it does not support -frounding-math there (overriding currently unsupported rounding mode on this target) and optimizes floating-point operations as if they were always rounded to nearest. GAOL computes with the rounding direction set upward, so that the intervals it computes would not enclose the values they should (built by Clang 21, 4556 of 16000 random products, squares and cubes did not).
Build GAOL with GCC, which honours the rounding direction on 32-bit ARM processors: CXX=g++ CC=gcc ./configure, or CXX=arm-linux-gnueabihf-g++ to cross-compile.])
fi
gaol_save_CXXFLAGS=$CXXFLAGS
CXXFLAGS="-frounding-math"
AC_MSG_CHECKING([whether the C++ compiler accepts -frounding-math])
AC_COMPILE_IFELSE([AC_LANG_PROGRAM([[]], [[]])],[has_rounding_math_opt=yes],[has_rounding_math_opt=no])
AC_MSG_RESULT(${has_rounding_math_opt})
if test "${has_rounding_math_opt}" = yes; then
    CXXFLAGS="-Werror -frounding-math"
    AC_MSG_CHECKING([whether the C++ compiler honours the rounding direction on this target])
    AC_COMPILE_IFELSE([AC_LANG_PROGRAM([[]], [[]])],[honours_rounding=yes],[honours_rounding=no])
    AC_MSG_RESULT(${honours_rounding})
    if test "${honours_rounding}" = no; then
        AC_MSG_ERROR([GAOL cannot be built with $CXX for this target.
This compiler does not honour the rounding direction on this target, and says so of -frounding-math (overriding currently unsupported rounding mode on this target). GAOL computes with the rounding direction set upward, so that the intervals it computes would not enclose the values they should (built by Clang 14 for 64-bit ARM processors, bounds of pow() and nth_root() did not).
Build GAOL with a compiler that honours the rounding direction on this target, such as GCC or a later version of Clang (Clang 18 does on 64-bit ARM processors).])
    fi
fi
CXXFLAGS=$gaol_save_CXXFLAGS
# mingw-w64 is no longer refused for its version: GAOL takes no function from
# the math library of the system, and no longer changes the rounding direction
# for its elementary functions (see CMakeLists.txt)
AC_LANG_POP([C++])

# --enable-simd
AC_ARG_ENABLE(simd,
                [AS_HELP_STRING([--enable-simd],
                        [use SIMD instructions to implement arithmetic operations])],
                [simd=${enable_simd}],
                [simd="yes"])
AM_CONDITIONAL([SIMD_COND],[test "$simd" == "yes"])
# On x86 processors (the target of the compiler, rather than the processor of
# the machine building, as CMakeLists.txt): the intervals are computed with
# SSE2 instructions (gaol_interval_sse.cpp), SSE3 instructions serve
# gaol::interval2f (gaol_interval2f.cpp), and on 32-bit x86 the doubles are
# computed with SSE2 (-mfpmath=sse) rather than on the x87 unit, where GAOL's
# bounds and mathlib's results are wrong (see gaol/gaol_config.h). Not the SSE2
# intervals on 32-bit Windows, where a std::vector of them crashes (see
# CMakeLists.txt).
using_sse2=no
has_sse3=no
AC_LANG_PUSH([C++])
AC_MSG_CHECKING([whether the target is an x86 processor])
AC_COMPILE_IFELSE([AC_LANG_PROGRAM([[#if !(defined(__i386__) || defined(__x86_64__))
#error not an x86 processor
#endif]], [[]])],[target_x86=yes],[target_x86=no])
AC_MSG_RESULT(${target_x86})
AC_MSG_CHECKING([whether the target is a 32-bit x86 processor])
AC_COMPILE_IFELSE([AC_LANG_PROGRAM([[#if !defined(__i386__)
#error not a 32-bit x86 processor
#endif]], [[]])],[target_x86_32=yes],[target_x86_32=no])
AC_MSG_RESULT(${target_x86_32})
AC_MSG_CHECKING([whether the target is 32-bit Windows])
AC_COMPILE_IFELSE([AC_LANG_PROGRAM([[#if !(defined(_WIN32) && !defined(_WIN64))
#error not 32-bit Windows
#endif]], [[]])],[target_win32=yes],[target_win32=no])
AC_MSG_RESULT(${target_win32})
AC_LANG_POP([C++])
if test "${target_x86}" = yes; then
    if test "${target_x86_32}" = yes; then
        GAOL_ADD_INTERVAL_FLAG([-msse2 -mfpmath=sse])
    fi
    if test "$simd" = yes && test "${target_win32}" = no; then
        GAOL_CHECK_CXX_FLAG([-msse2], [has_sse2])
        if test "${has_sse2}" = yes; then
            using_sse2=yes
            CXXFLAGS="${CXXFLAGS} -msse2"
            GAOL_PC_CFLAGS="${GAOL_PC_CFLAGS} -msse2"
            AC_DEFINE([USING_SSE2_INSTRUCTIONS],1,
                [Define this to 1 if SSE2 instructions are to be used])
            GAOL_CHECK_CXX_FLAG([-msse3], [has_sse3])
            if test "${has_sse3}" = yes; then
                CXXFLAGS="${CXXFLAGS} -msse3"
                GAOL_PC_CFLAGS="${GAOL_PC_CFLAGS} -msse3"
                AC_DEFINE([USING_SSE3_INSTRUCTIONS],1,
                    [Define this to 1 if SSE3 instructions are to be used])
            fi
        fi
    fi
fi
# --enable-float-intervals
AC_ARG_ENABLE(float-intervals,
              [AS_HELP_STRING([--enable-float-intervals],
                              [compile gaol::intervalf, and gaol::interval2f with SSE3 @<:@default=no@:>@])],
              [float_intervals=${enable_float_intervals}],
              [float_intervals="no"])
if test "${float_intervals}" = yes; then
   AC_DEFINE(GAOL_FLOAT_INTERVALS,1,[Define this to 1 to compile gaol::intervalf and gaol::interval2f])
fi
AM_CONDITIONAL([FLOAT_INTERVALS_COND],[test "${float_intervals}" = yes])
AM_CONDITIONAL([COMPILE_INTERVAL2],[test ${has_sse3} = yes && test "${float_intervals}" = yes])

# --enable-fma: the fused multiply-add instructions of the processor, as
# CMakeLists.txt and meson.build take them (see CMakeLists.txt): -mfma on x86,
# where it defines __FMA__, but with GCC for Windows (MinGW), which does not
# align the stack for the AVX that -mfma turns on; -mfpu=neon-vfpv4
# -mfloat-abi=hard on 32-bit ARM, where FMA is not on already. Given to the C
# and C++ sources (GAOL, CORE-MATH, mathlib), and to the code using GAOL
# (gaol.pc), compiled for the same processor. -ffp-contract=off stays. Left out
# where the machine building does not run a program compiled with them, but
# when cross-compiling (see CMakeLists.txt: macOS 14 under Rosetta).
AC_ARG_ENABLE(fma,
              [AS_HELP_STRING([--enable-fma],
                              [compile GAOL, CORE-MATH and mathlib with the fused multiply-add instructions of the processor, where the compiler has a flag for them (no: a library for any processor of the architecture) @<:@default=yes@:>@])],
              [fma=${enable_fma}],
              [fma="yes"])
# GAOL_CHECK_FMA_FLAGS(flags, macro, variable): whether the C++ and the C
# compilers, given flags, define macro, and link a program so compiled
AC_DEFUN([GAOL_CHECK_FMA_FLAGS],
[$3=no
AC_MSG_CHECKING([whether $1 gives the fused multiply-add instructions])
AC_LANG_PUSH([C++])
gaol_save_CXXFLAGS=$CXXFLAGS
CXXFLAGS="${CXXFLAGS} $1"
AC_LINK_IFELSE([AC_LANG_PROGRAM([[#if !defined($2)
#error no fused multiply-add instructions
#endif]], [[]])],[$3=yes])
CXXFLAGS=$gaol_save_CXXFLAGS
AC_LANG_POP([C++])
if test "${$3}" = yes; then
    AC_LANG_PUSH([C])
    gaol_save_CFLAGS=$CFLAGS
    CFLAGS="${CFLAGS} $1"
    AC_LINK_IFELSE([AC_LANG_PROGRAM([[#if !defined($2)
#error no fused multiply-add instructions
#endif]], [[]])],[],[$3=no])
    CFLAGS=$gaol_save_CFLAGS
    AC_LANG_POP([C])
fi
AC_MSG_RESULT([${$3}])])
gaol_fma_flags=""
if test "$fma" = yes; then
    AC_LANG_PUSH([C++])
    AC_COMPILE_IFELSE([AC_LANG_PROGRAM([[#if !(defined(_WIN32) && defined(__GNUC__) && !defined(__clang__))
#error not GCC for Windows
#endif]], [[]])],[gaol_mingw_gcc=yes],[gaol_mingw_gcc=no])
    AC_COMPILE_IFELSE([AC_LANG_PROGRAM([[#if !defined(__arm__) || defined(__ARM_FEATURE_FMA)
#error not a 32-bit ARM target without FMA
#endif]], [[]])],[gaol_arm32_without_fma=yes],[gaol_arm32_without_fma=no])
    AC_LANG_POP([C++])
    if test "${gaol_mingw_gcc}" = yes; then
        AC_MSG_NOTICE([fused multiply-add instructions left out: GCC for Windows does not align the stack for AVX])
    else
        GAOL_CHECK_FMA_FLAGS([-mfma], [__FMA__], [gaol_has_fma])
        if test "${gaol_has_fma}" = yes; then
            gaol_fma_flags="-mfma"
        elif test "${gaol_arm32_without_fma}" = yes; then
            GAOL_CHECK_FMA_FLAGS([-mfpu=neon-vfpv4 -mfloat-abi=hard], [__ARM_FEATURE_FMA], [gaol_has_fma])
            if test "${gaol_has_fma}" = yes; then
                gaol_fma_flags="-mfpu=neon-vfpv4 -mfloat-abi=hard"
            fi
        fi
    fi
    if test -n "${gaol_fma_flags}"; then
        # fma(a, b, -1) is -2^-60 exactly, which a product rounded first gives as 0
        AC_LANG_PUSH([C++])
        gaol_save_CXXFLAGS=$CXXFLAGS
        CXXFLAGS="${CXXFLAGS} ${gaol_fma_flags}"
        AC_MSG_CHECKING([whether this machine runs a program compiled with ${gaol_fma_flags}])
        AC_RUN_IFELSE([AC_LANG_PROGRAM([[#include <cmath>]],
[[volatile double a = 1.0 + 1.0 / 1073741824.0, b = 1.0 - 1.0 / 1073741824.0, c = -1.0;
return std::fma(a, b, c) == -1.0 / 1152921504606846976.0 ? 0 : 1;]])],
            [gaol_runs_fma=yes], [gaol_runs_fma=no], [gaol_runs_fma="not checked, cross-compiling"])
        CXXFLAGS=$gaol_save_CXXFLAGS
        AC_LANG_POP([C++])
        AC_MSG_RESULT([${gaol_runs_fma}])
        if test "${gaol_runs_fma}" = no; then
            AC_MSG_NOTICE([fused multiply-add instructions left out: this machine does not run a program compiled with ${gaol_fma_flags}])
            gaol_fma_flags=""
        fi
    fi
    CXXFLAGS="${CXXFLAGS} ${gaol_fma_flags}"
    CFLAGS="${CFLAGS} ${gaol_fma_flags}"
    GAOL_PC_CFLAGS="${GAOL_PC_CFLAGS} ${gaol_fma_flags}"
fi




# The mathematical library
#
# GAOL bounds every one of its elementary functions with CORE-MATH
# (https://core-math.gitlabpages.inria.fr, MIT licence), whose sources are in
# 3rd/math-core and are compiled into libgaol itself (gaol/Makefile.am): there
# is no library to find, to build apart or to link along with GAOL.
#
# CORE-MATH is correctly rounded in the rounding direction in effect, so GAOL
# takes its values as bounds without moving them outward and without switching
# the rounding direction (gaol/gaol_double_op.h). mathlib (the IBM Accurate
# Portable Mathematical Library), CRlibm and the math library of the system,
# which GAOL could be built with, are no longer used: the first two are
# correctly rounded to nearest only, and the third is not accurate enough on
# every system for its values moved outward to be bounds.
#
# The options naming them are refused rather than ignored, so that a script
# still giving them is told what happened.
AC_ARG_WITH([mathlib],
    [AS_HELP_STRING([--with-mathlib],[removed: GAOL computes its elementary functions with CORE-MATH only])],
    [AC_MSG_ERROR([--with-mathlib is no longer available: GAOL bounds every elementary function with CORE-MATH, whose sources (3rd/math-core) are compiled into the library. mathlib, CRlibm and the math library of the system are no longer used.])])
AC_ARG_WITH([mathlib-include],
    [AS_HELP_STRING([--with-mathlib-include],[removed, see --with-mathlib])],
    [AC_MSG_ERROR([--with-mathlib-include is no longer available: CORE-MATH is compiled into GAOL, there is no mathematical library to find.])])
AC_ARG_WITH([mathlib-lib],
    [AS_HELP_STRING([--with-mathlib-lib],[removed, see --with-mathlib])],
    [AC_MSG_ERROR([--with-mathlib-lib is no longer available: CORE-MATH is compiled into GAOL, there is no mathematical library to link.])])
AC_MSG_NOTICE([elementary functions: CORE-MATH, from 3rd/math-core (correctly rounded)])
MATHLIB=""
AC_SUBST([MATHLIB])



# --enable-verbose-mode
AC_ARG_ENABLE(verbose-mode,
                  [AS_HELP_STRING([--enable-verbose-mode],
                         [write a line on the standard error when GAOL initializes and cleans up @<:@default=no@:>@])],
                 [verbose=${enable_verbose_mode}],
                 [verbose="no"])
if test "$verbose" = yes; then
  AC_DEFINE(GAOL_VERBOSE_MODE,1,
        [Define this to 1 to enable verbose mode])
fi

# --enable-optimize
AC_ARG_ENABLE(optimize,
	    [AS_HELP_STRING([--enable-optimize],
                            [compile gaol with full optimization @<:@default=yes@:>@])],
            [optimize=${enable_optimize}],
            [optimize="yes"])
# With any compiler, each flag where the compiler accepts it (they were added
# when the compiler was named g++ exactly, and clang++ or g++-13 compiled
# without any optimization), and NDEBUG, as the Release build of CMake and
# meson define it
if test $optimize = yes; then
    for gaol_flag in -O3 -funroll-loops -fomit-frame-pointer -fexpensive-optimizations; do
        GAOL_ADD_FLAG([${gaol_flag}])
    done
    CPPFLAGS="${CPPFLAGS} -DNDEBUG"
else
   CXXFLAGS="${CXXFLAGS} -O"
   CFLAGS="${CFLAGS} -O"
fi
GAOL_CHECK_CXX_FLAG([-fvisibility-inlines-hidden -fvisibility=hidden], [has_visibility_opt])
if test "${has_visibility_opt}" = yes; then
    CXXFLAGS="${CXXFLAGS} -fvisibility-inlines-hidden -fvisibility=hidden"
    AC_DEFINE(HAVE_VISIBILITY_OPTIONS,1,
                [Define this to 1 if the compiler supports the "-fvisibility" option])
fi

CFLAGS="${CFLAGS} -Wall -Wconversion"
CXXFLAGS="${CXXFLAGS} -Wall -Wconversion"


# -ffloat-store only where doubles are still computed on the x87 unit
# (FLT_EVAL_METHOD not 0), whose 80-bit registers keep more digits than a
# double. It was added wherever SSE2 was not used, 64-bit ARM included, where
# doubles gain nothing from it (as CMakeLists.txt)
AC_LANG_PUSH([C++])
OLD_CXXFLAGS=$CXXFLAGS
CXXFLAGS="${CXXFLAGS} -ffloat-store"
AC_MSG_CHECKING(whether doubles are computed in extended precision, with -ffloat-store accepted)
AC_COMPILE_IFELSE([AC_LANG_PROGRAM([[#include <cfloat>
#if defined(FLT_EVAL_METHOD) && FLT_EVAL_METHOD == 0
#error doubles are computed in double precision
#endif]], [[]])],[float_store_needed=yes],[float_store_needed=no])
AC_MSG_RESULT(${float_store_needed})
if test "${float_store_needed}" = no; then
    CXXFLAGS=${OLD_CXXFLAGS}
else
    GAOL_PC_CFLAGS="${GAOL_PC_CFLAGS} -ffloat-store"
fi
AC_LANG_POP([C++])

# --enable-debug
AC_ARG_ENABLE(debug,
            [AS_HELP_STRING([--enable-debug],
                            [compile gaol with debugging info @<:@default=no@:>@])],
            [debug=${enable_debug}],
            [debug="no"])
if test "$debug" = yes; then
   AC_DEFINE(GAOL_DEBUGGING,1,[define this if you want debugging facilities])
    case $CXX in
    icpc)
        CXXFLAGS="${CXXFLAGS} -g"
        ;;
    g++)
        CXXFLAGS="${CXXFLAGS} -g -ansi"
        CXXFLAGS="${CXXFLAGS} -Weffc++ -pedantic"
        ;;
    esac
fi

# --enable-preserve-rounding
AC_ARG_ENABLE(preserve-rounding,
            [AS_HELP_STRING([--enable-preserve-rounding],
                            [restore the rounding direction after each operation, rather than leaving it upward @<:@default=no@:>@])],
            [preserve_rounding=${enable_preserve_rounding}],
            [preserve_rounding="no"])
if test "${preserve_rounding}" = yes; then
   AC_DEFINE(GAOL_PRESERVE_ROUNDING,1,[define this to 1 if rounding direction should be preserved])
fi

# --enable-relations
AC_ARG_ENABLE(relations,
            [AS_HELP_STRING([--enable-relations=KIND],
                            [use KIND symbols of relations, with  KIND being "set", "certainly" or "possibly" @<:@default=certainly@:>@])],
            [relations=${enable_relations}],
            [relations="certainly"])
case "$relations" in
set)
    AC_DEFINE(GAOL_SET_RELATIONS,1,
    [Define this if relation symbols should stand for \"set relations\"])
    ;;
certainly)
    AC_DEFINE(GAOL_CERTAINLY_RELATIONS,1,
    [Define this if relation symbols should stand for \"certainly relations\"])
    ;;
possibly)
    AC_DEFINE(GAOL_POSSIBLY_RELATIONS,1,
    [Define this if relation symbols should stand for \"possibly relations\"])
    ;;
*)
    AC_MSG_ERROR(\"$relations\": unrecognized default relation (set/possibly/certainly))
   ;;
esac


# --enable-exceptions
AC_ARG_ENABLE(exceptions,
              [AS_HELP_STRING([--enable-exceptions],
                              [raise exceptions to signal errors @<:@default=yes@:>@])],
              [exceptions=${enable_exceptions}],
              [exceptions="yes"])
if test "$exceptions" = "yes"; then
   AC_DEFINE(GAOL_EXCEPTIONS_ENABLED,1,
    [Define this to 1 to allow raising exceptions instead of simply aborting])
fi

# --enable-asm
AC_ARG_ENABLE(asm,
              [AS_HELP_STRING([--enable-asm],
                              [use assembler code @<:@default=yes@:>@])],
              [using_asm=${enable_asm}],
              [using_asm="yes"])
if test "${using_asm}" = yes; then
   AC_DEFINE(GAOL_USING_ASM,1,[define this to 1 if you want to allow ASM support])
fi


# The processor and the system: gaol/gaol_config.h names them from the macros
# of the compiler, for the three builds alike (configure defined IX86_LINUX,
# AARCH64_LINUX, IX86_MACOSX and ARM_MACOSX from $host, and refused the other
# systems: macOS on Apple silicon was arm-apple-* and x86_64-apple-* neither)
AC_CANONICAL_HOST
case $host in
*-cygwin*)
	AM_CONDITIONAL([IS_CYGWIN_COND],[true])
	;;
*)
	AM_CONDITIONAL([IS_CYGWIN_COND],[false])
	;;
esac



# Checking for dl library
AC_CHECK_LIB([dl], [dlopen],have_dl=yes,have_dl=no)
if test "${have_dl}" = no; then
   dllib=""
else
   dllib="-ldl"
fi
# The check programs link it where it exists: Windows has none
AC_SUBST([DLLIB], [${dllib}])

# Checking for cppunit
AC_LANG_PUSH([C++])
AC_ARG_WITH([cppunit-include],
		  [AS_HELP_STRING([--with-cppunit-include],[Specify the include path for cppunit])],
		  cppunit_incpath="-I${with_cppunit_include}",cppunit_incpath="")
AC_ARG_WITH([cppunit-lib],
		  [AS_HELP_STRING([--with-cppunit-lib],[Specify the library path for cppunit])],
		  cppunit_libpath="-L${with_cppunit_lib}",cppunit_libpath="")
LDFLAGS="$LDFLAGS ${cppunit_libpath}"
CXXFLAGS="${CXXFLAGS} ${cppunit_incpath}"
AC_MSG_CHECKING([for cppunit/ui/text/TestRunner.h])
AC_CHECK_HEADERS("cppunit/ui/text/TestRunner.h",have_TestRunner=yes,have_TestRunner=no)
if test "${have_TestRunner}" = no; then
  AM_CONDITIONAL([CPPUNIT_COND],[test ${have_TestRunner} = yes])
    AC_MSG_WARN("*******************************************")
    AC_MSG_WARN("Library 'cppunit' not present.")
    AC_MSG_WARN("You will not be able to check the library.")
    AC_MSG_WARN("*******************************************")
else
  AC_MSG_CHECKING([for libcppunit])
  OLD_LIBS=$LIBS
  LIBS="-lcppunit $dllib"
  AC_COMPILE_IFELSE([AC_LANG_PROGRAM([[#include <cppunit/ui/text/TestRunner.h>]], [[CppUnit::TextUi::TestRunner runner;]])],[have_cppunit=yes],[have_cppunit=no])
  AC_MSG_RESULT(${have_cppunit})
  AM_CONDITIONAL([CPPUNIT_COND],[test ${have_cppunit} = yes])
  LIBS=$OLD_LIBS
fi
AC_LANG_POP([C++])

AC_CHECK_LIB([m],[nextafter],nextafter_found=yes,nextafter_found=no)
if test "${nextafter_found}" = yes; then
   AC_DEFINE(HAVE_NEXTAFTER,1,[ Define to 1 if you have the `nextafter' function.])
fi

# Checking for tools to re-create the pdf documentation
AC_CHECK_PROG(PDFLATEX,pdflatex,pdflatex,no)
if test "$PDFLATEX" = "no"; then
    AC_MSG_WARN("****************************************************")
    AC_MSG_WARN("No tool to compile a LaTeX file into a PDF file found")
    AC_MSG_WARN("You will not be able to re-create the PDF documentation")
    AC_MSG_WARN("****************************************************")
fi
AC_CHECK_PROG(BIBTEX,bibtex,bibtex,no)
AC_CHECK_PROG(MAKEINDEX,makeindex,makeindex,no)
if test "$MAKEINDEX" = "no"; then
  AC_CHECK_PROG(MAKEINDEX,makeidx,makeidx,no)
fi
AM_CONDITIONAL(PDF_TO_LATEX,
               [test "${PDFLATEX}" != "no" -a "$MAKEINDEX" != "no" -a "$BIBTEX" != "no"])



# Checking for tools to re-create the html documentation
AC_CHECK_PROG(DOXYGEN,doxygen,doxygen,no)
if test "${DOXYGEN}" = "no"; then
   AC_MSG_WARN("****************************************************")
   AC_MSG_WARN("'doxygen' not found:                                ")
   AC_MSG_WARN("You will not be able to create html documentation   ")
   AC_MSG_WARN("****************************************************")
fi
AM_CONDITIONAL(DOXYGEN, test "${DOXYGEN}" = "doxygen")
AC_CHECK_PROG(DOT,dot,dot,no)
if test "${DOT}" = "no"; then
   AC_MSG_WARN("******************************************************")
   AC_MSG_WARN("'dot' not found:                                      ")
   AC_MSG_WARN("You will not be able to create full html documentation")
   AC_MSG_WARN("You should consider installing the GraphViz package   ")
   AC_MSG_WARN("                           http://www.graphviz.org/   ")
   AC_MSG_WARN("******************************************************")
fi


# Checks for C header files
AC_HEADER_STDC
AC_CHECK_HEADERS([fenv.h float.h stddef.h stdlib.h string.h sys/time.h unistd.h])

# Checking for C++ header files
AC_LANG([C++])
AC_CHECK_HEADERS([limits cassert])


# Checks for typedefs, structures, and compiler characteristics.
AC_HEADER_STDBOOL
AC_C_CONST
AC_C_INLINE
AC_TYPE_SIZE_T

AC_C_BIGENDIAN
AC_CHECK_SIZEOF(int, 4)
AC_CHECK_SIZEOF(long, 4)
AC_CHECK_SIZEOF(long long int,8)



# The flags of interval arithmetic, for the C++ and the C compiler, each where
# the compiler accepts it, as CMakeLists.txt gives them: -frounding-math (GAOL
# then negates a double with the minus sign, HAVE_ROUNDING_MATH_OPTION),
# -ffp-contract=off (without it, compilers contract a multiplication and an
# addition into a fused one as if rounding to nearest: on 64-bit ARM,
# -(.5*l - .5*r) and -.5*l + .5*r both became fnma(.5, l, .5*r), and mid() a
# single double), -fno-fast-math (the fast-math optimizations would round to
# nearest and drop the checks of NaN and infinities)
AC_LANG([C])
if test "${has_rounding_math_opt}" = yes; then
    AC_DEFINE(HAVE_ROUNDING_MATH_OPTION,1,
        [Define this to 1 if the compiler supports the "-frounding-math" option])
fi
for gaol_flag in -frounding-math -ffp-contract=off -fno-fast-math; do
    GAOL_ADD_INTERVAL_FLAG([${gaol_flag}])
done
AC_SUBST([GAOL_PC_CFLAGS])
# The version for gaol.pc, without the quotes VERSION carries
GAOL_PC_VERSION=`echo "$VERSION" | tr -d '"'`
AC_SUBST([GAOL_PC_VERSION])
# No mathematical library is named in gaol.pc any more: the functions of
# CORE-MATH are compiled into libgaol itself
GAOL_PC_MATHLIB=""
AC_SUBST([GAOL_PC_MATHLIB])


# Checks for library functions.
AC_LANG([C++])
AC_FUNC_MALLOC
AC_FUNC_REALLOC
AC_CHECK_FUNCS([fesetround floor localeconv memset pow sqrt])
AC_CHECK_FUNCS(getrusage clock __signbit round rint finite)

# Checking for timing functions
AC_CHECK_HEADERS(sys/resource.h,sys_resource_found=yes,sys_resource_found=no)
if test "${sys_resource_found}" = yes; then
   AC_EGREP_HEADER("getrusage",sys/resource.h,
	    AC_DEFINE(GETRUSAGE_IN_HEADER,1,
               [Define this to 1 if function getrusage() is declared in sys/resource.h]),
	    AC_DEFINE(GETRUSAGE_IN_HEADER,0,
               [Define this to 1 if function getrusage() is declared in sys/resource.h]))
else
   AC_CHECK_HEADERS(time.h,time_found=yes,time_found=no)
   if test "${time_found}" = yes; then
      AC_EGREP_HEADER("clock",time.h,
	AC_DEFINE(CLOCK_IN_HEADER,1,
	  [Define this to 1 if function clock() is declared in time.h]),
	AC_DEFINE(CLOCK_IN_HEADER,0,
	  [Define this to 1 if function clock() is declared in time.h]))
   fi
fi



AC_SUBST(CPPFLAGS)
AC_SUBST(CXXFLAGS)
AC_SUBST(CFLAGS)
AC_SUBST(CXX)
AC_SUBST(CC)
AC_SUBST(LDFLAGS)

AC_CONFIG_FILES([gaol.pc
                 Makefile
        3rd/Makefile
        check/Makefile
		  manual/gaol_doxygen.cfg
        manual/Makefile
        manual/gaol_version.tex
		  gaol.spec
		  gaol/Makefile
		  examples/Makefile])

AC_OUTPUT

printf "\nConfigured gaol v. ${GAOL_VERSION} on $host\n"
printf "CC\t\t\t= ${CC}\n"
printf "CFLAGS\t\t\t= ${CFLAGS}\n"
printf "CXX\t\t\t= ${CXX}\n"
printf "CXXFLAGS\t\t= ${CXXFLAGS}\n"
   printf "elementary functions\t= CORE-MATH (3rd/math-core, correctly rounded)\n"
printf "optimize\t\t= $optimize\n"
printf "debug\t\t\t= $debug\n"
printf "preserve rounding\t= ${preserve_rounding}\n"
printf "relations\t\t= $relations\n"
printf "exceptions\t\t= $exceptions\n"
printf "asm\t\t\t= ${using_asm}\n"
printf "SSE2\t\t\t= ${using_sse2}\n"
printf "SSE3\t\t\t= ${has_sse3}\n"
printf "float intervals\t\t= ${float_intervals}\n"
printf "doxygen\t\t\t= ${DOXYGEN}\n"
printf "dot\t\t\t= ${DOT}\n"
printf "pdflatex\t\t= $PDFLATEX\n"
printf "makeindex\t\t= $MAKEINDEX\n"
printf "bibtex\t\t\t= $BIBTEX\n"

