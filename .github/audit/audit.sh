#!/bin/sh
# audit.sh LABEL CC CXX SOURCES UNUSED OUT [MESON]: configures GAOL with CMake, autotools and meson, and
# writes OUT/LABEL.json (see probe_build.py). The fifth argument named where a mathlib was installed and is
# ignored: GAOL bounds its elementary functions with the CORE-MATH of 3rd/math-core, compiled into the library
label=$1; cc=$2; cxx=$3; src=$(cd "$4" && pwd); out=$6; meson=${7:-meson}
cmake_mathlib=""; configure_mathlib=""; meson_mathlib=""
here=$(cd "$(dirname "$0")" && pwd)
mkdir -p "$out"; out=$(cd "$out" && pwd); work=$out/$label; rm -rf "$work"; mkdir -p "$work"
python3 "$here/make_probe.py" > "$work/probe.cpp"
cmake -S "$src" -B "$work/cmake" -DCMAKE_BUILD_TYPE=Release -DCMAKE_EXPORT_COMPILE_COMMANDS=ON -DGAOL_BUILD_TESTS=OFF \
  $cmake_mathlib -DCMAKE_C_COMPILER="$cc" -DCMAKE_CXX_COMPILER="$cxx" > "$work/cmake.log" 2>&1
echo $? > "$work/cmake.status"
mkdir -p "$work/autotools"
(cd "$src" && tar --exclude=.git -cf - .) | (cd "$work/autotools" && tar -xf -)
(cd "$work/autotools" && find . -exec touch -h -t 202001010000 {} + && CC="$cc" CXX="$cxx" sh ./configure \
  $configure_mathlib > ../autotools.log 2>&1 \
  && make -n -C gaol gaol_interval.lo > ../autotools.make-n.txt 2>&1)
echo $? > "$work/autotools.status"
CC="$cc" CXX="$cxx" $meson setup "$work/meson" "$src" $meson_mathlib > "$work/meson.log" 2>&1
echo $? > "$work/meson.status"
python3 "$here/probe_build.py" "$work" "$label" > "$out/$label.json"
