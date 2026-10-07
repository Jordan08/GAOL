#!/usr/bin/env bash
# Fetches the benchmark of Tang et al. (2021), "A Cross-Platform Benchmark for
# Interval Computation Libraries" (see ../tang2021.md), into $WORK/intervals,
# at the commit INTERVALS_COMMIT of https://github.com/geometryprocessing/intervals,
# and adds GAOL to it with gaol.patch. Nothing of the benchmark is kept in
# GAOL's repository.
#   - the repository is cloned without its directory rational_datas (1.5 GB of
#     compressed inputs);
#   - of each file of recorded inputs of rational_datas (the 28 first
#     expressions: the paper's inputs), only the first N inputs (default 100000
#     of the 1000000) are downloaded, decompressed on the fly; FPBench's 104
#     expressions have no recorded inputs, and the benchmark draws them;
#   - GMP, which the benchmark needs, is built under $WORK/prefix when the
#     system has no gmp.h (the version doc/compare/code/env.sh takes).
# Each step is skipped when its result is already there.
#
# Copyright (c) 2026 ENSTA, France
#
# Created 2026-10-06 by Jordan NININ
set -euo pipefail
source "$(dirname "$0")/env.sh"

mkdir -p "$WORK"

## The benchmark, at the pinned commit, without rational_datas
if [ -d "$INTERVALS_DIR/.git" ]; then
  echo "== benchmark: already in $INTERVALS_DIR"
else
  echo "== benchmark ${INTERVALS_COMMIT:0:7} (without rational_datas)"
  git clone -q --filter=blob:none --no-checkout "$INTERVALS_REPO" "$INTERVALS_DIR"
  git -C "$INTERVALS_DIR" config core.sparseCheckout true
  printf '/*\n!/rational_datas/\n' > "$INTERVALS_DIR/.git/info/sparse-checkout"
  git -C "$INTERVALS_DIR" checkout -q "$INTERVALS_COMMIT"
fi
[ "$(git -C "$INTERVALS_DIR" rev-parse HEAD)" = "$INTERVALS_COMMIT" ] \
  || die "$INTERVALS_DIR is not at $INTERVALS_COMMIT"

## GAOL's back end
if git -C "$INTERVALS_DIR" apply --reverse --check "$TANG_DIR/gaol.patch" 2>/dev/null; then
  echo "== gaol.patch: already applied"
else
  echo "== gaol.patch"
  git -C "$INTERVALS_DIR" apply "$TANG_DIR/gaol.patch"
fi

## The first N recorded inputs of the 28 first expressions: one rational a line,
## VARIABLE_COUNT lines an input
variable_count() {
  case "$1" in
    addition|subtraction|multiplication|division) echo 2 ;;
    square_root|exponential|sin|cos) echo 1 ;;
    expr1|expr2|expr3) echo 10 ;;
    *) sed -n "s/^const int $1_variable_count = \([0-9]*\);.*/\1/p" "$INTERVALS_DIR/src/methods.hpp" ;;
  esac
}
mkdir -p "$INTERVALS_DIR/rational_datas"
for e in addition subtraction multiplication division square_root exponential sin cos \
         expr1 expr2 expr3 expr4 expr5 expr6 expr7 expr8 expr9 expr10 \
         extra_function1 extra_function2 extra_function3 extra_function4 extra_function5 \
         extra_function6 extra_function7 extra_function8 extra_function9 extra_function10; do
  k=$(variable_count "$e")
  [ -n "$k" ] || die "no variable count for $e"
  lines=$((N * k))
  file="$INTERVALS_DIR/rational_datas/${e}_used_rationals.txt"
  if [ -f "$file" ] && [ "$(wc -l < "$file")" -ge "$lines" ]; then
    continue
  fi
  echo "== the first $N inputs of $e ($lines lines)"
  # head stops the download once it has its lines: curl and gunzip then end
  # on a broken pipe, which is why only the count of the lines is checked
  (set +o pipefail
   curl -fsSL "$INTERVALS_RAW/$INTERVALS_COMMIT/rational_datas/${e}_used_rationals.txt.gz" 2>/dev/null \
     | gunzip -c 2>/dev/null | head -n "$lines" > "$file.part")
  [ "$(wc -l < "$file.part")" -eq "$lines" ] || die "$e: fewer than $lines recorded inputs"
  mv "$file.part" "$file"
done

## GMP
if [ -f "$PREFIX/include/gmp.h" ]; then
  echo "== GMP: already in $PREFIX"
elif echo "#include <gmp.h>" | "$CC" -E -x c - > /dev/null 2>&1; then
  echo "== GMP: the system's"
else
  echo "== GMP $GMP_VERSION"
  mkdir -p "$WORK/src"
  (cd "$WORK/src" &&
   { [ -f "gmp-$GMP_VERSION.tar.xz" ] || curl -fsSL -o "gmp-$GMP_VERSION.tar.xz" "https://gmplib.org/download/gmp/gmp-$GMP_VERSION.tar.xz"; } &&
   echo "$GMP_SHA256  gmp-$GMP_VERSION.tar.xz" | sha256sum -c --quiet - &&
   tar xf "gmp-$GMP_VERSION.tar.xz" &&
   cd "gmp-$GMP_VERSION" &&
   ./configure --prefix="$PREFIX" --libdir="$PREFIX/lib" --disable-shared --enable-static CC="$CC" > configure.log &&
   make -j"$JOBS" > make.log && make install > install.log) || die "GMP did not build (see $WORK/src/gmp-$GMP_VERSION)"
fi

echo "== done: the benchmark is in $INTERVALS_DIR"
