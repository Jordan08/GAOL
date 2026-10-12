#!/usr/bin/env bash
# ITF1788 on GAOL v5: fetches ITF1788 (fetch.sh), builds and installs GAOL v5
# from this repository with CMake, once with the SSE2 intervals (the default)
# and once with the FPU ones (GAOL_SIMD=OFF), generates a program per .itl
# file of the suite (itf1788.py generate), compiles and runs the programs with
# each build, and writes the tables of doc/compare/itf1788.md (itf1788.py
# report), between its markers.
#
# Variables:
#   WORK          where everything goes (default: doc/compare/itf1788/work, ignored by git)
#   CC, CXX       the compilers of GAOL and of the programs (default: those CMake finds; g++)
#   CMAKE         CMake (default: cmake)
#   JOBS          parallel jobs of the builds and of the compilations (default: 2)
#   BUILDS        the builds to run: sse, fpu or both (default: "sse fpu")
#   CXXFLAGS_ITF  the flags of the programs, besides those of gaol.pc (default: -O2)
#   NOT_PROVIDED  operations of gaol_ieee1788 to count as not provided rather than
#                 run, separated by commas (default: none)
#   FORCE_GAOL    rebuild GAOL v5 even when it is installed in WORK already
#   REPORT        the report (default: doc/compare/itf1788.md)
#
# Copyright (c) 2026 ENSTA, France
#
# Created 2026-10-06 by Jordan NININ
set -euo pipefail
HERE="$(cd "$(dirname "$0")" && pwd)"
ROOT="$(cd "$HERE/../../.." && pwd)"
export WORK="${WORK:-$HERE/work}"
CMAKE="${CMAKE:-cmake}"
CXX="${CXX:-g++}"
JOBS="${JOBS:-2}"
BUILDS="${BUILDS:-sse fpu}"
CXXFLAGS_ITF="${CXXFLAGS_ITF:--O2}"
NOT_PROVIDED="${NOT_PROVIDED:-}"
REPORT="${REPORT:-$HERE/../itf1788.md}"

"$HERE/fetch.sh"

for b in $BUILDS; do
  case "$b" in
    sse) options="" ;;
    fpu) options="-DGAOL_SIMD=OFF" ;;
    *) echo "error: unknown build $b (sse or fpu)" >&2; exit 1 ;;
  esac
  if [ -n "${FORCE_GAOL:-}" ] || [ ! -f "$WORK/$b/prefix/lib/pkgconfig/gaol.pc" ]; then
    echo "== GAOL v5, $b build"
    CXX="$CXX" "$CMAKE" -S "$ROOT" -B "$WORK/$b/build" -DCMAKE_BUILD_TYPE=Release \
      -DCMAKE_INSTALL_PREFIX="$WORK/$b/prefix" -DCMAKE_INSTALL_LIBDIR=lib $options > "$WORK/$b-cmake.log"
    "$CMAKE" --build "$WORK/$b/build" -j"$JOBS" > "$WORK/$b-build.log"
    "$CMAKE" --install "$WORK/$b/build" > "$WORK/$b-install.log"
  fi
done

echo "== generating the programs"
mkdir -p "$WORK/programs"
python3 "$HERE/itf1788.py" generate "$WORK/ITF1788/itl" "$WORK/programs" --not-provided "$NOT_PROVIDED"

reports=()
for b in $BUILDS; do
  echo "== compiling and running the programs, $b build"
  mkdir -p "$WORK/$b/bin" "$WORK/$b/results"
  cflags="$(PKG_CONFIG_PATH="$WORK/$b/prefix/lib/pkgconfig" pkg-config --cflags gaol)"
  libs="$(PKG_CONFIG_PATH="$WORK/$b/prefix/lib/pkgconfig" pkg-config --libs gaol)"
  names="$(cd "$WORK/programs" && for f in *.cpp; do echo "${f%.cpp}"; done)"
  # shellcheck disable=SC2086
  echo "$names" | xargs -P "$JOBS" -I{} \
    "$CXX" -std=c++11 $CXXFLAGS_ITF -I"$HERE" $cflags "$WORK/programs/{}.cpp" -o "$WORK/$b/bin/{}" $libs
  for name in $names; do
    # A program that stops before its end is reported by itf1788.py report
    "$WORK/$b/bin/$name" > "$WORK/$b/results/$name.txt" || true
  done
  reports+=("$b=$WORK/$b/results")
done

echo "== report"
# GAOL v5 is named by the last commit of its sources, gaol/ and 3rd/
gaol="$(git -C "$ROOT" log -1 --format=%h -- gaol 3rd 2>/dev/null || echo unknown)"
git -C "$ROOT" diff --quiet HEAD -- gaol 3rd 2>/dev/null || gaol="$gaol with local changes"
python3 "$HERE/itf1788.py" report "$REPORT" "$WORK/programs" --gaol "$gaol" \
  --itf1788 "$(git -C "$WORK/ITF1788" rev-parse --short HEAD)" "${reports[@]}"
echo "== tables written to $REPORT"
