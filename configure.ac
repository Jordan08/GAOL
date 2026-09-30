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

# autoconf 2.64 for m4_esyscmd_s, which reads VERSION.txt (GAOL v5)
AC_PREREQ(2.64)

# The version of GAOL is the line of the file VERSION.txt, which CMakeLists.txt
# and meson.build read too (not named VERSION: see CMakeLists.txt). configure
# reads it when it runs (below), whence the macros GAOL_*VERSION, the shared
# library, gaol.pc and the manuals: a new version needs VERSION.txt alone.
# autoconf reads it as well, for AC_INIT, whose version configure --version,
# configure --help, config.status --version and config.log give: they keep the
# one of the last generation of configure, which is to be generated again
# before VERSION.txt is committed (see doc/building.md); configure warns
# meanwhile. The m4 macro is not named gaol_version, which m4 replaced in
# manual/v5/gaol_version.tex, giving config.status "manual/v5/5.0.0.tex"
# (GAOL v5). autoconf reads it with the commands of configure below: a UTF-8
# byte order mark at the start of VERSION.txt, which some editors of Windows
# write, is removed, and so are the blanks around the version, not those
# within it. The lines left are joined by a blank, which refuses a second
# line: a regular expression of m4 matches at the end of any line. m4 reads
# the text again, where a comma or a parenthesis would end an argument, a
# bracket open a quotation, and a word such as dnl be a macro: the characters
# other than digits, dots and blanks are written ?, which refuses them as the
# other builds do (autoconf took "5.0.0)" for 5.0.0), the bytes in
# hexadecimal of the message showing them. The bytes of the mark, EF BB BF,
# are made by printf: sed has no escape for them that every sed knows, and
# the C locale makes it read bytes. gaol_m4_version_utf16 is empty, or says
# why a file of UTF-16 characters is refused (without a comma, for the same
# reason).
m4_define([gaol_m4_version], m4_esyscmd_s([LC_ALL=C tr -d '\000' < VERSION.txt | LC_ALL=C sed -e "1s/^`printf '\357\273\277'`//" -e 's/^[[:space:]]*//' -e 's/[[:space:]]*$//' -e '/^$/d' | tr '\n' ' ' | sed -e 's/ $//' | LC_ALL=C tr -c '0-9. \n' '?']))
m4_define([gaol_m4_version_utf16], m4_esyscmd_s([case `od -An -v -tx1 -N 2 < VERSION.txt | tr -d ' \n'` in fffe|feff) echo '; the file is UTF-16: save it as UTF-8 or ASCII' ;; *) if od -An -v -tx1 < VERSION.txt | grep ' 00' > /dev/null; then echo '; the file holds a NUL byte as UTF-16 does: save it as UTF-8 or ASCII'; fi ;; esac]))
m4_bmatch(gaol_m4_version[]gaol_m4_version_utf16, [^\(0\|[1-9][0-9]*\)\.\(0\|[1-9][0-9]*\)\.\(0\|[1-9][0-9]*\)$], [],
          [m4_fatal([VERSION.txt holds "]gaol_m4_version[" (bytes in hexadecimal: ]m4_esyscmd_s([od -An -v -tx1 -N 32 < VERSION.txt | tr -s ' \n' ' ' | sed -e 's/^ *//' -e 's/ *$//'])[), where it should hold the version of GAOL, three numbers without leading zeros such as 5.0.0]gaol_m4_version_utf16)])
dnl autom4te (its cache) and automake follow the files that m4_include reads,
dnl not those of m4_esyscmd_s: VERSION.txt is included too, into the diversion
dnl KILL, which m4 discards, so that a change of VERSION.txt generates configure
dnl again, and makes make do it in maintainer mode (GAOL v5)
m4_divert_push([KILL])m4_include([VERSION.txt])m4_divert_pop([KILL])

