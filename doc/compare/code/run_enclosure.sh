#!/usr/bin/env bash
# The test of enclosure: draws the arguments of the elementary functions
# (enclosure.py data), compiles enclosure_gaol.cpp and enclosure_boost.cpp,
# runs them, and checks their results against mpmath (enclosure.py report),
# which writes the tables into doc/compare/enclosure.md (between its markers).
# Run setup.sh first.
#
# ENCLOSURE_N   arguments of each function (default 100000); mpmath computes
#               their values with 2000 bits, about 5 minutes for the 16
#               functions, once: they are kept in $WORK/enclosure/ref_*.bin
#
# Copyright (c) 2026 ENSTA, France
#
# Created 2026-10-06 by Jordan NININ
set -euo pipefail
source "$(dirname "$0")/env.sh"

N="${ENCLOSURE_N:-100000}"
OUT="$WORK/enclosure"
REPORT="${REPORT:-$CODE_DIR/../enclosure.md}"
mkdir -p "$OUT"

if [ "$(cut -d' ' -f1 "$OUT/args.txt" 2>/dev/null)" != "$N" ]; then
  echo "== drawing $N arguments of each function"
  python3 "$CODE_DIR/enclosure.py" data "$N" "$OUT"
fi

echo "== GAOL"
$CXX -std=c++11 -O2 $(gaol_cflags) -I"$CODE_DIR" "$CODE_DIR/enclosure_gaol.cpp" -o "$OUT/enclosure_gaol" $(gaol_libs)
"$OUT/enclosure_gaol" "$OUT"

echo "== Boost.Interval"
$CXX -std=c++11 -O2 $FMA_FLAGS $IA_CXXFLAGS $(boost_cflags) -I"$CODE_DIR" "$CODE_DIR/enclosure_boost.cpp" \
     -o "$OUT/enclosure_boost"
"$OUT/enclosure_boost" "$OUT"

echo "== checking the results with mpmath"
python3 "$CODE_DIR/enclosure.py" report "$OUT" "$REPORT"
echo "== tables written to $REPORT"
