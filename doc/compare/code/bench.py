#!/usr/bin/env python3
"""The benchmark of GAOL, libieeep1788, filib++, PROFIL/BIAS, Boost.Interval and Solaris Studio.

    bench.py data N FILE
        writes N random intervals of each kind the benchmark uses (see
        bench_common.h) into FILE: an int64 N, then for each kind the N lower
        bounds and the N upper bounds, as little-endian doubles
    bench.py report RESULTS.csv MACHINE.txt REPORT.md
        writes the tables of the results between the markers of REPORT.md (the
        whole file when it does not exist)
    bench.py lines-report RESULTS.csv MACHINE.txt REPORT.md
        writes the tables of the long lines (run_lines.sh) between their own
        markers at the end of REPORT.md
"""

# Copyright (c) 2026 ENSTA, France
#
# Created 2026-09-20 by Jordan NININ

import csv
import struct
import sys

SEED = 1788

# The operations, in the order of bench_ops.h, and what they compute
OPERATIONS = [
    ("add", "a + b"),
    ("sub", "a − b"),
    ("mul", "a × b"),
    ("div", "a / p"),
    ("sqr", "a² (`sqr`, `square` in Boost.Interval, `x**2` in Fortran)"),
    ("sqrt", "√p"),
    ("exp", "exp(a)"),
    ("log", "log(p)"),
    ("sin", "sin(a)"),
    ("cos", "cos(a)"),
    ("pow_int", "a³ (`pow(x, int)` in GAOL and Boost.Interval, `pown`, `power(x, int)` in filib++, `x**3`)"),
    ("pow_real", "p^e (`pow(x, y)`, `x**y`; exp(e log p) with Boost.Interval, which has no real power)"),
    ("line_arith", "(a + b)(a − b) / p"),
    ("line_trig", "sin(a) cos(b) + a²"),
    ("line_pow", "√p · a³ − exp(b / p)"),
    ("shekel5", "Shekel 5: −Σᵢ 1 / (Σⱼ (xⱼ − aᵢⱼ)² + cᵢ), 5 terms, 4 variables"),
    ("block5", "five lines: t1 = ab + p; t2 = sin(t1) cos(b); t3 = a² + t2/p; t4 = exp(t2) − b³; t3 t4 + √p"),
]

# The long lines of + - * / alone, in the order of bench_ops.h (run_lines.sh):
# the name, the expression, and what it tests. The number of operations is
# counted in the expression
LINES = [
    ("sum16", "a + b + p + e + s1 + s2 + s3 + s4 + a + b + p + e + s1 + s2 + s3 + s4",
     "additions, one after the other"),
    ("diff16", "a − b + p − e + s1 − s2 + s3 − s4 − a + b − p + e − s1 + s2 − s3 + s4",
     "additions and subtractions, one after the other"),
    ("mul8", "a · b · p · e · s1 · s2 · s3 · s4", "products, one after the other, of operands of both signs"),
    ("mul8_tree", "((a · b)(p · e))((s1 · s2)(s3 · s4))", "the same products in a tree"),
    ("quot8", "a / p / e / p / e / p / e / p", "quotients by intervals with no zero, one after the other"),
    ("mix13", "((((((a + b) · p − e) · s1 + s2) · p − s3) / e + a) · b − s4) / p + e",
     "the four operations, one after the other"),
    ("mix_tree", "((a + b)(p − e) + (s1 − s2)(s3 + s4)) / (p + e) + ((a − b)(s1 + s2) − (p · e) / (p + e)) / (e + p)",
     "the four operations, in a tree"),
    ("pos_sum16", "p + e + p + e + p + e + p + e + p + e + p + e + p + e + p + e",
     "additions of intervals of one sign, which every line repeats"),
    ("pos_mul8", "p · e · p · e · p · e · p · e", "products of intervals of one sign"),
    ("pos_quot8", "p / e / p / e / p / e / p / e", "quotients of intervals of one sign"),
]
LINE_LIBRARIES = ["double", "libieeep1788", "gaol5", "gaol", "filib", "profil", "boost"]


def line_operations(formula):
    """The number of operations of an expression of bench_ops.h: its binary operators"""
    return sum(formula.count(f" {op} ") for op in "+−/") + formula.count(" · ") + formula.count(")(")


