#!/usr/bin/env bash
# make perf: GAOL v5 alone measured again on the benchmark of
# ../performance.md, and the tables of the report written again with its new
# times. The target perf of the three builds (CMake, configure, meson) compiles
# bench_gaol.cpp with the flags and the GAOL of the build, and runs:
#
#   run_perf.sh <bench_gaol program> [<build>] [<C++ compiler>]
#
# GAOL v5 built with GAOL_PRESERVE_ROUNDING=ON, GAOL 4.2.3, filib++,
# libieeep1788, PROFIL/BIAS, Solaris Studio, Boost.Interval and the doubles are
# not run: their columns keep the times results.csv holds, which run_bench.sh
# measured with them (run_all.sh measures them all again). The new times of
# GAOL v5 replace its rows of results.csv, whose tables bench.py then writes
# into the report, between its markers, and machine.txt says when and how GAOL
# v5 was measured. The text around the tables, which comments on them, is to
# be checked by hand.
#
# It needs Python 3 with numpy, which draws the intervals (bench.py data), the
# same ones as run_bench.sh draws. The variables of run_bench.sh apply: N,
# ROUNDS, REPEATS, OPS, CPU (taskset -c), and WORK, where the intervals are
# drawn (default: perf in the current directory, the build directory), and
# REPORT, the report written (default: ../performance.md). The machine should
# do nothing else meanwhile, and be the one results.csv was measured on (see
# machine.txt), for the columns to compare.
#
# Copyright (c) 2026 ENSTA, France
#
# Created 2026-09-27 by Jordan NININ
set -euo pipefail

