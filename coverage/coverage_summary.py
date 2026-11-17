#!/usr/bin/env python3
"""The page of conclusions of the coverage report of GAOL (GAOL v5).

Reads the JSON gcovr writes (coverage/coverage.json) and writes a Markdown page
(coverage/README.md) saying, file by file and for the whole:

- how many lines and branches the tests run, and how many they do not;
- whether the 80 % of lines the fork aims at is reached;
- what the lines never run are, so that the next tests know where to go.

GAOL's own sources (gaol/) are counted apart from those of CORE-MATH
(3rd/math-core): the accurate phases of the latter are reached by a handful of
arguments in a million, by design, so counting them with the rest would say
nothing of either.

    python3 coverage/coverage_summary.py coverage/coverage.json coverage/README.md

The intervals are computed by one set of files per processor family, chosen by
the configuration: SSE2 on x86-64, NEON on 64-bit ARM, the FPU elsewhere or with
GAOL_SIMD=OFF (and the AVX-512 path of x86-64 on request). One run compiles one
family only, so the page has a section for the files of each family, and
several runs can be merged first, with gcovr --add-tracefile, into the JSON this
script reads; each --run adds a line saying what a run was:

    python3 coverage/coverage_summary.py merged.json coverage/README.md \\
        --run "SSE: x86-64, GCC 15.2" --run "FPU: x86-64, -DGAOL_SIMD=OFF"
"""

# Copyright (c) 2026 ENSTA, France
#
# By: Jordan NININ (created 2026-09-20)
#
# GAOL is a software distributed WITHOUT ANY WARRANTY. Read the associated
# COPYING.LIB file for information.

import argparse
import json
import os
import sys
from datetime import date

TARGET = 80.0

# The files of the intervals of each processor family: a run compiles the files
# of one family, so a page made of one run shows one of these.
FAMILIES = [
    ("SSE (x86-64)", "gaol/core/gaol_interval_sse"),
    ("AVX-512 (x86-64)", "gaol/core/gaol_interval_avx512"),
    ("NEON (64-bit ARM)", "gaol/core/gaol_interval_neon"),
    ("FPU", "gaol/core/gaol_interval_fpu"),
]


def percent(covered, total):
    return 100.0 * covered / total if total else 100.0


def bar(p):
    """A bar of twenty cells, readable in a terminal as in a browser."""
    full = int(round(p / 5.0))
    return "█" * full + "░" * (20 - full)


def counts(f):
    lines = f.get("lines", [])
    runnable = [l for l in lines if l.get("gcovr/noncode") is not True]
    covered = [l for l in runnable if l.get("count", 0) > 0]
    branches = [b for l in runnable for b in l.get("branches", [])]
    taken = [b for b in branches if b.get("count", 0) > 0]
    missed = sorted(l["line_number"] for l in runnable if l.get("count", 0) == 0)
    return len(runnable), len(covered), len(branches), len(taken), missed


def ranges(numbers):
    """The line numbers as ranges: 12, 15-19, 31."""
    out, start, previous = [], None, None
    for n in numbers:
        if start is None:
            start = previous = n
        elif n == previous + 1:
            previous = n
        else:
            out.append((start, previous))
            start = previous = n
    if start is not None:
        out.append((start, previous))
    return ", ".join(str(a) if a == b else "%d-%d" % (a, b) for a, b in out)


def group_of(path):
    if path.startswith("3rd/"):
        return "CORE-MATH"
    return "GAOL"


def table(rows):
    out = ["| File | Lines | Covered | % | Branches taken |",
           "|---|---:|---:|---:|---:|"]
    for path, runnable, covered, branches, taken, _ in rows:
        out.append("| `%s` | %d | %d | %.1f | %s |"
                   % (path, runnable, covered, percent(covered, runnable),
                      "%.1f %%" % percent(taken, branches) if branches else "--"))
    return "\n".join(out)


def totals(rows):
    runnable = sum(r[1] for r in rows)
    covered = sum(r[2] for r in rows)
    branches = sum(r[3] for r in rows)
    taken = sum(r[4] for r in rows)
    return runnable, covered, branches, taken


