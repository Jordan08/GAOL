#!/bin/sh
# Checks the headers of an installed GAOL as the code using it sees them
# (GAOL v5), in the continuous integration:
#
#   sh .github/scripts/headers.sh <prefix of GAOL>
#
# - each header of include/gaol, included alone, compiles with -std=c++11,
#   the oldest standard GAOL takes, and -Wall -Wextra -Werror, but those that
#   other headers include in the middle of their own code (gaol_interval_fpu.h
#   and gaol_interval_sse.h, the inline functions of gaol_interval.h) or with
#   Visual C++ only (gaol_fpu_msvc.h);
# - including gaol/gaol, gaol/gaol_expression.h and gaol/gaol_expr_eval.h
#   defines no macro whose name does not start with GAOL_ or gaol_ (__GAOL,
#   __gaol for the include guards): the macros those headers define are those
#   the preprocessor has after them and not after the headers of the system
#   they include, which an earlier file includes alone (with __has_include, for
#   those of other systems, and only for Windows those of Windows: the
#   intrin.h of Clang includes the one of the system).
#
# The flags are those of pkg-config --cflags gaol, where the prefix has
# gaol.pc, as the code using GAOL is compiled; TEST_FLAGS (see tests.sh)
# otherwise. CXX is the compiler (c++ by default).
#
# Copyright (c) 2026 ENSTA, France
#
# Created 2026-10-04 by Jordan NININ
set -e
prefix=${1:?the prefix of an installed GAOL}
cxx=${CXX:-c++}
pc=$(ls "$prefix"/lib*/pkgconfig/gaol.pc "$prefix"/lib/*/pkgconfig/gaol.pc 2> /dev/null | head -n 1)
if [ -n "$pc" ] && command -v pkg-config > /dev/null; then
  flags=$(PKG_CONFIG_PATH=$(dirname "$pc") pkg-config --cflags gaol)
else
  flags="-I$prefix/include ${TEST_FLAGS:--O2 -frounding-math -fno-fast-math -ffp-contract=off}"
  case "$($cxx -dumpmachine 2> /dev/null)" in
    i?86-*) flags="$flags -msse2 -mfpmath=sse" ;;
  esac
fi
flags="-std=c++11 $flags -Wall -Wextra -Werror"
echo "$cxx $flags"
work=$(mktemp -d "${TMPDIR:-/tmp}/gaol-headers.XXXXXX")
trap 'rm -rf "$work"' EXIT INT TERM
status=0

for f in "$prefix"/include/gaol/*; do
  h=$(basename "$f")
  case "$h" in
    gaol_interval_fpu.h|gaol_interval_sse.h|gaol_fpu_msvc.h) continue ;;
  esac
  [ -f "$f" ] || continue
  printf '#include <gaol/%s>\n' "$h" > "$work/alone.cpp"
  if $cxx $flags -fsyntax-only "$work/alone.cpp" > "$work/alone.log" 2>&1; then
    echo "gaol/$h alone: ok"
  else
    echo "gaol/$h alone: FAILED"
    cat "$work/alone.log"
    status=1
  fi
done

# The headers of the system the headers of GAOL include
printf '#include <gaol/gaol>\n#include <gaol/gaol_expression.h>\n#include <gaol/gaol_expr_eval.h>\n' > "$work/gaol.cpp"
sed -n 's/^[[:space:]]*#[[:space:]]*include[[:space:]]*<\([^>]*\)>.*/\1/p' "$prefix"/include/gaol/* |
  grep -v '^gaol/' | sort -u |
  while read -r s; do
    case "$s" in
      intrin.h|sal.h) printf '#if defined(_WIN32) && __has_include(<%s>)\n' "$s" ;;
      *) printf '#if __has_include(<%s>)\n' "$s" ;;
    esac
    printf '#  include <%s>\n#endif\n' "$s"
  done > "$work/system.cpp"
cat "$work/system.cpp" "$work/gaol.cpp" > "$work/both.cpp"
# The macros defined after each file, the output of the preprocessor written
# to a file first: in a pipe, its failure would go unseen
for f in system both; do
  if ! $cxx $flags -E -dM "$work/$f.cpp" > "$work/$f.dM" 2> "$work/$f.log"; then
    echo "The preprocessor failed on the headers of GAOL and of the system:"
    cat "$work/$f.cpp" "$work/$f.log"
    exit 1
  fi
  sed -n 's/^#define \([A-Za-z_][A-Za-z0-9_]*\).*/\1/p' "$work/$f.dM" | sort -u > "$work/$f.txt"
done
leaked=$(comm -13 "$work/system.txt" "$work/both.txt" | grep -v -E '^(GAOL_|gaol_|__GAOL|__gaol)' || true)
if [ -n "$leaked" ]; then
  echo "Macros of GAOL's headers whose name does not start with GAOL_ or gaol_:"
  echo "$leaked"
  status=1
else
  echo "The macros of GAOL's headers start with GAOL_ or gaol_: $(comm -13 "$work/system.txt" "$work/both.txt" | wc -l) macros"
fi
exit $status
