#!/bin/sh
# The locale writing a decimal comma that tests/numbers.cpp reads and writes
# numbers under (GAOL v5), on the Ubuntu runners of the continuous integration.
# The test sets the first of fr_FR.UTF-8, de_DE.UTF-8... the system has, and
# where it has none, says "No locale writing a decimal comma" and passes
# without a number read under such a locale, which is what the runners, which
# have none, made it do. To be run from the root of GAOL's sources, in the job
# of a runner:
#
#   sh .github/scripts/comma-locale.sh generate            before the tests
#   sh .github/scripts/comma-locale.sh check <build dir>   after the tests
#
# generate makes fr_FR.UTF-8 with locale-gen, after installing the package
# locales where the image has not got it, and fails if that locale does not
# write a decimal comma afterwards: locale-gen succeeds, and says nothing,
# where it generated nothing. It is meant for Ubuntu, whose locale-gen takes
# the name of a locale (Debian's takes none, and reads /etc/locale.gen).
#
# check fails if the output of the tests that ctest ran in <build dir> has no
# check of numbers under a locale writing a decimal comma. ctest keeps that
# output, of the tests that passed too, in Testing/Temporary/LastTest.log, so
# that the tests need not be run a second time (numbers takes more than two
# minutes with the sanitizers); check has to run right after the tests, before
# another ctest writes that file again.
#
# Copyright (c) 2026 ENSTA, France
#
# Created 2026-09-29 by Jordan NININ
set -e
locale=fr_FR.UTF-8
case "$1" in
  generate)
    dpkg -s locales > /dev/null 2>&1 || { sudo apt-get update -q; sudo apt-get install -y -q locales; }
    # The locales of the runner, before and after
    locale -a
    sudo locale-gen "$locale"
    locale -a
    if [ "$(LC_ALL=$locale locale decimal_point)" != , ]; then
      echo "$locale was not generated, or does not write a decimal comma"
      exit 1
    fi
    ;;
  check)
    log="${2:?the build directory of the tests}/Testing/Temporary/LastTest.log"
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
    echo "usage: sh .github/scripts/comma-locale.sh generate | check <build dir>"
    exit 2
    ;;
esac