# configure --help ends with the address for the bug reports and the page of
# GAOL v5 (GAOL v5)
AC_INIT([gaol],[gaol_m4_version],[jordan.ninin@ensta.fr],[gaol],[https://github.com/Jordan08/GAOL])
# no-dist: no make dist nor make distcheck, the archives of the sources and
# the packages being made by CMake (cmake --build <build> --target
# package_source, package; see doc/building.md) (GAOL v5). no-define: no
# PACKAGE nor VERSION in gaol/gaol_configuration.h, which the code using GAOL
# includes.
AM_INIT_AUTOMAKE([gnu 1.7 no-dist no-define subdir-objects])

AM_MAINTAINER_MODE([disable])
# In maintainer mode, the rule of automake that regenerates
# gaol/gaol_configuration.h.in runs no autoheader, which would overwrite that
# template, written by hand (GAOL v5)
AUTOHEADER=true

AC_CONFIG_SRCDIR([gaol/gaol_config.h])

# The version of GAOL, read from VERSION.txt when configure runs, without the
# UTF-8 byte order mark that some editors of Windows write at the start of a
# file (EF BB BF, made by printf as above), nor the blanks around it (the CR
# of the line ends of Windows among them) and the empty lines: GAOL_VERSION,
# and its three numbers gaol_major, gaol_minor and gaol_micro, refused unless
# they are three numbers without leading zeros, as with CMake and meson. The
# blanks are the six of ASCII, the class space of the C locale, which CMake
# and meson strip too; the lines left are joined by a blank, as autoconf
# joins them above. A file of UTF-16 characters starts with its byte order
# mark (FF FE, FE FF) or holds NUL bytes, which the shell drops from the text
# of a command (they are taken out of it before): od finds them, and the file
# is refused. The error gives the first bytes of the file in hexadecimal, for
# the characters it does not show, and says to save a file of UTF-16
# characters as UTF-8 or ASCII (GAOL v5)
gaol_bom=`printf '\357\273\277'`
GAOL_VERSION=`LC_ALL=C tr -d '\000' < "$srcdir/VERSION.txt" | LC_ALL=C sed -e "1s/^$gaol_bom//" -e 's/^[[[:space:]]]*//' -e 's/[[[:space:]]]*$//' -e '/^$/d' | tr '\n' ' ' | sed -e 's/ $//'`
gaol_version_utf16=
case `od -An -v -tx1 -N 2 < "$srcdir/VERSION.txt" | tr -d ' \n'` in
  fffe|feff) gaol_version_utf16="; the file is UTF-16: save it as UTF-8 or ASCII" ;;
  *) if od -An -v -tx1 < "$srcdir/VERSION.txt" | grep ' 00' > /dev/null; then
       gaol_version_utf16="; the file holds a NUL byte as UTF-16 does: save it as UTF-8 or ASCII"
     fi ;;
esac
gaol_version_ok=no
if test -z "$gaol_version_utf16" &&
   expr "x$GAOL_VERSION" : 'x[[0-9]][[0-9]]*\.[[0-9]][[0-9]]*\.[[0-9]][[0-9]]*$' > /dev/null; then
  case .$GAOL_VERSION in
    *.0[[0-9]]*) ;;
    *) gaol_version_ok=yes ;;
  esac
fi
if test "${gaol_version_ok}" = no; then
  gaol_version_bytes=`od -An -v -tx1 -N 32 < "$srcdir/VERSION.txt" | tr -s ' \n' ' ' | sed -e 's/^ *//' -e 's/ *$//'`
  AC_MSG_ERROR([VERSION.txt holds "$GAOL_VERSION" (bytes in hexadecimal: $gaol_version_bytes), where it should hold the version of GAOL, three numbers without leading zeros such as 5.0.0$gaol_version_utf16])
fi
gaol_major=`expr "x$GAOL_VERSION" : 'x\([[0-9]]*\)'`
gaol_minor=`expr "x$GAOL_VERSION" : 'x[[0-9]]*\.\([[0-9]]*\)'`
gaol_micro=`expr "x$GAOL_VERSION" : 'x[[0-9]]*\.[[0-9]]*\.\([[0-9]]*\)'`
AC_MSG_NOTICE([the version of GAOL, from VERSION.txt: $GAOL_VERSION])
gaol_version_warning=""
if test "x$GAOL_VERSION" != "x[]gaol_m4_version"; then
  gaol_version_warning="GAOL is configured as $GAOL_VERSION, which VERSION.txt holds, but configure was generated for GAOL gaol_m4_version, which configure --version and configure --help still give: generate configure again, as doc/building.md says"
  AC_MSG_WARN([$gaol_version_warning])
