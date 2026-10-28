#!/usr/bin/env bash
# The long lines of + - * / on a 64-bit ARM processor (an Apple M2 under macOS,
# or an arm64 Linux): GAOL V5.0.0, GAOL V5.0.0 with the rounding direction
# preserved and GAOL 4.2.3, the last version of Frédéric Goualard, on the same
# intervals as run_lines.sh, and the table of the results at the end of
# doc/compare/performance.md, between its markers GENERATED ARM64.
#
# It builds the three GAOLs itself: setup.sh builds libraries that are x86
# only (libieeep1788 and the others are not measured here, and it passes
# -msse2 -msse3 and -mfma to the compilers). Needs a C++ compiler (c++ and cc,
# or CXX and CC), cmake, make, git, curl, pkg-config (or pkgconf), and Python 3
# with numpy for the intervals. bash 3.2 (macOS) is enough.
#
# N               evaluations of each line (default 1000000)
# ROUNDS          times each program is run, in turn with the others, the best
#                 time of all being kept (default 3)
# REPEATS         runs of each line in a round (default 5)
# LINE_LIST       a comma-separated list of lines (default: all of them, see
#                 bench_ops.h)
# WORK, JOBS      where everything is built (default: work/arm64 under this
#                 directory, ignored by git), and the parallel jobs (default 4)
# FORCE_GAOL      1: rebuild the two GAOLs of this repository after a change
# GAOL_COMMIT     what the description of the machine calls the commit of this
#                 checkout, when it has no .git (a copy made by git archive)
# MATHLIB_HOST    a platform name given to mathlib's configure with --host (on
#                 macOS: aarch64-unknown-linux-gnu): its configure.ac knows no
#                 macOS on ARM, and stops with "this system is not yet
#                 supported"; the name makes it define MATHLIB_AARCH64, which
#                 uses only <fenv.h>. It is only for linking GAOL 4.2.3: the
#                 lines + - * / do not call mathlib
# REPORT          the report to write (default: ../performance.md)
#
# macOS has no taskset, so the programs are not pinned to a processor: the
# machine should be on mains power and do nothing else meanwhile.
#
# Copyright (c) 2026 ENSTA, France
#
# Created 2026-10-25 by Jordan NININ
set -euo pipefail

CXX="${CXX:-c++}"
CC="${CC:-cc}"
JOBS="${JOBS:-4}"
# The fused multiply-add and SSE flags of env.sh are for x86: a 64-bit ARM
# processor has the first without a flag, and its compilers refuse the others
FMA_FLAGS=
SSE_FLAGS=
CODE_DIR="$(cd "$(dirname "$0")" && pwd)"
WORK="${WORK:-$CODE_DIR/work/arm64}"
source "$CODE_DIR/env.sh"

# mathlib's configure refuses macOS on ARM (see the header)
if [ "$(uname -s)" = Darwin ]; then MATHLIB_HOST="${MATHLIB_HOST-aarch64-unknown-linux-gnu}"; fi
N="${N:-1000000}"
ROUNDS="${ROUNDS:-3}"
REPEATS="${REPEATS:-5}"
ALL_LINES="sum16,diff16,mul8,mul8_tree,quot8,mix13,mix_tree,pos_sum16,pos_mul8,pos_quot8"
LINE_LIST="${LINE_LIST-$ALL_LINES}"
REPORT="${REPORT:-$CODE_DIR/../performance.md}"
OUT="$WORK/lines"
DATA="$WORK/bench/data-$N.bin"

case "$(uname -m)" in
  arm64|aarch64) ;;
  *) die "this is for a 64-bit ARM processor, not $(uname -m)" ;;
esac
mkdir -p "$WORK/src" "$PREFIX" "$OUT" "$WORK/bench"

