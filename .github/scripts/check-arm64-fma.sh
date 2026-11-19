#!/bin/sh
# check-arm64-fma.sh LIBMINGWEX_A OBJECT_DIR...: checks, on the objects of
# code for 64-bit ARM, that fma() is the fused multiply-add instruction (GAOL
# v5), which is what lets GAOL accept mingw-w64 for 64-bit ARM before version
# 12, whose math library (libmingwex) holds its own fma() and round(). Three
# things are read, in the code, not taken from a comment:
#
#  - the fma() of LIBMINGWEX_A (the library of the toolchain, which
#    `clang -print-file-name=libmingwex.a` gives) is one fmadd and nothing
#    else that computes: no fmul, fadd or fsub, no call or jump to another
#    function. The member of the archive that defines it is found with the
#    symbols of the archive; none is no success, as in a toolchain that has
#    taken fma() out of libmingwex (mingw-w64 12), which is no case for this
#    check;
#  - the objects under OBJECT_DIR (build/CMakeFiles/gaol.dir and
#    build/CMakeFiles/gaol_core_math.dir) hold fused instructions (fmadd,
#    fmsub, fnmadd, fnmsub): __builtin_fma() of the sources of CORE-MATH and
#    the exact products of GAOL are the instruction, and not a call of the
#    library (the number is printed);
#  - and no object calls the fma() or the round() of the C library (the
#    undefined symbols of the objects, which nm lists; check-no-fma-call.sh
#    does the first on the platforms where that fma() is wrong): the round()
#    of mingw-w64's own library depends on the rounding direction, and GAOL
#    calls its own gaol_round() instead.
#
# The tools are those of LLVM, which the toolchains of mingw-w64 for ARM are
# made of: llvm-nm, llvm-ar and llvm-objdump, or the NM, AR and OBJDUMP of the
# environment. Exits 1 with an error for each thing that fails, 0 when all
# hold.
#
# Copyright (c) 2026 ENSTA, France
#
# By: Jordan NININ (created 2026-11-18)
#
# GAOL is a software distributed WITHOUT ANY WARRANTY. Read the associated
# COPYING.LIB file for information.

nm=${NM:-llvm-nm}
ar=${AR:-llvm-ar}
objdump=${OBJDUMP:-llvm-objdump}
for tool in "$nm" "$ar" "$objdump"; do
  if ! command -v "$tool" > /dev/null 2>&1; then
    echo "::error::$tool not found: the code cannot be read"
    exit 1
  fi
done
if [ $# -lt 2 ]; then
  echo "usage: check-arm64-fma.sh LIBMINGWEX_A OBJECT_DIR..."
  exit 2
fi
library=$1
shift
if [ ! -f "$library" ]; then
  echo "::error::$library: no such file"
  exit 1
fi
work=$(mktemp -d) || exit 1
trap 'rm -rf "$work"' EXIT HUP INT TERM
failed=0

# The member of the library that defines fma: the symbols of an archive come
# in blocks, a line `member.o:` and then its symbols
"$nm" --defined-only "$library" 2> "$work/err" |
  awk '/^[^ ]+:$/ { member = $0; sub(/:$/, "", member); next } $3 == "fma" && $2 ~ /^[TtDd]$/ { print member; exit }' > "$work/member"
member=$(cat "$work/member")
if [ -z "$member" ]; then
  echo "::error::$library defines no fma(): this toolchain does not have its own, and there is nothing to check here"
  exit 1
fi
(cd "$work" && "$ar" x "$library" "$member") || { echo "::error::cannot extract $member of $library"; exit 1; }
"$objdump" -d --no-show-raw-insn "$work/$member" > "$work/disassembly" || { echo "::error::cannot read $member"; exit 1; }
# the instructions of fma: from its label to the next blank line
awk '/^[0-9a-f]+ <fma>:/ { inside = 1; next } inside && /^$/ { exit } inside { print }' "$work/disassembly" > "$work/fma"
echo "fma() of $library, member $member:"
cat "$work/fma"
if ! grep -E '[[:space:]]fmadd[[:space:]]' "$work/fma" > /dev/null; then
  echo "::error::the fma() of $library is not a fmadd"
  failed=1
fi
if grep -E '[[:space:]](fmul|fadd|fsub|fnmul|bl|blr|b|br)[[:space:]]' "$work/fma" > /dev/null; then
  echo "::error::the fma() of $library computes with something besides one fmadd"
  failed=1
fi

# The objects of the library
for dir in "$@"; do
  find "$dir" \( -name '*.obj' -o -name '*.o' \) -print
done | sort > "$work/objects"
count=$(wc -l < "$work/objects" | tr -d ' ')
if [ "$count" = 0 ]; then
  echo "::error::no object (*.obj, *.o) under $*: nothing to check"
  exit 1
fi
fused=0
undefined=0
while IFS= read -r object; do
  "$objdump" -d --no-show-raw-insn "$object" > "$work/object.s" 2> "$work/err" || { echo "::error::cannot read $object: $(head -1 "$work/err")"; failed=1; continue; }
  fused=$((fused + $(grep -cE '[[:space:]](fmadd|fmsub|fnmadd|fnmsub)[[:space:]]' "$work/object.s")))
  if ! "$nm" -u "$object" > "$work/symbols" 2> "$work/err"; then
    echo "::error::$nm -u failed on $object: $(head -1 "$work/err")"
    failed=1
    continue
  fi
  undefined=$((undefined + $(wc -l < "$work/symbols")))
  if grep -E '(^|[[:space:]])U[[:space:]]+(__imp__?|_)?(fma|round)$' "$work/symbols" > /dev/null; then
    echo "::error::$object calls the fma() or the round() of the C library"
    failed=1
  fi
done < "$work/objects"
echo "$count objects checked, $fused fused multiply-add instructions, $undefined undefined symbols"
if [ "$fused" = 0 ]; then
  echo "::error::no fused multiply-add instruction in $count objects: __builtin_fma() is not the instruction"
  failed=1
fi
if [ "$undefined" = 0 ]; then
  echo "::error::no undefined symbol in $count objects: $nm did not read them"
  failed=1
fi
exit "$failed"
