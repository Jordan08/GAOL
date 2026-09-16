#!/usr/bin/env bash
# The whole comparison: setup.sh, run_cases.sh and run_bench.sh (see README.md)
set -euo pipefail
cd "$(dirname "$0")"
./setup.sh
./run_cases.sh
./run_bench.sh
