#!/bin/sh
# Builds CRlibm (https://github.com/taschini/crlibm, GNU LGPL v2 or later) and
# installs it under the prefix given:
#
#   sh scripts/install-crlibm.sh <prefix>
#
# For the builds of GAOL with CRlibm rather than mathlib: configure
# (--with-mathlib=crlibm --with-mathlib-include=<prefix>/include
# --with-mathlib-lib=<prefix>/lib) and meson (-Dwith-mathlib=crlibm
# -Dwith-mathlib-include, -Dwith-mathlib-lib). Needs git, autoconf, automake,
# make and a C compiler. The continuous integration builds CRlibm with it.
set -e

prefix=$1
# The last commit of the repository (4 January 2016)
commit=eb3063791aa75bc9705b49283bf14250465220a7
work=${RUNNER_TEMP:-/tmp}/crlibm-work
rm -rf "$work"
mkdir -p "$work"

git clone -q https://github.com/taschini/crlibm.git "$work/crlibm"
cd "$work/crlibm"
git checkout -q "$commit"
autoreconf -i
# Position-independent code, for the shared library of GAOL the meson build
# links it into
./configure --prefix="$prefix" CFLAGS="-O2 -fPIC"
make -j 4
make install