fi
# gaol/gaol_configuration.h.in is written by hand, not by autoheader: it holds
# the macros GAOL's sources read, the same as the headers the CMake and the
# meson builds write (cmake/gaol_configuration.h.in,
# gaol/gaol_configuration_meson.h.in; see doc/building.md), and nothing else
# (GAOL v5)
AC_CONFIG_HEADERS([gaol/gaol_configuration.h])
AC_COPYRIGHT([Copyright (c) 2001-2020 Frederic Goualard
Copyright (c) 2026 ENSTA, France])

AC_CONFIG_MACRO_DIR([m4])

AC_DEFINE_UNQUOTED([GAOL_MAJOR_VERSION],[$gaol_major],[Major version for gaol])
AC_DEFINE_UNQUOTED([GAOL_MINOR_VERSION],[$gaol_minor],[Minor version for gaol])
AC_DEFINE_UNQUOTED([GAOL_MICRO_VERSION],[$gaol_micro],[Micro version for gaol])
AC_DEFINE_UNQUOTED([GAOL_VERSION],["$GAOL_VERSION"],[Version number for gaol])
# Version for the manuals: GAOL_EDITION and GAOL_UPDATED are those of the
# manual of GAOL 4 (manual/v4), GAOL_V5_EDITION and GAOL_V5_UPDATED those of
# the manual of GAOL v5 (manual/v5)
GAOL_EDITION="4.6"
GAOL_UPDATED="18 September 2026"
GAOL_V5_EDITION="5.0"
GAOL_V5_UPDATED="22 September 2026"
AC_SUBST(GAOL_EDITION)
AC_SUBST(GAOL_UPDATED)
AC_SUBST(GAOL_V5_EDITION)
AC_SUBST(GAOL_V5_UPDATED)
AC_SUBST(GAOL_VERSION)


# The version of the shared library, libgaol.so.5.0.0 for GAOL 5.0.0, its
# soname libgaol.so.5, as the CMake and the meson builds name it (VERSION and
# SOVERSION, version and soversion): libtool makes libgaol.so.<c-a>.<a>.<r> of
# -version-info c:r:a, hence c = major + minor, r = micro, a = minor. It
# made libgaol-5.0.so.0 of -release 5.0 (GAOL v5).
LT_VERSION_INFO="`expr $gaol_major + $gaol_minor`:$gaol_micro:$gaol_minor"
AC_SUBST(LT_VERSION_INFO)

LT_PREREQ([2.2.6])
# CFLAGS and CXXFLAGS left empty when not given, rather than the "-g -O2" that
# AC_PROG_CC and AC_PROG_CXX give them otherwise: LT_INIT calls AC_PROG_CC, and
# the C sources (CORE-MATH) were compiled with "-g -O2 ... -O3".
# --enable-optimize gives the C and C++ sources -O3 alike, --enable-debug -g
# (see below).
: ${CFLAGS=""}
: ${CXXFLAGS=""}
AC_PROG_CC
AC_PROG_CXX
LT_INIT

# GAOL is compiled in C++11 (gaol/Makefile.am), as with CMake and meson, and
# the tests and the examples in C++17 (tests/Makefile.am,
# examples/Makefile.am): the standard is not in CXXFLAGS, which comes after
# the flags of each directory and would override theirs
GAOL_CXX_STD="-std=c++11"
AC_SUBST([GAOL_CXX_STD])

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
#   rounding mode on this target"), as Clang 14 for 64-bit ARM processors.
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
# mingw-w64 is refused by gaol/gaol_config.h only, when compiling: on x86-64
# before version 12 or linked with msvcrt.dll, whose fma() and round(), those
# of mingw-w64's own math library, are wrong; on 32-bit x86 before 11, as the
# mingw-w64 9 of WinLibs, whose fma() is wrong; on ARM before 11, not tested
# (see CMakeLists.txt)
AC_LANG_POP([C++])

