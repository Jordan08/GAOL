# Variables shared by fetch.sh and run.sh (sourced, not run).
#
# WORK      where the benchmark is fetched, built and run (default:
#           doc/compare/tang2021/work, ignored by git)
# PREFIX    where GMP is built when the system has no gmp.h
# N         inputs of each expression (default 100000; the paper took 1000000)
# CC, CXX   the compilers of GAOL and of the benchmark (default: gcc, g++)
# JOBS      parallel jobs of the builds, and processes of the checks (default 2)
#
# Copyright (c) 2026 ENSTA, France
#
# Created 2026-10-06 by Jordan NININ

TANG_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
ROOT_DIR="$(cd "$TANG_DIR/../../.." && pwd)"
WORK="${WORK:-$TANG_DIR/work}"
PREFIX="${PREFIX:-$WORK/prefix}"
N="${N:-100000}"
CC="${CC:-gcc}"
CXX="${CXX:-g++}"
JOBS="${JOBS:-2}"

# The benchmark: the last commit of its repository (24 May 2023), which the
# paper's web page, https://geometryprocessing.github.io/intervals/, links to
INTERVALS_REPO=https://github.com/geometryprocessing/intervals
INTERVALS_RAW=https://raw.githubusercontent.com/geometryprocessing/intervals
INTERVALS_COMMIT=0a41a99c22689892da026b25398ff975f1d741d5
INTERVALS_DIR="$WORK/intervals"

# GMP, as doc/compare/code/env.sh
GMP_VERSION=6.3.0
GMP_SHA256=a3c2b80201b89e68616f4ad30bc66aee4927c3ce50e33929ca819d5c43538898

die() { echo "error: $*" >&2; exit 1; }
