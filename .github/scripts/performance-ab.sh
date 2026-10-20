#!/bin/sh
# Two builds of GAOL with GAOL_PRESERVE_ROUNDING, one with the control register
# of the rounding direction read and written directly (the default on 64-bit
# and 32-bit ARM with GCC) and one without, measured with gaol_performance
# three times each, alternately (GAOL v5). To be run from the root of GAOL's
# sources, in the container or on the runner that has the tools:
#
#   sh .github/scripts/performance-ab.sh '<CMake options of the build before>'
#
# It writes perf-after-1.md to perf-after-3.md and perf-before-1.md to
# perf-before-3.md in the current directory, which
# .github/scripts/compare-performance.py puts side by side.
set -e
before_options=$1
for v in after before; do
  options=
  if [ "$v" = before ]; then options=$before_options; fi
  cmake -S . -B "/tmp/performance-$v" -DCMAKE_BUILD_TYPE=Release -DWITH_TESTS=ON -DGAOL_PRESERVE_ROUNDING=ON $options > /dev/null
  cmake --build "/tmp/performance-$v" -j 4 --target gaol_performance > /dev/null
done
for i in 1 2 3; do
  for v in after before; do
    "/tmp/performance-$v/tests/gaol_performance" > "perf-$v-$i.md"
  done
done