if [ $# -lt 1 ]; then
  echo "usage: run_perf.sh <bench_gaol program> [<build>] [<C++ compiler>]" >&2
  exit 2
fi
BENCH="$(cd "$(dirname "$1")" && pwd)/$(basename "$1")"
BUILD="${2:-}"
BUILD_CXX="${3:-${CXX:-c++}}"
CODE_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
ROOT_DIR="$(cd "$CODE_DIR/../../.." && pwd)"

N="${N:-1000000}"
ROUNDS="${ROUNDS:-3}"
REPEATS="${REPEATS:-5}"
OPS="${OPS:-}"
CPU="${CPU:-}"
WORK="${WORK:-$PWD/perf}"
REPORT="${REPORT:-$CODE_DIR/../performance.md}"
RESULTS="$CODE_DIR/results.csv"
MACHINE="$CODE_DIR/machine.txt"
RESULTS_LINES="$CODE_DIR/results_lines.csv"
MACHINE_LINES="$CODE_DIR/machine_lines.txt"
mkdir -p "$WORK"

python3 -c "import numpy" 2> /dev/null || { echo "run_perf.sh: Python 3 with numpy is needed to draw the intervals" >&2; exit 1; }
# The long lines: all ten of them unless LINE_LIST says otherwise, even empty
LINE_LIST="${LINE_LIST-$(python3 "$CODE_DIR/bench.py" line-names)}"
[ -f "$RESULTS" ] && [ -f "$MACHINE" ] || { echo "run_perf.sh: $RESULTS or $MACHINE is missing: run run_bench.sh once" >&2; exit 1; }
# The same number of operations as the other libraries were measured with
measured_n="$(head -1 "$RESULTS" | cut -d, -f3)"
if [ "$measured_n" != "$N" ]; then
  echo "run_perf.sh: results.csv was measured with N=$measured_n, not N=$N" >&2
  exit 1
fi
if [ -n "$LINE_LIST" ]; then
  [ -f "$RESULTS_LINES" ] && [ -f "$MACHINE_LINES" ] || { echo "run_perf.sh: $RESULTS_LINES or $MACHINE_LINES is missing: run run_lines.sh once" >&2; exit 1; }
  measured_n="$(head -1 "$RESULTS_LINES" | cut -d, -f3)"
  if [ "$measured_n" != "$N" ]; then
    echo "run_perf.sh: results_lines.csv was measured with N=$measured_n, not N=$N" >&2
    exit 1
  fi
fi

DATA="$WORK/data-$N.bin"
if [ ! -f "$DATA" ]; then
  echo "== drawing $N intervals of each kind"
  python3 "$CODE_DIR/bench.py" data "$N" "$DATA"
fi

run() {
  if [ -n "$CPU" ]; then taskset -c "$CPU" "$@"; else "$@"; fi
}

NEW="$WORK/gaol5.csv"
: > "$NEW"
for round in $(seq "$ROUNDS"); do
  echo "== round $round/$ROUNDS: GAOL v5 ($BENCH)"
  run "$BENCH" "$DATA" "$REPEATS" "$OPS" | tee -a "$NEW"
done

# The long lines, run on the same intervals (GAOL v5)
if [ -n "$LINE_LIST" ]; then
  NEW_LINES="$WORK/gaol5_lines.csv"
  : > "$NEW_LINES"
  for round in $(seq "$ROUNDS"); do
    echo "== round $round/$ROUNDS: GAOL v5, the long lines ($BENCH)"
    run "$BENCH" "$DATA" "$REPEATS" "$LINE_LIST" | tee -a "$NEW_LINES"
  done
  python3 "$CODE_DIR/bench.py" merge "$RESULTS_LINES" "$NEW_LINES" "$WORK/results_lines.csv"
fi

# The rows of the other libraries, and of the operations of GAOL v5 not run
# (OPS), then the new ones
python3 - "$RESULTS" "$NEW" "$WORK/results.csv" <<'EOF'
import csv, sys
old, new, out = sys.argv[1:4]
new_rows = [r for r in csv.reader(open(new)) if len(r) == 8]
ops = {r[1].strip() for r in new_rows}
kept = [r for r in csv.reader(open(old)) if len(r) == 8 and not (r[0].strip() == 'gaol5' and r[1].strip() in ops)]
with open(out, 'w', newline='') as f:
    csv.writer(f, lineterminator='\n').writerows(kept + new_rows)
EOF

# What GAOL v5 was measured on, in place of its line of machine.txt (and of
# machine_lines.txt, for the long lines). The processor is the one /proc/cpuinfo
# names ("model name"), or sysctl gives on macOS, which has no /proc/cpuinfo;
# the line says it does not know the processor otherwise (GAOL v5: the failed
# grep ended the script, with pipefail)
gaol_commit="$(git -C "$ROOT_DIR" describe --always 2>/dev/null || echo "?")"
git -C "$ROOT_DIR" diff --quiet HEAD -- . ':!doc' 2>/dev/null || gaol_commit="$gaol_commit-dirty"
processor="$(grep -m1 'model name' /proc/cpuinfo 2>/dev/null | cut -d: -f2 | sed 's/^ *//' || true)"
[ -n "$processor" ] || processor="$(sysctl -n machdep.cpu.brand_string 2>/dev/null || true)"
[ -n "$processor" ] || processor="an unknown processor"
compiler="$($BUILD_CXX --version 2>/dev/null | head -1)"
line="GAOL V5.0.0:     measured again on $(date '+%Y-%m-%d') by make perf${BUILD:+ ($BUILD build)}, the branch of this checkout ($gaol_commit), $compiler, on $processor${CPU:+ (taskset -c $CPU)}, CORE-MATH of 3rd/math-core compiled into the library"
machine_with_line() {
python3 - "$1" "$2" "$line" <<'EOF'
import sys
src, out, line = sys.argv[1:4]
lines = [l.rstrip('\n') for l in open(src)]
lines = [line if l.startswith('GAOL V5.0.0:') else l for l in lines]
open(out, 'w').write('\n'.join(lines) + '\n')
EOF
}
machine_with_line "$MACHINE" "$WORK/machine.txt"

python3 "$CODE_DIR/bench.py" report "$WORK/results.csv" "$WORK/machine.txt" "$REPORT"
cp "$WORK/results.csv" "$RESULTS"
cp "$WORK/machine.txt" "$MACHINE"
echo "== GAOL v5 written into the tables of $REPORT, and into $RESULTS and $MACHINE"
if [ -n "$LINE_LIST" ]; then
  machine_with_line "$MACHINE_LINES" "$WORK/machine_lines.txt"
  python3 "$CODE_DIR/bench.py" lines-report "$WORK/results_lines.csv" "$WORK/machine_lines.txt" "$REPORT"
  cp "$WORK/results_lines.csv" "$RESULTS_LINES"
  cp "$WORK/machine_lines.txt" "$MACHINE_LINES"
  echo "== the long lines of GAOL v5 written into the tables of $REPORT, and into $RESULTS_LINES and $MACHINE_LINES"
fi
