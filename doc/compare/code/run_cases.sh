#!/usr/bin/env bash
# The special cases: generates the programs of cases.py, compiles them with
# GAOL, libieeep1788, filib++ and Solaris Studio, runs them, and writes the
# table of their results into doc/compare/special_cases.md (between its
# markers).
# Run setup.sh first.
set -euo pipefail
source "$(dirname "$0")/env.sh"

OUT="$WORK/cases"
REPORT="${REPORT:-$CODE_DIR/../special_cases.md}"
mkdir -p "$OUT"
python3 "$CODE_DIR/cases.py" generate "$OUT"

echo "== GAOL"
$CXX -std=c++11 -O2 $(gaol_cflags) "$OUT/cases_gaol.cpp" -o "$OUT/cases_gaol" $(gaol_libs)
"$OUT/cases_gaol" > "$OUT/gaol.txt"

echo "== libieeep1788"
$CXX -std=c++11 -O2 $IA_CXXFLAGS $(p1788_cflags) "$OUT/cases_p1788.cpp" -o "$OUT/cases_p1788" $(p1788_libs)
"$OUT/cases_p1788" > "$OUT/p1788.txt"

echo "== filib++"
$CXX -O2 $IA_CXXFLAGS $(filib_cflags) "$OUT/cases_filib.cpp" -o "$OUT/cases_filib" $(filib_libs)
"$OUT/cases_filib" > "$OUT/filib.txt"

echo "== Solaris Studio"
(cd "$OUT" && "$F90" $F90FLAGS_CASES cases_sun.f90 -o cases_sun > f90.log 2>&1) \
  || { cat "$OUT/f90.log"; die "f90 failed"; }
"$OUT/cases_sun" > "$OUT/sun.txt"

python3 "$CODE_DIR/cases.py" report "$OUT" "$REPORT"
echo "== table written to $REPORT"