# The reference of the ratios is GAOL V5.0.0, the version that bounds every
# elementary function with CORE-MATH; GAOL 4.2.3 is the last version of
# Frédéric Goualard, which GAOL V5.0.0 continues, run on the same intervals
REFERENCE = "gaol5"
LIBRARIES = [("double", "double (reference)"), ("libieeep1788", "libieeep1788"),
             ("gaol5", "GAOL V5.0.0"), ("gaol", "GAOL 4.2.3"),
             ("filib", "filib++"), ("solaris_f90", "Solaris Studio f90"), ("profil", "PROFIL/BIAS"),
             ("boost", "Boost.Interval")]


def data(n, path):
    import numpy
    rng = numpy.random.RandomState(SEED)

    def intervals(cmin, cmax, wmin, wmax):
        # Centres uniform in [cmin, cmax], widths 10^u with u uniform in [wmin, wmax]
        c = rng.uniform(cmin, cmax, n)
        w = 10.0 ** rng.uniform(wmin, wmax, n)
        return c - w / 2, c + w / 2

    kinds = [intervals(-10, 10, -6, 0),   # a
             intervals(-10, 10, -6, 0),   # b
             intervals(1, 10, -6, 0),     # p, positive
             intervals(0.5, 2.5, -6, -1)]  # e, exponents
    kinds += [intervals(0, 10, -6, 0) for _ in range(4)]  # Shekel 5
    with open(path, "wb") as f:
        f.write(struct.pack("<q", n))
        for lo, hi in kinds:
            assert (lo <= hi).all()
            f.write(lo.astype("<f8").tobytes())
            f.write(hi.astype("<f8").tobytes())


def read_results(path):
    results = {}
    with open(path) as f:
        for row in csv.reader(f):
            if len(row) != 8:
                continue
            lib, op, n, repeats, best, ns, sum_mid, sum_wid = [x.strip() for x in row]
            r = {"n": int(n), "repeats": int(repeats), "rounds": 1, "best": float(best), "ns": float(ns),
                 "sum_mid": float(sum_mid), "sum_wid": float(sum_wid)}
            # The best time of all the rounds
            old = results.get((lib, op))
            if old:
                r["repeats"] += old["repeats"]
                r["rounds"] += old["rounds"]
                if old["best"] < r["best"]:
                    r["best"], r["ns"] = old["best"], old["ns"]
            results[(lib, op)] = r
    return results


def ns(value):
    if value >= 1000:
        return f"{value:,.0f}".replace(",", " ")
    if value >= 100:
        return f"{value:.0f}"
    if value >= 10:
        return f"{value:.1f}"
    return f"{value:.2f}"


def ratio(x, y):
    return f"{x / y:.1f}" if x / y < 100 else f"{x / y:.0f}"


BEGIN = "<!-- BEGIN GENERATED TABLES (doc/compare/code/bench.py) -->"
END = "<!-- END GENERATED TABLES -->"


