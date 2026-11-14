#!/bin/sh
# logs.sh WORK STATUS: when STATUS is not 0, prints the end of the logs of the
# three builds configured in WORK by audit.sh, and in WORK-<variant> for each
# set of options it audited, then exits with STATUS
#
# Copyright (c) 2026 ENSTA, France
#
# By: Jordan NININ (created 2026-09-20)
#
# GAOL is a software distributed WITHOUT ANY WARRANTY. Read the associated
# COPYING.LIB file for information.

work=$1
status=$2
if [ "$status" != 0 ]; then
  for dir in "$work" "$work"-*; do
    [ -d "$dir" ] || continue
    for build in cmake autotools meson; do
      echo "==== $(basename "$dir"), $build (status $(cat "$dir/$build.status" 2>/dev/null)), the end of its log:"
      tail -30 "$dir/$build.log" 2>/dev/null
    done
  done
fi
exit "$status"
