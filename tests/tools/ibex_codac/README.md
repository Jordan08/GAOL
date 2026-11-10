<!-- Copyright (c) 2026 ENSTA, France

     By: Jordan NININ (created 2026-11-02)

     GAOL is a software distributed WITHOUT ANY WARRANTY. Read the associated
     COPYING.LIB file for information. -->
# Tools of test_ibex_codac

Part of the [tests](../../../doc/tests.md) of GAOL v5.

`tests/test_ibex_codac.cpp` checks GAOL against the cases of the unit tests that
IBEX and Codac run on their intervals. Each case is a row of a table: the
intervals, and the result to find, which is the exact one, computed apart from
GAOL and from the two projects. The scripts of this directory hold the cases,
compute the exact results (exact rational arithmetic, and mpmath with 2000 bits
for the elementary functions, as `tests/elementary_values.py` does), write the
rows of the tables between the lines `// BEGIN <table>` and `// END <table>` of
the test, and check them. They are not part of the build nor of `make test`:
whoever changes a case, or finds the exact result wrong, runs them.

Each script is the source of the tables of one section of the test, and
`--check` ignores the tables of the other sections, so that the three can check
the same file. Python 3.9 or later (3.8 for the two scripts of IBEX), and
mpmath 1.1 or later, run as `python3 -I`.

| File | Section of the test | What it holds |
|---|---|---|
| `ibex_forward_cases.py` | `ibex_` | The forward tests of IBEX (`TestInterval.cpp`, `TestArith.cpp` of ibex-lib commit 65ed587): the constants, the constructors, the sets, the accessors and relations, the arithmetic, the elementary functions, `atan2`, `floor`, `ceil`, `integer`, the powers and roots |
| `ibexb_cases.py` | `ibexb_` | The backward contractors of IBEX (`bwd_*`), its generalized division `div2`, and its distances, written with the operations and the reverse functions of GAOL. It imports `ibex_forward_cases.py`, which has to be in the same directory |
| `codac_cases.py` | `codac_` | The tests of Codac (Codac 2 commit 6419940, and the branch GAOL of the fork of Jordan Ninin, commit f0d06ff0): `codac2_tests_Interval.cpp`, `codac2_tests_Interval_operations.cpp`, and the part of `codac2_tests_Interval_rounding.cpp` that the tests of GAOL do not have |

Each script says in its header what its exact results are and what is a
convention (the midpoint of an unbounded interval, `contains(+oo)`, the angle of
`atan2` on the half-line y = 0, x < 0...), and which tests of the project it
leaves out.

    python3 -I ibex_forward_cases.py                          the tables, on the standard output
    python3 -I ibex_forward_cases.py --write ../../test_ibex_codac.cpp    replaces its tables in the test
    python3 -I ibex_forward_cases.py --check ../../test_ibex_codac.cpp    every row is the one the cases give
    python3 -I ibex_forward_cases.py --originals [-v]         the disagreements of IBEX's expectations with the exact results
    python3 -I ibex_forward_cases.py --selftest               the oracle against sampling

`ibexb_cases.py` has the same options. `codac_cases.py` has `--check`, `--write`,
`--list` (the rows of every group), `--selftest`, `--elementary-values
../../elementary_values.h` (the points of the rounding test that the table of
`tests/elementary.cpp` already has) and `--quiet`. The exit status is 1 when
`--check` finds a row that differs, `--originals` a disagreement that is not
explained in the script, or `--selftest` a failure.

The expectations of IBEX and Codac, with their tolerances (1e-10 absolute for
IBEX, `Approx` for Codac), are in the scripts and are only compared with the
exact results: 658 comparisons for the forward tests of IBEX, 702 for the
backward ones, 580 for the tests of Codac, with no disagreement beyond the
tolerance of the project. Where an expectation is not an enclosure of the exact
result at an integer bound, the script says so (23 for the forward tests of
IBEX, for example `tan02`, which expects [-1, 1] where the exact result is ±(1 +
2^-52)).
