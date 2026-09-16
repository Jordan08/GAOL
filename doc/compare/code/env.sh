# Variables shared by the scripts of doc/compare/code (sourced, not run).
#
# WORK      where the dependencies are downloaded and built, and where the
#           programs and their outputs go (default: doc/compare/code/work,
#           ignored by git)
# PREFIX    where GMP, MPFR, libieeep1788 and GAOL are installed
# CXX       the C++ compiler for GAOL and libieeep1788 (default: g++)
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

GMP_VERSION=6.3.0
GMP_SHA256=a3c2b80201b89e68616f4ad30bc66aee4927c3ce50e33929ca819d5c43538898
MPFR_VERSION=4.2.1
MPFR_SHA256=277807353a6726978996945af13e52829e3abd7a9a5b7fb2793894e18f1fcbb2
# Last commit of https://github.com/nehmeier/libieeep1788 (30 March 2015)
P1788_COMMIT=1f10b896ff532e95818856614ab3073189e81199

# The flags of interval arithmetic for GCC (see doc/using.md), and those of
# the benchmarks. Solaris Studio needs -xia for its interval type.
IA_CXXFLAGS="-frounding-math -fno-fast-math -ffp-contract=off"
CXXFLAGS_BENCH="${CXXFLAGS_BENCH:--O3 -DNDEBUG}"
F90FLAGS_BENCH="${F90FLAGS_BENCH:--O3 -xia}"
F90FLAGS_CASES="${F90FLAGS_CASES:--O3 -xia}"

export PKG_CONFIG_PATH="$PREFIX/lib/pkgconfig:$PREFIX/lib64/pkgconfig:${PKG_CONFIG_PATH:-}"
export LD_LIBRARY_PATH="$PREFIX/lib:${LD_LIBRARY_PATH:-}"

# Compiler flags of GAOL, as installed (gaol.pc), and of libieeep1788
gaol_cflags() { pkg-config --cflags gaol; }
gaol_libs() { pkg-config --libs gaol; }
p1788_cflags() { echo "-std=c++11 -I$PREFIX/include"; }
p1788_libs() { echo "-L$PREFIX/lib -lmpfr -lgmp"; }

die() { echo "error: $*" >&2; exit 1; }