# A file from a URL, its SHA-256 checked (shasum on macOS, sha256sum on Linux)
sha256_of() { if command -v sha256sum >/dev/null; then sha256sum "$1" | cut -d' ' -f1; else shasum -a 256 "$1" | cut -d' ' -f1; fi; }
fetch() {
  local url="$1" file="$2" sha256="$3"
  [ -f "$file" ] || curl -fsSL "$url" -o "$file"
  [ "$(sha256_of "$file")" = "$sha256" ] || die "$file: wrong SHA-256"
}

## GAOL, from this repository, and again with the rounding direction preserved
## (GAOL_PRESERVE_ROUNDING=ON), each in a prefix of its own. A 64-bit ARM
## processor needs no flag for NEON nor for the fused multiply-add
if [ -f "$PREFIX/include/gaol/gaol.h" ] && [ -z "${FORCE_GAOL:-}" ]; then
  echo "== GAOL: already in $PREFIX (FORCE_GAOL=1 to rebuild)"
else
  echo "== GAOL"
  cmake -S "$ROOT_DIR" -B "$WORK/gaol-build" -DCMAKE_BUILD_TYPE=Release \
        -DCMAKE_INSTALL_PREFIX="$PREFIX" -DCMAKE_INSTALL_LIBDIR=lib \
        -DCMAKE_CXX_COMPILER="$CXX" -DCMAKE_C_COMPILER="$CC" > "$WORK/gaol-configure.log"
  cmake --build "$WORK/gaol-build" -j"$JOBS" > "$WORK/gaol-build.log"
  cmake --install "$WORK/gaol-build" > "$WORK/gaol-install.log"
fi
if [ -f "$GAOL_PRESERVE_PREFIX/include/gaol/gaol.h" ] && [ -z "${FORCE_GAOL:-}" ]; then
  echo "== GAOL, rounding direction preserved: already in $GAOL_PRESERVE_PREFIX (FORCE_GAOL=1 to rebuild)"
else
  echo "== GAOL, rounding direction preserved"
  cmake -S "$ROOT_DIR" -B "$WORK/gaol-preserve-build" -DCMAKE_BUILD_TYPE=Release \
        -DCMAKE_INSTALL_PREFIX="$GAOL_PRESERVE_PREFIX" -DCMAKE_INSTALL_LIBDIR=lib \
        -DCMAKE_CXX_COMPILER="$CXX" -DCMAKE_C_COMPILER="$CC" -DGAOL_PRESERVE_ROUNDING=ON \
        > "$WORK/gaol-preserve-configure.log"
  cmake --build "$WORK/gaol-preserve-build" -j"$JOBS" > "$WORK/gaol-preserve-build.log"
  cmake --install "$WORK/gaol-preserve-build" > "$WORK/gaol-preserve-install.log"
fi
for prefix in "$PREFIX" "$GAOL_PRESERVE_PREFIX"; do
  grep -q "^#define GAOL_USING_NEON_INSTRUCTIONS 1" "$prefix/include/gaol/gaol_configuration.h" \
    || die "$prefix: GAOL did not take the NEON intervals"
done
grep -q "^#define GAOL_PRESERVE_ROUNDING 1" "$GAOL_PRESERVE_PREFIX/include/gaol/gaol_configuration.h" \
  || die "$GAOL_PRESERVE_PREFIX: GAOL does not preserve the rounding direction"

## The last GAOL of Frédéric Goualard with mathlib, built with their configure
## as setup.sh does, without the x86 flags, static, the rounding direction not
## preserved, its verbose mode off
goualard_build=
if [ "$(uname -s)" = Darwin ]; then goualard_build=--build=arm-apple-darwin; fi
if [ -f "$GAOL_GOUALARD_PREFIX/include/gaol/gaol.h" ]; then
  echo "== GAOL $GAOL_GOUALARD_VERSION of Frédéric Goualard: already in $GAOL_GOUALARD_PREFIX"
