#!/usr/bin/env bash
# The benchmark: draws the intervals (bench.py), compiles bench_gaol.cpp,
# bench_p1788.cpp, bench_filib.cpp, bench_profil.cpp, bench_boost.cpp,
# bench_double.cpp and bench_sun.f90, runs them, and writes
# the tables of the results into doc/compare/performance.md (between its
# markers). Run setup.sh first.
#
# N               operations of each kind (default 1000000)
# ROUNDS          times each program is run, in turn with the others, the best
#                 time of all being kept (default 3): a slowdown of the machine
#                 lasting a few seconds then spoils one round only
# REPEATS         runs of each operation in a round (default 5)
# P1788_REPEATS   the same for libieeep1788, far slower (default 1)
# OPS             a comma-separated list of operations (default: all of them,
#                 see bench_ops.h)
# CPU             a processor to run the programs on, with taskset
# LIBS            the libraries to run (default: "double gaol5 gaol5p gaol filib profil boost sun p1788"):
#                 gaol5 is the GAOL of this repository, installed in PREFIX,
#                 gaol5p the same built with GAOL_PRESERVE_ROUNDING=ON (the
#                 AVX-512 path of + - * / sqr sqrt and pown left on, which
#                 that option turns on by default), installed in
#                 GAOL_PRESERVE_PREFIX,
#                 and gaol the last GAOL of Frédéric Goualard, in
#                 GAOL_GOUALARD_PREFIX
# MERGE           1: the times of what ran (LIBS, OPS) replace their rows of
#                 results.csv, the other rows being kept, and the report is
#                 written from all of them, as make perf does for GAOL v5
#                 (default: the report is written from what ran alone)
#
# Copyright (c) 2026 ENSTA, France
#
# Created 2026-09-20 by Jordan NININ
set -euo pipefail
source "$(dirname "$0")/env.sh"
source "$(dirname "$0")/build_benchmarks.sh"

N="${N:-1000000}"
ROUNDS="${ROUNDS:-3}"
REPEATS="${REPEATS:-5}"
P1788_REPEATS="${P1788_REPEATS:-1}"
OPS="${OPS:-}"
CPU="${CPU:-}"
LIBS="${LIBS:-double gaol5 gaol5p gaol filib profil boost sun p1788}"
MERGE="${MERGE:-}"
OUT="$WORK/bench"
REPORT="${REPORT:-$CODE_DIR/../performance.md}"
mkdir -p "$OUT"

DATA="$OUT/data-$N.bin"
if [ ! -f "$DATA" ]; then
  echo "== drawing $N intervals of each kind"
  python3 "$CODE_DIR/bench.py" data "$N" "$DATA"
fi

run() {
  if [ -n "$CPU" ]; then taskset -c "$CPU" "$@"; else "$@"; fi
}

echo "== compiling"
compile_cpp_benchmarks
(cd "$OUT" && "$F90" $F90FLAGS_BENCH "$CODE_DIR/bench_sun.f90" -o bench_sun > f90.log 2>&1) \
  || { cat "$OUT/f90.log"; die "f90 failed"; }

RESULTS="$OUT/results.csv"
: > "$RESULTS"
for round in $(seq "$ROUNDS"); do
  for lib in $LIBS; do
    echo "== round $round/$ROUNDS: $lib"
    case "$lib" in
      double) run "$OUT/bench_double" "$DATA" "$REPEATS" "$OPS" ;;
      gaol) run "$OUT/bench_gaol" "$DATA" "$REPEATS" "$OPS" ;;
      gaol5) run "$OUT/bench_gaol5" "$DATA" "$REPEATS" "$OPS" ;;
      gaol5p) run "$OUT/bench_gaol5p" "$DATA" "$REPEATS" "$OPS" ;;
      p1788) run "$OUT/bench_p1788" "$DATA" "$P1788_REPEATS" "$OPS" ;;
      filib) run "$OUT/bench_filib" "$DATA" "$REPEATS" "$OPS" ;;
      profil) run "$OUT/bench_profil" "$DATA" "$REPEATS" "$OPS" ;;
      boost) run "$OUT/bench_boost" "$DATA" "$REPEATS" "$OPS" ;;
      sun) run "$OUT/bench_sun" "$DATA" "$REPEATS" "$OPS" ;;
      *) die "unknown library $lib" ;;
    esac | tee -a "$RESULTS"
  done
done

if [ -n "$MERGE" ]; then
  # The rows of what was not run, then the new ones; and, in machine.txt, the
  # lines of the GAOL of this repository that ran in place of the old ones
  [ -f "$CODE_DIR/results.csv" ] && [ -f "$CODE_DIR/machine.txt" ] \
    || die "MERGE needs results.csv and machine.txt: run run_bench.sh once"
  machine_description > "$OUT/machine-new.txt"
  python3 "$CODE_DIR/bench.py" merge "$CODE_DIR/results.csv" "$RESULTS" "$OUT/results-merged.csv"
  python3 "$CODE_DIR/bench.py" merge-machine "$CODE_DIR/machine.txt" "$OUT/machine-new.txt" "$OUT/machine.txt" \
          "$LIBS" "$(date '+%Y-%m-%d')"
  python3 "$CODE_DIR/bench.py" report "$OUT/results-merged.csv" "$OUT/machine.txt" "$REPORT"
  echo "== tables written to $REPORT, from the results of $LIBS merged into results.csv"
  cp "$OUT/results-merged.csv" "$CODE_DIR/results.csv"
  cp "$OUT/machine.txt" "$CODE_DIR/machine.txt"
  echo "== results kept in $CODE_DIR/results.csv and machine.txt"
  exit 0
fi

# What the results were measured on
machine_description fortran > "$OUT/machine.txt"

python3 "$CODE_DIR/bench.py" report "$RESULTS" "$OUT/machine.txt" "$REPORT"
echo "== tables written to $REPORT"
# The results of a whole run, which make perf (run_perf.sh) keeps for the
# libraries other than GAOL v5 when it measures GAOL v5 alone again
if [ -z "$OPS" ] && [ "$LIBS" = "double gaol5 gaol5p gaol filib profil boost sun p1788" ]; then
  cp "$RESULTS" "$CODE_DIR/results.csv"
  cp "$OUT/machine.txt" "$CODE_DIR/machine.txt"
  echo "== results kept in $CODE_DIR/results.csv and machine.txt, for make perf"
fi
