# Variables shared by the scripts of doc/compare/code (sourced, not run).
#
# WORK      where the dependencies are downloaded and built, and where the
#           programs and their outputs go (default: doc/compare/code/work,
#           ignored by git)
# PREFIX    where GMP, MPFR, libieeep1788 and GAOL are installed
# FILIB_DIR an installed filib++ (include/interval/interval.hpp and
#           lib/libprim.a); setup.sh builds one under PREFIX otherwise, and
#           remembers the one it was given in WORK/filib-dir
# CXX       the C++ compiler for GAOL, libieeep1788 and filib++ (default: g++)
# F90       Solaris Studio's Fortran compiler (default: f90)
# JOBS      parallel jobs of the builds

CODE_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
ROOT_DIR="$(cd "$CODE_DIR/../../.." && pwd)"
WORK="${WORK:-$CODE_DIR/work}"
PREFIX="${PREFIX:-$WORK/prefix}"
CXX="${CXX:-g++}"
CC="${CC:-gcc}"
F90="${F90:-f90}"
JOBS="${JOBS:-$(nproc 2>/dev/null || echo 4)}"
FILIB_DIR_GIVEN="${FILIB_DIR:-}"
FILIB_DIR="${FILIB_DIR:-$(cat "$WORK/filib-dir" 2>/dev/null || echo "$PREFIX")}"

GMP_VERSION=6.3.0
GMP_SHA256=a3c2b80201b89e68616f4ad30bc66aee4927c3ce50e33929ca819d5c43538898
MPFR_VERSION=4.2.1
MPFR_SHA256=277807353a6726978996945af13e52829e3abd7a9a5b7fb2793894e18f1fcbb2
# Last commit of https://github.com/nehmeier/libieeep1788 (30 March 2015)
P1788_COMMIT=1f10b896ff532e95818856614ab3073189e81199
# filib++ 3.0.2.2, as IBEX distributes it
FILIB_VERSION=3.0.2.2
FILIB_URL=https://github.com/ibex-team/ibex-lib/raw/master/interval_lib_wrapper/filib/3rd/filibsrc-$FILIB_VERSION.tar.gz
FILIB_SHA256=799d89cd0166c61d46bdd4e11db28f5650b5302f1190479765f8bcf01eb3db41
# PROFIL/BIAS 2.0.8 (Olaf Knüppel, Christian Keil, TU Hamburg-Harburg, 2009),
# from its site; PROFIL_TGZ gives a copy of the archive when the site is down
PROFIL_VERSION=2.0.8
PROFIL_URL=https://www.tuhh.de/ti3/keil/profil/Profil-$PROFIL_VERSION.tgz
PROFIL_SHA256=1706e684166360d60f33b4c9301cfe48a6153fc0c624f2087efa9bc94aa07c20
PROFIL_TGZ="${PROFIL_TGZ:-}"
PROFIL_DIR="$PREFIX/profil"

# The flags of interval arithmetic for GCC (see doc/using.md), and those of
# the benchmarks. Solaris Studio needs -xia for its interval type.
IA_CXXFLAGS="-frounding-math -fno-fast-math -ffp-contract=off"
CXXFLAGS_BENCH="${CXXFLAGS_BENCH:--O3 -DNDEBUG}"
F90FLAGS_BENCH="${F90FLAGS_BENCH:--O3 -xia}"
F90FLAGS_CASES="${F90FLAGS_CASES:--O3 -xia}"

export PKG_CONFIG_PATH="$PREFIX/lib/pkgconfig:$PREFIX/lib64/pkgconfig:${PKG_CONFIG_PATH:-}"
export LD_LIBRARY_PATH="$PREFIX/lib:${LD_LIBRARY_PATH:-}"

# Compiler flags of GAOL, as installed (gaol.pc), of libieeep1788 and of filib++
gaol_cflags() { pkg-config --cflags gaol; }
gaol_libs() { pkg-config --libs gaol; }
p1788_cflags() { echo "-std=c++11 -I$PREFIX/include"; }
p1788_libs() { echo "-L$PREFIX/lib -lmpfr -lgmp"; }
# filib++'s headers have dynamic exception specifications, deprecated in C++11
filib_cflags() { echo "-std=c++11 -Wno-deprecated -I$FILIB_DIR/include"; }
filib_libs() { echo "-L$FILIB_DIR/lib -lprim"; }
# PROFIL/BIAS: its headers are in the include directory itself (Interval.h)
profil_cflags() { echo "-I$PROFIL_DIR/include"; }
profil_libs() { echo "-L$PROFIL_DIR/lib -lProfil -lBias -llr"; }

die() { echo "error: $*" >&2; exit 1; }