else
  cd "$WORK/src"
  echo "== mathlib $MATHLIB_VERSION"
  fetch "$MATHLIB_URL" "mathlib-$MATHLIB_VERSION.tar.gz" "$MATHLIB_SHA256"
  rm -rf "mathlib-$MATHLIB_VERSION" && tar xzf "mathlib-$MATHLIB_VERSION.tar.gz"
  (cd "mathlib-$MATHLIB_VERSION" &&
   ./configure --prefix="$GAOL_GOUALARD_PREFIX" --libdir="$GAOL_GOUALARD_PREFIX/lib" --disable-shared \
               ${MATHLIB_HOST:+--host="$MATHLIB_HOST"} \
               CC="$CC" CFLAGS="-O3 -ffp-contract=off" > configure.log &&
   make -j"$JOBS" > make.log && make install > install.log)

  echo "== GAOL $GAOL_GOUALARD_VERSION of Frédéric Goualard (${GAOL_GOUALARD_COMMIT:0:7})"
  if [ ! -d gaol-goualard ]; then
    git clone -q "$GAOL_GOUALARD_REPO" gaol-goualard
  fi
  git -C gaol-goualard checkout -q "$GAOL_GOUALARD_COMMIT"
  # Its configure.ac knows arm-apple-* (ARM_MACOSX), but the configure it
  # commits (2020) does not, and the config.guess of automake 1.19 says
  # aarch64-apple-darwin: configure is generated again, and told arm-apple-darwin
  if [ "$(uname -s)" = Darwin ]; then
    (cd gaol-goualard && LIBTOOLIZE=glibtoolize autoreconf -fi > autoreconf.log 2>&1)
  fi
  (cd gaol-goualard &&
   ./configure $goualard_build --prefix="$GAOL_GOUALARD_PREFIX" --libdir="$GAOL_GOUALARD_PREFIX/lib" --disable-shared \
               --disable-preserve-rounding --disable-verbose-mode \
               --with-mathlib-include="$GAOL_GOUALARD_PREFIX/include" \
               --with-mathlib-lib="$GAOL_GOUALARD_PREFIX/lib" \
               CC="$CC" CXX="$CXX" CFLAGS="-O3 -ffp-contract=off" \
               CXXFLAGS="-O3 -DNDEBUG -std=c++11 $IA_CXXFLAGS -fvisibility-inlines-hidden -fvisibility=hidden -funroll-loops -fomit-frame-pointer" \
               > configure.log &&
   make -j"$JOBS" > make.log && make install > install.log)
fi

## The intervals, drawn once (bench.py data: the same as run_bench.sh's)
if [ ! -f "$DATA" ]; then
  echo "== drawing $N intervals of each kind"
  python3 "$CODE_DIR/bench.py" data "$N" "$DATA"
fi

echo "== compiling"
# The flags and the library of a GAOL of this repository, from the gaol.pc it installed
pc() { PKG_CONFIG_PATH="$1/lib/pkgconfig" pkg-config "$2" gaol; }
$CXX -std=c++11 $CXXFLAGS_BENCH $(pc "$PREFIX" --cflags) -DGAOL_BENCH_NAME='"gaol5"' -I"$CODE_DIR" \
     "$CODE_DIR/bench_gaol.cpp" -o "$OUT/bench_gaol5" $(pc "$PREFIX" --libs)
$CXX -std=c++11 $CXXFLAGS_BENCH $(pc "$GAOL_PRESERVE_PREFIX" --cflags) -DGAOL_BENCH_NAME='"gaol5p"' -I"$CODE_DIR" \
     "$CODE_DIR/bench_gaol.cpp" -o "$OUT/bench_gaol5p" $(pc "$GAOL_PRESERVE_PREFIX" --libs)
$CXX -std=c++11 $CXXFLAGS_BENCH $(gaol_goualard_cflags) -I"$CODE_DIR" "$CODE_DIR/bench_gaol.cpp" \
     -o "$OUT/bench_gaol" $(gaol_goualard_libs)

