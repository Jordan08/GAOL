#!/bin/sh
# audit.sh LABEL CC CXX SOURCES MATHLIB_PREFIX OUT [MESON]: configures GAOL with CMake, autotools and meson, and
# writes OUT/LABEL.json (see probe_build.py)
label=$1; cc=$2; cxx=$3; src=$(cd "$4" && pwd); ml=$5; out=$6; meson=${7:-meson}
here=$(cd "$(dirname "$0")" && pwd)
mkdir -p "$out"; out=$(cd "$out" && pwd); work=$out/$label; rm -rf "$work"; mkdir -p "$work"
python3 "$here/make_probe.py" > "$work/probe.cpp"
cmake -S "$src" -B "$work/cmake" -DCMAKE_BUILD_TYPE=Release -DCMAKE_EXPORT_COMPILE_COMMANDS=ON -DGAOL_BUILD_TESTS=OFF \
  -DGAOL_BUILD_MATHLIB=OFF -DMATHLIB_DIR="$ml" -DCMAKE_C_COMPILER="$cc" -DCMAKE_CXX_COMPILER="$cxx" > "$work/cmake.log" 2>&1
echo $? > "$work/cmake.status"
mkdir -p "$work/autotools"
(cd "$src" && tar --exclude=.git -cf - .) | (cd "$work/autotools" && tar -xf -)
(cd "$work/autotools" && find . -exec touch -h -t 202001010000 {} + && CC="$cc" CXX="$cxx" sh ./configure \
  --with-mathlib-include="$ml/include" --with-mathlib-lib="$ml/lib" > ../autotools.log 2>&1 \
  && make -n -C gaol gaol_interval.lo > ../autotools.make-n.txt 2>&1)
echo $? > "$work/autotools.status"
CC="$cc" CXX="$cxx" CPATH="$ml/include" LIBRARY_PATH="$ml/lib" $meson setup "$work/meson" "$src" -Dwith-mathlib=apmathlib > "$work/meson.log" 2>&1
echo $? > "$work/meson.status"
# gaol/gaol_double_op.h, which gaol/gaol includes, is made when building
[ "$(cat "$work/meson.status")" = 0 ] && $meson compile -C "$work/meson" gen-math-header >> "$work/meson.log" 2>&1
python3 "$here/probe_build.py" "$work" "$label" > "$out/$label.json"
