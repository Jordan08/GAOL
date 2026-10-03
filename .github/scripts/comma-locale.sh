#!/bin/sh
# The locale writing a decimal comma that tests/numbers.cpp reads and writes
# numbers under (GAOL v5), in the jobs of the continuous integration. The test
# sets the first of fr_FR.UTF-8, de_DE.UTF-8... French_France.1252 the system
# has, and where it has none, says "No locale writing a decimal comma" and
# passes without a number read under such a locale, which is what the Ubuntu
# runners and the Debian images, which have none, made it do. To be run from
# the root of GAOL's sources, in the job of a runner or in a container:
#
#   sh .github/scripts/comma-locale.sh generate        before the tests
#   sh .github/scripts/comma-locale.sh check <build dir | log file>
#                                                       after them
#
# generate makes fr_FR.UTF-8 with localedef, after installing the package
# locales where the system has not got it, on Ubuntu and on Debian (whose
# locale-gen takes no name of a locale, and reads /etc/locale.gen), with sudo
# unless it runs as root, as in a container; it fails if that locale does not
# write a decimal comma afterwards. macOS and Windows have such a locale:
# their jobs only check.
#
# check fails if the output of the tests has no check of numbers under a
# locale writing a decimal comma. Given a build directory, it reads the output
# that ctest ran there, which ctest keeps, of the tests that passed too, in
# Testing/Temporary/LastTest.log, so that the tests need not be run a second
# time (numbers takes minutes with the sanitizers): check has to run right
# after the tests, before another ctest writes that file again. Given a file,
# it reads it: tests/numbers.log of make test with the autotools,
# meson-logs/testlog.txt of meson.
#
# Copyright (c) 2026 ENSTA, France
#
# Created 2026-09-29 by Jordan NININ
set -e
locale=fr_FR.UTF-8
case "$1" in
  generate)
    # As root in a container, where there may be no sudo
    as_root() { if [ "$(id -u)" = 0 ]; then "$@"; else sudo "$@"; fi; }
    dpkg -s locales > /dev/null 2>&1 || { as_root apt-get update -q; as_root apt-get install -y -q locales; }
    # The locales of the system, before and after
    locale -a
    as_root localedef -i fr_FR -f UTF-8 "$locale"
    locale -a
    if [ "$(LC_ALL=$locale locale decimal_point)" != , ]; then
      echo "$locale was not generated, or does not write a decimal comma"
      exit 1
    fi
    ;;
  check)
    log="${2:?the build directory of the tests, or the file of their output}"
    if [ -d "$log" ]; then
      log="$log/Testing/Temporary/LastTest.log"
    fi
    # The lines of the summary of the test (summary() of tests/gaol_tests.h):
    # the name of a check, which those of tests/numbers.cpp under that locale
    # end with, its number of checks and of failures. Its message where it
    # found no locale is not one.
    if ! grep -E "under a locale writing a decimal comma +[0-9]+ checks, [0-9]+ failed" "$log"; then
      echo "tests/numbers.cpp checked no number under a locale writing a decimal comma, and said:"
      grep -i "decimal comma" "$log" || echo "(nothing about it in $log)"
      exit 1
    fi
    ;;
  *)
    echo "usage: sh .github/scripts/comma-locale.sh generate | check <build dir | log file>"
    exit 2
    ;;
esac
