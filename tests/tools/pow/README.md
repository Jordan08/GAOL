<!-- Copyright (c) 2026 ENSTA, France
     Created 2026-10-04 by Jordan NININ -->
# Verification tools of pow

Part of the [tests](../../../doc/tests.md) of GAOL v5.

The programs and scripts of this directory check a change of the `pow`
functions of `gaol/gaol_interval_core.cpp`: `pow_standard()`, the pow of
IEEE 1788-2015 (Table 9.1), which `gaol_ieee1788::pow` is, the integer power
that `gaol::pow(x, y)` (`gaol_pow_hybrid()`) and `gaol::pow(x, p)`
(`gaol_pow_real()`) add to it, the corners of a box (`pow_lo()`, `pow_hi()`,
`pow_is_double()`), and the integer power itself, `pown` (`pown_products()`,
`pown_point()`, `pown_upward()` and the powers beyond the ints, GAOL v5,
point B of `TODO.md`). They were written when the pow of
the standard was written once (#37), to show that no bound changed and that
`pow_on_boxes()` of `tests/ieee1788.cpp` fails on a wrong `pow`, and kept for
the points B.1 and B.2 of `TODO.md`: B.1 removes a check, which must change
no bound; B.2 extends the corners to the boxes with an infinite bound and to
the bases from 0, which changes bounds of `pow_on_boxes()`, to be computed
again and checked apart from GAOL.

They are not part of the build nor of `make test`: they compare two builds of
GAOL, or a build with mpmath, and their results are read by whoever changes
`pow`.

| File | What it does |
|---|---|
| `diffpow.cpp` | Prints the bounds of `gaol_ieee1788::pow(x, y)`, `gaol::pow(x, y)`, `gaol::pow(x, p)` and `gaol_ieee1788::pow(x, p)` on 10 500 grid boxes, under the four rounding directions at the call, and on 60 000 random boxes |
| `diffpow.py` | Builds `diffpow.cpp` against two builds of GAOL or more, runs it and compares the outputs: identical lines, lines that differ only in the sign of zero bounds, lines that differ otherwise |
| `evalrows.cpp` | Computes `gaol_ieee1788::pow` and `gaol::pow` on the boxes of a rows file |
| `gen_table.py` | Reads the table of `pow_on_boxes()` into a rows file (`extract`), writes the table from a rows file (`cpp`, `--into` to replace it in `tests/ieee1788.cpp`), and lists the boxes whose bounds differ between two rows files (`diff`) |
| `checkrows.py` | Checks the bounds of a rows file against the exact power, computed with 500 bits by mpmath: they have to enclose it, within one double of the tightest bounds |
| `mutate.py` | Mutation testing: builds GAOL with one deliberate error in `pow` or in the integer power at a time (100 mutants), runs the tests, and runs `diffpow` on the mutants that survive, to tell an equivalent mutant from a box the tests lack |
| `pown_bound.py` | Checks with rationals the induction of the comment of `pown_products()`, which bounds beforehand the error of the exact products of the integer power, and the bound K drawn from it, for c<sup>m</sup> and c<sup>−m</sup> |
| `bench.cpp` | The time per call of the pow functions, on each path of `pow_standard()` |
| `build.py` | Compiles `diffpow.cpp`, `evalrows.cpp` or `bench.cpp` against a build of GAOL; the other scripts import it |

A rows file has one box per line, `X | Y | S | H`: the base X, the exponent Y,
S = `gaol_ieee1788::pow(X, Y)` and H = `gaol::pow(X, Y)`, each `empty` or two
numbers `l u` (decimal or hexadecimal, `inf`, `-inf`); the lines starting with
`#` are the comments of the table.

## What they need

- A C++ compiler and CMake, as GAOL does; `cmake --build` with the Unix
  Makefiles generator, the default on Linux and macOS, for `build.py` to take
  the link line of the tests (with another generator, it links with the
  libgaol of the build and the compiler of the compile command);
- Python 3.8 or later, and [mpmath](https://mpmath.org) for `checkrows.py`
  (mpmath 1.1 was used).

Every path is given on the command line: the builds of GAOL to `diffpow.py`
and `build.py`, the sources and the work directory to `mutate.py`
(`--sources`, by default `$GAOL_SOURCES` or the tree the script is in;
`--work`, by default `$GAOL_POW_MUTANTS` or `gaol-pow-mutants` in the
temporary directory, `/tmp/gaol-pow-mutants` on Linux). No file of the
sources is written, except by `gen_table.py cpp --into`.

## The builds of GAOL

Each build is a CMake build with the tests and the compile commands:

```bash
cmake -S . -B build-sse -DCMAKE_BUILD_TYPE=Release -DWITH_TESTS=ON -DCMAKE_EXPORT_COMPILE_COMMANDS=ON
cmake -S . -B build-fpu -DCMAKE_BUILD_TYPE=Release -DWITH_TESTS=ON -DCMAKE_EXPORT_COMPILE_COMMANDS=ON -DGAOL_SIMD=OFF -DGAOL_ARM64=OFF
cmake --build build-sse -j 4
cmake --build build-fpu -j 4
```

`build-sse` has the SSE2 intervals, the default on x86 (the NEON ones on 64-bit
ARM, GAOL v5), and `build-fpu` the FPU ones (`-DGAOL_SIMD=OFF
-DGAOL_ARM64=OFF`: the second switch is for 64-bit ARM, and harmless on x86).
To compare a change with the code before it,
build the code before it the same way from a second tree, for instance
`git worktree add ../gaol-base configure-clean`, then
`cmake -S ../gaol-base -B ../gaol-base/build-sse ...`.

`build.py` compiles a program with the command `compile_commands.json` gives
for `tests/ieee1788.cpp` (the compiler, the include directories, C++17 and the
flags of interval arithmetic, `-frounding-math -fno-fast-math
-ffp-contract=off`, and `-msse2 -mfma`...) and links it with the line of
`tests/CMakeFiles/gaol_test_ieee1788.dir/link.txt`:

```bash
python3 tests/tools/pow/build.py --build build-sse -o /tmp/pow/sse/bench tests/tools/pow/bench.cpp
```

For an installed GAOL, `--pkg-config` takes the flags of `pkg-config --cflags
--libs gaol` (`PKG_CONFIG_PATH` finds `gaol.pc`), the compiler `$CXX` and the
flags `$CXXFLAGS` (`c++` and `-O2 -std=c++17` by default); `--cxx`,
`--cxxflags` and `--ldflags` give them all.

## A full run

The commands are run from the root of the sources; `/tmp/pow` holds the
programs and their outputs.

**1. The differential test.** Between the SSE2 and the FPU intervals, and
between the code before a change and after it:

```bash
python3 tests/tools/pow/diffpow.py run --out /tmp/pow/simd sse=build-sse fpu=build-fpu
python3 tests/tools/pow/diffpow.py run --out /tmp/pow/change base=../gaol-base/build-sse new=build-sse
python3 tests/tools/pow/diffpow.py compare /tmp/pow/simd/sse/diffpow.txt /tmp/pow/change/new/diffpow.txt
```

Each line of `diffpow.txt` is a box and its results in hexadecimal, the signs
of the zeros included. `diffpow.py` exits with 1 when two outputs differ by
more than the sign of zero bounds, or when a result depends on the rounding
direction at the call; it prints the first 20 lines that differ (`--show`).
B.1 changed no line; B.2 changed the lines of the 2 214 boxes it made
tighter (an infinite bound, or a base from 0 with an exponent that is not
above 0), which `compare` lists, and no other: every result it changed is
inside the former one, and the tightest one against mpmath.

**2. The table of `pow_on_boxes()`.** The expected bounds of the table, then
those a build computes, both checked against mpmath:

```bash
python3 tests/tools/pow/gen_table.py extract tests/ieee1788.cpp > /tmp/pow/expected.txt
python3 tests/tools/pow/checkrows.py /tmp/pow/expected.txt
python3 tests/tools/pow/build.py --build build-sse -o /tmp/pow/sse/evalrows tests/tools/pow/evalrows.cpp
/tmp/pow/sse/evalrows /tmp/pow/expected.txt > /tmp/pow/results.txt
python3 tests/tools/pow/checkrows.py /tmp/pow/results.txt
python3 tests/tools/pow/gen_table.py diff /tmp/pow/expected.txt /tmp/pow/results.txt
python3 tests/tools/pow/gen_table.py cpp /tmp/pow/results.txt --into tests/ieee1788.cpp
```

`checkrows.py` prints each result with its distance from the tightest bounds,
and the tightest bounds where it is not them (`--quiet`: only those and the
failures); `--max-distance` changes the limit of one double. `diff` lists the
boxes whose bounds changed, each bound tighter or looser by so many doubles.
`cpp --into` writes the table from the results, which `extract` followed by
`cpp` gives back as it is written. For new boxes, add lines `X | Y` (or C++ rows to the table, then
`extract`) and run `evalrows`, which fills in S and H; it also checks that
`gaol_ieee1788::pow(X, p)` and `gaol::pow(X, p)` give S and H for a degenerate
Y = [p], as the test does, and exits with 1 where they do not.

**3. Mutation testing.**

```bash
python3 tests/tools/pow/mutate.py --work /tmp/pow/mutants
python3 tests/tools/pow/mutate.py --work /tmp/pow/mutants --test ieee1788
python3 tests/tools/pow/mutate.py --work /tmp/pow/mutants-fpu --cmake-option=-DGAOL_SIMD=OFF --cmake-option=-DGAOL_ARM64=OFF corner gaol-
python3 tests/tools/pow/mutate.py --list
# the integer powers: the SSE2 build with FMA, then the FPU one
T="--test ieee1788 --test elementary --test arithmetic --test rounding_direction --test expressions"
python3 tests/tools/pow/mutate.py --work /tmp/pow/mutants $T std corner gaol real ieee pown sse-fma
python3 tests/tools/pow/mutate.py --work /tmp/pow/mutants-fpu --cmake-option=-DGAOL_SIMD=OFF --cmake-option=-DGAOL_ARM64=OFF $T std corner gaol real ieee pown fpu
```

The sources are copied to `WORK/src` and built in `WORK/build`; each mutant
edits the copy of `gaol/gaol_interval_core.cpp`, rebuilds the tests (`--jobs`
processes, 2 by default) and runs them: `ieee1788` and `elementary` by
default, the tests given with `--test` otherwise (`--test ieee1788` alone
shows what `pow_on_boxes()` catches; `arithmetic`, `rounding_direction`,
`core_math` and `other_functions` call `pow` too, and take a few seconds
more). A CMake option is written `--cmake-option=-DGAOL_SIMD=OFF`, with `=`; the FPU
build also needs `--cmake-option=-DGAOL_ARM64=OFF` on 64-bit ARM (GAOL v5).
A run of the 48 mutants takes about 5 minutes (4 jobs); the arguments
after the options select the mutants whose name contains one of them.

Each mutant is `KILLED` (with the number of failed checks of each name),
`expected to survive` (an equivalent mutant: the list says why its bounds are
the same, and `diffpow` finds the same bounds as without it), `SURVIVED` (no
test failed: `diffpow` prints the first box where the bounds differ from
those of the sources, a box the tests lack, or finds none, and the mutant is
equivalent but not listed as such), `NOT APPLIED` (the text it replaces is not
in the sources once: the list is out of date) or `COMPILE ERROR`. A mutant
listed as equivalent can be killed all the same by a test that sees more than
the bounds, or by a box of one build only: `gaol-infinite-guard-dropped`
by the exponents [+oo] and [-oo] that `elementary` builds from an SSE2
register.
`WORK/mutants/NAME/` keeps the diff, the build log and the outputs of the
tests and of `diffpow`. The exit status is 1 for a mutant that survives but
is not listed as equivalent, or that cannot be applied or compiled.

The mutants are the list `MUTANTS` of `mutate.py`, written for the code of
`configure-clean` on 2026-10-04 and brought up to date with the pull
requests of B (the corners and their limits of B.2 included) and with #183,
#184 and #191: a change of
`pow` that edits the code they replace has to bring their texts and the
equivalent mutants up to date too (`--list` tells which no longer apply), or
give another list with `--mutants FILE`.

The mutants of the integer power (GAOL v5, the third pull request of B) are
named `pown-*` where they edit the code of all the builds (the generic part,
`gaol/gaol_interval_core.cpp`), `sse-fma-*` where they edit the two lanes in
an SSE2 register (`gaol/gaol_interval_sse.cpp`), compiled with the SSE2
intervals and the FMA instructions only, `er-*` for the embedded rounding of
AVX-512 (`gaol/gaol_interval_avx512.cpp`), `neon-*` for the NEON register
(`gaol/gaol_interval_neon.cpp`, compiled on a 64-bit ARM processor only,
where they mirror the `sse-fma-*` ones, but for `sse-fma-exact-flag-as-doubles`,
which has no NEON twin, and `neon-sign-dropped`, which has no SSE2 one) and `fpu-*` where they edit the pair
of doubles of the other builds (`pown_scalar_ops::large()`; the tests
`ieee1788` and `elementary` kill `fpu-no-exact-flag`, `--test arithmetic`
kills `fpu-inverse-uncorrected` and `fpu-powers-of-c`): each family is run in
the build that compiles it, with the arguments `pown sse-fma`, `pown fpu` or
`pown neon` (above), a mutant of the other family surviving there, not being
compiled. The error bound K of the
exact products itself has no mutant: with K = 0, the bounds are those of
h + l rounded, tighter where c<sup>n</sup> is within K of a double, and no
test finds them wrong; `pown_bound.py` checks its proof instead. Its value 0
for the squares, whose h + l is exact, has one (`pown-square-bound`), which
x<sup>2</sup> one double beyond `sqr(x)` near 1 kills. In the FPU build,
`std-infinite-exponent-unchecked` survives, `diffpow` finding the same
bounds: the exponents [+oo] and [-oo] that kill it in the SSE2 build are
built from an SSE2 register, and no FPU interval holds them. On 13 October
2026 (Clang 21), the SSE2 build killed 64 of its 75 mutants, the 11 others
being listed as equivalent, and the FPU build 58 of its 70, 11 listed and
`std-infinite-exponent-unchecked`.

**4. The timings.** The same program linked with the two builds, run in turn
on a pinned processor of an idle machine, the best time of each row kept
(the program prints the best of seven runs; the loop alternates the builds so
that a change of the load affects both):

```bash
python3 tests/tools/pow/build.py --build ../gaol-base/build-sse -o /tmp/pow/base/bench tests/tools/pow/bench.cpp
python3 tests/tools/pow/build.py --build build-sse -o /tmp/pow/new/bench tests/tools/pow/bench.cpp
for i in 1 2 3 4 5 6; do
  for b in base new; do taskset -c 2 /tmp/pow/$b/bench > /tmp/pow/$b/bench-$i.txt; done
done
```

## What a passing run looks like

On `configure-clean` at `065191e` (GCC 9.4, x86-64), with the builds above,
before B.2 (the table had 92 boxes, `mutate.py` 40 mutants); since, the
table has 117 boxes, all but one the tightest, and the 48 mutants of B.2
are all killed but 7 equivalent ones:

```text
$ python3 tests/tools/pow/diffpow.py run --out /tmp/pow/simd sse=build-sse fpu=build-fpu
.../sse/diffpow.txt: 70500 lines (diffpow: GAOL 5.0.0, SSE2 intervals)
.../fpu/diffpow.txt: 70500 lines (diffpow: GAOL 5.0.0, FPU intervals)
identical: 69221
different in the sign of zero bounds only: 1279 (std 201, hyb 1279, real 1279, other rounding directions 0)
different otherwise: 0 (std 0, hyb 0, real 0, other rounding directions 0)
results that depend on the rounding direction at the call: 0 in the first, 0 in the second
```

The SSE2 intervals give a lower bound -0 where the FPU ones give +0 (integer
powers of an interval holding 0, and of a base whose lower bound is -0), which
no test checks: the two builds agree, the sign of a zero aside.

```text
$ python3 tests/tools/pow/checkrows.py /tmp/pow/expected.txt --quiet
box  82 std ok   0 2 | -1.5 -0.5 -> 0x1.6a09e667f3bcbp-2 inf: encloses, distance lower 1 upper 0, tightest 0x1.6a09e667f3bccp-2 inf
box  82 hyb ok   0 2 | -1.5 -0.5 -> 0x1.6a09e667f3bcbp-2 inf: encloses, distance lower 1 upper 0, tightest 0x1.6a09e667f3bccp-2 inf
box  86 std ok   1 3 | -inf 2 -> 0 0x1.2000000000001p+3: encloses, distance lower 0 upper 1, tightest 0x0p+0 0x1.2p+3
...
boxes: 92, results: 184, failed: 0; results by distance from the tightest bounds, in doubles: 0: 174, 1: 10
```

The five boxes one double from the tightest bounds (`{I(0, 2), I(-1.5,
-0.5)}`, `{I(1, 3), I(-oo, 2)}`, `{I(0.25, 0.5), I(-oo, -1)}`, `{I(2, 4), I(1,
oo)}`, `{I(4, oo), P(-0.5)}`, each for both functions) were those computed by
exp(y log x), which B.2 replaced by the corners and their limits: their
bounds are now the tightest ones, as those of the 117 boxes of the table but
one, an integer power (`{I(0, 0x1.0000000000001p+0), P(-2)}`, one double
below, (1 + 2<sup>−52</sup>)<sup>−2</sup> being within the bound K of the
error of the integer powers of a double; `{I(-2, 3), P(100)}`, one double
above before the third pull request of B, is the tightest now).

```text
$ python3 tests/tools/pow/mutate.py --work /tmp/pow/mutants
without a mutant: ieee1788 passes (6245 checks, 0 failed)
without a mutant: elementary passes (367525 checks, 0 failed)
std-empty-y-unchecked              expected to survive  an empty y has NaN bounds: ...; diffpow: the same bounds on its 70500 lines
std-no-cut-to-positive             KILLED               ieee1788 6245 checks, 26 failed (...)
...
mutants: 40 in 143 s; KILLED 33, expected to survive 7
```

The exit status is then 0. With `--test ieee1788` alone, two mutants
survived then, which `elementary` killed: `pow_on_boxes()` had no box with
an empty base and an integer exponent beyond the ints, nor a base [1] with
an exponent that is not a multiple of 2^-10; B.2 added both
(`{none, P(2147483648.0), none}` and `{P(1), P(0.1), P(1)}`).

```text
$ taskset -c 2 /tmp/pow/sse/bench
gaol::pow(x, [0.5,1.5]), x > 0                     110.83 ns
...
gaol::pow(x, 3), x > 0 (pown alone)                 14.70 ns
gaol_ieee1788::pow(x, [1e10]), x > 0                57.21 ns
gaol_ieee1788::pow(x, [0.5,1.5]), x = [a, +oo]      70.43 ns
...
```

(Intel Core i7-1185G7.) This run is from before B.2: the boxes with an
infinite bound or a base from 0 took exp(y log x) then, 70 to 75 ns where
two corners cost 90 to 110 ns; they take the corners now, where a limit costs
no call of CORE-MATH. With B.2, built with Clang 18 (`-falign-functions=64`,
the best of 8 alternated runs), `gaol_ieee1788::pow(x, [1,+oo])` takes 33 ns
rather than 80, `x = [0, b]` 64 rather than 69, `x = [a, +oo]` 70 rather
than 71; a base holding 1 inside takes one corner for each bound where the
exponent has one sign, rather than two, and a degenerate exponent one corner
for each bound whatever the base: `gaol::pow(x, [0.5,1.5])`, x > 0, 105 ns
rather than 115, and `gaol::pow(x, 0.75)` 103 rather than 117;
`gaol_ieee1788::pow(x, [1e10])` takes 68 ns rather than 63, the tests of
`pow_lo()` and `pow_hi()` at its corners (x = 1, the limits, the powers that
are doubles) being made where the block of the integers beyond the ints
called CORE-MATH at once.

## Limits

- The differential test compares builds with each other, not with the exact
  powers: two builds wrong in the same way agree. `checkrows.py` checks the
  boxes of a rows file only (the 117 of the table, or those `compare` lists).
  B.2 changed the bounds of 2 214 boxes of `diffpow`: each is inside the
  former one, and the tightest against `checkrows.py`.
- `checkrows.py` computes the powers with 500 bits, and takes an exact power
  within 2^-200 of a double for that double (a power it computes inexactly,
  such as 4^0.5); its model of `gaol::pow` is the one of
  `gaol/gaol_interval.h` (pown within the ints, [-oo, +oo] beyond them), and
  it does not check the sign of a zero.
- The rows file and the table do not tell -0 from +0, nor does the test; the
  differential test does.
- `mutate.py` knows the code of the pull requests of B, #183, #184 and #191;
  its texts have to follow the code. It edits one file at a time and runs only the tests
  given; `diffpow` tells an equivalent mutant on its boxes only.
- `bench.cpp` measures one machine and one compiler; the 2 to 3 % of #37 were
  measured with GCC 9.4 only.
- `build.py` reads the link line that the Unix Makefiles generator writes; the
  scripts were run on Linux x86-64 only (Python 3.8, mpmath 1.1, GCC 9.4).