def families_table(rows):
    """The rows of GAOL by processor family, with the total of each family."""
    out = ["| Family | File | Lines | Covered | % | Branches taken |",
           "|---|---|---:|---:|---:|---:|"]
    found = False
    for name, prefix in FAMILIES:
        members = sorted((r for r in rows if r[0].startswith(prefix)),
                         key=lambda r: r[0])
        if not members:
            continue
        found = True
        for path, runnable, covered, branches, taken, _ in members:
            out.append("| %s | `%s` | %d | %d | %.1f | %s |"
                       % (name, path, runnable, covered,
                          percent(covered, runnable),
                          "%.1f %%" % percent(taken, branches) if branches else "--"))
        runnable, covered, branches, taken = totals(members)
        out.append("| | **%s** | %d | %d | **%.1f** | %s |"
                   % (name, runnable, covered, percent(covered, runnable),
                      "%.1f %%" % percent(taken, branches) if branches else "--"))
    return "\n".join(out) if found else None


def main():
    parser = argparse.ArgumentParser(description="Page of conclusions of the coverage of GAOL")
    parser.add_argument("source", help="the JSON of gcovr")
    parser.add_argument("target", help="the Markdown page to write")
    parser.add_argument("--run", action="append", default=[], metavar="TEXT",
                        help="a run the JSON merges, one line of the page; repeatable")
    args = parser.parse_args()
    source, target, runs = args.source, args.target, args.run
    with open(source) as f:
        data = json.load(f)

    rows = {"GAOL": [], "CORE-MATH": []}
    for entry in data.get("files", []):
        path = entry.get("file", "")
        runnable, covered, branches, taken, missed = counts(entry)
        if runnable == 0:
            continue
        rows[group_of(path)].append((path, runnable, covered, branches, taken, missed))
    for group in rows:
        rows[group].sort(key=lambda r: percent(r[2], r[1]))

    gaol_runnable, gaol_covered, gaol_branches, gaol_taken = totals(rows["GAOL"])
    gaol_percent = percent(gaol_covered, gaol_runnable)
    reached = gaol_percent >= TARGET

    out = []
    out.append("<!-- Copyright (c) 2026 ENSTA, France")
    out.append("")
    out.append("     By: Jordan NININ (created 2026-09-20)")
    out.append("")
    out.append("     GAOL is a software distributed WITHOUT ANY WARRANTY. Read the associated")
    out.append("     COPYING.LIB file for information. -->")
    out.append("# Coverage of the tests")
    out.append("")
    out.append("*Written by `coverage/coverage_summary.py` from the report of gcovr, "
               "which `cmake --build <build> --target coverage` runs "
               "(`-DGAOL_COVERAGE=ON`). Last run: %s.*" % date.today().isoformat())
    out.append("")
    out.append("The line-by-line report is [coverage.html](coverage.html), one page holding its own style.")
    out.append("")
    if runs:
        out.append("## The runs this page merges")
        out.append("")
        out.append("A run compiles the files of one processor family and the tests run the code "
                   "of that family only. The runs below are merged (`gcovr --add-tracefile`), so "
                   "that a line is covered if one of them runs it, and each file of the "
                   "intervals is counted in the run that compiles it.")
        out.append("")
        for run in runs:
            out.append("- " + run)
        out.append("")
        out.append("Each run is `cmake -S . -B <build> -DCMAKE_BUILD_TYPE=Debug -DWITH_TESTS=ON "
                   "-DWITH_EXAMPLES=ON -DGAOL_COVERAGE=ON` (with `-DGAOL_SIMD=OFF` for the FPU) and "
                   "`cmake --build <build> --target coverage`, which leaves `<build>/coverage.json`. "
                   "The merge is `gcovr --root . --merge-mode-functions=merge-use-line-min "
                   "--filter \"$PWD/gaol/\" --filter \"$PWD/3rd/math-core/\" "
                   "--add-tracefile <run1>.json --add-tracefile <run2>.json ... "
                   "--html --html-self-contained --html-title \"Coverage of GAOL\" "
                   "--output coverage/coverage.html --json merged.json`, and this page is "
                   "`python3 coverage/coverage_summary.py merged.json coverage/README.md --run ...`.")
        out.append("")
    out.append("## Conclusion")
    out.append("")
    out.append("| | Lines | Covered | Not run | % | |")
    out.append("|---|---:|---:|---:|---:|---|")
    out.append("| **GAOL (`gaol/`)** | %d | %d | %d | **%.1f** | `%s` |"
               % (gaol_runnable, gaol_covered, gaol_runnable - gaol_covered,
                  gaol_percent, bar(gaol_percent)))
    cm_runnable, cm_covered, cm_branches, cm_taken = totals(rows["CORE-MATH"])
    if cm_runnable:
        cm_percent = percent(cm_covered, cm_runnable)
        out.append("| CORE-MATH (`3rd/math-core/`) | %d | %d | %d | %.1f | `%s` |"
                   % (cm_runnable, cm_covered, cm_runnable - cm_covered,
                      cm_percent, bar(cm_percent)))
    out.append("")
    if reached:
        out.append("**The tests run %.1f %% of the lines of GAOL, above the %.0f %% "
                   "GAOL v5 aims at.**" % (gaol_percent, TARGET))
    else:
        out.append("**The tests run %.1f %% of the lines of GAOL, below the %.0f %% "
                   "GAOL v5 aims at**: the files at the top of the table below are "
                   "where the next tests are worth writing." % (gaol_percent, TARGET))
    out.append("")
    out.append("Branches: %.1f %% of those of GAOL are taken both ways."
               % percent(gaol_taken, gaol_branches))
    out.append("")
    out.append("The lines of CORE-MATH that are never run are its accurate phases, "
               "which a handful of arguments in a million reach, and the paths of the "
               "architectures %s not. They are counted among the lines not run, and are checked otherwise: the accurate "
               "path of `sin` is compared with the upstream sources compiled unchanged, "
               "and that of `cos` with its roundings computed by mpmath (see "
               "`doc/tests.md`); the other accurate phases are reached by the tests of "
               "their bounds only." % ("the runs are" if runs else "this machine is"))
    out.append("")
    family_table = families_table(rows["GAOL"])
    if family_table:
        out.append("## The intervals of each processor family")
        out.append("")
        out.append("The configuration chooses the files that compute the intervals: SSE2 on x86-64, "
                   "NEON on 64-bit ARM, the FPU elsewhere and with `-DGAOL_SIMD=OFF`, and the "
                   "AVX-512 path of x86-64 where it is asked for. A run compiles one family, so "
                   "the files of the others are in the page only if a run of them was merged.")
        out.append("")
        out.append(family_table)
        out.append("")
        absent = [name for name, prefix in FAMILIES
                  if runs and not any(r[0].startswith(prefix) for r in rows["GAOL"])]
        if absent:
            out.append("No merged run compiled the files of: %s." % ", ".join(absent))
            out.append("")
    out.append("## GAOL, file by file")
    out.append("")
    out.append("Sorted by coverage, least covered first.")
    out.append("")
    out.append(table(rows["GAOL"]))
    out.append("")
    out.append("## The lines the tests never run")
    out.append("")
    any_missed = False
    for path, runnable, covered, branches, taken, missed in rows["GAOL"]:
        if missed:
            any_missed = True
            out.append("- `%s`: %s" % (path, ranges(missed)))
    if not any_missed:
        out.append("None: the tests run every line of GAOL.")
    out.append("")
    if rows["CORE-MATH"]:
        out.append("## CORE-MATH, file by file")
        out.append("")
        out.append(table(rows["CORE-MATH"]))
        out.append("")

    os.makedirs(os.path.dirname(os.path.abspath(target)), exist_ok=True)
    with open(target, "w") as f:
        f.write("\n".join(out) + "\n")
    print("Coverage of GAOL: %.1f %% of %d lines (%s the %.0f %% aimed at), "
          "written in %s" % (gaol_percent, gaol_runnable,
                             "above" if reached else "below", TARGET, target))
    return 0


if __name__ == "__main__":
    sys.exit(main())
