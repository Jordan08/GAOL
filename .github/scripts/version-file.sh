#!/bin/sh
# Checks how configure, meson or autoconf reads the file VERSION.txt, on a copy
# of the sources in a temporary directory whose VERSION.txt is written in
# several ways. To be run from the root of GAOL's sources, where no build has
# been configured in place (configure refuses to configure a copy of such a
# tree):
#
#   sh .github/scripts/version-file.sh configure|meson|autoconf
#
# A UTF-8 byte order mark at the start of the file (EF BB BF, which some
# editors of Windows write), the line ends of Windows (CR LF), blanks and empty
# lines around the version have to be ignored, and any other text, a file of
# UTF-16 characters (which Windows PowerShell 5 writes for a redirection)
# included, has to be refused with a message giving the first bytes of the file
# in hexadecimal, where the mark, invisible, made the build refuse "5.0.0" as
# VERSION.txt holding "5.0.0". The blanks are the six of ASCII, which CMake
# strips: configure dropped a CR within the version, and meson stripped the
# blanks of Unicode too (a no-break space); configure accepted a file holding
# NUL bytes, which the shell drops from the text of a command. autoconf reads
# the file too, when it generates configure, for the version that configure
# --version gives: the mode autoconf generates configure in the copy with the
# autoconf of PATH, and asks configure --version. autoconf took the blanks out
# of the version and let NUL bytes through, as configure did, and m4, which
# reads the text again, took "7.3.11)" for 7.3.11 and stopped on a bracket.
# The message says to save a file of UTF-16 characters as UTF-8 or ASCII. The
# CMake build reads the file with cmake/gaol_version.cmake, which the test
# version_file of tests/ checks (ctest -R version_file).
#
# Copyright (c) 2026 ENSTA, France
#
# Created 2026-09-29 by Jordan NININ
set -e
build=$1
case "$build" in
  configure|meson|autoconf) ;;
  *) echo "usage: sh .github/scripts/version-file.sh configure|meson|autoconf"; exit 2 ;;
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

# Configures a build directory with the sources in src, or generates
# src/configure, whose output goes to log; the status of the build, or of
# autoconf, is the status of the function
run() {
  rm -rf build
  mkdir build
  case "$build" in
    configure) (cd build && ../src/configure) > log 2>&1 ;;
    meson) meson setup build src > log 2>&1 ;;
    # -f: VERSION.txt is new, whatever the dates of the files
    autoconf) (cd src && rm -rf autom4te.cache && autoconf -f) > log 2>&1 ;;
  esac
}

