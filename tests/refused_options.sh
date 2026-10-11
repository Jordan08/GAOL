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

exit $status
