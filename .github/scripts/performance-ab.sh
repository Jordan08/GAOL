#!/bin/sh
# Two builds of GAOL, built and measured in turn in the same job (GAOL v5):
# gaol_performance and, with --lines, the long lines of
# doc/compare/code/run_lines.sh (gaol_bench), run three times with each build,
# the two alternately, so that what a shared runner does between two moments
# (its speed changes by several times from one run of the continuous
# integration to the next) reaches both. A build before and a build after,
# which differ by their sources, by CMake options, or both: the base of a pull
# request and its head (.github/workflows/performance.yml), or one source built
# with two options (the control register of the rounding direction, in
# containers.yml). It needs the tools that build GAOL (CMake, a C++ compiler),
# and with --lines Python 3 with numpy, which it installs with pip where it is
# missing.
#
#   sh .github/scripts/performance-ab.sh [options]
#
#   --before-src DIR          the sources of the build before (default: .)
#   --after-src DIR           the sources of the build after (default: .)
#   --options OPTIONS         CMake options of both builds, separated by blanks,
#                             none holding one (-DGAOL_PRESERVE_ROUNDING=ON)
#   --before-options OPTIONS  the same, for the build before only
#   --after-options OPTIONS   the same, for the build after only
#   --runs N                  runs of each build (default: 3)
#   --lines                   the long lines too: gaol_bench of each build, run
#                             on the same intervals, drawn by the bench.py of
#                             the build after (default: no)
#   --before-cache DIR        a directory that keeps the programs of the build
#                             before from one run to the next (the cache of a
#                             workflow, keyed by the commit built and the
#                             options): if it holds gaol_performance (and with
#                             --lines gaol_bench, or the note that the build has
#                             none), the build before is not made and those are
#                             run; else it is made, and the programs are copied
#                             to DIR. The build after is always made
#   --build-dir DIR           where the builds go (default: perf-build)
#   --output DIR              where the outputs go (default: .)
#
# The environment: CMAKE_GENERATOR, CMAKE_GENERATOR_PLATFORM and
# CMAKE_GENERATOR_TOOLSET are CMake's (they choose Visual Studio and its
# platform), CONFIG is the configuration (default: Release), PYTHON the Python
# of --lines (default: python3) and JOBS the parallel jobs of the builds
# (default: 4). The builds are in Release with the tests turned on (WITH_TESTS,
# which gaol_performance needs), and only the target gaol_performance and, with
# --lines, gaol_bench are built.
#
# It writes perf-before-1.md to perf-before-N.md and perf-after-1.md to
# perf-after-N.md, the outputs of gaol_performance, and with --lines
# perf-lines-before-1.csv to perf-lines-after-N.csv, those of gaol_bench, in
# the output directory, which .github/scripts/compare-performance.py puts side
# by side. The first run is of the build before first, the second of the build
# after first, and so on. A build of gaol_performance that fails, or a run that
# does, fails the script. The long lines are an addition: a build of gaol_bench
# that fails (a base that has no such target on this system, Visual C++...), or
# numpy that cannot be installed, makes the script say so, as a warning of the
# job, and leave the lines out.
set -e

before_src=.
after_src=.
options=
before_options=
after_options=
runs=3
lines=0
before_cache=
build_dir=perf-build
output=.
python=${PYTHON:-python3}
config=${CONFIG:-Release}
jobs=${JOBS:-4}
# The intervals of the long lines, and the repeats of each: run_lines.sh draws
# 10^6 and repeats 5 times; here 200 000 and 3, to keep the six runs short (a
# choice: it was not compared with 10^6). On an Apple M2 this part, with the
# build of gaol_bench in both trees, took about 5 s.
lines_n=200000
lines_repeats=3

while [ $# -gt 0 ]; do
  case $1 in
    --lines) lines=1; shift; continue ;;
    --before-src|--after-src|--options|--before-options|--after-options|--runs|--build-dir|--output|--before-cache)
      if [ $# -lt 2 ]; then echo "performance-ab.sh: $1 needs a value" >&2; exit 2; fi ;;
    *) echo "performance-ab.sh: unknown option $1 (see the comment at the top of the script)" >&2; exit 2 ;;
  esac
  case $1 in
    --before-src) before_src=$2 ;;
    --after-src) after_src=$2 ;;
    --options) options=$2 ;;
    --before-options) before_options=$2 ;;
    --after-options) after_options=$2 ;;
    --runs) runs=$2 ;;
    --build-dir) build_dir=$2 ;;
    --output) output=$2 ;;
    --before-cache) before_cache=$2 ;;
  esac
  shift 2
done

# The lines of a step in the log of GitHub Actions, folded; plain text elsewhere
group() { if [ -n "$GITHUB_ACTIONS" ]; then echo "::group::$*"; else echo "== $*"; fi; }
endgroup() { if [ -n "$GITHUB_ACTIONS" ]; then echo "::endgroup::"; fi; return 0; }
warning() { if [ -n "$GITHUB_ACTIONS" ]; then echo "::warning::$*"; else echo "warning: $*"; fi; }

# find_exe DIR NAME: the program NAME that a build in DIR made, in the directory
# of its configuration with Visual Studio, in DIR with the other generators
find_exe() {
  for exe in "$1/$config/$2.exe" "$1/$2.exe" "$1/$config/$2" "$1/$2"; do
    if [ -f "$exe" ]; then echo "$exe"; return 0; fi
  done
  return 1
}

# sources_of, options_of and exe_of, for the build before and the build after
sources_of() { if [ "$1" = before ]; then echo "$before_src"; else echo "$after_src"; fi; }
options_of() { if [ "$1" = before ]; then echo "$options $before_options"; else echo "$options $after_options"; fi; }
exe_of() { if [ "$1" = before ]; then echo "$before_exe"; else echo "$after_exe"; fi; }

