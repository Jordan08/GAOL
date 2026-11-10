#!/usr/bin/env bash
# The whole comparison: setup.sh, run_cases.sh, run_enclosure.sh, run_bench.sh
# and run_lines.sh (see README.md)
#
# Copyright (c) 2026 ENSTA, France
#
# By: Jordan NININ (created 2026-09-20)
#
# GAOL is a software distributed WITHOUT ANY WARRANTY. Read the associated
# COPYING.LIB file for information.

set -euo pipefail
cd "$(dirname "$0")"
./setup.sh
./run_cases.sh
./run_enclosure.sh
./run_bench.sh
./run_lines.sh