# --enable-simd
AC_ARG_ENABLE(simd,
                [AS_HELP_STRING([--enable-simd],
                        [use SIMD instructions to implement arithmetic operations])],
                [simd=${enable_simd}],
                [simd="yes"])
# On x86 processors (the target of the compiler, rather than the processor of
# the machine building, as CMakeLists.txt): the intervals are computed with
# SSE2 instructions (gaol_interval_sse.cpp), -msse3 is given where the
# compiler takes it, and on 32-bit x86 the doubles are
# computed with SSE2 (-mfpmath=sse) rather than on the x87 unit, where CORE-MATH
# rounds some results twice and some bounds are wrong (see CMakeLists.txt). Not
# the SSE2 intervals on 32-bit Windows, where a std::vector of them crashes (see
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
# No option for the intervals of floats, gaol::intervalf and gaol::interval2f,
# unfinished, which only a developer of GAOL compiles (see gaol/gaol_config.h)

# --enable-fma: the fused multiply-add instructions of the processor, as
# CMakeLists.txt and meson.build take them (see CMakeLists.txt): -mfma on x86,
# where it defines __FMA__, but with GCC for Windows (MinGW), which does not
# align the stack for the AVX that -mfma turns on; -mfpu=neon-vfpv4
# -mfloat-abi=hard on 32-bit ARM, where FMA is not on already. Given to the C
# and C++ sources (GAOL, CORE-MATH), and to the code using GAOL
# (gaol.pc), compiled for the same processor. -ffp-contract=off stays. Left out
# where the machine building does not run a program compiled with them, but
# when cross-compiling (see CMakeLists.txt: macOS 14 under Rosetta).
AC_ARG_ENABLE(fma,
              [AS_HELP_STRING([--enable-fma],
                              [compile GAOL and CORE-MATH with the fused multiply-add instructions of the processor, where the compiler has a flag for them (no: a library for any processor of the architecture) @<:@default=yes@:>@])],
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
# the rounding direction (gaol/gaol_double_op.h).
# -fno-lto for the sources of CORE-MATH, where the user compiles GAOL with
# -flto: the interprocedural optimization is worth having on GAOL's own
# operations and costs on the elementary functions (see doc/three-builds.md)
GAOL_CHECK_C_FLAG([-fno-lto], [core_math_takes_fno_lto])
CORE_MATH_NO_LTO=""
if test "${core_math_takes_fno_lto}" = yes; then
   CORE_MATH_NO_LTO="-fno-lto"
fi
AC_SUBST([CORE_MATH_NO_LTO])

# CORE-MATH asks for its computations to be rounded to double precision
# (FLT_EVAL_METHOD = 0, see 3rd/math-core/README.md), which GAOL gets on x86 by
# computing with SSE2 and which this asks the compiler for as well
GAOL_CHECK_C_FLAG([-fexcess-precision=standard], [core_math_takes_excess_precision])
CORE_MATH_EXCESS_PRECISION=""
if test "${core_math_takes_excess_precision}" = yes; then
   CORE_MATH_EXCESS_PRECISION="-fexcess-precision=standard"
fi
AC_SUBST([CORE_MATH_EXCESS_PRECISION])

AC_MSG_NOTICE([elementary functions: CORE-MATH, from 3rd/math-core (correctly rounded)])



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

# --enable-debug: GAOL compiled for debugging, as by the Debug build of CMake
# (CMAKE_BUILD_TYPE=Debug) and meson (--buildtype=debug): -g, no optimization,
# no NDEBUG, and GAOL_DEBUGGING, with which GAOL checks its assertions
# (gaol/gaol_assert.h) (GAOL v5). It gave g++ alone -g -ansi -Weffc++
# -pedantic, on top of the optimizations of --enable-optimize.
AC_ARG_ENABLE(debug,
            [AS_HELP_STRING([--enable-debug],
                            [compile GAOL for debugging, as the Debug build of CMake: -g, no optimization, and GAOL checks its assertions (GAOL_DEBUGGING); --enable-optimize is then ignored @<:@default=no@:>@])],
            [debug=${enable_debug}],
            [debug="no"])

# --enable-optimize
AC_ARG_ENABLE(optimize,
	    [AS_HELP_STRING([--enable-optimize],
                            [compile gaol with full optimization, as the Release build of CMake @<:@default=yes@:>@])],
            [optimize=${enable_optimize}],
            [optimize="yes"])
if test "$debug" = yes; then
    optimize="no (--enable-debug)"
    AC_DEFINE(GAOL_DEBUGGING,1,[Define this to 1 to have GAOL check its assertions (Debug build)])
    CXXFLAGS="${CXXFLAGS} -g"
    CFLAGS="${CFLAGS} -g"
elif test "$optimize" = yes; then
    # With any compiler, each flag where the compiler accepts it (they were
    # added when the compiler was named g++ exactly, and clang++ or g++-13
    # compiled without any optimization), and NDEBUG, as the Release build of
    # CMake and meson define it
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

# --enable-preserve-rounding
AC_ARG_ENABLE(preserve-rounding,
            [AS_HELP_STRING([--enable-preserve-rounding],
                            [restore the rounding direction after each operation, rather than leaving it upward @<:@default=no@:>@])],
            [preserve_rounding=${enable_preserve_rounding}],
            [preserve_rounding="no"])
if test "${preserve_rounding}" = yes; then
   AC_DEFINE(GAOL_PRESERVE_ROUNDING,1,[define this to 1 if rounding direction should be preserved])
fi

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


# --with-tests: the unit tests of tests/, built by make and run by make test
# and make check, as WITH_TESTS of CMake and with-tests of meson (GAOL v5)
AC_ARG_WITH([tests],
            [AS_HELP_STRING([--with-tests],
                            [build the unit tests of tests/, which make test and make check run @<:@default=no@:>@])],
            [with_tests=${withval}],
            [with_tests="no"])
AM_CONDITIONAL([WITH_TESTS], [test "${with_tests}" = yes])

# --with-examples: the examples of examples/, built by make and run by make
# check, as WITH_EXAMPLES of CMake and with-examples of meson (GAOL v5)
AC_ARG_WITH([examples],
            [AS_HELP_STRING([--with-examples],
                            [build the examples of examples/, which make check runs @<:@default=no@:>@])],
            [with_examples=${withval}],
            [with_examples="no"])
AM_CONDITIONAL([WITH_EXAMPLES], [test "${with_examples}" = yes])
# The threads of the example 13_rounding_environment (std::thread), which the
# glibc before 2.34 needs -pthread for
GAOL_CHECK_CXX_FLAG([-pthread], [gaol_cxx_pthread])
PTHREAD_FLAGS=""
if test "${gaol_cxx_pthread}" = yes; then
    PTHREAD_FLAGS="-pthread"
fi
AC_SUBST([PTHREAD_FLAGS])
# tests/automatic_cleanup.cpp, on Linux only, as with CMake and meson
case $host_os in
  linux*) gaol_host_linux=yes ;;
  *) gaol_host_linux=no ;;
