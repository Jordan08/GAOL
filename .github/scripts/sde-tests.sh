#!/bin/sh
# The unit tests of GAOL run under Intel SDE (GAOL v5), which emulates a
# processor with the AVX-512 instructions (Skylake-X): the runners of GitHub
# for Windows have none, so that the AVX-512 path of +, -, *, / and sqrt
# (GAOL_AVX512_ARITHMETIC) is configured there by hand, and run nowhere else.
# SDE is 20 to 100 times as slow as the processor: the tests that do not
# compute with intervals are left out (numbers reads and writes numbers,
# core_math and u128 test CORE-MATH and the 128-bit integers), and four run at
# a time. A test that skips itself (exit status 77) passes. To be run after the
# build, from any directory:
#
#   sh .github/scripts/sde-tests.sh <sde.exe> <directory of the test programs>
#
# The output of the tests that failed is printed after the list.
if [ "$1" = "--one" ]; then
  # One test: SDE ($2) runs the program ($4), whose output and exit status go
  # into the directory $3
  name=$(basename "$4" .exe)
  timeout 1500 "$2" -skx -- "$4" > "$3/$name.log" 2>&1
  status=$?
  echo "$status" > "$3/$name.status"
  [ "$status" = 77 ] && status=0
  if [ "$status" = 0 ]; then echo "passed: $name"; else echo "FAILED ($status): $name"; fi
  exit 0
fi
sde=$1
dir=$2
if [ ! -f "$sde" ] || [ ! -d "$dir" ]; then
  echo "usage: sde-tests.sh <sde.exe> <directory of the test programs>" >&2
  exit 2
fi
logs=$(mktemp -d)
for exe in "$dir"/gaol_test_*.exe; do
  case $exe in
    */gaol_test_numbers.exe | */gaol_test_core_math.exe | */gaol_test_u128.exe) ;;
    *) echo "$exe" >> "$logs/list" ;;
  esac
done
if [ ! -s "$logs/list" ]; then
  echo "::error::no test program in $dir"
  exit 2
fi
total=$(wc -l < "$logs/list")
echo "$total test programs under SDE ($sde)"
xargs -P 4 -n 1 sh "$0" --one "$sde" "$logs" < "$logs/list"
ran=0
failed=0
while read -r exe; do
  name=$(basename "$exe" .exe)
  ran=$((ran + 1))
  status=$(cat "$logs/$name.status" 2>/dev/null || echo missing)
  if [ "$status" != 0 ] && [ "$status" != 77 ]; then
    failed=$((failed + 1))
    echo "::group::$name, exit status $status"
    cat "$logs/$name.log" 2>/dev/null
    echo "::endgroup::"
  fi
done < "$logs/list"
echo "$ran test programs, $failed failed"
if [ "$failed" != 0 ]; then
  echo "::error::$failed unit tests failed under SDE"
  exit 1
fi
