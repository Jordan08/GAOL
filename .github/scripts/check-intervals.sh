#!/bin/sh
# The intervals GAOL was configured to compute with (GAOL v5): NEON on 64-bit
# ARM processors, SSE2 on x86, the FPU where neither is taken (GAOL_ARM64=OFF,
# --disable-arm64, -Denable-arm64=false, or a processor that has neither). A
# build that falls back to the FPU intervals silently keeps the continuous
# integration green with the wrong code, which the tests do not tell apart:
# this fails if the configuration is not the expected one. To be run after the
# configuration, from any directory:
#
#   sh .github/scripts/check-intervals.sh <gaol_configuration.h> neon|sse2|fpu
#
header=$1
expected=$2
if [ ! -f "$header" ] || [ -z "$expected" ]; then
  echo "usage: check-intervals.sh <gaol_configuration.h> neon|sse2|fpu" >&2
  exit 2
fi
grep -H -E "^#[[:space:]]*define[[:space:]]+GAOL_USING_(SSE2|NEON)_INSTRUCTIONS[[:space:]]" "$header" || true
neon=0
sse2=0
grep -q -E "^#define GAOL_USING_NEON_INSTRUCTIONS 1" "$header" && neon=1
grep -q -E "^#define GAOL_USING_SSE2_INSTRUCTIONS 1" "$header" && sse2=1
if [ "$neon" = 1 ]; then got=neon; elif [ "$sse2" = 1 ]; then got=sse2; else got=fpu; fi
echo "The intervals: $got (expected: $expected)"
if [ "$got" != "$expected" ]; then
  echo "::error::GAOL computes the $got intervals, not the $expected ones"
  exit 1
fi