mkdir -p "$build_dir" "$output"

# A variable of the generator that a workflow leaves empty is none: CMake would
# read it as an empty name
[ -n "$CMAKE_GENERATOR" ] || unset CMAKE_GENERATOR
[ -n "$CMAKE_GENERATOR_PLATFORM" ] || unset CMAKE_GENERATOR_PLATFORM
[ -n "$CMAKE_GENERATOR_TOOLSET" ] || unset CMAKE_GENERATOR_TOOLSET

# A generator of several configurations takes none on the command line of the
# configuration, only on that of the build
case "${CMAKE_GENERATOR:-}" in
  Visual*|Xcode*|*Multi-Config*) build_type= ;;
  *) build_type=-DCMAKE_BUILD_TYPE=$config ;;
esac

# The build before kept by an earlier run (--before-cache): gaol_performance is
# there, and with --lines gaol_bench too, or the note that the build had none
cache_hit=0
if [ -n "$before_cache" ] && find_exe "$before_cache" gaol_performance > /dev/null; then
  if [ "$lines" = 0 ] || find_exe "$before_cache" gaol_bench > /dev/null || [ -f "$before_cache/no-gaol_bench" ]; then
    cache_hit=1
  fi
fi

for v in before after; do
  if [ "$v" = before ] && [ "$cache_hit" = 1 ]; then
    group "GAOL before: the programs kept in $before_cache"
    ls -l "$before_cache"
    endgroup
    continue
  fi
  group "GAOL $v: $(sources_of "$v") $(options_of "$v")"
  # The options are split on their blanks, as they say
  # shellcheck disable=SC2046,SC2086
  cmake -S "$(sources_of "$v")" -B "$build_dir/$v" $build_type -DWITH_TESTS=ON $(options_of "$v")
  cmake --build "$build_dir/$v" --config "$config" -j "$jobs" --target gaol_performance
  endgroup
done
if [ "$cache_hit" = 1 ]; then
  before_exe=$(find_exe "$before_cache" gaol_performance)
else
  before_exe=$(find_exe "$build_dir/before/tests" gaol_performance) || { echo "performance-ab.sh: no gaol_performance in $build_dir/before/tests" >&2; exit 1; }
fi
after_exe=$(find_exe "$build_dir/after/tests" gaol_performance) || { echo "performance-ab.sh: no gaol_performance in $build_dir/after/tests" >&2; exit 1; }

# run_all NAME EXTENSION [ARGUMENT...]: the programs of the two builds, NAME-before-i
# and NAME-after-i of the output directory, in turn, the build before first in
# the odd runs and the build after in the even ones
run_all() {
  name=$1; extension=$2; shift 2
  i=1
  while [ "$i" -le "$runs" ]; do
    if [ $((i % 2)) = 1 ]; then order="before after"; else order="after before"; fi
    for v in $order; do
      "$(exe_of "$v")" "$@" > "$output/$name-$v-$i.$extension" || return 1
    done
    i=$((i + 1))
  done
}

group "gaol_performance, $runs runs of each build, in turn"
run_all perf md
endgroup

if [ "$lines" = 1 ]; then
  group "The long lines of doc/compare/code/run_lines.sh"
  ok=1
  for v in before after; do
    if [ "$v" = before ] && [ "$cache_hit" = 1 ]; then
      if [ -f "$before_cache/no-gaol_bench" ]; then
        warning "the build before has no gaol_bench that builds here (kept note): the long lines are left out"
        ok=0
      fi
      continue
    fi
    if ! cmake --build "$build_dir/$v" --config "$config" -j "$jobs" --target gaol_bench; then
      warning "the build $v has no gaol_bench that builds here: the long lines are left out"
      ok=0
      if [ "$v" = before ]; then no_bench_before=1; fi
    fi
  done
  if [ "$ok" = 1 ] && ! "$python" -c 'import numpy' > /dev/null 2>&1; then
    # --break-system-packages: the pip of the system Pythons of Debian, Ubuntu and
    # Homebrew refuses without it, and nothing else is installed in a runner
    "$python" -m pip install --quiet --break-system-packages numpy || "$python" -m pip install --quiet numpy || {
      warning "numpy cannot be installed: the long lines are left out"
      ok=0
    }
  fi
  if [ "$ok" = 1 ]; then
    # shellcheck disable=SC2015
    if [ "$cache_hit" = 1 ]; then before_bench_dir=$before_cache; else before_bench_dir=$build_dir/before; fi
    before_exe=$(find_exe "$before_bench_dir" gaol_bench) && after_exe=$(find_exe "$build_dir/after" gaol_bench) \
      && "$python" "$after_src/doc/compare/code/bench.py" data "$lines_n" "$build_dir/lines-data.bin" \
      && line_list=$("$python" "$after_src/doc/compare/code/bench.py" line-names) \
      && run_all perf-lines csv "$build_dir/lines-data.bin" "$lines_repeats" "$line_list" || {
      warning "the long lines could not be measured: they are left out"
      rm -f "$output"/perf-lines-*
    }
  fi
  endgroup
fi

# The programs of the build before, kept for the next run (--before-cache): only
# when this run made them, and what was measured is what is kept
if [ -n "$before_cache" ] && [ "$cache_hit" = 0 ]; then
  mkdir -p "$before_cache"
  cp "$(find_exe "$build_dir/before/tests" gaol_performance)" "$before_cache/"
  if [ "$lines" = 1 ]; then
    if exe=$(find_exe "$build_dir/before" gaol_bench); then
      cp "$exe" "$before_cache/"
    elif [ "${no_bench_before:-0}" = 1 ]; then
      echo "the build before has no gaol_bench that builds here" > "$before_cache/no-gaol_bench"
    fi
  fi
fi
