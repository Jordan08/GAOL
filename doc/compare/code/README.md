# Running the comparison again

The scripts of this directory compare GAOL (this repository),
[libieeep1788](https://github.com/nehmeier/libieeep1788),
[filib++](https://www2.math.uni-wuppertal.de/wrswt/software/filib.html),
[PROFIL/BIAS](https://www.tuhh.de/ti3/keil/profil/) and
the intervals of Solaris Studio's Fortran (`f90 -xia`), and write the tables
of
[special_cases.md](../special_cases.md) and
[performance.md](../performance.md). They run on Linux x86-64.

## What they need

- GCC or Clang (`g++` and `gcc` unless `CXX` and `CC` give others), CMake,
  git, curl, `pkg-config`;
- Python 3 with mpmath and numpy;
- Solaris Studio (Oracle Developer Studio 12.4 was used) with its `bin`
  directory in `PATH`, or its `f90` given by `F90`.

`setup.sh` downloads and builds the rest under `work/`, which git ignores:
GMP and MPFR (unless the system has their headers), libieeep1788 at its last
commit (header-only, it needs MPFR), filib++ 3.0.2.2 from the archive IBEX
distributes, unless `FILIB_DIR` gives an installed filib++, PROFIL/BIAS 2.0.8
from its site (or from the archive `PROFIL_TGZ` gives), built with its
configuration `x86-64-Linux-compat-gcc`, its `gcc` replaced by `CC` and `CXX`,
and checked with `make check`, and GAOL from this repository, built with CMake
in Release and installed with mathlib. All of them are compiled by `CC` and
`CXX` with `-O3` (GMP, MPFR and PROFIL/BIAS compile with `-O2` on their own,
and the configure of filib++ without any optimization) and with the fused
multiply-add instructions of the processor, `-mfma` (`FMA_FLAGS`), as GAOL is
by default (`GAOL_FMA`), filib++ and PROFIL/BIAS in C++11: Clang 16 and GCC 11
compile C++17 by default, where their dynamic exception specifications and
`register` are errors. The benchmark and the special cases compile their
programs with `-mfma` too. Solaris Studio's intervals are computed by
`libsunimath`, compiled already, which no flag reaches.

## Running

```bash
cd doc/compare/code
export PATH=/path/to/solarisstudio12.4/bin:$PATH
FILIB_DIR=/path/to/filib ./setup.sh   # once; without FILIB_DIR, it builds filib++
                        # FORCE_GAOL=1 ./setup.sh rebuilds GAOL after a change
./run_cases.sh          # the special cases        -> ../special_cases.md
CPU=2 ./run_bench.sh    # the benchmark, pinned on processor 2 -> ../performance.md
```

`./run_all.sh` runs the three in turn. The scripts rewrite only the tables
between the `<!-- BEGIN GENERATED TABLES -->` and `<!-- END GENERATED TABLES -->`
markers of the reports: the text around them, which comments on the results,
has to be checked by hand against the new tables.

The benchmark takes about 10 minutes on an Intel Core i7-1185G7,
libieeep1788 most of it; the machine should do nothing else meanwhile. Its
variables:

| Variable | Default | |
|---|---|---|
| `N` | `1000000` | Operations of each kind |
| `ROUNDS` | `3` | Times each program is run, in turn with the others, the best time of all being kept |
| `REPEATS` | `5` | Runs of each operation in a round |
| `P1788_REPEATS` | `1` | The same for libieeep1788, far slower than the others |
| `OPS` | all | Operations to run, separated by commas: `add,sin,shekel5` |
| `LIBS` | `double gaol filib profil sun p1788` | Libraries to run |
| `CPU` | | Processor to run on (`taskset -c`) |
| `CXX`, `CC`, `F90` | `g++`, `gcc`, `f90` | Compilers, the same for `setup.sh` and the other scripts |
| `CXXFLAGS_BENCH`, `F90FLAGS_BENCH` | `-O3 -DNDEBUG`, `-O3 -xia` | Their flags |
| `FMA_FLAGS` | `-mfma` | The flag of the fused multiply-add instructions, given to every library `setup.sh` builds and to every program; empty (`FMA_FLAGS=`) for a processor without them |
| `WORK`, `PREFIX` | `work`, `work/prefix` | Where everything is built and installed |
| `FILIB_DIR` | the one `setup.sh` was given, or `PREFIX` | An installed filib++ (`include/interval/interval.hpp`, `lib/libprim.a`) |
| `PROFIL_TGZ` | | A copy of `Profil-2.0.8.tgz`, for `setup.sh` when the site of PROFIL/BIAS is down |

## The files

| File | |
|---|---|
| `env.sh` | The variables shared by the scripts: directories, versions, compiler flags |
| `setup.sh` | Downloads and builds GMP, MPFR, libieeep1788, filib++, PROFIL/BIAS and GAOL; checks `f90 -xia` |
| `cases.py` | The 291 special cases, each written once as an expression, taken from GAOL's tests; generates a program per library (`generate`), and compares what they print with IEEE 1788-2015, computed with mpmath (`report`) |
| `run_cases.sh` | Generates, compiles and runs the five programs of the special cases, and writes their table |
| `bench.py` | Draws the intervals of the benchmark (`data`), and writes the tables of its results (`report`) |
| `bench_common.h`, `bench_ops.h` | The benchmark in C++: reading the intervals, timing, and the operations, written once for every C++ library |
| `bench_gaol.cpp`, `bench_p1788.cpp`, `bench_filib.cpp`, `bench_profil.cpp`, `bench_double.cpp` | The benchmark with GAOL, libieeep1788, filib++ and PROFIL/BIAS, and on doubles for reference |
| `bench_sun.f90` | The same operations in Fortran, for Solaris Studio |
| `run_bench.sh` | Draws the intervals, compiles and runs the six programs, and writes the tables |
| `run_all.sh` | `setup.sh`, `run_cases.sh` and `run_bench.sh` |

To add a special case, add a line `case(expression, result of IEEE 1788, note)`
to its group in `cases.py` (the expressions are built with `iv`, `op` and
`text`; see the ones there). To add an operation to the benchmark, add an
`OP(...)` line to `bench_ops.h`, the same block to `bench_sun.f90`, and its
description to `OPERATIONS` in `bench.py`.

## filib++'s intervals

The programs use `filib::interval<double, native_switched,
i_mode_extended_flag>`, the intervals of IBEX built with filib++, and call
`fp_traits<double, native_switched>::setup()` first. filib++'s headers declare
`pow(x, int)` and `power(x, y)`, but define `power(x, int)` and `pow(x, y)`:
the programs call the latter. Their dynamic exception specifications need
C++11 or C++14 (`-std=c++11 -Wno-deprecated`).

## Solaris Studio's intervals

`f90 -xia` gives Fortran the type `interval(8)`, whose operations are
computed by `libsunimath`, following the containment sets of Sun's interval
arithmetic (G. W. Walster), not IEEE 1788-2015. Its manual says that `-xia` is
not available on Linux; Solaris Studio 12.4 compiles and runs it on Linux
x86-64 nonetheless. With `-xia`, `[a, b]` is an interval constant: the arrays
are written `(/ ... /)`. Its intervals have neither `sqr` (`x**2` instead),
nor `asinh`, `acosh`, `atanh`, nor n-th roots, and `system_clock` counts
milliseconds only: `bench_sun.f90` calls `clock_gettime()` instead.
