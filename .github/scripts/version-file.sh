#!/bin/sh
# Checks how configure or meson reads the file VERSION.txt, on a copy of the
# sources in a temporary directory whose VERSION.txt is written in several
# ways. To be run from the root of GAOL's sources, where no build has been
# configured in place (configure refuses to configure a copy of such a tree):
#
#   sh .github/scripts/version-file.sh configure|meson
#
# A UTF-8 byte order mark at the start of the file (EF BB BF, which some
# editors of Windows write), the line ends of Windows (CR LF), blanks and empty
# lines around the version have to be ignored, and any other text, a file of
# UTF-16 characters (which Windows PowerShell 5 writes for a redirection)
# included, has to be refused with a message giving the first bytes of the file
# in hexadecimal, where the mark, invisible, made the build refuse "5.0.0" as
# VERSION.txt holding "5.0.0". The CMake build reads the file with
# cmake/gaol_version.cmake, which the test version_file of tests/ checks (ctest
# -R version_file).
#
# Copyright (c) 2026 ENSTA, France
#
# Created 2026-09-29 by Jordan NININ
set -e
build=$1
case "$build" in
  configure|meson) ;;
  *) echo "usage: sh .github/scripts/version-file.sh configure|meson"; exit 2 ;;
esac
root=$(pwd)
work=$(mktemp -d "${TMPDIR:-/tmp}/gaol-version.XXXXXX")
trap 'rm -rf "$work"' EXIT INT TERM
mkdir "$work/src"
# -h: README is a symbolic link to README.md, which tar of MSYS2 cannot create when it
# extracts it ("Cannot create symlink"): the copy holds the file itself
tar -c -h -f - --exclude=.git --exclude=autom4te.cache . | tar -x -f - -C "$work/src"
cd "$work"

# The version is not the one of GAOL: it is the file the build reads, not a
# default, that gives the macros of gaol_configuration.h (configure warns that
# it was generated for another version, as it does for any change of VERSION.txt
# before it is generated again)
status=0
fail() {
  echo "FAILED: $1"
  status=1
}

# Configures a build directory with the sources in src, whose output goes to
# log; the status of the build is the status of the function
run() {
  rm -rf build
  mkdir build
  if [ "$build" = configure ]; then
    (cd build && ../src/configure) > log 2>&1
  else
    meson setup build src > log 2>&1
  fi
}

# accepted <name> <content of VERSION.txt, as printf writes it>: the version
# has to be 7.3.11, in the macros GAOL_VERSION, GAOL_MAJOR_VERSION, ... of
# gaol/gaol_configuration.h, which both builds write
accepted() {
  printf "$2" > src/VERSION.txt
  if ! run; then
    fail "$1: refused"; grep 'VERSION.txt' log | head -3
  elif ! grep -q '^#define GAOL_VERSION "7\.3\.11"$' build/gaol/gaol_configuration.h ||
       ! grep -q '^#define GAOL_MAJOR_VERSION 7$' build/gaol/gaol_configuration.h ||
       ! grep -q '^#define GAOL_MINOR_VERSION 3$' build/gaol/gaol_configuration.h ||
       ! grep -q '^#define GAOL_MICRO_VERSION 11$' build/gaol/gaol_configuration.h; then
    fail "$1: not the version 7.3.11"; grep 'define GAOL_.*VERSION' build/gaol/gaol_configuration.h
  else
    echo "ok: $1"
  fi
}

# refused <name> <content of VERSION.txt> <its first bytes in hexadecimal>: the
# build has to stop, and its message to give the bytes
refused() {
  printf "$2" > src/VERSION.txt
  if run; then
    fail "$1: accepted"
  elif ! LC_ALL=C grep -a -q "VERSION.txt holds .* (bytes in hexadecimal: $3), where it should hold the version of GAOL" log; then
    fail "$1: the message does not give the bytes $3"; grep -a 'VERSION.txt' log | head -3
  else
    echo "ok: $1"
  fi
}

accepted "a line" '7.3.11\n'
accepted "no line end" '7.3.11'
accepted "a mark, a line" '\357\273\2777.3.11\n'
accepted "a mark, no line end" '\357\273\2777.3.11'
accepted "CR LF" '7.3.11\r\n'
accepted "a mark, CR LF" '\357\273\2777.3.11\r\n'
accepted "a mark, empty lines and blanks" '\357\273\277\r\n  7.3.11 \t\r\n\r\n'
# The bytes: 37 2e 33 2e 31 31 is 7.3.11, 78 is x, 0a the line end
refused "a mark alone" '\357\273\277' 'ef bb bf'
refused "two marks" '\357\273\277\357\273\2777.3.11' 'ef bb bf ef bb bf 37 2e 33 2e 31 31'
refused "a mark inside" '7.3.\357\273\27711\n' '37 2e 33 2e ef bb bf 31 31 0a'
refused "a letter" '7.3.x\n' '37 2e 33 2e 78 0a'
refused "a zero-width space" '7.3.11\342\200\213\n' '37 2e 33 2e 31 31 e2 80 8b 0a'
# 7.3.11 in UTF-16, little-endian, with its byte order mark, as Windows
# PowerShell 5 writes for a redirection: the NUL bytes are not for the message
refused "UTF-16" '\377\3767\000.\0003\000.\0001\0001\000\n\000' 'ff fe 37 00 2e 00 33 00 2e 00 31 00 31 00 0a 00'
refused "a leading zero" '\357\273\27707.3.11\n' 'ef bb bf 30 37 2e 33 2e 31 31 0a'

cd "$root"
exit $status