RESULTS="$OUT/results.csv"
: > "$RESULTS"
for round in $(seq "$ROUNDS"); do
  for lib in gaol5 gaol5p gaol; do
    echo "== round $round/$ROUNDS: $lib"
    "$OUT/bench_$lib" "$DATA" "$REPEATS" "$LINE_LIST" | tee -a "$RESULTS"
  done
done

## What the results were measured on
processor() {
  if [ "$(uname -s)" = Darwin ]; then sysctl -n machdep.cpu.brand_string
  else grep -m1 -E 'model name|Model|Hardware' /proc/cpuinfo | cut -d: -f2 | sed 's/^ *//'; fi
}
system() {
  if [ "$(uname -s)" = Darwin ]; then echo "$(uname -sr), macOS $(sw_vers -productVersion)"
  else echo "$(uname -sr), $(. /etc/os-release 2>/dev/null && echo "$PRETTY_NAME")"; fi
}
power() {
  if command -v pmset >/dev/null; then pmset -g batt | head -1 | sed -e "s/Now drawing from //" -e "s/'//g"
  else echo "?"; fi
}
gaol_commit="${GAOL_COMMIT:-$(git -C "$ROOT_DIR" describe --always 2>/dev/null || echo "?")}"
{
  echo "Date:            $(date '+%Y-%m-%d')"
  echo "Processor:       $(processor) (not pinned to a processor: no taskset on macOS), power: $(power)"
  echo "System:          $(system)"
  echo "C++ compiler:    $($CXX --version | head -1)"
  echo "C++ flags:       -std=c++11 $CXXFLAGS_BENCH (GAOL: $(pc "$PREFIX" --cflags | sed -e "s# *-I[^ ]*##g" -e "s#^ *##"); GAOL 4.2.3: $IA_CXXFLAGS)"
  echo "GAOL V5.0.0:     the branch of this checkout ($gaol_commit), CMake Release, NEON intervals, CORE-MATH of 3rd/math-core compiled into the library, the rounding direction not preserved"
  echo "GAOL V5.0.0 preserved: the same, built with GAOL_PRESERVE_ROUNDING=ON, the rounding direction restored after each operation (the FPCR register, read and written by each operation)"
  goualard_note="its configure"
  [ -n "${goualard_build:-}" ] && goualard_note="its configure generated again by autoreconf and told $goualard_build (the one it commits does not know macOS on ARM)"
  mathlib_note="mathlib $MATHLIB_VERSION"
  [ -n "${MATHLIB_HOST:-}" ] && mathlib_note="$mathlib_note configured with --host=$MATHLIB_HOST (its configure knows no macOS on ARM; no line here calls it)"
  # What the intervals of GAOL 4.2.3 are: it has no NEON, and SSE2 only on x86
  goualard_intervals="the FPU intervals (it has no instructions of 64-bit ARM to compute them with)"
  grep -q "^#define USING_SSE2_INSTRUCTIONS" "$GAOL_GOUALARD_PREFIX/include/gaol/gaol_configuration.h" 2>/dev/null \
    && goualard_intervals="the SSE2 intervals"
  echo "GAOL $GAOL_GOUALARD_VERSION:      the last version of Frédéric Goualard (${GAOL_GOUALARD_COMMIT:0:7} of $GAOL_GOUALARD_REPO), $goualard_note, the rounding direction not preserved, $goualard_intervals, $mathlib_note"
} > "$OUT/machine.txt"

python3 "$CODE_DIR/bench.py" arm64-report "$RESULTS" "$OUT/machine.txt" "$REPORT"
echo "== table written to $REPORT"
# The results of a whole run
if [ "$LINE_LIST" = "$ALL_LINES" ]; then
  cp "$RESULTS" "$CODE_DIR/results_lines_arm64.csv"
  cp "$OUT/machine.txt" "$CODE_DIR/machine_lines_arm64.txt"
  echo "== results kept in $CODE_DIR/results_lines_arm64.csv and machine_lines_arm64.txt"
fi
