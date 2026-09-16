# Running the comparison again

The scripts of this directory compare GAOL (this repository),
[libieeep1788](https://github.com/nehmeier/libieeep1788) and the intervals of
Solaris Studio's Fortran (`f90 -xia`), and write the tables of
[special_cases.md](../special_cases.md) and
[performance.md](../performance.md). They run on Linux x86-64.

## What they need

- GCC (`g++`, `gcc`), CMake, git, curl, `pkg-config`;
- Python 3 with mpmath and numpy;
- Solaris Studio (Oracle Developer Studio 12.4 was used) with its `bin`
  directory in `PATH`, or its `f90` given by `F90`.

`setup.sh` downloads and builds the rest under `work/`, which git ignores:
GMP and MPFR (unless the system has their headers), libieeep1788 at its last
commit (header-only, it needs MPFR), and GAOL from this repository, built
with CMake in Release and installed with mathlib.

## Running

```bash
cd doc/compare/code
export PATH=/path/to/solarisstudio12.4/bin:$PATH
./setup.sh              # once; FORCE_GAOL=1 ./setup.sh rebuilds GAOL after a change
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
| `P1788_REPEATS` | `1` | The same for libieeep1788, 13 to 90 times slower than GAOL |
| `OPS` | all | Operations to run, separated by commas: `add,sin,shekel5` |
| `LIBS` | `double gaol sun p1788` | Libraries to run |
| `CPU` | | Processor to run on (`taskset -c`) |
| `CXX`, `CC`, `F90` | `g++`, `gcc`, `f90` | Compilers |
| `CXXFLAGS_BENCH`, `F90FLAGS_BENCH` | `-O3 -DNDEBUG`, `-O3 -xia` | Their flags |
| `WORK`, `PREFIX` | `work`, `work/prefix` | Where everything is built and installed |

## The files

| File | |
|---|---|
| `env.sh` | The variables shared by the scripts: directories, versions, compiler flags |
| `setup.sh` | Downloads and builds GMP, MPFR, libieeep1788 and GAOL; checks `f90 -xia` |
| `cases.py` | The 226 special cases, each written once as an expression, taken from GAOL's tests; generates a program per library (`generate`), and compares what they print with IEEE 1788-2015, computed with mpmath (`report`) |
| `run_cases.sh` | Generates, compiles and runs the three programs of the special cases, and writes their table |
| `bench.py` | Draws the intervals of the benchmark (`data`), and writes the tables of its results (`report`) |
| `bench_common.h`, `bench_ops.h` | The benchmark in C++: reading the intervals, timing, and the operations, written once for every C++ library |
| `bench_gaol.cpp`, `bench_p1788.cpp`, `bench_double.cpp` | The benchmark with GAOL, with libieeep1788, and on doubles for reference |
| `bench_sun.f90` | The same operations in Fortran, for Solaris Studio |
| `run_bench.sh` | Draws the intervals, compiles and runs the four programs, and writes the tables |
| `run_all.sh` | `setup.sh`, `run_cases.sh` and `run_bench.sh` |

To add a special case, add a line `case(expression, result of IEEE 1788, note)`
to its group in `cases.py` (the expressions are built with `iv`, `op` and
`text`; see the ones there). To add an operation to the benchmark, add an
`OP(...)` line to `bench_ops.h`, the same block to `bench_sun.f90`, and its
description to `OPERATIONS` in `bench.py`.

## Solaris Studio's intervals

`f90 -xia` gives Fortran the type `interval(8)`, whose operations are
computed by `libsunimath`, following the containment sets of Sun's interval
arithmetic (G. W. Walster), not IEEE 1788-2015. Its manual says that `-xia` is
not available on Linux; Solaris Studio 12.4 compiles and runs it on Linux
x86-64 nonetheless. With `-xia`, `[a, b]` is an interval constant: the arrays
are written `(/ ... /)`. Its intervals have neither `sqr` (`x**2` instead),
nor `asinh`, `acosh`, `atanh`, nor n-th roots, and `system_clock` counts
milliseconds only: `bench_sun.f90` calls `clock_gettime()` instead.
