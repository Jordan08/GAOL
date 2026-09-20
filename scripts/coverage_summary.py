#!/usr/bin/env python3
"""The page of conclusions of the coverage report of GAOL (fork of GAOL).

Reads the JSON gcovr writes (coverage/coverage.json) and writes a Markdown page
(coverage/README.md) saying, file by file and for the whole:

- how many lines and branches the tests run, and how many they do not;
- whether the 80 % of lines the fork aims at is reached;
- what the lines never run are, so that the next tests know where to go.

GAOL's own sources (gaol/) are counted apart from those of CORE-MATH
(3rd/math-core): the accurate phases of the latter are reached by a handful of
arguments in a million, by design, so counting them with the rest would say
nothing of either.

    python3 scripts/coverage_summary.py coverage/coverage.json coverage/README.md
"""

# Copyright (c) 2026 ENSTA, France
#
# Created 2026-09-20 by Jordan NININ

import json
import os
import sys
from datetime import date

TARGET = 80.0


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


def main():
    if len(sys.argv) != 3:
        sys.stderr.write(__doc__)
        return 2
    source, target = sys.argv[1], sys.argv[2]
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
    out.append("# Coverage of the tests")
    out.append("")
    out.append("*Written by `scripts/coverage_summary.py` from the report of gcovr, "
               "which `cmake --build <build> --target coverage` runs "
               "(`-DGAOL_COVERAGE=ON`). Last run: %s.*" % date.today().isoformat())
    out.append("")
    out.append("The line-by-line report is [coverage.html](coverage.html), one page holding its own style.")
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
               "architectures this machine is not: they are covered by the comparison "
               "with the upstream sources instead (see `doc/tests.md`), not by counting "
               "lines.")
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