# accepted <name> <content of VERSION.txt, as printf writes it> [<version>]:
# the version has to be <version>, 7.3.11 by default, in the macros
# GAOL_VERSION, GAOL_MAJOR_VERSION, ... of gaol/gaol_configuration.h, which
# configure and meson write, or in the first line of configure --version,
# which autoconf writes
accepted() {
  version=${3:-7.3.11}
  major=${version%%.*}
  minor=${version#*.}
  micro=${minor#*.}
  minor=${minor%%.*}
  pattern=$(echo "$version" | sed 's/\./\\./g')
  header=build/gaol/gaol_configuration.h
  printf "$2" > src/VERSION.txt
  if ! run; then
    fail "$1: refused"; grep 'VERSION.txt' log | head -3
  elif [ "$build" = autoconf ]; then
    given=$(sh src/configure --version | sed -n 1p)
    if [ "$given" != "gaol configure $version" ]; then
      fail "$1: configure --version gives \"$given\", not gaol configure $version"
    else
      echo "ok: $1"
    fi
  elif ! grep -q "^#define GAOL_VERSION \"$pattern\"\$" $header ||
       ! grep -q "^#define GAOL_MAJOR_VERSION $major\$" $header ||
       ! grep -q "^#define GAOL_MINOR_VERSION $minor\$" $header ||
       ! grep -q "^#define GAOL_MICRO_VERSION $micro\$" $header; then
    fail "$1: not the version $version"; grep 'define GAOL_.*VERSION' $header
  else
    echo "ok: $1"
  fi
}

# refused <name> <content of VERSION.txt> <its first bytes in hexadecimal>
# [utf-16|nul]: the build, or autoconf, has to stop, and its message to give
# the bytes, and to end with the hint for a file of UTF-16 characters, which
# starts with the byte order mark of UTF-16 (utf-16) or holds a NUL byte (nul),
# or without it. The message of autoconf is the error of configure.ac, not
# one of m4. The CRs are taken out of log: meson run by a Python of Windows
# ends its lines with CR LF there, and $ does not match before a CR for GNU
# grep, but the grep 3.0 of MSYS2, which takes the CRs out itself
refused() {
  case "$4" in
    utf-16) hint='; the file is UTF-16: save it as UTF-8 or ASCII' ;;
    nul) hint='; the file holds a NUL byte as UTF-16 does: save it as UTF-8 or ASCII' ;;
    *) hint='' ;;
  esac
  case "$build" in
    autoconf) where='^configure\.ac:[0-9][0-9]*: error: ' ;;
    *) where='' ;;
  esac
  printf "$2" > src/VERSION.txt
  if run; then
    fail "$1: accepted"
  elif ! LC_ALL=C tr -d '\r' < log |
       LC_ALL=C grep -a -q "${where}VERSION\\.txt holds .* (bytes in hexadecimal: $3), where it should hold the version of GAOL, three numbers without leading zeros such as 5\\.0\\.0$hint\$"; then
    fail "$1: the message does not give the bytes $3${4:+ and the hint for $4}"; grep -a 'VERSION.txt' log | head -3
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
accepted "vertical tabs and form feeds around" '\v\f7.3.11\v\f\n'
accepted "numbers of several digits" '\357\273\27712.345.6789\r\n' 12.345.6789
accepted "zeros" '\357\273\2770.0.0\n' 0.0.0
# The bytes: 37 2e 33 2e 31 31 is 7.3.11, 78 is x, 0a the line end
refused "a mark alone" '\357\273\277' 'ef bb bf'
refused "two marks" '\357\273\277\357\273\2777.3.11' 'ef bb bf ef bb bf 37 2e 33 2e 31 31'
refused "a mark inside" '7.3.\357\273\27711\n' '37 2e 33 2e ef bb bf 31 31 0a'
refused "two numbers" '7.3\n' '37 2e 33 0a'
refused "four numbers" '7.3.11.0\n' '37 2e 33 2e 31 31 2e 30 0a'
refused "a letter" '7.3.x\n' '37 2e 33 2e 78 0a'
refused "a zero-width space" '7.3.11\342\200\213\n' '37 2e 33 2e 31 31 e2 80 8b 0a'
refused "a blank inside" '7. 3.11\n' '37 2e 20 33 2e 31 31 0a'
refused "a CR inside" '7.3.\r11\n' '37 2e 33 2e 0d 31 31 0a'
refused "two lines" '7.3.11\n8.0.0\n' '37 2e 33 2e 31 31 0a 38 2e 30 2e 30 0a'
# Characters that m4 reads when autoconf generates configure: a parenthesis
# ended an argument, and a bracket opened a quotation
refused "a parenthesis after" '7.3.11)\n' '37 2e 33 2e 31 31 29 0a'
refused "a bracket after" '7.3.11[\n' '37 2e 33 2e 31 31 5b 0a'
# Blanks that are not of ASCII, which strip() of Python stripped: a no-break
# space, an em space, a file separator
refused "a no-break space after" '7.3.11\302\240\n' '37 2e 33 2e 31 31 c2 a0 0a'
refused "an em space after" '7.3.11\342\200\203\n' '37 2e 33 2e 31 31 e2 80 83 0a'
refused "a file separator after" '7.3.11\034\n' '37 2e 33 2e 31 31 1c 0a'
# 7.3.11 in UTF-16, little-endian, with its byte order mark, as Windows
# PowerShell 5 writes for a redirection: the NUL bytes are not for the message
refused "UTF-16" '\377\3767\000.\0003\000.\0001\0001\000\n\000' 'ff fe 37 00 2e 00 33 00 2e 00 31 00 31 00 0a 00' utf-16
refused "UTF-16, big-endian" '\376\377\0007\000.\0003\000.\0001\0001\000\n' 'fe ff 00 37 00 2e 00 33 00 2e 00 31 00 31 00 0a' utf-16
# The shell drops the NUL bytes from the text of a command: configure read
# these two files as 7.3.11
refused "UTF-16 without a mark" '7\000.\0003\000.\0001\0001\000\n\000' '37 00 2e 00 33 00 2e 00 31 00 31 00 0a 00' nul
refused "a NUL byte after the version" '7.3.11\000\n' '37 2e 33 2e 31 31 00 0a' nul
refused "a leading zero" '\357\273\27707.3.11\n' 'ef bb bf 30 37 2e 33 2e 31 31 0a'
refused "an empty file" '' ''

cd "$root"
exit $status