esac
AM_CONDITIONAL([HOST_LINUX], [test "${gaol_host_linux}" = yes])

# The tools that compile the manuals (make -C manual pdf)
AC_CHECK_PROG(PDFLATEX,pdflatex,pdflatex,no)
AC_CHECK_PROG(BIBTEX,bibtex,bibtex,no)
AC_CHECK_PROG(MAKEINDEX,makeindex,makeindex,no)
if test "$MAKEINDEX" = "no"; then
  AC_CHECK_PROG(MAKEINDEX,makeidx,makeidx,no)
fi
AM_CONDITIONAL(PDF_TO_LATEX,
               [test "${PDFLATEX}" != "no" -a "$MAKEINDEX" != "no" -a "$BIBTEX" != "no"])

# What GAOL's sources read of the system (gaol/gaol_fpu.h, gaol/gaol_exact.h,
# gaol/gaol_profile.cpp), as the CMake and the meson builds check it. The
# checks of headers, functions, types and sizes nothing read are gone, as
# AC_FUNC_MALLOC and AC_FUNC_REALLOC, which replaced malloc() and realloc()
# by rpl_malloc() and rpl_realloc() in a cross-compilation (GAOL v5).
AC_LANG([C++])
AC_CHECK_HEADER([fenv.h],
                [AC_DEFINE([HAVE_FENV_H],1,[Define to 1 if you have the <fenv.h> header file.])],
                [], [/* fenv.h alone */])
