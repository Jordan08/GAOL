#!/bin/sh
# audit.sh LABEL CC CXX SOURCES OUT [MESON]: configures GAOL with CMake, autotools and meson, and
# writes OUT/LABEL.json (see probe_build.py), first with the default options, then with each set of
# options of VARIANTS below, in OUT/LABEL-<variant>.json (GAOL v5: the default options only)
#
# GAOL_AUDIT_VARIANTS='' audits the default options alone; a list of names of VARIANTS audits those
#
# Copyright (c) 2026 ENSTA, France
#
# By: Jordan NININ (created 2026-09-20)
#
# GAOL is a software distributed WITHOUT ANY WARRANTY. Read the associated
# COPYING.LIB file for information.

label=$1; cc=$2; cxx=$3; src=$(cd "$4" && pwd); out=$5; meson=${6:-meson}
here=$(cd "$(dirname "$0")" && pwd)
mkdir -p "$out"; out=$(cd "$out" && pwd)

# The sets of options that change the configuration: the options of the user
# (GAOL_PRESERVE_ROUNDING, GAOL_SIMD) and the settings of the developers (the fused multiply-add
# instructions, the AVX-512 path, the exceptions, the verbose mode), which the three builds spell
# alike (doc/building.md). Each build configures in a minute or less, and compiles nothing
VARIANTS=${GAOL_AUDIT_VARIANTS-"preserve preserve-no-avx512 no-simd no-fma no-exceptions-verbose"}

# options VARIANT: the options of CMake, of configure and of meson for that set
options() {
  case $1 in
    '') cmake_opts=''; configure_opts=''; meson_opts='' ;;
    preserve)
      cmake_opts='-DGAOL_PRESERVE_ROUNDING=ON'; configure_opts='--enable-preserve-rounding'
      meson_opts='-Denable-preserve-rounding=true' ;;
    preserve-no-avx512)
      cmake_opts='-DGAOL_PRESERVE_ROUNDING=ON -DGAOL_PREFER_AVX512=OFF'
      configure_opts='--enable-preserve-rounding --disable-prefer-avx512'
      meson_opts='-Denable-preserve-rounding=true -Denable-prefer-avx512=false' ;;
    no-simd)
      cmake_opts='-DGAOL_SIMD=OFF'; configure_opts='--disable-simd'; meson_opts='-Denable-simd=false' ;;
    no-fma)
      cmake_opts='-DGAOL_FMA=OFF'; configure_opts='--disable-fma'; meson_opts='-Denable-fma=false' ;;
    no-exceptions-verbose)
      cmake_opts='-DGAOL_EXCEPTIONS_ENABLED=OFF -DGAOL_VERBOSE_MODE=ON'
      configure_opts='--disable-exceptions --enable-verbose-mode'
      meson_opts='-Denable-exception=false -Denable-verbose-mode=true' ;;
    *) echo "audit.sh: no set of options named $1" >&2; exit 2 ;;
  esac
}

# audit LABEL VARIANT: configures the three builds with the options of VARIANT in OUT/LABEL, and
# writes OUT/LABEL.json
audit() {
  name=$1; options "$2"
  work=$out/$name; rm -rf "$work"; mkdir -p "$work"
  python3 "$here/make_probe.py" > "$work/probe.cpp"
  python3 "$here/make_probe.py" c > "$work/probe.c"
  # shellcheck disable=SC2086
  cmake -S "$src" -B "$work/cmake" -DCMAKE_BUILD_TYPE=Release -DCMAKE_EXPORT_COMPILE_COMMANDS=ON \
    -DCMAKE_C_COMPILER="$cc" -DCMAKE_CXX_COMPILER="$cxx" $cmake_opts > "$work/cmake.log" 2>&1
  echo $? > "$work/cmake.status"
  mkdir -p "$work/autotools"
  (cd "$src" && tar --exclude=.git -cf - .) | (cd "$work/autotools" && tar -xf -)
  # shellcheck disable=SC2086
  (cd "$work/autotools" && find . -exec touch -h -t 202001010000 {} + && CC="$cc" CXX="$cxx" sh ./configure $configure_opts \
    > ../autotools.log 2>&1 \
    && make -n -C gaol gaol_interval.lo > ../autotools.make-n.txt 2>&1 \
    && make -n -C 3rd > ../autotools.make-n-c.txt 2>&1)
  echo $? > "$work/autotools.status"
  # shellcheck disable=SC2086
  CC="$cc" CXX="$cxx" $meson setup "$work/meson" "$src" $meson_opts > "$work/meson.log" 2>&1
  echo $? > "$work/meson.status"
  python3 "$here/probe_build.py" "$work" "$name" > "$out/$name.json"
}

audit "$label" ''
for variant in $VARIANTS; do
  audit "$label-$variant" "$variant"
done
