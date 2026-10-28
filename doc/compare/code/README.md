<!-- Copyright (c) 2026 ENSTA, France
     Created 2026-09-20 by Jordan NININ -->
# Running the comparison again

The scripts of this directory compare GAOL (this repository), the last
version of [Frédéric Goualard's GAOL](https://github.com/goualard-f/GAOL),
[libieeep1788](https://github.com/nehmeier/libieeep1788),
[filib++](https://www2.math.uni-wuppertal.de/wrswt/software/filib.html),
[PROFIL/BIAS](https://www.tuhh.de/ti3/keil/profil/),
the intervals of Solaris Studio's Fortran (`f90 -xia`) and
[Boost.Interval](https://www.boost.org/libs/numeric/interval), and write the
tables of
[special_cases.md](../special_cases.md),
[enclosure.md](../enclosure.md) and
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
and checked with `make check`, GAOL from this repository, built with CMake
in Release and installed, and again with `GAOL_PRESERVE_ROUNDING=ON` in a
prefix of its own (`work/prefix/gaol-preserve`; AVX-512 is left enabled, which
that option turns on by default where the compiler and the machine have it:
`setup.sh` says whether the library took it), and the last GAOL of Frédéric
Goualard (4.2.3, the master branch of his repository) with
[mathlib 2.1.1](https://frederic.goualard.net/software/mathlib-2.1.1.tar.gz)
from his site, built with their configure, without the preservation of the
rounding direction, as GAOL v5, and with the flags GAOL v5 is built with,
which his configure gives to `g++` alone, and the headers of Boost.Interval
of Boost 1.92.0, with those of Config and Detail, the two modules of Boost
they include, from the archives of the three modules at the tag of the release
on GitHub. All of them are compiled by `CC` and `CXX` with `-O3` (GMP, MPFR
and PROFIL/BIAS compile with `-O2` on their own,
and the configure of filib++ without any optimization) and with the fused
multiply-add instructions of the processor, `-mfma` (`FMA_FLAGS`), as GAOL is
by default (`GAOL_FMA`), filib++ and PROFIL/BIAS in C++11: Clang 16 and GCC 11
compile C++17 by default, where their dynamic exception specifications and
`register` are errors. PROFIL/BIAS is compiled with `-ffp-contract=off` too:
with `-mfma`, Clang contracted its outward roundings into fused multiply-adds
whose subnormal addend made `sqrt`, `exp` and `log` six to fifteen times slower
(see `setup.sh`). The benchmark and the special cases compile their
programs with `-mfma` too. Solaris Studio's intervals are computed by
`libsunimath`, compiled already, which no flag reaches.

## Running

```bash
cd doc/compare/code
export PATH=/path/to/solarisstudio12.4/bin:$PATH
FILIB_DIR=/path/to/filib ./setup.sh   # once; without FILIB_DIR, it builds filib++
                        # FORCE_GAOL=1 ./setup.sh rebuilds GAOL after a change
./run_cases.sh          # the special cases        -> ../special_cases.md
./run_enclosure.sh      # the test of enclosure    -> ../enclosure.md
CPU=2 ./run_bench.sh    # the benchmark, pinned on processor 2 -> ../performance.md
CPU=2 ./run_lines.sh    # the long lines of + - * /, the same -> the end of ../performance.md
```

The tables of [performance.md](../performance.md),
[special_cases.md](../special_cases.md) and [enclosure.md](../enclosure.md)
were computed with Clang 21, in a work directory of its own, with four jobs
(GCC gives the same special cases; its timings differ, the libraries not being
compiled the same way):

```bash
export CC=clang-21 CXX=clang++-21 WORK=$PWD/work/clang21 JOBS=4
./setup.sh && ./run_cases.sh && ./run_enclosure.sh && CPU=2 ./run_bench.sh && CPU=2 ./run_lines.sh
```

GAOL v5 is measured by the benchmark under the names of IEEE 1788-2015, the
namespace `gaol_ieee1788` (`pown(x, n)`, and `pow(x, y)` the real power of the
standard), as the other libraries of the standard are, and GAOL 4.2.3 under
its own, `gaol`; the special cases and the test of enclosure run GAOL v5 under
both (see [Using GAOL](../../using.md#the-powers-in-the-two-namespaces)).

`./run_all.sh` runs the five in turn. The scripts rewrite only the tables
between the `<!-- BEGIN GENERATED TABLES -->` and `<!-- END GENERATED TABLES -->`
markers of the reports (`<!-- BEGIN GENERATED LINES -->` and `<!-- END GENERATED
LINES -->` for `run_lines.sh`, at the end of `performance.md`): the text around
them, which comments on the results, has to be checked by hand against the new
tables.

The benchmark takes about 6 minutes on an Intel Core i7-1185G7,
libieeep1788 most of it, and the long lines 3 more; the machine should do
nothing else meanwhile. Their variables:

| Variable | Default | |
|---|---|---|
| `N` | `1000000` | Operations of each kind |
| `ROUNDS` | `3` | Times each program is run, in turn with the others, the best time of all being kept |
| `REPEATS` | `5` | Runs of each operation in a round |
| `P1788_REPEATS` | `1` | The same for libieeep1788, far slower than the others |
| `OPS` | all | Operations to run, separated by commas: `add,sin,shekel5` |
| `LINE_LIST` | all | The same for `run_lines.sh`, the long lines of `bench_ops.h` (`sum16,mul8,mix13...`), which `run_bench.sh` does not run; `LIBS` has no `sun` there, and has `gaol5p` there too (a column of the table of the times of the lines only) |
| `LIBS` | `double gaol5 gaol5p gaol filib profil boost sun p1788` | Libraries to run: `gaol5` is the GAOL of this repository, `gaol5p` the same built with `GAOL_PRESERVE_ROUNDING=ON` (a column of the table of the times only), `gaol` the last GAOL of Frédéric Goualard |
| `MERGE` | | `1`: the times of what ran (`LIBS`, `OPS`) replace their rows of `results.csv`, the other rows being kept, and the report is written from all of them, as `make perf` does for GAOL v5: `MERGE=1 LIBS=gaol5p CPU=2 ./run_bench.sh` measures the preserved direction alone |
| `CPU` | | Processor to run on (`taskset -c`) |
| `CXX`, `CC`, `F90` | `g++`, `gcc`, `f90` | Compilers, the same for `setup.sh` and the other scripts |
| `CXXFLAGS_BENCH`, `F90FLAGS_BENCH` | `-O3 -DNDEBUG`, `-O3 -xia` | Their flags |
| `FMA_FLAGS` | `-mfma` | The flag of the fused multiply-add instructions, given to every library `setup.sh` builds and to every program; empty (`FMA_FLAGS=`) for a processor without them |
| `WORK`, `PREFIX` | `work`, `work/prefix` | Where everything is built and installed |
| `FILIB_DIR` | the one `setup.sh` was given, or `PREFIX` | An installed filib++ (`include/interval/interval.hpp`, `lib/libprim.a`) |
| `PROFIL_TGZ` | | A copy of `Profil-2.0.8.tgz`, for `setup.sh` when the site of PROFIL/BIAS is down |
| `ENCLOSURE_N` | `100000` | Arguments of each function for `run_enclosure.sh`, whose values mpmath computes with 2000 bits, about 5 minutes the first time for 100 000 |

## The long lines on a 64-bit ARM processor

`run_lines_arm64.sh` runs the long lines on a 64-bit ARM processor, with three
GAOLs only: GAOL V5.0.0, GAOL V5.0.0 built with `GAOL_PRESERVE_ROUNDING=ON`
(the NEON intervals reading and writing FPCR, where the library on x86 has the
AVX-512 instructions) and GAOL 4.2.3, whose configure knows macOS on ARM as
`arm-apple-*`. The other libraries are not built there: `setup.sh` builds
libraries and passes flags that are x86 only. It needs a C++ compiler, CMake,
git, curl, pkg-config and Python 3 with numpy, builds under `work/arm64`, and
draws the same intervals as `run_bench.sh` (the generator is seeded). macOS has
no `taskset`: the programs are not pinned, and the machine should be on mains
power and do nothing else meanwhile.

```bash
cd doc/compare/code && ./run_lines_arm64.sh   # -> the end of ../performance.md
```

## GAOL v5 alone: `make perf`

The target `perf` of the three builds of GAOL (`make perf` with CMake and
configure, `ninja perf` with meson) measures GAOL v5 again without building the
other libraries: it compiles `bench_gaol.cpp` with the flags and the library of
the build, and runs `run_perf.sh`, which draws the intervals (Python 3 with
numpy, the same intervals as `run_bench.sh`), runs the program `ROUNDS` times,
replaces the rows of GAOL v5 in `results.csv` by the new ones, and writes the
tables of [performance.md](../performance.md) from them, the line of GAOL v5 of
`machine.txt` saying when and how it was measured. It runs the ten long lines of
`run_lines.sh` too (GAOL v5), on the same intervals and in the same way: the rows
of GAOL v5 of `results_lines.csv` and its line of `machine_lines.txt` are
replaced, and the tables at the end of `performance.md` written from them.
GAOL 4.2.3, filib++,
libieeep1788, PROFIL/BIAS, Solaris Studio, Boost.Interval and the doubles keep
the times of the last whole run, which `results.csv` and `results_lines.csv`
hold: the machine should be the one they were measured on (`machine.txt`),
doing nothing else. With CMake and meson the target exists on Linux and on macOS
(GAOL v5: on Linux only before); the `perf` of configure has never depended on
the system, only on bash. `run_perf.sh` names the processor with `sysctl` where
`/proc/cpuinfo` has none. `N`, `ROUNDS`, `REPEATS`, `OPS`, `WORK` (default:
`perf` in the build directory) and `REPORT` apply as for `run_bench.sh`. `CPU`
pins the programs with `taskset`, which macOS has not: there it is ignored, and
the script says so (GAOL v5: it stopped, `taskset: command not found`).
`LINE_LIST` names the long lines to run, as for `run_lines.sh`; set and empty
(`LINE_LIST=`) it runs none, in both (GAOL v5: `run_lines.sh` ran the ten):

```bash
cmake -S . -B build && CPU=2 cmake --build build --target perf   # Linux: pinned on processor 2
./configure && make && CPU=2 make perf
meson setup build && CPU=2 ninja -C build perf
cmake -S . -B build && cmake --build build --target perf         # macOS: no taskset, no CPU
```

`results.csv` is written by `run_bench.sh` after a whole run (all the
libraries, all the operations). The one committed holds the whole run of 17
October 2026 that the tables of `performance.md` were written from (Clang
21.1, three rounds, Ubuntu 26.04.1, `7c1ebb9`), and `results_lines.csv` the one of the long lines, of the same day. Under Ubuntu 20.04, on the
same processor with Clang 18.1.8, every library took about 30 % more time: the
times of `make perf` compare with those of `results.csv` only on the system
and with the compiler they were measured with.

The continuous integration runs the same ten lines, with `bench_gaol.cpp`
built by CMake as `gaol_bench` (not by default), for the base of a pull request
and for its head, measured in turn in the same job and compared in the summary
of the job (`performance.yml`, with `.github/scripts/performance-ab.sh`: see
[Continuous integration](../../continuous-integration.md#the-speed-of-a-pull-request)).

## The files

| File | |
|---|---|
| `env.sh` | The variables shared by the scripts: directories, versions, compiler flags |
| `setup.sh` | Downloads and builds GMP, MPFR, libieeep1788, filib++, PROFIL/BIAS, GAOL, and Goualard's GAOL with mathlib, and downloads the headers of Boost.Interval; checks `f90 -xia` |
| `cases.py` | The 290 special cases, each written once as an expression, taken from GAOL's tests; generates a program per library (`generate`), and compares what they print with IEEE 1788-2015, computed with mpmath (`report`) |
| `run_cases.sh` | Generates, compiles and runs the six programs of the special cases, and writes their table |
| `bench.py` | Draws the intervals of the benchmark (`data`), writes the tables of its results (`report`) and those of the long lines (`lines-report`), and prints the names of the lines (`line-names`) |
| `bench_common.h`, `bench_ops.h` | The benchmark in C++: reading the intervals, timing, and the operations and the long lines, written once for every C++ library |
| `build_benchmarks.sh` | The compilation of the C++ programs and the description of the machine, which `run_bench.sh` and `run_lines.sh` share (sourced) |
| `run_lines.sh` | Compiles and runs the long lines of + - * / of `bench_ops.h` with the C++ libraries, and writes their tables at the end of `performance.md`; after a whole run, keeps its results in `results_lines.csv` and `machine_lines.txt` |
| `run_lines_arm64.sh` | The same long lines on a 64-bit ARM processor (an Apple M2 under macOS), with GAOL V5.0.0, GAOL V5.0.0 with the rounding direction preserved and GAOL 4.2.3 alone: builds the three itself, and writes the table at the very end of `performance.md`, keeping its results in `results_lines_arm64.csv` and `machine_lines_arm64.txt` |
| `bench_gaol.cpp`, `bench_p1788.cpp`, `bench_filib.cpp`, `bench_profil.cpp`, `bench_boost.cpp`, `bench_double.cpp` | The benchmark with GAOL, libieeep1788, filib++, PROFIL/BIAS and Boost.Interval, and on doubles for reference |
| `boost_policies.h` | The intervals of Boost.Interval that the benchmark, the special cases and the test of enclosure use, and why |
| `bench_sun.f90` | The same operations in Fortran, for Solaris Studio |
| `run_bench.sh` | Draws the intervals, compiles and runs the seven programs, and writes the tables; after a whole run, keeps its results in `results.csv` and `machine.txt` |
| `enclosure.py` | Draws the arguments of the elementary functions for the test of enclosure (`data`), and checks the results of the libraries against mpmath and writes their tables (`report`) |
| `enclosure.h`, `enclosure_gaol.cpp`, `enclosure_boost.cpp` | The test of enclosure in C++, written once for every library, with GAOL, and with Boost.Interval under its two policies and with the C library alone |
| `run_enclosure.sh` | Draws the arguments, compiles and runs the two programs of the test of enclosure, and writes its tables |
| `run_perf.sh` | `make perf`: runs the benchmark of GAOL v5 alone, the long lines included, and writes the tables with its new times (see below) |
| `results.csv`, `machine.txt`, `results_lines.csv`, `machine_lines.txt` | The results of the last whole run, of the operations and of the long lines, and what they were measured on, which `run_perf.sh` keeps for the other libraries |
| `results_lines_arm64.csv`, `machine_lines_arm64.txt` | The results of the long lines on the 64-bit ARM processor, and what they were measured on |
| `run_all.sh` | `setup.sh`, `run_cases.sh`, `run_enclosure.sh` and `run_bench.sh` |

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

## Boost.Interval's intervals

`boost::numeric::interval<double>`, with its default policies, has no
elementary function: its rounding policy, `rounded_math<double>`, is
`save_state<rounded_arith_opp<double> >`, which has no `exp_down()` nor
`sin_up()`, and `exp(x)` does not compile. The elementary functions come with
one of the policies `rounded_transc_std` and `rounded_transc_opp`, which call
the C library's `std::exp` or `std::sin` with the rounding direction set
downward for the lower bound and upward for the upper one. The programs take
`save_state<rounded_transc_opp<double> >`, built on the same
`rounded_arith_opp<double>`, so that everything but the elementary functions
is the code of the default `interval<double>`, as in the example of
Boost.Interval that uses them (`examples/findroot_demo.cpp`), and keep the
default checking, `checking_strict<double>`, which throws
`std::runtime_error` where the empty set would be created
(`boost_policies.h`). The test of enclosure measures `rounded_transc_std` too.
Boost.Interval reads no interval from text, and has no real power: the
benchmark computes `pow(x, y)` as exp(y·log(x)), and the special cases mark
both n/a.
