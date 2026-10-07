#!/usr/bin/env bash
# Runs the benchmark of Tang et al. (2021) on GAOL v5 (see ../tang2021.md):
# fetch.sh first, then, for each build of GAOL of VARIANTS (default "sse fpu":
# GAOL_SIMD ON and OFF), <variant> below being followed by SUFFIX (default
# empty: SUFFIX=-clang for another compiler, for instance),
#   - builds GAOL from this repository with CMake in Release and installs it
#     under $WORK/gaol-<variant> (FORCE_GAOL=1 builds it again after a change);
#   - builds the benchmark with it, in $WORK/intervals/build-<variant>, with
#     TEST_SIZE=$N inputs of each expression;
#   - runs its queries, in $JOBS processes, each on a share of the expressions
#     (INTERVALS_SHARD), each checked by check.py, and writes the tables of
#     $WORK/results/<variant>/results.md;
#   - with SPEED=1, runs its speed test as well (1000 inputs of each expression,
#     each evaluated 10000 times, by filib's C version and GAOL), into
#     $WORK/results/<variant>/time.txt and the table of speed.md.
# Nothing is sent anywhere; the network is needed by fetch.sh only.
#
# Copyright (c) 2026 ENSTA, France
#
# Created 2026-10-06 by Jordan NININ
set -euo pipefail
source "$(dirname "$0")/env.sh"
VARIANTS="${VARIANTS:-sse fpu}"
SPEED="${SPEED:-0}"
SUFFIX="${SUFFIX:-}"
CMAKE="${CMAKE:-cmake}"
PYTHON="${PYTHON:-python3}"
# The benchmark asks for CMake 3.1, which CMake 4 refuses without
# CMAKE_POLICY_VERSION_MINIMUM: in the environment, for the projects that it
# configures to download its dependencies, and when a build configures it again
export CMAKE_POLICY_VERSION_MINIMUM=3.5

"$TANG_DIR/fetch.sh"

# The expressions, in the order of the benchmark
order=$(grep -o 'RUN_QUERY([A-Za-z0-9_]*,' "$INTERVALS_DIR/tests/query/query.cpp" \
          | sed -e 's/RUN_QUERY(//' -e 's/,$//' | grep -v '^METHOD$' | paste -sd, -)

for v in $VARIANTS; do
  case "$v" in
    sse) simd=ON ;;
    fpu) simd=OFF ;;
    *) die "unknown variant $v (sse or fpu)" ;;
  esac
  name="$v$SUFFIX"
  gaol="$WORK/gaol-$name"
  if [ -f "$gaol/lib/cmake/gaol/gaolConfig.cmake" ] && [ -z "${FORCE_GAOL:-}" ]; then
    echo "== GAOL ($name): already in $gaol (FORCE_GAOL=1 to rebuild)"
  else
    echo "== GAOL ($name, GAOL_SIMD=$simd, $(git -C "$ROOT_DIR" describe --always --dirty 2>/dev/null || echo "?"))"
    "$CMAKE" -S "$ROOT_DIR" -B "$WORK/gaol-build-$name" -DCMAKE_BUILD_TYPE=Release -DGAOL_SIMD=$simd \
             -DCMAKE_C_COMPILER="$CC" -DCMAKE_CXX_COMPILER="$CXX" \
             -DCMAKE_INSTALL_PREFIX="$gaol" -DCMAKE_INSTALL_LIBDIR=lib > "$WORK/gaol-configure-$name.log" 2>&1 \
      || die "see $WORK/gaol-configure-$name.log"
    "$CMAKE" --build "$WORK/gaol-build-$name" -j"$JOBS" > "$WORK/gaol-build-$name.log" 2>&1 \
      || die "see $WORK/gaol-build-$name.log"
    "$CMAKE" --install "$WORK/gaol-build-$name" > "$WORK/gaol-install-$name.log" 2>&1
  fi

  # The benchmark, which downloads filib's C version and CLI11 when it is
  # configured
  build="$INTERVALS_DIR/build-$name"
  echo "== the benchmark with GAOL ($name), $N inputs of each expression"
  "$CMAKE" -S "$INTERVALS_DIR" -B "$build" -DCMAKE_BUILD_TYPE=Release \
           -DCMAKE_C_COMPILER="$CC" -DCMAKE_CXX_COMPILER="$CXX" \
           -DCMAKE_PREFIX_PATH="$gaol;$PREFIX" -DCMAKE_CXX_FLAGS="-DTEST_SIZE=$N" \
           > "$WORK/intervals-configure-$name.log" 2>&1 || die "see $WORK/intervals-configure-$name.log"
  "$CMAKE" --build "$build" -j"$JOBS" > "$WORK/intervals-build-$name.log" 2>&1 \
    || die "see $WORK/intervals-build-$name.log"

  results="$WORK/results/$name"
  mkdir -p "$results"
  rm -f "$results"/check-*.json "$results"/query-*.log
  echo "== the queries ($name), checked by $JOBS processes"
  # Run from the build directory, as the benchmark's scripts: it reads ../rational_datas
  pids=()
  for ((k = 0; k < JOBS; k++)); do
    (cd "$build" && INTERVALS_SHARD="$k/$JOBS" ./tests/query_bin 2> "$results/query-$k.log" \
       | "$PYTHON" -I "$TANG_DIR/check.py" check --sources "$INTERVALS_DIR" --out "$results/check-$k.json") &
    pids+=($!)
  done
  for p in "${pids[@]}"; do wait "$p" || die "the queries of $name failed (see $results/query-*.log)"; done
  "$PYTHON" -I "$TANG_DIR/check.py" report --order "$order" "$results"/check-*.json > "$results/results.md"
  echo "   $results/results.md"

  if [ "$SPEED" = 1 ]; then
    echo "== the speed test ($name)"
    (cd "$build" && ./tests/speed_bin > "$results/time.txt")
    "$PYTHON" -I "$TANG_DIR/check.py" speed "$results/time.txt" > "$results/speed.md"
    echo "   $results/speed.md"
  fi
done
