#!/bin/sh
# check-no-fma-call.sh OBJECT_DIR...: checks that no object under the
# directories calls the fma() of the C library, which GAOL does not, where that
# library's is wrong and the build has no fused multiply-add instruction: with
# GAOL_FMA=OFF, or on a processor that has none, the sources of CORE-MATH and
# the exact products of GAOL compute with gaol_fma() (gaol/math/gaol_fma.h,
# GAOL_SOFT_FMA of gaol/gaol_config.h), and with -mfma with the instruction
# (GAOL v5). The fma() of mingw-w64's own math library (before version 12, or
# linked with msvcrt.dll) and that of Cygwin are not correctly rounded, and the
# bounds of the elementary functions that call them do not enclose the exact
# values; the check finds the day a source calls it again, with std::fma() or
# a builtin that the macros of gaol/math/gaol_core_math_port.h do not replace.
#
# It reads the undefined symbols of the objects with nm, GCC's and Clang's
# (the objects of Visual C++ are no input), and counts them as well: a total
# of zero is no mere failure to read the objects. Give the directories of the
# objects of the library, build/CMakeFiles/gaol.dir and
# build/CMakeFiles/gaol_core_math.dir, not those of the tests, which call
# std::fma() to compare with. Exits 1 with an error for each object that calls
# it, 0 when there is none.
#
# Copyright (c) 2026 ENSTA, France
#
# By: Jordan NININ (created 2026-11-18)
#
# GAOL is a software distributed WITHOUT ANY WARRANTY. Read the associated
# COPYING.LIB file for information.

nm=${NM:-nm}
if ! command -v "$nm" > /dev/null 2>&1; then
  echo "::error::$nm not found: the objects cannot be checked"
  exit 1
fi
if [ $# -eq 0 ]; then
  echo "usage: check-no-fma-call.sh OBJECT_DIR..."
  exit 2
fi
work=$(mktemp -d) || exit 1
trap 'rm -rf "$work"' EXIT HUP INT TERM
failed=0

for dir in "$@"; do
  find "$dir" \( -name '*.obj' -o -name '*.o' \) -print
done | sort > "$work/objects"
count=$(wc -l < "$work/objects" | tr -d ' ')
if [ "$count" = 0 ]; then
  echo "::error::no object (*.obj, *.o) under $*: nothing to check"
  exit 1
fi

# The undefined symbols named fma: fma on most systems, _fma on 32-bit x86
# Windows, and the import of the DLL the program is linked with
undefined=0
while IFS= read -r object; do
  if ! "$nm" -u "$object" > "$work/symbols" 2> "$work/err"; then
    echo "::error::$nm -u failed on $object: $(head -1 "$work/err")"
    failed=1
    continue
  fi
  undefined=$((undefined + $(wc -l < "$work/symbols")))
  if grep -E '(^|[[:space:]])U[[:space:]]+(__imp__?|_)?fma$' "$work/symbols" > /dev/null; then
    echo "::error::$object calls the fma() of the C library"
    failed=1
  fi
done < "$work/objects"
echo "$count objects checked, $undefined undefined symbols"
if [ "$undefined" = 0 ]; then
  echo "::error::no undefined symbol in $count objects: $nm did not read them"
  failed=1
fi
exit "$failed"
