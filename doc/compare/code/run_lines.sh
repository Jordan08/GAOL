#!/usr/bin/env bash
# The long lines of + - * /: compiles the programs of the benchmark, runs the
# lines of bench_ops.h on the intervals of run_bench.sh (drawn once, shared),
# and writes the tables of the results into doc/compare/performance.md, between
# its markers GENERATED LINES at the end. Run setup.sh first.
#
# N               evaluations of each line (default 1000000)
# ROUNDS          times each program is run, in turn with the others, the best
#                 time of all being kept (default 3)
# REPEATS         runs of each line in a round (default 5)
# P1788_REPEATS   the same for libieeep1788, far slower (default 1)
# LINE_LIST       a comma-separated list of lines (default: all of them, see
#                 bench_ops.h; set and empty: none, as for run_perf.sh)
# CPU             a processor to run the programs on, with taskset
# LIBS            the libraries to run (default: "double gaol5 gaol filib
#                 profil boost p1788", Solaris Studio having no line)
#
# Copyright (c) 2026 ENSTA, France
#
# Created 2026-10-17 by Jordan NININ
set -euo pipefail
# shellcheck source=env.sh
source "$(dirname "$0")/env.sh"
# shellcheck source=build_benchmarks.sh
source "$(dirname "$0")/build_benchmarks.sh"

N="${N:-1000000}"
ROUNDS="${ROUNDS:-3}"
REPEATS="${REPEATS:-5}"
P1788_REPEATS="${P1788_REPEATS:-1}"
ALL_LINES="sum16,diff16,mul8,mul8_tree,quot8,mix13,mix_tree,pos_sum16,pos_mul8,pos_quot8"
LINE_LIST="${LINE_LIST-$ALL_LINES}"
if [ -z "$LINE_LIST" ]; then
  echo "== LINE_LIST is empty: no line to run"
  exit 0
fi
CPU="${CPU:-}"
LIBS="${LIBS:-double gaol5 gaol filib profil boost p1788}"
OUT="$WORK/lines"
REPORT="${REPORT:-$CODE_DIR/../performance.md}"
mkdir -p "$OUT"

# The intervals of run_bench.sh, which it draws once
DATA="$WORK/bench/data-$N.bin"
if [ ! -f "$DATA" ]; then
  echo "== drawing $N intervals of each kind"
  mkdir -p "$WORK/bench"
  python3 "$CODE_DIR/bench.py" data "$N" "$DATA"
fi

run() {
  if [ -n "$CPU" ]; then taskset -c "$CPU" "$@"; else "$@"; fi
}

echo "== compiling"
compile_cpp_benchmarks

RESULTS="$OUT/results.csv"
: > "$RESULTS"
for round in $(seq "$ROUNDS"); do
  for lib in $LIBS; do
    echo "== round $round/$ROUNDS: $lib"
    case "$lib" in
      double) run "$OUT/bench_double" "$DATA" "$REPEATS" "$LINE_LIST" ;;
      gaol) run "$OUT/bench_gaol" "$DATA" "$REPEATS" "$LINE_LIST" ;;
      gaol5) run "$OUT/bench_gaol5" "$DATA" "$REPEATS" "$LINE_LIST" ;;
      p1788) run "$OUT/bench_p1788" "$DATA" "$P1788_REPEATS" "$LINE_LIST" ;;
      filib) run "$OUT/bench_filib" "$DATA" "$REPEATS" "$LINE_LIST" ;;
      profil) run "$OUT/bench_profil" "$DATA" "$REPEATS" "$LINE_LIST" ;;
      boost) run "$OUT/bench_boost" "$DATA" "$REPEATS" "$LINE_LIST" ;;
      *) die "unknown library $lib" ;;
    esac | tee -a "$RESULTS"
  done
done

machine_description > "$OUT/machine.txt"

python3 "$CODE_DIR/bench.py" lines-report "$RESULTS" "$OUT/machine.txt" "$REPORT"
echo "== tables written to $REPORT"
# The results of a whole run, as run_bench.sh keeps its own
if [ "$LINE_LIST" = "$ALL_LINES" ] && [ "$LIBS" = "double gaol5 gaol filib profil boost p1788" ]; then
  cp "$RESULTS" "$CODE_DIR/results_lines.csv"
  cp "$OUT/machine.txt" "$CODE_DIR/machine_lines.txt"
  echo "== results kept in $CODE_DIR/results_lines.csv and machine_lines.txt"
fi
