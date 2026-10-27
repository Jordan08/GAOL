<!-- Copyright (c) 2026 ENSTA, France
     Created 2026-10-24 by Jordan NININ -->
# The scripts of the repository

Part of the documentation of [GAOL v5](../README.md#documentation).

Every script of the repository, by what it is for. Each one starts with a
comment that says how to run it and what it needs; this page says what it is
good for, so that the right one is found without opening them all. The scripts
written for GAOL are in the first seven sections, and the files taken from other
projects, which GAOL does not document one by one, are at the end. The scripts
that are sourced, not run, say so.

## Generated files, manual and coverage

| Script | What it is for |
|---|---|
| [gaol/parser/regenerate_parser.sh](../gaol/parser/regenerate_parser.sh) | Regenerates the lexer and the parser from `.lpp` and `.ypp` (flex 2.6.4, bison 3.8.2). Run it after any change of these sources. |
| [gaol/float/gaol_generator_mul_i2f.py](../gaol/float/gaol_generator_mul_i2f.py) | Writes the 256 cases of the multiplication of `interval2f` by the signs of the operands. A tool of the author (Python 3 and `ply`), which no build runs. |
| [manual/build-pdf.sh](../manual/build-pdf.sh) | Builds `gaol.pdf` of the manuals v5 and v4 with `pdflatex`. `make pdf` and the target `pdf` of meson call it. |
| [manual/check_examples.py](../manual/check_examples.py) | Compiles and runs each example of the manual v5 with an installed GAOL, and compares what it prints with what `gaol.tex` shows. |
| [coverage/coverage_summary.py](../coverage/coverage_summary.py) | Reads the JSON of gcovr and writes `coverage/README.md`: lines and branches by file, GAOL apart from CORE-MATH, and the lines never run. |
| [cmake/gaol_version.cmake](../cmake/gaol_version.cmake) | CMake module: `gaol_read_version()` reads `VERSION.txt` before `project()`, as `configure.ac` and `meson.build` do. |
| [cmake/gaol_package_source.cmake](../cmake/gaol_package_source.cmake) | Included by CPack: brings the name and the version of the archive of the sources up to date after a change of `VERSION.txt`. |

## Continuous integration: the audit of the three builds

In [.github/audit/](../.github/audit/).

| Script | What it is for |
|---|---|
| [audit.sh](../.github/audit/audit.sh) | Configures GAOL with CMake, autotools and meson, and writes `OUT/LABEL.json` (with `probe_build.py`). |
| [probe_build.py](../.github/audit/probe_build.py) | For the three configured builds, reads the compiler flags and the configuration GAOL sees, for a C++ source and for a C source of CORE-MATH. |
| [make_probe.py](../.github/audit/make_probe.py) | Writes a C++ file which, preprocessed with the flags of a build, shows the configuration GAOL sees, or with the argument `c` a C file for the C sources. |
| [compare.py](../.github/audit/compare.py) | Compares the macros and the flags of the three builds, and with `--check` fails if they differ. |
| [logs.sh](../.github/audit/logs.sh) | When the audit fails, prints the end of the logs of the three builds, then exits with the same status. |

## Continuous integration: the checks

In [.github/scripts/](../.github/scripts/).

| Script | What it is for |
|---|---|
| [tests.sh](../.github/scripts/tests.sh) | Builds the tests with a GAOL installed by autotools or meson (`static` or `shared`), then runs them. |
| [headers.sh](../.github/scripts/headers.sh) | Checks that each installed header compiles alone in C++11 with `-Wall -Wextra -Wold-style-cast -Wundef -Werror`. |
| [check-intervals.sh](../.github/scripts/check-intervals.sh) | Fails unless the build computes with the expected intervals (`neon`, `sse2` or `fpu`), so that a silent fall back to the FPU does not leave the CI green. |
| [check-gcc-windows-fma.sh](../.github/scripts/check-gcc-windows-fma.sh) | Checks that no object of GCC for Windows names a 256-bit or 512-bit register (GCC bug 54412, the stack aligned on 16 bytes). |
| [comma-locale.sh](../.github/scripts/comma-locale.sh) | Generates or finds a locale that writes a decimal comma, so that `tests/numbers.cpp` does not pass without having tried one. |
| [sde-tests.sh](../.github/scripts/sde-tests.sh) | Runs the tests under Intel SDE, which emulates AVX-512: the runners of GitHub for Windows have none, and this covers the AVX-512 path there. |
| [version-file.sh](../.github/scripts/version-file.sh) | Checks how `configure`, meson and autoconf read `VERSION.txt` written in several ways (UTF-8 byte order mark...). |
| [performance-ab.sh](../.github/scripts/performance-ab.sh) | Builds and measures two builds in turn in the same job: the base and the head of a pull request, or one source with two options. |
| [compare-performance.py](../.github/scripts/compare-performance.py) | Compares two series of measures (median, ratio, spread) and writes a Markdown table for the summary of a job. |
| [test-performance-tools.py](../.github/scripts/test-performance-tools.py) | Tests `compare-performance.py` and `processor_name()` of `run_perf.sh`, in Python alone and without a build. |

The workflows of [.github/workflows/](../.github/workflows/) are read by
[actionlint](https://github.com/rhysd/actionlint), which runs `shellcheck` on
the steps that have a script, and reports nothing (GAOL v5):

```sh
docker run --rm -v "$PWD:/repo:ro" --workdir /repo rhysd/actionlint:latest
```

[.github/actionlint.yaml](../.github/actionlint.yaml) lists the one remark it
leaves out, with the reason. The scripts of [.github/](../.github/) and of
[doc/compare/code/](compare/code/) are read by `shellcheck`, run from the root
of the repository (its options are in [.shellcheckrc](../.shellcheckrc)), which
reports nothing either: the lists of flags that the shell has to split carry a
`# shellcheck disable=` with the reason.

## Tests

In [tests/](../tests/). The five `*_values.py` scripts write the headers of
reference values that the test programs include; they are run by hand when a
table changes, and need [mpmath](https://mpmath.org). See [Tests](tests.md) for
what the programs check.

| Script | What it is for |
|---|---|
| [elementary_values.py](../tests/elementary_values.py) | Writes `elementary_values.h`: for each f(x), the greatest double below and the least double above, computed with mpmath, for `elementary.cpp`. |
| [extended_precision_values.py](../tests/extended_precision_values.py) | Writes `extended_precision_values.h`: the bounding values and the nearest double, to compare CORE-MATH and GAOL. |
| [reverse_values.py](../tests/reverse_values.py) | Writes `reverse_values.h`: the cases of the reverse functions of IEEE 1788-2015 (10.5.4) for `reverse.cpp`. |
| [atan2_rel_values.py](../tests/atan2_rel_values.py) | Writes `atan2_rel_values.h`: the cases of `atan2Rev1` and `atan2Rev2`, for `reverse.cpp`. |
| [pow_rel_values.py](../tests/pow_rel_values.py) | Writes `pow_rel_values.h`: the cases of `powRev1` and `powRev2`, for `reverse.cpp`. |
| [refused_options.sh](../tests/refused_options.sh) | Checks that autotools and meson refuse `-ffast-math` and `-ffinite-math-only`, as the tests `refused_*` of CMake do. |
| [release_flags.py](../tests/release_flags.py) | Checks that a project which brings GAOL in without a build type compiles it in release (`-O3`, `NDEBUG`, `GAOL_DEBUGGING` at 0). |
| [meson_subproject/check_flags.py](../tests/meson_subproject/check_flags.py) | Checks that a meson project which takes GAOL as a subproject gets the flags of its `gaol.pc`. |
| [compile_error.cmake](../tests/compile_error.cmake) | A `cmake -P` script that compiles a source whose compilation has to fail (the tests `refused_*` and `nodiscard_discard_cxx<N>`). |
| [cpack_stale_configure.cmake](../tests/cpack_stale_configure.cmake) | Tests the warning of CPack when the committed `configure` was generated for another version. |
| [automatic_configuration.cmake](../tests/automatic_configuration.cmake) | Tests the configuration that GAOL decides by itself (the fused multiply-add instructions, the AVX-512 path, the 128-bit integer, the debugging, the vector intervals) and the settings of the developers, in several configurations of one directory and in a cross-compilation. |
| [version_file.cmake](../tests/version_file.cmake) | Tests the reading of `VERSION.txt` by `gaol_read_version()` (byte order mark and other ways of writing it). |

## Tools for the tests

In [tests/tools/](../tests/tools/), with a `README.md` in each directory.

| Script | What it is for |
|---|---|
| [arith/diffarith.py](../tests/tools/arith/diffarith.py) | Differential test of the arithmetic: compiles `diffarith.cpp` against two builds or more and compares the outputs. |
| [pow/build.py](../tests/tools/pow/build.py) | Compiles a program of `tests/tools/pow` against a build of GAOL, with the flags that build gives its own tests. |
| [pow/diffpow.py](../tests/tools/pow/diffpow.py) | Differential test of `pow`, on the same principle as `diffarith.py`. |
| [pow/gen_table.py](../tests/tools/pow/gen_table.py) | Reads and rewrites the table `pow_on_boxes()` of `tests/ieee1788.cpp` as lines "X \| Y \| S \| H". |
| [pow/checkrows.py](../tests/tools/pow/checkrows.py) | Checks the bounds of `pow` in a file of such lines against the exact power (mpmath, 500 bits). |
| [pow/mutate.py](../tests/tools/pow/mutate.py) | Mutation testing of `pow`: puts one deliberate error in the code, and the tests have to fail. |
| [pow/pown_bound.py](../tests/tools/pow/pown_bound.py) | Checks, with rationals, the bound K of the error of the integer powers of `pown_products()`. |

## Comparisons with other libraries

In [doc/compare/](compare/README.md). The scripts of `code/` need `setup.sh`
first, which downloads and builds what the comparisons need under `$WORK`.

| Script | What it is for |
|---|---|
| [code/setup.sh](compare/code/setup.sh) | Downloads and builds GMP, MPFR, libieeep1788, filib++, PROFIL/BIAS and GAOL (with and without `GAOL_PRESERVE_ROUNDING`) for the comparisons. |
| [code/env.sh](compare/code/env.sh) | The variables shared by the scripts of `code/` (`WORK`, `PREFIX`, `FILIB_DIR`...). Sourced, not run. |
| [code/build_benchmarks.sh](compare/code/build_benchmarks.sh) | Compiles the programs of the benchmark and describes the machine. Sourced by `run_bench.sh` and `run_lines.sh`. |
| [code/run_all.sh](compare/code/run_all.sh) | Runs the whole comparison: setup, special cases, enclosure, benchmark, long lines. |
| [code/run_bench.sh](compare/code/run_bench.sh) | Draws the intervals, compiles and runs the benchmark of each library, and writes the tables of `performance.md`. |
| [code/run_lines.sh](compare/code/run_lines.sh) | Measures the long lines of `+ - * /` (`bench_ops.h`) and writes their tables in `performance.md`. |
| [code/run_perf.sh](compare/code/run_perf.sh) | `make perf`: measures GAOL v5 alone again on the benchmark and rewrites the tables with its new times. |
| [code/run_cases.sh](compare/code/run_cases.sh) | Generates and runs the programs of special cases for each library, and writes `special_cases.md`. |
| [code/run_enclosure.sh](compare/code/run_enclosure.sh) | Tests the enclosure of the elementary functions (GAOL, Boost) against mpmath, and writes `enclosure.md`. |
| [code/bench.py](compare/code/bench.py) | Draws the random intervals of the benchmark and turns its results into tables. |
| [code/cases.py](compare/code/cases.py) | Writes each special case once, generates a program per library, and computes the result the standard gives with mpmath. |
| [code/enclosure.py](compare/code/enclosure.py) | Draws the arguments of the elementary functions and checks their bounds against mpmath at 2000 bits. |
| [itf1788/fetch.sh](compare/itf1788/fetch.sh) | Fetches ITF1788, the test suite of IEEE 1788-2015, at a fixed commit, without copying it into the repository. |
| [itf1788/run.sh](compare/itf1788/run.sh) | Builds GAOL (SSE2 or NEON, then FPU), generates a program per `.itl` file, runs them and writes `itf1788.md`. |
| [itf1788/itf1788.py](compare/itf1788/itf1788.py) | The back end of GAOL v5 for ITF1788: reads the `.itl` files, generates the test programs and writes the report. |
| [tang2021/env.sh](compare/tang2021/env.sh) | The variables shared by `fetch.sh` and `run.sh` (`WORK`, `N`, `CC`, `CXX`, `JOBS`). Sourced, not run. |
| [tang2021/fetch.sh](compare/tang2021/fetch.sh) | Fetches the benchmark of Tang et al. (2021), without its 1.5 GB of inputs, and adds GAOL to it with a patch. |
| [tang2021/run.sh](compare/tang2021/run.sh) | Builds GAOL for each variant (`sse`, `fpu`), builds the benchmark with it, runs its queries and writes the tables of the results. |
| [tang2021/check.py](compare/tang2021/check.py) | Checks the queries `lo <= expression <= hi` of the benchmark with exact rationals, without Mathematica. |

## Tools for the maintainers and their agents

In [todo-notes/orchestration/](../todo-notes/orchestration/README.md).

| Script | What it is for |
|---|---|
| [bin/check_branch](../todo-notes/orchestration/bin/check_branch) | Checks a branch before it is pushed: commit messages, registers, build products, whitespace. |
| [bin/gcore](../todo-notes/orchestration/bin/gcore) | Shares the 4 permitted cores among several agents with `flock` tokens. Copy it out of the tree before use. |
| [points_w4.py](../todo-notes/orchestration/points_w4.py) | The detailed plans of the large points 25 to 30 (sub-features in a branch each, joined in one pull request). None of it was done. |

## Taken from other projects

GAOL does not change these files, and does not document them one by one
([3rd/README.md](../3rd/README.md) lists what it changed in CORE-MATH).

| Files | What they are for |
|---|---|
| `config.guess`, `config.sub`, `configure`, `depcomp`, `compile`, `install-sh`, `ltmain.sh`, `missing`, `test-driver` at the root | Provided or generated by autoconf, automake and libtool, for the autotools build. |
| [3rd/math-core/](../3rd/math-core/): `check.sh`, `ci.sh`, `perf.sh`, `perf-all.sh`, `prof.sh`, `doc/Fortran.sh` | The tools of CORE-MATH: checking against MPFR, its CI, speed measures, profiling, and the Fortran module. |
| `3rd/math-core/ci/` (12 scripts) | The CI of CORE-MATH: Docker images, LLVM and RLIBM, and comparisons with these libraries. |
| `3rd/math-core/src/binary64/*/doit*.sh` (8) and `src/binary128/sin/sinq_tables.py` | The author's scripts of CORE-MATH: fits of polynomials with Sollya, the error bound of `log`, the tables of `sinq` and `cosq`. |