def report(results_path, machine_path, out):
    results = read_results(results_path)
    with open(machine_path) as f:
        machine = f.read().strip()
    libs = [(key, name) for key, name in LIBRARIES if any(k[0] == key for k in results)]
    ops = [(op, what) for op, what in OPERATIONS if any(k[1] == op for k in results)]
    lines = []
    lines.append("Measured on:\n\n```\n" + machine + "\n```\n\n")

    n = max(r["n"] for r in results.values())
    runs = {key: max((r["repeats"] for k, r in results.items() if k[0] == key), default=0) for key, _ in libs}
    rounds = max(r["rounds"] for r in results.values())
    lines.append(f"{n:,}".replace(",", " ") + " operations of each kind, on the same intervals. Each time is "
                 f"the best of {rounds} rounds, each program being run in turn with the others: "
                 + ", ".join(f"{runs[key]} runs for {name}" for key, name in libs) + " in all.\n\n")

    lines.append("#### The operations\n\n")
    lines.append("a and b are intervals centred in [−10, 10], p in [1, 10], e in [0.5, 2.5], the arguments of "
                 "Shekel 5 in [0, 10]; their widths are 10^u, u uniform in [−6, 0] ([−6, −1] for e).\n\n")
    lines.append("| Operation | Computes |\n|---|---|\n")
    for op, what in ops:
        lines.append(f"| `{op}` | {what} |\n")
    lines.append("\n")

    lines.append("#### Time per operation (nanoseconds)\n\n")
    head = "| Operation | " + " | ".join(name for _, name in libs)
    reference = REFERENCE if any(k[0] == REFERENCE for k in results) else "gaol"
    reference_name = dict(LIBRARIES)[reference]
    ratios = [(key, name) for key, name in libs if key not in ("double", reference)]
    head += "".join(f" | {name} / {reference_name}" for _, name in ratios) + " |\n"
    lines.append(head)
    lines.append("|---" * (1 + len(libs) + len(ratios)) + "|\n")
    for op, _ in ops:
        cells = []
        for key, _ in libs:
            r = results.get((key, op))
            cells.append(ns(r["ns"]) if r else "—")
        g = results.get((reference, op))
        for key, _ in ratios:
            r = results.get((key, op))
            cells.append(ratio(r["ns"], g["ns"]) if r and g else "—")
        lines.append(f"| `{op}` | " + " | ".join(cells) + " |\n")
    lines.append("\n")

    lines.append("#### Total time of the " + f"{n:,}".replace(",", " ") + " operations (seconds)\n\n")
    lines.append("| Operation | " + " | ".join(name for _, name in libs) + " |\n")
    lines.append("|---" * (1 + len(libs)) + "|\n")
    for op, _ in ops:
        cells = []
        for key, _ in libs:
            r = results.get((key, op))
            cells.append(f"{r['best']:.3f}" if r else "—")
        lines.append(f"| `{op}` | " + " | ".join(cells) + " |\n")
    lines.append("\n")

    lines.append("#### Same results?\n\n")
    lines.append("The sum of the midpoints of the results, relative to libieeep1788's, and the mean width of the "
                 "results, with the relative excess of the other libraries over libieeep1788, whose bounds are "
                 "the tightest:\n\n")
    ref = "libieeep1788"
    others = [(key, name) for key, name in libs if key not in ("double", ref)]
    lines.append("| Operation | Σ midpoints (libieeep1788) | " + " | ".join(f"Δ {name}" for _, name in others)
                 + " | mean width (libieeep1788) | " + " | ".join(f"excess {name}" for _, name in others) + " |\n")
    lines.append("|---" * (3 + 2 * len(others)) + "|\n")
    for op, _ in ops:
        p = results.get((ref, op))
        if not p:
            continue
        mid_cells, wid_cells = [], []
        for key, _ in others:
            r = results.get((key, op))
            mid_cells.append(f"{(r['sum_mid'] - p['sum_mid']) / abs(p['sum_mid']):.1e}" if r else "—")
            wid_cells.append(f"{(r['sum_wid'] - p['sum_wid']) / p['sum_wid']:.1e}" if r else "—")
        lines.append(f"| `{op}` | {p['sum_mid']:.10g} | " + " | ".join(mid_cells)
                     + f" | {p['sum_wid'] / p['n']:.6g} | " + " | ".join(wid_cells) + " |\n")
    lines.append("\n")

    table = "".join(lines)
    block = BEGIN + "\n\n" + table + END + "\n"
    try:
        with open(out) as f:
            content = f.read()
    except FileNotFoundError:
        content = ""
    if BEGIN in content and END in content:
        head, rest = content.split(BEGIN, 1)
        tail = rest.split(END, 1)[1]
        content = head + block + tail.lstrip("\n")
    else:
        content = block
    with open(out, "w") as f:
        f.write(content)


BEGIN_LINES = "<!-- BEGIN GENERATED LINES (doc/compare/code/bench.py) -->"
END_LINES = "<!-- END GENERATED LINES -->"


