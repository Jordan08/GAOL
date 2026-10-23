#!/bin/sh
# check-gcc-windows-fma.sh BUILD_DIR...: checks the objects of a CMake build of
# GAOL by GCC for Windows (x86-64 and 32-bit x86), which GAOL_FMA gives -mfma
# -mprefer-vector-width=128 (CMakeLists.txt; GAOL v5, point 110 of TODO.md):
#
# - no object, GAOL's, CORE-MATH's and the tests', holds an instruction that
#   names a 256-bit or 512-bit register (%ymm, %zmm). GCC takes the stack to be
#   aligned on 32 bytes for the ymm registers it spills, where the Windows x64
#   ABI aligns it on 16 (GCC bug 54412): -mfma alone, which turns AVX on, had
#   the vectorizer of GCC 16.2 write 30598 such lines in the objects of GAOL,
#   CORE-MATH and the tests, and -mprefer-vector-width=128 none. On 32-bit x86
#   the same flags gave the same objects with GCC 15.2 and 14.2 (4709 lines with
#   -mfma alone, none with both), but GCC 13.2 leaves 9 lines in two test
#   objects: there the lines are listed and noticed, not an error, the bug not
#   being known on 32-bit x86. The check finds the day the option is lost, or a
#   GCC stops honouring it;
# - the objects of gaol_interval_core.cpp (the exact products, the integer
#   powers) and of the cos of CORE-MATH hold fused multiply-add instructions
#   (vfmadd): without them CORE-MATH calls a software fma(), and pow(x, 3),
#   log and cos take 3 to 6 times as long (point 110). The instructions also
#   show that objdump disassembles these objects, so that a count of zero
#   %ymm is no mere failure to read them.
#
# The output of objdump is read, so the objects have to be those of the
# compiler (Visual C++ objects are no input). Exits 1 with an error for each
# thing found, 0 when there is none.
#
# Copyright (c) 2026 ENSTA, France
#
# Created 2026-10-20 by Jordan NININ
objdump=${OBJDUMP:-objdump}
if ! command -v "$objdump" > /dev/null 2>&1; then
  echo "::error::$objdump not found: the objects cannot be checked"
  exit 1
fi
if [ $# -eq 0 ]; then
  echo "usage: check-gcc-windows-fma.sh BUILD_DIR..."
  exit 2
fi
work=$(mktemp -d) || exit 1
trap 'rm -rf "$work"' EXIT HUP INT TERM
failed=0

# Every object under the directories, as the compiler left them
for dir in "$@"; do
  find "$dir" \( -name '*.obj' -o -name '*.o' \) -print
done | sort > "$work/objects"
count=$(wc -l < "$work/objects" | tr -d ' ')
if [ "$count" = 0 ]; then
  echo "::error::no object (*.obj, *.o) under $*: nothing to check"
  exit 1
fi

# The lines naming %ymm or %zmm, in every object. On x86-64 they are an error
# (GCC bug 54412); on 32-bit x86 they are listed and noticed only: that bug was
# not shown there (GAOL v5), and GCC 13.2 leaves a few in test objects that
# copy structures, in spite of -mprefer-vector-width=128
total=0
x86_32=0
while IFS= read -r object; do
  if ! "$objdump" -d --no-show-raw-insn "$object" > "$work/dis" 2> "$work/err"; then
    echo "::error::$objdump -d failed on $object: $(head -1 "$work/err")"
    failed=1
    continue
  fi
  if "$objdump" -f "$object" 2> /dev/null | grep -q -E 'file format .*i386'; then
    x86_32=1
  fi
  n=$(grep -c -E '%[yz]mm' "$work/dis")
  if [ "$n" != 0 ]; then
    echo "  $n lines with %ymm or %zmm: $object"
    grep -E '%[yz]mm' "$work/dis" | head -6 | sed 's/^/      /'
    total=$((total + n))
  fi
done < "$work/objects"
echo "$count objects checked, $total lines with %ymm or %zmm"
if [ "$total" != 0 ]; then
  if [ "$x86_32" = 1 ]; then
    echo "::notice::$total instructions on 256-bit or 512-bit registers in the objects of GCC for 32-bit Windows: GCC bug 54412 is not known there (GAOL v5), see the lines above"
  else
    echo "::error::$total instructions on 256-bit or 512-bit registers in the objects of GCC for Windows: GCC bug 54412 (it spills them to a stack aligned on 16 bytes, the Windows x64 ABI, with aligned moves for 32), which -mfma -mprefer-vector-width=128 of GAOL_FMA (CMakeLists.txt) keeps out of the code"
    failed=1
  fi
fi

# The fused multiply-add instructions, in the objects that compute with fma()
for source in gaol_interval_core.cpp cos.c; do
  find "$@" \( -name "$source.obj" -o -name "$source.o" \) -print | sort > "$work/fma-objects"
  if [ ! -s "$work/fma-objects" ]; then
    echo "::error::no object of $source: the fused multiply-add instructions cannot be looked for"
    failed=1
    continue
  fi
  while IFS= read -r object; do
    if ! "$objdump" -d --no-show-raw-insn "$object" > "$work/dis" 2> "$work/err"; then
      echo "::error::$objdump -d failed on $object: $(head -1 "$work/err")"
      failed=1
      continue
    fi
    n=$(grep -c -E '[[:space:]]vfn?m(add|sub)[0-9]*[sp][sd][[:space:]]' "$work/dis")
    echo "$n fused multiply-add instructions: $object"
    if [ "$n" = 0 ]; then
      echo "::error::no fused multiply-add instruction in $object: GAOL_FMA gave GCC for Windows no -mfma (CMakeLists.txt), so that CORE-MATH calls a software fma() and pow(x, 3), log and cos take 3 to 6 times as long (point 110 of TODO.md)"
      failed=1
    fi
  done < "$work/fma-objects"
done
exit "$failed"
