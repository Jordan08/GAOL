#!/bin/sh
# Copyright (c) 2026 ENSTA, France
#
# Created 2026-09-20 by Jordan NININ
#
# logs.sh WORK STATUS: when STATUS is not 0, prints the end of the logs of the
# three builds configured in WORK by audit.sh, then exits with STATUS
work=$1
status=$2
if [ "$status" != 0 ]; then
  for build in cmake autotools meson; do
    echo "==== $build (status $(cat "$work/$build.status" 2>/dev/null)), the end of its log:"
    tail -30 "$work/$build.log" 2>/dev/null
  done
fi
exit "$status"
