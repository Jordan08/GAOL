#!/bin/sh
# Builds the tests of tests/ and tests/performance.cpp with a GAOL installed by
# the autotools or the meson build, and runs the tests, as the continuous
# integration does. To be run from the root of GAOL's sources:
#
#   sh .github/scripts/tests.sh <prefix of GAOL> <prefix of mathlib> static|shared
#
# The prefix of mathlib is the one of GAOL when the mathlib of 3rd/mathlib was
# installed with it. static links libgaol.a, shared links libgaol.so (or
# .dylib) with an rpath. The flags of the tests are those of TEST_FLAGS, with
# which the code using GAOL is compiled (see CMakeLists.txt); on a 32-bit x86
# processor, -msse2 -mfpmath=sse too, without which gaol/gaol_config.h refuses
# to compile.
set -e
prefix=$1
mathlib=$2
linking=$3
flags="${TEST_FLAGS:--std=c++17 -O2 -frounding-math -fno-fast-math -ffp-contract=off}"
case "$(${CXX:-c++} -dumpmachine 2>/dev/null)" in
  i?86-*) flags="$flags -msse2 -mfpmath=sse" ;;
esac
if [ "$linking" = shared ]; then
  libs="-L$prefix/lib -lgaol $mathlib/lib/libultim.a -Wl,-rpath,$prefix/lib"
else
  libs="$prefix/lib/libgaol.a $mathlib/lib/libultim.a"
fi
grep -H -E "GAOL_PRESERVE_ROUNDING|USING_SSE2_INSTRUCTIONS|USING_SSE3_INSTRUCTIONS|GAOL_VERBOSE_MODE" "$prefix/include/gaol/gaol_configuration.h" || true
status=0
for test in arithmetic elementary numbers other_functions rounding_direction; do
  ${CXX:-c++} $flags -I"$prefix/include" -Itests tests/$test.cpp $libs -o $test
  # The checks that failed, which the last lines do not show
  if ./$test > $test.log 2>&1; then
    tail -1 $test.log
  else
    grep -E "checks, [1-9][0-9]* failed|^FAILED" $test.log | head -60
    status=1
  fi
done
${CXX:-c++} $flags -I"$prefix/include" tests/performance.cpp $libs -o performance
# The same test built with the flags and libraries of gaol.pc alone, where
# pkg-config exists (its Cflags carry the flags of interval arithmetic)
if command -v pkg-config > /dev/null; then
  export PKG_CONFIG_PATH="$prefix/lib/pkgconfig"
  # -lgaol takes the shared library where there is one: on Windows, its DLL
  # has to be on the PATH (meson installs it in bin), as a Unix path under
  # MSYS2, where the colon of a Windows path breaks the PATH
  if command -v cygpath > /dev/null; then
    export PATH="$(cygpath -u "$prefix")/bin:$(cygpath -u "$prefix")/lib:$PATH"
  else
    export PATH="$prefix/bin:$prefix/lib:$PATH"
  fi
  echo "pkg-config --cflags --libs gaol: $(pkg-config --cflags --libs gaol)"
  ${CXX:-c++} -std=c++17 -O2 $(pkg-config --cflags gaol) -Itests tests/rounding_direction.cpp $(pkg-config --libs gaol) \
    -Wl,-rpath,"$prefix/lib" -o rounding_direction_pc
  if ./rounding_direction_pc > rounding_direction_pc.log 2>&1; then
    echo "with pkg-config: $(tail -1 rounding_direction_pc.log)"
  else
    echo "with pkg-config: rounding_direction failed (exit code $?)"
    grep -E "checks, [1-9][0-9]* failed|^FAILED" rounding_direction_pc.log | head -20
    status=1
  fi
else
  echo "pkg-config not found: gaol.pc not checked"
fi
exit $status
