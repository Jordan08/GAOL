#!/bin/sh
# The options gaol/gaol_config.h refuses where GCC and Clang define a macro for
# them, in the autotools and meson builds (GAOL v5), as the tests
# refused_finite_math_only, refused_fast_math and refused_positive of
# tests/CMakeLists.txt check them in the CMake build: refused_options.cpp, a
# program using GAOL, is compiled with the compiler and the flags the build
# gives the code using GAOL ($GAOL_CXX and $GAOL_CXXFLAGS), followed by
# -ffinite-math-only, then by -ffast-math, and the compilation has to fail with
# the message of the refusal; with neither, it has to succeed. The option comes
# after the flags, whose -fno-fast-math turns it off otherwise. A compilation
# fails for many reasons, so its exit status only counts for the one that has
# to succeed. Only the preprocessor and the parser run (-fsyntax-only): the
# refusal is an #error. Not with Visual C++, which has neither option nor
# macro: neither build runs it there. As CMake runs its tests with GCC and Clang
# only, a compiler that does not define __GNUC__ skips it (exit status 77).

: "${GAOL_CXX:?the compiler of the code using GAOL}"
source=$(dirname "$0")/refused_options.cpp
status=0

case $($GAOL_CXX $GAOL_CXXFLAGS -dM -E "$source" 2>/dev/null) in
  *__GNUC__*) ;;
  *) echo "SKIP: neither GCC nor Clang"; exit 77 ;;
esac

if output=$($GAOL_CXX $GAOL_CXXFLAGS -fsyntax-only "$source" 2>&1); then
  echo "PASS: compiled without a refused option"
else
  printf '%s\n' "$output"
  echo "FAIL: not compiled without a refused option"
  status=1
fi

for option in -ffinite-math-only -ffast-math; do
  output=$($GAOL_CXX $GAOL_CXXFLAGS $option -fsyntax-only "$source" 2>&1)
  case $output in
    *"GAOL cannot be compiled with $option"*)
      echo "PASS: $option refused" ;;
    *)
      printf '%s\n' "$output"
      echo "FAIL: $option not refused"
      status=1 ;;
  esac
done

# The SSE2 intervals on 32-bit Windows (GAOL v5), which gaol/gaol_config.h
# refuses where the compiler is not told to ask for aligned memory, as the
# tests refused_aligned_new, refused_aligned_new_cxx17 and
# refused_aligned_new_positive of tests/CMakeLists.txt check them in the CMake
# build: refused_aligned_new.cpp, which includes that header alone, is compiled
# with -D_WIN32 -U_WIN64 for the target and without -faligned-new=8, which the
# flags carry on 32-bit Windows, in C++11 and in C++17 (the flags of the
# autotools build are in C++17 already, which would leave out the case where GCC
# has no __cpp_aligned_new), and the compilation has to fail with the message
# of the refusal; with the option it has to succeed. Only where the intervals are the SSE2 ones, and on a system
# that is not Windows, whose headers take no -U_WIN64.
aligned_source=$(dirname "$0")/refused_aligned_new.cpp
aligned_message="need the compiler to ask for aligned memory"
aligned_flags=$(printf '%s\n' "$GAOL_CXXFLAGS" | sed 's/-faligned-new=8//g')
case $(uname -s 2>/dev/null) in
  MINGW*|MSYS*|CYGWIN*|Windows*)
    echo "SKIP: aligned new, on a system of Windows" ;;
  *)
    if printf '#include <gaol/gaol_config.h>\n#if !GAOL_USING_SSE2_INSTRUCTIONS\n#error not the SSE2 intervals\n#endif\n' \
         | $GAOL_CXX $GAOL_CXXFLAGS -x c++ -fsyntax-only - > /dev/null 2>&1; then
      for standard in -std=c++11 -std=c++17; do
        output=$($GAOL_CXX $aligned_flags $standard -D_WIN32 -U_WIN64 -fsyntax-only "$aligned_source" 2>&1)
        case $output in
          *"$aligned_message"*)
            echo "PASS: the SSE2 intervals on 32-bit Windows refused without -faligned-new=8 ($standard)" ;;
          *)
            printf '%s\n' "$output"
            echo "FAIL: the SSE2 intervals on 32-bit Windows not refused without -faligned-new=8 ($standard)"
            status=1 ;;
        esac
      done
      if output=$($GAOL_CXX $aligned_flags -D_WIN32 -U_WIN64 -faligned-new=8 -fsyntax-only "$aligned_source" 2>&1); then
        echo "PASS: the SSE2 intervals on 32-bit Windows accepted with -faligned-new=8"
      else
        printf '%s\n' "$output"
        echo "FAIL: the SSE2 intervals on 32-bit Windows refused with -faligned-new=8"
        status=1
      fi
    else
      echo "SKIP: aligned new, the intervals are not the SSE2 ones"
    fi ;;
esac

exit $status