AC_MSG_CHECKING([for getrusage in sys/resource.h])
AC_LINK_IFELSE([AC_LANG_PROGRAM([[#include <sys/resource.h>]],
                                [[struct rusage r; return getrusage(RUSAGE_SELF, &r);]])],
               [have_getrusage=yes], [have_getrusage=no])
AC_MSG_RESULT([${have_getrusage}])
if test "${have_getrusage}" = yes; then
    AC_DEFINE([HAVE_GETRUSAGE],1,[Define to 1 if you have the `getrusage' function in <sys/resource.h>.])
fi


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

# The link option -mno-daz-ftz, where the compiler takes it (GCC 13 and later on
# x86, and the releases 11.4 and 12.4; GCC 9.4 and Clang 18 refuse it), which
# gaol.pc gives the code using GAOL in Libs, as CMakeLists.txt gives it to
# gaol::gaol (GAOL v5). GCC links
# crtfastmath.o into a program linked with -Ofast, -ffast-math or
# -funsafe-math-optimizations, and its constructor sets the flush-to-zero and
# denormals-are-zero modes of the SSE instructions, which make the bounds of
# every operation with a subnormal operand or result wrong. The option keeps
# that file out of the link; each operation of GAOL clears the two modes when
# it starts (gaol/gaol_fpu.h). Checked by linking, since it is a link option.
GAOL_PC_LIBS=""
AC_LANG_PUSH([C++])
gaol_save_LDFLAGS=$LDFLAGS
LDFLAGS="-Werror -mno-daz-ftz"
AC_MSG_CHECKING([whether the C++ compiler links with -mno-daz-ftz])
AC_LINK_IFELSE([AC_LANG_PROGRAM([[]], [[]])],[gaol_takes_mno_daz_ftz=yes],[gaol_takes_mno_daz_ftz=no])
AC_MSG_RESULT([${gaol_takes_mno_daz_ftz}])
LDFLAGS=$gaol_save_LDFLAGS
AC_LANG_POP([C++])
if test "${gaol_takes_mno_daz_ftz}" = yes; then
    GAOL_PC_LIBS="-mno-daz-ftz"
fi
AC_SUBST([GAOL_PC_LIBS])


AC_CONFIG_FILES([gaol.pc
                 Makefile
                 3rd/Makefile
                 gaol/Makefile
                 tests/Makefile
                 examples/Makefile
                 manual/Makefile
                 manual/v4/gaol_version.tex
                 manual/v5/gaol_version.tex])

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
printf "exceptions\t\t= $exceptions\n"
printf "asm\t\t\t= ${using_asm}\n"
printf "SSE2\t\t\t= ${using_sse2}\n"
printf "SSE3\t\t\t= ${has_sse3}\n"
printf "tests\t\t\t= ${with_tests}\n"
printf "examples\t\t= ${with_examples}\n"
printf "pdflatex\t\t= $PDFLATEX\n"
printf "makeindex\t\t= $MAKEINDEX\n"
printf "bibtex\t\t\t= $BIBTEX\n"
# The warning of a configure generated for another version than that of
# VERSION.txt, again after the summary, where it is seen (GAOL v5)
if test -n "$gaol_version_warning"; then
  AC_MSG_WARN([$gaol_version_warning])
fi
