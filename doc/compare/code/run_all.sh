#!/usr/bin/env bash
# The whole comparison: setup.sh, run_cases.sh and run_bench.sh (see README.md)
#
# Copyright (c) 2026 ENSTA, France
#
# Created 2026-09-20 by Jordan NININ
set -euo pipefail
cd "$(dirname "$0")"
./setup.sh
./run_cases.sh
./run_bench.sh
