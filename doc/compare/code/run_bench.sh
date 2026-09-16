#!/usr/bin/env bash
# The benchmark: draws the intervals (bench.py), compiles bench_gaol.cpp,
# bench_p1788.cpp, bench_double.cpp and bench_sun.f90, runs them, and writes
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
# LIBS            the libraries to run (default: "double gaol sun p1788")
set -euo pipefail
source "$(dirname "$0")/env.sh"

N="${N:-1000000}"
ROUNDS="${ROUNDS:-3}"
REPEATS="${REPEATS:-5}"
P1788_REPEATS="${P1788_REPEATS:-1}"
OPS="${OPS:-}"
CPU="${CPU:-}"
LIBS="${LIBS:-double gaol sun p1788}"
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
$CXX -std=c++11 $CXXFLAGS_BENCH -I"$CODE_DIR" "$CODE_DIR/bench_double.cpp" -o "$OUT/bench_double"
$CXX -std=c++11 $CXXFLAGS_BENCH $(gaol_cflags) -I"$CODE_DIR" "$CODE_DIR/bench_gaol.cpp" -o "$OUT/bench_gaol" $(gaol_libs)
$CXX -std=c++11 $CXXFLAGS_BENCH $IA_CXXFLAGS $(p1788_cflags) -I"$CODE_DIR" "$CODE_DIR/bench_p1788.cpp" \
     -o "$OUT/bench_p1788" $(p1788_libs)
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
      p1788) run "$OUT/bench_p1788" "$DATA" "$P1788_REPEATS" "$OPS" ;;
      sun) run "$OUT/bench_sun" "$DATA" "$REPEATS" "$OPS" ;;
      *) die "unknown library $lib" ;;
    esac | tee -a "$RESULTS"
  done
done

# What the results were measured on
{
  echo "Date:            $(date '+%Y-%m-%d')"
  echo "Processor:       $(grep -m1 'model name' /proc/cpuinfo | cut -d: -f2 | sed 's/^ *//')${CPU:+ (taskset -c $CPU)}"
  echo "System:          $(uname -sr), $(. /etc/os-release 2>/dev/null && echo "$PRETTY_NAME")"
  echo "C++ compiler:    $($CXX --version | head -1)"
  echo "C++ flags:       -std=c++11 $CXXFLAGS_BENCH (GAOL: $(gaol_cflags | sed -e "s# *-I[^ ]*##g" -e "s#^ *##"); libieeep1788: $IA_CXXFLAGS)"
  echo "Fortran:         $("$F90" -V 2>&1 | head -1), flags: $F90FLAGS_BENCH"
  echo "GAOL:            $(git -C "$ROOT_DIR" rev-parse --short HEAD 2>/dev/null || echo '?'), CMake Release, mathlib 2.1.1"
  echo "libieeep1788:    ${P1788_COMMIT:0:7}, MPFR $(grep -m1 '#define MPFR_VERSION_STRING' "$PREFIX/include/mpfr.h" 2>/dev/null | cut -d'"' -f2), GMP $(grep -m1 -E '^#define __GNU_MP_VERSION ' "$PREFIX/include/gmp.h" 2>/dev/null | awk '{print $3}').$(grep -m1 -E '^#define __GNU_MP_VERSION_MINOR ' "$PREFIX/include/gmp.h" 2>/dev/null | awk '{print $3}').$(grep -m1 -E '^#define __GNU_MP_VERSION_PATCHLEVEL ' "$PREFIX/include/gmp.h" 2>/dev/null | awk '{print $3}')"
} > "$OUT/machine.txt"

python3 "$CODE_DIR/bench.py" report "$RESULTS" "$OUT/machine.txt" "$REPORT"
echo "== tables written to $REPORT"