def lines_report(results_path, machine_path, out):
    results = read_results(results_path)
    with open(machine_path) as f:
        machine = f.read().strip()
    names = dict(LIBRARIES)
    libs = [(key, names[key]) for key in LINE_LIBRARIES if any(k[0] == key for k in results)]
    lines = [(name, formula, what, line_operations(formula)) for name, formula, what in LINES
             if any(k[1] == name for k in results)]
    text = []
    text.append("Measured on:\n\n```\n" + machine + "\n```\n\n")
    n = max(r["n"] for r in results.values())
    runs = {key: max((r["repeats"] for k, r in results.items() if k[0] == key), default=0) for key, _ in libs}
    rounds = max(r["rounds"] for r in results.values())
    text.append(f"{n:,}".replace(",", " ") + " evaluations of each line, on the same intervals. Each time is the "
                f"best of {rounds} rounds, each program being run in turn with the others: "
                + ", ".join(f"{runs[key]} runs for {name}" for key, name in libs) + " in all.\n\n")

    text.append("#### The lines\n\n")
    text.append("a and b are intervals centred in [−10, 10], p in [1, 10], e in [0.5, 2.5], s1 to s4 in [0, 10]; "
                "their widths are 10^u, u uniform in [−6, 0] ([−6, −1] for e), the intervals of the table above.\n\n")
    text.append("| Line | Computes | Operations | What it shows |\n|---|---|---:|---|\n")
    for name, formula, what, ops in lines:
        text.append(f"| `{name}` | {formula} | {ops} | {what} |\n")
    text.append("\n")

    reference = REFERENCE if any(k[0] == REFERENCE for k in results) else "gaol"
    reference_name = names[reference]
    ratios = [(key, name) for key, name in libs if key not in ("double", reference)]

    text.append("#### Time per evaluation of a line (nanoseconds)\n\n")
    text.append("| Line | " + " | ".join(name for _, name in libs)
                + "".join(f" | {name} / {reference_name}" for _, name in ratios) + " |\n")
    text.append("|---" * (1 + len(libs) + len(ratios)) + "|\n")
    for name, _, _, _ in lines:
        cells = []
        for key, _ in libs:
            r = results.get((key, name))
            cells.append(ns(r["ns"]) if r else "—")
        g = results.get((reference, name))
        for key, _ in ratios:
            r = results.get((key, name))
            cells.append(ratio(r["ns"], g["ns"]) if r and g else "—")
        text.append(f"| `{name}` | " + " | ".join(cells) + " |\n")
    text.append("\n")

    text.append("#### Time per operation of a line (nanoseconds)\n\n")
    text.append("The time of the line divided by its number of operations.\n\n")
    text.append("| Line | " + " | ".join(name for _, name in libs) + " |\n")
    text.append("|---" * (1 + len(libs)) + "|\n")
    for name, _, _, ops in lines:
        cells = []
        for key, _ in libs:
            r = results.get((key, name))
            cells.append(ns(r["ns"] / ops) if r else "—")
        text.append(f"| `{name}` | " + " | ".join(cells) + " |\n")
    text.append("\n")

    text.append("#### Same results?\n\n")
    text.append("The sum of the midpoints of the results, relative to libieeep1788's, and the mean width of the "
                "results, with the relative excess of the other libraries over libieeep1788, whose bounds are "
                "the tightest:\n\n")
    ref = "libieeep1788"
    others = [(key, name) for key, name in libs if key not in ("double", ref)]
    if any(k[0] == ref for k in results):
        text.append("| Line | Σ midpoints (libieeep1788) | " + " | ".join(f"Δ {name}" for _, name in others)
                    + " | mean width (libieeep1788) | " + " | ".join(f"excess {name}" for _, name in others) + " |\n")
        text.append("|---" * (3 + 2 * len(others)) + "|\n")
        for name, _, _, _ in lines:
            p = results.get((ref, name))
            if not p:
                continue
            mid_cells, wid_cells = [], []
            for key, _ in others:
                r = results.get((key, name))
                mid_cells.append(f"{(r['sum_mid'] - p['sum_mid']) / abs(p['sum_mid']):.1e}" if r else "—")
                wid_cells.append(f"{(r['sum_wid'] - p['sum_wid']) / p['sum_wid']:.1e}" if r else "—")
            text.append(f"| `{name}` | {p['sum_mid']:.10g} | " + " | ".join(mid_cells)
                        + f" | {p['sum_wid'] / p['n']:.6g} | " + " | ".join(wid_cells) + " |\n")
        text.append("\n")

    block = BEGIN_LINES + "\n\n" + "".join(text) + END_LINES + "\n"
    try:
        with open(out) as f:
            content = f.read()
    except FileNotFoundError:
        content = ""
    if BEGIN_LINES in content and END_LINES in content:
        head, rest = content.split(BEGIN_LINES, 1)
        tail = rest.split(END_LINES, 1)[1]
        content = head + block + tail.lstrip("\n")
    else:
        content = content.rstrip("\n") + "\n\n## Long lines of arithmetic operators\n\n" + block
    with open(out, "w") as f:
        f.write(content)


if __name__ == "__main__":
    if len(sys.argv) == 4 and sys.argv[1] == "data":
        data(int(sys.argv[2]), sys.argv[3])
    elif len(sys.argv) == 5 and sys.argv[1] == "report":
        report(sys.argv[2], sys.argv[3], sys.argv[4])
    elif len(sys.argv) == 5 and sys.argv[1] == "lines-report":
        lines_report(sys.argv[2], sys.argv[3], sys.argv[4])
    else:
        sys.exit(__doc__)
