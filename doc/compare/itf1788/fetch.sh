#!/usr/bin/env bash
# Fetches ITF1788, the test suite of IEEE 1788-2015 (https://github.com/oheim/ITF1788),
# at the commit ITF1788_COMMIT into $WORK/ITF1788, unless it is there already.
# The suite is not copied into GAOL's repository: its .itl files are read
# where this script puts them.
#
# Copyright (c) 2026 ENSTA, France
#
# Created 2026-10-06 by Jordan NININ
set -euo pipefail
HERE="$(cd "$(dirname "$0")" && pwd)"
WORK="${WORK:-$HERE/work}"
ITF1788_REPO="${ITF1788_REPO:-https://github.com/oheim/ITF1788.git}"
# The last commit of ITF1788 (23 September 2018), the one doc/compare/itf1788.md reports
ITF1788_COMMIT="${ITF1788_COMMIT:-b6ee1e24d209c289f99a68ddc357839935799eae}"

mkdir -p "$WORK"
if [ ! -d "$WORK/ITF1788/.git" ]; then
  git clone -q "$ITF1788_REPO" "$WORK/ITF1788"
fi
if [ "$(git -C "$WORK/ITF1788" rev-parse HEAD)" != "$ITF1788_COMMIT" ]; then
  git -C "$WORK/ITF1788" fetch -q origin
  git -C "$WORK/ITF1788" checkout -q "$ITF1788_COMMIT"
fi
echo "== ITF1788 $(git -C "$WORK/ITF1788" rev-parse --short HEAD) in $WORK/ITF1788"
